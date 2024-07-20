#pragma once

#include "com.hpp"

namespace dsp
{
  #if defined(_WIN32) || defined(__linux__)
  constexpr bool can_multi_ctx;
  #endif

  typedef unsigned char px_t;

  struct map_t
  {
    px_t* p;
    
    union
    {
      short s[2]; // The size but per frame
      short w, h; // The size but per frame
    };
    int _ppf; // Pixels per frame
    short fi = 0; // Frame index
    short fn; // Total frames

    map_t(int w, int h);
    map_t(int w, int h, int f);
    // Only supports 
    map_t(const char* fp);
    ~map_t();

    inline void go(unsigned frame)
    {
      COM_PARANOID_A(frame < fn, "map_t::go(): Bad frame");
      fi = frame;
    }

    inline void set(px_t p, short x, short y)
    {
      this->p[x + y * w + _ppf] = p;
    }

    inline void set(px_t p, short i)
    {
      this->p[i] = p;
    }
  };

  // A map, but with frames
  struct frame_map_t : map_t
  {


  };

  struct ctx_t
  {
    
  };
}
