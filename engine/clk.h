// Cloak module, for taking care of time and shit

#pragma once

// Measured in miliseconds
typedef unsigned int clk_time_t;

// The target tick time that we want, may not always be able to fufil this time, depending on the performance of the program and how high it is. Check clk_tick_time.
// In seconds, the module uses this to decide how much it must wait at the end of the tick to fufil the time.
// Can be modified in real time.
extern clk_time_t clk_target_tick_time;

// The true time that took the tick is stored here, it will most likely be equal to clk_target_tick_time, but it may not if the tick took too long.
extern clk_time_t clk_tick_time;

// Updated as clk_now() the moment you call clk_begin_tick
extern clk_time_t clk_begin_time;

// Initialize right before the loop
extern void
clk_init(clk_time_t initial_tick_time);

// Returns the time in miliseconds since initialization of the cloak module
extern clk_time_t
clk_now();

extern void
clk_wait(clk_time_t t);

extern void
clk_begin_tick();

// Returns how many frames to skip(optimize, like remove rendering or just heavy and not necessary parts) due to the frame taking too long
extern int
clk_end_tick();
