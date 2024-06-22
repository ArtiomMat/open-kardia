#include "../tmr.h"

#include <time.h>
#include <stdio.h>
#include <linux/time.h>
#include <bits/time.h>

static tmr_time_t t0;

// Initialize right before the loop
void
tmr_init(tmr_time_t initial_tick_time)
{
  tmr_target_tick_time = tmr_tick_time = initial_tick_time;

  struct timespec res;
  clock_getres(CLOCK_MONOTONIC, &res);
  
  t0 = tmr_now();

  printf("tmr_init(): Timer module initialized, %ldns resolution.\n", res.tv_nsec);
}

void
tmr_wait(tmr_time_t millis)
{
  struct timespec req, rem;
  req.tv_sec = millis / 1000;
  req.tv_nsec = (millis % 1000) * 1000000;
  nanosleep(&req, &rem);
}

tmr_time_t
tmr_now()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000)) - t0;
}

