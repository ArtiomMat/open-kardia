#include "net.h"
#include "ser.h"
#include "cli.h"
#include "clk.h"
#include "local.h"

#include <stdio.h>

#define MAX_WAIT_TIME 1000

static const char* alias;

net_sock_t* ser_sock = NULL;

ser_client_t clients[SER_MAX_CLIENTS] = {0};
static int clients_n = 0;

static clk_time_t last_tick;

int
ser_init(const char* _alias)
{
  alias = _alias;

  clients_n = 0;
  last_tick = clk_now();

  for (int i = 0; i < SER_MAX_CLIENTS; i++)
  {
    clients[i].status = SER_CLI_FREE;
  }
  
  if (cli_sock == NULL)
  {
    ser_sock = net_open(1);
  }
  else
  {
    net_close(cli_sock);
    cli_sock = ser_sock = net_open(1);
  }
}

static void
free_client(int i)
{
  clients[i].status = SER_CLI_FREE;
}

void
ser_run()
{
  ser_event_t e;

  // Time for a tick!
  if (clk_now() - last_tick >= SER_TICK_RATE)
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
          if (clients[i].last_pack_time >= MAX_WAIT_TIME) // Too late to confirm.
          {
            free_client(i);
          }
          else if (clients[i].last_pack_time >= MAX_WAIT_TIME/2) // Last chance to confirm the accept, we resend it.
          {
            char data[] = {i};
            net_sendto(ser_sock, data, 1);
          }
        }
      }
    }

    last_tick = clk_now();
  }

  // Now requests
  while (net_refresh(ser_sock))
  {
    uint8_t u8;
    int8_t i8;

    if (!net_can_get8(ser_sock))
    {
      continue;
    }
    net_get8(ser_sock, &i8);

    // Join request
    if (i8 == -1)
    {
      int alias_n = net_gets_n(ser_sock);

      if (alias_n <= SER_MAX_ALIAS)
      {
        e.type = SER_E_JOIN;
        e.join.accepted = 1;
        ser_on(&e);

        net_set_addr(ser_sock, &ser_sock->pin.addr, ser_sock->pin.port);
        net_rewind(ser_sock);
        
        if (e.join.accepted)
        {
          if (clients_n >= SER_MAX_CLIENTS) // Too many
          {
            goto _reject_client;
          }
          // Find a free slot
          char client_i;
          for (client_i = 0; client_i < SER_MAX_CLIENTS; client_i++)
          {
            if (clients[client_i].status == SER_CLI_FREE)
            {
              break;
            }
          }
          // Setup the client
          clients[client_i].status = SER_CLI_WAIT;
          clients[client_i].last_pack_time = clk_now();
          clients[client_i].port = ser_sock->pin.port;
          clients[client_i].addr = ser_sock->pin.addr;
          // TODO: Alias stuff

          net_put8(ser_sock, client_i);
        }
        else // !e.join.accepted
        {
          _reject_client:
          net_put8(ser_sock, -1);
        }

        net_flush(ser_sock);
      }
      else // Alias too long
      {

      }

      continue;
    }

    // Invalid in general
    if (i8 < 0 || i8 >= SER_MAX_CLIENTS || clients[i8].status == SER_CLI_FREE)
    {
      continue;
    }

    // The wrong address sent this index.
    if (
      ser_sock->pin.addr.l[0] != clients[i8].addr.l[0] || 
      ser_sock->pin.addr.l[1] != clients[i8].addr.l[1]
      )
    {
      // TODO: something about it.
      continue;
    }

    clients[i8].port = ser_sock->pin.port; // Port may have changed.
    
    // TODO: Now process request
  }
}

void
ser_free()
{
  net_close(ser_sock);
  ser_sock = NULL;
}
