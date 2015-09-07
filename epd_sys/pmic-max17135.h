/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

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
 * max17135-pmic.h -- Driver for the MAXIM MAX17135 HVPMIC device.
 *
 * Authors:
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_MAX17135_PMIC_H
#define INCLUDE_MAX17135_PMIC_H 1

#include <stdint.h>

struct max17135_info;

enum {
	MAX17135_SEQ_0,	// Type4, Maxim Driver timing
	MAX17135_SEQ_1	// Type11, ST driver timing
};

extern int max17135_init(struct pl_i2c *i2c, uint8_t i2c_addr,
			 struct max17135_info **pmic);
extern int max17135_configure(struct max17135_info *pmic, struct vcom_cal *cal,
		       int power_sequence);
extern int max17135_set_vcom_voltage(struct max17135_info *pmic, int mv);
extern int max17135_set_vcom_register(struct max17135_info *pmic,
				      int dac_value);

extern int max17135_wait_pok(struct max17135_info *pmic);
extern int max17135_enable(struct max17135_info *pmic);
extern int max17135_disable(struct max17135_info *pmic);

extern int max17135_temp_enable(struct max17135_info *pmic);
extern int max17135_temp_disable(struct max17135_info *pmic);
extern int max17135_temperature_measure(struct max17135_info *pmic,
					int16_t *measured);

#endif /* INCLUDE_MAX17135_PMIC_H */
