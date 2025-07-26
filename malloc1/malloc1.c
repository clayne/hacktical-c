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

/* Bump */

static void *bump_acquire(struct hc_malloc *a, size_t size) {
  if (size <= 0) {
    hc_throw(HC_INVALID_SIZE);
  } 

  struct hc_bump_alloc *ba = hc_baseof(a, struct hc_bump_alloc, malloc);
  
  if (ba->size - ba->offset < size) {
    hc_throw(HC_NO_MEMORY);
  } 

  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  ba->offset = ba->offset + pa - p + size;
  return pa;
}

static void bump_release(struct hc_malloc *a, void *p) {
  //Do nothing
}

void hc_bump_alloc_init(struct hc_bump_alloc *a,
			struct hc_malloc *source,
			size_t size) {
  a->malloc.acquire = bump_acquire;
  a->malloc.release = bump_release;
  a->source = source;
  a->size = size;
  a->offset = 0;
  a->memory = hc_acquire(source, size);
}

void hc_bump_alloc_deinit(struct hc_bump_alloc *a) {
  hc_release(a->source, a->memory);
}
