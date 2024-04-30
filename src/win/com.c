#include "com.h"

#include <stdio.h>
#include <unistd.h>

#include <windows.h>

int
com_init(int _args_n, const char** _args)
{
  args = _args;
  args_n = _args_n;
  
  dir_end = GetModuleFileName(NULL, com_dir, COM_PATH_SIZE) - 1;
  
  return _com_init_dir();
}


