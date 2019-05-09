#ifndef __FS_DRIVER_H
#define __FS_DRIVER_H

/**
 * Invalid read operation was attempted
 */
#define FS_ERR_INVALID_READ -11

/**
 * Invalid write operation was attempted
 */
#define FS_ERR_INVALID_WRITE -12

/**
 * Interface for a storage driver.
 */
typedef struct {
    /**
     * Name for the driver.
     */
    const char *name;

    /**
     * Open the storage device.
     * @return Storage device
     */
    void *(*open)();

    /**
     * Close the storage device.
     * @param fsd Storage device
     * @return Error code if unsuccessful
     */
    int (*close)(void *fsd);

    /**
     * Read bytes from the specified device into a buffer.
     * @param fsd Storage device
     * @param offset Location to start reading
     * @param len Number of bytes to read
     * @param buf Buffer to store data read
     * @return Number of bytes read or error code if unsuccessful
     */
    int (*read)(void *fsd, unsigned int offset, unsigned int len, unsigned char buf[]);

    /**
     * Write bytes from a buffer to the specified device.
     * @param fsd Storage device
     * @param offset Location to start writing
     * @param len Number of bytes to write
     * @param buf Buffer of bytes to write
     * @return Number of bytes read or error code if unsuccessful
     */
    int (*write)(void *fsd, unsigned int offset, unsigned int len, const unsigned char buf[]);
} fs_driver;

#endif //__FS_DRIVER_H
