## Extensible Streams
C++'s stream implementation missed the target in many ways, but that doesn't mean that an extensible stream API is a bad idea.

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

```
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

The first implementation is file streams.

```
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

const struct hc_stream hc_file_stream = {
  .get = file_get,
  .put = file_put,
  .vprintf = file_vprintf
};
```