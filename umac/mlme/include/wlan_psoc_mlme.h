/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: Define PSOC MLME structure
 */
#ifndef _WLAN_PSOC_MLME_H_
#define _WLAN_PSOC_MLME_H_

#include <wlan_ext_mlme_obj_types.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>
#include <qdf_timer.h>

/* Max RNR size given max vaps are 16 */
#define MAX_RNR_SIZE 256

/**
 * struct wlan_rnr_global_cache - RNR cache buffer per soc
 * @rnr_buf: RNR cache buffer
 * @rnr_cnt: Count of APs in cache
 * @rnr_size: Size of RNR cache (RNR IE)
 */
struct wlan_6ghz_rnr_global_cache {
	char rnr_buf[MAX_RNR_SIZE];
	int rnr_cnt;
	uint16_t rnr_size;
};

/**
 * struct psoc_mlme_obj -  PSoC MLME component object
 * @psoc:                  PSoC object
 * @ext_psoc_ptr:        PSoC legacy pointer
 * @psoc_vdev_rt:       PSoC Vdev response timer
 * @psoc_mlme_wakelock:     Wakelock to prevent system going to suspend
 * @rnr_6ghz_cache:        Cache of 6Ghz vap in RNR ie format
 */
struct psoc_mlme_obj {
	struct wlan_objmgr_psoc *psoc;
	mlme_psoc_ext_t *ext_psoc_ptr;
	struct vdev_response_timer psoc_vdev_rt[WLAN_UMAC_PSOC_MAX_VDEVS];
#ifdef FEATURE_VDEV_RSP_WAKELOCK
	struct psoc_mlme_wakelock psoc_mlme_wakelock;
#endif
	struct wlan_6ghz_rnr_global_cache rnr_6ghz_cache;
};

#endif
