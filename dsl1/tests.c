#include <assert.h>
#include "dsl1.h"
#include "malloc2/malloc2.h"

static void read_tests() {
  struct hc_sloc sloc = hc_sloc("read_tests", 0, 0);
  
  struct hc_list out;
  hc_list_init(&out);

  const char *s = " foo";
  const char *in = s;

  struct hc_slab_alloc a;
  hc_slab_alloc_init(&a, hc_malloc(), 10*sizeof(struct hc_form));

  hc_malloc_do(&a) {
    assert(hc_read_form(&in, &out, &sloc));
  }
  
  struct hc_form *f = hc_baseof(out.next, struct hc_form, list);
  assert(f->type == hc_id_form());

  hc_list_do(&out, f) {
    hc_form_deinit(hc_baseof(f, struct hc_form, list));
  }

  hc_slab_alloc_deinit(&a);
}

static void emit_tests() {
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

void dsl1_tests() {
  read_tests();
  emit_tests();
}
