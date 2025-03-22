#include <stdlib.h>
#include "malloc.h"

void *hc_acquire(struct hc_malloc *m, size_t size) {
  return m->acquire(m, size);
}

void hc_release(struct hc_malloc *m, void *v) {
  m->release(m, v);
}

static void *acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

static void release(struct hc_malloc *m, void *p) {
  free(p);
}

struct hc_malloc hc_malloc = {.acquire = acquire,
			      .release = release};

struct hc_bump_alloc *hc_bump_alloc_new(struct hc_malloc *source,
					size_t size) {
  struct hc_bump_alloc *a = hc_acquire(source,
				       sizeof(struct hc_bump_alloc) + size);
  a->source = source;
  a->size = size;
  a->offset = 0;
  return a;
}

void hc_bump_alloc_free(struct hc_bump_alloc *a) {
  hc_release(a->source, a);
}
