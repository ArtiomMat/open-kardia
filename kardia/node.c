#include "../engine/vid.h"
#include "node.h"
#include "../engine/fip.h"
#include "k.h"
#include "../engine/clk.h"
#include "ekg.h"
#include "../engine/gui.h"
#include "../engine/mix.h"
#include "../engine/com.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

node_t node_all[NODE_MAX];

static int draw_flow;

// X is between(including) XI and XF, essentially the closer X is to XF, the more the color will be that of the right node.
static unsigned char
color_for(node_t* left, node_t* right, int x, int xi, int xf)
{
  unsigned char left_pick, right_pick;
  int grad_i = draw_flow ? K_EKG_GRAD : K_NODE_GRAD;

  // We need to left get the picks for each node
  left_pick = mix_pick(grad_i, left->ion, left->bias);
  right_pick = mix_pick(grad_i, right->ion, right->bias);

  // Then we return the color between them depending where x is
  return left_pick + (right_pick - left_pick) * (x-xi) / xf;
}

void
node_draw_line(node_t* root_node, node_t* next)
{
  int xi, yi, xf, yf;
  xi = FIPTOI(8,root_node->pos[0]);
  yi = FIPTOI(8,root_node->pos[1]);
  xf = FIPTOI(8,next->pos[0]);
  yf = FIPTOI(8,next->pos[1]);

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
    l->pos[0] <<= 12 - 8;
    l->pos[1] <<= 12 - 8;
    r->pos[0] <<= 12 - 8;
    r->pos[1] <<= 12 - 8;

    fip_t slope = FIP_DIV(12,r->pos[1] - l->pos[1], r->pos[0] - l->pos[0]); // How much y's per 1 x

    fip_t abs_slope = slope < 0 ? -slope : slope;
    int sign = slope < 0 ? -1 : 1;

    for (fip_t x = l->pos[0], y = l->pos[1]; x < r->pos[0]; x += ITOFIP(12,1), y += slope)
    {
      int c = color_for(l, r, x, l->pos[0], r->pos[0]);

      for (fip_t i = 0; i < abs_slope; i += ITOFIP(12,1))
      {
        fip_t set_y = FIPTOI(12,sign * i + y);

        if (set_y >= vid_size[1] || set_y < 0)
        {
          break;
        }

        vid_set(c, FIPTOI(12,x) + set_y * K_VID_SIZE);
      }
    }

    // Back down from that black magic
    l->pos[0] >>= 12 - 8;
    l->pos[1] >>= 12 - 8;
    r->pos[0] >>= 12 - 8;
    r->pos[1] >>= 12 - 8;
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
      // printf("%f\n", FIPTOF(8,node_flow[0]), FIPTOF(8,node_flow[1]));
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

      fip_t fip_clk_tick_time = (clk_tick_time/1000) * (1 << 8);
      fip_t real_flow = FIP_MUL(8,node->flow, fip_clk_tick_time);
      // Because if the flow is too big for this beat, we have unexpected behaviour when just using it, we need to normalize it
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
