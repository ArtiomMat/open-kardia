// 3D Graphics module!!!

// Rule of thumb: distance is measured in centimeter(-like) distance, angles are measured in radians, both use FIP, so 1 is not just =1, it's ITOFIP(G3D_DB, 1)

#pragma once

#include "fip.h"

// How many bits are used as fraction bits in distances
#define G3D_DB 8
// k*(2^G3D_AB) where k is a natural number is equivalent to 2*k*PI radians.
// Note that the bigger this is the more memory it takes to cache the tan and sin functions.
// This is also the fraction bits for an angle value.
#define G3D_AB 8

// Where the z of the far plane is
#define G3D_FAR_Z ITOFIP(G3D_DB, 256)

typedef int g3d_i3_t[3], g3d_i2_t[2], g3d_i1_t;
typedef fip_t g3d_f3_t[3], g3d_f2_t[2], g3d_f1_t;

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
  g3d_f3_t* points; // Contains all the frames of the model, each frame of points must be in the same triangle order, points must be preserved in all frames.
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
  g3d_f3_t origin;
  g3d_f3_t angles;
  union
  {
    struct
    {
      g3d_model_t* p;
      unsigned char frame;
      unsigned char cut;
    } model;
  };
} g3d_thing_t;

typedef struct
{
  g3d_f3_t origin;
  g3d_f3_t angles;
  g3d_f1_t fov;
  g3d_f1_t _tg; // Cached tg(fov/2)
} g3d_eye_t;

typedef struct
{
  g3d_f3_t origin;
  g3d_f3_t angles;
  g3d_f1_t fov;
  g3d_f1_t brightness; // Can also do anti brightness :D
} g3d_light_t;

extern g3d_eye_t* g3d_eye;

// Requires vid, resolution must be sizeof(long long) divisible.
// If initial_eye is NULL initializes a default eye 0'd out, with FOV of 90
// g3d_set_fov() does not need to be used, it is automatically called on initial_eye using its own fov.
extern int
g3d_init(g3d_eye_t* initial_eye);

// A function is required because some info is also cached
extern void
g3d_set_fov(g3d_eye_t* eye, g3d_f1_t fov);

extern void
g3d_free();

// Wipes the zbuf
extern void
g3d_wipe();

extern void
g3d_draw(g3d_model_t* model);

extern int
g3d_open(g3d_model_t* model, const char* fp);

// The fraction bits as input are AB, and output are DB.
g3d_f1_t
g3d_tan(g3d_f1_t a);
// The fraction bits as input are AB, and output are DB.
g3d_f1_t
g3d_sin(g3d_f1_t a);
// The fraction bits as input are AB, and output are DB.
g3d_f1_t
g3d_cos(g3d_f1_t a);

