## Exceptions
How to best deal with errors is something we're still very much learning in software development. It is however very clear to me that there will be no one true error handling strategy to rule them all. Sometimes returning error codes is the right thing to do, other times exceptions are a much better solution.

C lacks native exception support, so we're going to roll our own using `setjmp` and `longjmp`. `setjmp` saves the current execution context into a variable of type `struct jmp_buf` and returns `0` the first time, and `longjmp` restores it which causes a second (non-`0`) return from `setjmp`.

```C
#define _hc_catch(_e, _f, h)					
  jmp_buf _e;							
  bool _f = true;						
  if (setjmp(_e)) {						
    h(hc_error);						
  } else							
    for (hc_catch_push(_e); _f; _f = false, hc_catch_pop())	

#define hc_catch(h)				
  _hc_catch(hc_unique(env), hc_unique(flag), h)
```

We'll use a thread local `struct hc_vector` to store handlers. Note that the only way to deallocate it is by manually calling `hc_errors_deinit()` at the end of  every thread.

```C
static struct hc_vector *handlers() {
  static bool init = true;
  static __thread struct hc_vector handlers;

  if (init) {
    hc_vector_init(&handlers, sizeof(jmp_buf));
    init = false;
  }
  
  return &handlers;
}

void hc_catch_push(jmp_buf h) {
  memcpy((jmp_buf *)hc_vector_push(handlers()), h, sizeof(jmp_buf));
}

void hc_catch_pop() {
  hc_vector_pop(handlers());
}

void hc_errors_deinit() {
  hc_vector_deinit(handlers());
}
```

`hc_throw()` takes a code and a `printf`-compatable format and argument list.

```C
#define hc_throw(c, m, ...) {					\
      struct hc_error *e =					\
	hc_error_new((c), "Failure %d in '%s', line %d:\n" m,	\
		     (c), __FILE__, __LINE__, ##__VA_ARGS__);	\
      _hc_throw(e);						\
  } do while(0)
```

Example:
```C
void on_catch(struct hc_error *e) {
  printf(e->message);
  free(e);
}
  
hc_catch(on_catch) {
  hc_throw(12345, "here %d", 42);
}
```

Output:
```
Failure 12345 in 'error/tests.c', line 14:
here 42
```

Errors are defined as dynamically sized structs.

```C
struct hc_error {
  int code;
  char message[];
};
```