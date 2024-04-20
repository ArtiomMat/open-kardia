#include "gui.h"
#include "fip.h"
#include "k.h"

#include <stdlib.h>
#include <stddef.h>

#define BTHICK (GUI_BORDER_WH>>1)

psf_font_t* font;

gui_window_t gui_window = {0};

int gui_title_h = 0;

static inline void
draw_xline(int xi, int xf, int y, int color)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = max(left, 0); x <= min(right, vid_w-1); x++)
  {
    vid_set(color, y*vid_w + x);
  }
}

static inline void
draw_yline(int yi, int yf, int x, int color)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = max(bottom, 0); y <= min(top, vid_h-1); y++)
  {
    vid_set(color, y*vid_w + x);
  }
}

static inline int
in_rect(int x_test, int y_test, int x, int y, int w, int h)
{
  return x_test < x+w && x_test >= x && y_test < y+h && y_test >= y;
}

static inline void
draw_rect(int x, int y, int w, int h, int color)
{
  draw_xline(x, x+w-1, y, color);
  draw_xline(x, x+w-1, y+h-1, color);
  
  draw_yline(y, y+h-1, x, color);
  draw_yline(y, y+h-1, x+w-1, color);
}

static inline void
draw_filled_rect(int x, int y, int w, int h, int color, int fill)
{
  draw_rect(x, y, w, h, color);

  for (int _x = x+1; _x < x+w-1; _x++)
  {
    for (int _y = y+1; _y < y+h-1; _y++)
    {
      vid_set(fill, _y*vid_w + _x);
    }
  }
}


void
gui_init(int w, int h, const char* title, psf_font_t* _font)
{
  font = _font;

  gui_window.things = NULL;

  gui_window.title = title;

  gui_window.w = w;
  gui_window.h = h;

  gui_window.x = gui_window.y = 0;

  gui_title_h = font->height + 2;

  puts("gui_init(): GUI module initialized, Motif-like!");
}

int
gui_on_vid(vid_event_t* e)
{
  switch (e->type)
  {
    case VID_E_PRESS:
    if (e->press.code == KEY_LMOUSE)
    {
      if (in_rect(mouse_x, mouse_y, gui_window.x+BTHICK-1, gui_window.y+BTHICK-1, gui_window.w-GUI_BORDER_WH+2, gui_title_h))
      {
        gui_window.flags |= GUI_WND_MOVING;
        gui_window.mouse_x_rel = mouse_x - gui_window.x;
        gui_window.mouse_y_rel = mouse_y - gui_window.y;
      }
    }
    break;
    
    case VID_E_RELEASE:
    if (e->release.code == KEY_LMOUSE)
    {
      gui_window.flags &= ~GUI_WND_MOVING;
    }
    break;
  }
  return 0;
}

void
gui_draw_window()
{
  // Move the window if necessary and other logic to keep track of movement
  if (gui_window.flags & GUI_WND_MOVING)
  {
    gui_window.x = mouse_x-gui_window.mouse_x_rel;
    gui_window.y = mouse_y-gui_window.mouse_y_rel;


    gui_window.x = min(max(gui_window.x, 0), vid_w-gui_window.w-1);
    gui_window.y = min(max(gui_window.y, 0), vid_h-gui_window.h-1);

  }

  int color0 = k_pickc(40,40,0);
  int color1 = k_pickc(90,90,0);
  int color2 = k_pickc(120,120,0);

  // Outer border
  draw_filled_rect(gui_window.x, gui_window.y, gui_window.w, gui_window.h, color0, color2);
  // Inner border
  draw_filled_rect(gui_window.x+BTHICK-1, gui_window.y+BTHICK-1, gui_window.w-GUI_BORDER_WH+2, gui_window.h-GUI_BORDER_WH+2, color0, color1);
  // Line from the borderto the 
  draw_filled_rect(gui_window.x+BTHICK-1, gui_window.y+BTHICK-1, gui_window.w-GUI_BORDER_WH+2, gui_title_h, color0, color2);
}

void
gui_draw_line(int xi, int yi, int xf, int yf, unsigned char color)
{
  // Vertical line
  if (xi == xf)
  {
    draw_yline(yi, yf, xi, color);
  }
  // Horizontal line
  else if (yi == yf)
  {
    draw_xline(xi, xf, yi, color);
  }
  // Angled line
  else
  {
    #undef FIP_FRAC_BITS
    #define FIP_FRAC_BITS 16
    
    fip_t ypx = ITOFIP(yf-yi);
    ypx /= xf-xi;

    int right = xi > xf ? xi : xf;
    int left = right == xi? xf : xi;

    fip_t y = left == xi ? yi : yf; // Depends on which one is left
    y = max(y, 0); // Just limit it
    y = ITOFIP(y);
    
    fip_t absi_ypx = ypx < 0 ? -ypx : ypx; // An absolute value
    
    fip_t sign = absi_ypx == ypx ? 1 : -1; // What value we increment in the y for loop

    int x;
    for (int x = max(left, 0); x < min(right, vid_w); x++)
    {
      fip_t i;
      for (i = 0; i < absi_ypx; i += ITOFIP(1))
      {
        fip_t set_y = FIPTOI(y + i*sign);
        if (set_y >= vid_h)
        {
          return; // Nothing happens after the loop anyway
        }
        vid_set(color, set_y*vid_w + x);
      }
      y += ypx;
    }

    #undef FIP_FRAC_BITS
    #define FIP_FRAC_BITS FIP_DEF_FRAC_BITS
  }
}

void
gui_draw_font(psf_font_t* f, int _x, int _y, int g, unsigned char color)
{
  int add_x = _x < 0 ? -_x : 0;
  int add_y = _y < 0 ? -_y : 0;

  int width = psf_get_width(f);
  char* glyph = psf_get_glyph(f, g);

  int padding = width % 8;

  // b is the bit index, it goes through glyph as a whole as if it were a bit buffer
  int b = f->row_size * add_y;
  for (int y = _y + add_y; y < f->height + _y && y < vid_h; y++)
  {
    b += add_x;

    for (int x = _x + add_x; x < width + _x && x < vid_w; x++, b++)
    {
      char byte = glyph[b >> 3];
      
      // We just shift the byte left by b%8(to get the current bit) and just check if that lsb is on.
      // We do it from left to right because that's how we draw
      if ((byte << (b % 8)) & (1 << 7))
      {
        vid_set(color, x + y * vid_w);
      }
    }

    b += padding;
  }
}