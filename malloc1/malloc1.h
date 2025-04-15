#ifndef HACKTICAL_MALLOC1_H
#define HACKTICAL_MALLOC1_H

#include <stddef.h>
#include <stdint.h>

#define __hc_malloc_do(m, _pm, _done)		\
  bool _done = false;				\
  for (struct hc_malloc *_pm = hc_mallocp;	\
       !_done && (hc_mallocp = (m));		\
       hc_mallocp = _pm, _done = true)

#define _hc_malloc_do(m)						\
  __hc_malloc_do(m, hc_unique(malloc_pm), hc_unique(malloc_done))

#define hc_malloc_do(m)				\
  _hc_malloc_do(&(m)->malloc)

#define __hc_acquire(m, _m, s) ({		\
      struct hc_malloc *_m = m;			\
      _m->acquire(_m, s);			\
    })

#define _hc_acquire(m, s)			\
  __hc_acquire(m, hc_unique(malloc_m), s)

#define hc_acquire(s)				\
  _hc_acquire(hc_malloc(), s)

#define __hc_release(m, _m, p) do {		\
  struct hc_malloc *_m = m;			\
  _m->release(_m, p);				\
} while (0)
    
#define _hc_release(m, p)			\
  __hc_release(m, hc_unique(malloc_m), p)

#define hc_release(p)				\
  _hc_release(hc_malloc(), p)


struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

extern __thread struct hc_malloc *hc_mallocp;
struct hc_malloc *hc_malloc();

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
