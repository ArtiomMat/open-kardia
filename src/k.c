#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "clk.h"
#include "k.h"
#include "ekg.h"
#include "aud.h"
#include "psf.h"
#include "gui.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int edit_mode = 0;
static int flow_mode = 0;

// static 

fip_t k_tick_time;
unsigned long long k_ticks;

psf_font_t* k_font;

int args_n;
const char** args;

int mouse_x, mouse_y;

void
k_gradient_rgb(int x, int x_max, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char R, unsigned char G, unsigned char B)
{
  *r += ((R-*r) * x) / x_max; // We just do a classic x*(now/max)
  *g += ((G-*g) * x) / x_max;
  *b += ((B-*b) * x) / x_max;
}

int
k_gradient(int x, int x_max, unsigned char r, unsigned char g, unsigned char b, unsigned char R, unsigned char G, unsigned char B)
{
  k_gradient_rgb(x, x_max, &r, &g, &b, R, G, B);
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

static int
on_vid(vid_event_t* e)
{
  // Pipe to GUI, and if eaten return 1
  if (gui_on_vid(e))
  {
    return 1;
  }

  switch (e->type)
  {
    case VID_E_MOVE:
    mouse_x = e->move.x;
    mouse_y = e->move.y;
    break;

    case VID_E_PRESS:
    if (e->press.code == 'e')
    {
      edit_mode = !edit_mode;
    }
    else if (e->press.code == 'm')
    {
      flow_mode = !flow_mode;
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
    return edit_on_vid(e);
  }
  return 1;
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

  if (k_lile16(1) == (short)1)
  {
    puts("k_init(): Kardia module initialized, lil endian system.");
  }
  else
  {
    puts("k_init(): Kardia module initialized, big endian system.");
  }
}

int
main(int _args_n, const char** _args)
{
  args_n = _args_n;
  args = _args;

  static const char* fp = NULL;
  const char* font_fp = K_FONT_REL_FP;

  // Reading the arguments
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
      else if (f[0] == 'v' && !f[1])
      {
        i++;
        if (i >= args_n)
        {
          printf("main(): Volume flag requires volume, 0 to 255.\n", f);
          return 1;
        }
        
        ekg_amp = atoi(args[i]);
        printf("main(): Volume set to %hhu.\n", ekg_amp);
      }
      else if (f[0] == 'f' && !f[1])
      {
        i++;
        if (i >= args_n)
        {
          printf("main(): File path for overriden font required.\n", f);
          return 1;
        }

        font_fp = args[i];
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

  psf_font_t font;
  if (!psf_open(&font, font_fp, PSF_P_AUTO))
  {
    k_font = NULL;
  }
  else
  {
    k_font = &font;
  }

  int screen_rate = vid_init(K_VID_SIZE, K_VID_SIZE);
  vid_set_title("Open Kardia");
  vid_on = on_vid;

  gui_init(100, 100, "Open Kardia", k_font);

  aud_init(16000);

  clk_init(ITOFIP(1) / screen_rate);

  node_init(NULL);

  k_init();

  edit_init(fp);
  ekg_init(ITOFIP(200), K_VID_SIZE - K_VID_SIZE/10);

  node_all[0].next_flows_n=1;
  node_all[0].next_flows[0] = 2;
  node_all[0].next_draws_n=0;
  node_all[0].ion = 0;
  node_all[0].flow = ITOFIP(50);
  node_all[0].halt = 0;
  node_all[0].countdown = 0;
  node_all[0].bias = NODE_MAX_ION/2;

  node_all[1].next_flows_n=1;
  node_all[1].next_flows[0] = 0;
  node_all[1].next_draws_n=1;
  node_all[1].next_draws[0] = 0;
  node_all[1].ion = 0;
  node_all[1].flow = ITOFIP(50);
  node_all[1].halt = 0;
  node_all[1].countdown = 0;
  node_all[1].bias = NODE_MAX_ION/2;

  node_all[2].next_flows_n=1;
  node_all[2].next_flows[0] = 1;
  node_all[2].next_flows[1] = 0;
  node_all[2].next_draws_n=1;
  node_all[2].next_draws[0] = 1;
  node_all[2].ion = NODE_MAX_ION;
  node_all[2].flow = ITOFIP(50);
  node_all[2].halt = FTOFIP(0.3f);
  node_all[2].countdown = FTOFIP(0.1f);
  node_all[2].bias = NODE_MAX_ION;

  node_all[2].pol_pos[0] = ITOFIP(10);
  node_all[2].pol_pos[1] = ITOFIP(10);
  node_all[2].pos[0] = ITOFIP(10);
  node_all[2].pos[1] = ITOFIP(10);
  node_all[2].depol_off[0] = ITOFIP(5);
  node_all[2].depol_off[1] = ITOFIP(15);

  node_all[1].pol_pos[0] = ITOFIP(330);
  node_all[1].pol_pos[1] = ITOFIP(211);
  node_all[1].pos[0] = ITOFIP(10);
  node_all[1].pos[1] = ITOFIP(10);
  node_all[1].depol_off[0] = ITOFIP(-30);
  node_all[1].depol_off[1] = ITOFIP(-11);

  node_all[0].pol_pos[0] = ITOFIP(230);
  node_all[0].pol_pos[1] = ITOFIP(360);
  node_all[0].pos[0] = ITOFIP(10);
  node_all[0].pos[1] = ITOFIP(10);
  node_all[0].depol_off[0] = ITOFIP(-30);
  node_all[0].depol_off[1] = ITOFIP(-60);

  fip_t time = 0, count = 0;
  fip_t times[] = {0};// {FTOFIP(1), FTOFIP(0.7), FTOFIP(0.6), FTOFIP(1), FTOFIP(1), FTOFIP(1), FTOFIP(0.5), FTOFIP(0.3), FTOFIP(0.25), FTOFIP(0.25), FTOFIP(0.15), FTOFIP(0.15), FTOFIP(0.15)};
  
  int skip_draw = 0;
  while(1)
  {
    clk_begin_tick();

    vid_run();
    node_beat();

    vid_wipe(k_pickc(0,0,0));

    // gui_draw_fontg(&font, 1,1, 'F', 255);
    // gui_draw_fontg(&font, 2,1, 'u', 255);
    // gui_draw_fontg(&font, 3,1, 'c', 255);
    // gui_draw_fontg(&font, 4,1, 'k', 244);
    // gui_draw_fontg(&font, 5,1, '!', 244);
    // gui_draw_font(k_font, 3, 0, 'A', EKG_C);
    // gui_draw_line(300,300, 350,399, k_pickc(100, 100, 100));
    ekg_draw();
    node_draw(flow_mode);


    gui_draw_window();

    // if (count < sizeof(times))
    // {
    //   if (time >= times[count])
    //   {
    //     time = 0;
    //     node_all[2].ion = NODE_MAX_ION;
    //     count++;
    //   }
    //   else
    //   {
    //     time += clk_tick_time;
    //   }
    // }

    vid_refresh();

    skip_draw = clk_end_tick();
  }
  return 0;
}

