#include "edit.h"
#include "vid.h"
#include "k.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

static int hover_node_i = -1;

static void
put_square(int color, int _x, int _y, int size)
{
  for (int x = max(0, _x - size/2); x < K_VID_SIZE && x <= _x + size/2; x++)
  {
    for (int y = max(0, _y - size/2); y < K_VID_SIZE && y <= _y + size/2; y++)
    {
      vid_set(color, x + y * K_VID_SIZE);
    }
  }
}

static int
in_square(int x, int y, int s_x, int s_y,int size)
{
  return (x >= s_x - size/2) && (x <= s_x + size/2) && (y >= s_y - size/2) && (y <= s_y + size/2);
}

void
edit_event_handler(vid_event_t* e)
{

}

void
edit_free()
{
  puts("edit_free(): Editor freed.");
}

void
edit_init(const char* fp)
{
  printf("edit_init(): Editor initialized, writing to '%s'.\n", fp);
}

int
edit_run()
{
  hover_node_i = -1;
  for (int i = 0; i < NODE_MAX; i++)
  {
    node_t* node = &node_all[i];
    
    if (node->pos[0] < 0) // Null terminating node
    {
      break;
    }
    for (int j = 0; j < node->nexts_n; j++)
    {
      node_draw_line(node, &node_all[node->nexts[j]]);
    }

    int x = fiptoi(node->pos[0]), y = fiptoi(node->pos[1]);
    int c;
    if (in_square(mouse_x, mouse_y, x, y, 7))
    {
      c = NODE_NODE_DEPOL_C;
      hover_node_i = i;
    }
    else
    {
      c = NODE_POL_C;
    }
    put_square(c, x, y, 7);
  }


}
