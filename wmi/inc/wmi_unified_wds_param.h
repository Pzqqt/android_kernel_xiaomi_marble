/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * This file contains the parameter definitions for the WDS WMI APIs.
 */

#ifndef _WMI_UNIFIED_WDS_PARAM_H_
#define _WMI_UNIFIED_WDS_PARAM_H_

/**
 * struct peer_add_wds_entry_params - WDS peer entry add params
 * @dest_addr: destination macaddr
 * @peer_addr: peer mac addr
 * @flags: WMI_HOST_WDS_FLAG_STATIC
 * @vdev_id: Vdev id
 */
struct  peer_add_wds_entry_params {
	uint8_t dest_addr[QDF_MAC_ADDR_SIZE];
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
	uint32_t flags;
	uint32_t vdev_id;
};

/**
 * struct peer_del_wds_entry_params - WDS peer entry del params
 * @dest_addr: destination macaddr
 * @vdev_id: Vdev id
 */
struct peer_del_wds_entry_params {
	uint8_t dest_addr[QDF_MAC_ADDR_SIZE];
	uint32_t vdev_id;
};

/**
 * struct peer_update_wds_entry_params - WDS peer entry update params
 * @dest_addr: destination macaddr
 * @peer_addr: peer mac addr
 * @flags: WMI_HOST_WDS_FLAG_STATIC
 * @vdev_id: Vdev id
 */
struct peer_update_wds_entry_params {
	uint8_t dest_addr[QDF_MAC_ADDR_SIZE];
	uint8_t peer_addr[QDF_MAC_ADDR_SIZE];
	uint32_t flags;
	uint32_t vdev_id;
};
#endif /* _WMI_UNIFIED_WDS_PARAM_H_ */
