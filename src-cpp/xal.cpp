#include "xal.hpp"

#include <cstdlib>

extern void main_shutdown_all();

namespace xal
{
  bool initialized = false;

  wav::file_t* menu_music = nullptr;

  wav::player_t* player = nullptr;

  dsp_context_t* context = nullptr;

  psf::file_t* font_f = nullptr;

  server_t* server = nullptr;
  client_t* client = nullptr;

  void dsp_context_t::handler(dsp::context_t::event_t& e)
  {
    switch (e.type)
    {
      case dsp::E_CLOSE:
      main_shutdown_all();
      exit(0);
      break;

      case dsp::E_RELEASE:
      
      break;
    }
  }

  void server_t::handler(net::server_t::event_t& e)
  {
    if (e.type == net::E_DISJOIN)
    {
      puts("Client disjoined");
    }
  }

  void client_t::handler(net::client_t::event_t& e)
  {
    
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
    player = new wav::player_t(*menu_music);

    context = new dsp_context_t(640, 400);

    font_f = new psf::file_t(com::relfp("roman.psf"));

    client = new client_t(net::host_name);
    // client->sock.set_addr(net::host_addr, server->sock.bound_port);
    client->join();

    context->palette[0][0] = 0;
    context->palette[0][1] = 0;
    context->palette[0][2] = 0;

    context->palette[1][0] = 255;
    context->palette[1][1] = 255;
    context->palette[1][2] = 255;

    context->realize_palette();

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

    delete context;

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

    context->run();

    if (server != nullptr)
    {
      server->run();
    }
    if (client != nullptr)
    {
      client->run();
    }
    
    context->map.clear(0);
    context->map.put(*font_f, 'G', 10, 15, 1, 0);
    context->map.put(*font_f, 'g', 19, 15, 1, 0);
    context->refresh();

    tmr::end_tick();
  }
}
