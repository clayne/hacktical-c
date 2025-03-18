## Lightweight Concurrent Tasks
Concurrent tasks allows keeping the flow of control intact where it would otherwise easily get lost in the noise of a solution based on discrete events. Note that concurrent only means interleaved, not parallel; which means that system threads would add a lot of complexity and overhead for no gain. C lacks built in support for coroutines, but if you're willing to live with a few limitations, the same effect can be achieved without breaking any rules.

We'll start by defining an abstraction to represent a task.

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

We'll also need a way to track of a list of tasks, a scheduler.

```
struct hc_task_list {
  struct hc_list tasks;
};
```

We now have all the pieces needed to define our tasks. For this example, we'll use a simple counter to represent the data being produced/consumed, but you could pass any kind of data.

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

The producer increases the counter and then yields control to the consumer, execution resumes on the next line following `hc_task_yield()` on the next call.

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

While the consumer decreases the counter in a similar fashion.

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

`hc_task_yield()` is implemented as a macro that updates the task state and adds a `case`.

```C
#define hc_task_yield(task)			
  do {					
    task->state = __LINE__;			
    return;					
    case __LINE__:;			        
  } while (0)				      
```

The reason this works is because C allows `case` to appear at any nesting level within a `switch`. The discovery of this fact is often credited to a guy named [Tom Duff](https://en.wikipedia.org/wiki/Duff%27s_device).

### Limitations
Since we're skipping around inside the function, any local variables that span calls to `hc_task_yield()` need to be placed inside `struct my_task`.