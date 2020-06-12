
CC=gcc
CFLAGS= -Wall -ansi -pedantic

hello: Hello.c
	@ $(CC) $(CFLAGS) -o hello hello.c

test: hello
	@ ./hello

clean:
	rm -fr hello
