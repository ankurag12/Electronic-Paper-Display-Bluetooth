/*
 * jpeg2pnm.h
 *
 *  Created on: Oct 14, 2015
 *      Author: Robo
 */

#ifndef JPEG_DECOMP_JPEG2PNM_H_
#define JPEG_DECOMP_JPEG2PNM_H_

#include "tjpgd.h"
#include "../FFIS/FlashFileIndexSystem.h"

typedef struct {
	fileIndexEntry fin;
	fileIndexEntry fout;
	uint32_t fileWriteLoc;
	uint8_t pnmHeaderLen;
} IODEV;




UINT in_func (JDEC* jd, BYTE* buff, UINT nbyte);
void cleanScribbleArea();
JRESULT decompressJPEG();


extern FlashHW flashHWobj;

#endif /* JPEG_DECOMP_JPEG2PNM_H_ */
