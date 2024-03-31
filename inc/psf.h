// PC Screen Font API, intertwined with vid_

#pragma once

// For integration with Kardia's video module
// #define PSF_X_KARDIA

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
  void* data; // Depends on priority and implementation, not recommended to play with
  union
  {
    struct psf2_s
    {
      int magic; // 72 b5 4a 86
      int version;
      int size;
      int flags; // 
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
  int data_size;
  unsigned char type; // Either sizeof(psf2_s) or sizeof(psf1_s)
  unsigned char p; // Priority
  unsigned char row_size;
} psf_font_t;

// Do not forget to psf_close()
extern int
psf_open(psf_font_t* f, const char* fp, int priority);
extern void
psf_close(psf_font_t* f);
extern void*
psf_get_glyph(int i);
extern int
psf_get_width();
extern int
psf_get_width();

#ifdef PSF_X_KARDIA
  // Requires vid_init()
  extern void
  psf_gdraw(int x, int y, int i, unsigned char color);
  // Requires vid_init()
  // Free draw, no 
  extern void
  psf_fdraw(int x, int y, int i, unsigned char color);
#endif
