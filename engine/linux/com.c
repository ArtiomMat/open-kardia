#include "../com.h"

#include <stdio.h>
#include <unistd.h>

#include <X11/extensions/Xfixes.h>


extern Display* vid_nix_dsp;
extern int vid_nix_scr;
extern Visual* vid_nix_visual;
extern Atom vid_nix_wmdeletewnd_atom;
extern GC vid_nix_gc;
extern Window vid_nix_window;
extern XImage* vid_nix_image;

int
com_init(int _args_n, const char** _args)
{
  com_args = _args;
  com_args_n = _args_n;
  
  // -1 for the null terminator
  _com_dir_end = readlink("/proc/self/exe", com_dir, COM_PATH_SIZE) - 1;

  return _com_init_dir();
}
