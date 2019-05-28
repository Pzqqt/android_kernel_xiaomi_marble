/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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
/**
 * DOC: declare internal APIs related to the blacklist component
 */

#include <wlan_objmgr_pdev_obj.h>
#include <wlan_blm_core.h>
#include <qdf_mc_timer.h>
#include <wlan_scan_public_structs.h>
#include "wlan_blm_tgt_api.h"

#define SECONDS_TO_MS(params)       (params * 60)
#define MINUTES_TO_MS(params)       (SECONDS_TO_MS(params) * 1000)

QDF_STATUS
blm_filter_bssid(struct wlan_objmgr_pdev *pdev,
		 qdf_list_t *scan_list)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
blm_add_bssid_to_reject_list(struct wlan_objmgr_pdev *pdev,
			     struct reject_ap_info *ap_info)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
blm_add_userspace_black_list(struct wlan_objmgr_pdev *pdev,
			     struct qdf_mac_addr *bssid_black_list,
			     uint8_t num_of_bssid)
{
	return QDF_STATUS_SUCCESS;
}

uint8_t
blm_get_bssid_reject_list(struct wlan_objmgr_pdev *pdev,
			  struct reject_ap_config_params *reject_list,
			  uint8_t max_bssid_to_be_filled,
			  enum blm_reject_ap_type reject_ap_type)
{
	return 0;
}
