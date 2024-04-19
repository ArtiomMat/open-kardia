// Graphics API that should be implementable via OpenGL and software
// Builds on top of vid, uses its color palette.

#pragma once

#include "fip.h"

typedef fip_t    gfx_v1_t;
typedef gfx_v1_t gfx_v2_t[2];
typedef gfx_v1_t gfx_v3_t[3];

typedef struct
{
  
} gfx_image_t;

extern void
gfx_init();


extern int
gfx_open_image(gfx_image_t* image, const char* fp);

// Supports GIF and PNG only.
// Converts color palette to the current vid_colors, so make sure to load the vid_colors with the palette you want
// You may also open an image that doesn't exist, creating a new file on closing(or new files depending on if you make it a multi-frame and you open a PNG).
// You may also not specify fp, in which case no file is ascosiated with it.
// Returns 1 if all is ok(file extension is ok and etc).
extern int
gfx_open_image(gfx_image_t* image, const char* fp);

// Writes whatever you did to the file/s.
// Specify NULL for fp to write it to the file opened.
extern void
gfx_save_image(gfx_image_t* image, const char* fp);

