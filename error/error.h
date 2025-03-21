#ifndef HACKTICAL_ERROR_H
#define HACKTICAL_ERROR_H

#include <stdbool.h>
#include <setjmp.h>
#include "macro/macro.h"

#define hc_throw(c, m, ...) do {					\
    struct hc_error *e =						\
      hc_error_new((c), "Failure %d in '%s', line %d:\n" m "\n",	\
		   (c), __FILE__, __LINE__, ##__VA_ARGS__);		\
    _hc_throw(e);							\
  } while(0)

#define _hc_catch(_e, _f, h)					\
  jmp_buf _e;							\
  bool _f = true;						\
  if (setjmp(_e)) {						\
    h(hc_error);						\
  } else							\
    for (hc_catch_push(_e); _f; _f = false, hc_catch_pop())	\

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

#endif
