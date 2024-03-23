// Node module, includes logic for everything node related, for how 

#pragma once

#include "fip.h"

// Drawing is recursive
#define NODE_MAX_NODES 256

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
  struct node_s* nexts;
  int nexts_n;
  fip_t pos[2]; // In screen space pixels

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
      fip_t ion;
      // In seconds, after this node is emptied, how long this nodes holds on to the next node's flow before allowing it to begin flowing. This allows for delays, the heart is know to have those.
      fip_t halt;
    } signal;
  };
  
  unsigned char type;
} node_t;

extern node_t node_all[NODE_MAX_NODES];

// Make muscle node interact with an ion node
extern void
node_interact(node_t* muscle, node_t* signal);
// Loops over each node in node_all and just draws a line between it and its next one, ensuring the drawing of all the shapes without any infinite loops or shit, O(1).
extern void
node_draw_all();

