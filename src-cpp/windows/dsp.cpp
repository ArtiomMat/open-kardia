#include "../dsp.hpp"

#include <windows.h>

#include <cstdio>

#define WSTYLE	WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE

#define DSP_CLASS "DSP_CLASS"

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

  void initialize()
  {
    if (initialized)
    {
      return;
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
      throw com::system_ex_t("Registering window class.");
    }

    puts("Display module initialized.");
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }

    UnregisterClass(DSP_CLASS, GetModuleHandle(NULL));
    initialized = false;
  }


  ctx_t::ctx_t(short _vid_w, short _vid_h, const char* title) : map(_vid_w, _vid_h, 1)
  {
    sys = new system_data_t;

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

    sys->hwnd = CreateWindowEx(0, DSP_CLASS, title,
      WSTYLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      w, h,
      NULL, NULL, GetModuleHandle(NULL), NULL);
    
    if (!sys->hwnd)
    {
      throw com::system_ex_t("Creating window.");
    }
    
    sys->hdc = GetDC(sys->hwnd);
    
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
    
    _aligned_free(map.p); // FIXME: Obviously will cause a segfault, but we need to find a way to either make CreateDIBSection allocate aligned data, or just ditch the aligned optimizations we did, or make the optimizations slower by first getting to aligned data and then starting all the bullshit, no idea.
    
    if (!( sys->hdib = CreateDIBSection( sys->hdc, (BITMAPINFO*)&di, DIB_RGB_COLORS, reinterpret_cast<void**>(&map.p), NULL, 0) ))
    {
      com::system_ex_t("Creating DIB section.");
    }
    
    if (!( sys->hdibdc = CreateCompatibleDC( sys->hdc ) ))
    {
      com::system_ex_t("Creating compatible DC.");
    }
    
    sys->old_hdib = SelectObject( sys->hdibdc, sys->hdib );
    if (!( SetDIBColorTable(sys->hdibdc, 0, 256, di.colors) ))
    {
      com::system_ex_t("Creating palette.");
    }
  }

  void ctx_t::run()
  {
    running_ctx = this;

    while(PeekMessage(&(sys->msg), NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&(sys->msg));
      DispatchMessage(&(sys->msg));
    }
  }

  void ctx_t::realize_palette()
  {
    RGBQUAD rgbs[256];

    for (int i = 0; i < 256; i++)
    {
      rgbs[i].rgbRed   = palette[i][0];
      rgbs[i].rgbGreen = palette[i][1];
      rgbs[i].rgbBlue  = palette[i][2];
      rgbs[i].rgbReserved = 0;
    }

    SetDIBColorTable(sys->hdibdc, 0, 256, rgbs);
  }

  void ctx_t::refresh()
  {
    BitBlt(sys->hdc, 0, 0, map.w, map.h, sys->hdibdc, 0, 0, SRCCOPY);
  }

  ctx_t::~ctx_t()
  {
    SelectObject(sys->hdc, sys->old_hdib);
    DeleteDC(sys->hdibdc);
    DeleteObject(sys->hdib);
    DestroyWindow(sys->hwnd);

    delete sys;
  }
}