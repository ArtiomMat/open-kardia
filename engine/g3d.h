// 3D Graphics module!!!

#pragma once

#include "fip.h"

// How many bits are used as fraction bits in G3D's fixed point math
#define G3D_DISTANCEB 10
#define G3D_ANGLEB 10

// Fundamental D unit, 1 of it is the smallest measure of distance. Can be thought of as ~cm/1000
typedef int gui_d_t;
typedef gui_d_t gui_d3_t[3];
// Kilo-Distance is 1024*D. Can be tought of as ~cm. Kilo-distance is crucial
typedef short gui_kd_t;
typedef gui_kd_t gui_kd3_t[3];
// Fundamental A unit, 1 of it is the smallest measure of angles.
typedef short gui_a_t;
typedef gui_a_t gui_a3_t;

enum
{
  G3D_CUT_LOOP,
};

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
  gui_d3_t* points; // Contains all the frames of the model, each frame of points must be in the correct order, points cannot be added or removed.
  g3d_tri_t* tris; // Trios of indices that construct triangles
  g3d_cut_t* cuts;
  unsigned short tris_n;
  unsigned char frame_points_n; // How many points per frame
  unsigned char frames_n; // Total frames number
  unsigned char cuts_n;
  unsigned char flags;
} g3d_model_t;

typedef struct
{
  gui_d3_t point;
  gui_a3_t angles; 
} g3d_eye_t;

extern g3d_eye_t g3d_eye;

extern int
g3d_init();
