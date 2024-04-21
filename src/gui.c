#include "gui.h"
#include "fip.h"
#include "k.h"

#include <stdlib.h>
#include <stddef.h>

#define BORDER_THICKNESS (GUI_BORDER_WH>>1)

#define BORDER_RIGHT (gui_window.x + gui_window.w - 1)
#define BORDER_LEFT (gui_window.x)
#define BORDER_TOP (gui_window.y)
#define BORDER_BOTTOM (gui_window.y + gui_window.h - 1)

#define TITLE_RIGHT (BORDER_RIGHT - BORDER_THICKNESS)
#define TITLE_LEFT (BORDER_LEFT + BORDER_THICKNESS)
#define TITLE_TOP (BORDER_TOP + BORDER_THICKNESS)
#define TITLE_BOTTOM (TITLE_TOP + gui_title_h - 1)

#define CONTENT_RIGHT TITLE_RIGHT
#define CONTENT_LEFT TITLE_LEFT
#define CONTENT_TOP TITLE_TOP
#define CONTENT_BOTTOM (BORDER_BOTTOM - BORDER_THICKNESS)

psf_font_t* font;

gui_window_t gui_window = {0};

int gui_title_h = 0;

// The 5 shades of the window
static unsigned char shades[5];

static inline void
draw_xline(int xi, int xf, int y, int color)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = MAX(left, 0); x <= MIN(right, vid_w-1); x++)
  {
    vid_set(color, y*vid_w + x);
  }
}

static inline void
draw_yline(int yi, int yf, int x, int color)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = MAX(bottom, 0); y <= MIN(top, vid_h-1); y++)
  {
    vid_set(color, y*vid_w + x);
  }
}

static inline int
in_rect(int x_test, int y_test, int left, int top, int right, int bottom)
{
  return x_test <= right && x_test >= left && y_test <= bottom && y_test >= top;
}

static inline void
draw_rect(int left, int top, int right, int bottom, int light, int dark)
{
  draw_xline(left, right, top, light);
  draw_xline(left, right, bottom, dark);
  
  draw_yline(top, bottom, left, light);
  draw_yline(top, bottom, right, dark);
}

static inline void
draw_filled_rect(int left, int top, int right, int bottom, int light, int dark, int fill)
{
  draw_rect(left, top, right, bottom, light, dark);
  // Fill the mf now
  for (int _x = left+1; _x < right; _x++)
  {
    for (int _y = top+1; _y < bottom; _y++)
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
  if (gui_window.title == NULL)
  {
    gui_window.title = "NULL";
  }

  gui_window.min_w = gui_window.min_h = 0;

  gui_window.w = w;
  gui_window.h = h;

  gui_window.x = gui_window.y = 0;

  gui_title_h = font->height + 3;

  shades[0] = k_pickc(40,40,0);
  shades[1] = k_pickc(90,90,0);
  shades[2] = k_pickc(120,120,0);
  shades[3] = k_pickc(180,180,0);
  shades[4] = k_pickc(220,220,0);

  puts("gui_init(): GUI module initialized, Motif-like!");
}

static void
save_mouse_rel()
{
  gui_window.mouse_x_rel = mouse_x - gui_window.x;
  gui_window.mouse_y_rel = mouse_y - gui_window.y;
}

int
gui_on_vid(vid_event_t* e)
{
  switch (e->type)
  {
    case VID_E_PRESS:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_MMOUSE)
    {
      if (in_rect(mouse_x, mouse_y, TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM))
      {
        if (e->press.code == KEY_LMOUSE)
        {
          gui_window.flags |= GUI_WND_MOVING;
          save_mouse_rel();
        }
        else
        {
          gui_window.flags |= GUI_WND_HIDE;
        }
        return 1;
      }
      // Note that it also takes into account MMOUSE, gotta do something about it
      else if (!(gui_window.flags & GUI_WND_FIX_SIZE) && in_rect(mouse_x, mouse_y, CONTENT_RIGHT+1, CONTENT_BOTTOM+1, BORDER_RIGHT, BORDER_BOTTOM))
      {
        gui_window.flags |= GUI_WND_RESIZING;
        save_mouse_rel();
      }
    }
    break;
    
    case VID_E_RELEASE:
    if (e->release.code == KEY_LMOUSE)
    {
      gui_window.flags &= ~GUI_WND_MOVING;
      gui_window.flags &= ~GUI_WND_RESIZING;
    }
    break;
  }
  return 0;
}

void
gui_draw_window()
{
  if (gui_window.flags & GUI_WND_HIDE)
  {
    return;
  }
  // Move the window if necessary and other logic to keep track of movement
  if (gui_window.flags & GUI_WND_MOVING)
  {
    gui_window.x = mouse_x-gui_window.mouse_x_rel;
    gui_window.y = mouse_y-gui_window.mouse_y_rel;


    gui_window.x = MIN(MAX(gui_window.x, 0), vid_w-gui_window.w);
    gui_window.y = MIN(MAX(gui_window.y, 0), vid_h-gui_window.h);

  }
  // Resize the window and keep track of resizing too
  else if (gui_window.flags & GUI_WND_RESIZING)
  {
    gui_window.w += mouse_x - (gui_window.mouse_x_rel + gui_window.x);
    gui_window.h += mouse_y - (gui_window.mouse_y_rel + gui_window.y);
    save_mouse_rel();

    int min_w = min_w ? min_w : 64;
    int min_h = min_h ? min_h : 64;
    // Limit window to the vid_w/h and also limit it to min_w/h
    gui_window.w = MIN(MAX(gui_window.w, min_w), vid_w - gui_window.x - 1);
    gui_window.h = MIN(MAX(gui_window.h, min_h), vid_h - gui_window.y - 1);
  }


  // Outer border
  draw_filled_rect(BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM, shades[3], shades[1], shades[2]);
  draw_filled_rect(CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM, shades[0], shades[3], shades[1]);
  draw_filled_rect(TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM, shades[3], shades[0], shades[2]);

  if (gui_window.title != NULL)
  {
    int x = TITLE_LEFT+2;
    int width = psf_get_width(font);
    for (int i = 0; gui_window.title[i]; i++, x+=width)
    {
      // Adds ... if the title is too long
      // Very hardcoded rn, gotta improve code
      if (x+width*4 > TITLE_RIGHT+1)
      {
        for (int j = 0; j < 3; j++, x+=width)
        {
          gui_draw_font(font, x, TITLE_TOP+1, '.', shades[4]);
        }
        break;
      }

      gui_draw_font(font, x, TITLE_TOP+1, gui_window.title[i], shades[4]);
    }
  }
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
    y = MAX(y, 0); // Just limit it
    y = ITOFIP(y);
    
    fip_t absi_ypx = ypx < 0 ? -ypx : ypx; // An absolute value
    
    fip_t sign = absi_ypx == ypx ? 1 : -1; // What value we increment in the y for loop

    int x;
    for (int x = MAX(left, 0); x < MIN(right, vid_w); x++)
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