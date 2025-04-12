#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "error/error.h"
#include "macro/macro.h"
#include "malloc1/malloc1.h"
#include "slog.h"

__thread struct hc_slog *_hc_slog = NULL;

struct hc_slog *hc_slog() {
  if (_hc_slog != NULL) {
    return _hc_slog;
  }
  
  static __thread bool init = true;
  static __thread struct hc_slog_stream s;

  if (init) {
    hc_slog_stream_init(&s, hc_stdout(), false);
    init = false;
  }

  return &s.slog;
}

static void field_deinit(struct hc_slog_field *f) {
  free(f->name);
  
  switch (f->type) {
  case HC_SLOG_STRING:
    free(f->as_string);
    break;
  default:
    break;
  }
}

static void slog_write(struct hc_slog *s,
		       const size_t n,
		       struct hc_slog_field fields[]) {
  assert(s->write);
  s->write(s, n, fields);
}

void __hc_slog_write(struct hc_slog *s,
		     const size_t n,
		     struct hc_slog_field fields[]) {
  slog_write(s, n, fields);
  
  for(size_t i = 0; i < n; i++) {
    field_deinit(fields+i);
  }
}

void _hc_slog_deinit(struct hc_slog *s) {
  if (s->deinit) {
    s->deinit(s);
  }
}

static struct hc_slog_field *field_init(struct hc_slog_field *f,
					const char *name,
					enum hc_slog_field_t type) {
  f->name = strdup(name);
  f->type = type;
  return f;
}

static void field_write(struct hc_slog_field *f, struct hc_stream *out) {
  switch (f->type) {
  case HC_SLOG_BOOL:
    _hc_stream_printf(out, "%s=%s", f->name, f->as_bool ? "true" : "false");
    break;
  case HC_SLOG_INT:
    _hc_stream_printf(out, "%s=%d", f->name, f->as_int);
    break;
  case HC_SLOG_STRING:
    _hc_stream_printf(out, "%s=\"%s\"", f->name, f->as_string);
    break;
  case HC_SLOG_TIME: {
    char *s = hc_time_printf(&f->as_time, "%Y-%m-%dT%H:%M:%S");
    _hc_stream_printf(out, "%s=%s", f->name, s);
    hc_release(s);
    break;
  }
  default:
    hc_throw(0, "Invalid slog field type: %d", f->type);
  }
}

struct hc_slog_field hc_slog_bool(const char *name, const bool value) {
  struct hc_slog_field f;
  field_init(&f, name, HC_SLOG_BOOL);
  f.as_bool = value;
  return f;
}

struct hc_slog_field hc_slog_int(const char *name, const int value) {
  struct hc_slog_field f;
  field_init(&f, name, HC_SLOG_INT);
  f.as_int = value;
  return f;
}

struct hc_slog_field hc_slog_string(const char *name, const char *value) {
  struct hc_slog_field f;
  field_init(&f, name, HC_SLOG_STRING);
  f.as_string = strdup(value);
  return f;
}

struct hc_slog_field hc_slog_time(const char *name,
				  const struct hc_time value) {
  struct hc_slog_field f;  
  field_init(&f, name, HC_SLOG_TIME);
  f.as_time = value;
  return f;
}

void stream_deinit(struct hc_slog *s) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);
  if (ss->close_out) { _hc_stream_deinit(ss->out); }
}

static void stream_write(struct hc_slog *s,
			 const size_t n,
			 struct hc_slog_field fields[]) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);

  for(size_t i = 0; i < n; i++) {
    struct hc_slog_field f = fields[i];
    if (i) { _hc_stream_puts(ss->out, ", "); }
    field_write(&f, ss->out);
  }

  _hc_stream_putc(ss->out, '\n');
}

struct hc_slog_stream *hc_slog_stream_init(struct hc_slog_stream *s,
					   struct hc_stream *out,
					   const bool close_out) {
  s->slog.deinit = stream_deinit;
  s->slog.write = stream_write;
  s->out = out;
  s->close_out = close_out;
  return s;
}

static void context_deinit(struct hc_slog *s) {
  struct hc_slog_context *sc = hc_baseof(s, struct hc_slog_context, slog);

  for (size_t i = 0; i < sc->length; i++) {
    field_deinit(sc->fields + i);
  }

  free(sc->fields);
}

static void context_write(struct hc_slog *s,
			  const size_t n,
			  struct hc_slog_field fields[]) {
  struct hc_slog_context *c = hc_baseof(s, struct hc_slog_context, slog);
  struct hc_slog_field fs[c->length + n];
  memcpy(fs, c->fields, sizeof(struct hc_slog_field)*c->length);
  memcpy(fs+c->length, fields, sizeof(struct hc_slog_field)*n);
  slog_write(c->parent, c->length+n, fs);
}

struct hc_slog_context *hc_slog_context_init(struct hc_slog_context *c,
					     size_t length,
					     struct hc_slog_field fields[]) {
  c->slog.deinit = context_deinit;
  c->slog.write = context_write;
  c->parent = hc_slog();
  c->length = length;
  size_t s = sizeof(struct hc_slog_field)*length;
  c->fields = malloc(s);
  memcpy(c->fields, fields, s);
  return c;
}
