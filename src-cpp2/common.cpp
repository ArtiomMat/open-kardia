#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "common.hpp"

namespace axe
{
  bool initialized = false;

  char _global_dir[PATH_SIZE]; // We copy from it into the thread_local dir
  // For internal use
  int _dir_end;

  thread_local char dir[PATH_SIZE] = {0};
  static thread_local bool dir_copied = false; // Because we may not have copied it after we opened the thread

  const char** args;
  int args_n;

  void _initialize2()
  {
    // Setting up _dir_end, and null terminating _global_dir after the slash.
    int found_slash = 0;
    for (; _dir_end >= 0; _dir_end--)
    {
      if (_global_dir[_dir_end] == '\\' || _global_dir[_dir_end] == '/')
      {
        found_slash = 1;
        _global_dir[++_dir_end] = 0;
        break;
      }
    }

    // If we didn't find a slash it means something went wrong, perhaps _dir_end was =-1 to begin with
    if (!found_slash)
    {
      throw system_ex_t("Getting executable's directory failed.");
    }

    puts(
      "Common module initialized, "
      #ifdef COM_LILE
        "lil"
      #else
        "big"
      #endif
      " endian system.");

    initialized = true;
  }

  void shutdown()
  {
    if (!initialized)
    {
      return;
    }
    puts("Common module shutdown.");
    initialized = false;
  }

  int find_arg(const char* str)
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

  const char* relfp(const char* p)
  {
    if (!dir_copied) // Didn't copy _global_dir to thread local dir yet
    {
      for (int i = 0; i < _dir_end; i++)
      {
        dir[i] = _global_dir[i];
      }
      dir[_dir_end] = 0;

      dir_copied = true;
    }

    int dir_i = _dir_end;
    for (int p_i = 0; p[p_i]; p_i++, dir_i++)
    {
      if (dir_i >= PATH_SIZE-1) // Too long!
      {
        return NULL;
      }

      dir[dir_i] = p[p_i];
    }

    dir[dir_i] = 0;

    return dir;
  }
  
  thread_local time_t target_frame_time = 16;
  thread_local time_t frame_time = 0;

  thread_local time_t frame_begin_time = 0;

  bool initialized = false;

  void wait_for_frame()
  {
    time_t delta = now() - frame_begin_time;

    if (delta < target_frame_time)
    {
      sleep(target_frame_time-delta);
      frame_time = target_frame_time;
    }
    // printf("%d %d\n", delta, target_frame_time);

    COM_PARANOID_M("tmr::end_tick(): tick longer than target_frame_time.");

    frame_time = delta; // The real tick time is just delta

    frame_begin_time = now();
  }
}
