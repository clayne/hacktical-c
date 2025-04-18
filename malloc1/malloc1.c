#include <stdlib.h>
#include <stdio.h>
#include "error/error.h"
#include "macro/macro.h"
#include "malloc1.h"

static void *default_acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

static void default_release(struct hc_malloc *m, void *p) {
  free(p);
}

struct hc_malloc hc_malloc_default = {.acquire = default_acquire,
				      .release = default_release};

__thread struct hc_malloc *hc_mallocp = NULL;

struct hc_malloc *hc_malloc() {
  return hc_mallocp ? hc_mallocp : &hc_malloc_default;
}

size_t hc_alignof(size_t size) {
  const size_t max = _Alignof(max_align_t);
  if (size >= max) { return max; }
  size_t v = 1;
  while (v < size) { v <<= 1; }
  return v;
}

/* Bump */

static void *bump_acquire(struct hc_malloc *a, size_t size) {
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

static void bump_release(struct hc_malloc *a, void *p) {
  // Do nothing
}

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
