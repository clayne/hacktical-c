#include <assert.h>
#include "set.h"

static enum hc_order cmp(const void *x, const void *y) {
  return hc_cmp(*(const int *)x, *(const int *)y);
}

void set_tests() {
  const int n = 10;
  
  struct hc_set s;
  hc_set_init(&s, sizeof(int), cmp);
  
  for (int i = n-1; i >= 0; i--) {
    *(int *)hc_set_add(&s, &i, false) = i;
  }

  assert(hc_set_length(&s) == n);
  
  for (int i = 0; i < n; i++) {
    assert(*(int *)hc_set_find(&s, &i) == i);
  }

  hc_set_clear(&s);
  assert(hc_set_length(&s) == 0);
  
  hc_set_deinit(&s);
}
