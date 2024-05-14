// ! YOUR COMPILER MUST SUPPORT ARITHMETIC SHIFT! CHECK IF IT DOES OR YOU GET UNEXPRECTED RESULTS !
// Fixed point arithmetic HEADER-ONLY base header.
// Entirely defined through macros, which allows you to dynamically change the fraction bits number through your source files for different purposes

// NOTE TO SELF: AVOID DOUBLE EVALUATION!!!!!

#pragma once

typedef int fipany_t;

#define FIP_MUL(a,b,frac_bits) (((fipany_t)(a)*(fipany_t)(b)) >> frac_bits)

#define FIP_DIV(A,B,frac_bits) ((fipany_t)((((long long)(A)) << frac_bits) / ((long long)(B))))

#define FIP_FRAC(fip,frac_bits) ((fipany_t)(fip) & ((1 << frac_bits) - 1))

#define ITOFIP(i,frac_bits) ((fipany_t)(i) << frac_bits)

#define FIPTOI(fip,frac_bits) ((fipany_t)(fip) >> frac_bits)

// Rounding too
#define FIPTOI_R(fip,frac_bits) \
  ({ fipany_t __fip__ = fip; /* Avoid double evaluation */\
    FIPTOI(__fip__) + ((__fip__ & (1 << (frac_bits-1))) ? 1 : 0); \
  })

#define FIPTOF(fip,frac_bits) ((float)(fip) / (1 << frac_bits))

#define FTOFIP(f,frac_bits) ((fipany_t)((f) * (1 << frac_bits)))
