// GUI module, directly depends on video

#pragma once

#include "psf.h"

enum
{
  _GUI_E_NULL,
  GUI_E_HOVER,
  GUI_E_PRESS,
  GUI_E_
};

typedef struct
{
  int type;
  union
  {
    struct
    {
      int code;
    } press, release;
    struct
    {
      int x, y;
    } move;
  };
} gui_event_t;

// It's a bad idea to completely override it because nothing would work then, a better idea is to override it if you need to but also make sure that you call gui_def_event_handler() in the end.
extern void (*gui_event_handler)(gui_event_t* event);

// Requires vid_init()
// Free draw, in pixels not in grid units.
// negative or too big x/y are still drawn partially if possible.
extern void
gui_fdraw(psf_font_t* f, int x, int y, int g, unsigned char color);

extern void
gui_fdraws(psf_font_t* f, int x, int y, const char* str, unsigned char color);

// Requires vid_init()
// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_gdraw(psf_font_t* f, int x, int y, int g, unsigned char color)
{
  gui_fdraw(f, x * f->row_size * 8, y * f->height, g, color);
}

// Requires vid_init()
// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_gdraws(psf_font_t* f, int x, int y, const char* str, unsigned char color)
{
  gui_fdraws(f, x * f->row_size * 8, y * f->height, str, color);
}

// Just like psf_gdraw() but it wraps around!
static inline void
gui_gdraw_w(psf_font_t* f, int x, int y, int g, unsigned char color)
{
  gui_fdraw(f, x * f->row_size * 8, y * f->height, g, color);
}