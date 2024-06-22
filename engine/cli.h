// Client module
#pragma once

#include "net.h"

enum
{
  CLI_E_JOIN, // The server responded about join, the big surpris is in e->enter.accepted.
  CLI_E_REPLY, // A reply for a request, net_get.
  CLI_E_TICK, // A global message, net_get.
  CLI_E_INFO, // The server responded with info, can net_get now.
};

typedef struct
{
  int type;
  union
  {
    struct
    {
      char accepted;
    } join;
  };
} cli_event_t;

extern int (*cli_on)(cli_event_t* e);

// Will always be NULL if client not initialized
extern net_sock_t* cli_sock;

extern int
cli_init(const char* alias);

extern void
cli_free();

extern void
cli_run();

// Returns if the join was sent, from our end.
extern int
cli_join(net_addr_t* addr, net_port_t port);

// Returns if the info request was sent from our end.
extern int
cli_info(net_addr_t* addr, net_port_t port);

// Calls net_rewind because header must be in beginning. Sets up headers so the server can understand what's going on. After calling this use net_put and flush when ready with cli_sock.
extern void
cli_begin_request(int want_reply);
