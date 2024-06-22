#include "net.h"
#include "ser.h"
#include "cli.h"
#include "clk.h"
#include "local.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_REFRESHES_PER_RUN (SER_MAX_CLIENTS+8)

#define MAX_WAIT_TIME 1000

#define INFO_COOLDOWN 500

static const char* alias;

net_sock_t* ser_sock = NULL;

ser_client_t clients[SER_MAX_CLIENTS] = {0};
static int clients_n = 0;

static clk_time_t last_tick_time;
static clk_time_t last_info_time; // Last time info was requested

int ser_def_on(ser_event_t* e)
{
  return 0;
}

int (*ser_on)(ser_event_t* e) = ser_def_on;

int
ser_init(const char* _alias)
{
  alias = _alias;

  clients_n = 0;
  last_tick_time = clk_now();

  for (int i = 0; i < SER_MAX_CLIENTS; i++)
  {
    clients[i].status = SER_CLI_FREE;
  }
  
  ser_sock = net_open(1);
}

static void
free_client(int i)
{
  clients[i].status = SER_CLI_FREE;
  clients_n--;
}

void
ser_run()
{
  ser_event_t e;

  // Time for a tick!
  if (clk_now() - last_tick_time >= SER_TICK_RATE)
  {
    // ser_on() the tick
    e.type = SER_E_TICK;
    net_rewind(ser_sock);
    ser_on(&e);
    
    // Only send if there is any data in the tick, also, retry 
    if (ser_sock->pout.cur > 0)
    {
      for (int i = 0; i < SER_MAX_CLIENTS; i++)
      {
        if (clients[i].status == SER_CLI_LIVE)
        {
          net_set_addr(ser_sock, &clients[i].addr, clients[i].port);
          net_flush(ser_sock);
        }
        // The client status is in wait, gotta make sure they got the memo.
        else if (clients[i].status == SER_CLI_WAIT)
        {
          clk_time_t now = clk_now();
          if (now - clients[i].last_pack_time >= MAX_WAIT_TIME) // Too late to confirm.
          {
            free_client(i);
          }
          else if (now - clients[i].last_pack_time >= MAX_WAIT_TIME/2) // Last chance to confirm the accept, we resend it.
          {
            char data[2] = {SER_I_ACCEPT, i};
            net_sendto(ser_sock, data, 2);
          }
        }
      }
    }

    last_tick_time = clk_now();
  }

  // Now requests
  for (int _refresh_i = 0; _refresh_i < MAX_REFRESHES_PER_RUN && net_refresh(ser_sock); _refresh_i++)
  {
    uint8_t u8;
    int8_t i8;

    if (!net_can_get8(ser_sock))
    {
      continue;
    }
    net_get8(ser_sock, &i8);

    // Join request
    if (i8 == CLI_I_JOIN)
    {
      int c_alias_n = net_gets_n(ser_sock);

      if (c_alias_n <= SER_MAX_CLI_ALIAS) // Everything is fine, can technically accept
      {
        const char* c_alias;
        net_gets(ser_sock, &c_alias);

        net_set_addr(ser_sock, &ser_sock->pin.addr, ser_sock->pin.port);
        net_rewind(ser_sock);

        ser_sock->pout.cur = 2; // Allocate space for either reject or accept headers
        e.type = SER_E_JOIN;
        e.join.accepted = 1;
        ser_on(&e);
        
        if (e.join.accepted)
        {
          // Too many
          if (clients_n >= SER_MAX_CLIENTS)
          {
            goto _reject_client;
          }
          // Find a free slot
          char ci;
          for (ci = 0; ci < SER_MAX_CLIENTS; ci++)
          {
            if (clients[ci].status == SER_CLI_FREE)
            {
              break;
            }
          }
          // Setup the client
          clients[ci].status = SER_CLI_WAIT;
          clients[ci].last_pack_time = clk_now();
          clients[ci].port = ser_sock->pin.port;
          clients[ci].addr = ser_sock->pin.addr;
          memcpy(clients[ci].alias, c_alias, c_alias_n);

          ser_sock->pout.data[0] = SER_I_ACCEPT;
          ser_sock->pout.data[1] = ci;
          net_flush(ser_sock);

          printf("ser_run(): '%s' has been accepted as #%i.\n", clients[ci].alias, ci);
        }
        else // !e.join.accepted
        {
          goto _reject_client;
        }
      }
      else
      {
        _reject_client:
        ser_sock->pout.data[0] = SER_I_REJECT;
        ser_sock->pout.data[1] = 0; // Padding
        net_flush(ser_sock);
      }

      continue;
    }

    // Otherwise the client is connected.
    int8_t ci;
    if (!net_can_get8(ser_sock))
    {
      continue;
    }
    net_get8(ser_sock, &ci);

    // Invalid in general
    if (ci < 0 || ci >= SER_MAX_CLIENTS || clients[ci].status == SER_CLI_FREE)
    {
      continue;
    }

    // The wrong address sent this index.
    if (
      ser_sock->pin.addr.l[0] != clients[ci].addr.l[0] || 
      ser_sock->pin.addr.l[1] != clients[ci].addr.l[1]
      )
    {
      // TODO: something about it.
      continue;
    }

    clients[ci].port = ser_sock->pin.port; // Port may have changed.
    clients[ci].last_pack_time = clk_now();

    net_rewind(ser_sock);

    switch(i8)
    {
      case CLI_I_GOT_ACCEPT:
      clients[ci].status = SER_CLI_LIVE;
      break;

      case CLI_I_REQUEST:
      e.type = SER_I_REPLY;
      ser_on(&e);
      break;

      case CLI_I_INFO:
      // TODO
      break;
    }

    // If the cursor was anything more than 0 we send
    if (ser_sock->pout.cur > 0)
    {
      net_set_addr(ser_sock, &clients[ci].addr, clients[ci].port);
      net_flush(ser_sock);
    }
  }
}

void
ser_free()
{
  net_close(ser_sock);
  ser_sock = NULL;
}
