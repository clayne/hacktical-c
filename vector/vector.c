#include <stdlib.h>
#include <string.h>
#include "vector.h"

static void grow(struct hc_vector *v) {
  hc_vector_grow(v, v->capacity ? v->capacity*2 : 2);
}

struct hc_vector *hc_vector_init(struct hc_vector *v, int item_size) {
  v->item_size = item_size;
  v->capacity = v->length = 0;
  v->items = v->start = v->end = NULL;
  return v;
}

void hc_vector_deinit(struct hc_vector *v) {
  if (v->items) { free(v->items); }
}

void hc_vector_grow(struct hc_vector *v, int capacity) {
  v->capacity = capacity;

  v->items = realloc(v->items,
		     hc_align(v->item_size*(v->capacity+1), v->item_size));

  v->start = hc_align(v->items, v->item_size);
  v->end = v->start + v->item_size*v->length;
}

void hc_vector_clear(struct hc_vector *v) {
  v->length = 0;
  v->end = v->start;
}

void *hc_vector_get(struct hc_vector *v, int i) {
  return v->items ? v->start + v->item_size*i : NULL;
}

void *hc_vector_push(struct hc_vector *v) {
  if (v->length == v->capacity) { grow(v); }
  void *p = v->end;
  v->end += v->item_size;
  v->length++;
  return p;
}

void *hc_vector_peek(struct hc_vector *v) {
  return v->length ? v->end - v->item_size : NULL;
}

void *hc_vector_pop(struct hc_vector *v) {
  if (!v->length) { return NULL; }
  v->end -= v->item_size;
  v->length--;
  return v->end;
}

void *hc_vector_insert(struct hc_vector *v, int i, int n) {
  while (v->length+n > v->capacity) { grow(v); } 
  uint8_t *const p = hc_vector_get(v, i);
  memmove(p+v->item_size*n, p, (v->length-i)*v->item_size);
  v->length += n;
  v->end += n;
  return p;
}

bool hc_vector_delete(struct hc_vector *v, int i, int n) {
  const in m = i+n;
  if (v->length < m) { return false; }

  if (m < v->length) {
    uint8_t *const p = hc_vector_get(v, i);
    memmove(p, p + n*v->item_size, i + (v->length-n)*v->item_size);
  }

  v->length -= n;
  v->end -= n*v->item_size;
  return true;
}
