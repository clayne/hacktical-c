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

The only missing pieces at this point are reading template code and emitting [VM](https://github.com/codr7/hacktical-c/tree/main/vm) operations.

First of all we need a way to skip whitespace.

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

Next up is a parser for calls; which first reads a call target and then the arguments.

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

static void call_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  hc_putc(out, '(');
  hc_form_print(f->target, out);

  hc_list_do(&f->args, i) {
    hc_putc(out, ' ');
    hc_form_print(hc_baseof(i, struct hc_form, owner), out);
  }
  
  hc_putc(out, ')');
}

static void call_free(struct hc_form *_f) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  hc_form_free(f->target);  

  hc_list_do(&f->args, i) {
    hc_form_free(hc_baseof(i, struct hc_form, owner));
  }

  free(f);
}

const struct hc_form_type hc_call = {
  .emit = call_emit,
  .print = call_print,
  .value = NULL,
  .free = call_free
};
```

To be continued...