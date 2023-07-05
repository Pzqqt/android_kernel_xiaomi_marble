/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : os_if_son.c
 *
 * WLAN Host Device Driver file for son (Self Organizing Network)
 * support.
 *
 */

#include <os_if_son.h>
#include <qdf_trace.h>
#include <qdf_module.h>
#include <wlan_cfg80211.h>
#include <son_ucfg_api.h>
#include <wlan_dfs_ucfg_api.h>
#include <wlan_reg_ucfg_api.h>
#include <wlan_vdev_mgr_ucfg_api.h>
#include <wlan_mlme_ucfg_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_dcs_ucfg_api.h>

static struct son_callbacks g_son_os_if_cb;

void os_if_son_register_hdd_callbacks(struct wlan_objmgr_psoc *psoc,
				      struct son_callbacks *cb_obj)
{
	g_son_os_if_cb = *cb_obj;
}

qdf_freq_t os_if_son_get_freq(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	qdf_freq_t freq;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return 0;
	}

	freq = ucfg_son_get_operation_chan_freq_vdev_id(pdev,
							wlan_vdev_get_id(vdev));
	osif_debug("vdev %d get freq %d", wlan_vdev_get_id(vdev), freq);

	return freq;
}
qdf_export_symbol(os_if_son_get_freq);

uint32_t os_if_son_is_acs_in_progress(struct wlan_objmgr_vdev *vdev)
{
	uint32_t acs_in_progress;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	acs_in_progress = g_son_os_if_cb.os_if_is_acs_in_progress(vdev);
	osif_debug("vdev %d acs_in_progress %d",
		   wlan_vdev_get_id(vdev), acs_in_progress);

	return acs_in_progress;
}
qdf_export_symbol(os_if_son_is_acs_in_progress);

uint32_t os_if_son_is_cac_in_progress(struct wlan_objmgr_vdev *vdev)
{
	uint32_t cac_in_progress;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	cac_in_progress = ucfg_son_is_cac_in_progress(vdev);
	osif_debug("vdev %d cac_in_progress %d",
		   wlan_vdev_get_id(vdev), cac_in_progress);

	return cac_in_progress;
}
qdf_export_symbol(os_if_son_is_cac_in_progress);

int os_if_son_set_chan_ext_offset(struct wlan_objmgr_vdev *vdev,
				  enum sec20_chan_offset son_chan_ext_offset)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	ret = g_son_os_if_cb.os_if_set_chan_ext_offset(vdev,
						       son_chan_ext_offset);
	osif_debug("vdev %d set_chan_ext_offset %d, ret %d",
		   wlan_vdev_get_id(vdev), son_chan_ext_offset, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_chan_ext_offset);

enum sec20_chan_offset os_if_son_get_chan_ext_offset(
						struct wlan_objmgr_vdev *vdev)
{
	enum sec20_chan_offset chan_ext_offset;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	chan_ext_offset = g_son_os_if_cb.os_if_get_chan_ext_offset(vdev);
	osif_debug("vdev %d chan_ext_offset %d",
		   wlan_vdev_get_id(vdev), chan_ext_offset);

	return chan_ext_offset;
}
qdf_export_symbol(os_if_son_get_chan_ext_offset);

int os_if_son_set_bandwidth(struct wlan_objmgr_vdev *vdev,
			    uint32_t son_bandwidth)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	ret = g_son_os_if_cb.os_if_set_bandwidth(vdev, son_bandwidth);
	osif_debug("vdev %d son_bandwidth %d ret %d",
		   wlan_vdev_get_id(vdev), son_bandwidth, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_bandwidth);

uint32_t os_if_son_get_bandwidth(struct wlan_objmgr_vdev *vdev)
{
	uint32_t bandwidth;

	if (!vdev) {
		osif_err("null vdev");
		return NONHT;
	}

	bandwidth = g_son_os_if_cb.os_if_get_bandwidth(vdev);
	osif_debug("vdev %d son_bandwidth %d",
		   wlan_vdev_get_id(vdev), bandwidth);

	return bandwidth;
}
qdf_export_symbol(os_if_son_get_bandwidth);

static uint32_t os_if_band_bitmap_to_son_band_info(
					uint32_t reg_wifi_band_bitmap)
{
	uint32_t son_band_info = FULL_BAND_RADIO;

	if (!(reg_wifi_band_bitmap & BIT(REG_BAND_5G)) &&
	    !(reg_wifi_band_bitmap & BIT(REG_BAND_6G)))
		return NON_5G_RADIO;
	if (reg_wifi_band_bitmap & BIT(REG_BAND_6G) &&
	    !(reg_wifi_band_bitmap & BIT(REG_BAND_2G)) &&
	    !(reg_wifi_band_bitmap & BIT(REG_BAND_5G)))
		return BAND_6G_RADIO;

	return son_band_info;
}

uint32_t os_if_son_get_band_info(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_pdev *pdev;
	uint32_t reg_wifi_band_bitmap;
	uint32_t band_info;

	if (!vdev) {
		osif_err("null vdev");
		return NO_BAND_INFORMATION_AVAILABLE;
	}
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return NO_BAND_INFORMATION_AVAILABLE;
	}

	status = ucfg_reg_get_band(pdev, &reg_wifi_band_bitmap);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		osif_err("failed to get band");
		return NO_BAND_INFORMATION_AVAILABLE;
	}

	band_info = os_if_band_bitmap_to_son_band_info(reg_wifi_band_bitmap);
	osif_debug("vdev %d band_info %d",
		   wlan_vdev_get_id(vdev), band_info);

	return band_info;
}
qdf_export_symbol(os_if_son_get_band_info);

#define BW_WITHIN(min, bw, max) ((min) <= (bw) && (bw) <= (max))
/**
 * os_if_son_fill_chan_info() - fill chan info
 * @chan_info: chan info to fill
 * @chan_num: chan number
 * @primary_freq: chan frequency
 * @ch_num_seg1: channel number for segment 1
 * @ch_num_seg2: channel number for segment 2
 *
 * Return: void
 */
static void os_if_son_fill_chan_info(struct ieee80211_channel_info *chan_info,
				     uint8_t chan_num, qdf_freq_t primary_freq,
				     uint8_t ch_num_seg1, uint8_t ch_num_seg2)
{
	chan_info->ieee = chan_num;
	chan_info->freq = primary_freq;
	chan_info->vhtop_ch_num_seg1 = ch_num_seg1;
	chan_info->vhtop_ch_num_seg2 = ch_num_seg2;
}

/**
 * os_if_son_update_chan_info() - update chan info
 * @pdev: pdev
 * @flag_160: flag indicating the API to fill the center frequencies of 160MHz.
 * @cur_chan_list: pointer to regulatory_channel
 * @chan_info: chan info to fill
 * @half_and_quarter_rate_flags: half and quarter rate flags
 *
 * Return: void
 */
static void os_if_son_update_chan_info(
			struct wlan_objmgr_pdev *pdev, bool flag_160,
			struct regulatory_channel *cur_chan_list,
			struct ieee80211_channel_info *chan_info,
			uint64_t half_and_quarter_rate_flags)
{
	qdf_freq_t primary_freq = cur_chan_list->center_freq;
	struct ch_params chan_params = {0};

	if (!chan_info) {
		osif_err("null chan info");
		return;
	}
	if (cur_chan_list->chan_flags & REGULATORY_CHAN_NO_OFDM)
		chan_info->flags |=
			VENDOR_CHAN_FLAG2(QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_B);
	else
		chan_info->flags |= ucfg_son_get_chan_flag(pdev, primary_freq,
							   flag_160,
							   &chan_params);
	if (cur_chan_list->chan_flags & REGULATORY_CHAN_RADAR) {
		chan_info->flags_ext |=
			QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS;
		chan_info->flags_ext |=
			QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DISALLOW_ADHOC;
		chan_info->flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_PASSIVE;
	} else if (cur_chan_list->chan_flags & REGULATORY_CHAN_NO_IR) {
		/* For 2Ghz passive channels. */
		chan_info->flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_PASSIVE;
	}

	if (WLAN_REG_IS_6GHZ_PSC_CHAN_FREQ(primary_freq))
		chan_info->flags_ext |=
			QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_PSC;

	os_if_son_fill_chan_info(chan_info, cur_chan_list->chan_num,
				 primary_freq,
				 chan_params.center_freq_seg0,
				 chan_params.center_freq_seg1);
}

int os_if_son_get_chan_list(struct wlan_objmgr_vdev *vdev,
			    struct ieee80211_ath_channel *chan_list,
			    struct ieee80211_channel_info *chan_info,
			    uint8_t *nchans, bool flag_160)
{
	struct regulatory_channel *cur_chan_list;
	int i;
	uint32_t phybitmap;
	uint32_t reg_wifi_band_bitmap;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct regulatory_channel *chan;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	if (!chan_info) {
		osif_err("null chan info");
		return -EINVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return -EINVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("null psoc");
		return -EINVAL;
	}

	status = ucfg_reg_get_band(pdev, &reg_wifi_band_bitmap);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		osif_err("failed to get band");
		return -EINVAL;
	}

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS *
			sizeof(struct regulatory_channel));
	if (!cur_chan_list) {
		osif_err("cur_chan_list allocation fails");
		return -EINVAL;
	}

	if (wlan_reg_get_current_chan_list(
	    pdev, cur_chan_list) != QDF_STATUS_SUCCESS) {
		qdf_mem_free(cur_chan_list);
		osif_err("fail to get current chan list");
		return -EINVAL;
	}

	ucfg_reg_get_band(pdev, &phybitmap);

	for (i = 0; i < NUM_CHANNELS; i++) {
		uint64_t band_flags;
		qdf_freq_t primary_freq = cur_chan_list[i].center_freq;
		uint64_t half_and_quarter_rate_flags = 0;

		chan = &cur_chan_list[i];
		if ((chan->chan_flags & REGULATORY_CHAN_DISABLED) &&
		    chan->state == CHANNEL_STATE_DISABLE &&
		    !chan->nol_chan && !chan->nol_history)
			continue;
		if (WLAN_REG_IS_6GHZ_CHAN_FREQ(primary_freq)) {
			if (!(reg_wifi_band_bitmap & BIT(REG_BAND_6G)))
				continue;
			band_flags = VENDOR_CHAN_FLAG2(
				QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_6GHZ);
		} else if (WLAN_REG_IS_24GHZ_CH_FREQ(primary_freq)) {
			if (!(reg_wifi_band_bitmap & BIT(REG_BAND_2G)))
				continue;
			band_flags = QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_2GHZ;
		} else if (WLAN_REG_IS_5GHZ_CH_FREQ(primary_freq)) {
			if (!(reg_wifi_band_bitmap & BIT(REG_BAND_5G)))
				continue;
			band_flags = QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_5GHZ;
		} else if (WLAN_REG_IS_49GHZ_FREQ(primary_freq)) {
			if (!(reg_wifi_band_bitmap & BIT(REG_BAND_5G)))
				continue;
			band_flags = QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_5GHZ;
			/**
			 * If 4.9G Half and Quarter rates are supported
			 * by the channel, update them as separate entries
			 * to the list
			 */
			if (BW_WITHIN(chan->min_bw, BW_10_MHZ, chan->max_bw)) {
				os_if_son_fill_chan_info(&chan_info[*nchans],
							 chan->chan_num,
							 primary_freq, 0, 0);
				chan_info[*nchans].flags |=
					QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HALF;
				chan_info[*nchans].flags |=
					VENDOR_CHAN_FLAG2(
					QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_A);
				half_and_quarter_rate_flags =
					chan_info[*nchans].flags;
				if (++(*nchans) >= IEEE80211_CHAN_MAX)
					break;
			}
			if (BW_WITHIN(chan->min_bw, BW_5_MHZ, chan->max_bw)) {
				os_if_son_fill_chan_info(&chan_info[*nchans],
							 chan->chan_num,
							 primary_freq, 0, 0);
				chan_info[*nchans].flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_QUARTER;
				chan_info[*nchans].flags |=
					VENDOR_CHAN_FLAG2(
					QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_A);
				half_and_quarter_rate_flags =
					chan_info[*nchans].flags;
				if (++(*nchans) >= IEEE80211_CHAN_MAX)
					break;
			}
		} else {
			continue;
		}

		os_if_son_update_chan_info(pdev, flag_160, chan,
					   &chan_info[*nchans],
					   half_and_quarter_rate_flags);

		if (++(*nchans) >= IEEE80211_CHAN_MAX)
			break;
	}

	qdf_mem_free(cur_chan_list);
	osif_debug("vdev %d channel_info exit", wlan_vdev_get_id(vdev));

	return 0;
}
qdf_export_symbol(os_if_son_get_chan_list);

uint32_t os_if_son_get_sta_count(struct wlan_objmgr_vdev *vdev)
{
	uint32_t sta_count;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	sta_count = ucfg_son_get_sta_count(vdev);
	osif_debug("vdev %d sta count %d", wlan_vdev_get_id(vdev), sta_count);

	return sta_count;
}
qdf_export_symbol(os_if_son_get_sta_count);

int os_if_son_get_bssid(struct wlan_objmgr_vdev *vdev,
			uint8_t bssid[QDF_MAC_ADDR_SIZE])
{
	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	ucfg_wlan_vdev_mgr_get_param_bssid(vdev, bssid);
	osif_debug("vdev %d bssid " QDF_MAC_ADDR_FMT,
		   wlan_vdev_get_id(vdev), QDF_MAC_ADDR_REF(bssid));

	return 0;
}
qdf_export_symbol(os_if_son_get_bssid);

int os_if_son_get_ssid(struct wlan_objmgr_vdev *vdev,
		       char ssid[WLAN_SSID_MAX_LEN + 1],
		       uint8_t *ssid_len)
{
	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	ucfg_wlan_vdev_mgr_get_param_ssid(vdev, ssid, ssid_len);
	osif_debug("vdev %d ssid %s", wlan_vdev_get_id(vdev), ssid);

	return 0;
}
qdf_export_symbol(os_if_son_get_ssid);

int os_if_son_set_chan(struct wlan_objmgr_vdev *vdev,
		       int chan, enum wlan_band_id son_band)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	ret = g_son_os_if_cb.os_if_set_chan(vdev, chan, son_band);
	osif_debug("vdev %d chan %d son_band %d", wlan_vdev_get_id(vdev),
		   chan, son_band);

	return ret;
}
qdf_export_symbol(os_if_son_set_chan);

int os_if_son_set_cac_timeout(struct wlan_objmgr_vdev *vdev,
			      int cac_timeout)
{
	struct wlan_objmgr_pdev *pdev;
	int status;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return -EINVAL;
	}

	if (QDF_IS_STATUS_ERROR(ucfg_dfs_override_cac_timeout(
		pdev, cac_timeout, &status))) {
		osif_err("cac timeout override fails");
		return -EINVAL;
	}
	osif_debug("vdev %d cac_timeout %d status %d",
		   wlan_vdev_get_id(vdev), cac_timeout, status);

	return status;
}
qdf_export_symbol(os_if_son_set_cac_timeout);

int os_if_son_get_cac_timeout(struct wlan_objmgr_vdev *vdev,
			      int *cac_timeout)
{
	struct wlan_objmgr_pdev *pdev;
	int status;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return -EINVAL;
	}

	if (QDF_IS_STATUS_ERROR(ucfg_dfs_get_override_cac_timeout(
		pdev, cac_timeout, &status))) {
		osif_err("fails to get cac timeout");
		return -EINVAL;
	}
	osif_debug("vdev %d cac_timeout %d status %d",
		   wlan_vdev_get_id(vdev), *cac_timeout, status);

	return status;
}
qdf_export_symbol(os_if_son_get_cac_timeout);

int os_if_son_set_country_code(struct wlan_objmgr_vdev *vdev,
			       char *country_code)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	ret = g_son_os_if_cb.os_if_set_country_code(vdev, country_code);
	osif_debug("vdev %d country_code %s ret %d",
		   wlan_vdev_get_id(vdev), country_code, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_country_code);

int os_if_son_get_country_code(struct wlan_objmgr_vdev *vdev,
			       char *country_code)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("null psoc");
		return -EINVAL;
	}
	status = ucfg_reg_get_current_country(psoc, country_code);
	osif_debug("vdev %d country_code %s status %d",
		   wlan_vdev_get_id(vdev), country_code, status);

	return qdf_status_to_os_return(status);
}
qdf_export_symbol(os_if_son_get_country_code);

int os_if_son_set_candidate_freq(struct wlan_objmgr_vdev *vdev,
				 qdf_freq_t freq)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	ret = g_son_os_if_cb.os_if_set_candidate_freq(vdev, freq);
	osif_debug("vdev %d set_candidate_freq %d ret %d",
		   wlan_vdev_get_id(vdev), freq, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_candidate_freq);

qdf_freq_t os_if_son_get_candidate_freq(struct wlan_objmgr_vdev *vdev)
{
	qdf_freq_t freq;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	freq = g_son_os_if_cb.os_if_get_candidate_freq(vdev);
	osif_debug("vdev %d candidate_freq %d",
		   wlan_vdev_get_id(vdev), freq);

	return freq;
}
qdf_export_symbol(os_if_son_get_candidate_freq);

QDF_STATUS os_if_son_set_acl_policy(struct wlan_objmgr_vdev *vdev,
				    ieee80211_acl_cmd son_acl_policy)
{
	QDF_STATUS ret;

	if (!vdev) {
		osif_err("null vdev");
		return QDF_STATUS_E_INVAL;
	}

	ret = g_son_os_if_cb.os_if_set_acl_policy(vdev, son_acl_policy);
	osif_debug("set acl policy %d status %d", son_acl_policy, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_acl_policy);

ieee80211_acl_cmd os_if_son_get_acl_policy(struct wlan_objmgr_vdev *vdev)
{
	ieee80211_acl_cmd son_acl_policy;

	if (!vdev) {
		osif_err("null vdev");
		return IEEE80211_MACCMD_DETACH;
	}
	son_acl_policy = g_son_os_if_cb.os_if_get_acl_policy(vdev);
	osif_debug("get acl policy %d", son_acl_policy);

	return son_acl_policy;
}
qdf_export_symbol(os_if_son_get_acl_policy);

int os_if_son_add_acl_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *acl_mac)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	ret = g_son_os_if_cb.os_if_add_acl_mac(vdev, acl_mac);
	osif_debug("add_acl_mac " QDF_MAC_ADDR_FMT " ret %d",
		   QDF_MAC_ADDR_REF(acl_mac->bytes), ret);

	return ret;
}
qdf_export_symbol(os_if_son_add_acl_mac);

int os_if_son_del_acl_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *acl_mac)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	ret = g_son_os_if_cb.os_if_del_acl_mac(vdev, acl_mac);
	osif_debug("del_acl_mac " QDF_MAC_ADDR_FMT " ret %d",
		   QDF_MAC_ADDR_REF(acl_mac->bytes), ret);

	return ret;
}
qdf_export_symbol(os_if_son_del_acl_mac);

int os_if_son_kickout_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *mac)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}
	ret = g_son_os_if_cb.os_if_kickout_mac(vdev, mac);
	osif_debug("kickout mac " QDF_MAC_ADDR_FMT " ret %d",
		   QDF_MAC_ADDR_REF(mac->bytes), ret);

	return ret;
}
qdf_export_symbol(os_if_son_kickout_mac);

uint8_t os_if_son_get_chan_util(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_host_dcs_ch_util_stats dcs_son_stats = {};
	struct wlan_objmgr_psoc *psoc;
	uint8_t mac_id;
	QDF_STATUS status;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("null psoc");
		return 0;
	}
	status = policy_mgr_get_mac_id_by_session_id(psoc,
						     wlan_vdev_get_id(vdev),
						     &mac_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to get mac_id");
		return 0;
	}

	ucfg_dcs_get_ch_util(psoc, mac_id, &dcs_son_stats);
	osif_debug("get_chan_util %d", dcs_son_stats.total_cu);

	return dcs_son_stats.total_cu;
}
qdf_export_symbol(os_if_son_get_chan_util);

void os_if_son_get_phy_stats(struct wlan_objmgr_vdev *vdev,
			     struct ol_ath_radiostats *phy_stats)
{
	struct wlan_host_dcs_ch_util_stats dcs_son_stats = {};
	struct wlan_objmgr_psoc *psoc;
	uint8_t mac_id;
	QDF_STATUS status;

	if (!vdev) {
		osif_err("null vdev");
		return;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		osif_err("null psoc");
		return;
	}
	status = policy_mgr_get_mac_id_by_session_id(psoc,
						     wlan_vdev_get_id(vdev),
						     &mac_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_err("Failed to get mac_id");
		return;
	}

	ucfg_dcs_get_ch_util(psoc, mac_id, &dcs_son_stats);

	phy_stats->ap_rx_util = dcs_son_stats.rx_cu;
	phy_stats->ap_tx_util = dcs_son_stats.tx_cu;
	phy_stats->obss_rx_util = dcs_son_stats.obss_rx_cu;
	if (dcs_son_stats.total_cu < 100)
		phy_stats->free_medium = 100 - dcs_son_stats.total_cu;
	else
		phy_stats->free_medium = 0;
	phy_stats->chan_nf = dcs_son_stats.chan_nf;
	osif_debug("rx_util %d tx_util %d obss_rx_util %d free_medium %d noise floor %d",
		   phy_stats->ap_rx_util, phy_stats->ap_tx_util,
		   phy_stats->obss_rx_util, phy_stats->free_medium,
		   phy_stats->chan_nf);
}
qdf_export_symbol(os_if_son_get_phy_stats);

int os_if_son_set_phymode(struct wlan_objmgr_vdev *vdev,
			  enum ieee80211_phymode mode)
{
	int ret;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	ret = g_son_os_if_cb.os_if_set_phymode(vdev, mode);
	osif_debug("vdev %d phymode %d ret %d",
		   wlan_vdev_get_id(vdev), mode, ret);

	return ret;
}
qdf_export_symbol(os_if_son_set_phymode);

enum ieee80211_phymode os_if_son_get_phymode(struct wlan_objmgr_vdev *vdev)
{
	enum ieee80211_phymode phymode;

	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	phymode = g_son_os_if_cb.os_if_get_phymode(vdev);
	osif_debug("vdev %d phymode %d",
		   wlan_vdev_get_id(vdev), phymode);

	return phymode;
}
qdf_export_symbol(os_if_son_get_phymode);

QDF_STATUS os_if_son_vdev_ops(struct wlan_objmgr_vdev *vdev,
			      enum wlan_mlme_vdev_param type,
			      void *data, void *ret)
{
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(os_if_son_vdev_ops);

QDF_STATUS os_if_son_peer_ops(struct wlan_objmgr_peer *peer,
			      enum wlan_mlme_peer_param type,
			      union wlan_mlme_peer_data *in,
			      union wlan_mlme_peer_data *out)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!peer) {
		osif_err("null peer");
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_peer_get_vdev(peer);
	if (!vdev) {
		osif_err("null vdev");
		return QDF_STATUS_E_INVAL;
	}
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		osif_err("null pdev");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		osif_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}
	osif_debug("type %d", type);
	/* All PEER MLME operations exported to SON component */
	switch (type) {
	case PEER_SET_KICKOUT_ALLOW:
		if (!in) {
			osif_err("invalid input parameter");
			return QDF_STATUS_E_INVAL;
		}
		status = ucfg_son_set_peer_kickout_allow(vdev, peer,
							 in->enable);
		break;
	default:
		osif_err("invalid type: %d", type);
		status = QDF_STATUS_E_INVAL;
	}

	return status;
}

qdf_export_symbol(os_if_son_peer_ops);

QDF_STATUS os_if_son_scan_db_iterate(struct wlan_objmgr_pdev *pdev,
				     scan_iterator_func handler, void *arg)
{
	return ucfg_scan_db_iterate(pdev, handler, arg);
}

qdf_export_symbol(os_if_son_scan_db_iterate);

bool os_if_son_acl_is_probe_wh_set(struct wlan_objmgr_vdev *vdev,
				   const uint8_t *mac_addr,
				   uint8_t probe_rssi)
{
	return false;
}

qdf_export_symbol(os_if_son_acl_is_probe_wh_set);

int os_if_son_set_chwidth(struct wlan_objmgr_vdev *vdev,
			  enum ieee80211_cwm_width son_chwidth)
{
	if (!vdev) {
		osif_err("null vdev");
		return -EINVAL;
	}

	return g_son_os_if_cb.os_if_set_chwidth(vdev, son_chwidth);
}
qdf_export_symbol(os_if_son_set_chwidth);

enum ieee80211_cwm_width os_if_son_get_chwidth(struct wlan_objmgr_vdev *vdev)
{
	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	return g_son_os_if_cb.os_if_get_chwidth(vdev);
}
qdf_export_symbol(os_if_son_get_chwidth);

u_int8_t os_if_son_get_rx_streams(struct wlan_objmgr_vdev *vdev)
{
	if (!vdev) {
		osif_err("null vdev");
		return 0;
	}

	return g_son_os_if_cb.os_if_get_rx_nss(vdev);
}

qdf_export_symbol(os_if_son_get_rx_streams);

QDF_STATUS os_if_son_cfg80211_reply(qdf_nbuf_t sk_buf)
{
	return wlan_cfg80211_qal_devcfg_send_response(sk_buf);
}

qdf_export_symbol(os_if_son_cfg80211_reply);

bool os_if_son_vdev_is_wds(struct wlan_objmgr_vdev *vdev)
{
	return true;
}

qdf_export_symbol(os_if_son_vdev_is_wds);

void os_if_son_deauth_peer_sta(struct wlan_objmgr_vdev *vdev,
			       uint8_t *peer_mac,
			       bool ignore_frame)
{
	if (!vdev || !peer_mac) {
		osif_err("null vdev / peer_mac");
		return;
	}
	if (g_son_os_if_cb.os_if_deauth_sta)
		g_son_os_if_cb.os_if_deauth_sta(vdev, peer_mac, ignore_frame);
}

qdf_export_symbol(os_if_son_deauth_peer_sta);

void os_if_son_modify_acl(struct wlan_objmgr_vdev *vdev,
			  uint8_t *peer_mac,
			  bool allow_auth)
{
	if (!vdev || !peer_mac) {
		osif_err("null vdev / peer_mac");
		return;
	}
	if (g_son_os_if_cb.os_if_modify_acl)
		g_son_os_if_cb.os_if_modify_acl(vdev, peer_mac, allow_auth);
}

qdf_export_symbol(os_if_son_modify_acl);

static
int os_if_son_reg_get_ap_hw_cap(struct wlan_objmgr_pdev *pdev,
				struct wlan_radio_basic_capabilities *hwcap)
{
	QDF_STATUS status;
	uint8_t idx;
	uint8_t max_supp_op_class = REG_MAX_SUPP_OPER_CLASSES;
	uint8_t n_opclasses = 0;
	/* nsoc = Number of supported operating classes */
	uint8_t nsoc = 0;
	struct regdmn_ap_cap_opclass_t *reg_ap_cap;

	if (!pdev || !hwcap)
		return nsoc;

	reg_ap_cap = qdf_mem_malloc(max_supp_op_class * sizeof(*reg_ap_cap));
	if (!reg_ap_cap) {
		osif_err("Memory allocation failure");
		return nsoc;
	}
	status = wlan_reg_get_opclass_details(pdev, reg_ap_cap, &n_opclasses,
					      max_supp_op_class, true);
	if (status == QDF_STATUS_E_FAILURE) {
		osif_err("Failed to get SAP regulatory capabilities");
		goto end_reg_get_ap_hw_cap;
	}
	osif_debug("n_opclasses: %u", n_opclasses);

	for (idx = 0; reg_ap_cap[idx].op_class && idx < n_opclasses; idx++) {
		osif_debug("idx: %d op_class: %u ch_width: %d  max_tx_pwr_dbm: %u",
			   idx, reg_ap_cap[idx].op_class,
			   reg_ap_cap[idx].ch_width,
			   reg_ap_cap[idx].max_tx_pwr_dbm);
		if (reg_ap_cap[idx].ch_width == BW_160_MHZ)
			continue;
		hwcap->opclasses[nsoc].opclass = reg_ap_cap[idx].op_class;
		hwcap->opclasses[nsoc].max_tx_pwr_dbm =
					reg_ap_cap[idx].max_tx_pwr_dbm;
		hwcap->opclasses[nsoc].num_non_oper_chan =
					reg_ap_cap[idx].num_non_supported_chan;
		qdf_mem_copy(hwcap->opclasses[nsoc].non_oper_chan_num,
			     reg_ap_cap[idx].non_sup_chan_list,
			     reg_ap_cap[idx].num_non_supported_chan);
		hwcap->wlan_radio_basic_capabilities_valid = 1;
		nsoc++;
	}
	hwcap->num_supp_op_classes = nsoc;

end_reg_get_ap_hw_cap:

	qdf_mem_free(reg_ap_cap);
	return nsoc;
}

static void os_if_son_reg_get_op_channels(struct wlan_objmgr_pdev *pdev,
					  struct wlan_op_chan *op_chan,
					  bool dfs_required)
{
	QDF_STATUS status;
	uint8_t idx;
	uint8_t max_supp_op_class = REG_MAX_SUPP_OPER_CLASSES;
	uint8_t n_opclasses = 0;
	/* nsoc = Number of supported operating classes */
	uint8_t nsoc = 0;
	struct regdmn_ap_cap_opclass_t *reg_ap_cap =
		qdf_mem_malloc(max_supp_op_class * sizeof(*reg_ap_cap));

	if (!reg_ap_cap) {
		osif_err("Memory allocation failure");
		return;
	}
	status = wlan_reg_get_opclass_details(pdev, reg_ap_cap, &n_opclasses,
					      max_supp_op_class, true);
	if (status == QDF_STATUS_E_FAILURE) {
		osif_err("Failed to get SAP regulatory capabilities");
		goto end_reg_get_op_channels;
	}
	osif_debug("n_opclasses: %u op_chan->opclass: %u",
		   n_opclasses, op_chan->opclass);
	for (idx = 0; reg_ap_cap[idx].op_class && idx < n_opclasses; idx++) {
		osif_debug("idx: %d op_class: %u ch_width: %d  max_tx_pwr_dbm: %u",
			   idx, reg_ap_cap[idx].op_class,
			   reg_ap_cap[idx].ch_width,
			   reg_ap_cap[idx].max_tx_pwr_dbm);
		if ((reg_ap_cap[idx].ch_width == BW_160_MHZ) ||
		    (op_chan->opclass != reg_ap_cap[idx].op_class))
			continue;
		if (reg_ap_cap[idx].op_class == op_chan->opclass) {
			switch (reg_ap_cap[idx].ch_width) {
			case BW_20_MHZ:
			case BW_25_MHZ:
				op_chan->ch_width = CH_WIDTH_20MHZ;
				break;
			case BW_40_MHZ:
				op_chan->ch_width = CH_WIDTH_40MHZ;
				break;
			case BW_80_MHZ:
				if (reg_ap_cap[idx].behav_limit ==
				    BIT(BEHAV_BW80_PLUS))
					op_chan->ch_width = CH_WIDTH_80P80MHZ;
				else
					op_chan->ch_width = CH_WIDTH_80MHZ;
				break;
			case BW_160_MHZ:
				op_chan->ch_width  = CH_WIDTH_160MHZ;
				break;
			default:
				op_chan->ch_width = INVALID_WIDTH;
				break;
			}
			op_chan->num_oper_chan =
					reg_ap_cap[idx].num_supported_chan;
			qdf_mem_copy(op_chan->oper_chan_num,
				     reg_ap_cap[idx].sup_chan_list,
				     reg_ap_cap[idx].num_supported_chan);
			osif_debug("num of supported channel: %u",
				   op_chan->num_oper_chan);
		}
	}
	/*
	 * TBD: DFS channel support needs to be added
	 * Variable nsoc will be update whenever we add DFS
	 * channel support for Easymesh.
	 */
	op_chan->num_supp_op_classes = nsoc;

end_reg_get_op_channels:

	qdf_mem_free(reg_ap_cap);
}

/* size of sec chan offset element */
#define IEEE80211_SEC_CHAN_OFFSET_BYTES             3
/* no secondary channel */
#define IEEE80211_SEC_CHAN_OFFSET_SCN               0
/* secondary channel above */
#define IEEE80211_SEC_CHAN_OFFSET_SCA               1
/* secondary channel below */
#define IEEE80211_SEC_CHAN_OFFSET_SCB               3

static void os_if_son_reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
					      struct wlan_op_class *op_class)
{
	QDF_STATUS status;
	uint8_t i;
	uint8_t idx;
	uint8_t n_opclasses = 0;
	uint8_t chan_idx;
	uint8_t max_supp_op_class = REG_MAX_SUPP_OPER_CLASSES;
	struct regdmn_ap_cap_opclass_t *reg_ap_cap =
			qdf_mem_malloc(max_supp_op_class * sizeof(*reg_ap_cap));

	if (!reg_ap_cap) {
		osif_err("Memory allocation failure");
		return;
	}
	status = wlan_reg_get_opclass_details(pdev, reg_ap_cap, &n_opclasses,
					      max_supp_op_class, true);
	if (status == QDF_STATUS_E_FAILURE) {
		osif_err("Failed to get SAP regulatory capabilities");
		goto end_reg_get_opclass_details;
	}
	osif_debug("n_opclasses: %u", n_opclasses);

	for (idx = 0; reg_ap_cap[idx].op_class && idx < n_opclasses; idx++) {
		osif_debug("idx: %d op_class: %u ch_width: %d",
			   idx, reg_ap_cap[idx].op_class,
			   reg_ap_cap[idx].ch_width);
		if ((op_class->opclass != reg_ap_cap[idx].op_class) ||
		    (reg_ap_cap[idx].ch_width == BW_160_MHZ))
			continue;
		switch (reg_ap_cap[idx].ch_width) {
		case BW_20_MHZ:
		case BW_25_MHZ:
			op_class->ch_width = CH_WIDTH_20MHZ;
			break;
		case BW_40_MHZ:
			op_class->ch_width = CH_WIDTH_40MHZ;
			break;
		case BW_80_MHZ:
			if (reg_ap_cap[idx].behav_limit == BIT(BEHAV_BW80_PLUS))
				op_class->ch_width = CH_WIDTH_80P80MHZ;
			else
				op_class->ch_width = CH_WIDTH_80MHZ;
			break;
		case BW_160_MHZ:
			op_class->ch_width  = CH_WIDTH_160MHZ;
			break;
		default:
			op_class->ch_width = CH_WIDTH_INVALID;
			break;
		}
		switch (reg_ap_cap[idx].behav_limit) {
		case BIT(BEHAV_NONE):
			op_class->sc_loc = IEEE80211_SEC_CHAN_OFFSET_SCN;
			break;
		case BIT(BEHAV_BW40_LOW_PRIMARY):
			op_class->sc_loc = IEEE80211_SEC_CHAN_OFFSET_SCA;
			break;
		case BIT(BEHAV_BW40_HIGH_PRIMARY):
			op_class->sc_loc = IEEE80211_SEC_CHAN_OFFSET_SCB;
			break;
		case BIT(BEHAV_BW80_PLUS):
			op_class->sc_loc = IEEE80211_SEC_CHAN_OFFSET_SCN;
			break;
		default:
			op_class->sc_loc = IEEE80211_SEC_CHAN_OFFSET_SCN;
			break;
		}
		osif_debug("num_supported_chan: %u num_non_supported_chan: %u",
			   reg_ap_cap[idx].num_supported_chan,
			   reg_ap_cap[idx].num_non_supported_chan);
		i = 0;
		chan_idx = 0;
		while ((i < reg_ap_cap[idx].num_supported_chan) &&
		       (chan_idx < MAX_CHANNELS_PER_OP_CLASS))
			op_class->channels[chan_idx++] =
				reg_ap_cap[idx].sup_chan_list[i++];
		i = 0;
		while ((i < reg_ap_cap[idx].num_non_supported_chan) &&
		       (chan_idx < MAX_CHANNELS_PER_OP_CLASS))
			op_class->channels[chan_idx++] =
				reg_ap_cap[idx].non_sup_chan_list[i++];

		 op_class->num_chan = chan_idx;
	}

end_reg_get_opclass_details:

	qdf_mem_free(reg_ap_cap);
}

QDF_STATUS os_if_son_pdev_ops(struct wlan_objmgr_pdev *pdev,
			      enum wlan_mlme_pdev_param type,
			      void *data, void *ret)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	union wlan_mlme_pdev_data *in = (union wlan_mlme_pdev_data *)data;
	union wlan_mlme_pdev_data *out = (union wlan_mlme_pdev_data *)ret;
	wlan_esp_data *esp_info;

	if (!out)
		return QDF_STATUS_E_INVAL;

	osif_debug("Type: %d", type);
	switch (type) {
	case PDEV_GET_ESP_INFO:
		esp_info = &out->esp_info;
		/* BA Window Size of 16 */
		esp_info->per_ac[WME_AC_BE].ba_window_size = ba_window_size_16;
		esp_info->per_ac[WME_AC_BE].est_air_time_fraction = 0;
		/* Default : 250us PPDU Duration in native format */
		esp_info->per_ac[WME_AC_BE].data_ppdu_dur_target =
			MAP_DEFAULT_PPDU_DURATION * MAP_PPDU_DURATION_UNITS;
		break;
	case PDEV_GET_CAPABILITY:
		os_if_son_reg_get_ap_hw_cap(pdev, &out->cap);
		break;
	case PDEV_GET_OPERABLE_CHAN:
		memcpy(&out->op_chan, &in->op_chan,
		       sizeof(struct wlan_op_chan));
		os_if_son_reg_get_op_channels(pdev, &out->op_chan,
					      in->op_chan.dfs_required);
		break;
	case PDEV_GET_OPERABLE_CLASS:
		memcpy(&out->op_class, &in->op_class,
		       sizeof(struct wlan_op_class));
		os_if_son_reg_get_opclass_details(pdev, &out->op_class);
		break;
	default:
		break;
	}

	return status;
}

qdf_export_symbol(os_if_son_pdev_ops);
