// Node module, includes logic for everything node related, for how 

#pragma once

#include "fip.h"

// Drawing is recursive
#define NODE_MAX_MUSCLE 64
#define NODE_MAX_SIGNAL 64

#define NODE_MAX_ION (8 << FIP_FRAC_BITS)

enum
{
  NODE_NULL,
  NODE_MUSCLE,
  NODE_SIGNAL,
};

typedef struct node_s
{
  // Need to actually flow the ionization.
  // TODO: Maybe make it an index, can decrease memory used by nodes by ~3 approx, or even ~7 depending on data type.
  struct node_s* nexts;
  int nexts_n;
  fip_t pos[2]; // In screen space pixels

  // Type is dependent on the array that is accessed
  union
  {
    struct
    {
      fip_t relax; // In pixels per second, how fast it relaxes back
      fip_t bias; // Ionization required for full depol
      fip_t pol_pos[2]; // Polarized position in pixels
      fip_t depol_pos[2]; // Depolarized positon in pixels
    } muscle;
    
    struct
    {
      // In ionization per second, how much ionization flows from this node to the next nodes, if multiple nodes the flow is halved to each one.
      fip_t flow;
      // The current ionization in this node, NODE_MAX_ION is the max.
      // If ionization reaches beyond NODE_MAX_ION it is capped, various factors depend on NODE_MAX_ION being the maximum possible value.
      fip_t ion;
      // In seconds, after this node is emptied, how long this nodes holds on to the next node's flow before allowing it to begin flowing. This allows for delays, the heart is know to have those.
      fip_t halt;
      // In seconds, the halt that the parent signal node sent to this one, the node needs to decrease this value until it's zero.
      fip_t countdown;
    } signal;
  };
} node_t;

extern node_t node_muscles[NODE_MAX_MUSCLE], node_signals[NODE_MAX_SIGNAL];

// If file is NULL then initializes the rest, node_muscles/signals are intialized to 0.
extern void
node_init(const char* fp);
extern void
node_free();
// Quite optimized, doesn't mindlessly loop over everything.
extern void
node_beat();
// Loops over each node in the arrays and just draws a line an element and its next one/s, ensuring the drawing of all the shapes without any infinite loops or shit, O(1).
// NOTE: If pos of the node is <0(sign bit is on) then it is considered a null terminating node! If no null terminating node we just stop at NODE_MAX_MUSCLE/SIGNAL.
extern void
node_draw();
