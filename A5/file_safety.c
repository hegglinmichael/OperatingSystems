
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

/**
 *  Description:
 *    This method does a seek for the user
 *    but also does error checking
 **/
void my_fseek(FILE * system_ptr, int offset, int where, char * called_from)
{
  if (fseek(system_ptr, offset, where) != 0)
  {
    printf("%s: exiting\n", called_from);
    exit(1);
  }
}

/**
 *  Description:
 *    This method does a read for the user
 *    but also does error checking
 **/
void my_fread(
  void * ptr, 
  int size, 
  int how_many, 
  FILE * system_ptr, 
  char * where
)
{
  int t;

  t = fread(ptr, size, how_many, system_ptr);
  if (t != how_many * size)
  {
    printf("%s: exiting\n", where);
    exit(1);
  }
}

/**
 *  Description:
 *    This method does a write for the user
 *    but also does error checking
 **/
void my_fwrite(
  void * ptr, 
  size_t size, 
  size_t how_many, 
  char character, 
  char * from
)
{
  int w;

  w = fwrite(&character, size, how_many, ptr);
  if (w != how_many)
  {
    printf("%s: exiting(%d)\n", from, w);
    exit(1);
  }
}






