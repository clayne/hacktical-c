#include <assert.h>

#include "vm.h"
#include "malloc1/malloc1.h"

static void emit_tests() {
  struct hc_vm vm;
  hc_vm_init(&vm, &hc_malloc_default);
  hc_defer(hc_vm_deinit(&vm));
  struct hc_push_op op;
  hc_value_init(&op.value, &HC_FIX)->as_fix = hc_fix(0, 42);
  hc_vm_emit(&vm, &HC_PUSH, &op);
  hc_vm_eval(&vm, 0, -1);
  assert(vm.stack.length == 1);
  assert(hc_vm_pop(&vm)->as_fix == op.value.as_fix);
}

void vm_tests() {
  emit_tests();
}
