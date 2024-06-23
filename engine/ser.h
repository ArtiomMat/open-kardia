// Server module
#pragma once

#include "tmr.h"
#include "net.h"

#define SER_MAX_CLIENTS 24

// How much time between server ticks
#define SER_TICK_RATE 32

#define SER_MAX_CLI_ALIAS 24

#define SER_MAX_SER_ALIAS 32
#define SER_MAX_SER_DESC 256

enum
{
  SER_E_JOIN, // e->join.accepted can be changed to 0, by default will be 1. can freely net_get and net_put your custom extra data, the server already allocated space for headers before calling on.
  SER_E_REQUEST, // The client sent a request, may expect a reply(depends fully on your custom protocol). You now can net_get, and also net_put, if net_put the reply is sent, otherwise no reply.
  SER_E_TICK, // It's time to net_put a message for all clients, begin writing, if cursor exceeds 0 will be sent to all clients, otherwise this tick is not considered.
  // SER_E_ALIVE, // A general alive message if the client has nothing to request.
  SER_E_INFO, // The client asked for info about the server, can net_put custom extra info now, if cursor exceeds 0 info is sent.
  SER_E_EXIT, // Client is exiting. Just note it.
};

enum
{
  SER_CLI_FREE,
  SER_CLI_LIVE,
  SER_CLI_WAIT, // Waiting confirmation that the client knows they were accepted.
};

typedef struct
{
  uint8_t type;
  uint8_t i; // Client index -1 for N/A like for tick, or join(can read address in ser_sock->pin.addr)
  union
  {
    struct
    {
      char accepted; // Whether or not to accpet the join request of the user. 1 by default.
    } join;
  };
} ser_event_t;

typedef struct ser_client_s
{
  net_addr_t addr; // Also used as a sort of key, when clients send their index, checked if the address is in that client index.
  tmr_ms_t last_pack_ms;
  net_port_t port;
  char status;
  char alias[SER_MAX_CLI_ALIAS];
  char requests_n; // How many requests the client made this run, if exceeds a certain value then we ignore the requests to let others request too.
} ser_client_t;

extern int (*ser_on)(ser_event_t* e);

extern ser_client_t ser_clis[SER_MAX_CLIENTS];
extern int ser_clis_n;

// Will always be NULL if server not initialized
extern net_sock_t* ser_sock;

extern int
ser_init(const char* alias);

extern void
ser_free();

// Refreshes socket until dried out, and calls ser_on().
extern void
ser_run();

extern void
ser_reply(ser_client_t* c);
