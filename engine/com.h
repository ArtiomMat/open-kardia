// Common functionality module, a m
#pragma once

#include <stdint.h>

#if INTPTR_MAX != INT64_MAX
  #error Only 64 bit systems.
#endif

#define COM_PATH_SIZE 512

// Little endian
#ifdef __x86_64__
  #define COM_LILE 1

  static inline unsigned short
  com_big16(unsigned short _x)
  {
    union
    {
      struct
      {
        unsigned short i;
        unsigned char a[2];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[1];
    y.a[1] = x.a[0];

    return y.i;
  }
  static inline unsigned int
  com_big32(unsigned int _x)
  {
    union
    {
      struct
      {
        unsigned int i;
        unsigned char a[4];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[3];
    y.a[1] = x.a[2];
    y.a[2] = x.a[1];
    y.a[3] = x.a[0];

    return y.i;
  }
  static inline unsigned long long
  com_big64(unsigned long long _x)
  {
    union
    {
      struct
      {
        unsigned long long i;
        unsigned char a[8];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[7];
    y.a[1] = x.a[6];
    y.a[2] = x.a[5];
    y.a[3] = x.a[4];
    y.a[4] = x.a[3];
    y.a[5] = x.a[2];
    y.a[6] = x.a[1];
    y.a[7] = x.a[0];

    return y.i;
  }

  static inline unsigned short
  com_lil16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  com_lil32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  com_lil64(unsigned long long _x)
  {
    return _x;
  }
#elif __arm__
  #define COM_BIGE 1

  static inline unsigned short
  com_big16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  com_big32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  com_big64(unsigned long long _x)
  {
    return _x;
  }

  static inline unsigned short
  com_lil16(unsigned short _x)
  {
    union
    {
      struct
      {
        unsigned short i;
        unsigned char a[2];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[1];
    y.a[1] = x.a[0];

    return y.i;
  }
  static inline unsigned int
  com_lil32(unsigned int _x)
  {
    union
    {
      struct
      {
        unsigned int i;
        unsigned char a[4];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[3];
    y.a[1] = x.a[2];
    y.a[2] = x.a[1];
    y.a[3] = x.a[0];

    return y.i;
  }
  static inline unsigned long long
  com_lil64(unsigned long long _x)
  {
    union
    {
      struct
      {
        unsigned long long i;
        unsigned char a[8];
      };
    } x, y;
    x.i = _x;

    y.a[0] = x.a[7];
    y.a[1] = x.a[6];
    y.a[2] = x.a[5];
    y.a[3] = x.a[4];
    y.a[4] = x.a[3];
    y.a[5] = x.a[2];
    y.a[6] = x.a[1];
    y.a[7] = x.a[0];

    return y.i;
  }
#endif

static inline long long
max(long long a, long long b)
{
  return a > b ? a : b;
}

static inline long long
min(long long a, long long b)
{
  return a > b ? b : a;
}

// The directory in which the executable is located
extern char com_dir[COM_PATH_SIZE];

extern const char** com_args;
extern int com_args_n;

// For internal use
extern int _com_dir_end;
// For internal use
extern int
_com_init_dir();

// Some COM functionality depends on the initialization!
// Returns 0 if something failed, for instance getting com_dir, otherwise 1.
extern int
com_init(int args_n, const char** args);
extern int
com_arg(const char* arg);
// Returns a string to a path relative to the executable file.
// Returns NULL if the path became too large.
// NOT THREAD SAFE IT USES ONE BUFFER ONLY!
extern const char*
com_relfp(const char* p);

// typedef struct com_node_s
// {
//   struct com_node_s *r, *l;
//   union
//   {
//     void* p;
//     unsigned long long u;
//     long long i;
//   };
// } com_node_t;

// extern com_node_t*
// com_init_node(void* p);
// // Pushes so that left->next = n(and other stuff for safety).
// // If left is NULL returns n, otherwise returns left. This is incase you want to make a sort of list, and have a variable called "nodes" or some shit, and it starts out as NULL because there are no "nodes".
// extern com_node_t*
// com_push_node(com_node_t* left, com_node_t* n);
// // Returns the left node to n, NULl if there was none.
// extern com_node_t*
// com_pull_node(com_node_t* n);
// // Also calls com_pull_node(), essentially all it adds is a free() call, so you can do it too if you want to avoid double calling of pull.
// // Returns what pull returns.
// extern com_node_t* 
// com_free_node(com_node_t* n);
// // Free all nodes, NULL can be put in place ofn
// extern void
// com_free_nodes(com_node_t* n);
// // Return the left most node in the list
// extern com_node_t*
// com_leftest_node(com_node_t* n);
// // Return the right most node in the list
// extern com_node_t*
// com_rightest_node(com_node_t* n);
