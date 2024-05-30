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
 * DOC : contains interface prototypes for son api
 */

#include <son_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_mlme_api.h>
#include <ieee80211_external.h>

/**
 * struct son_mlme_deliver_cbs - son mlme deliver callbacks
 * @deliver_opmode: cb to deliver opmode
 * @deliver_smps: cb to deliver smps
 */
struct son_mlme_deliver_cbs {
	mlme_deliver_cb deliver_opmode;
	mlme_deliver_cb deliver_smps;
};

static struct son_mlme_deliver_cbs g_son_mlme_deliver_cbs;

QDF_STATUS
wlan_son_register_mlme_deliver_cb(struct wlan_objmgr_psoc *psoc,
				  mlme_deliver_cb cb,
				  enum SON_MLME_DELIVER_CB_TYPE type)
{
	if (!psoc) {
		qdf_err("invalid psoc");
		return QDF_STATUS_E_INVAL;
	}

	switch (type) {
	case SON_MLME_DELIVER_CB_TYPE_OPMODE:
		g_son_mlme_deliver_cbs.deliver_opmode = cb;
		break;
	case SON_MLME_DELIVER_CB_TYPE_SMPS:
		g_son_mlme_deliver_cbs.deliver_smps = cb;
		break;
	default:
		qdf_err("invalid type");
		break;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_son_is_he_supported() - is he supported or not
 * @psoc: pointer to psoc
 * @he_supported: he supported or not
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11AX
static void wlan_son_is_he_supported(struct wlan_objmgr_psoc *psoc,
				     bool *he_supported)
{
	tDot11fIEhe_cap *he_cap = NULL;

	*he_supported = false;
	mlme_cfg_get_he_caps(psoc, he_cap);
	if (he_cap && he_cap->present)
		*he_supported = true;
}
#else
static void wlan_son_is_he_supported(struct wlan_objmgr_psoc *psoc,
				     bool *he_supported)
{
	*he_supported = false;
}
#endif /*WLAN_FEATURE_11AX*/

uint32_t wlan_son_get_chan_flag(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq, bool flag_160,
				struct ch_params *chan_params)
{
	uint32_t flags = 0;
	qdf_freq_t sec_freq;
	struct ch_params ch_width40_ch_params;
	uint8_t sub_20_channel_width = 0;
	enum phy_ch_width bandwidth = mlme_get_vht_ch_width();
	struct wlan_objmgr_psoc *psoc;
	bool is_he_enabled;

	if (!pdev) {
		qdf_err("invalid pdev");
		return flags;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		qdf_err("invalid psoc");
		return flags;
	}

	wlan_son_is_he_supported(psoc, &is_he_enabled);

	wlan_mlme_get_sub_20_chan_width(wlan_pdev_get_psoc(pdev),
					&sub_20_channel_width);

	qdf_mem_zero(chan_params, sizeof(*chan_params));
	qdf_mem_zero(&ch_width40_ch_params, sizeof(ch_width40_ch_params));
	if (wlan_reg_is_24ghz_ch_freq(freq)) {
		if (bandwidth == CH_WIDTH_80P80MHZ ||
		    bandwidth == CH_WIDTH_160MHZ ||
		    bandwidth == CH_WIDTH_80MHZ)
			bandwidth = CH_WIDTH_40MHZ;
	}

	switch (bandwidth) {
	case CH_WIDTH_80P80MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (!flag_160) {
				chan_params->ch_width = CH_WIDTH_80P80MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80_80);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80_80;
		}
		bandwidth = CH_WIDTH_160MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_160MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (flag_160) {
				chan_params->ch_width = CH_WIDTH_160MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE160);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT160;
		}
		bandwidth = CH_WIDTH_80MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_80MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (!flag_160 &&
			    chan_params->ch_width != CH_WIDTH_80P80MHZ) {
				chan_params->ch_width = CH_WIDTH_80MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
					QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80;
		}
		bandwidth = CH_WIDTH_40MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_40MHZ:
		ch_width40_ch_params.ch_width = bandwidth;
		wlan_reg_set_channel_params_for_freq(pdev, freq, 0,
						     &ch_width40_ch_params);

		if (ch_width40_ch_params.sec_ch_offset == LOW_PRIMARY_CH)
			sec_freq = freq + 20;
		else if (ch_width40_ch_params.sec_ch_offset == HIGH_PRIMARY_CH)
			sec_freq = freq - 20;
		else
			sec_freq = 0;

		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       sec_freq) !=
		    CHANNEL_STATE_INVALID) {
			if (ch_width40_ch_params.sec_ch_offset ==
			    LOW_PRIMARY_CH) {
				if (is_he_enabled)
				  flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40PLUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40PLUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS;
			} else if (ch_width40_ch_params.sec_ch_offset ==
				   HIGH_PRIMARY_CH) {
				if (is_he_enabled)
				  flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40MINUS;
				flags |=
				   QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40MINUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS;
			}
		}
		bandwidth = CH_WIDTH_20MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_20MHZ:
		flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT20;
		flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT20;
		if (is_he_enabled)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE20;
		bandwidth = CH_WIDTH_10MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_10MHZ:
		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       0) !=
		     CHANNEL_STATE_INVALID &&
		     sub_20_channel_width == WLAN_SUB_20_CH_WIDTH_10)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HALF;
		bandwidth = CH_WIDTH_5MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_5MHZ:
		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       0) !=
		    CHANNEL_STATE_INVALID &&
		    sub_20_channel_width == WLAN_SUB_20_CH_WIDTH_5)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_QUARTER;
		break;
	default:
		qdf_info("invalid channel width value %d", bandwidth);
	}

	return flags;
}

QDF_STATUS wlan_son_peer_set_kickout_allow(struct wlan_objmgr_vdev *vdev,
					   struct wlan_objmgr_peer *peer,
					   bool kickout_allow)
{
	struct peer_mlme_priv_obj *peer_priv;

	if (!peer) {
		qdf_err("invalid peer");
		return QDF_STATUS_E_INVAL;
	}
	if (!vdev) {
		qdf_err("invalid vdev");
		return QDF_STATUS_E_INVAL;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_MLME);
	if (!peer_priv) {
		qdf_err("invalid vdev");
		return QDF_STATUS_E_INVAL;
	}

	peer_priv->allow_kickout = kickout_allow;

	return QDF_STATUS_SUCCESS;
}

bool wlan_son_peer_is_kickout_allow(struct wlan_objmgr_vdev *vdev,
				    uint8_t *macaddr)
{
	bool kickout_allow = true;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_psoc *psoc;
	struct peer_mlme_priv_obj *peer_priv;

	if (!vdev) {
		qdf_err("invalid vdev");
		return kickout_allow;
	}
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		qdf_err("invalid psoc");
		return kickout_allow;
	}
	peer = wlan_objmgr_get_peer_by_mac(psoc, macaddr,
					   WLAN_SON_ID);

	if (!peer) {
		qdf_err("peer is null");
		return kickout_allow;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_MLME);
	if (!peer_priv) {
		qdf_err("invalid vdev");
		wlan_objmgr_peer_release_ref(peer, WLAN_SON_ID);
		return kickout_allow;
	}
	kickout_allow = peer_priv->allow_kickout;
	wlan_objmgr_peer_release_ref(peer, WLAN_SON_ID);

	return kickout_allow;
}

void wlan_son_ind_assoc_req_frm(struct wlan_objmgr_vdev *vdev,
				uint8_t *macaddr, bool is_reassoc,
				uint8_t *frame, uint16_t frame_len,
				QDF_STATUS status)
{
	struct wlan_objmgr_peer *peer;
	struct wlan_lmac_if_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	uint16_t assocstatus = IEEE80211_STATUS_UNSPECIFIED;
	uint16_t sub_type = IEEE80211_FC0_SUBTYPE_ASSOC_REQ;

	if (!vdev) {
		qdf_err("invalid vdev");
		return;
	}
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		qdf_err("invalid psoc");
		return;
	}
	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops || !rx_ops->son_rx_ops.process_mgmt_frame) {
		qdf_err("invalid rx ops");
		return;
	}
	peer = wlan_objmgr_get_peer_by_mac(psoc, macaddr,
					   WLAN_SON_ID);
	if (!peer) {
		qdf_err("peer is null");
		return;
	}

	if (is_reassoc)
		sub_type = IEEE80211_FC0_SUBTYPE_REASSOC_REQ;
	if (QDF_IS_STATUS_SUCCESS(status))
		assocstatus = IEEE80211_STATUS_SUCCESS;
	qdf_debug("subtype %u frame_len %u assocstatus %u",
		  sub_type, frame_len, assocstatus);
	rx_ops->son_rx_ops.process_mgmt_frame(vdev, peer, sub_type,
					      frame, frame_len,
					      &assocstatus);
	wlan_objmgr_peer_release_ref(peer, WLAN_SON_ID);
}

static int wlan_son_deliver_mlme_event(struct wlan_objmgr_vdev *vdev,
				       struct wlan_objmgr_peer *peer,
				       uint32_t event,
				       void *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_rx_ops *rx_ops;
	int ret;

	if (!vdev)
		return -EINVAL;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (rx_ops && rx_ops->son_rx_ops.deliver_event) {
		qdf_debug("deliver mlme event %d", event);
		ret = rx_ops->son_rx_ops.deliver_event(vdev,
						       peer,
						       event,
						       event_data);
	} else {
		return -EINVAL;
	}

	return ret;
}

int wlan_son_deliver_tx_power(struct wlan_objmgr_vdev *vdev,
			      int32_t max_pwr)
{
	int ret;

	qdf_debug("tx power %d", max_pwr);
	ret = wlan_son_deliver_mlme_event(vdev,
					  NULL,
					  MLME_EVENT_TX_PWR_CHANGE,
					  &max_pwr);

	return ret;
}

int wlan_son_deliver_vdev_stop(struct wlan_objmgr_vdev *vdev)
{
	int ret;

	struct wlan_vdev_state_event event;

	event.state = VDEV_STATE_STOPPED;
	qdf_debug("state %d", event.state);
	ret = wlan_son_deliver_mlme_event(vdev,
					  NULL,
					  MLME_EVENT_VDEV_STATE,
					  &event);

	return ret;
}

int wlan_son_deliver_inst_rssi(struct wlan_objmgr_vdev *vdev,
			       struct wlan_objmgr_peer *peer,
			       uint32_t irssi)
{
	struct wlan_peer_inst_rssi event;
	int ret;

	if (irssi > 0 && irssi <= 127) {
		event.iRSSI = irssi;
		event.valid = true;
		qdf_debug("irssi %d", event.iRSSI);
	} else {
		event.valid = false;
		qdf_debug("irssi invalid");
	}

	ret = wlan_son_deliver_mlme_event(vdev,
					  peer,
					  MLME_EVENT_INST_RSSI,
					  &event);

	return ret;
}

int wlan_son_deliver_opmode(struct wlan_objmgr_vdev *vdev,
			    uint8_t bw,
			    uint8_t nss,
			    uint8_t *addr)
{
	struct wlan_objmgr_psoc *psoc;
	struct ieee80211_opmode_update_data opmode;

	if (!vdev)
		return -EINVAL;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	opmode.max_chwidth = bw;
	opmode.num_streams = nss;
	qdf_mem_copy(opmode.macaddr, addr, QDF_MAC_ADDR_SIZE);

	qdf_debug("bw %d, nss %d, addr " QDF_FULL_MAC_FMT,
		  bw, nss, QDF_FULL_MAC_REF(addr));

	if (!g_son_mlme_deliver_cbs.deliver_opmode) {
		qdf_err("invalid deliver opmode cb");
		return -EINVAL;
	}

	g_son_mlme_deliver_cbs.deliver_opmode(vdev,
					      sizeof(opmode),
					      (uint8_t *)&opmode);

	return 0;
}

int wlan_son_deliver_smps(struct wlan_objmgr_vdev *vdev,
			  uint8_t is_static,
			  uint8_t *addr)
{
	struct wlan_objmgr_psoc *psoc;
	struct ieee80211_smps_update_data smps;

	if (!vdev)
		return -EINVAL;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return -EINVAL;

	smps.is_static = is_static;
	qdf_mem_copy(smps.macaddr, addr, QDF_MAC_ADDR_SIZE);

	qdf_debug("is_static %d, addr" QDF_FULL_MAC_FMT,
		  is_static, QDF_FULL_MAC_REF(addr));

	if (!g_son_mlme_deliver_cbs.deliver_smps) {
		qdf_err("invalid deliver smps cb");
		return -EINVAL;
	}

	g_son_mlme_deliver_cbs.deliver_smps(vdev,
					    sizeof(smps),
					    (uint8_t *)&smps);

	return 0;
}

int wlan_son_deliver_rrm_rpt(struct wlan_objmgr_vdev *vdev,
			     uint8_t *mac_addr,
			     uint8_t *frm,
			     uint32_t flen)
{
	struct wlan_act_frm_info rrm_info;
	struct wlan_lmac_if_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_peer *peer;
	uint8_t sub_type = IEEE80211_FC0_SUBTYPE_ACTION;
	struct ieee80211_action ia;
	const uint8_t *ie, *pos, *end;
	uint8_t total_bcnrpt_count = 0;

	if (!vdev) {
		qdf_err("invalid vdev");
		return -EINVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		qdf_err("invalid psoc");
		return -EINVAL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops || !rx_ops->son_rx_ops.process_mgmt_frame) {
		qdf_err("invalid rx ops");
		return -EINVAL;
	}

	peer = wlan_objmgr_get_peer_by_mac(psoc, mac_addr, WLAN_SON_ID);
	if (!peer) {
		qdf_err("peer is null");
		return -EINVAL;
	}

	ia.ia_category = ACTION_CATEGORY_RRM;
	ia.ia_action = RRM_RADIO_MEASURE_RPT;
	qdf_mem_zero(&rrm_info, sizeof(rrm_info));
	rrm_info.ia = &ia;
	rrm_info.ald_info = 0;
	qdf_mem_copy(rrm_info.data.rrm_data.macaddr,
		     mac_addr,
		     QDF_MAC_ADDR_SIZE);
	/* IEEE80211_ACTION_RM_TOKEN */
	rrm_info.data.rrm_data.dialog_token = *frm;

	/* Points to Measurement Report Element */
	++frm;
	--flen;
	pos = frm;
	end = pos + flen;

	while ((ie = wlan_get_ie_ptr_from_eid(WLAN_ELEMID_MEASREP,
					      pos, end - pos))) {
		if (ie[1] < 3) {
			qdf_err("Bad Measurement Report element");
			wlan_objmgr_peer_release_ref(peer, WLAN_SON_ID);
			return -EINVAL;
		}
		if (ie[4] == SIR_MAC_RRM_BEACON_TYPE)
			++total_bcnrpt_count;
		pos = ie + ie[1] + 2;
	}

	rrm_info.data.rrm_data.num_meas_rpts = total_bcnrpt_count;

	qdf_debug("Sta: " QDF_FULL_MAC_FMT
		  "Category %d Action %d Num_Report %d Rptlen %d",
		  QDF_FULL_MAC_REF(mac_addr),
		  ACTION_CATEGORY_RRM,
		  RRM_RADIO_MEASURE_RPT,
		  total_bcnrpt_count,
		  flen);

	rx_ops->son_rx_ops.process_mgmt_frame(vdev, peer, sub_type,
					      frm, flen, &rrm_info);

	wlan_objmgr_peer_release_ref(peer, WLAN_SON_ID);

	return 0;
}
