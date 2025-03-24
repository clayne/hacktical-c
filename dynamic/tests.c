#include <assert.h>
#include "dynamic.h"

void dynamic_tests() {
  const char *out = "/var/tmp/dynamic_test.so";

  hc_compile("#include <stdio.h>\n"
	     "int dynamic() { return 42; }",
	     out);
}
