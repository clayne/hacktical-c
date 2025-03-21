#include <assert.h>
#include "fix.h"
#include "macro/macro.h"

uint32_t hc_scale(uint8_t exp) {
  static const uint32_t scale[HC_FIX_MAX_EXP+1] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000};

  assert(exp <= HC_FIX_MAX_EXP);
  return scale[exp];
}

hc_fix hc_fix_new(uint8_t exp, int64_t val) {
  return (hc_fix)hc_bitm(exp, HC_FIX_EXP) +
    (hc_fix)(((val < 0) ? 1 : 0) << HC_FIX_EXP) +
    (hc_fix)(hc_abs(val) << HC_FIX_HDR);
}

uint8_t hc_fix_exp(hc_fix x) {
  return hc_bitm(x, HC_FIX_EXP);
}

int64_t hc_fix_val(hc_fix x) {
  const int64_t v = x >> HC_FIX_HDR;
  return ((x >> HC_FIX_EXP) & 1) ? -v : v;
}

int64_t hc_fix_int(hc_fix x) {
  return hc_fix_val(x) / hc_scale(hc_fix_exp(x));
}

int64_t hc_fix_frac(hc_fix x) {
  const int64_t xv = hc_fix_val(x);
  const uint32_t xs = hc_scale(hc_fix_exp(x));
  return xv - (xv / xs) * xs;
}

double hc_fix_double(hc_fix x) {
  return hc_fix_val(x) / (double)hc_scale(hc_fix_exp(x));
}

hc_fix hc_fix_add(hc_fix x, hc_fix y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix_new(xe, hc_fix_val(x) + hc_fix_val(y));
  }
    
  return hc_fix_new(xe, hc_fix_val(x) +
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}

hc_fix hc_fix_sub(hc_fix x, hc_fix y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix_new(xe, hc_fix_val(x) - hc_fix_val(y));
  }

  return hc_fix_new(xe, hc_fix_val(x) -
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}

hc_fix hc_fix_mul(hc_fix x, hc_fix y) {
  return hc_fix_new(hc_fix_exp(x), hc_fix_val(x) *
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}

hc_fix hc_fix_div(hc_fix x, hc_fix y) {
  return hc_fix_new(hc_fix_exp(x), hc_fix_val(x) /
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}
