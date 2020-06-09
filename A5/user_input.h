
#ifndef USER_INPUT_H
#define USER_INPUT_H

#define LS_FLAG 1
#define GET_FLAG 0

void validity_check(int argc, char ** argv, int prog);
void sort_options(int argc, char ** argv, int prog);
void print_usage(int prog);

char * get_filename();
char * get_src_path();
char * get_dst_path();
int get_partition();
int get_subpartition();
int get_verbose();

#endif
