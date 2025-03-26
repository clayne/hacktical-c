#ifndef HACKTICAL_LIST_H
#define HACKTICAL_LIST_H

#include <stdbool.h>
#include "../macro/macro.h"

#define _hc_list_do(l, i, _next)				\
  for (struct hc_list *i = (l)->next, *_next = i->next;		\
       i != (l);						\
       i = _next, _next = i->next)

#define hc_list_do(l, i)			\
  _hc_list_do(l, i, hc_unique(next))

struct hc_list {
  struct hc_list *prev, *next;
};

void hc_list_init(struct hc_list *l);
bool hc_list_nil(const struct hc_list *l);
struct hc_list *hc_list_delete(struct hc_list *l);

void hc_list_push_front(struct hc_list *l, struct hc_list *it);
struct hc_list *hc_list_pop_front(struct hc_list *l);
struct hc_list *hc_list_peek_front(struct hc_list *l);

void hc_list_push_back(struct hc_list *l, struct hc_list *it);
struct hc_list *hc_list_pop_back(struct hc_list *l);
struct hc_list *hc_list_peek_back(struct hc_list *l);

#endif
