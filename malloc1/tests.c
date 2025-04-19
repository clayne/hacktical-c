#include <assert.h>
#include "malloc1.h"

void malloc1_tests() {
  assert(hc_align(0, 4) == 0);
  assert(hc_align(1, 4) == 4);
  assert(hc_align(3, 4) == 4);
  assert(hc_align(4, 4) == 4);
  assert(hc_align(5, 4) == 8);

  const int s = 1024;
  struct hc_bump_alloc a;
  hc_bump_alloc_init(&a, hc_malloc(), s);
  hc_defer(hc_bump_alloc_deinit(&a));
  assert(a.size == s);
  assert(a.offset == 0);

  hc_malloc_do(&a) {
    int *ip = hc_acquire(sizeof(int));
    *ip = 42;
    
    long *lp = hc_acquire(sizeof(long));
    *lp = 42L;
    
    assert(a.offset >= sizeof(int) + sizeof(long));
    bool caught = false;
    
    void on_catch(struct hc_error *e) {
      assert(hc_streq(e->message, HC_NO_MEMORY) == 0);
      caught = true;
    }
    
    hc_catch(on_catch) {
      hc_acquire(s);
      assert(false);
    }

    assert(caught);
  }
}
