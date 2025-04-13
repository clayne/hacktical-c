#ifndef HACKTICAL_DSL_H
#define HACKTICAL_DSL_H

#include "fix/fix.h"
#include "vector/vector.h"

struct hc_op;

typedef size_t hc_pc;

struct hc_dsl {
  struct hc_vector code;
  struct hc_vector registers;
  struct hc_vector stack;
};

void hc_dsl_init(struct hc_dsl *dsl);
void hc_dsl_deinit(struct hc_dsl *dsl);

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data);

void hc_dsl_eval(struct hc_dsl *dsl, const hc_pc pc);

void hc_dsl_push(struct hc_dsl *dsl, hc_fix v);
hc_fix hc_dsl_peek(struct hc_dsl *dsl);
hc_fix hc_dsl_pop(struct hc_dsl *dsl);

struct hc_op {
  const char *name;
  size_t size;
  
  const uint8_t *(*eval)(struct hc_dsl *dsl, const uint8_t *data);
};

typedef const uint8_t *(*hc_eval)(struct hc_dsl *dsl, const uint8_t *data);

struct hc_push_op {
  hc_fix value;
};

extern const struct hc_op hc_push_op;

extern const struct hc_op hc_stop_op;

#endif
