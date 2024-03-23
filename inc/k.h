// Kardia main module, general/utility definitions and functions
// Contains the heart!

#pragma once

#include "node.h"
#include "fip.h"

#define K_VID_SIZE 400

#define _K_RED_DEPTH 3
#define _K_GREEN_DEPTH 3
#define _K_BLUE_DEPTH 2

typedef union
{
  struct
  {
    unsigned char
    r : _K_RED_DEPTH,
    g : _K_GREEN_DEPTH,
    b : _K_BLUE_DEPTH;
  };
  unsigned char c; // All bits together
} k_rgb_t;

typedef struct
{
  
} k_heart_t;

extern k_heart_t heart;

// How long in seconds it takes for a single in Kardia, in a single tick all the nodes of the heart are updated. By default is 0.03 for 30 ticks per second
extern fip_t k_tick_time;
extern unsigned long long k_ticks;

extern fip_t
k_now();

