// ! YOUR COMPILER MUST SUPPORT ARITHMETIC SHIFT! CHECK IF IT DOES OR YOU GET UNEXPRECTED RESULTS !
// Fixed point arithmetic HEADER-ONLY module.
// Entirely defined through macros, which allows you to dynamically change the fraction bits number through your source files for different purposes

// NOTE TO SELF: AVOID DOUBLE EVALUATION!!!!!

#pragma once

#define FIP_DEF_FRAC_BITS 10

// How many bits the fraction part has
#ifndef FIP_FRAC_BITS
  #define FIP_FRAC_BITS FIP_DEF_FRAC_BITS
#endif

typedef int x ## FIP_DEF_FRAC_BITS ## _t;

#define FIP_MUL(a,b) (((fip_t)(a)*(fip_t)(b)) >> FIP_FRAC_BITS)

#define FIP_DIV(A,B) ((fip_t)((((long long)(A)) << FIP_FRAC_BITS) / ((long long)(B))))

#define FIP_FRAC(fip) ((fip_t)(fip) & ((1 << FIP_FRAC_BITS) - 1))

#define ITOFIP(i) ((fip_t)(i) << FIP_FRAC_BITS)

#define FIPTOI(fip) ((fip_t)(fip) >> FIP_FRAC_BITS)

// Rounding too
#define FIPTOI_R(fip) \
  ({ fip_t __fip__ = fip; /* Avoid double evaluation */\
    FIPTOI(__fip__) + ((__fip__ & (1 << (FIP_FRAC_BITS-1))) ? 1 : 0); \
  })

#define FIPTOF(fip) ((float)(fip) / (1 << FIP_FRAC_BITS))

#define FTOFIP(f) ((fip_t)((f) * (1 << FIP_FRAC_BITS)))

typedef int fip_t;
