#ifndef HACKTICAL_DSL_H
#define HACKTICAL_DSL_H

#include "vm/vm.h"

enum hc_order hc_strcmp(const char *x, const char *y);
char *hc_upcase(char *s);

struct hc_dsl {
  struct hc_set env;
  struct hc_stream *out;

  struct hc_vm vm;
};

void hc_dsl_init(struct hc_dsl *dsl, struct hc_malloc *malloc);
void hc_dsl_deinit(struct hc_dsl *dsl);

struct hc_value* hc_dsl_getenv(struct hc_dsl *dsl, const char *key);

struct hc_value *hc_dsl_setenv(struct hc_dsl *dsl,
			       const char *key,
			       const struct hc_type *type);

void hc_dsl_set_fun(struct hc_dsl *dsl, const char *key, hc_vm_fun_t val);
void hc_dsl_set_string(struct hc_dsl *dsl, const char *key, const char *val);
void hc_dsl_eval(struct hc_dsl *dsl, const char *in);

struct hc_form;

enum hc_dsl_parsed {
  HC_DSL_CALL = 1, HC_DSL_ID, HC_DSL_TEXT
};

struct hc_form_type {
  void (*emit)(struct hc_form *, struct hc_dsl *);
  void (*print)(const struct hc_form *, struct hc_stream *);
  struct hc_value *(*value)(const struct hc_form *, struct hc_dsl *);
  void (*free)(struct hc_form *);
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
struct hc_value *hc_form_value(const struct hc_form *f, struct hc_dsl *dsl);
void hc_form_free(struct hc_form *f);

extern const struct hc_form_type HC_CALL_FORM;

struct hc_call {
  struct hc_form form;
  struct hc_form *target;
  struct hc_list args;
};

void hc_call_init(struct hc_call *f,
		  struct hc_sloc sloc,
		  struct hc_list *owner,
		  struct hc_form *target);

extern const struct hc_form_type HC_ID_FORM;

struct hc_id {
  struct hc_form form;
  char *name;
};

void hc_id_init(struct hc_id *f,
		struct hc_sloc sloc,
		struct hc_list *owner,
		const char *name);

extern const struct hc_form_type HC_LITERAL;

struct hc_literal {
  struct hc_form form;
  struct hc_value value;
};

void hc_literal_init(struct hc_literal *f,
		     struct hc_sloc sloc,
		     struct hc_list *owner);

void hc_skip_ws(const char **in, struct hc_sloc *sloc);

void hc_read_call(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc);

bool hc_read_expr(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc);

void hc_read_id(const char **in,
		struct hc_list *out,
		struct hc_sloc *sloc);

bool hc_read_next(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc);

bool hc_read_text(const char **in,
		  struct hc_list *out,
		  struct hc_sloc *sloc);

void hc_forms_emit(struct hc_list *in, struct hc_dsl *dsl);
void hc_forms_free(struct hc_list *in);

#endif
