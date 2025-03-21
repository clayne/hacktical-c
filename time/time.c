#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include "error/error.h"
#include "time.h"

void hc_utc(struct timespec *out) {
  if (!timespec_get(out, TIME_UTC)) {
    hc_throw(0, "Failed getting time: %d", errno);
  }
}

uint64_t hc_sleep(uint64_t ns) {
  struct timespec t = {0};
  t.tv_nsec = ns;

  switch (nanosleep(&t, &t)) {
  case 0:
    break;
  case EINTR:
    return t.tv_nsec;
  default:
    hc_throw(0, "Failed sleeping: %d", errno);
  }

  return 0;
}

void hc_timer_init(struct hc_timer *t) {
  hc_utc(&t->start);
}

uint64_t hc_timer_ns(const struct hc_timer *t) {
  struct timespec end;
  hc_utc(&end);
  
  return
    (end.tv_sec - t->start.tv_sec) * 1000000000 +
    (end.tv_nsec - t->start.tv_nsec);
}

uint64_t hc_timer_print(const struct hc_timer *t, const char *m) {
  printf("%s%" PRIu64 "ns\n", m, hc_timer_ns(t));
}
