CC=gcc
CFLAGS=-g -I.
PARTS=build/list.o build/task.o

build/test: main.c $(PARTS) 
	$(CC) $(CFLAGS) $^ -o build/test

build/list.o:
	$(MAKE) -C list

build/task.o:
	$(MAKE) -C task

clean:
	rm build/*
