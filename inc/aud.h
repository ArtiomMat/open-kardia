// Audio module, a simple one specifically designed to beep...

#pragma once

// Please initialize clk because we need the tick time
extern int
aud_init(unsigned int sample_rate);
extern void
aud_free();
// freq of 255 means it literally fills up every single sample with the 
extern void
aud_play(unsigned char freq, unsigned char amp);
