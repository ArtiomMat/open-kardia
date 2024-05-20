#include "edit.h"
#include "k.h"
#include "../engine/com.h"
#include "../engine/fip.h"

#include <stdio.h>
#include <stdlib.h>

static int hover_node_i = -1;

static void
put_square(int color, int _x, int _y, int size)
{
  for (int x = max(0, _x - size/2); x < K_VID_SIZE && x <= _x + size/2; x++)
  {
    for (int y = max(0, _y - size/2); y < K_VID_SIZE && y <= _y + size/2; y++)
    {
      vid_put(color, x + y * K_VID_SIZE);
    }
  }
}

static int
in_square(int x, int y, int s_x, int s_y,int size)
{
  return (x >= s_x - size/2) && (x <= s_x + size/2) && (y >= s_y - size/2) && (y <= s_y + size/2);
  ({1;});
}


int
edit_on_vid(vid_event_t* e)
{
  return 0;
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
    for (int j = 0; j < node->next_flows_n; j++)
    {
      node_draw_line(node, &node_all[node->next_flows[j]]);
    }

    int x = FIPTOI(8,node->pos[0]), y = FIPTOI(8,node->pos[1]);
    int c;
    if (in_square(k_mouse[0], k_mouse[1], x, y, 7))
    {
      c = 1;
      hover_node_i = i;
    }
    else
    {
      c = 0;
    }
    put_square(c, x, y, 7);
  }

  return 0;
}
