#include "../net.hpp"
#include "../com.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <cerrno>
#include <cstdio>

namespace net
{
  bool initialized = false;

  static WSADATA wsadata;

  void initialize()
  { 
    if (initialized)
    {
      return;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
    {
      throw com::system_ex_t("WSAStartup failed.");
    }

    host_name = new char[NI_MAXHOST];
    gethostname(const_cast<char*>(host_name), NI_MAXHOST);


    struct addrinfo* result;

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET6;   // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int result2;
    if ( (result2 = getaddrinfo(host_name, NULL, &hints, &result)) ) {
      throw com::system_ex_t("Failed to get list of host addresses.");
    }

    for (; result != nullptr; result = result->ai_next)
    {
      if (result->ai_family == AF_INET6 && !(result->ai_flags & IFF_LOOPBACK)) // IPv6, not loopback
      {
        struct sockaddr_in6* IPv6 = reinterpret_cast<struct sockaddr_in6*>(result->ai_addr);
        memcpy(&host_addr, &(IPv6->sin6_addr), 16);
        break; // That's it found.
      }
    }

    char str[ADDRSTRLEN];
    atos(str, host_addr, ADDRSTRLEN);
    printf("Networking module initialized, host address is %s.\n", str);

    initialized = true;
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }
    delete [] host_name;
    WSACleanup();

    puts("Networking module shutdown.");
    initialized = false;
  }

  void stoa(addr_t& addr, const char* str)
  {
    struct sockaddr_in6 sin6;
    inet_pton(AF_INET6, str, &(sin6.sin6_addr));
    memcpy(&addr, &(sin6.sin6_addr.s6_addr), 16);
  }

  void atos(char* str, const addr_t& addr, int size)
  {	
    struct sockaddr_in6 sin6;

    sin6.sin6_family = AF_INET6;
    memcpy(&(sin6.sin6_addr.s6_addr), &addr, 16);

    // inet_ntop stack overflows if the size is incorrect
    if (size < INET6_ADDRSTRLEN)
    {
      if (size)
      {
        str[0] = 0;
      }
      return;
    }

    // Convert the IPv6 address to a string
    inet_ntop(AF_INET6, &(sin6.sin6_addr), str, size);
  }

  sock_t::sock_t()
  {
    fd = INVALID_SOCKET;
  }

  sock_t::sock_t(bool server)
  {
    fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (fd == INVALID_SOCKET)
    {
      throw com::net_ex_t("Opening socket.");
    }

    // Make socket non blocking
    {
      unsigned long mode = 1; 
      if (ioctlsocket(fd, FIONBIO, &mode))
      {
        throw com::net_ex_t("Making socket non blocking, essential for net.");
      }
    }

    if (server)
    {
      struct sockaddr_in6 sin6 = {0};
      sin6.sin6_family = AF_INET6;

      if (bind(fd, (const struct sockaddr *)&sin6, sizeof(sin6)))
      {
        throw com::net_ex_t("Binding socket for server use.");
      }

      socklen_t addr_len = sizeof(sin6);
      getsockname(fd, (struct sockaddr *)&sin6, &addr_len);
      bound_port = sin6.sin6_port;
    }
    else
    {
      bound_port = 0; // No bind port.
    }
  }

  void sock_t::close()
  {
    if (fd != INVALID_SOCKET) // Because it's also called in destructor and sock() is allowed
    {
      fd = INVALID_SOCKET;
      closesocket(fd);
    }
  }

  bool sock_t::sendto(const char* data, int n)
  {
    COM_PARANOID_A(fd != INVALID_SOCKET, "net_t::sock_t: Operation with invalid socket");

    struct sockaddr_in6 addr = {0};

    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &pout.addr, 16);
    addr.sin6_port = pout.port;
    
    int r = ::sendto(fd, data, n, 0, (const struct sockaddr*)&addr, sizeof(addr));
    
    if (r == n)
    {
      return true;
    }

    return false;
  }

  bool sock_t::flush()
  {
    COM_PARANOID_A(fd != INVALID_SOCKET, "net_t::sock_t: Operation with invalid socket");

    pout.size = pout.cur;

    struct sockaddr_in6 addr = {0};

    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &pout.addr, 16);
    addr.sin6_port = pout.port;
    
    int r = ::sendto(fd, pout.data, pout.size, 0, (const struct sockaddr*)&addr, sizeof(addr));
    
    if (r == pout.size)
    {
      return true;
    }

    return false;
  }

  bool sock_t::refresh()
  {
    COM_PARANOID_A(fd != INVALID_SOCKET, "net_t::sock_t: Operation with invalid socket");

    struct sockaddr_in6 addr = {0};
    socklen_t addr_len = sizeof(addr);
    
    int r = ::recvfrom(fd, pin.data, MAX_PACK_SIZE, 0, (struct sockaddr*)&addr, &addr_len);
    
    if (r > 0 && addr_len == sizeof(addr))
    {
      pin.port = addr.sin6_port;
      memcpy(&pin.addr, &addr.sin6_addr, 16);
      pin.size = r;

      pin.cur = 0; // Reset the cursor, this was not here until a very long time just saying, had to fix this.

      return true;
    }
    
    return false;
  }
}
