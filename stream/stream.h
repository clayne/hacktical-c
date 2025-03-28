#ifndef HACKTICAL_STREAM_H
#define HACKTICAL_STREAM_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "vector/vector.h"

struct hc_stream {
  size_t (*put)(struct hc_stream *s, const uint8_t *data, size_t n);
  size_t (*get)(struct hc_stream *s, uint8_t *data, size_t n);
  int (*vprintf)(struct hc_stream *s, const char *spec, va_list args);
};

struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
};

extern const struct hc_stream hc_file_stream;
struct hc_stream *hc_file_stream_init(struct hc_file_stream *s, FILE *file);

struct hc_memory_stream {
  struct hc_stream stream;
  struct hc_vector data;
  size_t rpos;
};

extern const struct hc_stream hc_memory_stream;
struct hc_stream *hc_memory_stream_init(struct hc_memory_stream *s);

size_t hc_stream_vprintf(struct hc_stream *s, const char *spec, va_list args);
size_t hc_stream_put(struct hc_stream *s, uint8_t *data, size_t n);
size_t hc_stream_get(struct hc_stream *s, uint8_t *data, size_t n);

#endif
