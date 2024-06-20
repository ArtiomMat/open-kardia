#include "net.h"
#include "cli.h"
#include "ser.h"

#include <stdio.h>

static const char* alias;

net_sock_t* cli_sock = NULL;

int
cli_init(const char* _alias)
{
  alias = _alias;
  if (ser_sock == NULL)
  {
    cli_sock = net_open(0);
  }
  else
  {
    cli_sock = ser_sock;
  }
}
