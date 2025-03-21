#ifndef HACKTICAL_TIME_H
#define HACKTICAL_TIME_H

#include <stdint.h>
#include <time.h>

struct hc_timer {
  struct timespec start;
};

void hc_utc(struct timespec *out);
uint64_t hc_sleep(uint64_t ns);

void hc_timer_init(struct hc_timer *t);
uint64_t hc_timer_ns(const struct hc_timer *t);
uint64_t hc_timer_print(const struct hc_timer *t, const char *m);

#endif
