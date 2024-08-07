// GUI module, directly depends on video

#pragma once

#include "vid.h"

#include <stdint.h>

// The width/height that the border adds to the window(includes both sides of the window, so gui_border_wh/2 is for one side, always divisable by 2)
#define GUI_BORDER_WH (6 * 2)

// How much the resize on the edges bleeds into the horizontal and vertical border sections
#define GUI_RESIZE_BLEED (10)

#define GUI_SHADES_N 5

// How deep we can do recursion, 0 indicates a that the draw function can only be called once, meaning we can't go any deeper in a thing
#define GUI_RECURSION_DEPTH 4

// GUI unit, the unit is in pixel length
// The size of the unit must allow for a value at least (2^10-1), while at the same time being signed to allow for special values up to -(2^3-1), which is why it's int16_t.
typedef int16_t gui_u_t;

enum
{
  _GUI_E_NULL, // For internal use
  _GUI_E_EAT, // Does not get sent to the game, just means that the GUI module should eat the event without notifying the gui_on() callback

  GUI_E_B_PRESS,
  GUI_E_B_RELEASE,

  GUI_E_CLOSE, // X button pressed

  GUI_E_ITXT_DONE, // When enter is pressed with an itext or just exiting it, escape is a special key that deselects the itext but doesn't send this event

  GUI_E_TICK, // The tickbox was toggled, either ticked or unticked, check e->thing

  GUI_E_SLIDE,
};

enum
{
  // FLAGS FOR WINDOW

  GUI_WND_INVISIBLE = 1<<0, // The window itself becomes completely invisible and non interactable, things are still drawn
  GUI_WND_FIX_SIZE = 1<<1, // The window cannot be resized
  GUI_WND_HIDE = 1<<2, // The window is hidden and not drawn! including things!
  GUI_WND_XRAY = 1<<3, // Show what is behind the window, only in the content area, essenitally does not render content. DEPRECATED!

  GUI_WND_UNFOCUSED = 1<<4,

  GUI_WND_RELOCATING = 1<<5, // The window is currently being moved, for internal use
  GUI_WND_RESIZING = ((1<<6) | (1<<7) | (1<<8) | (1<<9)), // The window is currently being resized, for internal use
  GUI_WND_RESIZING_L = 1<<6,
  GUI_WND_RESIZING_R = 1<<7,
  GUI_WND_RESIZING_T = 1<<8,
  GUI_WND_RESIZING_B = 1<<9,
  GUI_WND_NOT_RESIZABLE = 1<<10, // Just cached if it can be resized, this is done during opening of window

  // FORMAT STUFF FOR ITEXT

  GUI_ITXT_PASSWORD = 1<<0, // Shown as ****
  GUI_ITXT_NOALP = 1<<1, // No alphabet
  GUI_ITXT_NONUM = 1<<2, // No numbers
  GUI_ITXT_NOSYM = 1<<3, // No symbols
  GUI_ITXT_NOSPC = 1<<4, // No white space

  GUI_ITXT_SELECTED = 1<<5, // If the text is currently selected

  GUI_ITXT_NOT_VIRGIN = 1<<6, // If the flag is not on the moment the user begins typing all the text is overriden
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
  GUI_T_ROWMAP,
  GUI_T_ITEXT,
  GUI_T_OTEXT,
  GUI_T_BMAP,
  GUI_T_BUTTON,
  GUI_T_TICKBOX,
  GUI_T_SLIDER,

  _GUI_TYPES_N,

  // FLAGS
  GUI_T_FOUND         = 1<<0,

  GUI_T_LEFT          = 1<<1, // Align the thing to the left of its parent
  GUI_T_RIGHT         = 1<<2, // Align the thing to the right of its parent
  GUI_T_TOP           = 1<<3, // Align the thing to the top of its parent
  GUI_T_BOTTOM        = 1<<4, // Align the thing to the bottom of its parent

  GUI_T_AUTO_WIDTH    = 1<<5,
  GUI_T_AUTO_HEIGHT   = 1<<6,

  GUI_T_PARENT_WIDTH  = 1<<7, // Cannot be paired with AUTO_WIDTH
  GUI_T_PARENT_HEIGHT = 1<<8, // Cannot be paired with AUTO_HEIGHT

  GUI_T_HIDE          = 1<<9,

  // ARCHIVED BECAUSE parent ALREADY TELLS ME EVERYTHING.
  // GUI_T_IS_CHILD      = 1<<10, // The thing is a child of another in one way or another. If a thing has this as 0, it also indicates it's a "root" thing.
};

enum
{
  GUI_FONTP_AUTO,
  GUI_FONTP_SPEED, // Prioritize speed, cache all characters in advance
  GUI_FONTP_MEMORY, // Prioritize less memory, expands single bit to bitmaps in realtime
};

typedef struct
{
  unsigned int magic; // 72 b5 4a 86
  int version;
  int size; // Header size
  int flags;
  int length; // how many glyphs
  int glyph_size;
  int height;
  int width;
} gui_psf2_t;
typedef struct
{
  unsigned short magic; // 36 04
  char mode;
  unsigned char size;
} gui_psf1_t;

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
    gui_psf1_t psf1;
    gui_psf2_t psf2;
    int __array[8]; // For quickly swapping endian
  };
} gui_font_t;

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


typedef struct gui_bmap
{
  unsigned char* data;
  unsigned short size[2];
  int flags;
} gui_bmap_t;

typedef struct gui_thing gui_thing_t;

typedef struct gui_thing
{
  struct gui_thing* prev, * next;
  struct gui_thing* parent; // The parent, mostly for internal use 
  
  char* text; // Depends on what the thing is, but it's usually displayed as the lable of the thing.
  gui_u_t min_size[2], max_size[2]; // Always valid and no flag can make it unused!
  gui_u_t size[2]; // If overriden by flags the size is automatically modified by GUI.
  gui_u_t pos[2]; // If overriden by flags the position is automatically modified by GUI.
  short flags;
  unsigned char type;
  // unsigned char text_color; // The color of text usually, sometimes not even used, for window it's the titlebar color. If not specified in the GUI file, it will be set to the default logical color by GUI.
  char* id; // The string based ID
  union
  {
    // A row map contains rows and columns, it's like a table, but it's actually dynamic in number of columns per row.
    // Maybe I will add a column map some other time.
    struct
    {
      struct gui_thing** things;
      struct
      {
        unsigned char* cols_n;
        int size;
      } rows;
      unsigned char* cols_n; // Gives number of columns per each row index.
      unsigned char rows_n;
    } rowmap;

    struct
    {
      struct gui_thing* child;

      // The relative coordinates of the mouse to the x and y of the window when it was pressed on the title bar on the previous frame
      // For internal use
      gui_u_t mouse_rel[2];
      // The size last frame, for internal use, crucial for calculating resizing for good UX
      gui_u_t size_0[2];
      int flags;
    } window;

    struct
    {
      const char* fp;
      gui_bmap_t bmap;
    } bmap;

    struct
    {
      char flags;
      char format;
      uint16_t cursor; // Where the 
      uint16_t nmem; // Number of bytes for text in memory
    } itext;

    struct
    {
      char pressed; // 0=not, 1=yes, 2=wanna not, 3=next frame not
    } button;
    struct
    {
      char ticked;
    } tickbox;
    struct
    {
      unsigned short value; // 0 is the far left, maximum value is the right
    } slider;
  };
} gui_thing_t;

typedef struct
{
  gui_thing_t* thing;
  int type;
} gui_event_t;

/**
 * 5 shades, [0] is the darkest, [4] is the lightest. GUI uses this to color windows and stuff, choose them wisely! default is 0,1,2,3,4. Modifiable in realtime.
*/
extern unsigned char gui_shades[GUI_SHADES_N];

// It's a bad idea to completely override it because nothing would work then, a better idea is to override it if you need to but also make sure that you call gui_def_event_handler() in the end.
extern int (*gui_on)(gui_event_t* event);

// The height that the title bar part adds to the total window, this depends heavily on the font used
extern int gui_title_h;

extern px_t* gui_px;

/**
 * Event handler that must be called so GUI properly works.
 * Returns 1 if the even was eaten, meaning your program shouldn't handle it because it was in the jurisdiction of GUI, returns 0 if it was not eaten, this does not always mean that GUI didn't collect information about the event for itself.
*/
extern int
gui_on_vid(vid_event_t* event);

// Requires vid_init()
// If you change the order or the array of things itself you must gui_recache_all()!
extern void
gui_init(gui_font_t* font);
// If t is NULL fress the entire module with all things(must do that when you end the program).
// Frees a thing and its children/subthings(in maps or windows).
// NOTE: Freeing a thing that is referenced by any other thing WILL lead to problems, because that parent thing will not be notified, there is no way to do so in the data structure of gui_thing_t.
extern void
gui_free(gui_thing_t* t);

// Opens a file into memory, returns the first thing that was loaded
extern gui_thing_t*
gui_open(const char* fp);
// Searches through all things and locates the thing with this ID, returns NULL if not found.
// TYPE can be -1 to indicate that we actually have no idea what the type is(which would be quite interesting how the hell you don't know it), otherwise type would speed up the search drastically, internally the list of things is sorted by type.
// If ONETIME=1 then the thing can never be found again, this can speed up finding later on of other things. ONETIME can also be useful if you plan on loading the same file again or something similar.
extern gui_thing_t*
gui_find(int type, const char* id, char onetime);

// You should use this function only for non-windows, there is a special function for drawing windows.
// A thing is drawn, with its bounds being the entire screen, depending on what it is, it may stretch to the entire screen.
// Recursion(drawing children) works
extern void
gui_draw(gui_thing_t* t);
// Utility function to draw every window, instead of you worrying about it.
extern void
gui_draw_windows();

extern void
gui_set_flag(gui_thing_t* t, int flag, int yes);
extern void
gui_toggle_flag(gui_thing_t* t, int flag);

extern void
gui_draw_line(unsigned char color, gui_u_t xi, gui_u_t yi, gui_u_t xf, gui_u_t yf);

// Draw font, in pixels not in grid units.
// negative or too big x/y are still drawn partially if possible.
extern void
gui_draw_font(gui_font_t* f, gui_u_t x, gui_u_t y, int g, unsigned char color);

// Draws on a grid, x and y are in grid units based on the font dimentions.
static inline void
gui_draw_fontg(gui_font_t* f, gui_u_t x, gui_u_t y, int g, unsigned char color)
{
  gui_draw_font(f, x * f->row_size * 8, y * f->height, g, color);
}
