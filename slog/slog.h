#ifndef HACKTICAL_SLOG_H
#define HACKTICAL_SLOG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "stream/stream.h"

#define __hc_slog_do(s, _ps)				\
  for (struct hc_slog *_ps = hc_slog();			\
       _ps && (_hc_slog = (s));				\
       _hc_slog = _ps, _ps = NULL)

#define _hc_slog_do(s)				\
  __hc_slog_do((s), hc_unique(ps))

#define hc_slog_do(s)				\
  _hc_slog_do(&(s)->slog)

struct hc_slog {
  struct hc_stream *out;
  bool close_out;
};

extern __thread struct hc_slog *_hc_slog;
struct hc_slog *hc_slog();

struct hc_slog_field;

struct hc_slog_field hc_slog_int(const char *name, int value);

struct hc_slog_field hc_slog_string(const char *name, const char *value);

struct hc_slog_field hc_slog_timestamp(const char *name,
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

#define hc_slog_init(s, out, close_out)		\
  _hc_slog_init(&(s)->slog, out, close_out)

#define hc_slog_deinit(s)			\
  _hc_slog_deinit(&(s)->slog)

#define _hc_slog_write(s, ...) do {				\
    struct hc_slog_field fs[] = {__VA_ARGS__};			\
    size_t n = sizeof(fs) / sizeof(struct hc_slog_field);	\
    __hc_slog_write((s), n, fs);			\
  } while (0)

#define hc_slog_write(...)			\
  _hc_slog_write(hc_slog(), ##__VA_ARGS__)

struct hc_slog *_hc_slog_init(struct hc_slog *s,
			      struct hc_stream *out,
			      bool close_out);

struct hc_slog *_hc_slog_deinit(struct hc_slog *s);

void __hc_slog_write(struct hc_slog *s, size_t n, struct hc_slog_field fields[]);

#endif
