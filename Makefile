export CC=ccache gcc
export CFLAGS=-g -O0 -flto -Wall -fsanitize=undefined -I. -lm
export LDFLAGS=

CHAPTERS=build/chrono.o build/dsl1.o build/dynamic.o build/error.o build/fix.o build/list.o build/malloc1.o build/malloc2.o build/set.o build/slog.o build/stream1.o build/task.o build/vector.o

all: clean build/test build/benchmark

build/test: tests.c $(CHAPTERS) 
	$(CC) $(CFLAGS) tests.c $(CHAPTERS) -o build/test
	valgrind build/test

build/benchmark: benchmarks.c $(CHAPTERS) 
	$(CC) $(CFLAGS) benchmarks.c $(CHAPTERS) -o build/benchmark
	build/benchmark

build/dsl1.o:
	$(MAKE) -C dsl1

build/dynamic.o:
	$(MAKE) -C dynamic

build/error.o:
	$(MAKE) -C error

build/fix.o:
	$(MAKE) -C fix

build/list.o:
	$(MAKE) -C list

build/malloc1.o:
	$(MAKE) -C malloc1

build/malloc2.o:
	$(MAKE) -C malloc2

build/set.o:
	$(MAKE) -C set

build/slog.o:
	$(MAKE) -C slog

build/stream1.o:
	$(MAKE) -C stream1

build/task.o:
	$(MAKE) -C task

build/chrono.o:
	$(MAKE) -C chrono

build/vector.o:
	$(MAKE) -C vector

clean:
	rm -f build/*
