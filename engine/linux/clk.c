#include "../clk.h"

#include <time.h>
#include <stdio.h>

static clk_time_t t0;

// Initialize right before the loop
void
clk_init(clk_time_t initial_tick_time)
{
  clk_target_tick_time = clk_tick_time = initial_tick_time;

  struct timespec res;
  clock_getres(CLOCK_MONOTONIC, &res);
  
  t0 = clk_now();

  printf("clk_init(): Cloak module initialized, %ldns resolution.\n", res.tv_nsec);
}

void
clk_wait(clk_time_t millis)
{
  struct timespec req, rem;
  req.tv_sec = millis / 1000;
  req.tv_nsec = (millis % 1000) * 1000000;
  nanosleep(&req, &rem);
}

clk_time_t
clk_now()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000)) - t0;
}

