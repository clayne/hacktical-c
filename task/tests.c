#include <assert.h>
#include <stdio.h>
#include "task.h"

int pc = 0, cc = 0;

static void producer(struct hc_task *task) {
  switch (task->state) {
  case 0:
    assert(cc == 0);
    pc++;
    hc_task_yield();
    assert(cc == 1);
    pc++;
  }
  
  task->done = true;
}

static void consumer(struct hc_task *task) {
  switch (task->state) {
  case 0:
    assert(pc == 1);
    cc++;
    hc_task_yield();
    assert(pc == 2);
    cc++;
  }
  
  task->done = true;
}

void task_tests() {
  struct hc_task_list tl;
  hc_task_list_init(&tl);
  
  struct hc_task pt, ct;
  hc_task_init(&pt, &tl, &producer);
  hc_task_init(&ct, &tl, &consumer);

  hc_task_list_run(&tl); 
  assert(pc == 2);
  assert(cc == 2);
}
