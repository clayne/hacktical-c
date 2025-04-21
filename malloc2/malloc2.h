#ifndef HACKTICAL_MALLOC2_H
#define HACKTICAL_MALLOC2_H

#include <stddef.h>
#include <stdint.h>

#include "list/list.h"
#include "malloc1/malloc1.h"
#include "set/set.h"

/* Memo */

struct hc_memo_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  struct hc_set memo;
};

struct hc_memo_alloc *hc_memo_alloc_init(struct hc_memo_alloc *a,
					 struct hc_malloc *source);

void hc_memo_alloc_deinit(struct hc_memo_alloc *a);

/* Slab */

struct hc_slab_alloc {
  struct hc_malloc malloc;
  struct hc_malloc *source;
  struct hc_list slabs;

  size_t slot_count;
  size_t slot_size;
  size_t slot_index;
};

struct hc_slab_alloc *hc_slab_alloc_init(struct hc_slab_alloc *a,
					 struct hc_malloc *source,
					 size_t slot_count,
					 size_t slot_size);

void hc_slab_alloc_deinit(struct hc_slab_alloc *a);

#endif
