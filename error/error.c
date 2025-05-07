#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "malloc1/malloc1.h"
#include "vector/vector.h"

__thread struct hc_error *hc_error = NULL;

static struct hc_vector *handlers() {
  static bool init = true;
  static __thread struct hc_vector handlers;

  if (init) {
    hc_vector_init(&handlers, &hc_malloc_default, sizeof(jmp_buf));
    init = false;
  }
  
  return &handlers;
}

void hc_catch_push(jmp_buf h) {
  memcpy((jmp_buf *)hc_vector_push(handlers()), h, sizeof(jmp_buf));
}

void hc_catch_pop() {
  hc_vector_pop(handlers());
}

void hc_errors_deinit() {
  hc_vector_deinit(handlers());
}

void _hc_throw(struct hc_error *e) {
  struct hc_vector *hs = handlers();

  if (!hs->length) {
    fputs(e->message, stderr);
    hc_error_free(e);
    abort();
  }
  
  jmp_buf t;
  memcpy(t, *(jmp_buf *)hc_vector_pop(hs), sizeof(jmp_buf));
  hc_error = e;
  longjmp(t, 1);
}

struct hc_error *hc_error_new(const char *message, ...) {
  va_list args;
  va_start(args, message);
  
  va_list tmp_args;
  va_copy(tmp_args, args);
  int len = vsnprintf(NULL, 0, message, tmp_args);
  va_end(tmp_args);

  if (len < 0) {
    vfprintf(stderr, message, args);
    abort();
  }
  
  len++;
  struct hc_error *e = malloc(sizeof(struct hc_error));
  e->message = malloc(len);
  vsnprintf(e->message, len, message, args);
  va_end(args);
  return e;
}

void hc_error_free(struct hc_error *e) {
  free(e->message);
  free(e);
}

bool hc_streq(const char *l, const char *r) {
  for (; *l && *l == *r; l++, r++);
  return *l == *r;
}
