// PC Screen Font API, intertwined with vid_

#pragma once

#include <stdio.h>

// For integration with Kardia's video module
#ifndef PSF_X_KARDIA
  #define PSF_X_KARDIA
#endif

enum
{
  PSF_P_AUTO,
  PSF_P_SPEED, // Prioritize speed, cache all characters in advance
  PSF_P_MEMORY, // Prioritize less memory, expands single bit to bitmaps in realtime
};

enum
{
  PSF1_MODE512 = 1,
  PSF1_MODEHASTAB = 2,
  PSF1_MODESEQ = 4,

  PSF2_HAS_UNICODE_TABLE = 1,
};

typedef struct
{
  // Union because depends on priority
  void* data; // For speed priority
  FILE* fd; // For memory priority
  unsigned data_size;
  unsigned char type; // Either sizeof(psf2_s) or sizeof(psf1_s)
  unsigned char p; // Priority
  unsigned char row_size; // In bytes(8 pixels)
  unsigned char height; // In actual pixels
  union
  {
    struct psf2_s
    {
      int magic; // 72 b5 4a 86
      int version;
      int size; // Header size
      int flags;
      int length; // how many glyphs
      int glyph_size;
      int height;
      int width;
    } psf2;
    int __array[8]; // For quickly swapping endian
    struct psf1_s
    {
      short magic; // 36 04
      char mode;
      unsigned char size;
    } psf1;
  };
} psf_font_t;

enum
{
  PSF1 = sizeof(struct psf1_s),
  PSF2 = sizeof(struct psf2_s),
};

// Do not forget to psf_close()
extern int
psf_open(psf_font_t* f, const char* fp, int priority);
extern void
psf_close(psf_font_t* f);
// Returns a pointer to the glyph, it is structured in row0,row1,...
// Note that it is in compressed bit form that needs to be expanded into a bitmap if you wish
extern void*
psf_get_glyph(psf_font_t* f, int g);

#ifdef PSF_X_KARDIA
  // Requires vid_init()
  // Free draw, in pixels not in grid units.
  // negative x/y are not drawn, no matter if they can show up on the screen, it breaks stuff.
  extern void
  psf_fdraw(psf_font_t* f, int x, int y, int g, unsigned char color);
  
  // Requires vid_init()
  // Draws on a grid, x and y are in grid units based on the font dimentions.
  static inline void
  psf_gdraw(psf_font_t* f, int x, int y, int g, unsigned char color)
  {
    psf_fdraw(f, x * f->row_size * 8, y * f->height, g, color);
  }
#endif
