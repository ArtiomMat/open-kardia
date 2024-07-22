#include "dsp.hpp"

#include <cstdlib>
#include <cstdio>
#include <malloc.h>

namespace dsp
{
  map_t::map_t(int w, int h, int fn)
  {
    p = static_cast<px_t*>( _aligned_malloc(w * h * fn, 8) );

    if (p == NULL)
    {
      throw com::memory_ex_t("Allocating pixels.");
    }

    this->w = w;
    this->h = h;
    this->fn = fn;

    _ppf = w * h;
  }
  
  map_t::~map_t()
  {
    _aligned_free(p);
  }

  map_t::map_t(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw com::open_ex_t("Opening map file.");
    }

    free(p);

    int status = 1;

    // Header
    status &= fread(&w, 2, 1, f);
    w = com::lil16(w);
    
    status &= fread(&h, 2, 1, f);
    h = com::lil16(h);

    status &= fread(&fn, 2, 1, f);
    fn = com::lil16(fn);

    // Cached stuff
    _ppf = w * h;

    // Data
    p = static_cast<px_t*>( _aligned_malloc(w * h * fn, 8) );
    if (p == nullptr)
    {
      throw com::read_ex_t("Allocating pixels.");
    }

    status &= fread(p, w * h * fn, 1, f);

    if (!status)
    {
      throw com::read_ex_t("Reading map file.");
    }
  }

  void map_t::clear(px_t color)
  {
    unsigned long i;
  
    for (i = 0; i < sizeof(long long); i++)
    {
      p[i] = color;
    }

    int total = s[0] * s[1];
    for (i = 1; i < total / sizeof(long long); i++)
    {
      ((long long*)(p))[i] = *((long long*)(p));
    }
  }

  void map_t::save(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw com::open_ex_t("Opening map file.");
    }
    uint16_t u16;
    
    int status = 1;

    // Header
    u16 = com::lil16(w);
    status &= fwrite(&u16, 2, 1, f);
    u16 = com::lil16(h);
    status &= fwrite(&u16, 2, 1, f);
    u16 = com::lil16(fn);
    status &= fwrite(&u16, 2, 1, f);

    // Data
    status &= fwrite(p, w * h * fn, 1, f);

    if (!status)
    {
      throw com::write_ex_t("Writing map file.");
    }
  }

  void map_t::put(psf::font_t& font, unsigned glyph_i, px_t fg, px_t bg)
  {
    char* glyph = font.get_glyph(glyph_i);
    int font_width = font.get_width();
    int padding = font_width % 8; // font_width is allowed to not be a multiple of 8

    // bit_i is the bit index, it goes through glyph as a whole as if it were a bit buffer
    int bit_i = 0;
    for (int y = 0; y < font.height && y < s[1]; y++)
    {
      for (int x = 0; x < font_width && x < s[0]; x++, bit_i++)
      {
        char byte = glyph[bit_i / 8];
        
        // We just shift the byte left by bit_i%8(to get the current bit) and just check if that lsb is on.
        // We do it from left to right because that's how we draw
        // If the bit is on then pixel=1 otherwise 0 ofc
        px_t pixel = ((byte << (bit_i % 8)) & (1 << 7)) ? fg : bg;
        
        put(pixel, x, y);
      }

      bit_i += padding;
    }
  }
}