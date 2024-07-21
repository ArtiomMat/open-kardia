#include <stdio.h>
#include <unistd.h>

#include <windows.h>

#include "../com.hpp"

namespace com
{
  void initialize(int _args_n, const char** _args)
  {
    if (initialized)
    {
      return;
    }
    
    args = _args;
    args_n = _args_n;
    
    _dir_end = GetModuleFileName(NULL, _global_dir, PATH_SIZE) - 1;
    
    _initialize2();
  }

  int get_cb_size()
  {
    if (!OpenClipboard(NULL))
    {
      return 0;
    }
    HANDLE hclipboard = GetClipboardData(CF_TEXT);
    if (hclipboard == NULL)
    {
      CloseClipboard();
      return 0;
    }

    return GlobalSize(hclipboard);
  }

  int get_cb(char* get_cb, int max_size)
  {
    if (!OpenClipboard(NULL))
    {
      return 0;
    }
    HANDLE hclipboard = GetClipboardData(CF_TEXT);
    if (hclipboard == NULL)
    {
      CloseClipboard();
      return 0;
    }

    SIZE_T size = GlobalSize(hclipboard);
    char* data = static_cast<char*>(GlobalLock(hclipboard));

    int copy_size = min(max_size, size); // Avoid segfaults

    memcpy(get_cb, data, copy_size);

    get_cb[copy_size-1] = 0; // Regardless of anything, any format we support must be null terminated.

    GlobalUnlock(hclipboard);
    CloseClipboard();

    return 1;
  }

  int set_cb(const char* set_cb, int size)
  {
    if (!OpenClipboard(NULL))
    {
      return 0;
    }
    EmptyClipboard(); 
    
    HGLOBAL hdata = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hdata == NULL)
    {
      CloseClipboard();
      return 0;
    }

    char* data = static_cast<char*>(GlobalLock(hdata));
    memcpy(data, set_cb, size);
    data[size-1] = 0; // Regardless put null-terminator

    GlobalUnlock(hdata);

    SetClipboardData(CF_TEXT, hdata);

    CloseClipboard();
    return 1;
  }

}
