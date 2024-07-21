#include "dsp.hpp"

#include <cstdlib>
#include <cstdio>

namespace dsp
{
  bool initialized = false;

  map_t::map_t(int w, int h, int fn)
  {
    this->w = w;
    this->h = h;
    this->fn = fn;

    p = static_cast<px_t*>( _aligned_malloc(w * h * fn, 8) );

    _ppf = w * h;
  }
  
  map_t::~map_t()
  {
    free(p);
  }

  bool map_t::open(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      printf("map_t::save(): Could not open a file to read '%s'!\n", fp);
      return false;
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
      printf("map_t::save(): Could not read all data to '%s'!\n", fp);
      return false;
    }

    return true;
  }

  bool map_t::save(const char* fp)
  {
    FILE* f = fopen(fp, "wb");
    if (f == NULL)
    {
      printf("map_t::save(): Could not open a file to write '%s'!\n", fp);
      return false;
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
      printf("map_t::save(): Could not write all data to '%s'!\n", fp);
      return false;
    }

    return true;
  }
}
