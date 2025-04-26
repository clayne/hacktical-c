#include "chrono/chrono.h"
#include "fix.h"

void fix_benchmark() {
  struct hc_time t;
  const int n = 100000;

  t = hc_now();
  double dv = 0;
  
  for (int i = 0; i < n; i++) {
    dv += 0.001;
  }

  hc_time_print(&t, "double: "); 

  t = hc_now();
  hc_fix fv = hc_fix_new(3, 0);
  hc_fix fd = hc_fix_new(3, 1);
  
  for (int i = 0; i < n; i++) {
    fv = hc_fix_add(fv, fd);
  }

  hc_time_print(&t, "fix: "); 
}
