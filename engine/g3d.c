#include "g3d.h"
#include "vid.h"
#include "mix.h"
#include "com.h"

#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SHORT_MAX ((unsigned short)(1 << (sizeof(short)*8-1)) - 1)
#define TBLS (1 << G3D_AB) // Table size 

// Just the inverse after PI, could go further and even do it with PI/2 but it would probably have a bigger speed cost
static short sintbl[TBLS/2];
// Repeats every PI/2
static g3d_f1_t tantbl[TBLS/2];

px_t z_px;

g3d_eye_t* g3d_eye;
static g3d_eye_t def_eye = {0};

int
g3d_init(g3d_eye_t* initial_eye)
{
  if ((vid_px.s[0]*vid_px.s[1]) % sizeof(long long))
  {
    return 1;
  }

  px_init(&z_px, vid_px.s[0], vid_px.s[1]);

  // SIN/TAN TABLES SETUP
  for (int i = 0; i < (TBLS/2); i++)
  {
    double a = ((double)i / TBLS) * 3.1415926535f * 2;

    sintbl[i] = SHORT_MAX * sin(a);
    tantbl[i] = FTOFIP(G3D_DB, tan(a));
  }
  
  // FIXME: NOT FREEING THE EYE
  // EYE SETUP
  if (initial_eye != NULL)
  {
    g3d_eye = initial_eye;
    g3d_set_fov(g3d_eye, ITOFIP(G3D_AB, 1) / 4); // PI/2
  }
  else
  {
    def_eye = (g3d_eye_t){0};
    g3d_eye = &def_eye;
    g3d_eye->fov = ITOFIP(G3D_AB, 1) / 4; // PI/2
  }
  g3d_set_fov(g3d_eye, g3d_eye->fov);

  printf("g3d_init(): 3D Graphics module initialized, table size is %.1fKb.\n", (sizeof(sintbl) + sizeof(tantbl)) / 1024.0f);

  return 1;
}

// void
// g3d_wipe()
// {
//   long long* zbuf_ll = (long long*)zbuf;
//   for (int i = 0; i < vid_px.s[1]*vid_px.s[0] / (sizeof(long long) / sizeof(*zbuf)); i++)
//   {
//     zbuf_ll[i] = wipe_cache; // We use the padding as the index, I am a fucking genius 
//   }
// }

void
g3d_wipe()
{
  px_wipe(&z_px, 255);
}

g3d_f1_t
g3d_tan(g3d_f1_t a)
{
  a = FIP_FRAC(G3D_AB - 1, a);
  return tantbl[a];
}

g3d_f1_t
g3d_sin(g3d_f1_t a)
{
  a = FIP_FRAC(G3D_AB, a);
  fip_t s;
  if (a < TBLS/2)
  {
    s = sintbl[a]; // Implicit conversion into int
  }
  else
  {
    s = -sintbl[a - TBLS/2];
  }
  s >>= (sizeof(short)*8-1) - G3D_DB;

  return s;
}

g3d_f1_t
g3d_cos(g3d_f1_t a)
{
  int shift = TBLS >> 2; // Shift of +pi/2 in the g3d angles
  return g3d_sin(a + shift);
}

void
g3d_set_fov(g3d_eye_t* eye, g3d_f1_t fov)
{
  // Limit to a little less than PI, rendering breaks beyond that!
  fov = min(FIP_FRAC(G3D_AB, fov), FTOFIP(G3D_AB, 0.48f));

  eye->fov = fov;
  eye->_tg = g3d_tan(fov / 2);
}

// static void
// draw_scan(unsigned char color

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

  // Above OR below screen. Or just straight up dimentionless
  if (c[1] < 0 || a[1] >= vid_px.s[1] || a[1] == c[1])
  {
    return;
  }

  int z_depth = (c[2] + b[2] + a[2]) / 3; // Average, XXX: Maybe divide by 4 to make it faster?
  z_depth = z_depth * G3D_FAR_Z / 255; // Scale it to what is allowed in the z-buffer.

  g3d_f1_t x_l, x_r;
  g3d_f1_t m_l, m_r;
  // NOTE: slopes are dx/dy, not dy/dx, forgor why, sticked around from older attempts
  g3d_f1_t m_ac = FIP_DIV(16, ITOFIP(16, a[0]-c[0]), ITOFIP(16, a[1]-c[1]));

  // The top triangle, only if atleast b is below top
  if (b[1] >= 0 && a[1] != b[1])
  {
    g3d_f1_t m_ab = FIP_DIV(16, ITOFIP(16, a[0]-b[0]), ITOFIP(16, a[1]-b[1]));
    
    #define F1_SBIT (1<<(sizeof(g3d_f1_t)*8-1))
    
    // This and the one below, we simply made by drawing the 3 possible options for the top/bottom triangle slopes, and just finding the relationship between bigger/smaller and right/left slopes.
    if (m_ac > m_ab)
    {
      m_r = m_ac;
      m_l = m_ab;
    }
    else
    {
      m_r = m_ab;
      m_l = m_ac;
    }

    x_l = x_r = ITOFIP(16,a[0]); 
    
    for (int y = a[1]; y <= b[1]; y++, x_l += m_l, x_r += m_r)
    {
      if (y >= vid_px.s[1])
      {
        break; // We done at this point
      }
      else if (y < 0)
      {
        continue; 
      }

      int from = max(FIPTOI(16,x_l), 0);
      int to = min(FIPTOI(16,x_r), vid_px.s[0]-1);

      for (int x = from; x <= to; x++)
      {
        if (z_px.p[x + y * vid_px.s[0]] > z_depth)
        {
          vid_px.p[x + y * vid_px.s[0]] = color;
        }
      }
    }
  }

  // The bottom triangle, only if b is above bottom
  if (b[1] < vid_px.s[1] && b[1] != c[1])
  {
    g3d_f1_t m_bc = FIP_DIV(16, ITOFIP(16, b[0]-c[0]), ITOFIP(16, b[1]-c[1]));
    
    x_l = x_r = ITOFIP(16,c[0]); 
    
    if (m_ac > m_bc) // Sign is different
    {
      m_l = m_ac;
      m_r = m_bc;
    }
    else
    {
      m_l = m_bc;
      m_r = m_ac;
    }

    for (int y = c[1]; y > b[1]; y--, x_l -= m_l, x_r -= m_r)
    {
      if (y >= vid_px.s[1])
      {
        continue;
      }
      else if (y < 0)
      {
        break; // We done at this point
      }

      int from = max(FIPTOI(16,x_l), 0);
      int to = min(FIPTOI(16,x_r), vid_px.s[0]-1);

      for (int x = from; x <= to; x++)
      {
        if (z_px.p[x + y * vid_px.s[0]] > z_depth)
        {
          vid_px.p[x + y * vid_px.s[0]] = color;
        }
      }
    }
  }
}

// Assumes that camera is at 0,0,0... For now.
// out[0] will be on the scale of -.5 and .5
static void
rasterize(g3d_i3_t out, g3d_f3_t in)
{
  if (in[2] <= 0)
  {
    out[2] = -1;
    return;
  }

  g3d_f1_t plane = FIP_MUL(G3D_DB, g3d_eye->_tg, in[2]);
  
  g3d_f1_t xrat = FIP_DIV(G3D_DB, in[0], plane);
  g3d_f1_t yrat = FIP_DIV(G3D_DB, in[1], plane);

  xrat = xrat * (vid_px.s[0]/2);
  yrat = yrat * (vid_px.s[1]/2);

  out[0] = FIPTOI(G3D_DB, xrat) + vid_px.s[0]/2;
  out[1] = FIPTOI(G3D_DB, yrat) + vid_px.s[1]/2;
  out[2] = FIPTOI(G3D_DB, in[2]); // Convert to integer
}

void
g3d_draw(g3d_model_t* model)
{
  static g3d_f3_t i[3];
  static g3d_i3_t o[3];
  static int x = 0;
  x+=1;
  
  i[0][0] = ITOFIP(G3D_DB, 10);
  i[0][1] = ITOFIP(G3D_DB, 1);
  i[0][2] = ITOFIP(G3D_DB, 2+x);
  
  
  i[1][0] = ITOFIP(G3D_DB, -2);
  i[1][1] = ITOFIP(G3D_DB, 10);
  i[1][2] = ITOFIP(G3D_DB, 2+x);
  
  
  i[2][0] = ITOFIP(G3D_DB, -3);
  i[2][1] = ITOFIP(G3D_DB, -8);
  i[2][2] = ITOFIP(G3D_DB, 2+x);
  
  rasterize(o[0], i[0]);
  rasterize(o[1], i[1]);
  rasterize(o[2], i[2]);
  
  // FIXME: Causes memory curroption all the way in vid_nix_image's malloc size apparently.
  draw_tri(1, o[0], o[1], o[2]);
}
