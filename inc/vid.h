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
  KEY_ESC=-64,
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

  KEY_LMOUSE, // Left mouse
  KEY_MMOUSE, // Middle mouse
  KEY_RMOUSE, // Right mouse
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
extern unsigned char* vid_pixels;

extern int vid_w, vid_h;

extern int
vid_def_on(vid_event_t* event);

extern void
vid_screen_size(int* w, int* h);

extern void
vid_set_title(const char* title);

/**
 * Returns 0 if fails, returns main refresh rate of screen if succeeds!
*/
extern int
vid_init(int w, int h);

/**
 * Run the window, depending on underlying API may not blit the pixels though, vid_refresh() does.
*/
extern void
vid_run();

/**
 * Set pixel of index I to the rgb value of COLOR_INDEX.
 * Must call vid_refresh() to actually blit the pixels.
*/
extern void
vid_set(unsigned char color, int i);

extern void
vid_wipe(int i);

/**
 * Refresh the window with all the new pixels that were set.
*/
extern void
vid_refresh();

extern void
vid_free();
