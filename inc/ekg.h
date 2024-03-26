// EKG module, external to node/k because it's not essential to its function
// Reads heart state, simulates an EKG and generates a graph over time.

#pragma once

#include "k.h"

extern int
ekg_bpm;

// Sensitivity is the factor by which the node_flow is multiplied, _y0 is the y at which EKG voltage of 0 is drawn.
extern void
ekg_init(fip_t sensitivty, int _y0);

extern void
ekg_draw();
