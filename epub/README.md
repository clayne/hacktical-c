# Hacktical C - epub

This directory contains utilities for generating an ebook version of this book in EPUB format.

It collects chapter contents from each chapter's README and supplemental code source files.

## Dependencies

* python
    - minimum: v3.9
    - recommended: v3.13
* [pandoc](https://pandoc.org)

## Building

`python ./build_ebook.py`

The build utilities are designed to be runnable with no arguments and default options, but also support a number of overrides for experimentation and other non-default uses.

Use `python ./build_ebook.py -h` to get a list of these additional configuration options and overrides.
