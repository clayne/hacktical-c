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
  
  if (t->copy) {
    dst->type = t;
    t->copy(dst, src);
  } else {
    *dst = *src;
  }

  return dst;
}

void hc_value_print(struct hc_value *v, struct hc_stream *out) {
  if (v->type->print) {
    v->type->print(v, out);
  } else {
    hc_value_write(v, out);
  }
}

void hc_value_write(struct hc_value *v, struct hc_stream *out) {
  assert(v->type->write);
  v->type->write(v, out);
}

static void bool_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_puts(out, v->as_bool ? "true" : "false");
}

const struct hc_type HC_BOOL = {
  .name = "Bool",
  .copy = NULL,
  .write = bool_write
};

static void fix_write(const struct hc_value *v, struct hc_stream *out) {
  hc_fix_print(v->as_fix, out);
}

const struct hc_type HC_FIX = {
  .name = "Fix",
  .copy = NULL,
  .write = fix_write
};

static void int_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_printf(out, "%d", v->as_int);
}

const struct hc_type HC_INT = {
  .name = "Int",
  .copy = NULL,
  .write = int_write
};

static void string_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_string = strdup(src->as_string);
}

static void string_deinit(struct hc_value *v) {
  free(v->as_string);
}

static void string_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_putc(out, '"');
  _hc_stream_puts(out, v->as_string);
  _hc_stream_putc(out, '"');
}

const struct hc_type HC_STRING = {
  .name = "String",
  .copy = string_copy,
  .deinit = string_deinit,
  .write = string_write
};

static void time_write(const struct hc_value *v, struct hc_stream *out) {
  hc_time_printf(&v->as_time, HC_TIME_FORMAT, out);
}

const struct hc_type HC_TIME = {
  .name = "Time",
  .copy = NULL,
  .write = time_write
};
