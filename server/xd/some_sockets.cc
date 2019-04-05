// Copyright 2077 jabka

#include "xd/some_sockets.h"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cstring>
#include <iostream>

#include "core/thread.h"

namespace tin {
  using std::size_t;

  void pisanko(int fd) {
    for (int i = 0; i < 10; ++i) {
      sleep(1);
      dprintf(fd, "xdxdxd %d %d\n", 666, i);
    }
    dprintf(fd, "koniec\n");
  }

  void czytanko(int fd) {
    static constexpr std::size_t BUFF_LEN = 256;
    while (true) {
      char buff[BUFF_LEN + 1];
      ssize_t len;
      len = read(fd, buff, BUFF_LEN);
      if (len == 0) {
        return;
      }
      buff[len] = '\0';
      std::cout << "Czytanko: " << buff;
      if (buff[len - 1] != '\n') {
        std::cout << '\n';
      }
      if (std::strcmp(buff, "koniec\n") == 0) {
        return;
      }
    }
  }

  void *server_func(void *vp) {
    int serv_sock;
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) {
      std::cerr << "server socket creation failed: " << serv_sock << ", "
        << errno << '\n';
      return nullptr;
    }
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(33000);
    int bind_ret;
    bind_ret = bind(serv_sock,
      reinterpret_cast<struct sockaddr *>(&saddr),
      sizeof(saddr));
    if (bind_ret != 0) {
      std::cerr << "bind error: " << bind_ret << '\n'
        << "errno: " << std::strerror(errno) << '\n';
      return nullptr;
    }
    int listen_ret;
    listen_ret = listen(serv_sock, 16);
    if (listen_ret != 0) {
      std::cerr << "listen error: " << listen_ret << '\n'
        << "errno: " << std::strerror(errno) << '\n';
      return nullptr;
    }
    struct sockaddr cliaddr;
    int connection_socket;
    socklen_t addr_size;
    connection_socket = accept(serv_sock, &cliaddr, &addr_size);
    if (listen_ret != 0) {
      std::cerr << "accept error: " << connection_socket << '\n'
        << "errno: " << std::strerror(errno) << '\n';
      return nullptr;
    }
    std::cout << "Serwer się połączył z klientem xd\n";
    czytanko(connection_socket);
    close(connection_socket);
    close(serv_sock);
    return nullptr;
  }

  void *client_func(void *vp) {
    sleep(3);
    int cli_sock;
    cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_sock < 0) {
      std::cerr << "error client socket: " << std::strerror(errno) << '\n';
      return nullptr;
    }
    int connect_ret;
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    saddr.sin_port = htons(33000);
    connect_ret = connect(cli_sock,
      reinterpret_cast<struct sockaddr *>(&saddr),
      sizeof(saddr));
    if (connect_ret < 0) {
      std::cerr << "connect error: " << std::strerror(errno) << '\n';
      return nullptr;
    }
    std::cout << "Połączyło się z serwerem :3\n";
    pisanko(cli_sock);
    close(cli_sock);
    return nullptr;
  }

  int func(int argc, char **argv, char **env) {
    Thread client_thr, serv_thr;
    serv_thr = Thread(server_func, nullptr);
    client_thr = Thread(client_func, nullptr);
    serv_thr.Join();
    client_thr.Join();
    return 0;
  }
}  // namespace tin