#include "vid.h"

#include <windows.h>

#include <stdio.h>

#define WSTYLE	WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE

typedef struct
{
  BITMAPINFOHEADER header;
  RGBQUAD colors[256];
} dibinfo_t;


// I shamelesly copied from Quake 2
typedef struct
{
  WORD palVersion;
  WORD palNumEntries;
  PALETTEENTRY palEntries[256];
} identitypalette_t;

static identitypalette_t s_ipal;

HWND vid_win_hwnd;
HDC vid_win_hdc;

HBITMAP vid_win_hdib;
HDC vid_win_hdibdc;

HPALETTE vid_win_hpal;
HPALETTE vid_win_hpalold;

static MSG msg;

void
vid_free()
{
}

LRESULT CALLBACK 
wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  
}

int
vid_init(int _vid_w, int _vid_h)
{
  // Create the class
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = wndproc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = "VIDCLASS";
  if (!RegisterClassEx(&wc))
  {
    fprintf(stderr, "vid_init(): Class failed to register.\n");
    return 0;
  }

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

  vid_win_hwnd = CreateWindowEx(0, "VIDCLASS", "",
    WSTYLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    w, h,
    NULL, NULL, GetModuleHandle(NULL), NULL);
  
  if (!vid_win_hwnd)
  {
    fprintf(stderr, "vid_init(): Window creation failed.\n");
    return 0;
  }
  
  vid_win_hdc = GetDC(vid_win_hwnd);
  
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

  px_init(&vid_px, w, h); // Initialize px now

  if (!( vid_win_hdib = CreateDIBSection( vid_win_hdc, (BITMAPINFO*)&di, DIB_RGB_COLORS, (void**)&vid_px.p, NULL, 0) ))
  {
    fprintf(stderr, "vid_init(): Creation of DIB section failed.\n");
    return 0;
  }
  
  if (!( vid_win_hdibdc = CreateCompatibleDC( vid_win_hdc ) ))
  {
    fprintf(stderr, "vid_init(): Creation of compatible DC failed failed.\n");
    return 0;
  }
  
  SelectObject( vid_win_hdibdc, vid_win_hdib );
  if (!( SetDIBColorTable(vid_win_hdibdc, 0, 256, di.colors) ))
  {
    fprintf(stderr, "vid_init(): Palette was not created.\n");
    return 0;
  }

  return 1;
}

void
vid_realize_colors()
{
  char *_pal = vid_colors; // Save of pal
  LOGPALETTE *logpal = ( LOGPALETTE * ) &s_ipal;
  RGBQUAD			colors[256];

  for (int i = 0; i < 256; i++, _pal += 3 )
  {
    colors[i].rgbRed   = _pal[0];
    colors[i].rgbGreen = _pal[1];
    colors[i].rgbBlue  = _pal[2];
    colors[i].rgbReserved = 0;
  }
  SetDIBColorTable(vid_win_hdibdc, 0, 256, colors);
}

void
vid_screen_size(int* w, int* h)
{
}

void
vid_set_title(const char* title)
{
  SetWindowText(vid_win_hwnd, title);
}

void
vid_run()
{
  while(PeekMessage(&(msg), NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&(msg));
    DispatchMessage(&(msg));
  }
}

void
vid_refresh()
{
  BitBlt(vid_win_hdc, 0, 0, vid_px.s[0], vid_px.s[1], vid_win_hdibdc, 0, 0, SRCCOPY);
}

void
vid_set_cursor_type(int t)
{
  
}

void
vid_get_clipboard()
{
  if (!OpenClipboard(NULL))
  {
    return;
  }
  HANDLE h = GetClipboardData(CF_TEXT);
  
  char* data = GlobalLock(h);
  puts(data);
  
  GlobalUnlock(h);
  CloseClipboard();
}

void
vid_get_clipboard()
{
  if (!OpenClipboard(NULL))
  {
    return;
  }
  HANDLE h = GetClipboardData(CF_TEXT);
  
  char* data = GlobalLock(h);
  puts(data);
  
  GlobalUnlock(h);
  CloseClipboard();
}
