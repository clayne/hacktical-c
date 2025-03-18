#include <stddef.h>
#include "list.h"

void hc_list_init(struct hc_list *l) {
  l->prev = l->next = l;
}

struct hc_list *hc_list_delete(struct hc_list *l) {
  l->prev->next = l->next;
  l->next->prev = l->prev;
  return l;
}

void hc_list_push_front(struct hc_list *l, struct hc_list *it) {
  hc_list_push_back(l->next, it);
}

struct hc_list *hc_list_pop_front(struct hc_list *l) {
  struct hc_list *it = l->next;
  return (it == l) ? NULL : hc_list_delete(it);
}

struct hc_list *hc_list_peek_front(struct hc_list *l) {
  struct hc_list *it = l->next;
  return (it == l) ? NULL : it;
}

void hc_list_push_back(struct hc_list *l, struct hc_list *it) {
  it->prev = l->prev;
  l->prev->next = it;
  it->next = l;
  l->prev = it;
}

struct hc_list *hc_list_pop_back(struct hc_list *l) {
  struct hc_list *it = l->prev;
  return (it == l) ? NULL : hc_list_delete(it);
}

struct hc_list *hc_list_peek_back(struct hc_list *l) {
  struct hc_list *it = l->prev;
  return (it == l) ? NULL : it;
}

#include "tests.c"
