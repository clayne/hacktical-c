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
  struct hc_vm vm;
  hc_dsl_init(&vm);
  hc_defer(hc_vm_deinit(&vm));
  struct hc_memory_stream out;
  hc_memory_stream_init(&out, hc_malloc());
  hc_defer(hc_stream_deinit(&out.stream));
  vm.out = &out.stream;
  hc_vm_setenv(&vm, "foo", &HC_STRING)->as_string = strdup("ghi");
  hc_dsl_eval(&vm, "abc $(print foo) def");
  assert(strcmp("abc ghi def", hc_memory_stream_string(&out)) == 0);
}

void dsl_tests() {
  read_id_tests();
  read_call_tests();
  eval_tests();
}
