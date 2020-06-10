/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <qdf_types.h>

#ifndef _WLAN_IOT_SIM__PUBLIC_STRUCTS_H_
#define _WLAN_IOT_SIM__PUBLIC_STRUCTS_H_

/**
 * struct simulation_test_params
 * pdev_id: pdev id
 * vdev_id: vdev id
 * peer_macaddr: peer MAC address
 * test_cmd_type: test command type
 * test_subcmd_type: test command sub type
 * frame_type: frame type
 * frame_subtype: frame subtype
 * seq: sequence number
 * offset: Frame content offset
 * frame_length: Frame content length
 * buf_len: Buffer length
 * bufp: buffer
 */
struct simulation_test_params {
	u32 pdev_id;
	u32 vdev_id;
	u8 peer_mac[QDF_MAC_ADDR_SIZE];
	u32 test_cmd_type;
	u32 test_subcmd_type;
	u8 frame_type;
	u8 frame_subtype;
	u8 seq;
	u8 reserved;
	u16 offset;
	u16 frame_length;
	u32 buf_len;
	u8 *bufp;
};

#endif	/* _WLAN_IOT_SIM__PUBLIC_STRUCTS_H_ */
