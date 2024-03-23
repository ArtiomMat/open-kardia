#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "k.h"

#include <stdio.h>

fip_t k_tick_time = ftofip(0.03f);
unsigned long long k_ticks;

static unsigned char
channel_color(int value, int depth)
{
  int max = ((1 << depth)-1);
  int increment = 255 / max;
  return value * increment;
}

static void
k_init()
{
  for (unsigned i = 0; i < 256; i++)
  {
    k_rgb_t rgb = {.c=i};
    vid_colors[i][0] = channel_color(rgb.r, _K_RED_DEPTH);
    vid_colors[i][1] = channel_color(rgb.g, _K_GREEN_DEPTH);
    vid_colors[i][2] = channel_color(rgb.b, _K_BLUE_DEPTH);
  }
  
  puts("k_init(): Kardia module initialized, Kardia is ready to beat!");
}

int
main(int args_n, char** args)
{
  vid_init(K_VID_SIZE, K_VID_SIZE);
  k_init();
  
  node_t y = {.color=1,.nexts_n=0};
  node_t x = {.color=1,.nexts_n=1,.nexts=&y};
  y.pos[0] = itofip(29);
  y.pos[1] = itofip(23);
  x.pos[0] = itofip(231);
  x.pos[1] = itofip(156);
  
  while(1)
  {
    vid_wipe((k_rgb_t){.r=7,.g=0,.b=3}.c);
    node_draw(&x);
    vid_refresh();
    vid_run();
  }
  return 0;
}

