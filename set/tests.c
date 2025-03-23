#include <assert.h>
#include <stdio.h>
#include "set.h"

struct map_item {
  int k, v;
};

static enum hc_order cmp(const void *x, const void *y) {
  const int *xi = x;
  const int *yi = y;
  return hc_cmp(*xi, *yi);
}

static const void *key(const void *x) {
  const struct map_item *xi = x;
  return &xi->k;
}

void set_tests() {
  int n = 10;
  struct hc_set s;
  hc_set_init(&s, sizeof(struct map_item), cmp);
  s.key = key;
  
  for (int i = 0; i < n; i++) {
    struct map_item *it = hc_set_add(&s, &i, false);
    assert(it);
    *it = (struct map_item){.k = i, .v = i};
  }

  assert(hc_set_length(&s) == n);
  
  for (int i = 0; i < n; i++) {
    struct map_item *it = hc_set_find(&s, &i);
    assert(it);
    assert(it->k == i);
    assert(it->v == i);
  }

  hc_set_clear(&s);
  assert(hc_set_length(&s) == 0);  
  hc_set_deinit(&s);
}
