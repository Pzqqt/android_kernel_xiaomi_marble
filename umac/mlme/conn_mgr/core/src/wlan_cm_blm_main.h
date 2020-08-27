/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Define APIs related to the Blacklist manager
 */

#ifndef _WLAN_CM_BLM_MAIN_H_
#define _WLAN_CM_BLM_MAIN_H_
#include <wlan_scan_public_structs.h>

/**
 * blm_entry_iter_obj - Object of blm iter function
 * @db_entry: scan entry object
 * @sta_vdev: station's vdev object
 * @match: Hold the operation result
 */
struct blm_entry_iter_obj {
	struct scan_cache_entry *db_entry;
	struct wlan_objmgr_vdev *sta_vdev;
	bool match;
};
#endif
