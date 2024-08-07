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
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XShm.h> // For fast drawing!

#define BIT_DEPTH 24

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

Display* vid_nix_dsp = NULL;
int vid_nix_scr;
Visual* vid_nix_visual;
Atom vid_nix_wmdeletewnd_atom;
GC vid_nix_gc;
Window vid_nix_window;
XImage* vid_nix_image;

static unsigned char* image_data;

static Window root_window;
static Colormap colormap;

static Cursor cursors[_VID_CUR_N];
static Pixmap null_pixmap;
static char null_data[8] = {0,0,0,0, 0,0,0,0};
static XColor black = {0};

static int xf_queried;
static int xf_event_base, xf_error_base;
Atom xf_buf_atom;

// TODO: It's not exactly that safe
void
vid_free()
{
  free(vid_colors);
  free(vid_px.p);
  vid_px.p = NULL;
  vid_colors = NULL;

  XDestroyWindow(vid_nix_dsp, vid_nix_window);

  if (vid_nix_image)
  {
    XDestroyImage(vid_nix_image);

    // Also means cursors were allocated
    for (int i = 0; i < _VID_CUR_N; i++)
    {
      XFreeCursor(vid_nix_dsp, cursors[i]);
    }
    XFreePixmap(vid_nix_dsp, null_pixmap);
  }
  vid_nix_image = 0;

  XCloseDisplay(vid_nix_dsp);

  puts("vid_free(): Video module freed.");
}

static int
x_error_handler(Display * d, XErrorEvent * e)
{
  static char text[512];
  fprintf(stderr, "x_error_handler(): Fatal error going to exit(): %hhu\n", e->error_code);
  XGetErrorText(d, e->error_code, text, 512);
  puts(text);
  exit(1);
  return 0;
}

int
vid_init(int _vid_w, int _vid_h)
{
  if (!px_init(&vid_px, _vid_w, _vid_h))
  {
    fputs("vid_init(): Failed to allocate px.\n", stderr);
    return 0;
  }

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
  if (!XMatchVisualInfo(vid_nix_dsp, vid_nix_scr, BIT_DEPTH, TrueColor, &xvisualinfo))
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
    0, 0, vid_px.s[0], vid_px.s[1],
    0, // border width
    BIT_DEPTH, // bit depth
    InputOutput,
    vid_nix_visual,
    CWBorderPixel | CWEventMask,
    &attribs
  );
  
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
  char* data = malloc(vid_px.s[0]*32/8 * vid_px.s[1]); // 32 because of padding >:(
  if (data == NULL)
  {
    fputs("vid_init(): Failed to allocate image data.", stderr);
    vid_free();
    return 0;
  }

  vid_nix_image = XCreateImage(
    vid_nix_dsp,
    vid_nix_visual,
    BIT_DEPTH,
    ZPixmap,
    0,
    data,
    vid_px.s[0], vid_px.s[1],
    32, // bitmap_pad, just the alignment of each pixel I guess
    vid_px.s[0]*32/8
  ); // bytes per scanline, since we have padding of 32, we use 32 instead of 24.

  if (!vid_nix_image)
  {
    fputs("vid_init(): Failed to create image.", stderr);
    vid_free();
    return 0;
  }

  image_data = (unsigned char*) vid_nix_image->data;

  // Limit window size
  XSizeHints size_hints;
  size_hints.flags = PMinSize | PMaxSize;
  size_hints.min_width = vid_px.s[0];
  size_hints.max_width = vid_px.s[0];
  size_hints.min_height = vid_px.s[1];
  size_hints.max_height = vid_px.s[1];
  XSetWMNormalHints(vid_nix_dsp, vid_nix_window, &size_hints);

  vid_colors = calloc(256, sizeof (*vid_colors));

  root_window = XRootWindow(vid_nix_dsp, vid_nix_scr);
  colormap = XDefaultColormap(vid_nix_dsp, XDefaultScreen (vid_nix_dsp));

  XRRScreenConfiguration* screen_info = XRRGetScreenInfo(vid_nix_dsp, vid_nix_window);
  if (screen_info == NULL)
  {
    fputs("vid_init(): Could not get screen info, extra screen information not available.", stderr);
  }

  // Cursors
  cursors[VID_CUR_POINTER] = XCreateFontCursor(vid_nix_dsp, XC_left_ptr);
  cursors[VID_CUR_SELECT] = XCreateFontCursor(vid_nix_dsp, XC_hand2);
  cursors[VID_CUR_TEXT] = XCreateFontCursor(vid_nix_dsp, XC_xterm);
  cursors[VID_CUR_WAIT] = XCreateFontCursor(vid_nix_dsp, XC_watch);
  null_pixmap = XCreateBitmapFromData(vid_nix_dsp, vid_nix_window, null_data, 1, 1);
  // Idk why this is not black tbh, some dark magic, but it's fully transparent.
  cursors[VID_CUR_NULL] = XCreatePixmapCursor(vid_nix_dsp, null_pixmap,null_pixmap, &black, &black, 0, 0);

  vid_set_cursor_type(VID_CUR_POINTER);

  // XFixes, for clipboard
  if (!(xf_queried = XFixesQueryExtension(vid_nix_dsp, &xf_event_base, &xf_error_base)))
  {
    puts("vid_init(): Could not query XFixes, necessary for clipboard stuff!");
  }
  xf_buf_atom = XInternAtom(vid_nix_dsp, "CLIPBOARD", False);
  XFixesSelectSelectionInput(vid_nix_dsp, root_window, xf_buf_atom, XFixesSetSelectionOwnerNotifyMask);

  // Get screen refresh rate
  int rates_n;
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
  for (int i = 0, j = 0; i < vid_px._total; i++, j+=4)
  {
    // image_data[j+3] = 0;
    image_data[j+2] = vid_colors[vid_px.p[i]][0];
    image_data[j+1] = vid_colors[vid_px.p[i]][1];
    image_data[j+0] = vid_colors[vid_px.p[i]][2];
  }

  XPutImage
  (
    vid_nix_dsp,
    vid_nix_window,
    vid_nix_gc,
    vid_nix_image,
    0, 0,
    0, 0,
    vid_px.s[0], vid_px.s[1]
  );
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
