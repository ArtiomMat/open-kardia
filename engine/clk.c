#include "clk.h"

clk_time_t clk_target_tick_time;
clk_time_t clk_tick_time;

static clk_time_t begin_time;

void
clk_begin_tick()
{
  begin_time = clk_now();
}

int
clk_end_tick()
{
  clk_time_t now = clk_now();
  clk_time_t delta = now - begin_time;

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

