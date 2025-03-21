#include <assert.h>
#include "time.h"

void time_tests() {
  struct hc_timer t;
  hc_timer_init(&t);
  const int ns = 1000;
  assert(sleep(ns) == 0);
  assert(hc_timer_nsecs(&t) >= ns);
}
