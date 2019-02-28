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
	int status;
	struct pdev_cfr *pa;
	struct peer_cfr *pe;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (NULL == pa) {
		cfr_err("PDEV cfr object is NULL!\n");
		return -EINVAL;
	}

	if (!(pa->is_cfr_capable)) {
		qdf_info("cfr is not supported on this chip\n");
		return -EINVAL;
	}

	/* Get peer private object */
	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (NULL == pe) {
		cfr_err("PEER cfr object is NULL!\n");
		return -EINVAL;
	}

	if ((params->period < 0) || (params->period > MAX_CFR_PRD) ||
		(params->period % 10) ||
		(!(params->period) && (pa->cfr_timer_enable))) {
		qdf_info("Invalid period\n");
		return -EINVAL;
	}

	if (params->period) {
		if (pa->cfr_current_sta_count == pa->cfr_max_sta_count) {
			qdf_info("max periodic cfr clients reached\n");
			return -EINVAL;
		}
		if (!(pe->request))
			pa->cfr_current_sta_count++;
	}

	status = tgt_cfr_start_capture(pdev, peer, params);

	if (status == 0) {
		pe->bandwidth = params->bandwidth;
		pe->period = params->period;
		pe->capture_method = params->method;
		pe->request = PEER_CFR_CAPTURE_ENABLE;
	} else
		pa->cfr_current_sta_count--;

	return status;
}

int ucfg_cfr_set_timer(struct wlan_objmgr_pdev *pdev, uint32_t value)
{
	struct pdev_cfr *pa;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL) {
		cfr_err("PDEV cfr object is NULL!\n");
		return -EINVAL;
	}

	if (!(pa->is_cfr_capable)) {
		qdf_info("cfr is not supported on this chip\n");
		return -EINVAL;
	}

	return tgt_cfr_enable_cfr_timer(pdev, value);
}
qdf_export_symbol(ucfg_cfr_set_timer);

int ucfg_cfr_get_timer(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL) {
		cfr_err("PDEV cfr object is NULL!\n");
		return -EINVAL;
	}

	if (!(pa->is_cfr_capable)) {
		qdf_info("cfr is not supported on this chip\n");
		return -EINVAL;
	}

	return pa->cfr_timer_enable;
}
qdf_export_symbol(ucfg_cfr_get_timer);

int ucfg_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer)
{
	int status;
	struct peer_cfr *pe;
	struct pdev_cfr *pa;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL) {
		cfr_err("PDEV cfr object is NULL!\n");
		return -EINVAL;
	}

	if (!(pa->is_cfr_capable)) {
		qdf_info("cfr is not supported on this chip\n");
		return -EINVAL;
	}

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (pe == NULL) {
		cfr_err("PEER cfr object is NULL!\n");
		return -EINVAL;
	}

	if ((pe->period) && (pe->request))
		status = tgt_cfr_stop_capture(pdev, peer);
	else {
		qdf_info("periodic cfr not started for the client\n");
		return -EINVAL;
	}

	if (status == 0)
		pe->request = PEER_CFR_CAPTURE_DISABLE;
		pa->cfr_current_sta_count--;

	return status;
}

int ucfg_cfr_list_peers(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}
