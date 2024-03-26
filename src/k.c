#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "clk.h"
#include "k.h"
#include "ekg.h"
#include "aud.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int edit_mode = 0;

fip_t k_tick_time;
unsigned long long k_ticks;

int args_n;
const char** args;

int mouse_x, mouse_y;

int
k_gradient(int x, int x_max, unsigned char r, unsigned char g, unsigned char b, unsigned char R, unsigned char G, unsigned char B)
{
  r += ((R-r) * x) / x_max; // We just do a classic x*(now/max)
  g += ((G-g) * x) / x_max;
  b += ((B-b) * x) / x_max;
  return k_pickc(r, g, b);
}

int
k_arg(const char* str)
{
  for (int i = 1; i < args_n; i++)
  {
    if (!strcmp(str, args[i]))
    {
      return i;
    }
  }
  return 0;
}

static unsigned char
channel_color(int value, int depth)
{
  int max = ((1 << depth)-1);
  int increment = 255 / max;
  return value * increment;
}

static void
event_handler(vid_event_t* e)
{
  switch (e->type)
  {
    case VID_E_MOVE:
    mouse_x = e->move.x;
    mouse_y = e->move.y;
    break;

    case VID_E_PRESS:
    if (e->press.code == 26)
    {
      edit_mode = !edit_mode;
    }
    else
    {
      node_all[2].ion = NODE_MAX_ION;
    }
    break;

    case VID_E_CLOSE:
    edit_free();
    vid_free();
    aud_free();
    exit(0);
    break;
  }

  if (edit_mode)
  {
    edit_event_handler(e);
  }
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
main(int _args_n, const char** _args)
{
  args_n = _args_n;
  args = _args;

  static const char* fp = NULL;

  for (int i = 1; i < args_n; i++)
  {
    // Flag
    if (args[i][0] == '-')
    {
      const char* f = args[i] + 1;
      
      if (f[0] == 'e' && !f[1])
      {
        edit_mode = 1;
      }
      else
      {
        printf("main(): Unrecognized flag '%s'.\n", f);
        return 1;
      }
    }
    else if (fp == NULL)
    {
      fp = args[i];
    }
    else
    {
      printf("main(): File path already specified '%s' is ignored.\n", fp);
    }
  }

  vid_init(K_VID_SIZE, K_VID_SIZE);
  vid_set_title("Open Kardia");
  vid_event_handler = event_handler;

  aud_init(8000);

  clk_init(ftofip(0.03f));

  node_init(NULL);

  k_init();

  edit_init(fp);
  ekg_init(itofip(200.0f), K_VID_SIZE/2);
  
  int x=0, y=1, z=2;

  node_all[0].nexts_n=0;
  node_all[0].ion = 0;
  node_all[0].flow = itofip(60);
  node_all[0].halt = 0;
  node_all[0].countdown = 0;

  node_all[1].nexts_n=1;
  node_all[1].nexts=&x;
  node_all[1].ion = 0;
  node_all[1].flow = itofip(60);
  node_all[1].halt = 0;
  node_all[1].countdown = 0;

  node_all[2].nexts_n=1;
  node_all[2].nexts=&y;
  node_all[2].ion = NODE_MAX_ION;
  node_all[2].flow = itofip(60);
  node_all[2].halt = ftofip(0.1f);
  node_all[2].countdown = ftofip(0.1f);

  node_all[2].pos[0] = itofip(10);
  node_all[2].pos[1] = itofip(10);

  node_all[1].pos[0] = itofip(330);
  node_all[1].pos[1] = itofip(211);

  node_all[0].pos[0] = itofip(230);
  node_all[0].pos[1] = itofip(360);

  fip_t time = 0, count = 0;
  fip_t times[] = {ftofip(1), ftofip(0.7), ftofip(0.6), ftofip(1), ftofip(1), ftofip(1), ftofip(0.5), ftofip(0.3), ftofip(0.25), ftofip(0.25), ftofip(0.15), ftofip(0.15), ftofip(0.15)};
  while(1)
  {
    clk_begin_tick();

    vid_wipe(k_pickc(0,0,0));

    if (edit_mode)
    {
      edit_run();
    }
    else
    {
      node_draw();
      ekg_draw();

      node_beat();

      if (count < sizeof(times))
      {
        if (time >= times[count])
        {
          time = 0;
          node_all[2].ion = NODE_MAX_ION;
          count++;
        }
        else
        {
          time += clk_tick_time;
        }
      }

    }

    vid_run();
    vid_refresh();
    clk_end_tick();
  }
  return 0;
}

