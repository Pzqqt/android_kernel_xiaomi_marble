/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/**
 *  DOC: wlan_mgmt_txrx_rx_reo_public_structs.h
 *  Contains MGMT Rx REO structure definitions
 */

#ifndef _WLAN_MGMT_TXRX_RX_REO_PUBLIC_STRUCTS_H
#define _WLAN_MGMT_TXRX_RX_REO_PUBLIC_STRUCTS_H

#ifdef WLAN_MGMT_RX_REO_SUPPORT

/*
 * struct mgmt_rx_reo_snapshot_simplified - Represents the simplified version of
 * Management Rx Frame snapshot for Host use. Note that this is different from
 * the structure shared between the Host and FW/HW
 * @valid: Whether this snapshot is valid
 * @mgmt_pkt_ctr: MGMT packet counter. This will be local to a particular
 * HW link
 * @global_timestamp: Global timestamp.This is taken from a clock which is
 * common across all the HW links
 */
struct mgmt_rx_reo_snapshot_simplified {
	bool valid;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
};

#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_PUBLIC_STRUCTS_H */
