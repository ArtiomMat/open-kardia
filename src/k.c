#include "vid.h"
#include "fip.h"
#include "node.h"
#include "edit.h"
#include "clk.h"
#include "k.h"
#include "ekg.h"
#include "aud.h"
#include "gui.h"
#include "com.h"
#include "mix.h"

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
  mix_push(K_NODE_GRAD, 32, 255,100,0, 0,100,255);
  mix_push(K_EKG_GRAD, 32, 0,0,0, 0,255,100);
  int next;
  // x = mix_push(K_GUI_GRAD, GUI_SHADES_N, 64,64,10, 200,200,50);
  /*       mix_push1(K_GUI_GRAD, 60,60,10);
         mix_push1(K_GUI_GRAD, 80,80,30);
         mix_push1(K_GUI_GRAD, 100,100,40);
         mix_push1(K_GUI_GRAD, 160,160,80);
  next = mix_push1(K_GUI_GRAD, 235,235,180);*/
  mix_push1(K_GUI_GRAD, 20,20,20);
  mix_push1(K_GUI_GRAD, 80,80,80);
  mix_push1(K_GUI_GRAD, 130,130,130);
  mix_push1(K_GUI_GRAD, 180,180,180);
  mix_push1(K_GUI_GRAD, 255,255,255);
  next = mix_push1(K_GUI_TITLE_COLO, 10,25,200);
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

  node_all[i].pol_pos[0] = ITOFIP(pol_x);
  node_all[i].pol_pos[1] = ITOFIP(pol_y);

  node_all[i].depol_off[0] = ITOFIP(depol_off_x);
  node_all[i].depol_off[1] = ITOFIP(depol_off_y);

  node_all[i].flow = ITOFIP(10);
}

int
main(int args_n, const char** args)
{
  puts("\nINITIALISING...\n");

  com_init(args_n, args);

  static const char* fp = "NULL";
  const char* font_fp = K_FONT_REL_FP;

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

  /*gui_thing_t gui_thing = {.type = GUI_T_TEXT};
  gui_thing.str = "HELLO!";
  gui_thing.text.color = 255;*/
  // Set the GUI shades
  for (int i = 0; i < GUI_SHADES_N; i++)
  {
    gui_shades[i] = mix_grads[K_GUI_GRAD].start + i;
  }
  gui_window.color = mix_pickl(K_GUI_TITLE_COLO);
  gui_init(100, 100, "Open Kardia", NULL, k_font);

  for (int i = 0; i < 2; i++)
  {
    gui_window.pos[i] = vid_size[i]/2 - gui_window.size[i]/2;
  }
  // gui_set_flag(GUI_WND_UNFOCUSED, 1);

  aud_init(16000);

  // Set clk rate to fps if it was initialized by user otherwise screen rate
  clk_init(ITOFIP(1) / (fps == -1 ? screen_rate : fps));

  node_init(NULL);

  edit_init(com_relfp(fp));
  ekg_init(ITOFIP(3000), K_VID_SIZE - K_VID_SIZE/10);

  // Begin top
  set_node(0, 50,50, 10,10, (int[]){1,2,3,-1}, 1, (int[]){1,2,3,-1});
  node_all[0].flow = ITOFIP(120);

  // Right atrium
  set_node(1, 200,100, -50,-20, (int[]){-1}, 3, (int[]){-1});
  // Left atrium
  set_node(2, 10,120, 50,-20, (int[]){-1}, 3, (int[]){-1});

  // Middle cunt
  set_node(3, 170,170, -30,-10, (int[]){4,5,-1}, 3, (int[]){4,5,6,7,-1});
  node_all[3].halt = FTOFIP(0.2f);
  node_all[3].flow = ITOFIP(30);

  // Left lower cunt
  set_node(4, 340,360, -20,-50, (int[]){6,-1}, 2, (int[]){6,5,-1});
  // Right lower cunt
  set_node(5, 360,360, -20,-60, (int[]){7,-1}, 2, (int[]){7,-1});

  // Left ventrical
  set_node(6, 20,130, 120,40, (int[]){-1}, 1, (int[]){2,-1});
  // Right ventrical
  set_node(7, 200,120, -120,40, (int[]){-1}, 1, (int[]){1,-1});


  // fip_t time = 0, count = 0;
  // fip_t times[] = {0};// {FTOFIP(1), FTOFIP(0.7), FTOFIP(0.6), FTOFIP(1), FTOFIP(1), FTOFIP(1), FTOFIP(0.5), FTOFIP(0.3), FTOFIP(0.25), FTOFIP(0.25), FTOFIP(0.15), FTOFIP(0.15), FTOFIP(0.15)};

  puts("\nRUNNING...\n");

  while(1)
  {
    clk_begin_tick();

    vid_wipe(mix_grads[K_EKG_GRAD].start);

    node_draw(flow_mode);
    ekg_draw();


    vid_run();
    node_beat();

    gui_draw(&gui_window, 0,0, vid_size[0]-1, vid_size[1]-1);

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

    vid_refresh();

    clk_end_tick();
  }
  return 0;
}

