
#ifndef PARTITION_INFO_H
#define PARTITION_INFO_H

#define OFFSET_510 510
#define OFFSET_511 510
#define VALID_510 0x55
#define VALID_511 0xAA

#define READ_ONE 1
#define PART_SIZE 16
#define PARTITION_INFO 0x1BE

#define SUPER_SIZE sizeof(struct superblock)
#define SUPER_OFFSET 1024
#define SECTOR_SIZE 512

#define MINIX_TYPE 0x81
#define MINIX_MAGIC 0x4D5A

struct __attribute__ ((__packed__)) partition 
{
	uint8_t   bootind;
	uint8_t   start_head;
	uint8_t   start_sec;
	uint8_t   start_cyl;
	uint8_t   type;
	uint8_t   end_head;
	uint8_t   end_sec;
	uint8_t   end_cyl;
	uint32_t  lFirst;
	uint32_t  size;
};

struct __attribute__ ((__packed__)) superblock
{
	uint32_t ninodes;
	uint16_t pad1;
	int16_t  i_blocks;
	int16_t  z_blocks;
	uint16_t firstdata;
	int16_t  log_zone_size;
	int16_t  pad2;
	uint32_t max_file;
	uint32_t zones;
	int16_t  magic;
	int16_t  pad3;
	uint16_t blocksize;
	uint8_t  subversion;
};

void read_partition_info(FILE * system_ptr, uint32_t offset, int partition);
void validate_partition(FILE * system_ptr, uint32_t offset);
void read_superblock(
  FILE * system_ptr, 
  int partition, 
  int subpartition,
  int verbose
);

void verbose_superblock();

struct superblock * get_super_struct();
struct partition * get_partition_struct();
int get_lFirst();

#endif



