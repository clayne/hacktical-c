CC=gcc
CFLAGS=

build/test: main.c build/list.o
	$(CC) $(CFLAGS) $^ -o build/test

build/list.o:
	$(MAKE) -C list

clean:
	rm build/*
