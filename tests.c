#include "error/error.h"

#include "error/tests.c"
#include "fix/tests.c"
#include "list/tests.c"
#include "macro/tests.c"
#include "malloc/tests.c"
#include "task/tests.c"
#include "time/tests.c"
#include "vector/tests.c"

int main() {
  error_tests();
  fix_tests();
  list_tests();
  macro_tests();
  malloc_tests();
  task_tests();
  time_tests();
  vector_tests();

  hc_errors_deinit();
  return 0;
}
