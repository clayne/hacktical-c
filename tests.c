#include "fix/tests.c"
#include "list/tests.c"
#include "macro/tests.c"
#include "task/tests.c"
#include "time/tests.c"
#include "vector/tests.c"

int main() {
  fix_tests();
  list_tests();
  macro_tests();
  task_tests();
  time_tests();
  vector_tests();
  return 0;
}
