#ifndef HACKTICAL_DSL2_H
#define HACKTICAL_DSL2_H

#include "dsl1/dsl1.h"

struct hc_form;

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

extern const struct hc_form_type hc_call;

struct hc_call {
  struct hc_form form;
  struct hc_form *target;
  struct hc_list *args;
};

void hc_call_init(struct hc_call *f,
		  struct hc_sloc sloc,
		  struct hc_list *owner,
		  struct hc_form *target,
		  struct hc_list *args);

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
void hc_dsl_evals(struct hc_dsl *dsl, const char *in);

#endif
