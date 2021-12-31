/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: I2C Library
 *
 * Library for interfacing I2C with Zynthian
 *
 * Copyright (C) 2021 Brian Walton <riban@zynthian.org>
 *
 * ******************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE.txt file.
 *
 * ******************************************************************
 */

#include "i2c.h"

int i2cFd = -1; // File descriptor for I2C device

int i2cGetFd() {
    return i2cFd;
}

int i2cOpen() {
    if(i2cFd >= 0)
        return i2cFd; // Already open
    i2cFd = open("/dev/i2c-1", O_RDWR);
    return i2cFd;
}

void i2cClose() {
    if(i2cFd < 0)
        return;
    close(i2cFd);
    i2cFd = -1;
}

int i2cSelectDevice(uint8_t address) {
    if(i2cFd < 0)
        return -1;
    return ioctl(i2cFd, I2C_SLAVE, address);
}

void i2cWriteByte(uint8_t value) {
    if(i2cFd < 0)
        return;
    write(i2cFd, &value, 1);
}

uint8_t i2cReadByte() {
    if(i2cFd < 0)
        return 0;
    uint8_t value = 0;
    read(i2cFd, &value, 1);
    return value;
}
