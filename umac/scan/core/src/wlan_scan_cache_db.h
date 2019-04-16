/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan cache entry api
 */

#ifndef _WLAN_SCAN_CACHE_DB_H_
#define _WLAN_SCAN_CACHE_DB_H_

#include <scheduler_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>

#define SCAN_HASH_SIZE 64
#define SCAN_GET_HASH(addr) \
	(((const uint8_t *)(addr))[QDF_MAC_ADDR_SIZE - 1] % SCAN_HASH_SIZE)

#define SCM_PCL_RSSI_THRESHOLD -75
#define BEST_CANDIDATE_MAX_BSS_SCORE 10000

#define ADJACENT_CHANNEL_RSSI_THRESHOLD -80

/**
 * struct scan_dbs - scan cache data base definition
 * @num_entries: number of scan entries
 * @scan_hash_tbl: link list of bssid hashed scan cache entries for a pdev
 */
struct scan_dbs {
	uint32_t num_entries;
	qdf_spinlock_t scan_db_lock;
	qdf_list_t scan_hash_tbl[SCAN_HASH_SIZE];
};

/**
 * struct scan_bcn_probe_event - beacon/probe info
 * @frm_type: frame type
 * @rx_data: mgmt rx data
 * @psoc: psoc pointer
 * @buf: rx frame
 */
struct scan_bcn_probe_event {
	uint32_t frm_type;
	struct mgmt_rx_event_params *rx_data;
	struct wlan_objmgr_psoc *psoc;
	qdf_nbuf_t buf;
};

/**
 * scm_handle_bcn_probe() - Process beacon and probe rsp
 * @msg: schedular msg with bcn info;
 *
 * API to handle the beacon/probe resp. msg->bodyptr will be consumed and freed
 * by this func
 *
 * Return: QDF status.
 */
QDF_STATUS scm_handle_bcn_probe(struct scheduler_msg *msg);

/**
 * __scm_handle_bcn_probe() - Process beacon and probe rsp
 * @bcn: beacon info;
 *
 * API to handle the beacon/probe resp. bcn will be consumed and freed by this
 * func
 *
 * Return: QDF status.
 */
QDF_STATUS __scm_handle_bcn_probe(struct scan_bcn_probe_event *bcn);

/**
 * scm_age_out_entries() - Age out entries older than aging time
 * @psoc: psoc pointer
 * @scan_db: scan database
 *
 * Return: void.
 */
void scm_age_out_entries(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db);

/**
 * scm_get_scan_result() - fetches scan result
 * @pdev: pdev info
 * @filter: Filters
 *
 * This function fetches scan result
 *
 * Return: scan list
 */
qdf_list_t *scm_get_scan_result(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter);

/**
 * scm_purge_scan_results() - purge the scan list
 * @scan_result: scan list to be purged
 *
 * This function purge the temp scan list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_purge_scan_results(qdf_list_t *scan_result);

/**
 * scm_update_scan_mlme_info() - updates scan entry with mlme data
 * @pdev: pdev object
 * @scan_entry: source scan entry to read mlme info
 *
 * This function updates scan db with scan_entry->mlme_info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_update_scan_mlme_info(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *scan_entry);

/**
 * scm_flush_results() - flush scan entries matching the filter
 * @pdev: vdev object
 * @filter: filter to flush the scan entries
 *
 * Flush scan entries matching the filter.
 *
 * Return: QDF status.
 */
QDF_STATUS scm_flush_results(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter);

/**
 * scm_filter_valid_channel() - The Public API to filter scan result
 * based on valid channel list
 * @pdev: pdev object
 * @chan_list: valid channel list
 * @num_chan: number of valid channels
 *
 * The Public API to to filter scan result
 * based on valid channel list.
 *
 * Return: void.
 */
void scm_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint8_t *chan_list, uint32_t num_chan);

/**
 * scm_iterate_scan_db() - function to iterate scan table
 * @pdev: pdev object
 * @func: iterator function pointer
 * @arg: argument to be passed to func()
 *
 * API, this API iterates scan table and invokes func
 * on each scan enetry by passing scan entry and arg.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
scm_iterate_scan_db(struct wlan_objmgr_pdev *pdev,
	scan_iterator_func func, void *arg);

/**
 * scm_scan_register_bcn_cb() - API to register api to indicate bcn/probe
 * as soon as they are received
 * @pdev: psoc
 * @cb: callback to be registered
 * @type: Type of callback to be registered
 *
 * Return: enum scm_scan_status
 */
QDF_STATUS scm_scan_register_bcn_cb(struct wlan_objmgr_psoc *psoc,
	update_beacon_cb cb, enum scan_cb_type type);

/**
 * scm_db_init() - API to init scan db
 * @psoc: psoc
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_db_init(struct wlan_objmgr_psoc *psoc);

/**
 * scm_db_deinit() - API to deinit scan db
 * @psoc: psoc
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_db_deinit(struct wlan_objmgr_psoc *psoc);

/**
 * scm_validate_scoring_config() - validate score config
 * @score_cfg: config to be validated
 *
 * Return: void
 */
void scm_validate_scoring_config(
			struct scoring_config *score_cfg);

/**
 * scm_scan_update_mlme_by_bssinfo() - updates scan entry with mlme data
 * @pdev: pdev object
 * @bss_info: BSS information
 *
 * This function updates scan db with scan_entry->mlme_info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_scan_update_mlme_by_bssinfo(struct wlan_objmgr_pdev *pdev,
		struct bss_info *bss_info, struct mlme_info *mlme);
#endif
