#include "tmr.hpp"

#include <cstdio>

namespace tmr
{
  ms_t target_tick_time;
  ms_t tick_time;

  ms_t begin_time = 0;

  bool initialized = false;

  void begin_tick()
  {
    begin_time = now();
  }

  int end_tick()
  {
    ms_t delta = now() - begin_time;

    if (delta < target_tick_time)
    {
      wait(target_tick_time-delta);
      tick_time = target_tick_time;
      return 0;
    }
    // printf("%d %d\n", delta, target_tick_time);

    COM_PARANOID_M("tmr::end_tick(): tick longer than target_tick_time.");

    tick_time = delta; // The real tick time is just delta
    return 1;
  }
}
