#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"

#include <stdlib.h>

node_t node_all[NODE_MAX_NODES] = {0};

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
    int first = (255 * first_ion) / NODE_MAX_ION;
    int second = (255 * second_ion) / NODE_MAX_ION;
    return gradient(x-xi, xf-xi, 0,first,0, 0,second,0);

    case NODE_NULL:
    return 255;
  }
}

// Draws line from root_node to next
static void
draw_line(node_t* root_node, node_t* next)
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
      vid_set(color_for(b->signal.ion, t->signal.ion, y, yi, yf, root_node->type), xi + y * K_VID_SIZE);
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
        vid_set(color_for(l->signal.ion, r->signal.ion, x, l->pos[0], r->pos[0], root_node->type), fiptoi(x) + fiptoi(y + i) * K_VID_SIZE);
        
        i += itofip(1);
      }
      while (i < slope);
    }
  }
}

void
node_draw_all()
{
  for (int i = 0; i < NODE_MAX_NODES; i++)
  {
    node_t* node = &node_all[i];
    for (int j = 0; j < node->nexts_n; j++)
    {
      draw_line(node, &node->nexts[j]);
    }
  }
}

