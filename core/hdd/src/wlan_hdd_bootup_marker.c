/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_hdd_bootup_marker.c
 *
 *  WLAN Host Device Driver bootup marker implementation
 *
 */

#include "wlan_hdd_bootup_marker.h"
#include <soc/qcom/boot_stats.h>
#include <qdf_trace.h>

#ifdef DYNAMIC_SINGLE_CHIP
#define PREFIX_MARKER DYNAMIC_SINGLE_CHIP
#else
#ifdef MULTI_IF_NAME
#define PREFIX_MARKER MULTI_IF_NAME
#else
#define PREFIX_MARKER ""
#endif
#endif

#ifndef WLAN_PLACEMARKER_PREFIX
#define WLAN_PLACEMARKER_PREFIX ""
#define WLAN_MARKER WLAN_PLACEMARKER_PREFIX PREFIX_MARKER " "
#else
#define WLAN_MARKER WLAN_PLACEMARKER_PREFIX " " PREFIX_MARKER " "
#endif

void hdd_place_marker(struct hdd_adapter *adapter,
		      const char *format,
		      uint8_t *mac)
{
	char marker[100];

	if (adapter) {
		if (mac)
			qdf_snprintf(marker, sizeof(marker),
				     (WLAN_MARKER "%s:%s %s:" QDF_MAC_ADDR_FMT),
				     (adapter->dev->name),
				     qdf_opmode_str(adapter->device_mode),
				     format, QDF_MAC_ADDR_REF(mac));
		else
			qdf_snprintf(marker, sizeof(marker),
				     (WLAN_MARKER "%s:%s %s"),
				     (adapter->dev->name),
				     qdf_opmode_str(adapter->device_mode),
				     format);
	} else {
		qdf_snprintf(marker, sizeof(marker),
			     (WLAN_MARKER "%s"),
			     format);
	}

	place_marker(marker);
}

