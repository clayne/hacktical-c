## Extensible Streams - Part 1
C++'s stream implementation may have missed the target in many ways, that doesn't mean extensible stream APIs in general are a bad idea. The problem we're trying to solve is providing a stream API where one end doesn't need to know what's on the other end of a stream. C's standard library leaves a lot to wish for; there are extensions for custom `FILE *`-streams, but so far with very spotty support.

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
  void   (*deinit)(struct hc_stream *s);
  size_t (*get)(struct hc_stream *s, uint8_t *data, size_t n);
  size_t (*put)(struct hc_stream *s, const uint8_t *data, size_t n);
  int    (*vprintf)(struct hc_stream *s, const char *spec, va_list args);
};
```

`vprintf` defaults to using a temporary buffer.

```C
size_t hc_stream_vprintf(struct hc_stream *s,
			 const char *spec,
			 va_list args) {
  if (s->vprintf) {
    return s->vprintf(s, spec, args);
  }

  char *data = hc_vsprintf(spec, args);
  hc_defer(hc_release(data));
  return _hc_stream_put(s, data, strlen(data));
}
```

The first implementation is file streams, which simply delegate to `stdio`. If `close` is `true`, the file is closed with the stream.

```C
struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
  bool close;
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

int file_vprintf(struct hc_stream *s, const char *spec, va_list args) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return vfprintf(fs->file, spec, args);
}

struct hc_stream hc_file_stream = {
  .deinit  = file_deinit,
  .get     = file_get,
  .put     = file_put,
  .vprintf = file_vprintf
};

struct hc_file_stream *hc_file_stream_init(struct hc_file_stream *s,
					   FILE *file,
					   bool close) {
  s->stream = hc_file_stream;
  s->file = file;
  s->close = close;
  return s;
};
```

The next most obvious variation is memory streams. We'll use a `struct hc_vector` to manage the data.

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

`get()` clamps the number of read bytes to the length of `data` minus `rpos` and uses `memcpy()` to copy data, and finally updates `rpos`. 

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

struct hc_stream hc_memory_stream = {
  .deinit  = memory_deinit,
  .get     = memory_get,
  .put     = memory_put,
  .vprintf = NULL
};

struct hc_memory_stream *hc_memory_stream_init(struct hc_memory_stream *s) {
  s->stream = hc_memory_stream;
  hc_vector_init(&s->data, 1);
  s->rpos = 0;
  return s;
}
```