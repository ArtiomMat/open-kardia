#include "Video.hpp"
#include "Errors.hpp"

#include <stddef.h>
#include <stdlib.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>

#include <iostream>

struct SystemVideo
{
  Display* dsp = nullptr;
  int scr = -1;
  Visual* visual = nullptr;
  Atom wmdeletewnd_atom = 0;
  GC gc = nullptr;
  Window window = 0;
  XImage* image = nullptr;

  unsigned char* pixels; // Retrieved from Img
};

static const char keyMap[256] = {
	0,0,0,0,0,0,0,0,0, // First 9 are for some reason not mapped?
	KEY_ESC,'1','2','3','4','5','6','7','8','9','0','-','=',KEY_BS,
	KEY_TAB,'q','w','e','r','t','y','u','i','o','p','[',']',KEY_ENTER,
	KEY_CTRL,'a','s','d','f','g','h','j','k','l',';','\'','`',KEY_LSHIFT,
	'\\','z','x','c','v','b','n','m',',','.','/',KEY_RSHIFT,
	// NOTE: *, + etc are from the numpad.
	'*',KEY_LALT,KEY_SPACE,KEY_CAPSLOCK,
	KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10, // KF11=95 for some reason.
	KEY_NUMLOCK, KEY_SCROLLLOCK,
};
static const char buttonMap[4] = {-1, KEY_LMOUSE, KEY_MMOUSE,KEY_RMOUSE};

static int
x_error_handler(Display * d, XErrorEvent * e)
{
  static char text[512];
  fprintf(stderr, "x_error_handler(): %hhu\n", e->error_code);
  XGetErrorText(d, e->error_code, text, 512);
  puts(text);
  return 0;
}

int Video::screenRate = 30;

void Video::free()
{
  if (sys != nullptr)
  {
    if (sys->dsp != nullptr)
    {
      if (sys->window != 0)
      {
        XDestroyWindow(sys->dsp, sys->window);
      }

      if (sys->image != nullptr)
      {
        // Should theoretically also free the pixel data we allocated
        XDestroyImage(sys->image);
      }

      XCloseDisplay(sys->dsp);
    }

    delete sys;
  }
}

Video::Video(int width, int height) : noter("Video")
{
  size[0] = width;
  size[1] = height;

  XSetErrorHandler(x_error_handler);

  sys = new SystemVideo;

  sys->dsp = XOpenDisplay("");
  if (!sys->dsp)
  {
    throw SystemError("Can't XOpenDisplay(NULL)");
  }

  sys->scr = DefaultScreen(sys->dsp);

  // Custom WM_DELETE_WINDOW protocol
  sys->wmdeletewnd_atom = XInternAtom(sys->dsp, "WM_DELETE_WINDOW", False);

  // Visuals and stuff for drawing.
  XVisualInfo xvisualinfo;
  if (!XMatchVisualInfo(sys->dsp, sys->scr, 24, TrueColor, &xvisualinfo))
  {
    throw SystemError("Failed to XMatchVisualInfo(), screen may not fit TrueColor 24 bit depth");
  }

  sys->visual = xvisualinfo.visual;

  XSetWindowAttributes attribs = {0};
  attribs.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | StructureNotifyMask;

  sys->window = XCreateWindow(
    sys->dsp,
    RootWindow(sys->dsp, sys->scr),
    0, 0, size[0], size[1],
    0, // border width
    24, // bit depth
    InputOutput,
    sys->visual,
    CWBorderPixel | CWEventMask,
    &attribs
  );

  // XDefineCursor(sys->dsp, sys->window, createnullcursor(sys->dsp, sys->window));

  // GC Creation, before mapping.
  XGCValues xgcvalues;
  xgcvalues.graphics_exposures = False;
  sys->gc = XCreateGC(sys->dsp, sys->window, GCGraphicsExposures, &xgcvalues );

  XSelectInput(sys->dsp, sys->window, attribs.event_mask);

  sys->window = sys->window;

  // Set our custom WM_DELETE_WINDOW protocol
  XSetWMProtocols(sys->dsp, sys->window, &sys->wmdeletewnd_atom, 1);

  XMapWindow(sys->dsp, sys->window);
  XFlush(sys->dsp); // Wait until window is visible.

  // Image creation
  char* data = (char*) malloc(size[0]*32/8 * size[1]); // 32 because of padding >:(
  if (!data)
  {
    throw MemoryError("Failed to allocate image data");
  }

  sys->image = XCreateImage(
    sys->dsp,
    sys->visual,
    24,
    ZPixmap,
    0,
    data,
    size[0], size[1],
    32, // bitmap_pad, no clue, 32 is "if unsure".
    size[0]*32/8
  ); // bytes per scanline, since we have padding of 32, we use 32 instead of 24.

  if (!sys->image)
  {
    throw SystemError("Failed to create image object");
  }

  sys->pixels = (unsigned char*) sys->image->data;

  // Limit window size
  XSizeHints size_hints;
  size_hints.flags = PMinSize | PMaxSize;
  size_hints.min_width = size[0];
  size_hints.max_width = size[0];
  size_hints.min_height = size[1];
  size_hints.max_height = size[1];
  XSetWMNormalHints(sys->dsp, sys->window, &size_hints);

  //root_window = XRootWindow(sys->dsp, sys->scr);
  //colormap = XDefaultColormap(sys->dsp, XDefaultScreen (sys->dsp));

  XRRScreenConfiguration* screen_info = XRRGetScreenInfo(sys->dsp, sys->window);
  if (screen_info == NULL)
  {
    noter.note(NOTE_DEBUG, "Could not get screen info, extra screen information not available\n");
  }

  int rates_n;

  short* rates = XRRConfigRates(screen_info, 0, &rates_n);
  if (screen_info == NULL)
  {
    noter.note(NOTE_DEBUG, "Could not find refresh rate, so defaulting to %dHz!\n");
  }

  screenRate = rates[0];

  noter.note(NOTE_DEBUG, "Success, refresh rate %dHz\n", screenRate);
}

Video::~Video()
{
  free();
}


void Video::endFrame()
{
  XEvent xe;

  VideoEvent e;
  e.type = VideoListener::_EV_NULL;

  while (XPending(sys->dsp))
  {
    XNextEvent(sys->dsp, &xe);

    switch (xe.type)
    {
      case ClientMessage:
      if (!(
        xe.xclient.message_type == XInternAtom(sys->dsp, "WM_PROTOCOLS", True) &&
        (Atom)xe.xclient.data.l[0] == sys->wmdeletewnd_atom
      ))
        break;
      // Otherwise proceed here(most likely it wont, we overrode it, but just in case):
      case DestroyNotify:
      e.type = VideoListener::EV_CLOSE;
      break;

      case KeyPress:
      e.type = VideoListener::EV_PRESS;
      e.press.code = keyMap[xe.xkey.keycode];
      break;

      case KeyRelease:
      e.type = VideoListener::EV_RELEASE;
      e.release.code = keyMap[xe.xkey.keycode];
      break;

      case ButtonPress:
      e.type = VideoListener::EV_PRESS;
      e.press.code = buttonMap[xe.xbutton.button];
      break;

      case ButtonRelease:
      e.type = VideoListener::EV_RELEASE;
      e.release.code = buttonMap[xe.xbutton.button];
      break;

      case UnmapNotify:
      e.type = VideoListener::EV_HIDE;
      break;

      case Expose:
      e.type = VideoListener::EV_SHOW;
      break;

      case MotionNotify:
      e.type = VideoListener::EV_MOVE;
      e.move.x = xe.xmotion.x;
      e.move.y = xe.xmotion.y;
      break;
    }

    if (e.type != VideoListener::_EV_NULL)
    {
      reporter.report(e);
    }
  }

  // Blit the changes
  XPutImage
  (
    sys->dsp,
    sys->window,
    sys->gc,
    sys->image,
    0, 0,
    0, 0,
    size[0], size[1]
  );
}

void Video::wipe(unsigned char color)
{
  for (int i = 0; i < size[0] * size[1]; i++)
  {
    setPixel(i, color);
  }
}

unsigned char Video::getPixel(int i)
{
  return sys->pixels[i*4+3];
}
void Video::setPixel(int i, unsigned char c)
{
  sys->pixels[i*4+3] = c; // We use the useless padding as the color index, I am a fucking genius
  sys->pixels[i*4+2] = colors[c][0];
  sys->pixels[i*4+1] = colors[c][1];
  sys->pixels[i*4+0] = colors[c][2];
}
