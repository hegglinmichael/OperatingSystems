
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) 
{
	char *t;
	char *s;

	s = malloc(10000);
	printf("Original Addr: %p\n", s);
	t = malloc(20000);
	s = realloc(s, 30000);
	printf("New Addr: %p\n", s);

	puts(s);
	free(s);
	return 0;
}
