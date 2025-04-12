#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

#include "chrono.h"
#include "error/error.h"
#include "malloc1/malloc1.h"

struct hc_time hc_now() {
  struct hc_time t;
  
  if (!timespec_get(&t.value, TIME_UTC)) {
    hc_throw(0, "Failed getting time: %d", errno);
  }

  return t;
}

struct hc_time hc_time(int year,
		       int month,
		       int day,
		       int hour,
		       int minute,
		       int second) {
  struct tm t = {0};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  t.tm_sec = second;

  struct hc_time result = {0};
  result.value.tv_sec = timegm(&t);
  return result;
}

uint64_t hc_time_ns(const struct hc_time *t) {
  const struct timespec now = hc_now().value;
  
  return
    (now.tv_sec - t->value.tv_sec) * 1000000000 +
    (now.tv_nsec - t->value.tv_nsec);
}

void hc_time_print(const struct hc_time *t, const char *m) {
  printf("%s%" PRIu64 "ns\n", m, hc_time_ns(t));
}

char *hc_time_printf(const struct hc_time *t, const char *spec) {
  struct tm tm;
  gmtime_r(&(t->value.tv_sec), &tm);
  size_t len = 8;
  char *result = hc_acquire(len);

  for (;;) {
    const size_t n = strftime(result, len, spec, &tm);

    if (n) {
      result[n] = 0;
      break;
    }
    
    len *= 2;
    hc_release(result);
    result = hc_acquire(len);
  }
    
  return result;
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
