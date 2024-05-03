// GUI module, directly depends on video

#pragma once

#include "vid.h"

// The width/height that the border adds to the window(includes both sides of the window, so gui_border_wh/2 is for one side, always divisable by 2)
#define GUI_BORDER_WH (6 * 2)

// How much the resize on the edges bleeds into the horizontal and vertical border sections
#define GUI_RESIZE_BLEED (10)

#define GUI_SHADES_N 5

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
  GUI_POS_LEFT=-1,
  GUI_POS_TOP=GUI_POS_LEFT,
  
  GUI_POS_RIGHT=-2,
  GUI_POS_BOTTOM=GUI_POS_RIGHT,
  
  GUI_SIZE_FILL=-1, // Can be used for both
  GUI_SIZE_SMART=-2, // Can be used for both
};

enum
{
  // TYPES
  GUI_T_WINDOW,
  GUI_T_MAP,
  GUI_T_ITEXT,
  GUI_T_OTEXT,
  GUI_T_BMAP,
  GUI_T_BUTTON,
  GUI_T_TICKBOX,
  GUI_T_SLIDER,

  // FLAGS

  GUI_T_LEFT          = 0b1, // Align the thing to the left of its parent
  GUI_T_RIGHT         = 0b10, // Align the thing to the right of its parent
  GUI_T_TOP           = 0b100, // Align the thing to the top of its parent
  GUI_T_BOTTOM        = 0b1000, // Align the thing to the bottom of its parent
  
  GUI_T_AUTO_WIDTH    = 0b10000,
  GUI_T_AUTO_HEIGHT   = 0b100000,
  
  GUI_T_PARENT_WIDTH  = 0b1000000, // Cannot be paired with AUTO_WIDTH
  GUI_T_PARENT_HEIGHT = 0b10000000, // Cannot be paired with AUTO_HEIGHT
  
  GUI_T_HIDE          =,
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

/**
 * Used religiously in various shit, and it is just a base struct for multi prupose use, like gui_film_t, cached image of the window, x button, etc.
*/
typedef struct gui_bmap
{
  unsigned char* data;
  unsigned short size[2];
  int flags;
} gui_bmap_t;

#if 0

typedef struct gui_thing
{
  struct gui_thing* next;
  
  union
  {
    /*struct gui_table
    {
      struct gui_table_e
      {
        struct gui_thing_s* t;
      }* e;
      int cols, rows;
    } table;*/

    struct gui_text
    {
      unsigned char color;
      unsigned short line_size; // in characters. 0 for unlimited line size
    } text; // Uses str as text
    
    gui_bmap_t bmap;

    struct gui_button
    {
      char pressed; // 1 for pressed, 0 for not
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
  
  const char* str;
  int flags;
  char type;
  unsigned short size[2];
  // short pos_cache[2]; // Cached position of the thing in the content_cache of the window, to quickly draw it.
} gui_thing_t;

#endif

typedef struct gui_thing gui_thing_t;

typedef struct gui_thing
{
  char* str; // Depends on what the thing is, but it's usually displayed as the lable of the thing.
  short min_size[2], max_size[2]; // Always valid and no flag can make it unused!
  short size[2]; // If overriden by flags the size is automatically modified by GUI.
  short pos[2]; // If overriden by flags the position is automatically modified by GUI.
  short flags;
  char type;
  unsigned char color; // The color of text usually, sometimes not even used, for window it's the titlebar color. If not specified in the GUI file, it will be set to the default logical color by GUI.
  unsigned char id; // Mainly so you can identify stuff, you can use the same ID for multiple things
  union
  {
    // A map contains rows and columns, it's like a table, but it's actually dynamic in number of columns per row.
    // It would be benefitial to have another column based map, but that's what you got, deal with it.
    struct
    {
      unsigned char* indices;
      unsigned char* cols_n; // Gives number of columns per each row index.
      unsigned char rows_n;
    } map;
    
    struct
    {
      unsigned char thing;
      // The relative coordinates of the mouse to the x and y of the window when it was first pressed on the title bar
      // For internal use
      short mouse_rel[2];
      // The size before we began resizing, for internal use, crucial for calculating resizing for good UX 
      short size_0[2];
      int flags;
    } window;
    
    gui_bmap_t bmap;
    
    struct
    {
      char pressed;
    } button;
    struct
    {
      char ticked;
    } tick;
    struct gui_slider
    {
      unsigned short value; // 0 is the far left, maximum value is the right
    } slider;
  };
} gui_thing_t;


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
extern unsigned char gui_shades[GUI_SHADES_N];

// It's a bad idea to completely override it because nothing would work then, a better idea is to override it if you need to but also make sure that you call gui_def_event_handler() in the end.
extern int (*gui_on)(gui_event_t* event);

// The initial window, currently only one window can work.
extern gui_thing_t gui_window;

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
gui_init(int w, int h, const char* title, gui_thing_t* thing, gui_font_t* _font);

// Essentially rerenders and reinitializes the entire content_cache of the window, this is done when things are moved around the window, should not be called too much.
extern void
gui_recache_all();

// If you modified a thing without fucking with its width and height directly or indirectly, you need to call this to recache it into the window cache bitmap.
// If you changed the size, you must use gui_recache_all()
extern void
gui_recache_thing(gui_thing_t* thing);

extern void
gui_free();

extern int
gui_open(const char* fp);
// Returns an index
extern unsigned char
gui_find(const char* id);

// This function is the starting point of any drawn thing.
// the rectangle given, is the area with which the thing is allowed to work with, it is guaranteed that the thing will not dare step outside these coordinates. Depending on flags and shit, the thing may align itself insisde the rectangle.
extern void
gui_draw(gui_thing_t* t, int left, int top, int right, int bottom);

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
