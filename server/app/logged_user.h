// Copyright 2019 ja

#ifndef SERVER_APP_LOGGED_USER_H_
#define SERVER_APP_LOGGED_USER_H_

#include <string>

#include "app/session.h"
#include "core/username.h"

namespace tin {
class LoggedUser{
 public:
  constexpr static int MAX_ARTIST_ID = 1000;

  constexpr LoggedUser() : session_id_(0), sock_fd_(-1), artist_id_(0) {}
  LoggedUser(const Username &, SessionId);
  LoggedUser(const LoggedUser &) = delete;
  LoggedUser(LoggedUser &&);
  LoggedUser &operator=(LoggedUser &&);

  SessionId GetSession() const {return session_id_;}
  const Username &GetName() const {return name_;}
  int GetSock() const {return sock_fd_;}
  void SetSock(int fd) {sock_fd_ = fd;}
  void ClrSock() {sock_fd_ = -1;}
 private:
  void Move_(LoggedUser *);
  void Clear_();

  Username name_;  // nadmiar, bo będzie też wyszukiwanie po numerze sesji
  SessionId session_id_;  // nr sesji 0 to jej brak
  int sock_fd_;
  int artist_id_;  // 0 - nie ma artysty
};  // class LoggedUser
}  // namespace tin

#endif  // SERVER_APP_LOGGED_USER_H_
