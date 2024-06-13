#include <stddef.h>
#include <stdlib.h>

#include "vid.h"

px_t vid_px;

unsigned char (*vid_colors)[3] = NULL;

int (*vid_on)(vid_event_t*) = vid_def_on;

int vid_cursor[2];

int
vid_def_on(vid_event_t* e)
{
  switch(e->type)
  {
    case VID_E_CLOSE:
    exit(0);
    break;
  }
  return 1;
}

