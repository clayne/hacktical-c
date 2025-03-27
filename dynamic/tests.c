#include <assert.h>
#include "dynamic.h"

void dynamic_tests() {
  const char *out = "/var/tmp/libdynamic.so";
  
  hc_compile("#include <stdio.h>\n"
	     "int dynamic() { return 42; }",
	     out);

  struct hc_dlib lib;
  hc_dlib_init(&lib, out);
  hc_defer(hc_dlib_deinit(&lib));
  int (*fn)() = hc_dlib_find(&lib, "dynamic");
  assert(fn() == 42);
}
