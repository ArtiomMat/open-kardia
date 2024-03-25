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

void
ekg_draw()
{
  if (x >= K_VID_SIZE)
  {
    x = 0;
  }
  fip_t y = fip_mul(node_flow[0], sensitivty);
  
  buf[x] = -fiptoi(y);
  
  for (int raw_i = 0, last = 0; raw_i <= EKG_DRAW_RANGE; raw_i++)
  {
    int color = k_gradient(raw_i, EKG_DRAW_RANGE, 0,0,0, 0,225,130); // Make it prettier

    int i = x - EKG_DRAW_RANGE + raw_i;
    if (i < 0)
    {
      i += K_VID_SIZE;
    }

    int top = buf[i] > last ? buf[i] : last;
    int bottom = top == buf[i] ? last : buf[i];

    if (bottom + y0 <= 0)
    {
      bottom = -y0;
    }

    for (int p = bottom; p <= top; p++)
    {
      vid_set(color, i + (p + y0) * K_VID_SIZE);
    }

    last = buf[i];
  }
  
  x++;
}
