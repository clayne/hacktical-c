#include <assert.h>
#include "reflect.h"

void reflect_tests() {
  struct hc_type t;
  hc_type_init(&t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab");
  
  assert(strncmp(t.name,
		 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
		 sizeof(t.name)-1) == 0);
}
