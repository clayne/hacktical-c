## Lightweight Concurrent Tasks
Concurrent tasks allows keeping the flow of control intact where it would otherwise get lost in the noise of a solution based on discrete events. Note that concurrent means interleaved, not parallel; system threads would add a lot of complexity and overhead in comparison. C lacks built in support for coroutines, but the same effect can be achieved without breaking any rules.

We start by defining an abstraction to represent a task.

```C
struct hc_task {
  struct hc_list list;
  hc_task_body body;
  int state;
  bool done;
};
```

The task body is simply a regular pointer to a function taking a `struct hc_task *`-argument.

```C
typedef void (*hc_task_body)(struct hc_task *);
```

We also need a way to track of a list of tasks, a scheduler.

```C
struct hc_task_list {
  struct hc_list tasks;
};
```

To keep it simple, we'll simply keep running tasks until all are `done`. One obvious improvement would be to keep running tasks on a separate list.

```C
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
```

We now have all the pieces needed to define our tasks. For this example, we'll use a simple counter to represent the data being produced/consumed.

```C
struct my_task {
  struct hc_task task;
  int *value;
};

void producer(struct hc_task *task);
void consumer(struct hc_task *task);

int main() {
  struct hc_task_list tl;
  hc_task_list_init(&tl);
  
  int value = 0;  

  struct my_task pt = {.value = &value};
  hc_task_init(&pt.task, &tl, &producer);

  struct my_task ct = {.value = &value};
  hc_task_init(&ct.task, &tl, &consumer);

  hc_task_list_run(&tl);
}
```

The producer increases the counter and then yields control to the consumer, execution resumes on the next line following `hc_task_yield()` on next call.

```C
void producer(struct hc_task *task) {
  int *value = hc_baseof(task, struct my_task, task)->value;
  
  switch (task->state) {
  case 0:
    (*value)++;
    hc_task_yield(task);
    (*value)++;
  }
  
  task->done = true;
}
```

The consumer decreases the counter in a similar fashion.

```C
static void consumer(struct hc_task *task) {
  int *value = hc_baseof(task, struct my_task, task)->value;

  switch (task->state) {
  case 0:
    (*value)--;
    hc_task_yield(task);
    (*value)--;
  }
  
  task->done = true;
}
```

`hc_task_yield()` is implemented as a macro that updates the task state and adds a matching `case`. The `__LINE__` macro expands to the current source code line number.

```C
#define hc_task_yield(task)			
  do {					
    task->state = __LINE__;			
    return;					
    case __LINE__:;			        
  } while (0)				      
```

The reason this works as well as it does is because C allows `case` to appear at any nesting level within a `switch`. The discovery of this feature is often credited to a guy named [Tom Duff](https://en.wikipedia.org/wiki/Duff%27s_device).

### Limitations
Since we're skipping around inside the task's function body, any local variables that span calls to `hc_task_yield()` need to be placed inside `struct my_task`.