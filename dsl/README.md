## Domain Specific Languages
A domain specific language (DSL) is a tiny embedded language specialized at solving a specific category of problems. Once you start looking for them, they're everywhere; `printf` formats and regular expressions are two obvious examples.

We're going to build a template engine that allows splicing runtime evaluated expressions into strings. Templates are compiled into operations for the [virtual machine](https://github.com/codr7/hacktical-c/tree/main/vm) we built in previous chapter.

Example:
```C
struct hc_vm vm;
hc_dsl_init(&vm);
hc_defer(hc_vm_deinit(&vm));
struct hc_memory_stream out;
hc_memory_stream_init(&out, hc_malloc());
hc_defer(hc_stream_deinit(&out->stream));
vm.out = &out.stream;
hc_vm_setenv(&vm, "foo", &HC_STRING)->as_string = strdup("ghi");
hc_dsl_eval(&vm, "abc $(print foo) def");
assert(strcmp("abc ghi def", hc_memory_stream_string(&out)) == 0);
```

Our DSL is represented by a [vm](https://github.com/codr7/hacktical-c/tree/main/vm), configured with the functions we want to support.

```C
void hc_dsl_init(struct hc_vm *vm) {
  hc_vm_init(vm, &hc_malloc_default);
  hc_vm_setenv(vm, "print", &HC_VM_FUN)->as_other = lib_print;
}
```

To be continued...