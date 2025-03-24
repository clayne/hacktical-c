## Composable Memory Allocators - Part 2
Welcome back to memory allocator land. We now have enough features in place to design more interesting allocators. Before we're done, the goal is to share enough examples of what's possible for you to start dreaming up your own designs.

### Recycling
Recycling memory is a common requirement for allocators, we'll design the feature as a separate allocator that can be conveniently added at any point in a pipeline. A multi-`struct hc_set` ordered by allocation size is used to track allocations.

```C
struct hc_memo_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  struct hc_set memo;
};

struct hc_memo_alloc *hc_memo_alloc_init(struct hc_memo_alloc *a,
					 struct hc_malloc *source) {
  a->malloc.acquire = memo_acquire;
  a->malloc.release = memo_release;
  a->source = source;
  hc_set_init(&a->memo, sizeof(struct memo *), memo_cmp);
  a->memo.key = memo_key;
}

void hc_memo_alloc_deinit(struct hc_memo_alloc *a) {
  hc_vector_do(&a->memo.items, _m) {
    struct memo *m = *(struct memo **)_m;
    _hc_release(a->source, m);
  }
  
  hc_set_deinit(&a->memo);
}
```

Each allocation consists of a size and the data.

```C
struct memo {
  size_t size;
  uint8_t data[];
};

enum hc_order memo_cmp(const void *l, const void *r) {
  return hc_cmp(*(size_t *)l, *(size_t *)r);
}

const void *memo_key(const void *p) {
  struct memo *m = *(struct memo **)p;
  return &m->size;
}
```

`acquire` first checks for recycled allocations of the correct size, and delegates to the source allocator if none was found.

```C
void *memo_acquire(struct hc_malloc *a, size_t size) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);

  if (hc_set_length(&ma->memo)) {
    bool ok = false;
    size_t i = hc_set_index(&ma->memo, &size, &ok);

    if (ok) {
      struct hc_vector *is = &ma->memo.items;
      struct memo *m = *(struct memo **)hc_vector_get(is, i);
      hc_vector_delete(is, i, 1);
      return m->data;
    }
  }

  struct memo *m = _hc_acquire(ma->source, sizeof(struct memo) + size);
  m->size = size;
  return m->data;
}
```

While `release` registers the allocation for recycling.

```C
void memo_release(struct hc_malloc *a, void *p) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);
  struct memo *m = hc_baseof(p, struct memo, data);
  *(struct memo **)hc_set_add(&ma->memo, &m->size, true) = m;
}
```