#include <assert.h>
#include <stdlib.h>

#include "dsl1.h"
#include "malloc2/malloc2.h"

static void read_call_tests() {
  struct hc_sloc sloc = hc_sloc("read_call_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);

  const char *s = "( foo bar )";
  const char *in = s;
  assert(hc_read_form(&in, &out, &sloc));
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, owner);
  assert(f->type == &hc_call);

  hc_list_do(&out, i) {
    hc_form_free(hc_baseof(i, struct hc_form, owner));
  }
}

static void read_id_tests() {
  struct hc_sloc sloc = hc_sloc("read_id_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);

  const char *s = " foo";
  const char *in = s;
  assert(hc_read_form(&in, &out, &sloc));
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, owner);
  assert(f->type == &hc_id);

  hc_list_do(&out, i) {
    hc_form_free(hc_baseof(i, struct hc_form, owner));
  }
}

static void read_tests() {
  read_id_tests();
  read_call_tests();
}

static void emit_tests() {
  struct hc_dsl dsl;
  hc_dsl_init(&dsl, &hc_malloc_default);
  hc_defer(hc_dsl_deinit(&dsl));
  struct hc_push_op op;
  hc_value_init(&op.value, &HC_FIX)->as_fix = hc_fix(0, 42);
  hc_dsl_emit(&dsl, &HC_PUSH, &op);
  hc_dsl_eval(&dsl, 0, -1);
  assert(dsl.stack.length == 1);
  assert(hc_dsl_pop(&dsl)->as_fix == op.value.as_fix);
}

void dsl1_tests() {
  read_tests();
  emit_tests();
}
