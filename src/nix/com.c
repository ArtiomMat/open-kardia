#include "com.h"

#include <stdio.h>
#include <unistd.h>

int
com_init(int _args_n, const char** _args)
{
  args = _args;
  args_n = _args_n;
  
  // -1 for the null terminator
  dir_end = readlink("/proc/self/exe", com_dir, COM_PATH_SIZE) - 1;

  return _com_init_dir();
}
