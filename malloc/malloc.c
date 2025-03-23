#include <stdlib.h>
#include "macro/macro.h"
#include "malloc.h"

static void *acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

static void release(struct hc_malloc *m, void *p) {
  free(p);
}

struct hc_malloc hc_malloc = {.acquire = acquire,
			      .release = release};


static void *bump_acquire(struct hc_malloc *m, size_t size) {
  struct hc_bump_alloc *bm = hc_baseof(m, struct hc_bump_alloc, malloc);
  uint8_t *p = bm->memory + bm->offset;
  uint8_t *pa = hc_align(p, size);
  bm->offset += pa - p + size;
  return pa;
}

static void bump_release(struct hc_malloc *m, void *p) {
  // Release is a no op in a bump allocator
}

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
