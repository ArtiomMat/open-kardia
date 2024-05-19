// Fixed point arithmetic HEADER-ONLY module, fixed point replaced with floats.

// NOTE TO SELF: AVOID DOUBLE EVALUATION!!!!!

#pragma once

#define FIP_MUL(frac_bits,a,b) (((fip_t)(a)*(fip_t)(b)))

#define FIP_DIV(frac_bits,A,B) ((fip_t)(A) / (long long)(B))

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

typedef float fip_t;
