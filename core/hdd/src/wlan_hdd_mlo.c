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
 * DOC : wlan_hdd_mlo.c
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_mlo.h"

void hdd_update_mld_mac_addr(struct hdd_context *hdd_ctx,
			     struct qdf_mac_addr hw_macaddr)
{
	uint8_t i;
	uint8_t mldaddr_b2, tmp_br2;
	struct hdd_mld_mac_info *mac_info;

	mac_info = &hdd_ctx->mld_mac_info;
	for (i = 0; i < WLAN_MAX_MLD; i++) {
		qdf_mem_copy(mac_info->mld_mac_list[i].mld_addr.bytes,
			     hw_macaddr.bytes, QDF_MAC_ADDR_SIZE);
		mldaddr_b2 = mac_info->mld_mac_list[i].mld_addr.bytes[2];
		tmp_br2 = ((mldaddr_b2 >> 4 & INTF_MACADDR_MASK) + i) &
			  INTF_MACADDR_MASK;
		mldaddr_b2 += tmp_br2;

		/* XOR-ing bit-24 of the mac address. This will give enough
		 * mac address range before collision
		 */
		mldaddr_b2 ^= (1 << 7);

		/* Set locally administered bit */
		mac_info->mld_mac_list[i].mld_addr.bytes[0] |= 0x02;
		mac_info->mld_mac_list[i].mld_addr.bytes[2] = mldaddr_b2;
		hdd_debug("mld addr[%d]: "
			QDF_MAC_ADDR_FMT, i,
		    QDF_MAC_ADDR_REF(mac_info->mld_mac_list[i].mld_addr.bytes));

		mac_info->mld_mac_list[i].device_mode = QDF_MAX_NO_OF_MODE;
		mac_info->num_mld_addr++;

		hdd_debug("num_mld_addr: %d", mac_info->num_mld_addr);
	};
}

