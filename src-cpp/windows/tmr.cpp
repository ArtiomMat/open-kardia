#include "../tmr.hpp"

#include <windows.h>
#include <stdio.h>

namespace tmr
{
  static TIMECAPS tc;

  static ULONGLONG t0;

  // Initialize right before the loop
  void initialize(ms_t initial_tick_time)
  {
    if (initialized)
    {
      return;
    }

    target_tick_time = initial_tick_time;

    timeGetDevCaps(&tc, sizeof (tc));

    timeBeginPeriod(tc.wPeriodMin);

    t0 = GetTickCount64();
    
    printf("Timer module initialized, %dms resolution.\n", tc.wPeriodMin);

    initialized = true;
  }

  void wait(ms_t milis)
  {
    Sleep(milis);
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }
    timeEndPeriod(tc.wPeriodMin);
    initialized = false;
  }

  ms_t now()
  {
    return GetTickCount64() - t0;
  }
}
