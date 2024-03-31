#include "psf.h"
#include "k.h"

#include <stdio.h>
#include <stdlib.h>

void
psf_close(psf_font_t* f)
{
  free(f->data);
}

int
psf_open(psf_font_t* font, const char* fp, int priority)
{
  FILE* f = fopen(fp, "rb");
  font->type = 0;

  if (!f)
  {
    printf("psf_open(): File '%s' does not exist.\n", fp);
    return 0;
  }

  fread(&font->psf1.magic, sizeof(font->psf1.magic), 1, f);
  if (font->psf1.magic == 0x0436)
  {
    font->type = sizeof(struct psf1_s);
  }
  else
  {
    rewind(f);
    fread(&font->psf2.magic, sizeof(font->psf2.magic), 1, f);
   
    if (font->psf2.magic == 0x864ab572)
    {
      font->type = sizeof(struct psf2_s);
    }
    else
    {
      printf("psf_open(): Unknown magic bytes for '%s'.\n", fp);
      return 0;
    }
  }

  rewind(f);
  fread(&font->psf1, font->type, 1, f); // font->type is the size

  // Just opt for memory priority since it's the safe option
  font->p = (priority == PSF_P_AUTO) ? PSF_P_MEMORY : priority;

  if (font->type == sizeof(struct psf1_s))
  {
    if (font->psf1.mode)
    {
      printf("psf_open(): Currently modes aren't supported, '%s'.\n", fp);
      return 0;
    }

    font->row_size = font->psf1.size/8;
    if (font->psf1.size > font->row_size*8) // Check for a rounding error and add padding
    {
      font->row_size++;
    }

    int data_size = font->row_size*font->psf1.size;
    // If we prioritize speed we will read all the characters, otherwise we read one for each glyph we get
    if (font->p == PSF_P_SPEED)
    {
      data_size *= 256;
    }

    font->data = malloc(data_size);

    if (font->p == PSF_P_SPEED) // THen we already want to read
    {
      int read;
      if ((read = fread(font->data, 0, data_size, f)) != data_size)
      {
        printf("psf_open(): '%s' is a bad PSF file. Read %d but need %d\n", fp, read, data_size);
        return 0;
      }
      printf("%d\n\n\n", fgetc(f) == EOF);
    }
  }
  else
  {
    printf("psf_open(): PSF2 not supported currently, '%s'.\n", fp);
    return 0;

    // Make endian readable on this system
    for (int i = 0; i < sizeof(font->__array)/sizeof(font->__array[0]); i++)
    {
      font->__array[i] = k_lile32(font->__array[i]);
    }
  }

  return 1;
}
