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
 * DOC : wlan_hdd_eht.c
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_eht.h"
#include "osif_sync.h"
#include "wlan_utility.h"
#include "wlan_mlme_ucfg_api.h"

void hdd_update_tgt_eht_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg)
{
}

void wlan_hdd_check_11be_support(struct hdd_beacon_data *beacon,
				 struct sap_config *config)
{
	const uint8_t *ie;

	ie = wlan_get_ext_ie_ptr_from_ext_id(EHT_CAP_OUI_TYPE, EHT_CAP_OUI_SIZE,
					     beacon->tail, beacon->tail_len);
	if (ie)
		config->SapHw_mode = eCSR_DOT11_MODE_11be;
}
