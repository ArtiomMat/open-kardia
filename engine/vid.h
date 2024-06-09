// Video module

#pragma once

enum
{
  _VID_E_NULL,
  VID_E_CLOSE, // Close event, user wants to close the application
  VID_E_HIDE, // When the video module is hidden and cannot be seen by the user
  VID_E_SHOW, // When the video module is shown and seen by the user(sends it on vid_init() too)

  VID_E_PRESS, // Press key/button
  VID_E_RELEASE, // Release key/button
  VID_E_MOVE, // Move mouse
};

enum keys {
  KEY_ESCAPE=-64,
  KEY_ENTER,
  KEY_SPACE,
  KEY_BS,
  KEY_TAB,
  KEY_RALT,
  KEY_CTRL,
  KEY_LALT,
  KEY_LSHIFT,
  KEY_RSHIFT,
  KEY_CAPSLOCK,
  KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,
  KEY_NUMLOCK,KEY_SCROLLLOCK,

  KEY_LEFT,KEY_UP,KEY_RIGHT,KEY_DOWN,

  KEY_LMOUSE, // Left mouse
  KEY_MMOUSE, // Middle mouse
  KEY_RMOUSE, // Right mouse
};

enum
{
  VID_CUR_POINTER, // Regular pointer
  VID_CUR_SELECT, // Selection cursor(i.e hover over links)
  VID_CUR_TEXT, // Text hover
  VID_CUR_WAIT, // Waiting for something

  _VID_CUR_N,
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
} vid_event_t;

// 1 for event handled, 0 for event not handled
extern int (*vid_on)(vid_event_t*);

/**
 * An array of 256 colors with 3 channels.
 * First index is the color itself, second index is the channel(RGB).
 * Is actually a single buffer, vid_colors itself is structured in RGBRGBRGB... 256 times.
 *
 * malloc()-ed when vid_init() with , if you know what you are doing you may swap it with your own(making sure you avoid leaks), as long as yours adheres to the specification.
*/
extern unsigned char (*vid_colors)[3];

// This is platform specific so it should not be really accessed unless you know what you are doing
// vid_put() is hinted as inline and is part of the header, so use it, aswell as vid_get()
extern unsigned char* vid_pixels;

extern int vid_size[2];

// Position of the cursor
extern int vid_cursor[2];

extern int
vid_def_on(vid_event_t* event);

extern void
vid_set_title(const char* title);

/**
 * Returns 0 if fails, returns main refresh rate of screen if succeeds!
*/
extern int
vid_init(int w, int h);

extern void
vid_realize_colors();

/**
 * Run the window, meaning for one send all the events to vid_on, depending on underlying API may not blit the pixels though, call vid_refresh() to be 100% sure.
*/
extern void
vid_run();

/**
 * Set pixel of index I to the rgb value of COLOR_INDEX.
 * Must call vid_refresh() to actually blit the pixels.
*/

static inline void
vid_put(unsigned char color, int i)
{
#ifdef __linux__
  vid_pixels[i*4+3] = color; // We use the padding as the index, I am a fucking genius
  vid_pixels[i*4+2] = vid_colors[color][0];
  vid_pixels[i*4+1] = vid_colors[color][1];
  vid_pixels[i*4+0] = vid_colors[color][2];
#elif _WIN32_
  vid_pixels[i] = color;
#endif
}
extern void
vid_put_line(unsigned char color, int xi, int yi, int xf, int yf);
extern void
vid_put_xline(unsigned char color, int xi, int xf, int y);
extern void
vid_put_yline(unsigned char color, int yi, int yf, int x);
extern void
vid_put_rect(unsigned char fill, int left, int top, int right, int bottom);

static inline unsigned char
vid_get(int i)
{
#ifdef __linux__
  return vid_pixels[i*4+3];
#elif _WIN32_
  return vid_pixels[i];
#endif
}

extern void
vid_wipe(int i);

/**
 * Refresh the window with all the new pixels that were set.
*/
extern void
vid_refresh();

extern void
vid_free();

extern void
vid_set_cursor_type(int t);
