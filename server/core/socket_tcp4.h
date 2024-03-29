// Copyright 2019 TIN

#ifndef SERVER_CORE_SOCKET_TCP4_H_
#define SERVER_CORE_SOCKET_TCP4_H_

#include <cstdint>

namespace tin {

class SocketTCP4 {
 public:
  enum Status {
    BLANK,
    CREATED,
    BOND,
    LISTENING,
    CONNECTED,
  };

  SocketTCP4();
  //  explicit SocketTCP4(uint32_t address, uint16_t port);
  SocketTCP4(const SocketTCP4 &) = delete;
  SocketTCP4(SocketTCP4 &&) noexcept;
  SocketTCP4 &operator=(SocketTCP4 &&) noexcept;
  ~SocketTCP4();

  explicit operator int() {return fd_;}

  int Open();
  int Bind(uint32_t, uint16_t);
  int BindAny(uint16_t);
  int Listen(int queue_length);
  int Connect(uint32_t, uint16_t);
  int Accept(SocketTCP4 *);
  int Close();
  int Shutdown(int how);

  Status GetStatus() const {return status_;}
  int GetFD() const {return fd_;}  // This shall be used for 'select'.

 private:
  static const struct ACCEPT_t {} ACCEPT;
  //  explicit SocketTCP4(Accept);

  void Destroy_();
  void Move_(SocketTCP4 *other);

  int fd_;
  Status status_;
  uint32_t addr_here_;
  uint16_t port_here_;
  uint32_t addr_there_;
  uint16_t port_there_;
};

}  // namespace tin

#endif  // SERVER_CORE_SOCKET_TCP4_H_
