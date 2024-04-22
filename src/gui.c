#include "gui.h"
#include "fip.h"
#include "k.h"

#include <stdlib.h>
#include <stddef.h>

#define BORDER_THICKNESS (GUI_BORDER_WH>>1)

#define BORDER_RIGHT (gui_window.pos[0] + gui_window.size[0] - 1)
#define BORDER_LEFT (gui_window.pos[0])
#define BORDER_TOP (gui_window.pos[1])
#define BORDER_BOTTOM (gui_window.pos[1] + gui_window.size[1] - 1)

#define TITLE_RIGHT (BORDER_RIGHT - BORDER_THICKNESS)
#define TITLE_LEFT (BORDER_LEFT + BORDER_THICKNESS)
#define TITLE_TOP (BORDER_TOP + BORDER_THICKNESS)
#define TITLE_BOTTOM (TITLE_TOP + gui_title_h - 1)

#define CONTENT_RIGHT TITLE_RIGHT
#define CONTENT_LEFT TITLE_LEFT
#define CONTENT_TOP TITLE_BOTTOM
#define CONTENT_BOTTOM (BORDER_BOTTOM - BORDER_THICKNESS)

psf_font_t* font;

gui_window_t gui_window = {0};

int gui_title_h = 0;

// The 5 shades of the window
static unsigned char shades[5];

int (*gui_on)(gui_event_t* event) = NULL;

static void
send_event(gui_event_t* e)
{
  if (gui_on != NULL && !gui_on(e))
  {
    puts("FUCK!");
  }
}

static inline void
draw_xline(int xi, int xf, int y, int color)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = MAX(left, 0); x <= MIN(right, vid_size[0]-1); x++)
  {
    vid_set(color, y*vid_size[0] + x);
  }
}

static inline void
draw_yline(int yi, int yf, int x, int color)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = MAX(bottom, 0); y <= MIN(top, vid_size[1]-1); y++)
  {
    vid_set(color, y*vid_size[0] + x);
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
      // if ((gui_window.flags & GUI_WND_XRAY) && in_rect(_x, _y, CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM))
      // {
      //   continue;
      // }
      vid_set(fill, _y*vid_size[0] + _x);
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
    gui_window.title = "\xFF\xFF\xFF";
  }

  gui_window.min_size[0] = gui_window.min_size[1] = 64;

  gui_window.size[0] = w;
  gui_window.size[1] = h;

  gui_window.pos[0] = gui_window.pos[1] = 0;

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
  int mouse_x = MIN(MAX(mouse_pos[0], 0), vid_size[0]-1);
  int mouse_y = MIN(MAX(mouse_pos[1], 0), vid_size[1]-1);

  gui_window.mouse_rel[0] = mouse_x - gui_window.pos[0];
  gui_window.mouse_rel[1] = mouse_y - gui_window.pos[1];
}

int
gui_on_vid(vid_event_t* e)
{
  switch (e->type)
  {
    case VID_E_PRESS:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_RMOUSE || e->press.code == KEY_MMOUSE)
    {
      // Inside of the title bar
      if (in_rect(mouse_pos[0], mouse_pos[1], TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM))
      {
        if (e->press.code == KEY_LMOUSE)
        {
          gui_window.flags |= GUI_WND_RELOCATING;
          save_mouse_rel();
        }
        else if (e->press.code == KEY_MMOUSE)
        {
          gui_window.flags |= GUI_WND_HIDE;
        }
        else if (e->press.code == KEY_RMOUSE)
        {
          gui_window.flags ^= GUI_WND_XRAY; // Toggle!
        }
        return 1;
      }
      // Inside of the content zone
      else if (!(gui_window.flags & GUI_WND_XRAY) && in_rect(mouse_pos[0], mouse_pos[1], CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM))
      {
        return 1;
      }
      // We are 100% either in border or outside the window alltogether
      else
      {
        int flags_tmp = gui_window.flags;

        // Right and left
        if (in_rect(mouse_pos[0], mouse_pos[1], CONTENT_RIGHT+1, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM))
        {
          gui_window.flags |= GUI_WND_RESIZING_R;
        }
        else if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, BORDER_TOP, CONTENT_LEFT-1, BORDER_BOTTOM))
        {
          gui_window.flags |= GUI_WND_RESIZING_L;
        }

        // Top and bottom
        if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, TITLE_TOP-1))
        {
          gui_window.flags |= GUI_WND_RESIZING_T;
        }
        else if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, CONTENT_BOTTOM+1, BORDER_RIGHT, BORDER_BOTTOM))
        {
          gui_window.flags |= GUI_WND_RESIZING_B;
        }

        // NOTE: Introduces thread unsafety because we do comparison if flags changed assuming they can't outside this scope.
        if (gui_window.flags != flags_tmp)
        {
          gui_window.size_0[0] = gui_window.size[0];
          gui_window.size_0[1] = gui_window.size[1];

          save_mouse_rel();
        }
      }
    }
    break;
    
    case VID_E_RELEASE:
    if (e->release.code == KEY_LMOUSE)
    {
      gui_window.flags &= ~GUI_WND_RELOCATING;
      gui_window.flags &= ~GUI_WND_RESIZING;
    }
    break;
  }
  return 0;
}

static void
resize_right(int i)
{
  int mouse_delta = mouse_pos[i] - (gui_window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.size[i] = gui_window.size_0[i] + mouse_delta;
  gui_window.size[i] = MIN(MAX(gui_window.size[i], gui_window.min_size[i]), vid_size[i] - gui_window.pos[i]);
}

static void
resize_left(int i)
{
  int mouse_delta = mouse_pos[i] - (gui_window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.pos[i] += mouse_delta;

  if (gui_window.pos[i] < 0)
  {
    mouse_delta -= gui_window.pos[i];
    gui_window.pos[i] = 0;
  }

  gui_window.size[i] = gui_window.size_0[i] - mouse_delta;

  if (gui_window.size[i] < gui_window.min_size[i])
  {
    gui_window.pos[i] -= gui_window.min_size[i] - gui_window.size[i];
    gui_window.size[i] = gui_window.min_size[i];
  }

  // So that in the next call to resize_left(), the size[i] does not return to be size_0(old). Since the X keeps updating, we need to shift the width with it, so that mouse_delta keeps being current. I know this is not really a good explanation, but just run this on a fucking paper, just shift the window do all the operations without this line, you will understand what I mean.
  gui_window.size_0[i] = gui_window.size[i];
}

void
gui_draw_window()
{
  if (gui_window.flags & GUI_WND_HIDE)
  {
    return;
  }
  // Move the window if necessary and other logic to keep track of movement
  if (gui_window.flags & GUI_WND_RELOCATING)
  {
    gui_event_t e;
    gui_window.pos[0] = e.relocate.delta[0] = mouse_pos[0]-gui_window.mouse_rel[0];
    gui_window.pos[1] = e.relocate.delta[1] = mouse_pos[1]-gui_window.mouse_rel[1];

    gui_window.pos[0] = e.relocate.normalized[0] = MIN(MAX(gui_window.pos[0], 0), vid_size[0]-gui_window.size[0]);
    gui_window.pos[1] = e.relocate.normalized[1] = MIN(MAX(gui_window.pos[1], 0), vid_size[1]-gui_window.size[1]);
    send_event(&e);
  }
  // Resize the window and keep track of resizing too
  else if (gui_window.flags & GUI_WND_RESIZING)
  {
    int flag = gui_window.flags & GUI_WND_RESIZING;
    int min_w = gui_window.min_size[0];
    int min_h = gui_window.min_size[1];


    if (flag & GUI_WND_RESIZING_R)
    {
      resize_right(0);
    }
    else if (flag & GUI_WND_RESIZING_L)
    {
      resize_left(0);
    }


    if (flag & GUI_WND_RESIZING_B)
    {
      resize_right(1);
    }
    else if (flag & GUI_WND_RESIZING_T)
    {
      resize_left(1);
    }
  }


  draw_filled_rect(BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM, shades[3], shades[1], shades[2]);
  draw_filled_rect(TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM, shades[3], shades[1], shades[2]);
  draw_filled_rect(CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM, shades[0], shades[3], shades[1]);

  if (gui_window.title != NULL)
  {
    int x = TITLE_LEFT+2;
    int width = psf_get_width(font);
    for (int i = 0; gui_window.title[i] && x+width < TITLE_RIGHT; i++, x+=width)
    {
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
    for (int x = MAX(left, 0); x < MIN(right, vid_size[0]); x++)
    {
      fip_t i;
      for (i = 0; i < absi_ypx; i += ITOFIP(1))
      {
        fip_t set_y = FIPTOI(y + i*sign);
        if (set_y >= vid_size[1])
        {
          return; // Nothing happens after the loop anyway
        }
        vid_set(color, set_y*vid_size[0] + x);
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
  for (int y = _y + add_y; y < f->height + _y && y < vid_size[1]; y++)
  {
    b += add_x;

    for (int x = _x + add_x; x < width + _x && x < vid_size[0]; x++, b++)
    {
      char byte = glyph[b >> 3];
      
      // We just shift the byte left by b%8(to get the current bit) and just check if that lsb is on.
      // We do it from left to right because that's how we draw
      if ((byte << (b % 8)) & (1 << 7))
      {
        vid_set(color, x + y * vid_size[0]);
      }
    }

    b += padding;
  }
}