#include <assert.h>
#include "dsl2.h"

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

void dsl2_tests() {
  read_id_tests();
  read_call_tests();
}
