#include "gui.h"
#include "vid.h"

void
psf_fdraw(psf_font_t* f, int _x, int _y, int g, unsigned char color)
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
psf_fdraws(psf_font_t* f, int x, int y, const char* str, unsigned char color)
{
  int width = psf_get_width(f);
  while(*str)
  {
    
  }
}