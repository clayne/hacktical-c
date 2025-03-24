## Dynamic Compilation

Never put off till run-time what you can do at compile-time.

~ D. Gries

I would go one step further: don't evaluate what you can compile. In languages like Lisp this is made a lot easier by its built in support for dynamic compilation. To get access to similar features in C, we'll have to cast some non-trivial Unix spells and sacrifice portability. It's not that these features aren't available on other platforms; rather that they're implemented in slightly different ways, using different names.