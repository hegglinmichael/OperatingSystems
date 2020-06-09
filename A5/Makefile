
CC= gcc
CFLAGS = -Wall

all: minls minget

minls: helper user safety info my_minls.c my_minls.h 
	$(CC) $(CFLAGS) -o minls helper.o user_input.o file_safety.o partition_info.o my_minls.c 

minget: helper user safety info my_minget.c my_minget.h
	$(CC) $(CFLAGS) -o minget helper.o user_input.o file_safety.o partition_info.o my_minget.c 

helper: helper.c helper.h
	$(CC) $(CFLAGS) -c -o helper.o helper.c

user: user_input.c user_input.h
	$(CC) $(CFLAGS) -c -o user_input.o user_input.c

safety: file_safety.c file_safety.h
	$(CC) $(CFLAGS) -c -o file_safety.o file_safety.c

info: partition_info.c partition_info.h
	$(CC) $(CFLAGS) -c -o partition_info.o partition_info.c

clean:
	rm -fr *.o minls minget



