/*
 * FFIS_HAL.h
 *
 *  Created on: Oct 1, 2015
 *      Author: Robo
 */

#ifndef FFIS_FFIS_HAL_H_
#define FFIS_FFIS_HAL_H_

#include "FlashFileIndexSystem.h"

// Flash Hardware specific

//#define USE_MICRON_FLASH
#define USE_SPANSION_FLASH

#define FLASH_NUMBER_OF_SECTORS		(256)
#define FLASH_SUBSECTOR_SIZE		(4096)
#define FLASH_SECTOR_SIZE			(65536)
#define FLASH_SUBSECTORS_PER_SECTOR (16)

#ifdef USE_MICRON_FLASH
static FFISretVal flashRead_Micron(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
static FFISretVal flashWrite_Micron(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
static FFISretVal flashEraseSector_Micron(FlashHW *flashObj, uint16_t startSecNum, uint16_t numOfSectors);
static FFISretVal flashEraseSubsector_Micron(FlashHW *flashObj, uint16_t startSubsecNum, uint16_t numOfSubsectors);
static FFISretVal flashInitialize_Micron(FlashHW *flashObj);
#endif

#ifdef  USE_SPANSION_FLASH
static FFISretVal flashRead_Spansion(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
static FFISretVal flashWrite_Spansion(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
static FFISretVal flashEraseSector_Spansion(FlashHW *flashObj, uint16_t startSecNum, uint16_t numOfSectors);
static FFISretVal flashEraseSubsector_Spansion(FlashHW *flashObj, uint16_t startSubsecNum, uint16_t numOfSubsectors);
static FFISretVal flashInitialize_Spansion(FlashHW *flashObj);
#endif

FFISretVal FFIS_init(FlashHW *flashObj);


#endif /* FFIS_FFIS_HAL_H_ */
