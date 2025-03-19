#ifndef HACKTICAL_VECTOR_H
#define HACKTICAL_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/macros.h"

#define hc_vector_do(v, var)			\
  for (void *var = (v)->start;			\
       var < (void *)(v)->end;			\
       var += (v)->item_size)

struct hc_vector {
  int item_size, capacity, length;
  uint8_t *items, *start, *end;
};

struct hc_vector *hc_vector_init(struct hc_vector *v, int item_size);
void hc_vector_deinit(struct hc_vector *v);
void hc_vector_grow(struct hc_vector *v, int capacity);
void hc_vector_clear(struct hc_vector *v);
void *hc_vector_get(struct hc_vector *v, int i);
void *hc_vector_push(struct hc_vector *v);
void *hc_vector_peek(struct hc_vector *v);
void *hc_vector_pop(struct hc_vector *v);
void *hc_vector_insert(struct hc_vector *v, int i, int n);
bool hc_vector_delete(struct hc_vector *v, int i, int n);

#endif
