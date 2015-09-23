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
 * epson-s1d135xx.c -- Common Epson S1D135xx primitives
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "../epson/epson-s1d135xx.h"

#include "../Hardware/msp430/msp430-spi.h" /* until this becomse <pl/spi.h> */
#include "../pl/wflib.h"
#include <stdlib.h>
#include "../epd_sys/assert-pl.h"

/* until the i/o operations are abstracted */
#include "../epd_sys/pnm-utils.h"
#include "../pl/endian.h"
#include "../pl/gpio.h"
#include "../pl/types.h"

#define LOG_TAG "s1d135xx"
#include "../epd_sys/utils.h"

#include "../epd_app/parser.h"
#include "../epd_app/slideshow.h"

/* Set to 1 to enable verbose update and EPD power on/off log messages */
#define VERBOSE 0

#define DATA_BUFFER_LENGTH              512

#define S1D135XX_WF_MODE(_wf)           (((_wf) << 8) & 0x0F00)
#define S1D135XX_XMASK                  0x0FFF
#define S1D135XX_YMASK                  0x0FFF
#define S1D135XX_INIT_CODE_CHECKSUM_OK  (1 << 15)
#define S1D135XX_PWR_CTRL_UP            0x8001
#define S1D135XX_PWR_CTRL_DOWN          0x8002
#define S1D135XX_PWR_CTRL_BUSY          0x0080
#define S1D135XX_PWR_CTRL_CHECK_ON      0x2200

enum s1d135xx_cmd {
	S1D135XX_CMD_INIT_SET         = 0x00, /* to load init code */
	S1D135XX_CMD_RUN              = 0x02,
	S1D135XX_CMD_STBY             = 0x04,
	S1D135XX_CMD_SLEEP            = 0x05,
	S1D135XX_CMD_INIT_STBY        = 0x06, /* init then standby */
	S1D135XX_CMD_INIT_ROT_MODE    = 0x0B,
	S1D135XX_CMD_READ_REG         = 0x10,
	S1D135XX_CMD_WRITE_REG        = 0x11,
	S1D135XX_CMD_BST_RD_SDR       = 0x1C,
	S1D135XX_CMD_BST_WR_SDR       = 0x1D,
	S1D135XX_CMD_BST_END_SDR      = 0x1E,
	S1D135XX_CMD_LD_IMG           = 0x20,
	S1D135XX_CMD_LD_IMG_AREA      = 0x22,
	S1D135XX_CMD_LD_IMG_END       = 0x23,
	S1D135XX_CMD_WAIT_DSPE_TRG    = 0x28,
	S1D135XX_CMD_WAIT_DSPE_FREND  = 0x29,
	S1D135XX_CMD_UPD_INIT         = 0x32,
	S1D135XX_CMD_UPDATE_FULL      = 0x33,
	S1D135XX_CMD_UPDATE_FULL_AREA = 0x34,
	S1D135XX_CMD_EPD_GDRV_CLR     = 0x37,
};

static int get_hrdy(struct s1d135xx *p);
static int do_fill(struct s1d135xx *p, const struct pl_area *area,
		   unsigned bpp, uint8_t g);
static int wflib_wr(void *ctx, const uint8_t *data, size_t n);
static int transfer_file(FIL *file);
static int transfer_file_directstream(unsigned char **dataPtr, unsigned int dataLen);
static int transfer_image(FIL *f, const struct pl_area *area, int left,
			  int top, int width);
static void transfer_data(const uint8_t *data, size_t n);
static void send_cmd_area(struct s1d135xx *p, uint16_t cmd, uint16_t mode,
			  const struct pl_area *area);
static void send_cmd_cs(struct s1d135xx *p, uint16_t cmd);
static void send_cmd(struct s1d135xx *p, uint16_t cmd);
static void send_params(const uint16_t *params, size_t n);
static void send_param(uint16_t param);
static void set_cs(struct s1d135xx *p, int state);
static void set_hdc(struct s1d135xx *p, int state);

/* ----------------------------------------------------------------------------
 * public functions
 */

void s1d135xx_hard_reset(struct pl_gpio *gpio,
			 const struct s1d135xx_data *data)
{
	if (data->reset == PL_GPIO_NONE) {
		LOG("Warning: no hard reset");
		return;
	}

	pl_gpio_set(gpio, data->reset, 0);
	mdelay(4);
	pl_gpio_set(gpio, data->reset, 1);
	mdelay(10);
}

int s1d135xx_soft_reset(struct s1d135xx *p)
{
	s1d135xx_write_reg(p, S1D135XX_REG_SOFTWARE_RESET, 0xFF);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_check_prod_code(struct s1d135xx *p, uint16_t ref_code)
{
	uint16_t prod_code;

	prod_code = s1d135xx_read_reg(p, S1D135XX_REG_REV_CODE);

	LOG("Product code: 0x%04X", prod_code);

	if (prod_code != ref_code) {
		LOG("Invalid product code, expected 0x%04X", ref_code);
		return -1;
	}

	return 0;
}

int s1d135xx_load_init_code(struct s1d135xx *p)
{
	static const char init_code_path[] = "bin/Ecode.bin";
	FIL init_code_file;
	uint16_t checksum;
	int stat;

	if (f_open(&init_code_file, init_code_path, FA_READ) != FR_OK)
		return -1;

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_SET);
	stat = transfer_file(&init_code_file);
	set_cs(p, 1);
	f_close(&init_code_file);

	if (stat) {
		LOG("Failed to transfer init code file");
		return -1;
	}

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_STBY);
	send_param(0x0500);
	set_cs(p, 1);
	mdelay(100);

	if (s1d135xx_wait_idle(p))
		return -1;

	checksum = s1d135xx_read_reg(p, S1D135XX_REG_SEQ_AUTOBOOT_CMD);

	if (!(checksum & (uint16_t)S1D135XX_INIT_CODE_CHECKSUM_OK)) {
		LOG("Init code checksum error");
		return -1;
	}

	return 0;
}

int s1d135xx_load_wflib(struct s1d135xx *p, struct pl_wflib *wflib,
			uint32_t addr)
{
	uint16_t params[4];
	uint32_t size2 = wflib->size / 2;

	if (s1d135xx_wait_idle(p))
		return -1;

	params[0] = addr & 0xFFFF;
	params[1] = (addr >> 16) & 0xFFFF;
	params[2] = size2 & 0xFFFF;
	params[3] = (size2 >> 16) & 0xFFFF;
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_BST_WR_SDR);
	send_params(params, ARRAY_SIZE(params));
	set_cs(p, 1);

	if (wflib->xfer(wflib, wflib_wr, p))
		return -1;

	if (s1d135xx_wait_idle(p))
		return -1;

	send_cmd_cs(p, S1D135XX_CMD_BST_END_SDR);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_init_gate_drv(struct s1d135xx *p)
{
	send_cmd_cs(p, S1D135XX_CMD_EPD_GDRV_CLR);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_wait_dspe_trig(struct s1d135xx *p)
{
	send_cmd_cs(p, S1D135XX_CMD_WAIT_DSPE_TRG);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_clear_init(struct s1d135xx *p)
{
	send_cmd_cs(p, S1D135XX_CMD_UPD_INIT);

	if (s1d135xx_wait_idle(p))
		return -1;

	return s1d135xx_wait_dspe_trig(p);
}

int s1d135xx_fill(struct s1d135xx *p, uint16_t mode, unsigned bpp,
		  const struct pl_area *a, uint8_t grey)
{
	struct pl_area full_area;
	const struct pl_area *fill_area;

	set_cs(p, 0);

	if (a != NULL) {
		send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, a);
		fill_area = a;
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(mode);
		full_area.top = 0;
		full_area.left = 0;
		full_area.width = p->xres;
		full_area.height = p->yres;
		fill_area = &full_area;
	}

	set_cs(p, 1);

	return do_fill(p, fill_area, bpp, grey);
}

int s1d135xx_pattern_check(struct s1d135xx *p, uint16_t height, uint16_t width, uint16_t checker_size, uint16_t mode)
{
	uint16_t i = 0, j = 0, k = 0;
	uint16_t val = 0;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_LD_IMG);
	send_param(mode);
	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);

	for (i = 0; i < height; i++) {
		k = i / checker_size;
		for (j = 0; j < width; j += 2) {
			val = (k + (j / checker_size)) % 2 ? 0xFFFF : 0x0;
			send_param(val);
		}
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return 0;
}

int s1d135xx_load_image(struct s1d135xx *p, const char *path, uint16_t mode,
			unsigned bpp, const struct pl_area *area, int left,
			int top)
{
	struct pnm_header hdr;
	FIL img_file;
	int stat;

	if (f_open(&img_file, path, FA_READ) != FR_OK)
		return -1;

	if (pnm_read_header(&img_file, &hdr))
		return -1;

	set_cs(p, 0);

	if (area != NULL) {
		send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, area);
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(mode);
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);

	if (area == NULL)
		stat = transfer_file(&img_file);
	else
		stat = transfer_image(&img_file, area, left, top, hdr.width);

	set_cs(p, 1);
	f_close(&img_file);

	if (stat)
		return -1;

	if (s1d135xx_wait_idle(p))
		return -1;

	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return s1d135xx_wait_idle(p);
}


int s1d135xx_load_image_directstream(struct s1d135xx *p, unsigned char **dataPtr, int dataLen, uint16_t mode,
			unsigned bpp, const struct pl_area *area, int left,
			int top, file_streaming_stage_t stage)
{
	int stat;
	switch(stage)
	{
		case HEADER:
		{
			struct pnm_header hdr;
			//FIL img_file;
			// if (f_open(&img_file, path, FA_READ) != FR_OK)
			//	return -1;

			if (pnm_read_header_directstream(dataPtr, dataLen, &hdr))
				return -1;

			set_cs(p, 0);

			if (area != NULL) {
				send_cmd_area(p, S1D135XX_CMD_LD_IMG_AREA, mode, area);
			} else {
				send_cmd(p, S1D135XX_CMD_LD_IMG);
				send_param(mode);
			}

			set_cs(p, 1);

			if (s1d135xx_wait_idle(p))
				return -1;

			set_cs(p, 0);
			send_cmd(p, S1D135XX_CMD_WRITE_REG);
			send_param(S1D135XX_REG_HOST_MEM_PORT);
			break;
		}
		case BODY:
		{

			if (area == NULL)
				return transfer_file_directstream(dataPtr, dataLen);
			else
				//stat = transfer_image(&img_file, area, left, top, hdr.width);
				LOG("Area is not NULL, transferring partial image not yet implemented");
			break;
		}

		case FINISH:
		{
			set_cs(p, 1);

			if (s1d135xx_wait_idle(p))
				return -1;

			send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

			return s1d135xx_wait_idle(p);

		}

		default:
			LOG("Unknown streaming stage \r\n");
			return -1;

	}
	return 0;

}


int s1d135xx_update(struct s1d135xx *p, int wfid, const struct pl_area *area)
{
#if VERBOSE
	if (area != NULL)
		LOG("update area %d (%d, %d) %dx%d", wfid,
		    area->left, area->top, area->width, area->height);
	else
		LOG("update %d", wfid);
#endif

	set_cs(p, 0);

	/* wfid = S1D135XX_WF_MODE(wfid); */

	if (area != NULL) {
		send_cmd_area(p, S1D135XX_CMD_UPDATE_FULL_AREA,
			      S1D135XX_WF_MODE(wfid), area);
	} else {
		send_cmd(p, S1D135XX_CMD_UPDATE_FULL);
		send_param(S1D135XX_WF_MODE(wfid));
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	return s1d135xx_wait_dspe_trig(p);
}

int s1d135xx_wait_update_end(struct s1d135xx *p)
{
	send_cmd_cs(p, S1D135XX_CMD_WAIT_DSPE_FREND);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_wait_idle(struct s1d135xx *p)
{
	unsigned long timeout = 1000000;

	while (!get_hrdy(p) && --timeout);

	if (!timeout) {
		LOG("HRDY timeout");
		return -1;
	}

	return 0;
}

int s1d135xx_set_power_state(struct s1d135xx *p,
			     enum pl_epdc_power_state state)
{
	const struct s1d135xx_data *data = p->data;
	int stat;

	set_cs(p, 1);
	set_hdc(p, 1);
	pl_gpio_set(p->gpio, data->vcc_en, 1);
	pl_gpio_set(p->gpio, data->clk_en, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	switch (state) {
	case PL_EPDC_RUN:
		send_cmd_cs(p, S1D135XX_CMD_RUN);
		stat = s1d135xx_wait_idle(p);
		break;

	case PL_EPDC_STANDBY:
		send_cmd_cs(p, S1D135XX_CMD_STBY);
		stat = s1d135xx_wait_idle(p);
		break;

	case PL_EPDC_SLEEP:
		send_cmd_cs(p, S1D135XX_CMD_STBY);
		stat = s1d135xx_wait_idle(p);
		pl_gpio_set(p->gpio, data->clk_en, 0);
		break;

	case PL_EPDC_OFF:
		send_cmd_cs(p, S1D135XX_CMD_SLEEP);
		stat = s1d135xx_wait_idle(p);
		pl_gpio_set(p->gpio, data->clk_en, 0);
		pl_gpio_set(p->gpio, data->vcc_en, 0);
		set_hdc(p, 0);
		set_cs(p, 0);
		break;
	}

	return stat;
}

int s1d135xx_set_epd_power(struct s1d135xx *p, int on)
{
	uint16_t arg = on ? S1D135XX_PWR_CTRL_UP : S1D135XX_PWR_CTRL_DOWN;
	uint16_t tmp;

#if VERBOSE
	LOG("EPD power o%s", on ? "n" : "ff");
#endif

	if (s1d135xx_wait_idle(p))
		return -1;

	s1d135xx_write_reg(p, S1D135XX_REG_PWR_CTRL, arg);

	do {
		tmp = s1d135xx_read_reg(p, S1D135XX_REG_PWR_CTRL);
	} while (tmp & S1D135XX_PWR_CTRL_BUSY);

	if (on && ((tmp & S1D135XX_PWR_CTRL_CHECK_ON) !=
		   S1D135XX_PWR_CTRL_CHECK_ON)) {
		LOG("Failed to turn the EPDC power on");
		return -1;
	}

	return 0;
}

void s1d135xx_cmd(struct s1d135xx *p, uint16_t cmd, const uint16_t *params,
		  size_t n)
{
	set_cs(p, 0);
	send_cmd(p, cmd);
	send_params(params, n);
	set_cs(p, 1);
}

uint16_t s1d135xx_read_reg(struct s1d135xx *p, uint16_t reg)
{
	uint16_t val;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_READ_REG);
	send_param(reg);
	spi_read_bytes((uint8_t *)&val, sizeof(uint16_t));
	spi_read_bytes((uint8_t *)&val, sizeof(uint16_t));
	set_cs(p, 1);

	return be16toh(val);
}

void s1d135xx_write_reg(struct s1d135xx *p, uint16_t reg, uint16_t val)
{
	const uint16_t params[] = { reg, val };

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_params(params, ARRAY_SIZE(params));
	set_cs(p, 1);
}

int s1d135xx_load_register_overrides(struct s1d135xx *p)
{
	static const char override_path[] = "bin/override.txt";
	static const char sep[] = ", ";
	FIL file;
	FRESULT res;
	int stat;
	uint16_t reg, val;

	res = f_open(&file, override_path, FA_READ);
	if (res != FR_OK) {
		if (res == FR_NO_FILE) {
			return 0;
		}
		else {
			LOG("Failed to open register override file");
			return -1;
		}
	}

	stat = 0;
	while (!stat) {
		char line[81];
		int len;
		stat = parser_read_file_line(&file, line, sizeof(line));
		if (stat < 0) {
			LOG("Failed to read line");
			break;
		}
		else if (!stat) {
			/* End of file */
			break;
		}

		if ((line[0] == '\0') || (line[0] == '#')) {
			stat = 0;
			continue;
		}

		/* Assume failure */
		stat = -1;

		len = parser_read_word(line, sep, &reg);
		if (len <= 0)
			break;

		len = parser_read_word(line + len, sep, &val);
		if (len <= 0)
			break;

		s1d135xx_write_reg(p, reg, val);
		if (val == s1d135xx_read_reg(p, reg)) {
			stat = 0;	/* success */
		}
	}

	f_close(&file);

	return stat;
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int get_hrdy(struct s1d135xx *p)
{
	uint16_t status;

	if (p->data->hrdy != PL_GPIO_NONE)
		return pl_gpio_get(p->gpio, p->data->hrdy);

	status = s1d135xx_read_reg(p, S1D135XX_REG_SYSTEM_STATUS);

	return ((status & p->hrdy_mask) == p->hrdy_result);
}

static int do_fill(struct s1d135xx *p, const struct pl_area *area,
		   unsigned bpp, uint8_t g)
{
	uint16_t val16;
	uint16_t lines;
	uint16_t pixels;

	/* Only 16-bit transfers for now... */
	assert(!(area->width % 2));

	switch (bpp) {
	case 1:
	case 2:
		LOG("Unsupported bpp");
		return -1;
	case 4:
		val16 = g & 0xF0;
		val16 |= val16 >> 4;
		val16 |= val16 << 8;
		pixels = area->width / 4;
		break;
	case 8:
		val16 = g | (g << 8);
		pixels = area->width / 2;
		break;
	default:
		assert_fail("Invalid bpp");
	}

	lines = area->height;

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);

	while (lines--) {
		uint16_t x = pixels;

		while (x--)
			send_param(val16);
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	send_cmd_cs(p, S1D135XX_CMD_LD_IMG_END);

	return s1d135xx_wait_idle(p);
}

static int wflib_wr(void *ctx, const uint8_t *data, size_t n)
{
	struct s1d135xx *p = ctx;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);
	transfer_data(data, n);
	set_cs(p, 1);

	return 0;
}

static int transfer_file(FIL *file)
{
	uint8_t data[DATA_BUFFER_LENGTH];

	for (;;) {
		size_t count;

		if (f_read(file, data, sizeof(data), &count) != FR_OK)
			return -1;

		if (!count)
			break;

		transfer_data(data, count);
	}

	return 0;
}

static int transfer_file_directstream(unsigned char **dataPtr, unsigned int dataLen)
{

	transfer_data((uint8_t *)(*dataPtr), (size_t)dataLen);

	return 0;
}

static int transfer_image(FIL *f, const struct pl_area *area, int left,
			  int top, int width)
{
	uint8_t data[DATA_BUFFER_LENGTH];
	size_t line;

	/* Simple bounds check */
	if (width < area->width || width < (left + area->width)) {
		LOG("Invalid combination of width/left/area");
		return -1;
	}

	if (f_lseek(f, f->fptr + ((long)top * (unsigned long)width)) != FR_OK)
		return -1;

	for (line = area->height; line; --line) {
		size_t count;
		size_t remaining = area->width;

		/* Find the first relevant pixel (byte) on this line */
		if (f_lseek(f, f->fptr + (unsigned long)left) != FR_OK)
			return -1;

		/* Transfer data of interest in chunks */
		while (remaining) {
			size_t btr = (remaining <= DATA_BUFFER_LENGTH) ?
					remaining : DATA_BUFFER_LENGTH;

			if (f_read(f, data, btr, &count) != FR_OK)
				return -1;

			transfer_data(&data[0], btr);

			remaining -= btr;
		}

		/* Move file pointer to end of line */
		if (f_lseek(f, f->fptr + (width - (left + area->width))) != FR_OK)
			return -1;
	}

	return 0;
}

static void transfer_data(const uint8_t *data, size_t n)
{
	const uint16_t *data16 = (const uint16_t *)data;

	n /= 2;

	while (n--)
		send_param(*data16++);
}

static void send_cmd_area(struct s1d135xx *p, uint16_t cmd, uint16_t mode,
			  const struct pl_area *area)
{
	const uint16_t args[] = {
		mode,
		(area->left & S1D135XX_XMASK),
		(area->top & S1D135XX_YMASK),
		(area->width & S1D135XX_XMASK),
		(area->height & S1D135XX_YMASK),
	};

	send_cmd(p, cmd);
	send_params(args, ARRAY_SIZE(args));
}

static void send_cmd_cs(struct s1d135xx *p, uint16_t cmd)
{
	set_cs(p, 0);
	send_cmd(p, cmd);
	set_cs(p, 1);
}

static void send_cmd(struct s1d135xx *p, uint16_t cmd)
{
	cmd = htobe16(cmd);

	set_hdc(p, 0);
	spi_write_bytes((uint8_t *)&cmd, sizeof(uint16_t));
	set_hdc(p, 1);
}

static void send_params(const uint16_t *params, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i)
		send_param(params[i]);
}

static void send_param(uint16_t param)
{
	param = htobe16(param);
	spi_write_bytes((uint8_t *)&param, sizeof(uint16_t));
}

static void set_cs(struct s1d135xx *p, int state)
{
	pl_gpio_set(p->gpio, p->data->cs0, state);
}

static void set_hdc(struct s1d135xx *p, int state)
{
	const unsigned hdc = p->data->hdc;

	if (hdc != PL_GPIO_NONE)
		pl_gpio_set(p->gpio, hdc, state);
}
