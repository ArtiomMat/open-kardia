#include "edit.h"
#include "vid.h"
#include "k.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

static int hover_node_i = -1;

static inline node_t*
index_node(int index, int type)
{
  return type == NODE_MUSCLE ? &node_muscles[index] : &node_signals[index];
}

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
  for (int i = 0; i < NODE_MAX_SIGNAL; i++)
  {
    node_t* node = &node_signals[i];
    node->signal.ion = NODE_MAX_ION;
    if (node->pos[0] < 0) // Null terminating node
    {
      break;
    }
    for (int j = 0; j < node->nexts_n; j++)
    {
      node_draw_line(node, index_node(node->nexts[j], NODE_SIGNAL), NODE_SIGNAL);
    }

    int x = fiptoi(node->pos[0]), y = fiptoi(node->pos[1]);
    int c;
    if (in_square(mouse_x, mouse_y, x, y, 7))
    {
      c = NODE_NODE_SIGNAL1_C;
      hover_node_i = i;
    }
    else
    {
      c = NODE_MUSCLE_C;
    }
    put_square(c, x, y, 7);
  }


}
