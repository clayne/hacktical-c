#include <assert.h>
#include "dcgen.h"

void dcgen_tests() {
  hc_compile("#include <stdio.h>\n"
	     "int dcgen() { return 42; }",
	     "/var/tmp/dcgen.so");

  //TODO dynload & check result
}
