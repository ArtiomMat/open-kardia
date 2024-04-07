#include "psf.h"
#include "k.h"

#ifdef PSF_X_KARDIA
  #include "vid.h"
#endif

#include <stdio.h>
#include <stdlib.h>

void
psf_close(psf_font_t* f)
{
  free(f->data);
  fclose(f->fd);
}

int
psf_open(psf_font_t* font, const char* fp, int priority)
{
  font->fd = fopen(fp, "rb");
  font->type = 0;

  if (font->fd == NULL)
  {
    printf("psf_open(): File '%s' does not exist.\n", fp);
    return 0;
  }

  fread(&font->psf1.magic, sizeof(font->psf1.magic), 1, font->fd);
  if (font->psf1.magic == 0x0436)
  {
    font->type = PSF1;
  }
  else
  {
    rewind(font->fd);
    fread(&font->psf2.magic, sizeof(font->psf2.magic), 1, font->fd);
   
    if (font->psf2.magic == 0x864ab572)
    {
      font->type = PSF2;
    }
    else
    {
      printf("psf_open(): Unknown magic bytes for '%s'.\n", fp);
      return 0;
    }
  }

  // Read the correct header
  rewind(font->fd);
  fread(&font->psf1, font->type, 1, font->fd); // font->type is the size

  // Just opt for memory priority since it's the safe option
  font->p = (priority == PSF_P_AUTO) ? PSF_P_MEMORY : priority;

  if (font->type == PSF1)
  {
    int chars_n = 256;

    if (font->psf1.mode)
    {
      printf("psf_open(): Currently modes aren't supported, '%s'.\n", fp);
      return 0;
    }

    // Setup row size, psf1 width is always 8 pixels
    font->row_size = 1;
    font->height = font->psf1.size;
    int char_size = font->row_size*font->psf1.size;

    // If we prioritize speed we will read all the characters, otherwise we read one for each glyph we get
    if (font->p == PSF_P_SPEED)
    {

      font->data = malloc(chars_n * char_size);

      int read;
      if ((read = fread(font->data, char_size, chars_n, font->fd)) != chars_n)
      {
        printf("psf_open(): '%s' is a bad PSF file. Read %d characters but need %d.\n", fp, read, chars_n);
        psf_close(font);
        return 0;
      }

      // We don't need it anymore
      fclose(font->fd);
      font->fd = NULL;
    }
    else // PSF_P_MEMORY
    {
      font->data = malloc(char_size);
    }
  }
  else
  {
    printf("psf_open(): PSF2 not supported currently, '%s'.\n", fp);
    fclose(font->fd);
    return 0;

    // Make endian readable on this system
    for (int i = 0; i < sizeof(font->__array)/sizeof(font->__array[0]); i++)
    {
      font->__array[i] = k_lile32(font->__array[i]);
    }
  }

  printf("psf_open(): '%s' has been successfully opened!\n", fp);
  return 1;
}


void*
psf_get_glyph(psf_font_t* font, int g)
{
  int index =  g * font->row_size * font->height;
  if (font->p == PSF_P_SPEED)
  {
    return &((char*)font->data)[index];
  }
  else
  {
    // TODO: Make it depend on flags too
    fseek(font->fd, font->type + index, SEEK_SET);
    fread(font->data, 1, font->row_size * font->height, font->fd);
    return font->data;
  }
}