#ifndef HACKTICAL_VECTOR_H
#define HACKTICAL_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "macro/macro.h"

#define _hc_vector_do(v, _v, var)			\
  struct hc_vector *_v = v;				\
  for (uint8_t *var = _v->start;			\
       var < _v->end;					\
       var += _v->item_size)

#define hc_vector_do(v, var)				\
  _hc_vector_do(v, hc_unique(vector), var)

struct hc_malloc;

struct hc_vector {
  size_t item_size, capacity, length;
  uint8_t *items, *start, *end;
  struct hc_malloc *malloc;
};

struct hc_vector *hc_vector_init(struct hc_vector *v, size_t item_size);
void hc_vector_deinit(struct hc_vector *v);
void hc_vector_grow(struct hc_vector *v, size_t capacity);
void hc_vector_clear(struct hc_vector *v);
void *hc_vector_get(struct hc_vector *v, size_t i);
const void *hc_vector_get_const(const struct hc_vector *v, size_t i);
void *hc_vector_push(struct hc_vector *v);
void *hc_vector_peek(struct hc_vector *v);
void *hc_vector_pop(struct hc_vector *v);
void *hc_vector_insert(struct hc_vector *v, size_t i, size_t n);
bool hc_vector_delete(struct hc_vector *v, size_t i, size_t n);

#endif
