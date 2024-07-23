#pragma once

#include "com.hpp"
#include "psf.hpp"

namespace dsp
{
  #if defined(_WIN32) || defined(__linux__)
  constexpr bool can_multi_ctx = true;
  #endif

  typedef uint8_t px_t;

  struct map_t
  {
    px_t* p = nullptr;
    
    union
    {
      uint16_t s[2]; // The size but per frame
      struct
      {
        uint16_t w, h; // The size but per frame
      };
    };
    uint32_t _ppf; // Cached pixels per frame(w*h)
    uint16_t fi = 0; // Frame index
    uint16_t fn; // Total frames

    map_t(int w, int h) : map_t(w, h, 1) { }
    map_t(int w, int h, int frames);
    // If you wish it's available.
    map_t(px_t* data, int w, int h, int frames);
    // Loads the data. Throws a specific file exception if fails.
    map_t(const char* fp);
    ~map_t();

    // Throws a specific file exception if fails.
    void save(const char* fp);

    inline void go(unsigned frame)
    {
      COM_PARANOID_A(frame < (unsigned)fn, "map_t::go(): Bad frame");
      fi = frame;
    }

    inline void put(px_t p, uint16_t x, uint16_t y)
    {
      COM_PARANOID_A(x+y*w < w*h*fn, "map_t::go(): Bad frame");
      this->p[x + y * w + fi] = p;
    }

    inline void put(px_t p, uint32_t i)
    {
      this->p[i + fi] = p;
    }

    // Appxoimately 8 times faster than manual put, because utilizes 64 bit aligned memory and shit.
    void clear(px_t p);
    
    // Puts m at its current frame, at x and y, on the current frame of this one(fi).
    // Quite fast like clear because also utilizes memory alignment.
    void put(map_t& m, uint16_t x, uint16_t y);

    // As funny as it is, it serves the specific purpose of emulating terminal graphics.
    // Puts m at its current frame, at x and y, on the current frame of this one(fi).
    // If we encounter p[i]=0 then we replace it with bg, if we encounter any other color we replace it with fg.
    // Slower than regular put, because needs to compare individual pixels.
    void put(map_t& m, uint16_t x, uint16_t y, px_t fg, px_t bg);

    void put(psf::file_t& font, unsigned glyph, uint16_t x, uint16_t y, px_t fg, px_t bg);
  };

  enum event_type_t
  {
    _E_NULL,
    E_CLOSE, // Close event, user wants to close the application
    E_HIDE, // When the video module is hidden and cannot be seen by the user
    E_SHOW, // When the video module is shown and seen by the user(sends it on ctx_t() too)

    E_PRESS, // Press key/button
    E_RELEASE, // Release key/button
    E_MOVE, // Move mouse
  };

  struct system_data_t;

  // It is highly recommended to inherit this class and override handler to something that suits you!
  struct ctx_t
  {
    struct event_t
    {
      int type;
      union
      {
        struct
        {
          int code;
        } press, release;
        struct
        {
          int x, y;
        } move;
      };
    };

    system_data_t* sys = nullptr;
    map_t map;
    char palette[256][3];
    char pixel_size;

    virtual void handler(event_t& e);
    
    // Throws com::system_ex_t if fails.
    ctx_t(short w, short h);

    ~ctx_t();

    // handler will be called if events are present, so make sure to set it up!
    void run();
    // Refreshes the actual context on the system level, the window buffer, using map.
    void refresh();
    // Realizes the palette, many systems require copying of the palette to an internal buffer.
    void realize_palette();
  };

  extern bool initialized;

  void initialize(const char* title);
  void shutdown();
}
