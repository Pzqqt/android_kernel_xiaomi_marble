/* Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: declare the thermal public structure
 */
#ifndef _WLAN_THERMAL_PUBLIC_STRUCT_H_
#define _WLAN_THERMAL_PUBLIC_STRUCT_H_

/**
 * enum thermal_throttle_level - firmware offload throttle level
 * @THERMAL_FULLPERF: no any throtting
 * @THERMAL_MITIGATION: throtting tx to do mitigation
 * @THERMAL_SHUTOFF: shut off the tx completely
 * @THERMAL_SHUTDOWN_TARGET: target will be shutdown entirely
 * @THERMAL_UNKNOWN: unknown level from target.
 */
enum thermal_throttle_level {
	 THERMAL_FULLPERF,
	 THERMAL_MITIGATION,
	 THERMAL_SHUTOFF,
	 THERMAL_SHUTDOWN_TARGET,
	 THERMAL_UNKNOWN,
};

enum thermal_stats_request_type {
	thermal_stats_none = 0,
	thermal_stats_init,
	thermal_stats_req,
	thermal_stats_clear,
	thermal_stats_current_all_sensors_temp,
};

/**
 * thermal_throt_level_stats - thermal throttle info from Target
 * @start_temp_level: Start temperature range to capture thermal stats
 * @end_temp_level: End temperature range to capture thermal stats
 * @total_time_ms_lo: Start time for every thermal stats level in msec
 * @total_time_ms_hi: End time for every thermal stats level in msec
 * @num_entry: Thermal stats counter for every time temp level for this range
 */
struct thermal_throt_level_stats {
	uint32_t start_temp_level;
	uint32_t end_temp_level;
	uint32_t total_time_ms_lo;
	uint32_t total_time_ms_hi;
	uint32_t num_entry;
};

#endif /* _WLAN_THERMAL_PUBLIC_STRUCT_H_ */
