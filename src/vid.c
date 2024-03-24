#include "vid.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <X11/Xlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// Writable frame buffer, essentially blitting:
// https://bbs.archlinux.org/viewtopic.php?id=225741

Display* vid_nix_dsp = 0;
int vid_nix_scr;
Visual* vid_nix_visual;
Atom vid_nix_wmdeletewnd_atom;
GC vid_nix_gc;
Window vid_nix_window;
XImage* vid_nix_image;

unsigned char (*vid_colors)[3] = NULL;

unsigned char* vid_pixels = NULL;

int vid_w, vid_h;

void
vid_def_event_handler(vid_event_t* e)
{
  switch(e->type)
  {
    case VID_E_CLOSE:
    exit(0);
    break;
  }
}

void (*vid_event_handler)() = vid_def_event_handler;

// TODO: It's not exactly that safe
void
vid_free()
{
  free(vid_colors);

  XDestroyWindow(vid_nix_dsp, vid_nix_window);
  
  if (vid_nix_image)
    XDestroyImage(vid_nix_image);
  vid_nix_image = 0;

  XCloseDisplay(vid_nix_dsp);
  
  puts("vid_free(): Video module freed.");
}

static int
x_error_handler(Display * d, XErrorEvent * e)
{
  static char text[512];
  printf("x_error_handler(): %hhu\n", e->error_code);
  XGetErrorText(d, e->error_code, text, 512);
  puts(text);
  return 0;
}

int
vid_init(int _vid_w, int _vid_h)
{
  vid_w = _vid_w;
  vid_h = _vid_h;

  XSetErrorHandler(x_error_handler);

  vid_nix_dsp = XOpenDisplay(NULL);
  if (!vid_nix_dsp)
  {
    puts("vid_init(): Can't XOpenDisplay(NULL)!");
    return 0;
  }

  vid_nix_scr = DefaultScreen(vid_nix_dsp);

  // Custom WM_DELETE_WINDOW protocol
  vid_nix_wmdeletewnd_atom = XInternAtom(vid_nix_dsp, "WM_DELETE_WINDOW", False);

  // Visuals and stuff for drawing.
  XVisualInfo xvisualinfo;
  if (!XMatchVisualInfo(vid_nix_dsp, vid_nix_scr, 24, TrueColor, &xvisualinfo))
  {
    puts("vid_init(): Failed to XMatchVisualInfo(). Are you sure you have 24 bit depth TrueColor?");
    vid_free();
    return 0;
  }

  vid_nix_visual = xvisualinfo.visual;

  XSetWindowAttributes attribs = {0};
  attribs.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;
  
  vid_nix_window = XCreateWindow(
    vid_nix_dsp,
    RootWindow(vid_nix_dsp, vid_nix_scr),
    0, 0, vid_w, vid_h,
    0, // border width
    24, // bit depth
    InputOutput,
    vid_nix_visual,
    CWBorderPixel | CWEventMask,
    &attribs
  );
  
  // XDefineCursor(vid_nix_dsp, vid_nix_window, createnullcursor(vid_nix_dsp, vid_nix_window));
  
  // GC Creation, before mapping.
  XGCValues xgcvalues;
  xgcvalues.graphics_exposures = False;
  vid_nix_gc = XCreateGC(vid_nix_dsp, vid_nix_window, GCGraphicsExposures, &xgcvalues );

  XSelectInput(vid_nix_dsp, vid_nix_window, attribs.event_mask);

  vid_nix_window = vid_nix_window;

  // Set our custom WM_DELETE_WINDOW protocol
  XSetWMProtocols(vid_nix_dsp, vid_nix_window, &vid_nix_wmdeletewnd_atom, 1);

  XMapWindow(vid_nix_dsp, vid_nix_window);
  XFlush(vid_nix_dsp); // Wait until window is visible.

  // Image creation
  char* data = malloc(vid_w*32/8 * vid_h); // 32 because of padding >:(
  if (!data)
  {
    puts("vid_init(): Failed to allocate image data.");
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
    vid_w, vid_h,
    32, // bitmap_pad, no clue, 32 is "if unsure".
    vid_w*32/8
  ); // bytes per scanline, since we have padding of 32, we use 32 instead of 24.

  if (!vid_nix_image)
  {
    puts("vid_init(): Failed to create image.");
    vid_free();
    return 0;
  }

  vid_pixels = vid_nix_image->data;

  // Limit window size
  XSizeHints size_hints;
  size_hints.flags = PMinSize | PMaxSize;
  size_hints.min_width = vid_w;
  size_hints.max_width = vid_w;
  size_hints.min_height = vid_h;
  size_hints.max_height = vid_h;
  XSetWMNormalHints(vid_nix_dsp, vid_nix_window, &size_hints);

  vid_colors = calloc(256, sizeof (*vid_colors));

  printf("vid_init(): Video module initialized, 24 bit TrueColor.\n");
  return 1;	
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
  static int mouse = 0, mousex = 0, mousey = 0;

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
      e.press.code = xe.xkey.keycode;
      break;
      
      case KeyRelease:
      e.type = VID_E_RELEASE;
      e.release.code = xe.xkey.keycode;
      break;

      case ButtonPress:
      e.type = VID_E_PRESS;
      e.press.code = xe.xbutton.button;
      break;

      case ButtonRelease:
      e.type = VID_E_RELEASE;
      e.release.code = xe.xbutton.button;
      break;

      case MotionNotify:
      e.type = VID_E_MOVE;
      e.move.x = xe.xmotion.x;
      e.move.y = xe.xmotion.y;
      break;
    }

    if (e.type != _VID_E_NULL)
    {
      vid_event_handler(&e);
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
    vid_w, vid_h
  );
}

void
vid_wipe(int color)
{
  for (int i = 0; i < vid_h*vid_w; i++)
  {
    vid_pixels[i*4+2] = vid_colors[color][0];
    vid_pixels[i*4+1] = vid_colors[color][1];
    vid_pixels[i*4+0] = vid_colors[color][2];
  }
}

void
vid_set(unsigned char color, int i)
{
  vid_pixels[i*4+2] = vid_colors[color][0];
  vid_pixels[i*4+1] = vid_colors[color][1];
  vid_pixels[i*4+0] = vid_colors[color][2];
}
