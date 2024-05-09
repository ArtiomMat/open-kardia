// Mixing module, for creating gradients within vid_colors, a pillar of Kardia's graphics.

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

// Includes how many shifts left and right can be done on this color.
extern struct mix_shift
{
  unsigned char grad_i;
} mix_shifts[256];

/**
 * Just sets a vid_colors[color] to the rgb value, this doesn't have anything to do with mix_grads, just for a single line modification.
 */
extern void
mix_set(int color, int r, int g, int b);

/**
 * Push a single color into the gradient of index I in vid_colors.
 * Returns the index that comes after this last pushed color.
*/
extern unsigned char
mix_push(int i, int r, int g, int b);

/**
 * Initialize a gradient of index I in mix_grads, the last pushed color is the initial color, and it pushes N more colors that ransition into R2 G2 B2. So if N=1 it would actually be literally the same as doing mix_push(i, r2, g2, b2)
 * Returns the index that comes after this last pushed gradient.
 */
extern unsigned char
mix_push_gradient(int i, int n, int r2, int g2, int b2);

static inline unsigned char
mix_pick(int i, int x, int max_x)
{
  if (x > max_x)
  {
    x = max_x;
  }
  return mix_grads[i].start + ((mix_grads[i].n - 1) * x / max_x);
}

/**
 * Pick leftest color
 */
static inline unsigned char
mix_pickl(int i)
{
  return mix_grads[i].start;
}

/**
 * Pick rightest color
 */
static inline unsigned char
mix_pickr(int i)
{
  return mix_grads[i].n - 1;
}

// NOTE THAT N MUST BE POSITIVE FOR INTEGER N USE MIX_SH()!
// Shift a color left(towards smaller color index) by N
static inline unsigned char
mix_shl(unsigned char color_i, unsigned n)
{
  int start = mix_grads[mix_shifts[color_i].grad_i].start;
  if ((int)color_i - n < start)
  {
    return start;
  }

  return color_i - n;
}

// NOTE THAT N MUST BE POSITIVE FOR INTEGER N USE MIX_SH()!
// Shift a color right(towards bigger color index) by N
static inline unsigned char
mix_shr(unsigned char color_i, unsigned n)
{
  mix_grad_t* g = &mix_grads[mix_shifts[color_i].grad_i];
  int end = g->start + g->n - 1;
  if ((int)color_i + n > end)
  {
    return end;
  }

  return color_i + n;
}

// Shift a color to a direction defined by N's sign and magnitude of N(- is left to negative colors, + is right to positive colors)
static inline unsigned char
mix_sh(unsigned char color_i, int n)
{
  if (n > 0)
  {
    mix_shl(color_i, n);
  }
  return mix_shl(color_i, -n);
}

