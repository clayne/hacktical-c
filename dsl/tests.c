#include <assert.h>
#include "dsl.h"

void dsl_tests() {
  struct hc_dsl dsl;
  hc_dsl_init(&dsl);
  hc_defer(hc_dsl_deinit(&dsl));
  const hc_fix v = hc_fix_new(0, 42);
  
  hc_dsl_emit(&dsl,
	      &hc_push_op,
	      &(struct hc_push_op){
		.value = v
	      });

  hc_dsl_eval(&dsl, 0, -1);
  assert(dsl.stack.length == 1);
  assert(hc_dsl_pop(&dsl) == v);
}
