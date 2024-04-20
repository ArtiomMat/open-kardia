#include "ekg.h"
#include "vid.h"
#include "node.h"
#include "k.h"
#include "aud.h"
#include "clk.h"
#include "psf.h"

#include <stdio.h>
#include <stdlib.h>

#define EKG_DRAW_RANGE (K_VID_SIZE/5)

static int x;
static fip_t sensitivty;

static fip_t y0;

static int buf[K_VID_SIZE] = {0}; // Stores the entire ekg voltage history(that is on the screen and wraps around it)

int ekg_bpm = 0; // TODO: Implement

unsigned char ekg_amp = 100;

// Always psotive but can be indicated for negative voltage peaks too
// At what voltage the ekg will beep, changes each beat(that is detected by finding the )
static fip_t beep_bias;

void
ekg_init(fip_t _sensitivty, int _y0)
{
  beep_bias = FTOFIP(0.15);

  y0 = _y0;
  x = 0;
  sensitivty = _sensitivty;

  printf("ekg_init(): EKG module initialized, drawing horizontally *%.3f.\n", FIPTOF(sensitivty));
}

// Also updates beep_bias and beeps
static void
read_into_buf()
{
  static int beep = 0; // 0 means didn't do, 1 means did so wait until can for next time
  static int time_since_beep = 0;

  x++;
  time_since_beep += clk_tick_time;

  if (x >= K_VID_SIZE)
  {
    x = 0;
  }
  
  int i;
  fip_t total_voltage = 0;
  for (i = 0; i < NODE_MAX; i++)
  {
    node_t* node = &node_all[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      break;
    }

    // calculate distance between both electrodes
    fip_t ld = node->pos[0];
    fip_t rd = ITOFIP(K_VID_SIZE) - node->pos[0];
    // Proceed to calculate "voltage"
    total_voltage += FIP_DIV(node->ion, rd) - FIP_DIV(node->ion, ld);
  }
  total_voltage /= i; // Allowed to use integers
  
  buf[x] = FIPTOI(FIP_MUL(total_voltage, sensitivty)); // Negative due to y+ being down
  
  if (abs(total_voltage) >= beep_bias)
  {
    if (!beep) // We beep here, nested if for the else below
    {
      aud_play(240, ekg_amp);
      beep = 1;

      ekg_bpm = FIPTOI(FIP_DIV(ITOFIP(60), time_since_beep));

      time_since_beep = 0;
    }
  }
  else
  {
    beep = 0;
  }
}

void
ekg_draw()
{
  read_into_buf();

  // The loop actually just draws the visible portion that hasn't faded
  for (int raw_i = 0, last = 0; raw_i <= EKG_DRAW_RANGE; raw_i++)
  {
    int color = k_gradient(raw_i, EKG_DRAW_RANGE, 0,0,0, 0,EKG_C_G,EKG_C_B);

    // Setup the video i that is the x on the screen
    int i = x - EKG_DRAW_RANGE + raw_i;
    if (i < 0) // Return to the right side if we are negative
    {
      i += K_VID_SIZE;
    }

    // Determining top and bottom points in screen space
    int top = buf[i] > last ? buf[i] : last;
    int bottom = top == buf[i] ? last : buf[i];
    
    bottom += y0;
    if (bottom < 0)
    {
      bottom = 0;
    }
    top += y0;
    if (top >= K_VID_SIZE)
    {
      top = K_VID_SIZE-1;
    }

    // Now we just draw a vertical line
    for (int p = bottom; p <= top; p++)
    {
      vid_set(color, i + p * K_VID_SIZE);
    }

    last = buf[i];
  }
}
