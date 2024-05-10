#include "clk.h"

#include <time.h>
#include <stdio.h>

// Initialize right before the loop
void
clk_init(fip_t initial_tick_time)
{
  clk_target_tick_time = clk_tick_time = initial_tick_time;

  struct timespec res;
  clock_getres(CLOCK_MONOTONIC, &res);
  printf("clk_init(): Cloak module initialized, %ldns resolution.\n", res.tv_nsec);
}

void
clk_wait(fip_t secs)
{
  struct timespec req, rem;
  req.tv_sec = FIPTOI(secs);
  req.tv_nsec = FIP_FRAC(secs) * 1000000000L / (1 << FIP_FRAC_BITS);
  nanosleep(&req, &rem);
}

fip_t
clk_now()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return ITOFIP(tp.tv_sec) + (tp.tv_nsec * (1 << FIP_FRAC_BITS) / 1000000000L);
}

