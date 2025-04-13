#ifndef HACKTICAL_SLOG_H
#define HACKTICAL_SLOG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "chrono/chrono.h"
#include "stream1/stream1.h"

#define __hc_slog_do(s, _ps)			\
  for (struct hc_slog *_ps = hc_slog();		\
       _ps && (_hc_slog = (s));			\
       _hc_slog = _ps, _ps = NULL)

#define _hc_slog_do(s)				\
  __hc_slog_do((s), hc_unique(ps))

#define hc_slog_do(s)				\
  _hc_slog_do(&(s)->slog)

struct hc_slog;

enum hc_slog_field_t {
  HC_SLOG_BOOL, HC_SLOG_INT, HC_SLOG_STRING, HC_SLOG_TIME
};

struct hc_slog_field {
  char *name;
  enum hc_slog_field_t type;

  union {
    bool as_bool;
    int as_int;
    char *as_string;
    struct hc_time as_time;
  };  
};

struct hc_slog {
  void (*deinit)(struct hc_slog *s);
  void (*write)(struct hc_slog *s,
		size_t n,
		struct hc_slog_field fields[]);
};

struct hc_slog_stream {
  struct hc_slog slog;
  struct hc_stream *out;
  bool close_out;
};

extern __thread struct hc_slog *_hc_slog;
struct hc_slog *hc_slog();

struct hc_slog_field;

struct hc_slog_field hc_slog_bool(const char *name, bool value);
struct hc_slog_field hc_slog_int(const char *name, int value);
struct hc_slog_field hc_slog_string(const char *name, const char *value);
struct hc_slog_field hc_slog_time(const char *name, struct hc_time value);

#define hc_slog_deinit(s)			\
  _hc_slog_deinit(&(s)->slog)

#define _hc_slog_write(s, ...) do {				\
    struct hc_slog_field fs[] = {__VA_ARGS__};			\
    size_t n = sizeof(fs) / sizeof(struct hc_slog_field);	\
    __hc_slog_write((s), n, fs);				\
  } while (0)

#define hc_slog_write(...)			\
  _hc_slog_write(hc_slog(), ##__VA_ARGS__)

struct hc_slog_stream *hc_slog_stream_init(struct hc_slog_stream *s,
					   struct hc_stream *out,
					   bool close_out);

void _hc_slog_deinit(struct hc_slog *s);

void __hc_slog_write(struct hc_slog *s,
		     size_t n,
		     struct hc_slog_field fields[]);

#define _hc_slog_context_do(_c, _n, _fs, ...)			\
  struct hc_slog_context _c;					\
  struct hc_slog_field _fs[] = {__VA_ARGS__};			\
  size_t _n = sizeof(_fs) / sizeof(struct hc_slog_field);	\
  hc_slog_context_init(&_c, _n, _fs);				\
  hc_defer(hc_slog_deinit(&_c));				\
  hc_slog_do(&_c)

#define hc_slog_context_do(...)			\
  _hc_slog_context_do(hc_unique(slog_c),	\
		      hc_unique(slog_n),	\
		      hc_unique(slog_fs),	\
		      ##__VA_ARGS__)

struct hc_slog_context {
  struct hc_slog slog;
  struct hc_slog *parent;
  size_t length;
  struct hc_slog_field *fields;
};

struct hc_slog_context *hc_slog_context_init(struct hc_slog_context *c,
					     size_t length,
					     struct hc_slog_field fields[]);

#endif
