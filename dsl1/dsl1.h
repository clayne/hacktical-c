#ifndef HACKTICAL_DSL1_H
#define HACKTICAL_DSL1_H

#include "fix/fix.h"
#include "list/list.h"
#include "reflect/reflect.h"
#include "set/set.h"
#include "stream1/stream1.h"
#include "vector/vector.h"

struct hc_op;
struct hc_sloc;

typedef size_t hc_pc;

enum hc_order hc_strcmp(const char *x, const char *y);

struct hc_dsl {
  struct hc_vector code;
  struct hc_set env;
  struct hc_vector ops;
  struct hc_vector stack;
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

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data);

void hc_dsl_eval(struct hc_dsl *dsl, hc_pc start_pc, hc_pc end_pc);

typedef void (*hc_dsl_fun_t)(struct hc_dsl *, struct hc_sloc);

extern const struct hc_type HC_DSL_FUN;

struct hc_sloc {
  char source[32];
  int row;
  int col;
};

struct hc_sloc hc_sloc(const char *source, int row, int col);

struct hc_form;

struct hc_form_type {
  void (*emit)(const struct hc_form *, struct hc_dsl *);
  void (*free)(struct hc_form *);
  void (*print)(const struct hc_form *, struct hc_stream *);
};
  
struct hc_form {
  const struct hc_form_type *type;
  struct hc_sloc sloc;
  struct hc_list owner;
};

void hc_form_init(struct hc_form *f,
		  const struct hc_form_type *type,
		  struct hc_sloc sloc,
		  struct hc_list *owner);

void hc_form_emit(struct hc_form *f, struct hc_dsl *dsl);
void hc_form_print(struct hc_form *f, struct hc_stream *out);
void hc_form_free(struct hc_form *f);

extern const struct hc_form_type hc_expr;

struct hc_expr {
  struct hc_form form;
  struct hc_list forms;
};

void hc_expr_init(struct hc_expr *f,
		  struct hc_sloc sloc,
		  struct hc_list *owner,
		  struct hc_list items);

extern const struct hc_form_type hc_id;

struct hc_id {
  struct hc_form form;
  char *name;
};

void hc_id_init(struct hc_id *f,
		struct hc_sloc sloc,
		struct hc_list *owner,
		const char *name);

extern const struct hc_form_type hc_literal;

struct hc_literal {
  struct hc_form form;
  struct hc_value value;
};

void hc_literal_init(struct hc_literal *f,
		     struct hc_sloc sloc,
		     struct hc_list *owner,
		     struct hc_value *value);

void hc_skip_ws(const char **in, struct hc_sloc *sloc);

bool hc_read_id(const char **in,
		struct hc_list *out,
		struct hc_sloc *sloc);

bool hc_read_form(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc);

typedef const uint8_t *(*hc_op_eval_t)(struct hc_dsl *, const uint8_t *);

struct hc_op {
  size_t size;
  const char *name;

  void (*deinit)(uint8_t *);
  hc_op_eval_t eval;
};

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
