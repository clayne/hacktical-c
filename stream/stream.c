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

size_t hc_stream_put(struct hc_stream *s, uint8_t *data, size_t n) {
  assert(s->put);
  return s->put(s, data, n);
}

size_t hc_stream_get(struct hc_stream *s, uint8_t *data, size_t n) {
  assert(s->get);
  return s->get(s, data, n);
}
