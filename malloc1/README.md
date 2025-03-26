## Composable Memory Allocators - Part 1
Writing your own memory allocator is a common rite of passage for novice C programmers. Which makes sense, since one of the defining features of C is raw memory access.

Here we're going to explore a composable design that allows defining pipelines of allocators to conveniently custom tailor the allocation strategy. Individual allocators follow the Unix-principle of doing one thing well.

Allocators are required to support the following API:

```C
struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};
```

The default/root allocator delegates to `malloc`/`free`.

```C
void *default_acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

void default_release(struct hc_malloc *m, void *p) {
  free(p);
}

struct hc_malloc hc_malloc = {.acquire = default_acquire,
			      .release = default_release};
```

A set of macros are provided to simplify use. `_x()`-variants are intended for use with bare `struct hc_malloc`-pointers, while the outer layer adds `->malloc` to avoid having to type it out at every use.

```C
#define __hc_malloc_do(m, _pm)						
  for (struct hc_malloc *_pm = hc_malloc;				
       _pm && (hc_malloc = (m));					
       hc_malloc = _pm, _pm = NULL)

#define _hc_malloc_do(m)			
  __hc_malloc_do(m, hc_unique(malloc_p))

#define hc_malloc_do(m)				
  _hc_malloc_do(&(m)->malloc)

#define __hc_acquire(m, _m, 
      struct hc_malloc *_m = m;			
      _m->acquire(_m, s);			
    })

#define _hc_acquire(m, s)			
  __hc_acquire(m, hc_unique(malloc_m), s)

#define hc_acquire(s)				
  _hc_acquire(hc_malloc, s)

#define __hc_release(m, _m, p)			
  struct hc_malloc *_m = m;			
  _m->release(_m, p)

#define _hc_release(m, p)			
  __hc_release(m, hc_unique(malloc_m), p)

#define hc_release(p)				
  _hc_release(hc_malloc, p)
```

### Bump Allocation

A bump allocator consists of a fixed block of memory, a size and an offset. The memory block is allocated together with the structure. Bump allocators lack built-in support for releasing memory.

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
void *bump_acquire(struct hc_malloc *m, size_t size) {
  struct hc_bump_alloc *ba = hc_baseof(m, struct hc_bump_alloc, malloc);
  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  const size_t new_offset = ba->offset + pa - p + size;
  
  if (new_offset >= ba->size) {
    hc_throw(0, "Out of memory");
  }   

  ba->offset = new_offset;
  return pa;
}

void bump_release(struct hc_malloc *m, void *p) {
  // Do nothing
}
```