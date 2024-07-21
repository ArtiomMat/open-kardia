#include "../dsp.hpp"

#include <windows.h>

#include <cstdio>

#define WSTYLE	WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE

namespace dsp
{
  typedef struct
  {
    BITMAPINFOHEADER header;
    RGBQUAD colors[256];
  } dibinfo_t;

  struct system_data_t
  {
    HWND hwnd = NULL; // NULL means vid isn't initialized
    HDC hdc;

    HBITMAP hdib;
    HGDIOBJ old_hdib;
    HDC hdibdc;

    HPALETTE hpal;
    HPALETTE hpalold;

    MSG msg;
  };

  // I shamelesly copied from Quake 2
  typedef struct
  {
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palEntries[256];
  } identitypalette_t;

  // The current ctx that run was called on, to identify in wndproc
  static thread_local ctx_t* running_ctx = nullptr;

  LRESULT CALLBACK 
  wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
  {
    ctx_t::event_t e;
    switch (msg)
    {
      case WM_CLOSE:
      e.type = E_CLOSE;
      running_ctx->handler(e);
      break;

      default:
      return DefWindowProc(hwnd, msg, wp, lp);
      break;
    }

    return 0;
  }

  bool initialize()
  {
    if (initialized)
    {
      return true;
    }

    // Create the class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = wndproc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "DSP_CLASS";
    if (!RegisterClassEx(&wc))
    {
      fprintf(stderr, "vid_init(): Class failed to register.\n");
      return false;
    }

    return true;
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }

    UnregisterClass("DSP_CLASS", GetModuleHandle(NULL));
    initialized = false;
  }


  ctx_t::ctx_t(short _vid_w, short _vid_h, short frames) : map(_vid_w, _vid_h, frames)
  {

    // Adjusting the window size
    // Thank you so much Id for making Quake open source.
    RECT r;
    int w, h;
    
    r.left = 0;
    r.top = 0;
    r.right = _vid_w;
    r.bottom = _vid_h;
    
    AdjustWindowRect (&r, WSTYLE, 0);
    w = r.right - r.left;
    h = r.bottom - r.top;

    sys.hwnd = CreateWindowEx(0, "VIDCLASS", "",
      WSTYLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      w, h,
      NULL, NULL, GetModuleHandle(NULL), NULL);
    
    if (!sys.hwnd)
    {
      fprintf(stderr, "vid_init(): Window creation failed.\n");
      return 0;
    }
    
    sys.hdc = GetDC(sys.hwnd);
    
    dibinfo_t di;
    di.header.biSize          = sizeof(BITMAPINFOHEADER);
    di.header.biWidth         = _vid_w;
    di.header.biHeight        = -_vid_h;
    di.header.biPlanes        = 1;
    di.header.biBitCount      = 8;
    di.header.biCompression   = BI_RGB;
    di.header.biSizeImage     = 0;
    di.header.biXPelsPerMeter = 0;
    di.header.biYPelsPerMeter = 0;
    di.header.biClrUsed       = 256;
    di.header.biClrImportant  = 256;

    if (!( vid_win_hdib = CreateDIBSection( sys.hdc, (BITMAPINFO*)&di, DIB_RGB_COLORS, (void**)&vid_px.p, NULL, 0) ))
    {
      fprintf(stderr, "vid_init(): Creation of DIB section failed.\n");
      return 0;
    }

    vid_colors = calloc(3 * 256, 1);
    
    if (!( vid_win_hdibdc = CreateCompatibleDC( sys.hdc ) ))
    {
      fprintf(stderr, "vid_init(): Creation of compatible DC failed failed.\n");
      return 0;
    }
    
    old_hdib = SelectObject( vid_win_hdibdc, vid_win_hdib );
    if (!( SetDIBColorTable(vid_win_hdibdc, 0, 256, di.colors) ))
    {
      fprintf(stderr, "vid_init(): Palette was not created.\n");
      return 0;
    }

    return 1;
  }
}