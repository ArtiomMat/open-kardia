#include "../net.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

int
net_stoa(net_addr_t* restrict addr, const char* restrict str)
{
  if (inet_pton(AF_INET6, str, addr) == 1)
  {
    return 1;
  }
  return 0;
}

int
net_atos(char* restrict str, const net_addr_t* restrict addr)
{
  inet_ntop(AF_INET6, addr, str, NET_ADDRSTRLEN);
  return 1;
}

net_sock_t*
net_open(int server)
{
  net_sock_t* sock = malloc(sizeof (net_sock_t));
  sock->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // Make socket non blocking
  {
    int flags = fcntl(sock->fd, F_GETFL, 0);

    if (flags == -1)
    {
      _oopsies:
      fputs("net_open(): Failed to make socket non blocking, essential for net.\n", stderr);
      close(sock->fd);
      return NULL;
    }

    flags |= O_NONBLOCK;
    if (!fcntl(sock->fd, F_SETFL, flags))
    {
      goto _oopsies;
    }
  }
}

void
net_close(net_sock_t* s)
{

}

void
net_blacklist(net_sock_t* s)
{

}

int
net_send(net_sock_t* s)
{

}

int
net_recv(net_sock_t* s)
{

}

