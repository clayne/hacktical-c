## Exceptions
How to best deal with errors is something we're still very much learning in software development. It is however very clear to me that there will be no one true error handling strategy to rule them all. Sometimes returning error codes is the right thing to do, other times exceptions are a much better solution.

Standard C lacks exception support, so we're going to roll our own using `setjmp` and `longjmp`. `setjmp` saves the current execution context into a variable of type `struct jmp_buf`, and `longjmp` restores it.