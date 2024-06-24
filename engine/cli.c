#include "net.h"
#include "cli.h"
#include "ser.h"
#include "local.h"

#include <string.h>
#include <stdio.h>

// To also avoid any accidental receives.
#define MAX_REFRESHES_PER_RUN 3

static const char* alias;

net_sock_t* cli_sock = NULL;

static int8_t my_index = -1;

static char want_reply;
static char want_info; // 0 for nope, 1 for yope but not as a client, 2 for yep but as a joined client.
static char want_join;

tmr_ms_t info_ask_ms; // When the info was asked for

int cli_def_on(cli_event_t* e)
{
  return 0;
}

int (*cli_on)(cli_event_t* e) = cli_def_on;

int
cli_init(const char* _alias)
{
  alias = _alias;
  if (strlen(alias)+1 >= SER_MAX_CLI_ALIAS)
  {
    puts("cli_init(): Alias too long.");
    return 0;
  }

  if ((cli_sock = net_open(0)) == NULL)
  {
    return 0;
  }
  want_reply = want_info = want_join = 0;
  my_index = -1;

  puts("cli_init(): Opened client socket.");
  return 1;
}

static inline int is_disjoined()
{
  return my_index == -1 && !want_join;
}

void
cli_free()
{
  cli_exit(); // Do the server a favor
  net_close(cli_sock);
}

void
cli_run()
{
  // Disconnected
  if (is_disjoined())
  {
    return;
  }

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
    int8_t first_byte;
    if (!net_can_get8(cli_sock))
    {
      continue;
    }
    net_get8(cli_sock, &first_byte);

    switch (first_byte)
    {
      case SER_I_JOIN:
      if (!want_join)
      {
        break;
      }
      want_join = 0;

      if (!net_can_get8(cli_sock))
      {
        _bad_server:
        puts("cli_run(): Bad server.");
        return;
      }
      net_get8(cli_sock, &my_index);

      if (my_index >= 0) // ACCEPTED :D
      {
        e.join.accepted = 1;

        // Notify server that we got it
        net_rewind(cli_sock);
        net_put8(cli_sock, CLI_I_GOT_ACCEPT);
        net_put8(cli_sock, my_index);
        net_flush(cli_sock);

        printf("cli_run(): Joined at index %hhi.\n", my_index);
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
      want_reply = 0;

      e.type = CLI_E_REPLY;
      cli_on(&e);
      break;

      case SER_I_TICK:
      e.type = CLI_E_TICK;
      cli_on(&e);
      break;

      case SER_I_INFO:
      tmr_ms_t now = tmr_now();
      if (!want_info)
      {
        break;
      }
      int prev_want_info = want_info;
      want_info = 0;

      // clis_n
      if (!net_can_get8(cli_sock))
      {
        break;
      }
      net_get8(cli_sock, &e.info.clis_n);

      if (prev_want_info == 2) // Extra info
      {
        // We can net_gets with no can function it's safe.
        net_gets(cli_sock, &e.info.alias);
        net_gets(cli_sock, &e.info.desc);
      }
      else
      {
        e.info.alias = e.info.desc = NULL;
      }

      e.info.pp_ms = now - info_ask_ms; // Ping pong time
      e.type = CLI_E_INFO;
      cli_on(&e);
      break;
    }
  }
}

int
cli_begin_request(int _want_reply)
{
  // If this is a request we want a reply for again, but shit that is not good.
  if (_want_reply && want_reply)
  {
    return 0;
  }
  
  net_rewind(cli_sock);
  net_put8(cli_sock, CLI_I_REQUEST);
  net_put8(cli_sock, my_index);
  want_reply = _want_reply;

  return 1;
}

int
cli_info(int as_joined)
{
  if (want_info)
  {
    return 0;
  }

  net_rewind(cli_sock);
  net_put8(cli_sock, CLI_I_INFO);
  // my_index is -1 anyway if not joined, so still ok.
  net_put8(cli_sock, as_joined ? my_index : -1);
  
  want_info = 1 + !(as_joined);

  info_ask_ms = tmr_now();
  return net_flush(cli_sock);
}

int
cli_exit()
{
  if (is_disjoined())
  {
    return 0;
  }

  net_rewind(cli_sock);
  net_put8(cli_sock, CLI_I_EXIT);
  net_put8(cli_sock, my_index);

  my_index = -1;
  want_reply = want_info = want_join = 0;

  return net_flush(cli_sock);
}

int
cli_join()
{
  cli_exit(); // Exit first any server we are in

  net_rewind(cli_sock);
  
  net_put8(cli_sock, CLI_I_JOIN);
  net_put8(cli_sock, -1);
  net_puts(cli_sock, alias);
  
  want_join = 1;

  return net_flush(cli_sock);
}
