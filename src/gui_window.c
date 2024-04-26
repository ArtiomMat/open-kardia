#include "gui.h"
#include "fip.h"

#include "mix.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#ifndef MIN
  #define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
   _a > _b ? _a : _b; })
#endif
#ifndef MIN
  #define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
   _a < _b ? _a : _b; })
#endif

#define X_WIDTH 12

#define BORDER_THICKNESS (GUI_BORDER_WH>>1)

#define BORDER_RIGHT (gui_window.pos[0] + gui_window.size[0] - 1)
#define BORDER_LEFT (gui_window.pos[0])
#define BORDER_TOP (gui_window.pos[1])
#define BORDER_BOTTOM (gui_window.pos[1] + gui_window.size[1] - 1)

#define TITLE_RIGHT (BORDER_RIGHT - BORDER_THICKNESS)
#define TITLE_LEFT (BORDER_LEFT + BORDER_THICKNESS)
#define TITLE_TOP (BORDER_TOP + BORDER_THICKNESS)
#define TITLE_BOTTOM (TITLE_TOP + gui_title_h - 1)

// All that is used because X is literally inside of the title
#define X_LEFT (TITLE_RIGHT - X_WIDTH)
#define X_BOTTOM (TITLE_BOTTOM - 2)


#define CONTENT_RIGHT TITLE_RIGHT
#define CONTENT_LEFT TITLE_LEFT
#define CONTENT_TOP (TITLE_BOTTOM+0)
#define CONTENT_BOTTOM (BORDER_BOTTOM - BORDER_THICKNESS)

gui_font_t* font;

gui_window_t gui_window = {0};

int gui_title_h = 0;

unsigned char gui_shades[GUI_SHADES_N] = {0,1,2,3,4};

static int mouse_pos[2] = {0};

unsigned char get_shade(int i)
{
  // Return darker shade if unfocused
  if (gui_window.flags & GUI_WND_UNFOCUSED)
  {
    return gui_shades[MAX(i - 1, 0)];
  }
  return gui_shades[i];
}

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

// Draws a rectangle with light being the color on the top and left, dark is right and bottom
static inline void
draw_rect(int left, int top, int right, int bottom, int light, int dark)
{
  draw_xline(left, right, top, light);
  draw_xline(left, right, bottom, dark);
  
  draw_yline(top, bottom, left, light);
  draw_yline(top, bottom, right, dark);
}

// Extends draw_rect and also fills the rectangle.
// If xray is enabled it avoids filling the rectangle there.
static inline void
draw_filled_rect(int left, int top, int right, int bottom, int light, int dark, int fill)
{
  draw_rect(left, top, right, bottom, light, dark);
  // Fill the mf now
  for (int _x = left+1; _x < right; _x++)
  {
    // if ((gui_window.flags & GUI_WND_XRAY) && _x >= CONTENT_LEFT)
    // {
    //   _x = CONTENT_RIGHT-1;
    //   continue;
    // }

    for (int _y = top+1; _y < bottom; _y++)
    {
      vid_set(fill, _y*vid_size[0] + _x);
    }
  }
}

void
gui_set_flag(int flag, int yes)
{
  if (yes)
  {
    gui_window.flags |= flag;
    return;
  }
  gui_window.flags &= ~flag;
}

void
gui_toggle_flag(int flag)
{
  gui_window.flags ^= flag;
}

static void
get_thing_width(gui_thing_t* t, short out[2])
{
  switch (t->type)
  {
    case GUI_T_TEXT: ;
    int lines = 0; // How many lines in total
    int max_line_n = 0, line_n = 0; // max and current line length
    int text_len, c;

    for (text_len = 0; c = t->str[text_len]; text_len++, line_n++)
    {
      if (c == '\n')
      {
        if (line_n > max_line_n)
        {
          max_line_n = line_n;
        }
        line_n = -1;
        lines++;
      }
      else if (text_len % t->text.line_size)
      {

      }
    }
    
    return;
  }
}

void
gui_recache_all()
{
  gui_window.content_cache_size[0] = gui_window.content_cache_size[1] = 0;
  short size[2] = {0}; // For storring current sizes of this line
  for (int i = 0; i < gui_window.things_n; i++)
  {
    gui_thing_t* t = gui_window.things+i;
  }
}

void
gui_init(int w, int h, const char* title, gui_thing_t* things, int things_n, gui_font_t* _font)
{
  font = _font;

  gui_window.things = NULL;

  gui_window.title = title;
  if (gui_window.title == NULL)
  {
    gui_window.title = "\xFF";
  }

  gui_window.min_size[0] = gui_window.min_size[1] = 64;

  gui_window.size[0] = w;
  gui_window.size[1] = h;

  gui_window.pos[0] = gui_window.pos[1] = 0;

  gui_title_h = font->height + 3;

  gui_window.things = things;
  gui_window.things_n = things_n;

  gui_window.content_cache = NULL;

  printf("gui_init(): GUI module initialized, '%s' is no more a dream!\n", gui_window.title);
}

void
gui_free()
{
  free(gui_window.content_cache);
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
  if (gui_window.flags & GUI_E_HIDE)
  {
    return 0;
  }

  gui_event_t gui_e = {.type = _GUI_E_NULL};

  switch (e->type)
  {
    case VID_E_MOVE:
    mouse_pos[0] = e->move.x;
    mouse_pos[1] = e->move.y;
    break;

    case VID_E_PRESS:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_RMOUSE || e->press.code == KEY_MMOUSE)
    {
      // Inside of the title bar
      if (in_rect(mouse_pos[0], mouse_pos[1], TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM))
      {
        if (mouse_pos[0] >= X_LEFT && mouse_pos[1] <= X_BOTTOM)
        {
          gui_set_flag(GUI_WND_HIDE, 1);
          gui_set_flag(GUI_WND_UNFOCUSED, 1); // Also we unfocus so the user can interact again
          
          gui_e.type = GUI_E_HIDE;
          send_event(&gui_e);
          break;
        }

        if (e->press.code == KEY_LMOUSE)
        {
          gui_set_flag(GUI_WND_RELOCATING, 1);
          save_mouse_rel();
        }
        // else if (e->press.code == KEY_MMOUSE)
        // {
        //   gui_set_flag(GUI_WND_HIDE, 1);
        //   gui_set_flag(GUI_WND_UNFOCUSED, 1); // Also we unfocus so the user can interact again
          
        //   gui_e.type = GUI_E_HIDE;
        //   send_event(&gui_e);
        //   break;
        // }
        else if (e->press.code == KEY_RMOUSE)
        {
          gui_toggle_flag(GUI_WND_XRAY); // Toggle!
        }

        // We need to automatically focus back the window regardless, we don't reach this is middle clicked
        gui_window.flags &= ~GUI_WND_UNFOCUSED;

        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        gui_e.type = GUI_E_UNFOCUS;
        break;
      }
      // Inside of the content zone
      else if (!(gui_window.flags & GUI_WND_XRAY) && in_rect(mouse_pos[0], mouse_pos[1], CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM))
      {
        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        gui_e.type = GUI_E_HIDE;
        break;
      }
      // We are 100% either in border or outside the window alltogether
      else
      {
        int flags_tmp = gui_window.flags; // Save the flag for future

        // Right and left border
        if (in_rect(mouse_pos[0], mouse_pos[1], CONTENT_RIGHT+1 - GUI_RESIZE_BLEED, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM))
        {
          gui_set_flag(GUI_WND_RESIZING_R, 1);
        }
        else if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, BORDER_TOP, CONTENT_LEFT-1 + GUI_RESIZE_BLEED, BORDER_BOTTOM))
        {
          gui_set_flag(GUI_WND_RESIZING_L, 1);
        }

        // Top and bottom border, disconnected to combine the two in the corners
        if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, TITLE_TOP-1 + GUI_RESIZE_BLEED))
        {
          gui_set_flag(GUI_WND_RESIZING_T, 1);
        }
        else if (in_rect(mouse_pos[0], mouse_pos[1], BORDER_LEFT, CONTENT_BOTTOM+1 - GUI_RESIZE_BLEED, BORDER_RIGHT, BORDER_BOTTOM))
        {
          gui_set_flag(GUI_WND_RESIZING_B, 1);
        }

        // NOTE: Introduces thread unsafety because we do comparison if flags changed assuming they can't outside this scope.
        if (gui_window.flags != flags_tmp)
        {
          gui_window.size_0[0] = gui_window.size[0];
          gui_window.size_0[1] = gui_window.size[1];

          save_mouse_rel();
          gui_set_flag(GUI_WND_UNFOCUSED, 0);
          gui_e.type = GUI_E_FOCUS;
          break;
        }
        // 100% outside the window, if not already set we set and put the event for eaten
        // We should only send the event if it's the first time, sending the event otherwise is both unnecessary and causes bugs(cannot interact outside GUI)
        else if (!(gui_window.flags & GUI_WND_UNFOCUSED))
        {
          gui_set_flag(GUI_WND_UNFOCUSED, 1);
          gui_e.type = GUI_E_UNFOCUS;
          break;
        }
        // Otherwise it's not eaten ofc
      }
    }
    break;
    
    case VID_E_RELEASE:
    if (e->press.code == KEY_LMOUSE || e->press.code == KEY_RMOUSE || e->press.code == KEY_MMOUSE)
    {
      gui_set_flag(GUI_WND_RELOCATING, 0);
      gui_set_flag(GUI_WND_RESIZING, 0);
    }
    break;
  }
  
  // Decide if we ate or not!
  if (gui_e.type == _GUI_E_NULL)
  {
    return 0;
  }
  send_event(&gui_e);
  return 1;
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

  // HANDLE WINDOW LOGIC!

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

  // Draw the window decorations and stuff
  draw_filled_rect(BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM, get_shade(3), get_shade(1), get_shade(2));
  
  // draw_rect(CONTENT_LEFT, CONTENT_TOP-1, CONTENT_RIGHT, CONTENT_BOTTOM, get_shade(3), get_shade(1));

  draw_filled_rect(TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM, get_shade(3), get_shade(1), get_shade(2));
  
  // draw_filled_rect(CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM, get_shade(0), get_shade(3), get_shade(1));

  draw_yline(TITLE_TOP, TITLE_BOTTOM-1, X_LEFT, get_shade(4));
  // draw_filled_rect(X_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM, get_shade(3), get_shade(1), get_shade(2));
  gui_draw_font(font, X_LEFT + X_WIDTH/2 - 3, TITLE_TOP, 'x', get_shade(4));
  
  // Draw window title
  {
    int x = TITLE_LEFT+2;
    int width = gui_get_font_width(font);
    for (int i = 0; gui_window.title[i] && x+width < X_LEFT; i++, x+=width)
    {
      gui_draw_font(font, x, TITLE_TOP+2, gui_window.title[i], get_shade(4));
    }
  }

  for (int x = BORDER_LEFT; x < BORDER_RIGHT; x++)
  {
    int i = x + (BORDER_BOTTOM+1) * vid_size[0];
    // printf("%d, ", vid_get(i));
    vid_set(mix_shr(vid_get(i), 10), i);
  }
  // printf("\n");

  // Drawing the elements and shit
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
