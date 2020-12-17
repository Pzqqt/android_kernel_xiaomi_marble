/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/* This file has WIN specific WMI structures and macros. */

#ifndef _WMI_UNIFIED_AP_PARAMS_H_
#define _WMI_UNIFIED_AP_PARAMS_H_

/* Country code string length*/
#define COUNTRY_CODE_LEN 2
/* Civic information size in bytes */
#define CIVIC_INFO_LEN 256

/**
 * wmi_wifi_pos_lcr_info - LCR info structure
 * @pdev_id: host pdev id
 * @req_id: LCR request id
 * @civic_len: Civic info length
 * @country_code: Country code string
 * @civic_info: Civic info
 */
struct wmi_wifi_pos_lcr_info {
	uint32_t pdev_id;
	uint16_t req_id;
	uint32_t civic_len;
	uint8_t  country_code[COUNTRY_CODE_LEN];
	uint8_t civic_info[CIVIC_INFO_LEN];
};

/**
 * lci_motion_pattern - LCI motion pattern
 * @LCI_MOTION_NOT_EXPECTED: Not expected to change location
 * @LCI_MOTION_EXPECTED: Expected to change location
 * @LCI_MOTION_UNKNOWN: Movement pattern unknown
 */
enum wifi_pos_motion_pattern {
	wifi_pos_MOTION_NOT_EXPECTED = 0,
	wifi_pos_MOTION_EXPECTED     = 1,
	wifi_pos_MOTION_UNKNOWN      = 2
};

/**
 * wifi_pos_lci_info - LCI info structure
 * @pdev_id: host pdev_id
 * @req_id: LCI request id
 * @latitude: Latitude value
 * @longitude: Longitude value
 * @altitude: Altitude value
 * @latitude_unc: Latitude uncertainty value
 * @longitude_unc: Longitude uncertainty value
 * @altitude_unc: Altitude uncertainty value
 * @motion_pattern: Motion pattern
 * @floor: Floor value
 * @height_above_floor: Height above floor
 * @height_unc: Height uncertainty value
 * @usage_rules: Usage rules
 */
struct wifi_pos_lci_info {
	uint32_t pdev_id;
	uint16_t req_id;
	int64_t latitude;
	int64_t longitude;
	int32_t altitude;
	uint8_t latitude_unc;
	uint8_t longitude_unc;
	uint8_t altitude_unc;
	enum wifi_pos_motion_pattern motion_pattern;
	int32_t floor;
	int32_t height_above_floor;
	int32_t height_unc;
	uint32_t usage_rules;
};
#endif
