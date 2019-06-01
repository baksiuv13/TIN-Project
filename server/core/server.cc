// Copyright 2019 TIN

#include "core/server.h"

#include <unistd.h>

#include <set>
#include <vector>
#include <iostream>
#include <cstring>
#include <ios>
#include <utility>

#include "core/socket_tcp4.h"
#include "core/nquad.h"
#include "core/mquads.h"
#include "core/logger.h"

#include "send_msgs/sig.h"
#include "send_msgs/log_ok.h"
#include "send_msgs/log_off.h"
#include "send_msgs/user_msg.h"

static constexpr int STDIN_FD = STDIN_FILENO;

namespace {

class TempIO {
 public:
  explicit TempIO(std::ios *ios)
    : ios_(ios), ios_state_(nullptr) {
    ios_state_.copyfmt(*ios);
  }
  ~TempIO() {
    ios_->copyfmt(ios_state_);
  }
 private:
  std::ios *ios_;
  std::ios ios_state_;
};
}  // namespace

namespace tin {
Server::Server()
    : runs_(false), next_sock_id_(1) {
  if (pipe(end_pipe_)) {
    LogH << "Serwer się nie zrobił bo potok się nie otworzył :< "
      << std::strerror(errno) << '\n';
    throw std::runtime_error("Server couldn't create a pipe.");
  }
  return;
}
Server::~Server() {
  close(end_pipe_[0]);
  close(end_pipe_[1]);
}

int Server::InitializeListener_(uint16_t port, int queue_size) {
  int result = 0;
  result = listening_sock_.Open();
  if (result < 0) return -1;
  result = listening_sock_.BindAny(port);
  if (result < 0) return -2;
  result = listening_sock_.Listen(queue_size);
  if (result < 0) return -3;
  LogL << "Numer gniazda słuchającego: " << listening_sock_.GetFD()
    << '\n';
  return 0;
}

void Server::Run(uint16_t port, int queue_size) {
  LogM << "Port: " << port << '\n';
  int init_ret = InitializeListener_(port, queue_size);
  if (init_ret < 0) {
    LogH << "Nie udało się zainicjalizować listenera.\n";
    return;
  }
  bool not_exit = true;
  runs_ = true;
  while (not_exit && runs_) {
    LoopTick_();
  }
}

int Server::FeedSel_() {
  sel_.Zero();
  if (DEAL_WITH_STDIN) {
    sel_.AddFD(STDIN_FD, Sel::READ);
  }
  sel_.AddFD(end_pipe_[0], Sel::READ);
  sel_.AddFD(listening_sock_.GetFD(), Sel::READ);
  for (auto sock_it = client_socks_.begin();
      sock_it != client_socks_.end();
      ++sock_it) {
    if (sock_it->second.ShallRead()) {
      sel_.AddFD(sock_it->second.GetSocket().GetFD(), Sel::READ);
    }
    if (sock_it->second.ShallWrite()) {
      sel_.AddFD(sock_it->second.GetSocket().GetFD(), Sel::WRITE);
    }
  }
  return 0;
}

int Server::RegisterSockFromAccept_(SocketTCP4 &&sock) {
  SockId id = NextSockId_();
  auto emplace_ret = client_socks_.emplace(id,
    SocketStuff {this, id, std::move(sock)});
  if (!emplace_ret.second) {
    LogH << "Nie udało się dodać socketu do mapy z socketami :<\n"
      "id: " << id
      << " trzeba coś z tym zrobić.\n";
    return -1;
  }
  emplace_ret.first->second.SetReading(true);
  return 0;
}

int Server::DoSel_() {
  return sel_.Select();
}

int Server::ReadMainFds_() {
  static constexpr std::streamsize IN_BUF_SIZE = 256;
  char in_buf[IN_BUF_SIZE];
  if (Sel::READ & sel_.Get(end_pipe_[0])) {
    LogH << "Przyszło zamknięcie ze specjanego potoku.\n";
    runs_ = false;
    return 1;
  }
  if (DEAL_WITH_STDIN && (Sel::READ & sel_.Get(STDIN_FD))) {
    std::cin.getline(in_buf, IN_BUF_SIZE);
    if (std::cin.eof()) {
      LogH << "Na stdin przyszedł koniec, zamykanko :>\n";
      runs_ = false;
    } else if (std::cin.good()) {
      LogM << "Na stdin wpisano:\n" << in_buf << '\n';
      DealWithStdinBuf_(in_buf);
    } else if (std::cin.bad()) {
      LogH << "Przyszło coś na stdin, ale coś było nie tak, jak trzeba, "
        << " pomijam.\n";
    }
    std::cin.clear();
    return 1;
  }
  if (Sel::READ & sel_.Get(listening_sock_.GetFD())) {
    LogH << "Nowe połącznoko :>\n";
    SocketTCP4 sock;
    int acc_ret = listening_sock_.Accept(&sock);
    if (acc_ret < 0) {
      LogH << "Jakiś błąd przy accepcie :<\n";
    } else {
      RegisterSockFromAccept_(std::move(sock));
    }
  }
  return 0;
}

int Server::DoWorldWork_() {
  Username un;
  const std::string *str_ptr;
  while (world_.NextMsg(&un, &str_ptr) == 0) {
    PushMsg_(OutMessage::UP(new UserMsg(un, *str_ptr)));
  }
  return 0;
}

Username Server::SockToUn(SockId id) {
  try {
    return socks_to_users_.at(id)->GetName();
  } catch(std::out_of_range &e) {
    return Username();
  }
}

int Server::DealWithStdinBuf_(const char *s) {
  if (strncmp(s, "stop", 4) == 0 && (s[4] == '\n' || s[4] == ' ')) {
    StopRun();
  }
  return 0;
}

int Server::StopRun() {
  return write(end_pipe_[1], "", 1);
}

int Server::WriteToSocks_() {
  static constexpr int WRT_BUF = WriteBuf::SIZE;
  char buf[WRT_BUF];
  int sockets_written = 0;
  for (auto it = client_socks_.begin(); it != client_socks_.end(); ++it) {
    if (!(it->second.ShallWrite())
        || !(Sel::WRITE & sel_.Get(it->second.GetSocket().GetFD()))) {
      continue;
    }
    WriteBuf &swb = it->second.WrBuf();
    int loaded_from_buf = swb.Get(buf, WRT_BUF);
    if (loaded_from_buf < 0) {
      // Tutaj mamy obsługę gniazda w tak złym stanie, że musimy je zamknąć.
      it->second.ForceRemove();
      continue;
    }
    int written = write(it->second.GetSocket().GetFD(), buf, loaded_from_buf);
    if (written < 0) {
      // Tutaj też jest źle.
      it->second.ForceRemove();
      continue;
    }
    if (written == 0) {
      // chyba zamknięte
      LogVL << "gniazdo zamknięte??\n";
      it->second.Remove();
    }
    swb.Pop(written);
    ++sockets_written;
  }
  return sockets_written;
}

int Server::MsgsToBufs_() {
  OutMessage *msg = nullptr;
  int added_to_buf;
  while ((msg = FirstMsg_()) != nullptr) {
    switch (msg->Audience()) {
      case OutMessage::BROADCAST_S:
        LogM << "OutMessage::BROADCAST_S\n";
        for (auto &x : client_socks_) {
          added_to_buf = msg->AddToBuf(&x.second.WrBuf());
          if (added_to_buf < 0) {
            LogH << "Błąd przy dodawaniu wiadomości do bufora. :<\n" <<
              "Socket o id " << x.first << " i fd " <<
              x.second.GetSocket().GetFD() << '\n';
            if (socks_to_users_.count(x.first) > 0) {
              LogM << "Jest na nim zalogowany user [[" <<
                socks_to_users_.at(x.first)->GetName() << "]]\n";
            } else {
              LogM << "Niezalogowane gniazdo :>\n";
            }
            x.second.ForceRemove();
            // Force, bo już i tak nic nie napiszemy raczej
          }
        }
        break;
      case OutMessage::BROADCAST_U:
        LogM << "OutMessage::BROADCAST_U\n";
        for (auto &x : users_) {
          SocketStuff *stuff = &client_socks_.at(x.second.GetSockId());
          added_to_buf = msg->AddToBuf(&stuff->WrBuf());
          if (added_to_buf < 0) {
            LogH << "Błąd przy dodawaniu wiadomości do bufora. :<\n" <<
              "Socket o id " << stuff->GetId() << " i fd " <<
              stuff->GetSocket().GetFD() << "\nuser: " << x.first << '\n';
            stuff->ForceRemove();
            // :>
          }
        }
        break;
      case OutMessage::LIST_S:
        LogH << "OutMessage::LIST_S jeszcze nie działa xd\n";
        throw std::runtime_error("OutMessage::LIST_S not implemented");
      case OutMessage::LIST_U:
        LogH << "OutMessage::LIST_U jeszcze nie działa xd\n";
        throw std::runtime_error("OutMessage::LIST_U not implemented");
      case OutMessage::ONE_S:
        LogM << "OutMessage::ONE_S\n";
        {
          SockId id = msg->Sock();
          if (client_socks_.count(id) < 1) {
            LogH << "Lol, nie ma gniazda o id " << id << " xd\n";
            break;
          }
          auto &stuff = client_socks_.at(id);
          added_to_buf = msg->AddToBuf(&stuff.WrBuf());
          if (added_to_buf < 0) {
            LogH << "Błąd przy dodawaniu wiadomości do bufora. :<\n" <<
              "Socket o id " << id << " i fd " <<
              stuff.GetSocket().GetFD() << '\n';
            if (socks_to_users_.count(id) > 0) {
              LogH << "Jest na nim zalogowany user [[" <<
                socks_to_users_.at(id)->GetName() << "]]\n";
            } else {
              LogH << "Niezalogowane gniazdo :>\n";
            }
            stuff.ForceRemove();
            // Force, bo już i tak nic nie napiszemy raczej
          }
        }
        break;
      case OutMessage::ONE_U:
        LogH << "OutMessage::ONE_U jeszcze nie działa xd\n";
        throw std::runtime_error("OutMessage::ONE_U not implemented");
    }
    PopMsg_();
  }
  return 0;
}

int Server::ReadClients_() {
  int sockets_read = 0;
  for (auto it = client_socks_.begin(); it != client_socks_.end(); ++it) {
    if (!(it->second.ShallRead())
         || !(Sel::READ & sel_.Get(it->second.GetSocket().GetFD()))) {
       continue;
    }
    int read_chars = it->second.ReadCharsFromSocket();
    if (read_chars < 0) {
      LogM << "Był błąd przy czytaniu socketu o id " << it->first <<
        " i fd " << it->second.GetSocket().GetFD() << '\n';
      it->second.Remove();
      continue;
    } else if (read_chars == 0) {
      LogM << "Zamykanko przyszło " << it->first <<
        " i fd " << it->second.GetSocket().GetFD() << '\n';
      it->second.Remove();
      continue;
    }
    int deal_ret = it->second.DealWithReadBuf(&world_, &Server::PushMsg_);
    if (deal_ret < 0) {
      LogM << "Jakiś błąd przy ogarnianiu rzeczy z socketu " << it->first
        << " i fd " << it->second.GetSocket().GetFD() << '\n';
      it->second.Remove();
      continue;
    }
    ++sockets_read;
  }
  return sockets_read;
}

int Server::DropSock_(SockId id) {
  LogM << "Próba dropnięcioa socketu id " << id << "\n";
  if (client_socks_.count(id) < 1) {
    LogM << "Nie ma takiego socketu, wychodzonko\n";
    return -1;
  }
  if (socks_to_users_.count(id) > 0) {
    Username un = socks_to_users_.at(id)->GetName();
    LogM << "Na tym gnioeździe jest ktoś zalogowany\n"
      << "[[" << un << "]]\n";
    users_.erase(un);
    socks_to_users_.erase(id);
  }
  client_socks_.erase(id);
  LogM << "drop: ok, wychodzonko\n";
  return 0;
}

void Server::SpecialHardcodeInit() {
  return;
}

int Server::LoopTick_() {
  int feed_sel_ret = FeedSel_();
  int do_sel_ret = DoSel_();
  int read_main_fds_ret = ReadMainFds_();
  if (read_main_fds_ret > 0)
    return 0;
  int write_to_socks_ret = WriteToSocks_();
  int read_clients_ret = ReadClients_();
  int do_world_work_ret = DoWorldWork_();
  int msgs_to_bufs_ret = MsgsToBufs_();
  int delete_marked_socks_ret = DeleteMarkedSocks_();
  {
    (void)feed_sel_ret;
    (void)do_sel_ret;
    (void)msgs_to_bufs_ret;
    (void)read_main_fds_ret;
    (void)write_to_socks_ret;
    (void)read_clients_ret;
    (void)do_world_work_ret;
    (void)delete_marked_socks_ret;
  }

  return 0;
}

int Server::LogInUser(const Username &un, const std::string &pw,
    SockId sock_id, bool generate_response) {
  if (socks_to_users_.count(sock_id) > 0) {
    LogM << "gniazdo o id " << sock_id << " i fd " <<
      client_socks_.at(sock_id).GetSocket().GetFD() << " jest już zalogowane\n";
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(sock_id, MQ::ERR_WAS_LOGGED, false,
        "no już sie zalogowałes no")));
    return -1;
  }
  if (!un.Good()) {
    LogM << "Podana nazwaw jest niepoprawna!!!\n";
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(sock_id, MQ::ERR_BAD_LOG, false,
        "nie można mieć takiej nazwy")));
    return -1;
  }
  auto it = users_.find(un);
  if (it != users_.end()) {
    LogM << "user [[" << un << "]] jest zalogowany już\n"
      << "[[" << it->first << "]]\n";
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(sock_id, MQ::ERR_ACC_OCCUPIED, false)));
    return -1;
  }
  auto emplace_ret = users_.emplace(un, std::move(LoggedUser(un, sock_id)));
  if (emplace_ret.second == false) {
    LogH << "ojej, nie udało się dodać usera do mapy :\n";
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(sock_id, MQ::ERR_OTHER, false)));
    return -1;
  }
  LoggedUser *lu = &emplace_ret.first->second;
  auto emplace_ret2 = socks_to_users_.emplace(sock_id, lu);
  if (emplace_ret2.second == false) {
    LogH << "nie udało się uzupełnić mapy z id socketa na userów nowym "
      "userem :<\n";
    auto rm = users_.erase(un);
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(sock_id, MQ::ERR_OTHER, false)));
    assert(rm == 1);
    return -1;
  }
  LogM << "zalogowano [[" << un << "]]\n";
  world_.AddArtist(un);
  if (generate_response)
    PushMsg_(std::unique_ptr<OutMessage>(new LogOk(sock_id)));
  return 0;
}

int Server::LogOutUser(SockId id, bool generate_response) {
  LogM << "Próba wylogowania socketu " << id << '\n';
  if (socks_to_users_.count(id) < 1) {
    LogM << "Ten socket nie jest zalogowany.\n";
    if (client_socks_.count(id) < 1) {
      LogM << "Nawet go nie ma xd\n";
    }
    if (generate_response)
      PushMsg_(std::unique_ptr<OutMessage>(
        new Sig(id, MQ::ERR_NOT_LOGGED, false)));
    return -1;
  }
  Username un = socks_to_users_.at(id)->GetName();
  int rm = users_.erase(un);
  assert(rm == 1);
  rm = socks_to_users_.erase(id);
  assert(rm == 1);
  LogM << "Na tym sockecie był zalogowany [[" << un << "]]\n";
  if (generate_response)
    PushMsg_(std::unique_ptr<OutMessage>(new LogOff(id)));
  return 0;
}

int Server::DeleteMarkedSocks_() {
  auto it = client_socks_.begin();
  while (it != client_socks_.end()) {
    if (it->second.ShallBeRemoved()) {
      SockId id = it->first;
      ++it;
      DropSock_(id);
    } else {
      ++it;
    }
  }
  return 0;
}

int Server::PushMsg_(std::unique_ptr<OutMessage> msg) {
  messages_to_send_.emplace_back(std::move(msg));
  return 0;
}

OutMessage *Server::FirstMsg_() {
  if (messages_to_send_.size() < 1) return nullptr;
  return &*messages_to_send_.front();
}

int Server::PopMsg_() {
  messages_to_send_.erase(messages_to_send_.begin());
  return 0;
}

}  // namespace tin