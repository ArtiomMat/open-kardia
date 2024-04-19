#include "gui.h"
#include "fip.h"

psf_font_t* font;

gui_window_t gui_window;

int gui_title_h;
int gui_border_wh;

void
gui_init(psf_font_t* _font)
{
  font = _font;
  
}

int
gui_on_vid(vid_event_t* e)
{
  switch (e->type == VID_E_MOVE)
  {

  }
  return 0;
}

void
gui_draw_line(int xi, int yi, int xf, int yf)
{
  static int const color = 255;
  // Vertical line
  if (xi == xf)
  {
    int top = yi > yf ? yi : yf;
    int bottom = top == yi? yf : yi;

    for (int y = bottom; y <= top; y++)
    {
      vid_set(color, y*vid_w + xi);
    }
  }
  // Horizontal line
  else if (yi == yf)
  {
    int right = xi > xf ? xi : xf;
    int left = right == xi? xf : xi;

    for (int x = left; x <= right; x++)
    {
      vid_set(color, yi*vid_w + x);
    }
  }
  // Angled line
  else
  {
    // Our own little fip, 8 bits
    int ypx = itofip(yf-yi);
    ypx /= xf-xi;
    
    printf("%d\n", ypx);

    int right = xi > xf ? xi : xf;
    int left = right == xi? xf : xi;

    int y = left == xi ? yi : yf; // Depends on which one is left
    int y_top = y == yi ? yf : yi;
    
    int abs_ypx = ypx < 0 ? -ypx : ypx; // An absolute value
    int inc = abs_ypx == ypx ? 1 : -1; // What value we increment in the y for loop

    for (int x = left; x <= right; x++)
    {
      for (int i = 0; i < abs_ypx && y != y_top; i++)
      {
        vid_set(color, y*vid_w + x);
        y += inc;
      }
    }
  }
}

void
gui_fdraw(psf_font_t* f, int _x, int _y, int g, unsigned char color)
{
  int add_x = _x < 0 ? -_x : 0;
  int add_y = _y < 0 ? -_y : 0;

  int width = psf_get_width(f);
  char* glyph = psf_get_glyph(f, g);

  int padding = width % 8;

  // b is the bit index, it goes through glyph as a whole as if it were a bit buffer
  int b = f->row_size * add_y;
  for (int y = _y + add_y; y < f->height + _y && y < vid_h; y++)
  {
    b += add_x;

    for (int x = _x + add_x; x < width + _x && x < vid_w; x++, b++)
    {
      char byte = glyph[b >> 3];
      
      // We just shift the byte left by b%8(to get the current bit) and just check if that lsb is on.
      // We do it from left to right because that's how we draw
      if ((byte << (b % 8)) & (1 << 7))
      {
        vid_set(color, x + y * vid_w);
      }
    }

    b += padding;
  }
}

void
gui_fdraws(psf_font_t* f, int x, int y, const char* str, unsigned char color)
{
  int width = psf_get_width(f);
  while(*str)
  {
    
  }
}
