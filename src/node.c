#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"
#include "clk.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

node_t node_all[NODE_MAX];

static int
color_for(fip_t first_ion, fip_t second_ion, int x, int xi, int xf)
{
  // switch(type)
  // {
  //   case NODE_MUSCLE:
  //   return NODE_MUSCLE_C;
    
  //   int g = (NODE_NODE_SIGNAL1_C_G * first_ion) / NODE_MAX_ION;
  //   int b = (NODE_NODE_SIGNAL1_C_B * first_ion) / NODE_MAX_ION;
    
  //   int G = (NODE_NODE_SIGNAL1_C_G * second_ion) / NODE_MAX_ION;
  //   int B = (NODE_NODE_SIGNAL1_C_B * second_ion) / NODE_MAX_ION;
    
  //   return k_gradient(x-xi, xf-xi, 0,g,b, 0,G,B);

  //   case NODE_NULL:
  //   return 255;
  // }

  unsigned char r = NODE_POL_C_R,g = NODE_POL_C_G,b = NODE_POL_C_B;
  unsigned char R = NODE_POL_C_R,G = NODE_POL_C_G,B = NODE_POL_C_B;

  k_gradient_rgb(first_ion, NODE_MAX_ION, &r, &g, &b, NODE_DEPOL_C_R, NODE_DEPOL_C_G, NODE_DEPOL_C_B);
  k_gradient_rgb(second_ion, NODE_MAX_ION, &R, &G, &B, NODE_DEPOL_C_R, NODE_DEPOL_C_G, NODE_DEPOL_C_B);
  
  return k_gradient(x-xi, xf-xi, r,g,b, R,G,B);
}

void
node_draw_line(node_t* root_node, node_t* next)
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
      vid_set(color_for(b->ion, t->ion, y, yi, yf), xi + y * K_VID_SIZE);
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
        vid_set(color_for(l->ion, r->ion, x, l->pos[0], r->pos[0]), fiptoi(x) + fiptoi(y + i) * K_VID_SIZE);
        
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
    memset(node_all, -1, sizeof(node_all));

    printf("node_init(): Node module initialized, %d max nodes.\n", NODE_MAX);
  }
  else
  {
    printf("node_init(): Node module initialized, '%s'[^%d] is ready to beat!\n", fp, NODE_MAX);
  }
}

void
node_beat()
{
  for (int i = 0; i < NODE_MAX; i++)
  {
    node_t* node = &node_all[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      // printf("%f\n", fiptof(node_flow[0]), fiptof(node_flow[1]));
      return;
    }

    if (node->ion > 0)
    {
      if (node->ion > NODE_MAX_ION) // Cap ionization
      {
        node->ion = NODE_MAX_ION;
      }

      if (node->countdown > 0) // If we have a countdown we decrease it.
      {
        node->countdown -= clk_tick_time; // The time between beats
        
        if (node->countdown > 0) // If we overshot we can safely just begin flowing the ionization, otherwise we still wait!
        {
          continue;
        }
        else
        {
          node->countdown = 0;
        }
      }

      int send_halt = 0; // If we need to now send the halt, 1 when the node empties
      
      // Because if the flow is too big for this beat, we have unexpected behaviour when just using it, we need to normalize it
      int real_flow = fip_mul(node->flow, clk_tick_time);
      if (real_flow >= node->ion)
      {
        send_halt = 1; // Because that's it this is the one
        real_flow -= real_flow - node->ion;
      }

      node->ion -= real_flow; // Perfect decrease

      // Send the ionization to the next nodes
      for (int j = 0; j < node->nexts_n; j++)
      {
        node_all[node->nexts[j]].ion += real_flow / node->nexts_n;

        if (send_halt)
        {
          node_all[node->nexts[j]].countdown = node->halt;
        }
        else
        {
          node_all[node->nexts[j]].countdown = clk_tick_time * 100; // The loop may just go over this one next and prematurely flow its ionization too, we don't want that until the next beat
        }
      }
    }
  }
}

void
node_draw()
{
  for (int i = 0; i < NODE_MAX; i++)
  {
    node_t* node = &node_all[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      return;
    }
    for (int j = 0; j < node->nexts_n; j++)
    {
      node_draw_line(node, &node_all[node->nexts[j]]);
    }
  }
}
