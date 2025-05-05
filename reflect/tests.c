#include <assert.h>
#include "reflect.h"

void reflect_tests() {
  struct hc_value v;
  hc_value_init(&v, &HC_STRING)->as_string = strdup("foo");
  hc_defer(hc_value_deinit(&v));
  struct hc_value c;
  hc_value_copy(&c, &v);
  hc_defer(hc_value_deinit(&c));
  assert(strcmp(c.as_string, v.as_string) == 0);
}
