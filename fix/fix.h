#ifndef HACKTICAL_FIX_H
#define HACKTICAL_FIX_H

#include <stdint.h>

#define HC_FIX_EXP 3
#define HC_FIX_HDR (HC_FIX_EXP+1)

#define HC_FIX_MAX_EXP 7

struct hc_stream;

typedef uint64_t hc_fix_t;

uint32_t hc_scale(uint8_t exp);
hc_fix_t hc_fix(uint8_t exp, int64_t val);

uint8_t hc_fix_exp(hc_fix_t x);
int64_t hc_fix_val(hc_fix_t x);
double hc_fix_double(hc_fix_t x);

hc_fix_t hc_fix_add(hc_fix_t x, hc_fix_t y);
hc_fix_t hc_fix_sub(hc_fix_t x, hc_fix_t y);
hc_fix_t hc_fix_mul(hc_fix_t x, hc_fix_t y);
hc_fix_t hc_fix_div(hc_fix_t x, hc_fix_t y);

int64_t hc_fix_int(hc_fix_t x);
int64_t hc_fix_frac(hc_fix_t x);

void hc_fix_print(const hc_fix_t v, struct hc_stream *out);

#endif
