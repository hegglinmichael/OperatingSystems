
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "user_input.h"

char * filename;
char * src_file_path = "root";
char * dest_file_path;

int t_partition = -1;
int t_subpartition = -1;
int t_verbose = 0;
/*
--------------------------------------------------------------------------------
*/
/**
 *  Description:
 *    Runs a validity check on the arguments inputed
 **/
void validity_check(int argc, char ** argv, int prog)
{
  switch (argc)
  {
    case 1:
      print_usage(prog);
      exit(1);
      break;

    case 2:
      filename = argv[1];
      break;

    default:
      sort_options(argc, argv, prog);
      break;
  }
}

/**
 *  Description:
 *    This method looks for options like -v and 
 *    correctly checks if they were given
 **/
void sort_options(int argc, char ** argv, int prog)
{
  char option;

  while((option = getopt(argc, argv, "p:s:hv")) != -1)
  {
    switch(option)
    {
      case 'p':
        t_partition = atoi(optarg);
        if (t_partition < 0 || t_partition > 4) { exit(1); }
        break;

      case 's':
        t_subpartition = atoi(optarg);
        break;

      case 'h':
        print_usage(prog);
        break;

      case 'v':
        t_verbose = 1;
        break;

      default:
        printf("invalid option character\n");
        break;
    }
  }

  if (t_subpartition != -1 && t_partition == -1) {
    printf("You cannot have a subpartition without a partition\n");
    exit(1);
  }

  if (optind < argc) { 
    filename = argv[optind++]; 
  } else { 
    printf("file not given\n"); 
  }

  if (optind < argc) { 
    src_file_path = argv[optind++]; 
  } else if (optind >= argc && GET_FLAG == prog) {
    print_usage(prog);
    exit(1);
  } 

  (optind < argc) ? (dest_file_path = argv[optind]) : (dest_file_path = NULL);
}

/**
 *  Description:
 *    prints out possible usage
 **/
void print_usage(int prog)
{
  (prog == LS_FLAG) ?
  (printf("usage: minls  [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n")) :
  (printf("usage: minls  [ -v ] [ -p num [ -s num ] ] \
    imagefile srcpath [ dstpath ]\n"));

  printf("Options:\n");
  printf("-p  part    \
          --- select partition for filesystem (default: none)\n");
  printf("-s  sub     \
          --- select subpartition for filesystem (default: none)\n");
  printf("-h  help    \
          --- print usage information and exit\n");
  printf("-v  verbose \
          --- increase verbosity level\n");
}

/**
 *  Description:
 *    Below are getter methods for the 
 *    parsed information
 **/
char * get_filename() { return filename; }
char * get_src_path() { return src_file_path; }
char * get_dst_path() { return dest_file_path; }
int get_partition() { return t_partition; }
int get_subpartition() {return t_subpartition; }
int get_verbose() { return t_verbose; } 

