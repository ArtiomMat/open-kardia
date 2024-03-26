#include "edit.h"
#include "vid.h"
#include "k.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif


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
  put_square(NODE_MUSCLE_C, 100, 100, 7);
  if (in_square(mouse_x, mouse_y, 100, 100, 7))
    put_square(NODE_NODE_SIGNAL1_C, 100, 100, 7);
}
