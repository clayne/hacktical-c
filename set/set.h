#ifndef HACKTICAL_SET_H
#define HACKTICAL_SET_H

#include <stdbool.h>
#include "vector/vector.h"

#define hc_cmp(x, y) ({					\
      __auto_type _x = x;				\
      __auto_type _y = y;				\
      (_x < _y) ? HC_LT : ((_x > _y) ? HC_GT : HC_EQ);	\
    })

struct hc_malloc;

enum hc_order {HC_LT = -1, HC_EQ = 0, HC_GT = 1};

typedef enum hc_order (*hc_cmp_t)(const void *, const void *);

typedef const void *(*hc_set_key_t)(const void *); 

struct hc_set {
  struct hc_vector items;
  hc_cmp_t cmp;
  hc_set_key_t key;
};

struct hc_set *hc_set_init(struct hc_set *s,
			   struct hc_malloc *malloc,
			   size_t item_size,
			   hc_cmp_t cmp);

void hc_set_deinit(struct hc_set *s);
size_t hc_set_index(const struct hc_set *s, const void *key, bool *ok);
size_t hc_set_length(const struct hc_set *s);
void *hc_set_find(struct hc_set *s, const void *key);
void *hc_set_add(struct hc_set *s, const void *key, bool force);
void hc_set_clear(struct hc_set *s);

#endif
