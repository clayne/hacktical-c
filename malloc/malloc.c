#include <stdlib.h>
#include <stdio.h>
#include "error/error.h"
#include "macro/macro.h"
#include "malloc.h"

static void *default_acquire(struct hc_malloc *m, size_t size) {
  return malloc(size);
}

static void default_release(struct hc_malloc *m, void *p) {
  free(p);
}

__thread struct hc_malloc default_malloc = {.acquire = default_acquire,
					    .release = default_release};

__thread struct hc_malloc *hc_malloc = NULL;

void hc_malloc_init() {
  hc_malloc = &default_malloc;
}

/* Bump */

static void *bump_acquire(struct hc_malloc *a, size_t size) {
  struct hc_bump_alloc *ba = hc_baseof(a, struct hc_bump_alloc, malloc);
  uint8_t *p = ba->memory + ba->offset;
  uint8_t *pa = hc_align(p, size);
  const size_t no = ba->offset + pa - p + size;
  
  if (no >= ba->size) {
    hc_throw(0, "Out of memory");
  }   

  ba->offset = no;
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

/* Memo */

struct memo {
  size_t size;
  uint8_t data[];
};

static void *memo_acquire(struct hc_malloc *a, size_t size) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);

  if (hc_set_length(&ma->memo)) {
    bool ok = false;
    size_t i = hc_set_index(&ma->memo, &size, &ok);

    if (ok) {
      struct hc_vector *is = &ma->memo.items;
      struct memo *m = *(struct memo **)hc_vector_get(is, i);
      hc_vector_delete(is, i, 1);
      return m->data;
    }
  }

  struct memo *m = _hc_acquire(ma->source, sizeof(struct memo) + size);
  m->size = size;
  return m->data;
}

static void memo_release(struct hc_malloc *a, void *p) {
  struct hc_memo_alloc *ma = hc_baseof(a, struct hc_memo_alloc, malloc);
  struct memo *m = hc_baseof(p, struct memo, data);
  *(struct memo **)hc_set_add(&ma->memo, &m->size, true) = m;
}

static enum hc_order memo_cmp(const void *l, const void *r) {
  return hc_cmp(*(size_t *)l, *(size_t *)r);
}

static const void *memo_key(const void *p) {
  struct memo *m = *(struct memo **)p;
  return &m->size;
}

struct hc_memo_alloc *hc_memo_alloc_init(struct hc_memo_alloc *a,
					 struct hc_malloc *source) {
  a->malloc.acquire = memo_acquire;
  a->malloc.release = memo_release;
  a->source = source;
  hc_set_init(&a->memo, sizeof(struct memo *), memo_cmp);
  a->memo.key = memo_key;
}

void hc_memo_alloc_deinit(struct hc_memo_alloc *a) {
  hc_vector_do(&a->memo.items, _m) {
    struct memo *m = *(struct memo **)_m;
    _hc_release(a->source, m);
  }
  
  hc_set_deinit(&a->memo);
}
