#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void error_tests() {
  void on_catch(struct hc_error *e) {
    assert(hc_streq("E123", e->message) == 0);
  }
  
  hc_catch(on_catch) {
    hc_throw("E123 Going %s", "Down!");
  }
}
