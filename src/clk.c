#include "clk.h"

fip_t clk_target_tick_time;
fip_t clk_tick_time;

static fip_t begin_time; 

#ifdef __linux__
  #include "nix/com.c"
#elif _WIN32
  #include "win/com.c"
#endif

void
clk_begin_tick()
{
  begin_time = clk_now();
}

int
clk_end_tick()
{
  fip_t now = clk_now();
  fip_t delta = now - begin_time;

  if (delta < clk_target_tick_time)
  {
    clk_wait(clk_target_tick_time-delta);
    clk_tick_time = clk_target_tick_time;
    return 0;
  }
  
  #ifdef DEBUG
    puts("SKIP");
  #endif
  clk_tick_time = delta; // The real tick time is just delta
  return 1;
}

