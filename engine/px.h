// Pixel module, just basic bitmap stuff

#pragma once

typedef struct
{
  // Pixels are 8 bit, while it can be applied to anything you wish, the colormap is in vid_, vid_colors. If feeding this px_t to any vid_ based module the colormap used is vid_colors.
  unsigned char* p;
  unsigned short s[2];
  unsigned int _total; // Total number of bytes
} px_t;

// What color index is ignored and not drawn
// So for instance if px_put_px encounters a color px_nil_color it will not be drawn onto the used px.
extern int px_nil_color;

extern int
px_init(px_t* m, int w, int h);
extern void
px_free(px_t* m);

// Wipes ~8 times faster than just writing byte by byte. (Tested on intel).
extern void
px_wipe(px_t* m, unsigned char color);

extern void
px_put_line(px_t* m, unsigned char color, int xi, int yi, int xf, int yf);
extern void
px_put_xline(px_t* m, unsigned char color, int xi, int xf, int y);
extern void
px_put_yline(px_t* m, unsigned char color, int yi, int yf, int x);
extern void
px_put_rect(px_t* m, unsigned char fill, int left, int top, int right, int bottom);

// Put OTHER on M with the top-left corner of OTHER on M being Y-X
extern void
px_put_px(px_t* restrict m, px_t* restrict o, int x, int y);

// Loads a PNG/JPG image into m, returns 0 if fails, returns 1 if succeeds.
// NOTE: It converts truecolor into 8 bit indices for vid_colors, the conversion depends on the current state of vid_colors. Additionally, if a PNG is opened, trasnparent pixels are replaced with px_nil_color.
extern int
px_load(px_t* m, const char* fp);
