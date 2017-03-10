/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * DOC: wlan_hdd_debugfs_llstat.c
 *
 * WLAN Host Device Driver implementation to update
 * debugfs with Link Layer statistics
 */

#include <wlan_hdd_debugfs_llstat.h>
#include <cds_sched.h>
#include <wlan_hdd_stats.h>
#include <wma_api.h>

struct ll_stats_buf {
	ssize_t len;
	uint8_t *result;
};

static struct ll_stats_buf ll_stats;

void hdd_debugfs_process_iface_stats(hdd_adapter_t *adapter,
		void *data, uint32_t num_peers)
{
	tpSirWifiIfaceStat iface_stat;
	tpSirWifiInterfaceInfo iface_info;
	tpSirWifiWmmAcStat ac_stat;
	struct wifi_iface_offload_stat *offload_stat;
	uint64_t average_tsf_offset;
	int i;
	ssize_t len = 0;
	uint8_t *buffer;

	ENTER();
	iface_stat = data;

	buffer = ll_stats.result;
	buffer += ll_stats.len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\n\n===LL_STATS_IFACE: num_peers: %d===", num_peers);

	if (false == hdd_get_interface_info(adapter, &iface_stat->info)) {
		hdd_err("hdd_get_interface_info get fail");
		return;
	}

	iface_info = &iface_stat->info;
	buffer += len;
	ll_stats.len += len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\nmode: %u, MAC_ADDR: %pM, state: %u, roaming: %u, capabilities: %u, SSID: %s, BSSID_MAC: %pM, ap_country_str: %s, country_str: %s",
			iface_info->mode, &iface_info->macAddr.bytes[0],
			iface_info->state, iface_info->roaming,
			iface_info->capabilities, iface_info->ssid,
			&iface_info->bssid.bytes[0], iface_info->apCountryStr,
			iface_info->countryStr);

	average_tsf_offset =  iface_stat->avg_bcn_spread_offset_high;
	average_tsf_offset =  (average_tsf_offset << 32) |
				iface_stat->avg_bcn_spread_offset_low;

	buffer += len;
	ll_stats.len += len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\nbeacon_rx: %u, mgmt_rx: %u, mgmt_action_rx: %u, mgmt_action_tx: %u, rssi_mgmt: %u, rssi_data: %u, rssi_ack: %u, is_leaky_ap: %u, avg_rx_frms_leaked: %u, rx_leak_window: %u, average_tsf_offset: %llu, Tx RTS success count: %u, Tx RTS fail count: %u, Tx ppdu success count: %u, Tx ppdu fail count: %u, Connected duration: %u, Disconnected duration: %u, RTT ranging duration: %u, RTT responder duration: %u, Num tx probes: %u, Num beacon miss: %u,\n\nNumber of AC: %d",
			iface_stat->beaconRx, iface_stat->mgmtRx,
			iface_stat->mgmtActionRx, iface_stat->mgmtActionTx,
			iface_stat->rssiMgmt, iface_stat->rssiData,
			iface_stat->rssiAck, iface_stat->is_leaky_ap,
			iface_stat->avg_rx_frms_leaked,
			iface_stat->rx_leak_window, average_tsf_offset,
			iface_stat->tx_rts_succ_cnt,
			iface_stat->tx_rts_fail_cnt,
			iface_stat->tx_ppdu_succ_cnt,
			iface_stat->tx_ppdu_fail_cnt,
			iface_stat->connected_duration,
			iface_stat->disconnected_duration,
			iface_stat->rtt_ranging_duration,
			iface_stat->rtt_responder_duration,
			iface_stat->num_probes_tx, iface_stat->num_beacon_miss,
			iface_stat->num_ac);

	for (i = 0; i < iface_stat->num_ac; i++) {
		ac_stat = &iface_stat->AccessclassStats[i];
		buffer += len;
		ll_stats.len += len;
		len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"\nAC: %d, tx_mpdu: %u, rx_mpdu: %u, tx_mcast: %u, rx_mcast: %u, rx_ampdu: %u tx_ampdu: %u, mpdu_lost: %u, retries: %u, retries_short: %u, retries_long: %u, contention_time: min-%u max-%u avg-%u, contention num samples: %u",
				ac_stat->ac, ac_stat->txMpdu, ac_stat->rxMpdu,
				ac_stat->txMcast, ac_stat->rxMcast,
				ac_stat->rxAmpdu, ac_stat->txAmpdu,
				ac_stat->mpduLost, ac_stat->retries,
				ac_stat->retriesShort, ac_stat->retriesLong,
				ac_stat->contentionTimeMin,
				ac_stat->contentionTimeMax,
				ac_stat->contentionTimeAvg,
				ac_stat->contentionNumSamples);
	}

	buffer += len;
	ll_stats.len += len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\n\nNumber of offload stats: %d",
			iface_stat->num_offload_stats);

	for (i = 0; i < iface_stat->num_offload_stats; i++) {
		offload_stat = &iface_stat->offload_stat[i];
		buffer += len;
		ll_stats.len += len;
		len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"\ntype: %d, rx_count: %u, drp_count: %u, fwd_count: %u",
				offload_stat->type, offload_stat->rx_count,
				offload_stat->drp_count,
				offload_stat->fwd_count);
	}

	ll_stats.len += len;
	EXIT();
}

void hdd_debugfs_process_peer_stats(hdd_adapter_t *adapter, void *data)
{
	tpSirWifiPeerStat peer_stat;
	tpSirWifiPeerInfo peer_info;
	tpSirWifiRateStat rate_stat;
	int i, j, num_rate;
	ssize_t len = 0;
	uint8_t *buffer;

	ENTER();

	if (!ll_stats.result) {
		hdd_err("LL statistics buffer is NULL");
		return;
	}

	peer_stat = (tpSirWifiPeerStat) data;

	buffer = ll_stats.result;
	buffer += ll_stats.len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\n\n===LL_STATS_PEER_ALL : num_peers %u===",
			peer_stat->numPeers);

	peer_info = (tpSirWifiPeerInfo) ((uint8_t *) peer_stat->peerInfo);
	for (i = 1; i <= peer_stat->numPeers; i++) {
		buffer += len;
		ll_stats.len += len;
		len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"\nType: %d, peer_mac: %pM, capabilities: %u\nnum_rates: %d",
				wmi_to_sir_peer_type(peer_info->type),
				&peer_info->peerMacAddress.bytes[0],
				peer_info->capabilities, peer_info->numRate);

		num_rate = peer_info->numRate;
		for (j = 0; j < num_rate; j++) {
			rate_stat = (tpSirWifiRateStat) ((uint8_t *)
					peer_info->rateStats + (j *
					sizeof(tSirWifiRateStat)));
			buffer += len;
			ll_stats.len += len;
			len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"\npreamble: %0x, nss: %0x, bw: %0x, mcs: %0x, bitrate: %0x, txmpdu: %u, rxmpdu: %u, mpdu_lost: %u, retries: %u, retries_short: %u, retries_long: %u",
				rate_stat->rate.preamble, rate_stat->rate.nss,
				rate_stat->rate.bw, rate_stat->rate.rateMcsIdx,
				rate_stat->rate.bitrate, rate_stat->txMpdu,
				rate_stat->rxMpdu, rate_stat->mpduLost,
				rate_stat->retries, rate_stat->retriesShort,
				rate_stat->retriesLong);
		}
		peer_info = (tpSirWifiPeerInfo) ((uint8_t *)
				peer_stat->peerInfo + (i *
				sizeof(tSirWifiPeerInfo)) +
				(num_rate * sizeof(tSirWifiRateStat)));
	}
	ll_stats.len += len;
	EXIT();

}

void hdd_debugfs_process_radio_stats(hdd_adapter_t *adapter,
		uint32_t more_data, void *data, uint32_t num_radio)
{
	int i, j;
	ssize_t len = 0;
	uint8_t *buffer;
	tSirWifiRadioStat *radio_stat = (tpSirWifiRadioStat) data;
	tSirWifiChannelStats *chan_stat;

	ENTER();

	if (!ll_stats.result) {
		hdd_err("LL statistics buffer is NULL");
		return;
	}

	buffer = ll_stats.result;
	buffer += ll_stats.len;
	len = scnprintf(buffer, DEBUGFS_LLSTATS_BUF_SIZE,
			 "\n\n===LL_STATS_RADIO: number of radios: %u===",
			  num_radio);

	for (i = 0; i < num_radio; i++) {
		buffer += len;
		ll_stats.len += len;
		len = scnprintf(buffer,
			DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\nRadio: %u on_time: %u, tx_time: %u, rx_time: %u, on_time_scan: %u, on_time_nbd: %u, on_time_gscan: %u, on_time_roam_scan: %u, on_time_pno_scan: %u  on_time_hs20: %u, on_time_host_scan: %u, on_time_lpi_scan: %u\ntotal_num_tx_pwr_levels: %u\n",
			radio_stat->radio, radio_stat->onTime,
			radio_stat->txTime, radio_stat->rxTime,
			radio_stat->onTimeScan, radio_stat->onTimeNbd,
			radio_stat->onTimeGscan,
			radio_stat->onTimeRoamScan,
			radio_stat->onTimePnoScan,
			radio_stat->onTimeHs20,
			radio_stat->on_time_host_scan,
			radio_stat->on_time_lpi_scan,
			radio_stat->total_num_tx_power_levels);

		for (j = 0; j < radio_stat->total_num_tx_power_levels; j++) {
			buffer += len;
			ll_stats.len += len;
			len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"%d ", radio_stat->tx_time_per_power_level[j]);
		}

		buffer += len;
		ll_stats.len += len;
		len = scnprintf(buffer,
			DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
			"\nNum channels: %d", radio_stat->numChannels);

		for (j = 0; j < radio_stat->numChannels; j++) {
			chan_stat = (tSirWifiChannelStats *)
					((uint8_t *)radio_stat->channels +
					  (j * sizeof(tSirWifiChannelStats)));

			buffer += len;
			ll_stats.len += len;
			len = scnprintf(buffer,
				DEBUGFS_LLSTATS_BUF_SIZE - ll_stats.len,
				"\nChan width: %d, center_freq: %d, center_freq0: %d, center_freq1: %d, on_time: %d, cca_busy_time: %d",
				chan_stat->channel.width,
				chan_stat->channel.centerFreq,
				chan_stat->channel.centerFreq0,
				chan_stat->channel.centerFreq1,
				chan_stat->onTime, chan_stat->ccaBusyTime);
		}

		radio_stat++;
	}
	ll_stats.len += len;
	EXIT();
}

static inline void wlan_hdd_llstats_free_buf(void)
{
	qdf_mem_free(ll_stats.result);
	ll_stats.result = NULL;
	ll_stats.len =  0;
}

static int wlan_hdd_llstats_alloc_buf(void)
{
	ll_stats.len = 0;

	ll_stats.result = qdf_mem_malloc(DEBUGFS_LLSTATS_BUF_SIZE);
	if (!ll_stats.result) {
		hdd_err("LL Stats buffer allocation failed");
		return -EINVAL;
	}

	return 0;
}

/**
 * hdd_debugfs_stats_update() - Update userspace with local statistics buffer
 * @buf: userspace buffer (to which data is being copied into)
 * @count: max data that can be copied into buf
 * @pos: offset (where data should be copied into)
 *
 * This function should copies link layer statistics buffer into debugfs
 * entry.
 *
 * Return: number of characters copied; 0 on no-copy
 */
static ssize_t hdd_debugfs_stats_update(char __user *buf, size_t count,
				     loff_t *pos)
{
	ssize_t ret_cnt;

	ENTER();

	if (!ll_stats.result) {
		hdd_err("Trying to read from NULL buffer");
		return 0;
	}

	hdd_info("LL stats read req: count: %zu, pos: %lld", count, *pos);
	ret_cnt = simple_read_from_buffer(buf, count, pos,
			ll_stats.result, ll_stats.len);

	EXIT();
	return ret_cnt;
}

/**
 * __wlan_hdd_read_ll_stats_debugfs() - API to collect LL stats from FW
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @pos: position pointer
 *
 * Return: Number of bytes read on success, error number otherwise
 */
static ssize_t __wlan_hdd_read_ll_stats_debugfs(struct file *file,
			char __user *buf, size_t count, loff_t *pos)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	ssize_t ret = 0;

	ENTER();

	adapter = (hdd_adapter_t *)file->private_data;
	if ((!adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* All the events are received and buffer is populated */
	ret = hdd_debugfs_stats_update(buf, count, pos);
	hdd_info("%zu characters written into debugfs", ret);

	EXIT();
	return ret;

}

/**
 * wlan_hdd_read_ll_stats_debugfs() - SSR wrapper function to read LL debugfs
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @pos: position pointer
 *
 * Return: Number of bytes read on success, error number otherwise
 */
static ssize_t wlan_hdd_read_ll_stats_debugfs(struct file *file,
		char __user *buf,
		size_t count, loff_t *pos)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_read_ll_stats_debugfs(file, buf, count, pos);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_open_ll_stats_debugfs() - Function to save private on open
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int __wlan_hdd_open_ll_stats_debugfs(struct inode *inode,
					    struct file *file)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	if (inode->i_private)
		file->private_data = inode->i_private;

	adapter = (hdd_adapter_t *)file->private_data;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = wlan_hdd_llstats_alloc_buf();
	if (0 != ret)
		return ret;

	ret = wlan_hdd_ll_stats_get(adapter, DEBUGFS_LLSTATS_REQID,
				    DEBUGFS_LLSTATS_REQMASK);
	if (0 != ret)
		return ret;

	EXIT();
	return 0;
}


/**
 * wlan_hdd_open_ll_stats_debugfs() - SSR wrapper function to save private
 *                                    on open
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int wlan_hdd_open_ll_stats_debugfs(struct inode *inode,
					  struct file *file)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_open_ll_stats_debugfs(inode, file);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_release_ll_stats_debugfs() - Function to save private on release
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int __wlan_hdd_release_ll_stats_debugfs(struct inode *inode,
					    struct file *file)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	if (inode->i_private)
		file->private_data = inode->i_private;

	adapter = (hdd_adapter_t *)file->private_data;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	wlan_hdd_llstats_free_buf();

	EXIT();
	return 0;
}

/**
 * wlan_hdd_release_ll_stats_debugfs() - SSR wrapper function to save private
 *                                       on release
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int wlan_hdd_release_ll_stats_debugfs(struct inode *inode,
					  struct file *file)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_release_ll_stats_debugfs(inode, file);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct file_operations fops_ll_stats_debugfs = {
	.read = wlan_hdd_read_ll_stats_debugfs,
	.open = wlan_hdd_open_ll_stats_debugfs,
	.release = wlan_hdd_release_ll_stats_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

int wlan_hdd_create_ll_stats_file(hdd_adapter_t *adapter)
{
	if (!debugfs_create_file("ll_stats", 0444, adapter->debugfs_phy,
				 adapter, &fops_ll_stats_debugfs))
		return -EINVAL;

	return 0;
}
