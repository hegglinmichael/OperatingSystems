/**
 *  Author: Michael Hegglin
 *  Date: 6-5-2020
 *  
 *  Description:
 *    Program to read a minix filesystem
 **/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "my_minls.h"
#include "helper.h"
#include "user_input.h"
#include "partition_info.h"

int main(int argc, char ** argv)
{
  parse_input(argc, argv);
  start_reading( get_filename(), get_partition() );
  return 0;
}

/**
 *  Description
 *    Checks user inputs
 *    Upon sucess, sets variables in the 
 *    helper script for other methods
 **/
void parse_input(int argc, char ** argv)
{
  validity_check(argc, argv, LS_FLAG);

  set_subpartition( get_subpartition() );
  set_partition( get_partition() );
  set_src_path( get_src_path() );
  set_dst_path( get_dst_path() );
  set_filename( get_filename() );
  set_verbose( get_verbose() );
}

/**
 *  Description:
 *    This method acts like a control panel 
 *    for the reading of partition information
 **/
void parse_partition_info(FILE * system_ptr, int partition)
{
  if (partition != -1)
  {
    validate_partition(system_ptr, 0);
    read_partition_info(system_ptr, 0, partition);
  }

  read_superblock(system_ptr, partition, get_subpartition(), get_verbose());
  set_super_struct( get_super_struct() );
  set_partition_struct( get_partition_struct() );
  set_part_lFirst( get_lFirst() );
}

/**
 *  Description:
 *    Handles the actual reading of the 
 *    image
 **/
void start_reading(char * filename, int partition)
{
  FILE * system_ptr;
  system_ptr = fopen(filename, "r");

  if (system_ptr == NULL)
  {
    printf("fopen failed: exiting\n");
    exit(2);
  }

  parse_partition_info(system_ptr, partition);
  read_root_inode(system_ptr);
  files_from_inode(system_ptr, READING);
  fclose(system_ptr);
}



























































































