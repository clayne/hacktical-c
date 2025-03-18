#ifndef HACKTICAL_MACROS_H
#define HACKTICAL_MACROS_H

#include <stddef.h>
#include <stdint.h>

#define hc_align(base, size) ({						\
      __auto_type _base = base;						\
      __auto_type _size = hc_min((size), _Alignof(max_align_t));	\
      (_base) + _size - ((ptrdiff_t)(_base)) % _size;			\
    })									\

#define hc_baseof(p, t, m) ({			\
      uint8_t *_p = (uint8_t *)p;		\
      _p ? ((t *)(_p - offsetof(t, m))) : NULL;	\
    })

#define _hc_id(x, y)				\
  x##y

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

#define hc_unique(x)				\
  hc_id(x, __COUNTER__)

#endif
