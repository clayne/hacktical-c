## Reflection
Reflection is the ability of a program to examine and introspect its own structure and behavior. Since `C` lacks built-in support, we're going to add a layer of types and values on top. By itself, the functionality described here doesn't look very useful, but it enables generalizing over types when implementing other features.

Example:
```C
struct hc_value v;
hc_value_init(&v, &HC_STRING)->as_string = strdup("foo");
hc_defer(hc_value_deinit(&v));
struct hc_value c;
hc_value_copy(&c, &v);
hc_defer(hc_value_deinit(&c));
assert(strcmp(c.as_string, v.as_string) == 0);
```

A type contains a name and a set of basic operations we want to be able to perform polymorphically.

```C
struct hc_type {
  const char *name;
  
  void (*copy)(struct hc_value *dst, struct hc_value *src);
  void (*deinit)(struct hc_value *);
  void (*print)(const struct hc_value *, struct hc_stream *out);
  void (*write)(const struct hc_value *, struct hc_stream *out);
};
```

Values are implemented as tagged unions, with the option to store other kinds of values as `void *`.

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

`deinit` is optional.

```C
void hc_value_deinit(struct hc_value *v) {
  if (v->type->deinit) {
    v->type->deinit(v);
  }
}
```

`copy` is optional and defaults to bit-wise.

```C
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
```

`print` defaults to `write` if not defined.

```C
void hc_value_print(struct hc_value *v, struct hc_stream *out) {
  if (v->type->print) {
    v->type->print(v, out);
  } else {
    hc_value_write(v, out);
  }
}
```

The standard types are defined as constants.

```C
static void bool_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_puts(out, v->as_bool ? "true" : "false");
}

const struct hc_type HC_BOOL = {
  .name = "Bool",
  .copy = NULL,
  .write = bool_write
};
```

```C
static void fix_write(const struct hc_value *v, struct hc_stream *out) {
  hc_fix_print(v->as_fix, out);
}

const struct hc_type HC_FIX = {
  .name = "Fix",
  .copy = NULL,
  .write = fix_write
};
```

```C
static void int_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_printf(out, "%d", v->as_int);
}

const struct hc_type HC_INT = {
  .name = "Int",
  .copy = NULL,
  .write = int_write
};
```

The string type stands out in two ways; values are dynamically allocated, and it uses different syntax suitable for programmatic reading when being written to a stream.

```C
static void string_copy(struct hc_value *dst, struct hc_value *src) {
  dst->as_string = strdup(src->as_string);
}

static void string_deinit(struct hc_value *v) {
  free(v->as_string);
}

static void string_print(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_puts(out, v->as_string);
}

static void string_write(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_putc(out, '"');
  string_print(v, out);
  _hc_stream_putc(out, '"');
}

const struct hc_type HC_STRING = {
  .name = "String",
  .copy = string_copy,
  .deinit = string_deinit,
  .print = string_print,
  .write = string_write
};
```

```C
static void time_write(const struct hc_value *v, struct hc_stream *out) {
  hc_time_printf(&v->as_time, HC_TIME_FORMAT, out);
}

const struct hc_type HC_TIME = {
  .name = "Time",
  .copy = NULL,
  .write = time_write
};
```