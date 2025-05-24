#include "error/error.h"

#include "chrono/tests.c"
#include "dsl/tests.c"
#include "dynamic/tests.c"
#include "error/tests.c"
#include "fix/tests.c"
#include "list/tests.c"
#include "macro/tests.c"
#include "malloc1/tests.c"
#include "malloc2/tests.c"
#include "reflect/tests.c"
#include "set/tests.c"
#include "slog/tests.c"
#include "stream1/tests.c"
#include "task/tests.c"
#include "vector/tests.c"
#include "vm/tests.c"

int main() {
  chrono_tests();
  dsl_tests();
  dynamic_tests();
  error_tests();
  fix_tests();
  list_tests();
  macro_tests();
  malloc1_tests();
  malloc2_tests();
  reflect_tests();
  set_tests();
  slog_tests();
  stream1_tests();
  task_tests();
  vector_tests();
  vm_tests();

  hc_errors_deinit();
  return 0;
}
