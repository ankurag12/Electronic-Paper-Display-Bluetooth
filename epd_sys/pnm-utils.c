/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * pnm-utils.c -- Utilities for dealing with PNM format graphics files
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 * Utilities for dealing with PNM format graphics files
 * We are only interested in PGM and PBM
 *
 * Details of the format can be found at:
 * http://en.wikipedia.org/wiki/Netpbm_format
 * However, this seems to indicate that the header format is more free format
 * http://paulbourke.net/dataformats/ppm/
 * Lines terminated with cr and/or lf or just white space
 * Header example 1
 * P6 1024 788 255
 * Header example 2
 * P6
 * 1024 788
 * # A comment
 * 255
 * Header example 3
 * P3
 * 1024 # the image width
 * 788 # the image height
 * # A comment
 * 1023
 *
 *
 * P4
 * # Comment string
 * 24 7
 *
 * P5
 * # Comment string
 * 24 7
 * 15
 *
 */

#include "pnm-utils.h"

#include <stdlib.h>

#include "assert-pl.h"
#include "utils.h"


int pnm_read_int32(struct _fileIndexEntry *pnm_file, int32_t *value)
{
	int count;
	uint8_t ch;
	int digits = 0;
	int in_comment = 0;
	int found = 0;
	int32_t val = 0;

	assert(pnm_file != NULL);
	assert(value != NULL);

	while (!found &&
			(fileRead(&flashObj, pnm_file, &ch, 1, &count) == FFIS_OK) && count == 1)
	{
		switch (ch)
		{
			case '#':
				in_comment = 1;
				break;
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				if (!in_comment)
				{
					if (digits) {
						found = 1;
					}
				}
				if (ch == '\r' || ch == '\n')
					in_comment = 0;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (!in_comment) {
					val = val * 10 + (ch - '0');
					digits++;

					if (digits == pnm_file->fileSize) {
						found = 1;
					}
				}
				break;
			default:
				break;
		}
	}

	if (!found)
		return -1;

	*value = val;

	return 0;
}


int pnm_read_header(fileIndexEntry *pnm_file, struct pnm_header *hdr)
{
	char buffer[2];
	int count;

	assert(pnm_file);
	assert(hdr);

	hdr->type = PNM_UNKNOWN;

	if (fileRead(&flashObj, pnm_file, (uint8_t*)buffer, 2, &count) != FFIS_OK)
		goto read_error;

	if (buffer[0] != 'P')
		goto format_error;

	if (buffer[1] == '4')
		hdr->type = PNM_BITMAP;
	else if (buffer[1] == '5')
		hdr->type = PNM_GREYSCALE;
	else
		goto format_error;

	hdr->max_gray = 1;
	hdr->width = pnm_read_int(pnm_file);
	hdr->height = pnm_read_int(pnm_file);

	if (hdr->type == PNM_GREYSCALE) {
		hdr->max_gray = pnm_read_int(pnm_file);
	}
	// check to see if any of the data items were not read correctly
	if (hdr->width <= 0 || hdr->height <= 0 || hdr->max_gray <= 0)
		goto format_error;

	// read pointer is now positioned at start of image data
	return 0;

format_error:
read_error:
	return -1;
}

int pnm_read_int32_directstream(unsigned char **data, unsigned int dataLen, int32_t *value)
{
	//unsigned int count;
	char ch;
	int digits = 0;
	int in_comment = 0;
	int found = 0;
	int32_t val = 0;
	unsigned int ctr = 0;

	//assert(pnm_file != NULL);
	assert(value != NULL);

	while (!found)
	{
		ch=*(*data);
		(*data)++;
		ctr++;
		if(ctr>dataLen)
		{
			LOG("File header must be less than %d bytes \r\n", dataLen);
			break;
		}
		switch (ch)
		{
			case '#':
				in_comment = 1;
				break;
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				if (!in_comment)
				{
					if (digits) {
						found = 1;
					}
				}
				if (ch == '\r' || ch == '\n')
					in_comment = 0;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (!in_comment) {
					val = val * 10 + (ch - '0');
					digits++;

					//if (f_eof(pnm_file)) {
					//	found = 1;
					//}
				}
				break;
			default:
				break;
		}
	}

	if (!found)
		return -1;

	*value = val;

	return 0;
}


int pnm_read_header_directstream(unsigned char **dataPtr, unsigned int dataLen, struct pnm_header *hdr)
{
	char buffer[2];
	//unsigned int count;

	//assert(pnm_file);
	assert(hdr);

	hdr->type = PNM_UNKNOWN;

	memcpy(buffer, *dataPtr, 2);
	(*dataPtr)+=2;

	//if (f_read(pnm_file,buffer,2,&count) != FR_OK)
	//	goto read_error;

	if (buffer[0] != 'P')
		goto format_error;

	if (buffer[1] == '4')
		hdr->type = PNM_BITMAP;
	else if (buffer[1] == '5')
		hdr->type = PNM_GREYSCALE;
	else
		goto format_error;

	hdr->max_gray = 1;
	hdr->width = pnm_read_int_directstream(dataPtr, dataLen);
	hdr->height = pnm_read_int_directstream(dataPtr, dataLen);

	if (hdr->type == PNM_GREYSCALE) {
		hdr->max_gray = pnm_read_int_directstream(dataPtr, dataLen);
	}
	// check to see if any of the data items were not read correctly
	if (hdr->width <= 0 || hdr->height <= 0 || hdr->max_gray <= 0)
		goto format_error;

	// read pointer is now positioned at start of image data
	return 0;

format_error:
//read_error:
	return -1;
}
