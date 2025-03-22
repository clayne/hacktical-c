#include <assert.h>
#include "malloc.h"

void malloc_tests() {
  const int s = 1024;
  struct hc_bump_alloc *a = hc_bump_alloc_new(&hc_malloc, s);
  assert(a->size == s);
  assert(a->offset == 0);
  hc_bump_alloc_free(a);
}
