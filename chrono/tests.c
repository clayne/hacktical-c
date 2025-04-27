#include <assert.h>
#include "chrono.h"

void chrono_tests() {
  hc_time_t t = hc_now();
  const int ns = 1000;
  assert(hc_sleep(ns) == 0);
  assert(hc_time_ns(&t) >= ns);
}
