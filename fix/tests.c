#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "fix.h"

static void test_add() {
  assert(hc_fix_add(hc_fix_new(2, 175), hc_fix_new(2, 25)) ==
	 hc_fix_new(2, 200));
    
  assert(hc_fix_add(hc_fix_new(2, 175), hc_fix_new(2, -25)) ==
	 hc_fix_new(2, 150));
}

static void test_div() {
  assert(hc_fix_div(hc_fix_new(2, 150), hc_fix_new(0, 2)) ==
	 hc_fix_new(2, 75));

  assert(hc_fix_div(hc_fix_new(2, 150), hc_fix_new(0, -2)) ==
	 hc_fix_new(2, -75));

  assert(hc_fix_div(hc_fix_new(2, -150), hc_fix_new(0, -2)) ==
	 hc_fix_new(2, 75));
}

static void test_mul() {
  assert(hc_fix_mul(hc_fix_new(2, 150), hc_fix_new(1, 5)) ==
	 hc_fix_new(2, 75));

  assert(hc_fix_mul(hc_fix_new(2, 150), hc_fix_new(1, -5)) ==
	 hc_fix_new(2, -75));

  assert(hc_fix_mul(hc_fix_new(2, -150), hc_fix_new(1, -5)) ==
	 hc_fix_new(2, 75));
}

static void test_new() {
  hc_fix_t x = hc_fix_new(2, -125);
  assert(hc_fix_exp(x) == 2);
  assert(hc_fix_val(x) == -125);
  assert(hc_fix_int(x) == -1);
  assert(hc_fix_frac(x) == -25);
  assert(hc_fix_double(x) == -1.25);
}

static void test_sub() {
  assert(hc_fix_sub(hc_fix_new(2, 175), hc_fix_new(2, 25)) ==
	 hc_fix_new(2, 150));

  assert(hc_fix_sub(hc_fix_new(2, 175), hc_fix_new(2, -25)) ==
	 hc_fix_new(2, 200));
}

void fix_tests() {
  test_add();
  test_div();
  test_mul();
  test_new();
  test_sub();
}
