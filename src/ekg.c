#include "ekg.h"
#include "vid.h"
#include "node.h"
#include "k.h"

#define EKG_DRAW_RANGE (K_VID_SIZE/5)

static int x;
static fip_t sensitivty;

static fip_t y0;

static int buf[K_VID_SIZE] = {0}; // Stores the entire ekg history(on the screen)

void
ekg_init(fip_t _sensitivty, int _y0)
{
  y0 = _y0;
  x = 0;
  sensitivty = _sensitivty;

  puts("ekg_init(): EKG module initialized, drawing horizontally *%.3f.", fiptof(sensitivty));
}

static void
read_into_buf()
{
  if (x >= K_VID_SIZE)
  {
    x = 0;
  }
  fip_t y = fip_mul(node_flow[0], sensitivty);
  
  buf[x++] = -fiptoi(y); // Negative due to y+ being down
}

void
ekg_draw()
{
  read_into_buf();

  // The loop actually just draws the visible portion that hasn't faded
  for (int raw_i = 0, last = 0; raw_i <= EKG_DRAW_RANGE; raw_i++)
  {
    int color = k_gradient(raw_i, EKG_DRAW_RANGE, 0,0,0, 0,225,130);

    // Setup the video i that is the x on the screen
    int i = x - EKG_DRAW_RANGE + raw_i;
    if (i < 0) // Return to the right side if we are negative
    {
      i += K_VID_SIZE;
    }

    // Determining top and bottom points in screen space
    int top = buf[i] > last ? buf[i] : last;
    int bottom = top == buf[i] ? last : buf[i];
    if (bottom + y0 <= 0)
    {
      bottom = -y0;
    }

    // Now we just draw a vertical line
    for (int p = bottom; p <= top; p++)
    {
      vid_set(color, i + (p + y0) * K_VID_SIZE);
    }

    last = buf[i];
  }
}
