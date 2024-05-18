#include "gui_local.h"
#include "com.h"
#include "fip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int gui_title_h = 0;
unsigned char gui_shades[GUI_SHADES_N] = {0,1,2,3,4};
int (*gui_on)(gui_event_t* event) = NULL;

static gui_font_t* font;
int font_w;

// The currently focused thing, for instance a button, eg if enter is pressed we press it.
// static gui_thing_t* focused_thing = NULL;

gui_thing_t* gui_things = NULL;
static gui_thing_t* last_thing = NULL;

gui_thing_t** gui_thing_refs;

int gui_mouse_pos[2];

static void
send_event(gui_event_t* e)
{
  if (gui_on != NULL && !gui_on(e))
  {
    puts("FUCK!");
  }
}

void
gui_set_flag(gui_thing_t* t, int flag, int yes)
{
  if (yes)
  {
    t->flags |= flag;
  }
  else
  {
    t->flags &= ~flag;
  }
}

void
gui_toggle_flag(gui_thing_t* t, int flag)
{
  t->flags ^= flag;
}

void
gui_init(gui_font_t* _font)
{
  font = _font;
  font_w = gui_get_font_width(font);

  gui_thing_refs = calloc( vid_size[0] * vid_size[1], sizeof (gui_thing_t*));

  gui_title_h = font->height + 2;

  printf("gui_init(): GUI module initialized.\n");
}

void
gui_free(gui_thing_t* t)
{
  puts("gui_free(): GUI module freed.");
  if (t == NULL)
  {
    for (gui_thing_t* _t = t; _t != NULL;)
    {
      gui_thing_t* next = _t->next;
      free(t);
      _t = next;
    }

    free(gui_thing_refs);
  }
  else
  {
    switch (t->type)
    {
      case GUI_T_WINDOW:
      gui_free(t->window.child);
      break;

      case GUI_T_ROWMAP:
      {
        int i = 0;
        for (int r = 0; r < t->rowmap.rows_n; r++)
        {
          for (int c = 0; c < t->rowmap.cols_n[r]; c++, i++)
          {
            gui_free(t->rowmap.things[i]);
          }
        }
      }
      break;
    }

    if (t->next != NULL)
    {
      t->next->prev = t->prev;
    }

    if (t->prev != NULL)
    {
      t->prev->next = t->next;
    }
    
    // Free and return here if it's the last thing
    if (t->next == NULL && t->prev == NULL)
    {
      last_thing = gui_things = NULL;
    }

    free(t);
  }
}

static gui_thing_t*
get_pointed_thing()
{
  if (gui_mouse_pos[0] < 0 || gui_mouse_pos[1] < 0 || gui_mouse_pos[0] >= vid_size[0] ||  gui_mouse_pos[1] >= vid_size[1])
  {
    return NULL;
  }
  return gui_thing_refs[gui_mouse_pos[0] + gui_mouse_pos[1] * vid_size[0]];
}

//////////////////////////////////////////////////////////////////////////////////
//                                  WINDOW ON
//////////////////////////////////////////////////////////////////////////////////

static void
save_mouse_rel(gui_thing_t* gui_window)
{
  gui_u_t mouse_x = min(max(gui_mouse_pos[0], 0), vid_size[0]-1);
  gui_u_t mouse_y = min(max(gui_mouse_pos[1], 0), vid_size[1]-1);

  gui_window->window.mouse_rel[0] = mouse_x - gui_window->pos[0];
  gui_window->window.mouse_rel[1] = mouse_y - gui_window->pos[1];
}

// i is the dimention of the size/pos vector
// Can also be used to resize towards the bottom of the screen, i is for that
static void
resize_right(gui_thing_t* gui_window, int i, gui_u_t max_r)
{
  int mouse_delta = gui_mouse_pos[i] - (gui_window->window.mouse_rel[i] + gui_window->pos[i]);

  gui_window->size[i] = gui_window->window.size_0[i] + mouse_delta;
  gui_window->size[i] = min(max(gui_window->size[i], gui_window->min_size[i]), max_r + 1 - gui_window->pos[i]);
}
// i is the dimention of the size/pos vector
// Can also be used to resize to the top of the screen
static void
resize_left(gui_thing_t* gui_window, int i, gui_u_t min_l)
{
  int mouse_delta = gui_mouse_pos[i] - (gui_window->window.mouse_rel[i] + gui_window->pos[i]);

  gui_window->pos[i] += mouse_delta;

  if (gui_window->pos[i] < min_l)
  {
    mouse_delta -= gui_window->pos[i];
    gui_window->pos[i] = 0;
  }

  gui_window->size[i] = gui_window->window.size_0[i] - mouse_delta;

  if (gui_window->size[i] < gui_window->min_size[i])
  {
    gui_window->pos[i] -= gui_window->min_size[i] - gui_window->size[i];
    gui_window->size[i] = gui_window->min_size[i];
  }

  // So that in the next call to resize_left(), the size[i] does not return to be size_0(old). Since the X keeps updating, we need to shift the width with it, so that mouse_delta keeps being current. I know this is not really a good explanation, but just run this on a fucking paper, just shift the window do all the operations without this line, you will understand what I mean.
  gui_window->window.size_0[i] = gui_window->size[i];
}

/**
 * Test if X_TEST and Y_TEST are within a rectangle.
*/
static inline int
in_rect(gui_u_t x_test, gui_u_t y_test, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  return x_test <= right && x_test >= left && y_test <= bottom && y_test >= top;
}

static void
window_on_move(gui_thing_t* window, gui_event_t* gui_e)
{
  gui_u_t
    l = 0, t = 0,
    r = vid_size[0]-1, b = vid_size[1]-1;
  // Move the window if necessary and other logic to keep track of movement
  if (window->window.flags & GUI_WND_RELOCATING)
  {
    gui_event_t e;
    window->pos[0] = e.relocate.delta[0] = gui_mouse_pos[0]-window->window.mouse_rel[0];
    window->pos[1] = e.relocate.delta[1] = gui_mouse_pos[1]-window->window.mouse_rel[1];

    window->pos[0] = e.relocate.normalized[0] = min(max(window->pos[0], l), r+1-window->size[0]);
    window->pos[1] = e.relocate.normalized[1] = min(max(window->pos[1], l), b+1-window->size[1]);
    send_event(&e);
  }
  // Resize the window and keep track of resizing too
  else if (window->window.flags & GUI_WND_RESIZING)
  {
    int flag = window->window.flags & GUI_WND_RESIZING;

    if (flag & GUI_WND_RESIZING_R)
    {
      resize_right(window, 0, r);
    }
    else if (flag & GUI_WND_RESIZING_L)
    {
      resize_left(window, 0, l);
    }

    if (flag & GUI_WND_RESIZING_B)
    {
      resize_right(window, 1, b);
    }
    else if (flag & GUI_WND_RESIZING_T)
    {
      resize_left(window, 1, t);
    }
  }
}

// gui_e is a pointer to the gui event that would be sent, if its ->type is untouched nothing will be sent.
static void
window_on_mpress(gui_thing_t* thing, gui_event_t* gui_e)
{
  // window_on_move(thing, gui_e);
  // Inside of the title bar
  if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], TITLE_LEFT((*thing)), TITLE_TOP((*thing)), TITLE_RIGHT((*thing)), TITLE_BOTTOM((*thing))))
  {
    if (gui_mouse_pos[0] >= X_LEFT((*thing)) && gui_mouse_pos[1] <= X_BOTTOM((*thing)))
    {
      gui_free(thing);

      gui_e->type = GUI_E_HIDE;
      return;
    }

    thing->window.flags |= GUI_WND_RELOCATING;
    save_mouse_rel(thing);

    // We need to automatically focus back the window regardless, we don't reach this is middle clicked
    thing->window.flags &= ~GUI_WND_UNFOCUSED;

    thing->window.flags &= ~GUI_WND_UNFOCUSED;
    gui_e->type = GUI_E_UNFOCUS;
    return;
  }
  // Inside of the content zone
  else if (!(thing->window.flags & GUI_WND_XRAY) && in_rect(gui_mouse_pos[0], gui_mouse_pos[1], CONTENT_LEFT((*thing)), CONTENT_TOP((*thing)), CONTENT_RIGHT((*thing)), CONTENT_BOTTOM((*thing))))
  {
    gui_e->type = GUI_E_PRESS;
    return;
  }
  // We are 100% either in border or outside the window alltogether
  else
  {
    int flags_tmp = thing->window.flags; // Save the flag for future

    // Right and left border
    if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], CONTENT_RIGHT((*thing))+1 - GUI_RESIZE_BLEED, BORDER_TOP((*thing)), BORDER_RIGHT((*thing)), BORDER_BOTTOM((*thing))))
    {
      thing->window.flags |= GUI_WND_RESIZING_R;
    }
    else if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*thing)), BORDER_TOP((*thing)), CONTENT_LEFT((*thing))-1 + GUI_RESIZE_BLEED, BORDER_BOTTOM((*thing))))
    {
      thing->window.flags |= GUI_WND_RESIZING_L;
    }

    // Top and bottom border, disconnected to combine the two in the corners
    if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*thing)), BORDER_TOP((*thing)), BORDER_RIGHT((*thing)), TITLE_TOP((*thing))-1 + GUI_RESIZE_BLEED))
    {
      thing->window.flags |= GUI_WND_RESIZING_T;
    }
    else if (in_rect(gui_mouse_pos[0], gui_mouse_pos[1], BORDER_LEFT((*thing)), CONTENT_BOTTOM((*thing))+1 - GUI_RESIZE_BLEED, BORDER_RIGHT((*thing)), BORDER_BOTTOM((*thing))))
    {
      thing->window.flags |= GUI_WND_RESIZING_B;
    }

    // NOTE: Introduces thread unsafety because we do comparison if flags changed assuming they can't outside this scope.
    if (thing->window.flags != flags_tmp)
    {
      thing->window.size_0[0] = thing->size[0];
      thing->window.size_0[1] = thing->size[1];

      save_mouse_rel(thing);
      thing->window.flags &= ~GUI_WND_UNFOCUSED;
      gui_e->type = GUI_E_FOCUS;
      return;
    }
    // 100% outside the window, if not already set we set and put the event for eaten
    // We should only send the event if it's the first time, sending the event otherwise is both unnecessary and causes bugs(cannot interact outside GUI)
    else if (!(thing->window.flags & GUI_WND_UNFOCUSED))
    {
      thing->window.flags |= GUI_WND_UNFOCUSED;
      gui_e->type = GUI_E_UNFOCUS;
      return;
    }
  }
}

static void
window_on_mrelease(gui_thing_t* thing, gui_event_t* gui_e)
{
  thing->window.flags &= ~GUI_WND_RELOCATING;
  thing->window.flags &= ~GUI_WND_RESIZING;
}

//////////////////////////////////////////////////////////////////////////////////
//                                  BUTTON ON
//////////////////////////////////////////////////////////////////////////////////

static void
button_on_mpress(gui_thing_t* t, gui_event_t* gui_e)
{
  t->button.pressed = 1;
  gui_e->type = GUI_E_PRESS;
}

static void
button_on_mrelease(gui_thing_t* t, gui_event_t* gui_e)
{
  t->button.pressed = 0;
  gui_e->type = GUI_E_RELEASE;
}

//////////////////////////////////////////////////////////////////////////////////
//                               ITEXT ON
//////////////////////////////////////////////////////////////////////////////////

static int itext_shift = 0;

static void
itext_on_deselect(gui_thing_t* t, gui_event_t* gui_e)
{
  t->itext.flags &= ~GUI_ITXT_SELECTED;
}

static void
itext_on_mpress(gui_thing_t* t, gui_event_t* gui_e)
{
  itext_shift = 0; // Reset shift incase it was not registered

  if (!(t->itext.flags & GUI_ITXT_NOT_VIRGIN))
  {
    // TODO: Override all text
    t->itext.flags |= GUI_ITXT_NOT_VIRGIN;
  }
  t->itext.flags |= GUI_ITXT_SELECTED;
  gui_e->type = GUI_E_PRESS;
}

static void
itext_on_press(gui_thing_t* t, int code, gui_event_t* gui_e)
{
  if (!(t->itext.flags & GUI_ITXT_SELECTED))
  {
    return;
  }
  
  switch (code)
  {
    case KEY_BS:
    if (t->itext.cursor)
    {
      t->itext.cursor--;
    }
    t->text[t->itext.cursor] = 0;
    break;

    case KEY_LSHIFT:
    case KEY_RSHIFT:
    itext_shift = 1;
    break;

    case KEY_ENTER:
    itext_on_deselect(t, gui_e);
    return;
    break;

    case KEY_RIGHT: // Move only if we can right
    if (t->text[t->itext.cursor])
    {
      t->itext.cursor++;
    }
    break;

    case KEY_LEFT: // Move only if we can left
    if (t->itext.cursor)
    {
      t->itext.cursor--;
    }
    break;

    case KEY_SPACE:
    code = ' ';
    default:
    if (code >= 'a' && code <= 'z')
    {
      code -= itext_shift ? 32 : 0;
    }
    
    t->text[t->itext.cursor] = code;

    t->itext.cursor++;
    if (t->itext.cursor >= t->itext.nmem)
    {
      t->itext.nmem *= 2;
      t->text = realloc(t->text, t->itext.nmem);
      printf("REALLOC %d\n", t->itext.nmem);
    }
    t->text[t->itext.cursor] = 0;

    break;
  }


  gui_e->type = GUI_E_PRESS;
}

static void
itext_on_release(gui_thing_t* selected, int code, gui_event_t* gui_e)
{
  if (!(selected->itext.flags & GUI_ITXT_SELECTED))
  {
    return;
  }

  switch (code)
  {
    case KEY_LSHIFT:
    case KEY_RSHIFT:
    itext_shift = 0;
    break;
  }
  gui_e->type = GUI_E_RELEASE;
}


//////////////////////////////////////////////////////////////////////////////////
//                               TICKBOX ON
//////////////////////////////////////////////////////////////////////////////////

static void
tickbox_on_mpress(gui_thing_t* t, gui_event_t* gui_e)
{
  t->tickbox.ticked = !t->tickbox.ticked;
  gui_e->type = GUI_E_PRESS;
}

//////////////////////////////////////////////////////////////////////////////////
//                                GUI ON PIPE
//////////////////////////////////////////////////////////////////////////////////

static void
thing_on_mrelease(gui_thing_t* released, gui_event_t* gui_e)
{
  switch (released->type)
  {
    case GUI_T_WINDOW:
    window_on_mrelease(released, gui_e);
    break;
    case GUI_T_BUTTON:
    button_on_mrelease(released, gui_e);
    break;
  }
}

static void
thing_on_mpress(gui_thing_t* pressed, gui_event_t* gui_e)
{
  switch (pressed->type)
  {
    case GUI_T_WINDOW:
    window_on_mpress(pressed, gui_e);
    break;
    case GUI_T_TICKBOX:
    tickbox_on_mpress(pressed, gui_e);
    break;
    case GUI_T_BUTTON:
    button_on_mpress(pressed, gui_e);
    break;
    case GUI_T_ITEXT:
    itext_on_mpress(pressed, gui_e);
    break;
  }
}

static void
thing_on_move(gui_thing_t* pressed, gui_event_t* gui_e)
{
  switch (pressed->type)
  {
    case GUI_T_WINDOW:
    window_on_move(pressed, gui_e);
    break;
  }
}

static void
thing_on_deselect(gui_thing_t* selected, gui_event_t* gui_e)
{
  switch (selected->type)
  {
    case GUI_T_ITEXT:
    itext_on_deselect(selected, gui_e);
    break;
  }
}

// Test if it's a mouse or enter press, assumes e->type=VID_E_PRESS/RELEASE
static int
test_mpress(vid_event_t* e)
{
  return e->press.code == KEY_LMOUSE;
}

static void
thing_on_press(gui_thing_t* selected, int code, gui_event_t* gui_e)
{
  switch (selected->type)
  {
    case GUI_T_ITEXT:
    itext_on_press(selected, code, gui_e);
    break;
  }
}

static void
thing_on_release(gui_thing_t* selected, int code, gui_event_t* gui_e)
{
  switch (selected->type)
  {
    case GUI_T_ITEXT:
    itext_on_release(selected, code, gui_e);
    break;
  }
}

int
gui_on_vid(vid_event_t* e)
{
  // pointed that got pressed
  static gui_thing_t* pressed = NULL;
  // Same as pressed, but ignores release, only a of a different thing changes it
  static gui_thing_t* selected = NULL;
  static gui_thing_t* pointed = NULL;

  gui_event_t gui_e = {.type = _GUI_E_NULL};

  switch (e->type)
  {
    case VID_E_MOVE:
    // We also limit it to avoid any possible segfault
    gui_mouse_pos[0] = e->move.x;
    gui_mouse_pos[1] = e->move.y;

    pointed = get_pointed_thing();
    // We should only change the pointed if we are not pressing anything
    if (pressed != NULL)
    {
      thing_on_move(pressed, &gui_e);
    }
    
    break;

    case VID_E_RELEASE:
    if (test_mpress(e))
    {
      if (pressed != NULL)
      {
        thing_on_mrelease(pressed, &gui_e);
        pressed = NULL;
      }
    }
    else if (selected != NULL)
    {
      thing_on_release(selected, e->release.code, &gui_e);
    }
    break;

    case VID_E_PRESS:
    if (test_mpress(e))
    {
      pressed = pointed;

      if (selected != NULL && pressed != selected)
      {
        thing_on_deselect(selected, &gui_e);
      }
      selected = pressed;

      if (pressed != NULL)
      {
        thing_on_mpress(pressed, &gui_e);
      }
    }
    else if (selected != NULL)
    {
      thing_on_press(selected, e->press.code, &gui_e);
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

//////////////////////////////////////////////////////////////////////////////////
//                                     MISC DRAW
//////////////////////////////////////////////////////////////////////////////////

static unsigned char
get_shade(int i)
{
  // Return darker shade if unfocused
  // if (gui_window.window.flags & GUI_WND_UNFOCUSED)
  // {
  //   return gui_shades[max(i - 1, 0)];
  // }
  return gui_shades[i];
}

//////////////////////////////////////////////////////////////////////////////////
//                                    LINES
//////////////////////////////////////////////////////////////////////////////////

static inline void
draw_xline(gui_u_t xi, gui_u_t xf, gui_u_t y, int color)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = max(left, 0); x <= min(right, vid_size[0]-1); x++)
  {
    vid_set(color, y*vid_size[0] + x);
  }
}

static inline void
draw_yline(gui_u_t yi, gui_u_t yf, gui_u_t x, int color)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = max(bottom, 0); y <= min(top, vid_size[1]-1); y++)
  {
    vid_set(color, y*vid_size[0] + x);
  }
}

void
gui_draw_line(gui_u_t xi, gui_u_t yi, gui_u_t xf, gui_u_t yf, unsigned char color)
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

//////////////////////////////////////////////////////////////////////////////////
//                                  RECTS
//////////////////////////////////////////////////////////////////////////////////

// Extends draw_rect and also fills the rectangle.
// If xray is enabled it avoids filling the rectangle there.
static inline void
draw_ref_rect(gui_thing_t* t, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  for (int _x = left; _x <= right; _x++)
  {
    for (int _y = top; _y <= bottom; _y++)
    {
      gui_thing_refs[_y*vid_size[0] + _x] = t;
    }
  }
}

// Draws a rectangle with light being the color on the top and left, dark is right and bottom
static inline void
draw_rect(gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom, int light, int dark)
{
  draw_xline(left, right, top, light);
  draw_xline(left, right, bottom, dark);

  draw_yline(top, bottom, left, light);
  draw_yline(top, bottom, right, dark);
}

// Extends draw_rect and also fills the rectangle.
// If xray is enabled it avoids filling the rectangle there.
static inline void
draw_filled_rect(gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom, int light, int dark, int fill)
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

//////////////////////////////////////////////////////////////////////////////////
//                               TEXT
//////////////////////////////////////////////////////////////////////////////////

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
draw_text(unsigned char color, const char* text, gui_u_t l, gui_u_t t, gui_u_t r, gui_u_t b)
{
  if (text == NULL)
  {
    return 0;
  }

  int i = 0;
  for (int y = t+1; y+font->height <= b; y+=font->height)
  {
    int nl = 0;
    for (int x = l+1; x+font_w < r && !nl; i++, x+=font_w)
    {
      if (!text[i])
      {
        return i;
      }
      else if (text[i] == '\n')
      {
        nl = 1;
        continue;
      }
      gui_draw_font(font, x, y, text[i], color);
    }
  }
  return i;
}

//////////////////////////////////////////////////////////////////////////////////
//                              DRAW THINGS
//////////////////////////////////////////////////////////////////////////////////

// A window is not bound to a rectangle
static void
draw_window(gui_thing_t* t)
{
  draw_ref_rect(t, BORDER_LEFT((*t)), BORDER_TOP((*t)), BORDER_RIGHT((*t)), BORDER_BOTTOM((*t)));

  // Draw the window decorations and stuff
  draw_filled_rect(BORDER_LEFT((*t)), BORDER_TOP((*t)), BORDER_RIGHT((*t)), BORDER_BOTTOM((*t)), get_shade(3), get_shade(1), get_shade(2));

  // draw_filled_rect(CONTENT_LEFT((*t)), CONTENT_TOP((*t)), CONTENT_RIGHT((*t)), CONTENT_BOTTOM((*t)), get_shade(1), get_shade(1), get_shade(2));

  // draw_filled_rect(TITLE_LEFT((*t)), TITLE_TOP((*t)), TITLE_RIGHT((*t)), TITLE_BOTTOM((*t)), get_shade(3), get_shade(1), get_shade(2));

  // Seperate x button from the rest of the title
  // draw_yline(TITLE_TOP((*t)), TITLE_BOTTOM((*t))-1, X_LEFT((*t)), get_shade(1));

  // X button text
  gui_u_t xx=X_LEFT((*t)) + X_WIDTH/2 - 3, xy=TITLE_TOP((*t))+1;
  gui_draw_font(font, xx, xy, '\\', get_shade(0));
  gui_draw_font(font, xx, xy,  '/', get_shade(0));

  // Window title text
  draw_text(get_shade(4), t->text, TITLE_LEFT((*t)), TITLE_TOP((*t)), X_LEFT((*t)), TITLE_BOTTOM((*t)));

  // Three dots on the corner
  // gui_draw_font(font, BORDER_RIGHT((*t))-6, BORDER_BOTTOM((*t))-font->height-2, '.', get_shade(3));
  // gui_draw_font(font, BORDER_RIGHT((*t))-6, BORDER_BOTTOM((*t))-font->height+2, '.', get_shade(3));
  // gui_draw_font(font, BORDER_RIGHT((*t))-10, BORDER_BOTTOM((*t))-font->height+2, '.', get_shade(3));

  // Drawing the child
  // printf("%p %p\n", t, t->window.child);
  gui_draw(t->window.child, CONTENT_LEFT((*t)), CONTENT_TOP((*t)), CONTENT_RIGHT((*t)), CONTENT_BOTTOM((*t)));
}

static void
draw_rowmap(gui_thing_t* t, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  int i = 0;
  int col_h = (bottom-top) / t->rowmap.rows_n;

  for (int rowi = 0; rowi < t->rowmap.rows_n; rowi++)
  {
    int colsn = t->rowmap.cols_n[rowi];
    int col_w = (right-left) / colsn;

    for (int coli = 0; coli < colsn; coli++, i++)
    {
      int _left = left + coli * col_w;
      int _top = top + rowi * col_h;
      gui_draw(t->rowmap.things[i], _left, _top, _left + col_w - 1, _top + col_h - 1);
    }
  }
}

void
gui_draw(gui_thing_t* t, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  if (t == NULL)
  {
    return;
  }
  
  if (t->flags & GUI_T_HIDE)
  {
    return;
  }

  int yes_text = 1;
  gui_u_t center_y = top + (bottom-top)/2;
  gui_u_t center_x = left + (right-left)/2;

  #ifdef DEBUG
    // draw_rect(left, top, right, bottom, gui_shades[4],gui_shades[4]);
  #endif

  switch(t->type)
  {
    case GUI_T_WINDOW:
    {
      draw_window(t);
      yes_text = 0;
    }
    break;

    case GUI_T_ROWMAP:
    {
      draw_rowmap(t, left, top, right, bottom);
      yes_text = 0;
    }
    break;

    case GUI_T_OTEXT:
    break;

    case GUI_T_TICKBOX:
    {
      int tick = t->tickbox.ticked;
      int y0 = max(center_y - TICKBOX_SIZE/2, top);
      int y1 = min(center_y + TICKBOX_SIZE/2, bottom);

      draw_filled_rect(right-TICKBOX_SIZE, y0, right, y1, get_shade(1), get_shade(3), tick?get_shade(4):get_shade(0));
      draw_ref_rect(t, right-TICKBOX_SIZE, y0, right, y1);

      right -= TICKBOX_SIZE-1;
    }
    break;

    case GUI_T_ITEXT:
    {
      int selected = t->itext.flags & GUI_ITXT_SELECTED ? 1 : 0;

      draw_filled_rect(left, top, right, bottom, get_shade(0), get_shade(3), get_shade(1));

      if (selected)
      {
        draw_yline(top+1, top+font->height, left + 1 + t->itext.cursor * font_w, get_shade(3));
      }

      draw_text(get_shade(3 + selected), t->text, left, top, right, bottom);

      draw_ref_rect(t, left, top, right, bottom);
      yes_text = 0;
    }
    break;

    case GUI_T_BUTTON:
    {
      int p = t->button.pressed;
      // Doing math with p to just reverse if pressed or not.
      draw_filled_rect(left, top, right, bottom, get_shade(p?1:3), get_shade(p?3:1), get_shade(2));

      draw_ref_rect(t, left, top, right, bottom);
    }
    break;
  }

  if (yes_text)
  {
    draw_text(get_shade(4), t->text, left, top, right, bottom);
  }
}

//////////////////////////////////////////////////////////////////////////////////
//                                 THING OPEN STUFF
//////////////////////////////////////////////////////////////////////////////////

gui_thing_t*
gui_open(const char* fp)
{
  FILE* f = fopen(fp, "rb");
  uint16_t n;
  uint16_t u16;

  char magic[3];
  fread(magic, 3, 1, f);
  if (magic[0] != 'G' || magic[1] != 'U' || magic[2] != 'I')
  {
    fprintf(stderr, "gui_open(): Invalid GUI file '%s'.\n", fp);
    return NULL;
  }

  fread(&n, 2, 1, f);
  n = com_lil16(n);

  // "OHHHHH BUT THERE IS A BETTER WAY!!!!" Shut up! check the gui_free() code, it's gonna be a pain to implement this differently.
  gui_thing_t* buf[n];
  for (int i = 0; i < n; i++)
  {
    buf[i] = calloc(sizeof (gui_thing_t), 1);
  }

  if (gui_things == NULL)
  {
    gui_things = buf[0];
  }

  for (int i = 0; i < n; last_thing = buf[i++])
  {
    buf[i]->prev = last_thing;
    buf[i]->next = (i < n-1) ? buf[i+1] : NULL;

    fread(&buf[i]->type, 1, 1, f);
    
    uint16_t id_s, str_s;

    fread(&id_s, 2, 1, f);
    id_s = com_lil16(id_s);
    buf[i]->id = malloc(id_s);

    fread(&str_s, 2, 1, f);
    str_s = com_lil16(str_s);
    buf[i]->text = malloc(str_s);

    fread(buf[i]->id, id_s, 1, f);

    fread(buf[i]->text, str_s, 1, f);

    fread(&buf[i]->pos[0], 2, 1, f);
    buf[i]->pos[0] = com_lil16(buf[i]->pos[0]);

    fread(&buf[i]->pos[1], 2, 1, f);
    buf[i]->pos[1] = com_lil16(buf[i]->pos[1]);

    for (int j = 0; j < 2; j++)
    {
      fread(&buf[i]->size[j], 2, 1, f);
      buf[i]->size[j] = com_lil16(buf[i]->size[j]);

      fread(&buf[i]->min_size[j], 2, 1, f);
      buf[i]->min_size[j] = com_lil16(buf[i]->min_size[j]);

      fread(&buf[i]->max_size[j], 2, 1, f);
      buf[i]->max_size[j] = com_lil16(buf[i]->max_size[j]);
    }

    switch(buf[i]->type)
    {
      case GUI_T_ITEXT:
      fread(&buf[i]->itext.format, 1, 1, f);
      
      // Setup the n and nmem
      buf[i]->itext.n = buf[i]->itext.nmem = str_s;
      break;

      case GUI_T_WINDOW:
      fread(&u16, 2, 1, f);
      u16 = com_lil16(u16);
      buf[i]->window.child = buf[u16];
      break;

      case GUI_T_ROWMAP:
      fread(&buf[i]->rowmap.rows_n, 1, 1, f);
      buf[i]->rowmap.cols_n = malloc(sizeof (uint8_t) * buf[i]->rowmap.rows_n);

      int total_cols = 0;
      for (int j = 0; j < buf[i]->rowmap.rows_n; j++)
      {
        fread(&buf[i]->rowmap.cols_n[j], 1, 1, f);
        total_cols += buf[i]->rowmap.cols_n[j];
      }
      buf[i]->rowmap.things = malloc(sizeof (gui_thing_t*) * total_cols);
      
      for (int j = 0; j < total_cols; j++)
      {
        fread(&u16, 2, 1, f);
        u16 = com_lil16(u16);
        buf[i]->rowmap.things[j] = buf[u16];
      }
      break;
    }
  }
  printf( "gui_open(): Loaded %hu things from '%s'.\n", n, fp);

  return buf[0];
}

gui_thing_t*
gui_find(gui_thing_t* from, const char* id, char onetime)
{
  if (from == NULL)
  {
    from = gui_things;
  }

  for (gui_thing_t* _t = from; _t != NULL; _t = _t->next)
  {
    if (!(_t->flags & GUI_T_FOUND) && !strcmp(id, _t->id))
    {
      if (onetime)
      {
        _t->flags |= GUI_T_FOUND;
      }

      return _t;
    }
  }

  return NULL;
}
