#include "client.hpp"

#include <cstring>
#include <cstdio>

namespace axe
{
  client_t::client_t(const char* _alias) : sock(false)
  {
    alias = _alias;
    if (strlen(alias)+1 >= MAX_CLIENT_ALIAS)
    {
      puts("Alias for client was too long, truncating.");

      // Handle this shit gracefully
      alias_allocated = true;

      char* _alias = new char[MAX_CLIENT_ALIAS];
      
      unsigned i;
      for (i = 0; i < MAX_CLIENT_ALIAS-1; i++)
      {
        _alias[i] = alias[i];
      }
      _alias[i] = 0;

      alias = _alias;
    }

    want_reply = false;
    want_info = 0;
    want_join = false;
    my_index = -1;

    puts("CLIENT: Opened socket.");
  }

  client_t::~client_t()
  {
    disjoin(); // Be a nice guy and notify server.

    if (alias_allocated)
    {
      delete [] alias;
    }
  }

  void client_t::run()
  {
    // Disconnected
    if (is_disjoined())
    {
      return;
    }

    event_t e;

    for (unsigned _refresh_i = 0; _refresh_i < MAX_CLIENT_REFRESHES_PER_RUN && sock.refresh(); _refresh_i++)
    {
      // YOU PICKED THE WRONG HOUSE FOOOOL
      if (
        sock.pin.addr.l[0] != sock.pout.addr.l[0] || 
        sock.pin.addr.l[1] != sock.pout.addr.l[1] ||
        sock.pin.port != sock.pout.port
        )
      {
        continue;
      }

      // All good, this is the server talking
      int8_t first_byte;
      if (!sock.can_get8())
      {
        continue;
      }
      first_byte = sock.get8();

      switch (first_byte)
      {
        case SERVER_B_JOIN:
        if (!want_join)
        {
          break;
        }
        want_join = false;

        if (!sock.can_get8())
        {
          // puts("CLIENT: Bad server.");
          return;
        }
        my_index = sock.get8();

        if (my_index >= 0) // ACCEPTED :D
        {
          e.join.accepted = true;

          // Notify server that we got it
          sock.rewind();
          sock.put8(CLIENT_B_GOT_ACCEPT);
          sock.put8(my_index);
          sock.flush();

          // printf("CLIENT: Joined at index %hhi.\n", my_index);
        }
        else // REJECTED :(
        {
          e.join.accepted = false;
        }
        
        e.type = E_JOIN;
        handler(e);
        break;

        case SERVER_B_REPLY:
        if (!want_reply)
        {
          break;
        }
        want_reply = false;

        e.type = E_REPLY;
        handler(e);
        break;

        case SERVER_B_TICK:
        if (!SERVER_TICK_RATE) // Perhaps handle it differently because literally the server sent a tick but the engine doesn't allow server ticks.
        {
          break;
        }
        e.type = E_TICK;
        handler(e);
        break;

        case SERVER_B_INFO:
        tmr::ms_t now = tmr::now();
        if (!want_info)
        {
          break;
        }
        int prev_want_info = want_info;
        want_info = false;

        // clients_n
        if (!sock.can_get8())
        {
          break;
        }
        e.info.clients_n = sock.get8();

        if (prev_want_info == 1) // As disjoined
        {
          // We can net_gets with no can function it's safe.
          e.info.alias = sock.gets();
          e.info.desc = sock.gets();
        }
        else
        {
          e.info.alias = e.info.desc = nullptr;
        }

        e.info.pp_ms = now - info_ask_ms; // Ping pong time
        e.type = E_INFO;
        handler(e);
        break;
      }
    }
  }

  void client_t::join()
  {
    disjoin(); // Exit first any server we are in

    sock.rewind();
    
    sock.put8(CLIENT_B_JOIN);
    sock.put8(-1);
    sock.puts(alias);
    
    want_join = 1;

    sock.flush();
  }

  void client_t::info(bool as_joined)
  {
    if (want_info)
    {
      return;
    }

    sock.rewind();
    sock.put8(CLIENT_B_INFO);
    // my_index is -1 anyway if not joined, so still ok.
    sock.put8(as_joined ? my_index : -1);
    
    want_info = 1 + as_joined;

    info_ask_ms = tmr::now();
    sock.flush();
  }

  void client_t::disjoin()
  {
    if (is_disjoined())
    {
      return;
    }

    sock.rewind();
    sock.put8(CLIENT_B_DISJOIN);
    sock.put8(my_index);

    want_reply = false;
    want_info = 0;
    want_join = false;
    my_index = -1;

    sock.flush();
  }

  bool client_t::begin_request(bool _want_reply)
  {
    // Can't want a reply again until a reply to the previous pack.
    if (_want_reply && want_reply)
    {
      return false;
    }
    
    sock.rewind();
    sock.put8(CLIENT_B_REQUEST);
    sock.put8(my_index);
    want_reply = _want_reply;

    return true;
  }
}