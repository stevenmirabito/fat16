#include "stdio.h"
#include "fat16.h"

int main() {
    int res = 0;

    printf("[FS] Initializing Filesystem - Author: Steven Mirabito <stm4445@rit.edu>\n");

    Fat16Context *ctx;
    res = fat16_init(&ctx);

    if (res != 0) {
        // Error occurred during initialization
        if (res == FAT16_ERR_NO_PARTITION_FOUND) {
            printf("[FS] FATAL: Failed to find FAT16 partition\n");
        } else if (res == FAT16_ERR_INVALID_SECTOR_SIZE) {
            printf("[FS] FATAL: Invalid or unsupported sector size\n");
        } else {
            printf("[FS] FATAL: Unknown error occurred during initialization\n");
        }

        fat16_free(ctx);
        return res;
    }

    // Read filesystem info
    unsigned char buf[sizeof(Fat16BootSector)];
    fat16_bs(ctx, buf);
    Fat16BootSector *fs_info = (Fat16BootSector *) buf;

    // Print filesystem info
    printf("[FS] Found FAT16 filesystem with label [%.11s]\n", fs_info->volume_label);
    printf("  Jump code: %02X:%02X:%02X\n", fs_info->jmp[0], fs_info->jmp[1], fs_info->jmp[2]);
    printf("  OEM code: [%.8s]\n", fs_info->oem);
    printf("  sector_size: %d\n", fs_info->sector_size);
    printf("  sectors_per_cluster: %d\n", fs_info->sectors_per_cluster);
    printf("  reserved_sectors: %d\n", fs_info->reserved_sectors);
    printf("  number_of_fats: %d\n", fs_info->number_of_fats);
    printf("  root_dir_entries: %d\n", fs_info->root_dir_entries);
    printf("  total_sectors_short: %d\n", fs_info->total_sectors_short);
    printf("  media_descriptor: 0x%02X\n", fs_info->media_descriptor);
    printf("  fat_size_sectors: %d\n", fs_info->fat_size_sectors);
    printf("  sectors_per_track: %d\n", fs_info->sectors_per_track);
    printf("  number_of_heads: %d\n", fs_info->number_of_heads);
    printf("  hidden_sectors: %d\n", fs_info->hidden_sectors);
    printf("  total_sectors_long: %d\n", fs_info->total_sectors_long);
    printf("  drive_number: 0x%02X\n", fs_info->drive_number);
    printf("  current_head: 0x%02X\n", fs_info->current_head);
    printf("  boot_signature: 0x%02X\n", fs_info->boot_signature);
    printf("  volume_id: 0x%08X\n", fs_info->volume_id);
    printf("  Filesystem type: [%.8s]\n", fs_info->fs_type);
    printf("  Boot sector signature: 0x%04X\n", fs_info->boot_sector_signature);

    // Open file
    printf("[FS] Attempting to open HAMLET.TXT\n");
    int file_size = fat16_open(ctx, "HAMLET  ", "TXT");
    if (file_size < 0) {
        res = file_size;

        // Read error occurred
        if (res == FAT16_ERR_FILE_READ) {
            printf("[FS] FATAL: Failed to read file\n");
        } else if (res == FAT16_ERR_FILE_NOT_FOUND) {
            printf("[FS] FATAL: File not found\n");
        } else {
            printf("[FS] FATAL: Unknown read error\n");
        }

        fat16_free(ctx);
        return res;
    }

    // Read file
    printf("[FS] Attempting to read HAMLET.TXT (%d bytes)\n", file_size);
    unsigned char file_buf[file_size];
    int bytes = fat16_read(ctx, (unsigned int) file_size, file_buf);
    if (bytes < 1) {
        // Read error occurred
        res = FAT16_ERR_FILE_READ;
        printf("[FS] FATAL: Failed to read file\n");

        fat16_free(ctx);
        return res;
    }

    // Successfully read file
    printf("[FS] Successfully read file!\n%s\n", file_buf);

    return 0;
}