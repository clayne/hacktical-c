#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "reflect.h"
#include "stream1/stream1.h"

struct hc_value *hc_value_init(struct hc_value *v, const struct hc_type *t) {
  v->type = t;
  return v;
}

void hc_value_deinit(struct hc_value *v) {
  if (v->type->deinit) {
    v->type->deinit(v);
  }
}

struct hc_value *hc_value_copy(struct hc_value *dst, struct hc_value *src) {
  const struct hc_type *t = src->type;
  dst->type = t;
  assert(t->copy);
  t->copy(dst, src);
  return dst;
}

void hc_value_put(struct hc_value *v, struct hc_stream *out) {
  assert(v->type->put);
  v->type->put(v, out);
}

static void bool_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_bool = src->as_bool;
}

static void bool_put(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_puts(out, v->as_bool ? "true" : "false");
}

const struct hc_type *HC_BOOL() {
  static __thread struct hc_type t = {
    .name = "Bool",
    .copy = bool_copy,
    .put = bool_put
  };

  return &t;
}

static void fix_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_fix = src->as_fix;
}

static void fix_put(const struct hc_value *v, struct hc_stream *out) {
  hc_fix_print(v->as_fix, out);
}

const struct hc_type *HC_FIX() {
  static __thread struct hc_type t = {
    .name = "Fix",
    .copy = fix_copy,
    .put = fix_put
  };

  return &t;
}

static void int_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_int = src->as_int;
}

static void int_put(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_printf(out, "%d", v->as_int);
}

const struct hc_type *HC_INT() {
  static __thread struct hc_type t = {
    .name = "Int",
    .copy = int_copy,
    .put = int_put
  };

  return &t;
}

static void string_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_string = strdup(src->as_string);
}

static void string_deinit(struct hc_value *v) {
  free(v->as_string);
}

static void string_put(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_putc(out, '"');
  _hc_stream_puts(out, v->as_string);
  _hc_stream_putc(out, '"');
}

const struct hc_type *HC_STRING() {
  static __thread struct hc_type t = {
    .name = "String",
    .copy = string_copy,
    .deinit = string_deinit,
    .put = string_put
  };

  return &t;
}

static void time_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_time = src->as_time;
}

static void time_put(const struct hc_value *v, struct hc_stream *out) {
  hc_time_printf(&v->as_time, HC_TIME_FORMAT, out);
}

const struct hc_type *HC_TIME() {
  static __thread struct hc_type t = {
    .name = "Time",
    .copy = time_copy,
    .put = time_put
  };

  return &t;
}
