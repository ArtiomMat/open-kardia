// Includes information about the colors k_ puts into vid_colors, all the partitioning.

#pragma once

#define MIX_MAX_GRADS 8

/**
 * Grad is a gradient, from one color to another
 */
typedef struct
{
  unsigned char start, n;
} mix_grad_t;

extern mix_grad_t mix_grads[MIX_MAX_GRADS];

/**
 * Initialize a gradient of index I in vid_colors, generates a gradient that goes from RGB to R2G2B2, at index of the last pushed gradient.
 * Returns the index that comes after this last pushed gradient.
 */
extern unsigned char
mix_push(int i, int n, int r, int g, int b, int r2, int g2, int b2);

/**
 * Returns an index for a color in the palette 
 */
static inline unsigned char
mix_pick(int i, int x, int max_x)
{
  if (x > max_x)
  {
    x = max_x;
  }
  return mix_grads[i].start + ((mix_grads[i].n - 1) * x / max_x);
}

#undef NEXT

