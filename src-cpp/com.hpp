// Common functionality module, a m
#pragma once

#include <cstdint>

#if INTPTR_MAX != INT64_MAX
  #error Only 64 bit systems are supported.
#endif

#ifdef PARANOID
#include <cstdio>
// Paranoid mesage
#define COM_PARANOID_M(msg) \
    do { \
      fprintf(stderr, "PARANOIAC(MSG): %s\n", msg); \
    } while (0)
// Paranoid assersion
#define COM_PARANOID_A(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "PARANOIAC(FAT): %s\n", msg); \
            abort(); \
        } \
    } while (0)
#else
#define COM_PARANOID_M(msg) do { } while (0)
#define COM_PARANOID_A(cond, msg) do { } while (0)
#endif

namespace com
{
  // Little endian
  #ifdef __x86_64__
    #define COM_LILE 1

    // TODO: CONSTEXPR, but it will require a restructure of the function as "x,y are not initialized when defined"

    static inline std::uint16_t
    big16(std::uint16_t _x)
    {
      union
      {
        std::uint16_t i;
        std::uint8_t a[2];
      } x, y;
      x.i = _x;

      y.a[0] = x.a[1];
      y.a[1] = x.a[0];
      
      return y.i;
    }
    static inline std::uint32_t
    big32(std::uint32_t _x)
    {
      union
      {
        std::uint32_t i;
        std::uint8_t a[4];
      } x, y;
      x.i = _x;

      y.a[0] = x.a[3];
      y.a[1] = x.a[2];
      y.a[2] = x.a[1];
      y.a[3] = x.a[0];

      return y.i;
    }
    static inline std::uint64_t
    big64(std::uint64_t _x)
    {
      union
      {
        std::uint64_t i;
        std::uint8_t a[8];
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

    static inline std::uint16_t
    lil16(std::uint16_t _x)
    {
      return _x;
    }
    static inline std::uint32_t
    lil32(std::uint32_t _x)
    {
      return _x;
    }
    static inline std::uint64_t
    lil64(std::uint64_t _x)
    {
      return _x;
    }
  #elif __arm__
    #define COM_BIGE 1

    static inline std::uint16_t
    big16(std::uint16_t _x)
    {
      return _x;
    }
    static inline std::uint32_t
    big32(std::uint32_t _x)
    {
      return _x;
    }
    static inline std::uint64_t
    big64(std::uint64_t _x)
    {
      return _x;
    }

    static inline std::uint16_t
    lil16(std::uint16_t _x)
    {
      union
      {
        std::uint16_t i;
        std::uint8_t a[2];
      } x, y;
      x.i = _x;

      y.a[0] = x.a[1];
      y.a[1] = x.a[0];

      return y.i;
    }
    static inline std::uint32_t
    lil32(std::uint32_t _x)
    {
      union
      {
        std::uint32_t i;
        std::uint8_t a[4];
      } x, y;
      x.i = _x;

      y.a[0] = x.a[3];
      y.a[1] = x.a[2];
      y.a[2] = x.a[1];
      y.a[3] = x.a[0];

      return y.i;
    }
    static inline std::uint64_t
    lil64(std::uint64_t _x)
    {
      union
      {
        std::uint64_t i;
        std::uint8_t a[8];
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

  constexpr int PATH_SIZE = 512;

  struct str_t
  {
    public:
    char* c; // Be careful when using this, don't screw around.
    int n_real, n;

    str_t(const char* str);
    str_t();

    ~str_t();

    str_t& put(const char* x);
    str_t& put(str_t& x);
    str_t& put(char x);
    str_t& put(long long x);
    str_t& put(double x);

    str_t& put(float x) { return put(static_cast<double>(x)); }
    str_t& put(short x) { return put(static_cast<long long>(x)); }
    str_t& put(int x) { return put(static_cast<long long>(x)); }
    str_t& put(unsigned long long x) { return put(static_cast<long long>(x)); }
    str_t& put(unsigned short x) { return put(static_cast<long long>(x)); }
    str_t& put(unsigned int x) { return put(static_cast<long long>(x)); }

    template<typename t_t>
    inline str_t& operator +(t_t x)
    {
      return put(x);
    }

    inline operator const char*()
    {
      return c;
    }

    inline char get(int i)
    {
      return c[i];
    }

    // Returns -1 if not found.
    int find(const char* substr) { return find(substr, 0, n); }
    // Returns -1 if not found.
    int find(const char* substr, int from) { return find(substr, from, n); }
    // Returns -1 if not found.
    int find(const char* substr, int from, int to);
  };

  static inline constexpr long long
  max(long long a, long long b)
  {
    return a > b ? a : b;
  }

  static inline constexpr long long
  min(long long a, long long b)
  {
    return a > b ? b : a;
  }

  // The directory in which the executable is located
  extern char _global_dir[PATH_SIZE];

  extern const char** args;
  extern int args_n;

  // For internal use
  extern int _dir_end;
  
  extern bool initialized;

  // For internal use, this is the cross platform part of initialize()
  bool _initialize2();

  // Some COM functionality depends on the initialization!
  // Returns 0 if something failed, for instance getting com::dir, otherwise 1.
  bool initialize(int args_n, const char** args);
  void shutdown();
  
  int arg(const char* arg);
  // Returns a string to a path relative to the executable file.
  // Returns NULL if the path became too large.
  // Thread safe, we have a buffer we return per thread!
  const char* relfp(const char* p);

  // Get size of clipboard
  int get_cb_size();
  // Read the clipboard into a buffer, the buffer is guaranteed to be null terminated
  int get_cb(char* get_cb, int max_size);
  // Write a buffer into the clipboard
  int set_cb(const char* set_cb, int size);

}