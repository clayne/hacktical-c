#include <stdlib.h>

#include "chrono/chrono.h"
#include "malloc2.h"

static void run_malloc(const int n, const int s) {
  int *ps[n];
  struct hc_time t = hc_now();
  
  for (int i = 0; i < n; i++) {
    ps[i] = malloc(s);
  }

  for (int i = 0; i < n; i++) {
    free(ps[i]);
  }

  hc_time_print(&t, "malloc: "); 
}

static void run_bump(const int n, const int s) {
  struct hc_bump_alloc a;
  hc_bump_alloc_init(&a, hc_malloc(), n * s);
  struct hc_time t = hc_now();

  hc_malloc_do(&a) {
    for (int i = 0; i < n; i++) {
      hc_acquire(s);
    }
  }

  hc_bump_alloc_deinit(&a);
  hc_time_print(&t, "bump: ");
}

static void run_slab(const int n, const int s) {
  struct hc_slab_alloc a;
  hc_slab_alloc_init(&a, hc_malloc(), n * s / 10);
  struct hc_time t = hc_now();

  hc_malloc_do(&a) {
    for (int i = 0; i < n; i++) {
      hc_acquire(s);
    }
  }

  hc_slab_alloc_deinit(&a);
  hc_time_print(&t, "slab: ");
}

void malloc2_benchmarks() {
  const int n = 1000;
  const int s = sizeof(int);
  
  run_malloc(n, s);
  run_bump(n, s);
  run_slab(n, s);
}
