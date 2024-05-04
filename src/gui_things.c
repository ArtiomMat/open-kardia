#include "gui.h"
#include "fip.h"
#include "com.h"
#include "mix.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define TICKBOX_SIZE 8

#define BORDER_THICKNESS (GUI_BORDER_WH>>1)
#define BORDER_RIGHT (gui_window.pos[0] + gui_window.size[0] - 1)
#define BORDER_LEFT (gui_window.pos[0])
#define BORDER_TOP (gui_window.pos[1])
#define BORDER_BOTTOM (gui_window.pos[1] + gui_window.size[1] - 1)

#define TITLE_RIGHT (BORDER_RIGHT - BORDER_THICKNESS)
#define TITLE_LEFT (BORDER_LEFT + BORDER_THICKNESS)
#define TITLE_TOP (BORDER_TOP + BORDER_THICKNESS)
#define TITLE_BOTTOM (TITLE_TOP + gui_title_h - 1)

// All that is used because X is inside of the title so it inherits some stuff
#define X_LEFT (TITLE_RIGHT - X_WIDTH)
#define X_BOTTOM (TITLE_BOTTOM - 2)
#define X_WIDTH 12

#define CONTENT_RIGHT (TITLE_RIGHT-1)
#define CONTENT_LEFT (TITLE_LEFT+1)
#define CONTENT_TOP (TITLE_BOTTOM+1)
#define CONTENT_BOTTOM (BORDER_BOTTOM - BORDER_THICKNESS - 1)

static gui_font_t* font;
int font_w;
static int mouse_pos[2] = {0};
static int mouse_state; // 1 for pressed, 0 for not!

// The currently focused thing, for instance a button, eg if enter is pressed we press it.
// static gui_thing_t* focused_thing = NULL;

gui_thing_t gui_window = {0};

int gui_title_h = 0;

unsigned char gui_shades[GUI_SHADES_N] = {0,1,2,3,4};

gui_thing_t* things;
int gui_things_n = 0;

int (*gui_on)(gui_event_t* event) = NULL;

static gui_bmap_t main_map;

static unsigned char
get_shade(int i)
{
  // Return darker shade if unfocused
  if (gui_window.window.flags & GUI_WND_UNFOCUSED)
  {
    return gui_shades[max(i - 1, 0)];
  }
  return gui_shades[i];
}

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

  for (int x = max(left, 0); x <= min(right, vid_size[0]-1); x++)
  {
    vid_set(color, y*vid_size[0] + x);
  }
}

static inline void
draw_yline(int yi, int yf, int x, int color)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = max(bottom, 0); y <= min(top, vid_size[1]-1); y++)
  {
    vid_set(color, y*vid_size[0] + x);
  }
}

/**
 * Test if X_TEST and Y_TEST are within a rectangle.
*/
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
    gui_window.window.flags |= flag;
    return;
  }
  gui_window.window.flags &= ~flag;
}

void
gui_toggle_flag(int flag)
{
  gui_window.window.flags ^= flag;
}

/*
static void
get_thing_size(gui_thing_t* t, short out[2])
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
}*/


void
gui_init(int w, int h, const char* title, gui_thing_t* thing, gui_font_t* _font)
{
  font = _font;
  font_w = gui_get_font_width(font);

  gui_window.str = (char*) title;
  if (gui_window.str == NULL)
  {
    gui_window.str = "\xFF";
  }

  gui_window.type = GUI_T_WINDOW;

  gui_window.min_size[0] = gui_window.min_size[1] = 64;

  gui_window.size[0] = w;
  gui_window.size[1] = h;

  gui_window.pos[0] = gui_window.pos[1] = 0;

  gui_title_h = font->height + 3;

  // gui_window.thing = thing;

  // gui_window.content_cache = NULL;

  printf("gui_init(): GUI module initialized, '%s' is no more a dream!\n", gui_window.str);
}

void
gui_free()
{
  // TODO: Actually free stuff
  gui_things_n = 0;
  // free(gui_window.content_cache);
}

static void
save_mouse_rel()
{
  int mouse_x = min(max(mouse_pos[0], 0), vid_size[0]-1);
  int mouse_y = min(max(mouse_pos[1], 0), vid_size[1]-1);

  gui_window.window.mouse_rel[0] = mouse_x - gui_window.pos[0];
  gui_window.window.mouse_rel[1] = mouse_y - gui_window.pos[1];
}


int
gui_on_vid(vid_event_t* e)
{
  if (gui_window.window.flags & GUI_E_HIDE)
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
      mouse_state = 1;

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
        gui_window.window.flags &= ~GUI_WND_UNFOCUSED;

        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        gui_e.type = GUI_E_UNFOCUS;
        break;
      }
      // Inside of the content zone
      else if (!(gui_window.window.flags & GUI_WND_XRAY) && in_rect(mouse_pos[0], mouse_pos[1], CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM))
      {
        gui_set_flag(GUI_WND_UNFOCUSED, 0);
        gui_e.type = GUI_E_HIDE;
        break;
      }
      // We are 100% either in border or outside the window alltogether
      else
      {
        int flags_tmp = gui_window.window.flags; // Save the flag for future

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
        if (gui_window.window.flags != flags_tmp)
        {
          gui_window.window.size_0[0] = gui_window.size[0];
          gui_window.window.size_0[1] = gui_window.size[1];

          save_mouse_rel();
          gui_set_flag(GUI_WND_UNFOCUSED, 0);
          gui_e.type = GUI_E_FOCUS;
          break;
        }
        // 100% outside the window, if not already set we set and put the event for eaten
        // We should only send the event if it's the first time, sending the event otherwise is both unnecessary and causes bugs(cannot interact outside GUI)
        else if (!(gui_window.window.flags & GUI_WND_UNFOCUSED))
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

// i is the dimention of the size/pos vector
// Can also be used to resize towards the bottom of the screen, i is for that
static void
resize_right(int i, int max_r)
{
  int mouse_delta = mouse_pos[i] - (gui_window.window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.size[i] = gui_window.window.size_0[i] + mouse_delta;
  gui_window.size[i] = min(max(gui_window.size[i], gui_window.min_size[i]), max_r + 1 - gui_window.pos[i]);
}
// i is the dimention of the size/pos vector
// Can also be used to resize to the top of the screen
static void
resize_left(int i, int min_l)
{
  int mouse_delta = mouse_pos[i] - (gui_window.window.mouse_rel[i] + gui_window.pos[i]);

  gui_window.pos[i] += mouse_delta;

  if (gui_window.pos[i] < min_l)
  {
    mouse_delta -= gui_window.pos[i];
    gui_window.pos[i] = 0;
  }

  gui_window.size[i] = gui_window.window.size_0[i] - mouse_delta;

  if (gui_window.size[i] < gui_window.min_size[i])
  {
    gui_window.pos[i] -= gui_window.min_size[i] - gui_window.size[i];
    gui_window.size[i] = gui_window.min_size[i];
  }

  // So that in the next call to resize_left(), the size[i] does not return to be size_0(old). Since the X keeps updating, we need to shift the width with it, so that mouse_delta keeps being current. I know this is not really a good explanation, but just run this on a fucking paper, just shift the window do all the operations without this line, you will understand what I mean.
  gui_window.window.size_0[i] = gui_window.size[i];
}

// How much text would fit per line, given a width of a rectangle
static inline int
text_per_line(int width)
{
  return width / font_w;
}
// How many lines fit in a rectangle that has a height
static inline int
text_lines_n(int height)
{
  return height / font->height;
}
// How much text would fit in a rectangle with width and height
static inline int
text_in_rect(int width, int height)
{
  return text_per_line(width) * text_lines_n(height);
}

// Safe to pass NULL as string
static int
draw_str(const char* str, int l, int t, int r, int b)
{
  if (str == NULL)
  {
    return 0;
  }

  int i = 0;
  for (int y = t+1; y+font->height <= b; y+=font->height)
  {
    int nl = 0;
    for (int x = l+1; x+font_w < r && !nl; i++, x+=font_w)
    {
      if (!str[i])
      {
        return i;
      }
      else if (str[i] == '\n')
      {
        nl = 1;
        continue;
      }
      gui_draw_font(font, x, y, str[i], get_shade(4));
    }
  }
  return i;
}

void
gui_draw_window(int l, int t, int r, int b)
{
  if (gui_window.window.flags & GUI_WND_HIDE)
  {
    return;
  }

  // HANDLE WINDOW LOGIC!

  // Move the window if necessary and other logic to keep track of movement
  if (gui_window.window.flags & GUI_WND_RELOCATING)
  {
    gui_event_t e;
    gui_window.pos[0] = e.relocate.delta[0] = mouse_pos[0]-gui_window.window.mouse_rel[0];
    gui_window.pos[1] = e.relocate.delta[1] = mouse_pos[1]-gui_window.window.mouse_rel[1];

    gui_window.pos[0] = e.relocate.normalized[0] = min(max(gui_window.pos[0], l), r+1-gui_window.size[0]);
    gui_window.pos[1] = e.relocate.normalized[1] = min(max(gui_window.pos[1], l), b+1-gui_window.size[1]);
    send_event(&e);
  }
  // Resize the window and keep track of resizing too
  else if (gui_window.window.flags & GUI_WND_RESIZING)
  {
    int flag = gui_window.window.flags & GUI_WND_RESIZING;

    if (flag & GUI_WND_RESIZING_R)
    {
      resize_right(0, r);
    }
    else if (flag & GUI_WND_RESIZING_L)
    {
      resize_left(0, l);
    }

    if (flag & GUI_WND_RESIZING_B)
    {
      resize_right(1, b);
    }
    else if (flag & GUI_WND_RESIZING_T)
    {
      resize_left(1, t);
    }
  }


  // Draw the window decorations and stuff
  draw_filled_rect(BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM, get_shade(3), get_shade(1), get_shade(2));

  // draw_filled_rect(CONTENT_LEFT, CONTENT_TOP, CONTENT_RIGHT, CONTENT_BOTTOM, get_shade(1), get_shade(1), get_shade(2));

  //draw_filled_rect(TITLE_LEFT, TITLE_TOP, TITLE_RIGHT, TITLE_BOTTOM, get_shade(3), get_shade(1), get_shade(2));

  // Seperate x button from the rest of the title
  draw_yline(TITLE_TOP, TITLE_BOTTOM-1, X_LEFT, get_shade(1));

  // X button text
  int xx=X_LEFT + X_WIDTH/2 - 3, xy=TITLE_TOP+1;
  gui_draw_font(font, xx, xy, '\\', get_shade(0));
  gui_draw_font(font, xx, xy,  '/', get_shade(0));

  // Window title text
  draw_str(gui_window.str, TITLE_LEFT, TITLE_TOP, X_LEFT, TITLE_BOTTOM);

  // Three dots on the corner
  //gui_draw_font(font, BORDER_RIGHT-6, BORDER_BOTTOM-font->height-2, '.', get_shade(3));
  //gui_draw_font(font, BORDER_RIGHT-6, BORDER_BOTTOM-font->height+2, '.', get_shade(3));
  //gui_draw_font(font, BORDER_RIGHT-10, BORDER_BOTTOM-font->height+2, '.', get_shade(3));

  // Drawing the things and shit
  static gui_thing_t th = {0};
  th.str = "Ouah";
  th.type = GUI_T_TICKBOX;
  th.button.pressed = 0;
  gui_draw(&th, CONTENT_LEFT,CONTENT_TOP, CONTENT_RIGHT,CONTENT_TOP+font->height+1);
}


void
gui_draw(gui_thing_t* t, int left, int top, int right, int bottom)
{
  int yes_text = 1;
  int center_y = top + (bottom-top)/2;
  // int center_x = left + (right-left)/2;

  #ifdef DEBUG
    // draw_rect(left, top, right, bottom, gui_shades[4],gui_shades[4]);
  #endif

  switch(t->type)
  {
    case GUI_T_WINDOW:
    gui_draw_window(left, top, right, bottom);
    yes_text = 0;
    break;

    case GUI_T_MAP:

    break;

    case GUI_T_OTEXT:
    break;

    case GUI_T_TICKBOX:
    {
      int tick = t->tickbox.ticked;
      int y0 = max(center_y - TICKBOX_SIZE/2, top);
      int y1 = min(center_y + TICKBOX_SIZE/2, bottom);
      draw_filled_rect(right-font_w, y0, right, y1, get_shade(1), get_shade(3), tick?get_shade(4):get_shade(0));

      right -= TICKBOX_SIZE-1;
      break;
    }

    case GUI_T_ITEXT:
    draw_filled_rect(left, top, right, bottom, get_shade(0), get_shade(3), get_shade(1));
    break;

    case GUI_T_BUTTON:
    {
      int p = t->button.pressed;
      // Doing math with p to just reverse if pressed or not.
      draw_filled_rect(left, top, right, bottom, get_shade(p?1:3), get_shade(p?3:1), get_shade(2));
      break;
    }
  }

  if (yes_text)
  {
    draw_str(t->str, left, top, right, bottom);
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
    y = max(y, 0); // Just limit it
    y = ITOFIP(y);

    fip_t absi_ypx = ypx < 0 ? -ypx : ypx; // An absolute value

    fip_t sign = absi_ypx == ypx ? 1 : -1; // What value we increment in the y for loop

    for (int x = max(left, 0); x < min(right, vid_size[0]); x++)
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
