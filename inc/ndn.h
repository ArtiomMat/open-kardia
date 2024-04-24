// Endian 

#pragma once

#ifdef __x86_64__
  #define NDN_BIG 0
  #define NDN_LITTLE 1

  static inline unsigned short
  ndn_b16(unsigned short _x)
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
  ndn_b32(unsigned int _x)
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
  ndn_b64(unsigned long long _x)
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
  ndn_l16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  ndn_l32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  ndn_l64(unsigned long long _x)
  {
    return _x;
  }
#elif
  #define NDN_BIG 1
  #define NDN_LITTLE 0

  static inline unsigned short
  ndn_b16(unsigned short _x)
  {
    return _x;
  }
  static inline unsigned int
  ndn_b32(unsigned int _x)
  {
    return _x;
  }
  static inline unsigned long long
  ndn_b64(unsigned long long _x)
  {
    return _x;
  }

  static inline unsigned short
  ndn_l16(unsigned short _x)
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
  ndn_l32(unsigned int _x)
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
  ndn_l64(unsigned long long _x)
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
