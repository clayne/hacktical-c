#ifndef HACKTICAL_REFLECT_H
#define HACKTICAL_REFLECT_H

#include <stdbool.h>

#include "chrono/chrono.h"
#include "fix/fix.h"

struct hc_value;

struct hc_type {
  const char *name;
  
  void (*copy)(struct hc_value *dst, struct hc_value *src);
  void (*deinit)(struct hc_value *);
  void (*print)(const struct hc_value *, struct hc_stream *out);
  void (*write)(const struct hc_value *, struct hc_stream *out);
};

struct hc_value {
  const struct hc_type *type;
  
  union {
    bool as_bool;
    hc_fix_t as_fix;
    int as_int;
    void *as_other;
    char *as_string;
    hc_time_t as_time;
  };
};

struct hc_value *hc_value_init(struct hc_value *v, const struct hc_type *t);
void hc_value_deinit(struct hc_value *v);
struct hc_value *hc_value_copy(struct hc_value *dst, struct hc_value *src);
void hc_value_print(struct hc_value *v, struct hc_stream *out);
void hc_value_write(struct hc_value *v, struct hc_stream *out);

extern const struct hc_type HC_BOOL;
extern const struct hc_type HC_FIX;
extern const struct hc_type HC_INT;
extern const struct hc_type HC_STRING;
extern const struct hc_type HC_TIME;

#endif
