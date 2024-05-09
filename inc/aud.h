// Audio module, a simple one specifically designed to beep...

#pragma once

// Please initialize clk because we need the tick time
extern int
aud_init();
extern void
aud_free();
// Plays a "short" beep for an implementation defined time with the frequency and amplitude specified.
// freq of 255 means it literally fills up every single sample with the 
extern void
aud_play(unsigned char freq, unsigned char amp);

