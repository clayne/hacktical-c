#include <assert.h>
#include "dcgen.h"

void dcgen_tests() {
  hc_compile("#include <stdio.h> void dcgen() { printf(\"hello dcgen\n\"); }",
	     "/tmp/dcgen.so");
}
