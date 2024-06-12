#include "mix.h"
#include "vid.h"

#include <stdio.h>

mix_set_t mix_sets[MIX_MAX_SETS] = {0};

static int push_start = 0; // Where we start this current push

struct mix_shift mix_shifts[256] = {0};

void
mix_set(int color, int r, int g, int b)
{
  vid_colors[color][0] = r;
  vid_colors[color][1] = g;
  vid_colors[color][2] = b;
}

unsigned char
mix_push(int i, int r, int g, int b)
{
  // If it's the first push for this gradient we need to init the start
  if (mix_sets[i].n == 0)
  {
    mix_sets[i].start = push_start;
  }

  // Add 1 to n
  mix_sets[i].n++;
  mix_set(push_start, r, g, b);
  mix_shifts[push_start].grad_i = i;

  push_start++;
  return push_start;
}

unsigned char
mix_push_gradient(int i, int n, int r2, int g2, int b2)
{
  // Setup the starting point
  int r=0, g=0, b=0;
  if (push_start) // We need push_start to be something actually
  {
    r = vid_colors[push_start - 1][0];
    g = vid_colors[push_start - 1][1];
    b = vid_colors[push_start - 1][2];
  }
  
  if (n > 1)
  {
    int dr = (r2-r)/(n-1);
    int dg = (g2-g)/(n-1);
    int db = (b2-b)/(n-1);

    // n-1 because at the last I do it manually
    for (int j = 0; j < n-1; j++)
    {
      r += dr;
      g += dg;
      b += db;
      
      mix_push(i, r, g, b);
    }

    // Because there may sometimes be rounding errors in dr/g/b that lead to just below/above the desired last value
    mix_push(i, r2,g2,b2);

  }
  else
  {
    fprintf(stderr, "mix_push(): A gradient with n<=1 is not a gradient, i=%d!!!\n", i);
  }

  return push_start;
}
