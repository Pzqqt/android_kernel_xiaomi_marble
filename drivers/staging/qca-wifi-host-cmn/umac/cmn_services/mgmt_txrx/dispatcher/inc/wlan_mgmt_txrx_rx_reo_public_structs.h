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
 * enum mgmt_rx_reo_shared_snapshot_id - Represents the management
 * rx-reorder snapshots shared between host and target in the host DDR.
 * These snapshots are written by FW/HW and read by Host.
 * @MGMT_RX_REO_SHARED_SNAPSHOT_MAC_HW: MAC HW snapshot
 * @MGMT_RX_REO_SHARED_SNAPSHOT_FW_CONSUMED: FW consumed snapshot
 * @MGMT_RX_REO_SHARED_SNAPSHOT_FW_FORWADED: FW forwarded snapshot
 * @MGMT_RX_REO_SHARED_SNAPSHOT_MAX: Max number of snapshots
 * @MGMT_RX_REO_SHARED_SNAPSHOT_INVALID: Invalid snapshot
 */
enum mgmt_rx_reo_shared_snapshot_id {
	MGMT_RX_REO_SHARED_SNAPSHOT_MAC_HW = 0,
	MGMT_RX_REO_SHARED_SNAPSHOT_FW_CONSUMED = 1,
	MGMT_RX_REO_SHARED_SNAPSHOT_FW_FORWADED = 2,
	MGMT_RX_REO_SHARED_SNAPSHOT_MAX = 3,
	MGMT_RX_REO_SHARED_SNAPSHOT_INVALID,
};

/*
 * struct mgmt_rx_reo_snapshot_params - Represents the simplified version of
 * Management Rx Frame snapshot for Host use. Note that this is different from
 * the structure shared between the Host and FW/HW
 * @valid: Whether this snapshot is valid
 * @mgmt_pkt_ctr: MGMT packet counter. This will be local to a particular
 * HW link
 * @global_timestamp: Global timestamp.This is taken from a clock which is
 * common across all the HW links
 */
struct mgmt_rx_reo_snapshot_params {
	bool valid;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
};

/*
 * struct mgmt_rx_reo_snapshot - Represents the management rx-reorder snapshot
 * @mgmt_rx_reo_snapshot_low: Lower 32 bits of the reo snapshot
 * @mgmt_rx_reo_snapshot_high: Higher 32 bits of the reo snapshot
 */
struct mgmt_rx_reo_snapshot {
	uint32_t mgmt_rx_reo_snapshot_low;
	uint32_t mgmt_rx_reo_snapshot_high;
};

/*
 * struct mgmt_rx_reo_params - MGMT Rx REO parameters
 * @valid: Whether these params are valid
 * @pdev_id: pdev ID for which FW consumed event is received
 * @link_id: link ID for which FW consumed event is received
 * @mgmt_pkt_ctr: MGMT packet counter of the frame that is consumed
 * @global_timestamp: Global timestamp of the frame that is consumed
 */
struct mgmt_rx_reo_params {
	bool valid;
	uint8_t pdev_id;
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
};

/*
 * struct mgmt_rx_reo_filter - MGMT Rx REO filter
 * @filter_low: Least significant 32-bits of the filter
 * @filter_high: Most significant 32-bits of the filter
 */
struct mgmt_rx_reo_filter {
	uint32_t low;
	uint32_t high;
};
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_PUBLIC_STRUCTS_H */
