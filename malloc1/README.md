## Composable Memory Allocators - Part 1
Writing your own memory allocator is a common rite of passage for novice C programmers. Which makes sense, since one of the defining features of C is direct memory access.

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
```

```C
#define _hc_acquire(m, _m, s) ({		
      struct hc_malloc *_m = m;			
      assert(_m->acquire);			
      _m->acquire(_m, s);			
    })

#define hc_acquire(m, s)			
  _hc_acquire(m, hc_unique(malloc_m), s)

#define _hc_release(m, _m, p) do {		
  struct hc_malloc *_m = m;			
  assert(_m->release);				
  _m->release(_m, p);				
} while (0)
    
#define hc_release(m, p)			
  _hc_release(m, hc_unique(malloc_m), p)
```

### Alignment
Before we dive into the first real implementation, alignment deserves a brief discussion. The short story is that the CPU requires data to be aligned to size multiples, meaning the start address is required to be a multiple of the size (up to `_Alignof(max_align_t)`). Since this is something we're going to do now and then, `hc_align()` is provided to simplify the process.

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
  for (size_t nv = 1; nv <= size; v = nv, nv = v << 1);
  return v;
}
```

### Bump Allocation

A bump allocator consists of a fixed block of memory, a size and an offset.

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
  a->memory = hc_acquire(source, size);
}

void hc_bump_alloc_deinit(struct hc_bump_alloc *a) {
  hc_release(a->source, a->memory);
}
```

`acquire()` bumps the offset to a correctly aligned address plus the requested size.

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
```

`release()` is a no op.

```C
void bump_release(struct hc_malloc *m, void *p) {
  // Do nothing
}
```

Continued in [Part 2](https://github.com/codr7/hacktical-c/tree/main/malloc2).