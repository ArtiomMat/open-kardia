#include <cstdio>

#include "com.hpp"
#include "net.hpp"
#include "dsp.hpp"
#include "tmr.hpp"

int main(int args_n, const char** args)
{
  puts("\nINITIALIZE...\n");

  com::initialize(args_n, args);
  net::initialize();
  dsp::initialize();
  tmr::initialize(30);

  com::str_t lol = "Fuck";
  puts(lol + " me. I have " + 1.2 + " shekels");

  // net::sock_t sock(true);
  dsp::ctx_t x(720, 1080);

  puts("\nRUN...\n");

  while (1)
  {
    tmr::begin_tick();

    x.run();
    x.refresh();

    tmr::end_tick();
  }

  puts("\nSHUTDOWN...\n");

  dsp::shutdown();
  net::shutdown();
  com::shutdown();
  tmr::shutdown();
  
  return 0;
}
