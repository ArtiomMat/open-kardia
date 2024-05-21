#include <stdio.h>

#include "../engine/com.h"
#include "../engine/vid.h"
#include "../engine/clk.h"
#include "../engine/gui.h"

int main(int args_n, const char** args)
{
  com_init(args_n, args);
  vid_init(400, 400);
  clk_init(1000 / 30);

  while(1)
  {
    clk_begin_tick();

    vid_wipe(255);
    vid_run();

    clk_end_tick();
  }

  return 0;
}
