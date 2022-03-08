/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *
 * This file sch_api.cc contains functions related to the API exposed
 * by scheduler module
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "ani_global.h"
#include "wni_cfg.h"

#include "sir_mac_prot_def.h"
#include "sir_mac_prop_exts.h"
#include "sir_common.h"

#include "lim_api.h"

#include "sch_api.h"

#include "lim_trace.h"
#include "lim_types.h"
#include "lim_utils.h"

#include "wma_types.h"

#ifdef WLAN_FEATURE_11BE_MLO
#include "lim_mlo.h"
#endif

#include <target_if_vdev_mgr_tx_ops.h>
#include <wlan_cmn_ieee80211.h>
#include <wlan_mgmt_txrx_utils_api.h>

/* Fils Dicovery Frame */
/**
 * struct fd_action_header - FILS Discovery Action frame header
 * @action_header: WLAN Action frame header
 * @fd_frame_cntl: FILS Disovery Frame Control
 * @timestamp:     Time stamp
 * @bcn_interval:  Beacon Interval
 * @elem:          variable len sub element fields
 */
struct fd_action_header {
	struct action_frm_hdr action_header;
	uint16_t              fd_frame_cntl;
	uint8_t               timestamp[WLAN_TIMESTAMP_LEN];
	uint16_t              bcn_interval;
	uint8_t               elem[];
} qdf_packed;

/**
 * struct tpe_ie - Transmit Power Enevolpe IE
 * @tpe_header:           WLAN IE Header
 * @max_tx_pwr_count:     Maximum Transmit Power Count
 * @max_tx_pwr_interpret: Maximum Transmit Power Interpretation
 * @max_tx_pwr_category:  Maximum Transmit Power category
 * @tx_pwr_info:          Transmit power Information
 * @elem:                 variable len sub element fields
 */
struct tpe_ie {
	struct ie_header tpe_header;
	union {
		struct {
			uint8_t max_tx_pwr_count:3;
			uint8_t max_tx_pwr_interpret:3;
			uint8_t max_tx_pwr_category:2;
		};
		uint8_t tx_pwr_info;
	};
	uint8_t elem[];
} qdf_packed;

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * lim_notify_link_info() - notify partner link to update beacon template
 * @pe_session: pointer to pe session
 *
 * Return: void
 */
static void lim_notify_link_info(struct pe_session *pe_session)
{
	struct wlan_objmgr_vdev *wlan_vdev_list[WLAN_UMAC_MLO_MAX_VDEVS];
	uint16_t vdev_count = 0;
	int link;

	if (!pe_session->mlo_link_info.upt_bcn_mlo_ie &&
	    !mlme_is_notify_co_located_ap_update_rnr(pe_session->vdev))
		return;
	pe_session->mlo_link_info.upt_bcn_mlo_ie = false;
	mlme_set_notify_co_located_ap_update_rnr(pe_session->vdev, false);
	pe_debug("vdev id %d mlo notify beacon change info to partner link",
		 wlan_vdev_get_id(pe_session->vdev));
	lim_get_mlo_vdev_list(pe_session, &vdev_count,
			      wlan_vdev_list);
	for (link = 0; link < vdev_count; link++) {
		if (!wlan_vdev_list[link])
			continue;
		if (wlan_vdev_list[link] == pe_session->vdev) {
			lim_mlo_release_vdev_ref(wlan_vdev_list[link]);
			continue;
		}
		lim_partner_link_info_change(wlan_vdev_list[link]);
		lim_mlo_release_vdev_ref(wlan_vdev_list[link]);
	}
}

/**
 * lim_update_sch_mlo_partner() - update partner information needed in mlo IE
 * @mac: pointer to mac
 * @pe_session: pointer to pe session
 * @bcn_param: pointer to tpSendbeaconParams
 *
 * Return: void
 */
static void lim_update_sch_mlo_partner(struct mac_context *mac,
				       struct pe_session *pe_session,
				       tpSendbeaconParams bcn_param)
{
	int link;
	struct ml_sch_partner_info *sch_info;
	struct ml_bcn_partner_info *bcn_info;

	bcn_param->mlo_partner.num_links = mac->sch.sch_mlo_partner.num_links;
	for (link = 0; link < mac->sch.sch_mlo_partner.num_links; link++) {
		sch_info = &mac->sch.sch_mlo_partner.partner_info[link];
		bcn_info = &bcn_param->mlo_partner.partner_info[link];
		bcn_info->vdev_id = sch_info->vdev_id;
		bcn_info->beacon_interval = sch_info->beacon_interval;
		bcn_info->csa_switch_count_offset = sch_info->bcn_csa_cnt_ofst;
		bcn_info->ext_csa_switch_count_offset =
					sch_info->bcn_ext_csa_cnt_ofst;
	}
}
#else
static void lim_notify_link_info(struct pe_session *pe_session)
{
}

static void lim_update_sch_mlo_partner(struct mac_context *mac,
				       struct pe_session *pe_session,
				       tpSendbeaconParams bcn_param)
{
}
#endif

#ifdef WLAN_FEATURE_11BE
/**
 * lim_fd_cap_channel_width320() - populate the capability field for
 * 320 channel width in the fils discovery template
 * @pe_session: pointer to pe session
 * @fd_cap: pointer to fils discovery capability variable
 *
 * Return: void
 */
static void lim_fd_cap_channel_width320(struct pe_session *pe_session,
					uint8_t *fd_cap)
{
	if (pe_session->ch_width == CH_WIDTH_320MHZ) {
		*fd_cap |= (WLAN_FD_CHWIDTH_320 << WLAN_FD_CAP_BSS_CHWIDTH_S);
	} else {
		pe_err("channel width : %d is not supported",
		       pe_session->ch_width);
	}
}

/**
 * lim_populate_fd_capability() - populate the capability field for
 * EHT phymode in the fils discovery template
 * @cur_phymode: current phymode
 * @fd_cap: pointer to fils discovery capability variable
 *
 * Return: void
 */
static void lim_fd_cap_phymode_EHT(enum wlan_phymode phymode, uint8_t *fd_cap)
{
	switch (phymode) {
	case WLAN_PHYMODE_11BEA_EHT20:
	case WLAN_PHYMODE_11BEG_EHT20:
	case WLAN_PHYMODE_11BEA_EHT40:
	case WLAN_PHYMODE_11BEG_EHT40:
	case WLAN_PHYMODE_11BEG_EHT40PLUS:
	case WLAN_PHYMODE_11BEG_EHT40MINUS:
	case WLAN_PHYMODE_11BEA_EHT80:
	case WLAN_PHYMODE_11BEA_EHT160:
	case WLAN_PHYMODE_11BEA_EHT320:
		*fd_cap |= (WLAN_FD_CAP_PHY_INDEX_EHT <<
						WLAN_FD_CAP_PHY_INDEX_S);
		break;
	default:
		*fd_cap |= (WLAN_FD_CAP_PHY_INDEX_NON_HT_OFDM <<
						WLAN_FD_CAP_PHY_INDEX_S);
		break;
	}
}
#else
static void lim_fd_cap_channel_width320(struct pe_session *pe_session,
					uint8_t *fd_cap)
{
	pe_err("channel width : %d is not supported", pe_session->ch_width);
}

static void lim_fd_cap_phymode_EHT(enum wlan_phymode phymode, uint8_t *fd_cap)
{
	*fd_cap |= (WLAN_FD_CAP_PHY_INDEX_NON_HT_OFDM <<
						WLAN_FD_CAP_PHY_INDEX_S);
}
#endif /* WLAN_FEATURE_11BE */

/**
 * lim_populate_fd_capability() - populate the capability field in the
 * fils discovery template
 * @pe_session: pointer to pe session
 * @cur_phymode: current phymode
 * @fd_cap: pointer to fils discovery capability array
 *
 * Return: void
 */
static void lim_populate_fd_capability(struct pe_session *pe_session,
				       enum wlan_phymode cur_phymode,
				       uint8_t *fd_cap)
{
	/* Setting ESS and Privacy bits */
	fd_cap[0] |= ((!WLAN_FD_CAP_ESS_ENABLE << WLAN_FD_CAP_ESS_S) |
		((pe_session->privacy) << WLAN_FD_CAP_PRIVACY_S));

	/* Channel Width Selection */
	switch (pe_session->ch_width) {
	case CH_WIDTH_20MHZ:
		fd_cap[0] |= (WLAN_FD_CHWIDTH_20 << WLAN_FD_CAP_BSS_CHWIDTH_S);
		break;
	case CH_WIDTH_40MHZ:
		fd_cap[0] |= (WLAN_FD_CHWIDTH_40 << WLAN_FD_CAP_BSS_CHWIDTH_S);
		break;
	case CH_WIDTH_80MHZ:
		fd_cap[0] |= (WLAN_FD_CHWIDTH_80 << WLAN_FD_CAP_BSS_CHWIDTH_S);
		break;
	case CH_WIDTH_160MHZ:
	case CH_WIDTH_80P80MHZ:
		fd_cap[0] |= (WLAN_FD_CHWIDTH_160_80_80 <<
						WLAN_FD_CAP_BSS_CHWIDTH_S);
		break;
	default:
		lim_fd_cap_channel_width320(pe_session, &fd_cap[0]);
		break;
	}

	/* Max Num of Spatial Steam */
	switch (pe_session->nss) {
	case WLAN_FD_CAP_NSS_MODE_1:
	case WLAN_FD_CAP_NSS_MODE_2:
		fd_cap[0] |= ((pe_session->nss - 1) << WLAN_FD_CAP_NSS_S);
		break;
	case WLAN_FD_CAP_NSS_MODE_3:
	case WLAN_FD_CAP_NSS_MODE_4:
	case WLAN_FD_CAP_NSS_MODE_5:
	case WLAN_FD_CAP_NSS_MODE_6:
	case WLAN_FD_CAP_NSS_MODE_7:
	case WLAN_FD_CAP_NSS_MODE_8:
		fd_cap[0] |= (WLAN_FD_CAP_NSS_GTE_5 << WLAN_FD_CAP_NSS_S);
		break;
	default:
		pe_err("NSS value: %d is not supported", pe_session->nss);
		break;
	}

	/* Set PHY index */
	switch (cur_phymode) {
	case WLAN_PHYMODE_11AXA_HE20:
	case WLAN_PHYMODE_11AXG_HE20:
	case WLAN_PHYMODE_11AXA_HE40:
	case WLAN_PHYMODE_11AXG_HE40:
	case WLAN_PHYMODE_11AXG_HE40PLUS:
	case WLAN_PHYMODE_11AXG_HE40MINUS:
	case WLAN_PHYMODE_11AXA_HE80:
	case WLAN_PHYMODE_11AXA_HE160:
	case WLAN_PHYMODE_11AXA_HE80_80:
		fd_cap[1] |= (WLAN_FD_CAP_PHY_INDEX_HE <<
					WLAN_FD_CAP_PHY_INDEX_S);
		break;
	case WLAN_PHYMODE_11AC_VHT20:
	case WLAN_PHYMODE_11AC_VHT40:
	case WLAN_PHYMODE_11AC_VHT80:
	case WLAN_PHYMODE_11AC_VHT160:
	case WLAN_PHYMODE_11AC_VHT80_80:
		fd_cap[1] |= (WLAN_FD_CAP_PHY_INDEX_VHT <<
					WLAN_FD_CAP_PHY_INDEX_S);
		break;
	case WLAN_PHYMODE_11NA_HT20:
	case WLAN_PHYMODE_11NG_HT20:
	case WLAN_PHYMODE_11NG_HT40PLUS:
	case WLAN_PHYMODE_11NG_HT40MINUS:
	case WLAN_PHYMODE_11NG_HT40:
	case WLAN_PHYMODE_11NA_HT40:
		fd_cap[1] |= (WLAN_FD_CAP_PHY_INDEX_HT <<
					WLAN_FD_CAP_PHY_INDEX_S);
		break;
	default:
		lim_fd_cap_phymode_EHT(cur_phymode, &fd_cap[1]);
		break;
	}

	/* FILS Min Rate */
	fd_cap[1] |= (WLAN_FD_CAP_MIN_RATE << WLAN_FD_CAP_MIN_RATE_S);
}

/**
 * lim_populate_fd_tmpl_frame() - populate the fils discovery frame
 * @mac: pointer to mac structure
 * @frm: pointer to fils discovery frame
 * @pe_session:pointer to pe session
 * @frame_size: pointer to fils discovery frame size
 *
 * return: success: QDF_STATUS_SUCCESS failure: QDF_STATUS_E_FAILURE
 */
static QDF_STATUS lim_populate_fd_tmpl_frame(struct mac_context *mac,
					     struct pe_session *pe_session,
					     uint8_t *frm, uint32_t *frame_size)
{
	uint16_t fd_cntl_subfield = 0;
	struct fd_action_header *fd_header;
	struct wlan_objmgr_vdev *vdev;
	uint8_t fd_cap[WLAN_FD_CAP_LEN] = {0};
	uint8_t length = 0;
	uint8_t ssid_len = 0, ssid[WLAN_SSID_MAX_LEN + 1] = {0};
	uint32_t shortssid;
	uint16_t chwidth = pe_session->ch_width;
	qdf_freq_t cur_chan_freq = pe_session->curr_op_freq;
	struct wlan_channel *des_chan;
	enum wlan_phymode cur_phymode;
	uint16_t tpe_num = 0;
	tDot11fIEtransmit_power_env tpe[WLAN_MAX_NUM_TPE_IE];
	struct tpe_ie *tpe_ie;
	uint8_t i, idx;
	tSirMacMgmtHdr *mac_hdr;
	struct qdf_mac_addr broadcast_mac_addr = QDF_MAC_ADDR_BCAST_INIT;

	pe_debug("FD TMPL freq: %d chWidth: %d", cur_chan_freq, chwidth);

	vdev = pe_session->vdev;
	if (!vdev) {
		pe_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);
	if (!des_chan) {
		pe_err("des_chan is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	cur_phymode = des_chan->ch_phymode;

	lim_populate_mac_header(mac, frm, SIR_MAC_MGMT_FRAME,
				SIR_MAC_MGMT_ACTION, broadcast_mac_addr.bytes,
				pe_session->self_mac_addr);
	mac_hdr = (tpSirMacMgmtHdr)frm;
	sir_copy_mac_addr(mac_hdr->bssId, pe_session->bssId);
	frm += sizeof(*mac_hdr);
	*frame_size = sizeof(*mac_hdr);

	/* filling fd header */
	fd_header = (struct fd_action_header *)frm;
	fd_header->action_header.action_category = ACTION_CATEGORY_PUBLIC;
	fd_header->action_header.action_code  = WLAN_ACTION_FILS_DISCOVERY;

	/*
	 * FILS DIscovery Frame Control Subfield - 2 byte
	 * Enable Short SSID
	 * When the Short SSID Indicator subfield is equal to 1,
	 * the SSID Length subfield is equal to 3
	 */
	fd_cntl_subfield = WLAN_FD_SSID_LEN_PRES(WLAN_FD_FRAMECNTL_SHORTSSID_LEN);
	fd_cntl_subfield |= WLAN_FD_FRAMECNTL_SHORTSSID;

	if (wlan_reg_is_6ghz_chan_freq(cur_chan_freq)) {
		fd_cntl_subfield |= WLAN_FD_FRAMECNTL_CAP;
		pe_debug("FD Capability Present");
		length = WLAN_FD_CAP_LEN;
	}

	/* For 80+80 set Channel center freq segment 1 */
	if (IS_WLAN_PHYMODE_160MHZ(cur_phymode)) {
		fd_cntl_subfield |= WLAN_FD_FRAMECNTL_CH_CENTERFREQ;
		pe_debug("Center frequenceny Present");
		length += 1;
	}

	/* Update the length field */
	/*Indicates length from FD cap to Mobility Domain */
	if (length)
		fd_cntl_subfield |= WLAN_FD_FRAMECNTL_LEN_PRES;

	/* FD Control - 2 bytes */
	fd_header->fd_frame_cntl = qdf_cpu_to_le16(fd_cntl_subfield);

	/* Timestamp - 8 bytes */
	qdf_mem_zero(fd_header->timestamp, sizeof(fd_header->timestamp));

	/* Beacon Interval - 2 bytes */
	fd_header->bcn_interval =
		qdf_cpu_to_le16(pe_session->beaconParams.beaconInterval);

	*frame_size += sizeof(*fd_header);

	/* Variable length data */
	frm = &fd_header->elem[0];

	/* Short SSID - 4 bytes */
	wlan_vdev_mlme_get_ssid(vdev, ssid, &ssid_len);
	shortssid = wlan_construct_shortssid(ssid, ssid_len);
	*(uint32_t *)frm = qdf_cpu_to_le32(shortssid);
	frm += 4;
	*frame_size += 4;
	pe_debug("Category:%02x Action:%02x  fd_cntl:%02x bcn_intvl:%02x short ssid:%02x frame_size:%02x",
		 fd_header->action_header.action_category,
		 fd_header->action_header.action_code, fd_cntl_subfield,
		 fd_header->bcn_interval, shortssid, *frame_size);

	/* Length - 1 byte */
	if (length) {
		*frm = length;
		pe_debug("length: %d", length);
		frm++;
		*frame_size += length + 1;
	}

	/* FD Capabilities - 2 bytes */
	if (WLAN_FD_IS_CAP_PRESENT(fd_cntl_subfield)) {
		lim_populate_fd_capability(pe_session, cur_phymode, &fd_cap[0]);
		qdf_mem_copy(frm, fd_cap, WLAN_FD_CAP_LEN);
		frm += WLAN_FD_CAP_LEN;
		pe_debug("fd_cap: %02x %02x", fd_cap[0], fd_cap[1]);
	}

	/* Channel Center Freq Segment 1 - 1 byte */
	if (WLAN_FD_IS_FRAMECNTL_CH_CENTERFREQ(fd_cntl_subfield)) {
		/* spec has seg0 and seg1 naming while we use seg1 and seg2 */
		*frm = des_chan->ch_freq_seg1;
		frm++;
		pe_debug("ch_center_freq: %02x", des_chan->ch_freq_seg1);
	}

	/* Add TPE IE */
	if ((wlan_reg_is_6ghz_chan_freq(cur_chan_freq)) ||
	    (pe_session->vhtCapability)) {
		populate_dot11f_tx_power_env(mac, &tpe[0], chwidth,
					     cur_chan_freq, &tpe_num, false);
		pe_debug("tpe_num: %02x", tpe_num);
		if (tpe_num > WLAN_MAX_NUM_TPE_IE) {
			pe_err("tpe_num  %d greater than max size", tpe_num);
			return QDF_STATUS_E_FAILURE;
		}

		for (idx = 0; idx < tpe_num; idx++) {
			/* filling tpe_header header */
			tpe_ie = (struct tpe_ie *)frm;
			tpe_ie->tpe_header.ie_id = WLAN_ELEMID_VHT_TX_PWR_ENVLP;

			if (tpe[idx].num_tx_power > WLAN_MAX_NUM_TPE_IE) {
				pe_err("num_tx_power %d greater than max num",
				       tpe[idx].num_tx_power);
				return QDF_STATUS_E_FAILURE;
			}

			/* +1 for including tx power info */
			tpe_ie->tpe_header.ie_len  = tpe[idx].num_tx_power + 1;

			if (tpe_ie->tpe_header.ie_len < WLAN_TPE_IE_MIN_LEN ||
			    tpe_ie->tpe_header.ie_len > WLAN_TPE_IE_MAX_LEN) {
				pe_err("tpe length %d less than min len or greater than max len",
				       tpe_ie->tpe_header.ie_len);
				return QDF_STATUS_E_FAILURE;
			}

			tpe_ie->max_tx_pwr_count = tpe[idx].max_tx_pwr_count;
			tpe_ie->max_tx_pwr_interpret =
						tpe[idx].max_tx_pwr_interpret;
			tpe_ie->max_tx_pwr_category =
						tpe[idx].max_tx_pwr_category;
			pe_debug("tx_pwr_info: %02x", tpe_ie->tx_pwr_info);
			frm = &tpe_ie->elem[0];

			for (i = 0; i < tpe[idx].num_tx_power; i++) {
				*frm = tpe[idx].tx_power[i];
				pe_debug("tx_pwr[%d]: %02x", i, *frm);
				frm++;
			}

			/* +2 for including element id and length */
			*frame_size += tpe_ie->tpe_header.ie_len + 2;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * lim_send_fils_discovery_template() - send fils discovery template to
 * target_if
 * @mac: pointer to mac structure
 * @pe_session:pe session
 *
 * return: status
 */
static QDF_STATUS lim_send_fils_discovery_template(struct mac_context *mac,
						   struct pe_session *pe_session)
{
	struct fils_discovery_tmpl_params *fd_params;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t n_bytes = sizeof(*fd_params);

	fd_params = qdf_mem_malloc(n_bytes);

	if (!fd_params)
		return QDF_STATUS_E_NOMEM;

	fd_params->vdev_id = pe_session->vdev_id;

	fd_params->frm = qdf_mem_malloc(SIR_MAX_FD_TMPL_SIZE);
	if (!(fd_params->frm)) {
		qdf_mem_free(fd_params);
		return QDF_STATUS_E_NOMEM;
	}

	status = lim_populate_fd_tmpl_frame(mac, pe_session, fd_params->frm,
					    &n_bytes);

	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("FAIL bytes %d retcode[%X]", n_bytes, status);
		goto memfree;
	}

	pe_debug("Fils Discovery template created successfully %d", n_bytes);

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
			   fd_params->frm, n_bytes);

	fd_params->tmpl_len = n_bytes;
	fd_params->tmpl_len_aligned = roundup(fd_params->tmpl_len,
					      sizeof(uint32_t));

	/* Sending data to wmi layer via target_if */
	status = target_if_vdev_mgr_send_fd_tmpl(pe_session->vdev,
						 fd_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("FAIL bytes %d retcode[%X]", n_bytes, status);
	} else {
		pe_debug("Fils Discovery tmpl msg posted to HAL of bytes %d",
			 n_bytes);
	}

memfree:
	qdf_mem_free(fd_params->frm);
	qdf_mem_free(fd_params);
	return status;
}

QDF_STATUS sch_send_beacon_req(struct mac_context *mac, uint8_t *beaconPayload,
			       uint16_t size, struct pe_session *pe_session,
			       enum sir_bcn_update_reason reason)
{
	struct scheduler_msg msgQ = {0};
	tpSendbeaconParams beaconParams = NULL;
	QDF_STATUS retCode;

	pe_debug("Indicating HAL to copy the beacon template [%d bytes] to memory, reason %d",
		size, reason);

	if (LIM_IS_AP_ROLE(pe_session) &&
	   (mac->sch.beacon_changed)) {
		retCode = lim_send_probe_rsp_template_to_hal(mac,
				pe_session,
				&pe_session->DefProbeRspIeBitmap[0]);
		if (QDF_STATUS_SUCCESS != retCode)
			pe_err("FAILED to send probe response template with retCode %d",
				retCode);
		/*Fils Discovery Template */
		retCode = lim_send_fils_discovery_template(mac, pe_session);
		if (QDF_STATUS_SUCCESS != retCode)
			pe_err("FAILED to send fils discovery template retCode %d",
			       retCode);
	}

	beaconParams = qdf_mem_malloc(sizeof(tSendbeaconParams));
	if (!beaconParams)
		return QDF_STATUS_E_NOMEM;

	msgQ.type = WMA_SEND_BEACON_REQ;

	/* No Dialog Token reqd, as a response is not solicited */
	msgQ.reserved = 0;

	/* Fill in tSendbeaconParams members */
	qdf_mem_copy(beaconParams->bssId, pe_session->bssId,
		     sizeof(pe_session->bssId));


	beaconParams->timIeOffset = pe_session->schBeaconOffsetBegin;
	if (pe_session->dfsIncludeChanSwIe) {
		beaconParams->csa_count_offset = mac->sch.csa_count_offset;
		beaconParams->ecsa_count_offset = mac->sch.ecsa_count_offset;
	}
	lim_update_sch_mlo_partner(mac, pe_session, beaconParams);
	beaconParams->vdev_id = pe_session->smeSessionId;
	beaconParams->reason = reason;

	/* p2pIeOffset should be atleast greater than timIeOffset */
	if ((mac->sch.p2p_ie_offset != 0) &&
	    (mac->sch.p2p_ie_offset <
	     pe_session->schBeaconOffsetBegin)) {
		pe_err("Invalid p2pIeOffset:[%d]",
			mac->sch.p2p_ie_offset);
		QDF_ASSERT(0);
		qdf_mem_free(beaconParams);
		return QDF_STATUS_E_FAILURE;
	}
	beaconParams->p2pIeOffset = mac->sch.p2p_ie_offset;

	if (size > SIR_MAX_BEACON_SIZE) {
		pe_err("beacon size (%d) exceed host limit %d",
		       size, SIR_MAX_BEACON_SIZE);
		QDF_ASSERT(0);
		qdf_mem_free(beaconParams);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(beaconParams->beacon, beaconPayload, size);

	beaconParams->beaconLength = (uint32_t) size;
	msgQ.bodyptr = beaconParams;
	msgQ.bodyval = 0;

	MTRACE(mac_trace_msg_tx(mac, pe_session->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode)
		pe_err("Posting SEND_BEACON_REQ to HAL failed, reason=%X",
			retCode);

	if (QDF_IS_STATUS_SUCCESS(retCode)) {
		if (wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
			lim_notify_link_info(pe_session);
		else
			lim_ap_mlme_vdev_rnr_notify(pe_session);
	}

	return retCode;
}

static uint32_t lim_remove_p2p_ie_from_add_ie(struct mac_context *mac,
					      struct pe_session *pe_session,
					      uint8_t *addIeWoP2pIe,
					      uint32_t *addnIELenWoP2pIe)
{
	uint32_t left = pe_session->add_ie_params.probeRespDataLen;
	uint8_t *ptr = pe_session->add_ie_params.probeRespData_buff;
	uint8_t elem_id, elem_len;
	uint32_t offset = 0;
	uint8_t eid = 0xDD;

	qdf_mem_copy(addIeWoP2pIe, ptr, left);
	*addnIELenWoP2pIe = left;

	if (addIeWoP2pIe) {
		while (left >= 2) {
			elem_id  = ptr[0];
			elem_len = ptr[1];
			left -= 2;
			if (elem_len > left) {
				pe_err("Invalid IEs");
				return QDF_STATUS_E_FAILURE;
			}
			if ((elem_id == eid) &&
				(!qdf_mem_cmp(&ptr[2],
					"\x50\x6f\x9a\x09", 4))) {
				left -= elem_len;
				ptr += (elem_len + 2);
				qdf_mem_copy(&addIeWoP2pIe[offset], ptr, left);
				*addnIELenWoP2pIe -= (2 + elem_len);
			} else {
				left -= elem_len;
				ptr += (elem_len + 2);
				offset += 2 + elem_len;
			}
		}
	}
	return QDF_STATUS_SUCCESS;
}

uint32_t lim_send_probe_rsp_template_to_hal(struct mac_context *mac,
					    struct pe_session *pe_session,
					    uint32_t *IeBitmap)
{
	struct scheduler_msg msgQ = {0};
	uint8_t *pFrame2Hal = pe_session->pSchProbeRspTemplate;
	tpSendProbeRespParams pprobeRespParams = NULL;
	uint32_t retCode = QDF_STATUS_E_FAILURE;
	uint32_t nPayload, nBytes = 0, nStatus;
	tpSirMacMgmtHdr pMacHdr;
	uint32_t addnIEPresent = false;
	uint8_t *addIE = NULL;
	uint8_t *addIeWoP2pIe = NULL;
	uint32_t addnIELenWoP2pIe = 0;
	uint32_t retStatus;
	tDot11fIEExtCap extracted_extcap;
	bool extcap_present = false;
	tDot11fProbeResponse *prb_rsp_frm;
	QDF_STATUS status;
	uint16_t addn_ielen = 0;

	/* Check if probe response IE is present or not */
	addnIEPresent = (pe_session->add_ie_params.probeRespDataLen != 0);
	if (addnIEPresent) {
		/*
		* probe response template should not have P2P IE.
		* In case probe request has P2P IE or WPS IE, the
		* probe request will be forwarded to the Host and
		* Host will send the probe response. In other cases
		* FW will send the probe response. So, if the template
		* has P2P IE, the probe response sent to non P2P devices
		* by the FW, may also have P2P IE which will fail
		* P2P cert case 6.1.3
		*/
		addIeWoP2pIe = qdf_mem_malloc(pe_session->add_ie_params.
						probeRespDataLen);
		if (!addIeWoP2pIe)
			return QDF_STATUS_E_NOMEM;

		retStatus = lim_remove_p2p_ie_from_add_ie(mac, pe_session,
					addIeWoP2pIe, &addnIELenWoP2pIe);
		if (retStatus != QDF_STATUS_SUCCESS) {
			qdf_mem_free(addIeWoP2pIe);
			return QDF_STATUS_E_FAILURE;
		}

		/* Probe rsp IE available */
		/*need to check the data length */
		addIE = qdf_mem_malloc(addnIELenWoP2pIe);
		if (!addIE) {
			qdf_mem_free(addIeWoP2pIe);
			return QDF_STATUS_E_NOMEM;
		}
		addn_ielen = addnIELenWoP2pIe;

		if (addn_ielen <= WNI_CFG_PROBE_RSP_ADDNIE_DATA1_LEN &&
		    addn_ielen && (nBytes + addn_ielen) <= SIR_MAX_PACKET_SIZE)
			qdf_mem_copy(addIE, addIeWoP2pIe, addnIELenWoP2pIe);

		qdf_mem_free(addIeWoP2pIe);

		qdf_mem_zero((uint8_t *)&extracted_extcap,
			     sizeof(tDot11fIEExtCap));
		status = lim_strip_extcap_update_struct(mac, addIE,
				&addn_ielen, &extracted_extcap);
		if (QDF_STATUS_SUCCESS != status) {
			pe_debug("extcap not extracted");
		} else {
			extcap_present = true;
		}
	}

	/*
	 * Extcap IE now support variable length, merge Extcap IE from addn_ie
	 * may change the frame size. Therefore, MUST merge ExtCap IE before
	 * dot11f get packed payload size.
	 */
	prb_rsp_frm = &pe_session->probeRespFrame;
	if (extcap_present)
		lim_merge_extcap_struct(&prb_rsp_frm->ExtCap,
					&extracted_extcap,
					true);

	nStatus = dot11f_get_packed_probe_response_size(mac,
			&pe_session->probeRespFrame, &nPayload);
	if (DOT11F_FAILED(nStatus)) {
		pe_err("Failed to calculate the packed size for a Probe Response (0x%08x)",
			nStatus);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeResponse);
	} else if (DOT11F_WARNED(nStatus)) {
		pe_err("There were warnings while calculating the packed size for a Probe Response (0x%08x)",
			nStatus);
	}

	nBytes += nPayload + sizeof(tSirMacMgmtHdr);

	if (addnIEPresent) {
		if ((nBytes + addn_ielen) <= SIR_MAX_PROBE_RESP_SIZE)
			nBytes += addn_ielen;
		else
			addnIEPresent = false;  /* Dont include the IE. */
	}

	/* Make sure we are not exceeding allocated len */
	if (nBytes > SIR_MAX_PROBE_RESP_SIZE) {
		pe_err("nBytes %d greater than max size", nBytes);
		qdf_mem_free(addIE);
		return QDF_STATUS_E_FAILURE;
	}

	/* Paranoia: */
	qdf_mem_zero(pFrame2Hal, nBytes);

	/* Next, we fill out the buffer descriptor: */
	lim_populate_mac_header(mac, pFrame2Hal, SIR_MAC_MGMT_FRAME,
					     SIR_MAC_MGMT_PROBE_RSP,
					     pe_session->self_mac_addr,
					     pe_session->self_mac_addr);

	pMacHdr = (tpSirMacMgmtHdr) pFrame2Hal;

	sir_copy_mac_addr(pMacHdr->bssId, pe_session->bssId);

	/* That done, pack the Probe Response: */
	nStatus =
		dot11f_pack_probe_response(mac, &pe_session->probeRespFrame,
					   pFrame2Hal + sizeof(tSirMacMgmtHdr),
					   nPayload, &nPayload);

	if (DOT11F_FAILED(nStatus)) {
		pe_err("Failed to pack a Probe Response (0x%08x)",
			nStatus);

		qdf_mem_free(addIE);
		return retCode; /* allocated! */
	} else if (DOT11F_WARNED(nStatus)) {
		pe_warn("There were warnings while packing a P"
			"robe Response (0x%08x)", nStatus);
	}

	if (addnIEPresent) {
		qdf_mem_copy(&pFrame2Hal[nBytes - addn_ielen],
			     &addIE[0], addn_ielen);
	}

	qdf_mem_free(addIE);

	pprobeRespParams = qdf_mem_malloc(sizeof(tSendProbeRespParams));
	if (!pprobeRespParams) {
		pe_err("malloc failed for bytes %d", nBytes);
	} else {
		sir_copy_mac_addr(pprobeRespParams->bssId, pe_session->bssId);
		qdf_mem_copy(pprobeRespParams->probeRespTemplate,
			     pFrame2Hal, nBytes);
		pprobeRespParams->probeRespTemplateLen = nBytes;
		qdf_mem_copy(pprobeRespParams->ucProxyProbeReqValidIEBmap,
			     IeBitmap, (sizeof(uint32_t) * 8));
		msgQ.type = WMA_SEND_PROBE_RSP_TMPL;
		msgQ.reserved = 0;
		msgQ.bodyptr = pprobeRespParams;
		msgQ.bodyval = 0;

		retCode = wma_post_ctrl_msg(mac, &msgQ);
		if (QDF_STATUS_SUCCESS != retCode) {
			pe_err("FAIL bytes %d retcode[%X]", nBytes, retCode);
			qdf_mem_free(pprobeRespParams);
		} else {
			pe_debug("Probe response template msg posted to HAL of bytes %d",
				nBytes);
		}
	}

	return retCode;
}

/**
 * sch_gen_timing_advert_frame() - Generate the TA frame and populate the buffer
 * @mac: the global MAC context
 * @self_addr: the self MAC address
 * @buf: the buffer that will contain the frame
 * @timestamp_offset: return for the offset of the timestamp field
 * @time_value_offset: return for the time_value field in the TA IE
 *
 * Return: the length of the buffer.
 */
int sch_gen_timing_advert_frame(struct mac_context *mac_ctx, tSirMacAddr self_addr,
	uint8_t **buf, uint32_t *timestamp_offset, uint32_t *time_value_offset)
{
	tDot11fTimingAdvertisementFrame frame;
	uint32_t payload_size, buf_size;
	int status;
	struct qdf_mac_addr wildcard_bssid = {
		{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	};

	qdf_mem_zero((uint8_t *)&frame, sizeof(tDot11fTimingAdvertisementFrame));

	/* Populate the TA fields */
	status = populate_dot11f_timing_advert_frame(mac_ctx, &frame);
	if (status) {
		pe_err("Error populating TA frame %x", status);
		return status;
	}

	status = dot11f_get_packed_timing_advertisement_frame_size(mac_ctx,
		&frame, &payload_size);
	if (DOT11F_FAILED(status)) {
		pe_err("Error getting packed frame size %x", status);
		return status;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("Warning getting packed frame size");
	}

	buf_size = sizeof(tSirMacMgmtHdr) + payload_size;
	*buf = qdf_mem_malloc(buf_size);
	if (!*buf)
		return QDF_STATUS_E_FAILURE;

	payload_size = 0;
	status = dot11f_pack_timing_advertisement_frame(mac_ctx, &frame,
		*buf + sizeof(tSirMacMgmtHdr), buf_size -
		sizeof(tSirMacMgmtHdr), &payload_size);
	pe_err("TA payload size2 = %d", payload_size);
	if (DOT11F_FAILED(status)) {
		pe_err("Error packing frame %x", status);
		goto fail;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("Warning packing frame");
	}

	lim_populate_mac_header(mac_ctx, *buf, SIR_MAC_MGMT_FRAME,
		SIR_MAC_MGMT_TIME_ADVERT, wildcard_bssid.bytes, self_addr);

	/* The timestamp field is right after the header */
	*timestamp_offset = sizeof(tSirMacMgmtHdr);

	*time_value_offset = sizeof(tSirMacMgmtHdr) +
		sizeof(tDot11fFfTimeStamp) + sizeof(tDot11fFfCapabilities);

	/* Add the Country IE length */
	dot11f_get_packed_ie_country(mac_ctx, &frame.Country,
		time_value_offset);
	/* Add 2 for Country IE EID and Length fields */
	*time_value_offset += 2;

	/* Add the PowerConstraint IE size */
	if (frame.Country.present == 1)
		*time_value_offset += 3;

	/* Add the offset inside TA IE */
	*time_value_offset += 3;

	return payload_size + sizeof(tSirMacMgmtHdr);

fail:
	if (*buf)
		qdf_mem_free(*buf);
	return status;
}
