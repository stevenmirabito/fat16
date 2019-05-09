#include "stdlib.h"
#include "stdio.h"
#include "virtual.h"
#include "testimg.h"

void *virtual_open() {
    virtual_device *device = malloc(sizeof(virtual_device));
    device->img_size = TESTIMG_SIZE;
    device->img = malloc(TESTIMG_SIZE);

    // Copy test image into memory
    for (int i = 0; i < TESTIMG_SIZE; i++) {
        device->img[i] = TESTIMG[i];
    }

    return device;
}

int virtual_close(void *fsd) {
    virtual_device *device = fsd;
    free(device->img);
    free(device);

    return 0;
}

int virtual_read(void *fsd, unsigned int offset, unsigned int len, unsigned char buf[]) {
    virtual_device *device = fsd;

    // Overflow check
    if (device->img == 0 || offset > device->img_size || (offset + len) > device->img_size) {
        return FS_ERR_INVALID_READ;
    }

    for (int i = 0; i < len; i++) {
        buf[i] = device->img[offset + i];
    }

    return 0;
}

int virtual_write(void *fsd, unsigned int offset, unsigned int len, const unsigned char buf[]) {
    virtual_device *device = fsd;

    // Overflow check
    if (device->img == 0 || offset > device->img_size || (offset + len) > device->img_size) {
        return FS_ERR_INVALID_WRITE;
    }

    for (int i = 0; i < len; i++) {
        device->img[i] = buf[offset + i];
    }

    return 0;
}

const fs_driver virtual_driver = {
        .name   = VIRTUAL_DRIVER_NAME,
        .open   = virtual_open,
        .close  = virtual_close,
        .read   = virtual_read,
        .write  = virtual_write,
};
