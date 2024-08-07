#include "../engine/vid.h"
#include "../engine/fip.h"
#include "../engine/tmr.h"
#include "../engine/aud.h"
#include "../engine/gui.h"
#include "../engine/com.h"
#include "../engine/mix.h"
#include "../engine/g3d.h"
#include "../engine/net.h"
#include "../engine/ser.h"
#include "../engine/cli.h"
#include "../engine/mt.h"

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
    case GUI_E_CLOSE:
    gui_free(e->thing);
    break;
  }
  return 1;
}

static int velocity = 0;

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
    else if (e->press.code == 'w')
    {
      velocity = 120;
    }
    else if (e->press.code == 's')
    {
      velocity = -120;
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

static int
on_cli(cli_event_t* e)
{
  if (e->type == CLI_E_JOIN && e->join.accepted)
  {
    cli_info(0);
  }
  else if (e->type == CLI_E_INFO)
  {
    printf("on_cli(): Server name '%s', description '%s'\n", e->info.alias, e->info.desc);
  }
}

static void
k_init()
{
  // mix_push(K_MISC_MIXS, 0, 0, 0);
  
  // mix_push(K_NODE_MIXS, 255,255,255);
  // mix_push_gradient(K_NODE_MIXS, 254, 0,0,0);
  
  mix_push(K_EKG_MIXS, 0,0,0);
  mix_push_gradient(K_EKG_MIXS, 31, 0,255,100);

  // x = mix_push(K_GUI_MIXS, GUI_SHADES_N, 64,64,10, 200,200,50);
  // mix_push(K_GUI_MIXS, 50,45,0);
  // mix_push(K_GUI_MIXS, 90,80,0);
  // mix_push(K_GUI_MIXS, 130,115,0);
  // mix_push(K_GUI_MIXS, 167,160,0);
  // mix_push(K_GUI_MIXS, 240,235,0);

  // mix_push(K_GUI_MIXS, 45,45,10);
  // mix_push(K_GUI_MIXS, 80,80,30);
  // mix_push(K_GUI_MIXS, 115,115,40);
  // mix_push(K_GUI_MIXS, 160,160,80);
  // mix_push(K_GUI_MIXS, 235,235,180);
  
  mix_push(K_GUI_MIXS, 40,40,40);
  mix_push(K_GUI_MIXS, 80,80,80);
  mix_push(K_GUI_MIXS, 130,130,130);
  mix_push(K_GUI_MIXS, 180,180,180);
  mix_push(K_GUI_MIXS, 255,255,255);
  
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

void
test_free()
{
  puts("Free lol");
}
void*
test_init()
{
  puts("Init lol");
  while (1) {tmr_wait(100);}
  return NULL;
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
    gui_shades[i] = mix_sets[K_GUI_MIXS].start + i;
  }
  gui_thing_t* opened = gui_open(com_relfp("example.gui"));
  gui_on = on_gui;

  aud_init();

  // Set clk rate to fps if it was initialized by user otherwise screen rate
  tmr_init(1000 / (fps == -1 ? screen_rate : fps));

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


  // tmr_ms_t time = 0, count = 0;
  // tmr_ms_t times[] = {0};// {FTOFIP(8,1), FTOFIP(8,0.7), FTOFIP(8,0.6), FTOFIP(8,1), FTOFIP(8,1), FTOFIP(8,1), FTOFIP(8,0.5), FTOFIP(8,0.3), FTOFIP(8,0.25), FTOFIP(8,0.25), FTOFIP(8,0.15), FTOFIP(8,0.15), FTOFIP(8,0.15)};

  g3d_init(NULL);
  
  net_init();
  cli_init("Klaus");
  ser_init("test", "a test server, that's all.");
  cli_on = on_cli;

  puts("\nRUNNING...\n");

  net_set_addr(cli_sock, &net_loopback, ser_sock->bind_port);
  cli_join();

  while(1)
  {
    tmr_begin_tick();

    g3d_eye->origin[2] += velocity;
    // printf("%d\n", g3d_eye->origin[2]);

    // node_beat();
    vid_run();

    ser_run();
    cli_run();
    
    px_wipe(&vid_px, 0);
      g3d_wipe();
      g3d_draw(NULL);

      // node_draw(flow_mode);
      // ekg_draw();
      gui_draw_windows();
    vid_refresh();
    

    // if (i++ > 20)
    // {
    //   printf("%d\n", tmr_tick_time);
    // }

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
        time += tmr_tick_time;
      }
    }
    #endif


    tmr_end_tick();
  }
  return 0;
}

