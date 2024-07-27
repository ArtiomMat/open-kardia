#pragma once

#include "common.hpp"
#include "font.hpp"

namespace axe
{
  typedef uint8_t px_t;

  struct image_t
  {
    px_t* p = nullptr;
    
    union
    {
      uint16_t size[2]; // The size but per frame
      struct
      {
        uint16_t width, height; // The size but per frame
      };
    };
    uint32_t _ppf; // Cached pixels per frame(w*h)
    uint16_t fi = 0; // Frame index
    uint16_t fn; // Total frames

    image_t(int w, int h) : image_t(w, h, 1) { }
    image_t(int w, int h, int frames);
    // If you wish it's available.
    image_t(px_t* data, int w, int h, int frames);
    // Loads the data. Throws a specific file exception if fails.
    image_t(const char* fp);
    ~image_t();

    // Throws a specific file exception if fails.
    void save(const char* fp);

    inline void go(unsigned frame)
    {
      COM_PARANOID_A(frame < (unsigned)fn, "map_t::go(): Bad frame");
      fi = frame;
    }

    inline void put(px_t p, uint16_t x, uint16_t y)
    {
      COM_PARANOID_A(x+y*width < width*height*fn, "map_t::go(): Bad frame");
      this->p[x + y * width + fi] = p;
    }

    inline void put(px_t p, uint32_t i)
    {
      this->p[i + fi] = p;
    }

    // Appxoimately 8 times faster than manual put, because utilizes 64 bit aligned memory and shit.
    void clear(px_t p);
    
    // Puts m at its current frame, at x and y, on the current frame of this one(fi).
    // Quite fast like clear because also utilizes memory alignment.
    void put(image_t& m, uint16_t x, uint16_t y);
    
    void put(axe::font_t& font, unsigned glyph, uint16_t x, uint16_t y, px_t fg, px_t bg);
  };
}