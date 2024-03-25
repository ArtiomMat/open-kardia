// EKG module, external to node/k because it's not essential to its function
// Reads heart state, simulates an EKG and generates a graph over time.

#pragma once

#include "k.h"

// yi and yf is where we are going to draw the ekg on the screen
extern void
ekg_init(fip_t sensitivty, int _y0);

extern void
ekg_draw();
