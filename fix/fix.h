#ifndef HACKTICAL_FIX_H
#define HACKTICAL_FIX_H

#include <stdint.h>

#define HC_FIX_EXP 3
#define HC_FIX_HDR (HC_FIX_EXP+1)

typedef uint64_t hc_fix;

uint32_t hc_scale(uint8_t exp);
hc_fix hc_fix_new(uint8_t exp, int64_t val);

uint8_t hc_fix_exp(hc_fix x);
int64_t hc_fix_val(hc_fix x);
double hc_fix_double(hc_fix x);

hc_fix hc_fix_add(hc_fix x, hc_fix y);
hc_fix hc_fix_sub(hc_fix x, hc_fix y);
hc_fix hc_fix_mul(hc_fix x, hc_fix y);
hc_fix hc_fix_div(hc_fix x, hc_fix y);

int64_t hc_fix_int(hc_fix x);
int64_t hc_fix_frac(hc_fix x);

#endif
