#include "../tmr.h"

#ifndef __USE_POSIX199309
  #define __USE_POSIX199309
#endif

#include <time.h>
#include <stdio.h>

static unsigned long long t0;

// Initialize right before the loop
void
tmr_init(tmr_ms_t initial_tick_time)
{
  tmr_target_tick_time = tmr_tick_time = initial_tick_time;

  // Setup t0
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  t0 = ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000));

  #define RESOLUTION_TEST_TIME 100

  tmr_wait(RESOLUTION_TEST_TIME);
  tmr_ms_t t1 = tmr_now();

  if (RESOLUTION_TEST_TIME - t1 <= 0)
  {
    printf("tmr_init(): Timer module initialized, <1ms resolution.\n");
  }
  else
  {
    printf("tmr_init(): Timer module initialized, %lldms resolution.\n", RESOLUTION_TEST_TIME - t1);
  }
}

void
tmr_wait(tmr_ms_t millis)
{
  struct timespec req, rem;
  req.tv_sec = millis / 1000;
  req.tv_nsec = (millis % 1000) * 1000000;
  nanosleep(&req, &rem);
}

tmr_ms_t
tmr_now()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return ((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000)) - t0;
}

