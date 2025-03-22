#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void error_tests() {
  void on_catch(struct hc_error *e) {
    assert(e->code == 12345);
    free(e);
  }
  
  hc_catch(on_catch) {
    hc_throw(12345, "Going %s", "Down!");
  }
}
