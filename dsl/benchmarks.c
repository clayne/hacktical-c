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
  
  struct hc_vm vm;
  hc_dsl_init(&vm);
  hc_defer(hc_vm_deinit(&vm));
  struct hc_memory_stream out;
  hc_memory_stream_init(&out, hc_malloc());
  hc_defer(hc_stream_deinit(&out.stream));
  vm.out = &out.stream;
  hc_dsl_set_string(&vm, "foo", "ghi");
  hc_dsl_eval(&vm, "abc $(print foo) def");

  t = hc_now();

  for (int i = 0; i < n; i++) {
    hc_vector_clear(&out.data);
    hc_vm_eval(&vm, 0, -1);
  }

  hc_time_print(&t, "dsl: "); 
}
