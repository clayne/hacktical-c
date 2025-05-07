## Fixed-Point Arithmetic
Fixed-point is a method of representing fractional values by storing a fixed number of digits of the fractional part, accomplished by multiplying with a fixed power of ten. This means they can be stored and processed much like regular integers. Floating point values are more complicated and inconsistent to deal with. As a result it's often recommended to use fixed-points when dealing with numbers that need to be exact, for instance time and money.

Fixed-points are normally implemented by simply multiplying all values by a static power of ten. We'll add a tiny bit of flexibility and safety by remembering the exponent used to create the value and supporting operations on values with different exponents. All arithmetic operations use the precision of the left hand argument for their result.

We use unsigned 64-bit integers; 3 bits for the exponent (power of 10), 1 for sign and the remaining 60 bits for the value.

```C
#define HC_FIX_EXP 3
#define HC_FIX_HDR (HC_FIX_EXP+1)

#define HC_FIX_MAX_EXP 7

typedef uint64_t hc_fix_t;
```

The constructor takes an exponent and a pre-scaled, signed value.

```C
hc_fix_t hc_fix(uint8_t exp, int64_t val) {
  return (hc_fix_t)hc_bitmask(exp, HC_FIX_EXP) +
    (hc_fix_t)(((val < 0) ? 1 : 0) << HC_FIX_EXP) +
    (hc_fix_t)(hc_abs(val) << HC_FIX_HDR);
}
```

Example:
```C
hc_fix_t x = hc_fix(2, -125);
assert(hc_fix_exp(x) == 2);
assert(hc_fix_val(x) == -125);
```

`hc_scale()` converts exponents to multipliers.

```C
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
```

`hc_fix_int()` and `hc_fix_frac()` return signed integer and fractional parts respectively.

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
const hc_fix_t x = hc_fix(2, -125);
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
hc_fix_t x = hc_fix(2, -125);
assert(hc_fix_double(x) == -1.25);
```

Addition and subtraction are identical except for the operator, the left hand side exponent is preserved. A fast path is provided for the case when both values share the same exponent.

```C
hc_fix_t hc_fix_add(hc_fix_t x, hc_fix_t y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix(xe, hc_fix_val(x) + hc_fix_val(y));
  }
    
  return hc_fix(xe, hc_fix_val(x) +
		    hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}
```

```C
hc_fix_t hc_fix_sub(hc_fix_t x, hc_fix_t y) {
  const uint8_t xe = hc_fix_exp(x);
  const uint8_t ye = hc_fix_exp(y);

  if (xe == ye) {
    return hc_fix(xe, hc_fix_val(x) - hc_fix_val(y));
  }

  return hc_fix(xe, hc_fix_val(x) -
		hc_fix_val(y) * hc_scale(xe) / hc_scale(ye));
}
```

Example:
```C
hc_fix_t x = hc_fix(2, 175);
hc_fix_t y = hc_fix(2, 25);
assert(hc_fix_add(x, y) == hc_fix(2, 200));
```

Multiplication and division are likewise identical except for the operator.

```C
hc_fix_t hc_fix_mul(hc_fix_t x, hc_fix_t y) {
  return hc_fix(hc_fix_exp(x), hc_fix_val(x) *
		    hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}
```

```C
hc_fix_t hc_fix_div(hc_fix_t x, hc_fix_t y) {
  return hc_fix(hc_fix_exp(x), hc_fix_val(x) /
		hc_fix_val(y) / hc_scale(hc_fix_exp(y)));
}
```

Example:
```C
hc_fix_t x = hc_fix(2, 150);
hc_fix_t y = hc_fix(1, 5);
assert(hc_fix_mul(x, y) == hc_fix(2, 75));
```