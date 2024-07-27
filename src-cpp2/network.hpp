// Low(er)-level networking module, platform independent.
// This module is not meant to be used by games, it's only meant to be used for the se and cl modules.

#pragma once

#include "common.hpp"


namespace axe
{
  constexpr unsigned MAX_SERVER_CLIENTS = 24;

  // How much time between server ticks, can be set to 0 to disable server ticks if the game doesn't need it.
  constexpr time_t SERVER_TICK_RATE = 0;

  constexpr unsigned MAX_CLIENT_ALIAS = 24;

  constexpr unsigned MAX_SERVER_ALIAS = 32;
  constexpr unsigned MAX_SERVER_DESC = 256;
  
  constexpr time_t MAX_WAIT_MS = 3000;

  constexpr time_t MAX_SERVER_REFRESHES_PER_RUN = MAX_SERVER_CLIENTS + 16;
  constexpr time_t MAX_CLIENT_REFRESHES_PER_RUN = 8;

  // Values for that first byte that the client sends
  enum
  {
    CLIENT_B_INFO = -127, // for info
    CLIENT_B_ERR, // Who the fuck be the client that they send errors to the server, bro, who gives a shit? Idk if I should keep it here.
    CLIENT_B_JOIN, // for join
    CLIENT_B_GOT_ACCEPT,
    CLIENT_B_REQUEST,
    CLIENT_B_DISJOIN,
  };

  enum
  {
    SERVER_B_INFO = -127, // for info
    SERVER_B_ERR,
    SERVER_B_JOIN,
    SERVER_B_REPLY,
    SERVER_B_TICK,
  };

  constexpr addr_t loopback = []
  {
    addr_t a{};
    a.b[15] = 1;
    return a;
  }();

  // Address of the host itself.
  extern addr_t host_addr;
  // Initialized only on init
  extern const char* host_name;

  extern bool initialized;

  void initialize();
  
  void shutdown();

  // IPv6 address string to an actual IPv6.
  void stoa(addr_t& addr, const char* str);
  // Converts IPv6 address to IPv6 string.
  // str must be valid.
  void atos(char* str, const addr_t& addr, int size);
}
