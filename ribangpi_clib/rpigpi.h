/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: GPI Library
 *
 * Library for interfacing naitive RPI GPI with Zynthian
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

/*  Raspberry Pi uses BCM2835 (and similar) SoC with 54 multifunction GPI pins

    GPI 0,1 are used as I2C bus 0 to read hat EEPROMs and should not be used
    GPI 2,3 may be used  as I2C bus 1 - Zynthian uses this bus to access I2C devices
        Note: GPI 3 may also be used to boot device
    GPI 4-13 are available
    GPI 14,15 configured as UART 0 are used for MIDI
    GPI 16-17 are available
    GPI 18-21 configured as I2S are used for audio
    GPI 22-27 are available
    This library exposes GPI 0-32 but disables access to 0,1,28,29,30,31 - user must be careful of other used pins
*/

#ifndef ZYNRPIGPI_H_INCLUDED
#define ZYNRPIGPI_H_INCLUDED

#include "gpi.h"

/*  Ensure GPI driver type is unique */
#define GPI_DRIVER_RPI          1

/** @brief  Instantiate an instance of a naitive Raspberry Pi GPI interface driver providing 16 GPI pins
*   @retval int Index of new GPI driver or -1 on failure
*   @note   Index of GPI depends on order of instantiation
*   @todo   Maybe this is always instantiated
*/
int addRpiGpiDevice();

/** @brief  Device specific action called during driver removal
*/
void destroyRpiGpiDevice();

/** @brief  Set GPI state
*   @param  gpi Index of GPI within global gpimap
*   @param  state New GPI state
*/
void setRpiGpiState(uint32_t gpi, uint8_t state);

/** @brief  Get GPI state
*   @param  gpi Index of GPI within global gpimap
*   @retval uint8_t GPI state
*/
uint8_t getRpiGpiState(uint32_t gpi);

/** @brief  Set GPI direction
*   @param  gpi Index of GPI within global gpimap
*   @param  dir Direction [0:Input, 1:Output]
*/
void setRpiGpiDirection(uint32_t gpi, uint8_t dir);

/** @brief  Set GPI pull up/down mode
*   @param  gpi Index of GPI within global gpimap
*   @param  dir mode [PUD_OFF|PUD_DOWN|PUD_UP]
*/
void setRpiGpiPull(uint32_t gpi, uint8_t mode);

/** @brief  Poll for change of state
*   @param  gpi Index of GPI within global gpimap
*   @retval uint8_t 1 if any GPI within driver has changed else 0
*/
uint8_t pollRpiGpi(uint32_t driver);


//-----------------------------------------------------------------------------
#endif // ZYNRPIGPI_H_INCLUDED
