#pragma once

#include "common.hpp"

namespace axe
{
  struct client_t
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

    struct event_t
    {
      int type;
      union
      {
        struct
        {
          bool accepted;
        } join;

        struct
        {
          const char* alias; // Temporary and should be copied if needed
          const char* desc; // Temporary and should be copied if needed
          tmr::ms_t pp_ms; // Ping-pong time, may not describe the exact time to literally send and receive.
          uint8_t clients_n;
        } info;
      };
    };

    sock_t sock;

    tmr::ms_t info_ask_ms;

    const char* alias;

    bool want_join = false;
    bool want_reply = false;
    int want_info = 0; // 0 for nope, 1 for yope but not as a client, 2 for yep but as a joined client.
    // Just for us
    bool alias_allocated = false;

    int8_t my_index = -1;

    virtual void handler(event_t& e) = 0;

    client_t(const char * _alias);
    virtual ~client_t();
    
    // Not thread safe, use mutexes if multi-threading.
    void run();
    // Call net_set_addr() first to setup who we connect to, calls cli_exit() if already joined.
    // Returns if the join was sent, from our end.
    void join();
    void disjoin();
    // Uses cli_sock->pout.addr and port, so call net_set_addr() first
    // Returns if the info request was sent from our end, if already waiting for info, returns 0.
    // as_client 1 means that you receive the info as a client(only if connected, otherwise ignored), otherwise you receive this as a non client, which includes more info, read README for more documentation on what you receive.
    void info(bool as_joined);
    // Calls net_rewind because header must be in beginning. Sets up headers so the server can understand what's going on. After calling this use net_put and flush when ready with cli_sock.
    // Returns false if a reply for a previous request was not sent yet and want_reply=1, you should block until it returns true.
    bool begin_request(bool _want_reply);

    inline bool is_disjoined() const { return my_index == -1 && !want_join; }
  };
}