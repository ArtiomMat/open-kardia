// Audio module, a simple one specifically designed to beep...

#pragma once

// Please initialize clk because we need the tick time
extern int
aud_init(unsigned int sample_rate);
extern void
aud_free();
extern void
aud_play(short freq, float amplitude);
