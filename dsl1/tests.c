#include <assert.h>

#include "dsl1.h"
#include "malloc1/malloc1.h"

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
  emit_tests();
}
