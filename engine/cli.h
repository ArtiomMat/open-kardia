// Client module, not thread safe by default.
#pragma once

#include "net.h"
#include "tmr.h"

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

    struct
    {
      const char* alias; // Temporary and should be copied if needed
      const char* desc; // Temporary and should be copied if needed
      tmr_ms_t pp_ms; // Ping-pong time, may not describe the exact time to literally send and receive.
      uint8_t clis_n;
    } info;
  };
} cli_event_t;

extern int (*cli_on)(cli_event_t* e);

// Will always be NULL if client not initialized
extern net_sock_t* cli_sock;

extern int
cli_init(const char* alias);

extern void
cli_free();

// Not thread safe, use mutexes if multi-threading.
extern void
cli_run();

// Call net_set_addr() first to setup who we connect to, calls cli_exit() if already joined.
// Returns if the join was sent, from our end.
extern int
cli_join();

extern int
cli_exit();

// Uses cli_sock->pout.addr and port, so call net_set_addr() first
// Returns if the info request was sent from our end, if already waiting for info, returns 0.
// as_client 1 means that you receive the info as a client(only if connected, otherwise ignored), otherwise you receive this as a non client, which includes more info, read README for more documentation on what you receive.
extern int
cli_info(int as_client);

// Calls net_rewind because header must be in beginning. Sets up headers so the server can understand what's going on. After calling this use net_put and flush when ready with cli_sock.
// Returns 0 if a reply for a previous request was not sent yet and want_reply=1, you should block until it returns 1.
extern int
cli_begin_request(int want_reply);
