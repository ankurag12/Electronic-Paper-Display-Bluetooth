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
 * utils.c -- random homeless functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "../epd_sys/utils.h"

#include "../Hardware/msp430/msp430-gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "../epd_sys/pnm-utils.h"
#include "../pl/endian.h"
#include "../pl/types.h"
#include "assert-pl.h"

void swap32(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[3];
	b[3] = tmp;
	tmp = b[1];
	b[1] = b[2];
	b[2] = tmp;
}

void swap32_array(int32_t **x, uint16_t n)
{
	while (n--)
		swap32(*x++);
}

void swap16(void *x)
{
	uint8_t *b = x;
	uint8_t tmp;

	tmp = b[0];
	b[0] = b[1];
	b[1] = tmp;
}

void swap16_array(int16_t **x, uint16_t n)
{
	while (n--)
		swap16(*x++);
}

int is_file_present(uint8_t id)
{
	fileIndexEntry f;

	if (fileCheckOut(&flashHWobj, id, &f, READ) != FFIS_OK)
		return 0;

	fileCheckIn(&flashHWobj, &f);

	return 1;
}


/* ----------------------------------------------------------------------------
 * Debug utilies
 */

/* Defined in main.c */
extern void abort_now(const char *abort_msg, enum abort_error error_code);

static void do_abort_msg(const char *file, unsigned line,
			 const char *error_str, const char *message,
			 enum abort_error error_code)
{
	/* Following conversion of line to a string is a workaround
	 * for a problem with fprintf(stderr, "%u", line) that only
	 * occurs when NOT debugging and prevents further code execution
	 * (possibly a heap size issue?)
	 */
	char temp[16];
	sprintf(temp, "%u", line);
	fprintf(stderr, "%s, line %s: %s\n", file, temp, message);

	abort_now(error_str, error_code);
}

void do_abort_msg_assert(const char *file, unsigned line, const char *message)
{
	do_abort_msg(file, line, "Assertion failed\n", message, ABORT_ASSERT);
}

void do_abort_msg_error(const char *file, unsigned line, const char *message, enum abort_error error_code)
{
	do_abort_msg(file, line, "Fatal error\n", message, error_code);
}

void dump_hex(const void *data, uint16_t len)
{
	static const char hex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	char s[] = "[XXXX] XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX";
	char *cur;
	uint16_t i;

	if (!len)
		return;

	for (i = 0, cur = s; i < len; ++i) {
		const uint8_t byte = ((const uint8_t *)data)[i];

		if (!(i & 0xF)) {
			uint16_t addr = i;
			uint16_t j;

			if (i)
				puts(s);

			cur = s + 4;

			for (j = 4; j; --j) {
				*cur-- = hex[addr & 0xF];
				addr >>= 4;
			}

			cur = s + 7;
		}

		*cur++ = hex[byte >> 4];
		*cur++ = hex[byte & 0xF];
		++cur;
	}

	i %= 16;

	if (i) {
		cur = s + 6 + (i * 3);
		*cur++ = '\n';
		*cur++ = '\0';
	}

	puts(s);
}
