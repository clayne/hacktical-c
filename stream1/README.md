## Extensible Streams - Part 1
C++'s stream implementation may have missed the target in many ways, that doesn't mean extensible stream APIs in general are a bad idea. The problem we're trying to solve is providing an interface where one end doesn't need to know what's on the other end of a stream. C's standard library leaves a lot to wish for; there are extensions for custom `FILE *`-streams, but so far with spotty support.

Example:
```C
  struct hc_memory_stream s;
  hc_memory_stream_init(&s);
  hc_defer(hc_stream_deinit(&s.stream));
  hc_printf(&s.stream, "%s%d", "foo", 42);
  assert(strcmp("foo42", hc_memory_stream_string(&s)) == 0);
```

We'll start with defining the interface for a stream.

```C
struct hc_stream {
  void (*deinit)(struct hc_stream *);
  
  size_t (*read)(struct hc_stream *, uint8_t *, size_t);
  size_t (*write)(struct hc_stream *, const uint8_t *, size_t);
};
```

`deinit`, `read` & `write` delegate to respective stored function pointer.

```C
void hc_stream_deinit(struct hc_stream *s) {
  assert(s->deinit);
  s->deinit(s);
}

size_t hc_read(struct hc_stream *s, uint8_t *data, const size_t n) {
  assert(s->read);
  return s->read(s, data, n);
}

size_t hc_write(struct hc_stream *s,
	        const uint8_t *data,
    	        const size_t n) {
  assert(s->write);
  return s->write(s, data, n);
}
```

`putc` and `puts` are trivially implemented using `write`.

```C
size_t hc_putc(struct hc_stream *s, const char data) {
  const uint8_t d[2] = {data, 0};
  return hc_write(s, d, 1);
}

size_t hc_puts(struct hc_stream *s, const char *data) {
  return hc_write(s, (const uint8_t *)data, strlen(data));
}
```

`vprintf` uses a temporary buffer to format the message.

```C
size_t hc_vprintf(struct hc_stream *s,
	          const char *spec,
	          va_list args) {
  char *data = hc_vsprintf(spec, args);
  hc_defer(free(data));
  return hc_write(s, data, strlen(data));
}

size_t hc_printf(struct hc_stream *s, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  hc_defer(va_end(args));
  return hc_vprintf(s, spec, args);
}
```

The first implementation is file streams, which simply delegate to `stdio`. If `close_file` is `true`, the file is closed with the stream.

```C
struct hc_file_stream_opts {
  bool close_file;
};

struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
  struct hc_file_stream_opts opts;
};

#define hc_file_stream_init(s, f, ...)					
  _hc_file_stream_init(s, f, (struct hc_file_stream_opts){		
      .close_file = false,						
      ##__VA_ARGS__							
    })

struct hc_file_stream *_hc_file_stream_init(struct hc_file_stream *s,
			 		    FILE *file,
					    struct hc_file_stream_opts opts) {
  s->stream = (struct hc_stream){
    .deinit = file_deinit,
    .read   = file_read,
    .write  = file_write,
  };

  s->file = file;
  s->opts = opts;
  return s;
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

size_t file_read(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fread(data, n, 1, fs->file);
}

size_t file_write(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  return fwrite(data, n, 1, fs->file);
}
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

struct hc_memory_stream *hc_memory_stream_init(struct hc_memory_stream *s,
                                               struct hc_malloc *malloc) {
  s->stream = (struct hc_stream){
    .deinit = memory_deinit,
    .read   = memory_read,
    .write  = memory_write,
  };

  hc_vector_init(&s->data, malloc, 1);
  s->rpos = 0;
  return s;
}
```

`read()` clamps the number of read bytes to the length of `data - rpos`, uses `memcpy()` to copy data, and finally updates `rpos`. 

```C
size_t memory_read(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);

  if (ms->rpos + n > ms->data.length) {
    n = ms->data.length - ms->rpos;
  }
  
  memcpy(data, ms->data.start + ms->rpos, n);
  ms->rpos += n;
  return n;
}
```

`write()` inserts a block of `n` bytes at the end of the vector and uses `memcpy()` to copy data.

```C
size_t memory_write(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  uint8_t *const dst = hc_vector_insert(&ms->data, ms->data.length, n);
  memcpy(dst, data, n);
  return n;
}
```