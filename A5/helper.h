
#ifndef HELPER_H
#define HELPER_H

#include "partition_info.h"
/*
--------------------------------------------------------------------------------
START STRUCT SIZES
*/

#define ZONE_NUM sizeof(uint32_t)
#define INODE_SIZE sizeof(struct inode)
#define DIRECT_SIZE sizeof(struct directory_entry)
#define TOTAL_SUPER 1024
#define TABLE_ENTRY_SIZE 128
#define DIRECT_ZONES 7

/*
END STRUCT SIZES
--------------------------------------------------------------------------------
START BIT MASKS
*/

#define REG_MASK  0100000
#define DIR_MASK  0040000
#define OR_MASK  0000400
#define OW_MASK  0000200
#define OX_MASK  0000100
#define GR_MASK  0000040
#define GW_MASK  0000020
#define GX_MASK  0000010
#define OTR_MASK 0000004
#define OTW_MASK 0000002
#define OTX_MASK 0000001
#define SYM 0170000

/*
END BIT MASKS
--------------------------------------------------------------------------------
START PARITION VARS/OFFSETS
*/

#define TWO_INDIRECT 2
#define INDIRECT 1

/*
END VALID VALS
--------------------------------------------------------------------------------
START MISC
*/

#define DEBUG 0
#define LS_FLAG 1
#define GET_FLAG 0
#define READ_ONE 1
#define CHAR_SIZE 1
#define MAX_PATH_SIZE 10000
#define ROOT 1
#define READING 1
#define WRITING 0
#define OPTION 0
#define PRINT 1

/*
END MISC
--------------------------------------------------------------------------------
START STRUCT DEFS
*/


struct __attribute__ ((__packed__)) inode
{
	uint16_t mode;
	uint16_t links;
	uint16_t uid;
	uint16_t gid;
	uint32_t size;
	int32_t atime;
	int32_t mtime;
	int32_t ctime;
	uint32_t zone[DIRECT_ZONES];
	uint32_t indirect;
	uint32_t two_indirect;
	uint32_t unused;
};

struct __attribute__ ((__packed__)) directory_entry
{
	uint32_t inode;
	unsigned char name[60];
};

/*
END STRUCT DEFS
--------------------------------------------------------------------------------
*/

void set_super_struct(struct superblock * s);
void set_partition_struct(struct partition * p);
void set_part_lFirst(int l);

/*
--------------------------------------------------------------------------------
START PARSING METHODS
*/

void set_partition(int p);
void set_subpartition(int s);
void set_src_path(char * path);
void set_dst_path(char * path);
void set_filename(char * f);
void set_verbose(int v);

/*
END PARSING METHODS
--------------------------------------------------------------------------------
START INODE METHOD
*/

int get_highest(FILE * system_ptr, struct inode ino, uint32_t loop_limit);
void write_zone(FILE * system_ptr, uint32_t read_size, FILE * out, int f);
void files_from_inode(FILE * system_ptr, int rw_flag);
void read_root_inode(FILE * system_ptr);
void verbose_inode(struct inode ino);
void list_indirect(FILE * out, FILE * system_ptr, struct inode ino, 
  char * token, uint32_t file_type, int p_flag, int d_flag, int rw_flag);

void loop_through_inode(FILE * system_ptr, struct inode ino,
                        char * token, int rw_flag, int print_flag);

void list_dir_contents(uint32_t read_size, FILE * system_ptr,
                        int print_flag, int file_type, char * token);

void write_indirect(
  FILE * out,
  FILE * system_ptr, 
  struct inode ino, 
  int d_flag
 );

 void write_contents(FILE * system_ptr);


/*
END INODE METHOD
--------------------------------------------------------------------------------
START PRINTING METHODS
*/

void list_permissions();
void list_contents(FILE * system_ptr, struct inode ino);
void list_contents_helper(
  FILE * system_ptr, 
  uint8_t file_type, 
  uint32_t read_size
 );

void indirect_contents(
  FILE * system_ptr, 
  struct inode ino, 
  int d_flag
 );


/*
END FILE MANIPULATION METHODS
--------------------------------------------------------------------------------
START MISC METHODS
*/

void copy_inode(FILE * system_ptr, uint32_t i_num);
int get_file_type(struct inode ino, int mask);
uint32_t min(uint32_t a, uint32_t b);
void my_date_time(uint32_t time);

/*
END MISC METHODS
--------------------------------------------------------------------------------
*/


#endif














