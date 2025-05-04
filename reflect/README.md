## Reflection
Reflection is the ability of a program to examine and introspect its own structure and behavior. Since `C` lacks built-in support, we're going to add a layer of types and values on top. By itself, the functionality doesn't look very useful, but it enables generalizing over types when implementing other features.

A type contains a name and the operations we wan't to be able to perform polymorphically.

```C
struct hc_type {
  const char *name;
  
  void (*copy)(struct hc_value *dst, struct hc_value *src);
  void (*deinit)(struct hc_value *);
  void (*put)(const struct hc_value *, struct hc_stream *out);
};
```

Values are implemented as tagged unions, with an option to store other kinds of values as `void *`.

```C
struct hc_value {
  const struct hc_type *type;
  
  union {
    bool as_bool;
    hc_fix_t as_fix;
    int as_int;
    void *as_other;
    char *as_string;
    hc_time_t as_time;
  };
};
```

The standard types are provided as thread local constants.

```C
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
```