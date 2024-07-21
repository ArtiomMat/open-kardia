#include "dsp.hpp"

#include <cstdlib>
#include <cstdio>
#include <malloc.h>

namespace dsp
{
  bool initialized = false;

  void ctx_t::handler(event_t& e)
  {
    if (e.type == E_CLOSE)
    {
      exit(0);
    }
  }

  map_t::map_t(int w, int h, int fn)
  {
    p = static_cast<px_t*>( _aligned_malloc(w * h * fn, 8) );

    if (p == NULL)
    {
      throw com::memory_ex_t("Allocation of pixels.");
    }

    this->w = w;
    this->h = h;
    this->fn = fn;

    _ppf = w * h;
  }
  
  map_t::~map_t()
  {
    _aligned_free(p);
  }

  map_t::map_t(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw com::open_ex_t("");
    }

    free(p);

    int status = 1;

    // Header
    status &= fread(&w, 2, 1, f);
    w = com::lil16(w);
    
    status &= fread(&h, 2, 1, f);
    h = com::lil16(h);

    status &= fread(&fn, 2, 1, f);
    fn = com::lil16(fn);

    // Cached stuff
    _ppf = w * h;

    // Data
    p = static_cast<px_t*>( _aligned_malloc(w * h * fn, 8) );
    status &= fread(p, w * h * fn, 1, f);

    if (!status)
    {
      throw com::read_ex_t("");
    }
  }

  void map_t::save(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      throw com::open_ex_t("");
    }
    uint16_t u16;
    
    int status = 1;

    // Header
    u16 = com::lil16(w);
    status &= fwrite(&u16, 2, 1, f);
    u16 = com::lil16(h);
    status &= fwrite(&u16, 2, 1, f);
    u16 = com::lil16(fn);
    status &= fwrite(&u16, 2, 1, f);

    // Data
    status &= fwrite(p, w * h * fn, 1, f);

    if (!status)
    {
      throw com::write_ex_t("");
    }
  }
}
