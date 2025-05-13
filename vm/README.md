## Virtual Machines
A virtual machine (VM) is a machine emulated using software; commonly used to implement programming languages and emulators.

Virtual machines come in two main flavors; stack based like Forth, Java or Python; and register based like Lua or Erlang.

Stack based machines use smaller instructions, since the stack takes care of addressing; on the other hand they require evaluating more operations to reorder values on the stack. Register based machines keep values in slots and use wider instructions that contain the addresses they operate on.

Here we will build a simple stack based machine. Our machine consists of an environment, a stack and the code. For reasons that will be explained shortly, we'll store the operations and the code to be evaluated separately.

```C
struct hc_vm {
  struct hc_set env;
  struct hc_vector stack;  
  struct hc_vector ops;
  struct hc_vector code;
};
```

The environment maps strings to [values](https://github.com/codr7/hacktical-c/tree/main/reflect).

```C
enum hc_order hc_strcmp(const char *x, const char *y) {
  const int result = strcmp(x, y);
  if (!result) { return HC_EQ; }
  return (result < 0) ? HC_LT : HC_GT;
}

enum hc_order env_cmp(const void *x, const void *y) {
  return hc_strcmp(*(const char **)x, *(const char **)y);
}

const void *env_key(const void *x) {
  return &((const struct env_item *)x)->key;
}

void hc_vm_init(struct hc_vm *vm, struct hc_malloc *malloc) {
  hc_set_init(&vm->env, malloc, sizeof(struct env_item), env_cmp);
  vm->env.key = env_key;

  hc_vector_init(&vm->stack, malloc, sizeof(struct hc_value));
  hc_vector_init(&vm->ops, malloc, sizeof(const struct hc_op *));
  hc_vector_init(&vm->code, malloc, sizeof(hc_op_eval_t));
}
```

Each operation contains a name, alignment and size, and function pointers for evaluation and cleanup.

```C
typedef uint8_t *(*hc_op_eval_t)(struct hc_vm *, uint8_t *);

struct hc_op {
  const char *name;

  size_t align;
  size_t size;

  hc_op_eval_t eval;
  void (*deinit)(uint8_t *);
};
```

The evaluation loop is by far the most performance critical part of a virtual machine, since it executes code for each and every operation. The following implementation has the added advantage of not statically limiting the set of available operations, which enables extending the machine from user code.

```C
void hc_vm_eval(struct hc_vm *vm,
		size_t start_pc,
		size_t end_pc) {
  uint8_t *ep = (end_pc == -1)
    ? vm->code.end
    : hc_vector_get(&vm->code, end_pc);

  for (uint8_t *p = hc_vector_get(&vm->code, start_pc);
       p != ep;
       p = (*(hc_op_eval_t *)p)(vm, p + vm->code.item_size));
}
```

This brings us back to the question of why we store evaluated code separately. The evaluation loop is very dependent on memory locality, which means we want to store the minimum amount of data that's absolutely needed to evaluate an operation quickly. This allows the CPU to cache larger chunks of the code in one go.

The `call` operation is used to call C functions.

```C
typedef void (*hc_vm_fun_t)(struct hc_vm *, struct hc_sloc);

struct hc_call_op {
  hc_vm_fun_t target;
  struct hc_sloc sloc;
};

static uint8_t *call_eval(struct hc_vm *vm, uint8_t *data) {
  struct hc_call_op *op = (void *)hc_align(data, alignof(struct hc_call_op));
  op->target(vm, op->sloc);
  return (uint8_t *)op + sizeof(struct hc_call_op);
}

const struct hc_op HC_CALL = (struct hc_op){
  .name = "call",
  .align = alignof(struct hc_call_op),
  .size = sizeof(struct hc_call_op),
  .eval = call_eval,
  .deinit = NULL
};
```

Calls include the source location where the call was made to enable better error reporting.

```C
struct hc_sloc {
  char source[32];
  char out[64];
  int row;
  int col;
};

struct hc_sloc hc_sloc(const char *source, const int row, const int col) {
  struct hc_sloc s = {.source = {0}, .row = row, .col = col};
  strncpy(s.source, source, sizeof(s.source)-1);
  return s;
}

const char *hc_sloc_string(struct hc_sloc *sloc) {
  sprintf(sloc->out, "'%s'; row %d, column %d",
	  sloc->source, sloc->row, sloc->col);
  return sloc->out;
}
```

The `push` operation pushes a [value](https://github.com/codr7/hacktical-c/tree/main/reflect) on the stack.

```C
struct hc_push_op {
  struct hc_value value;
};

static void push_deinit(uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_value_deinit(&op->value);
}

static uint8_t *push_eval(struct hc_vm *vm, uint8_t *data) {
  struct hc_push_op *op = (void *)hc_align(data, alignof(struct hc_push_op));
  hc_value_copy(hc_vm_push(vm), &op->value);
  return (uint8_t *)op + sizeof(struct hc_push_op);
}

const struct hc_op HC_PUSH = (struct hc_op){
  .name = "push",
  .align = alignof(struct hc_push_op),
  .size = sizeof(struct hc_push_op),
  .eval = push_eval,
  .deinit = push_deinit
};
```

To be continued...