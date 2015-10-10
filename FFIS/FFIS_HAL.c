#include <stdint.h>

#include "FlashFileIndexSystem.h"

#include <stdio.h>
#include <string.h>
#include "FFIS_HAL.h"

#include "Flash_SPI_LowLevel.h"

#ifdef USE_MICRON_FLASH
#include "../Micron/N25Q.h"
#include "../Micron/Serialize.h"
static FLASH_DEVICE_OBJECT flashDeviceObject;
#endif

#ifdef USE_SPANSION_FLASH
#include "../Spansion/slld.h"
#endif

FFISretVal FFIS_init(FlashHW *flashObj)
{

	//FFISretVal retVal;

	flash_spi_init();		// Initialize SPI Communication (See Flash_SPI_LowLevel.c)

#ifdef USE_MICRON_FLASH

	flashObj->flashInitialize = flashInitialize_Micron;
	flashObj->flashRead = flashRead_Micron;
	flashObj->flashWrite = flashWrite_Micron;
	flashObj->flashEraseSector = flashEraseSector_Micron;
	flashObj->flashEraseSubsector = flashEraseSubsector_Micron;

#endif

#ifdef USE_SPANSION_FLASH

	flashObj->flashInitialize = flashInitialize_Spansion;
	flashObj->flashRead = flashRead_Spansion;
	flashObj->flashWrite = flashWrite_Spansion;
	flashObj->flashEraseSector = flashEraseSector_Spansion;
	flashObj->flashEraseSubsector = flashEraseSubsector_Spansion;

#endif

	if(flashObj->flashInitialize(flashObj))
		return FFIS_INIT_FAILED;

	return FFIS_OK;

}


#ifdef USE_SPANSION_FLASH

static FFISretVal flashRead_Spansion(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len)
{
	SLLD_STATUS retv;
	if(retv = slld_ReadOp(addr, buff, len)) {
		//printf("Error (%d) in read operation. \r\n", retv);
		return FLASH_READ_ERROR;
	}
	return FFIS_OK;
}

static FFISretVal flashWrite_Spansion(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len)
{
	SLLD_STATUS retv;
	DEVSTATUS devStat;
	if(retv = slld_BufferedProgramOp(addr, buff, len, &devStat)) {
		//printf("Error (%d) in buffered write operation. \r\n", retv);
		return FLASH_WRITE_ERROR;
	}
	return FFIS_OK;
}

static FFISretVal flashEraseSector_Spansion(FlashHW *flashObj, uint16_t startSecNum, uint16_t numOfSectors)
{
	int i;
	SLLD_STATUS retv;
	DEVSTATUS devStat;
	if((startSecNum + numOfSectors)>FLASH_NUMBER_OF_SECTORS)
		return FFIS_INVALID_PARAMS;
	for (i=0; i<numOfSectors; i++) {
		if(retv = slld_SEOp((startSecNum + i)*FLASH_SECTOR_SIZE, &devStat)) {
			//printf("Error (%d) in sector erase operation at sector num (%d). \r\n", retv, i);
			return FLASH_ERASE_ERROR;
		}
	}

	return FFIS_OK;
}

static FFISretVal flashEraseSubsector_Spansion(FlashHW *flashObj, uint16_t startSubsecNum, uint16_t numOfSubsectors)
{
	int i;
	SLLD_STATUS retv;
	DEVSTATUS devStat;
	if((startSubsecNum + numOfSubsectors)>FLASH_SUBSECTORS_PER_SECTOR)
		return FFIS_INVALID_PARAMS;
	for (i=0; i<numOfSubsectors; i++) {
		if(retv = slld_P4EOp((startSubsecNum + i)*FLASH_SUBSECTOR_SIZE, &devStat)) {
			//printf("Error (%d) in sub-sector erase operation at sub-sector num (%d). \r\n", retv, i);
			return FLASH_ERASE_ERROR;
		}
	}

	return FFIS_OK;

}

static FFISretVal flashInitialize_Spansion(FlashHW *flashObj)
{

	BYTE devID[2];
	SLLD_STATUS retv;
	if(retv = slld_Read_IdentificationCmd(devID, 0x0)) {
		//printf("Error (%d) in executing slld_Read_IdentificationCmd. \r\n", retv);
		return FLASH_NO_DEVICE_DETECTED;
	}
	if(devID[0]!=0x01) {
		//printf("devID[0] != 0x01 ! \r\n");
		return FLASH_NO_DEVICE_DETECTED;
	}

	// Add more commands to set sector size, flash size, etc. from the device directly, rather than setting as macros
	return FFIS_OK;
}
#endif


#ifdef USE_MICRON_FLASH

static FFISretVal flashRead_Micron(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len)
{
	ParameterType para;
	para.Read.udAddr = addr;
	para.Read.pArray = buff;
	para.Read.udNrOfElementsToRead = len;
	if(flashDeviceObject.GenOp.DataRead(Read, &para)!=Flash_Success)
		return FLASH_READ_ERROR;
	return FFIS_OK;

}

static FFISretVal flashWrite_Micron(FlashHW *flashObj, uint32_t addr, uint8_t *buff, uint32_t len)
{
	ParameterType para;
	para.PageProgram.udAddr = addr;
	para.PageProgram.pArray = buff;
	para.PageProgram.udNrOfElementsInArray = len;
	if(flashDeviceObject.GenOp.DataProgram(PageProgram, &para)!=Flash_Success)
		return FLASH_WRITE_ERROR;
	return FFIS_OK;
}

static FFISretVal flashEraseSector_Micron(FlashHW *flashObj, uint16_t startSecNum, uint16_t numOfSectors)
{
	int i;
	ReturnType ret;
	if((startSecNum + numOfSectors)>flashDeviceObject.Desc.FlashSectorCount)
		return FFIS_INVALID_PARAMS;
	for (i=0; i<numOfSectors; i++) {
		do {
			ret = flashDeviceObject.GenOp.SectorErase(startSecNum+i);
			if(ret!=Flash_OperationOngoing && ret!=Flash_Success) {
				printf("Error number (%d) in sector number (%d) \r\n", ret, i);
				return FLASH_ERASE_ERROR;
			}
		} while(ret==Flash_OperationOngoing);
	}
	
	return FFIS_OK;
}

static FFISretVal flashEraseSubsector_Micron(FlashHW *flashObj, uint16_t startSubsecNum, uint16_t numOfSubsectors)
{
	int i;
	ReturnType ret;
	if((startSubsecNum + numOfSubsectors)>flashDeviceObject.Desc.FlashSubSectorCount)
		return FFIS_INVALID_PARAMS;
	for (i=0; i<numOfSubsectors; i++) {
		do {
			ret = flashDeviceObject.GenOp.SubSectorErase(startSubsecNum+i);
			if(ret!=Flash_OperationOngoing && ret!=Flash_Success) {
				printf("Error number (%d) in subsector number (%d) \r\n", ret, i);
				return FLASH_ERASE_ERROR;
			}
		} while(ret==Flash_OperationOngoing);
	}
	
	return FFIS_OK;
}

static FFISretVal flashInitialize_Micron(FlashHW *flashObj)
{

	if(Driver_Init(&flashDeviceObject) == Flash_WrongType)  	//  initialize the flash driver
		return FLASH_NO_DEVICE_DETECTED;

	return FFIS_OK;
}
#endif

