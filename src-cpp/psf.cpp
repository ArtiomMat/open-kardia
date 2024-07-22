#include "psf.hpp"
#include "com.hpp"

#include <cstdio>

namespace psf
{
  font_t::~font_t()
  {
    delete [] data;
    fclose(fd);
  }

  font_t::font_t(const char* fp, int priority)
  {
    fd = fopen(fp, "rb");
    type = 0;

    if (fd == nullptr)
    {
      throw com::open_ex_t("Opening file.");
    }

    if (!fread(&psf1.magic, sizeof(psf1.magic), 1, fd))
    {
      _magic_fail:
      throw com::read_ex_t("Reading magic bytes.");
    }
    if (psf1.magic == 0x0436)
    {
      type = PSF1;
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
        type = PSF2;
      }
      else
      {
        throw com::read_ex_t("Idnetifying magic bytes.");
      }
    }

    // Read the correct header
    rewind(fd);
    if (!fread(&psf1, type, 1, fd)) // type is the size
    {
      throw com::read_ex_t("Reading header.");
    }
    
    // Just opt for memory priority since it's the safe option
    p = (priority == P_AUTO) ? P_MEMORY : priority;

    if (type == PSF1)
    {
      int chars_n = 256;

      if (psf1.mode)
      {
        throw com::ex_t("Modes are not supported yet!");
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
          throw com::read_ex_t(com::str_t("Bad PSF file. Read") + read + "characters but need" + chars_n + ".");
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
      throw com::ex_t("PSF2 not yet supported.");

      // Make endian readable on this system
      for (unsigned long i = 0; i < sizeof(__array)/sizeof(__array[0]); i++)
      {
        __array[i] = com::lil32(__array[i]);
      }
    }

    printf("gui_open_font(): '%s' has been successfully opened!\n", fp);
  }

  int font_t::get_width()
  {
    return type == PSF1 ? 8 : psf2.width;
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
      fseek(fd, type + index, SEEK_SET);
      if (!fread(data, row_size * height, 1, fd))
      {
        return nullptr;
      }
      return data;
    }
  }
}
