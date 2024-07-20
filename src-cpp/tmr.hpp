// Cloak module, for taking care of time and shit

#pragma once

#include "com.hpp"

namespace tmr
{
  // Miliseconds, can contain ((2^31)-1)/1000/60/60/24=24.8 days worth of play-time, since everything is measured relative to tmr_init.
  // Signed so that there is no unexpected stuff when math is done and <0 is used.
  typedef int32_t ms_t;

  // The target tick time that we want, may not always be able to fufil this time, depending on the performance of the program and how high it is. Check tick_time.
  // In seconds, the module uses this to decide how much it must wait at the end of the tick to fufil the time.
  // Can be modified in real time.
  extern ms_t target_tick_time;

  // The true time that took the tick is stored here, it will most likely be equal to target_tick_time, but it may not if the tick took too long.
  extern ms_t tick_time;

  // Updated as now() the moment you call begin_tick
  extern ms_t begin_time;

  extern bool initialized;

  // Initialize right before the loop
  void initialize(ms_t initial_tick_time);

  void shutdown();

  // Thread safe.
  // Returns the time in miliseconds since initialization of the cloak module
  ms_t now();

  // Thread safe.
  void wait(ms_t t);

  void begin_tick();

  // Returns how many frames to skip(optimize, like remove rendering or just heavy and not necessary parts) due to the frame taking too long
  int end_tick();
}
