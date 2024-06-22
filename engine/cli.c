#include "net.h"
#include "cli.h"
#include "ser.h"
#include "local.h"

#include <stdio.h>

// To also avoid any accidental receives.
#define MAX_REFRESHES_PER_RUN 3

static const char* alias;

net_sock_t* cli_sock = NULL;

static uint8_t my_index;

static char want_reply;
static char want_info;
static char want_join;

int cli_def_on(cli_event_t* e)
{
  return 0;
}

int (*cli_on)(cli_event_t* e) = cli_def_on;

int
cli_init(const char* _alias)
{
  alias = _alias;
  cli_sock = net_open(0);
  want_reply = want_info = want_join = 0;
}

void
cli_run()
{
  cli_event_t e;

  for (int _refresh_i = 0; _refresh_i < MAX_REFRESHES_PER_RUN && net_refresh(cli_sock); _refresh_i++)
  {
    // Wrong port or wrong address
    if (
      cli_sock->pin.addr.l[0] != cli_sock->pout.addr.l[0] || 
      cli_sock->pin.addr.l[1] != cli_sock->pout.addr.l[1] ||
      cli_sock->pin.port != cli_sock->pout.port
      )
    {
      continue;
    }

    // All good, this is the server talking
    int8_t i8;
    if (!net_can_get8(cli_sock))
    {
      continue;
    }
    net_get8(cli_sock, &i8);

    switch (i8)
    {
      case SER_I_ACCEPT: // Accepted :D
      case SER_I_REJECT:

      if (!want_join)
      {
        break;
      }

      if (i8 == SER_I_ACCEPT) // ACCEPTED :D
      {
        if (!net_can_get8(cli_sock))
        {
          _bad_server:
          puts("cli_run(): Bad server.");
          return;
        }
        net_get8(cli_sock, &my_index);
        e.join.accepted = 1;
        puts("cli_run(): OMFG I WAS ACCEPTED!!!");
      }
      else // REJECTED :(
      {
        e.join.accepted = 0;
      }
      
      e.type = CLI_E_JOIN;
      cli_on(&e);
      break;

      case SER_I_REPLY:
      if (!want_reply)
      {
        break;
      }
      e.type = CLI_E_REPLY;
      cli_on(&e);
      break;

      case SER_I_TICK:
      e.type = CLI_E_TICK;
      cli_on(&e);
      break;
    }
  }
}

void
cli_begin_request(int _want_reply)
{
  net_rewind(cli_sock);
  net_put8(cli_sock, CLI_I_REQUEST);
  net_put8(cli_sock, my_index);
  want_reply = _want_reply;
}

int
cli_join(net_addr_t* addr, net_port_t port)
{
  net_set_addr(cli_sock, addr, port);
  net_rewind(cli_sock);
  
  net_put8(cli_sock, CLI_I_JOIN);
  net_puts(cli_sock, alias);
  
  net_flush(cli_sock);
  want_join = 1;
}
