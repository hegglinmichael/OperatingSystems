/**
  This is the worst spagetti code I've written since
  highschool.  Dear god 

  Remember me for the Asign 2 code Professor
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "helper.h"
#include "file_safety.h"
#include "user_input.h"

/*
--------------------------------------------------------------------------------
START MICS VARS
*/

int found = 0;
unsigned long written = 0;

/*
END MISC VARS
--------------------------------------------------------------------------------
STARTING PARSING METHOD VARIBALES
*/

char * filename;
char * src_file_path;
char * dest_file_path;

int verbose = 0;
int partition = -1;
int sub_partition = -1;

/*
END PARSING METHOD VARIBALES
--------------------------------------------------------------------------------
START SUPER BLOCK VARIABLES
*/

struct superblock * super;
struct partition * part;
uint32_t part_lFirst = 0;

/*
END SUPER BLOCK VARIABLES
--------------------------------------------------------------------------------
START OFFSET VARIABLES
*/

uint32_t num_zones;
uint32_t zone_size;
uint32_t inode_offset;
uint32_t inode_bitmap;
uint32_t zone_bitmap;

/*
END PARTITION VARIABLES
--------------------------------------------------------------------------------
START INODE VARS
*/

struct inode root;
struct inode temp;

/*
END INODE VARS
--------------------------------------------------------------------------------
START INODE METHODS
*/

/**
 *  Description:
 *    Gets the highest index of data
 *    being held in an inodes zones
 **/
int get_highest(FILE * system_ptr, struct inode ino, uint32_t loop_limit)
{
  int i;
  uint32_t zone;
  uint32_t high = 0;
  uint32_t position;

  if (loop_limit == DIRECT_ZONES)
  {
    for (i = 0; i < loop_limit; i++)
    {
      if (ino.zone[i] != 0) 
      { 
        high = i; 
      }
    }

    if (ino.indirect != 0) { high = 7; }
    if (ino.two_indirect != 0) { high = 8; }
  }
  else
  {
    position = ftell(system_ptr);

    for (i = 0; i < loop_limit; i++)
    {
      my_fread(&zone, READ_ONE, ZONE_NUM, system_ptr, "get_highest()");
      if (zone != 0) 
      { 
        high = i; 
      }
    }

    my_fseek(system_ptr, position, SEEK_SET, "get_highest()");
  }

  return high;
}

/**
 *  Description:
 *    Loops through directory to either prints
 *    stuff out, or find a specific inode
 **/
void list_dir_contents(uint32_t read_size, FILE * system_ptr,
                        int print_flag, int file_type, char * token)
{
  void * helper;
  unsigned long position;
  char name[MAX_PATH_SIZE];
  struct directory_entry de;
  uint8_t character, size_max_width = 10;

  (file_type == DIRECT_SIZE) ? (helper = &de) : (helper = &character);

  if (file_type == CHAR_SIZE && print_flag)
  {
    list_permissions(temp);
    printf("%*d ", size_max_width, temp.size);
    printf("%s\n", src_file_path);
    return;
  }

  while (read_size > 0)
  {
    my_fread(helper, READ_ONE, file_type, system_ptr, "list_dir_contents()");
    position = ftell(system_ptr);
    read_size -= file_type;

    if (file_type == DIRECT_SIZE && de.inode != 0)
    {
      snprintf(name, MAX_PATH_SIZE, "%s", de.name);
      copy_inode(system_ptr, de.inode);

      if (print_flag)
      {
        copy_inode(system_ptr, de.inode);
        list_permissions(temp);
        printf("%*d ", size_max_width, temp.size);
        printf("%s\n", de.name);
      }
      else if (strcmp(token, (const char *)name) == 0 && print_flag == 0) 
      {
        copy_inode(system_ptr, de.inode);
        memset(name, 0, MAX_PATH_SIZE);
        found = 1;
        return;
      }
      
      found = 0;
    }

    my_fseek(system_ptr, position, SEEK_SET, "rec_read_zone()");
  }
}

/**
 *  Description:
 *    Loops through specific inodes zones
 **/
void loop_through_inode(FILE * system_ptr, struct inode ino,
                        char * token, int rw_flag, int print_flag)
{
  int i;
  uint8_t dir_or_file;
  uint32_t file_type, read_size, offset;

  dir_or_file = get_file_type(ino, DIR_MASK);
  (dir_or_file == 1) ? (file_type = DIRECT_SIZE) : (file_type = CHAR_SIZE);

  if (file_type == CHAR_SIZE && print_flag)
  {
    list_dir_contents(zone_size, system_ptr, PRINT, file_type, NULL);
    return;
  }
  
  if (!dir_or_file) 
  { 
    printf("bailout\n"); 
    exit(1); 
  }

  if (print_flag == PRINT) { printf("%s:\n", src_file_path); }
  if (verbose) { verbose_inode(ino); }

  for (i = 0; i < DIRECT_ZONES; i++)
  {
    if (ino.zone[i] != 0)
    {
      offset = part_lFirst * SECTOR_SIZE;
      offset += zone_size * ino.zone[i];
      my_fseek(system_ptr, offset, SEEK_SET, "find_path()");

      read_size = min(zone_size, ino.size);

      (print_flag == PRINT) ?
      (list_dir_contents(zone_size, system_ptr, PRINT, file_type, NULL)) :
      (list_dir_contents(read_size, system_ptr, OPTION, file_type, token));

      if (found == 1) { 
        return; 
      }
    }
  }

  if (ino.indirect)
  {
    list_indirect(
      NULL,
      system_ptr, 
      ino, 
      token, 
      file_type, 
      print_flag, 
      INDIRECT, 
      rw_flag
    );
  }

  if (ino.two_indirect)
  {
    list_indirect(
      NULL,
      system_ptr, 
      ino, 
      token, 
      file_type, 
      print_flag, 
      TWO_INDIRECT, 
      rw_flag
    );
  }
}

/**
 *  Description:
 *    Handles indirect zones containing data
 **/
void list_indirect(FILE * out, FILE * system_ptr, struct inode ino, 
  char * token, uint32_t file_type, int p_flag, int d_flag, int rw_flag)
{
  int i, q;
  uint32_t offset, position, zone, highest = 0;

  offset = part_lFirst * SECTOR_SIZE;

  (d_flag == INDIRECT) ?
  (offset += zone_size * ino.indirect) :
  (offset += zone_size * ino.two_indirect);

  my_fseek(system_ptr, offset, SEEK_SET, "list_indirect()");
  highest = get_highest(system_ptr, ino, num_zones);

  if (d_flag == INDIRECT)
  {
    for (i = 0; i < num_zones; i++)
    {
      my_fread(&zone, READ_ONE, ZONE_NUM, system_ptr, "list_indirect()");
      position = ftell(system_ptr);

      offset = part_lFirst * SECTOR_SIZE;
      offset += zone_size * zone;
      my_fseek(system_ptr, offset, SEEK_SET, "list_indirect()");

      if (zone) 
      {
        switch (rw_flag)
        {
          case READING:
            list_dir_contents(zone_size, system_ptr, p_flag, file_type, token);
            break;

          case WRITING:
            if (i == highest && written + zone_size > ino.size) {
              write_zone(system_ptr, ino.size % zone_size, out, 1);
            } else {
              write_zone(system_ptr, zone_size, out, 1);
            }
            break;
        }
      }
      else if (i < highest && WRITING == rw_flag)
      {
        write_zone(system_ptr, zone_size, out, 0);
      }

      my_fseek(system_ptr, position, SEEK_SET, "list_indirect()");
    }
  }
  else
  {
    if (!ino.indirect && rw_flag == WRITING)
    {
      for (q = 0; q < num_zones * zone_size; q++)
      {
        written += (num_zones * zone_size);
        my_fwrite(out, READ_ONE, READ_ONE, 0, "list_indirect()");
      }
    }

    for (i = 0; i < num_zones; i++)
    {
      my_fread(&zone, READ_ONE, ZONE_NUM, system_ptr, "list_indirect()");
      position = ftell(system_ptr);
      ino.indirect = zone;

      if (zone != 0)
      {
        list_indirect(
          out, 
          system_ptr, 
          ino, 
          token, 
          file_type, 
          p_flag, 
          INDIRECT, 
          rw_flag
        );
        my_fseek(system_ptr, position, SEEK_SET, "list_indirect()");
      }
      else if (i < highest && rw_flag == WRITING)
      {
        for (q = 0; q < num_zones * zone_size; q++)
        {
          written += (num_zones * zone_size);
          my_fwrite(out, READ_ONE, READ_ONE, 0, "list_indirect()");
        }
      }
    }
  }
}

/**
 *  Description:
 *    Finds the root inode and reads it
 *    It also sets the table offset for the rest of the program
 **/
void read_root_inode(FILE * system_ptr)
{
  int z_block_offset = super->blocksize * super->z_blocks;
  int i_block_offset = super->blocksize * super->i_blocks;

  if (partition == -1)
  {
    inode_offset = 2 * super->blocksize;
    inode_offset += z_block_offset + i_block_offset;
  }
  else
  {
    inode_offset = 2 * super->blocksize;
    inode_offset += z_block_offset + i_block_offset;
    inode_offset += (part_lFirst) * SECTOR_SIZE;
  }
  
  my_fseek(system_ptr, inode_offset, SEEK_SET, "read_root_inode()");
  my_fread(&root, READ_ONE, INODE_SIZE, system_ptr, "read_root_inode()");
  zone_size = super->blocksize << super->log_zone_size;
  num_zones = zone_size / 4;
}

/**
 *  Description:
 *    Writes stuff into the output file
 **/
void write_zone(FILE * system_ptr, uint32_t read_size, FILE * out, int f)
{
  uint8_t character;
  written += read_size;

  while (read_size > 0)
  {
    my_fread(&character, READ_ONE, READ_ONE, system_ptr, "write_zone1()");
    (f) ?
    (my_fwrite(out, READ_ONE, READ_ONE, character, "write_zone2()")) :
    (my_fwrite(out, READ_ONE, READ_ONE, 0, "write_zone3()"));
    read_size -= sizeof(uint8_t);
  }
}

/**
 *  Description:
 *    Kicks everything off once the inode to write
 *    is found
 **/
void write_contents(FILE * system_ptr)
{
  int dir_or_file, i, highest;
  uint32_t offset, read_size, temp_var;
  FILE * out;

  temp_var = temp.size;
  dir_or_file = get_file_type(temp, REG_MASK);

  if (!dir_or_file) { exit(1); }
  (dest_file_path == NULL) ?
  (out = stdout) :
  (out = fopen(dest_file_path, "w"));

  highest = get_highest(system_ptr, temp, DIRECT_ZONES);

  if (temp.two_indirect != 0) 
  { 
    if (!temp.indirect) { temp_var -= zone_size; }
  }

  for (i = 0; i < highest; i++)
  {
    if (highest < 7 && temp.zone[i] == 0)
    {
      temp_var -= zone_size;
    }
  }

  for (i = 0; i < DIRECT_ZONES; i++)
  {
    if (temp.zone[i] != 0 || i < highest)
    {
      offset = part_lFirst * SECTOR_SIZE;
      offset += zone_size * temp.zone[i];

      read_size = min(zone_size, temp_var);
      my_fseek(system_ptr, offset, SEEK_SET, "write_contents()");
      if (temp.zone[i] == 0)
      {
        read_size = zone_size;
        write_zone(system_ptr, read_size, out, 0);
      }
      else
      {
        temp_var -= read_size;
        write_zone(system_ptr, read_size, out, 1);
      }
    }
  }

  if (temp.indirect)
  {
    list_indirect(
      out,
      system_ptr, 
      temp, 
      NULL, 
      CHAR_SIZE, 
      OPTION, 
      INDIRECT, 
      WRITING);
  }

  if (temp.two_indirect)
  {
    list_indirect(
      out,
      system_ptr, 
      temp, 
      NULL, 
      CHAR_SIZE, 
      OPTION, 
      TWO_INDIRECT, 
      WRITING
    );
  }
}

/**
 *  Description:
 *    Kicks everything off and calls all
 *    necessary methods
 **/
void files_from_inode(FILE * system_ptr, int rw_flag)
{
  char * token;
  char delim[2] = "/";
  char original[MAX_PATH_SIZE];

  strcpy(original, src_file_path);
  if (strcmp(src_file_path, "root") == 0)
  {
    src_file_path = "/";
    (rw_flag == READING) ?
    (loop_through_inode(system_ptr, root, NULL, rw_flag, PRINT)) :
    (write_contents(system_ptr));
  }
  else
  {
    token = strtok(src_file_path, delim);
    copy_inode(system_ptr, ROOT);

    while( token != NULL ) 
    {
      loop_through_inode(system_ptr, temp, token, READING, OPTION);
      token = strtok(NULL, delim);
    }

    strcpy(src_file_path, original);
    if (verbose) { verbose_inode(temp); }
    if ((temp.mode&SYM) != REG_MASK &&(temp.mode&SYM) != DIR_MASK) { exit(1); }

    if (found)
    {
      (rw_flag == READING) ?
      (loop_through_inode(system_ptr, temp, NULL, rw_flag, PRINT)) :
      (write_contents(system_ptr));
    }
    else
    {
      printf("%s: File not found\n", src_file_path);
      exit(1);
    }
  }
}

/*
END INODE METHODS
--------------------------------------------------------------------------------
START MISC METHODS
*/

/**
 *  Description:
 *    converts and prints out the date and time
 **/
void my_date_time(uint32_t time)
{
  time_t temp_time;
  temp_time = (time_t)time;

  printf("%s", ctime(&temp_time) );
}

/**
 *  Descrition:
 *    returns if the inode mode
 *    contains certain permissions or 
 *    if it is a directory
 **/
int get_file_type(struct inode ino, int mask)
{
  if ((ino.mode & mask) == mask) { return 1; }
  return 0;
}

/**
 *  Description:
 *    returns the minimum number
 **/
uint32_t min(uint32_t a, uint32_t b)
{
  if (a < b) { return a; }
  return b;
}

/**
 *  Description:
 *    This method copies in an inode into
 *    the temp inode structure
 **/
void copy_inode(FILE * system_ptr, uint32_t i_num)
{
  int total_seek = inode_offset + (i_num - 1) * INODE_SIZE;
  my_fseek(system_ptr, total_seek, SEEK_SET, "copy_inode()");
  my_fread(&temp, READ_ONE, INODE_SIZE, system_ptr, "copy_inode()");
}

/*
END MISC METHODS
--------------------------------------------------------------------------------
START PRINTING METHODS
*/

/**
 *  Description:
 *    Prints out the inode verbosely
 **/
void verbose_inode(struct inode ino)
{
  int i;
  int max_width = 10;
  int max_width_zone = 10;
  int max_width_upper = 15;

  printf("File inode:\n");
  printf("  uint16_t mode   %#*x\t(", max_width_upper, ino.mode);
  list_permissions(ino);
  printf(")\n");
  printf("  uint16_t links  %*d\n", max_width_upper, ino.links);
  printf("  uint16_t uid    %*d\n", max_width_upper, ino.uid);
  printf("  uint16_t gid    %*d\n", max_width_upper, ino.gid);
  printf("  uint32_t size   %*d\n", max_width_upper, ino.size);

  printf("  uint32_t atime  %*d --- ", max_width_upper, ino.atime);
  my_date_time(ino.atime);
  printf("  uint32_t mtime  %*d --- ", max_width_upper, ino.mtime);
  my_date_time(ino.mtime);
  printf("  uint32_t ctime  %*d --- ", max_width_upper, ino.ctime);
  my_date_time(ino.ctime);

  printf("  Direct zones:\n");
  for (i = 0; i < DIRECT_ZONES; i++)
  {
    printf("        \
      zone[%d]   =  %*d\n", i, max_width_zone, ino.zone[i]);
  }

  printf("    uint32_t indirect   =  %*d\n", max_width, ino.indirect);
  printf("    uint32_t double     =  %*d\n", max_width, ino.two_indirect);
}

/**
 *  Description:
 *    This method lists the permissions of the
 *    temp structure
 **/
void list_permissions(struct inode ino)
{
  (get_file_type(ino, DIR_MASK)) ?
  (printf("d")) :
  (printf("-"));

  (get_file_type(ino, OR_MASK)) ?
  (printf("r")) :
  (printf("-"));

  (get_file_type(ino, OW_MASK)) ?
  (printf("w")) :
  (printf("-"));

  (get_file_type(ino, OX_MASK)) ?
  (printf("x")) :
  (printf("-"));

  (get_file_type(ino, GR_MASK)) ?
  (printf("r")) :
  (printf("-"));

  (get_file_type(ino, GW_MASK)) ?
  (printf("w")) :
  (printf("-"));

  (get_file_type(ino, GX_MASK)) ?
  (printf("x")) :
  (printf("-"));

  (get_file_type(ino, OTR_MASK)) ?
  (printf("r")) :
  (printf("-"));

  (get_file_type(ino, OTW_MASK)) ?
  (printf("w")) :
  (printf("-"));

  (get_file_type(ino, OTX_MASK)) ?
  (printf("x")) :
  (printf("-"));
}

/*
END PRINTING METHODS
--------------------------------------------------------------------------------
*/

void set_partition(int p) { partition = p; }
void set_subpartition(int s) { sub_partition = s; }
void set_src_path(char * path) { src_file_path = path; }
void set_dst_path(char * path) { dest_file_path = path; }
void set_filename(char * f) { filename = f; }
void set_verbose(int v) { verbose = v; }

/*
--------------------------------------------------------------------------------
*/

void set_super_struct(struct superblock * s) { super = s; }
void set_partition_struct(struct partition * p) { part = p; }
void set_part_lFirst(int l) { part_lFirst = l; }

/*
--------------------------------------------------------------------------------
*/
