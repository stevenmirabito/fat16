#include "fat16.h"
#include "stdlib.h"
#include "stdio.h"
#include "mbr.h"
#include "fs_drivers/virtual.h"

/**
 * Initialize the FAT16 library
 * @param ctx Pointer for opaque filesystem context (must be freed with fat16_free)
 * @return FAT16_ERR if an error occurred
 */
int fat16_init(Fat16Context **ctx) {
    *ctx = malloc(sizeof(Fat16Context));

    (*ctx)->driver = virtual_driver;
    (*ctx)->fsd = (*ctx)->driver.open();

    // Find first FAT16 partition
    int i = 0;
    for (i = 0; i < 4; i++) {
        unsigned char buf[sizeof(MBRPartitionTable)];
        uint32_t table_offset = MBR_TABLE_START + (sizeof(MBRPartitionTable) * i);
        (*ctx)->driver.read((*ctx)->fsd, table_offset, sizeof(MBRPartitionTable), buf);

        MBRPartitionTable *part = (MBRPartitionTable *) buf;

        if (part->partition_type == 4 || part->partition_type == 6 || part->partition_type == 14) {
            // Found a FAT16 partition
            (*ctx)->fat_start = 512 * part->start_sector;
            break;
        }
    }

    if (i == 4) {
        // Didn't find a FAT16 partition
        return FAT16_ERR_NO_PARTITION_FOUND;
    }

    unsigned char buf[sizeof(Fat16BootSector)];
    fat16_bs(*ctx, buf);
    Fat16BootSector *boot = (Fat16BootSector *) buf;

    if (boot->sector_size != 512) {
        return FAT16_ERR_INVALID_SECTOR_SIZE;
    }

    (*ctx)->fat_start += boot->reserved_sectors * 512;
    (*ctx)->current_pos =
            (*ctx)->fat_start + (unsigned int) boot->fat_size_sectors * (unsigned int) boot->number_of_fats * 512;
    (*ctx)->data_start = (*ctx)->current_pos + sizeof(Fat16Entry) * (unsigned int) boot->root_dir_entries;
    (*ctx)->sectors_per_cluster = boot->sectors_per_cluster;
    (*ctx)->file_left = boot->root_dir_entries * sizeof(Fat16Entry);
    (*ctx)->cluster_left = 0xFFFFFFFF; // Avoid unnecessary FAT lookups in root dir

    return 0;
}

/**
 * Reads and returns the FAT16 boot sector
 * @param ctx Pointer for opaque filesystem context (must be freed with fat16_free)
 * @return FAT16_ERR if an error occurred
 */
int fat16_bs(Fat16Context *ctx, unsigned char buf[]) {
    ctx->driver.read(ctx->fsd, ctx->fat_start, sizeof(Fat16BootSector), buf);
    return 0;
}

/**
 * Open file
 * @param ctx Filesystem state
 * @param filename File name
 * @param ext File extension
 */
int fat16_open(Fat16Context *ctx, char *filename, char *ext) {
    do {
        unsigned char buf[sizeof(Fat16Entry)];
        int bytes_read = fat16_read(ctx, sizeof(Fat16Entry), buf);

        if (bytes_read < sizeof(Fat16Entry)) {
            return FAT16_ERR_FILE_READ;
        }

        Fat16Entry *file = (Fat16Entry *) buf;

        // Compare filename and extension
        int i = 0;
        for (i = 0; i < 8; i++) {
            if (file->filename[i] != filename[i]) {
                // Not the correct filename
                break;
            }
        }
        if (i < 8) {
            // Didn't find the correct filename, continue searching
            ctx->current_pos = ctx->current_pos + sizeof(Fat16Entry);
            continue;
        }

        for (i = 0; i < 3; i++) {
            if (file->ext[i] != ext[i]) {
                // Not the correct file extension
                break;
            }
        }
        if (i < 3) {
            // Didn't find the correct file extension, continue searching
            ctx->current_pos = ctx->current_pos + sizeof(Fat16Entry);
            continue;
        }

        // Found the file
        ctx->cluster = file->starting_cluster;
        ctx->cluster_left = (unsigned int) ctx->sectors_per_cluster * 512;

        // Is this a directory?
        if (file->filename[0] == 0x2E || file->attributes & 0x10) {
            // File is a directory
            ctx->file_left = 0xFFFFFFFF;
        } else {
            // Normal file
            ctx->file_left = file->file_size;
        }

        // Move to first cluster of file
        ctx->current_pos =
                ctx->data_start + (unsigned int) (ctx->cluster - 2) * (unsigned int) ctx->sectors_per_cluster * 512;

        return file->file_size;
    } while (ctx->file_left > 0);

    // Didn't find the requested file
    return FAT16_ERR_FILE_NOT_FOUND;
}

/**
 * Read file
 * @param ctx Filesystem state
 * @param read_size Number of bytes to read
 * @param res_buf Buffer to hold file being read
 * @return Number of bytes read
 */
int fat16_read(Fat16Context *ctx, unsigned int read_size, unsigned char res_buf[]) {
    unsigned int bytes_read = 0;
    unsigned int buf_pos = 0;

    while (ctx->file_left > 0 && bytes_read < read_size) {
        if (ctx->cluster_left == 0) {
            unsigned char buf[2];
            ctx->driver.read(ctx->fsd, ctx->fat_start + (unsigned int) ctx->cluster * 2, 2, buf);
            ctx->cluster = ((unsigned short *) buf)[0];
            ctx->cluster_left = (unsigned int) ctx->sectors_per_cluster * 512;

            if (ctx->cluster == 0xFFFF) {
                // End of cluster linked list
                ctx->file_left = 0;
                return 0;
            }

            // Move to next cluster
            ctx->current_pos =
                    ctx->data_start + (unsigned int) (ctx->cluster - 2) * (unsigned int) ctx->sectors_per_cluster * 512;
        }

        // Determine size of next read operation
        unsigned int len = ctx->file_left;

        if (len > ctx->cluster_left) {
            len = ctx->cluster_left;
        }

        if (len > (read_size - bytes_read)) {
            len = read_size - bytes_read;
        }

        // Perform read
        unsigned char buf[len];
        ctx->driver.read(ctx->fsd, ctx->current_pos, len, buf);

        // Copy into result buffer
        for (int i = 0; i < len; i++) {
            res_buf[buf_pos] = buf[i];
            buf_pos++;
        }

        // Update state
        ctx->current_pos += len;
        ctx->file_left -= len;
        ctx->cluster_left -= len;
        bytes_read += len;
    }

    return bytes_read;
}

/**
 * De-initialize the FAT16 library
 * @param ctx Filesystem state
 * @return A FAT16_ERR code if de-initialization failed
 */
int fat16_free(Fat16Context *ctx) {
    // De-initialize the driver
    ctx->driver.close(ctx->fsd);

    // Free the struct
    free(ctx);
    return 0;
}