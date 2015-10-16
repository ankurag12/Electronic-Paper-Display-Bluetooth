/*
 * FlashFileIndexSystem.c
 *
 *  Created on: Sep 30, 2015
 *      Author: Robo
 */

/* One sector is dedicated for the file index, and each file starts from a new sector.
 * No two files share the same sector.
 * In order to save RAM, each new index entry is made on a new sub sector of the index sector,
 * (until all subsectors are filled, after which next entry is made in the first subsector and so on).
 * This way, less RAM is consumed in making a backup of the index before updating it.
 * If all entries were made sequentially, the whole index would need to be backed up.
 * Just to recall, flash cannot have overwrite, but the whole (sub)sector needs to be erased and written again,
 * and so a backup must be made in RAM if only a part of the (sub)sector needs to be overwritten.
 *
 * Each file is stored in contigous sectors, so the amount by which a file size could be expanded really depends
 * on if the file is the last file (expandable to the flash size limit),
 * or the amount of space left in the last sector of the file.
 * There is no check implemented yet if file size expansion is attempted over the limit,
 * and doing so will silently overwrite the file in the next sector(s).
 * Therefore, file size should not be increased, unless the file in question is the last file.
 * If the application requires too much variation in file size, add spare sectors between files
 * by changing definition FILE_APPENDIX_SECTORS_CNT.
 *
 * At file checkout, index entry is read of the file with the specified ID.
 * If the file does not exist and checkout is in READ mode, error is thrown,
 * If the file does not exist and checkout is in WRITE mode,
 * a new index entry is assigned, but that is still in RAM.
 * File must be checked in to update the index saved on flash.
 */

#include "FlashFileIndexSystem.h"
#include <stdint.h>
#include "FFIS_HAL.h"
#include <stdlib.h>

FFISretVal fileCheckOut(FlashHW *flashObj, uint8_t id, fileIndexEntry *entry, checkOutMode mode) {
	uint8_t nextID = 0;
	fileIndexEntry lastEntry;
	uint32_t currAddr = (uint32_t)FFIS_INDEX_SECTOR_NUM*(uint32_t)FLASH_SECTOR_SIZE;
	uint32_t nextAddr = (uint32_t)FFIS_INDEX_SECTOR_NUM*(uint32_t)FLASH_SECTOR_SIZE;
	uint8_t rbuffer[sizeof(fileIndexEntry)];
	
	if(flashObj->flashRead(flashObj, currAddr, &nextID, sizeof(nextID)))
		return FLASH_READ_ERROR;
		
	do {
			
		if(nextID == id) {
			if(flashObj->flashRead(flashObj, nextAddr, rbuffer, sizeof(fileIndexEntry)))
				return FLASH_READ_ERROR;
			memcpy(entry, rbuffer, sizeof(fileIndexEntry));	// Don't care about padding, we have sufficient space
			entry->bookMark=entry->startAddr;
			if(mode == WRITE)
				entry->fileSize = 0;
			break;
		}
		
		if(nextID == 0xFF) {
			if(mode == READ)
				return FILE_DOES_NOT_EXIST;

			if(flashObj->flashRead(flashObj, currAddr, rbuffer, sizeof(fileIndexEntry)))
				return FLASH_READ_ERROR;
			memcpy(&lastEntry, rbuffer, sizeof(fileIndexEntry));	// Don't care about padding, we have sufficient space
			if (lastEntry.fileID == 0xFF) {		// Case for writing the first file
				entry->fileID = id;
				entry->startSecNum = FFIS_FILEDATA_SECTOR_NUM;
				entry->secSpan = 1;
				entry->startAddr = FFIS_FILEDATA_SECTOR_NUM*FLASH_SECTOR_SIZE;
				entry->fileSize = 0;
				entry->bookMark = entry->startAddr + entry->fileSize;
			}
			else {
				entry->fileID = id;
				entry->startSecNum = lastEntry.startSecNum + lastEntry.secSpan + FILE_APPENDIX_SECTORS_CNT;
				entry->secSpan = 1;
				entry->startAddr = (entry->startSecNum)*FLASH_SECTOR_SIZE;
				entry->fileSize = 0;
				entry->bookMark = entry->startAddr + entry->fileSize;
			}
			
			break;
		}
		currAddr = nextAddr;
		nextAddr = currAddr + FLASH_SUBSECTOR_SIZE;
		if(nextAddr>=(FFIS_INDEX_SECTOR_NUM+1)*FLASH_SECTOR_SIZE)
			nextAddr -=  FLASH_SECTOR_SIZE + sizeof(fileIndexEntry);
			
		if(flashObj->flashRead(flashObj, nextAddr, &nextID, sizeof(nextID)))
			return FLASH_READ_ERROR;
		
	} while(1);
	

	// If WRITE mode, then index needs to be updated at check-in (irrespective of whether a new entry or rewrite on old)
	// Also, if WRITE mode, then old file's sectors must be erased (editing file is not implemented; only full replacement)
	if(mode == WRITE)
		entry->statFlag=0;
	else if(mode == READ)
		entry->statFlag=1;
	
	return FFIS_OK;

		
}

// Check-in basically updates the index

FFISretVal fileCheckIn(FlashHW *flashObj, fileIndexEntry *entry) {
	uint8_t nextID = 0;
	uint32_t currAddr = (uint32_t)FFIS_INDEX_SECTOR_NUM*(uint32_t)FLASH_SECTOR_SIZE;
	uint16_t subSecNum = (uint16_t)FFIS_INDEX_SECTOR_NUM*(uint16_t)FLASH_SUBSECTORS_PER_SECTOR;
	int indexCnt = 0;
	int indexPos = 0;
	fileIndexEntry	*indexBackup;


	do {
		if(flashObj->flashRead(flashObj, currAddr, &nextID, sizeof(nextID)))
			return FLASH_READ_ERROR;

		if(nextID == 0xFF) {

			if(entry->statFlag==0) {					// Make a new entry

				entry->statFlag=0xFF;
				entry->bookMark = entry->startAddr;

				if(currAddr%FLASH_SUBSECTOR_SIZE==0) {
					if(flashObj->flashEraseSubsector(flashObj, (uint16_t)(currAddr/FLASH_SUBSECTOR_SIZE), 1))	// Not really required, just to be safe
						return FLASH_ERASE_ERROR;
				}
				if(flashObj->flashWrite(flashObj, currAddr, (uint8_t*)(entry), sizeof(fileIndexEntry)))
					return FLASH_WRITE_ERROR;

				break;
			}
			else
				return FILE_DOES_NOT_EXIST;
		}


		if(nextID == entry->fileID) {

			if(entry->statFlag==1)		// READ mode, do nothing
				return FFIS_OK;

			indexPos = (int)((currAddr%FLASH_SUBSECTOR_SIZE)/sizeof(fileIndexEntry));		// Position of index of interest within the subsector
			indexCnt=indexPos;
			do {
				indexCnt++;
				currAddr+=sizeof(fileIndexEntry);
				if(flashObj->flashRead(flashObj, currAddr, &nextID, sizeof(nextID)))
					return FLASH_READ_ERROR;

			} while(nextID!=0xFF);


			indexBackup = (fileIndexEntry*) malloc(sizeof(fileIndexEntry)*indexCnt);	// MALLOC USED HERE !!

			currAddr = (uint32_t)(subSecNum*FLASH_SUBSECTOR_SIZE);

			if(flashObj->flashRead(flashObj, currAddr, (uint8_t*)(indexBackup), sizeof(fileIndexEntry)*indexCnt))
				return FLASH_READ_ERROR;

			entry->statFlag=0xFF;
			entry->bookMark = entry->startAddr;
			memcpy(&indexBackup[indexPos], entry, sizeof(fileIndexEntry));

			if(flashObj->flashEraseSubsector(flashObj, (uint16_t)(currAddr/FLASH_SUBSECTOR_SIZE), 1))
				return FLASH_ERASE_ERROR;

			if(flashObj->flashWrite(flashObj, currAddr, (uint8_t*)(indexBackup), sizeof(fileIndexEntry)*indexCnt))
				return FLASH_WRITE_ERROR;
			
			free(indexBackup);
			break;

		}

		currAddr += FLASH_SUBSECTOR_SIZE;
		subSecNum++;
		if(currAddr>=(FFIS_INDEX_SECTOR_NUM+1)*FLASH_SECTOR_SIZE) {
			currAddr =  currAddr - FLASH_SECTOR_SIZE + sizeof(fileIndexEntry);
			subSecNum -= FLASH_SUBSECTORS_PER_SECTOR;
		}

	} while(1);

	return FFIS_OK;

}



FFISretVal fileWrite(FlashHW *flashObj, fileIndexEntry *entry, uint8_t *data, uint16_t len, uint16_t *bw) {
	uint16_t eraseSecSt;
	uint8_t xSecBndry;
	if(entry->statFlag!=0)
		return FILE_WRONG_CHECKOUT_MODE;
	
	xSecBndry = (uint16_t)((entry->bookMark + len)/FLASH_SECTOR_SIZE) - (uint16_t)((entry->bookMark -1)/FLASH_SECTOR_SIZE);
	eraseSecSt = (uint16_t)((entry->bookMark + len)/FLASH_SECTOR_SIZE);
	if(xSecBndry>0) {
		if(flashObj->flashEraseSector(flashObj, eraseSecSt, xSecBndry))
			return FLASH_ERASE_ERROR;
	}
	
	if(flashObj->flashWrite(flashObj, entry->bookMark, data, len))
		return FLASH_WRITE_ERROR;
	*bw = len;	
		
	entry->bookMark+=len;
	entry->fileSize+=len;
	entry->secSpan = (uint16_t)((entry->startAddr+entry->fileSize-1)/FLASH_SECTOR_SIZE) - (uint16_t)((entry->startAddr)/FLASH_SECTOR_SIZE)+1;

	return FFIS_OK;

}


FFISretVal fileRead(FlashHW *flashObj, fileIndexEntry *entry, uint8_t *data, uint16_t len, uint16_t *br) {
	
	if(entry->statFlag!=1)				// Not in read mode
		return FILE_WRONG_CHECKOUT_MODE;
		
	if (entry->bookMark + len > entry->startAddr + entry->fileSize)
		len = entry->startAddr + entry->fileSize - entry->bookMark;
	
	if(flashObj->flashRead(flashObj, entry->bookMark, data, len))
		return FLASH_READ_ERROR;
		
	*br = len;
	entry->bookMark+=len;
	
	return FFIS_OK;
	
}

FFISretVal fileSeek(fileIndexEntry *entry, uint32_t offset) {

	if(entry->statFlag==0xFF)							// Not checked out
		return FILE_WRONG_CHECKOUT_MODE;

	if(entry->statFlag==1 && offset > entry->fileSize)	// Reading over file limit (writing is allowed)
		return FFIS_INVALID_PARAMS;

	entry->bookMark = entry->startAddr + offset;

	return FFIS_OK;

}

/* fileErase() DOES NOT delete a file from the index, it only erases the content sectors
 * (from start of file for len bytes) so that the next write
 * can be fast in critical situations like writing a file received over bluetooth.
 * To erase a file, it must be checked out in WRITE mode, and checked back in after erase operation
 * This function is equivalent to writing 0xFFFF...... on the file.
 */

FFISretVal fileErase(FlashHW *flashObj, fileIndexEntry *entry, uint32_t len) {

	uint16_t eraseSecSt = entry->startSecNum;
	uint16_t secCount = (len - 1)/FLASH_SECTOR_SIZE + 1;

	if(entry->statFlag!=0)
		return FILE_WRONG_CHECKOUT_MODE;

	if(flashObj->flashEraseSector(flashObj, eraseSecSt, secCount))
		return FLASH_ERASE_ERROR;

	entry->bookMark=entry->startAddr + len;

	if(len>entry->fileSize)
		entry->fileSize = len;

	entry->secSpan = (uint16_t)((entry->startAddr+entry->fileSize-1)/FLASH_SECTOR_SIZE) - (uint16_t)((entry->startAddr)/FLASH_SECTOR_SIZE)+1;

	return FFIS_OK;

}

/* This file write function assumes that the flash area has already been erased (by flashErase())
 * so that the write operation is very fast. This is useful in critical situations like
 * writing file received over BT.
 */

FFISretVal fileWriteNoErase(FlashHW *flashObj, fileIndexEntry *entry, uint8_t *data, uint16_t len, uint16_t *bw) {

	if(entry->statFlag!=0)
		return FILE_WRONG_CHECKOUT_MODE;

	if(flashObj->flashWrite(flashObj, entry->bookMark, data, len))
		return FLASH_WRITE_ERROR;
	*bw = len;

	entry->bookMark+=len;
	entry->fileSize+=len;
	entry->secSpan = (uint16_t)((entry->startAddr+entry->fileSize-1)/FLASH_SECTOR_SIZE) - (uint16_t)((entry->startAddr)/FLASH_SECTOR_SIZE)+1;

	return FFIS_OK;

}
