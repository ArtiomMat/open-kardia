#include "g3d.h"
#include "vid.h"
#include "mix.h"
#include "com.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SHORT_MAX ((unsigned short)(1 << (sizeof(short)*8-1)) - 1)
#define TBLS (1 << G3D_AB) // Table size

// TODO: Shorten it by 2, maybe even by 4 if you wanna go crazy, by just inverting if the angle is above 0.5
static short sintbl[TBLS];
// TODO: Same shit here I think?
static g3d_f1_t tantbl[TBLS];

void
g3d_init(g3d_eye_t* initial_eye)
{
  for (int i = 0; i < (TBLS); i++)
  {
    double a = ((double)i / TBLS) * M_PI * 2;

    sintbl[i] = SHORT_MAX * sin(a);
    tantbl[i] = FTOFIP(G3D_DB, tan(a));
  }

  printf("%d\n", sintbl[TBLS/6]); // Should be 0.5

  printf("g3d_init(): 3D Graphics module initialized, table length is %d.\n", TBLS);
}

g3d_f1_t
g3d_tan(g3d_f1_t a)
{
  a = FIP_FRAC(G3D_AB, a);
  return tantbl[a];
}

g3d_f1_t
g3d_sin(g3d_f1_t a)
{
  a = FIP_FRAC(G3D_AB, a);
  fip_t s = sintbl[a]; // Implicit conversion into int
  s >>= (sizeof(short)*8-1) - G3D_DB;

  return s;
}

g3d_f1_t
g3d_cos(g3d_f1_t a)
{
  int shift = TBLS >> 2; // Shift of +pi/2 in the g3d angles
  return g3d_sin(a + shift);
}

static void
draw_tri(unsigned char color, g3d_i3_t a, g3d_i3_t b, g3d_i3_t c)
{
  // Order so that a is the top, b is the middle, c is the bottom, but depends on if y's of different points are the same
  if (a[1] > c[1])
  {
    void* tmp = a;
    a = c;
    c = tmp;
  }
  if (a[1] > b[1])
  {
    void* tmp = a;
    a = b;
    b = tmp;
  }
  if (b[1] > c[1])
  {
    void* tmp = b;
    b = c;
    c = tmp;
  }

  if (a[1] == c[1]) // Literally nothing to draw
  {
    return;
  }

  g3d_f1_t x_l, x_r;

  // NOTE: slopes are dx/dy, not dy/dx, forgor why, sticked around from older attempts
  g3d_f1_t m_ac = FIP_DIV(16, ITOFIP(16, a[0]-c[0]), ITOFIP(16, a[1]-c[1]));

  // The top triangle
  if (a[1] != b[1])
  {
    g3d_f1_t m_ab = FIP_DIV(16, ITOFIP(16, a[0]-b[0]), ITOFIP(16, a[1]-b[1]));

    x_l = x_r = ITOFIP(16,a[0]);

    for (int y = a[1]; y <= b[1]; y++, x_l += m_ab, x_r += m_ac)
    {
      vid_put_xline(color, FIPTOI(16,x_l), FIPTOI(16,x_r), y);
    }
  }
  else
  {
    x_r = ITOFIP(16,a[0]);
  }

  // The bottom triangle
  if (b[1] != c[1])
  {
    g3d_f1_t m_bc = FIP_DIV(16, ITOFIP(16, b[0]-c[0]), ITOFIP(16, b[1]-c[1]));
    
    x_l = ITOFIP(16,b[0]);

    for (int y = b[1]; y <= c[1]; y++, x_l += m_bc, x_r += m_ac)
    {
      vid_put_xline(color, FIPTOI(16,x_l), FIPTOI(16,x_r), y);
    }
  }
}

static void
rasterize(g3d_i3_t out, g3d_f3_t in)
{
  
}

void
g3d_draw(g3d_model_t* model)
{
  for (int i = 0; i < 1024*2; i++)
  {
    g3d_i3_t a = {rand()%20+50, rand()%300+2, 0};
    g3d_i3_t b = {rand()%300+2, rand()%300+2, 0};
    g3d_i3_t c = {rand()%20+50, rand()%300+50, 0};
    draw_tri(rand()%16, a, b, c);
  }
}
