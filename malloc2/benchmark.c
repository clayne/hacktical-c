#include <stdlib.h>

#include "chrono/chrono.h"
#include "malloc2.h"

static void malloc_benchmark(const int n, const int s) {
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

static void bump_benchmark(const int n, const int s) {
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

static void slab_benchmark(const int n, const int s) {
  struct hc_slab_alloc a;
  hc_slab_alloc_init(&a, hc_malloc(), n * s);
  struct hc_time t = hc_now();

  hc_malloc_do(&a) {
    for (int i = 0; i < n; i++) {
      hc_acquire(s);
    }
  }

  hc_slab_alloc_deinit(&a);
  hc_time_print(&t, "slab: ");
}

void malloc2_benchmark() {
  const int n = 1000000;
  const int s = sizeof(int);
  
  malloc_benchmark(n, s);
  bump_benchmark(n, s);
  slab_benchmark(n, s);
}
