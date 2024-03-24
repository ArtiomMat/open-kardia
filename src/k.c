#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "clk.h"
#include "k.h"

#include <stdio.h>

fip_t k_tick_time;
unsigned long long k_ticks;

static unsigned char
channel_color(int value, int depth)
{
  int max = ((1 << depth)-1);
  int increment = 255 / max;
  return value * increment;
}

static void
k_init(const char* heartfp)
{
  for (unsigned i = 0; i < 256; i++)
  {
    k_rgb_t rgb = {.c=i};
    vid_colors[i][0] = channel_color(rgb.r, _K_RED_DEPTH);
    vid_colors[i][1] = channel_color(rgb.g, _K_GREEN_DEPTH);
    vid_colors[i][2] = channel_color(rgb.b, _K_BLUE_DEPTH);
  }
}

int
main(int args_n, char** args)
{
  vid_init(K_VID_SIZE, K_VID_SIZE);
  vid_set_title("Open Kardia");

  clk_init(ftofip(0.03f));

  node_init(NULL);

  k_init(NULL);
  
  node_muscles[0].nexts_n=0;
  node_muscles[0].signal.ion = NODE_MAX_ION/3;

  node_muscles[1].nexts_n=1;
  node_muscles[1].nexts=&node_muscles[0];
  node_muscles[1].signal.ion = NODE_MAX_ION;

  node_muscles[1].pos[0] = itofip(29);
  node_muscles[1].pos[1] = itofip(23);
  node_muscles[0].pos[0] = itofip(100);
  node_muscles[0].pos[1] = itofip(356/2);
  
  while(1)
  {
    clk_begin_tick();

    vid_wipe(k_pickc(0,0,0));
    node_draw_all();
    vid_refresh();
    vid_run();

    clk_end_tick();
  }
  return 0;
}

