#include "../clk.h"

#include <windows.h>
#include <stdio.h>

static int res;
static TIMECAPS tc;

static clk_time_t t0;

// Initialize right before the loop
void
clk_init(clk_time_t initial_tick_time)
{
  timeGetDevCaps(&tc, sizeof (tc));

  int res = tc.wPeriodMin; // in milliseconds
  timeBeginPeriod(tc.wPeriodMin);

  t0 = clk_now();
  
  printf("clk_init(): Cloak module initialized, %dns resolution.\n", res*1000000);
}

void
clk_wait(clk_time_t milis)
{
  Sleep(milis);
}

void clk_free()
{
  timeEndPeriod(tc.wPeriodMin);
}

clk_time_t
clk_now()
{
  return GetTickCount64();
}

