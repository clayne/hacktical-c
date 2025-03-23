#ifndef HACKTICAL_MALLOC_H
#define HACKTICAL_MALLOC_H

#include <stddef.h>
#include <stdint.h>

//TODO add slab alloc
//TODO add ordered sets
//TODO add memo_alloc
//TODO add bump benchmark

#define _hc_acquire(_m, s) ({			\
      struct hc_malloc *m = _m;			\
      m->acquire(m, s);				\
    })

#define hc_acquire(m, s)			\
  _hc_acquire(&(m)->malloc, s)

#define _hc_release(_m, p)			\
  do {						\
    struct hc_malloc *m = _m;			\
    m->release(m, p);				\
  } while (0)

#define hc_release(m, p)			\
  _hc_release(&(m)->malloc, p)


struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

extern struct hc_malloc hc_malloc;

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
