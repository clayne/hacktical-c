#ifndef HACKTICAL_CHRONO_H
#define HACKTICAL_CHRONO_H

#include <stdint.h>
#include <time.h>

struct hc_time {
  struct timespec value;
};

struct hc_time hc_now();

struct hc_time hc_time(int year,
		       int month,
		       int day,
		       int hour,
		       int minute,
		       int second);

uint64_t hc_time_ns(const struct hc_time *t);
void hc_time_print(const struct hc_time *t, const char *m);
char *hc_time_printf(const struct hc_time *t, const char *spec);

uint64_t hc_sleep(uint64_t ns);

#endif
