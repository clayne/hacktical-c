#ifndef HACKTICAL_VM_H
#define HACKTICAL_VM_H

#include "list/list.h"
#include "malloc1/malloc1.h"
#include "reflect/reflect.h"
#include "set/set.h"
#include "stream1/stream1.h"
#include "vector/vector.h"

enum hc_order hc_strcmp(const char *x, const char *y);

struct hc_sloc {
  char source[32];
  char out[64];
  int row;
  int col;
};

struct hc_sloc hc_sloc(const char *source, int row, int col);
const char *hc_sloc_string(struct hc_sloc *sloc);

struct hc_vm {
  struct hc_set env;
  struct hc_vector stack;
  struct hc_stream *out;
  struct hc_vector ops;
  struct hc_vector code;
};

void hc_vm_init(struct hc_vm *vm, struct hc_malloc *malloc);
void hc_vm_deinit(struct hc_vm *vm);

struct hc_value* hc_vm_getenv(struct hc_vm *vm, const char *key);

struct hc_value *hc_vm_setenv(struct hc_vm *vm,
			      const char *key,
			      const struct hc_type *type);

struct hc_value *hc_vm_push(struct hc_vm *vm);
struct hc_value *hc_vm_peek(struct hc_vm *vm);
struct hc_value *hc_vm_pop(struct hc_vm *vm);

struct hc_op;

size_t hc_vm_emit(struct hc_vm *vm,
		  const struct hc_op *op,
		  const void *data);

void hc_vm_eval(struct hc_vm *vm, size_t start_pc, size_t end_pc);

extern const struct hc_type HC_VM_FUN;
typedef void (*hc_vm_fun_t)(struct hc_vm *, struct hc_sloc);

typedef uint8_t *(*hc_op_eval_t)(struct hc_vm *, uint8_t *);

struct hc_op {
  const char *name;

  size_t align;
  size_t size;

  hc_op_eval_t eval;
  void (*deinit)(uint8_t *);
};

struct hc_call_op {
  hc_vm_fun_t target;
  struct hc_sloc sloc;
};

extern const struct hc_op HC_CALL;

struct hc_push_op {
  struct hc_value value;
};

extern const struct hc_op HC_PUSH;

#endif
