#include "../net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
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
  inet_ntop(AF_INET6, addr->b, str, NET_ADDRSTRLEN);
  return 1;
}

int
net_init()
{
  struct ifaddrs* ia = NULL;

  getifaddrs(&ia);

  int found = 0;
  for (struct ifaddrs* x = ia; x != NULL; x = x->ifa_next)
  {
    if (x->ifa_addr != NULL && x->ifa_addr->sa_family == AF_INET6)
    {
      // Skip loopback address
      if (!strcmp("lo", x->ifa_name))
      {
        continue;
      }

      found = 1;
      memcpy(&net_host_addr, &((struct sockaddr_in6 *)x->ifa_addr)->sin6_addr, 16);

      break;
    }
  }

  if (ia != NULL)
  {
    freeifaddrs(ia);
  }

  if (found)
  {
    char str[64];
    net_atos(str, &net_host_addr);
    printf("net_init(): Networking module initialized, host address is %s.\n", str);
    return 1;
  }
  
  fputs("net_init(): No IPv6 address available.", stderr);
  return 0;
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
      _oopsies2:
      fputs("net_open(): Failed to make socket non blocking, essential for net.\n", stderr);
      _oopsies:
      net_close(sock);
      return NULL;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sock->fd, F_SETFL, flags))
    {
      goto _oopsies2;
    }
  }

  if (server)
  {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sock->fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      fputs("net_open(): Failed to bind socket for server use.\n", stderr);
      goto _oopsies;
    }
  }

  return sock;
}

void
net_close(net_sock_t* s)
{
  if (s == NULL)
  {
    return;
  }
  close(s->fd);
  free(s);
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

