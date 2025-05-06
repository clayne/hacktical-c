## Macros
Before we move on, macros deserve a closer look. Many developers never get past the initial stage of utilizing their most basic features; which is a shame, because macros enable many powerful techniques.

It's often useful to pass arguments through an additional layer of macros to force expansion. `hc_id()` is used to concatenate identifiers and utilizes this technique to expand arguments before concatenating using `##`.

```C
#define _hc_id(x, y)
  x ## y

#define hc_id(x, y)
  _hc_id(x, y)
```

Example:
```C
int hc_id(foo, bar) = 42;
assert(foobar == 42);
```

`hc_unique()` generates unique identifiers with specified prefix. It uses a system macro named `__COUNTER__` for the unique part, this macro increases its value on each expansion.

```C
#define hc_unique(x)
  hc_id(x, __COUNTER__)
```

Since it doesn't make much sense to generate unique identifiers that are never used, this macro is mostly used to generate arguments for macros where the generated id can be reused.

`hc_defer()` is used to register scoped destructors, it uses `hc_unique()` to generate a name for the temporary variable to which the `__cleanup__` attribute is applied, as well as for the destructor trampoline. Registered destructors are executed in reverse order. `__VA__ARGS__` represents the argument list in macros that take a variable number of arguments. It is sometimes used as `##__VA_ARGS__`, which removes the preceding `,` when the argument list is empty.

```C
#define _hc_defer(_d, _v, ...)			
  void _d(int *) { __VA_ARGS__; }		
  int _v __attribute__ ((__cleanup__(_d)))

#define hc_defer(...)
  _hc_defer(hc_unique(defer_d), hc_unique(defer_v), ##__VA_ARGS__)
```
Example:
```C
int foo = 0;

{
  hc_defer(assert(foo++ == 1));
  hc_defer(assert(foo++ == 0));
}

assert(foo == 2);
```

A common foot gun that is best avoided in macro context is double expansion of macro arguments. What it means is that macros simply paste arguments as is; if the argument has an effect, the effect will be repeated for every expansion by default. That's the reason why we're assigning temporaries whenever we need to refer to a macro argument multiple times within the body.

`__auto_type` allows inferring types from macro arguments, this may be used to define generic macros such as `hc_min()`. Enclosing the macro body in `({`/`})` enables multi-line expressions.

```C
#define hc_min(x, y) ({				
      __auto_type _x = x;			
      __auto_type _y = y;			
      _x < _y ? _x : _y;			
    })						
```

Example:
```C
assert(hc_min(7, 42) == 7);
```

Passing arrays as function arguments is slightly problematic in C due to the fact that the array decays into a pointer, which means the length is lost in the call. `hc_array` takes a type and a prefix, and defines the array and its length as `[prefix]_a`/`[prefix]_n`.

```C
#define _hc_array(t, a, n, ...)			
  t a[] = { __VA_ARGS__ };			
  const size_t n = sizeof(a) / sizeof(t)		

#define hc_array(t, p, ...)					
  _hc_array(t, hc_id(p, _a), hc_id(p, _n), ##__VA_ARGS__)
```