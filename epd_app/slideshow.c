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
 * app/slideshow.c -- Basic slideshow app
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include "../epd_sys/assert-pl.h"
#include "../epd_app/app.h"
#include "../pl/epdc.h"
#include "../pl/epdpsu.h"
#include "../pl/platform.h"

#define LOG_TAG "slideshow"
#include "../epd_sys/utils.h"
#include "slideshow.h"

//static int cmd_next = 0;

int show_image(struct pl_platform *plat, uint8_t id, struct disp_coord *coord)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	int wfid;
	struct pl_area *area = &coord->area;
	int left_in = coord->left_in;
	int top_in = coord->top_in;

	wfid = pl_epdc_get_wfid(epdc, wf_refresh);

	if (wfid < 0)
		return -1;


	if (epdc->load_image(epdc, id, area, left_in, top_in))
		return -1;

	if (epdc->update_temp(epdc))
		return -1;

	if (psu->on(psu))
		return -1;

	if (epdc->update(epdc, wfid, area))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	if (psu->off(psu))
		return -1;

	return 0;
}


int show_image_directstream(struct pl_platform *plat, unsigned char **data, int dataLen, struct disp_coord *coord, file_streaming_stage_t stage)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	//char path[MAX_PATH_LEN];
	int wfid;
	struct pl_area *area = &coord->area;
	int left_in = coord->left_in;
	int top_in = coord->top_in;

	switch(stage) {
		case HEADER:
			wfid = pl_epdc_get_wfid(epdc, wf_refresh);

			if (wfid < 0)
				return -1;

			//if (join_path(path, sizeof(path), dir, file))
			//	return -1;

			if (epdc->load_image_directstream(epdc, data, dataLen, area, left_in, top_in, stage))
				return -1;

			break;

		case BODY:
			if (epdc->load_image_directstream(epdc, data, dataLen, area, left_in, top_in, stage))
				return -1;
			break;

		case FINISH:
			if (epdc->load_image_directstream(epdc, data, dataLen, area, left_in, top_in, stage))
				return -1;

			if (epdc->update_temp(epdc))
				return -1;

			if (psu->on(psu))
				return -1;

			if (epdc->update(epdc, wfid, area))
				return -1;

			if (epdc->wait_update_end(epdc))
				return -1;

			if (psu->off(psu))
				return -1;

			break;

		default:
			LOG("Unknown file streaming stage");
			return -1;
	}


	return 0;
}
