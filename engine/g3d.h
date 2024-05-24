// 3D Graphics module!!!

// Rule of thumb: distance is measured in centimeter(-like) distance, angles are measured in radians, both use FIP, so 1 is not just =1, it's ITOFIP(G3D_FB, 1)

#pragma once

#include "fip.h"

// How many bits are used as fraction bits in G3D's fixed point math
#define G3D_FB 10

typedef int gui_i3_t[3], gui_i2_t[2], gui_i1_t;
typedef fip_t gui_f3_t[3], gui_f2_t[2], gui_f1_t;

enum
{
  G3D_CUT_LOOP,
};

// A tri contains information about a triangle, made of a trio of indices that indexes a point in points of a model
typedef struct
{
  unsigned char color;
  unsigned char indices[3];
} g3d_tri_t;

// A cut is a piece of animation from the frames
typedef struct
{
  unsigned char start; // First frame
  unsigned char end; // Last frame
  unsigned char flags;
} g3d_cut_t;

typedef struct
{
  gui_f3_t* points; // Contains all the frames of the model, each frame of points must be in the same triangle order, points must be preserved in all frames.
  g3d_tri_t* tris;
  g3d_cut_t* cuts;
  unsigned short tris_n;
  unsigned char frame_points_n; // How many points per frame
  unsigned char frames_n; // Total frames number
  unsigned char cuts_n;
  unsigned char flags;
} g3d_model_t;

typedef struct
{
  gui_f3_t origin;
  gui_f3_t angles;
  gui_f1_t fov;
  gui_f1_t _tg; // Cached tg(fov/2)
} g3d_eye_t;

typedef struct
{
  gui_f3_t origin;
  gui_f3_t angles;
  gui_f1_t fov;
  gui_f1_t brightness; // Can also do anti brightness :D
} g3d_light_t;

extern g3d_eye_t* g3d_eye;

// Requires vid
extern int
g3d_init(g3d_eye_t* initial_eye);

extern void
g3d_free();

extern void
g3d_draw(g3d_model_t* model);

extern int
g3d_open(g3d_model_t* model, const char* fp);
