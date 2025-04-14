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

/* Bump */

static void *bump_acquire(struct hc_malloc *a, size_t size) {
  struct hc_bump_alloc *ba = hc_baseof(a, struct hc_bump_alloc, malloc);
  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  const size_t new_offset = ba->offset + pa - p + size;
  
  if (new_offset >= ba->size) {
    hc_throw(0, "Out of memory");
  }   

  ba->offset = new_offset;
  return pa;
}

static void bump_release(struct hc_malloc *a, void *p) {
  // Do nothing
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
