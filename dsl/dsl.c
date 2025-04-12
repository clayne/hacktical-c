#include <assert.h>
#include <string.h>
#include "dsl.h"

static void push_eval(struct hc_dsl *dsl, void *data) {
  struct hc_push_op *op = data;
  *(hc_fix *)hc_vector_push(&dsl->stack) = op->value;
}

const struct hc_op hc_push_op = (struct hc_op){
  .code = 0,
  .name = "push",
  .size = sizeof(struct hc_push_op),
  .eval = push_eval
};

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_vector_init(&dsl->code, 1);
  hc_vector_init(&dsl->ops, sizeof(struct hc_op));
  hc_vector_init(&dsl->registers, sizeof(hc_fix));
  hc_vector_init(&dsl->stack, sizeof(hc_fix));

  *(struct hc_op *)hc_vector_push(&dsl->ops) = hc_push_op;
}

void hc_dsl_deinit(struct hc_dsl *dsl) {
  hc_vector_deinit(&dsl->code);
  hc_vector_deinit(&dsl->ops);
  hc_vector_deinit(&dsl->registers);
  hc_vector_deinit(&dsl->stack);
}

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data) {
  size_t pc = dsl->code.length;
  *(uint8_t *)hc_vector_push(&dsl->code) = op->code;
  size_t offset = hc_align(dsl->code.end, op->size) - dsl->code.start;
  
  hc_vector_insert(&dsl->code,
		   dsl->code.length,
		   offset + op->size);

  memcpy(dsl->code.start + offset, data, op->size);
  return pc;
}
