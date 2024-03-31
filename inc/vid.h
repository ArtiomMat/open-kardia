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

extern void (*vid_event_handler)(vid_event_t* event);

/**
 * An array of 256 colors with 3 channels.
 * First index is the color itself, second index is the channel(RGB).
 * Is actually a single buffer, vid_colors itself is structured in RGBRGBRGB... 256 times.
*/
extern unsigned char (*vid_colors)[3];

// BGRX form, X is nothing, because initially was for x11 24 bit depth 32 bit aligned.
// extern unsigned char* vid_pixels;

extern int vid_w, vid_h;

extern void
vid_def_event_handler(vid_event_t* event);

extern void
vid_screen_size(int* w, int* h);

extern void
vid_set_title(const char* title);

/**
 * 
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
