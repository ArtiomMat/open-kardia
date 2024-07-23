#include <cstdio>

#include "com.hpp"
#include "net.hpp"
#include "dsp.hpp"
#include "tmr.hpp"
#include "psf.hpp"
#include "wav.hpp"

int main(int args_n, const char** args)
{
  puts("\nINITIALIZE...\n");

  com::initialize(args_n, args);
  net::initialize();
  dsp::initialize("Xalurzia");
  tmr::initialize(30);
  wav::initialize(60);

  com::str_t lol = "Fuck";
  puts(lol + " me. I have " + 1.2 + " shekels");

  // net::sock_t sock(true);
  dsp::ctx_t x(640, 400);

  wav::file_t music(com::relfp("nolove.wav"));
  wav::source_t music_src(music);

  psf::file_t font(com::relfp("roman.psf"));


  x.palette[0][0] = 0;
  x.palette[0][1] = 0;
  x.palette[0][2] = 0;

  x.palette[1][0] = 170;
  x.palette[1][1] = 128;
  x.palette[1][2] = 200;

  x.palette[2][0] = 0;
  x.palette[2][1] = 128;
  x.palette[2][2] = 0;
  x.realize_palette();

  puts("\nRUN...\n");

  while (1)
  {
    tmr::begin_tick();

    wav::begin();
    music_src.play();
    wav::end();

    // x.map.clear(5);
    x.map.put(font, 'G', 10, 15, 1, 0);
    x.map.put(font, 'g', 19, 15, 1, 0);
    // x.map.put(1, 10, 10);
    x.refresh();
    x.run();

    tmr::end_tick();
  }

  puts("\nSHUTDOWN...\n");

  wav::shutdown();
  dsp::shutdown();
  net::shutdown();
  com::shutdown();
  tmr::shutdown();
  
  return 0;
}
