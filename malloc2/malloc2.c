#include <stdlib.h>
#include <stdio.h>
#include "error/error.h"
#include "macro/macro.h"
#include "malloc2.h"

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
  return a;
}

void hc_memo_alloc_deinit(struct hc_memo_alloc *a) {
  hc_vector_do(&a->memo.items, _m) {
    struct memo *m = *(struct memo **)_m;
    _hc_release(a->source, m);
  }
  
  hc_set_deinit(&a->memo);
}

/* Slab */

struct slab {
  struct hc_list slabs;
  uint8_t *next;
  uint8_t memory[];
};

static struct slab *add_slab(struct hc_slab_alloc *a) {
  struct slab *s = _hc_acquire(a->source,
			       sizeof(struct slab) +
			       a->slab_size);
  
  hc_list_push_front(&a->slabs, &s->slabs);
  s->next = s->memory;
  return s;
}

static struct slab *get_slab(struct hc_slab_alloc *a, const size_t size) {
  struct slab *result = NULL;
  
  hc_list_do(&a->slabs, sl) {
    struct slab *s = hc_baseof(sl, struct slab, slabs);
    uint8_t *p = hc_align(s->next, size);

    if (p + size > s->memory + a->slab_size) {
      break;
    }

    result = s;
  }

  return result ? result : add_slab(a);
}

static void *slab_acquire(struct hc_malloc *a, const size_t size) {
  struct hc_slab_alloc *sa = hc_baseof(a, struct hc_slab_alloc, malloc);

  if (size > sa->slab_size) {
    hc_throw(HC_INVALID_SIZE);
  }

  struct slab *s = get_slab(sa, size);
  uint8_t *p = hc_align(s->next, size);
  s->next = p + size;

  while (s->slabs.next != &s->slabs) {
    struct slab *ns = hc_baseof(s->slabs.next, struct slab, slabs);

    if (ns->next - ns->memory > s->next - s->memory) {
      hc_list_shift_back(&s->slabs);
    } else {
      break;
    }
  }
  
  return p;
}

static void slab_release(struct hc_malloc *a, void *p) {
  // Do nothing
}

struct hc_slab_alloc *hc_slab_alloc_init(struct hc_slab_alloc *a,
					 struct hc_malloc *source,
					 size_t slot_count,
					 size_t slot_size) {
  a->malloc.acquire = slab_acquire;
  a->malloc.release = slab_release;
  a->source = source;
  hc_list_init(&a->slabs);
  a->slab_size = slot_count * slot_size;
  return a;
}

void hc_slab_alloc_deinit(struct hc_slab_alloc *a) {
  hc_list_do(&a->slabs, _s) {
    struct slab *s = hc_baseof(_s, struct slab, slabs);
    _hc_release(a->source, s);
  }
}
