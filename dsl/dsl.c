#include <assert.h>
#include <math.h>
#include <string.h>

#include "dsl.h"

static const uint8_t *push_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_dsl_push(dsl, op->value);
  return data + sizeof(hc_push_op);
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

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_vector_init(&dsl->code, sizeof(hc_eval));
  hc_vector_init(&dsl->registers, sizeof(hc_fix));
  hc_vector_init(&dsl->stack, sizeof(hc_fix));
}

void hc_dsl_deinit(struct hc_dsl *dsl) {
  hc_vector_deinit(&dsl->code);
  hc_vector_deinit(&dsl->registers);
  hc_vector_deinit(&dsl->stack);
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
  *(hc_eval *)p = op->eval;
  memcpy(p + dsl->code.item_size, data, op->size);
  return pc;
}

void hc_dsl_eval(struct hc_dsl *dsl, const hc_pc pc) {
  for (const uint8_t *p = hc_vector_get(&dsl->code, pc);
       p < dsl->code.end;
       p = (*(hc_eval *)p)(dsl, p + dsl->code.item_size));
}

struct hc_sloc hc_sloc(const char *source, int row, int col) {
  struct hc_sloc s = {.source = {0}, .row = row, .col = col};
  strncpy(s.source, source, sizeof(s.source)-1);
  return s;
}
