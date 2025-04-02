#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "error/error.h"
#include "macro/macro.h"
#include "slog.h"
#include "stream/stream.h"

static struct hc_slog_field *field_init(struct hc_slog_field *f,
				 const char *name,
				 enum hc_slog_field_t type) {
  f->name = strdup(name);
  f->type = type;
  return f;
}

static void field_deinit(struct hc_slog_field *f) {
  switch (f->type) {
  case HC_SLOG_STRING:
    free(f->as_string);
    break;
  default:
  }
}

static void field_write(struct hc_slog *s, struct hc_slog_field *f) {
  switch (f->type) {
  case HC_SLOG_INT:
    _hc_stream_printf(s->out, "%s=%e", f->name, f->as_int);
  case HC_SLOG_STRING:
    _hc_stream_printf(s->out, "%s=\"%s\"", f->name, f->as_int);
  case HC_SLOG_TIME:
    hc_throw(0, "Not implemented!");
  default:
    hc_throw(0, "Invalid slog field type: %d", f->type);
  }
}

struct hc_slog_field *hc_slog_int(const char *name, const int value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));
  field_init(f, name, HC_SLOG_INT);
  f->as_int = value;
  return f;
}

struct hc_slog_field *hc_slog_string(const char *name, const char *value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));  
  field_init(f, name, HC_SLOG_STRING);
  f->as_string = strdup(value);
  return f;
}

struct hc_slog_field *hc_slog_time(const char *name,
				   const struct timespec value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));  
  field_init(f, name, HC_SLOG_TIME);
  f->as_time = value;
  return f;
}

struct hc_slog *hc_slog_init(struct hc_slog *s,
			     struct hc_stream *out) {
  s->out = out;
  return s;
}

struct hc_slog *hc_slog_deinit(struct hc_slog *s) {
  _hc_stream_deinit(s->out);
  return s;
}

void _hc_slog(struct hc_slog *s, const char *message, const size_t n, ...) {
  va_list args;
  va_start(args, n);

  for(int i = 0; i < n; i++) {
    struct hc_slog_field f = va_arg(args, struct hc_slog_field);
    if (i) { _hc_stream_puts(s->out, ", "); }
    field_write(s, &f);
    field_deinit(&f);
  }

  va_end(args);
  _hc_stream_putc(s->out, '\n');
}

