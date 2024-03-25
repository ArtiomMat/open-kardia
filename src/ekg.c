#include "ekg.h"
#include "vid.h"
#include "node.h"
#include "k.h"

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
  
  for (int i = 0, last = 0; i < K_VID_SIZE; i++)
  {
    int top = buf[i] > last ? buf[i] : last;
    int bottom = top == buf[i] ? last : buf[i];

    if (i == x)
      printf("%d %d\n", top, bottom);

    if (bottom + y0 <= 0)
    {
      bottom = -y0;
    }

    for (int p = bottom; p <= top; p++)
    {
      vid_set(k_pickc(0,255,130), i + (p + y0) * K_VID_SIZE);
    }

    last = buf[i];
  }
  
  x++;
}
