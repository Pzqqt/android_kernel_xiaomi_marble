/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef EVENT_DEFS_H
#define EVENT_DEFS_H

typedef enum {
	EVENT_DROP_ID = 0,

	/* Events between 0x1 to 0x674 are not used */

	EVENT_WLAN_SECURITY = 0x675, /* 13 byte payload */
	EVENT_WLAN_STATUS, /* 15 byte payload */

	/* Events 0x677 and 0x678 are not used */

	EVENT_WLAN_QOS = 0x679, /* 2 byte payload */
	EVENT_WLAN_PE, /* 16 byte payload */

	/* Events between 0x67b to 0x67f are not used */

	EVENT_WLAN_BRINGUP_STATUS = 0x680, /* 12 byte payload */
	EVENT_WLAN_POWERSAVE_GENERIC, /* 16 byte payload */
	EVENT_WLAN_POWERSAVE_WOW, /* 11 byte payload */

	/* Events between 0x683 to 0x690 are not used */

	EVENT_WLAN_BTC = 0x691, /* 15 byte payload */
	EVENT_WLAN_EAPOL = 0xA8D,/* 18 bytes payload */
	EVENT_WLAN_WAKE_LOCK = 0xAA2, /* 96 bytes payload */
	EVENT_WLAN_BEACON_RECEIVED = 0xAA6, /* FW event: 2726 */
	EVENT_WLAN_LOG_COMPLETE = 0xAA7, /* 16 bytes payload */
	EVENT_WLAN_STATUS_V2 = 0xAB3,
	EVENT_WLAN_TDLS_TEARDOWN = 0xAB5,
	EVENT_WLAN_TDLS_ENABLE_LINK = 0XAB6,
	EVENT_WLAN_SUSPEND_RESUME = 0xAB7,
	EVENT_WLAN_OFFLOAD_REQ = 0xAB8,
	EVENT_TDLS_SCAN_BLOCK = 0xAB9,
	EVENT_WLAN_TDLS_TX_RX_MGMT = 0xABA,
	EVENT_WLAN_LOW_RESOURCE_FAILURE = 0xABB,

	EVENT_MAX_ID = 0x0FFF
} event_id_enum_type;

#endif /* EVENT_DEFS_H */
