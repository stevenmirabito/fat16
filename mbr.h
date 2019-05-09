#ifndef __FS_MBR_H
#define __FS_MBR_H

/**
 * Offset to the beginning of the partition table
 */
static const int MBR_TABLE_START = 0x1BE;

/**
 * MBR Partition Table
 */
typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned int start_sector;
    unsigned int length_sectors;
} __attribute((packed)) MBRPartitionTable;

#endif //__FS_MBR_H
