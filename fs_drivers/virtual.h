/**
 * @file virtual.h
 * @brief Virtual storage driver
 */

#ifndef __FS_DRIVER_VIRTUAL_H
#define __FS_DRIVER_VIRTUAL_H

#include "../driver.h"

/**
 * Name of the driver.
 */
#define VIRTUAL_DRIVER_NAME "virtual"

/**
 * Failed to open filesystem image.
 */
#define FS_ERR_VIRTUAL_OPEN -101;

/**
 * Internal device context.
 */
typedef struct {
    unsigned int img_size;
    unsigned char *img;
} virtual_device;

extern const fs_driver virtual_driver;

#endif //__FS_DRIVER_VIRTUAL_H
