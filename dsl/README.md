## Domain Specific Languages
A domain specific language (DSL) is a tiny embedded language specialized at solving a specific category of problems. Once you start looking for them, they're everywhere; `printf` formats and regular expressions are two obvious examples.

We're going to build a template engine that allows splicing runtime evaluated expressions into strings. Templates are compiled into operations for the [virtual machine](https://github.com/codr7/hacktical-c/tree/main/vm) we built in previous chapter.

Example:
```C
struct hc_dsl dsl;
hc_dsl_init(&dsl, &hc_malloc_default);
hc_defer(hc_dsl_deinit(&dsl));
struct hc_memory_stream out;
hc_memory_stream_init(&out, hc_malloc());
hc_defer(hc_stream_deinit(&out->stream));
dsl.out = &out.stream;
hc_dsl_set_string(&dsl, "foo", "ghi");
hc_dsl_eval(&dsl, "abc $(print (upcase foo)) def");
assert(strcmp("abc GHI def", hc_memory_stream_string(&out)) == 0);
```

Our DSL consists of an environment, a standard output and a [vm](https://github.com/codr7/hacktical-c/tree/main/vm).

```C
struct hc_dsl {
  struct hc_set env;
  struct hc_stream *out;

  struct hc_vm vm;
};

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_set_init(&dsl->env, malloc, sizeof(struct env_item), env_cmp);
  dsl->env.key = env_key;
  dsl->out = hc_stdout();
  
  hc_vm_init(vm, &hc_malloc_default);
  hc_dsl_set_fun(vm, "print", lib_print);
  hc_dsl_set_fun(vm, "upcase", lib_upcase);
}

enum hc_order env_cmp(const void *x, const void *y) {
  return hc_strcmp(*(const char **)x, *(const char **)y);
}

const void *env_key(const void *x) {
  return &((const struct env_item *)x)->key;
}
```

`print` pops a value from the stack and prints it to `vm.stdout`.

```C
void lib_print(struct hc_vm *vm, const struct hc_sloc sloc) {
  struct hc_value *v = hc_vm_pop(vm);
  struct hc_dsl *dsl = hc_baseof(vm, struct hc_dsl, vm);
  hc_value_print(v, dsl->out);
  hc_value_deinit(v);
}
```

While `upcase` transforms the top value on the stack to uppercase.

```C
void lib_upcase(struct hc_vm *vm, const struct hc_sloc sloc) {
  struct hc_value *v = hc_vm_peek(vm);

  if (v->type != &HC_STRING) {
    hc_throw("Error in %s: Expected string (%s)",
	     hc_sloc_string(&sloc), v->type->name);
  }

  hc_upcase(v->as_string);
}

char *hc_upcase(char *s) {
  while (*s) {
    *s = toupper(*s);
    s++;
  }

  return s;
}
```

The only missing piece of the puzzle at this point is transforming template code into [VM](https://github.com/codr7/hacktical-c/tree/main/vm) operations, aka. syntax.

```C
void hc_dsl_eval(struct hc_dsl *dsl, const char *in) {
  struct hc_list forms;
  hc_list_init(&forms);
  hc_defer(hc_forms_free(&forms));
  struct hc_sloc sloc = hc_sloc("eval", 0, 0);
  while (hc_read_next(&in, &forms, &sloc));
  const size_t pc = dsl->vm.code.length;
  hc_forms_emit(&forms, dsl);
  hc_vm_eval(&dsl->vm, pc, -1);
}
```

The top layer of our parser simply checks for `$` and uses that decide what do next.

```C
bool hc_read_next(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  if (**in == '$') {
    (*in)++;
    hc_read_call(in, out, sloc);
    return true;
  }
  
  return hc_read_text(in, out, sloc);
}
```

A call consists of a target and optional arguments.

```C
void hc_read_call(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  struct hc_sloc floc = *sloc;

  if (**in != '(') {
    hc_throw("Error in %s: Invalid call syntax",
	     hc_sloc_string(sloc));
  }

  (*in)++;
  sloc->col++;
  hc_skip_ws(in, sloc);
  
  if (!hc_read_expr(in, out, sloc)) {
    hc_throw("Error in %s: Missing call target",
	     hc_sloc_string(sloc));
  }

  struct hc_form *t = hc_baseof(hc_list_pop_back(out),
  				struct hc_form,
				owner);
  
  hc_list_init(&t->owner);
  struct hc_call *f = malloc(sizeof(struct hc_call));
  hc_call_init(f, floc, out, t);
  
  for (bool done = false; !done;) {
    hc_skip_ws(in, sloc);
    
    switch (**in) {
    case 0:
      hc_form_free(f);

      hc_throw("Error in %s: Open call form",
	       hc_sloc_string(sloc));
    case ')':
      (*in)++;
      sloc->col++;
      done = true;
      continue;
    default:
      break;
    }

    if (!hc_read_expr(in, &f->args, sloc)) {
      hc_form_free(f);
      
      hc_throw("Error in %s: Invalid call syntax",
	       hc_sloc_string(sloc));
    }
  }
}
```

When emitted, calls get the value of the target and emits arguments if any followed by a `HC_CALL``-operation.

```C
static void call_emit(struct hc_form *_f, struct hc_vm *vm) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  struct hc_value *t = hc_form_value(f->target, vm);

  if (!t) {
    hc_throw("Error in %s: Missing call target",
	     hc_sloc_string(&_f->sloc));
  }

  if (t->type != &HC_VM_FUN) {
    hc_throw("Error in %s: '%s' isn't callable",
	     hc_sloc_string(&_f->sloc),
	     t->type->name);
  }

  hc_list_do(&f->args, a) {
    hc_form_emit(hc_baseof(a, struct hc_form, owner), vm);
  }
  
  hc_vm_emit(vm,
	      &HC_CALL,
	      &(struct hc_call_op){
		.target = t->as_other,
		.sloc = _f->sloc
	      });
}
```

`hc_skip_ws()` simply skips forward as long as the current char is some kind of whitespace.

```C
void hc_skip_ws(const char **in, struct hc_sloc *sloc) {
  for (;; (*in)++) {
    switch (**in) {
    case ' ':
    case '\t':
      sloc->col++;
      break;
    case '\n':
      sloc->row++;
      sloc->col = 0;
      break;
    default:
      return;
    }
  }
}
```

`hc_read_expr()` handles anything allowed inside `$()`, which means another call or an identifier.

```C
bool hc_read_expr(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  const char c = **in;
  
  switch (c) {
  case '(':
    hc_read_call(in, out, sloc);
    return true;
  default:
    if (isalpha(c)) {
      hc_read_id(in, out, sloc);
      return true;
    }

    break;
  }

  return false;
}
```

Identifiers are required to start with an alphabetic char; following that, anything except whitespace and parens is allowed.

```C
void hc_read_id(const char **in,
		struct hc_list *out,
		struct hc_sloc *sloc) {
  struct hc_sloc floc = *sloc;
  struct hc_memory_stream buf;
  hc_memory_stream_init(&buf, &hc_malloc_default);
  hc_defer(hc_stream_deinit(&buf.stream));
  char c = 0;

  while ((c = **in)) {
    if (isspace(c) || c == '(' || c == ')') {
      break;
    }
  
    hc_putc(&buf.stream, c);
    sloc->col++;
    (*in)++;
  }

  struct hc_id *f = malloc(sizeof(struct hc_id));
  hc_id_init(f, floc, out, hc_memory_stream_string(&buf));
}
```

Identifiers get their values from `dsl.env` and emit an operation to push it on the stack.

```C
void id_emit(struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  struct hc_value *v = hc_dsl_getenv(dsl, f->name);

  if (!v) {
    hc_throw("Error in %s: Unknown identifier '%s'",
	     hc_sloc_string(&_f->sloc), f->name);
  }

  struct hc_push_op op;
  hc_value_copy(&op.value, v);
  hc_vm_emit(&dsl->vm, &HC_PUSH, &op);
}
```

The text parser keeps going until a `$` is found or until it reaches the end of the string, it then constructs a `print` call with the text as argument.

```C
bool hc_read_text(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  struct hc_sloc floc = *sloc;
  const char *start = *in;
  
  while (**in && **in != '$') {
    if (**in == '\n') {
      sloc->row++;
    } else {
      sloc->col++;
    }

    (*in)++;
  }

  size_t n = *in - start;
  
  if (n) {
    struct hc_value v;
    hc_value_init(&v, &HC_STRING)->as_string = strndup(start, n);    
    struct hc_literal *vf = malloc(sizeof(struct hc_literal));
    hc_literal_init(vf, floc, out);
    vf->value = v;
    struct hc_id *t = malloc(sizeof(struct hc_literal));
    hc_id_init(t, floc, NULL, "print");
    struct hc_call *c = malloc(sizeof(struct hc_call));
    hc_call_init(c, floc, out, &t->form);
    return true;
  }

  return false;
}
```