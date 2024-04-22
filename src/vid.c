#ifdef __linux__
  #include "nix/vid.c"
#elif
  #include "win/vid.c"
#endif

unsigned char (*vid_colors)[3] = NULL;

unsigned char* vid_pixels = NULL;

int vid_size[2];

int (*vid_on)(vid_event_t*) = vid_def_on;

void
vid_wipe(int color)
{
  for (int i = 0; i < vid_size[1]*vid_size[0]; i++)
  {
    vid_pixels[i*4+2] = vid_colors[color][0];
    vid_pixels[i*4+1] = vid_colors[color][1];
    vid_pixels[i*4+0] = vid_colors[color][2];
  }
}

void
vid_set(unsigned char color, int i)
{
  vid_pixels[i*4+2] = vid_colors[color][0];
  vid_pixels[i*4+1] = vid_colors[color][1];
  vid_pixels[i*4+0] = vid_colors[color][2];
}
