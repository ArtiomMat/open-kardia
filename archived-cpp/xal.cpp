// Main part of the Xalurtia module

#include "xal.hpp"

#include <cstdlib>
#include <cstdio>

namespace xal
{
  bool initialized = false;

  wav::file_t* menu_music = nullptr;

  wav::speaker_t* player = nullptr;

  psf::file_t* font_f = nullptr;

  server_t* server = nullptr;
  client_t* client = nullptr;
  
  void total_shutdown(bool _exit)
  {
    puts("\nSHUTTING DOWN...\n");
    xal::shutdown();
    wav::shutdown();
    dsp::shutdown();
    net::shutdown();
    com::shutdown();
    tmr::shutdown();

    if (_exit)
    {
      std::exit(0);
    }
  }

  void initialize()
  {
    if (initialized)
    {
      return;
    }

    tmr::target_tick_time = 30;
    
    // net::sock_t sock(true);
    
    menu_music = new wav::file_t(com::relfp("nolove.wav"));
    player = new wav::speaker_t(*menu_music);

    client = new client_t(net::host_name);
    // client->sock.set_addr(net::host_addr, server->sock.bound_port);
    // client->join();

    initialize_dsp();

    puts("Xalartia module initialized.");
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

    delete dsp_context;

    delete font_f;

    delete client;
    delete server;

    puts("Xalartia module shutdown.");
    initialized = false;
  }
  
  void run()
  {
    tmr::begin_tick();

    wav::begin_playback();
    player->play();
    wav::end_playback();

    dsp_context->run();

    if (server != nullptr)
    {
      server->run();
    }
    if (client != nullptr)
    {
      client->run();
    }
    
    dsp_context->map.clear(0);
    dsp_context->map.put(*font_f, 'G', 10, 15, 1, 0);
    dsp_context->map.put(*font_f, 'g', 19, 15, 1, 0);
    dsp_context->refresh();

    tmr::end_tick();
  }
}
