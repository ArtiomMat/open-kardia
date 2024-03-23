// Video module

#pragma once

typedef struct
{
  
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
