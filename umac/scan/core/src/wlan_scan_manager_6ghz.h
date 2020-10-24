/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan manager functionality
 */

#ifndef _WLAN_SCAN_MANAGER_6GHZ_API_H_
#define _WLAN_SCAN_MANAGER_6GHZ_API_H_

#include "wlan_scan_main.h"

#ifdef CONFIG_BAND_6GHZ

/*
 * Userspace/framework tracks last few days(e.g: 10 days) history of
 * connected APs and maintains best 10 channels out of that list by
 * considering multiple parameters like scoring, location etc.
 * It's likely to find an AP in one of these channels when a scan issued.
 * So, framework issues first scan request with a maximum of 10 channels
 * Any scan request which consist of more than 10 channels can be considered as
 * full scan
 */
#define FULL_SCAN_CH_COUNT_MIN_BY_USERSPACE 11

/**
 * scm_update_6ghz_channel_list() - Update 6ghz channel list in the scan req
 * @req: scan start request
 * @scan_obj: scan component object
 *
 * Filter out the unsupported 6ghz channels from scan request and add supported
 * 6ghz channels based on the ini.
 *
 * Return: None
 */
void
scm_update_6ghz_channel_list(struct scan_start_request *req,
			     struct wlan_scan_obj *scan_obj);
#else
static inline void
scm_update_6ghz_channel_list(struct scan_start_request *req,
			     struct wlan_scan_obj *scan_obj)
{
}
#endif

#endif
