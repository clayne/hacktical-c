#include <assert.h>
#include <inttypes.h>

#include "fix.h"
#include "macro/macro.h"
#include "stream1/stream1.h"

uint32_t hc_scale(const uint8_t exp) {
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

hc_fix_t hc_fix_new(const uint8_t exp, const int64_t val) {
  return (hc_fix_t)hc_bitmask(exp, HC_FIX_EXP) +
    (hc_fix_t)(((val < 0) ? 1 : 0) << HC_FIX_EXP) +
    (hc_fix_t)(hc_abs(val) << HC_FIX_HDR);
}

uint8_t hc_fix_exp(const hc_fix_t x) {
  return hc_bitmask(x, HC_FIX_EXP);
}

int64_t hc_fix_val(const hc_fix_t x) {
  const int64_t v = x >> HC_FIX_HDR;
  return ((x >> HC_FIX_EXP) & 1) ? -v : v;
}

int64_t hc_fix_int(const hc_fix_t x) {
  return hc_fix_val(x) / hc_scale(hc_fix_exp(x));
}

int64_t hc_fix_frac(const hc_fix_t x) {
  const int64_t xv = hc_fix_val(x);
  const uint32_t xs = hc_scale(hc_fix_exp(x));
  return xv - (xv / xs) * xs;
}

double hc_fix_double(const hc_fix_t x) {
  return hc_fix_val(x) / (double)hc_scale(hc_fix_exp(x));
}

hc_fix_t hc_fix_add(const hc_fix_t x, const hc_fix_t y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix_new(xe, hc_fix_val(x) + hc_fix_val(y));
  }
    
  return hc_fix_new(xe, hc_fix_val(x) +
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}

hc_fix_t hc_fix_sub(const hc_fix_t x, const hc_fix_t y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix_new(xe, hc_fix_val(x) - hc_fix_val(y));
  }

  return hc_fix_new(xe, hc_fix_val(x) -
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}

hc_fix_t hc_fix_mul(const hc_fix_t x, const hc_fix_t y) {
  return hc_fix_new(hc_fix_exp(x), hc_fix_val(x) *
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}

hc_fix_t hc_fix_div(const hc_fix_t x, const hc_fix_t y) {
  return hc_fix_new(hc_fix_exp(x), hc_fix_val(x) /
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}

void hc_fix_print(const hc_fix_t v, struct hc_stream *out) {
  _hc_stream_printf(out,
		    "%" PRId64 ".%" PRId64,
		    hc_fix_int(v),
		    hc_fix_frac(v));
}
