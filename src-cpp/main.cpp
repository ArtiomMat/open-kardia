#include <cstdio>
#include <exception>
#include  <cstdlib>

#include "xal.hpp"

void main_shutdown_all()
{
  puts("\nSHUTTING DOWN...\n");
  xal::shutdown();
  wav::shutdown();
  dsp::shutdown();
  net::shutdown();
  com::shutdown();
  tmr::shutdown();
}

static void main_terminate()
{
  // fputs("Terminate called.\n", stderr);
  std::exception_ptr ex = std::current_exception();
  try
  {
    std::rethrow_exception(ex);
  }
  catch (com::ex_t& e)
  {
    fprintf(stderr, "Exception was uncaught: '%s'.\n", e.str);
  }

  main_shutdown_all();

  std::abort();
}

int main(int args_n, const char** args)
{
  puts("\nINITIALIZING...\n");

  std::set_terminate(main_terminate);

  com::initialize(args_n, args);
  net::initialize();
  dsp::initialize("Xalurtia");
  tmr::initialize(16);
  wav::initialize(30);
  xal::initialize();
  
  puts("\nRUNNIG...\n");

  while (xal::initialized)
  {
    xal::run();
  }

  main_shutdown_all();

  return 0;
}

