#include <stdlib.h>
#include "set.h"

struct hc_set *hc_set_new(size_t item_size, hc_cmp_t cmp) {
  return hc_set_init(malloc(sizeof(struct hc_set)), item_size, cmp);
}

struct hc_set *hc_set_init(struct hc_set *s, size_t item_size, hc_cmp_t cmp) {
  hc_vector_init(&s->items, item_size);
  s->cmp = cmp;
  s->key = NULL;
  return s;
}

void hc_set_deinit(struct hc_set *s) {
  hc_vector_deinit(&s->items);
}

size_t hc_set_index(const struct hc_set *s, const void *key, bool *ok) {
  size_t min = 0, max = s->items.length;

  while (min < max) {
    const size_t i = (min+max)/2;
    const void *v = hc_vector_get_const(&s->items, i);
    const void *k = s->key ? s->key(v) : v;

    switch (s->cmp(key, k)) {
    case HC_LT:
      max = i;
      break;
    case HC_GT:
      min = i+1;
      break;
    default:
      if (ok) {
	*ok = true;
      }
      
      return i;
    }
  }

  return min;
}

size_t hc_set_length(const struct hc_set *s) {
  return s->items.length;
}

void *hc_set_find(struct hc_set *s, const void *key) {
  bool ok = false;
  const size_t i = hc_set_index(s, key, &ok);
  return ok ? hc_vector_get_const(&s->items, i) : NULL;
}

void *hc_set_add(struct hc_set *s, const void *key, const bool force) {
  bool ok = false;
  const size_t i = hc_set_index(s, key, &ok);

  if (ok && !force) {
    return NULL;
  }
  
  return hc_vector_insert(&s->items, i, 1);
}

void hc_set_clear(struct hc_set *s) {
  hc_vector_clear(&s->items);
}
