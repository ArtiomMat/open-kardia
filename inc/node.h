// Node module, includes logic for everything node related, for how 

#pragma once

#include "fip.h"

enum
{
  NODE_NULL,
  NODE_MUSCLE,
  NODE_SIGNAL,
};

typedef struct node_s
{
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
      // Need to actually flow the ionization.
      node_t* nexts;
      int nexts_n;
      // In ionization per second, how much ionization flows from this node to the next nodes, if multiple nodes the flow is halved to each one.
      fip_t flow;
      // The current ionization in this node.
      fip_t ion;
      // In seconds, after this node is emptied, how long this nodes holds on to the next node's flow before allowing it to begin flowing. This allows for delays, the heart is know to have those.
      fip_t halt;
    } signal;
  };
  
  unsigned char type;
} node_t;

typedef struct
{
  node_t* nodes; // The array of nodes
  unsigned char(* lines)[2]; // Array of 2 node indices for a single line
  int nodes_n, lines_n;
} node_cluster_t;

// Make muscle node interact with an ion node
extern void
node_interact(node_t* muscle, node_t* signal);
// Recusively draws nodes, allowed to connect the last node to the root_node, if you want a closed loop, the function takes care of identifying the root_node and stopping the recursion.
// Please make sure all the nodes are within the width height limits, otherwise expect a segfault or weird positions.
extern void
node_draw(node_cluster_t* cluster);

