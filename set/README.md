## Ordered Sets and Maps
Besides lists and vectors, some kind of mapping/lookup functionality is often needed. The design described here is based on binary searched `struct hc_vector`s.

Most people would likely instinctively reach for hash tables, and typically spend the next few months researching optimal hash algorithms and table designs.

A binary searched vector is as simple as it gets and performs pretty good while being more predictable. The one worst case you want to avoid with a binary searched set is inserting items in reverse order, since that maximises the amount of work it has to do.

Most hash tables need to be resized at some point, leading to GC-like dips in performance. And no matter what hash algorithms you use, you will eventually run unpredictable issues with values clustering on a minority of buckets in the general case.

Besides, the natural dual to a lookup table is a list of pairs, and having an order strengthens that connection.

What we're actually going to build is an ordered set; but since it's value based, maps are easily implemented on top with low overhead.

Example:
```C
struct map_item {
  int k, v;
};

enum hc_order cmp(const void *x, const void *y) {
  return hc_cmp(*(const int *)x, *(const int *)y);
}

const void *key(const void *x) {
  return &((const struct map_item *)x)->k;
}

const int n = 10;
struct hc_set s;
hc_set_init(&s, sizeof(struct map_item), cmp);
  
for (int i = 0; i < n; i++) {
  struct map_item *it = hc_set_add(&s, &i, false);
  *it = (struct map_item){.k = i, .v = i};
}

for (int i = 0; i < n; i++) {
  struct map_item *it = hc_set_find(&s, &i);
  assert(it);
  assert(it->k == i);
  assert(it->v == i);
}

hc_set_deinit(&s);
```

A custom enum and convenience macro for comparisons is provided.

```C
#define hc_cmp(x, y) ({					\
      __auto_type _x = x;				\
      __auto_type _y = y;				\
      (_x < _y) ? HC_LT : ((_x > _y) ? HC_GT : HC_EQ);	\
    })

enum hc_order {HC_LT = -1, HC_EQ = 0, HC_GT = 1};

typedef enum hc_order (*hc_cmp_t)(const void *, const void *);
```

Besides a comparator for items, sets also feature an optional accessor for item keys.

```C
struct hc_set {
  struct hc_vector items;
  hc_cmp_t cmp;
  hc_set_key key;
};

struct hc_set *hc_set_init(struct hc_set *s, size_t item_size, hc_cmp_t cmp) {
  hc_vector_init(&s->items, item_size);
  s->cmp = cmp;
  s->key = NULL;
  return s;
}

void hc_set_deinit(struct hc_set *s) {
  hc_vector_deinit(&s->items);
}
```

Most of the weight is pulled by `hc_set_index()`; which returns an index for a key, and optionally sets a flag if the key is in the set.

```C
size_t hc_set_index(struct hc_set *s, void *key, bool *ok) {
  size_t min = 0, max = s->items.length;

  while (min < max) {
    const size_t i = (min+max)/2;
    const void *v = hc_vector_get(&s->items, i);
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
```

`hc_set_add()` adds an item with the specified key, provided it's not already in the set or the `force`-flag is `true`. A pointer to the item is returned for copying data.

```C
void *hc_set_add(struct hc_set *s, void *key, bool force) {
  bool ok = false;
  const size_t i = hc_set_index(s, key, &ok);

  if (ok && !force) {
    return NULL;
  }
  
  return hc_vector_insert(&s->items, i, 1);

}
```

`hc_set_find()` returns the item with the specified key if it's in the set, otherwise `NULL`.

```C
void *hc_set_find(struct hc_set *s, void *key) {
  bool ok = false;
  const size_t i = hc_set_index(s, key, &ok);
  return ok ? hc_vector_get(&s->items, i) : NULL;
}
```