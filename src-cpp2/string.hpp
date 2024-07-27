#pragma once

namespace axe
{
  struct string_t
  {
    public:
    char* c; // Be careful when using this, don't screw around.
    int n_real, n;

    string_t(const char* str);
    string_t();

    ~string_t();

    // Remove any extra memory that is taken, essentially make n_real=n
    void optimize();

    string_t& put(const char* x);
    string_t& put(string_t& x);
    string_t& put(char x);
    string_t& put(long long x);
    string_t& put(double x);

    string_t& put(float x) { return put(static_cast<double>(x)); }
    string_t& put(short x) { return put(static_cast<long long>(x)); }
    string_t& put(int x) { return put(static_cast<long long>(x)); }
    string_t& put(unsigned long long x) { return put(static_cast<long long>(x)); }
    string_t& put(unsigned short x) { return put(static_cast<long long>(x)); }
    string_t& put(unsigned int x) { return put(static_cast<long long>(x)); }

    template<typename t_t>
    inline string_t& operator +(t_t x)
    {
      return put(x);
    }
    
    template<typename t_t>
    inline string_t& operator +=(t_t x)
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
}