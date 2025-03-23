#ifndef HACKTICAL_MALLOC_H
#define HACKTICAL_MALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <set/set.h>

//TODO add benchmark (add/free vs memo)

#define __hc_acquire(m, _m, s) ({		\
      struct hc_malloc *_m = m;			\
      _m->acquire(_m, s);			\
    })

#define _hc_acquire(m, s)			\
  __hc_acquire(m, hc_unique(malloc), s)

#define hc_acquire(m, s)			\
  _hc_acquire(&(m)->malloc, s)

#define __hc_release(m, _m, p)			\
  do {						\
    struct hc_malloc *_m = m;			\
    _m->release(_m, p);				\
  } while (0)

#define _hc_release(m, p)			\
  __hc_release(m, hc_unique(malloc), p)

#define hc_release(m, p)			\
  _hc_release(&(m)->malloc, p)

struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

extern struct hc_malloc hc_malloc;

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

/* Memo */

struct hc_memo_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  struct hc_set memo;
};

struct hc_memo_alloc *hc_memo_alloc_init(struct hc_memo_alloc *a,
					 struct hc_malloc *source);

void hc_memo_alloc_deinit(struct hc_memo_alloc *a);


#endif
