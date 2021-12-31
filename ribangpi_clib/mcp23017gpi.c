/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: GPI Library
 *
 * Library for interfacing MCP23017 GPI with Zynthian
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

#include "mcp23017gpi.h"
#include "i2c.h" // Provides I2C interface

//  Structure describing MCP23017 GPI driver config
typedef struct mcp23017gpidata_t {
    uint8_t address;    // I2C address
    uint8_t interrupt;  // GPI pin of interrupt
} mcp23017gpidata_t;

/*  Private helper functions */
mcp23017gpidata_t* getMcp23017Config(uint8_t driver); // Get a pointer to the driver's config data or NULL for invalid driver


uint8_t readMcp23017Register(uint8_t address, uint8_t reg) {
    if(i2cGetFd() < 0)
        return 0;
    i2cSelectDevice(address);
    i2cWriteByte(0x40 | address << 1);
    i2cWriteByte(reg);
    return i2cReadByte();
}

void writeMcp23017Register(uint8_t address, uint8_t reg, uint8_t val) {
    if(i2cGetFd() < 0)
        return;
    i2cSelectDevice(address);
    i2cWriteByte(0x40 | address << 1);
    i2cWriteByte(reg);
    i2cWriteByte(val);
}

int addMcp23017GpiDevice(uint8_t address, uint8_t interrupt) {
    if(address < 0x20 || address > 0x27)
        return -1;
    uint8_t driverCount;
    for(driverCount = 0; driverCount < MAX_GPI_DRIVERS; ++driverCount) {
        if(gpiDrivers[driverCount].type == GPI_DRIVER_NONE)
            break;
        if(gpiDrivers[driverCount].type == GPI_DRIVER_MCP23017 && getMcp23017Config(driverCount)->address == address)
            return driverCount;
    }
    if(driverCount >= MAX_GPI_DRIVERS)
        return -1;
    if(i2cOpen() < 0)
        return -1;
    // Configure MCP23017
    writeMcp23017Register(address, MCP23017_REG_IOCON, 0b11100000);

    gpi_driver_t* driver = &gpiDrivers[driverCount];
    driver->type = GPI_DRIVER_MCP23017;
    driver->size = 16; // Device specific size
    driver->offset = zynGpiCount;
    driver->config = (uint8_t*)malloc(sizeof(mcp23017gpidata_t));
    getMcp23017Config(driverCount)->address = address;
    getMcp23017Config(driverCount)->interrupt = interrupt;
    driver->setState = setMcp23017GpiState;
    driver->setDirection = setMcp23017GpiDirection;
    driver->setPull = setMcp23017GpiPull;
    driver->gpis = (gpi_t*) malloc (driver->size * sizeof(gpi_t));
    for(int i  = 0; i < driver->size; ++i) {
        (driver->gpis[i]).value = 0;
        (driver->gpis[i]).enabled = 0;
        (driver->gpis[i]).dir = 0;
        gpimap[zynGpiCount].driver = driverCount;
        gpimap[zynGpiCount++].offset = i;
        setMcp23017GpiDirection(i, INPUT);
    }
    return driverCount;
}

mcp23017gpidata_t* getMcp23017Config(uint8_t driver) {
    if(driver >= MAX_GPI_DRIVERS || gpiDrivers[driver].type != GPI_DRIVER_MCP23017)
        return 0;
    return (mcp23017gpidata_t*)gpiDrivers[driver].config;
}

void setMcp23017GpiState(uint32_t gpi, uint8_t state) {
    //!@todo Validate GPI enabled and direction=output
    uint32_t offset = gpimap[gpi].offset;
    uint32_t driver = gpimap[gpi].driver;
    uint8_t address = getMcp23017Config(driver)->address;
    uint8_t reg = (offset | 0x08) ? MCP23017_REG_GPIO | 0x10 : MCP23017_REG_GPIO;
    uint8_t bit = offset | 0x07;
    uint8_t value = readMcp23017Register(address, reg);
    if(state)
        writeMcp23017Register(address, reg, bitClear(value, bit));
    else
        writeMcp23017Register(address, reg, bitSet(value, bit));
    getGpi(gpi).value = state?1:0; // Update value upon success
}

void setMcp23017GpiDirection(uint32_t gpi, uint8_t dir) {
    // Range check already performed by GPI library function
    uint32_t offset = gpimap[gpi].offset;
    uint32_t driver = gpimap[gpi].driver;
    uint8_t address = getMcp23017Config(driver)->address;
    uint8_t reg = (offset | 0x08) ? MCP23017_REG_IODIR | 0x10 : MCP23017_REG_IODIR;
    uint8_t bit = offset | 0x07;
    uint8_t value = readMcp23017Register(address, reg);
    if(dir)
        writeMcp23017Register(address, reg, bitClear(value, bit));
    else
        writeMcp23017Register(address, reg, bitSet(value, bit));
    getGpi(gpi).dir = dir?1:0; // Update value upon success
}

void setMcp23017GpiPull(uint32_t gpi, uint8_t mode) {
    // Range check already performed by GPI library function
    if(mode == PUD_DOWN)
        return; // MCP23017 does not support pull down
    uint32_t offset = gpimap[gpi].offset;
    uint32_t driver = gpimap[gpi].driver;
    uint8_t address = getMcp23017Config(driver)->address;
    uint8_t reg = (offset | 0x08) ? MCP23017_REG_GPPU | 0x10 : MCP23017_REG_GPPU;
    uint8_t bit = offset | 0x07;
    uint8_t value = readMcp23017Register(address, reg);
    if(mode)
        writeMcp23017Register(address, reg, bitSet(value, bit));
    else
        writeMcp23017Register(address, reg, bitClear(value, bit));
}

