#include "image.hpp"
#include "common.hpp"

#include <cstdlib>
#include <cstdio>
#include <malloc.h>

namespace axe
{
  image_t::image_t(int w, int h, int fn)
  {
    p = new px_t[w * h * fn];

    if (p == nullptr)
    {
      throw axe::memory_ex_t("Allocating pixels.");
    }

    this->width = w;
    this->height = h;
    this->fn = fn;

    _ppf = w * h;
  }

  image_t::image_t(px_t* data, int w, int h, int frames)
  {
    this->p = data;

    this->width = w;
    this->height = h;
    this->fn = frames;

    _ppf = w * h;
  }
  
  image_t::~image_t()
  {
    delete [] p;
  }

  image_t::image_t(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw open_ex_t("Opening map file.");
    }

    free(p);

    int status = 1;

    // Header
    status &= fread(&width, 2, 1, f);
    width = lil16(width);
    
    status &= fread(&height, 2, 1, f);
    height = lil16(height);

    status &= fread(&fn, 2, 1, f);
    fn = lil16(fn);

    // Cached stuff
    _ppf = width * height;

    // Data
    p = static_cast<px_t*>( _aligned_malloc(width * height * fn, 8) );
    if (p == nullptr)
    {
      throw read_ex_t("Allocating pixels.");
    }

    status &= fread(p, width * height * fn, 1, f);

    if (!status)
    {
      throw read_ex_t("Reading map file.");
    }
  }

  void image_t::clear(px_t color)
  {
    // unsigned long i;
  
    // for (i = 0; i < sizeof(long long); i++)
    // {
    //   p[i] = color;
    // }

    // int total = s[0] * s[1];
    // for (i = 1; i < total / sizeof(long long); i++)
    // {
    //   ((long long*)(p))[i] = *((long long*)(p));
    // }
    // TODO: Restore the peace and order, add back the alignment stuff
    for (uint32_t i = fi * _ppf; i < width * height + fi * _ppf; i++)
    {
      p[i] = color;
    }
  }

  void image_t::save(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw open_ex_t("Opening map file.");
    }
    uint16_t u16;
    
    int status = 1;

    // Header
    u16 = lil16(width);
    status &= fwrite(&u16, 2, 1, f);
    u16 = lil16(height);
    status &= fwrite(&u16, 2, 1, f);
    u16 = lil16(fn);
    status &= fwrite(&u16, 2, 1, f);

    // Data
    status &= fwrite(p, width * height * fn, 1, f);

    if (!status)
    {
      throw write_ex_t("Writing map file.");
    }
  }

  void image_t::put(axe::font_t& font, unsigned glyph_i, uint16_t _x, uint16_t _y, px_t fg, px_t bg)
  {
    char* glyph = font.get_glyph(glyph_i);
    int padding = font.width % 8; // font.width is allowed to not be a multiple of 8

    // bit_i is the bit index, it goes through glyph as a whole as if it were a bit buffer
    int bit_i = 0;

    for (int y = 0; y < font.height && (y+_y) < size[1]; y++)
    {
      for (int x = 0; x < font.width && (x+_x) < size[0]; x++, bit_i++)
      {
        char byte = glyph[bit_i / 8];
        
        // We just shift the byte left by bit_i%8(to get the current bit) and just check if that lsb is on.
        // We do it from left to right because that's how we draw
        // If the bit is on then pixel=1 otherwise 0 ofc
        px_t pixel = ((byte << (bit_i % 8)) & (1 << 7)) ? fg : bg;
        
        put(pixel, x + _x, y + _y);
      }

      bit_i += padding;
    }
  }
}