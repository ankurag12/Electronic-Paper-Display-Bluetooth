/*
 * msp430-flashmem.h
 *
 *  Created on: Aug 15, 2015
 *      Author: Robo
 */

#ifndef HARDWARE_MSP430_MSP430_FLASHMEM_H_
#define HARDWARE_MSP430_MSP430_FLASHMEM_H_


#define INFO_FLASH_SEG_A 	((char *) 0x001980)
#define INFO_FLASH_SEG_B 	((char *) 0x001900)
#define INFO_FLASH_SEG_C 	((char *) 0x001880)
#define INFO_FLASH_SEG_D 	((char *) 0x001800)


/*char* FlashSegAPtr = 0x001980;
char* FlashSegBPtr = 0x001900;
char* FlashSegCPtr = 0x001880;
char* FlashSegDPtr = 0x001800;*/



void EraseInfoFlashSeg(char *FlashSegPtr);

void WriteBytesToInfoFlashSeg(char *FlashSegPtr, int offset, char *data, int size);

void ReadBytesFromInfoFlashSeg(char *FlashSegPtr, int offset, char *data, int size);

#endif /* HARDWARE_MSP430_MSP430_FLASHMEM_H_ */
