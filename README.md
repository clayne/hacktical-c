# Hacktical C
A practical hacker's guide to the C programming language.

*In memory of Dennis Ritchie,
one of the greatest hackers this world has known.*

## About the book
This book assumes basic programming knowledge. We're not going to spend a lot of time and space on basic features, except where they behave differently in important ways compared to other mainstream languages. Instead we're going to focus on practical techniques for making the most out of the power and flexibility that C offers.

## About the author
You could say that there are two kinds of programmers, academics and hackers. I've always identified as a hacker. I like solving tricky problems, and I prefer using powerful tools that don't get in my way. To me; software is all about practical applications, about making a change in the real world.

I've been writing code for fun on a mostly daily basis since I got a Commodore 64 for Christmas in 1985, professionally in different roles/companies since 1998.

I started out with Basic on the Commodore 64, went on to learn Assembler on an Amiga 500, Pascal on PC, followed by C++. For a long time, I didn't care much about C at all, it looked very primitive compared to other languages. But gradually over time, I learned that the worst enemy in software is complexity, and started taking C more seriously. Since then I've written a lot of C and along the way I've picked up many interesting, lesser known techniques that helped me make the most out of the language and appreciate it for its strengths.

## Why (not) C?
The reason I believe C is and always will be important is that it stands in a class of its own as a mostly portable assembler language. C doesn't try to save you from making mistakes. It has very few opinions about your code and happily assumes that you know exactly what you're doing. Freedom with responsibility.

These days many programmers will recommend choosing a stricter language, regardless of the problem is being solved. Most of those programmers wouldn't even trust themselves with the kind of freedom C affords, many of them haven't even bothered to learn C properly.

Since most of the foundation of the digital revolution, including the Internet was built using C; it gets the blame for many problems that are more due to our inability and immaturity in designing and building complicated software systems than about programming languages.

The truth is that any reasonably complicated software system created by humans will have bugs, regardless of what technology was used to create it. Using a stricter language helps with reducing some classes of bugs, at the cost of reduced flexibility in expressing a solution and increased effort creating the software.

Programmers like to say that you should pick 'the right tool for the job'; what many fail to grasp is that the only people who have the right and capability to decide which tools are the right ones, are the people creating the software. Much effort has been wasted on arguing and bullying programmers into picking the tools other people prefer.

## Building the code

```
git clone https://github.com/codr7/hacktical-c.git
cd hacktical-c
mkdir build
make
build/test
```

## Chapters

- [Intrusive Doubly Linked Lists](https://github.com/codr7/hacktical-c/tree/main/list)