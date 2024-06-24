#include "net.h"
#include "ser.h"
#include "cli.h"
#include "tmr.h"
#include "local.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_REFRESHES_PER_RUN (SER_MAX_CLIENTS+8)

#define MAX_WAIT_MS 3000

#define INFO_COOLDOWN 500

static const char* alias;
static const char* desc;

net_sock_t* ser_sock = NULL;

ser_client_t ser_clis[SER_MAX_CLIENTS];
int ser_clis_n;

static tmr_ms_t last_tick_ms;
static tmr_ms_t last_info_ms; // Last time info was requested

int ser_def_on(ser_event_t* e)
{
  return 0;
}

int (*ser_on)(ser_event_t* e) = ser_def_on;

int
ser_init(const char* _alias, const char* _desc)
{
  alias = _alias;
  desc = _desc;

  ser_clis_n = 0;
  last_tick_ms = tmr_now();

  for (int i = 0; i < SER_MAX_CLIENTS; i++)
  {
    ser_clis[i].status = SER_CLI_FREE;
  }
  
  if ((ser_sock = net_open(1)) == NULL)
  {
    return 0;
  }

  int x = com_big16(ser_sock->bind_port);
  printf("net_open(): Opened server socket, port %i.\n", x);
  return 1;
}

static void
live_client(int i)
{
  if (ser_clis[i].status != SER_CLI_LIVE)
  {
    ser_clis[i].status = SER_CLI_LIVE;
    ser_clis_n++;
  }
}

static void
free_client(int i)
{
  if (ser_clis[i].status != SER_CLI_FREE)
  {
    // 0 out the address so a disconnected client can't just play around.
    ser_clis[i].addr.l[0] = 0;
    ser_clis[i].addr.l[1] = 0;

    ser_clis[i].status = SER_CLI_FREE;
    ser_clis_n--;
  }
}

//
static void
handle_wait_status(int i, tmr_ms_t now)
{
  if (now - ser_clis[i].last_pack_ms >= MAX_WAIT_MS) // Too late to confirm.
  {
    free_client(i);
  }
  else if (now - ser_clis[i].last_pack_ms >= MAX_WAIT_MS/2) // Last chance to confirm the accept, we resend it.
  {
    char data[2] = {SER_I_JOIN, i};
    net_sendto(ser_sock, data, 2);
  }
}

void
ser_run()
{
  ser_event_t e;
  tmr_ms_t now = tmr_now();

  // Time for a tick!
  if (now - last_tick_ms >= SER_TICK_RATE)
  {
    // ser_on() the tick
    e.type = SER_E_TICK;
    e.i = -1;
    net_rewind(ser_sock);
    net_put8(ser_sock, SER_I_TICK);
    ser_on(&e);
    
    if (ser_sock->pout.cur > 1) // If a tick was written handle both sending it and the wait ser_clis.
    {
      for (int i = 0; i < SER_MAX_CLIENTS; i++)
      {
        switch (ser_clis[i].status)
        {
          case SER_CLI_LIVE:
          // Idle for too long
          if (ser_clis[i].last_pack_ms >= PROT_MAX_IDLE_MS)
          {
            free_client(i);
            break;
          }

          net_set_addr(ser_sock, &ser_clis[i].addr, ser_clis[i].port);
          net_flush(ser_sock);
          break;

          case SER_CLI_WAIT:
          handle_wait_status(i, now);
          break;
        }
      }
    }
    else // Only handle waits
    {
      for (int i = 0; i < SER_MAX_CLIENTS; i++)
      {
        if (ser_clis[i].status == SER_CLI_WAIT)
        {
          handle_wait_status(i, now);
        }
      }
    }

    last_tick_ms = tmr_now();
  }

  // Now requests
  for (int _refresh_i = 0; _refresh_i < MAX_REFRESHES_PER_RUN && net_refresh(ser_sock); _refresh_i++)
  {
    int8_t first_byte;
    if (!net_can_get8(ser_sock))
    {
      continue;
    }
    net_get8(ser_sock, &first_byte);

    int8_t ci;
    if (!net_can_get8(ser_sock))
    {
      continue;
    }
    net_get8(ser_sock, &ci);

    // Join request
    if (first_byte == CLI_I_JOIN)
    {
      int c_alias_n = net_gets_n(ser_sock);

      if (c_alias_n <= SER_MAX_CLI_ALIAS) // Everything is fine, can technically accept
      {
        const char* c_alias;
        net_getb(ser_sock, &c_alias, c_alias_n);

        net_set_addr(ser_sock, &ser_sock->pin.addr, ser_sock->pin.port);
        net_rewind(ser_sock);

        ser_sock->pout.cur = 2; // Allocate space for either join header
        e.type = SER_E_JOIN;
        e.i = -1;
        e.join.accepted = 1;
        ser_on(&e);
        
        if (e.join.accepted)
        {
          if (ser_clis_n >= SER_MAX_CLIENTS)
          {
            goto _reject_client;
          }

          // Find a free slot
          unsigned char ci;
          for (ci = 0; ci < SER_MAX_CLIENTS; ci++)
          {
            if (ser_clis[ci].status == SER_CLI_FREE)
            {
              break;
            }
          }

          // Setup the client
          ser_clis[ci].status = SER_CLI_WAIT;
          ser_clis[ci].last_pack_ms = tmr_now();
          ser_clis[ci].port = ser_sock->pin.port;
          ser_clis[ci].addr = ser_sock->pin.addr;
          memcpy(ser_clis[ci].alias, c_alias, c_alias_n);

          ser_sock->pout.data[0] = SER_I_JOIN;
          ser_sock->pout.data[1] = ci;
          net_flush(ser_sock);

        }
        else // !e.join.accepted
        {
          goto _reject_client;
        }
      }
      else
      {
        _reject_client:
        ser_sock->pout.data[0] = SER_I_JOIN;
        ser_sock->pout.data[1] = -1; // Rejected
        net_flush(ser_sock);
      }

      continue;
    }
    // Info request, as disjoined client
    if (first_byte == CLI_I_INFO && ci < 0)
    {
      net_set_addr(ser_sock, &ser_sock->pin.addr, ser_sock->pin.port);
      net_rewind(ser_sock);
      net_put8(ser_sock, SER_I_INFO);
      net_put8(ser_sock, ser_clis_n);
      net_puts(ser_sock, alias);
      net_puts(ser_sock, desc);

      e.type = SER_E_INFO;
      e.i = -1;
      ser_on(&e);

      net_flush(ser_sock);

      continue;
    }
    
    // Invalid index
    if (ci < 0 || ci >= SER_MAX_CLIENTS || ser_clis[ci].status == SER_CLI_FREE)
    {
      continue;
    }

    // The wrong address sent this index.
    if ( ser_sock->pin.addr.l[0] != ser_clis[ci].addr.l[0] || 
         ser_sock->pin.addr.l[1] != ser_clis[ci].addr.l[1] )
    {
      continue;
    }

    ser_clis[ci].port = ser_sock->pin.port; // Port may have changed.
    ser_clis[ci].last_pack_ms = tmr_now();

    net_rewind(ser_sock);
    int do_flush = 0; // Whether to do a flush back to the client
    e.i = ci; // index for event

    // If the client is on wait status we only accept one type of pack
    if (ser_clis[ci].status == SER_CLI_WAIT)
    {
      if (first_byte == CLI_I_GOT_ACCEPT)
      {
        live_client(ci);
        printf("ser_run(): '%s' joined at index %hhi.\n", ser_clis[ci].alias, ci);
      }

      continue;
    }

    switch(first_byte)
    {
      case CLI_I_REQUEST:
      net_put8(ser_sock, SER_I_REPLY);

      e.type = SER_E_REQUEST;
      ser_on(&e);
      
      do_flush = ser_sock->pout.cur > 1;
      break;

      case CLI_I_EXIT:
      e.type = SER_E_EXIT;
      ser_on(&e);
      free_client(ci);
      break;

      case CLI_I_INFO: // Already did all the logic, it will 100% be info as joined client.
      net_put8(ser_sock, ser_clis_n);

      e.type = SER_E_INFO;
      ser_on(&e);

      do_flush = 1;
      break;
    }

    // If the cursor was anything more than 0 we send
    if (do_flush)
    {
      net_set_addr(ser_sock, &ser_clis[ci].addr, ser_clis[ci].port);
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
