#ifndef HACKTICAL_SLOG_H
#define HACKTICAL_SLOG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "stream/stream.h"

#define hc_slog(_s, _m, ...) do {					\
    struct hc_field[] fs = {__VA_ARGS__};				\
    _hc_slog((_s), (_m), sizeof(fs) / sizeof(struct hc_field),		\
	     hc_slog_timestamp("timestamp", cnow()),			\
	     hc_slog_string("message", _m), ##__VA_ARGS__, NULL)	\
      } while (0)

struct hc_slog {
  struct hc_stream *out;
};

struct hc_slog_field;
typedef void (*hc_slog_field_write)(struct hc_slog *, struct hc_slog_field *);

struct hc_slog_field *hc_slog_int(const char *name, int value);

struct hc_slog_field *hc_slog_string(const char *name, const char *value);

struct hc_slog_field *hc_slog_timestamp(const char *name,
					struct timespec value);

enum hc_slog_field_t {
  HC_SLOG_INT, HC_SLOG_STRING, HC_SLOG_TIME
};

struct hc_slog_field {
  char *name;
  enum hc_slog_field_t type;

  union {
    char *as_string;
    int as_int;
    struct timespec as_time;
  };  
};
  
struct hc_slog *hc_slog_init(struct hc_slog *s, struct hc_stream *out);
struct hc_slog *hc_slog_deinit(struct hc_slog *s);

void _hc_slog_write(struct hc_slog *s, const char *message, size_t n, ...);

#endif
