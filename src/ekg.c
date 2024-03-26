#include "ekg.h"
#include "vid.h"
#include "node.h"
#include "k.h"
#include "aud.h"

#include <stdio.h>

#define EKG_DRAW_RANGE (K_VID_SIZE/5)

static int x;
static fip_t sensitivty;

static fip_t y0;

static int buf[K_VID_SIZE] = {0}; // Stores the entire ekg voltage history(that is on the screen and wraps around it)

// Always psotive but can be indicated for negative voltage peaks too
// At what voltage the ekg will beep, changes each beat(that is detected by finding the )
static fip_t beep_bias;

void
ekg_init(fip_t _sensitivty, int _y0)
{
  beep_bias = ftofip(0.2);

  y0 = _y0;
  x = 0;
  sensitivty = _sensitivty;

  printf("ekg_init(): EKG module initialized, drawing horizontally *%.3f.\n", fiptof(sensitivty));
}

// Also updates beep_bias and beeps
static void
read_into_buf()
{
  static int do_beep = 0;

  x++;
  if (x >= K_VID_SIZE)
  {
    x = 0;
  }
  
  int i;
  fip_t total_voltage = 0;
  for (i = 0; i < NODE_MAX_SIGNAL; i++)
  {
    node_t* node = &node_signals[i];
    if (node->pos[0] < 0) // Null terminating node
    {
      break;
    }

    // calculate distance between both electrodes
    fip_t ld = node->pos[0];
    fip_t rd = itofip(K_VID_SIZE) - node->pos[0];
    // Proceed to calculate "voltage"
    total_voltage += fip_div(node->signal.ion, rd) - fip_div(node->signal.ion, ld);
  }
  total_voltage /= i; // Allowed to use integers
  
  buf[x] = fiptoi(fip_mul(total_voltage, sensitivty)); // Negative due to y+ being down
  
  if (abs(total_voltage) >= beep_bias)
  {
    do_beep = 1;
  }
  else if (abs(buf[x]) <= 1 && do_beep)
  {
    aud_play(2000, 0.3f);
    do_beep = 0;
  }
}

void
ekg_draw()
{
  read_into_buf();

  // The loop actually just draws the visible portion that hasn't faded
  for (int raw_i = 0, last = 0; raw_i <= EKG_DRAW_RANGE; raw_i++)
  {
    int color = k_gradient(raw_i, EKG_DRAW_RANGE, 0,0,0, 0,NODE_NODE_SIGNAL1_C_G,NODE_NODE_SIGNAL1_C_B);

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
