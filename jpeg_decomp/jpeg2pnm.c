/*
 * jpeg2pnm.c
 *
 *  Created on: Oct 14, 2015
 *      Author: Robo
 */
#include "jpeg2pnm.h"
#include <string.h>
#include <stdio.h>
#include "../epd_sys/config.h"
#include "../FFIS/FlashFileIndexSystem.h"

#if (JD_FORMAT == 0)
#define		PIXEL_SIZE (3)
#elif (JD_FORMAT == 1)
#define		PIXEL_SIZE (2)
#elif (JD_FORMAT == 2)
#define		PIXEL_SIZE (1)
#endif

#define SCRIBBLE_FILE_SIZE	(1245184L)		// TEMP, HARD CODED !! This value is display_width*display_height*pixel_size (Size of an uncompressed file)

/*------------------------------*/
/* User defined input funciton  */
/*------------------------------*/
UINT in_func (JDEC* jd, BYTE* buff, UINT nbyte)
{
    IODEV *dev = (IODEV*)(jd->device);   /* Device identifier for the session (5th argument of jd_prepare function) */
    FFISretVal ret;
    uint16_t br;


    if (buff) {
        /* Read bytes from input stream */
    	if(ret = fileRead(&flashHWobj, &(dev->fin), (uint8_t*)buff, nbyte, &br)) {
    		return JDR_INP;
    	}
        return (UINT)br;
    } else {
        /* Remove bytes from input stream */
    	ret = fileSeek(&(dev->fin), dev->fin.bookMark - dev->fin.startAddr + nbyte);
    	if(ret != FFIS_OK)
    		return 0;
    	else
    		return nbyte;
    }
}

/*------------------------------*/
/* User defined output funciton */
/*------------------------------*/

UINT out_func (JDEC* jd, void* bitmap, JRECT* rect)
{
    IODEV *dev = (IODEV*)jd->device;
    BYTE *src;
    UINT fBuffSize, rectWidth, rectHeight;
    FFISretVal ret;
    uint16_t bw;
	int i;


/*
    // Put progress indicator
    if (rect->left == 0) {
        printf("\r%lu%%", (rect->top << jd->scale) * 100UL / jd->height);
    }
*/

    /* Copy the decompressed RGB rectanglar to the frame buffer */
    src = (BYTE*)bitmap;

	rectWidth = (rect->right - rect->left + 1);
	rectHeight = (rect->bottom - rect->top + 1);

    fBuffSize = PIXEL_SIZE * rectWidth * rectHeight;

    // Write the decompressed rectangle at appropriate location in the file


	for (i=0; i<rectHeight; i++) {
		if(ret = fileSeek(&(dev->fout), dev->fileWriteLoc+dev->pnmHeaderLen))
			printf("Error (%d) in fileSeek \r\n", ret);

		ret = fileWriteNoErase(&flashHWobj, &(dev->fout), src + i*PIXEL_SIZE*rectWidth, PIXEL_SIZE*rectWidth, &bw);
		if(ret != FFIS_OK || bw != PIXEL_SIZE*rectWidth )
			printf("Error (%d) in file write \r\n", ret);

		dev->fileWriteLoc += PIXEL_SIZE*(jd->width);
	}

	dev->fileWriteLoc = dev->fileWriteLoc - (rectHeight)*PIXEL_SIZE*jd->width + PIXEL_SIZE*rectWidth;
	if((dev->fileWriteLoc)%(PIXEL_SIZE * jd->width)==0) {
		dev->fileWriteLoc += (rectHeight-1)*(PIXEL_SIZE*jd->width);
	}


	//printf("rect->right = %d, rect->left = %d, rect->top = %d, rect->bottom = %d, frameBuffSize = %d \r\n", rect->right, rect->left, rect->top, rect->bottom, fBuffSize );
    return 1;    /* Continue to decompress */
}


JRESULT decompressJPEG()
{
    BYTE work[3100];       	/* Decompressor work area */
    JDEC jdec;        		/* Decompression object */
    JRESULT res;      		/* Result code of TJpgDec API */
    IODEV devid;      		/* User defined device identifier */
    char header[20];
    uint16_t bw;
    memset(header, 0, sizeof(header));

    FFISretVal ret;
    if(ret = fileCheckOut(&flashHWobj, RECEIVED_JPEG_FILE_ID, &(devid.fin), READ))
    	printf("Error (%d) in checking out received jpeg file in read mode \r\n", ret);
    if(ret = fileCheckOut(&flashHWobj, RECEIVED_PNM_FILE_ID, &(devid.fout), WRITE))
        	printf("Error (%d) in checking out received pnm file in write mode \r\n", ret);

    res = jd_prepare(&jdec, in_func, work, sizeof(work), &devid);
    if (res == JDR_OK) {
        /* Ready to dcompress. Image info is available here. */
        //printf("Image dimensions: %u by %u. %u bytes used.\n", jdec.width, jdec.height, 3100 - jdec.sz_pool);

    	sprintf(header, "P5\n%d %d\n255\n", jdec.width, jdec.height);		// HARD CODED PNM HEADER !!

    	// Write the PNM header
    	ret = fileWriteNoErase(&flashHWobj, &(devid.fout), (uint8_t*)header, strlen(header), &bw);
    	if(ret != FFIS_OK || bw != strlen(header))
    		printf("Error (%d) in writing PNM header \r\n", ret);
    	devid.pnmHeaderLen = strlen(header);

    	devid.fileWriteLoc = 0;

    	/* Start to decompress with 1/1 scaling */
        res = jd_decomp(&jdec, out_func, 0);
        if (res == JDR_OK) {
            printf("Decompression succeeded \r\n");
        } else {
            printf("Failed to decompress: rc=%d \r\n", res);
        }

    } else {
        printf("Failed to prepare: rc=%d \r\n", res);
    }


	if(ret = fileCheckIn(&flashHWobj, &(devid.fin)))
		printf("Error (%d) in checking in received jpeg file \r\n", ret);
	if(ret = fileCheckIn(&flashHWobj, &(devid.fout)))
		printf("Error (%d) in checking in received pnm file \r\n", ret);

    return res;
}

/* This technique does not save as much time as expected.
 * Moreover, the time to decompress is so large that a few seconds saved by
 * not erasing sectors while writing received file is negligible.
 *
 */

void cleanScribbleArea()
{
	FFISretVal ret;
	fileIndexEntry entry1, entry2;
	uint32_t len;
	uint8_t buff[512];
	uint16_t br, bw;

	// Erase JPEG file
	if(ret = fileCheckOut(&flashHWobj, RECEIVED_JPEG_FILE_ID, &entry1, WRITE))
		printf("Error (%d) in checking out received JPEG file in write mode to erase \r\n", ret);
	len = SCRIBBLE_FILE_SIZE;
	if(ret = fileErase(&flashHWobj, &entry1, len))
		printf("Error (%d) in erasing recevied JPEG file \r\n", ret);
	if(ret = fileCheckIn(&flashHWobj, &entry1))
		printf("Error (%d) in checking in received JPEG file after erase \r\n", ret);

	// Copy converted PNM file (from JPEG) to a more permanent location

	if(ret = fileCheckOut(&flashHWobj, RECEIVED_PNM_FILE_ID, &entry1, READ))
		printf("Error (%d) in checking out received PNM file in read mode before erase \r\n", ret);
	if(ret = fileCheckOut(&flashHWobj, STORED_PNM_FILE_ID_1, &entry2, WRITE))
		printf("Error (%d) in checking out stored PNM file 1 in write mode \r\n", ret);

	while(1) {
		if (ret = fileRead(&flashHWobj, &entry1, buff, sizeof(buff), &br))
			printf("Error (%d) in reading from recevied PNM file \r\n", ret);
		if(br == 0)
			break;
		ret = fileWrite(&flashHWobj, &entry2, buff, br, &bw);
		if(ret != FFIS_OK || bw != br)
			printf("Error (%d) in writing to stored PNM file \r\n", ret);
	}

	if(ret = fileCheckIn(&flashHWobj, &entry1))
		printf("Error (%d) in checking in received PNM file \r\n", ret);
	if(ret = fileCheckIn(&flashHWobj, &entry2))
		printf("Error (%d) in checking in new PNM file \r\n", ret);


	// Erase the converted PNM file area for faster write the next time
	if(ret = fileCheckOut(&flashHWobj, RECEIVED_PNM_FILE_ID, &entry1, WRITE))
		printf("Error (%d) in checking out received PNM file in write mode to erase \r\n", ret);
	len = SCRIBBLE_FILE_SIZE;
	if(ret = fileErase(&flashHWobj, &entry1, SCRIBBLE_FILE_SIZE))
		printf("Error (%d) in erasing recevied PNM file \r\n", ret);
	if(ret = fileCheckIn(&flashHWobj, &entry1))
		printf("Error (%d) in checking in received PNM file after erase \r\n", ret);

}
