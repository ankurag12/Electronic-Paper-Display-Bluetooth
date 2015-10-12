/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * probe.c -- Probing the hardware
 *
 * Authors:
 *    Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */



#include <string.h>
#include <stdio.h>
#include "../FFIS/FlashFileIndexSystem.h"
#include "../epson/epson-epdc.h"
#include "../epson/epson-i2c.h"
#include "../pl/platform.h"
#include "../pl/hwinfo.h"
#include "../pl/dispinfo.h"
#include "../pl/wflib.h"
#include "../epd_sys/probe.h"
#include "../epd_sys/config.h"
#include "../epd_sys/i2c-eeprom.h"
#include "../epd_sys/vcom.h"
#include "../epd_sys/pmic-tps65185.h"
#include "../epd_sys/pmic-max17135.h"

#include "assert-pl.h"


#define LOG_TAG "probe"
#include "../epd_sys/utils.h"



/* ToDo: add to generic HV-PMIC interface */
#define I2C_PMIC_ADDR_TPS65185 0x68
#define I2C_PMIC_ADDR_MAX17135 0x48

//#if CONFIG_HWINFO_EEPROM
int probe_hwinfo(struct pl_platform *plat, const struct i2c_eeprom *hw_eeprom,
		 struct pl_hwinfo *hwinfo_nvm,
		 const struct pl_hwinfo *hwinfo_default)
{

	if (!pl_hwinfo_init(hwinfo_nvm, hw_eeprom)) {
		plat->hwinfo = hwinfo_nvm;
	}
	else if (!pl_hwinfo_init_flash(hwinfo_nvm)) {
		LOG("WARNING: EEPROM failed, using HW info from Flash");
		plat->hwinfo = hwinfo_nvm;
	}
	else {
		LOG("WARNING: EEPROM and Flash failed, using default HW Info");
		plat->hwinfo = hwinfo_default;
	}

	return 0;
}
//#endif

int probe_i2c(struct pl_platform *plat, struct s1d135xx *s1d135xx,
	      struct pl_i2c *host_i2c, struct pl_i2c *disp_i2c)
{
	static const char px[] = "I2C: ";
	int stat;

	switch (plat->hwinfo->board.i2c_mode) {
	case I2C_MODE_HOST: /* MSP430, I2C already initialised */
		LOG("%sHost", px);
		stat = 0;
		plat->i2c = host_i2c;
		break;
	case I2C_MODE_DISP:
		/* This must be the Epson S1D13541...
		 *
		 * Ideally we should check the display info, but this requires
		 * I2C so it's not practical.  Alternatively, we could check
		 * the io_config and board_type info to have more clues.  It
		 * may also be possible to probe for the presence of the
		 * S1D13541 via SPI by reading the product code register for
		 * example.  */
		LOG("%sS1D13541", px);
		stat = epson_i2c_init(s1d135xx, disp_i2c, EPSON_EPDC_S1D13541);
		plat->i2c = disp_i2c;
		break;
	case I2C_MODE_S1D13524:
		LOG("%sS1D13524", px);
		stat = epson_i2c_init(s1d135xx, disp_i2c, EPSON_EPDC_S1D13524);
		plat->i2c = disp_i2c;
		break;
	case I2C_MODE_SC18IS6XX:
		LOG("%snot supported", px);
		stat = -1;
		break;
	case I2C_MODE_NONE:
	default:
		assert_fail("Invalid I2C mode");
	}

	return stat;
}

int probe_dispinfo(struct pl_dispinfo *dispinfo, struct pl_wflib *wflib,
		   const struct i2c_eeprom *e,
		   struct pl_wflib_eeprom_ctx *e_ctx)
{
#if CONFIG_DISP_DATA_EEPROM_ONLY
	return (pl_dispinfo_init_eeprom(dispinfo, e) ||
		pl_wflib_init_eeprom(wflib, e_ctx, e, dispinfo));
#elif CONFIG_DISP_DATA_EEPROM_FLASH
	int retval;
	retval = (pl_dispinfo_init_eeprom(dispinfo, e) ||
			pl_wflib_init_eeprom(wflib, e_ctx, e, dispinfo) );
	if (retval)
		retval = (pl_dispinfo_init_ffis(dispinfo) ||
			pl_wflib_init_ffis(wflib, (uint8_t)WAVEFORM_FILE_ID));
	return retval;
#endif
}

/* interim solution */
static struct max17135_info *g_max17135;

int probe_hvpmic(struct pl_platform *plat, struct vcom_cal *vcom_cal,
		 struct pl_epdpsu_gpio *epdpsu_gpio,
		 struct tps65185_info *pmic_info)
{
	static const char px_psu[] = "EPD PSU: ";
	static const char px_pmic[] = "HV-PMIC: ";
	const struct pl_hwinfo *hwinfo = plat->hwinfo;
	int stat;

	/* ToDo: use hwinfo->board.io_config instead */
	if (!strcmp(hwinfo->board.board_type, "Raven")) {
		/* Warning: This must not call the epdc functions yet... */
		LOG("%sEPDC", px_psu);
		stat = pl_epdpsu_epdc_init(&plat->psu, &plat->epdc);
	} else {
		LOG("%sGPIO", px_psu);
		stat = pl_epdpsu_gpio_init(&plat->psu, epdpsu_gpio);
	}

	if (stat) {
		LOG("Failed to initialise EPD PSU");
		return -1;
	}

	vcom_init(vcom_cal, &hwinfo->vcom);

	switch (hwinfo->board.hv_pmic) {
	case HV_PMIC_NONE:
		LOG("%sNone", px_pmic);
		stat = 0;
		break;
	case HV_PMIC_MAX17135:
		LOG("%sMAX17135", px_pmic);
		stat = max17135_init(
			plat->i2c, I2C_PMIC_ADDR_MAX17135, &g_max17135);
		if (!stat)
			stat = max17135_configure(
				g_max17135, vcom_cal, MAX17135_SEQ_1);
		if (!stat)
			stat = max17135_set_vcom_voltage(
				g_max17135, plat->dispinfo->info.vcom);

		break;
	case HV_PMIC_TPS65185:
		LOG("%sTPS65185", px_pmic);
		stat = tps65185_init(pmic_info, plat->i2c,
				     I2C_PMIC_ADDR_TPS65185, vcom_cal);
		if (!stat) /* ToDo: generalise set_vcom with HV-PMIC API */
			stat = tps65185_set_vcom_voltage(
				pmic_info, plat->dispinfo->info.vcom);
		break;
	default:
		assert_fail("Invalid HV-PMIC id");
	}

	return stat;
}

int probe_epdc(struct pl_platform *plat, struct s1d135xx *s1d135xx)
{
	static const char px[] = "EPDC: ";
	const struct pl_hwinfo *hwinfo = plat->hwinfo;
	struct pl_epdc *epdc = &plat->epdc;
	int stat;

	switch (hwinfo->board.epdc_ref) {
	case EPDC_S1D13524:
		LOG("%sS1D13524", px);
		stat = epson_epdc_init(epdc, plat->dispinfo,
				       EPSON_EPDC_S1D13524, s1d135xx);
		break;
	case EPDC_S1D13541:
		LOG("%sS1D13541", px);
		stat = epson_epdc_init(epdc, plat->dispinfo,
				       EPSON_EPDC_S1D13541, s1d135xx);
		break;
	case EPDC_NONE:
#if PL_EPDC_STUB
		LOG("%sStub");
		stat = pl_epdc_stub_init(epdc);
		break;
#endif /* fall through otherwise */
	default:
		assert_fail("Invalid EPDC identifier");
	}

#if 0 /* enable during development of new EPDC implementations */
	if (stat)
		return -1;

	assert(epdc->clear_init != NULL);
	assert(epdc->load_wflib != NULL);
	assert(epdc->update != NULL);
	assert(epdc->wait_update_end != NULL);
	assert(epdc->set_power != NULL);
	assert(epdc->set_temp_mode != NULL);
	assert(epdc->update_temp != NULL);
	assert(epdc->fill != NULL);
	assert(epdc->load_image != NULL);
	assert(epdc->set_epd_power != NULL);
	assert(epdc->wf_table != NULL);
	assert(epdc->xres != 0);
	assert(epdc->yres != 0);
#endif

	return stat;
}
