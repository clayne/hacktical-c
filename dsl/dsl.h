#ifndef HACKTICAL_DSL_H
#define HACKTICAL_DSL_H

#include "fix/fix.h"
#include "stream1/stream1.h"
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

void hc_dsl_eval(struct hc_dsl *dsl, hc_pc start_pc, hc_pc end_pc);

void hc_dsl_push(struct hc_dsl *dsl, hc_fix v);
hc_fix hc_dsl_peek(struct hc_dsl *dsl);
hc_fix hc_dsl_pop(struct hc_dsl *dsl);

struct hc_sloc {
  char source[32];
  int row;
  int col;
};

struct hc_sloc hc_sloc(const char *source, int row, int col);

struct hc_form;

struct hc_form_type {
  void (*deinit)(struct hc_form *, struct hc_dsl *);
  void (*emit)(const struct hc_form *, struct hc_dsl *);
  void (*print)(const struct hc_form *, struct hc_stream *);
};
  
struct hc_form {
  const struct hc_form_type *type;
  struct hc_sloc sloc;
};

struct hc_id {
  struct hc_form form;
  char *name;
};

void hc_id_init(struct hc_id *f,
		struct hc_sloc sloc,
		const char *name);

struct hc_literal {
  struct hc_form form;
  hc_fix value;
};

void hc_literal_init(struct hc_literal *f,
		     struct hc_sloc sloc,
		     hc_fix value);

struct hc_op {
  const char *name;
  size_t size;
  
  const uint8_t *(*eval)(struct hc_dsl *, const uint8_t *);
};

typedef const uint8_t *(*hc_eval)(struct hc_dsl *dsl, const uint8_t *data);

struct hc_push_op {
  hc_fix value;
};

extern const struct hc_op hc_push_op;

extern const struct hc_op hc_stop_op;

#endif
