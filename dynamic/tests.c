#include <assert.h>
#include "dynamic.h"

void dynamic_tests() {
  const char *out = "/var/tmp/libtest.so";
  
  hc_compile("#include <stdio.h>\n"
	     "int test() { return 42; }",
	     out,
	     .cflags = (const char *[]){"-Wall",
					"-fsanitize=undefined",
					NULL});

  struct hc_dlib lib;
  hc_dlib_init(&lib, out);
  hc_defer(hc_dlib_deinit(&lib));
  int (*fn)() = hc_dlib_find(&lib, "test");
  assert(fn() == 42);
}
