## Fixed-Point Arithmetic
Fixed-point is a method of representing fractional values by storing a fixed number of digits of the fractional part; or, seen from another angle, by multiplying them with a fixed power of ten. This means they can be stored and processed as regular integers.

The reason one might wish to do o could be performance, but I feel correctness is the more important consideration. Floating point values are more complicated to implement and inconsistent to deal with. As a result it's often recommended to use fixed-points when dealing with numbers that need to be exact, for instance time and money.

We'll use unsigned 64-bit integers to store fixed-point values; using 3 bits for the exponent (power of 10), 1 for sign and the remaining 60 bits for the value.

```C
#define HC_FIX_EXP 3
#define HC_FIX_HDR (HC_FIX_EXP+1)

typedef uint64_t hc_fix_t;
```

The constructor takes an exponent and a pre-scaled, signed value.

```C
hc_fix_t hc_fix_new(uint8_t exp, int64_t val) {
  return (hc_fix_t)hc_bitmask(exp, HC_FIX_EXP) +
    (hc_fix_t)(((val < 0) ? 1 : 0) << HC_FIX_EXP) +
    (hc_fix_t)(hc_abs(val) << HC_FIX_HDR);
}
```

Example:
```C
const hc_fix_t x = hc_fix_new(2, -125);
assert(hc_fix_exp(x) == 2);
assert(hc_fix_val(x) == -125);
```

`hc_fix_int()` and `hc_fix_frac()` returns signed integer and fractional parts respectively.

```C
int64_t hc_fix_int(hc_fix_t x) {
  return hc_fix_val(x) / hc_scale(hc_fix_exp(x));
}

int64_t hc_fix_frac(hc_fix_t x) {
  const int64_t xv = hc_fix_val(x);
  const uint32_t xs = hc_scale(hc_fix_exp(x));
  return xv - (xv / xs) * xs;
}
```

Example:
```C
const hc_fix_t x = hc_fix_new(2, -125);
assert(hc_fix_int(x) == -1);
assert(hc_fix_frac(x) == -25);
```

`hc_fix_double()` converts to signed double precision floating point.

```C
double hc_fix_t_double(hc_fix_t x) {
  return hc_fix_val(x) / (double)hc_scale(hc_fix_exp(x));
}
```

Example:
```C
const hc_fix_t x = hc_fix_new(2, -125);
assert(hc_fix_double(x) == -1.25);
```

Addition and subtraction are identical except for the operator, the left hand side exponent is preserved. A fast path is provided for the case when both values share the same exponent.

```C
hc_fix_t hc_fix_add(hc_fix_t x, hc_fix_t y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix_new(xe, hc_fix_val(x) + hc_fix_val(y));
  }
    
  return hc_fix_new(xe, hc_fix_val(x) +
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}
```

Example:
```C
const hc_fix_t x = hc_fix_new(2, 175);
const hc_fix_t y = hc_fix_new(2, 25);
assert(hc_fix_add(x, y) == hc_fix_new(2, 200));
```

Multiplication and division are likewise identical except for the operator.

```C
hc_fix_t hc_fix_mul(hc_fix_t x, hc_fix_t y) {
  return hc_fix_new(hc_fix_exp(x), hc_fix_val(x) *
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}
```

Example:
```C
const hc_fix_t x = hc_fix_new(2, 150);
const hc_fix_t y = hc_fix_new(1, 5);
assert(hc_fix_mul(x, y) == hc_fix_new(2, 75));
```