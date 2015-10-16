/*
 * FlashFileIndexSystem.h
 *
 *  Created on: Sep 29, 2015
 *      Author: Robo
 */

#ifndef FFIS_FLASHFILEINDEXSYSTEM_H_
#define FFIS_FLASHFILEINDEXSYSTEM_H_

#include <stdint.h>

// Sector number at which index info is written (for some hardware (like Spansion), this might not be flexible)
#define FFIS_INDEX_SECTOR_NUM		(0)

//#define FFIS_MAX_NUM_OF_FILES 		(6)

// Sector number at which file data starts
#define FFIS_FILEDATA_SECTOR_NUM	(1)

// Nummber of extra sectors after each file just to allow small expansion in size
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
	ERASE_BEFORE_WRITE,
	NO_ERASE_BEFORE_WRITE
} writeMode;

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
FFISretVal fileWrite(FlashHW *flashObj, fileIndexEntry* entry, uint8_t *data, uint16_t len, uint16_t *bw);
FFISretVal fileRead(FlashHW *flashObj, fileIndexEntry* entry, uint8_t *data, uint16_t len, uint16_t *br);
FFISretVal fileSeek(fileIndexEntry *entry, uint32_t offset);
FFISretVal fileErase(FlashHW *flashObj, fileIndexEntry *entry, uint32_t len);		// This is NOT file delete. See function definition
FFISretVal fileWriteNoErase(FlashHW *flashObj, fileIndexEntry *entry, uint8_t *data, uint16_t len, uint16_t *bw);

#endif /* FFIS_FLASHFILEINDEXSYSTEM_H_ */
