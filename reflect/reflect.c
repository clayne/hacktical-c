#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "reflect.h"
#include "stream1/stream1.h"

void hc_strncpy(char *dst, const char *src, size_t n) {
  while (--n) {
    *dst = *src;
    dst++;
    src++;
  }

  *dst = 0;
}

void hc_type_init(struct hc_type *t, const char *name) {
  hc_strncpy(t->name, name, sizeof(t->name));
}

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

void hc_value_print(struct hc_value *v, struct hc_stream *out) {
  assert(v->type->print);
  v->type->print(v, out);
}

static void fix_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_fix = src->as_fix;
}

static void fix_print(const struct hc_value *v, struct hc_stream *out) {
  hc_fix_print(v->as_fix, out);
}

const struct hc_type *HC_FIX() {
  static __thread struct hc_type t = {
    .copy = fix_copy,
    .print = fix_print
  };

  hc_type_init(&t, "Fix");
  return &t;
}

static void int_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_int = src->as_int;
}

static void int_print(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_printf(out, "%d", v->as_int);
}

const struct hc_type *HC_INT() {
  static __thread struct hc_type t = {
    .copy = int_copy,
    .print = int_print
  };

  hc_type_init(&t, "Int");
  return &t;
}

static void string_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_string = strdup(src->as_string);
}

static void string_deinit(struct hc_value *v) {
  free(v->as_string);
}

static void string_print(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_puts(out, v->as_string);
}

const struct hc_type *HC_STRING() {
  static __thread struct hc_type t = {
    .copy = string_copy,
    .deinit = string_deinit,
    .print = string_print
  };

  hc_type_init(&t, "String");
  return &t;
}

static void time_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_time = src->as_time;
}

static void time_print(const struct hc_value *v, struct hc_stream *out) {
  hc_time_printf(&v->as_time, HC_TIME_FORMAT, out);
}

const struct hc_type *HC_TIME() {
  static __thread struct hc_type t = {
    .copy = time_copy,
    .print = time_print
  };

  hc_type_init(&t, "Time");
  return &t;
}
