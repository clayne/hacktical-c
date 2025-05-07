## Structured Logs
Logging is one aspect of software development that I feel deserves more focus than it usually gets. A well implemented log goes a long way to quickly resolving unforseen issues that pop up in production. I prefer my logs structured by name/value pairs since this increases their usefulness by making them convenient to work with programatically.

Example:
```C
  struct hc_slog_stream s;
  hc_slog_stream_init(&s, &hc_stdout(), true);
  
  hc_slog_do(&s) {
    hc_slog_context_do(hc_slog_string("foo", "bar")) {
      hc_slog_write(hc_slog_time("baz", hc_time(2025, 4, 13, 1, 40, 0)));
    }
  }
```
```
foo="bar", baz=2025-04-13T1:40:00
```

Let's start with the interface.

```C
struct hc_slog {
  void (*deinit)(struct hc_slog *);
  void (*write)(struct hc_slog *, size_t, struct hc_slog_field []);
};
```

A field consists of a name and a [value](https://github.com/codr7/hacktical-c/tree/main/reflect).

```C
struct hc_slog_field {
  char *name;
  struct hc_value value;
};
```

Each thread tracks its own default log, which defaults to `stdout`.

```C
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
```

A few macros to simplify typical use are provided:

```C
#define __hc_slog_do(s, _ps)			
  for (struct hc_slog *_ps = hc_slog();		
       _ps && (_hc_slog = (s));			
       _hc_slog = _ps, _ps = NULL)

#define _hc_slog_do(s)				
  __hc_slog_do((s), hc_unique(ps))

#define hc_slog_do(s)				
  _hc_slog_do(&(s)->slog)

#define hc_slog_deinit(s)			
  _hc_slog_deinit(&(s)->slog)
```

Since specifying fields needs to be as convenient as possible, we'll add a thin layer on top.

```C
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
```

`hc_slog_write()` converts its arguments to be passed as length/array to the function.

```C
#define _hc_slog_write(s, ...) do {				
    hc_array(struct hc_slog_field *, fs, ##__VA_ARGS__);	
    __hc_slog_write((s), fs_n, fs_a);				
  } while (0)

#define hc_slog_write(...)			
  _hc_slog_write(hc_slog(), ##__VA_ARGS__)
```

The primary kind of log writes fields to a `struct hc_stream`.

```C
struct hc_slog_stream_opts {
  bool close_out;
};

struct hc_slog_stream {
  struct hc_slog slog;
  struct hc_stream *out;
  struct hc_slog_stream_opts opts;
};

void stream_deinit(struct hc_slog *s) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);
  if (ss->opts.close_out) { _hc_stream_deinit(ss->out); }
}

void stream_write(struct hc_slog *s,
	          const size_t n,
		  struct hc_slog_field *fields[]) {
  struct hc_slog_stream *ss = hc_baseof(s, struct hc_slog_stream, slog);

  for(size_t i = 0; i < n; i++) {
    struct hc_slog_field *f = fields[i];
    if (i) { _hc_stream_puts(ss->out, ", "); }
    field_write(f, ss->out);
  }

  _hc_stream_putc(ss->out, '\n');
}

#define hc_slog_stream_init(s, out, ...)				
  _hc_slog_stream_init(s, out, (struct hc_slog_stream_opts){		
      .close_out = false,						
      ##__VA_ARGS__							
    })

struct hc_slog_stream *hc_slog_stream_init(struct hc_slog_stream *s,
					   struct hc_stream *out,
					   struct hc_slog_stream_opts opts) {
  s->slog.deinit = stream_deinit;
  s->slog.write = stream_write;
  s->out = out;
  s->opts = opts;
  return s;
}
```

Contexts are implemented as just another kind of log, which traps write calls and delegate them to the parent log prefixed with its own fields.

```C
#define __hc_slog_context_do(_c, _fs, _a, _n, ...)			
  struct hc_slog_context _c;						
  hc_array(struct hc_slog_field *, _fs, ##__VA_ARGS__);			
  hc_slog_context_init(&_c, _n, _a);					
  hc_defer(hc_slog_deinit(&_c));					
  hc_slog_do(&_c)

#define _hc_slog_context_do(_c, _fs, ...)	
  __hc_slog_context_do(_c,		
		       _fs,			
		       hc_id(_fs, _a),		
		       hc_id(_fs, _n),		
		       ##__VA_ARGS__)

#define hc_slog_context_do(...)			
  _hc_slog_context_do(hc_unique(slog_c),
		      hc_unique(slog_fs),
		      ##__VA_ARGS__)

struct hc_slog_context {
  struct hc_slog slog;
  struct hc_slog *parent;
  size_t length;
  struct hc_slog_field **fields;
};

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
```