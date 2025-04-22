#ifndef HACKTICAL_ERROR_H
#define HACKTICAL_ERROR_H

#include <stdbool.h>
#include <setjmp.h>
#include "macro/macro.h"

#define HC_NO_MEMORY "NO_MEMORY"
#define HC_INVALID_SIZE "INVALID_SIZE"

#define hc_throw(m, ...) do {					\
    struct hc_error *_e =					\
      hc_error_new("Error in '%s', line %d:\n" m "\n",		\
		   __FILE__, __LINE__, ##__VA_ARGS__);		\
    _hc_throw(_e);						\
  } while(0)

#define _hc_catch(_e, _f, h)						\
    jmp_buf _e;								\
    bool _f = true;							\
    if (setjmp(_e)) {							\
      h(hc_error);							\
      hc_error_free(hc_error);						\
    } else for (hc_catch_push(_e); _f; _f = false, hc_catch_pop())	\

#define hc_catch(h)				\
  _hc_catch(hc_unique(env), hc_unique(flag), h)

void hc_catch_push(jmp_buf h);
void hc_catch_pop();
void hc_errors_deinit();

struct hc_error {
  char *message;
};


extern __thread struct hc_error *hc_error;
struct hc_error *hc_error_new(const char *message, ...);
void _hc_throw(struct hc_error *e);
void hc_error_free(struct hc_error *e);

bool hc_streq(const char *l, const char *r);

#endif
