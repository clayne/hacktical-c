#include "error/error.h"

#include "chrono/tests.c"
#include "dynamic/tests.c"
#include "error/tests.c"
#include "fix/tests.c"
#include "list/tests.c"
#include "macro/tests.c"
#include "malloc1/tests.c"
#include "malloc2/tests.c"
#include "set/tests.c"
#include "stream/tests.c"
#include "task/tests.c"
#include "vector/tests.c"

int main() {
  hc_malloc_init();

  chrono_tests();
  dynamic_tests();
  error_tests();
  fix_tests();
  list_tests();
  macro_tests();
  malloc1_tests();
  malloc2_tests();
  set_tests();
  stream_tests();
  task_tests();
  vector_tests();

  hc_errors_deinit();
  return 0;
}
