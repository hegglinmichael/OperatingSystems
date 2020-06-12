# Makefile for lab01
# written by Michael Hegglin - 04/06/2020

CC= gcc
CFLAGS= -Wall -g -fpic

# cmd line arguments with make -----------------------

all: malloc intel-all
malloc: libmalloc.a libmalloc.so
intel-all: lib/libmalloc.so lib64/libmalloc.so 

# Library for static linking -------------------------

big_bird.o: big_bird.c big_bird.h
	$(CC) $(CFLAGS) -m64 -c -o big_bird.o big_bird.c

libmalloc.a: big_bird.o
	ar rc $@ big_bird.o
	ranlib $@

# Library for dynamic linking ------------------------

libmalloc.so: big_bird_d.o
	$(CC) $(CFLAGS) -shared -o $@ big_bird_d.o

big_bird_d.o: big_bird.c big_bird.h
	$(CC) $(CFLAGS) -m64 -c -o big_bird_d.o big_bird.c


# # Testing library cmds--------------------------------

lib/libmalloc.so: lib big_bird32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ big_bird32.o

lib64/libmalloc.so: lib64 big_bird64.o
	$(CC) $(CFLAGS) -shared -o $@ big_bird64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

big_bird32.o: big_bird.c
	$(CC) $(CFLAGS) -m32 -c -o big_bird32.o big_bird.c

big_bird64.o: big_bird.c
	$(CC) $(CFLAGS) -m64 -c -o big_bird64.o big_bird.c

# Clean up cmds----------------------------------------

clean:
	rm -fr lib64 lib *.o *.a *.so *.gch

# cmds for running test files -------------------------

# LD_LIBRARY_PATH=//home/mhegglin/453/A1/lib64/
# export LD_LIBRARY_PATH
# unset LD_LIBRARY_PATH

# DEBUG_MALLOC=
# export DEBUG_MALLOC
# unset DEBUG_MALLOC
