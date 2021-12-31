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
#ifndef ZYNGPI_H_INCLUDED
#define ZYNGPI_H_INCLUDED

#define MAX_GPI_DRIVERS         8 //!@todo Make this dynamic
#define MAX_GPI                 256 //!@todo Make this dynamic
#define POLL_SLEEP_US           10000 //!@todo Make this tunable

/*  List of GPI driver types */
#define GPI_DRIVER_NONE         0
#define GPI_DRIVER_RPI          1
#define GPI_DRIVER_MCP23008     2
#define GPI_DRIVER_MCP23017     3
#define GPI_DRIVER_RIBAN_I2C    4

#include "stdint.h" // Provides fixed width interger types
#include "stdio.h" // Provides NULL
#include <stdlib.h> // Provides free,malloc, etc
#include <unistd.h> // Provides usleep

#define INPUT       0
#define OUTPUT      1
#define PUD_OFF     0
#define PUD_DOWN    1
#define PUD_UP      2

 /* Helper functions */
#define getGpi(index) gpiDrivers[gpimap[index].driver].gpis[gpimap[index].offset]
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

//  Structure describing an individual GPI
typedef struct gpi_t {
    uint8_t value:1;        // Current state / value [0|1]
    uint8_t enabled:1;      // 1 if enabled
    uint8_t dir:1;          // Input / output
} gpi_t;

//  Structure describing GPI driver
typedef struct gpi_driver_t {
    uint8_t type;           // Driver type
    uint32_t size;          // Quantity of GPI provided by driver
    uint32_t offset;        // Index of first GPI in global driver map
    void* config;           // Pointer to device specific structure holding device configuration parameters
    gpi_t* gpis;            // Dynamic "array" of gpi structures

    // Driver specific functions
    void(*setPull)(uint32_t gpi, uint8_t mode);         // Set GPI pull up/down mode
    void(*setState)(uint32_t gpi, uint8_t state);   // Set GPI state
    void(*destroy)();                               // Function called when driver removed
    void(*setDirection)(uint32_t gpi, uint8_t dir); // Function to set GPI direction
    uint8_t(*poll)(uint32_t);                       // Function to poll GPI states, NULL for no polling
} gpi_driver_t;

//  Structure describing map of GPI index to its driver
typedef struct {
    uint8_t driver;         // Index of driver
    uint32_t offset;        // Offset of GPI within driver
} gpi_map_t;

extern gpi_driver_t gpiDrivers[]; // Map of driver structures mapped by global GPI driver number
extern gpi_map_t gpimap[];  // Map of drivers,offset indexed by global GPI number
extern uint32_t zynGpiCount;      // Quantity of instantiated GPIs


/** @brief  Initialise GPI driver
*/
void init();

/** @brief  Enable / disable GPI
*   @param  gpi Index of GPI
*   @param  enable 1 to enable, 0 to disable
*   @retval uint8_t 1 on success, 0 on failure, i.e. invalid index
*   @note   All GPI are disabled by default and must be individually enabled
*/
uint8_t enableGpi(uint32_t gpi, uint8_t enable);

/** @brief  Check if GPI is enabled
*   @param  gpi Index of GPI
*   @retval uint8_t 1 if enabled, 0 if disabled or invalid index
*/
uint8_t isEnabled(uint32_t gpi);

/** @brief Get quantity of GPI
*   @retval uint32_t Quantity of GPI
*/
uint32_t getCount();

/** @brief Get quantity of enabled GPI
*   @retval uint32_t Quantity of enabled GPI
*/
uint32_t getEnabledCount();

/** @brief  Get GPI direction
*   @param  gpi Index of gpi
*   @retval uint8_t Direction of GPI, 0:Input, 1:Output
*/
uint8_t getDirection(uint32_t gpi);

/** @brief  Set GPI direction
*   @param  gpi Index of gpi
*   @param  dir Direction, 0:Input, 1:Output
*/
void setDirection(uint32_t gpi, uint8_t dir);

/** @brief  Set GPI pull up/down mode
*   @param  gpi Index of GPI within global gpimap
*   @param  dir mode [PUD_OFF|PUD_DOWN|PUD_UP]
*/
void setPull(uint32_t gpi, uint8_t dir);

/** @brief  Get GPI state
*   @param  gpi Index of gpi
*   @retval uint8_t State of GPI, [0 | 1]
*   @note   Returns 0 for invalid GPI index
*/
uint8_t getState(uint32_t gpi);

/** @brief  Set GPI state
*   @param  gpi Index of gpi
*   @param  state New GPI state [0 | 1]
*   @note   Non-zero value interpreted as 1
*   @note   Invalid gpi index silently ignored
*/
void setState(uint32_t gpi, uint8_t state);

/** @brief  Instantiate an instance of a MCP23088 GPI interface driver providing 8 GPI pins
*   @param  address I2C address
*   @retval int Index of new GPI driver or -1 on failure
*   @note   Index of GPI depends on order of instantiation
*/
int addMcp23008GpiDevice(uint8_t I2C);

/** @brief  Instantiate an instance of a riban I2C GPI interface driver providing 50 GPI pins
*   @param  address I2C address
*   @retval int Index of new GPI driver or -1 on failure
*   @note   Index of GPI depends on order of instantiation
*/
int addRibanGpiDevice(uint8_t I2C);

//-----------------------------------------------------------------------------
#endif // ZYNGPI_H_INCLUDED
