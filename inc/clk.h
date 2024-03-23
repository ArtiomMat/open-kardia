// Cloak module, for taking care of time and shit

#pragma once

#include "fip.h"

// In seconds, the module uses this to decide how much it must wait at the end of the tick to fufil the time.
// Can be modified in real time.
extern fip_t clk_tick_time;

// Initialize right before the loop
extern void
clk_init(fip_t initial_tick_time);

extern fip_t
clk_now();

extern void
clk_wait(fip_t secs);

extern void
clk_begin_tick();

extern void
clk_end_tick();
