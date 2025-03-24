#include <assert.h>
#include "malloc2.h"

static void memo_tests() {
  struct hc_memo_alloc a;
  hc_memo_alloc_init(&a, hc_malloc);

  hc_malloc_do(&a) {
    int *ip1 = hc_acquire(sizeof(int));
    
    long *lp = hc_acquire(sizeof(long));
    assert((int *)lp != ip1);
    *lp = 42;
    
    hc_release(ip1);
    int *ip2 = hc_acquire(sizeof(int));
    assert(ip2 == ip1);
    *ip2 = 42;
    
    int *ip3 = hc_acquire(sizeof(int));
    assert(ip3 != ip1);
    *ip3 = 42;
    
    hc_release(lp);
    hc_release(ip2);
    hc_release(ip3);
  }

  hc_memo_alloc_deinit(&a);
}

void malloc2_tests() {
  memo_tests();
}
