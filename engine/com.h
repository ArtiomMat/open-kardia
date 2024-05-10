// Common functionality module, a m

#pragma once

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
#elif
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

