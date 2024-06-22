// Node module, includes logic for everything node related, for how

#pragma once

#include "../engine/tmr.h"
#include "../engine/fip.h"

#define NODE_MAX 64

#if NODE_MAX > 255
  #error NODE_MAX Cant be above 255, because of the data type used for counts and stuff
#endif

// At max ionization the muscle fully contracts
#define NODE_MAX_ION (8 << 8)
#define NODE_MAX_FLOW (NODE_MAX_ION*256)

typedef struct node_s
{
  // Need this to flow the ionization, can be NULL, signifying that ionization will just fade out here.
  unsigned char next_flows[8];
  unsigned char next_flows_n; // Max is 8
  // What are the next nodes we are drawing? NULL for this being a node without any
  unsigned char next_draws[8]; // Max is 8
  unsigned char next_draws_n;

  fip_t pos[2]; // In screen space pixels, don't worry handled by node_beat

  // fip_t relax; // In pixels per second, how fast it relaxes back
  fip_t bias; // Ionization required for full depol
  fip_t pol_pos[2]; // Polarized position in pixels
  fip_t depol_off[2]; // Depolarized offset in pixels

  // In ionization per second, how much ionization flows from this node to the next nodes, if multiple nodes the flow is halved to each one.
  fip_t flow;
  // The current ionization in this node, NODE_MAX_ION is the max.
  // If ionization reaches beyond NODE_MAX_ION it is capped, various factors depend on NODE_MAX_ION being the maximum possible value.
  fip_t ion;
  // In seconds, after this node is emptied, how long this nodes holds on to the next node's flow before allowing it to begin flowing. This allows for delays, the heart is know to have those.
  tmr_time_t halt;
  // In seconds, the halt that the parent signal node sent to this one, the node needs to decrease this value until it's zero.
  tmr_time_t countdown;
} node_t;

extern node_t node_all[NODE_MAX];

// Draws line from root_node to next, type is from the enum above.
// For editor to also be able to use.
extern void
node_draw_line(node_t* root_node, node_t* next);
// If file is NULL then initializes the rest, node_muscles/signals are intialized to 0.
extern void
node_init(const char* fp);
// Quite optimized, doesn't mindlessly loop over everything.
extern void
node_beat();
// Loops over each node in the arrays and just draws a line an element and its next one/s, ensuring the drawing of all the shapes without any infinite loops or shit, O(1).
// NOTE: If pos of the node is <0(sign bit is on) then it is considered a null terminating node! If no null terminating node we just stop at NODE_MAX_MUSCLE/SIGNAL.
// ion_flow is if to draw the ion channels or the actual muscle tissue
extern void
node_draw(int ion_flow);
