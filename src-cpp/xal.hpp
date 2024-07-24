// The module of Xalurtia

#pragma once

#include "com.hpp"
#include "net.hpp"
#include "dsp.hpp"
#include "tmr.hpp"
#include "psf.hpp"
#include "wav.hpp"

namespace xal
{
  struct dsp_context_t : dsp::context_t
  {
    dsp_context_t(short w, short h) : dsp::context_t(w, h) {}
    void handler(event_t& e);
  };

  struct server_t : net::server_t
  {
    server_t(const char* alias) : net::server_t(alias) {}
    void handler(event_t& e);
  };
  struct client_t : net::client_t
  {
    client_t(const char* alias) : net::client_t(alias) {}
    void handler(event_t& e);
  };

  extern bool initialized;

  extern wav::file_t* menu_music;
  extern wav::player_t* player;

  extern dsp_context_t* context;

  extern psf::file_t* font_f;

  extern server_t* server;
  extern client_t* client;

  void initialize();
  void shutdown();
  
  void run();
}

