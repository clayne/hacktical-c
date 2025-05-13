#include <assert.h>
#include "dsl2.h"

static void read_call_tests() {
  struct hc_sloc sloc = hc_sloc("read_call_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);
  hc_defer(hc_forms_free(&out));

  const char *s = "( foo bar )";
  const char *in = s;
  assert(hc_read_expr(&in, &out, &sloc));
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, owner);
  assert(f->type == &hc_call);
}

static void read_id_tests() {
  struct hc_sloc sloc = hc_sloc("read_id_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);
  hc_defer(hc_forms_free(&out));

  const char *s = "foo";
  const char *in = s;
  assert(hc_read_expr(&in, &out, &sloc));
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, owner);
  assert(f->type == &hc_id);
}

static void eval_tests() {
  struct hc_dsl dsl;
  hc_dsl_init(&dsl, &hc_malloc_default);
  hc_defer(hc_dsl_deinit(&dsl));
  hc_dsl_evals(&dsl, "abc $(print foo) def");
}

void dsl2_tests() {
  read_id_tests();
  read_call_tests();
  eval_tests();
}
