// Low(er)-level networking module, platform independent.
// This module is not meant to be used by games, it's only meant to be used for the se and cl modules.

#pragma once

#include "com.hpp"

#define NET_PAD(WHAT, P) \
  {\
    int __off = WHAT % P;\
    if (__off)\
    {\
      WHAT += (1<<P) - __off;\
    }\
  }

namespace net
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

  // Pack is short for packet
  struct pack_t
  {
    addr_t addr; // Address to send to or that was received from.
    char data[MAX_PACK_SIZE]; // Aligned to 8 bytes due to ordering in struct
    port_t port; // Always considered big endian/network order, so always must be stored as such.
    unsigned short cur; // Put and Get cursors
    unsigned short size; // How many bytes in data, can be up to MAX_PACK_SIZE. Remains unused until flush is called.
  };

  struct sock_t
  {
    unsigned long long fd;
    port_t bound_port; // The port to which we are bound. Always considered big endian/network order, so always must be stored as such.
    pack_t pin, pout; // In and out packs
    
    sock_t() = default;
    ~sock_t() { close(); }

    bool open(bool server);
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
    inline int get8(uint8_t& x)
    {
      x = pin.data[pin.cur++];
      return pin.size - pin.cur;
    }
    // Return same as put8. DOES PADDING IF MISALIGNED!
    int get16(uint16_t& x);
    // Return same as put8. DOES PADDING IF MISALIGNED!
    int get32(uint32_t& x);
    // Return same as put8. Puts pointer, doesn't copy data, advances cursor to after the null terminator. If there is no null terminator, no worries, net doesn't trust the sender, it manually null terminates the end of the pack in memory. If string overflowed and had to be null terminated manually, returns -1.
    int gets(const char*& str);

    // Get how big the string is(including null terminator), if reaches end without null terminator will return the size measured from the cursor to the end, exactly the way str would work.
    // After this you should use getb because you know n.
    int gets_n();

    // Return same as put8. Puts pointer, doesn't copy data, advances the cursor n bytes.
    int getb(const char*& data, int n);

    inline int can_get8()
    {
      return pin.size - (pin.cur + 1) >= 0;
    }

    inline int can_get16()
    {
      int cur = pin.cur; // Include padding
      NET_PAD(cur, 2);
      return pin.size - (cur + 2) >= 0;
    }
    inline int can_get32()
    {
      int cur = pin.cur; // Include padding
      NET_PAD(cur, 4);
      return pin.size - (cur + 4) >= 0;
    }

    inline int can_getb(int n)
    {
      return pin.size - (pin.cur + n) >= 0;
    }

    // Instead of sending pin->data, we send data, this has a very specific purpose in ser.
    bool sendto(const char* data, int n);

    // Send pout to TO. Returns if sent all the data fully(atleast from our side). Does not rewind() for you.
    bool flush();

    // Wrapper for recvfrom.
    // Returns 1 if bytes were received, if not 0. Writes the packet into pin.
    bool refresh();
  };

  constexpr addr_t loopback = []
  {
    addr_t a{};
    a.b[15] = 1;
    return a;
  }();

  // Address of the host itself.
  extern addr_t host_addr;
  // Initialized only on init
  extern const char* host_name;

  extern bool initialized;

  bool initialize();
  
  void shutdown();

  // IPv6 address string to an actual IPv6.
  void stoa(addr_t& addr, const char* str);
  // Converts IPv6 address to IPv6 string.
  // str must be valid.
  void atos(char* str, const addr_t& addr, int size);
}
