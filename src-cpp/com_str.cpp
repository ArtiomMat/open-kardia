#include "com.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace com
{
  str_t::str_t(const char* str)
  {
    n_real = strlen(str) + 1;
    n = n_real - 1;
    c = static_cast<char*>(malloc(n_real));
    memcpy(c, str, n_real);
  }
  str_t::str_t()
  {
    n = 0;
    n_real = 4;
    c = static_cast<char*>(malloc(n_real));
    c[0] = 0;
  }

  str_t::~str_t()
  {
    free(c);
  }

  str_t& str_t::put(str_t& x)
  {
    return put(x.c);
  }
  str_t& str_t::put(const char* x)
  {
    for (int i = 0; x[i]; i++, n++)
    {
      if (n >= n_real)
      {
        n_real *= 2;
        c = static_cast<char*>(realloc(c, n_real));
      }
      c[n] = x[i];
    }
    
    c[n] = 0;

    return *this;
  }
  str_t& str_t::put(char x)
  {
    if (n >= n_real)
    {
      n_real *= 2;
      c = static_cast<char*>(realloc(c, n_real));
    }
    c[n++] = x;
    c[n] = 0;

    return *this;
  }
  str_t& str_t::put(long long x)
  {
    char str[64];
    lltoa(x, str, 64);
    return put(str);
  }
  str_t& str_t::put(double x)
  {
    char str[64];
    snprintf(str, 64, "%.3lf", x);
    return put(str);
  }

  int str_t::find(const char* substr, int from, int to)
  {
    for (int i = from; i < min(to, n-1); i++)
    {
      int j;
      int i_save = i;
      for (j = 0; substr[j] == c[i]; j++, i++)
      {
        // Because what if substr[j] is malicious with more null terminators c[n+1] also is 0, we segfault. BTW if !c[i] then also !substr[i].
        if (!c[i])
        {
          return i_save;
        }
      }

      if (!substr[j]) // If substr ended we definately found the substring
      {
        return i_save;
      }
    }

    return -1;
  }
}