export CC=ccache gcc
export CFLAGS=-g -Wall -I.
PARTS=build/fix.o build/list.o build/task.o build/vector.o

build/test: clean tests.c $(PARTS) 
	$(CC) $(CFLAGS) tests.c $(PARTS) -o build/test
	valgrind build/test

build/fix.o:
	$(MAKE) -C fix

build/list.o:
	$(MAKE) -C list

build/task.o:
	$(MAKE) -C task

build/vector.o:
	$(MAKE) -C vector

clean:
	rm -f build/*
