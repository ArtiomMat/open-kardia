// Client module
#pragma once

#include "net.h"

enum
{
  CLI_E_JOIN, // The server responded about join, the big surpris is in e->enter.accepted.
  CLI_E_REPLY, // A reply for a request.
  CLI_E_TICK, // A global message.
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

// Returns if the join was sent, from our end.
extern int
cli_join(net_addr_t* addr, net_port_t port);

// Begin 
extern void
cli_begin_request();

extern void
cli_flush_request();
