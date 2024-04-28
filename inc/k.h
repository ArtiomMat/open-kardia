// Kardia main module, general/utility definitions and functions

#pragma once

#include "node.h"
#include "fip.h"
#include "gui.h"

// #include "k_endian.h"

// Relative font path
#define K_FONT_REL_FP "viewmax.psf"

// For certain areas of Kardia, thanks to stack overflow, avoids double evaluation
#ifndef MIN
  #define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })
#endif
#ifndef MIN
  #define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })
#endif

#define K_VID_SIZE 400

#define _K_RED_DEPTH 3
#define _K_GREEN_DEPTH 3
#define _K_BLUE_DEPTH 2

typedef union
{
  struct
  {
    unsigned char
    r : _K_RED_DEPTH,
    g : _K_GREEN_DEPTH,
    b : _K_BLUE_DEPTH;
  };
  unsigned char c; // All bits together
} k_rgb_t;

enum
{
  K_NODE_GRAD,
  K_EKG_GRAD,
  K_GUI_GRAD,
  K_GRADS_N,
};

// How long in seconds it takes for a single in Kardia, in a single tick all the nodes of the heart are updated. By default is 0.03 for 30 ticks per second
extern fip_t k_tick_time;
extern unsigned long long k_ticks;

extern gui_font_t* k_font;

// Directly from stdlib main()
// extern int args_n;
// extern const char** args;

extern int k_mouse[2];

static inline int
k_pickc_rounder(int v, int depth)
{
  return (v>>(8-depth)) + 1;
}

static inline int
k_pickc(unsigned char r, unsigned char g, unsigned char b)
{
  k_rgb_t rgb;

  rgb.r = (r>>(8-_K_RED_DEPTH));
  rgb.g = (g>>(8-_K_GREEN_DEPTH));
  rgb.b = (b>>(8-_K_BLUE_DEPTH));

  return rgb.c;
}

static inline void
k_vid_set(unsigned char r, unsigned char g, unsigned char b, int i)
{
  int check = (i-1)&1;

  k_rgb_t rgb;

  if (check)
  {
    rgb.r = (r>>(8-_K_RED_DEPTH));
    rgb.g = (g>>(8-_K_GREEN_DEPTH));
    rgb.b = (b>>(8-_K_BLUE_DEPTH));
  }
  else
  {
    rgb.r = k_pickc_rounder(r,_K_RED_DEPTH);
    rgb.g = k_pickc_rounder(g,_K_GREEN_DEPTH);
    rgb.b = k_pickc_rounder(b,_K_BLUE_DEPTH);
  }

  vid_set(rgb.c, i);
}

// Puts the new values in rgb, same as k_gradient() just returns in RGB rather than in index
extern void
k_gradient_rgb(int x, int x_max, unsigned char* r, unsigned char* g, unsigned char* b, unsigned char R, unsigned char G, unsigned char B);

// Returns an index for location x>=0 up to x_max, starts from rgb and fades into RGB
extern int
k_gradient(int x, int x_max, unsigned char r, unsigned char g, unsigned char b, unsigned char R, unsigned char G, unsigned char B);

// Finds an argument, if found returns index, otherwise returns 0 so can use !.
// Does not look on args[0]
extern int
k_arg(const char* str);
