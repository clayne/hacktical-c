# Hacktical C
A practical hacker's guide to the C programming language.

*In memory of [Dennis Ritchie](https://en.wikipedia.org/wiki/Dennis_Ritchie),
one of the greatest hackers this world has known.*

## About the book
This book assumes basic programming knowledge. We're not going to spend a lot of time and space on explaining basic features, except where they behave differently in important ways compared to other mainstream languages. Instead we're going to focus on practical techniques for making the most out of the power and flexibility C offers.

## About the author
You could say that there are two kinds of programmers, with very different motivations; academics and hackers. I've always identified as a hacker. I like solving tricky problems, and I prefer using powerful tools that don't get in my way. To me; software is all about practical application, about making a change in the real world.

I've been writing code for fun on a mostly daily basis since I got a Commodore 64 for Christmas in 1985, professionally in different roles/companies since 1998.

I started out with Basic on the Commodore 64, went on to learn Assembler on an Amiga 500, Pascal on PC, followed by C++.

For a long time, I didn't care much about C at all, it looked very primitive compared to other languages. But gradually over time, I learned that the worst enemy in software is complexity, and started taking C more seriously.

Since then I've written a ton of C; and along the way I've picked up many interesting, lesser known techniques that helped me make the most out of the language and appreciate it for its strengths.

## Why C?
The reason I believe C is and always will be important is that it stands in a class of its own as a mostly portable assembler language, offering similar levels of freedom.

C doesn't try to save you from making mistakes. It has very few opinions about your code and happily assumes that you know exactly what you're doing. Freedom with responsibility.

These days; many programmers will recommend choosing a stricter language, regardless of the problem being solved. Most of those programmers wouldn't trust themselves with the kind of freedom C offers, many haven't even bothered to learn the language properly.

Since most of the foundation of the digital revolution, including the Internet was built using C; it gets the blame for many problems that are more due to our immaturity in designing and building complicated software than about programming languages.

The truth is that any reasonably complicated software system created by humans will have bugs, regardless of what technology was used to create it. Using a stricter language helps with reducing some classes of bugs, at the cost of reduced flexibility in expressing a solution and increased effort creating the software.

Programmers like to say that you should pick 'the right tool for the job'; what many fail to grasp is that the only people who have the capability to decide which tools are right, are the people creating the software. Much effort has been wasted on arguing and bullying programmers into picking tools other people prefer.

## Building
The makefile requires `gcc`, `ccache` and `valgrind` to do its thing.

```
git clone https://github.com/codr7/hacktical-c.git
cd hacktical-c
mkdir build
make
```

## Benchmarks
Some chapters come with benchmarks, `make build/benchmark` builds and runs all of them.

## Extensions
The code in this book uses several GNU extensions that are not yet in the C standard. Cleanup attributes, multi-line expressions and nested functions specifically.

Some developers avoid extensions like the plague, some are happy to use them for everything and anything. I fall somewhere in the middle of the spectrum; comfortable with using extensions when there are no good standard alternatives, especially if they're supported by both `gcc` and `clang`. All of the extensions used in this book except nested functions (which is currently only supported by `gcc`) fall in that category.

I can think of one feature, `hc_defer()`, which would currently be absolutely impossible to do without extensions. In other cases, alternative solutions are simply less convenient.

## Donations
If you would like to help make this book happen, all contributions are welcome. The repository is set up for sponsoring via GitHub and I've launched a [gofundme campaign](https://gofund.me/46f9b954).

## Chapters
The structure of this book is designed to form a natural progression, where later chapters build upon concepts that have already been introduced.

- [Macros](https://github.com/codr7/hacktical-c/tree/main/macro)
- [Fixed-Point Arithmetic](https://github.com/codr7/hacktical-c/tree/main/fix)
- [Intrusive Doubly Linked Lists](https://github.com/codr7/hacktical-c/tree/main/list)
- [Lightweight Concurrent Tasks](https://github.com/codr7/hacktical-c/tree/main/task)
- [Vectors](https://github.com/codr7/hacktical-c/tree/main/vector)