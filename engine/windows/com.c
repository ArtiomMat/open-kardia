#include "com.h"

#include <stdio.h>
#include <unistd.h>

#include <windows.h>

int
com_init(int _args_n, const char** _args)
{
  com_args = _args;
  com_args_n = _args_n;
  
  _com_dir_end = GetModuleFileName(NULL, com_dir, COM_PATH_SIZE) - 1;
  
  return _com_init_dir();
}


