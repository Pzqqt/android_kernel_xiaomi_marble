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

/**
 * struct psoc_mlme_obj -  PSoC MLME component object
 * @psoc:                  PSoC object
 * @ext_psoc_ptr:        PSoC legacy pointer
 * @psoc_vdev_rt:       PSoC Vdev response timer
 */
struct psoc_mlme_obj {
	struct wlan_objmgr_psoc *psoc;
	mlme_psoc_ext_t *ext_psoc_ptr;
	struct vdev_response_timer psoc_vdev_rt[WLAN_UMAC_PSOC_MAX_VDEVS];
};

#endif
