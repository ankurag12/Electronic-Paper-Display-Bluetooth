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
 * main.c -- main entry point for code.
 *
 * Authors:
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */
#include "../../FFIS/FlashFileIndexSystem.h"
#include "../../FFIS/FFIS_HAL.h"
#include "../../epson/epson-s1d135xx.h"
#include "../../pl/platform.h"
#include "../../pl/gpio.h"
#include "../../pl/hwinfo.h"
#include "../../pl/wflib.h"
#include "../../epd_app/app.h"
#include <stdio.h>
#include "../../epd_sys/i2c-eeprom.h"
#include "../../epd_sys/vcom.h"
#include "../../epd_sys/pmic-tps65185.h"
#include "../../epd_sys/assert-pl.h"
#include "../../epd_sys/config.h"
#include "../../epd_sys/probe.h"
#include "msp430-i2c.h"
#include "msp430-gpio.h"
#include "msp430-sdcard.h"
#include "msp430-spi.h"
#include "msp430-uart.h"
#include "../../epd_app/slideshow.h"

#define LOG_TAG "msp430-epd"
#include "../../epd_sys/utils.h"

#include "../../Main.h"

/* I2C addresses */
#define I2C_HWINFO_EEPROM_ADDR 0x50
#define I2C_DISPINFO_EEPROM_ADDR 0x54

/* Navigation buttons */
#define	SW1         MSP430_GPIO(2,0)
#define	SW2         MSP430_GPIO(2,1)
#define	SW3         MSP430_GPIO(2,2)
#define	SW4         MSP430_GPIO(2,3)
#define	SW5         MSP430_GPIO(2,4)

/* User LEDs */
#define	LED1        MSP430_GPIO(8,0)
#define	LED2        MSP430_GPIO(8,1)
#define	LED3        MSP430_GPIO(8,2)
#define	LED4        MSP430_GPIO(8,3)

/* System LEDs */
#define	ASSERT_LED  MSP430_GPIO(7,7)

/* User selection switches */
#define	SEL1        MSP430_GPIO(8,4)
#define	SEL2        MSP430_GPIO(8,5)
#define	SEL3        MSP430_GPIO(8,6)
#define	SEL4        MSP430_GPIO(8,7)

/* Ruddock shutdown (power control) */
#define RUDDOCK_SHUTDOWN MSP430_GPIO(5,1)

/* Version of pl-mcu-epd */
static const char VERSION[] = "v008";

/* Platform instance, to be passed to other modules */
struct pl_platform g_plat;				// GLOBAL DEFINITION! Not Static. Use extern elsewhere
				//
FlashHW flashObj;				// GLOBAL DEFINITION! Not Static. Use extern elsewhere

/* --- System GPIOs --- */

static const struct pl_gpio_config g_gpios[] = {
	/* User selection switches */
	{ SEL1, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL2, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL3, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL4, PL_GPIO_INPUT | PL_GPIO_PU },

	/* Navigation buttons */
	{ SW1, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW2, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW3, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW4, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW5, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },

	/* User LEDs */
	{ LED1, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED2, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED3, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED4, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },

	/* System LEDs */
	{ ASSERT_LED, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },

	/* Ruddock shutdown (power control) */
	{ RUDDOCK_SHUTDOWN, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

static const struct pl_system_gpio g_sys_gpio = {
	{ SEL1, SEL2, SEL3, SEL4 },
	{ SW1, SW2, SW3, SW4, SW5 },
	{ LED1, LED2, LED3, LED4 },
	ASSERT_LED,
	RUDDOCK_SHUTDOWN,
};

/* --- HV-PMIC GPIOs --- */

#define HVSW_CTRL         MSP430_GPIO(1,2) /* VCOM switch enable */
#define PMIC_EN           MSP430_GPIO(1,1) /* HV-PMIC enable */
#define PMIC_POK          MSP430_GPIO(1,0) /* HV-PMIC power OK */
#define PMIC_FLT          MSP430_GPIO(2,5) /* HV-PMIC fault condition */

static const struct pl_gpio_config g_hvpmic_gpios[] = {
	{ HVSW_CTRL,  PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
	{ PMIC_FLT,   PL_GPIO_INPUT                   },
};

static struct pl_epdpsu_gpio g_epdpsu_gpio = {
	&g_plat.gpio, PMIC_EN, HVSW_CTRL, PMIC_POK, PMIC_FLT, 300, 5, 100
};

/* --- Epson GPIOs --- */

/* Optional pins used in Epson SPI interface */
/* HRDY indicates controller is ready */
#define SPI_HRDY_USED     0 /* HRDY pin is used */
/* HDC required by the 524 controller, optional on others */
#define SPI_HDC_USED      1 /* HDC pin is used */

/* Basic signals to enable Epson clock and PSU */
#define EPSON_VCC_EN      MSP430_GPIO(1,7)
#define EPSON_CLK_EN      MSP430_GPIO(1,6)

/* Interface control signals */
#define	EPSON_HIRQ        MSP430_GPIO(2,6)
#define	EPSON_HDC         MSP430_GPIO(1,3)
#define	EPSON_HRDY        MSP430_GPIO(2,7)
#define EPSON_RESET       MSP430_GPIO(5,0)
#define EPSON_CS_0        MSP430_GPIO(3,6)

/*
 Parallel interface
#define	EPSON_HDB0        MSP430_GPIO(4,0)
#define	EPSON_HDB1        MSP430_GPIO(4,1)
#define	EPSON_HDB2        MSP430_GPIO(4,2)
#define	EPSON_HDB3        MSP430_GPIO(4,3)
#define	EPSON_HDB4        MSP430_GPIO(4,4)
#define	EPSON_HDB5        MSP430_GPIO(4,5)
#define	EPSON_HDB6        MSP430_GPIO(4,6)
#define	EPSON_HDB7        MSP430_GPIO(4,7)
#define	EPSON_HDB8        MSP430_GPIO(6,0)
#define	EPSON_HDB9        MSP430_GPIO(6,1)
#define	EPSON_HDB10       MSP430_GPIO(6,2)
#define	EPSON_HDB11       MSP430_GPIO(6,3)
#define	EPSON_HDB12       MSP430_GPIO(6,4)
#define	EPSON_HDB13       MSP430_GPIO(6,5)
#define	EPSON_HDB14       MSP430_GPIO(6,6)
#define	EPSON_HDB15       MSP430_GPIO(6,7)

 TFT interface extensions
#define	EPSON_TFT_HSYNC   MSP430_GPIO(7,2)
#define	EPSON_TFT_VSYNC   MSP430_GPIO(7,3)
#define	EPSON_TFT_DE      MSP430_GPIO(7,4)
#define	EPSON_TFT_CLK     MSP430_GPIO(7,5)
*/

static const struct pl_gpio_config g_epson_gpios[] = {
	{ EPSON_VCC_EN,  PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_CLK_EN,  PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_HIRQ,    PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ EPSON_HRDY,    PL_GPIO_INPUT                   },
	{ EPSON_HDC,     PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_RESET,   PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_CS_0,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

/*
static const uint16_t g_epson_parallel[] = {
	EPSON_HDB0, EPSON_HDB1, EPSON_HDB2, EPSON_HDB3, EPSON_HDB4, EPSON_HDB5,
	EPSON_HDB6, EPSON_HDB7, EPSON_HDB8, EPSON_HDB9, EPSON_HDB10,
	EPSON_HDB11, EPSON_HDB12, EPSON_HDB13, EPSON_HDB14, EPSON_HDB15,
	EPSON_TFT_HSYNC, EPSON_TFT_VSYNC, EPSON_TFT_DE, EPSON_TFT_CLK,
};
*/

static const struct s1d135xx_data g_s1d135xx_data = {
	EPSON_RESET, EPSON_CS_0, EPSON_HIRQ,
#if SPI_HRDY_USED
	EPSON_HRDY,
#else
	PL_GPIO_NONE,
#endif
#if SPI_HDC_USED
	EPSON_HDC,
#else
	PL_GPIO_NONE,
#endif
	EPSON_CLK_EN, EPSON_VCC_EN
};

/* --- SPI bus configuration --- */

#define SPI_CHANNEL 0
#define SPI_DIVISOR 1

/* --- hardware info --- */

//#if CONFIG_HWINFO_DEFAULT

#if CONFIG_PLAT_Z6
#define BOARD_MAJ 6
#define BOARD_MIN 3
#elif CONFIG_PLAT_Z7
#define BOARD_MAJ 7
#define BOARD_MIN 2
#endif

static const struct pl_hwinfo g_hwinfo_default = {
	/* version */
	PL_HWINFO_VERSION,
#if CONFIG_PLAT_Z6 || CONFIG_PLAT_Z7
	/* vcom */
	{ 127, 4172, 381, 12490, 25080, -32300, 56886 },
	/* board */
	{ "HB", BOARD_MAJ, BOARD_MIN, 0, HV_PMIC_TPS65185, 0, 0, 0,
	  CONFIG_DEFAULT_I2C_MODE, TEMP_SENSOR_NONE, 0, EPDC_S1D13541, 1, 1 },
#elif CONFIG_PLAT_RAVEN
	/* vcom */
	{ 63, 4586, 189, 9800, 27770, -41520, 70000 },
	/* board */
	{ "Raven", 1, 0, 0, HV_PMIC_MAX17135, 0, 0, 0,
	  CONFIG_DEFAULT_I2C_MODE, TEMP_SENSOR_LM75, 0, EPDC_S1D13524, 1, 1 },
#else
# error "Sorry, no default hardware data available for this platform."
#endif
	/* CRC16 (not used when not reading from actual EEPROM) */
	0xFFFF,
};

#define HWINFO_DEFAULT (&g_hwinfo_default)

//#else /* !CONFIG_HWINFO_DEFAULT */
//#define HWINFO_DEFAULT (NULL)

//#endif /* CONFIG_HWINFO_DEFAULT */


/* --- main --- */

void epd_sys_init(void)
{
	struct pl_i2c host_i2c;
	struct pl_i2c disp_i2c;
//#if CONFIG_HWINFO_EEPROM
	const struct i2c_eeprom hw_eeprom = {
		&host_i2c, I2C_HWINFO_EEPROM_ADDR, EEPROM_24LC014,
	};
//#endif
	struct i2c_eeprom disp_eeprom = {
		NULL, I2C_DISPINFO_EEPROM_ADDR, EEPROM_24AA256
	};
//#if CONFIG_HWINFO_EEPROM
	struct pl_hwinfo hwinfo_nvm;
//#endif
	struct pl_wflib_eeprom_ctx wflib_eeprom_ctx;
	struct pl_dispinfo dispinfo;
	struct vcom_cal vcom_cal;
	struct tps65185_info pmic_info;
	struct s1d135xx s1d135xx = { &g_s1d135xx_data, &g_plat.gpio };

	//unsigned i;

	g_plat.sys_gpio = &g_sys_gpio;

	/* initialise GPIO interface */
	if (msp430_gpio_init(&g_plat.gpio))
		abort_msg("GPIO init failed", ABORT_MSP430_GPIO_INIT);

	/* initialise system GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_gpios, ARRAY_SIZE(g_gpios)))
		abort_msg("System GPIO init failed", ABORT_MSP430_GPIO_INIT);

	/* initialise MSP430 UART */
	//if (msp430_uart_init(&g_plat.gpio, BR_115200, 'N', 8, 1))
	//	abort_msg("UART init failed", ABORT_MSP430_COMMS_INIT);

	LOG("------------------------");
	LOG("Starting pl-mcu-epd %s", VERSION);

	/* initialize HV-PMIC GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_hvpmic_gpios,
				ARRAY_SIZE(g_hvpmic_gpios)))
		abort_msg("HV-PMIC GPIO init failed", ABORT_MSP430_GPIO_INIT);

	/* initialise Epson GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_epson_gpios,
				ARRAY_SIZE(g_epson_gpios)))
		abort_msg("Epson GPIO init failed", ABORT_MSP430_GPIO_INIT);

	/* hard-reset Epson controller to avoid errors during soft reset */
	s1d135xx_hard_reset(&g_plat.gpio, &g_s1d135xx_data);

	/* initialise Epson parallel interface GPIOs */
/*	for (i = 0; i < ARRAY_SIZE(g_epson_parallel); ++i) {
		if (g_plat.gpio.config(g_epson_parallel[i],
				       PL_GPIO_OUTPUT | PL_GPIO_INIT_L)) {
			abort_msg("Epson parallel GPIO init failed", ABORT_MSP430_GPIO_INIT);
		}
	}
*/

	/* initialise MSP430 I2C master 0 */
	if (msp430_i2c_init(&g_plat.gpio, 0, &host_i2c))
		abort_msg("I2C init failed", ABORT_MSP430_COMMS_INIT);

	/* initialise MSP430 SPI bus */
	if (spi_init(&g_plat.gpio, SPI_CHANNEL, SPI_DIVISOR))
		abort_msg("SPI init failed", ABORT_MSP430_COMMS_INIT);


	/*  Initialize  Flash File Index System */
	if(FFIS_init(&flashObj))
		abort_msg("FFIS init Failed", ABORT_MSP430_COMMS_INIT);

	/* load hardware information */
//#if CONFIG_HWINFO_EEPROM
	if (probe_hwinfo(&g_plat, &hw_eeprom, &hwinfo_nvm, HWINFO_DEFAULT))
		abort_msg("hwinfo probe failed", ABORT_HWINFO);
/*
#elif CONFIG_HWINFO_FLASH

#elif CONFIG_HWINFO_DEFAULT
	LOG("Using default hwinfo");
	g_plat.hwinfo = &g_hwinfo_default;
#else
#error "Invalid hwinfo build configuration, check CONFIG_HWINFO_ options"
#endif
*/
	pl_hwinfo_log(g_plat.hwinfo);

	/* initialise platform I2C bus */
	if (probe_i2c(&g_plat, &s1d135xx, &host_i2c, &disp_i2c))
		abort_msg("Platform I2C init failed", ABORT_I2C_INIT);

	/* load display information */
	disp_eeprom.i2c = g_plat.i2c;
	if (probe_dispinfo(&dispinfo, &g_plat.epdc.wflib, &disp_eeprom,
			   &wflib_eeprom_ctx))
		abort_msg("Failed to load dispinfo", ABORT_DISP_INFO);
	g_plat.dispinfo = &dispinfo;
	pl_dispinfo_log(&dispinfo);

	/* initialise EPD HV-PSU and HV-PMIC */
	if (probe_hvpmic(&g_plat, &vcom_cal, &g_epdpsu_gpio, &pmic_info))
		abort_msg("HV-PMIC and EPD PSU init failed", ABORT_HVPSU_INIT);

	/* initialise EPDC */
	if (probe_epdc(&g_plat, &s1d135xx))
		abort_msg("EPDC init failed", ABORT_EPDC_INIT);

	/* run the application */
	app_clear(&g_plat);			//TEMPORARY!!
	if (show_image(&g_plat, RECEIVED_IMG_FILE_ID)) {
		LOG("Failed to show saved image\r\n");
	}
	//msp430_uart_write(1,"Hello World",11);		// TEST FOR UART

	//app_slideshow(&g_plat,"img");
	//if (app_demo(&g_plat))
	//	abort_msg("Application failed", ABORT_APPLICATION);

}

/* When something fatal happens, this is called to print a message on stderr
 * and flash the "assert" LED according to the behaviour for the error_code.
 */
void abort_now(const char *abort_msg, enum abort_error error_code)
{
	if (abort_msg != NULL)
		fprintf(stderr, "%s\r\n", abort_msg);

	/* Force LED off for case where error_code == 0 */
	g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);

	for (;;) {
		int i;
		mdelay(1500);
		for (i = 0; i < error_code; i++)
		{
			g_plat.gpio.set(g_plat.sys_gpio->assert_led, 1);
			mdelay(250);
			g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);
			mdelay(250);
		}
	}
}
