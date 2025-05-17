#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "dsl.h"
#include "error/error.h"
#include "list/list.h"

static void lib_print(struct hc_vm *vm, struct hc_sloc) {
  struct hc_value *v = hc_vm_pop(vm);
  hc_value_print(v, vm->out);
  hc_value_deinit(v);
}

void hc_dsl_init(struct hc_vm *vm) {
  hc_vm_init(vm, &hc_malloc_default);
  hc_vm_setenv(vm, "print", &HC_VM_FUN)->as_other = lib_print;
}

void hc_dsl_set_string(struct hc_vm *vm, const char *key, const char *val) {
  hc_vm_setenv(vm, key, &HC_STRING)->as_string = strdup(val);
}

void hc_form_init(struct hc_form *f,
		  const struct hc_form_type *t,
		  const struct hc_sloc sloc,
		  struct hc_list *owner) {
  f->type = t;
  f->sloc = sloc;

  if (owner) {
    hc_list_push_back(owner, &f->owner);
  } else {
    hc_list_init(&f->owner);
  }
}

void hc_form_emit(struct hc_form *f, struct hc_vm *vm) {
  assert(f->type->emit);
  f->type->emit(f, vm);
}

void hc_form_print(struct hc_form *f, struct hc_stream *out) {
  assert(f->type->print);
  f->type->print(f, out);
}

struct hc_value *hc_form_value(const struct hc_form *f, struct hc_vm *vm) {
  return f->type->value ? f->type->value(f, vm) : NULL;
}

void hc_form_free(struct hc_form *f) {
  hc_list_delete(&f->owner);
  assert(f->type->free);
  f->type->free(f);
}

static void call_emit(struct hc_form *_f, struct hc_vm *vm) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  struct hc_value *t = hc_form_value(f->target, vm);

  if (!t) {
    hc_throw("Error in '%s': Missing call target",
	     hc_sloc_string(&_f->sloc));
  }

  if (t->type != &HC_VM_FUN) {
    hc_throw("Error in '%s': '%s' isn't callable",
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

void hc_call_init(struct hc_call *f,
		  const struct hc_sloc sloc,
		  struct hc_list *owner,
		  struct hc_form *target) {  
  hc_form_init(&f->form, &hc_call, sloc, owner);
  f->target = target;
  hc_list_init(&f->args);
}

static void id_emit(struct hc_form *_f, struct hc_vm *vm) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  struct hc_value *v = hc_vm_getenv(vm, f->name);

  if (!v) {
    hc_throw("Error in %s: Unknown identifier '%s'",
	     hc_sloc_string(&_f->sloc), f->name);
  }

  struct hc_push_op op;
  hc_value_copy(&op.value, v);
  hc_vm_emit(vm, &HC_PUSH, &op);
}

static void id_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  hc_puts(out, f->name);
}

static struct hc_value *id_value(const struct hc_form *_f,
				 struct hc_vm *vm) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  return hc_vm_getenv(vm, f->name);
}

static void id_free(struct hc_form *_f) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  free(f->name);
  free(f);
}

const struct hc_form_type hc_id = {
  .emit = id_emit,
  .print = id_print,
  .value = id_value,
  .free = id_free
};

void hc_id_init(struct hc_id *f,
		const struct hc_sloc sloc,
		struct hc_list *owner,
		const char *name) {
  hc_form_init(&f->form, &hc_id, sloc, owner);
  f->name = strdup(name);
}

static void literal_emit(struct hc_form *_f, struct hc_vm *vm) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form); 
  struct hc_push_op op;
  hc_value_copy(&op.value, &f->value);
  hc_vm_emit(vm, &HC_PUSH, &op);
}

static void literal_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  hc_value_write(&f->value, out);
}

static struct hc_value *literal_value(const struct hc_form *_f,
				      struct hc_vm *vm) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  return &f->value;
}

static void literal_free(struct hc_form *_f) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  hc_value_deinit(&f->value);
  free(f);
}

const struct hc_form_type hc_literal = {
  .emit = literal_emit,
  .print = literal_print,
  .value = literal_value,
  .free = literal_free,
};

void hc_literal_init(struct hc_literal *f,
		     const struct hc_sloc sloc,
		     struct hc_list *owner) {  
  hc_form_init(&f->form, &hc_literal, sloc, owner);
}

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

void hc_read_call(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  if (**in != '(') {
    hc_throw("Error in '%s': Invalid call syntax",
	     hc_sloc_string(sloc));
  }

  struct hc_sloc floc = *sloc;
  (*in)++;
  sloc->col++;
  hc_skip_ws(in, sloc);
  
  if (!hc_read_expr(in, out, sloc)) {
    hc_throw("Error in '%s': Missing call target",
	     hc_sloc_string(sloc));
  }

  struct hc_call *f = malloc(sizeof(struct hc_call));
  struct hc_form *t = hc_baseof(hc_list_pop_back(out),
				struct hc_form,
				owner);
  
  hc_list_init(&t->owner);
  hc_call_init(f, floc, out, t);
  
  for (bool done = false; !done;) {
    hc_skip_ws(in, sloc);
    
    switch (**in) {
    case 0:
      hc_throw("Error in '%s': Open call form",
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
      hc_throw("Error in '%s': Invalid call syntax",
	       hc_sloc_string(sloc));
    }
  }
}

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

void hc_forms_free(struct hc_list *in) {
  hc_list_do(in, i) {
    hc_form_free(hc_baseof(i, struct hc_form, owner));
  } 
}

void hc_forms_emit(struct hc_list *in, struct hc_vm *vm) {
  hc_list_do(in, i) {
    hc_form_emit(hc_baseof(i, struct hc_form, owner), vm);
  } 
}

void hc_dsl_eval(struct hc_vm *vm, const char *in) {
  struct hc_list forms;
  hc_list_init(&forms);
  hc_defer(hc_forms_free(&forms));
  struct hc_sloc sloc = hc_sloc("evals", 0, 0);
  while (hc_read_next(&in, &forms, &sloc));
  const size_t pc = vm->code.length;
  hc_forms_emit(&forms, vm);
  hc_vm_eval(vm, pc, -1);
}
