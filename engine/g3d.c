#include "g3d.h"
#include "vid.h"

void
draw_tri_top(unsigned char color, int top, int bot, int left, int right)
{
  // // gui_f1_t m_tr;
  // gui_f1_t m_tl = FIP_DIV(G3D_FB, ITOFIP(G3D_FB, top[1]-left[1]), ITOFIP(G3D_FB, top[0]-left[0]));
  
  // for (int x = left[0]; x <= top[0]; x++)
  // {
  //   // *x is ok because x is not fip
  //   int y0 = FIPTOI( G3D_FB, m_tl * x ) + top[1];
  //   vid_put_yline(color, y0, left[1], x);
  // }

}
