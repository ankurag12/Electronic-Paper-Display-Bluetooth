/*
 * msp430_spi.h
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */

#ifndef MSP430_SPI_H_
#define MSP430_SPI_H_

#include <stdint.h>
void flash_spi_init(void);

void flash_setCSHigh(void);
void flash_setCSLow(void);
void flash_CheckBusy(void);
void flash_spi_send_bytes(uint8_t *bytes, uint16_t len);
void flash_spi_read_bytes(uint8_t *bytes, uint16_t len);

extern struct pl_platform g_plat;

#endif /* MSP430_SPI_H_ */
