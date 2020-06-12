
#ifndef BIG_BIRD_H
#define BIG_BIRD_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define CHUNK_SIZE 64000
/* padding of 10% for new nodes */
#define SPACE_FOR_NEW_NODE 6400
#define PRE_ALLOCATE_MEMORY 1
#define HEADER_SIZE 32
#define FREE 0
#define NOT_FREE 1

struct chunk_hdr
{
	size_t size;
	int free_flag;
	struct chunk_hdr * prev;
	struct chunk_hdr * next;
};

void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *malloc(size_t size);
void free(void *ptr);

void debugging(char * debug_out);
void * get_more_memory(int multiple);

#endif
