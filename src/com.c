#include "com.h"

#include <string.h>
#include <stdio.h>

const char** com_args;
int com_args_n;

int _com_dir_end; // Where to splice the relative path in com_relfp

char com_dir[COM_PATH_SIZE];

// Initializes com_dir after the executable fp has been written to it
int
_com_init_dir()
{
  // Setting up _com_dir_end, and null terminating com_dir after the slash.
  int found_slash = 0;
  for (; _com_dir_end >= 0; _com_dir_end--)
  {
    if (com_dir[_com_dir_end] == '\\' || com_dir[_com_dir_end] == '/')
    {
      found_slash = 1;
      com_dir[++_com_dir_end] = 0;
      break;
    }
  }

  // If we didn't find a slash it means something went wrong, perhaps _com_dir_end was =-1 to begin with
  if (!found_slash)
  {
    fputs("com_init(): Getting executable's directory failed.", stderr);
    return 0;
  }

  puts(
    "com_init(): Common module initialized, "
    #ifdef COM_LILE
      "lil"
    #else
      "big"
    #endif
    " endian system.");

  return 1;
}

int
com_arg(const char* str)
{
  for (int i = 1; i < com_args_n; i++)
  {
    if (!strcmp(str, com_args[i]))
    {
      return i;
    }
  }
  return 0;
}

const char*
com_relfp(const char* p)
{
  int dir_i = _com_dir_end;
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