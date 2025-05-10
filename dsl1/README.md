## Domain Specific Languages - Part 1
A domain specific language, or DSL, is a tiny embedded language specialized at solving a specific category of problems. Once you start looking for them, they are everywhere; printf formats or regular expressions are two obvious examples.

We're going to build a template engine that allows splicing runtime evaluated expressions into strings. To make the process more interesting, we'll design our engine much the same way as an interpreter for a general purpose programming language.

This chapter is all about the virtual machine which will run the programs our templates are eventually compiled into.

Continued in [Part 2}(https://github.com/codr7/hacktical-c/tree/main/dsl2).