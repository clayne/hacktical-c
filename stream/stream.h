#ifndef HACKTICAL_STREAM_H
#define HACKTICAL_STREAM_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "vector/vector.h"

struct hc_stream {
  void   (*deinit)(struct hc_stream *s);
  size_t (*get)(struct hc_stream *s, uint8_t *data, size_t n);
  size_t (*put)(struct hc_stream *s, const uint8_t *data, size_t n);
  int    (*vprintf)(struct hc_stream *s, const char *spec, va_list args);
};

#define hc_stream_deinit(s)			\
  _hc_stream_deinit(&(s)->stream)

#define hc_stream_get(s, d, n)			\
  _hc_stream_get(&(s)->stream, d, n)

#define hc_stream_put(s, d, n)			\
  _hc_stream_put(&(s)->stream, d, n)

#define hc_stream_putc(s, d)			\
  _hc_stream_putc(&(s)->stream, d)

#define hc_stream_puts(s, d)			\
  _hc_stream_puts(&(s)->stream, d)

#define hc_stream_vprintf(s, spec, args)	\
  _hc_stream_vprintf(&(s)->stream, spec, args);

#define hc_stream_printf(s, spec, ...)			\
  _hc_stream_printf(&(s)->stream, spec, ##__VA_ARGS__);

void _hc_stream_deinit(struct hc_stream *s);
size_t _hc_stream_get(struct hc_stream *s, uint8_t *data, size_t n);
size_t _hc_stream_put(struct hc_stream *s, const uint8_t *data, size_t n);
size_t _hc_stream_putc(struct hc_stream *s, char data);
size_t _hc_stream_puts(struct hc_stream *s, const char *data);

size_t _hc_stream_vprintf(struct hc_stream *s,
			  const char *spec,
			  va_list args);

size_t _hc_stream_printf(struct hc_stream *s, const char *spec, ...);

struct hc_file_stream {
  struct hc_stream stream;
  FILE *file;
  bool close_file;
};

extern struct hc_stream hc_file_stream;
struct hc_file_stream *hc_file_stream_init(struct hc_file_stream *s,
					   FILE *file,
					   bool close_file);

struct hc_stream *hc_stdout();

struct hc_memory_stream {
  struct hc_stream stream;
  struct hc_vector data;
  size_t rpos;
};

extern struct hc_stream hc_memory_stream;
struct hc_memory_stream *hc_memory_stream_init(struct hc_memory_stream *s);
const char *hc_memory_stream_string(struct hc_memory_stream *s);

#endif
