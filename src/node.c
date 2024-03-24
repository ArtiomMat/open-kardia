#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

node_t node_muscles[NODE_MAX_MUSCLE], node_signals[NODE_MAX_SIGNAL];

// Returns an index for location x>=0 up to x_max, starts from rgb and fades into RGB
static int
gradient(int x, int x_max, unsigned char r, unsigned char g, unsigned char b, unsigned char R, unsigned char G, unsigned char B)
{
  r += ((R-r) * x)/x_max;
  g += ((G-g) * x)/x_max;
  b += ((B-b) * x)/x_max;
  return k_pickc(r, g, b);
}

static int
color_for(fip_t first_ion, fip_t second_ion, int x, int xi, int xf, int type)
{
  switch(type)
  {
    case NODE_MUSCLE:
    return k_pickc(255, 80, 70);
    
    case NODE_SIGNAL:
    int g = (255 * first_ion) / NODE_MAX_ION;
    int b = (130 * first_ion) / NODE_MAX_ION;
    
    int G = (255 * second_ion) / NODE_MAX_ION;
    int B = (130 * second_ion) / NODE_MAX_ION;
    
    return gradient(x-xi, xf-xi, 0,g,b, 0,G,B);

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

static void
node_draw_arr(node_t* nodes, int n, int type)
{
  for (int i = 0; i < n; i++)
  {
    node_t* node = &nodes[i];
    if (node->pos < 0) // Null terminating node
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
