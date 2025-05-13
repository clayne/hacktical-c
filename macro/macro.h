#ifndef HACKTICAL_MACRO_H
#define HACKTICAL_MACRO_H

#include <stddef.h>
#include <stdint.h>

#define hc_abs(x) ({				\
      __auto_type _x = x;			\
      _x < 0 ? -x : x;				\
    })						\

#define hc_align(base, size) ({				\
      __auto_type _base = base;				\
      __auto_type _size = hc_alignof(size);		\
      __auto_type _rest = (ptrdiff_t)_base % _size;	\
      (_rest) ? _base + _size - _rest : _base;		\
    })							\

size_t hc_alignof(size_t size);

#define _hc_array(t, a, n, ...)			\
  t a[] = { __VA_ARGS__ };			\
  const size_t n = sizeof(a) / sizeof(t)		

#define hc_array(t, p, ...)					\
  _hc_array(t, hc_id(p, _a), hc_id(p, _n), ##__VA_ARGS__)

#define hc_baseof(p, t, m) ({			\
      uint8_t *_p = (uint8_t *)p;		\
      _p ? ((t *)(_p - offsetof(t, m))) : NULL;	\
    })

#define hc_bitmask(v, bc)			\
  (v & ((1 << bc) - 1))

#define hc_const(x) ({						\
      sizeof(int) ==						\
      sizeof(*(8 ? ((void *)((long)(x) * 0l)) : (int *)8))	\
    })

#define _hc_defer(_d, _v, ...)			\
  void _d(int *) { __VA_ARGS__; }		\
  int _v __attribute__ ((__cleanup__(_d)))

#define hc_defer(...)							\
  _hc_defer(hc_unique(defer_d), hc_unique(defer_v), ##__VA_ARGS__)

#define _hc_id(x, y)				\
  x ## y

#define hc_id(x, y)				\
  _hc_id(x, y)

#define hc_max(x, y) ({				\
      __auto_type _x = x;			\
      __auto_type _y = y;			\
      _x > _y ? _x : _y;			\
    })						\

#define hc_min(x, y) ({				\
      __auto_type _x = x;			\
      __auto_type _y = y;			\
      _x < _y ? _x : _y;			\
    })						\

#define hc_sign(x)				\
  (x < 0 ? -1 : 1)				\

#define hc_unique(x)				\
  hc_id(x, __COUNTER__)

#endif
