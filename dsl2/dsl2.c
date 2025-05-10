#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "dsl2.h"
#include "error/error.h"
#include "malloc1/malloc1.h"

void hc_form_init(struct hc_form *f,
		  const struct hc_form_type *t,
		  const struct hc_sloc sloc,
		  struct hc_list *owner) {
  f->type = t;
  f->sloc = sloc;
  hc_list_push_back(owner, &f->owner);
}

void hc_form_emit(struct hc_form *f, struct hc_dsl *dsl) {
  assert(f->type->emit);
  f->type->emit(f, dsl);
}

void hc_form_print(struct hc_form *f, struct hc_stream *out) {
  assert(f->type->print);
  f->type->print(f, out);
}

struct hc_value *hc_form_value(const struct hc_form *f, struct hc_dsl *dsl) {
  return f->type->value ? f->type->value(f, dsl) : NULL;
}

void hc_form_free(struct hc_form *f) {
  hc_list_delete(&f->owner);
  assert(f->type->free);
  f->type->free(f);
}

static void call_emit(const struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  struct hc_value *t = hc_form_value(f->target, dsl);

  if (!t) {
    hc_throw("Missing call target");
  }

  if (t->type != &HC_DSL_FUN) {
    hc_throw("Not callable: %s", t->type->name);
  }
  
  hc_dsl_emit(dsl,
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

  hc_list_do(f->args, i) {
    hc_putc(out, ' ');
    hc_form_print(hc_baseof(i, struct hc_form, owner), out);
  }
  
  hc_putc(out, ')');
}

static void call_free(struct hc_form *_f) {
  struct hc_call *f = hc_baseof(_f, struct hc_call, form);
  hc_form_free(f->target);  

  hc_list_do(f->args, i) {
    hc_form_free(hc_baseof(i, struct hc_form, owner));
  }

  free(f->args);
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
		  struct hc_form *target,
		  struct hc_list *args) {  
  hc_form_init(&f->form, &hc_call, sloc, owner);
  f->target = target;
  f->args = args;
}

static void id_emit(const struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  struct hc_value *v = hc_dsl_getenv(dsl, f->name, _f->sloc);
  struct hc_push_op op;
  hc_value_copy(&op.value, v);
  hc_dsl_emit(dsl, &HC_PUSH, &op);
}

static void id_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  hc_puts(out, f->name);
}

static struct hc_value *id_value(const struct hc_form *_f,
				 struct hc_dsl *dsl) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  return hc_dsl_getenv(dsl, f->name, _f->sloc);
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

static void literal_emit(const struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form); 
  struct hc_push_op op;
  hc_value_copy(&op.value, &f->value);
  hc_dsl_emit(dsl, &HC_PUSH, &op);
}

static void literal_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  hc_value_write(&f->value, out);
}

static struct hc_value *literal_value(const struct hc_form *_f,
				      struct hc_dsl *dsl) {
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
		     struct hc_list *owner,
		     struct hc_value *value) {  
  hc_form_init(&f->form, &hc_literal, sloc, owner);
  hc_value_copy(&f->value, value);
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
  struct hc_sloc floc = *sloc;
  assert(**in == '(');
  (*in)++;

  if (!hc_read_form(in, out, sloc)) {
    hc_throw("Missing call target");
  }

  struct hc_form *t = hc_baseof(hc_list_pop_back(out),
				struct hc_form,
				owner);
  
  hc_list_init(&t->owner);
  struct hc_list *args = malloc(sizeof(struct hc_list));
  hc_list_init(args);
  
  for (bool done = false; !done;) {
    hc_skip_ws(in, sloc);
    
    switch (**in) {
    case 0:
      hc_throw("Open call form");
    case ')':
      (*in)++;
      done = true;
      continue;
    default:
      break;
    }

    if (!hc_read_form(in, args, sloc)) {
      hc_throw("Invalid call syntax");
    }
  }

  struct hc_call *f = malloc(sizeof(struct hc_call));
  hc_call_init(f, floc, out, t, args);
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
    if (isspace(c)) {
      break;
    }
  
    hc_putc(&buf.stream, c);
    sloc->col++;
    (*in)++;
  }

  struct hc_id *f = malloc(sizeof(struct hc_id));
  hc_id_init(f, floc, out, hc_memory_stream_string(&buf));
}

bool hc_read_form(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  hc_skip_ws(in, sloc);
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

static void fun_print(const struct hc_value *v, struct hc_stream *out) {
  hc_printf(out, "%p", v->as_other);
}

const struct hc_type HC_DSL_FUN = {
  .name = "DSL/Fun",
  .copy = NULL,
  .print = fun_print
};

