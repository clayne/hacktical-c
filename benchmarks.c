#include "error/error.h"

#include "fix/benchmark.c"
#include "malloc2/benchmark.c"

int main() {
  fix_benchmark();
  malloc2_benchmark();

  hc_errors_deinit();
  return 0;
}
