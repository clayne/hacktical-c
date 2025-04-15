#include <assert.h>
#include "malloc1.h"

void malloc1_tests() {
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
      assert(e->code == HC_NO_MEMORY);
      caught = true;
    }
    
    hc_catch(on_catch) {
      hc_acquire(s);
      assert(false);
    }

    assert(caught);
  }
}
