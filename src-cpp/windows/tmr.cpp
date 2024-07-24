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

    // timeGetDevCaps(&tc, sizeof (tc));
    // timeBeginPeriod(tc.wPeriodMin);

    t0 = GetTickCount64();
    
    printf("Timer module initialized, %dms resolution.\n", tc.wPeriodMin);

    initialized = true;
  }
  
  void shutdown()
  {
    if (!initialized)
    {
      return;
    }
    // timeEndPeriod(tc.wPeriodMin);

    puts("Timer module shutdown.");
    initialized = false;
  }

  void wait(ms_t milis)
  {
    Sleep(milis);
  }


  ms_t now()
  {
    return GetTickCount64() - t0;
  }
}
