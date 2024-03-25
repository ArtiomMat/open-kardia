#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"
#include "clk.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

node_t node_muscles[NODE_MAX_MUSCLE], node_signals[NODE_MAX_SIGNAL];

fip_t node_flow[2] = {0,0};

// Returns an index for location x>=0 up to x_max, starts from rgb and fades into RGB


static int
color_for(fip_t first_ion, fip_t second_ion, int x, int xi, int xf, int type)
{
  switch(type)
  {
    case NODE_MUSCLE:
    return k_gradient(x-xi, xf-xi, 255,80,70, 70,80,255);
    // return k_pickc(255, 80, 70);
    
    case NODE_SIGNAL:
    int g = (255 * first_ion) / NODE_MAX_ION;
    int b = (130 * first_ion) / NODE_MAX_ION;
    
    int G = (255 * second_ion) / NODE_MAX_ION;
    int B = (130 * second_ion) / NODE_MAX_ION;
    
    return k_gradient(x-xi, xf-xi, 0,g,b, 0,G,B);

    case NODE_NULL:
    return k_pickc(128, 128, 128);
  }
}

// Draws line from root_node to next
static void
draw_line(node_t* root_node, node_t* next, int type)
{
  int xi, yi, xf, yf;
  xi = fiptoi(root_node->pos[0]);
  yi = fiptoi(root_node->pos[1]);
  xf = fiptoi(next->pos[0]);
  yf = fiptoi(next->pos[1]);
  
  if (xi == xf)
  {
    node_t* b = root_node->pos[1] < next->pos[1] ? root_node : next;
    node_t* t = b == root_node ? next : root_node;
    if (yi > yf)
    {
      int tmp = yi;
      yi = yf;
      yf = tmp;
    }
    for (int y = yi; y < yf; y++)
    {
      vid_set(color_for(b->signal.ion, t->signal.ion, y, yi, yf, type), xi + y * K_VID_SIZE);
    }
  }
  else
  {
    node_t* l = root_node->pos[0] < next->pos[0] ? root_node : next;
    node_t* r = l == root_node ? next : root_node;

    fip_t slope = fip_div(r->pos[1] - l->pos[1], r->pos[0] - l->pos[0]);
    
    for (fip_t x = l->pos[0], y = l->pos[1]; x <= r->pos[0]; x += itofip(1), y += slope)
    {
      fip_t i = 0;
      do
      {
        vid_set(color_for(l->signal.ion, r->signal.ion, x, l->pos[0], r->pos[0], type), fiptoi(x) + fiptoi(y + i) * K_VID_SIZE);
        
        i += itofip(1);
      }
      while (i < slope);
    }
  }
}

void
node_init(const char* fp)
{
  if (fp == NULL)
  {
    memset(node_muscles, -1, sizeof(node_muscles));
    memset(node_signals, -1, sizeof(node_signals));

    printf("node_init(): Node module initialized, %d/%d max nodes.\n", NODE_MAX_MUSCLE, NODE_MAX_SIGNAL);
  }
  else
  {
    printf("node_init(): Node module initialized, '%s' is ready to beat!\n", fp);
  }
}

void
node_beat()
{
  node_flow[0] = node_flow[1] = 0;
  
  for (int i = 0; i < NODE_MAX_SIGNAL; i++)
  {
    node_t* node = &node_signals[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      printf("%f\n", fiptof(node_flow[0]), fiptof(node_flow[1]));
      return;
    }

    if (node->signal.ion > 0)
    {
      if (node->signal.countdown > 0) // If we have a countdown we decrease it.
      {
        node->signal.countdown -= clk_tick_time; // The time between beats
        
        if (node->signal.countdown > 0) // If we overshot we can safely just begin flowing the ionization, otherwise we still wait!
        {
          continue;
        }
        else
        {
          node->signal.countdown = 0;
        }
      }

      int send_halt = 0; // If we need to now send the halt, 1 when the node empties
      
      // Because if the flow is too big for this beat, we have unexpected behaviour when just using it, we need to normalize it
      int real_flow = fip_mul(node->signal.flow, clk_tick_time);
      if (real_flow >= node->signal.ion)
      {
        send_halt = 1; // Because that's it this is the one
        real_flow -= real_flow - node->signal.ion;
      }

      node->signal.ion -= real_flow; // Perfect decrease

      // Send the ionization to the next nodes
      for (int j = 0; j < node->nexts_n; j++)
      {
        node->nexts[j].signal.ion += real_flow / node->nexts_n;

        fip_t mag = abs(node->pos[0] - node->nexts[j].pos[0] + node->pos[1] - node->nexts[j].pos[1]);
        
        node_flow[0] += fip_mul((node->nexts[j].pos[0] - node->pos[0]), fip_div(node->signal.flow, NODE_MAX_FLOW));
        node_flow[1] += fip_mul((node->nexts[j].pos[0] - node->pos[0]), fip_div(node->signal.flow, NODE_MAX_FLOW));

        if (send_halt)
        {
          node->nexts[j].signal.countdown = node->signal.halt;
        }
        else
        {
          node->nexts[j].signal.countdown = clk_tick_time * 100; // The loop may just go over this one next and prematurely flow its ionization too, we don't want that until the next beat
        }
      }
    }
  }
}

static void
node_draw_arr(node_t* nodes, int n, int type)
{
  for (int i = 0; i < n; i++)
  {
    node_t* node = &nodes[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      return;
    }
    for (int j = 0; j < node->nexts_n; j++)
    {
      draw_line(node, &node->nexts[j], type);
    }
  }
}

void
node_draw()
{
  node_draw_arr(node_muscles, NODE_MAX_MUSCLE, NODE_MUSCLE);
  node_draw_arr(node_signals, NODE_MAX_SIGNAL, NODE_SIGNAL);
}
