/*
 * FlashFileIndexSystem.h
 *
 *  Created on: Sep 29, 2015
 *      Author: Robo
 */

#ifndef FFIS_FLASHFILEINDEXSYSTEM_H_
#define FFIS_FLASHFILEINDEXSYSTEM_H_

#include <stdint.h>


#define FFIS_INDEX_SECTOR_NUM		(0)
//#define FFIS_MAX_NUM_OF_FILES 		(6)
#define FFIS_FILEDATA_SECTOR_NUM	(1)
#define FILE_APPENDIX_SECTORS_CNT	(0)

typedef enum {
	FFIS_OK,
	FFIS_INIT_FAILED,
	FFIS_INVALID_PARAMS,
	FLASH_READ_ERROR,
	FLASH_WRITE_ERROR,
	FLASH_ERASE_ERROR,
	FILE_WRONG_CHECKOUT_MODE,
	FILE_DOES_NOT_EXIST,
	FLASH_NO_DEVICE_DETECTED

} FFISretVal;

typedef enum {
	READ,
	WRITE
} checkOutMode;

typedef enum {
	SECTOR,
	SUBSECTOR
} eraseMode;

typedef struct _fileIndexEntry {
	uint8_t fileID;
	uint8_t statFlag;
	uint16_t startSecNum;
	uint16_t secSpan;
	uint32_t startAddr;
	uint32_t fileSize;
	uint32_t bookMark;
} fileIndexEntry;

typedef struct _FlashHW {

	FFISretVal (*flashInitialize)(struct _FlashHW *flashObj);
	FFISretVal (*flashRead)(struct _FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
	FFISretVal (*flashWrite)(struct _FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len);
	FFISretVal (*flashEraseSector)(struct _FlashHW *flashObj, uint16_t startSecNum, uint16_t numOfSectors);
	FFISretVal (*flashEraseSubsector)(struct _FlashHW *flashObj, uint16_t startSubsecNum, uint16_t numOfSubsectors);
} FlashHW;

FFISretVal fileCheckOut(FlashHW *flashObj, uint8_t id, fileIndexEntry *entry, checkOutMode);
FFISretVal fileCheckIn(FlashHW *flashObj, fileIndexEntry *entry);
FFISretVal fileWrite(FlashHW *flashObj, fileIndexEntry* entry, uint8_t *data, int len, int *bw);
FFISretVal fileRead(FlashHW *flashObj, fileIndexEntry* entry, uint8_t *data, int len, int *br);


#endif /* FFIS_FLASHFILEINDEXSYSTEM_H_ */
