## Composable Memory Allocators - Part 2
Welcome back to memory allocator land.

We now have enough features in place to implement more elaborate allocators.

It might make sense to give [Part 1](https://github.com/codr7/hacktical-c/tree/main/malloc1) a quick scan before diving in.

### Recycling Memory
Memory recycling is a common requirement, we'll design the feature as a separate allocator that can be plugged in at any point.

Example:
```C
struct hc_memo_alloc a;
hc_memo_alloc_init(&a, hc_malloc());

hc_malloc_do(&a) {
  const int *p = hc_acquire(sizeof(int));
  hc_release(p);
  assert(hc_acquire(sizeof(int)) == p);
}
```

A multi-`struct hc_set` keyed on size is used to track allocations.

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

`release` registers the allocation for recycling.

```C
void memo_release(struct hc_malloc *a, void *p) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);
  struct memo *m = hc_baseof(p, struct memo, data);
  *(struct memo **)hc_set_add(&ma->memo, &m->size, true) = m;
}
```

### Slab Allocation
Slab allocators acquire memory in fixed size blocks. They're commonly used in combination with a fixed allocation size, where each block (or slab) contains the same number of slots. We're going to introduce a tiny bit of flexibility to allow different allocation sizes.

Example:
```C
struct hc_slab_alloc a;
hc_slab_alloc_init(&a, hc_malloc(), 2, sizeof(int));

hc_malloc_do(&a) {
  // Same slab
  const int *p1 = hc_acquire(sizeof(int));
  const int *p2 = hc_acquire(sizeof(int));
  assert(p2 == p1 + 1);

  // New slab
  const int *p3 = hc_acquire(sizeof(int));
  assert(p3 > p2 + 1);
}
```

We'll use a `struct hc_list` to keep track of our slabs.

```C
struct hc_slab_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  struct hc_list slabs;
  size_t slab_size;
};

struct hc_slab_alloc *hc_slab_alloc_init(struct hc_slab_alloc *a,
					 struct hc_malloc *source,
					 size_t slab_size) {
  a->malloc.acquire = slab_acquire;
  a->malloc.release = slab_release;
  a->source = source;
  hc_list_init(&a->slabs);
  a->slab_size = slab_size;
  return a;
}

void hc_slab_alloc_deinit(struct hc_slab_alloc *a) {
  hc_list_do(&a->slabs, _s) {
    struct slab *s = hc_baseof(_s, struct slab, slabs);
    _hc_release(a->source, s);
  }
}
```

Slabs are defined as dynamically sized structs that track the current offset.

```C
struct slab {
  struct hc_list slabs;
  uint8_t *next;
  uint8_t memory[];
};
```

Slabs are ordered by available memory, descending. Finding a slab means moving down the list until we reach a slab that can't fit the allocation and returning the previous slab. Allocation with sizes that exceed the slab size skip the search.

```C
struct slab *get_slab(struct hc_slab_alloc *a, const size_t size) {
  if (size > a->slab_size) {
    return add_slab(a, size);
  }

  struct slab *result = NULL;
  
  hc_list_do(&a->slabs, sl) {
    struct slab *s = hc_baseof(sl, struct slab, slabs);
    uint8_t *p = hc_align(s->next, size);

    if (p + size > s->memory + a->slab_size) {
      break;
    }

    result = s;
  }

  return result ? result : add_slab(a, a->slab_size);
}
```

If no suitable slabs are found, we add a new one.

```
struct slab *add_slab(struct hc_slab_alloc *a, size_t size) {
  struct slab *s = _hc_acquire(a->source,
			       sizeof(struct slab) +
			       size);
  
  hc_list_push_front(&a->slabs, &s->slabs);
  s->next = s->memory;
  return s;
}
```

`acquire()` gets a slab and adjusts it's position in the list before returning a pointer to the new allocation.

```C
void *slab_acquire(struct hc_malloc *a, const size_t size) {
  struct hc_slab_alloc *sa = hc_baseof(a, struct hc_slab_alloc, malloc);

  struct slab *s = get_slab(sa, size);
  uint8_t *p = hc_align(s->next, size);
  s->next = p + size;

  while (s->slabs.next != &s->slabs) {
    struct slab *ns = hc_baseof(s->slabs.next, struct slab, slabs);

    if (ns->next - ns->memory > s->next - s->memory) {
      hc_list_shift_back(&s->slabs);
    } else {
      break;
    }
  }
  
  return p;
}
```

`release()` is a no op.

```
void slab_release(struct hc_malloc *a, void *p) {
  // Do nothing
}
```