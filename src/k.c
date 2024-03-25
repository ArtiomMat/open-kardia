#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "clk.h"
#include "k.h"
#include "ekg.h"

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
k_init()
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
  ekg_init(itofip(3), K_VID_SIZE/2);

  k_init();
  
  node_signals[0].nexts_n=0;
  node_signals[0].signal.ion = 0;
  node_signals[0].signal.flow = itofip(50);
  node_signals[0].signal.halt = 0;
  node_signals[0].signal.countdown = 0;

  node_signals[1].nexts_n=1;
  node_signals[1].nexts=&node_signals[0];
  node_signals[1].signal.ion = 0;
  node_signals[1].signal.flow = itofip(50);
  node_signals[1].signal.halt = 0;
  node_signals[1].signal.countdown = 0;

  node_signals[2].nexts_n=1;
  node_signals[2].nexts=&node_signals[1];
  node_signals[2].signal.ion = NODE_MAX_ION;
  node_signals[2].signal.flow = itofip(50);
  node_signals[2].signal.halt = 0;
  node_signals[2].signal.countdown = 0;

  node_signals[2].pos[0] = itofip(10);
  node_signals[2].pos[1] = itofip(10);

  node_signals[1].pos[0] = itofip(330);
  node_signals[1].pos[1] = itofip(211);

  node_signals[0].pos[0] = itofip(230);
  node_signals[0].pos[1] = itofip(360);
  
  fip_t time = 0;
  while(1)
  {
    clk_begin_tick();

    vid_wipe(k_pickc(0,0,0));

    node_draw();
    ekg_draw();
    vid_refresh();

    vid_run();
    node_beat();


    if (time >= itofip(1)-100)
    {
      time = 0;
      node_signals[2].signal.ion = NODE_MAX_ION;
    }
    else
    {
      time+= clk_tick_time;
    }
    clk_end_tick();
  }
  return 0;
}

