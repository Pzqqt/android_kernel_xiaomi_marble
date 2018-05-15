/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: defines driver functions interfacing with linux kernel
 */

#include <qdf_list.h>
#include <qdf_status.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <wlan_cfg80211.h>
#include <wlan_osif_priv.h>
#include <qdf_mem.h>
#include <wlan_spectral_ucfg_api.h>
#include <wlan_cfg80211_spectral.h>
#include <spectral_ioctl.h>

static const struct nla_policy spectral_scan_policy[
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE] = {
							.type = NLA_U64},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT] = {
							.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL] = {
							.type = NLA_U32},
};

static void wlan_spectral_intit_config(struct spectral_config *config_req)
{
	config_req->ss_period =          SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_count =           SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_fft_period =      SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_short_report =    SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_spectral_pri =    SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_fft_size =        SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_gc_ena =          SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_restart_ena =     SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_noise_floor_ref = SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_init_delay =      SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_nb_tone_thr =     SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_str_bin_thr =     SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_wb_rpt_mode =     SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_rssi_rpt_mode =   SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_rssi_thr =        SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_pwr_format =      SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_rpt_mode =        SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_bin_scale =       SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_dbm_adj =         SPECTRAL_PHYERR_PARAM_NOVAL;
	config_req->ss_chn_mask =        SPECTRAL_PHYERR_PARAM_NOVAL;
}

static int wlan_spectral_set_config(struct wlan_objmgr_pdev *pdev,
				    struct spectral_config *config_req)
{
	int status;

	status = ucfg_spectral_control(pdev,
				       SPECTRAL_SET_CONFIG,
				       config_req,
				       sizeof(struct spectral_config),
				       NULL,
				       NULL);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_set_debug_level(struct wlan_objmgr_pdev *pdev,
					 uint32_t spectral_dbg_level)
{
	int status;

	status = ucfg_spectral_control(pdev,
				       SPECTRAL_SET_DEBUG_LEVEL,
				       &spectral_dbg_level,
				       sizeof(uint32_t),
				       NULL,
				       NULL);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_get_debug_level(struct wlan_objmgr_pdev *pdev,
					 uint32_t *spectral_dbg_level)
{
	int status;
	uint32_t outsize;

	outsize = sizeof(uint32_t);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_GET_DEBUG_LEVEL,
				       NULL,
				       0,
				       spectral_dbg_level,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_get_config(struct wlan_objmgr_pdev *pdev,
				    struct spectral_config *config_req)
{
	int status;
	uint32_t outsize;

	outsize = sizeof(struct spectral_config);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_GET_CONFIG,
				       NULL,
				       0,
				       config_req,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_get_cap(struct wlan_objmgr_pdev *pdev,
				 struct spectral_caps *spectral_cap)
{
	int status;
	uint32_t outsize;

	outsize = sizeof(struct spectral_caps);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_GET_CAPABILITY_INFO,
				       NULL,
				       0,
				       spectral_cap,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_get_diag_stats(
		struct wlan_objmgr_pdev *pdev,
		struct spectral_diag_stats *spectral_diag)
{
	int status;
	uint32_t outsize;

	outsize = sizeof(struct spectral_diag_stats);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_GET_DIAG_STATS,
				       NULL,
				       0,
				       spectral_diag,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_spectral_scan_get_status(
		struct wlan_objmgr_pdev *pdev,
		struct spectral_scan_state *sscan_state)
{
	uint32_t is_active;
	uint32_t is_enabled;
	int status;
	uint32_t outsize;

	outsize = sizeof(uint32_t);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_IS_ACTIVE,
				       NULL,
				       0,
				       &is_active,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	sscan_state->is_active = is_active;

	outsize = sizeof(uint32_t);
	status = ucfg_spectral_control(pdev,
				       SPECTRAL_IS_ENABLED,
				       NULL,
				       0,
				       &is_enabled,
				       &outsize);
	if (status < 0)
		return -EINVAL;

	sscan_state->is_enabled = is_enabled;

	return 0;
}

static int wlan_start_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	int status;

	status = ucfg_spectral_control(pdev,
				       SPECTRAL_ACTIVATE_SCAN,
				       NULL,
				       0,
				       NULL,
				       NULL);
	if (status < 0)
		return -EINVAL;

	return 0;
}

static int wlan_stop_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	int status;

	status = ucfg_spectral_control(pdev,
				       SPECTRAL_STOP_SCAN,
				       NULL,
				       0,
				       NULL,
				       NULL);
	if (status < 0)
		return -EINVAL;

	return 0;
}

int wlan_cfg80211_spectral_scan_config_and_start(struct wiphy *wiphy,
						 struct wlan_objmgr_pdev *pdev,
						 const void *data,
						 int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX + 1];
	struct spectral_config config_req;
	QDF_STATUS status;
	uint64_t cookie;
	struct sk_buff *skb;
	uint32_t spectral_dbg_level;
	uint32_t scan_req_type = 0;

	if (wlan_cfg80211_nla_parse(
			tb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX,
			data,
			data_len,
			spectral_scan_policy)) {
		qdf_print("Invalid Spectral Scan config ATTR");
		return -EINVAL;
	}

	wlan_spectral_intit_config(&config_req);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT])
		config_req.ss_count = nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD])
		config_req.ss_period = nla_get_u32(tb
		[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY])
		config_req.ss_spectral_pri = nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE])
		config_req.ss_fft_size = nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA])
		config_req.ss_gc_ena = nla_get_u32(tb
			[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA])
		config_req.ss_restart_ena = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF])
		config_req.ss_noise_floor_ref = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY])
		config_req.ss_init_delay = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR])
		config_req.ss_nb_tone_thr = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR])
		config_req.ss_str_bin_thr = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE])
		config_req.ss_wb_rpt_mode = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE])
		config_req.ss_rssi_rpt_mode = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR])
		config_req.ss_rssi_thr = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT])
		config_req.ss_pwr_format = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE])
		config_req.ss_rpt_mode = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE])
		config_req.ss_bin_scale = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ])
		config_req.ss_dbm_adj = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK])
		config_req.ss_chn_mask = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD])
		config_req.ss_fft_period = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT])
		config_req.ss_short_report = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT]);

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL]) {
		spectral_dbg_level = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL]);
		status = wlan_spectral_set_debug_level(pdev,
						       spectral_dbg_level);
		if (QDF_STATUS_SUCCESS != status)
			return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE])
		scan_req_type = nla_get_u32(tb
		   [QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE]);

	if (CONFIG_REQUESTED(scan_req_type)) {
		status = wlan_spectral_set_config(pdev, &config_req);
		if (QDF_STATUS_SUCCESS != status)
			return -EINVAL;
	}

	if (SCAN_REQUESTED(scan_req_type)) {
		status = wlan_start_spectral_scan(pdev);
		if (QDF_STATUS_SUCCESS != status)
			return -EINVAL;
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u64) +
		NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		qdf_print(" reply skb alloc failed");
		return -ENOMEM;
	}

	cookie = 0;
	if (wlan_cfg80211_nla_put_u64(skb,
				      QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE,
				      cookie)) {
		kfree_skb(skb);
		return -EINVAL;
	}

	cfg80211_vendor_cmd_reply(skb);

	return 0;
}

int wlan_cfg80211_spectral_scan_stop(struct wiphy *wiphy,
				     struct wlan_objmgr_pdev *pdev,
				     const void *data,
				     int data_len)
{
	QDF_STATUS status;

	status = wlan_stop_spectral_scan(pdev);
	if (QDF_STATUS_SUCCESS != status)
		return -EINVAL;
	return 0;
}

int wlan_cfg80211_spectral_scan_get_config(struct wiphy *wiphy,
					   struct wlan_objmgr_pdev *pdev,
					   const void *data,
					   int data_len)
{
	struct spectral_config config_buf;
	uint32_t spectral_dbg_level;
	struct sk_buff *skb;

	wlan_spectral_get_config(pdev, &config_buf);
	wlan_spectral_get_debug_level(pdev, &spectral_dbg_level);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, (21 * sizeof(u32)) +
		NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		qdf_print(" reply skb alloc failed");
		return -ENOMEM;
	}

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT,
			config_buf.ss_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD,
			config_buf.ss_period) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY,
			config_buf.ss_spectral_pri) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE,
			config_buf.ss_fft_size) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA,
			config_buf.ss_gc_ena) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA,
			config_buf.ss_restart_ena) ||
	    nla_put_u32(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF,
		config_buf.ss_noise_floor_ref) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY,
			config_buf.ss_init_delay) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR,
			config_buf.ss_nb_tone_thr) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR,
			config_buf.ss_str_bin_thr) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE,
			config_buf.ss_wb_rpt_mode) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE,
			config_buf.ss_rssi_rpt_mode) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR,
			config_buf.ss_rssi_thr) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT,
			config_buf.ss_pwr_format) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE,
			config_buf.ss_rpt_mode) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE,
			config_buf.ss_bin_scale) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ,
			config_buf.ss_dbm_adj) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK,
			config_buf.ss_chn_mask) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD,
			config_buf.ss_fft_period) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT,
			config_buf.ss_short_report) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL,
			spectral_dbg_level)) {
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_cmd_reply(skb);

	return 0;
}

int wlan_cfg80211_spectral_scan_get_cap(struct wiphy *wiphy,
					struct wlan_objmgr_pdev *pdev,
					const void *data,
					int data_len)
{
	struct spectral_caps spectral_cap;
	struct sk_buff *skb;

	wlan_spectral_get_cap(pdev, &spectral_cap);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 5 * sizeof(u32) +
		NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		qdf_print(" reply skb alloc failed");
		return -ENOMEM;
	}

	if (spectral_cap.phydiag_cap)
		if (nla_put_flag(
			skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_PHYDIAG))
			goto fail;

	if (spectral_cap.radar_cap)
		if (nla_put_flag(skb,
				 QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_RADAR))
			goto fail;

	if (spectral_cap.spectral_cap)
		if (nla_put_flag(
			skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_SPECTRAL))
			goto fail;

	if (spectral_cap.advncd_spectral_cap)
		if (nla_put_flag(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_ADVANCED_SPECTRAL))
			goto fail;

	if (nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_HW_GEN,
			spectral_cap.hw_gen))
		goto fail;

	cfg80211_vendor_cmd_reply(skb);

	return 0;

fail:
	kfree_skb(skb);
	return -EINVAL;
}

int wlan_cfg80211_spectral_scan_get_diag_stats(struct wiphy *wiphy,
					       struct wlan_objmgr_pdev *pdev,
					       const void *data,
					       int data_len)
{
	struct spectral_diag_stats spetcral_diag;
	struct sk_buff *skb;

	wlan_spectral_get_diag_stats(pdev, &spetcral_diag);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 5 * sizeof(u64) +
		NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		qdf_print(" reply skb alloc failed");
		return -ENOMEM;
	}

	if (wlan_cfg80211_nla_put_u64(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SIG_MISMATCH,
		spetcral_diag.spectral_mismatch) ||
	    wlan_cfg80211_nla_put_u64(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SEC80_SFFT_INSUFFLEN,
		spetcral_diag.spectral_sec80_sfft_insufflen) ||
	    wlan_cfg80211_nla_put_u64(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_NOSEC80_SFFT,
		spetcral_diag.spectral_no_sec80_sfft) ||
	    wlan_cfg80211_nla_put_u64(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG1ID_MISMATCH,
		spetcral_diag.spectral_vhtseg1id_mismatch) ||
	    wlan_cfg80211_nla_put_u64(
		skb,
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG2ID_MISMATCH,
		spetcral_diag.spectral_vhtseg2id_mismatch)) {
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_cmd_reply(skb);

	return 0;
}

int wlan_cfg80211_spectral_scan_get_status(struct wiphy *wiphy,
					   struct wlan_objmgr_pdev *pdev,
					   const void *data,
					   int data_len)
{
	struct spectral_scan_state sscan_state = { 0 };
	struct sk_buff *skb;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2 * sizeof(u32) +
		NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		qdf_print(" reply skb alloc failed");
		return -ENOMEM;
	}

	wlan_spectral_scan_get_status(pdev, &sscan_state);

	if (sscan_state.is_enabled)
		if (nla_put_flag(
			skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ENABLED))
			goto fail;

	if (sscan_state.is_active)
		if (nla_put_flag(
			skb,
			QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ACTIVE))
			goto fail;
	cfg80211_vendor_cmd_reply(skb);
	return 0;

fail:
	kfree_skb(skb);
	return -EINVAL;
}
