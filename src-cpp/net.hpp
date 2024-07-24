// Low(er)-level networking module, platform independent.
// This module is not meant to be used by games, it's only meant to be used for the se and cl modules.

#pragma once

#include "com.hpp"
#include "tmr.hpp"

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

    // Instead of sending pin->data, we send data, this has a very specific purpose in the server, when we can't wipe.
    bool sendto(const char* data, int n);

    // Send pout to TO. Returns if sent all the data fully(atleast from our side). Does not rewind() for you.
    bool flush();

    // Wrapper for recvfrom.
    // Returns 1 if bytes were received, if not 0. Writes the packet into pin.
    bool refresh();
  };

  constexpr unsigned MAX_SERVER_CLIENTS = 24;

  // How much time between server ticks, can be set to 0 to disable server ticks if the game doesn't need it.
  constexpr tmr::ms_t SERVER_TICK_RATE = 32;

  constexpr unsigned MAX_CLIENT_ALIAS = 24;

  constexpr unsigned MAX_SERVER_ALIAS = 32;
  constexpr unsigned MAX_SERVER_DESC = 256;
  
  constexpr tmr::ms_t MAX_WAIT_MS = 3000;

  constexpr tmr::ms_t MAX_SERVER_REFRESHES_PER_RUN = MAX_SERVER_CLIENTS + 16;
  constexpr tmr::ms_t MAX_CLIENT_REFRESHES_PER_RUN = 8;

  // Values for that first byte that the client sends
  enum
  {
    CLIENT_B_INFO = -127, // for info
    CLIENT_B_ERR, // Who the fuck be the client that they send errors to the server, bro, who gives a shit? Idk if I should keep it here.
    CLIENT_B_JOIN, // for join
    CLIENT_B_GOT_ACCEPT,
    CLIENT_B_REQUEST,
    CLIENT_B_EXIT,
  };

  enum
  {
    SERVER_B_INFO = -127, // for info
    SERVER_B_ERR,
    SERVER_B_JOIN,
    SERVER_B_REPLY,
    SERVER_B_TICK,
  };

  enum
  {
    E_JOIN, // e->join.accepted can be changed to 0, by default will be 1. can freely net_get and net_put your custom extra data, the server already allocated space for headers before calling on. If e->i is -1 it means the client wants the total server information as if not connected(whether they are or not).
    
    E_REQUEST, // The client sent a request, may expect a reply(depends fully on your custom protocol). You now can net_get, and also net_put, if net_put the reply is sent, otherwise no reply.
    
    E_TICK, // It's time to net_put a message for all clients, begin writing, if cursor exceeds 0 will be sent to all clients, otherwise this tick is not considered.
    // SER_E_ALIVE, // A general alive message if the client has nothing to request.
    
    E_INFO, // The client asked for info about the server, can net_put custom extra info now, if cursor exceeds 0 info is sent.
    
    E_CLIENT_EXIT, // Client is exiting. Just note it.

    E_REPLY, // For client only
  };

  struct server_t
  {
    enum
    {
      STATUS_FREE,
      STATUS_WAIT,
      STATUS_LIVE,
    };

    struct event_t
    {
      uint8_t type;
      uint8_t i; // Client index -1 for N/A like for tick, or join(can read address in ser_sock->pin.addr)
      union
      {
        struct
        {
          char accepted; // Whether or not to accpet the join request of the user. 1 by default.
        } join;
      };
    };

    struct client_t
    {
      addr_t addr; // Also used as a sort of key, when clients send their index, checked if the address is in that client index.
      tmr::ms_t last_pack_ms;
      port_t port;
      char status;
      char alias[MAX_CLIENT_ALIAS];
      char requests_n; // How many requests the client made this run, if exceeds a certain value then we ignore the requests to let others request too.
    };

    sock_t sock;

    const char* alias, * desc;

    tmr::ms_t last_tick_ms;
    tmr::ms_t last_info_ms; // Last time info was requested

    client_t clients[MAX_SERVER_CLIENTS];
    // Live clients, does not include clients with wait status
    unsigned clients_n;

    virtual void handler(event_t& e) = 0;

    // desc can be nullptr, alias has to be a valid string.
    server_t(const char* _alias, const char* desc);
    server_t(const char* _alias) : server_t(_alias, nullptr) {}
    virtual ~server_t() = default;

    // Refreshes socket until dried out, and calls handler().
    // Not thread safe, use mutexes if multi-threading.
    void run();

    void _live_client(int i);
    void _free_client(int i);
    void _handle_wait_status(int i, tmr::ms_t now);
  };

  struct client_t
  {
    struct event_t
    {
      int type;
      union
      {
        struct
        {
          char accepted;
        } join;

        struct
        {
          const char* alias; // Temporary and should be copied if needed
          const char* desc; // Temporary and should be copied if needed
          tmr::ms_t pp_ms; // Ping-pong time, may not describe the exact time to literally send and receive.
          uint8_t clients_n;
        } info;
      };
    };

    sock_t sock;

    tmr::ms_t info_ask_ms;

    const char* alias;

    bool want_join = false;
    bool want_reply = false;
    int want_info = 0; // 0 for nope, 1 for yope but not as a client, 2 for yep but as a joined client.
    // Just for us
    bool alias_allocated = false;

    int8_t my_index = -1;

    virtual void handler(event_t& e) = 0;

    client_t(const char * _alias);
    virtual ~client_t();
    
    // Not thread safe, use mutexes if multi-threading.
    void run();
    // Call net_set_addr() first to setup who we connect to, calls cli_exit() if already joined.
    // Returns if the join was sent, from our end.
    void join();
    void exit();
    // Uses cli_sock->pout.addr and port, so call net_set_addr() first
    // Returns if the info request was sent from our end, if already waiting for info, returns 0.
    // as_client 1 means that you receive the info as a client(only if connected, otherwise ignored), otherwise you receive this as a non client, which includes more info, read README for more documentation on what you receive.
    void info(bool as_joined);
    // Calls net_rewind because header must be in beginning. Sets up headers so the server can understand what's going on. After calling this use net_put and flush when ready with cli_sock.
    // Returns false if a reply for a previous request was not sent yet and want_reply=1, you should block until it returns true.
    bool begin_request(bool _want_reply);

    inline bool is_disjoined() const { return my_index == -1 && !want_join; }
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

  void initialize();
  
  void shutdown();

  // IPv6 address string to an actual IPv6.
  void stoa(addr_t& addr, const char* str);
  // Converts IPv6 address to IPv6 string.
  // str must be valid.
  void atos(char* str, const addr_t& addr, int size);
}
