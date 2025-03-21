#include "fix.h"
#include "time/time.h"

void fix_benchmark() {
  struct hc_timer t;
  const int n = 100000;

  hc_timer_init(&t);
  double dv = 0;
  
  for (int i = 0; i < n; i++) {
    dv += 0.001;
  }

  hc_timer_print(&t, "double: "); 

  hc_timer_init(&t);
  hc_fix fv = hc_fix_new(3, 0);
  hc_fix fd = hc_fix_new(3, 1);
  
  for (int i = 0; i < n; i++) {
    fv = hc_fix_add(fv, fd);
  }

  hc_timer_print(&t, "fix: "); 
}
