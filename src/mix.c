#include "mix.h"
#include "vid.h"

mix_grad_t mix_grads[MIX_MAX_GRADS];

static int push_start = 0; // Where we start this current push

unsigned char
mix_push(int i, int n, int r, int g, int b, int r2, int g2, int b2)
{
  int dr = (r2-r)/n;
  int dg = (g2-g)/n;
  int db = (b2-b)/n;

  int R = r, G = g, B = b;
  for (int j = 0; j < n; j++)
  {
    vid_colors[j + push_start][0] = r;
    vid_colors[j + push_start][1] = g;
    vid_colors[j + push_start][2] = b;

    r += dr;
    g += dg;
    b += db;
  }

  mix_grads[i].start = push_start;
  mix_grads[i].n = n;

  push_start += n;

  return push_start;
}
