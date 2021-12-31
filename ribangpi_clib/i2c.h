/*
 * ******************************************************************
 * ZYNTHIAN PROJECT: I2C Library
 *
 * Library for interfacing I2C with Zynthian
 *
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
#ifndef ZYNI2C_H_INCLUDED
#define ZYNI2C_H_INCLUDED

#include <stdint.h> //Provides fixed width integer definitions
#include <sys/ioctl.h> //Provides device driver i/o control
#include <linux/i2c-dev.h> //Provides userspace i2c interface
#include <fcntl.h> //Provides file open
#include <unistd.h> //Provides file close

/** @brief  Get file descriptor of I2C device
*   @retval int File descriptor or negative number if closed
*/
int i2cGetFd();

/** @brief  Open I2C device "/dev/i2c-1"
*   @retval int File descriptor or negative error
*   @note   Limited to RPI onboard I2C interface (>=V2)
*/
int i2cOpen();

/** @brief  Close I2C device
*/
void i2cClose();

/** @brief  Select remote I2C device to communicate with
*   @param  address I2C address of remote device
*   @retval int 0 on success or negative error
*/
int i2cSelectDevice(uint8_t address);

/** @brief  Write a single byte to the selected remote I2C device
*   @param  value Value to write
*/
void i2cWriteByte(uint8_t value);

/** @brief  Read a single byte from the selected remote I2C device
*   @retval uint8_t Value read from remote I2C device
*/
uint8_t i2cReadByte();

/** @brief  Read a register from MCP23017 device
*   @param  address I2C address of MCP23017 [0x20..0x27]
*   @param  reg Index of register to read
*   @retval uint8_t Value in register
*/
uint8_t i2cReadMcp23017Register(uint8_t address, uint8_t reg);

#endif // ZYNI2C_H_INCLUDED
