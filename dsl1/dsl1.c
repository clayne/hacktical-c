#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dsl1.h"
#include "error/error.h"
#include "malloc1/malloc1.h"

enum hc_order hc_strcmp(const char *x, const char *y) {
  const int result = strcmp(x, y);
  if (!result) { return HC_EQ; }
  return (result < 0) ? HC_LT : HC_GT;
}

struct env_item {
  const char *key;
  struct hc_value value;
};

static enum hc_order env_cmp(const void *x, const void *y) {
  return hc_strcmp(*(const char **)x, *(const char **)y);
}

static const void *env_key(const void *x) {
  return &((const struct env_item *)x)->key;
}

void hc_dsl_init(struct hc_dsl *dsl, struct hc_malloc *malloc) {
  hc_vector_init(&dsl->code, malloc, sizeof(hc_op_eval_t));

  hc_set_init(&dsl->env, malloc, sizeof(struct env_item), env_cmp);
  dsl->env.key = env_key;
  
  hc_vector_init(&dsl->ops, malloc, sizeof(const struct hc_op *));
  hc_vector_init(&dsl->stack, malloc, sizeof(struct hc_value));
}

static size_t op_size(const struct hc_op *op, struct hc_dsl *dsl) {
  return ceil(op->size / dsl->code.item_size);
}

static void deinit_ops(struct hc_dsl *dsl) {
  uint8_t *p = dsl->code.start;
  
  hc_vector_do(&dsl->ops, _op) {
    const struct hc_op *op = *(const struct hc_op **)_op;
    p += sizeof(hc_op_eval_t);

    if (op->deinit) {
      op->deinit(p);
    }

    p += op_size(op, dsl);
  }
}

void hc_dsl_deinit(struct hc_dsl *dsl) {  
  deinit_ops(dsl);
  hc_vector_deinit(&dsl->code);
  hc_set_deinit(&dsl->env);
  hc_vector_deinit(&dsl->ops);
  hc_vector_deinit(&dsl->stack);
}

struct hc_value *hc_dsl_getenv(struct hc_dsl *dsl,
			       const char *key,
			       const struct hc_sloc sloc) {
  struct env_item *found = hc_set_find(&dsl->env, &key);
  
  if (!found) {
    hc_throw("Unknown identifier: %s", key);
  }
  
  return &found->value;
}

void hc_dsl_setenv(struct hc_dsl *dsl,
		   const char *key,
		   struct hc_value value) {
  struct env_item *it = hc_set_find(&dsl->env, &key);

  if (!it) {
    it = hc_set_add(&dsl->env, &key, false);
  }
  
  it->value = value;
}

struct hc_value *hc_dsl_push(struct hc_dsl *dsl) {
  return hc_vector_push(&dsl->stack);
}

struct hc_value *hc_dsl_peek(struct hc_dsl *dsl) {
  return hc_vector_peek(&dsl->stack);
}

struct hc_value *hc_dsl_pop(struct hc_dsl *dsl) {
  return hc_vector_pop(&dsl->stack);
}

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data) {
  *(const struct hc_op **)hc_vector_push(&dsl->ops) = op;
  const hc_pc pc = dsl->code.length;
  
  uint8_t *p = hc_vector_insert(&dsl->code,
				dsl->code.length,
				op_size(op, dsl) + 1);
  
  *(hc_op_eval_t *)p = op->eval;
  memcpy(p + dsl->code.item_size, data, op->size);
  return pc;
}

void hc_dsl_eval(struct hc_dsl *dsl,
		 const hc_pc start_pc,
		 const hc_pc end_pc) {
  const uint8_t *const ep = (end_pc == -1)
    ? dsl->code.end
    : hc_vector_get(&dsl->code, end_pc);

  for (const uint8_t *p = hc_vector_get(&dsl->code, start_pc);
       p != ep;
       p = (*(hc_op_eval_t *)p)(dsl, p + dsl->code.item_size));
}

struct hc_sloc hc_sloc(const char *source, const int row, const int col) {
  struct hc_sloc s = {.source = {0}, .row = row, .col = col};
  strncpy(s.source, source, sizeof(s.source)-1);
  return s;
}

void hc_form_init(struct hc_form *f,
		  const struct hc_form_type *t,
		  const struct hc_sloc sloc,
		  struct hc_list *list) {
  f->type = t;
  f->sloc = sloc;
  hc_list_push_back(list, &f->list);
}

void hc_form_free(struct hc_form *f) {
  assert(f->type->free);
  f->type->free(f);
}
					
static void id_emit(const struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  struct hc_value *v = hc_dsl_getenv(dsl, f->name, _f->sloc);

  if (v->type == &HC_DSL_FUN) {
    hc_dsl_emit(dsl,
		&HC_CALL,
		&(struct hc_call_op){
		  .target = v->as_other,
		  .sloc = _f->sloc
		});
  } else {
    struct hc_push_op op;
    hc_value_copy(&op.value, v);
    hc_dsl_emit(dsl, &HC_PUSH, &op);
  }
}

static void id_free(struct hc_form *_f) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  free(f->name);
  free(f);
}

static void id_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_id *f = hc_baseof(_f, struct hc_id, form);
  _hc_stream_puts(out, f->name);
}

struct hc_form_type *hc_id_form() {
  static __thread struct hc_form_type t = {
    .emit = id_emit,
    .free = id_free,
    .print = id_print
  };

  return &t;
}

void hc_id_init(struct hc_id *f,
		const struct hc_sloc sloc,
		struct hc_list *list,
		const char *name) {
  hc_form_init(&f->form, hc_id_form(), sloc, list);
  f->name = strdup(name);
}

static void literal_emit(const struct hc_form *_f, struct hc_dsl *dsl) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form); 
  struct hc_push_op op;
  hc_value_copy(&op.value, &f->value);
  hc_dsl_emit(dsl, &HC_PUSH, &op);
}

static void literal_free(struct hc_form *_f) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  hc_value_deinit(&f->value);
  free(f);
}

static void literal_print(const struct hc_form *_f, struct hc_stream *out) {
  struct hc_literal *f = hc_baseof(_f, struct hc_literal, form);
  hc_value_write(&f->value, out);
}

void hc_literal_init(struct hc_literal *f,
		     const struct hc_sloc sloc,
		     struct hc_list *list,
		     struct hc_value *value) {
  static const struct hc_form_type type = {
    .emit = literal_emit,
    .free = literal_free,
    .print = literal_print
  };
  
  hc_form_init(&f->form, &type, sloc, list);
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

bool hc_read_id(const char **in,
		struct hc_list *out,
		struct hc_sloc *sloc) {
  struct hc_sloc floc = *sloc;
  struct hc_memory_stream buf;
  hc_memory_stream_init(&buf, &hc_malloc_default);
  char c = 0;

  while ((c = **in)) {
    if (isspace(c)) {
      break;
    }
  
    hc_stream_putc(&buf, c);
    sloc->col++;
    (*in)++;
  }

  struct hc_id *f = malloc(sizeof(struct hc_id));
  hc_id_init(f, floc, out, hc_memory_stream_string(&buf));
  hc_stream_deinit(&buf);
  return true;
}

bool hc_read_form(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc) {
  hc_skip_ws(in, sloc);
  const char c = **in;
  
  switch (c) {
  default:
    if (isalpha(c)) {
      return hc_read_id(in, out, sloc);
    }

    break;
  }

  return false;
}

static const uint8_t *call_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_call_op *op = (void *)data;
  op->target(dsl, op->sloc);
  return data + HC_CALL.size;
}

const struct hc_op HC_CALL = (struct hc_op){
  .name = "call",
  .size = sizeof(struct hc_call_op),
  .eval = call_eval
};

static void push_deinit(uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_value_deinit(&op->value);
}

static const uint8_t *push_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_value_copy(hc_dsl_push(dsl), &op->value);
  return data + HC_PUSH.size;
}

const struct hc_op HC_PUSH = (struct hc_op){
  .name = "push",
  .size = sizeof(struct hc_push_op),
  .deinit = push_deinit,
  .eval = push_eval
};

static const uint8_t *stop_eval(struct hc_dsl *dsl, const uint8_t *data) {
  return dsl->code.end;
}

const struct hc_op HC_STOP = (struct hc_op){
  .name = "stop",
  .size = 0,
  .eval = stop_eval
};

static void fun_print(const struct hc_value *v, struct hc_stream *out) {
  _hc_stream_printf(out, "%p", v->as_other);
}

const struct hc_type HC_DSL_FUN = {
  .name = "DSL/Fun",
  .copy = NULL,
  .print = fun_print
};

