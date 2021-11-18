/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_sysfs_bmiss.c
 *
 * Implementation for creating sysfs file bmiss
 */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_sysfs.h>
#include "osif_vdev_sync.h"
#include <wlan_hdd_sysfs_bmiss.h>
#include <wlan_hdd_stats.h>
#include <wlan_cp_stats_mc_ucfg_api.h>

static struct infra_cp_stats_event*
wlan_hdd_get_bmiss(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	int errno;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!hdd_sta_ctx) {
		hdd_debug("hdd_sta_ctx received NULL");
		return NULL;
	}
	qdf_mem_copy(peer_mac, hdd_sta_ctx->conn_info.bssid.bytes,
		     QDF_MAC_ADDR_SIZE);
	return wlan_cfg80211_mc_bmiss_get_infra_cp_stats(adapter->vdev,
							 peer_mac, &errno);
}

static ssize_t
__hdd_sysfs_bmiss_show(struct net_device *net_dev, char *buf)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	struct infra_cp_stats_event *ev_ptr;
	ssize_t ret = 0;
	int idx = 0;

	adapter = netdev_priv(net_dev);
	if (hdd_validate_adapter(adapter))
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ev_ptr = wlan_hdd_get_bmiss(adapter);
	if (!ev_ptr) {
		hdd_err_rl("GET_BMISS failed");
		return ret;
	}

	ret = scnprintf(buf, PAGE_SIZE, "num_pre_bmiss:%u\n",
			ev_ptr->bmiss_infra_cp_stats->num_pre_bmiss);
	if (ret <= 0)
		return ret;

	for (idx = 0; idx < BMISS_STATS_RSSI_SAMPLES_MAX; idx++) {
		if ((PAGE_SIZE - ret) <= 0)
			return ret;

		ret += scnprintf(
			buf + ret, PAGE_SIZE - ret,
			"rssi_sample%d-rssi:%d\n", idx,
			ev_ptr->bmiss_infra_cp_stats->rssi_samples[idx].rssi);
		if ((PAGE_SIZE - ret) <= 0)
			return ret;

		ret += scnprintf(
			buf + ret, PAGE_SIZE - ret,
			"rssi_sample%d-sample_time:%u\n", idx,
			ev_ptr->bmiss_infra_cp_stats->rssi_samples[idx].sample_time);
	}
	if ((PAGE_SIZE - ret) <= 0)
		return ret;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
			 "rssi_sample_curr_index:%u\n"
			 "num_first_bmiss:%u\n"
			 "num_final_bmiss:%u\n"
			 "num_null_sent_in_first_bmiss:%u\n"
			 "num_null_failed_in_first_bmiss:%u\n"
			 "num_null_sent_in_final_bmiss:%u\n"
			 "num_null_failed_in_final_bmiss:%u\n"
			 "cons_bmiss_stats.num_of_bmiss_sequences:%u\n"
			 "cons_bmiss_stats.num_bitmask_wraparound:%u\n"
			 "cons_bmiss_stats.num_bcn_hist_lost:%u\n",
			 ev_ptr->bmiss_infra_cp_stats->rssi_sample_curr_index,
			 ev_ptr->bmiss_infra_cp_stats->num_first_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->num_final_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->num_null_sent_in_first_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->num_null_failed_in_first_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->num_null_sent_in_final_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->num_null_failed_in_final_bmiss,
			 ev_ptr->bmiss_infra_cp_stats->cons_bmiss_stats.num_of_bmiss_sequences,
			 ev_ptr->bmiss_infra_cp_stats->cons_bmiss_stats.num_bitmask_wraparound,
			 ev_ptr->bmiss_infra_cp_stats->cons_bmiss_stats.num_bcn_hist_lost);

	qdf_mem_free(ev_ptr->bmiss_infra_cp_stats);
	qdf_mem_free(ev_ptr);
	return ret;
}

static ssize_t
hdd_sysfs_bmiss_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct net_device *net_dev;
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	net_dev = container_of(dev, struct net_device, dev);
	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_bmiss_show(net_dev, buf);
	osif_vdev_sync_op_stop(vdev_sync);
	return err_size;
}

static DEVICE_ATTR(bmiss, 0440, hdd_sysfs_bmiss_show, NULL);

int hdd_sysfs_bmiss_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev,
				   &dev_attr_bmiss);
	if (!error)
		hdd_err("could not create bmiss sysfs file");
	return error;
}

void hdd_sysfs_bmiss_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_bmiss);
}

