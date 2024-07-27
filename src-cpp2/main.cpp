#include "xal.hpp"

#include <cstdlib>
#include <cstdio>
#include <exception>

static void my_terminate()
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

  xal::total_shutdown(false);

  std::abort();
}

int main(int args_n, const char** args)
{
  puts("\nINITIALIZING...\n");
  std::set_terminate(my_terminate);

  com::initialize(args_n, args);
  net::initialize();
  dsp::initialize("Xalurtia");
  tmr::initialize(40);
  wav::initialize(100);
  xal::initialize();

  puts("\nRUNNIG...\n");

  while (xal::initialized)
  {
    xal::run();
  }

  xal::total_shutdown(false);

  return 0;
}