#include <assert.h>
#include <stdio.h>
#include "task.h"

struct my_task {
  struct hc_task task;
  int *value;
};

static void producer(struct hc_task *task) {
  int *value = hc_baseof(task, struct my_task, task)->value;
  
  switch (task->state) {
  case 0:
    assert(*value == 0);
    (*value)++;
    hc_task_yield(task);
    assert(*value == 2);
    (*value)++;
  }
  
  task->done = true;
}

static void consumer(struct hc_task *task) {
  int *value = hc_baseof(task, struct my_task, task)->value;

  switch (task->state) {
  case 0:
    assert(*value == 1);
    (*value)++;
    hc_task_yield(task);
    assert(*value == 3);
    (*value)++;
  }
  
  task->done = true;
}
  
void task_tests() {
  struct hc_task_list tl;
  hc_task_list_init(&tl);
  
  int value = 0;  

  struct my_task pt = {.value = &value};
  hc_task_init(&pt.task, &tl, &producer);

  struct my_task ct = {.value = &value};
  hc_task_init(&ct.task, &tl, &consumer);

  hc_task_list_run(&tl);
  assert(value == 4);
}
