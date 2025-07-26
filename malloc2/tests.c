#include <assert.h>
#include "malloc2.h"

static void memo_tests() {
  struct hc_memo_alloc a;
  hc_memo_alloc_init(&a, &hc_malloc_default);

  int *ip1 = hc_acquire(&a.malloc, sizeof(int));
    
  long *lp = hc_acquire(&a.malloc, sizeof(long));
  assert((int *)lp != ip1);
  *lp = 42;
    
  hc_release(&a.malloc, ip1);
  int *ip2 = hc_acquire(&a.malloc, sizeof(int));
  assert(ip2 == ip1);
  *ip2 = 42;
    
  int *ip3 = hc_acquire(&a.malloc, sizeof(int));
  assert(ip3 != ip1);
  *ip3 = 42;
    
  hc_release(&a.malloc, lp);
  hc_release(&a.malloc, ip2);
  hc_release(&a.malloc, ip3);

  hc_memo_alloc_deinit(&a);
}

static void slab_tests() {
  struct hc_slab_alloc a;
  hc_slab_alloc_init(&a, &hc_malloc_default, 2 * sizeof(int));
  assert(a.slab_size == 2 * sizeof(int));

  const int *p1 = hc_acquire(&a.malloc, sizeof(int));
  const int *p2 = hc_acquire(&a.malloc, sizeof(int));
  assert(p2 == p1 + 1);

  const int *p3 = hc_acquire(&a.malloc, sizeof(int));
  assert(p3 > p2 + 1);
  
  const int *p4 = hc_acquire(&a.malloc, 10 * sizeof(int));
  assert(p4 > p3 + 1);

  hc_slab_alloc_deinit(&a);
}

void malloc2_tests() {
  memo_tests();
  slab_tests();
}
