#include "error/error.h"

#include "dsl/benchmarks.c"
#include "fix/benchmarks.c"
#include "malloc2/benchmarks.c"

int main() {
  fix_benchmarks();
  malloc2_benchmarks();
  dsl_benchmarks();

  hc_errors_deinit();
  return 0;
}
