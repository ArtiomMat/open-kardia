// Local defines

#pragma once

// Maximum alias size a client can have, otherwise rejected.

#define PROT_MAJOR_VERSION 0
#define PROT_MINOR_VERSION 1

#define PROT_MAX_IDLE_MS 3000

// Values for that first byte that the client sends
enum
{
  CLI_I_INFO = -127, // for info
  CLI_I_ERR, // Who the fuck be the client that they send errors to the server, bro, who gives a shit? Idk if I should keep it here.
  CLI_I_JOIN, // for join
  CLI_I_GOT_ACCEPT,
  CLI_I_REQUEST,
  CLI_I_EXIT,
};

enum
{
  SER_I_INFO = -127, // for info
  SER_I_ERR,
  SER_I_ACCEPT,
  SER_I_REJECT, // rejection of a join request
  SER_I_REPLY,
  SER_I_TICK,
};
