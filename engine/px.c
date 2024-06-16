#include "px.h"
#include "fip.h"
#include "com.h"

#include <stdlib.h>

#include <png.h>

int
px_init(px_t* m, int w, int h)
{
  m->s[0] = w;
  m->s[1] = h;

  // Allocate but align to 8 bytes
  m->_total = w*h;
  m->p = aligned_alloc(sizeof(long long), m->_total + m->_total%sizeof(long long));

  return m->p != NULL;
}

void
px_free(px_t* m)
{
  free(m->p);
}

void
px_wipe(px_t* m, unsigned char color)
{
  unsigned long i;
  
  for (i = 0; i < sizeof(long long); i++)
  {
    m->p[i] = color;
  }

  for (i = 1; i < m->_total / sizeof(long long); i++)
  {
    ((long long*)(m->p))[i] = *((long long*)(m->p));
  }
}

void
px_put_xline(px_t* m, unsigned char color, int xi, int xf, int y)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = max(left, 0); x <= min(right, m->s[0]-1); x++)
  {
    m->p[y*m->s[0] + x] = color;
  }
}

void
px_put_yline(px_t* m, unsigned char color, int yi, int yf, int x)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = max(bottom, 0); y <= min(top, m->s[1]-1); y++)
  {
    m->p[y*m->s[0] + x] = color;
  }
}

void
px_put_rect(px_t* m, unsigned char fill, int left, int top, int right, int bottom)
{
  for (int _x = left; _x <= right; _x++)
  {
    for (int _y = top; _y <= bottom; _y++)
    {
      m->p[_y*m->s[0] + _x] = fill;
    }
  }
}

// Reutns slope
static fip_t
setup_line_params(int* xi, int* yi, int* xf, int* yf, fip_t* y)
{
  if (*xi > *xf)
  {
    int tmp = *xi;
    *xi = *xf;
    *xf = tmp;

    tmp = *yi;
    *yi = *yf;
    *yf = tmp;
  }

  fip_t m = FIP_DIV(16, ITOFIP(16, *yf-*yi), ITOFIP(16, *xf-*xi));

  *y = ITOFIP(16, *yi);

  return m;
}

void
px_put_line(px_t* map, unsigned char color, int xi, int yi, int xf, int yf)
{
  if (yi == yf)
  {
    px_put_xline(map, color, xi, xf, yi);
    return;
  }
  if (xi == xf)
  {
    px_put_yline(map, color, yi, yf, xi);
    return;
  }

  // XI must be less that xf
  if (abs(xf-xi) > abs(yf-yi))
  {
    fip_t y;
    fip_t m = setup_line_params(&xi, &yi, &xf, &yf, &y);

    for (int x = xi; x <= xf; x++, y += m)
    {
      map->p[x + FIPTOI(16, y) * map->s[0]] = color;
    }
  }
  else
  {
    fip_t x;
    fip_t m = setup_line_params(&yi, &xi, &yf, &xf, &x);

    for (int y = yi; y <= yf; y++, x += m)
    {
      map->p[FIPTOI(16, x) + y * map->s[0]] = color;
    }
  }
}

void
px_put_px(px_t* restrict m, px_t* restrict o, int x, int y)
{
  for (int oy = 0; oy < o->s[1]; oy++)
  {
    unsigned long ox;
    // First copy as much as possible as longs
    for (ox = 0; ox < o->s[0] / sizeof(long long); ox++)
    {
      ((long long*)(m->p))[(ox+x) + (oy+y) * m->s[0]] = 
      ((long long*)(o->p))[ox + oy * o->s[0]];
    }
    // Now copy individual bytes(residual)
    ox *= sizeof(long long);
    for (; ox < o->s[0]; ox++)
    {
      m->p[(ox+x) + (oy+y) * m->s[0]] = o->p[ox + oy * o->s[0]];
    }
  }
}

int
px_load(px_t* m, const char* fp)
{
  FILE* f = fopen(fp, "rb");

  if (f == NULL)
  {
    fprintf(stderr, "px_load(): '%s' does not exist.\n", fp);
    return 0;
  }

  unsigned char magic[4];

  // Test for png
  if (!fread(magic, 4, 1, f))
  {
    fclose(f);
    fprintf(stderr, "px_load(): '%s' is not a valid PNG file.\n", fp);
    return 0;
  }

  // Test for the PNG magic bytes in ASCII
  if (magic[0] != 0x89 || magic[1] != 'P' || magic[2] != 'N' || magic[3] != 'G')
  {
    fclose(f);
    fprintf(stderr, "px_load(): '%s' is not a valid PNG file.\n", fp);
    return 0;
  }
}
