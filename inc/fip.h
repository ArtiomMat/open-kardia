// ! YOUR COMPILER MUST SUPPORT ARITHMETIC SHIFT! CHECK IF IT DOES OR YOU GET UNEXPRECTED RESULTS !
// Fixed point arithmetic HEADER-ONLY module.
// Entirely defined through macros, which allows one to dynamically change the bits through their source files

#pragma once

// How many bits the fraction part has
#ifndef FIP_FRAC_BITS
  #define FIP_FRAC_BITS 8
#endif

typedef int fip_t;

#define fip_mul(a,b) (((fip_t)(a)*(fip_t)(b)) >> FIP_FRAC_BITS)
#define fip_div(A,B) ((fip_t)((((long long)(A)) << FIP_FRAC_BITS) / ((long long)(B))))
#define fip_frac(fip) ((fip_t)(fip) & ((1 << FIP_FRAC_BITS) - 1))
#define itofip(i) ((fip_t)(i) << FIP_FRAC_BITS)
#define fiptoi(fip) ((fip_t)(fip) >> FIP_FRAC_BITS)
// Rounding too
#define fiptoi_r(fip) (fiptoi(fip) + ((fip_t)(fip) & (1 << (FIP_FRAC_BITS-1))))
#define fiptof(fip) ((float)(fip) / (1 << FIP_FRAC_BITS))
#define ftofip(f) ((fip_t)((f) * (1 << FIP_FRAC_BITS)))
