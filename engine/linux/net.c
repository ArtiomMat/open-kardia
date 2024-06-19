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
#include <errno.h>
#include <sys/ioctl.h>
#define __USE_MISC // Should be defined, but the C/C++ vscode extension wont do that...
#include <net/if.h>

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
      // Narrow down search to public IP
      if (x->ifa_flags & IFF_LOOPBACK)
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
  
  net_stoa(&net_loopback, "::1");
  
  fputs("net_init(): No IPv6 address available.", stderr);
  return 0;
}

net_sock_t*
net_open(int server)
{
  net_sock_t* sock = calloc(sizeof (net_sock_t), 1);
  sock->fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  
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

    if (fcntl(sock->fd, F_SETFL, flags | O_NONBLOCK))
    {
      goto _oopsies2;
    }
  }

  if (server)
  {
    struct sockaddr_in6 addr = {0};
    addr.sin6_family = AF_INET6;

    if (bind(sock->fd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
      fputs("net_open(): Failed to bind socket for server use.\n", stderr);
      perror("bind");
      goto _oopsies;
    }

    int addr_len = sizeof(addr);
    getsockname(sock->fd, (struct sockaddr *)&addr, &addr_len);
    sock->bind_port = addr.sin6_port;

    printf("net_open(): Opened server, port %i.\n", ntohs(addr.sin6_port));
  }
  else
  {
    sock->bind_port = 0; // No bind port.
    printf("net_open(): Opened client.\n");
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
net_flush(net_sock_t* s)
{
  s->pout.size = s->pout.cur;

  struct sockaddr_in6 addr = {0};

  addr.sin6_family = AF_INET6;
  memcpy(addr.sin6_addr.__in6_u.__u6_addr8, &s->pout.addr, 16);
  addr.sin6_port = s->pout.port;

  ssize_t r = sendto(s->fd, s->pout.data, s->pout.size, 0, (const struct sockaddr*)&addr, sizeof(addr));
  return r == s->pout.size;
}

int
net_refresh(net_sock_t* s)
{
  struct sockaddr_in6 addr = {0};
  socklen_t addr_len = sizeof(addr);
  
  ssize_t r = recvfrom(s->fd, s->pin.data, NET_MAX_PACK_SIZE, 0, (const struct sockaddr*)&addr, &addr_len);
  
  if (r > 0 && addr_len == sizeof(addr))
  {
    s->pin.port = addr.sin6_port;
    memcpy(&s->pin.addr, addr.sin6_addr.__in6_u.__u6_addr8, 16);
    s->pin.size = r;

    return 1;
  }
  
  return 0;
}

