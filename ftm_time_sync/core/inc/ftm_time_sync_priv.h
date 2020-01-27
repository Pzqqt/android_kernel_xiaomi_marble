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

/**
 * DOC: Declare private API which shall be used internally only
 * in ftm_timesync component. This file shall include prototypes of
 * ftm_timesync parsing and send logic.
 *
 * Note: This API should be never accessed out of ftm_timesync component.
 */

#ifndef _FTM_TIME_SYNC_PRIV_STRUCT_H_
#define _FTM_TIME_SYNC_PRIV_STRUCT_H_

#include <qdf_list.h>
#include <qdf_types.h>
#include "ftm_time_sync_objmgr.h"

#define WLAN_FTM_TIME_SYNC_PAIR_MAX 32

/**
 * struct wlan_time_sync_pair - wlan time sync pair
 * @qtime_master: master qtime
 * @qtime_slave: slave qtime
 */
struct wlan_time_sync_pair {
	uint64_t qtime_master;
	uint64_t qtime_slave;
};

/**
 * struct ftm_timesync_vdev_priv - Private object to be stored in vdev
 * @qtime_ref: qtime ref
 * @mac_ref: mac time ref
 * @time_pair: array of master/slave qtime pair
 */

struct ftm_timesync_priv {
	uint64_t qtime_ref;
	uint64_t mac_ref;
	struct wlan_time_sync_pair time_pair[WLAN_FTM_TIME_SYNC_PAIR_MAX];
};

/**
 * struct ftm_timesync_vdev_priv - Private object to be stored in vdev
 * @vdev: pointer to vdev object
 * @ftm_ts_priv: time sync private struct
 */
struct ftm_timesync_vdev_priv {
	struct wlan_objmgr_vdev *vdev;
	struct ftm_timesync_priv ftm_ts_priv;
};

#endif /* End  of _FTM_TIME_SYNC_PRIV_STRUCT_H_ */
