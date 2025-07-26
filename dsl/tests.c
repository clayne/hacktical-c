#include <assert.h>
#include <string.h>
#include "dsl.h"

static void read_call_tests() {
  struct hc_sloc sloc = hc_sloc("read_call_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);
  hc_defer(hc_forms_free(&out));

  const char *s = "( foo bar )";
  const char *in = s;
  assert(hc_read_expr(&in, &out, &sloc));
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, owner);
  assert(f->type == &HC_CALL_FORM);
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
  assert(f->type == &HC_ID_FORM);
}

static void eval_tests() {
  struct hc_dsl dsl;
  hc_dsl_init(&dsl, &hc_malloc_default);
  hc_defer(hc_dsl_deinit(&dsl));
  struct hc_memory_stream out;
  hc_memory_stream_init(&out, &hc_malloc_default);
  hc_defer(hc_stream_deinit(&out.stream));
  dsl.out = &out.stream;
  hc_dsl_set_string(&dsl, "foo", "ghi");
  hc_dsl_eval(&dsl, "abc $(print (upcase foo)) def");
  assert(strcmp("abc GHI def", hc_memory_stream_string(&out)) == 0);
}

void dsl_tests() {
  read_id_tests();
  read_call_tests();
  eval_tests();
}
