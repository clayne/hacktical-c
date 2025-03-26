#include <assert.h>
#include "dynamic.h"

void dynamic_tests() {
  const char *out = "/var/tmp/lib_dynamic.1.0.0.so";
  
  hc_compile("#include <stdio.h>\n"
	     "int dynamic() { return 42; }",
	     out);

  //struct hc_dlib lib = hc_dlopen(out);
}
