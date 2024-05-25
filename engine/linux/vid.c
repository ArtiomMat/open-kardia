#include "../vid.h"
#include "../com.h"
#include "../fip.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <X11/Xlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>

// Writable frame buffer, essentially blitting:
// https://bbs.archlinux.org/viewtopic.php?id=225741

// From older project
static const char xkeymap[256] = {
	0,0,0,0,0,0,0,0,0, // First 9 are for some reason not mapped?
	KEY_ESCAPE,'1','2','3','4','5','6','7','8','9','0','-','=',KEY_BS,
	KEY_TAB,'q','w','e','r','t','y','u','i','o','p','[',']',KEY_ENTER,
	KEY_CTRL,'a','s','d','f','g','h','j','k','l',';','\'','`',KEY_LSHIFT,
	'\\','z','x','c','v','b','n','m',',','.','/',KEY_RSHIFT,
	// NOTE: *, + etc are from the numpad.
	'*',KEY_LALT,KEY_SPACE,KEY_CAPSLOCK,
	KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10, // KF11=95 for some reason.
	KEY_NUMLOCK, KEY_SCROLLLOCK,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,
  KEY_UP, 0, KEY_LEFT, KEY_RIGHT, 0, KEY_DOWN,
};

// bit array of all the key states that were pressed.
// bit that is 1 means the key has been aleady pressed, bit that is 0 means the key is released.
// This array is to avoid sending duplicate signals about a key being pressed
// static const char xkeys[32];

static const char xbuttonmap[4] = {-1, KEY_LMOUSE, KEY_MMOUSE,KEY_RMOUSE};

Display* vid_nix_dsp = 0;
int vid_nix_scr;
Visual* vid_nix_visual;
Atom vid_nix_wmdeletewnd_atom;
GC vid_nix_gc;
Window vid_nix_window;
XImage* vid_nix_image;

static Window root_window;

static Colormap colormap;

static Cursor cursors[_VID_CUR_N];

// TODO: It's not exactly that safe
void
vid_free()
{
  free(vid_colors);

  XDestroyWindow(vid_nix_dsp, vid_nix_window);

  if (vid_nix_image)
  {
    XDestroyImage(vid_nix_image);
  }
  vid_nix_image = 0;

  for (int i = 0; i < _VID_CUR_N; i++)
  {
    XFreeCursor(vid_nix_dsp, cursors[i]);
  }

  XCloseDisplay(vid_nix_dsp);


  puts("vid_free(): Video module freed.");
}

static int
x_error_handler(Display * d, XErrorEvent * e)
{
  static char text[512];
  fprintf(stderr, "x_error_handler(): %hhu\n", e->error_code);
  XGetErrorText(d, e->error_code, text, 512);
  puts(text);
  return 0;
}

int
vid_init(int _vid_w, int _vid_h)
{
  vid_size[0] = _vid_w;
  vid_size[1] = _vid_h;

  XSetErrorHandler(x_error_handler);

  vid_nix_dsp = XOpenDisplay(NULL);
  if (!vid_nix_dsp)
  {
    fputs("vid_init(): Can't XOpenDisplay(NULL)!", stderr);
    return 0;
  }

  vid_nix_scr = DefaultScreen(vid_nix_dsp);

  // Custom WM_DELETE_WINDOW protocol
  vid_nix_wmdeletewnd_atom = XInternAtom(vid_nix_dsp, "WM_DELETE_WINDOW", False);

  // Visuals and stuff for drawing.
  XVisualInfo xvisualinfo;
  if (!XMatchVisualInfo(vid_nix_dsp, vid_nix_scr, 24, TrueColor, &xvisualinfo))
  {
    fputs("vid_init(): Failed to XMatchVisualInfo(). Are you sure you have 24 bit depth TrueColor?", stderr);
    vid_free();
    return 0;
  }

  vid_nix_visual = xvisualinfo.visual;

  XSetWindowAttributes attribs = {0};
  attribs.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | StructureNotifyMask;

  vid_nix_window = XCreateWindow(
    vid_nix_dsp,
    RootWindow(vid_nix_dsp, vid_nix_scr),
    0, 0, vid_size[0], vid_size[1],
    0, // border width
    24, // bit depth
    InputOutput,
    vid_nix_visual,
    CWBorderPixel | CWEventMask,
    &attribs
  );

  // Cursors
  cursors[VID_CUR_POINTER] = XCreateFontCursor(vid_nix_dsp, XC_left_ptr);
  cursors[VID_CUR_SELECT] = XCreateFontCursor(vid_nix_dsp, XC_hand2);
  cursors[VID_CUR_TEXT] = XCreateFontCursor(vid_nix_dsp, XC_xterm);
  cursors[VID_CUR_WAIT] = XCreateFontCursor(vid_nix_dsp, XC_watch);

  // XDefineCursor(vid_nix_dsp, vid_nix_window, createnullcursor(vid_nix_dsp, vid_nix_window));

  // GC Creation, before mapping.
  XGCValues xgcvalues;
  xgcvalues.graphics_exposures = False;
  vid_nix_gc = XCreateGC(vid_nix_dsp, vid_nix_window, GCGraphicsExposures, &xgcvalues );

  XSelectInput(vid_nix_dsp, vid_nix_window, attribs.event_mask);

  // Set our custom WM_DELETE_WINDOW protocol
  XSetWMProtocols(vid_nix_dsp, vid_nix_window, &vid_nix_wmdeletewnd_atom, 1);

  XMapWindow(vid_nix_dsp, vid_nix_window);
  XFlush(vid_nix_dsp); // Wait until window is visible.

  // Image creation
  char* data = malloc(vid_size[0]*32/8 * vid_size[1]); // 32 because of padding >:(
  if (!data)
  {
    fputs("vid_init(): Failed to allocate image data.", stderr);
    vid_free();
    return 0;
  }

  vid_nix_image = XCreateImage(
    vid_nix_dsp,
    vid_nix_visual,
    24,
    ZPixmap,
    0,
    data,
    vid_size[0], vid_size[1],
    32, // bitmap_pad, no clue, 32 is "if unsure".
    vid_size[0]*32/8
  ); // bytes per scanline, since we have padding of 32, we use 32 instead of 24.

  if (!vid_nix_image)
  {
    fputs("vid_init(): Failed to create image.", stderr);
    vid_free();
    return 0;
  }

  vid_pixels = (unsigned char*) vid_nix_image->data;

  // Limit window size
  XSizeHints size_hints;
  size_hints.flags = PMinSize | PMaxSize;
  size_hints.min_width = vid_size[0];
  size_hints.max_width = vid_size[0];
  size_hints.min_height = vid_size[1];
  size_hints.max_height = vid_size[1];
  XSetWMNormalHints(vid_nix_dsp, vid_nix_window, &size_hints);

  vid_colors = calloc(256, sizeof (*vid_colors));

  root_window = XRootWindow(vid_nix_dsp, vid_nix_scr);
  colormap = XDefaultColormap(vid_nix_dsp, XDefaultScreen (vid_nix_dsp));

  XRRScreenConfiguration* screen_info = XRRGetScreenInfo(vid_nix_dsp, vid_nix_window);
  if (screen_info == NULL)
  {
    fputs("vid_init(): Could not get screen info, extra screen information not available.", stderr);
  }

//  int sizes_n;
  int rates_n;
//  XRRScreenSize *screen_sizes = XRRConfigSizes(screen_info, &sizes_n);

  short* rates = XRRConfigRates(screen_info, 0, &rates_n);
  if (screen_info == NULL)
  {
    fprintf(stderr, "vid_init(): Could not find refresh rate, so defaulting to %hdHz!", rates[0] = 30);
  }

  printf("vid_init(): Video module initialized, Screen refresh rate %hdHz.\n", rates[0]);
  return rates[0];
}

void
vid_screen_size(int* w, int* h)
{
  XWindowAttributes ra;
  XGetWindowAttributes(vid_nix_dsp, DefaultRootWindow(vid_nix_dsp), &ra);
  *w = ra.width;
  *h = ra.height;
}

void
vid_set_title(const char* title)
{
  XStoreName(vid_nix_dsp, vid_nix_window, title);
}

void
vid_run()
{
  XEvent xe;
  vid_event_t e = {.type = _VID_E_NULL};

  while (XPending(vid_nix_dsp))
  {
    XNextEvent(vid_nix_dsp, &xe);

    switch (xe.type)
    {
      case ClientMessage:
      if (!(
        xe.xclient.message_type == XInternAtom(vid_nix_dsp, "WM_PROTOCOLS", True) &&
        (Atom)xe.xclient.data.l[0] == vid_nix_wmdeletewnd_atom
      ))
        break;
      // Otherwise proceed here(most likely it wont, we overrode it, but just in case):
      case DestroyNotify:
      e.type = VID_E_CLOSE;
      break;

      case KeyPress:
      e.type = VID_E_PRESS;
      e.press.code = xkeymap[xe.xkey.keycode];
      break;

      case KeyRelease:
      e.type = VID_E_RELEASE;
      e.release.code = xkeymap[xe.xkey.keycode];
      break;

      case ButtonPress:
      e.type = VID_E_PRESS;
      e.press.code = xbuttonmap[xe.xbutton.button];
      break;

      case ButtonRelease:
      e.type = VID_E_RELEASE;
      e.release.code = xbuttonmap[xe.xbutton.button];
      break;

      case UnmapNotify:
      e.type = VID_E_HIDE;
      break;

      case Expose:
      e.type = VID_E_SHOW;
      break;

      case MotionNotify:
      e.type = VID_E_MOVE;
      e.move.x = vid_cursor[0] = xe.xmotion.x;
      e.move.y = vid_cursor[1] = xe.xmotion.y;
      break;
    }

    if (e.type != _VID_E_NULL)
    {
      if (vid_on != NULL && !vid_on(&e)) // If it wasn't handled
      {
        vid_def_on(&e);
      }
    }
  }
}

void
vid_refresh()
{
  XPutImage
  (
    vid_nix_dsp,
    vid_nix_window,
    vid_nix_gc,
    vid_nix_image,
    0, 0,
    0, 0,
    vid_size[0], vid_size[1]
  );
}

void
vid_wipe(int color)
{
  // int x, y;
  // Window child;
  // XWindowAttributes xwa;
  // XTranslateCoordinates( vid_nix_dsp, vid_nix_window, root_window, 0, 0, &x, &y, &child );
  // printf("%d,%d\n", x, y);
  // XGetWindowAttributes(vid_nix_dsp, vid_nix_window, &xwa);


  // XImage* scr_image = XGetImage(vid_nix_dsp, root_window, x, y, vid_size[0], vid_size[1], AllPlanes, XYPixmap);

  for (int i = 0; i < vid_size[1]*vid_size[0]; i++)
  {
    // if (x >= vid_size[0])
    // {
    //   x = 0;
    //   y++;
    // }

    vid_pixels[i*4+3] = color; // We use the padding as the index, I am a fucking genius
    vid_pixels[i*4+2] = vid_colors[color][0];
    vid_pixels[i*4+1] = vid_colors[color][1];
    vid_pixels[i*4+0] = vid_colors[color][2];

    // unsigned long pixel = XGetPixel(scr_image, x, y);
    // vid_pixels[i*4+2] = (pixel >> 16) & 0xFF; // Red component
    // vid_pixels[i*4+1] = (pixel >> 8) & 0xFF;  // Green component
    // vid_pixels[i*4+0] = pixel & 0xFF;         // Blue component
  }

  // XFree (scr_image);
}

static inline void
_set(unsigned char color, int i)
{
  vid_pixels[i*4+3] = color; // We use the padding as the index, I am a fucking genius
  vid_pixels[i*4+2] = vid_colors[color][0];
  vid_pixels[i*4+1] = vid_colors[color][1];
  vid_pixels[i*4+0] = vid_colors[color][2];
}

void
vid_put(unsigned char color, int i)
{
  _set(color, i);
}

void
vid_put_xline(unsigned char color, int xi, int xf, int y)
{
  int right = xi > xf ? xi : xf;
  int left = right == xi? xf : xi;

  for (int x = max(left, 0); x <= min(right, vid_size[0]-1); x++)
  {
    _set(color, y*vid_size[0] + x);
  }
}

void
vid_put_yline(unsigned char color, int yi, int yf, int x)
{
  int top = yi > yf ? yi : yf;
  int bottom = top == yi? yf : yi;

  for (int y = max(bottom, 0); y <= min(top, vid_size[1]-1); y++)
  {
    _set(color, y*vid_size[0] + x);
  }
}

void
vid_put_rect(unsigned char fill, int left, int top, int right, int bottom)
{
  for (int _x = left; _x <= right; _x++)
  {
    for (int _y = top; _y <= bottom; _y++)
    {
      vid_put(fill, _y*vid_size[0] + _x);
    }
  }
}

// Reutns slope
static fip_t
setup_line_params(int* xi, int* yi, int* xf, int* yf, fip_t* y)
{
  if (*xi > *xf)
  {
    int tmp = *xi;
    *xi = *xf;
    *xf = tmp;

    tmp = *yi;
    *yi = *yf;
    *yf = tmp;
  }

  fip_t m = FIP_DIV(16, ITOFIP(16, *yf-*yi), ITOFIP(16, *xf-*xi));

  *y = ITOFIP(16, *yi);

  return m;
}

void
vid_put_line(unsigned char color, int xi, int yi, int xf, int yf)
{
  if (yi == yf)
  {
    vid_put_xline(color, xi, xf, yi);
    return;
  }
  if (xi == xf)
  {
    vid_put_yline(color, yi, yf, xi);
    return;
  }

  // XI must be less that xf
  if (abs(xf-xi) > abs(yf-yi))
  {
    fip_t y;
    fip_t m = setup_line_params(&xi, &yi, &xf, &yf, &y);

    for (int x = xi; x <= xf; x++, y += m)
    {
      _set(color, x + FIPTOI(16, y) * vid_size[0]);
    }
  }
  else
  {
    fip_t x;
    fip_t m = setup_line_params(&yi, &xi, &yf, &xf, &x);

    for (int y = yi; y <= yf; y++, x += m)
    {
      _set(color, FIPTOI(16, x) + y * vid_size[0]);
    }
  }
}

unsigned char
vid_get(int i)
{
  return vid_pixels[i*4+3];
}

void
vid_realize_colors()
{
}

void
vid_set_cursor_type(int t)
{
  XDefineCursor(vid_nix_dsp, vid_nix_window, cursors[t]);
}
