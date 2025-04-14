#include <assert.h>
#include "malloc1.h"

void malloc1_tests() {
  const int s = 1024;
  struct hc_bump_alloc *a = hc_bump_alloc_new(hc_malloc(), s);
  assert(a->size == s);
  assert(a->offset == 0);

  hc_malloc_do(a) {
    int *ip = hc_acquire(sizeof(int));
    *ip = 42;
    
    long *lp = hc_acquire(sizeof(long));
    *lp = 42L;
  }

  assert(a->offset >= sizeof(int) + sizeof(long));
  hc_bump_alloc_free(a);
}
