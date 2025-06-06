#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

#include "error/error.h"
#include "macro/macro.h"
#include "vm.h"

struct hc_sloc hc_sloc(const char *source, const int row, const int col) {
  struct hc_sloc s = {.source = {0}, .row = row, .col = col};
  assert(strlen(source) < sizeof(s.source));
  strcpy(s.source, source);
  return s;
}

const char *hc_sloc_string(struct hc_sloc *sloc) {
  snprintf(sloc->out, sizeof(sloc->out), "'%s'; row %d, column %d",
	  sloc->source, sloc->row, sloc->col);
  return sloc->out;
}


void hc_vm_init(struct hc_vm *vm, struct hc_malloc *malloc) {
  hc_vector_init(&vm->stack, malloc, sizeof(struct hc_value));
  hc_vector_init(&vm->ops, malloc, sizeof(const struct hc_op *));
  hc_vector_init(&vm->code, malloc, sizeof(hc_op_eval_t));
}

static size_t op_items(const struct hc_op *op,
		       uint8_t *p,
		       struct hc_vm *vm) {
  const size_t s = op->size + hc_align(p, op->align) - p;
  return ceil(s / (double)vm->code.item_size);
}

static void deinit_stack(struct hc_vm *vm) {
  hc_vector_do(&vm->stack, v) {
    hc_value_deinit(v);
  }

  hc_vector_deinit(&vm->stack);
}

static void deinit_ops(struct hc_vm *vm) {
  uint8_t *p = vm->code.start;
  
  hc_vector_do(&vm->ops, _op) {
    const struct hc_op *op = *(const struct hc_op **)_op;
    p += sizeof(hc_op_eval_t);

    if (op->deinit) {
      op->deinit(hc_align(p, op->align));
    }

    p += op_items(op, p, vm) * vm->code.item_size;
  }

  hc_vector_deinit(&vm->ops);
}

void hc_vm_deinit(struct hc_vm *vm) {  
  deinit_stack(vm);
  deinit_ops(vm);
  hc_vector_deinit(&vm->code);
}

struct hc_value *hc_vm_push(struct hc_vm *vm) {
  return hc_vector_push(&vm->stack);
}

struct hc_value *hc_vm_peek(struct hc_vm *vm) {
  return hc_vector_peek(&vm->stack);
}

struct hc_value *hc_vm_pop(struct hc_vm *vm) {
  return hc_vector_pop(&vm->stack);
}

size_t hc_vm_emit(struct hc_vm *vm,
		  const struct hc_op *op,
		  const void *data) {
  *(const struct hc_op **)hc_vector_push(&vm->ops) = op;
  const size_t pc = vm->code.length;
  *(hc_op_eval_t *)hc_vector_push(&vm->code) = op->eval;
  
  uint8_t *const p = hc_vector_insert(&vm->code,
				      vm->code.length,
				      op_items(op, vm->code.end, vm));
  
  memcpy(hc_align(p, op->align), data, op->size);
  return pc;
}

void hc_vm_eval(struct hc_vm *vm,
		const size_t start_pc,
		const size_t end_pc) {
  const uint8_t *const ep = (end_pc == -1)
    ? vm->code.end
    : hc_vector_get(&vm->code, end_pc);

  for (uint8_t *p = hc_vector_get(&vm->code, start_pc);
       p != ep;
       p = (*(hc_op_eval_t *)p)(vm, p + vm->code.item_size));
}

static void fun_print(const struct hc_value *v, struct hc_stream *out) {
  hc_printf(out, "%p", v->as_other);
}

const struct hc_type HC_VM_FUN = {
  .name = "VM/Fun",
  .copy = NULL,
  .print = fun_print
};

static uint8_t *call_eval(struct hc_vm *vm, uint8_t *data) {
  struct hc_call_op *op = (void *)hc_align(data, alignof(struct hc_call_op));
  op->target(vm, op->sloc);
  return (uint8_t *)op + sizeof(struct hc_call_op);
}

const struct hc_op HC_CALL = (struct hc_op){
  .name = "call",
  .align = alignof(struct hc_call_op),
  .size = sizeof(struct hc_call_op),
  .eval = call_eval,
  .deinit = NULL
};

static void push_deinit(uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_value_deinit(&op->value);
}

static uint8_t *push_eval(struct hc_vm *vm, uint8_t *data) {
  struct hc_push_op *op = (void *)hc_align(data, alignof(struct hc_push_op));
  hc_value_copy(hc_vm_push(vm), &op->value);
  return (uint8_t *)op + sizeof(struct hc_push_op);
}

const struct hc_op HC_PUSH = (struct hc_op){
  .name = "push",
  .align = alignof(struct hc_push_op),
  .size = sizeof(struct hc_push_op),
  .eval = push_eval,
  .deinit = push_deinit
};
