// Low(er)-level networking module, platform independent.
// This module is not meant to be used by games, it's only meant to be used for the se and cl modules.

#pragma once

#include "com.h"

#define NET_MAX_PACK_SIZE 4096
// How many characters for a string
#define NET_ADDRSTRLEN 64

#define NET_PAD(WHAT, P) \
  {\
    int __off = WHAT % P;\
    if (__off)\
    {\
      WHAT += (1<<P) - __off;\
    }\
  }

typedef unsigned short net_port_t;

typedef union
{
  uint8_t  b[16];
  uint64_t l[2];
} net_addr_t;

// Pack is short for packet
typedef struct
{
  net_addr_t addr; // Address to send to or that was received from.
  char data[NET_MAX_PACK_SIZE]; // Aligned to 8 bytes due to ordering in struct
  net_port_t port; // Always considered big endian/network order, so always must be stored as such.
  unsigned short cur; // Put and Get cursors
  unsigned short size; // How many bytes in data, can be up to NET_MAX_PACK_SIZE. Remains unused until flush is called.
} net_pack_t;

typedef struct
{
  unsigned long long fd;
  net_port_t bind_port; // The port to which we are bound. Always considered big endian/network order, so always must be stored as such.
  net_pack_t pin, pout; // In and out packs
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

// Set the address of pout. port must be in network byte order(big endian).
static inline void
net_set_addr(net_sock_t* restrict s, net_addr_t* restrict addr, net_port_t port)
{
  s->pout.addr = *addr;
  s->pout.port = port;
}

// Set pout cursor to 0.
static inline void
net_rewind(net_sock_t* s)
{
  s->pout.cur = s->pout.size = 0;
}

// Returns how many bytes left to write into pout after this putting. Does not stop if exceeded write limit, so unsafe.
static inline int
net_put8(net_sock_t* s, uint8_t x)
{
  s->pout.data[s->pout.cur++] = x;
  return NET_MAX_PACK_SIZE - s->pout.cur;
}
// Return same as put8. DOES PADDING IF MISALIGNED!
extern int
net_put16(net_sock_t* s, uint16_t x);
// Return same as put8. DOES PADDING IF MISALIGNED!
extern int
net_put32(net_sock_t* s, uint32_t x);
// Return same as put8. Due to the nature of strings, it will stop if encounters the limit of writing(making this one safe), and will return -1 if an overflow is necessary to complete the putting operation.
extern int
net_puts(net_sock_t* s, const char* str);
// Return same as put8.
extern int
net_putb(net_sock_t* s, const char* data, int n);

// Returns how many more bytes can be read from pin after this getting. Does not stop if exceeded read limit, so unsafe.
static inline int
net_get8(net_sock_t* s, uint8_t* x)
{
  *x = s->pin.data[s->pin.cur++];
  return s->pin.size - s->pin.cur; // TODO: Not NET_MAX_PACK_SIZE, it's not the size
}
// Return same as put8. DOES PADDING IF MISALIGNED!
extern int
net_get16(net_sock_t* s, uint16_t* x);
// Return same as put8. DOES PADDING IF MISALIGNED!
extern int
net_get32(net_sock_t* s, uint32_t* x);
// Return same as put8. Puts pointer, doesn't copy data, advances cursor to after the null terminator. If there is no null terminator, no worries, net doesn't trust the sender, it manually null terminates the end of the pack in memory. If string overflowed and had to be null terminated manually, returns -1.
extern int
net_gets(net_sock_t* s, const char** str);
// Return same as put8. Puts pointer, doesn't copy data, advances the cursor n bytes.
extern int
net_getb(net_sock_t* s, const char** data, int n);

// Get how big the string is(including null terminator), if reaches end without null terminator will return the size measured from the cursor to the end, exactly the way str would work.
extern int
net_gets_n(net_sock_t* s);

static inline int
net_can_get8(net_sock_t* s)
{
  return s->pin.size - (s->pin.cur + 1) > 0;
}

static inline int
net_can_get16(net_sock_t* s)
{
  int cur = s->pin.cur; // Include padding
  NET_PAD(cur, 2);
  return s->pin.size - (cur + 2) > 0;
}
static inline int
net_can_get32(net_sock_t* s)
{
  int cur = s->pin.cur; // Include padding
  NET_PAD(cur, 4);
  return s->pin.size - (cur + 4) > 0;
}

static inline int
net_can_getb(net_sock_t* s, int n)
{
  return s->pin.size - (s->pin.cur + n) > 0;
}

// Instead of sending pin->data, we send data, this has a very specific purpose in ser.
extern int
net_sendto(net_sock_t* s, const char* data, int n);

// Send pout to TO. Returns if sent all the data fully(atleast from our side). Does not net_rewind() for you.
extern int
net_flush(net_sock_t* s);

// Wrapper for recvfrom.
// Returns 1 if bytes were received, if not 0. Writes the packet into pin.
extern int
net_refresh(net_sock_t* s);
