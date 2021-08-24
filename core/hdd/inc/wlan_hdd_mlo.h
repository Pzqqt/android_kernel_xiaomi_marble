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
 * DOC : wlan_hdd_mlo.h
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#if !defined(WLAN_HDD_MLO_H)
#define WLAN_HDD_MLO_H
#include "wlan_hdd_main.h"

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * struct hdd_mld_mac - hdd structure to hold mld address
 * @mld_mac: mld addr
 * @device_mode: Device mode for mld address
 */
struct hdd_mld_mac {
	struct qdf_mac_addr mld_addr;
	uint8_t device_mode;
};

/**
 * struct hdd_mld_mac_info - HDD structure to hold mld mac address information
 * @num_mld_addr: Number of mld address supported
 * @mld_intf_addr_mask: mask to dervice the multiple mld address
 * @mld_mac_list: Mac address assigned for device mode
 */
struct hdd_mld_mac_info {
	uint8_t num_mld_addr;
	unsigned long mld_intf_addr_mask;
	struct hdd_mld_mac mld_mac_list[WLAN_MAX_MLD];
};

/**
 * hdd_update_mld_mac_addr() - Derive mld mac address
 * @hdd_context: Global hdd context
 * @hw_macaddr: Hardware mac address
 *
 * This function dervies mld mac address based on the input hardware mac address
 *
 * Return: none
 */
void hdd_update_mld_mac_addr(struct hdd_context *hdd_ctx,
			     struct qdf_mac_addr hw_macaddr);
#else
static inline
void hdd_update_mld_mac_addr(struct hdd_context *hdd_ctx,
			     struct qdf_mac_addr hw_macaddr)
{
}
#endif
#endif
