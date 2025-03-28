## Extensible Streams
C++'s stream implementation might have missed the target in many ways, but that doesn't mean that an extensible stream API is a bad idea.

C `stdlib` provides support for file operations and formatting.

We'll start with defining the interface.

```C
struct hc_stream {
  size_t (*put)(struct hc_stream *s, const uint8_t *data, size_t n);
  size_t (*get)(struct hc_stream *s, uint8_t *data, size_t n);
  int (*vprintf)(struct hc_stream *s, const char *spec, va_list args);
};
```

`vprintf` defaults to using a temporary buffer, but is special cased for `FILE *`s to simply delegate to `vfprintf()`.

```C
size_t hc_stream_vprintf(struct hc_stream *s,
			 const char *spec,
			 va_list args) {
  if (s->vprintf) {
    return s->vprintf(s, spec, args);
  }

  char *data = hc_vsprintf(spec, args);
  hc_defer(hc_release(data));
  hc_stream_put(s, data, strlen(data));
}
```

The first implementation is file streams, which simply delegate to `stdio`.

```C
struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
};

size_t file_put(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fwrite(data, n, 1, fs->file);
}

size_t file_get(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fread(data, n, 1, fs->file);
}

int file_vprintf(struct hc_stream *s, const char *spec, va_list args) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return vfprintf(fs->file, spec, args);
}
```

The next most obvious variant is memory streams. We'll use a `struct hc_vector` to manage the data. `vprintf` defaults to using a temporary buffer.

```C
struct hc_memory_stream {
  struct hc_stream stream;
  struct hc_vector data;
  size_t rpos;
};
```

`put()` inserts a block of `n` bytes at the end of the vector and uses `memcpy()` to copy data.

```C
size_t memory_put(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  const size_t offset = ms->data.length;
  hc_vector_insert(&ms->data, ms->data.length, n);
  memcpy(ms->data.start + offset, data, n);
  return n;
}
```

`get()` clamps the number of read bytes to the length of `s->data` minus `s->rpos` and uses `memcpy()` to copy data. It also updates `rpos`. 

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
```