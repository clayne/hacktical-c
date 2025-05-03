#ifndef HACKTICAL_REFLECT_H
#define HACKTICAL_REFLECT_H

#include "chrono/chrono.h"
#include "fix/fix.h"

void hc_strncpy(char *dst, const char *src, size_t n);

struct hc_value;

struct hc_type {
  char name[32];
  
  void (*copy)(struct hc_value *dst, struct hc_value *src);
  void (*deinit)(struct hc_value *);
  void (*write)(const struct hc_value *, struct hc_stream *out);
};

void hc_type_init(struct hc_type *t, const char *name);

struct hc_value {
  const struct hc_type *type;
  
  union {
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
void hc_value_write(struct hc_value *v, struct hc_stream *out);

const struct hc_type *HC_FIX();
const struct hc_type *HC_INT();
const struct hc_type *HC_STRING();
const struct hc_type *HC_TIME();

#endif
