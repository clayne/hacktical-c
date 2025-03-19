export CC=ccache gcc
export CFLAGS=-g -Wall -I.
PARTS=build/list.o build/task.o build/vector.o

build/test: clean main.c $(PARTS) 
	$(CC) $(CFLAGS) main.c $(PARTS) -o build/test
	valgrind build/test

build/list.o:
	$(MAKE) -C list

build/task.o:
	$(MAKE) -C task

build/vector.o:
	$(MAKE) -C vector

clean:
	rm -f build/*
