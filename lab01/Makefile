# Makefile for lab01
# written by Michael Hegglin - 04/06/2020

CC = gcc
CFLAGS = -g -Wall
OBJFILES = pipeit.o
TARGET = pipeit

all: pipeit

pipeit: $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

clean:
	rm -f $(TARGET) *.o

