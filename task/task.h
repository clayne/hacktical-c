#ifndef HACKTICAL_TASK_H
#define HACKTICAL_TASK_H

#include <stdbool.h>
#include "list/list.h"

#define hc_task_yield()							\
  task->state = __LINE__;						\
  return;								\
  case __LINE__:;							

struct hc_task_list {
  struct hc_list tasks;
};

struct hc_task;

typedef void (*hc_task_body)(struct hc_task *);

struct hc_task {
  struct hc_list list;
  hc_task_body body;
  int state;
  bool done;
};

struct hc_task_list *hc_task_list_init(struct hc_task_list *tl);
void hc_task_list_run(struct hc_task_list *tl);

struct hc_task *hc_task_init(struct hc_task *t,
			     struct hc_task_list *tl,
			     hc_task_body body);

#endif
