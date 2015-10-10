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
 * app/sequencer.c -- Sequencer app
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   Andrew Cox <andrew.cox@plasticlogic.com>
 *
 */

#include <string.h>
#include "../epd_app/app.h"
#include "../pl/epdc.h"
#include "../pl/platform.h"

#define LOG_TAG "power-demo"
#include "../epd_sys/utils.h"

int app_power(struct pl_platform *plat, uint8_t fileID)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	int wfid; /* = pl_epdc_get_wfid(epdc, wf_refresh);*/

	wfid = pl_epdc_get_wfid(epdc, wf_refresh);

	if (wfid < 0)
		return -1;


	LOG("Running power sequence demo using image with fileID: %d", fileID);

	while (!app_stop) {
		/* --- RUN mode --- */

		LOG("RUN");

		if (epdc->set_power(epdc, PL_EPDC_RUN))
			return -1;

		if (epdc->load_image(epdc, fileID, NULL, 0, 0))
			return -1;

		if (epdc->set_power(epdc, PL_EPDC_RUN))
			return -1;

		if (pl_epdc_single_update(epdc, psu, wfid, NULL))
			return -1;

		msleep(2000);

		/* --- SLEEP mode --- */

		LOG("SLEEP");

		if (epdc->set_power(epdc, PL_EPDC_SLEEP))
			return -1;

		msleep(2000);

		/* --- STANDBY mode --- */

		LOG("STANDBY");

		if (epdc->set_power(epdc, PL_EPDC_STANDBY))
			return -1;

		msleep(2000);

		/* --- RUN mode again and update with same image --- */

		LOG("RUN");

		if (epdc->set_power(epdc, PL_EPDC_RUN))
			return -1;

		if (pl_epdc_single_update(epdc, psu, wfid, NULL))
			return -1;

		/* --- OFF mode and then resume --- */

		LOG("OFF");

		if (epdc->set_power(epdc, PL_EPDC_OFF))
			return -1;

		msleep(2000);

		LOG("Resuming now");

		if (epdc->set_power(epdc, PL_EPDC_RUN))
			return -1;

		if (pl_epdc_single_update(epdc, psu, wfid, NULL))
			return -1;

		msleep(1000);
	}

	return 0;
}
