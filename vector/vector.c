#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "malloc1/malloc1.h"

static void grow(struct hc_vector *v) {
  hc_vector_grow(v, v->capacity ? v->capacity*2 : 2);
}

struct hc_vector *hc_vector_init(struct hc_vector *v,
				 struct hc_malloc *malloc,
				 const size_t item_size) {
  v->malloc = malloc;
  v->item_size = item_size;
  v->capacity = 0;
  v->length = 0;
  v->start = v->end = NULL;
  return v;
}

void hc_vector_deinit(struct hc_vector *v) {
  if (v->start) { _hc_release(v->malloc, v->start); }
}

void hc_vector_grow(struct hc_vector *v, const size_t capacity) {
  v->capacity = capacity; 
  size_t size = v->item_size * (v->capacity+1);
  uint8_t *new_start = _hc_acquire(v->malloc, size);

  if (v->start) {
    memmove(new_start, v->start, v->length * v->item_size);
    _hc_release(v->malloc, v->start); 
  }
  
  v->start = new_start;
  v->end = v->start + v->item_size*v->length;
}

void hc_vector_clear(struct hc_vector *v) {
  v->length = 0;
  v->end = v->start;
}

void *hc_vector_get(struct hc_vector *v, const size_t i) {
  return v->start ? v->start + v->item_size*i : NULL;
}

const void *hc_vector_get_const(const struct hc_vector *v, const size_t i) {
  return v->start ? v->start + v->item_size*i : NULL;
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

void *hc_vector_insert(struct hc_vector *v, const size_t i, const size_t n) {
  const size_t m = v->length+n;
  if (m > v->capacity) { hc_vector_grow(v, m); } 
  uint8_t *const p = hc_vector_get(v, i);

  if (i < v->length) {
    memmove(p + v->item_size*n, p, (v->length - i) * v->item_size);
  }
  
  v->length += n;
  v->end += n*v->item_size;
  return p;
}

bool hc_vector_delete(struct hc_vector *v, const size_t i, const size_t n) {
  const size_t m = i+n;
  assert(m <= v->length);

  if (m < v->length) {
    uint8_t *const p = hc_vector_get(v, i);
    memmove(p, p + n*v->item_size, i + (v->length-n) * v->item_size);
  }

  v->length -= n;
  v->end -= n*v->item_size;
  return true;
}
