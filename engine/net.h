// Low(er)-level networking module, platform independent.
// This module is not meant to be used by games, it's only meant to be used for the se and cl modules.

#pragma once

#define NET_MAX_SOCK_DATA 4096
// How many characters for a string
#define NET_ADDRSTRLEN 64

typedef union
{
  char b[16];
  long long l[2];
} net_addr_t;

typedef struct
{
  net_addr_t addr; // Address to send if net_send(), address sent from if net_recv()
  char d[NET_MAX_SOCK_DATA]; // Ensures alignment to 8 bits
  unsigned long long fd;
} net_sock_t;

extern const net_addr_t net_loopback;

// Address of the host itself.
extern net_addr_t net_host_addr;

extern int
net_init();

// IPv6 address string to an actual IPv6.
extern int
net_stoa(net_addr_t* restrict addr, const char* restrict str);

// Converts IPv6 address to IPv6 string.
// str must be
extern int
net_atos(char* restrict str, const net_addr_t* restrict addr);

// If SERVER then we bind and make the server overall available.
extern net_sock_t*
net_open(int server);

extern void
net_close(net_sock_t* s);

// Blacklist current address in S.
extern void
net_blacklist(net_sock_t* s);

// Send a single packet to TO.
extern int
net_send(net_sock_t* s);

// Returns "from" socket, can be a new socket, net will never close the socket on its own.
// Data must be at-least max_data_size from net_init().
extern int
net_recv(net_sock_t* s);
