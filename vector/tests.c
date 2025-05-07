#include <assert.h>
#include <stdio.h>
#include "vector.h"

void vector_tests() {
  struct hc_vector v;
  hc_vector_init(&v, &hc_malloc_default, sizeof(int));
  const int n = 100;
  
  for (int i = 0; i < n; i++) {
    *(int *)hc_vector_push(&v) = i;
  }

  {
    int i = 0;
    
    hc_vector_do(&v, it) {
      assert(*(int *)it == i++);
    }
  }
  
  assert(v.length == n);
  
  for (int i = 0; i < n; i++) {
    assert(*(int *)hc_vector_get(&v, i) == i);
  }

  assert(*(int *)hc_vector_pop(&v) == n-1);
  assert(*(int *)hc_vector_peek(&v) == n-2);
  assert(v.length == n-1);
  
  for (int i = 0; i < n-1; i++) {
    assert(*(int *)hc_vector_get(&v, i) == i);
  }

  assert(hc_vector_delete(&v, 0, 1));
  assert(v.length == n-2);

  for (int i = 1; i < n-1; i++) {
    assert(*(int *)hc_vector_get(&v, i-1) == i);
  }

  (*(int *)hc_vector_insert(&v, 0, 1) = 0);
  assert(v.length == n-1);

  for (int i = 0; i < n-1; i++) {
    assert(*(int *)hc_vector_get(&v, i) == i);
  }

  hc_vector_clear(&v);
  assert(v.length == 0);
  
  hc_vector_deinit(&v);
}
