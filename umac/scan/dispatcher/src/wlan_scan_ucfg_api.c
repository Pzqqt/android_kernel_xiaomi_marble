/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan north bound interface definitions
 */

#include <wlan_cmn.h>
#include <wlan_scan_utils_api.h>
#include <wlan_scan_ucfg_api.h>
#include "../../core/src/wlan_scan_main.h"
#include <wlan_objmgr_global_obj.h>

QDF_STATUS ucfg_scan_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("Failed to register psoc create handler");
		goto fail_create_psoc;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_destroyed_notification, NULL);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		scm_info("scan create and delete handler registered with objmgr");
		return QDF_STATUS_SUCCESS;
	}
	scm_err("Failed to create psoc delete handler");

	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
			wlan_scan_psoc_created_notification, NULL);
fail_create_psoc:
	return status;
}

QDF_STATUS ucfg_scan_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister psoc create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_SCAN,
				wlan_scan_psoc_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister psoc delete handler");

	return status;
}

static QDF_STATUS wlan_scan_global_init(struct wlan_scan_obj *scan_obj)
{

	scan_obj->scan_def.active_dwell = SCAN_ACTIVE_DWELL_TIME;
	scan_obj->scan_def.passive_dwell = SCAN_PASSIVE_DWELL_TIME;
	scan_obj->scan_def.max_rest_time = SCAN_MAX_REST_TIME;
	scan_obj->scan_def.min_rest_time = SCAN_MIN_REST_TIME;
	scan_obj->scan_def.conc_active_dwell = SCAN_CONC_ACTIVE_DWELL_TIME;
	scan_obj->scan_def.conc_passive_dwell = SCAN_CONC_PASSIVE_DWELL_TIME;
	scan_obj->scan_def.conc_max_rest_time = SCAN_CONC_MAX_REST_TIME;
	scan_obj->scan_def.conc_min_rest_time = SCAN_CONC_MIN_REST_TIME;
	scan_obj->scan_def.conc_idle_time = SCAN_CONC_IDLE_TIME;
	scan_obj->scan_def.repeat_probe_time = SCAN_REPEAT_PROBE_TIME;
	scan_obj->scan_def.probe_spacing_time = SCAN_PROBE_SPACING_TIME;
	scan_obj->scan_def.probe_delay = SCAN_PROBE_DELAY;
	scan_obj->scan_def.burst_duration = SCAN_BURST_DURATION;
	scan_obj->scan_def.max_scan_time = SCAN_MAX_SCAN_TIME;
	scan_obj->scan_def.num_probes = SCAN_NUM_PROBES;
	scan_obj->scan_def.scan_cache_aging_time = SCAN_CACHE_AGING_TIME;
	scan_obj->scan_def.max_bss_per_pdev = SCAN_MAX_BSS_PDEV;
	scan_obj->scan_def.max_num_scan_allowed = SCAN_MAX_NUM_SCAN_ALLOWED;
	scan_obj->scan_def.scan_priority = SCAN_PRIORITY;
	scan_obj->scan_def.idle_time = SCAN_NETWORK_IDLE_TIMEOUT;
	/* scan contrl flags */
	scan_obj->scan_def.scan_f_passive = true;
	scan_obj->scan_def.scan_f_2ghz = true;
	scan_obj->scan_def.scan_f_5ghz = true;
	/* scan event flags */
	scan_obj->scan_def.scan_ev_started = true;
	scan_obj->scan_def.scan_ev_completed = true;
	scan_obj->scan_def.scan_ev_bss_chan = true;
	scan_obj->scan_def.scan_ev_foreign_chan = true;
	scan_obj->scan_def.scan_ev_dequeued = true;

	qdf_atomic_init(&scan_obj->scan_ids);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_scan_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_scan_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}


QDF_STATUS ucfg_scan_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (scan_obj == NULL) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_FAILURE;
	}
	/* Initialize the scan Globals */
	wlan_scan_global_init(scan_obj);

	scm_db_init(psoc);

	return status;
}

QDF_STATUS ucfg_scan_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (scan_obj == NULL) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_FAILURE;
	}

	scm_db_deinit(psoc);

	return status;
}
