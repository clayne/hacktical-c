#include <stddef.h>
#include <stdlib.h>
#include "task.h"

struct hc_task_list *hc_task_list_init(struct hc_task_list *tl) {
  hc_list_init(&tl->tasks);
}

void hc_task_list_run(struct hc_task_list *tl) {
  bool all_done = false;
  
  while (!all_done) {
    all_done = true;
    
    hc_list_do(&tl->tasks, i) {
      struct hc_task *t = hc_baseof(i, struct hc_task, list);

      if (!t->done) {
	t->body(t);
	all_done = false;
      }
    }
  }
}

struct hc_task *hc_task_init(struct hc_task *t,
			     struct hc_task_list *tl,
			     hc_task_body body) {
  t->body = body;
  t->state = 0;
  t->done = false;
  hc_list_push_back(&tl->tasks, &t->list);
}
