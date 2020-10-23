/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * @THERMAL_SHUTOFF: shut down the tx completely
 * @THERMAL_UNKNOWN: unknown level from target.
 */
enum thermal_throttle_level {
	 THERMAL_FULLPERF,
	 THERMAL_MITIGATION,
	 THERMAL_SHUTOFF,
	 THERMAL_UNKNOWN,
};

#endif /* _WLAN_THERMAL_PUBLIC_STRUCT_H_ */
