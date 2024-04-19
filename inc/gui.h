// GUI module, directly depends on video

#pragma once

#include "psf.h"
#include "vid.h"

enum
{
  _GUI_E_NULL,
  GUI_E_HOVER,
  GUI_E_PRESS,
  GUI_E_
};


enum
{
  // FLAGS

  GUI_WND_INVISIBLE = 0b1, // The window itself becomes completely invisible
  GUI_WND_RESIZABLE = 0b10, // The window itself becomes completely invisible
  GUI_WND_HIDE = 0b100, // The window itself becomes completely invisible
  GUI_WND_FOLDED = 0b1000, // Only the title bar is visible, until unfolded
};

enum
{
  // TYPES

  GUI_T_TEXT,
  GUI_T_IMAGE,
  GUI_T_VIDEO,
  GUI_T_BUTTON,
  GUI_T_TICKBOX,
  GUI_T_SLIDER,

  // FLAGS

  GUI_T_NEW_LINE = 0b1, // This thing is below the previous thing(next things are too now)
  GUI_T_RESIZABLE = 0b10, // The window itself becomes completely invisible
  GUI_T_HIDE = 0b100, // Becomes hidden(not drawn too)
  GUI_T_DISABLED = 0b1000, // Only the title bar is visible, until unfolded
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

typedef struct gui_thing_s
{
  struct gui_thing_s* next;
  struct gui_thing_s* prev;
  char type;
  char padding_t, padding_b; // Since things in a single line can have a conflict, this is only read for the GUI_T_NEW_LINE thing
  char padding_l;
  int flags;
  union
  {
    struct gui_text
    {
      char* str;
    } text;
    struct gui_image
    {
      char* fp;
      unsigned char* data; // may be NULL if still was not loaded
      unsigned short frames_n, frame;
      unsigned short fps, frame_time;
      unsigned short w, h;
      unsigned short loop : 1;
      unsigned short play : 1;
    } image, video;
    struct gui_button
    {
      char* str;
    } button;
    struct gui_tick
    {
      char ticked;
    } tickbox;
    struct gui_slider
    {
      unsigned short value; // 0 is obviously the far left, maximum value is the right
    } slider;
  };
} gui_thing_t;

// The container for all the elements
typedef struct
{
  gui_thing_t* things;
  int flags;
  unsigned short w, h;
  unsigned short x, y;
} gui_window_t;

// It's a bad idea to completely override it because nothing would work then, a better idea is to override it if you need to but also make sure that you call gui_def_event_handler() in the end.
// Returns 1 for event eaten, means that the program should not handle the vid event in general situations, otherwise it returns 0 for either partially handling the event(the program should too) or not at all.
extern int (*gui_on)(gui_event_t* event);

// The initial window, currently only one window can work.
extern gui_window_t gui_window;

// The height that the title bar part adds to the total window, this depends heavily on the font used
extern int gui_title_h;
// The width/height that the border adds to the window(includes both sides of the window, so gui_border_wh/2 is for one side, always divisable by 2)
extern int gui_border_wh;

// Requires vid_init()
extern void
gui_init(psf_font_t* font);

extern void
gui_draw_line(int xi, int yi, int xf, int yf);

// This must be called for GUI to actually handle shit from vid
extern int
gui_on_vid(vid_event_t* event);

// Free draw, in pixels not in grid units.
// negative or too big x/y are still drawn partially if possible.
extern void
gui_fdraw(psf_font_t* f, int x, int y, int g, unsigned char color);

extern void
gui_fdraws(psf_font_t* f, int x, int y, const char* str, unsigned char color);

// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_gdraw(psf_font_t* f, int x, int y, int g, unsigned char color)
{
  gui_fdraw(f, x * f->row_size * 8, y * f->height, g, color);
}

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