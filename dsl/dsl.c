#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dsl.h"
#include "error/error.h"

enum hc_order hc_strcmp(const char *x, const char *y) {
  const int result = strcmp(x, y);
  if (!result) { return HC_EQ; }
  return (result < 0) ? HC_LT : HC_GT;
}

struct env_item {
  const char *key;
  struct hc_dsl_value value;
};

static enum hc_order env_cmp(const void *x, const void *y) {
  return hc_strcmp(*(const char **)x, *(const char **)y);
}

static const void *env_key(const void *x) {
  return &((const struct env_item *)x)->key;
}

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_vector_init(&dsl->code, sizeof(hc_eval));

  hc_set_init(&dsl->env, sizeof(struct env_item), env_cmp);
  dsl->env.key = env_key;
  
  hc_vector_init(&dsl->registers, sizeof(hc_fix));
  hc_vector_init(&dsl->stack, sizeof(hc_fix));
}

void hc_dsl_deinit(struct hc_dsl *dsl) {
  hc_vector_deinit(&dsl->code);
  hc_set_deinit(&dsl->env);
  hc_vector_deinit(&dsl->registers);
  hc_vector_deinit(&dsl->stack);
}

struct hc_dsl_value hc_dsl_getenv(struct hc_dsl *dsl,
				  const char *key,
				  const struct hc_dsl_sloc sloc) {
  struct env_item *found = hc_set_find(&dsl->env, &key);

  if (found == NULL) {
    hc_throw(0, "Unknown identifier: %s", key);
  }

  return found->value;
}

void hc_dsl_setenv(struct hc_dsl *dsl,
		   const char *key,
		   struct hc_dsl_value value) {
  struct env_item *it = hc_set_find(&dsl->env, &key);

  if (it == NULL) {
    it = hc_set_add(&dsl->env, &key, false);
  }
  
  it->value = value;
}

void hc_dsl_push(struct hc_dsl *dsl, const hc_fix v) {
  *(hc_fix *)hc_vector_push(&dsl->stack) = v;
}

hc_fix hc_dsl_peek(struct hc_dsl *dsl) {
  return *(hc_fix *)hc_vector_peek(&dsl->stack);
}

hc_fix hc_dsl_pop(struct hc_dsl *dsl) {
  return *(hc_fix *)hc_vector_pop(&dsl->stack);
}

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_dsl_op *op,
		  const void *data) {
  const hc_pc pc = dsl->code.length;
  uint8_t *p = hc_vector_insert(&dsl->code,
				dsl->code.length,
				ceil(op->size / dsl->code.item_size) + 1);
  *(hc_eval *)p = op->eval;
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
       p = (*(hc_eval *)p)(dsl, p + dsl->code.item_size));
}

struct hc_dsl_sloc hc_dsl_sloc(const char *source, int row, int col) {
  struct hc_dsl_sloc s = {.source = {0}, .row = row, .col = col};
  strncpy(s.source, source, sizeof(s.source)-1);
  return s;
}

static void form_init(struct hc_form *f,
		      const struct hc_form_type *t,
		      const struct hc_dsl_sloc sloc) {
  f->type = t;
  f->sloc = sloc;
}

static void id_deinit(struct hc_form *f, struct hc_dsl *dsl) {
  free(hc_baseof(f, struct hc_id, form)->name);
}

static void id_emit(const struct hc_form *f, struct hc_dsl *dsl) {
  struct hc_id *id = hc_baseof(f, struct hc_id, form);
  const struct hc_dsl_value v = hc_dsl_getenv(dsl, id->name, f->sloc);

  switch (v.type) {
  case HC_DSL_FUN:
    //TODO emit call
    break;
  case HC_DSL_FIX:
    hc_dsl_emit(dsl,
		&hc_push_op,
		&(struct hc_push_op){
		  .value = v.as_fix
		});
    break;
  default:
    hc_throw(0, "Invald value: %d", v.type);
  }
}

static void id_print(const struct hc_form *f, struct hc_stream *out) {
  struct hc_id *id = hc_baseof(f, struct hc_id, form);
  _hc_stream_puts(out, id->name);
}

void hc_id_init(struct hc_id *f,
		const struct hc_dsl_sloc sloc,
		const char *name) {
  static const struct hc_form_type type = {
    .deinit = id_deinit,
    .emit = id_emit,
    .print = id_print
  };
  
  form_init(&f->form, &type, sloc);
  f->name = strdup(name);
}

static void literal_emit(const struct hc_form *f, struct hc_dsl *dsl) {
  struct hc_literal *lt = hc_baseof(f, struct hc_literal, form); 

  hc_dsl_emit(dsl,
	      &hc_push_op,
	      &(struct hc_push_op){
		.value = lt->value
	      });
}

static void literal_print(const struct hc_form *f, struct hc_stream *out) {
  struct hc_literal *lt = hc_baseof(f, struct hc_literal, form);
  hc_fix_print(lt->value, out);
}

void hc_literal_init(struct hc_literal *f,
		     const struct hc_dsl_sloc sloc,
		     const hc_fix value) {
  static const struct hc_form_type type = {
    .deinit = NULL,
    .emit = literal_emit,
    .print = literal_print
  };
  
  form_init(&f->form, &type, sloc);
  f->value = value;
}

static const uint8_t *push_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_dsl_push(dsl, op->value);
  return data + hc_push_op.size;
}

const struct hc_dsl_op hc_push_op = (struct hc_dsl_op){
  .name = "push",
  .size = sizeof(struct hc_push_op),
  .eval = push_eval
};

static const uint8_t *stop_eval(struct hc_dsl *dsl, const uint8_t *data) {
  return dsl->code.end;
}

const struct hc_dsl_op hc_stop_op = (struct hc_dsl_op){
  .name = "stop",
  .size = 0,
  .eval = stop_eval
};
