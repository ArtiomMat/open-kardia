// GUI module, directly depends on video

#pragma once

#include "psf.h"
#include "vid.h"

// The width/height that the border adds to the window(includes both sides of the window, so gui_border_wh/2 is for one side, always divisable by 2)
#define GUI_BORDER_WH (8 * 2)

enum
{
  _GUI_E_NULL,
  GUI_E_HOVER,
  GUI_E_PRESS,
  GUI_E_RELEASE,
  GUI_E_RELOCATE, // The user is moving a window
};


enum
{
  // FLAGS

  GUI_WND_INVISIBLE = 0b1, // The window itself becomes completely invisible and non interactable, things are still drawn
  GUI_WND_FIX_SIZE = 0b10, // The window cannot be resized
  GUI_WND_HIDE = 0b100, // The window is hidden and not drawn! including things!
  GUI_WND_FOLDED = 0b1000, // Only the title bar is visible, until unfolded

  GUI_WND_MOVING = 0b10000, // The window is currently being moved, for internal use

  GUI_WND_RESIZING = 0b111100000, // The window is currently being moved, for internal use

  GUI_WND_RESIZING_L = 0b100000000,
  GUI_WND_RESIZING_R = 0b010000000,
  GUI_WND_RESIZING_T = 0b001000000,
  GUI_WND_RESIZING_B = 0b000100000,
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
      unsigned char color;
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
      struct gui_text text;
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

// The container for things!
typedef struct gui_window_s
{
  const char* title;
  gui_thing_t* things;
  int flags;
  short w, h;
  short min_w, min_h;
  short x, y;

  // The relative coordinates of the mouse to the x and y of the window when it was first pressed on the title bar
  // For internal use
  int mouse_x_rel, mouse_y_rel;
} gui_window_t;

typedef struct
{
  int type;
  gui_window_t* window;
  gui_thing_t* thing;
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
extern int (*gui_on)(gui_event_t* event);

// The initial window, currently only one window can work.
extern gui_window_t gui_window;

// The height that the title bar part adds to the total window, this depends heavily on the font used
extern int gui_title_h;

// Requires vid_init()
extern void
gui_init(int w, int h, const char* title, psf_font_t* font);

extern void
gui_draw_window();

extern void
gui_show();

extern void
gui_hide();

extern void
gui_draw_line(int xi, int yi, int xf, int yf, unsigned char color);

// Returns 1 for event eaten, means that the program should not handle the vid event in general situations, otherwise it returns 0 for either partially handling the event(the program should too) or not at all.
// This must be called for GUI to actually handle shit from vid
extern int
gui_on_vid(vid_event_t* event);

// Free draw, in pixels not in grid units.
// negative or too big x/y are still drawn partially if possible.
extern void
gui_draw_font(psf_font_t* f, int x, int y, int g, unsigned char color);

// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_draw_fontg(psf_font_t* f, int x, int y, int g, unsigned char color)
{
  gui_draw_font(f, x * f->row_size * 8, y * f->height, g, color);
}