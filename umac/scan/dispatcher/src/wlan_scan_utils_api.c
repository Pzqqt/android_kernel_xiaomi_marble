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
 * DOC: Defines scan utility functions
 */

#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_utils_api.h>
#include <../../core/src/wlan_scan_cache_db.h>
#include <../../core/src/wlan_scan_main.h>

const char*
util_scan_get_ev_type_name(enum scan_event_type type)
{
	static const char * const event_name[] = {
		[SCAN_EVENT_TYPE_STARTED] = "STARTED",
		[SCAN_EVENT_TYPE_COMPLETED] = "COMPLETED",
		[SCAN_EVENT_TYPE_BSS_CHANNEL] = "HOME_CHANNEL",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL] = "FOREIGN_CHANNEL",
		[SCAN_EVENT_TYPE_DEQUEUED] = "DEQUEUED",
		[SCAN_EVENT_TYPE_PREEMPTED] = "PREEMPTED",
		[SCAN_EVENT_TYPE_START_FAILED] = "START_FAILED",
		[SCAN_EVENT_TYPE_RESTARTED] = "RESTARTED",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT] = "FOREIGN_CHANNEL_EXIT",
		[SCAN_EVENT_TYPE_SUSPENDED] = "SUSPENDED",
		[SCAN_EVENT_TYPE_RESUMED] = "RESUMED",
		[SCAN_EVENT_TYPE_NLO_COMPLETE] = "NLO_COMPLETE",
		[SCAN_EVENT_TYPE_INVALID] = "INVALID",
		[SCAN_EVENT_TYPE_GPIO_TIMEOUT] = "GPIO_TIMEOUT",
		[SCAN_EVENT_TYPE_RADIO_MEASUREMENT_START] =
			"RADIO_MEASUREMENT_START",
		[SCAN_EVENT_TYPE_RADIO_MEASUREMENT_END] =
			"RADIO_MEASUREMENT_END",
		[SCAN_EVENT_TYPE_BSSID_MATCH] = "BSSID_MATCH",
		[SCAN_EVENT_TYPE_FOREIGN_CHANNEL_GET_NF] =
			"FOREIGN_CHANNEL_GET_NF",
	};

	if (type >= SCAN_EVENT_TYPE_MAX) {
		scm_err("unknown type : %d", type);
		QDF_ASSERT(0);
		return "UNKNOWN";
	}

	return event_name[type];
}


const char*
util_scan_get_ev_reason_name(enum scan_completion_reason reason)
{
	static const char * const reason_name[] = {
		[SCAN_REASON_NONE] = "NONE",
		[SCAN_REASON_COMPLETED] = "COMPLETED",
		[SCAN_REASON_CANCELLED] = "CANCELLED",
		[SCAN_REASON_PREEMPTED] = "PREEMPTED",
		[SCAN_REASON_TIMEDOUT] = "TIMEDOUT",
		[SCAN_REASON_INTERNAL_FAILURE] = "INTERNAL_FAILURE",
		[SCAN_REASON_SUSPENDED] = "SUSPENDED",
		[SCAN_REASON_RUN_FAILED] = "RUN_FAILED",
		[SCAN_REASON_TERMINATION_FUNCTION] = "TERMINATION_FUNCTION",
		[SCAN_REASON_MAX_OFFCHAN_RETRIES] = "MAX_OFFCHAN_RETRIES",
	};

	if (reason >= SCAN_REASON_MAX) {
		scm_err("unknown reason : %d", reason);
		QDF_ASSERT(0);
		return "UNKNOWN";
	}

	return reason_name[reason];
}

qdf_time_t
util_get_last_scan_time(struct wlan_objmgr_vdev *vdev)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!vdev) {
		scm_warn("null vdev");
		QDF_ASSERT(0);
		return 0;
	}
	pdev_id = wlan_scan_vdev_get_pdev_id(vdev);
	scan_obj = wlan_vdev_get_scan_obj(vdev);

	return scan_obj->pdev_info[pdev_id].last_scan_time;
}
