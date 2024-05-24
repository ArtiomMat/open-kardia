// ! YOUR COMPILER MUST SUPPORT ARITHMETIC SHIFT! CHECK IF IT DOES OR YOU GET UNEXPRECTED RESULTS !
// Fixed point arithmetic HEADER-ONLY module.
// Entirely defined through macros, which allows you to dynamically change the fraction bits number through your source files for different purposes

// NOTE TO SELF: AVOID DOUBLE EVALUATION!!!!!

#pragma once

// PI constant for fip of 30 bits of fraction, 6 first fraction digits 100% accurate.
// translates to 3.1415927410d, PI is ~3.1415926535d.
#define FIP30_PI (3373259426)

#define FIP_PI(frac_bits) (FIP30_PI >> (30 - frac_bits))

#define FIP_MUL(frac_bits,a,b) (((fip_t)(a)*(fip_t)(b)) >> frac_bits)

#define FIP_DIV(frac_bits,A,B) ((fip_t)((((long long)(A)) << frac_bits) / ((long long)(B))))

#define FIP_FRAC(frac_bits,fip) ((fip_t)(fip) & ((1 << frac_bits) - 1))

#define ITOFIP(frac_bits,i) ((fip_t)(i) << frac_bits)

#define FIPTOI(frac_bits,fip) ((fip_t)(fip) >> frac_bits)

// Rounding too
#define FIPTOI_R(frac_bits,fip) \
  ({ fip_t __fip__ = fip; /* Avoid double evaluation */\
    FIPTOI(__fip__) + ((__fip__ & (1 << (frac_bits-1))) ? 1 : 0); \
  })

#define FIPTOF(frac_bits,fip) ((float)(fip) / (1 << frac_bits))

#define FTOFIP(frac_bits,f) ((fip_t)((f) * (1 << frac_bits)))

typedef int fip_t;
