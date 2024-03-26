#include "edit.h"
#include "vid.h"
#include "k.h"

#include <stdio.h>
#include <stdlib.h>

static void
put_square(int color, int _x, int _y, int size)
{
  for (int x = _x - size/2; x <= x + size/2; x++)
  {
    for (int y = _y - size/2; y <= y + size/2; y++)
    {
      vid_set(color, x + y * K_VID_SIZE);
    }
  }
}

void
edit_init(const char* fp)
{
  printf("edit_init(): Editor initialized, writing to '%s'.\n", fp);
}

int
edit_run()
{
  
}
