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
#ifndef ZYNMCP23017GPI_H_INCLUDED
#define ZYNMCP23017GPI_H_INCLUDED

#include "gpi.h"

/*  MCP21017 Registers
    IOCON.BANK=1    IOCON.BANK=0    Register
        00h             00h          IODIRA
        10h             01h          IODIRB
        01h             02h          IPOLA
        11h             03h          IPOLB
        02h             04h          GPINTENA
        12h             05h          GPINTENB
        03h             06h          DEFVALA
        13h             07h          DEFVALB
        04h             08h          INTCONA
        14h             09h          INTCONB
        05h             0Ah          IOCON
        15h             0Bh          IOCON
        06h             0Ch          GPPUA
        16h             0Dh          GPPUB
        07h             0Eh          INTFA
        17h             0Fh          INTFB
        08h             10h          INTCAPA
        18h             11h          INTCAPB
        09h             12h          GPIOA
        19h             13h          GPIOB
        0Ah             14h          OLATA
        1Ah             15h          OLATB
    ---------------------------------------
    Bank=1, upper nibble of address defines port A[0] B[1]
    Address    Register
        0       IODIR
        1       IPOL
        2       GPINTEN
        3       DEFVAL
        4       INTCON
        5       IOCON (not port specific)
        6       GPPU
        7       INTF
        8       INTCAP
        9       GPIO
        A       OLAT

    I2C address range 0x20..0x27
    Power on:
        All GPI are non-inverted inputs
        Bank=0
*/

/*  Ensure GPI driver type is unique */
#define GPI_DRIVER_MCP23017     3

#define MCP23017_REG_IODIR      0x0
#define MCP23017_REG_IPOL       0x1
#define MCP23017_REG_GPINTEN    0x2
#define MCP23017_REG_DEFVAL     0x3
#define MCP23017_REG_INTCON     0x4
#define MCP23017_REG_IOCON      0x5
#define MCP23017_REG_GPPU       0x6
#define MCP23017_REG_INTF       0x7
#define MCP23017_REG_INTCAP     0x8
#define MCP23017_REG_GPIO       0x9
#define MCP23017_REG_OLAT       0xA

/** @brief  Instantiate an instance of a MCP23017 GPI interface driver providing 16 GPI pins
*   @param  address I2C address
*   @param  interrupt GPI pin acting as interrupt signal
*   @retval int Index of new GPI driver or -1 on failure
*   @note   Index of GPI depends on order of instantiation
*/
int addMcp23017GpiDevice(uint8_t address, uint8_t interrupt);

/** @brief  Set GPI state
*   @param  gpi Index of GPI within global gpimap
*   @param  state New GPI state
*/
void setMcp23017GpiState(uint32_t gpi, uint8_t state);

/** @brief  Set GPI direction
*   @param  gpi Index of GPI within global gpimap
*   @param  dir Direction [0:Input, 1:Output]
*/
void setMcp23017GpiDirection(uint32_t gpi, uint8_t dir);

/** @brief  Set GPI pull up resistors
*   @param  gpi Index of GPI within global gpimap
*   @param  mode Pull mode [0:Disabled, 1:Pull-up enabled]
*/
void setMcp23017GpiPull(uint32_t gpi, uint8_t mode);

/** @brief  Poll for change of state
*   @param  gpi Index of GPI within global gpimap
*   @retval uint8_t 1 if any GPI within driver has changed else 0
*/
uint8_t pollMcp23017Gpi(uint32_t driver);
//-----------------------------------------------------------------------------
#endif // ZYNMCP23017GPI_H_INCLUDED
