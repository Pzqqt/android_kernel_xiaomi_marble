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
 * DOC: Declare various struct, macros which are used for obj mgmt  in ftm time
 * sync.
 *
 * Note: This file shall not contain public API's prototype/declarations.
 *
 */

#ifndef _WLAN_TIME_SYNC_FTM_PUBLIC_STRUCT_H_
#define _WLAN_TIME_SYNC_FTM_PUBLIC_STRUCT_H_

/**
 * struct wlan_ftm_timesync_tx_ops - structure of tx operation function
 *				     pointers for ftm timesync component
 * @ftm_time_sync_send_qtime: send qtime wmi cmd to FW
 * @ftm_time_sync_send_trigger: send ftm time sync trigger cmd
 *
 */
struct wlan_ftm_timesync_tx_ops {
	QDF_STATUS (*ftm_time_sync_send_qtime)(struct wlan_objmgr_psoc *psoc,
					       uint32_t vdev_id,
					       uint64_t lpass_ts);
	QDF_STATUS (*ftm_time_sync_send_trigger)(struct wlan_objmgr_psoc *psoc,
						 uint32_t vdev_id, bool mode);
};

/**
 * struct wlan_ftm_timesync_rx_ops - structure of rx operation function
 *				     pointers for ftm timesync component
 * @ftm_timesync_register_start_stop: register ftm timesync start stop event
 * @ftm_timesync_regiser_master_slave_offset: register master slave qtime
 *					      offset event
 */
struct wlan_ftm_timesync_rx_ops {
	QDF_STATUS (*ftm_timesync_register_start_stop)
					(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*ftm_timesync_regiser_master_slave_offset)
					(struct wlan_objmgr_psoc *psoc);
};
#endif /*_WLAN_TIME_SYNC_FTM_PUBLIC_STRUCT_H_ */
