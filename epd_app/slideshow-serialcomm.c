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
 * app/slideshow-serialcomm.c -- Basic slideshow app with next slide with spacebar
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <msp430-uart.h>
#include <stdio.h>
#include <string.h>
#include "../epd_sys/assert-pl.h"
#include "../epd_app/app.h"
#include "../pl/epdc.h"
#include "../pl/epdpsu.h"
#include "../pl/platform.h"
#define LOG_TAG "slideshow spacebar"
#include "../epd_sys/utils.h"

/* -- private functions -- */

static int show_image(struct pl_platform *plat, const char *dir,
		      const char *file);

/* -- public entry point -- */
char globalKey=0;

/*int app_slideshow_with_spacebar(struct pl_platform *plat, const char *path)
{
	DIR dir;
	FILINFO f;
	int dir_open = 0;
	//int temp;
	//char* str=NULL;

	//char ch;

	assert(plat != NULL);
	assert(path != NULL);

	LOG("Running slideshow - next slide with spacebar");

	do
	{
		if (!dir_open) {
			 (re-)open the directory
			if (f_opendir(&dir, path) != FR_OK) {
				LOG("Failed to open directory [%s]", path);
				return -1;
			}

			dir_open = 1;
		}

		 read next entry in the directory
		if (f_readdir(&dir, &f) != FR_OK) {
			LOG("Failed to read directory entry");
			return -1;
		}

		 end of the directory reached
		if (f.fname[0] == '\0') {
			dir_open = 0;
			continue;
		}

		 skip directories
		if ((f.fname[0] == '.') || (f.fattrib & AM_DIR))
			continue;

		 only show PGM files
		if (!strstr(f.fname, ".PGM"))
			continue;

		if (show_image(plat, path, f.fname)) {
			LOG("Failed to show image");
			return -1;
		}

		globalKey=0;
		while (globalKey!=32) {

			msp430_uart_putc(0);
			//str=(char*) calloc(11,sizeof(char));
			//msp430_uart_read(0,str,10);
			//msp430_uart_putc('\n');
			//msp430_uart_write(0,str,strlen(str));
			//free(str);

			//str=msp430_uart_read_line();
			//msp430_uart_write(0,str,strlen(str));
			//free(str);

		}



	} while (!app_stop);

	return 0;
}*/

static int show_image(struct pl_platform *plat, const char *dir,
		      const char *file)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	char path[MAX_PATH_LEN];
	int wfid;

	wfid = pl_epdc_get_wfid(epdc, wf_refresh);

	if (wfid < 0)
		return -1;

	if (join_path(path, sizeof(path), dir, file))
		return -1;

	if (epdc->load_image(epdc, path, NULL, 0, 0))
		return -1;

	if (epdc->update_temp(epdc))
		return -1;

	if (psu->on(psu))
		return -1;

	if (epdc->update(epdc, wfid, NULL))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	if (psu->off(psu))
		return -1;

	return 0;
}
