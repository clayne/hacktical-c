#include <assert.h>
#include "list.h"

struct my_item {
  struct hc_list ls;
  int value;
};
  
void list_tests() {
  struct hc_list head;
  hc_list_init(&head);

  const int n = 10;
  struct my_item items[n];
  
  for (int i = 0; i < n; i++) {
    items[i].value = i;
    hc_list_push_back(&head, &items[i].ls);
  }

  int i = 0;
  
  hc_list_do(&head, il) {
    assert(hc_baseof(il, struct my_item, ls)->value == i++);
  }
}
