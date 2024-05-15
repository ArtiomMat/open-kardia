// 3D Graphics module!!!

#pragma once

#include "fip.h"

typedef fip_t fip3_t[3];

enum
{
  G3D_SCENE_LOOP,
};

typedef struct
{
  unsigned char color;
  unsigned char indices[3];
} g3d_tri_t;

typedef struct
{
  unsigned char start; // First frame
  unsigned char end; // Last frame
  unsigned char flags;
} g3d_scene_t;

typedef struct
{
  fip3_t* points; // Contains all the frames of the model, each frame of points must be in the correct order, points cannot be added or removed.
  g3d_tri_t* tris; // Trios of indices that construct triangles
  g3d_scene_t* scenes;
  unsigned short tris_n;
  unsigned char frame_points_n; // How many points per frame
  unsigned char frames_n; // Total frames number
  unsigned char scenes_n;
  unsigned char flags;
} g3d_model_t;

