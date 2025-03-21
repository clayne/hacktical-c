#include "error/error.h"

#include "fix/benchmark.c"

int main() {
  fix_benchmark();

  hc_errors_deinit();
  return 0;
}
