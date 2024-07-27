#pragma once

#include "common.hpp"

namespace axe
{
  struct server_t
  {
    enum
    {
      E_JOIN, // e->join.accepted can be changed to 0, by default will be 1. can freely net_get and net_put your custom extra data, the server already allocated space for headers before calling on. If e->i is -1 it means the client wants the total server information as if not connected(whether they are or not).
      
      E_REQUEST, // The client sent a request, may expect a reply(depends fully on your custom protocol). You now can net_get, and also net_put, if net_put the reply is sent, otherwise no reply.
      
      E_TICK, // It's time to net_put a message for all clients, begin writing, if cursor exceeds 0 will be sent to all clients, otherwise this tick is not considered.
      // SER_E_ALIVE, // A general alive message if the client has nothing to request.
      
      E_INFO, // The client asked for info about the server, can net_put custom extra info now, if cursor exceeds 0 info is sent.
      
      E_DISJOIN, // Client is exiting. Just note it.

      E_REPLY, // For client only
    };

    enum
    {
      STATUS_FREE,
      STATUS_WAIT,
      STATUS_LIVE,
    };

    struct event_t
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
    };

    struct client_t
    {
      addr_t addr; // Also used as a sort of key, when clients send their index, checked if the address is in that client index.
      time_t last_pack_ms;
      port_t port;
      char status;
      char alias[MAX_CLIENT_ALIAS];
      char requests_n; // How many requests the client made this run, if exceeds a certain value then we ignore the requests to let others request too.
    };

    sock_t sock;

    const char* alias, * desc;

    time_t last_tick_ms;
    time_t last_info_ms; // Last time info was requested

    client_t clients[MAX_SERVER_CLIENTS];
    // Live clients, does not include clients with wait status
    unsigned clients_n;

    virtual void handler(event_t& e) = 0;

    // desc can be nullptr, alias has to be a valid string.
    server_t(const char* _alias, const char* desc);
    server_t(const char* _alias) : server_t(_alias, nullptr) {}
    virtual ~server_t() = default;

    // Refreshes socket until dried out, and calls handler().
    // Not thread safe, use mutexes if multi-threading.
    void run();

    void _live_client(int i);
    void _free_client(int i);
    void _handle_wait_status(int i, time_t now);
  };
}