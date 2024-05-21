#include "../engine/vid.h"
#include "../engine/fip.h"
#include "../engine/clk.h"
#include "../engine/aud.h"
#include "../engine/gui.h"
#include "../engine/com.h"
#include "../engine/mix.h"

#include "node.h"
#include "edit.h"
#include "k.h"
#include "ekg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static int edit_mode = 0;
static int flow_mode = 0;

// static

fip_t k_tick_time;
unsigned long long k_ticks;

gui_font_t* k_font;

int k_mouse[2];

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

static int
on_gui(gui_event_t* e)
{
  switch(e->type)
  {
    case GUI_E_WND_X:
    puts("OMG");
    break;
  }
  return 1;
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
    k_mouse[0] = e->move.x;
    k_mouse[1] = e->move.y;
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
    else if (e->press.code == 'b' || e->press.code == KEY_LMOUSE)
    {
      node_all[0].ion = NODE_MAX_ION;
    }
    break;

    case VID_E_CLOSE:
    puts("\nFREEING...\n");
    edit_free();
    vid_free();
    aud_free();
    gui_free(NULL);
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
  mix_push(K_NODE_GRAD, 255,100,0);
  mix_push_gradient(K_NODE_GRAD, 31, 0,100,255);
  
  mix_push(K_EKG_GRAD, 0,0,0);
  mix_push_gradient(K_EKG_GRAD, 31, 0,255,100);

  // x = mix_push(K_GUI_GRAD, GUI_SHADES_N, 64,64,10, 200,200,50);
  // mix_push(K_GUI_GRAD, 45,45,10);
  // mix_push(K_GUI_GRAD, 80,80,30);
  // mix_push(K_GUI_GRAD, 115,115,40);
  // mix_push(K_GUI_GRAD, 160,160,80);
  // mix_push(K_GUI_GRAD, 235,235,180);
  
  mix_push(K_GUI_GRAD, 40,40,40);
  mix_push(K_GUI_GRAD, 80,80,80);
  mix_push(K_GUI_GRAD, 130,130,130);
  mix_push(K_GUI_GRAD, 180,180,180);
  mix_push(K_GUI_GRAD, 255,255,255);
  
  mix_set(255, 0,0,0);
}

// draws and flows end at -1
// parent_flows_n is how many flows the parent has
static void
set_node(int i, int pol_x, int pol_y, int depol_off_x, int depol_off_y, int flows[], int parent_flows_n, int draws[])
{
  int j;

  node_all[i].pos[0] = pol_x;
  node_all[i].pos[1] = pol_y;

  node_all[i].ion = 0;
  node_all[i].halt = 0;
  node_all[i].countdown = 0;

  node_all[i].bias = NODE_MAX_ION / parent_flows_n;

  node_all[i].next_flows_n = node_all[i].next_draws_n = 0;

  for (j = 0; flows[j] != -1; j++)
  {
    node_all[i].next_flows[j] = flows[j];
  }
  node_all[i].next_flows_n = j;

  for (j = 0; draws[j] != -1; j++)
  {
    node_all[i].next_draws[j] = draws[j];
  }
  node_all[i].next_draws_n = j;

  node_all[i].pol_pos[0] = ITOFIP(8,pol_x);
  node_all[i].pol_pos[1] = ITOFIP(8,pol_y);

  node_all[i].depol_off[0] = ITOFIP(8,depol_off_x);
  node_all[i].depol_off[1] = ITOFIP(8,depol_off_y);

  node_all[i].flow = ITOFIP(8,10);
}

int
main(int args_n, const char** args)
{
  puts("\nINITIALISING...\n");

  com_init(args_n, args);

  static const char* fp = "NULL";
  const char* font_fp = K_FONT_REL_FP;
  puts(font_fp);

  int fps = -1;

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
          printf("main(): Volume flag requires volume, 0 to 255.\n");
          return 1;
        }

        ekg_amp = atoi(args[i]);
        printf("main(): Volume set to %hhu.\n", ekg_amp);
      }
      else if (f[0] == 'f' && f[1] == 'r' && !f[2])
      {
        i++;
        if (i >= args_n)
        {
          printf("main(): Need frame rate.\n");
          return 1;
        }

        fps = atoi(args[i]);
        printf("main(): Frame rate overriden to %i.\n", fps);
      }
      else if (f[0] == 'f' && !f[1])
      {
        i++;
        if (i >= args_n)
        {
          printf("main(): File path for overriden font required.\n");
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

  gui_font_t font;
  if (!gui_open_font(&font, com_relfp(font_fp), GUI_FONTP_AUTO))
  {
    return 1;
  }
  else
  {
    k_font = &font;
  }

  int screen_rate = vid_init(K_VID_SIZE, K_VID_SIZE);
  vid_set_title("Open Kardia");
  vid_on = on_vid;

  k_init();

  gui_init(k_font);
  for (int i = 0; i < GUI_SHADES_N; i++)
  {
    gui_shades[i] = mix_grads[K_GUI_GRAD].start + i;
  }
  gui_thing_t* opened = gui_open(com_relfp("example.gui"));
  gui_thing_t* gui_window = gui_find(opened, "W_MAIN", 1);
  opened = gui_open(com_relfp("example.gui"));
  gui_on = on_gui;

  aud_init();

  // Set clk rate to fps if it was initialized by user otherwise screen rate
  clk_init(1000 / (fps == -1 ? screen_rate : fps));

  node_init(NULL);

  edit_init(com_relfp(fp));
  ekg_init(ITOFIP(8,3000), K_VID_SIZE - K_VID_SIZE/10);

  // Begin top
  set_node(0, 50,50, 10,10, (int[]){1,2,3,-1}, 1, (int[]){1,2,3,-1});
  node_all[0].flow = ITOFIP(8,120);

  // Right atrium
  set_node(1, 200,100, -50,-20, (int[]){-1}, 3, (int[]){-1});
  // Left atrium
  set_node(2, 10,120, 50,-20, (int[]){-1}, 3, (int[]){-1});

  // Middle cunt
  set_node(3, 170,170, -30,-10, (int[]){4,5,-1}, 3, (int[]){4,5,6,7,-1});
  node_all[3].halt = 200;
  node_all[3].flow = ITOFIP(8,30);

  // Left lower cunt
  set_node(4, 340,360, -20,-50, (int[]){6,-1}, 2, (int[]){6,5,-1});
  // Right lower cunt
  set_node(5, 360,360, -20,-60, (int[]){7,-1}, 2, (int[]){7,-1});

  // Left ventrical
  set_node(6, 20,130, 120,40, (int[]){-1}, 1, (int[]){2,-1});
  // Right ventrical
  set_node(7, 200,120, -120,40, (int[]){-1}, 1, (int[]){1,-1});


  // clk_time_t time = 0, count = 0;
  // clk_time_t times[] = {0};// {FTOFIP(8,1), FTOFIP(8,0.7), FTOFIP(8,0.6), FTOFIP(8,1), FTOFIP(8,1), FTOFIP(8,1), FTOFIP(8,0.5), FTOFIP(8,0.3), FTOFIP(8,0.25), FTOFIP(8,0.25), FTOFIP(8,0.15), FTOFIP(8,0.15), FTOFIP(8,0.15)};

  puts("\nRUNNING...\n");

  while(1)
  {
    clk_begin_tick();


    node_beat();
    vid_run();

    vid_wipe(255);
      // node_draw(flow_mode);
      ekg_draw();
      if (gui_things != NULL)
      {
        gui_draw(gui_window);
        gui_draw(opened);
      }
    vid_refresh();
    
    
    #if 0
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
    #endif


    clk_end_tick();
  }
  return 0;
}

