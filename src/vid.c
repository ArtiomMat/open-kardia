#include <stddef.h>
#include <stdlib.h>

#include "vid.h"

#ifdef __linux__
  #include "nix/vid.c"
#elif _WIN32
  #include "win/vid.c"
#endif

unsigned char (*vid_colors)[3] = NULL;

unsigned char* vid_pixels = NULL;

int vid_size[2];

int (*vid_on)(vid_event_t*) = vid_def_on;

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

