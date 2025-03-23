#ifndef HACKTICAL_SET_H
#define HACKTICAL_SET_H

#include <stdbool.h>

#include "order.h"
#include "vector/vector.h"

typedef const void *(*hc_set_key)(const void *); 

struct hc_set {
  struct hc_vector items;
  hc_cmp_t cmp;
  hc_set_key key;
};

struct hc_set *hc_set_new(size_t size, hc_cmp_t cmp);
struct hc_set *hc_set_init(struct hc_set *s, size_t item_size, hc_cmp_t cmp);
void hc_set_deinit(struct hc_set *s);
size_t hc_set_index(struct hc_set *s, void *key, bool *ok);
size_t hc_set_length(struct hc_set *s);
void *hc_set_find(struct hc_set *s, void *key);
void *hc_set_add(struct hc_set *s, void *key, bool force);

#endif
