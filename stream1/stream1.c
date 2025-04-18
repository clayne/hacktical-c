#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dynamic/dynamic.h"
#include "error/error.h"
#include "malloc1/malloc1.h"
#include "macro/macro.h"
#include "stream1.h"

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

size_t _hc_stream_putc(struct hc_stream *s, const char data) {
  const uint8_t d[2] = {data, 0};
  return _hc_stream_put(s, d, 1);
}

size_t _hc_stream_puts(struct hc_stream *s, const char *data) {
  return _hc_stream_put(s, (const uint8_t *)data, strlen(data));
}

size_t _hc_stream_vprintf(struct hc_stream *s,
			 const char *spec,
			 va_list args) {
  char *data = hc_vsprintf(spec, args);
  hc_defer(hc_release(data));
  return _hc_stream_put(s, (uint8_t *)data, strlen(data));
}

size_t _hc_stream_printf(struct hc_stream *s, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  hc_defer(va_end(args));
  return _hc_stream_vprintf(s, spec, args);
}

void file_deinit(struct hc_stream *s) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  assert(fs->file);

  if (fs->opts.close_file) {  
    if (fclose(fs->file) == EOF) {
      hc_throw(0, "Failed closing file");
    }
    
    fs->file = NULL;
  }
}

size_t file_get(struct hc_stream *s, uint8_t *data, const size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  assert(fs->file);
  return fread(data, n, 1, fs->file);
}

size_t file_put(struct hc_stream *s, const uint8_t *data, const size_t n) {
  struct hc_file_stream *fs = hc_baseof(s, struct hc_file_stream, stream);
  assert(fs->file);
  return fwrite(data, n, 1, fs->file);
}

struct hc_file_stream *_hc_file_stream_init(struct hc_file_stream *s,
					    FILE *file,
					    const struct hc_file_stream_opts opts) {
  s->stream = (struct hc_stream){
    .deinit  = file_deinit,
    .get     = file_get,
    .put     = file_put,
  };
  
  s->file = file;
  s->opts = opts;
  return s;
};

struct hc_stream *hc_stdout() {
  static __thread bool init = true;
  static __thread struct hc_file_stream s;

  if (init) {
    hc_file_stream_init(&s, stdout);
    init = false;
  }

  return &s.stream;
}

void memory_deinit(struct hc_stream *s) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  hc_vector_deinit(&ms->data);
}

size_t memory_get(struct hc_stream *s, uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);

  if (ms->rpos + n > ms->data.length) {
    n = ms->data.length - ms->rpos;
  }
  
  memcpy(data, ms->data.start + ms->rpos, n);
  ms->rpos += n;
  return n;
}

size_t memory_put(struct hc_stream *s, const uint8_t *data, const size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  uint8_t *const dst = hc_vector_insert(&ms->data, ms->data.length, n);
  memcpy(dst, data, n);
  return n;
}

struct hc_memory_stream *hc_memory_stream_init(struct hc_memory_stream *s) {
  s->stream = (struct hc_stream){
    .deinit  = memory_deinit,
    .get     = memory_get,
    .put     = memory_put,
  };
  
  hc_vector_init(&s->data, 1);
  s->rpos = 0;
  return s;
}

const char *hc_memory_stream_string(struct hc_memory_stream *s) {
  if (!s->data.length || (*(s->data.end-1))) {
    *(uint8_t *)hc_vector_push(&s->data) = 0;
  }

  return (const char *)s->data.start;
}
