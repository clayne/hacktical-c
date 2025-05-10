#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dsl1.h"
#include "error/error.h"

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
  hc_set_init(&dsl->env, malloc, sizeof(struct env_item), env_cmp);
  dsl->env.key = env_key;

  hc_vector_init(&dsl->stack, malloc, sizeof(struct hc_value));
  hc_vector_init(&dsl->ops, malloc, sizeof(const struct hc_op *));
  hc_vector_init(&dsl->code, malloc, sizeof(hc_op_eval_t));
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

  hc_vector_deinit(&dsl->ops);
}

void hc_dsl_deinit(struct hc_dsl *dsl) {  
  hc_set_deinit(&dsl->env);
  hc_vector_deinit(&dsl->stack);
  deinit_ops(dsl);
  hc_vector_deinit(&dsl->code);
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
