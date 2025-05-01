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

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_vector_init(&dsl->code, sizeof(hc_op_eval));

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

struct hc_value hc_dsl_getenv(struct hc_dsl *dsl,
			      const char *key,
			      const struct hc_sloc sloc) {
  struct env_item *found = hc_set_find(&dsl->env, &key);

  if (!found) {
    hc_throw("Unknown identifier: %s", key);
  }

  return found->value;
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
		  const struct hc_op *op,
		  const void *data) {
  const hc_pc pc = dsl->code.length;
  uint8_t *p = hc_vector_insert(&dsl->code,
				dsl->code.length,
				ceil(op->size / dsl->code.item_size) + 1);
  *(hc_op_eval *)p = op->eval;
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
       p = (*(hc_op_eval *)p)(dsl, p + dsl->code.item_size));
}

struct hc_sloc hc_sloc(const char *source, int row, int col) {
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

void hc_form_deinit(struct hc_form *f) {
  if (f->type->deinit) {
    f->type->deinit(f);
  }
}
					
static void id_deinit(struct hc_form *f) {
  free(hc_baseof(f, struct hc_id, form)->name);
}

static void id_emit(const struct hc_form *f, struct hc_dsl *dsl) {
  struct hc_id *id = hc_baseof(f, struct hc_id, form);
  const struct hc_value v = hc_dsl_getenv(dsl, id->name, f->sloc);

  switch (v.type) {
  case HC_DSL_FUN:
    hc_dsl_emit(dsl,
		&hc_call_op,
		&(struct hc_call_op){
		  .target = v.as_fun,
		  .sloc = f->sloc
		});
    break;
  case HC_DSL_FIX:
    hc_dsl_emit(dsl,
		&hc_push_op,
		&(struct hc_push_op){
		  .value = v.as_fix
		});
    break;
  default:
    hc_throw("Invald value: %d", v.type);
  }
}

static void id_print(const struct hc_form *f, struct hc_stream *out) {
  struct hc_id *id = hc_baseof(f, struct hc_id, form);
  _hc_stream_puts(out, id->name);
}

struct hc_form_type *hc_id_form() {
  static __thread struct hc_form_type t = {
    .deinit = id_deinit,
    .emit = id_emit,
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
		     const struct hc_sloc sloc,
		     struct hc_list *list,
		     const hc_fix value) {
  static const struct hc_form_type type = {
    .deinit = NULL,
    .emit = literal_emit,
    .print = literal_print
  };
  
  hc_form_init(&f->form, &type, sloc, list);
  f->value = value;
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
  hc_memory_stream_init(&buf);
  char c = 0;

  while ((c = **in)) {
    if (isspace(c)) {
      break;
    }
  
    hc_stream_putc(&buf, c);
    sloc->col++;
    (*in)++;
  }

  struct hc_id *f = hc_acquire(sizeof(struct hc_id));
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
  return data + hc_call_op.size;
}

const struct hc_op hc_call_op = (struct hc_op){
  .name = "call",
  .size = sizeof(struct hc_call_op),
  .eval = call_eval
};

static const uint8_t *push_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_dsl_push(dsl, op->value);
  return data + hc_push_op.size;
}

const struct hc_op hc_push_op = (struct hc_op){
  .name = "push",
  .size = sizeof(struct hc_push_op),
  .eval = push_eval
};

static const uint8_t *stop_eval(struct hc_dsl *dsl, const uint8_t *data) {
  return dsl->code.end;
}

const struct hc_op hc_stop_op = (struct hc_op){
  .name = "stop",
  .size = 0,
  .eval = stop_eval
};
