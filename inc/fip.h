// Fixed point arithmetic module.

#pragma once

#ifdef __GNUC__
  #define SAL <<FIP_FRAC_BITS
  #define SAR >>FIP_FRAC_BITS
#else
  #define SAL *(1<<FIP_FRAC_BITS)
  #define SAR /(1<<FIP_FRAC_BITS)
#endif

// How many bits the fraction part has
#define FIP_FRAC_BITS 8

typedef int fip_t;

static inline int
fip_mul(fip_t a, fip_t b)
{
  return (a * b) SAR; // Not 100% sure why only shifting right works, my math is bad...
}

static inline int
fip_div(fip_t a, fip_t b) 
{
  if (a == 0)
  {
    return 0;
  }
  if (a < b)
  {
    long long A = a;
    long long B = b;
    return ((A SAL)|B)/B;
  }
  return (a / b) SAL;
}

// Get the fraction part
static inline int
fip_frac(fip_t fip)
{
  return fip & ((1 << FIP_FRAC_BITS) - 1);
}

// Always rounds down
static inline int
fiptoi(fip_t fip)
{
  return fip SAR;
}

// Unlike fiptoi it rounds it to bigger or lower value, a tiny itsy bit more expensive
static inline int
fip_round(fip_t fip)
{
  // Bitwise magic to determine if the fraction is >=0.5
  const int half = (1 << (FIP_FRAC_BITS-1));
  int frac_mask = (fip & ((1 << FIP_FRAC_BITS)-1));
  int add = frac_mask >= half;
  
  return fiptoi(fip) + add;
}

static inline fip_t
itofip(int i)
{
  return i SAL;
}

// Just for debugging shouldn't be relied on
static inline float
fiptof(fip_t fip)
{
  // int frac_mask = (fip & ((1  SAL)-1));
  // return fiptoi(fip) + (1.0f * (fip & frac_mask) / (1  SAL));
  return (float)(fip) / (1 SAL);
}
// Just for debugging shouldn't be relied on
static inline fip_t
ftofip(float f)
{
  return (int)(f * (1 SAL));
}

#undef SAL
#undef SAR

