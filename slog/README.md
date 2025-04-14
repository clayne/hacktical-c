## Structured Logs
Logging is one aspect of software development that I feel deserves more focus than it usually gets. A well implemented log goes a long way to quickly resolving unforseen issues that pop up in production. I prefer my logs structured by name/value pairs, which increases their usefulness by making them convenient to work with programatically.

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
  void (*deinit)(struct hc_slog *s);

  void (*write)(struct hc_slog *s,
  		size_t n,
		struct hc_slog_field fields[]);
};
```

A field contains a name, field type and value.

```C
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

To be continued...