#include "gui_local.h"
#include "com.h"
#include "fip.h"

#include <stdio.h>
#include <stdlib.h>

int gui_title_h = 0;
unsigned char gui_shades[GUI_SHADES_N] = {0,1,2,3,4};
int (*gui_on)(gui_event_t* event) = NULL;

static gui_font_t* font;
int font_w;

// The currently focused thing, for instance a button, eg if enter is pressed we press it.
// static gui_thing_t* focused_thing = NULL;

gui_thing_t gui_window = {0};
gui_thing_t* things = NULL;

gui_thing_t** gui_thing_refs;

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

  printf("gui_init(): GUI module initialized.\n");
}

void
gui_free(gui_thing_t* t)
{
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
    if (t->next != NULL)
    {
      t->next->prev = t->prev;
    }
    if (t->prev != NULL)
    {
      t->prev->next = t->next;
    }
    free(t);
  }
}

////////////////////////////////////////////
//             MISC DRAW
////////////////////////////////////////////

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

////////////////////////////////////////////
//                LINES
////////////////////////////////////////////

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


////////////////////////////////////////////
//                RECTS
////////////////////////////////////////////

// Extends draw_rect and also fills the rectangle.
// If xray is enabled it avoids filling the rectangle there.
static inline void
draw_ref_rect(gui_thing_t* t, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
  for (int _x = left; _x <= right; _x++)
  {
    for (int _y = top; _y <= bottom; _y++)
    {
      gui_thing_refs[ _y*vid_size[0] + _x] = t;
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

////////////////////////////////////////////
//                TEXT
////////////////////////////////////////////

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

////////////////////////////////////////////
//            DRAW THINGS
////////////////////////////////////////////

// A window is not bound to a rectangle
void
gui_draw_window(gui_thing_t* t)
{
  draw_ref_rect(t, BORDER_LEFT((*t)), BORDER_TOP((*t)), BORDER_RIGHT((*t)), BORDER_BOTTOM((*t)));

  // Draw the window decorations and stuff
  draw_filled_rect(BORDER_LEFT((*t)), BORDER_TOP((*t)), BORDER_RIGHT((*t)), BORDER_BOTTOM((*t)), get_shade(3), get_shade(1), get_shade(2));

  // draw_filled_rect(CONTENT_LEFT((*t)), CONTENT_TOP((*t)), CONTENT_RIGHT((*t)), CONTENT_BOTTOM((*t)), get_shade(1), get_shade(1), get_shade(2));

  //draw_filled_rect(TITLE_LEFT((*t)), TITLE_TOP((*t)), TITLE_RIGHT((*t)), TITLE_BOTTOM((*t)), get_shade(3), get_shade(1), get_shade(2));

  // Seperate x button from the rest of the title
  draw_yline(TITLE_TOP((*t)), TITLE_BOTTOM((*t))-1, X_LEFT((*t)), get_shade(1));

  // X button text
  gui_u_t xx=X_LEFT((*t)) + X_WIDTH/2 - 3, xy=TITLE_TOP((*t))+1;
  gui_draw_font(font, xx, xy, '\\', get_shade(0));
  gui_draw_font(font, xx, xy,  '/', get_shade(0));

  // Window title text
  draw_text(get_shade(4), t->text, TITLE_LEFT((*t)), TITLE_TOP((*t)), X_LEFT((*t)), TITLE_BOTTOM((*t)));

  // Three dots on the corner
  //gui_draw_font(font, BORDER_RIGHT((*t))-6, BORDER_BOTTOM((*t))-font->height-2, '.', get_shade(3));
  //gui_draw_font(font, BORDER_RIGHT((*t))-6, BORDER_BOTTOM((*t))-font->height+2, '.', get_shade(3));
  //gui_draw_font(font, BORDER_RIGHT((*t))-10, BORDER_BOTTOM((*t))-font->height+2, '.', get_shade(3));

  // Drawing the things and shit
  static gui_thing_t th = {0};
  th.text = "Ouah";
  th.type = GUI_T_TICKBOX;
  th.button.pressed = 0;
  gui_draw(&th, CONTENT_LEFT((*t)),CONTENT_TOP((*t)), CONTENT_RIGHT((*t)),CONTENT_TOP((*t))+font->height+1);
}

void
gui_draw(gui_thing_t* t, gui_u_t left, gui_u_t top, gui_u_t right, gui_u_t bottom)
{
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
      gui_draw_window(t);
      yes_text = 0;
    }
    break;

    case GUI_T_ROWMAP:
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
    }
    break;

    case GUI_T_ITEXT:
    {
      draw_filled_rect(left, top, right, bottom, get_shade(0), get_shade(3), get_shade(1));
    }
    break;

    case GUI_T_BUTTON:
    {
      int p = t->button.pressed;
      // Doing math with p to just reverse if pressed or not.
      draw_filled_rect(left, top, right, bottom, get_shade(p?1:3), get_shade(p?3:1), get_shade(2));
    }
    break;
  }

  if (yes_text)
  {
    draw_text(get_shade(4), t->text, left, top, right, bottom);
  }
}
