#include "clk.h"

#include <windows.h>
#include <stdio.h>

static int res;

// Initialize right before the loop
void
clk_init(fip_t initial_tick_time)
{
  TIMECAPS tc;
  timeGetDevCaps(&tc, sizeof (tc));

  int res = tc.wPeriodMin; // in milliseconds
  timeBeginPeriod(tc.wPeriodMin);

  printf("clk_init(): Cloak module initialized, %dns resolution.\n", res*1000000);
}

void
clk_wait(fip_t secs)
{
  unsigned long long m = FIP_FRAC(secs) * (1 << FIP_FRAC_BITS) / 1000;
  Sleep(m);
}

void clk_free()
{
  timeEndPeriod(tc.wPeriodMin);
}

fip_t
clk_now()
{
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  // Simple coversion from 100ns to fip seconds.
  return ft.dwLowDateTime * 10000000 / (1 << FIP_FRAC_BITS);
}

