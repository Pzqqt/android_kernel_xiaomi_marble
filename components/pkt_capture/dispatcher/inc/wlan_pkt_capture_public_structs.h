/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_
#define _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_

/**
 * enum pkt_capture_mode - packet capture modes
 * @PACKET_CAPTURE_MODE_DISABLE: packet capture mode disable
 * @PACKET_CAPTURE_MODE_MGMT_ONLY: capture mgmt packets only
 * @PACKET_CAPTURE_MODE_DATA_ONLY: capture data packets only
 * @PACKET_CAPTURE_MODE_DATA_MGMT: capture both data and mgmt packets
 */
enum pkt_capture_mode {
	PACKET_CAPTURE_MODE_DISABLE = 0,
	PACKET_CAPTURE_MODE_MGMT_ONLY,
	PACKET_CAPTURE_MODE_DATA_ONLY,
	PACKET_CAPTURE_MODE_DATA_MGMT,
};

#endif /* _WLAN_PKT_CAPTURE_PUBLIC_STRUCTS_H_ */
