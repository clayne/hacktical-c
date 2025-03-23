#include <assert.h>
#include "malloc.h"

static void bump_tests() {
  const int s = 1024;
  struct hc_bump_alloc *a = hc_bump_alloc_new(&hc_malloc, s);
  assert(a->size == s);
  assert(a->offset == 0);

  int *ip = hc_acquire(a, sizeof(int));
  *ip = 42;

  long *lp = hc_acquire(a, sizeof(long));
  *lp = 42L;

  assert(a->offset >= sizeof(int) + sizeof(long));
  hc_bump_alloc_free(a);
}

static void memo_tests() {
  struct hc_memo_alloc a;
  hc_memo_alloc_init(&a, &hc_malloc);
  
  int *ip1 = hc_acquire(&a, sizeof(int));
  
  long *lp = hc_acquire(&a, sizeof(long));
  assert((int *)lp != ip1);
  *lp = 42;
  
  hc_release(&a, ip1);
  int *ip2 = hc_acquire(&a, sizeof(int));
  assert(ip2 == ip1);
  *ip2 = 42;

  int *ip3 = hc_acquire(&a, sizeof(int));
  assert(ip3 != ip1);
  *ip3 = 42;

  hc_release(&a, lp);
  hc_release(&a, ip2);
  hc_release(&a, ip3);
  hc_memo_alloc_deinit(&a);
}

void malloc_tests() {
  bump_tests();
  memo_tests();
}
