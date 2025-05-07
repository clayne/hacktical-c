## Extensible Streams - Part 1
C++'s stream implementation may have missed the target in many ways, that doesn't mean extensible stream APIs in general are a bad idea. The problem we're trying to solve is providing an interface where one end doesn't need to know what's on the other end of a stream. C's standard library leaves a lot to wish for; there are extensions for custom `FILE *`-streams, but so far with spotty support.

Example:
```C
  struct hc_memory_stream s;
  hc_memory_stream_init(&s);
  hc_defer(hc_stream_deinit(&s));
  hc_stream_printf(&s, "%s%d", "foo", 42);
  assert(strcmp("foo42", hc_memory_stream_string(&s)) == 0);
```

We'll start with defining the interface for a stream.

```C
struct hc_stream {
  void   (*deinit)(struct hc_stream *);
  size_t (*get)(struct hc_stream *, uint8_t *, size_t);
  size_t (*put)(struct hc_stream *, const uint8_t *, size_t);
};
```

As well as a set of macros to simplify usage.

```C
#define hc_stream_deinit(s)			
  _hc_stream_deinit(&(s)->stream)

#define hc_stream_get(s, d, n)			
  _hc_stream_get(&(s)->stream, d, n)

#define hc_stream_put(s, d, n)			
  _hc_stream_put(&(s)->stream, d, n)

#define hc_stream_putc(s, d)			
  _hc_stream_putc(&(s)->stream, d)

#define hc_stream_puts(s, d)			
  _hc_stream_puts(&(s)->stream, d)

#define hc_stream_vprintf(s, spec, args)	
  _hc_stream_vprintf(&(s)->stream, spec, args);
```

`deinit`, `get` & `put` delegate to respective stored function pointer.

```C
void _hc_stream_deinit(struct hc_stream *s) {
  assert(s->deinit);
  s->deinit(s);
}

size_t _hc_stream_get(struct hc_stream *s, uint8_t *data, const size_t n) {
  assert(s->get);
  return s->get(s, data, n);
}

size_t _hc_stream_put(struct hc_stream *s,
		      const uint8_t *data,
		      const size_t n) {
  assert(s->put);
  return s->put(s, data, n);
}
```

`putc` and `puts` are trivially implemented using `put`.

```C
size_t _hc_stream_putc(struct hc_stream *s, const char data) {
  const uint8_t d[2] = {data, 0};
  return _hc_stream_put(s, d, 1);
}

size_t _hc_stream_puts(struct hc_stream *s, const char *data) {
  return _hc_stream_put(s, (const uint8_t *)data, strlen(data));
}
```

`vprintf` uses a temporary buffer to format the message.

```C
size_t _hc_stream_vprintf(struct hc_stream *s,
			 const char *spec,
			 va_list args) {
  char *data = hc_vsprintf(spec, args);
  hc_defer(free(data));
  return _hc_stream_put(s, data, strlen(data));
}
```

The first implementation is file streams, which simply delegate to `stdio`. If `opts.close_file` is `true`, the file is closed with the stream.

```C
struct hc_file_stream_opts {
  bool close_file;
};

struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
  struct hc_file_stream_opts opts;
};

void file_deinit(struct hc_stream *s) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);

  if (fs->close) {
    assert(fs->file);
  
    if (fclose(fs->file) == EOF) {
      hc_throw(0, "Failed closing file");
    }

    fs->file = NULL;
  }
}

size_t file_get(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fread(data, n, 1, fs->file);
}

size_t file_put(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fwrite(data, n, 1, fs->file);
}
```

We'll add a macro on top of the init function to make the option syntax nicer, this a very useful technique for implementing optional function arguments with defaults.

```C
#define hc_file_stream_init(s, f, ...)					
  _hc_file_stream_init(s, f, (struct hc_file_stream_opts){		
      .close_file = false,						
      ##__VA_ARGS__							
    })

struct hc_file_stream *_hc_file_stream_init(struct hc_file_stream *s,
			 		    FILE *file,
					    struct hc_file_stream_opts opts) {
  s->stream = (struct hc_stream){
    .deinit  = file_deinit,
    .get     = file_get,
    .put     = file_put,
  };

  s->file = file;
  s->opts = opts;
  return s;
};
```

Next up is implementing memory streams. We'll use a `struct hc_vector` to manage the stream data and track the current read position in `rpos`.

```C
struct hc_memory_stream {
  struct hc_stream stream;
  struct hc_vector data;
  size_t rpos;
};

void memory_deinit(struct hc_stream *s) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  hc_vector_deinit(&ms->data);
}
```

`put()` inserts a block of `n` bytes at the end of the vector and uses `memcpy()` to copy data.

```C
size_t memory_put(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  uint8_t *const dst = hc_vector_insert(&ms->data, ms->data.length, n);
  memcpy(dst, data, n);
  return n;
}
```

`get()` clamps the number of read bytes to the length of `data - rpos`, uses `memcpy()` to copy data, and finally updates `rpos`. 

```C
size_t memory_get(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);

  if (ms->rpos + n > ms->data.length) {
    n = ms->data.length - ms->rpos;
  }
  
  memcpy(data, ms->data.start + ms->rpos, n);
  ms->rpos += n;
  return n;
}

struct hc_memory_stream *hc_memory_stream_init(struct hc_memory_stream *s,
                                               struct hc_malloc *malloc) {
  s->stream = (struct hc_stream){
    .deinit  = memory_deinit,
    .get     = memory_get,
    .put     = memory_put,
  };

  hc_vector_init(&s->data, malloc, 1);
  s->rpos = 0;
  return s;
}
```