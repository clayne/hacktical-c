export CC=ccache gcc
export CFLAGS=-g -O0 -Wall -I.
PARTS=build/fix.o build/list.o build/task.o build/time.o build/vector.o

build/test: clean tests.c $(PARTS) 
	$(CC) $(CFLAGS) tests.c $(PARTS) -o build/test
	valgrind build/test

build/benchmark: clean benchmarks.c $(PARTS) 
	$(CC) $(CFLAGS) benchmarks.c $(PARTS) -o build/benchmark
	build/benchmark

build/fix.o:
	$(MAKE) -C fix

build/list.o:
	$(MAKE) -C list

build/task.o:
	$(MAKE) -C task

build/time.o:
	$(MAKE) -C time

build/vector.o:
	$(MAKE) -C vector

clean:
	rm -f build/*
