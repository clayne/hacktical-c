#ifndef HACKTICAL_CHRONO_H
#define HACKTICAL_CHRONO_H

#include <stdint.h>
#include <time.h>

struct hc_time {
  struct timespec value;
};

typedef struct hc_time hc_time_t;

hc_time_t hc_now();

hc_time_t hc_time(int year,
		       int month,
		       int day,
		       int hour,
		       int minute,
		       int second);

uint64_t hc_time_ns(const hc_time_t *t);
void hc_time_print(const hc_time_t *t, const char *m);
char *hc_time_printf(const hc_time_t *t, const char *spec);

uint64_t hc_sleep(uint64_t ns);

#endif
