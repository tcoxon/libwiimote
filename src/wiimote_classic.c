/* $Id$
 *
 * Copyright (C) 2007, Joel Andersson <bja@kth.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>

#include "wiimote.h"
#include "wiimote_io.h"
#include "wiimote_error.h"
#include "wiimote_classic.h"
#include "wiimote_event.h"

#define CLASSIC_MEM_START	0x04a40000
#define CLASSIC_MEM_END		0x04a400ff
#define CLASSIC_REG_CTRL	0x04a40040

#define classic_decode_byte(x)	(((x) ^ 0x17) + 0x17)

int wiimote_classic_enable(wiimote_t *wiimote, uint8_t enable)
{
	if (wiimote_write_byte(wiimote, CLASSIC_REG_CTRL, enable ? 0x00 : 0xff) < 0) {
		wiimote_error("wiimote_classic_enable(): unable to write nunchuk");
		return WIIMOTE_ERROR;
	}
	return WIIMOTE_OK;
}

int wiimote_classic_init(wiimote_t *wiimote)
{
	if (wiimote_classic_enable(wiimote, 1) < 0) {
		wiimote_error("wiimote_classic_init(): unable to initialize classic controller");
		return WIIMOTE_ERROR;
	}

//	if (wiimote_classic_calibrate(wiimote) < 0) {
//		wiimote_set_error("nunchuk_init(): unable to calibrate classic controller");
//		return WIIMOTE_ERROR;
//	}

	return WIIMOTE_OK;
}

void wiimote_classic_decode(uint8_t *data, uint32_t size)
{
	int i;
	for (i=0; i<size; i++) {
		data[i] = classic_decode_byte(data[i]);
	}
}

int wiimote_classic_update(wiimote_t *wiimote, uint8_t *data)
{
	uint8_t keys1 = data[4];
	uint8_t keys2 = data[5];

	wiimote->ext.items.classic.keys.items.plus  = (keys1 & (1<<2)) == 0;
	wiimote->ext.items.classic.keys.items.home  = (keys1 & (1<<3)) == 0;
	wiimote->ext.items.classic.keys.items.minus = (keys1 & (1<<4)) == 0;
	wiimote->ext.items.classic.keys.items.down  = (keys1 & (1<<6)) == 0;
	wiimote->ext.items.classic.keys.items.right = (keys1 & (1<<7)) == 0;
	wiimote->ext.items.classic.keys.items.up    = (keys2 & (1<<0)) == 0;
	wiimote->ext.items.classic.keys.items.left  = (keys2 & (1<<1)) == 0;
	wiimote->ext.items.classic.keys.items.x     = (keys2 & (1<<3)) == 0;
	wiimote->ext.items.classic.keys.items.a     = (keys2 & (1<<4)) == 0;
	wiimote->ext.items.classic.keys.items.y     = (keys2 & (1<<5)) == 0;
	wiimote->ext.items.classic.keys.items.b     = (keys2 & (1<<6)) == 0;

	wiimote->ext.items.classic.keys.items.l     = (keys1 & (1<<5)) == 0;
	wiimote->ext.items.classic.keys.items.r     = (keys1 & (1<<1)) == 0;
	wiimote->ext.items.classic.keys.items.zl    = (keys2 & (1<<7)) == 0;
	wiimote->ext.items.classic.keys.items.zr    = (keys2 & (1<<2)) == 0;

	wiimote->ext.items.classic.joyx1 = data[0] & 0x3f;
	wiimote->ext.items.classic.joyy1 = data[1] & 0x3f;
	wiimote->ext.items.classic.joyx2 = ((data[1] & 0xc0) >> 5) | ((data[0] & 0xc0) >> 3) | ((data[2] & 0x80) >> 7);
	wiimote->ext.items.classic.joyy2 = data[2] & 0x1f;

	wiimote->ext.items.classic.l = ((data[2] & 0x60) >> 2) | ((data[3] & 0xe0) >> 5);
	wiimote->ext.items.classic.r = (data[3] & 0x1f);

	wiimote->keys.items.plus  |= wiimote->ext.items.classic.keys.items.plus;
	wiimote->keys.items.home  |= wiimote->ext.items.classic.keys.items.home;
	wiimote->keys.items.minus |= wiimote->ext.items.classic.keys.items.minus;
	wiimote->keys.items.down  |= wiimote->ext.items.classic.keys.items.down;
	wiimote->keys.items.right |= wiimote->ext.items.classic.keys.items.right;
	wiimote->keys.items.up    |= wiimote->ext.items.classic.keys.items.up;
	wiimote->keys.items.left  |= wiimote->ext.items.classic.keys.items.left;
	wiimote->keys.items.one   |= wiimote->ext.items.classic.keys.items.x;
	wiimote->keys.items.a     |= wiimote->ext.items.classic.keys.items.a;
	wiimote->keys.items.two   |= wiimote->ext.items.classic.keys.items.y;
	wiimote->keys.items.b     |= wiimote->ext.items.classic.keys.items.b;

	return 0;
}
