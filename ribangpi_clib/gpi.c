/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: GPI Library
 *
 * Library for interfacing GPI with Zynthian
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

#include "gpi.h"
#include <pthread.h> // Provdes thread
#include <string.h> // Provides strerror

pthread_t pollThread;
gpi_driver_t gpiDrivers[MAX_GPI_DRIVERS];
gpi_map_t gpimap[MAX_GPI];
uint32_t zynGpiCount = 0;

/*  Define private functions */
void * poll_gpi(void *arg);
void resetDriver(uint8_t driver) {
        gpiDrivers[driver].type = GPI_DRIVER_NONE;
        gpiDrivers[driver].size = 0;
        gpiDrivers[driver].offset = 0;
        gpiDrivers[driver].config = NULL;
        gpiDrivers[driver].destroy = NULL;
        gpiDrivers[driver].setState = NULL;
        gpiDrivers[driver].setPull = NULL;
        gpiDrivers[driver].poll = NULL;
}

// Run when library loaded
void __attribute__ ((constructor)) init() {
    for(int i = 0; i < MAX_GPI_DRIVERS; ++i)
        resetDriver(i);
    int err = pthread_create(&pollThread, NULL, &poll_gpi, NULL);
	if (err)
		fprintf(stderr, "ZynGPI: Can't create poll thread :[%s]", strerror(err));

    //!@todo Init low-level libs as required, e.g. wiringPi
}

// Run when library unloaded
void __attribute__ ((destructor)) onexit() {
    printf("zyngpi lib exit\n");
    for(int i = MAX_GPI_DRIVERS - 1; i >= 0; --i) {
        if(gpiDrivers[i].type)
            printf("Need to destroy driver %d of type %d\n", i, gpiDrivers[i].type);
    }
}

void printInfo() {
    for(int i = 0; i < MAX_GPI_DRIVERS; ++i) {
        printf("Driver %d\n\ttype: %d\n\toffset: %u\n\tsize: %u\n", i, gpiDrivers[i].type, gpiDrivers[i].offset, gpiDrivers[i].size);
    }
}

uint8_t enableGpi(uint32_t gpi, uint8_t enable) {
    if(gpi >= zynGpiCount)
        return 0;
    getGpi(gpi).enabled = enable;
    return 1;
}

uint8_t isEnabled(uint32_t gpi) {
    if(gpi >= zynGpiCount)
        return 0;
    return getGpi(gpi).enabled;
}

uint32_t getCount() {
    return zynGpiCount;
}

uint32_t getEnabledCount() {
    uint32_t count = 0;
    for(int i = 0; i < zynGpiCount; ++i)
        if(isEnabled(i))
            ++count;
    return count;
}

uint8_t getDirection(uint32_t gpi) {
    if(gpi >= zynGpiCount)
        return 0;
    return getGpi(gpi).dir;
}

void setDirection(uint32_t gpi, uint8_t dir) {
    if(gpi >= zynGpiCount)
        return;
    if(gpiDrivers[gpimap[gpi].driver].setDirection)
        gpiDrivers[gpimap[gpi].driver].setDirection(gpi, dir);
}

void setPull(uint32_t gpi, uint8_t mode) {
    if(gpi >= zynGpiCount)
        return;
    if(gpiDrivers[gpimap[gpi].driver].setPull)
        gpiDrivers[gpimap[gpi].driver].setPull(gpi, mode);
}

uint8_t getState(uint32_t gpi) {
    if(gpi >= zynGpiCount)
        return 0;
    return getGpi(gpi).value;
}

void setState(uint32_t gpi, uint8_t state) {
    if(gpi >= zynGpiCount)
        return;
    gpiDrivers[gpimap[gpi].driver].setState(gpimap[gpi].offset, state?1:0);
    getGpi(gpi).value = state; //!@todo Move this to device specific to ensure the state is correct
}

void removeGpiDevice(uint32_t driver) {
    if(driver >= MAX_GPI_DRIVERS)
        return;

    uint32_t offset = gpiDrivers[driver].offset;
    uint32_t size = gpiDrivers[driver].size;

    // Call driver specific code
    if(gpiDrivers[driver].destroy)
        gpiDrivers[driver].destroy();

    // Recover memory
    free(gpiDrivers[driver].gpis);

    // Move drivers to fill the gap
    for(int i = driver; i < MAX_GPI_DRIVERS - 1; ++i) {
        // Iterate drivers from requested removal
        gpiDrivers[i] = gpiDrivers[i + 1];
        if(gpiDrivers[i].type == GPI_DRIVER_NONE)
            break;
        gpiDrivers[i].offset -= size;
    }
    resetDriver(MAX_GPI_DRIVERS - 1); // Last driver must be empty

    for(int i = offset; i < offset + size * 2; ++i) {
        gpimap[i] = gpimap[i + size]; // Implicit struct copy
        gpimap[i].offset -= size;
    }
    zynGpiCount -= size;
    //!@todo Update polling if required
    //!@todo Close I2C device if required
}

//  Thread to poll GPI
void * poll_gpi(void *arg) {
	while (1) {
		for(int i = 0; i < MAX_GPI_DRIVERS; ++i) {
            if(gpiDrivers[i].poll)
                gpiDrivers[i].poll(i);
		}
		usleep(POLL_SLEEP_US);
	}
	return NULL;
}
