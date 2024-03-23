#include "vid.h"
#include "node.h"
#include "fip.h"
#include "k.h"


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
    for (int y = yi; y < yf; y++)
    {
      vid_set(255, xi + y * K_VID_SIZE);
    }
  }
  else if (yi == yf)
  {
    for (int x = xi; x < xf; x++)
    {
      vid_set(255, x + yi * K_VID_SIZE);
    }
  }
  else
  {
    node_t* l = root_node->pos[0] < next->pos[0] ? root_node : next;
    node_t* r = l == root_node ? next : root_node;
    
    fip_t slope = fip_div(r->pos[1] - l->pos[1], r->pos[0] - l->pos[0]);
    // printf("%f\n", fiptof(fip_div(256, 256*2)));
    
    for (fip_t x = l->pos[0], y = l->pos[1]; x <= r->pos[0]; x += itofip(1), y += slope)
    {
      for (fip_t i = 0; i <= slope; i += itofip(1))
      {
        vid_set(255, fiptoi(x) + fiptoi(y + i) * K_VID_SIZE);
      }
    }
  }
}

void
node_draw(node_t* root_node)
{
  if (root_node == NULL)
  {
    return;
  }
  
  for (int i = 0; i < root_node->nexts_n; i++)
  {
    node_t* next = root_node->nexts + i;
    
    draw_line(root_node, next);
  }
}

