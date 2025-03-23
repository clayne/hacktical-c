#include "error/error.h"

#include "fix/benchmark.c"

int main() {
  hc_malloc_init();

  fix_benchmark();

  hc_errors_deinit();
  return 0;
}
