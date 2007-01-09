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
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <arpa/inet.h>
#include <errno.h>

#include "bthid.h"
#include "wiimote.h"
#include "wiimote_report.h"
#include "wiimote_error.h"
#include "wiimote_io.h"

static int __l2cap_connect(const char *host, uint16_t psm)
{
	struct sockaddr_l2 addr = { 0 };
	int sock = 0;

	sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (!sock) {
		wiimote_error("l2cap_connect(): socket");
		return WIIMOTE_ERROR;
	}

	addr.l2_family = AF_BLUETOOTH;
	addr.l2_psm = htobs(psm);
	str2ba(host, &addr.l2_bdaddr);

	if (connect(sock, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
		wiimote_error("l2cap_connect(): connect");
		return WIIMOTE_ERROR;
	}

	return sock;
}

static void init_calibration_data(wiimote_t *wiimote)
{
	uint8_t buf[16];
	memset(buf,0,16);
	wiimote_read(wiimote, 0x20, buf, 16);
	memcpy(&wiimote->cal, buf, sizeof (wiimote_cal_t));
}

static int set_link_addr(wiimote_t *wiimote, const char *host)
{
	int dev_id=-1, dd=-1;
	bdaddr_t addr;
	
	dev_id = hci_get_route(BDADDR_LOCAL);
	if (dev_id < 0) {
		wiimote_error("wiimote_connect(): hci_get_route: %m");
		return WIIMOTE_ERROR;
	}
	
	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		wiimote_error("wiimote_connect(): hci_open_dev: %m");
		return WIIMOTE_ERROR;
	}
	
    if (hci_read_bd_addr(dd, &addr, 0) < 0) {
    	wiimote_error("wiimote_connect(): hci_read_bd_addr: %m");
    	return WIIMOTE_ERROR;
    }

	if (hci_close_dev(dd) < 0) {
    	wiimote_error("wiimote_connect(): hci_close_dev: %m");
    	return WIIMOTE_ERROR;		
	}
    
    ba2str(&addr, wiimote->link.l_addr);
	strncpy(wiimote->link.r_addr, host, 19);
	
	return WIIMOTE_OK;
}

int wiimote_discover(wiimote_t **devices, uint8_t size)
{
	wiimote_error("wiimote_discover(): operation not implemented");
    return WIIMOTE_ERROR;
}

int wiimote_connect(wiimote_t *wiimote, const char *host)
{
	wiimote_report_t r = WIIMOTE_REPORT_INIT;
	
	if (wiimote->link.status == WIIMOTE_STATUS_CONNECTED) {
		wiimote_error("wiimote_connect(): already connected");
		return WIIMOTE_ERROR;
	}

	/* According to the BT-HID specification, the control channel should
	   be opened first followed by the interrupt channel. */

	wiimote->link.s_ctrl = __l2cap_connect(host, BTHID_PSM_CTRL);
	if (wiimote->link.s_ctrl < 0) {
		wiimote_error("wiimote_connect(): l2cap_connect");
		return WIIMOTE_ERROR;
    }

	wiimote->link.status = WIIMOTE_STATUS_UNDEFINED;
	wiimote->link.s_intr = __l2cap_connect(host, BTHID_PSM_INTR);
	if (wiimote->link.s_intr < 0) {
		wiimote_error("wiimote_connect(): l2cap_connect");
		return WIIMOTE_ERROR;
    }

	wiimote->link.status = WIIMOTE_STATUS_CONNECTED;
	wiimote->mode.bits = WIIMOTE_MODE_DEFAULT;
	wiimote->old.mode.bits = 0;

	/* Fill in the bluetooth address of the local and remote devices. */

	set_link_addr(wiimote, host);	

	init_calibration_data(wiimote);
	
	/* Prepare and send a status report request. This will initialize the
	   nunchuk if it is plugged in as a side effect. */

	r.channel = WIIMOTE_RID_STATUS;
	if (wiimote_report(wiimote, &r, sizeof (r.status)) < 0) {
		wiimote_error("wiimote_connect(): status report request failed");
	}

    return WIIMOTE_OK;
}

int wiimote_disconnect(wiimote_t *wiimote)
{
	struct req_raw_out r = { 0 };
	
	if (wiimote->link.status != WIIMOTE_STATUS_CONNECTED) {
		wiimote_set_error("wiimote_disconnect(): not connected");
		return WIIMOTE_OK;
	}
	
	/* Send a VIRTUAL_CABLE_UNPLUG HID_CONTROL request to the remote device. */
	
	r.header = BTHID_TYPE_HID_CONTROL | BTHID_PARAM_VIRTUAL_CABLE_UNPLUG;	
	r.channel = 0x01;
	
	if (send(wiimote->link.s_ctrl, (uint8_t *) &r, 2, 0) < 0) {
		wiimote_error("wiimote_disconnect(): send: %m");
		return WIIMOTE_ERROR;
	}

	/* BT-HID specification says HID_CONTROL requests should not generate
	   any HANDSHAKE responses, but it seems like the wiimote generates a
	   ERR_FATAL handshake response on a VIRTUAL_CABLE_UNPLUG request. */

	if (recv(wiimote->link.s_ctrl, (uint8_t *) &r, 2, 0) < 0) {
		wiimote_error("wiimote_disconnect(): recv: %m");
		return WIIMOTE_ERROR;
	}
		
    if (close(wiimote->link.s_intr) < 0) {
		wiimote_error("wiimote_disconnect(): close: %m");
		return WIIMOTE_ERROR;
    }
    
    if (close(wiimote->link.s_ctrl) < 0) {
		wiimote_error("wiimote_disconnect(): close: %m");
		return WIIMOTE_ERROR;
    }
    
    wiimote->link.status = WIIMOTE_STATUS_DISCONNECTED;
    
    ba2str(BDADDR_ANY, wiimote->link.l_addr);
    ba2str(BDADDR_ANY, wiimote->link.r_addr);
    
    return WIIMOTE_OK;
}
