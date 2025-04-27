#include <stdlib.h>

#include "chrono/chrono.h"
#include "malloc2.h"

#define N 1000
#define MAX_SIZE 64
#define GET_SIZE() ((rand() % MAX_SIZE) + 1)

static void run_malloc() {
  int *ps[N];
  struct hc_time t = hc_now();
  
  for (int i = 0; i < N; i++) {
    ps[i] = malloc(GET_SIZE());
  }

  for (int i = 0; i < N; i++) {
    free(ps[i]);
  }

  hc_time_print(&t, "malloc: "); 
}

static void run_bump() {
  struct hc_bump_alloc a;
  hc_bump_alloc_init(&a, hc_malloc(), N * MAX_SIZE);
  struct hc_time t = hc_now();

  hc_malloc_do(&a) {
    for (int i = 0; i < N; i++) {
      hc_acquire(GET_SIZE());
    }
  }

  hc_bump_alloc_deinit(&a);
  hc_time_print(&t, "bump: ");
}

static void run_slab() {
  struct hc_slab_alloc a;
  hc_slab_alloc_init(&a, hc_malloc(), N);
  struct hc_time t = hc_now();

  hc_malloc_do(&a) {
    for (int i = 0; i < N; i++) {
      hc_acquire(GET_SIZE());
    }
  }

  hc_slab_alloc_deinit(&a);
  hc_time_print(&t, "slab: ");
}

void malloc2_benchmarks() {
  const int s = time(NULL);
  
  srand(s);
  run_malloc();

  srand(s);
  run_bump();

  srand(s);
  run_slab();
}
