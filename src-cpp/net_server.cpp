#include "net.hpp"

#include "tmr.hpp"

#include <cstdio>
#include <cstring>

namespace net
{
  server_t::server_t(const char* _alias, const char* _desc) : sock(true)
  {
    alias = _alias;
    desc = _desc;

    clients_n = 0;
    last_tick_ms = tmr::now();

    for (unsigned i = 0; i < MAX_SERVER_CLIENTS; i++)
    {
      clients[i].status = STATUS_FREE;
    }

    int x = com::big16(sock.bound_port);
    printf("Opened server socket, port %i.\n", x);
  }

  void server_t::_live_client(int i)
  {
    if (clients[i].status != STATUS_LIVE)
    {
      clients[i].status = STATUS_LIVE;
      clients_n++;
    }
  }

  void server_t::_free_client(int i)
  {
    if (clients[i].status != STATUS_FREE)
    {
      // 0 out the address so a disconnected client can't just play around.
      clients[i].addr.l[0] = 0;
      clients[i].addr.l[1] = 0;

      clients[i].status = STATUS_FREE;
      clients_n--;
    }
  }

  void server_t::_handle_wait_status(int i, tmr::ms_t now)
  {
    if (now - clients[i].last_pack_ms >= MAX_WAIT_MS) // Too late to confirm.
    {
      _free_client(i);
    }
    else if (now - clients[i].last_pack_ms >= MAX_WAIT_MS/2) // Last chance to confirm the accept, we resend it.
    {
      char data[2] = {SERVER_B_JOIN, static_cast<char>(i)};
      sock.sendto(data, 2);
    }
  }

  void server_t::run()
  {
    event_t e;
    tmr::ms_t now = tmr::now();

    // Time for a tick! but only if the tick rate is non-zero
    if (SERVER_TICK_RATE != 0 && now - last_tick_ms >= SERVER_TICK_RATE)
    {
      // handler() the tick
      e.type = E_TICK;
      e.i = -1;
      sock.rewind();
      sock.put8(SERVER_B_TICK);
      handler(e);
      
      if (sock.pout.cur > 1) // If a tick was written handle both sending it and the wait clients.
      {
        for (unsigned i = 0; i < MAX_SERVER_CLIENTS; i++)
        {
          switch (clients[i].status)
          {
            case STATUS_LIVE:
            // Idle for too long, TODO: change to max IDLE time rather than wait but for now will do
            if (clients[i].last_pack_ms >= MAX_WAIT_MS)
            {
              _free_client(i);
              break;
            }

            sock.set_addr(clients[i].addr, clients[i].port);
            sock.flush();
            break;

            case STATUS_WAIT:
            _handle_wait_status(i, now);
            break;
          }
        }
      }
      else // Only handle waits
      {
        for (unsigned i = 0; i < MAX_SERVER_CLIENTS; i++)
        {
          if (clients[i].status == STATUS_WAIT)
          {
            _handle_wait_status(i, now);
          }
        }
      }

      last_tick_ms = tmr::now();
    }

    // Now requests
    for (unsigned _refresh_i = 0; _refresh_i < MAX_SERVER_REFRESHES_PER_RUN && sock.refresh(); _refresh_i++)
    {
      int8_t first_byte;
      if (!sock.can_get8())
      {
        continue;
      }
      first_byte = sock.get8();

      int8_t ci;
      if (!sock.can_get8())
      {
        continue;
      }
      ci = sock.get8();

      // Join request
      if (first_byte == CLIENT_B_JOIN)
      {
        unsigned c_alias_n = sock.gets_n();

        if (c_alias_n <= MAX_CLIENT_ALIAS) // Everything is fine, can technically accept
        {
          const char* c_alias = sock.getb(c_alias_n);

          sock.set_addr(sock.pin.addr, sock.pin.port);
          sock.rewind();

          sock.pout.cur = 2; // Allocate space for either join header
          e.type = E_JOIN;
          e.i = -1;
          e.join.accepted = 1;
          handler(e);
          
          if (e.join.accepted)
          {
            if (clients_n >= MAX_SERVER_CLIENTS)
            {
              goto _reject_client;
            }
            // Find a free slot
            unsigned char ci;
            for (ci = 0; ci < MAX_SERVER_CLIENTS; ci++)
            {
              if (clients[ci].status == STATUS_FREE)
              {
                break;
              }
            }

            // Setup the client
            clients[ci].status = STATUS_WAIT;
            clients[ci].last_pack_ms = tmr::now();
            clients[ci].port = sock.pin.port;
            clients[ci].addr = sock.pin.addr;
            memcpy(clients[ci].alias, c_alias, c_alias_n);

            sock.pout.data[0] = SERVER_B_JOIN;
            sock.pout.data[1] = ci;
            sock.flush();

          }
          else // !e.join.accepted
          {
            goto _reject_client;
          }
        }
        else
        {
          _reject_client:
          sock.pout.data[0] = SERVER_B_JOIN;
          sock.pout.data[1] = -1; // Rejected
          sock.flush();
        }

        continue;
      }
      // Info request, as disjoined client, and so we reply
      if (first_byte == CLIENT_B_INFO && ci < 0)
      {
        sock.set_addr(sock.pin.addr, sock.pin.port);
        sock.rewind();
        sock.put8(SERVER_B_INFO);
        sock.put8(clients_n);
        sock.puts(alias);
        sock.puts(desc);

        e.type = E_INFO;
        e.i = -1; // -1 for disjoined client info
        handler(e);

        sock.flush();

        continue;
      }

      // So far we handled stuff for disjoined clients, if we got to here the client MUST have sent a message as a joined client.
      
      // Invalid index?
      if (ci < 0 || ci >= (int)MAX_SERVER_CLIENTS || clients[ci].status == STATUS_FREE)
      {
        continue;
      }

      // The wrong address sent this index?
      if ( sock.pin.addr.l[0] != clients[ci].addr.l[0] || 
          sock.pin.addr.l[1] != clients[ci].addr.l[1] )
      {
        continue;
      }

      clients[ci].port = sock.pin.port; // Port may have changed.

      clients[ci].last_pack_ms = tmr::now();

      sock.rewind();
      bool do_flush = false; // Whether to do a flush back to the client
      e.i = ci; // index for event

      // If the client is on wait status we only accept one type of pack
      if (clients[ci].status == STATUS_WAIT)
      {
        if (first_byte == CLIENT_B_GOT_ACCEPT)
        {
          _live_client(ci);
          // printf("SERVER: '%s' joined at index %hhi.\n", clients[ci].alias, ci);
        }

        continue;
      }
      
      switch(first_byte)
      {
        case CLIENT_B_REQUEST:
        sock.put8(SERVER_B_REPLY);

        e.type = E_REQUEST;
        handler(e);
        
        do_flush = sock.pout.cur > 1;
        break;

        case CLIENT_B_EXIT:
        e.type = E_CLIENT_EXIT;
        handler(e);
        _free_client(ci);
        break;

        case CLIENT_B_INFO: // It will 100% be joined info as we tested before for disjoined.
        sock.put8(clients_n);

        e.type = E_INFO;
        handler(e);

        do_flush = true; // Do flush regardless
        break;
      }

      // do_flush was set up, so...
      if (do_flush)
      {
        sock.set_addr(clients[ci].addr, clients[ci].port);
        sock.flush();
      }
    }
  }
}