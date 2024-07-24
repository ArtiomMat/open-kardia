#pragma once

#include "com.hpp"

#include <cstdio>

namespace psf
{
  enum
  {
    PSF1_MODE512 = 1,
    PSF1_MODEHASTAB = 2,
    PSF1_MODESEQ = 4,

    PSF2_HAS_UNICODE_TABLE = 1,
  };
  
  typedef struct
  {
    uint32_t magic; // 72 b5 4a 86
    int32_t version;
    int32_t size; // Header size
    int32_t flags;
    int32_t length; // how many glyphs
    int32_t glyph_size;
    int32_t height;
    int32_t width;
  } psf2_t;

  typedef struct
  {
    uint16_t magic; // 36 04
    uint8_t mode;
    uint8_t size;
  } psf1_t;

  enum
  {
    PSF1 = sizeof (psf1_t),
    PSF2 = sizeof (psf2_t),
  };

  enum
  {
    P_AUTO,
    P_SPEED, // Prioritize speed, cache all characters in advance
    P_MEMORY, // Prioritize less memory, expands single bit to bitmaps in realtime
  };

  struct file_t
  {
    char* data = nullptr; // For speed priority
    FILE* fd = nullptr; // For memory priority
    unsigned data_size;
    unsigned char type; // Either sizeof(psf2_s) or sizeof(psf1_s)
    unsigned char p; // Priority
    unsigned char row_size; // In bytes(8 pixels)
    unsigned char height; // In actual pixels
    union
    {
      psf1_t psf1;
      psf2_t psf2;
      uint32_t __array[8]; // For quickly swapping endian in psf2
    };
    
    file_t() = default;

    file_t(const char* fp) : file_t(fp, P_AUTO) {}
    // p is priority, check out P_* enum
    file_t(const char* fp, int p) { open(fp, p); }
    ~file_t();
    
    void open(const char* fp, int p);

    int get_width();
    char* get_glyph(unsigned g);
  };
}
