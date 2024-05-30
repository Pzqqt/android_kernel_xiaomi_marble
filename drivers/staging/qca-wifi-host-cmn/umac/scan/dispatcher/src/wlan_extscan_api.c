/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains all EXTSSCAN component's APIs
 */

#include "wlan_extscan_api.h"
#include "cfg_ucfg_api.h"

bool extscan_get_enable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return false;
	}

	return scan_obj->extscan_cfg.extscan_enabled;
}

void extscan_get_passive_max_time(struct wlan_objmgr_psoc *psoc,
				  uint32_t *passive_max_chn_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return;
	}

	*passive_max_chn_time =
		scan_obj->extscan_cfg.extscan_passive_max_chn_time;
}

void extscan_get_active_max_time(struct wlan_objmgr_psoc *psoc,
				 uint32_t *active_max_chn_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return;
	}

	*active_max_chn_time =
		scan_obj->extscan_cfg.extscan_active_max_chn_time;
}

void extscan_get_active_min_time(struct wlan_objmgr_psoc *psoc,
				 uint32_t *active_min_chn_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return;
	}

	*active_min_chn_time =
		scan_obj->extscan_cfg.extscan_active_min_chn_time;
}

QDF_STATUS
wlan_extscan_global_init(struct wlan_objmgr_psoc *psoc,
			 struct wlan_scan_obj *scan_obj)
{
	struct extscan_def_config *extscan_def = &scan_obj->extscan_cfg;

	extscan_def->extscan_enabled = true;
	extscan_def->extscan_passive_max_chn_time =
			cfg_get(psoc, CFG_EXTSCAN_PASSIVE_MAX_CHANNEL_TIME);
	extscan_def->extscan_passive_min_chn_time =
			cfg_get(psoc, CFG_EXTSCAN_PASSIVE_MIN_CHANNEL_TIME);
	extscan_def->extscan_active_max_chn_time =
			cfg_get(psoc, CFG_EXTSCAN_ACTIVE_MAX_CHANNEL_TIME);
	extscan_def->extscan_active_min_chn_time =
			cfg_get(psoc, CFG_EXTSCAN_ACTIVE_MIN_CHANNEL_TIME);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_extscan_global_deinit()
{
	return QDF_STATUS_SUCCESS;
}
