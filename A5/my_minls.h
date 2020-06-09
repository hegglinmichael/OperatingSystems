
#ifndef MY_MINLS_H
#define MY_MINLS_H

void start_reading(char * filename, int partition);
void parse_input(int argc, char ** argv);
void parse_partition_info(FILE * system_ptr, int partition);

#endif
