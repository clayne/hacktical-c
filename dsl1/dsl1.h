#ifndef HACKTICAL_DSL1_H
#define HACKTICAL_DSL1_H

#include "list/list.h"
#include "reflect/reflect.h"
#include "set/set.h"
#include "stream1/stream1.h"
#include "vector/vector.h"

enum hc_order hc_strcmp(const char *x, const char *y);

struct hc_sloc {
  char source[32];
  int row;
  int col;
};

struct hc_sloc hc_sloc(const char *source, int row, int col);

typedef size_t hc_pc;

struct hc_dsl {
  struct hc_set env;
  struct hc_vector stack;
  
  struct hc_vector ops;
  struct hc_vector code;
};

void hc_dsl_init(struct hc_dsl *dsl, struct hc_malloc *malloc);
void hc_dsl_deinit(struct hc_dsl *dsl);

struct hc_value* hc_dsl_getenv(struct hc_dsl *dsl,
			       const char *key,
			       const struct hc_sloc sloc);

void hc_dsl_setenv(struct hc_dsl *dsl,
		   const char *key,
		   struct hc_value val);

struct hc_value *hc_dsl_push(struct hc_dsl *dsl);
struct hc_value *hc_dsl_peek(struct hc_dsl *dsl);
struct hc_value *hc_dsl_pop(struct hc_dsl *dsl);

struct hc_op;

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data);

void hc_dsl_eval(struct hc_dsl *dsl, hc_pc start_pc, hc_pc end_pc);

typedef const uint8_t *(*hc_op_eval_t)(struct hc_dsl *, const uint8_t *);

struct hc_op {
  size_t size;
  const char *name;

  void (*deinit)(uint8_t *);
  hc_op_eval_t eval;
};

typedef void (*hc_dsl_fun_t)(struct hc_dsl *, struct hc_sloc);

struct hc_call_op {
  hc_dsl_fun_t target;
  struct hc_sloc sloc;
};

struct hc_push_op {
  struct hc_value value;
};

extern const struct hc_op HC_CALL;
extern const struct hc_op HC_PUSH;
extern const struct hc_op HC_STOP;

#endif
