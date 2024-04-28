#include "com.h"

#include <string.h>

static int args_n;
static const char** args;

static int dir_end; // Where to splice the relative path in com_relfp

char com_dir[COM_PATH_SIZE];

#ifdef __linux__
  #include "nix/com.c"
#elif
  #include "win/com.c"
#endif

int
com_arg(const char* str)
{
  for (int i = 1; i < args_n; i++)
  {
    if (!strcmp(str, args[i]))
    {
      return i;
    }
  }
  return 0;
}

const char*
com_relfp(const char* p)
{
  int dir_i = dir_end;
  for (int i = 0; p[i]; i++, dir_i++)
  {
    if (dir_i >= COM_PATH_SIZE-1) // Too long!
    {
      return NULL;
    }

    com_dir[dir_i] = p[i];
  }

  com_dir[dir_i] = 0;

  return com_dir;
}
