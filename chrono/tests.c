#include <assert.h>
#include "chrono.h"

void chrono_tests() {
  struct hc_timer t;
  hc_timer_init(&t);
  const int ns = 1000;
  assert(hc_sleep(ns) == 0);
  assert(hc_timer_ns(&t) >= ns);
}
