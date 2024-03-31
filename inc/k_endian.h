// Meant to be included in k, just for organization of the header, it's really crowded

#pragma once

#ifdef __x86_64__
  static inline unsigned short
  k_bige16(unsigned short _x)
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
  k_bige32(unsigned int _x)
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
  k_bige64(unsigned long long _x)
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
  k_lile16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  k_lile32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  k_lile64(unsigned long long _x)
  {
    return _x;
  }
#elif
  static inline unsigned short
  k_bige16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  k_bige32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  k_bige64(unsigned long long _x)
  {
    return _x;
  }

  static inline unsigned short
  k_lile16(unsigned short _x)
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
  k_lile32(unsigned int _x)
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
  k_lile64(unsigned long long _x)
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
