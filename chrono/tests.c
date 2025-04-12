#include <assert.h>
#include "chrono.h"

void chrono_tests() {
  struct hc_time t = hc_now();
  const int ns = 1000;
  assert(hc_sleep(ns) == 0);
  assert(hc_time_ns(&t) >= ns);
}
