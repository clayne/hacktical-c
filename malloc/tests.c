#include <assert.h>
#include "malloc.h"

static void bump_tests() {
  const int s = 1024;
  struct hc_bump_alloc *a = hc_bump_alloc_new(&hc_malloc, s);
  assert(a->size == s);
  assert(a->offset == 0);

  int *ip = hc_acquire(a, sizeof(int));
  *ip = 42;

  int *lp = hc_acquire(a, sizeof(long));
  *lp = 42L;

  assert(a->offset >= sizeof(int) + sizeof(long));
  hc_bump_alloc_free(a);
}

void malloc_tests() {
  bump_tests();
}
