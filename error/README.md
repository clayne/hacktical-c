## Exceptions
How to best deal with errors is something we're still very much learning in software development. It is however very clear to me that there will be no one true error handling strategy to rule them all. Sometimes returning error codes is the right thing to do, other times exceptions make a much better solution.

C lacks native exception support, so we're going to roll our own using `setjmp` and `longjmp`. `setjmp` saves the current execution context into a variable of type `jmp_buf` and returns `0` the first time, and `longjmp` restores it which causes a second return from `setjmp` with the specified value.

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

We'll use a `for`-loop to push/pop handlers around the catch body, a neat trick for whenever you need to do something after a user defined block of code in macro context.

```C
#define _hc_catch(_e, _f, h)					
  jmp_buf _e;							
  bool _f = true;						
  if (setjmp(_e)) {						
    h(hc_error);						
  } else for (hc_catch_push(_e); _f; _f = false, hc_catch_pop())	

#define hc_catch(h)				
  _hc_catch(hc_unique(env), hc_unique(flag), h)
```

We'll use a static thread local `struct hc_vector` to store handlers. Note that the only way to deinitialize it is by manually calling `hc_errors_deinit()` at the end of every thread.

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

`hc_throw()` takes a code and a `printf`-compatable format and argument list. We're taking advantage of the fact that adjecent string literals are automagically concatenated by the compiler to add context to the message.

```C
#define hc_throw(c, m, ...) {						
      struct hc_error *e =					
	hc_error_new((c), "Failure %d in '%s', line %d:\n" m "\n",
		     (c), __FILE__, __LINE__, ##__VA_ARGS__);	
      _hc_throw(e);						
  } do while(0)

void _hc_throw(struct hc_error *e) {
  struct hc_vector *hs = handlers();

  if (!hs->length) {
    fputs(e->message, stderr);
    abort();
  }
  
  jmp_buf t;
  memcpy(t, *(jmp_buf *)hc_vector_peek(hs), sizeof(jmp_buf));
  hc_error = e;
  longjmp(t, 1);
}
```

Errors are defined as dynamically sized structs, it's up to the handler to free memory. We use `vsnprintf` with a `NULL` argument to get the message length before allocating memory for the error, this means that we have to copy the argument list since a `va_list` can only be comsumed once.

```C
struct hc_error {
  int code;
  char message[];
};

struct hc_error *hc_error_new(int code, const char *message, ...) {
  va_list args;
  va_start(args, message);
  
  va_list tmp_args;
  va_copy(tmp_args, args);
  int len = vsnprintf(NULL, 0, message, tmp_args);
  va_end(tmp_args);

  if (len < 0) {
    vfprintf(stderr, message, args);
    abort();
  }
  
  len++;
  struct hc_error *e = malloc(sizeof(struct hc_error) + len);
  e->code = code;
  vsnprintf(e->message, len, message, args);
  va_end(args);
  return e;
}
```