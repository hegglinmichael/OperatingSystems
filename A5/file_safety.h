

#ifndef FILE_SAFETY_H
#define FILE_SAFETY_H

void my_fseek(FILE * system_ptr, int offset, int where, char * called_from);

void my_fread(
  void * ptr, 
  int size, 
  int how_many, 
  FILE * system_ptr, 
  char * where
);

void my_fwrite(
  void * ptr, 
  size_t size, 
  size_t how_many, 
  char character, 
  char * from
);

#endif

