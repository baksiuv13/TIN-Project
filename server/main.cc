// Copyright 1077 jabka

#include <arpa/inet.h>

#include <iostream>
#include <array>
#include <string>

#include "xd/some_sockets.h"
#include "app/server.h"
#include "core/username.h"
#include "core/nquad.h"
#include "core/mquads.h"

/*
static std::array<char, 16> fgfg(uint32_t x) {
  // nie chciało mi się szukać tyhc funkcji więc napisałem sam na rzaie
  std::array<char, 16> xc;
  char *gd = xc.data();
  for (uint32_t i = 0; i < 32; i += 8) {
    gd += snprintf(gd, 16 * sizeof(char), "%d.", (x >> i) & 255);
  }
  *(gd - 1) = '\0';
  return xc;
}*/


void haha(tin::NQuad nq) {
  char x[5];
  x[4] = '\0';
  memcpy(x, &nq, 4);
  std::ios ios_state(nullptr);
  ios_state.copyfmt(std::cerr);
  std::cerr << std::hex << nq << " : " << x << '\n';
  std::cerr.copyfmt(ios_state);
}


int main(int argc, char **argv, char **env) {
  //  return tin::func(argc, argv, env);
  using tin::NQuad;
  using tin::Username;
  using tin::LoggedUser;
  NQuad qaz{"OwO!"};
  std::cerr << std::hex << qaz.Int() << '\n' << std::dec;

  Username x{"jabko"};
  Username c{"Mistrz Windowsa"};
  Username v(c, Username::CONDENSE);
  Username q("666");
  Username gaben("Gaben");
  std::cerr << "[[" << x << "]] " << x.GetState() << "\n";
  std::cerr << "[[" << c << "]] " << c.GetState() << "\n";
  std::cerr << "[[" << v << "]] " << v.GetState() << "\n";
  std::cerr << "[[" << q << "]] " << q.GetState() << "\n";

  std::cerr << (c < v) << '\n';
  std::cerr << (c > v) << '\n';
  std::cerr << (c <= v) << '\n';
  std::cerr << (c >= v) << '\n';
  std::cerr << (c == v) << '\n';
  std::cerr << (c != v) << '\n';
  std::cerr << c.RawEqual(v) << '\n';
  std::cerr << c.RawLess(v) << '\n';

  haha(tin::MQ::OWO);
  haha(tin::MQ::ZERO);
  haha(tin::MQ::CAPTURE_SESSION);
  haha(tin::MQ::REQUEST_LOGIN);
  haha(tin::MQ::SERVER_DISCONNECT);

  // Username gaben2 = gaben;


  // std::cerr << "[[" << gaben << "]] " << gaben.GetState() << "\n";
  // std::cerr << "[[" << gaben2 << "]] " << gaben2.GetState() << "\n";

  // LoggedUser gabe(gaben, 15);
  // std::cerr << gabe.GetName() << '\n';

  //  std::cout << fgfg(htonl(0x7f000001)).data() << '\n';
  //  std::cout << fgfg(htonl(0xc0a80164)).data() << '\n';
  //  std::cout << fgfg(htonl(0xffffffff)).data() << '\n';

  //  return 0;
  //  return 0;

  tin::Server server;
  server.SpecialHardcodeInit();
  server.Run();
}
