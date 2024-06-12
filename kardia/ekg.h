// EKG module, external to node/k because it's not essential to its function
// Reads heart state, simulates an EKG and generates a graph over time.

#pragma once

#include "k.h"

#define EKG_C_R 0
#define EKG_C_G 255
#define EKG_C_B 30
#define EKG_C k_pickc(EKG_C_R,EKG_C_G,EKG_C_B)

// Measured each beat, rather than over multiple beats, so can be innacurate
extern int ekg_bpm;

extern unsigned char ekg_amp; // 100 by default, from 255

// Sensitivity is the factor by which the node_flow is multiplied, _y0 is the y at which EKG voltage of 0 is drawn.
extern void
ekg_init(fip_t sensitivty, int _y0);

extern void
ekg_draw();
