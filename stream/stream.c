#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dynamic/dynamic.h"
#include "malloc1/malloc1.h"
#include "macro/macro.h"
#include "stream.h"

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

struct hc_stream *hc_file_stream_init(struct hc_file_stream *s, FILE *file) {
  s->file = file;
  return &s->stream;
};

size_t memory_put(struct hc_stream *s, const uint8_t *data, size_t n) {
  struct hc_memory_stream *ms = hc_baseof(s, struct hc_memory_stream, stream);
  const size_t offset = ms->data.length;
  hc_vector_insert(&ms->data, ms->data.length, n);
  memcpy(ms->data.start + offset, data, n);
  return n;
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

const struct hc_stream hc_memory_stream = {
  .get = memory_get,
  .put = memory_put,
  .vprintf = NULL
};

struct hc_stream *hc_memory_stream_init(struct hc_memory_stream *s) {
  hc_vector_init(&s->data, 1);
  s->rpos = 0;
  return &s->stream;
}

size_t hc_stream_vprintf(struct hc_stream *s,
			 const char *spec,
			 va_list args) {
  if (s->vprintf) {
    return s->vprintf(s, spec, args);
  }

  char *data = hc_vsprintf(spec, args);
  hc_defer(hc_release(data));
  return hc_stream_put(s, data, strlen(data));
}

size_t hc_stream_put(struct hc_stream *s, uint8_t *data, size_t n) {
  assert(s->put);
  return s->put(s, data, n);
}

size_t hc_stream_get(struct hc_stream *s, uint8_t *data, size_t n) {
  assert(s->get);
  return s->get(s, data, n);
}
