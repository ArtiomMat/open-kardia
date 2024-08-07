#include "../tmr.h"

#include <windows.h>
#include <stdio.h>

static int res;
static TIMECAPS tc;

static unsigned long long t0;

// Initialize right before the loop
void
tmr_init(tmr_ms_t initial_tick_time)
{
  timeGetDevCaps(&tc, sizeof (tc));

  int res = tc.wPeriodMin; // in milliseconds
  timeBeginPeriod(tc.wPeriodMin);

  t0 = GetTickCount64();
  
  printf("tmr_init(): Cloak module initialized, %dns resolution.\n", res*1000000);
}

void
tmr_wait(tmr_ms_t milis)
{
  Sleep(milis);
}

void tmr_free()
{
  timeEndPeriod(tc.wPeriodMin);
}

tmr_ms_t
tmr_now()
{
  return GetTickCount64() - t0;
}

