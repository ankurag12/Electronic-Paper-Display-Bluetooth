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
 * pl/platform.h
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_PLATFORM_H
#define INCLUDE_PL_PLATFORM_H 1

#include "../epd_sys/config.h"
#include "../epd_sys/i2c-eeprom.h"
#include "dispinfo.h"
#include "epdpsu.h"
#include "i2c.h"
#include "epdc.h"
#include "gpio.h"

struct pl_hwinfo;


/* Common platform data */

struct pl_system_gpio {
	unsigned sel[4];
	unsigned sw[5];
	unsigned led[4];
	unsigned assert_led;
	unsigned ruddock_shutdown;
};

struct pl_platform {
	struct pl_gpio gpio;
	struct pl_epdpsu psu;
	struct pl_epdc epdc;
	struct pl_i2c *i2c;
	const struct pl_system_gpio *sys_gpio;
	const struct pl_hwinfo *hwinfo;
	const struct pl_dispinfo *dispinfo;
};

#endif /* INCLUDE_PL_PLATFORM_H */
