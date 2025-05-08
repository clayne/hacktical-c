## Domain Specific Languages
A domain specific language, or DSL, is a tiny embedded language designed for solving a certain kind of problems. Once you start looking for them, they are everywhere; printf formats or regular expressions are two obvious examples.

In this chapter we're going to build a template engine that enables splicing the result evaluated expressions into strings. To make the process more interesting, and the result more useful; we're going to design our engine much the same way as an interpreter for a general purpose programming language.