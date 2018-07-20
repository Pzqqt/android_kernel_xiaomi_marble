/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains all SCAN component's APIs
 */

#include "cfg_ucfg_api.h"
#include "wlan_scan_api.h"

void wlan_scan_cfg_get_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;
	*dwell_time = scan_obj->scan_def.passive_dwell;
}

void wlan_scan_cfg_set_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					 uint32_t dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;
	scan_obj->scan_def.passive_dwell = dwell_time;
}

void wlan_scan_cfg_get_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					uint32_t *dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;
	*dwell_time = scan_obj->scan_def.active_dwell;
}

void wlan_scan_cfg_set_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					uint32_t dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;
	scan_obj->scan_def.active_dwell = dwell_time;
}

void wlan_scan_cfg_get_conc_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					     uint32_t *dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	*dwell_time = scan_obj->scan_def.conc_active_dwell;
}

void wlan_scan_cfg_set_conc_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					     uint32_t dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	scan_obj->scan_def.conc_active_dwell = dwell_time;
}

void wlan_scan_cfg_get_conc_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					      uint32_t *dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	*dwell_time = scan_obj->scan_def.conc_passive_dwell;
}

void wlan_scan_cfg_set_conc_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					      uint32_t dwell_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	scan_obj->scan_def.conc_passive_dwell = dwell_time;
}

void wlan_scan_cfg_get_conc_max_resttime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *rest_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	*rest_time = scan_obj->scan_def.conc_max_rest_time;
}

void wlan_scan_cfg_get_conc_min_resttime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *rest_time)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return;

	*rest_time = scan_obj->scan_def.conc_min_rest_time;
}
