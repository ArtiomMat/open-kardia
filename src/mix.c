#include "mix.h"
#include "vid.h"

#include <stdio.h>

mix_grad_t mix_grads[MIX_MAX_GRADS];

static int push_start = 0; // Where we start this current push

struct mix_shift mix_shifts[256] = {0};

unsigned char
mix_push(int i, int n, int r, int g, int b, int r2, int g2, int b2)
{
  if (n > 1)
  {
    int dr = (r2-r)/(n-1);
    int dg = (g2-g)/(n-1);
    int db = (b2-b)/(n-1);

    int R = r, G = g, B = b;
    // n-1 because at the last I do it manually
    for (int j = 0; j < n; j++)
    {
      vid_colors[j + push_start][0] = r;
      vid_colors[j + push_start][1] = g;
      vid_colors[j + push_start][2] = b;
      mix_shifts[j + push_start].grad_i = i;

      r += dr;
      g += dg;
      b += db;
    }

    // I do it because there may sometimes be rounding errors in dr/g/b that lead to just below the desired last value
    // vid_colors[n - 1 + push_start][0] = r2;
    // vid_colors[n - 1 + push_start][1] = g2;
    // vid_colors[n - 1 + push_start][2] = b2;
    // mix_shifts[n - 1 ....] do it if uncommenting!!!

    mix_grads[i].start = push_start;
    mix_grads[i].n = n;

    push_start += n;
  }
  else
  {
    fprintf(stderr, "mix_push(): A gradient with n<=1 is not a gradient, i=%d!!!\n", i);
  }

  return push_start;
}
