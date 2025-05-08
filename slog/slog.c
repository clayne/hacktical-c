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
    hc_slog_stream_init(&s, hc_stdout());
    init = false;
  }

  return &s.slog;
}

static void field_deinit(struct hc_slog_field *f) {
  free(f->name);
  hc_value_deinit(&f->value);
}

static void slog_write(struct hc_slog *s,
		       const size_t n,
		       struct hc_slog_field *fields[]) {
  assert(s->write);
  s->write(s, n, fields);
}

void __hc_slog_write(struct hc_slog *s,
		     const size_t n,
		     struct hc_slog_field *fields[]) {
  slog_write(s, n, fields);
  
  for(size_t i = 0; i < n; i++) {
    struct hc_slog_field *f = fields[i];
    field_deinit(f);
    free(f);
  }
}

void _hc_slog_deinit(struct hc_slog *s) {
  if (s->deinit) {
    s->deinit(s);
  }
}

static struct hc_value *field_init(struct hc_slog_field *f,
				   const char *name,
				   const struct hc_type *type) {
  f->name = strdup(name);
  hc_value_init(&f->value, type);
  return &f->value;
}

struct hc_slog_field *hc_slog_bool(const char *name, const bool value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));
  field_init(f, name, &HC_BOOL)->as_bool = value;
  return f;
}

struct hc_slog_field *hc_slog_int(const char *name, const int value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));
  field_init(f, name, &HC_INT)->as_int = value;
  return f;
}

struct hc_slog_field *hc_slog_string(const char *name, const char *value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));
  field_init(f, name, &HC_STRING)->as_string = strdup(value);
  return f;
}

struct hc_slog_field *hc_slog_time(const char *name, const hc_time_t value) {
  struct hc_slog_field *f = malloc(sizeof(struct hc_slog_field));
  field_init(f, name, &HC_TIME)->as_time = value;
  return f;
}

void stream_deinit(struct hc_slog *s) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);
  if (ss->opts.close_out) { hc_stream_deinit(ss->out); }
}

static void field_write(struct hc_slog_field *f, struct hc_stream *out) {
  hc_puts(out, f->name);
  hc_putc(out, '=');
  hc_value_write(&f->value, out);
}

static void stream_write(struct hc_slog *s,
			 const size_t n,
			 struct hc_slog_field *fields[]) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);

  for(size_t i = 0; i < n; i++) {
    struct hc_slog_field *f = fields[i];
    if (i) { hc_puts(ss->out, ", "); }
    field_write(f, ss->out);
  }

  hc_putc(ss->out, '\n');
}

struct hc_slog_stream *_hc_slog_stream_init(struct hc_slog_stream *s,
					    struct hc_stream *out,
					    const struct hc_slog_stream_opts opts) {
  s->slog.deinit = stream_deinit;
  s->slog.write = stream_write;
  s->out = out;
  s->opts = opts;
  return s;
}

static void context_deinit(struct hc_slog *s) {
  struct hc_slog_context *sc = hc_baseof(s, struct hc_slog_context, slog);

  for (size_t i = 0; i < sc->length; i++) {
    struct hc_slog_field *f = sc->fields[i];
    field_deinit(f);
    free(f);
  }

  free(sc->fields);
}

static void context_write(struct hc_slog *s,
			  const size_t n,
			  struct hc_slog_field *fields[]) {
  struct hc_slog_context *c = hc_baseof(s, struct hc_slog_context, slog);
  struct hc_slog_field *fs[c->length + n];
  memcpy(fs, c->fields, sizeof(struct hc_slog_field *) * c->length);
  memcpy(fs + c->length, fields, sizeof(struct hc_slog_field *) * n);
  slog_write(c->parent, c->length + n, fs);
}

struct hc_slog_context *hc_slog_context_init(struct hc_slog_context *c,
					     size_t length,
					     struct hc_slog_field *fields[]) {
  c->slog.deinit = context_deinit;
  c->slog.write = context_write;
  c->parent = hc_slog();
  c->length = length;
  size_t s = sizeof(struct hc_slog_field *) * length;
  c->fields = malloc(s);
  memcpy(c->fields, fields, s);
  return c;
}
