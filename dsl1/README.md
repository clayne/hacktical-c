## Domain Specific Languages
A domain specific language, or DSL, is a tiny embedded language specialized at solving a specific category of problems. Once you start looking for them, they are everywhere; printf formats or regular expressions are two obvious examples.

In this chapter we're going to build a template engine that allows splicing runtime-evaluated expressions into strings. To make the process more interesting, we're going to design our engine much the same way as you could design an interpreter for a general purpose programming language.