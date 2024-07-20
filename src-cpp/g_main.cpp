#include <cstdio>

#include "com.hpp"
#include "net.hpp"

int main(int args_n, const char** args)
{
  com::initialize(args_n, args);
  net::initialize();

  com::str_t lol = "Fuck";
  puts(lol + " me. I have " + 1.2 + " shekels");

  net::sock_t sock;
  if (sock.open(true))
  {
    puts(net::host_name);
  }

  net::shutdown();
  com::shutdown();
  
  return 0;
}
