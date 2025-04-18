#include <assert.h>
#include "macro.h"

void macro_tests() {
  int hc_id(foo, bar) = 42;
  assert(foobar == 42);

  assert(hc_min(7, 42) == 7);
  assert(hc_max(7.0, 42.0) == 42.0);

  {
    int foo = 0;
    
    {
      hc_defer(assert(foo++ == 1));
      hc_defer(assert(foo++ == 0));
    }

    assert(foo == 2);
  }

  assert(hc_align(0, 4) == 0);
  assert(hc_align(1, 4) == 4);
  assert(hc_align(3, 4) == 4);
  assert(hc_align(4, 4) == 4);
  assert(hc_align(5, 4) == 8);
}
