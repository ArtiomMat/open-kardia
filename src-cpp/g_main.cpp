#include <cstdio>

#include "com.hpp"
#include "net.hpp"

int main(int args_n, const char** args)
{
  com::initialize(args_n, args);
  net::initialize();

  net::sock_t sock;
  if (sock.open(1))
  {
    puts(net::host_name);
  }

  net::shutdown();
  com::shutdown();
  
  return 0;
}
