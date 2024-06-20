// Server module
#pragma once

#include "net.h"

#define SER_MAX_CLIENTS 32

// How much time between server ticks
#define SER_TICK_RATE 32

enum
{
  SER_E_JOIN, // e->join.accepted can be changed to 0, by default will be 1.
  SER_E_REQUEST, // The client sent a request, may expect a reply(depends on your protocol). You now can net_get, and also net_put, if cursor exceeds 0 reply is sent.
  SER_E_TICK, // It's time to net_put a message for all clients, begin writing, if cursor exceeds 0 will be sent to all clients.
  SER_E_ALIVE, // A general alive message if the client has nothing to request.
};

typedef struct
{
  int type;
  union
  {
    struct
    {
      char accepted; // Whether or not to accpet the join request of the user. 1 by default.
    } join;
    struct
    {
      char replied; // Whether or not the server replied to the request via net_put. 0 by default.
    } request;
  };
} ser_event_t;

typedef struct ser_client_s
{
  net_addr_t address; // Also used as a sort of key, when clients send their index, checked if the address is in that client index.
  net_port_t port;
  char free;
} ser_client_t;

extern int (*ser_on)(ser_event_t* e);

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
