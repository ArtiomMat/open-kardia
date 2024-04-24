// GUI module, directly depends on video

#pragma once

#include "vid.h"

// The width/height that the border adds to the window(includes both sides of the window, so gui_border_wh/2 is for one side, always divisable by 2)
#define GUI_BORDER_WH (6 * 2)

// How much the resize on the edges bleeds into the horizontal and vertical border sections
#define GUI_RESIZE_BLEED (10)

enum
{
  _GUI_E_NULL, // For internal use

  GUI_E_HOVER, // Buttons
  GUI_E_PRESS, // Buttons
  GUI_E_RELEASE, // Buttons
  
  GUI_E_RELOCATE, // The user is moving a window
  GUI_E_HIDE, // The user hid the window
  GUI_E_SHOW, // The user opened/showed the window
  GUI_E_FOCUS, // The user focused the window
  GUI_E_UNFOCUS, // The user unfocused the window
};


enum
{
  // FLAGS

  GUI_WND_INVISIBLE = 0b1, // The window itself becomes completely invisible and non interactable, things are still drawn
  GUI_WND_FIX_SIZE = 0b10, // The window cannot be resized
  GUI_WND_HIDE = 0b100, // The window is hidden and not drawn! including things!
  GUI_WND_XRAY = 0b1000, // Show what is behind the window, only in the content area, essenitally does not render content. DEPRECATED!

  GUI_WND_UNFOCUSED = 0b10000,

  GUI_WND_RELOCATING = 0b100000, // The window is currently being moved, for internal use
  GUI_WND_RESIZING = 0b1111000000, // The window is currently being resized, for internal use
  GUI_WND_RESIZING_L = 0b1000000000,
  GUI_WND_RESIZING_R = 0b0100000000,
  GUI_WND_RESIZING_T = 0b0010000000,
  GUI_WND_RESIZING_B = 0b0001000000,
};

enum
{
  // TYPES

  GUI_T_TEXT,
  GUI_T_IMAGE,
  GUI_T_BUTTON,
  GUI_T_TICKBOX,
  GUI_T_SLIDER,

  // FLAGS

  GUI_T_NEW_LINE = 0b1, // This thing is below the previous thing(next things are too now)
  GUI_T_HIDE = 0b100, // Becomes hidden(not drawn)
  GUI_T_DISABLED = 0b1000, // Only the title bar is visible, until unfolded
};

enum
{
  GUI_FONTP_AUTO,
  GUI_FONTP_SPEED, // Prioritize speed, cache all characters in advance
  GUI_FONTP_MEMORY, // Prioritize less memory, expands single bit to bitmaps in realtime
};

typedef struct
{
  // Union because depends on priority
  void* data; // For speed priority
  void* fd; // For memory priority, internally a FILE*
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
} gui_font_t;

typedef struct gui_thing_s
{
  const char* str;
  union
  {
    struct gui_text
    {
      unsigned char color;
      unsigned short line_size; // in characters. 0 for unlimited line size
    } text; // Uses str as text
    struct gui_image
    {
      char* fp;
      unsigned char* data; // may be NULL if still was not loaded
      unsigned short frames_n, frame;
      unsigned short fps, frame_time;
      unsigned short size[2];
      unsigned short loop : 1;
      unsigned short play : 1;
    } image;
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
      unsigned short width; // 0 for an automatically determined width
    } slider;
  };
  short pos_cache[2]; // Cached position of the thing in the content_cache of the window, to quickly draw it.
  int flags;
  char type;
  char padding_t, padding_b; // Since things in a single line can have a conflict, this is only read for the GUI_T_NEW_LINE thing
  char padding_l;
} gui_thing_t;

// The container for things!
typedef struct gui_window_s
{
  const char* title;
  gui_thing_t* things;
  int things_n;

  // An array allocated to the size of the content at its most revealed state.
  // The idea is that things that have been drawn are stored in this cache.
  struct
  {
    unsigned char color;
    unsigned char index; // thing index, we can directly reference a thing corresponding to this cached pixel
  }* content_cache;

  int flags;
  short size[2];
  short min_size[2];
  short content_cache_size[2];
  short pos[2];

  // The relative coordinates of the mouse to the x and y of the window when it was first pressed on the title bar
  // For internal use
  int mouse_rel[2];
  // The size before we began resizing, for internal use, crucial for calculating resizing for good UX 
  int size_0[2];
} gui_window_t;

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
      int delta[2]; // The intended move(without clamping to vid_size)
      int normalized[2]; // Normalized move(clamped to the vid_size)
    } relocate;
  };
} gui_event_t;

/**
 * 5 shades, [0] is the darkest, [4] is the lightest. GUI uses this to color windows and stuff, choose them wisely! default is 0,1,2,3,4. Modifiable in realtime.
*/
extern unsigned char gui_shades[5];

// It's a bad idea to completely override it because nothing would work then, a better idea is to override it if you need to but also make sure that you call gui_def_event_handler() in the end.
extern int (*gui_on)(gui_event_t* event);

// The initial window, currently only one window can work.
extern gui_window_t gui_window;

// The height that the title bar part adds to the total window, this depends heavily on the font used
extern int gui_title_h;

extern int
gui_open_font(gui_font_t* f, const char* fp, int priority);
extern void
gui_close_font(gui_font_t* f);
// Returns a pointer to the glyph, it is structured in row0,row1,...
// Note that it is in compressed bit form that needs to be expanded into a bitmap if you wish
extern void*
gui_get_glyph(gui_font_t* f, int g);
// Unlike height not straight forward manually
extern int
gui_get_font_width(gui_font_t* f);

// Requires vid_init()
// If you change the order or the array of things itself you must gui_recache_all()!
extern void
gui_init(int w, int h, const char* title,  gui_thing_t* things, int things_n, gui_font_t* font);

// Essentially rerenders and reinitializes the entire content_cache of the window, this is done when things are moved around the window, should not be called too much
// If you modified a thing without fucking with its width and height directly or indirectly, you should call gui_recache_thing()
extern void
gui_recache_all();

// If you changed a thing a little in a way that wont directly do anything to its size you can call this.
extern void
gui_recache_thing(gui_thing_t* thing);

extern void
gui_free();

extern void
gui_draw_window();

extern void
gui_set_flag(int flag, int yes);
extern void
gui_toggle_flag(int flag);

extern void
gui_draw_line(int xi, int yi, int xf, int yf, unsigned char color);

/**
 * Event handler that must be called so GUI properly works.
 * Returns 1 if the even was eaten, meaning your program shouldn't handle it because it was in the jurisdiction of GUI, returns 0 if it was not eaten, this does not always mean that GUI didn't collect information about the event for itself.
*/
extern int
gui_on_vid(vid_event_t* event);

// Free draw, in pixels not in grid units.
// negative or too big x/y are still drawn partially if possible.
/**
 * Draw a font on the screen, 
*/
extern void
gui_draw_font(gui_font_t* f, int x, int y, int g, unsigned char color);

// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_draw_fontg(gui_font_t* f, int x, int y, int g, unsigned char color)
{
  gui_draw_font(f, x * f->row_size * 8, y * f->height, g, color);
}