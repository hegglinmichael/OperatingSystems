
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "partition_info.h"
#include "file_safety.h"

struct partition t_part;
struct superblock t_super;
uint32_t t_part_lFirst = 0;
uint32_t t_part_size = 0;
uint32_t sb_offset;

/**
 *  Description:
 *    Reads the partition to check the offset
 *    and if the partition is part of a minix system
 **/
void read_partition_info(FILE * system_ptr, uint32_t offset, int partition)
{
  int total_offset = offset + PARTITION_INFO + (PART_SIZE * partition);
  my_fseek(system_ptr, total_offset, SEEK_SET, "read_partition_info()");
  my_fread(&t_part, READ_ONE, PART_SIZE, system_ptr, "read_partition_info()");
  t_part_lFirst = t_part.lFirst;
  t_part_size = t_part.size;

  if (t_part.type != MINIX_TYPE)
  {
    printf("Not a minix partition: exiting\n");
    exit(2);
  }
}

/**
 *  Description:
 *    Checks the validity of a partition
 **/
void validate_partition(FILE * system_ptr, uint32_t offset)
{
  uint8_t valid_one;
  uint8_t valid_two;

  my_fseek(system_ptr, OFFSET_510 + offset, SEEK_SET, "validate_partition()");
  my_fread(&valid_one, READ_ONE, READ_ONE, system_ptr, "validate_partition()");
  my_fread(&valid_two, READ_ONE, READ_ONE, system_ptr, "validate_partition()");

  if (valid_one != VALID_510 || valid_two != VALID_511)
  {
    printf("Bad magic number. exiting\n");
    printf("This doesn't look like a MINIX filesystem\n");
    exit(1);
  }
}

/**
 *  Description:
 *    This method reads in the superblock
 *    information for the rest of the program
 *    to go off of
 **/
void read_superblock(
  FILE * system_ptr, 
  int partition, 
  int subpartition,
  int verbose
)
{
  (partition == -1) ?
  (sb_offset = SUPER_OFFSET) :
  (sb_offset = SUPER_OFFSET + (SECTOR_SIZE * t_part_lFirst));

  if (subpartition != -1)
  {
    sb_offset -= SUPER_OFFSET;
    validate_partition(system_ptr, sb_offset);
    sb_offset += PART_SIZE * subpartition;
    read_partition_info(system_ptr, sb_offset, partition);
    sb_offset = SUPER_OFFSET + (SECTOR_SIZE * t_part_lFirst);
  }
  
  my_fseek(system_ptr, sb_offset, SEEK_SET, "read_superblock()");
  my_fread(&t_super, READ_ONE, SUPER_SIZE, system_ptr, "read_superblock()");

  if (t_super.magic != MINIX_MAGIC)
  {
    printf("Bad magic number. (0x%04x)\n", t_super.magic);
    printf("This doensn't look like a MINIX filesystem.\n");
    exit(1);
  }

  if (verbose) { verbose_superblock(t_super); }
}

/**
 *  Description:
 *    This method prints out the 
 *    super block verbosely
 **/
void verbose_superblock()
{
  int w = 6;
  int l = 10;

  printf("%u", t_super.ninodes);
  printf("\nSuperblock Contents:\n");
  printf("Stored Fields:\n");
  printf("  ninodes       %*d\n", w, t_super.ninodes);
  printf("  i_blocks      %*d\n", w, t_super.i_blocks);
  printf("  z_blocks      %*d\n", w, t_super.z_blocks);
  printf("  firstdata     %*d\n", w, t_super.firstdata);
  printf("  log_zone_size %*d ", w, t_super.log_zone_size);
  printf("(zone size: %d)\n", (t_super.blocksize << t_super.log_zone_size));
  printf("  max_file  %*u\n", l, t_super.max_file);
  printf("  magic     %#*x\n", l, t_super.magic);
  printf("  zones     %*d\n", l, t_super.zones);
  printf("  blocksize %*d\n", l, t_super.blocksize);
  printf("  subversion%*d\n\n", l, t_super.subversion);
}

/**
 *  Description:
 *    Bulk list of getter methods for the
 *    structures
 **/
struct superblock * get_super_struct() { return &t_super; }
struct partition * get_partition_struct() { return &t_part; }
int get_lFirst() { return t_part_lFirst; }
























