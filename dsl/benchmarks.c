#include "chrono/chrono.h"
#include "dsl.h"

void dsl_benchmarks() {
  hc_time_t t;
  const int n = 100000;

  char buf[32];
  t = hc_now();
  
  for (int i = 0; i < n; i++) {
    sprintf(buf, "abc %s def", "ghi");
  }

  hc_time_print(&t, "sprintf: "); 
  
  struct hc_dsl dsl;
  hc_dsl_init(&dsl, &hc_malloc_default);
  hc_defer(hc_dsl_deinit(&dsl));
  struct hc_memory_stream out;
  hc_memory_stream_init(&out, &hc_malloc_default);
  hc_defer(hc_stream_deinit(&out.stream));
  dsl.out = &out.stream;
  hc_dsl_set_string(&dsl, "foo", "ghi");
  hc_dsl_eval(&dsl, "abc $(print foo) def");

  t = hc_now();

  for (int i = 0; i < n; i++) {
    hc_vector_clear(&out.data);
    hc_vm_eval(&dsl.vm, 0, -1);
  }

  hc_time_print(&t, "dsl: ");
}
