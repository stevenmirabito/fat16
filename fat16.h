#ifndef __FS_FAT16_H
#define __FS_FAT16_H

#include "driver.h"
#include "mbr.h"

/**
 * No FAT16 partition is found on the device
 */
#define FAT16_ERR_NO_PARTITION_FOUND -1

/**
 * The sector size of the FAT16 partition is invalid
 */
#define FAT16_ERR_INVALID_SECTOR_SIZE -2

/**
 * Error reading file
 */
#define FAT16_ERR_FILE_READ -3

/**
 * File not found
 */
#define FAT16_ERR_FILE_NOT_FOUND -4

/**
 * FAT16 Boot Sector
 */
typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short; // if zero, later field is used
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_long;
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat16BootSector;

/**
 * FAT16 File Entry
 */
typedef struct {
    unsigned char filename[8];
    unsigned char ext[3];
    unsigned char attributes;
    unsigned char reserved[10];
    unsigned short modify_time;
    unsigned short modify_date;
    unsigned short starting_cluster;
    unsigned int file_size;
} __attribute((packed)) Fat16Entry;

/**
 * Filesystem Context
 */
typedef struct {
    unsigned int fat_start; // FAT start position
    unsigned int data_start; // data start position
    unsigned int current_pos; // current position
    unsigned char sectors_per_cluster; // cluster size in sectors
    unsigned short cluster; // current cluster being read
    unsigned int cluster_left; // bytes left in current cluster
    unsigned int file_left; // bytes left in the file being read
    fs_driver driver; // Driver instance
    void *fsd; // Opaque storage device
} __attribute((packed)) Fat16Context;

/**
 * Initialize the FAT16 library
 * @param ctx Pointer for opaque filesystem context (must be freed with fat16_free)
 * @return FAT16_ERR if an error occurred
 */
int fat16_init(Fat16Context **ctx);

/**
 * Reads and returns the FAT16 boot sector.
 * @param ctx Pointer for opaque filesystem context (must be freed with fat16_free)
 * @param buf Buffer to hold the boot sector read from the partition
 * @return FAT16_ERR if an error occurred
 */
int fat16_bs(Fat16Context *ctx, unsigned char buf[]);

/**
 * Open file
 * @param ctx Filesystem state
 * @param filename File name
 * @param ext File extension
 */
int fat16_open(Fat16Context *ctx, char *filename, char *ext);

/**
 * Read file
 * @param ctx Filesystem state
 * @param len Number of bytes to read
 * @param res_buf Pointer for buffer to hold file being read
 * @return Number of bytes read
 */
int fat16_read(Fat16Context *ctx, unsigned int len, unsigned char res_buf[]);

/**
 * De-initialize the FAT16 library
 * @param ctx Filesystem state
 * @return A FAT16_ERR code if de-initialization failed
 */
int fat16_free(Fat16Context *ctx);

#endif //__FS_FAT16_H
