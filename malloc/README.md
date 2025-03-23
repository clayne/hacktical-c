## Composable Memory Allocators
Writing your own memory allocator is a common rite of passage for novice C programmers. Which makes sense, since one of the defining features of C is raw memory access. Here we're going to explore a composable design that allows convenient definition of pipelines of allocators with different behaviors.

Allocators are required to support the following API:

```C
struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};
```

The default/root allocator delegates to `malloc`/`free`.

```C
static void *default_acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

static void default_release(struct hc_malloc *m, void *p) {
  free(p);
}

struct hc_malloc hc_malloc = {.acquire = default_acquire,
			      .release = default_release};
```

Next up is a bump allocator; which consists of a fixed block of memory, a size and an offset.

```C
struct hc_bump_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  size_t size, offset;
  uint8_t memory[];
};

struct hc_bump_alloc *hc_bump_alloc_new(struct hc_malloc *source,
					size_t size) {
  struct hc_bump_alloc *a = _hc_acquire(source,
					sizeof(struct hc_bump_alloc) + size);
  a->malloc.acquire = bump_acquire;
  a->malloc.release = bump_release;
  a->source = source;
  a->size = size;
  a->offset = 0;
  return a;
}

void hc_bump_alloc_free(struct hc_bump_alloc *a) {
  _hc_release(a->source, a);
}
```

`acquire()` bumps the offset, while `release()` is a no op.

```C
static void *bump_acquire(struct hc_malloc *m, size_t size) {
  struct hc_bump_alloc *ba = hc_baseof(m, struct hc_bump_alloc, malloc);
  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  const size_t no = ba->offset + pa - p + size;
  
  if (no >= ba->size) {
    hc_throw(0, "Out of memory");
  }   

  ba->offset = no;
  return pa;
}

static void bump_release(struct hc_malloc *m, void *p) {
  // Do nothing
}
```

Recycling memory is a common requirement, we'll design the feature as a separate allocator that can be conveniently added to a pipeline. A multi set ordered by allocation size is used to recycle allocations.

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

static enum hc_order memo_cmp(const void *l, const void *r) {
  return hc_cmp(*(size_t *)l, *(size_t *)r);
}

static const void *memo_key(const void *p) {
  struct memo *m = *(struct memo **)p;
  return &m->size;
}
```

`acquire` first checks for recycled allocations of the correct size, and delegates to the source allocator if none was found.

```C
static void *memo_acquire(struct hc_malloc *a, size_t size) {
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
static void memo_release(struct hc_malloc *a, void *p) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);
  struct memo *m = hc_baseof(p, struct memo, data);
  *(struct memo **)hc_set_add(&ma->memo, &m->size, true) = m;
}
```

A set of macros are provided to simplify usage. `_x()`-variants are meant mostly for internal use with bare `struct hc_malloc`-pointers, while the outer layer adds `&(m)->malloc` to avoid having to type it out at every use.

```C
#define __hc_acquire(m, _m, s) ({		
      struct hc_malloc *_m = m;			
      _m->acquire(_m, s);			
    })

#define _hc_acquire(m, s)			
  __hc_acquire(m, hc_unique(malloc), s)

#define hc_acquire(m, s)			
  _hc_acquire(&(m)->malloc, s)

#define __hc_release(m, _m, p)			
  do {						
    struct hc_malloc *_m = m;
    _m->release(_m, p);				
  } while (0)

#define _hc_release(m, p)
  __hc_release(m, hc_unique(malloc), p)

#define hc_release(m, p)
  _hc_release(&(m)->malloc, p)
```