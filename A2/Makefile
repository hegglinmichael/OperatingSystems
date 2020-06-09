
CC= gcc
CFLAGS= -g -Wall -fpic

liblwp.so: lwp.h fp.h thread_ahead.o
	$(CC) $(CFLAGS) -shared -o $@ thread_ahead.o magic64.o

thread_ahead.o: thread_ahead.h thread_ahead.c magic64.S
	$(CC) $(CFLAGS) -c thread_ahead.c magic64.S

clean:
	rm -fr *.o *.so

# ~pn-cs453/demos/tryAsgn2
