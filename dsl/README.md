## Domain Specific Languages
A domain specific language (DSL) is a tiny embedded language specialized at solving a specific category of problems. Once you start looking for them, they're everywhere; `printf` formats and regular expressions are two obvious examples.

We're going to build a template engine that allows splicing runtime evaluated expressions into strings. Templates are compiled into operations for the [virtual machine](https://github.com/codr7/hacktical-c/tree/main/vm) we built earlier.