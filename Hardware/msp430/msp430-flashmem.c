/*
 * msp430-flashmem.c
 *
 *  Created on: Aug 13, 2015
 *      Author: Robo
 */

#include <msp430.h>
#include "msp430-flashmem.h"

void EraseInfoFlashSeg(char *FlashSegPtr)
{
	__disable_interrupt();
	  FCTL3 = FWPW;                            // Clear Lock bit
	  FCTL1 = FWPW+ERASE;                      // Set Erase bit
	  *FlashSegPtr = 0;                          // Dummy write to erase Flash seg
	  FCTL3 = FWPW+LOCK;                       // Set LOCK bit
	  __enable_interrupt();
}

void WriteBytesToInfoFlashSeg(char *FlashSegPtr, int offset, char *dataPtr, int size)
{
	int i=0;
	__disable_interrupt();
	FCTL3 = FWPW;                            // Clear Lock bit
	FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation
	for(i=0; i<size; i++)
		*(FlashSegPtr+offset+i) = *(dataPtr+i);
	FCTL1 = FWPW;                            // Clear WRT bit
	FCTL3 = FWPW+LOCK;                       // Set LOCK bit
	__enable_interrupt();
}

void ReadBytesFromInfoFlashSeg(char *FlashSegPtr, int offset, char *dataPtr, int size)
{
	int i;
	for(i=0; i<size; i++)
		*(dataPtr+i) = *(FlashSegPtr+offset+i);
	//memcpy(data, FlashSegPtr+offset, size);
}
