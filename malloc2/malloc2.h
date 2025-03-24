#ifndef HACKTICAL_MALLOC2_H
#define HACKTICAL_MALLOC2_H

#include <stddef.h>
#include <stdint.h>
#include <set/set.h>
#include "malloc1/malloc1.h"

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
