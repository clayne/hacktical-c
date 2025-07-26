#ifndef HACKTICAL_MALLOC1_H
#define HACKTICAL_MALLOC1_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define _hc_acquire(m, _m, s) ({		\
      struct hc_malloc *_m = m;			\
      assert(_m->acquire);			\
      _m->acquire(_m, s);			\
    })

#define hc_acquire(m, s)			\
  _hc_acquire(m, hc_unique(malloc_m), s)

#define _hc_release(m, _m, p) do {		\
  struct hc_malloc *_m = m;			\
  assert(_m->release);				\
  _m->release(_m, p);				\
} while (0)
    
#define hc_release(m, p)			\
  _hc_release(m, hc_unique(malloc_m), p)

struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

extern struct hc_malloc hc_malloc_default;

/* Bump */

struct hc_bump_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  size_t size, offset;
  uint8_t *memory;
};

void hc_bump_alloc_init(struct hc_bump_alloc *a,
			struct hc_malloc *source,
			size_t size);

void hc_bump_alloc_deinit(struct hc_bump_alloc *a);

#endif
