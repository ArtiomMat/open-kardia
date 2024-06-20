#include "net.h"
#include "ser.h"
#include "cli.h"
#include "clk.h"

#include <stdio.h>

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
    clients[i].free = 1;
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
    
    // Only send if there is any data in the tick
    if (ser_sock->pout.cur > 0)
    {
      for (int j = 0, i = 0; j < clients_n, i < SER_MAX_CLIENTS; i++)
      {
        if (clients[i].free)
        {
          continue;
        }



        j++;
      }
    }

    last_tick = clk_now();
  }

  // Now requests
  while (net_refresh(ser_sock))
  {
    
  }
}

void
ser_free()
{
  net_close(ser_sock);
  ser_sock = NULL;
}
