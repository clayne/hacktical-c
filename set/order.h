#ifndef HACKTICAL_ORDER_H
#define HACKTICAL_ORDER_H

#define hc_cmp(x, y) ({					\
      __auto_type _x = x;				\
      __auto_type _y = y;				\
      (_x < _y) ? HC_LT : ((_x > _y) ? HC_GT : HC_EQ);	\
    })

enum hc_order {HC_LT = -1, HC_EQ = 0, HC_GT = 1};

typedef enum hc_order (*hc_cmp_t)(const void *, const void *);

#endif
