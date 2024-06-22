// Cloak module, for taking care of time and shit

#pragma once

// Measured in miliseconds
typedef long long tmr_time_t;

// The target tick time that we want, may not always be able to fufil this time, depending on the performance of the program and how high it is. Check tmr_tick_time.
// In seconds, the module uses this to decide how much it must wait at the end of the tick to fufil the time.
// Can be modified in real time.
extern tmr_time_t tmr_target_tick_time;

// The true time that took the tick is stored here, it will most likely be equal to tmr_target_tick_time, but it may not if the tick took too long.
extern tmr_time_t tmr_tick_time;

// Updated as tmr_now() the moment you call tmr_begin_tick
extern tmr_time_t tmr_begin_time;

// Initialize right before the loop
extern void
tmr_init(tmr_time_t initial_tick_time);

// Returns the time in miliseconds since initialization of the cloak module
extern tmr_time_t
tmr_now();

extern void
tmr_wait(tmr_time_t t);

extern void
tmr_begin_tick();

// Returns how many frames to skip(optimize, like remove rendering or just heavy and not necessary parts) due to the frame taking too long
extern int
tmr_end_tick();
