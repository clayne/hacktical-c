#ifndef HACKTICAL_MALLOC_H
#define HACKTICAL_MALLOC_H

#include <stddef.h>
#include <stdint.h>

//TODO Add bump test/benchmark
//TODO Figure out how to insert source in chain

struct hc_malloc {
  void *(*acquire)(struct hc_malloc *, size_t);
  void (*release)(struct hc_malloc *, void *);
};

void *hc_acquire(struct hc_malloc *m, size_t size);
void hc_release(struct hc_malloc *m, void *v);

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
