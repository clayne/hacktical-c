## Exceptions
How to best deal with errors is something we're still very much figuring out in software. But it is clear to me that there will always be a place for different strategies depending on needs.

The main issue with exceptions is that they transfer control in difficult to predict ways, and the main advantage is that they allow dealing with errors at the correct level without manually propagating them.

C lacks native exception support, so we're going to roll our own using `setjmp` and `longjmp`. `setjmp` saves the current execution context into a variable of type `jmp_buf` and returns `0` the first time, and `longjmp` restores it which causes a second return from `setjmp` with the specified value.

Example:
```C
void on_catch(struct hc_error *e) {
  printf(e->message);
  free(e);
}
  
hc_catch(on_catch) {
  hc_throw("E123 Going %s", "Down!");
}
```

Output:
```
Error in 'error/tests.c', line 14:
E123 Going Down!
```

We use a `for`-loop to push/pop handlers around the catch body, a neat trick for whenever you need to do something after a user defined block of code in macro context.

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

A static thread-local `struct hc_vector` stores currently active handlers. Note that the only way to deinitialize it is by manually calling `hc_errors_deinit()` at the end of every thread.

```C
static struct hc_vector *handlers() {
  static bool init = true;
  static __thread struct hc_vector handlers;

  if (init) {
    hc_vector_init(&handlers, &hc_malloc_default, sizeof(jmp_buf));
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

`hc_throw()` takes a `printf`-compatable format and argument list. We're taking advantage of the fact that adjacent string literals are automagically concatenated by the compiler to add context.

```C
#define hc_throw(m, ...) {						
  struct hc_error *e = hc_error_new("Failure in '%s', line %d:\n" m "\n",
                                    __FILE__, __LINE__, ##__VA_ARGS__);
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

Errors are defined as follows.

```C
struct hc_error {
  char *message[];
};
```

### Vararg Functions
From this point on we're going to be defining a fair amount of vararg functions. Like many other features, they require a tiny bit more discipline in C compared to most other languages.

`va_start` initializes a vararg, it expects the final argument in second position. A `va_list` can only be consumed once, since each call to `va_arg()` modifies the list so that the next call returns the next argument. Due to this it's not possible to retrieve an argument more than once. In the following example we use `va_copy` to get around the problem by duplicating the list.

Back in error land; calling `vsnprintf` with a `NULL` argument returns the message length, which we need to allocate memory for the error.

```C
struct hc_error *hc_error_new(const char *message, ...) {
  struct hc_error *e = malloc(sizeof(struct hc_error));
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
  e->message = malloc(len);
  vsnprintf(e->message, len, message, args);
  va_end(args);
  return e;
}
```

### Cleaning Up
Keep in mind that any code that interacts with exceptions has to use `hc_defer()` or similar functionality to clean up properly.