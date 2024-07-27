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
  struct resolution_t
  {
    com::str_t desc;
    union
    {
      uint16_t width, height;
      uint16_t size[2];
    };

    resolution_t(uint16_t width, uint16_t height);
  };

  extern unsigned resolutions_n;
  extern const resolution_t resolutions[];

  struct dsp_context_t : dsp::window_t
  {
    using dsp::window_t::window_t;
    void handler(event_t& e);
  };

  // Raw screen data that is to be put!
  struct screen_t
  {
    struct char_t
    {
      uint16_t glyph;
      uint8_t fg, bg;
    };

    dsp_context_t ctx;

    char_t (*chars)[] = nullptr;
    union
    {
      uint16_t cols, rows;
      uint16_t size[2];
    };

    screen_t(resolution_t& res);
    ~screen_t();

    void put();
  };

  extern screen_t* scr;

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

  extern wav::audio_t* menu_music;
  extern wav::speaker_t* player;

  extern dsp_context_t* dsp_context;

  extern psf::font_t* font_f;

  extern server_t* server;
  extern client_t* client;

  void total_shutdown(bool _exit);

  void initialize_dsp();

  void initialize();
  void shutdown();
  
  void run();
}

