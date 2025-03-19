#include "list/tests.c"
#include "task/tests.c"
#include "vector/tests.c"

int main() {
  list_tests();
  task_tests();
  vector_tests();
  return 0;
}
