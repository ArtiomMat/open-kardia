#pragma once

#include "common.hpp"

#define _NET_PAD(WHAT, P) \
  {\
    int __off = WHAT % P;\
    if (__off)\
    {\
      WHAT += (1<<P) - __off;\
    }\
  }

namespace axe
{
  constexpr int MAX_PACK_SIZE = 4096;
  // How many characters for a string that contains IPv6, dictated by WinSock2 as of now.
  constexpr int ADDRSTRLEN = 65;

  typedef unsigned short port_t;

  union addr_t
  {
    uint8_t  b[16];
    uint64_t l[2];
  };

  struct sock_t
  {
    // Pack is short for packet
    struct pack_t
    {
      addr_t addr; // Address to send to or that was received from.
      char data[MAX_PACK_SIZE]; // Aligned to 8 bytes due to ordering in struct
      port_t port; // Always considered big endian/network order, so always must be stored as such.
      unsigned short cur; // Put and Get cursors
      unsigned short size; // How many bytes in data, can be up to MAX_PACK_SIZE. Remains unused until flush is called.
    };

    unsigned long long fd;
    port_t bound_port; // The port to which we are bound. Always interpeted as big endian/network order, so always must be stored as such.
    pack_t pin, pout; // In and out packs
    
    sock_t();

    // Sets FD to a reset value, and platfrom dependent
    sock_t(bool server);
    ~sock_t() { close(); }

    void close();

    inline void set_addr(const addr_t& addr, port_t port)
    {
      pout.addr = addr;
      pout.port = port;
    }

    inline void rewind()
    {
      pout.cur = pout.size = 0;
    }

    // Returns how many bytes left to write into pout after this putting. Does not stop if exceeded write limit, so unsafe.
    inline int put8(uint8_t x)
    {
      pout.data[pout.cur++] = x;
      return MAX_PACK_SIZE - pout.cur;
    }
    // Return same as put8. DOES PADDING IF MISALIGNED!
    int put16(uint16_t x);
    // Return same as put8. DOES PADDING IF MISALIGNED!
    int put32(uint32_t x);
    // Return same as put8. Due to the nature of strings, it will stop if encounters the limit of writing(making this one safe), and will return -1 if an overflow is necessary to complete the putting operation.
    int puts(const char* str);
    // Return same as put8.
    int putb(const char* data, int n);

    // Returns how many more bytes can be read from pin after this getting. Does not stop if exceeded read limit, so unsafe.
    inline uint8_t get8()
    {
      return pin.data[pin.cur++];
    }
    // Return same as put8. DOES PADDING IF MISALIGNED!
    uint16_t get16();
    // Return same as put8. DOES PADDING IF MISALIGNED!
    uint32_t get32();
    // Return same as put8. Puts pointer, doesn't copy data, advances cursor to after the null terminator. If there is no null terminator, no worries, net doesn't trust the sender, it manually null terminates the end of the pack in memory. If string overflowed and had to be null terminated manually, returns -1.
    const char* gets();

    // Get how big the string is(including null terminator), if reaches end without null terminator will return the size measured from the cursor to the end, exactly the way str would work.
    // After this you should use getb because you know n.
    unsigned gets_n();

    // Return same as put8. Puts pointer, doesn't copy data, advances the cursor n bytes.
    const char* getb(int n);

    inline int can_get8()
    {
      return pin.size - (pin.cur + 1) >= 0;
    }

    inline int can_get16()
    {
      int cur = pin.cur; // Include padding
      _NET_PAD(cur, 2);
      return pin.size - (cur + 2) >= 0;
    }
    inline int can_get32()
    {
      int cur = pin.cur; // Include padding
      _NET_PAD(cur, 4);
      return pin.size - (cur + 4) >= 0;
    }

    inline int can_getb(int n)
    {
      return pin.size - (pin.cur + n) >= 0;
    }

    // Instead of sending pin->data, we send data, this has a very specific purpose in the server, when we can't wipe.
    bool sendto(const char* data, int n);

    // Send pout to TO. Returns if sent all the data fully(atleast from our side). Does not rewind() for you.
    bool flush();

    // Wrapper for recvfrom.
    // Returns 1 if bytes were received, if not 0. Writes the packet into pin.
    bool refresh();
  };
}