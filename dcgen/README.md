## Dynamic Compilation

*Never put off till run-time what you can do at compile-time.*

~ D. Gries

I would go one step further: don't evaluate what you can compile. In languages like Lisp this is a lot easier due to its built-in support for code generation and dynamic compilation.

To get access to these features in C, we'll have to cast a bunch of non-trivial Unix spells and sacrifice some measure of portability in the process.

It's not that these features aren't available on other platforms; rather that they're implemented in slightly different ways, using different names.