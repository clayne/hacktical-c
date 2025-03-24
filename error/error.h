#ifndef HACKTICAL_ERROR_H
#define HACKTICAL_ERROR_H

#include <stdbool.h>
#include <setjmp.h>
#include "macro/macro.h"

#define __hc_throw(c, _c, m, ...) do {				\
    int _c = c;							\
    struct hc_error *_e =					\
      hc_error_new(_c, "Error %d in '%s', line %d:\n" m "\n",	\
		   _c, __FILE__, __LINE__, ##__VA_ARGS__);	\
    _hc_throw(_e);						\
  } while(0)

#define hc_throw(c, m, ...)				\
  __hc_throw(c, hc_unique(throw_c), m, ##__VA_ARGS__)

#define _hc_catch(_e, _f, h)						\
  jmp_buf _e;								\
  bool _f = true;							\
  if (setjmp(_e)) {							\
    h(hc_error);							\
    hc_error_free(hc_error);						\
  } else for (hc_catch_push(_e); _f; _f = false, hc_catch_pop())

#define hc_catch(h)				\
  _hc_catch(hc_unique(env), hc_unique(flag), h)

void hc_catch_push(jmp_buf h);
void hc_catch_pop();
void hc_errors_deinit();

struct hc_error {
  int code;
  char message[];
};


extern __thread struct hc_error *hc_error;
struct hc_error *hc_error_new(int code, const char *message, ...);
void _hc_throw(struct hc_error *e);
void hc_error_free(struct hc_error *e);

#endif
