#include "font.hpp"
#include "common.hpp"
#include "string.hpp"

#include <cstdio>

namespace axe
{
  enum
  {
    PSF1_MODE512 = 1,
    PSF1_MODEHASTAB = 2,
    PSF1_MODESEQ = 4,

    PSF2_HAS_UNICODE_TABLE = 1,
  };
  
  typedef struct
  {
    uint32_t magic; // 72 b5 4a 86
    int32_t version;
    int32_t size; // Header size
    int32_t flags;
    int32_t length; // how many glyphs
    int32_t glyph_size;
    int32_t height;
    int32_t width;
  } psf2_t;

  typedef struct
  {
    uint16_t magic; // 36 04
    uint8_t mode;
    uint8_t size;
  } psf1_t;

  enum
  {
    PSF1 = sizeof (psf1_t),
    PSF2 = sizeof (psf2_t),
  };

  font_t::~font_t()
  {
    if (data == nullptr)
    {
      return;
    }
    
    delete [] data;
    fclose(fd);
  }

  void font_t::open(const char* fp, int priority)
  {
    union
    {
      psf1_t psf1;
      psf2_t psf2;
      uint32_t __array[8]; // For quickly swapping endian in psf2
    };

    fd = fopen(fp, "rb");

    if (fd == nullptr)
    {
      throw open_ex_t("Opening file.");
    }

    if (!fread(&psf1.magic, sizeof(psf1.magic), 1, fd))
    {
      _magic_fail:
      throw read_ex_t("Reading magic bytes.");
    }
    if (psf1.magic == 0x0436)
    {
      _type = PSF1;
    }
    else
    {
      rewind(fd);
      if (!fread(&psf2.magic, sizeof(psf2.magic), 1, fd))
      {
        goto _magic_fail;
      }
    
      if (psf2.magic == 0x864ab572)
      {
        _type = PSF2;
      }
      else
      {
        throw read_ex_t("Idnetifying magic bytes.");
      }
    }

    // Read the correct header
    rewind(fd);
    if (!fread(&psf1, _type, 1, fd)) // _type is the size
    {
      throw read_ex_t("Reading header.");
    }
    
    // Just opt for memory priority since it's the safe option
    p = (priority == P_AUTO) ? P_MEMORY : priority;

    if (_type == PSF1)
    {
      int chars_n = 256;

      if (psf1.mode)
      {
        if (psf1.mode & PSF1_MODE512)
        {
          chars_n = 512;
        }
        
        puts("Note frome font: Unicode table not supported yet.");
      }

      // Setup row size, psf1 width is always 8 pixels
      row_size = 1;
      height = psf1.size;
      int char_size = row_size*psf1.size;

      // If we prioritize speed we will read all the characters, otherwise we read one for each glyph we get
      if (p == P_SPEED)
      {
        data = new char[chars_n * char_size];

        int read;
        if ((read = fread(data, char_size, chars_n, fd)) != chars_n)
        {
          throw read_ex_t(string_t("Bad PSF file. Read") + read + "characters but need" + chars_n + ".");
        }

        // We don't need it anymore
        fclose(fd);
        fd = nullptr;
      }
      else // P_MEMORY
      {
        data = new char[char_size];
      }
    }
    else
    {
      throw ex_t("PSF2 not yet supported.");

      // Make endian readable on this system
      for (unsigned long i = 0; i < sizeof(__array)/sizeof(__array[0]); i++)
      {
        __array[i] = lil32(__array[i]);
      }


    }

    width = (_type == PSF1 ? 8 : psf2.width);
  }

  char* font_t::get_glyph(unsigned g)
  {
    // TODO: This is hard coded to strictly work with ascii, gotta improve.
    if (g < 0)
    {
      g = 256 + g;
    }

    int index =  g * row_size * height;
    if (p == P_SPEED)
    {
      return &((char*)data)[index];
    }
    else
    {
      // TODO: Make it depend on flags too
      fseek(fd, _type + index, SEEK_SET);
      if (!fread(data, row_size * height, 1, fd))
      {
        return nullptr;
      }
      return data;
    }
  }
}
