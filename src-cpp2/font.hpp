#pragma once

#include "common.hpp"

#include <cstdio>

namespace axe
{
  struct font_t
  {
    enum
    {
      P_AUTO,
      P_SPEED, // Prioritize speed, cache all characters in advance
      P_MEMORY, // Prioritize less memory, expands single bit to bitmaps in realtime
    };

    char* data = nullptr; // For speed priority
    FILE* fd = nullptr; // For memory priority
    unsigned char _type; // Either sizeof(psf2_s) or sizeof(psf1_s)
    unsigned char p; // Priority
    unsigned char row_size; // In bytes(8 pixels)
    union
    {
      unsigned char width; // In actual pixels
      unsigned char height; // In actual pixels
      unsigned char size[2];
    };

    font_t(const char* fp) : font_t(fp, P_AUTO) {}
    // p is priority, check out P_* enum
    font_t(const char* fp, int p) { open(fp, p); }
    ~font_t();
    
    void open(const char* fp, int p);

    char* get_glyph(unsigned g);
  };
}
