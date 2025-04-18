## Composable Memory Allocators - Part 1
Writing your own memory allocator is a common rite of passage for novice C programmers. Which makes sense, since one of the defining features of C is raw memory access.

Here we're going to explore a composable design that allows conveniently custom tailoring the allocation strategy. Individual allocators follow the Unix-principle of doing one thing well.

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

struct hc_malloc hc_malloc_default = {.acquire = default_acquire,
				      .release = default_release};

__thread struct hc_malloc *hc_mallocp = NULL;

struct hc_malloc *hc_malloc() {
  return hc_mallocp ? hc_mallocp : &hc_malloc_default;
}
```

A set of macros are provided to simplify use. `_x()`-variants are intended for use with bare `struct hc_malloc`-pointers, while the outer layer adds `->malloc` to avoid having to type it out at every use.

```C
#define __hc_malloc_do(m, _pm, _done)		
  bool _done = false;				
  for (struct hc_malloc *_pm = hc_mallocp;	
       !_done && (hc_mallocp = (m));		
       hc_mallocp = _pm, _done = true)

#define _hc_malloc_do(m)						
  __hc_malloc_do(m, hc_unique(malloc_pm), hc_unique(malloc_done))

#define hc_malloc_do(m)				
  _hc_malloc_do(&(m)->malloc)

#define __hc_acquire(m, _m, s) ({		
      struct hc_malloc *_m = m;			
      _m->acquire(_m, s);			
    })

#define _hc_acquire(m, s)			
  __hc_acquire(m, hc_unique(malloc_m), s)

#define hc_acquire(s)				
  _hc_acquire(hc_malloc(), s)

#define __hc_release(m, _m, p) do {		
  struct hc_malloc *_m = m;			
  _m->release(_m, p);
} while (0)

#define _hc_release(m, p)			
  __hc_release(m, hc_unique(malloc_m), p)

#define hc_release(p)				
  _hc_release(hc_malloc(), p)
```

### Alignment
Before we dive into the first real implementation, alignment deserves a brief discussion. The short story is that the CPU requires data to be aligned to size multiples, meaning the start address is required to be a multiple of the size (up to `_Alignof(max_align_t)`). Since this is something we're going to do now and then, a macro is provided to simplify the process.

```C
#define hc_align(base, size) ({						
      __auto_type _base = base;						
      __auto_type _size = hc_alignof(size);				
      __auto_type _rest = (ptrdiff_t)_base % _size;			
      (_rest) ? _base + _size - _rest : _base;				
    })

size_t hc_alignof(size_t size) {
  const size_t max = _Alignof(max_align_t);
  if (size >= max) { return max; }
  size_t v = 1;
  while (v < size) { v <<= 1; }
  return v;
}
```

### Bump Allocation

A bump allocator consists of a fixed block of memory, a size and an offset. Bump allocators lack support for memory recycling.

```C
struct hc_bump_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  size_t size, offset;
  uint8_t *memory;
};

void hc_bump_alloc_init(struct hc_bump_alloc *a,
			struct hc_malloc *source,
			size_t size) {
  a->malloc.acquire = bump_acquire;
  a->malloc.release = bump_release;
  a->source = source;
  a->size = size;
  a->offset = 0;
  a->memory = _hc_acquire(source, size);
}

void hc_bump_alloc_deinit(struct hc_bump_alloc *a) {
  _hc_release(a->source, a->memory);
}
```

`acquire()` bumps the offset to a correctly aligned address plus the requested size, `release()` is a no op.

```C
void *bump_acquire(struct hc_malloc *m, size_t size) {
  if (size <= 0) {
    hc_throw(HC_INVALID_SIZE, "Invalid size");
  } 

  struct hc_bump_alloc *ba = hc_baseof(a, struct hc_bump_alloc, malloc);
  
  if (ba->size - ba->offset < size) {
    hc_throw(HC_NO_MEMORY, "Out of memory");
  } 

  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  ba->offset = ba->offset + pa - p + size;
  return pa;
}

void bump_release(struct hc_malloc *m, void *p) {
  // Do nothing
}
```

Continued in [Part 2](https://github.com/codr7/hacktical-c/tree/main/malloc2).