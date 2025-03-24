#ifndef HACKTICAL_MALLOC1_H
#define HACKTICAL_MALLOC1_H

#include <stddef.h>
#include <stdint.h>

#define __hc_malloc_do(m, _pm)						\
  for (struct hc_malloc *_pm = hc_malloc;				\
       _pm && (hc_malloc = (m));					\
       hc_malloc = _pm, _pm = NULL)

#define _hc_malloc_do(m)			\
  __hc_malloc_do(m, hc_unique(malloc_p))

#define hc_malloc_do(m)				\
  _hc_malloc_do(&(m)->malloc)

#define __hc_acquire(m, _m, s) ({		\
      struct hc_malloc *_m = m;			\
      _m->acquire(_m, s);			\
    })

#define _hc_acquire(m, s)			\
  __hc_acquire(m, hc_unique(malloc_m), s)

#define hc_acquire(s)				\
  _hc_acquire(hc_malloc, s)

#define __hc_release(m, _m, p)			\
  struct hc_malloc *_m = m;			\
  _m->release(_m, p)

#define _hc_release(m, p)			\
  __hc_release(m, hc_unique(malloc_m), p)

#define hc_release(p)				\
  _hc_release(hc_malloc, p)


struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

extern __thread struct hc_malloc *hc_malloc;

void hc_malloc_init();

/* Bump */

struct hc_bump_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  size_t size, offset;
  uint8_t memory[];
};

struct hc_bump_alloc *hc_bump_alloc_new(struct hc_malloc *source,
					size_t size);

void hc_bump_alloc_free(struct hc_bump_alloc *a);

#endif
