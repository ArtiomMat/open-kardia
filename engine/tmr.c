#include "tmr.h"

#ifdef DEBUG
  #include <stdio.h>
#endif

tmr_time_t tmr_target_tick_time;
tmr_time_t tmr_tick_time;

tmr_time_t tmr_begin_time = 0;

void
tmr_begin_tick()
{
  tmr_begin_time = tmr_now();
}

int
tmr_end_tick()
{
  tmr_time_t now = tmr_now();
  tmr_time_t delta = now - tmr_begin_time;

  if (delta < tmr_target_tick_time)
  {
    tmr_wait(tmr_target_tick_time-delta);
    tmr_tick_time = tmr_target_tick_time;
    return 0;
  }

  #ifdef DEBUG
    puts("SKIP");
  #endif
  tmr_tick_time = delta; // The real tick time is just delta
  return 1;
}

