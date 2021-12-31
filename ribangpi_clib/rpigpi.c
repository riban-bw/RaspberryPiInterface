/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: GPI Library
 *
 * Library for interfacing naitive RPI GPI with Zynthian
 *
 * Copyright (C) 2015-2021 Fernando Moyano <jofemodo@zynthian.org>
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

#include "rpigpi.h"
#include <sys/mman.h> //Provides mmap
#include <fcntl.h> //Provides open
#include <unistd.h> //Provides close

#define MAX_RPI_GPI 32 // Actually 54 but only 2-27 available
#define BLOCK_SIZE  (4 * 1024)

//  BCM2835 Registers
#define BCM2835_GPSET0      7
#define BCM2835_GPCLR0      10
#define BCM2835_GPLEV0      13
#define BCM2835_GPEDS0      16
#define BCM2835_GPREN0      19
#define BCM2835_GPFEN0      22
#define BCM2835_GPHEN0      25
#define BCM2835_GPLEN0      28
#define BCM2835_GPAREN0     31
#define BCM2835_GPAFEN0     34
#define BCM2835_GPPUD       37
#define BCM2835_GPPUDCLK0   38

uint32_t* gpiMmap;
static const uint8_t unavailableGpi[MAX_RPI_GPI] = {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1};

int addRpiGpiDevice() {
    //!@todo Abstract device non-specific code
    uint8_t driverCount;
    for(driverCount = 0; driverCount < MAX_GPI_DRIVERS; ++driverCount) {
        if(gpiDrivers[driverCount].type == GPI_DRIVER_RPI)
            return driverCount;
        if(gpiDrivers[driverCount].type == GPI_DRIVER_NONE)
            break;
    }
    if(driverCount >= MAX_GPI_DRIVERS)
        return -1;

    // Create memory map of GPI
    int fd = open("/dev/gpiomem", O_RDWR|O_SYNC);
    if(fd < 0)
        return -1;
    gpiMmap = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); //Don't need the file open after memory map
    if(gpiMmap == MAP_FAILED)
        return -1;

    gpi_driver_t* driver = &gpiDrivers[driverCount];
    driver->type = GPI_DRIVER_RPI;
    driver->size = MAX_RPI_GPI; // Device specific size
    driver->offset = zynGpiCount;
    driver->setState = setRpiGpiState;
    driver->setDirection = setRpiGpiDirection;
    driver->setPull= setRpiGpiPull;
    driver->poll = pollRpiGpi;
    driver->destroy = destroyRpiGpiDevice;
    driver->gpis = (gpi_t*) malloc (driver->size * sizeof(gpi_t));
    for(int i  = 0; i < driver->size; ++i) {
        (driver->gpis[i]).value = 0;
        (driver->gpis[i]).enabled = 0;
        (driver->gpis[i]).dir = 0;
        gpimap[zynGpiCount].driver = driverCount;
        gpimap[zynGpiCount++].offset = i;
        setRpiGpiDirection(i, INPUT);
    }

    return driverCount;
}

void destroyRpiGpiDevice() {
    munmap(gpiMmap, BLOCK_SIZE);
}

void setRpiGpiState(uint32_t gpi, uint8_t state) {
    //!@todo Validate GPI enabled and direction=output
    uint32_t offset = gpimap[gpi].offset;
    if(offset > 31 | unavailableGpi[offset])
        return;
    if(state)
        *(gpiMmap + BCM2835_GPSET0) = 1 << offset;
    else
        *(gpiMmap + BCM2835_GPCLR0) = 1 << offset;
}

uint8_t getRpiGpiState(uint32_t gpi) {
    uint32_t offset = gpimap[gpi].offset;
    if(offset > 31 | unavailableGpi[offset])
        return 0;
//    return(((*(gpiMmap + BCM2835_GPLEV0 + offset / 32)) & (1 << (offset % 32))) != 0); //!@todo Should we optimise this due to fewer than 32 GPI being exposed?
    return(((*(gpiMmap + BCM2835_GPLEV0)) & (1 << offset)) != 0);
}

void setRpiGpiDirection(uint32_t gpi, uint8_t dir) {
    // Range check already performed by GPI library function
    uint32_t offset = gpimap[gpi].offset;
    if(offset > 31 | unavailableGpi[offset])
        return;
    //Clear configuration bits
    *(gpiMmap + (offset / 10)) &= ~(7 << ((offset % 10) * 3)); //reset 3 flags for this gpi
    //Set configuration bits to match requested mode
    *(gpiMmap + (offset / 10)) |= ((dir & 0x01) << ((offset % 10) * 3)); //Configure for function
    getGpi(gpi).dir = dir?1:0; // Update value upon success
}

void setRpiGpiPull(uint32_t gpi, uint8_t mode) {
    uint32_t offset = gpimap[gpi].offset;
    if(offset > 31 || unavailableGpi[offset])
        return;
    *(gpiMmap + BCM2835_GPPUD) = (uint32_t)mode & 3;
    usleep(1); //Need to wait 150 cycles which is 0.6us on the slowest RPi so let's wait 1us
//    *(gpiMmap + BCM2835_GPPUDCLK0 + (offset / 32)) = 1 << (offset % 32); //!@todo Should we optimise this due to fewer than 32 GPI being exposed?
    *(gpiMmap + BCM2835_GPPUDCLK0) = (1 << offset);
    usleep(1); //Need to wait 150 cycles which is 0.6us on the slowest RPi so let's wait 1us
//    *(gpiMmap + BCM2835_GPPUDCLK0 + (offset / 32)) = 0; //!@todo Should we optimise this due to fewer than 32 GPI being exposed?
    *(gpiMmap + BCM2835_GPPUD) = 0;
    *(gpiMmap + BCM2835_GPPUDCLK0) = 0;
}

uint8_t pollRpiGpi(uint32_t driver) {
    // If polling not required then return 0 immediately
    uint8_t value, changed = 0;
    gpi_driver_t* pDriver = &gpiDrivers[driver];
    for(int offset = 2; offset < 28; ++offset) {
        gpi_t* gpi = &(pDriver->gpis[offset]);
        if(gpi->enabled) {
            value = getRpiGpiState(offset);
            if(gpi->value == value)
                continue;
            changed = 1;
            gpi->value = value;
        }
    }
    return changed;
}

