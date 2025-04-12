#include <assert.h>
#include "dsl.h"

void dsl_tests() {
  struct hc_dsl dsl;
  hc_dsl_init(&dsl);
  hc_defer(hc_dsl_deinit(&dsl));

  hc_dsl_emit(&dsl,
	      &hc_push_op,
	      &(struct hc_push_op){
		.value = hc_fix_new(0, 42)
	      });
}
