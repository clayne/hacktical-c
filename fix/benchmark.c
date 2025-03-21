#include "fix.h"
#include "time/time.h"

void fix_benchmark() {
  struct hc_timer t;
  const int n = 10000;

  hc_timer_init(&t);
  double dv = 0;
  
  for (int i = 0; i < n; i++) {
    dv += 0.01;
  }

  hc_timer_print(&t, "double: "); 

  hc_timer_init(&t);
  hc_fix fv = hc_fix_new(HC_FIX_MAX_EXP, 0);
  hc_fix fd = hc_fix_new(2, 1);
  
  for (int i = 0; i < n; i++) {
    fv = hc_fix_add(fv, fd);
  }

  hc_timer_print(&t, "fix: "); 
}
