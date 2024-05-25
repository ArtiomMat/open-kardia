#include "g3d.h"
#include "vid.h"
#include "mix.h"
#include <stdlib.h>

void
draw_tri_top(unsigned char color, gui_i1_t topx, gui_i1_t topy, gui_i1_t bottomy, gui_i1_t leftx, gui_i1_t rightx)
{
  if (topy==bottomy)
  {
    return;
  }

  // dx/dy, not dy/dx
  gui_f1_t m_l = FIP_DIV(16, ITOFIP(16, topx-leftx), ITOFIP(16, topy-bottomy));
  gui_f1_t m_r = FIP_DIV(16, ITOFIP(16, topx-rightx), ITOFIP(16, topy-bottomy));

  gui_f1_t xi, xf;
  xi = xf = ITOFIP(16,topx);
  for (int y = topy; y <= bottomy; y++, xi += m_l, xf += m_r)
  {
    vid_put_xline(color, FIPTOI(16,xi), FIPTOI(16,xf), y);
  }
}

void
draw_tri_bottom(unsigned char color, gui_i1_t bottomx, gui_i1_t bottomy, gui_i1_t topy, gui_i1_t leftx, gui_i1_t rightx)
{
  if (topy==bottomy)
  {
    return;
  }

  // dx/dy, not dy/dx
  gui_f1_t m_l = FIP_DIV(16, ITOFIP(16, bottomx-leftx), ITOFIP(16, bottomy-topy));
  gui_f1_t m_r = FIP_DIV(16, ITOFIP(16, bottomx-rightx), ITOFIP(16, bottomy-topy));
  
  gui_f1_t xi, xf;
  xi = xf = ITOFIP(16,bottomx);
  // Inversin le slope cuz we workin our way from de right to left
  for (int y = bottomx; y >= topy; y--, xf -= m_l, xi -= m_r)
  {
    vid_put_xline(color, FIPTOI(16,xi), FIPTOI(16,xf), y);
  }
}

void
draw_tri(unsigned char color, gui_i3_t a, gui_i3_t b, gui_i3_t c)
{
  if (a[1] < c[1])
  {
    void* tmp = a;
    a = c;
    c = tmp;
  }
  if (b[1] < c[1])
  {
    void* tmp = b;
    b = c;
    c = tmp;
  }

  
}

void
g3d_draw(g3d_model_t* model)
{
  draw_tri_bottom(0, 200, 200,vid_cursor[1], vid_cursor[0], vid_cursor[0]+150);
}

