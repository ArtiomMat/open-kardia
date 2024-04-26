#include "mix.h"
#include "vid.h"

#include <stdio.h>

mix_grad_t mix_grads[MIX_MAX_GRADS] = {0};

static int push_start = 0; // Where we start this current push

struct mix_shift mix_shifts[256] = {0};

unsigned char
mix_push1(int i, int r, int g, int b)
{
  // If it's the first push for this gradient we need to init the start
  if (mix_grads[i].n == 0)
  {
    mix_grads[i].start = push_start;
  }

  // Add 1 to n
  mix_grads[i].n++;

  vid_colors[push_start][0] = r;
  vid_colors[push_start][1] = g;
  vid_colors[push_start][2] = b;
  mix_shifts[push_start].grad_i = i;

  push_start++;
  return push_start;
}

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
    for (int j = 0; j < n-1; j++)
    {
      // vid_colors[j + push_start][0] = r;
      // vid_colors[j + push_start][1] = g;
      // vid_colors[j + push_start][2] = b;
      // mix_shifts[j + push_start].grad_i = i;
      mix_push1(i, r, g, b);
      r += dr;
      g += dg;
      b += db;
    }

    // I do it because there may sometimes be rounding errors in dr/g/b that lead to just below the desired last value
    mix_push1(i, r2,g2,b2);

    push_start += n;
  }
  else
  {
    fprintf(stderr, "mix_push(): A gradient with n<=1 is not a gradient, i=%d!!!\n", i);
  }

  return push_start;
}
