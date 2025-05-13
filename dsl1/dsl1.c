#include <math.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

#include "dsl1.h"
#include "error/error.h"
#include "macro/macro.h"

enum hc_order hc_strcmp(const char *x, const char *y) {
  const int result = strcmp(x, y);
  if (!result) { return HC_EQ; }
  return (result < 0) ? HC_LT : HC_GT;
}

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

struct env_item {
  char *key;
  struct hc_value value;
};

static enum hc_order env_cmp(const void *x, const void *y) {
  return hc_strcmp(*(const char **)x, *(const char **)y);
}

static const void *env_key(const void *x) {
  return &((const struct env_item *)x)->key;
}

static void lib_print(struct hc_dsl *dsl, struct hc_sloc) {
  printf("PRINT\n");
}

void hc_dsl_init(struct hc_dsl *dsl, struct hc_malloc *malloc) {
  hc_set_init(&dsl->env, malloc, sizeof(struct env_item), env_cmp);
  dsl->env.key = env_key;

  hc_vector_init(&dsl->stack, malloc, sizeof(struct hc_value));
  hc_vector_init(&dsl->ops, malloc, sizeof(const struct hc_op *));
  hc_vector_init(&dsl->code, malloc, sizeof(hc_op_eval_t));

  hc_dsl_setenv(dsl, "print", &HC_DSL_FUN)->as_other = lib_print;
}

static size_t op_items(const struct hc_op *op,
		      uint8_t *p,
		      struct hc_dsl *dsl) {
  const size_t s = op->size + hc_align(p, op->align) - p;
  return ceil(s / (double)dsl->code.item_size);
}

static void deinit_env(struct hc_dsl *dsl) {
  hc_vector_do(&dsl->env.items, _it) {
    struct env_item *it = _it;
    free(it->key);
    hc_value_deinit(&it->value);
  }
  
  hc_set_deinit(&dsl->env);
}

static void deinit_stack(struct hc_dsl *dsl) {
  hc_vector_do(&dsl->stack, v) {
    hc_value_deinit(v);
  }

  hc_vector_deinit(&dsl->stack);
}

static void deinit_ops(struct hc_dsl *dsl) {
  uint8_t *p = dsl->code.start;
  
  hc_vector_do(&dsl->ops, _op) {
    const struct hc_op *op = *(const struct hc_op **)_op;
    p += sizeof(hc_op_eval_t);

    if (op->deinit) {
      op->deinit(hc_align(p, op->align));
    }

    p += op_items(op, p, dsl) * dsl->code.item_size;
  }

  hc_vector_deinit(&dsl->ops);
}

void hc_dsl_deinit(struct hc_dsl *dsl) {  
  deinit_env(dsl);
  deinit_stack(dsl);
  deinit_ops(dsl);
  hc_vector_deinit(&dsl->code);
}

struct hc_value *hc_dsl_getenv(struct hc_dsl *dsl, const char *key) {
  struct env_item *it = hc_set_find(&dsl->env, &key);  
  return it ? &it->value : NULL;
}

struct hc_value *hc_dsl_setenv(struct hc_dsl *dsl,
			       const char *key,
			       const struct hc_type *type) {
  struct env_item *it = hc_set_add(&dsl->env, &key, false);
  it->key = strdup(key);
  hc_value_init(&it->value, type);
  return &it->value;
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
  *(hc_op_eval_t *)hc_vector_push(&dsl->code) = op->eval;
  
  uint8_t *const p = hc_vector_insert(&dsl->code,
				      dsl->code.length,
				      op_items(op, dsl->code.end, dsl));
  
  memcpy(hc_align(p, op->align), data, op->size);
  return pc;
}

void hc_dsl_eval(struct hc_dsl *dsl,
		 const hc_pc start_pc,
		 const hc_pc end_pc) {
  const uint8_t *const ep = (end_pc == -1)
    ? dsl->code.end
    : hc_vector_get(&dsl->code, end_pc);

  for (uint8_t *p = hc_vector_get(&dsl->code, start_pc);
       p != ep;
       p = (*(hc_op_eval_t *)p)(dsl, p + dsl->code.item_size));
}

static void fun_print(const struct hc_value *v, struct hc_stream *out) {
  hc_printf(out, "%p", v->as_other);
}

const struct hc_type HC_DSL_FUN = {
  .name = "DSL/Fun",
  .copy = NULL,
  .print = fun_print
};

static uint8_t *call_eval(struct hc_dsl *dsl, uint8_t *data) {
  struct hc_call_op *op = (void *)hc_align(data, alignof(struct hc_call_op));
  op->target(dsl, op->sloc);
  return (uint8_t *)op + sizeof(struct hc_call_op);
}

const struct hc_op HC_CALL = (struct hc_op){
  .name = "call",
  .align = alignof(struct hc_call_op),
  .size = sizeof(struct hc_call_op),
  .deinit = NULL,
  .eval = call_eval
};

static void push_deinit(uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_value_deinit(&op->value);
}

static uint8_t *push_eval(struct hc_dsl *dsl, uint8_t *data) {
  struct hc_push_op *op = (void *)hc_align(data, alignof(struct hc_push_op));
  hc_value_copy(hc_dsl_push(dsl), &op->value);
  return (uint8_t *)op + sizeof(struct hc_push_op);
}

const struct hc_op HC_PUSH = (struct hc_op){
  .name = "push",
  .align = alignof(struct hc_push_op),
  .size = sizeof(struct hc_push_op),
  .deinit = push_deinit,
  .eval = push_eval
};
