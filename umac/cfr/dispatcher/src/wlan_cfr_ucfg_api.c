/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include <wlan_cfr_ucfg_api.h>
#include "../../core/inc/cfr_defs_i.h"
#include <wlan_cfr_utils_api.h>
#include <wlan_cfr_tgt_api.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_pdev_obj.h>

int ucfg_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
			   struct wlan_objmgr_peer *peer,
			   struct cfr_capture_params *params)
{
	struct pdev_cfr *pa;
	struct peer_cfr *pe;

	if (NULL == pdev) {
		cfr_err("PDEV is NULL!\n");
		return -EINVAL;
	}
	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (NULL == pa) {
		cfr_err("PDEV cfr object is NULL!\n");
		return -EINVAL;
	}

	/* Get peer private object */
	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (NULL == pe) {
		cfr_err("PEER cfr object is NULL!\n");
		return -EINVAL;
	}
	pe->bandwidth = params->bandwidth;
	pe->period = params->period;
	pe->capture_method = params->method;

	return tgt_cfr_start_capture(pdev, peer, params);
}

int ucfg_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer)
{
	if (NULL == pdev) {
		cfr_err("pdev is null!\n");
		return -EINVAL;
	}

	if (NULL == peer) {
		cfr_err("peer is null!\n");
		return -EINVAL;
	}

	return tgt_cfr_stop_capture(pdev, peer);
}

int ucfg_cfr_list_peers(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}
