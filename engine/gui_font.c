#include "gui.h"
#include "com.h"

#include <stdio.h>
#include <stdlib.h>

enum
{
  PSF1_MODE512 = 1,
  PSF1_MODEHASTAB = 2,
  PSF1_MODESEQ = 4,
  PSF2_HAS_UNICODE_TABLE = 1,
};
enum
{
  PSF1 = sizeof (gui_psf1_t),
  PSF2 = sizeof (gui_psf2_t),
};

void
gui_close_font(gui_font_t* f)
{
  free(f->data);
  fclose(f->fd);
}

int
gui_open_font(gui_font_t* font, const char* fp, int priority)
{
  font->fd = fopen(fp, "rb");
  font->type = 0;

  if (font->fd == NULL)
  {
    printf("gui_open_font(): File '%s' does not exist.\n", fp);
    return 0;
  }

  if (!fread(&font->psf1.magic, sizeof(font->psf1.magic), 1, font->fd))
  {
    _magic_fail:
    fprintf(stderr, "gui_open_font(): Failed to read magic bytes of '%s'.\n", fp);
    return 0;
  }
  if (font->psf1.magic == 0x0436)
  {
    font->type = PSF1;
  }
  else
  {
    rewind(font->fd);
    if (!fread(&font->psf2.magic, sizeof(font->psf2.magic), 1, font->fd))
    {
      goto _magic_fail;
    }
   
    if (font->psf2.magic == 0x864ab572)
    {
      font->type = PSF2;
    }
    else
    {
      printf("gui_open_font(): Unknown magic bytes for '%s'.\n", fp);
      return 0;
    }
  }

  // Read the correct header
  rewind(font->fd);
  if (!fread(&font->psf1, font->type, 1, font->fd)) // font->type is the size
  {
    fprintf(stderr, "gui_open_font(): Failed to read header of '%s'.\n", fp);
    return 0;
  }
  
  // Just opt for memory priority since it's the safe option
  font->p = (priority == GUI_FONTP_AUTO) ? GUI_FONTP_MEMORY : priority;

  if (font->type == PSF1)
  {
    int chars_n = 256;

    if (font->psf1.mode)
    {
      printf("gui_open_font(): Currently modes aren't supported, '%s'.\n", fp);
      return 0;
    }

    // Setup row size, psf1 width is always 8 pixels
    font->row_size = 1;
    font->height = font->psf1.size;
    int char_size = font->row_size*font->psf1.size;

    // If we prioritize speed we will read all the characters, otherwise we read one for each glyph we get
    if (font->p == GUI_FONTP_SPEED)
    {

      font->data = malloc(chars_n * char_size);

      int read;
      if ((read = fread(font->data, char_size, chars_n, font->fd)) != chars_n)
      {
        printf("gui_open_font(): '%s' is a bad PSF file. Read %d characters but need %d.\n", fp, read, chars_n);
        gui_close_font(font);
        return 0;
      }

      // We don't need it anymore
      fclose(font->fd);
      font->fd = NULL;
    }
    else // GUI_FONTP_MEMORY
    {
      font->data = malloc(char_size);
    }
  }
  else
  {
    printf("gui_open_font(): PSF2 not supported currently, '%s'.\n", fp);
    fclose(font->fd);
    return 0;

    // Make endian readable on this system
    for (int i = 0; i < sizeof(font->__array)/sizeof(font->__array[0]); i++)
    {
      font->__array[i] = com_lil32(font->__array[i]);
    }
  }

  printf("gui_open_font(): '%s' has been successfully opened!\n", fp);
  return 1;
}

int
gui_get_font_width(gui_font_t* f)
{
  return f->type == PSF1 ? 8 : f->psf2.width;
}

void*
gui_get_glyph(gui_font_t* font, int g)
{
  // TODO: This is hard coded to strictly work with ascii, gotta improve.
  if (g < 0)
  {
    g = 256 + g;
  }

  int index =  g * font->row_size * font->height;
  if (font->p == GUI_FONTP_SPEED)
  {
    return &((char*)font->data)[index];
  }
  else
  {
    // TODO: Make it depend on flags too
    fseek(font->fd, font->type + index, SEEK_SET);
    if (!fread(font->data, font->row_size * font->height, 1, font->fd))
    {
      return NULL;
    }
    return font->data;
  }
}

void
gui_draw_font(gui_font_t* f, gui_u_t _x, gui_u_t _y, int g, unsigned char color)
{
  int add_x = _x < 0 ? -_x : 0;
  int add_y = _y < 0 ? -_y : 0;

  int width = gui_get_font_width(f);
  char* glyph = gui_get_glyph(f, g);

  int padding = width % 8;

  // b is the bit index, it goes through glyph as a whole as if it were a bit buffer
  int b = f->row_size * add_y;
  for (int y = _y + add_y; y < f->height + _y && y < vid_size[1]; y++)
  {
    b += add_x;

    for (int x = _x + add_x; x < width + _x && x < vid_size[0]; x++, b++)
    {
      char byte = glyph[b >> 3];
      
      // We just shift the byte left by b%8(to get the current bit) and just check if that lsb is on.
      // We do it from left to right because that's how we draw
      if ((byte << (b % 8)) & (1 << 7))
      {
        vid_set(color, x + y * vid_size[0]);
      }
    }

    b += padding;
  }
}
