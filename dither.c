#include <stdlib.h>
#include <stdio.h>
#include <png.h>

#define OUT_DEPTH 1
#define OUT_COLOR_TYPE PNG_COLOR_TYPE_GRAY

static int width, height;

int
get_p_for_v(int x, int y, int v)
{
  // const int dither_size = 3;
  // const short dithers[] = {
  //   0b000000000,
  //   0b000010000,
  //   0b001000100,
  //   0b001010100,
  //   0b010101010,

  //   0b010111010,
    
  //   ~0b010101010,
  //   ~0b001010100,
  //   ~0b001000100,
  //   ~0b000010000,
  //   ~0b000000000,
  // };

  const int dither_size = 2;
  const char dithers[] = {
    0b0000,
    0b1000,
    0b1001,
    0b0111,
    0b1111,
  };
  const int dithers_n = (sizeof(dithers)/sizeof(dithers[0]));

  int dither_i = (dithers_n - 1) * v / 255;
  // int dither_i = 3;
  int bit = x%dither_size + dither_size*(y%dither_size);

  return (dithers[dither_i] & (1<<bit)) ? 1 : 0;
}

int
main(int args_n, const char** args)
{
  if (args_n < 3)
  {
    printf("Usage: %s <in.png> <out.png>\n", args[0]);
    return 1;
  }

  FILE* in =  fopen(args[1], "rb");
  FILE* out = fopen(args[2], "wb");

  if (in == NULL)
  {
    puts("Input not found.");
    return 1;
  }

  png_structp in_png, out_png;
  png_infop in_info, out_info;

  in_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  out_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (in_png == NULL || out_png == NULL)
  {
    puts("Failed to create png structs.");
    return 1;
  }

  // Input setup
  in_info = png_create_info_struct(in_png);
  png_init_io(in_png, in);
  png_read_info(in_png, in_info);
  
  width = png_get_image_width(in_png, in_info);
  height = png_get_image_height(in_png, in_info);
  
  png_byte in_color_type = png_get_color_type(in_png, in_info);

  // HOPEFULLY THIS IS ALL TO CONVERT THE INPUT INTO A PURE GRAYSCALE IMAGE.
  png_set_strip_alpha(in_png);
  if (in_color_type == PNG_COLOR_TYPE_PALETTE)
  {
    png_set_palette_to_rgb(in_png);
  }
  png_set_rgb_to_gray_fixed(in_png, 1, -1, -1);
  png_set_expand_gray_1_2_4_to_8(in_png);

  png_read_update_info(in_png, in_info);

  // Output setup
  out_info = png_create_info_struct(out_png);
  png_init_io(out_png, out);
  png_set_IHDR(
    out_png,
    out_info,
    width, height,
    1,
    PNG_COLOR_TYPE_GRAY,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(out_png, out_info);

  int in_row_n = png_get_rowbytes(in_png, in_info);
  int out_row_n = png_get_rowbytes(out_png, out_info);
  
  if (in_row_n != width && out_row_n != width / 8)
  {
    puts("Failed to do conversion of images to correct formats, this is my fault. Try a different format of the input image...");
  }

  png_byte in_row[in_row_n];
  png_byte out_row[out_row_n];

  for (int y = 0; y < height; y++)
  {
    png_read_row(in_png, in_row, NULL);
    
    for (int x = 0; x < width; x++)
    {
      int out_row_i = x / 8;

      int p = get_p_for_v(x, y, in_row[x]);
      int shift = 7 - (x % 8);

      p <<= shift;

      out_row[out_row_i] &= ~(1<<shift); // Zero out the bit first
      out_row[out_row_i] |= p;
    }
    png_write_row(out_png, out_row);
  }
  
  png_write_end(out_png, NULL);

  fclose(in);
  fclose(out);

  return 0;
}

