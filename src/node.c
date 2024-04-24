#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"
#include "clk.h"
#include "ekg.h"
#include "gui.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

node_t node_all[NODE_MAX];

static int draw_flow;

static void
put_square(int color, int _x, int _y, int size)
{
  for (int x = MAX(0, _x - size/2); x < K_VID_SIZE && x <= _x + size/2; x++)
  {
    for (int y = MAX(0, _y - size/2); y < K_VID_SIZE && y <= _y + size/2; y++)
    {
      vid_set(color, x + y * K_VID_SIZE);
    }
  }
}
static int
color_for(node_t* first, node_t* second, int x, int xi, int xf)
{
  unsigned char r,g,b,R,G,B;
  if (draw_flow)
  {
    r = 0,g = 0,b = 0;
    R = 0,G = 0,B = 0;

    // Gradient from polarized to depolarized
    k_gradient_rgb(first->ion, first->bias, &r, &g, &b, EKG_C_R, EKG_C_G, EKG_C_B);
    k_gradient_rgb(second->ion, second->bias, &R, &G, &B, EKG_C_R, EKG_C_G, EKG_C_B);
  }
  else
  {
    r = NODE_POL_C_R,g = NODE_POL_C_G,b = NODE_POL_C_B;
    R = NODE_POL_C_R,G = NODE_POL_C_G,B = NODE_POL_C_B;

    // Gradient from polarized to depolarized
    k_gradient_rgb(first->ion, first->bias, &r, &g, &b, NODE_DEPOL_C_R, NODE_DEPOL_C_G, NODE_DEPOL_C_B);
    k_gradient_rgb(second->ion, second->bias, &R, &G, &B, NODE_DEPOL_C_R, NODE_DEPOL_C_G, NODE_DEPOL_C_B);
  }
  
  return k_gradient(x-xi, xf-xi, r,g,b, R,G,B);
}

void
node_draw_line(node_t* root_node, node_t* next)
{
  
  int xi, yi, xf, yf;
  xi = FIPTOI(root_node->pos[0]);
  yi = FIPTOI(root_node->pos[1]);
  xf = FIPTOI(next->pos[0]);
  yf = FIPTOI(next->pos[1]);
  
  // gui_draw_line(xi, yi, xf, yf, 255);
  
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
      vid_set(color_for(b, t, y, yi, yf), xi + y * K_VID_SIZE);
    }
  }
  else
  {
    node_t* l = root_node->pos[0] < next->pos[0] ? root_node : next;
    node_t* r = l == root_node ? next : root_node;

    // Some magic to increase FIP accuracy
    l->pos[0] <<= 12 - FIP_DEF_FRAC_BITS;
    l->pos[1] <<= 12 - FIP_DEF_FRAC_BITS;
    r->pos[0] <<= 12 - FIP_DEF_FRAC_BITS;
    r->pos[1] <<= 12 - FIP_DEF_FRAC_BITS;
    #undef FIP_FRAC_BITS
    #define FIP_FRAC_BITS 12

    fip_t slope = FIP_DIV(r->pos[1] - l->pos[1], r->pos[0] - l->pos[0]); // How much y's per 1 x

    fip_t abs_slope = slope < 0 ? -slope : slope;

    for (fip_t x = l->pos[0], y = l->pos[1]; x < r->pos[0]; x += ITOFIP(1), y += slope)
    {
      int c = color_for(l, r, x, l->pos[0], r->pos[0]);
      fip_t i = 0;
      do
      {
        if (FIPTOI(y + i) >= vid_size[1])
        {
          break;
        }
        vid_set(c, FIPTOI(x) + FIPTOI(y + i) * K_VID_SIZE);
        
        i += ITOFIP(1);
      }
      while (i < abs_slope);
    }

    // Back down from that black magic
    l->pos[0] >>= 12 - FIP_DEF_FRAC_BITS;
    l->pos[1] >>= 12 - FIP_DEF_FRAC_BITS;
    r->pos[0] >>= 12 - FIP_DEF_FRAC_BITS;
    r->pos[1] >>= 12 - FIP_DEF_FRAC_BITS;
    #undef FIP_FRAC_BITS
    #define FIP_FRAC_BITS FIP_DEF_FRAC_BITS
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
    if (node->pos[0] == -1) // Null terminating node
    {
      // printf("%f\n", FIPTOF(node_flow[0]), FIPTOF(node_flow[1]));
      return;
    }

    // Set the position!
    {
      fip_t ion = node->ion;
      if (ion > node->bias)
      {
        ion = node->bias;
      }
      node->pos[0] = node->pol_pos[0] + (node->depol_off[0] * ion / node->bias);
      node->pos[1] = node->pol_pos[1] + (node->depol_off[1] * ion / node->bias);
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
        // If didn't overshoot we still wait, otherwise we can safely just begin flowing the ionization!
        if (node->countdown > 0)
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
      int real_flow = FIP_MUL(node->flow, clk_tick_time);
      if (real_flow >= node->ion)
      {
        send_halt = 1; // Because that's it this is the one
        real_flow -= real_flow - node->ion;
      }

      node->ion -= real_flow; // Perfect decrease

      // Send the ionization to the next nodes
      for (int j = 0; j < node->next_flows_n; j++)
      {
        node_all[node->next_flows[j]].ion += real_flow / node->next_flows_n;

        if (send_halt)
        {
          node_all[node->next_flows[j]].countdown = node->halt;
        }
        else
        {
          node_all[node->next_flows[j]].countdown = clk_tick_time * 100; // The loop may just go over this one next and prematurely flow its ionization too, we don't want that until the next beat
        }
      }
    }
  }
}


void
node_draw(int flow)
{
  draw_flow = flow;
  
  for (int i = 0; i < NODE_MAX; i++)
  {
    node_t* node = &node_all[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      return;
    }

    // TODO: Test if node is out of screen, provide some safety
    if (draw_flow)
    {
      for (int j = 0; j < node->next_flows_n; j++)
      {
        node_draw_line(node, &node_all[node->next_flows[j]]);
      }
    }
    else
    {
      for (int j = 0; j < node->next_draws_n; j++)
      {
        node_draw_line(node, &node_all[node->next_draws[j]]);
      }
    }
  }
}
