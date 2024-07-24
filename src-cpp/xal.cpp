#include "xal.hpp"

namespace xal
{
  bool initialized = false;

  wav::file_t* menu_music = nullptr;

  wav::player_t* player = nullptr;

  dsp_context_t* context = nullptr;

  psf::file_t* font_f = nullptr;

  void dsp_context_t::handler(dsp::context_t::event_t& e)
  {
    switch (e.type)
    {
      case dsp::E_CLOSE:
      shutdown();
      break;

      case dsp::E_RELEASE:
      
      break;
    }
  }

  void initialize()
  {
    if (initialized)
    {
      return;
    }
    
    // net::sock_t sock(true);
    
    menu_music = new wav::file_t(com::relfp("nolove.wav"));
    player = new wav::player_t(*menu_music);

    context = new dsp_context_t(640, 400);

    font_f = new psf::file_t(com::relfp("roman.psf"));

    context->palette[0][0] = 0;
    context->palette[0][1] = 0;
    context->palette[0][2] = 0;

    context->palette[1][0] = 255;
    context->palette[1][1] = 255;
    context->palette[1][2] = 255;

    context->realize_palette();

    initialized = true;
  }
  
  void shutdown()
  {
    if (!initialized)
    {
      return;
    }

    delete player;
    delete menu_music;

    delete context;

    delete font_f;

    initialized = false;
  }
  
  void run()
  {
    tmr::begin_tick();

    wav::begin_playback();
    player->play();
    wav::end_playback();

    context->run();
    
    context->map.clear(0);
    context->map.put(*font_f, 'G', 10, 15, 1, 0);
    context->map.put(*font_f, 'g', 19, 15, 1, 0);
    context->refresh();

    tmr::end_tick();
  }
}
