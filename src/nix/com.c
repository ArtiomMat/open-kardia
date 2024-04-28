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

  // Setting up dir_end, and null terminating com_dir after the slash.
  int found_slash = 0;
  for (; dir_end >= 0; dir_end--)
  {
    if (com_dir[dir_end] == '\\' || com_dir[dir_end] == '/')
    {
      found_slash = 1;
      com_dir[++dir_end] = 0;
      puts(com_dir);
      break;
    }
  }

  // If we didn't find a slash it means something went wrong, perhaps dir_end was =-1 to begin with
  if (!found_slash)
  {
    puts("com_init(): Getting executable's directory failed.");
    return 0;
  }

  puts(
    "com_init(): Common module initialized, "
    #ifdef COM_LILE
      "lil"
    #else
      "big"
    #endif
    " endian system.\n");

  return 1;
}
