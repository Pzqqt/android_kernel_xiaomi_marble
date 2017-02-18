/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 *  DOC:    wma_utis.c
 *  This file contains utilities and stats related functions.
 */

/* Header files */

#include "wma.h"
#include "wma_api.h"
#include "cds_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wni_api.h"
#include "ani_global.h"
#include "wmi_unified.h"
#include "wni_cfg.h"
#include "cfg_api.h"

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"
#include "ol_fw.h"

#include "wma_internal.h"
#include "wlan_policy_mgr_api.h"
#include "wmi_unified_param.h"
#include "linux/ieee80211.h"
#include <cdp_txrx_handle.h>
/* MCS Based rate table */
/* HT MCS parameters with Nss = 1 */
static struct index_data_rate_type mcs_nss1[] = {
	/* MCS L20  S20   L40   S40 */
	{0,  {65,  72},  {135,  150 } },
	{1,  {130, 144}, {270,  300 } },
	{2,  {195, 217}, {405,  450 } },
	{3,  {260, 289}, {540,  600 } },
	{4,  {390, 433}, {815,  900 } },
	{5,  {520, 578}, {1080, 1200} },
	{6,  {585, 650}, {1215, 1350} },
	{7,  {650, 722}, {1350, 1500} }
};

/* HT MCS parameters with Nss = 2 */
static struct index_data_rate_type mcs_nss2[] = {
	/* MCS L20  S20    L40   S40 */
	{0,  {130,  144},  {270,  300 } },
	{1,  {260,  289},  {540,  600 } },
	{2,  {390,  433},  {810,  900 } },
	{3,  {520,  578},  {1080, 1200} },
	{4,  {780,  867},  {1620, 1800} },
	{5,  {1040, 1156}, {2160, 2400} },
	{6,  {1170, 1300}, {2430, 2700} },
	{7,  {1300, 1440}, {2700, 3000} }
};

/* MCS Based VHT rate table */
/* MCS parameters with Nss = 1*/
static struct index_vht_data_rate_type vht_mcs_nss1[] = {
	/* MCS L20  S20    L40   S40    L80   S80 */
	{0,  {65,   72 }, {135,  150},  {293,  325} },
	{1,  {130,  144}, {270,  300},  {585,  650} },
	{2,  {195,  217}, {405,  450},  {878,  975} },
	{3,  {260,  289}, {540,  600},  {1170, 1300} },
	{4,  {390,  433}, {810,  900},  {1755, 1950} },
	{5,  {520,  578}, {1080, 1200}, {2340, 2600} },
	{6,  {585,  650}, {1215, 1350}, {2633, 2925} },
	{7,  {650,  722}, {1350, 1500}, {2925, 3250} },
	{8,  {780,  867}, {1620, 1800}, {3510, 3900} },
	{9,  {865,  960}, {1800, 2000}, {3900, 4333} }
};

/*MCS parameters with Nss = 2*/
static struct index_vht_data_rate_type vht_mcs_nss2[] = {
	/* MCS L20  S20    L40    S40    L80    S80 */
	{0,  {130,  144},  {270,  300},  { 585,  650} },
	{1,  {260,  289},  {540,  600},  {1170, 1300} },
	{2,  {390,  433},  {810,  900},  {1755, 1950} },
	{3,  {520,  578},  {1080, 1200}, {2340, 2600} },
	{4,  {780,  867},  {1620, 1800}, {3510, 3900} },
	{5,  {1040, 1156}, {2160, 2400}, {4680, 5200} },
	{6,  {1170, 1300}, {2430, 2700}, {5265, 5850} },
	{7,  {1300, 1444}, {2700, 3000}, {5850, 6500} },
	{8,  {1560, 1733}, {3240, 3600}, {7020, 7800} },
	{9,  {1730, 1920}, {3600, 4000}, {7800, 8667} }
};

#ifdef BIG_ENDIAN_HOST

/* ############# function definitions ############ */

/**
 * wma_swap_bytes() - swap bytes
 * @pv: buffer
 * @n: swap bytes
 *
 * Return: none
 */
void wma_swap_bytes(void *pv, uint32_t n)
{
	int32_t no_words;
	int32_t i;
	uint32_t *word_ptr;

	no_words = n / sizeof(uint32_t);
	word_ptr = (uint32_t *) pv;
	for (i = 0; i < no_words; i++)
		*(word_ptr + i) = __cpu_to_le32(*(word_ptr + i));
}

#define SWAPME(x, len) wma_swap_bytes(&x, len)
#endif /* BIG_ENDIAN_HOST */

/**
 * wma_mcs_rate_match() - find the match mcs rate
 * @match_rate:	the rate to look up
 * @is_sgi:	return if the SGI rate is found
 * @nss:	the nss in use
 * @nss1_rate:	the nss1 rate
 * @nss1_srate:	the nss1 SGI rate
 * @nss2_rate:	the nss2 rate
 * @nss2_srate:	the nss2 SGI rate
 *
 * This is a helper function to find the match of the tx_rate
 * in terms of the nss1/nss2 rate with non-SGI/SGI.
 *
 * Return: the found rate or 0 otherwise
 */
static inline uint16_t wma_mcs_rate_match(uint16_t match_rate, bool *is_sgi,
					  uint8_t nss, uint16_t nss1_rate,
					  uint16_t nss1_srate,
					  uint16_t nss2_rate,
					  uint16_t nss2_srate)
{
	WMA_LOGD("%s match_rate: %d, %d %d %d %d",
		__func__, match_rate, nss1_rate, nss1_srate, nss2_rate,
		nss2_srate);

	if (match_rate == nss1_rate) {
		return nss1_rate;
	} else if (match_rate == nss1_srate) {
		*is_sgi = true;
		return nss1_srate;
	} else if (nss == 2 && match_rate == nss2_rate)
		return nss2_rate;
	else if (nss == 2 && match_rate == nss2_srate) {
		*is_sgi = true;
		return nss2_srate;
	} else
		return 0;
}

/**
 * wma_get_mcs_idx() - get mcs index
 * @maxRate: max rate
 * @rate_flags: rate flags
 * @nss: number of nss
 * @mcsRateFlag: mcs rate flag
 *
 * Return: return mcs index
 */
static uint8_t wma_get_mcs_idx(uint16_t maxRate, uint8_t rate_flags,
			       uint8_t nss, uint8_t *mcsRateFlag)
{
	uint8_t  index = 0;
	uint16_t match_rate = 0;
	bool is_sgi = false;

	WMA_LOGD("%s rate:%d rate_flgs: 0x%x, nss: %d",
		 __func__, maxRate, rate_flags, nss);

	*mcsRateFlag = rate_flags;
	*mcsRateFlag &= ~eHAL_TX_RATE_SGI;
	for (index = 0; index < MAX_VHT_MCS_IDX; index++) {
		if (rate_flags & eHAL_TX_RATE_VHT80) {
			/* check for vht80 nss1/2 rate set */
			match_rate = wma_mcs_rate_match(maxRate, &is_sgi, nss,
					vht_mcs_nss1[index].ht80_rate[0],
					vht_mcs_nss1[index].ht80_rate[1],
					vht_mcs_nss2[index].ht80_rate[0],
					vht_mcs_nss2[index].ht80_rate[1]);
			if (match_rate)
				goto rate_found;
		}
		if ((rate_flags & eHAL_TX_RATE_VHT40) |
		    (rate_flags & eHAL_TX_RATE_VHT80)) {
			/* check for vht40 nss1/2 rate set */
			match_rate = wma_mcs_rate_match(maxRate, &is_sgi, nss,
					vht_mcs_nss1[index].ht40_rate[0],
					vht_mcs_nss1[index].ht40_rate[1],
					vht_mcs_nss2[index].ht40_rate[0],
					vht_mcs_nss2[index].ht40_rate[1]);
			if (match_rate) {
				*mcsRateFlag &= ~eHAL_TX_RATE_VHT80;
				goto rate_found;
			}
		}
		if ((rate_flags & eHAL_TX_RATE_VHT20) |
		    (rate_flags & eHAL_TX_RATE_VHT40) |
		    (rate_flags & eHAL_TX_RATE_VHT80)) {
			/* check for vht20 nss1/2 rate set */
			match_rate = wma_mcs_rate_match(maxRate, &is_sgi, nss,
					vht_mcs_nss1[index].ht20_rate[0],
					vht_mcs_nss1[index].ht20_rate[1],
					vht_mcs_nss2[index].ht20_rate[0],
					vht_mcs_nss2[index].ht20_rate[1]);
			if (match_rate) {
				*mcsRateFlag &= ~(eHAL_TX_RATE_VHT80 |
						eHAL_TX_RATE_VHT40);
				goto rate_found;
			}
		}
	}
	for (index = 0; index < MAX_HT_MCS_IDX; index++) {
		if (rate_flags & eHAL_TX_RATE_HT40) {
			/* check for ht40 nss1/2 rate set */
			match_rate = wma_mcs_rate_match(maxRate, &is_sgi, nss,
					mcs_nss1[index].ht40_rate[0],
					mcs_nss1[index].ht40_rate[1],
					mcs_nss2[index].ht40_rate[0],
					mcs_nss2[index].ht40_rate[1]);
			if (match_rate) {
				*mcsRateFlag = eHAL_TX_RATE_HT40;
				goto rate_found;
			}
		}
		if ((rate_flags & eHAL_TX_RATE_HT20) ||
			(rate_flags & eHAL_TX_RATE_HT40)) {
			/* check for ht20 nss1/2 rate set */
			match_rate = wma_mcs_rate_match(maxRate, &is_sgi, nss,
					mcs_nss1[index].ht20_rate[0],
					mcs_nss1[index].ht20_rate[1],
					mcs_nss2[index].ht20_rate[0],
					mcs_nss2[index].ht20_rate[1]);
			if (match_rate) {
				*mcsRateFlag = eHAL_TX_RATE_HT20;
				goto rate_found;
			}
		}
	}

rate_found:
	/* set SGI flag only if this is SGI rate */
	if (match_rate && is_sgi == true)
		*mcsRateFlag |= eHAL_TX_RATE_SGI;

	WMA_LOGD("%s - match_rate: %d index: %d rate_flag: 0x%x is_sgi: %d",
		 __func__, match_rate, index, *mcsRateFlag, is_sgi);

	return match_rate ? index : INVALID_MCS_IDX;
}

/**
 * wma_peek_vdev_req() - peek what request message is queued for response.
 *			 the function does not delete the node after found
 * @wma: WMA handle
 * @vdev_id: vdev ID
 * @type: request message type
 *
 * Return: the request message found
 */
static struct wma_target_req *wma_peek_vdev_req(tp_wma_handle wma,
						uint8_t vdev_id, uint8_t type)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	qdf_list_node_t *node1 = NULL, *node2 = NULL;

	qdf_spin_lock_bh(&wma->vdev_respq_lock);
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->vdev_resp_queue,
							&node2)) {
		qdf_spin_unlock_bh(&wma->vdev_respq_lock);
		return NULL;
	}

	do {
		node1 = node2;
		req_msg = qdf_container_of(node1, struct wma_target_req, node);
		if (req_msg->vdev_id != vdev_id)
			continue;
		if (req_msg->type != type)
			continue;

		found = true;
		break;
	} while (QDF_STATUS_SUCCESS == qdf_list_peek_next(&wma->vdev_resp_queue,
							  node1, &node2));
	qdf_spin_unlock_bh(&wma->vdev_respq_lock);
	if (!found) {
		WMA_LOGE(FL("target request not found for vdev_id %d type %d"),
			 vdev_id, type);
		return NULL;
	}
	WMA_LOGD(FL("target request found for vdev id: %d type %d msg %d"),
		 vdev_id, type, req_msg->msg_type);
	return req_msg;
}

void wma_lost_link_info_handler(tp_wma_handle wma, uint32_t vdev_id,
					int32_t rssi)
{
	struct sir_lost_link_info *lost_link_info;
	QDF_STATUS qdf_status;
	struct scheduler_msg sme_msg = {0};

	/* report lost link information only for STA mode */
	if (wma_is_vdev_up(vdev_id) &&
	    (WMI_VDEV_TYPE_STA == wma->interfaces[vdev_id].type) &&
	    (0 == wma->interfaces[vdev_id].sub_type)) {
		lost_link_info = qdf_mem_malloc(sizeof(*lost_link_info));
		if (NULL == lost_link_info) {
			WMA_LOGE("%s: failed to allocate memory", __func__);
			return;
		}
		lost_link_info->vdev_id = vdev_id;
		lost_link_info->rssi = rssi;
		sme_msg.type = eWNI_SME_LOST_LINK_INFO_IND;
		sme_msg.bodyptr = lost_link_info;
		sme_msg.bodyval = 0;
		WMA_LOGD("%s: post msg to SME, bss_idx %d, rssi %d",  __func__,
			 lost_link_info->vdev_id, lost_link_info->rssi);

		qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			WMA_LOGE("%s: fail to post msg to SME", __func__);
			qdf_mem_free(lost_link_info);
		}
	}
}

/**
 * host_map_smps_mode() - map fw smps mode to enum eSmpsModeValue
 * @fw_smps_mode: fw smps mode
 *
 * Return: return enum eSmpsModeValue
 */
enum eSmpsModeValue host_map_smps_mode(A_UINT32 fw_smps_mode)
{
	enum eSmpsModeValue smps_mode = SMPS_MODE_DISABLED;

	switch (fw_smps_mode) {
	case WMI_SMPS_FORCED_MODE_STATIC:
		smps_mode = STATIC_SMPS_MODE;
		break;
	case WMI_SMPS_FORCED_MODE_DYNAMIC:
		smps_mode = DYNAMIC_SMPS_MODE;
		break;
	default:
		smps_mode = SMPS_MODE_DISABLED;
	}

	return smps_mode;
}

/**
 * wma_smps_mode_to_force_mode_param() - Map smps mode to force
 * mode commmand param
 * @smps_mode: SMPS mode according to the protocol
 *
 * Return: int > 0 for success else failure
 */
int wma_smps_mode_to_force_mode_param(uint8_t smps_mode)
{
	int param = -EINVAL;

	switch (smps_mode) {
	case STATIC_SMPS_MODE:
		param = WMI_SMPS_FORCED_MODE_STATIC;
		break;
	case DYNAMIC_SMPS_MODE:
		param = WMI_SMPS_FORCED_MODE_DYNAMIC;
		break;
	case SMPS_MODE_DISABLED:
		param = WMI_SMPS_FORCED_MODE_DISABLED;
		break;
	default:
		WMA_LOGE(FL("smps mode cannot be mapped :%d "),
			 smps_mode);
	}
	return param;
}

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * wma_stats_ext_event_handler() - extended stats event handler
 * @handle:     wma handle
 * @event_buf:  event buffer received from fw
 * @len:        length of data
 *
 * Return: 0 for success or error code
 */
int wma_stats_ext_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	WMI_STATS_EXT_EVENTID_param_tlvs *param_buf;
	tSirStatsExtEvent *stats_ext_event;
	wmi_stats_ext_event_fixed_param *stats_ext_info;
	QDF_STATUS status;
	struct scheduler_msg cds_msg = {0};
	uint8_t *buf_ptr;
	uint32_t alloc_len;

	WMA_LOGD("%s: Posting stats ext event to SME", __func__);

	param_buf = (WMI_STATS_EXT_EVENTID_param_tlvs *) event_buf;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid stats ext event buf", __func__);
		return -EINVAL;
	}

	stats_ext_info = param_buf->fixed_param;
	buf_ptr = (uint8_t *) stats_ext_info;

	alloc_len = sizeof(tSirStatsExtEvent);
	alloc_len += stats_ext_info->data_len;

	stats_ext_event = (tSirStatsExtEvent *) qdf_mem_malloc(alloc_len);
	if (NULL == stats_ext_event) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return -ENOMEM;
	}

	buf_ptr += sizeof(wmi_stats_ext_event_fixed_param) + WMI_TLV_HDR_SIZE;

	stats_ext_event->vdev_id = stats_ext_info->vdev_id;
	stats_ext_event->event_data_len = stats_ext_info->data_len;
	qdf_mem_copy(stats_ext_event->event_data,
		     buf_ptr, stats_ext_event->event_data_len);

	cds_msg.type = eWNI_SME_STATS_EXT_EVENT;
	cds_msg.bodyptr = (void *)stats_ext_event;
	cds_msg.bodyval = 0;

	status = scheduler_post_msg(QDF_MODULE_ID_SME, &cds_msg);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to post stats ext event to SME", __func__);
		qdf_mem_free(stats_ext_event);
		return -EFAULT;
	}

	WMA_LOGD("%s: stats ext event Posted to SME", __func__);
	return 0;
}
#endif /* WLAN_FEATURE_STATS_EXT */


/**
 * wma_profile_data_report_event_handler() - fw profiling handler
 * @handle:     wma handle
 * @event_buf:  event buffer received from fw
 * @len:        length of data
 *
 * Return: 0 for success or error code
 */
int wma_profile_data_report_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len)
{
	WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *param_buf;
	wmi_wlan_profile_ctx_t *profile_ctx;
	wmi_wlan_profile_t *profile_data;
	uint32_t i = 0;
	uint32_t entries;
	uint8_t *buf_ptr;
	char temp_str[150];

	param_buf = (WMI_WLAN_PROFILE_DATA_EVENTID_param_tlvs *) event_buf;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid profile data event buf", __func__);
		return -EINVAL;
	}
	profile_ctx = param_buf->profile_ctx;
	buf_ptr = (uint8_t *)profile_ctx;
	buf_ptr = buf_ptr + sizeof(wmi_wlan_profile_ctx_t) + WMI_TLV_HDR_SIZE;
	profile_data = (wmi_wlan_profile_t *) buf_ptr;
	entries = profile_ctx->bin_count;
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
				"Profile data stats\n");
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
		"TOT: %d\n"
		"tx_msdu_cnt: %d\n"
		"tx_mpdu_cnt: %d\n"
		"tx_ppdu_cnt: %d\n"
		"rx_msdu_cnt: %d\n"
		"rx_mpdu_cnt: %d\n"
		"bin_count: %d\n",
		profile_ctx->tot,
		profile_ctx->tx_msdu_cnt,
		profile_ctx->tx_mpdu_cnt,
		profile_ctx->tx_ppdu_cnt,
		profile_ctx->rx_msdu_cnt,
		profile_ctx->rx_mpdu_cnt,
		profile_ctx->bin_count);

	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
		  "Profile ID: Count: TOT: Min: Max: hist_intvl: hist[0]: hist[1]:hist[2]");

	for (i = 0; i < entries; i++) {
		if (i == WMI_WLAN_PROFILE_MAX_BIN_CNT)
			break;
		snprintf(temp_str, sizeof(temp_str),
			 " %d : %d : %d : %d : %d : %d : %d : %d : %d",
			profile_data[i].id,
			profile_data[i].cnt,
			profile_data[i].tot,
			profile_data[i].min,
			profile_data[i].max,
			profile_data[i].hist_intvl,
			profile_data[i].hist[0],
			profile_data[i].hist[1],
			profile_data[i].hist[2]);
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR,
			"%s", temp_str);
	}

	return 0;
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

#define WMA_FILL_TX_STATS(eve, msg)   do {\
	(msg)->msdus = (eve)->tx_msdu_cnt;\
	(msg)->mpdus = (eve)->tx_mpdu_cnt;\
	(msg)->ppdus = (eve)->tx_ppdu_cnt;\
	(msg)->bytes = (eve)->tx_bytes;\
	(msg)->drops = (eve)->tx_msdu_drop_cnt;\
	(msg)->drop_bytes = (eve)->tx_drop_bytes;\
	(msg)->retries = (eve)->tx_mpdu_retry_cnt;\
	(msg)->failed = (eve)->tx_mpdu_fail_cnt;\
} while (0)

#define WMA_FILL_RX_STATS(eve, msg)       do {\
	(msg)->mpdus = (eve)->mac_rx_mpdu_cnt;\
	(msg)->bytes = (eve)->mac_rx_bytes;\
	(msg)->ppdus = (eve)->phy_rx_ppdu_cnt;\
	(msg)->ppdu_bytes = (eve)->phy_rx_bytes;\
	(msg)->mpdu_retry = (eve)->rx_mpdu_retry_cnt;\
	(msg)->mpdu_dup = (eve)->rx_mpdu_dup_cnt;\
	(msg)->mpdu_discard = (eve)->rx_mpdu_discard_cnt;\
} while (0)

/**
 * wma_get_ll_stats_ext_buf() - alloc result buffer for MAC counters
 * @len: buffer length output
 * @peer_num: peer number
 * @fixed_param: fixed parameters in WMI event
 *
 * Structure of the stats message
 * LL_EXT_STATS
 *  |
 *  |--Channel stats[1~n]
 *  |--Peer[1~n]
 *      |
 *      +---Signal
 *      +---TX
 *      |    +---BE
 *      |    +---BK
 *      |    +---VI
 *      |    +---VO
 *      |
 *      +---RX
 *           +---BE
 *           +---BK
 *           +---VI
 *           +---VO
 * For each Access Category, the arregation and mcs
 * stats are as this:
 *  TX
 *   +-BE/BK/VI/VO
 *         +----tx_mpdu_aggr_array
 *         +----tx_succ_mcs_array
 *         +----tx_fail_mcs_array
 *         +----tx_delay_array
 *  RX
 *   +-BE/BK/VI/VO
 *         +----rx_mpdu_aggr_array
 *         +----rx_mcs_array
 *
 * return: Address for result buffer.
 */
static tSirLLStatsResults *wma_get_ll_stats_ext_buf(uint32_t *len,
						    uint32_t peer_num,
			wmi_report_stats_event_fixed_param *fixed_param)
{
	tSirLLStatsResults *buf;
	uint32_t buf_len;

	if (!len || !fixed_param) {
		WMA_LOGE(FL("Invalid input parameters."));
		return NULL;
	}

	/*
	 * Result buffer has a structure like this:
	 *     ---------------------------------
	 *     |      trigger_cond_i           |
	 *     +-------------------------------+
	 *     |      cca_chgd_bitmap          |
	 *     +-------------------------------+
	 *     |      sig_chgd_bitmap          |
	 *     +-------------------------------+
	 *     |      tx_chgd_bitmap           |
	 *     +-------------------------------+
	 *     |      rx_chgd_bitmap           |
	 *     +-------------------------------+
	 *     |      peer_num                 |
	 *     +-------------------------------+
	 *     |      channel_num              |
	 *     +-------------------------------+
	 *     |      tx_mpdu_aggr_array_len   |
	 *     +-------------------------------+
	 *     |      tx_succ_mcs_array_len    |
	 *     +-------------------------------+
	 *     |      tx_fail_mcs_array_len    |
	 *     +-------------------------------+
	 *     |      tx_delay_array_len       |
	 *     +-------------------------------+
	 *     |      rx_mpdu_aggr_array_len   |
	 *     +-------------------------------+
	 *     |      rx_mcs_array_len         |
	 *     +-------------------------------+
	 *     |      pointer to CCA stats     |
	 *     +-------------------------------+
	 *     |      CCA stats                |
	 *     +-------------------------------+
	 *     |      peer_stats               |----+
	 *     +-------------------------------+    |
	 *     | TX aggr/mcs parameters array  |    |
	 *     | Length of this buffer is      |    |
	 *     | not fixed.                    |<-+ |
	 *     +-------------------------------+  | |
	 *     |      per peer tx stats        |--+ |
	 *     |         BE                    | <--+
	 *     |         BK                    |    |
	 *     |         VI                    |    |
	 *     |         VO                    |    |
	 *     +-------------------------------+    |
	 *     | TX aggr/mcs parameters array  |    |
	 *     | Length of this buffer is      |    |
	 *     | not fixed.                    |<-+ |
	 *     +-------------------------------+  | |
	 *     |      peer peer rx stats       |--+ |
	 *     |         BE                    | <--+
	 *     |         BK                    |
	 *     |         VI                    |
	 *     |         VO                    |
	 *     ---------------------------------
	 */
	buf_len = sizeof(tSirLLStatsResults) +
		  sizeof(struct sir_wifi_ll_ext_stats) +
		  fixed_param->num_chan_cca_stats *
		  sizeof(struct sir_wifi_chan_cca_stats) +
		  peer_num *
		  (sizeof(struct sir_wifi_ll_ext_peer_stats) +
		   WLAN_MAX_AC *
		   (sizeof(struct sir_wifi_tx) +
		    sizeof(struct sir_wifi_rx)) +
		    sizeof(uint32_t) * WLAN_MAX_AC *
		    (fixed_param->tx_mpdu_aggr_array_len +
		     fixed_param->tx_succ_mcs_array_len +
		     fixed_param->tx_fail_mcs_array_len +
		     fixed_param->tx_ppdu_delay_array_len +
		     fixed_param->rx_mpdu_aggr_array_len +
		     fixed_param->rx_mcs_array_len));

	buf = (tSirLLStatsResults *)qdf_mem_malloc(buf_len);
	if (buf == NULL) {
		WMA_LOGE("%s: Cannot allocate link layer stats.", __func__);
		buf_len = 0;
		return NULL;
	}

	*len = buf_len;
	return buf;
}

/**
 * wma_fill_tx_stats() - Fix TX stats into result buffer
 * @ll_stats: LL stats buffer
 * @fix_param: parameters with fixed length in WMI event
 * @param_buf: parameters without fixed length in WMI event
 * @buf: buffer for TLV parameters
 *
 * Return: None
 */
static void wma_fill_tx_stats(struct sir_wifi_ll_ext_stats *ll_stats,
			      wmi_report_stats_event_fixed_param *fix_param,
			      WMI_REPORT_STATS_EVENTID_param_tlvs *param_buf,
			      uint8_t **buf, uint32_t *buf_length)
{
	uint8_t *result;
	uint32_t i, j, k;
	wmi_peer_ac_tx_stats *wmi_peer_tx;
	wmi_tx_stats *wmi_tx;
	struct sir_wifi_tx *tx_stats;
	struct sir_wifi_ll_ext_peer_stats *peer_stats;
	uint32_t *tx_mpdu_aggr, *tx_succ_mcs, *tx_fail_mcs, *tx_delay;
	uint32_t len, dst_len, tx_mpdu_aggr_array_len, tx_succ_mcs_array_len,
		 tx_fail_mcs_array_len, tx_delay_array_len;

	result = *buf;
	dst_len = *buf_length;
	tx_mpdu_aggr_array_len = fix_param->tx_mpdu_aggr_array_len;
	ll_stats->tx_mpdu_aggr_array_len = tx_mpdu_aggr_array_len;
	tx_succ_mcs_array_len = fix_param->tx_succ_mcs_array_len;
	ll_stats->tx_succ_mcs_array_len = tx_succ_mcs_array_len;
	tx_fail_mcs_array_len = fix_param->tx_fail_mcs_array_len;
	ll_stats->tx_fail_mcs_array_len = tx_fail_mcs_array_len;
	tx_delay_array_len = fix_param->tx_ppdu_delay_array_len;
	ll_stats->tx_delay_array_len = tx_delay_array_len;
	wmi_peer_tx = param_buf->peer_ac_tx_stats;
	wmi_tx = param_buf->tx_stats;

	len = fix_param->num_peer_ac_tx_stats *
		WLAN_MAX_AC * tx_mpdu_aggr_array_len * sizeof(uint32_t);
	if (len <= dst_len) {
		tx_mpdu_aggr = (uint32_t *)result;
		qdf_mem_copy(tx_mpdu_aggr, param_buf->tx_mpdu_aggr, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("TX_MPDU_AGGR buffer length is wrong."));
		tx_mpdu_aggr = NULL;
	}

	len = fix_param->num_peer_ac_tx_stats * WLAN_MAX_AC *
		tx_succ_mcs_array_len * sizeof(uint32_t);
	if (len <= dst_len) {
		tx_succ_mcs = (uint32_t *)result;
		qdf_mem_copy(tx_succ_mcs, param_buf->tx_succ_mcs, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("TX_SUCC_MCS buffer length is wrong."));
		tx_succ_mcs = NULL;
	}

	len = fix_param->num_peer_ac_tx_stats * WLAN_MAX_AC *
		tx_fail_mcs_array_len * sizeof(uint32_t);
	if (len <= dst_len) {
		tx_fail_mcs = (uint32_t *)result;
		qdf_mem_copy(tx_fail_mcs, param_buf->tx_fail_mcs, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("TX_FAIL_MCS buffer length is wrong."));
		tx_fail_mcs = NULL;
	}

	len = fix_param->num_peer_ac_tx_stats *
		WLAN_MAX_AC * tx_delay_array_len * sizeof(uint32_t);
	if (len <= dst_len) {
		tx_delay = (uint32_t *)result;
		qdf_mem_copy(tx_delay, param_buf->tx_ppdu_delay, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("TX_DELAY buffer length is wrong."));
		tx_delay = NULL;
	}

	/* per peer tx stats */
	peer_stats = ll_stats->peer_stats;

	for (i = 0; i < fix_param->num_peer_ac_tx_stats; i++) {
		uint32_t peer_id = wmi_peer_tx[i].peer_id;
		struct sir_wifi_tx *ac;
		wmi_tx_stats *wmi_tx_stats;

		for (j = 0; j < ll_stats->peer_num; j++) {
			peer_stats += j;
			if (peer_stats->peer_id == WIFI_INVALID_PEER_ID ||
			    peer_stats->peer_id == peer_id)
				break;
		}

		if (j < ll_stats->peer_num) {
			peer_stats->peer_id = wmi_peer_tx[i].peer_id;
			peer_stats->vdev_id = wmi_peer_tx[i].vdev_id;
			tx_stats = (struct sir_wifi_tx *)result;
			for (k = 0; k < WLAN_MAX_AC; k++) {
				wmi_tx_stats = &wmi_tx[i * WLAN_MAX_AC + k];
				ac = &tx_stats[k];
				WMA_FILL_TX_STATS(wmi_tx_stats, ac);
				ac->mpdu_aggr_size = tx_mpdu_aggr;
				ac->aggr_len = tx_mpdu_aggr_array_len *
							sizeof(uint32_t);
				ac->success_mcs_len = tx_succ_mcs_array_len *
							sizeof(uint32_t);
				ac->success_mcs = tx_succ_mcs;
				ac->fail_mcs = tx_fail_mcs;
				ac->fail_mcs_len = tx_fail_mcs_array_len *
							sizeof(uint32_t);
				ac->delay = tx_delay;
				ac->delay_len = tx_delay_array_len *
							sizeof(uint32_t);
				peer_stats->ac_stats[k].tx_stats = ac;
				peer_stats->ac_stats[k].type = k;
				tx_mpdu_aggr += tx_mpdu_aggr_array_len;
				tx_succ_mcs += tx_succ_mcs_array_len;
				tx_fail_mcs += tx_fail_mcs_array_len;
				tx_delay += tx_delay_array_len;
			}
			result += WLAN_MAX_AC * sizeof(struct sir_wifi_tx);
		} else {
			/*
			 * Buffer for Peer TX counter overflow.
			 * There is peer ID mismatch between TX, RX,
			 * signal counters.
			 */
			WMA_LOGE(FL("One peer TX info is dropped."));

			tx_mpdu_aggr += tx_mpdu_aggr_array_len * WLAN_MAX_AC;
			tx_succ_mcs += tx_succ_mcs_array_len * WLAN_MAX_AC;
			tx_fail_mcs += tx_fail_mcs_array_len * WLAN_MAX_AC;
			tx_delay += tx_delay_array_len * WLAN_MAX_AC;
		}
	}
	*buf = result;
	*buf_length = dst_len;
}

/**
 * wma_fill_rx_stats() - Fix RX stats into result buffer
 * @ll_stats: LL stats buffer
 * @fix_param: parameters with fixed length in WMI event
 * @param_buf: parameters without fixed length in WMI event
 * @buf: buffer for TLV parameters
 *
 * Return: None
 */
static void wma_fill_rx_stats(struct sir_wifi_ll_ext_stats *ll_stats,
			      wmi_report_stats_event_fixed_param *fix_param,
			      WMI_REPORT_STATS_EVENTID_param_tlvs *param_buf,
			      uint8_t **buf, uint32_t *buf_length)
{
	uint8_t *result;
	uint32_t i, j, k;
	uint32_t *rx_mpdu_aggr, *rx_mcs;
	wmi_rx_stats *wmi_rx;
	wmi_peer_ac_rx_stats *wmi_peer_rx;
	struct sir_wifi_rx *rx_stats;
	struct sir_wifi_ll_ext_peer_stats *peer_stats;
	uint32_t len, dst_len, rx_mpdu_aggr_array_len, rx_mcs_array_len;

	rx_mpdu_aggr_array_len = fix_param->rx_mpdu_aggr_array_len;
	ll_stats->rx_mpdu_aggr_array_len = rx_mpdu_aggr_array_len;
	rx_mcs_array_len = fix_param->rx_mcs_array_len;
	ll_stats->rx_mcs_array_len = rx_mcs_array_len;
	wmi_peer_rx = param_buf->peer_ac_rx_stats;
	wmi_rx = param_buf->rx_stats;

	result = *buf;
	dst_len = *buf_length;
	len = sizeof(uint32_t) * (fix_param->num_peer_ac_rx_stats *
				  WLAN_MAX_AC * rx_mpdu_aggr_array_len);
	if (len <= dst_len) {
		rx_mpdu_aggr = (uint32_t *)result;
		qdf_mem_copy(rx_mpdu_aggr, param_buf->rx_mpdu_aggr, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("RX_MPDU_AGGR array length is wrong."));
		rx_mpdu_aggr = NULL;
	}

	len = sizeof(uint32_t) * (fix_param->num_peer_ac_rx_stats *
				  WLAN_MAX_AC * rx_mcs_array_len);
	if (len <= dst_len) {
		rx_mcs = (uint32_t *)result;
		qdf_mem_copy(rx_mcs, param_buf->rx_mcs, len);
		result += len;
		dst_len -= len;
	} else {
		WMA_LOGE(FL("RX_MCS array length is wrong."));
		rx_mcs = NULL;
	}

	/* per peer rx stats */
	peer_stats = ll_stats->peer_stats;
	for (i = 0; i < fix_param->num_peer_ac_rx_stats; i++) {
		uint32_t peer_id = wmi_peer_rx[i].peer_id;
		struct sir_wifi_rx *ac;
		wmi_rx_stats *wmi_rx_stats;

		for (j = 0; j < ll_stats->peer_num; j++) {
			peer_stats += j;
			if ((peer_stats->peer_id == WIFI_INVALID_PEER_ID) ||
			    (peer_stats->peer_id == peer_id))
				break;
		}

		if (j < ll_stats->peer_num) {
			peer_stats->peer_id = wmi_peer_rx[i].peer_id;
			peer_stats->vdev_id = wmi_peer_rx[i].vdev_id;
			peer_stats->sta_ps_inds = wmi_peer_rx[i].sta_ps_inds;
			peer_stats->sta_ps_durs = wmi_peer_rx[i].sta_ps_durs;
			peer_stats->rx_probe_reqs =
						wmi_peer_rx[i].rx_probe_reqs;
			peer_stats->rx_oth_mgmts = wmi_peer_rx[i].rx_oth_mgmts;
			rx_stats = (struct sir_wifi_rx *)result;

			for (k = 0; k < WLAN_MAX_AC; k++) {
				wmi_rx_stats = &wmi_rx[i * WLAN_MAX_AC + k];
				ac = &rx_stats[k];
				WMA_FILL_RX_STATS(wmi_rx_stats, ac);
				ac->mpdu_aggr = rx_mpdu_aggr;
				ac->aggr_len = rx_mpdu_aggr_array_len *
							sizeof(uint32_t);
				ac->mcs = rx_mcs;
				ac->mcs_len = rx_mcs_array_len *
							sizeof(uint32_t);
				peer_stats->ac_stats[k].rx_stats = ac;
				peer_stats->ac_stats[k].type = k;
				rx_mpdu_aggr += rx_mpdu_aggr_array_len;
				rx_mcs += rx_mcs_array_len;
			}
			result += WLAN_MAX_AC * sizeof(struct sir_wifi_rx);
		} else {
			/*
			 * Buffer for Peer RX counter overflow.
			 * There is peer ID mismatch between TX, RX,
			 * signal counters.
			 */
			WMA_LOGE(FL("One peer RX info is dropped."));
			rx_mpdu_aggr += rx_mpdu_aggr_array_len * WLAN_MAX_AC;
			rx_mcs += rx_mcs_array_len * WLAN_MAX_AC;
		}
	}
	*buf = result;
	*buf_length = dst_len;
}

/**
 * wma_ll_stats_evt_handler() - handler for MAC layer counters.
 * @handle - wma handle
 * @event - FW event
 * @len - length of FW event
 *
 * return: 0 success.
 */
static int wma_ll_stats_evt_handler(void *handle, u_int8_t *event,
				    u_int32_t len)
{
	WMI_REPORT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_report_stats_event_fixed_param *fixed_param;
	tSirLLStatsResults *link_stats_results;
	wmi_chan_cca_stats *wmi_cca_stats;
	wmi_peer_signal_stats *wmi_peer_signal;
	wmi_peer_ac_rx_stats *wmi_peer_rx;
	struct sir_wifi_ll_ext_stats *ll_stats;
	struct sir_wifi_ll_ext_peer_stats *peer_stats;
	struct sir_wifi_chan_cca_stats *cca_stats;
	struct sir_wifi_peer_signal_stats *peer_signal;
	uint8_t *result;
	uint32_t i, peer_num, result_size, dst_len;
	tpAniSirGlobal mac;
	struct scheduler_msg sme_msg = { 0 };
	QDF_STATUS qdf_status;

	mac = (tpAniSirGlobal)cds_get_context(QDF_MODULE_ID_PE);
	if (!mac) {
		WMA_LOGD("%s: NULL mac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!mac->sme.link_layer_stats_ext_cb) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: Posting MAC counters event to HDD", __func__);

	param_buf = (WMI_REPORT_STATS_EVENTID_param_tlvs *)event;
	fixed_param = param_buf->fixed_param;
	wmi_cca_stats = param_buf->chan_cca_stats;
	wmi_peer_signal = param_buf->peer_signal_stats;
	wmi_peer_rx = param_buf->peer_ac_rx_stats;

	/* Get the MAX of three peer numbers */
	peer_num = fixed_param->num_peer_signal_stats >
			fixed_param->num_peer_ac_tx_stats ?
			fixed_param->num_peer_signal_stats :
			fixed_param->num_peer_ac_tx_stats;
	peer_num = peer_num > fixed_param->num_peer_ac_rx_stats ?
			peer_num : fixed_param->num_peer_ac_rx_stats;

	if (peer_num == 0)
		return -EINVAL;

	link_stats_results = wma_get_ll_stats_ext_buf(&result_size,
						      peer_num,
						      fixed_param);
	if (!link_stats_results) {
		WMA_LOGE("%s: Fail to allocate stats buffer", __func__);
		return -EINVAL;
	}
	link_stats_results->paramId = WMI_LL_STATS_EXT_MAC_COUNTER;
	link_stats_results->num_peers = peer_num;
	link_stats_results->peer_event_number = 1;
	link_stats_results->moreResultToFollow = 0;

	ll_stats = (struct sir_wifi_ll_ext_stats *)link_stats_results->results;
	ll_stats->trigger_cond_id = fixed_param->trigger_cond_id;
	ll_stats->cca_chgd_bitmap = fixed_param->cca_chgd_bitmap;
	ll_stats->sig_chgd_bitmap = fixed_param->sig_chgd_bitmap;
	ll_stats->tx_chgd_bitmap = fixed_param->tx_chgd_bitmap;
	ll_stats->rx_chgd_bitmap = fixed_param->rx_chgd_bitmap;
	ll_stats->channel_num = fixed_param->num_chan_cca_stats;
	ll_stats->peer_num = peer_num;

	result = (uint8_t *)ll_stats->stats;
	peer_stats = (struct sir_wifi_ll_ext_peer_stats *)result;
	ll_stats->peer_stats = peer_stats;

	for (i = 0; i < peer_num; i++) {
		peer_stats[i].peer_id = WIFI_INVALID_PEER_ID;
		peer_stats[i].vdev_id = WIFI_INVALID_VDEV_ID;
	}

	/* Per peer signal */
	result_size -= sizeof(struct sir_wifi_ll_ext_stats);
	dst_len = sizeof(struct sir_wifi_peer_signal_stats);
	for (i = 0; i < fixed_param->num_peer_signal_stats; i++) {
		peer_stats[i].peer_id = wmi_peer_signal->peer_id;
		peer_stats[i].vdev_id = wmi_peer_signal->vdev_id;
		peer_signal = &peer_stats[i].peer_signal_stats;

		if (dst_len <= result_size) {
			qdf_mem_copy(peer_signal,
				     &wmi_peer_signal->vdev_id, dst_len);
			result_size -= dst_len;
		} else {
			WMA_LOGE(FL("Invalid length of PEER signal."));
		}
		wmi_peer_signal++;
	}

	result += peer_num * sizeof(struct sir_wifi_ll_ext_peer_stats);
	cca_stats = (struct sir_wifi_chan_cca_stats *)result;
	ll_stats->cca = cca_stats;
	dst_len = sizeof(struct sir_wifi_chan_cca_stats);
	for (i = 0; i < ll_stats->channel_num; i++) {
		if (dst_len <= result_size) {
			qdf_mem_copy(&cca_stats[i], &wmi_cca_stats->vdev_id,
				     dst_len);
			result_size -= dst_len;
		} else {
			WMA_LOGE(FL("Invalid length of CCA."));
		}
	}

	result += i * sizeof(struct sir_wifi_chan_cca_stats);
	wma_fill_tx_stats(ll_stats, fixed_param, param_buf,
			  &result, &result_size);
	wma_fill_rx_stats(ll_stats, fixed_param, param_buf,
			  &result, &result_size);
	sme_msg.type = eWMI_SME_LL_STATS_IND;
	sme_msg.bodyptr = (void *)link_stats_results;
	sme_msg.bodyval = 0;
	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGP(FL("Failed to post peer stat change msg!"));
		qdf_mem_free(link_stats_results);
		return -EINVAL;
	}

	return 0;
}

/**
 * wma_unified_link_peer_stats_event_handler() - peer stats event handler
 * @handle:          wma handle
 * @cmd_param_info:  data received with event from fw
 * @len:             length of data
 *
 * Return: 0 for success or error code
 */
static int wma_unified_link_peer_stats_event_handler(void *handle,
						     uint8_t *cmd_param_info,
						     uint32_t len)
{
	WMI_PEER_LINK_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_peer_stats_event_fixed_param *fixed_param;
	wmi_peer_link_stats *peer_stats, *temp_peer_stats;
	wmi_rate_stats *rate_stats;
	tSirLLStatsResults *link_stats_results;
	uint8_t *results, *t_peer_stats, *t_rate_stats;
	uint32_t count, num_rates = 0, rate_cnt;
	uint32_t next_res_offset, next_peer_offset, next_rate_offset;
	size_t peer_info_size, peer_stats_size, rate_stats_size;
	size_t link_stats_results_size;

	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_PEER_LINK_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_tlvs) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}
	/*
	 * cmd_param_info contains
	 * wmi_peer_stats_event_fixed_param fixed_param;
	 * num_peers * size of(struct wmi_peer_link_stats)
	 * num_rates * size of(struct wmi_rate_stats)
	 * num_rates is the sum of the rates of all the peers.
	 */
	fixed_param = param_tlvs->fixed_param;
	peer_stats = param_tlvs->peer_stats;
	rate_stats = param_tlvs->peer_rate_stats;

	if (!fixed_param || !peer_stats ||
	    (peer_stats->num_rates && !rate_stats)) {
		WMA_LOGA("%s: Invalid param_tlvs for Peer Stats", __func__);
		return -EINVAL;
	}

	/*
	 * num_rates - sum of the rates of all the peers
	 */
	temp_peer_stats = (wmi_peer_link_stats *) peer_stats;
	for (count = 0; count < fixed_param->num_peers; count++) {
		num_rates += temp_peer_stats->num_rates;
		temp_peer_stats++;
	}

	peer_stats_size = sizeof(tSirWifiPeerStat);
	peer_info_size = sizeof(tSirWifiPeerInfo);
	rate_stats_size = sizeof(tSirWifiRateStat);
	link_stats_results_size =
		sizeof(*link_stats_results) + peer_stats_size +
		(fixed_param->num_peers * peer_info_size) +
		(num_rates * rate_stats_size);

	link_stats_results = qdf_mem_malloc(link_stats_results_size);
	if (NULL == link_stats_results) {
		WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
			 __func__, link_stats_results_size);
		return -ENOMEM;
	}

	qdf_mem_zero(link_stats_results, link_stats_results_size);

	link_stats_results->paramId = WMI_LINK_STATS_ALL_PEER;
	link_stats_results->rspId = fixed_param->request_id;
	link_stats_results->ifaceId = 0;
	link_stats_results->num_peers = fixed_param->num_peers;
	link_stats_results->peer_event_number = fixed_param->peer_event_number;
	link_stats_results->moreResultToFollow = fixed_param->more_data;

	qdf_mem_copy(link_stats_results->results,
		     &fixed_param->num_peers, peer_stats_size);

	results = (uint8_t *) link_stats_results->results;
	t_peer_stats = (uint8_t *) peer_stats;
	t_rate_stats = (uint8_t *) rate_stats;
	next_res_offset = peer_stats_size;
	next_peer_offset = WMI_TLV_HDR_SIZE;
	next_rate_offset = WMI_TLV_HDR_SIZE;
	for (rate_cnt = 0; rate_cnt < fixed_param->num_peers; rate_cnt++) {
		qdf_mem_copy(results + next_res_offset,
			     t_peer_stats + next_peer_offset, peer_info_size);
		next_res_offset += peer_info_size;

		/* Copy rate stats associated with this peer */
		for (count = 0; count < peer_stats->num_rates; count++) {
			rate_stats++;

			qdf_mem_copy(results + next_res_offset,
				     t_rate_stats + next_rate_offset,
				     rate_stats_size);
			next_res_offset += rate_stats_size;
			next_rate_offset += sizeof(*rate_stats);
		}
		next_peer_offset += sizeof(*peer_stats);
		peer_stats++;
	}

	/* call hdd callback with Link Layer Statistics
	 * vdev_id/ifacId in link_stats_results will be
	 * used to retrieve the correct HDD context
	 */
	pMac->sme.pLinkLayerStatsIndCallback(pMac->hHdd,
					     WMA_LINK_LAYER_STATS_RESULTS_RSP,
					     link_stats_results);
	qdf_mem_free(link_stats_results);

	return 0;
}

/**
 * wma_unified_radio_tx_mem_free() - Free radio tx power stats memory
 * @handle: WMI handle
 *
 * Return: 0 on success, error number otherwise.
 */
static int wma_unified_radio_tx_mem_free(void *handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	tSirWifiRadioStat *rs_results;
	uint32_t i = 0;

	if (!wma_handle->link_stats_results)
		return 0;

	rs_results = (tSirWifiRadioStat *)
				&wma_handle->link_stats_results->results[0];
	for (i = 0; i < wma_handle->link_stats_results->num_radio; i++) {
		rs_results += i;
		if (rs_results->tx_time_per_power_level) {
			qdf_mem_free(rs_results->tx_time_per_power_level);
			rs_results->tx_time_per_power_level = NULL;
		}

		if (rs_results->channels) {
			qdf_mem_free(rs_results->channels);
			rs_results->channels = NULL;
		}
	}

	qdf_mem_free(wma_handle->link_stats_results);
	wma_handle->link_stats_results = NULL;

	return 0;
}

/**
 * wma_unified_radio_tx_power_level_stats_event_handler() - tx power level stats
 * @handle: WMI handle
 * @cmd_param_info: command param info
 * @len: Length of @cmd_param_info
 *
 * This is the WMI event handler function to receive radio stats tx
 * power level stats.
 *
 * Return: 0 on success, error number otherwise.
 */
static int wma_unified_radio_tx_power_level_stats_event_handler(void *handle,
			u_int8_t *cmd_param_info, u_int32_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_RADIO_TX_POWER_LEVEL_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_tx_power_level_stats_evt_fixed_param *fixed_param;
	uint8_t *tx_power_level_values;
	tSirLLStatsResults *link_stats_results;
	tSirWifiRadioStat *rs_results;

	tpAniSirGlobal mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!mac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_RADIO_TX_POWER_LEVEL_STATS_EVENTID_param_tlvs *)
								cmd_param_info;
	if (!param_tlvs) {
		WMA_LOGA("%s: Invalid tx power level stats event", __func__);
		return -EINVAL;
	}

	fixed_param = param_tlvs->fixed_param;
	if (!fixed_param) {
		WMA_LOGA("%s:Invalid param_tlvs for Radio tx_power level Stats",
			 __func__);
		return -EINVAL;
	}

	link_stats_results = wma_handle->link_stats_results;
	if (!link_stats_results) {
		WMA_LOGA("%s: link_stats_results is NULL", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: tot_num_tx_pwr_lvls: %u num_tx_pwr_lvls: %u pwr_lvl_offset: %u radio_id: %u",
			__func__, fixed_param->total_num_tx_power_levels,
			 fixed_param->num_tx_power_levels,
			 fixed_param->power_level_offset,
			 fixed_param->radio_id);

	rs_results = (tSirWifiRadioStat *) &link_stats_results->results[0] +
							 fixed_param->radio_id;
	tx_power_level_values = (uint8_t *) param_tlvs->tx_time_per_power_level;

	rs_results->total_num_tx_power_levels =
				fixed_param->total_num_tx_power_levels;
	if (!rs_results->total_num_tx_power_levels) {
		link_stats_results->nr_received++;
		goto post_stats;
	}

	if (!rs_results->tx_time_per_power_level) {
		rs_results->tx_time_per_power_level = qdf_mem_malloc(
				sizeof(uint32_t) *
				rs_results->total_num_tx_power_levels);
		if (!rs_results->tx_time_per_power_level) {
			WMA_LOGA("%s: Mem alloc fail for tx power level stats",
				 __func__);
			/* In error case, atleast send the radio stats without
			 * tx_power_level stats */
			rs_results->total_num_tx_power_levels = 0;
			link_stats_results->nr_received++;
			goto post_stats;
		}
	}
	qdf_mem_copy(&rs_results->tx_time_per_power_level[
					fixed_param->power_level_offset],
		tx_power_level_values,
		sizeof(uint32_t) * fixed_param->num_tx_power_levels);
	if (rs_results->total_num_tx_power_levels ==
	   (fixed_param->num_tx_power_levels +
					 fixed_param->power_level_offset)) {
		link_stats_results->moreResultToFollow = 0;
		link_stats_results->nr_received++;
	}

	WMA_LOGD("%s: moreResultToFollow: %u nr: %u nr_received: %u",
			__func__, link_stats_results->moreResultToFollow,
			link_stats_results->num_radio,
			link_stats_results->nr_received);

	/* If still data to receive, return from here */
	if (link_stats_results->moreResultToFollow)
		return 0;

post_stats:
	if (link_stats_results->num_radio != link_stats_results->nr_received) {
		/* Not received all radio stats yet, don't post yet */
		return 0;
	}

	/* call hdd callback with Link Layer Statistics
	 * vdev_id/ifacId in link_stats_results will be
	 * used to retrieve the correct HDD context
	 */
	mac->sme.pLinkLayerStatsIndCallback(mac->hHdd,
		WMA_LINK_LAYER_STATS_RESULTS_RSP,
		link_stats_results);
	wma_unified_radio_tx_mem_free(handle);

	return 0;
}

/**
 * wma_unified_link_radio_stats_event_handler() - radio link stats event handler
 * @handle:          wma handle
 * @cmd_param_info:  data received with event from fw
 * @len:             length of data
 *
 * Return: 0 for success or error code
 */
static int wma_unified_link_radio_stats_event_handler(void *handle,
						      uint8_t *cmd_param_info,
						      uint32_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_RADIO_LINK_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_radio_link_stats_event_fixed_param *fixed_param;
	wmi_radio_link_stats *radio_stats;
	wmi_channel_stats *channel_stats;
	tSirLLStatsResults *link_stats_results;
	uint8_t *results, *t_radio_stats, *t_channel_stats;
	uint32_t next_chan_offset, count;
	size_t radio_stats_size, chan_stats_size;
	size_t link_stats_results_size;
	tSirWifiRadioStat *rs_results;
	tSirWifiChannelStats *chn_results;

	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_RADIO_LINK_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_tlvs) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}

	/*
	 * cmd_param_info contains
	 * wmi_radio_link_stats_event_fixed_param fixed_param;
	 * size of(struct wmi_radio_link_stats);
	 * num_channels * size of(struct wmi_channel_stats)
	 */
	fixed_param = param_tlvs->fixed_param;
	radio_stats = param_tlvs->radio_stats;
	channel_stats = param_tlvs->channel_stats;

	if (!fixed_param || !radio_stats ||
	    (radio_stats->num_channels && !channel_stats)) {
		WMA_LOGA("%s: Invalid param_tlvs for Radio Stats", __func__);
		return -EINVAL;
	}

	radio_stats_size = sizeof(tSirWifiRadioStat);
	chan_stats_size = sizeof(tSirWifiChannelStats);
	link_stats_results_size = sizeof(*link_stats_results) +
				  fixed_param->num_radio * radio_stats_size;

	if (!wma_handle->link_stats_results) {
		wma_handle->link_stats_results = qdf_mem_malloc(
						link_stats_results_size);
		if (NULL == wma_handle->link_stats_results) {
			WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
				 __func__, link_stats_results_size);
			return -ENOMEM;
		}
	}
	link_stats_results = wma_handle->link_stats_results;

	WMA_LOGD("Radio stats Fixed Param:");
	WMA_LOGD("req_id: %u num_radio: %u more_radio_events: %u",
		 fixed_param->request_id, fixed_param->num_radio,
		 fixed_param->more_radio_events);

	WMA_LOGD("Radio Info: radio_id: %u on_time: %u tx_time: %u rx_time: %u on_time_scan: %u",
		radio_stats->radio_id, radio_stats->on_time,
		radio_stats->tx_time, radio_stats->rx_time,
		radio_stats->on_time_scan);
	WMA_LOGD("on_time_nbd: %u on_time_gscan: %u on_time_roam_scan: %u",
		radio_stats->on_time_nbd,
		radio_stats->on_time_gscan, radio_stats->on_time_roam_scan);
	WMA_LOGD("on_time_pno_scan: %u on_time_hs20: %u num_channels: %u",
		radio_stats->on_time_pno_scan, radio_stats->on_time_hs20,
		radio_stats->num_channels);

	link_stats_results->paramId = WMI_LINK_STATS_RADIO;
	link_stats_results->rspId = fixed_param->request_id;
	link_stats_results->ifaceId = 0;
	link_stats_results->num_radio = fixed_param->num_radio;
	link_stats_results->peer_event_number = 0;

	/*
	 * Backward compatibility:
	 * There are firmware(s) which will send Radio stats only with
	 * more_radio_events set to 0 and firmware which sends Radio stats
	 * followed by tx_power level stats with more_radio_events set to 1.
	 * if more_radio_events is set to 1, buffer the radio stats and
	 * wait for tx_power_level stats.
	 */
	link_stats_results->moreResultToFollow = fixed_param->more_radio_events;

	results = (uint8_t *) link_stats_results->results;
	t_radio_stats = (uint8_t *) radio_stats;
	t_channel_stats = (uint8_t *) channel_stats;

	rs_results = (tSirWifiRadioStat *) &results[0] + radio_stats->radio_id;
	rs_results->radio = radio_stats->radio_id;
	rs_results->onTime = radio_stats->on_time;
	rs_results->txTime = radio_stats->tx_time;
	rs_results->rxTime = radio_stats->rx_time;
	rs_results->onTimeScan = radio_stats->on_time_scan;
	rs_results->onTimeNbd = radio_stats->on_time_nbd;
	rs_results->onTimeGscan = radio_stats->on_time_gscan;
	rs_results->onTimeRoamScan = radio_stats->on_time_roam_scan;
	rs_results->onTimePnoScan = radio_stats->on_time_pno_scan;
	rs_results->onTimeHs20 = radio_stats->on_time_hs20;
	rs_results->total_num_tx_power_levels = 0;
	rs_results->tx_time_per_power_level = NULL;
	rs_results->numChannels = radio_stats->num_channels;
	rs_results->channels = NULL;

	if (rs_results->numChannels) {
		rs_results->channels = (tSirWifiChannelStats *) qdf_mem_malloc(
					radio_stats->num_channels *
					chan_stats_size);
		if (rs_results->channels == NULL) {
			WMA_LOGD("%s: could not allocate mem for channel stats (size=%zu)",
				 __func__, radio_stats->num_channels * chan_stats_size);
			wma_unified_radio_tx_mem_free(handle);
			return -ENOMEM;
		}

		chn_results = (tSirWifiChannelStats *) &rs_results->channels[0];
		next_chan_offset = WMI_TLV_HDR_SIZE;
		WMA_LOGD("Channel Stats Info");
		for (count = 0; count < radio_stats->num_channels; count++) {
			WMA_LOGD("channel_width %u center_freq %u center_freq0 %u",
				 channel_stats->channel_width,
				 channel_stats->center_freq,
				 channel_stats->center_freq0);
			WMA_LOGD("center_freq1 %u radio_awake_time %u cca_busy_time %u",
				 channel_stats->center_freq1,
				 channel_stats->radio_awake_time,
				 channel_stats->cca_busy_time);
			channel_stats++;

			qdf_mem_copy(chn_results,
				     t_channel_stats + next_chan_offset,
				     chan_stats_size);
			chn_results++;
			next_chan_offset += sizeof(*channel_stats);
		}
	}

	if (link_stats_results->moreResultToFollow) {
		/* More results coming, don't post yet */
		return 0;
	}
	link_stats_results->nr_received++;

	if (link_stats_results->num_radio != link_stats_results->nr_received) {
		/* Not received all radio stats yet, don't post yet */
		return 0;
	}

	pMac->sme.pLinkLayerStatsIndCallback(pMac->hHdd,
					     WMA_LINK_LAYER_STATS_RESULTS_RSP,
					     link_stats_results);
	wma_unified_radio_tx_mem_free(handle);

	return 0;
}

#ifdef WLAN_PEER_PS_NOTIFICATION
/**
 * wma_peer_ps_evt_handler() - handler for PEER power state change.
 * @handle: wma handle
 * @event: FW event
 * @len: length of FW event
 *
 * Once peer STA power state changes, an event will be indicated by
 * FW. This function send a link layer state change msg to HDD. HDD
 * link layer callback will converts the event to NL msg.
 *
 * Return: 0 Success. Others fail.
 */
static int wma_peer_ps_evt_handler(void *handle, u_int8_t *event,
				   u_int32_t len)
{
	WMI_PEER_STA_PS_STATECHG_EVENTID_param_tlvs *param_buf;
	wmi_peer_sta_ps_statechange_event_fixed_param *fixed_param;
	tSirWifiPeerStat *peer_stat;
	tSirWifiPeerInfo *peer_info;
	tSirLLStatsResults *link_stats_results;
	tSirMacAddr mac_address;
	uint32_t result_len;
	cds_msg_t sme_msg = { 0 };
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	tpAniSirGlobal mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGD("%s: NULL mac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!mac->sme.link_layer_stats_ext_cb) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: Posting Peer Stats PS event to HDD", __func__);

	param_buf = (WMI_PEER_STA_PS_STATECHG_EVENTID_param_tlvs *)event;
	fixed_param = param_buf->fixed_param;

	result_len = sizeof(tSirLLStatsResults) +
			sizeof(tSirWifiPeerStat) +
			sizeof(tSirWifiPeerInfo);
	link_stats_results = qdf_mem_malloc(result_len);
	if (link_stats_results == NULL) {
		WMA_LOGE("%s: Cannot allocate link layer stats.", __func__);
		return -EINVAL;
	}

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_param->peer_macaddr, &mac_address[0]);
	WMA_LOGD("Peer power state change event from FW");
	WMA_LOGD("Fixed Param:");
	WMA_LOGD("MAC address: %2x:%2x:%2x:%2x:%2x:%2x, Power state: %d",
		 mac_address[0], mac_address[1], mac_address[2],
		 mac_address[3], mac_address[4], mac_address[5],
		 fixed_param->peer_ps_state);

	link_stats_results->paramId            = WMI_LL_STATS_EXT_PS_CHG;
	link_stats_results->num_peers          = 1;
	link_stats_results->peer_event_number  = 1;
	link_stats_results->moreResultToFollow = 0;

	peer_stat = (tSirWifiPeerStat *)link_stats_results->results;
	peer_stat->numPeers = 1;
	peer_info = (tSirWifiPeerInfo *)peer_stat->peerInfo;
	qdf_mem_copy(&peer_info->peerMacAddress,
		     &mac_address,
		     sizeof(tSirMacAddr));
	peer_info->power_saving = fixed_param->peer_ps_state;

	sme_msg.type = eWMI_SME_LL_STATS_IND;
	sme_msg.bodyptr = link_stats_results;
	sme_msg.bodyval = 0;

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("%s: Fail to post ps change ind msg", __func__);
		qdf_mem_free(link_stats_results);
	}

	return 0;
}
#else
/**
 * wma_peer_ps_evt_handler() - handler for PEER power state change.
 * @handle: wma handle
 * @event: FW event
 * @len: length of FW event
 *
 * Once peer STA power state changes, an event will be indicated by
 * FW. This function send a link layer state change msg to HDD. HDD
 * link layer callback will converts the event to NL msg.
 *
 * Return: 0 Success. Others fail.
 */
static inline int wma_peer_ps_evt_handler(void *handle, u_int8_t *event,
					  u_int32_t len)
{
	return 0;
}
#endif

/**
 * wma_tx_failure_cb() - TX failure callback
 * @ctx: txrx context
 * @num_msdu: number of msdu with the same status
 * @tid: TID number
 * @status: failure status
 */
void wma_tx_failure_cb(void *ctx, uint32_t num_msdu,
		       uint8_t tid, enum htt_tx_status status)
{
	tSirLLStatsResults *results;
	struct sir_wifi_iface_tx_fail *tx_fail;
	struct scheduler_msg sme_msg = { 0 };
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGD("%s: NULL mac ptr. Exiting", __func__);
		return;
	}

	if (!mac->sme.link_layer_stats_ext_cb) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return;
	}

	results = qdf_mem_malloc(sizeof(tSirLLStatsResults) +
				 sizeof(struct sir_wifi_iface_tx_fail));
	if (results == NULL) {
		WMA_LOGE("%s: Cannot allocate link layer stats.", __func__);
		return;
	}

	results->paramId            = WMI_LL_STATS_EXT_TX_FAIL;
	results->num_peers          = 1;
	results->peer_event_number  = 1;
	results->moreResultToFollow = 0;

	tx_fail = (struct sir_wifi_iface_tx_fail *)results->results;
	tx_fail->tid = tid;
	tx_fail->msdu_num = num_msdu;
	tx_fail->status = status;

	sme_msg.type = eWMI_SME_LL_STATS_IND;
	sme_msg.bodyptr = results;
	sme_msg.bodyval = 0;

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("%s: Fail to post TX failure ind msg", __func__);
		qdf_mem_free(results);
	}
}

/**
 * wma_register_ll_stats_event_handler() - register link layer stats related
 *                                         event handler
 * @wma_handle: wma handle
 *
 * Return: none
 */
void wma_register_ll_stats_event_handler(tp_wma_handle wma_handle)
{
	if (NULL == wma_handle) {
		WMA_LOGE("%s: wma_handle is NULL", __func__);
		return;
	}

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_IFACE_LINK_STATS_EVENTID,
				wma_unified_link_iface_stats_event_handler,
				WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_PEER_LINK_STATS_EVENTID,
				wma_unified_link_peer_stats_event_handler,
				WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_RADIO_LINK_STATS_EVENTID,
				wma_unified_link_radio_stats_event_handler,
				WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_RADIO_TX_POWER_LEVEL_STATS_EVENTID,
			wma_unified_radio_tx_power_level_stats_event_handler,
			WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_STA_PS_STATECHG_EVENTID,
					   wma_peer_ps_evt_handler,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_REPORT_STATS_EVENTID,
					   wma_ll_stats_evt_handler,
					   WMA_RX_SERIALIZER_CTX);

}


/**
 * wma_process_ll_stats_clear_req() - clear link layer stats
 * @wma: wma handle
 * @clearReq: ll stats clear request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_process_ll_stats_clear_req(tp_wma_handle wma,
				 const tpSirLLStatsClearReq clearReq)
{
	struct ll_stats_clear_params cmd = {0};
	int ret;

	if (!clearReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd.stop_req = clearReq->stopReq;
	cmd.sta_id = clearReq->staId;
	cmd.stats_clear_mask = clearReq->statsClearReqMask;

	ret = wmi_unified_process_ll_stats_clear_cmd(wma->wmi_handle, &cmd,
				   wma->interfaces[clearReq->staId].addr);
	if (ret) {
		WMA_LOGE("%s: Failed to send clear link stats req", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_ll_stats_set_req() - link layer stats set request
 * @wma:       wma handle
 * @setReq:  ll stats set request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_process_ll_stats_set_req(tp_wma_handle wma,
				 const tpSirLLStatsSetReq setReq)
{
	struct ll_stats_set_params cmd = {0};
	int ret;

	if (!setReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd.mpdu_size_threshold = setReq->mpduSizeThreshold;
	cmd.aggressive_statistics_gathering =
		setReq->aggressiveStatisticsGathering;

	ret = wmi_unified_process_ll_stats_set_cmd(wma->wmi_handle,
					 &cmd);
	if (ret) {
		WMA_LOGE("%s: Failed to send set link stats request", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_ll_stats_get_req() - link layer stats get request
 * @wma:wma handle
 * @getReq:ll stats get request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_process_ll_stats_get_req(tp_wma_handle wma,
				 const tpSirLLStatsGetReq getReq)
{
	struct ll_stats_get_params cmd = {0};
	int ret;

	if (!getReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!wma->interfaces[getReq->staId].vdev_active) {
		WMA_LOGE("%s: vdev not created yet", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd.req_id = getReq->reqId;
	cmd.param_id_mask = getReq->paramIdMask;
	cmd.sta_id = getReq->staId;

	ret = wmi_unified_process_ll_stats_get_cmd(wma->wmi_handle, &cmd,
				   wma->interfaces[getReq->staId].addr);
	if (ret) {
		WMA_LOGE("%s: Failed to send get link stats request", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_unified_link_iface_stats_event_handler() - link iface stats event handler
 * @wma:wma handle
 * @cmd_param_info: data from event
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_unified_link_iface_stats_event_handler(void *handle,
					       uint8_t *cmd_param_info,
					       uint32_t len)
{
	WMI_IFACE_LINK_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_iface_link_stats_event_fixed_param *fixed_param;
	wmi_iface_link_stats *link_stats;
	wmi_wmm_ac_stats *ac_stats;
	tSirLLStatsResults *link_stats_results;
	uint8_t *results, *t_link_stats, *t_ac_stats;
	uint32_t next_res_offset, next_ac_offset, count;
	uint32_t roaming_offset, roaming_size;
	size_t link_stats_size, ac_stats_size, iface_info_size;
	size_t link_stats_results_size;

	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	param_tlvs = (WMI_IFACE_LINK_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_tlvs) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}

	/*
	 * cmd_param_info contains
	 * wmi_iface_link_stats_event_fixed_param fixed_param;
	 * wmi_iface_link_stats iface_link_stats;
	 * iface_link_stats->num_ac * size of(struct wmi_wmm_ac_stats)
	 */
	fixed_param = param_tlvs->fixed_param;
	link_stats = param_tlvs->iface_link_stats;
	ac_stats = param_tlvs->ac;

	if (!fixed_param || !link_stats || (link_stats->num_ac && !ac_stats)) {
		WMA_LOGA("%s: Invalid param_tlvs for Iface Stats", __func__);
		return -EINVAL;
	}

	link_stats_size = sizeof(tSirWifiIfaceStat);
	iface_info_size = sizeof(tSirWifiInterfaceInfo);
	ac_stats_size = sizeof(tSirWifiWmmAcStat);
	link_stats_results_size = sizeof(*link_stats_results) + link_stats_size;

	link_stats_results = qdf_mem_malloc(link_stats_results_size);
	if (!link_stats_results) {
		WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
			 __func__, link_stats_results_size);
		return -ENOMEM;
	}

	qdf_mem_zero(link_stats_results, link_stats_results_size);

	link_stats_results->paramId = WMI_LINK_STATS_IFACE;
	link_stats_results->rspId = fixed_param->request_id;
	link_stats_results->ifaceId = fixed_param->vdev_id;
	link_stats_results->num_peers = link_stats->num_peers;
	link_stats_results->peer_event_number = 0;
	link_stats_results->moreResultToFollow = 0;

	results = (uint8_t *) link_stats_results->results;
	t_link_stats = (uint8_t *) link_stats;
	t_ac_stats = (uint8_t *) ac_stats;

	/* Copy roaming state */
	roaming_offset = offsetof(tSirWifiInterfaceInfo, roaming);
	roaming_size = member_size(tSirWifiInterfaceInfo, roaming);

	qdf_mem_copy(results + roaming_offset, &link_stats->roam_state,
		     roaming_size);

	qdf_mem_copy(results + iface_info_size,
		     t_link_stats + WMI_TLV_HDR_SIZE,
		     link_stats_size - iface_info_size -
		     WIFI_AC_MAX * ac_stats_size);

	next_res_offset = link_stats_size - WIFI_AC_MAX * ac_stats_size;
	next_ac_offset = WMI_TLV_HDR_SIZE;

	for (count = 0; count < link_stats->num_ac; count++) {
		ac_stats++;

		qdf_mem_copy(results + next_res_offset,
			     t_ac_stats + next_ac_offset, ac_stats_size);
		next_res_offset += ac_stats_size;
		next_ac_offset += sizeof(*ac_stats);
	}

	/* call hdd callback with Link Layer Statistics
	 * vdev_id/ifacId in link_stats_results will be
	 * used to retrieve the correct HDD context
	 */
	pMac->sme.pLinkLayerStatsIndCallback(pMac->hHdd,
					     WMA_LINK_LAYER_STATS_RESULTS_RSP,
					     link_stats_results);
	qdf_mem_free(link_stats_results);

	return 0;
}

/**
 * wma_config_stats_ext_threshold - set threthold for MAC counters
 * @wma: wma handler
 * @threshold: threhold for MAC counters
 *
 * For each MAC layer counter, FW holds two copies. One is the current value.
 * The other is the last report. Once a current counter's increment is larger
 * than the threshold, FW will indicate that counter to host even if the
 * monitoring timer does not expire.
 *
 * Return: None
 */
void wma_config_stats_ext_threshold(tp_wma_handle wma,
				    struct sir_ll_ext_stats_threshold *thresh)
{
	uint32_t len, tag, hdr_len;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	wmi_pdev_set_stats_threshold_cmd_fixed_param *cmd;
	wmi_chan_cca_stats_thresh *cca;
	wmi_peer_signal_stats_thresh *signal;
	wmi_tx_stats_thresh *tx;
	wmi_rx_stats_thresh *rx;

	if (!thresh) {
		WMA_LOGE(FL("Invalid threshold input."));
		return;
	}

	len = sizeof(wmi_pdev_set_stats_threshold_cmd_fixed_param) +
	      sizeof(wmi_chan_cca_stats_thresh) +
	      sizeof(wmi_peer_signal_stats_thresh) +
	      sizeof(wmi_tx_stats_thresh) +
	      sizeof(wmi_rx_stats_thresh) +
	      5 * WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return;
	}

	buf_ptr = (u_int8_t *)wmi_buf_data(buf);
	tag = WMITLV_TAG_STRUC_wmi_pdev_set_stats_threshold_cmd_fixed_param;
	hdr_len = WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_stats_threshold_cmd_fixed_param);
	WMA_LOGD(FL("Setting fixed parameters. tag=%d, len=%d"), tag, hdr_len);
	cmd = (wmi_pdev_set_stats_threshold_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header, tag, hdr_len);
	cmd->enable_thresh = thresh->enable;
	cmd->use_thresh_bitmap = thresh->enable_bitmap;
	cmd->gbl_thresh = thresh->global_threshold;
	cmd->cca_thresh_enable_bitmap = thresh->cca_bitmap;
	cmd->signal_thresh_enable_bitmap = thresh->signal_bitmap;
	cmd->tx_thresh_enable_bitmap = thresh->tx_bitmap;
	cmd->rx_thresh_enable_bitmap = thresh->rx_bitmap;
	len = sizeof(wmi_pdev_set_stats_threshold_cmd_fixed_param);

	tag = WMITLV_TAG_STRUC_wmi_chan_cca_stats_thresh,
	hdr_len = WMITLV_GET_STRUCT_TLVLEN(wmi_chan_cca_stats_thresh);
	cca = (wmi_chan_cca_stats_thresh *)(buf_ptr + len);
	WMITLV_SET_HDR(&cca->tlv_header, tag, hdr_len);
	WMA_LOGD(FL("Setting cca parameters. tag=%d, len=%d"), tag, hdr_len);
	cca->idle_time = thresh->cca.idle_time;
	cca->tx_time = thresh->cca.tx_time;
	cca->rx_in_bss_time = thresh->cca.rx_in_bss_time;
	cca->rx_out_bss_time = thresh->cca.rx_out_bss_time;
	cca->rx_busy_time = thresh->cca.rx_busy_time;
	cca->rx_in_bad_cond_time = thresh->cca.rx_in_bad_cond_time;
	cca->tx_in_bad_cond_time = thresh->cca.tx_in_bad_cond_time;
	cca->wlan_not_avail_time = thresh->cca.wlan_not_avail_time;
	WMA_LOGD(FL("idle time=%d, tx_time=%d, in_bss=%d, out_bss=%d"),
		 cca->idle_time, cca->tx_time,
		 cca->rx_in_bss_time, cca->rx_out_bss_time);
	WMA_LOGD(FL("rx_busy=%d, rx_bad=%d, tx_bad=%d, not_avail=%d"),
		 cca->rx_busy_time, cca->rx_in_bad_cond_time,
		 cca->tx_in_bad_cond_time, cca->wlan_not_avail_time);
	len += sizeof(wmi_chan_cca_stats_thresh);

	signal = (wmi_peer_signal_stats_thresh *)(buf_ptr + len);
	tag = WMITLV_TAG_STRUC_wmi_peer_signal_stats_thresh;
	hdr_len = WMITLV_GET_STRUCT_TLVLEN(wmi_peer_signal_stats_thresh);
	WMA_LOGD(FL("Setting signal parameters. tag=%d, len=%d"), tag, hdr_len);
	WMITLV_SET_HDR(&signal->tlv_header, tag, hdr_len);
	signal->per_chain_snr = thresh->signal.snr;
	signal->per_chain_nf = thresh->signal.nf;
	WMA_LOGD(FL("snr=%d, nf=%d"), signal->per_chain_snr,
		 signal->per_chain_nf);
	len += sizeof(wmi_peer_signal_stats_thresh);

	tx = (wmi_tx_stats_thresh *)(buf_ptr + len);
	tag = WMITLV_TAG_STRUC_wmi_tx_stats_thresh;
	hdr_len = WMITLV_GET_STRUCT_TLVLEN(wmi_tx_stats_thresh);
	WMA_LOGD(FL("Setting TX parameters. tag=%d, len=%d"), tag, len);
	WMITLV_SET_HDR(&tx->tlv_header, tag, hdr_len);
	tx->tx_msdu_cnt = thresh->tx.msdu;
	tx->tx_mpdu_cnt = thresh->tx.mpdu;
	tx->tx_ppdu_cnt = thresh->tx.ppdu;
	tx->tx_bytes = thresh->tx.bytes;
	tx->tx_msdu_drop_cnt = thresh->tx.msdu_drop;
	tx->tx_drop_bytes = thresh->tx.byte_drop;
	tx->tx_mpdu_retry_cnt = thresh->tx.mpdu_retry;
	tx->tx_mpdu_fail_cnt = thresh->tx.mpdu_fail;
	tx->tx_ppdu_fail_cnt = thresh->tx.ppdu_fail;
	tx->tx_mpdu_aggr = thresh->tx.aggregation;
	tx->tx_succ_mcs = thresh->tx.succ_mcs;
	tx->tx_fail_mcs = thresh->tx.fail_mcs;
	tx->tx_ppdu_delay = thresh->tx.delay;
	WMA_LOGD(FL("msdu=%d, mpdu=%d, ppdu=%d, bytes=%d, msdu_drop=%d"),
		 tx->tx_msdu_cnt, tx->tx_mpdu_cnt, tx->tx_ppdu_cnt,
		 tx->tx_bytes, tx->tx_msdu_drop_cnt);
	WMA_LOGD(FL("byte_drop=%d, mpdu_retry=%d, mpdu_fail=%d, ppdu_fail=%d"),
		 tx->tx_drop_bytes, tx->tx_mpdu_retry_cnt,
		 tx->tx_mpdu_fail_cnt, tx->tx_ppdu_fail_cnt);
	WMA_LOGD(FL("aggr=%d, succ_mcs=%d, fail_mcs=%d, delay=%d"),
		 tx->tx_mpdu_aggr, tx->tx_succ_mcs, tx->tx_fail_mcs,
		 tx->tx_ppdu_delay);
	len += sizeof(wmi_tx_stats_thresh);

	rx = (wmi_rx_stats_thresh *)(buf_ptr + len);
	tag = WMITLV_TAG_STRUC_wmi_rx_stats_thresh,
	hdr_len = WMITLV_GET_STRUCT_TLVLEN(wmi_rx_stats_thresh);
	WMITLV_SET_HDR(&rx->tlv_header, tag, hdr_len);
	WMA_LOGD(FL("Setting RX parameters. tag=%d, len=%d"), tag, hdr_len);
	rx->mac_rx_mpdu_cnt = thresh->rx.mpdu;
	rx->mac_rx_bytes = thresh->rx.bytes;
	rx->phy_rx_ppdu_cnt = thresh->rx.ppdu;
	rx->phy_rx_bytes = thresh->rx.ppdu_bytes;
	rx->rx_disorder_cnt = thresh->rx.disorder;
	rx->rx_mpdu_retry_cnt = thresh->rx.mpdu_retry;
	rx->rx_mpdu_dup_cnt = thresh->rx.mpdu_dup;
	rx->rx_mpdu_discard_cnt = thresh->rx.mpdu_discard;
	rx->rx_mpdu_aggr = thresh->rx.aggregation;
	rx->rx_mcs = thresh->rx.mcs;
	rx->sta_ps_inds = thresh->rx.ps_inds;
	rx->sta_ps_durs = thresh->rx.ps_durs;
	rx->rx_probe_reqs = thresh->rx.probe_reqs;
	rx->rx_oth_mgmts = thresh->rx.other_mgmt;
	WMA_LOGD(FL("rx_mpdu=%d, rx_bytes=%d, rx_ppdu=%d, rx_pbytes=%d"),
		 rx->mac_rx_mpdu_cnt, rx->mac_rx_bytes,
		 rx->phy_rx_ppdu_cnt, rx->phy_rx_bytes);
	WMA_LOGD(FL("disorder=%d, rx_dup=%d, rx_aggr=%d, rx_mcs=%d"),
		 rx->rx_disorder_cnt, rx->rx_mpdu_dup_cnt,
		 rx->rx_mpdu_aggr, rx->rx_mcs);
	WMA_LOGD(FL("rx_ind=%d, rx_dur=%d, rx_probe=%d, rx_mgmt=%d"),
		 rx->sta_ps_inds, rx->sta_ps_durs,
		 rx->rx_probe_reqs, rx->rx_oth_mgmts);
	len += sizeof(wmi_rx_stats_thresh);

	WMA_LOGA("WMA --> WMI_PDEV_SET_STATS_THRESHOLD_CMDID(0x%x), length=%d",
		 WMI_PDEV_SET_STATS_THRESHOLD_CMDID, len);
	if (EOK != wmi_unified_cmd_send(wma->wmi_handle,
					buf, len,
					WMI_PDEV_SET_STATS_THRESHOLD_CMDID)) {
		WMA_LOGE("Failed to send WMI_PDEV_SET_STATS_THRESHOLD_CMDID");
		wmi_buf_free(buf);
	}
}

#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * wma_update_pdev_stats() - update pdev stats
 * @wma: wma handle
 * @pdev_stats: pdev stats
 *
 * Return: none
 */
static void wma_update_pdev_stats(tp_wma_handle wma,
				  wmi_pdev_stats *pdev_stats)
{
	tAniGetPEStatsRsp *stats_rsp_params;
	uint32_t temp_mask;
	uint8_t *stats_buf;
	tCsrGlobalClassAStatsInfo *classa_stats = NULL;
	struct wma_txrx_node *node;
	uint8_t i;

	for (i = 0; i < wma->max_bssid; i++) {
		node = &wma->interfaces[i];
		stats_rsp_params = node->stats_rsp;
		if (stats_rsp_params) {
			node->fw_stats_set |= FW_PDEV_STATS_SET;
			WMA_LOGD("<---FW PDEV STATS received for vdevId:%d", i);
			stats_buf = (uint8_t *) (stats_rsp_params + 1);
			temp_mask = stats_rsp_params->statsMask;
			if (temp_mask & (1 << eCsrSummaryStats))
				stats_buf += sizeof(tCsrSummaryStatsInfo);

			if (temp_mask & (1 << eCsrGlobalClassAStats)) {
				classa_stats =
					(tCsrGlobalClassAStatsInfo *) stats_buf;
				classa_stats->max_pwr = pdev_stats->chan_tx_pwr;
			}
		}
	}
}

/**
 * wma_vdev_stats_lost_link_helper() - helper function to extract
 * lost link information from vdev statistics event while deleting BSS.
 * @wma: WMA handle
 * @vdev_stats: statistics information from firmware
 *
 * This is for informing HDD to collect lost link information while
 * disconnection. Following conditions to check
 * 1. vdev is up
 * 2. bssid is zero. When handling DELETE_BSS request message, it sets bssid to
 * zero, hence add the check here to indicate the event comes during deleting
 * BSS
 * 3. DELETE_BSS is the request message queued. Put this condition check on the
 * last one as it consumes more resource searching entries in the  list
 *
 * Return: none
 */
static void wma_vdev_stats_lost_link_helper(tp_wma_handle wma,
					    wmi_vdev_stats *vdev_stats)
{
	struct wma_txrx_node *node;
	int32_t rssi;
	struct wma_target_req *req_msg;
	static const uint8_t zero_mac[QDF_MAC_ADDR_SIZE] = {0};
	int8_t bcn_snr, dat_snr;

	node = &wma->interfaces[vdev_stats->vdev_id];
	if (wma_is_vdev_up(vdev_stats->vdev_id) &&
	    !qdf_mem_cmp(node->bssid, zero_mac, QDF_MAC_ADDR_SIZE)) {
		req_msg = wma_peek_vdev_req(wma, vdev_stats->vdev_id,
					    WMA_TARGET_REQ_TYPE_VDEV_STOP);
		if ((NULL == req_msg) ||
		    (WMA_DELETE_BSS_REQ != req_msg->msg_type)) {
			WMA_LOGD(FL("cannot find DELETE_BSS request message"));
			return;
		 }
		bcn_snr = vdev_stats->vdev_snr.bcn_snr;
		dat_snr = vdev_stats->vdev_snr.dat_snr;
		WMA_LOGD(FL("get vdev id %d, beancon snr %d, data snr %d"),
			vdev_stats->vdev_id, bcn_snr, dat_snr);
		if ((bcn_snr != WMA_TGT_INVALID_SNR_OLD) &&
			(bcn_snr != WMA_TGT_INVALID_SNR_NEW))
			rssi = bcn_snr;
		else if ((dat_snr != WMA_TGT_INVALID_SNR_OLD) &&
			(dat_snr != WMA_TGT_INVALID_SNR_NEW))
			rssi = dat_snr;
		else
			rssi = WMA_TGT_INVALID_SNR_OLD;

		/* Get the absolute rssi value from the current rssi value */
		rssi = rssi + WMA_TGT_NOISE_FLOOR_DBM;
		wma_lost_link_info_handler(wma, vdev_stats->vdev_id, rssi);
	}
}

/**
 * wma_update_vdev_stats() - update vdev stats
 * @wma: wma handle
 * @vdev_stats: vdev stats
 *
 * Return: none
 */
static void wma_update_vdev_stats(tp_wma_handle wma,
				  wmi_vdev_stats *vdev_stats)
{
	tAniGetPEStatsRsp *stats_rsp_params;
	tCsrSummaryStatsInfo *summary_stats = NULL;
	uint8_t *stats_buf;
	struct wma_txrx_node *node;
	uint8_t i;
	int8_t rssi = 0;
	QDF_STATUS qdf_status;
	tAniGetRssiReq *pGetRssiReq = (tAniGetRssiReq *) wma->pGetRssiReq;
	struct scheduler_msg sme_msg = { 0 };
	int8_t bcn_snr, dat_snr;

	bcn_snr = vdev_stats->vdev_snr.bcn_snr;
	dat_snr = vdev_stats->vdev_snr.dat_snr;
	WMA_LOGD("vdev id %d beancon snr %d data snr %d",
		 vdev_stats->vdev_id, bcn_snr, dat_snr);

	node = &wma->interfaces[vdev_stats->vdev_id];
	stats_rsp_params = node->stats_rsp;
	if (stats_rsp_params) {
		stats_buf = (uint8_t *) (stats_rsp_params + 1);
		node->fw_stats_set |= FW_VDEV_STATS_SET;
		WMA_LOGD("<---FW VDEV STATS received for vdevId:%d",
			 vdev_stats->vdev_id);
		if (stats_rsp_params->statsMask & (1 << eCsrSummaryStats)) {
			summary_stats = (tCsrSummaryStatsInfo *) stats_buf;
			for (i = 0; i < 4; i++) {
				summary_stats->tx_frm_cnt[i] =
					vdev_stats->tx_frm_cnt[i];
				summary_stats->fail_cnt[i] =
					vdev_stats->fail_cnt[i];
				summary_stats->multiple_retry_cnt[i] =
					vdev_stats->multiple_retry_cnt[i];
			}

			summary_stats->rx_frm_cnt = vdev_stats->rx_frm_cnt;
			summary_stats->rx_error_cnt = vdev_stats->rx_err_cnt;
			summary_stats->rx_discard_cnt =
				vdev_stats->rx_discard_cnt;
			summary_stats->ack_fail_cnt = vdev_stats->ack_fail_cnt;
			summary_stats->rts_succ_cnt = vdev_stats->rts_succ_cnt;
			summary_stats->rts_fail_cnt = vdev_stats->rts_fail_cnt;
			/* Update SNR and RSSI in SummaryStats */
			if ((bcn_snr != WMA_TGT_INVALID_SNR_OLD) &&
				(bcn_snr != WMA_TGT_INVALID_SNR_NEW)) {
				summary_stats->snr = bcn_snr;
				summary_stats->rssi =
					bcn_snr + WMA_TGT_NOISE_FLOOR_DBM;
			} else if ((dat_snr != WMA_TGT_INVALID_SNR_OLD) &&
					(dat_snr != WMA_TGT_INVALID_SNR_NEW)) {
				summary_stats->snr = dat_snr;
				summary_stats->rssi =
					bcn_snr + WMA_TGT_NOISE_FLOOR_DBM;
			} else {
				summary_stats->snr = WMA_TGT_INVALID_SNR_OLD;
				summary_stats->rssi = 0;
			}
		}
	}

	if (pGetRssiReq && pGetRssiReq->sessionId == vdev_stats->vdev_id) {
		if ((bcn_snr == WMA_TGT_INVALID_SNR_OLD ||
				bcn_snr == WMA_TGT_INVALID_SNR_NEW) &&
				(dat_snr == WMA_TGT_INVALID_SNR_OLD ||
				  dat_snr == WMA_TGT_INVALID_SNR_NEW)) {
			/*
			 * Firmware sends invalid snr till it sees
			 * Beacon/Data after connection since after
			 * vdev up fw resets the snr to invalid.
			 * In this duartion Host will return the last know
			 * rssi during connection.
			 */
			WMA_LOGE("Invalid SNR from firmware");

		} else {
			if (bcn_snr != WMA_TGT_INVALID_SNR_OLD &&
				bcn_snr != WMA_TGT_INVALID_SNR_NEW) {
				rssi = bcn_snr;
			} else if (dat_snr != WMA_TGT_INVALID_SNR_OLD &&
				dat_snr != WMA_TGT_INVALID_SNR_NEW) {
				rssi = dat_snr;
			}

			/*
			 * Get the absolute rssi value from current rssi value
			 * the sinr value is hardcoded into 0 in the core stack
			 */
			rssi = rssi + WMA_TGT_NOISE_FLOOR_DBM;
		}

		WMA_LOGD("Average Rssi = %d, vdev id= %d", rssi,
			 pGetRssiReq->sessionId);

		/* update the average rssi value to UMAC layer */
		if (NULL != pGetRssiReq->rssiCallback) {
			((tCsrRssiCallback) (pGetRssiReq->rssiCallback))(rssi,
						pGetRssiReq->staId,
						pGetRssiReq->pDevContext);
		}

		qdf_mem_free(pGetRssiReq);
		wma->pGetRssiReq = NULL;
	}

	if (node->psnr_req) {
		tAniGetSnrReq *p_snr_req = node->psnr_req;

		if ((bcn_snr != WMA_TGT_INVALID_SNR_OLD) &&
			(bcn_snr != WMA_TGT_INVALID_SNR_NEW))
			p_snr_req->snr = bcn_snr;
		else if ((dat_snr != WMA_TGT_INVALID_SNR_OLD) &&
				(dat_snr != WMA_TGT_INVALID_SNR_NEW))
			p_snr_req->snr = dat_snr;
		else
			p_snr_req->snr = WMA_TGT_INVALID_SNR_OLD;

		sme_msg.type = eWNI_SME_SNR_IND;
		sme_msg.bodyptr = p_snr_req;
		sme_msg.bodyval = 0;

		qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			WMA_LOGE("%s: Fail to post snr ind msg", __func__);
			qdf_mem_free(p_snr_req);
		}

		node->psnr_req = NULL;
	}
	wma_vdev_stats_lost_link_helper(wma, vdev_stats);
}

/**
 * wma_post_stats() - update stats to PE
 * @wma: wma handle
 * @node: txrx node
 *
 * Return: none
 */
static void wma_post_stats(tp_wma_handle wma, struct wma_txrx_node *node)
{
	/* send response to UMAC */
	wma_send_msg(wma, WMA_GET_STATISTICS_RSP, node->stats_rsp, 0);
	node->stats_rsp = NULL;
	node->fw_stats_set = 0;
}

/**
 * wma_update_peer_stats() - update peer stats
 * @wma: wma handle
 * @peer_stats: peer stats
 *
 * Return: none
 */
static void wma_update_peer_stats(tp_wma_handle wma,
				  wmi_peer_stats *peer_stats)
{
	tAniGetPEStatsRsp *stats_rsp_params;
	tCsrGlobalClassAStatsInfo *classa_stats = NULL;
	struct wma_txrx_node *node;
	uint8_t *stats_buf, vdev_id, macaddr[IEEE80211_ADDR_LEN], mcsRateFlags;
	uint32_t temp_mask;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&peer_stats->peer_macaddr, &macaddr[0]);
	if (!wma_find_vdev_by_bssid(wma, macaddr, &vdev_id))
		return;

	node = &wma->interfaces[vdev_id];
	stats_rsp_params = (tAniGetPEStatsRsp *) node->stats_rsp;
	if (stats_rsp_params) {
		node->fw_stats_set |= FW_PEER_STATS_SET;
		WMA_LOGD("<-- FW PEER STATS received for vdevId:%d", vdev_id);
		stats_buf = (uint8_t *) (stats_rsp_params + 1);
		temp_mask = stats_rsp_params->statsMask;
		if (temp_mask & (1 << eCsrSummaryStats))
			stats_buf += sizeof(tCsrSummaryStatsInfo);

		if (temp_mask & (1 << eCsrGlobalClassAStats)) {
			classa_stats = (tCsrGlobalClassAStatsInfo *) stats_buf;
			WMA_LOGD("peer tx rate:%d", peer_stats->peer_tx_rate);
			/* The linkspeed returned by fw is in kbps so convert
			 * it in to units of 500kbps which is expected by UMAC
			 */
			if (peer_stats->peer_tx_rate) {
				classa_stats->tx_rate =
					peer_stats->peer_tx_rate / 500;
			}

			classa_stats->tx_rate_flags = node->rate_flags;
			if (!(node->rate_flags & eHAL_TX_RATE_LEGACY)) {
				classa_stats->mcs_index =
					wma_get_mcs_idx(
						(peer_stats->peer_tx_rate /
						100), node->rate_flags,
						node->nss, &mcsRateFlags);
				classa_stats->nss = node->nss;
				classa_stats->mcs_rate_flags = mcsRateFlags;
			}
			/* FW returns tx power in intervals of 0.5 dBm
			 * Convert it back to intervals of 1 dBm
			 */
			classa_stats->max_pwr =
				roundup(classa_stats->max_pwr, 2) >> 1;
		}
	}
}

/**
 * wma_post_link_status() - post link status to SME
 * @pGetLinkStatus: SME Link status
 * @link_status: Link status
 *
 * Return: none
 */
void wma_post_link_status(tAniGetLinkStatus *pGetLinkStatus,
			  uint8_t link_status)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct scheduler_msg sme_msg = { 0 };

	pGetLinkStatus->linkStatus = link_status;
	sme_msg.type = eWNI_SME_LINK_STATUS_IND;
	sme_msg.bodyptr = pGetLinkStatus;
	sme_msg.bodyval = 0;

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("%s: Fail to post link status ind msg", __func__);
		qdf_mem_free(pGetLinkStatus);
	}
}

/**
 * wma_update_per_chain_rssi_stats() - to store per chain rssi stats
 * @wma: wma handle
 * @rssi_stats: rssi stats
 * @rssi_per_chain_stats: buffer where rssi stats to be stored
 *
 * This function stores per chain rssi stats received from fw for all vdevs for
 * which the stats were requested into a csr stats structure.
 *
 * Return: void
 */
static void wma_update_per_chain_rssi_stats(tp_wma_handle wma,
		wmi_rssi_stats *rssi_stats,
		struct csr_per_chain_rssi_stats_info *rssi_per_chain_stats)
{
	int i;
	int8_t bcn_snr, dat_snr;

	for (i = 0; i < NUM_CHAINS_MAX; i++) {
		bcn_snr = rssi_stats->rssi_avg_beacon[i];
		dat_snr = rssi_stats->rssi_avg_data[i];
		WMA_LOGD("chain %d beacon snr %d data snr %d",
			i, bcn_snr, dat_snr);
		if ((dat_snr != WMA_TGT_INVALID_SNR_OLD) &&
			(dat_snr != WMA_TGT_INVALID_SNR_NEW))
			rssi_per_chain_stats->rssi[i] = dat_snr;
		else if ((bcn_snr != WMA_TGT_INVALID_SNR_OLD) &&
			(bcn_snr != WMA_TGT_INVALID_SNR_NEW))
			rssi_per_chain_stats->rssi[i] = bcn_snr;
		else
			/*
			 * Firmware sends invalid snr till it sees
			 * Beacon/Data after connection since after
			 * vdev up fw resets the snr to invalid.
			 * In this duartion Host will return an invalid rssi
			 * value.
			 */
			rssi_per_chain_stats->rssi[i] = WMA_TGT_RSSI_INVALID;

		/*
		 * Get the absolute rssi value from the current rssi value the
		 * sinr value is hardcoded into 0 in the qcacld-new/CORE stack
		 */
		rssi_per_chain_stats->rssi[i] += WMA_TGT_NOISE_FLOOR_DBM;
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&(rssi_stats->peer_macaddr),
			rssi_per_chain_stats->peer_mac_addr);
	}
}

/**
 * wma_update_rssi_stats() - to update rssi stats for all vdevs
 *         for which the stats were requested.
 * @wma: wma handle
 * @rssi_stats: rssi stats
 *
 * This function updates the rssi stats for all vdevs for which
 * the stats were requested.
 *
 * Return: void
 */
static void wma_update_rssi_stats(tp_wma_handle wma,
			wmi_rssi_stats *rssi_stats)
{
	tAniGetPEStatsRsp *stats_rsp_params;
	struct csr_per_chain_rssi_stats_info *rssi_per_chain_stats = NULL;
	struct wma_txrx_node *node;
	uint8_t *stats_buf;
	uint32_t temp_mask;
	uint8_t vdev_id;

	vdev_id = rssi_stats->vdev_id;
	node = &wma->interfaces[vdev_id];
	stats_rsp_params = (tAniGetPEStatsRsp *) node->stats_rsp;
	if (stats_rsp_params) {
		node->fw_stats_set |=  FW_RSSI_PER_CHAIN_STATS_SET;
		WMA_LOGD("<-- FW RSSI PER CHAIN STATS received for vdevId:%d",
				vdev_id);
		stats_buf = (uint8_t *) (stats_rsp_params + 1);
		temp_mask = stats_rsp_params->statsMask;

		if (temp_mask & (1 << eCsrSummaryStats))
			stats_buf += sizeof(tCsrSummaryStatsInfo);
		if (temp_mask & (1 << eCsrGlobalClassAStats))
			stats_buf += sizeof(tCsrGlobalClassAStatsInfo);
		if (temp_mask & (1 << eCsrGlobalClassDStats))
			stats_buf += sizeof(tCsrGlobalClassDStatsInfo);

		if (temp_mask & (1 << csr_per_chain_rssi_stats)) {
			rssi_per_chain_stats =
			     (struct csr_per_chain_rssi_stats_info *)stats_buf;
			wma_update_per_chain_rssi_stats(wma, rssi_stats,
					rssi_per_chain_stats);
		}
	}
}


/**
 * wma_link_status_event_handler() - link status event handler
 * @handle: wma handle
 * @cmd_param_info: data from event
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_link_status_event_handler(void *handle, uint8_t *cmd_param_info,
				  uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_UPDATE_VDEV_RATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_vdev_rate_stats_event_fixed_param *event;
	wmi_vdev_rate_ht_info *ht_info;
	struct wma_txrx_node *intr = wma->interfaces;
	uint8_t link_status = LINK_STATUS_LEGACY;
	int i;

	param_buf =
	      (WMI_UPDATE_VDEV_RATE_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}

	event = (wmi_vdev_rate_stats_event_fixed_param *)
						param_buf->fixed_param;
	ht_info = (wmi_vdev_rate_ht_info *) param_buf->ht_info;

	WMA_LOGD("num_vdev_stats: %d", event->num_vdev_stats);
	for (i = 0; (i < event->num_vdev_stats) && ht_info; i++) {
		WMA_LOGD("%s vdevId:%d  tx_nss:%d rx_nss:%d tx_preamble:%d rx_preamble:%d",
			__func__, ht_info->vdevid, ht_info->tx_nss,
			ht_info->rx_nss, ht_info->tx_preamble,
			ht_info->rx_preamble);
		if (ht_info->vdevid < wma->max_bssid
		    && intr[ht_info->vdevid].plink_status_req) {
			if (ht_info->tx_nss || ht_info->rx_nss)
				link_status = LINK_STATUS_MIMO;

			if ((ht_info->tx_preamble == LINK_RATE_VHT) ||
			    (ht_info->rx_preamble == LINK_RATE_VHT))
				link_status |= LINK_STATUS_VHT;

			if (intr[ht_info->vdevid].nss == 2)
				link_status |= LINK_SUPPORT_MIMO;

			if (intr[ht_info->vdevid].rate_flags &
				(eHAL_TX_RATE_VHT20 | eHAL_TX_RATE_VHT40 |
				eHAL_TX_RATE_VHT80))
				link_status |= LINK_SUPPORT_VHT;

			wma_post_link_status(
					intr[ht_info->vdevid].plink_status_req,
					link_status);
			intr[ht_info->vdevid].plink_status_req = NULL;
			link_status = LINK_STATUS_LEGACY;
		}

		ht_info++;
	}

	return 0;
}

int wma_rso_cmd_status_event_handler(wmi_roam_event_fixed_param *wmi_event)
{
	struct rso_cmd_status *rso_status;
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS qdf_status;

	rso_status = qdf_mem_malloc(sizeof(*rso_status));
	if (!rso_status) {
		WMA_LOGE("%s: malloc fails for rso cmd status", __func__);
		return -ENOMEM;
	}

	rso_status->vdev_id = wmi_event->vdev_id;
	if (WMI_ROAM_NOTIF_SCAN_MODE_SUCCESS == wmi_event->notif)
		rso_status->status = true;
	else if (WMI_ROAM_NOTIF_SCAN_MODE_FAIL == wmi_event->notif)
		rso_status->status = false;
	sme_msg.type = eWNI_SME_RSO_CMD_STATUS_IND;
	sme_msg.bodyptr = rso_status;
	sme_msg.bodyval = 0;
	WMA_LOGI("%s: Post RSO cmd status to SME",  __func__);

	qdf_status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("%s: fail to post RSO cmd status to SME", __func__);
		qdf_mem_free(rso_status);
	}
	return 0;
}

/**
 * wma_stats_event_handler() - stats event handler
 * @handle: wma handle
 * @cmd_param_info: data from event
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_stats_event_handler(void *handle, uint8_t *cmd_param_info,
			    uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *event;
	wmi_pdev_stats *pdev_stats;
	wmi_vdev_stats *vdev_stats;
	wmi_peer_stats *peer_stats;
	wmi_rssi_stats *rssi_stats;
	wmi_per_chain_rssi_stats *rssi_event;
	struct wma_txrx_node *node;
	uint8_t i, *temp;
	wmi_congestion_stats *congestion_stats;
	tpAniSirGlobal mac;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	temp = (uint8_t *) param_buf->data;

	if (event->num_pdev_stats > 0) {
		for (i = 0; i < event->num_pdev_stats; i++) {
			pdev_stats = (wmi_pdev_stats *) temp;
			wma_update_pdev_stats(wma, pdev_stats);
			temp += sizeof(wmi_pdev_stats);
		}
	}

	if (event->num_vdev_stats > 0) {
		for (i = 0; i < event->num_vdev_stats; i++) {
			vdev_stats = (wmi_vdev_stats *) temp;
			wma_update_vdev_stats(wma, vdev_stats);
			temp += sizeof(wmi_vdev_stats);
		}
	}

	if (event->num_peer_stats > 0) {
		for (i = 0; i < event->num_peer_stats; i++) {
			peer_stats = (wmi_peer_stats *) temp;
			wma_update_peer_stats(wma, peer_stats);
			temp += sizeof(wmi_peer_stats);
		}
	}

	rssi_event = (wmi_per_chain_rssi_stats *) param_buf->chain_stats;
	if (rssi_event) {
		if (((rssi_event->tlv_header & 0xFFFF0000) >> 16 ==
			  WMITLV_TAG_STRUC_wmi_per_chain_rssi_stats) &&
			  ((rssi_event->tlv_header & 0x0000FFFF) ==
			  WMITLV_GET_STRUCT_TLVLEN(wmi_per_chain_rssi_stats))) {
			if (rssi_event->num_per_chain_rssi_stats > 0) {
				temp = (uint8_t *) rssi_event;
				temp += sizeof(*rssi_event);

				/* skip past struct array tlv header */
				temp += WMI_TLV_HDR_SIZE;

				for (i = 0;
				     i < rssi_event->num_per_chain_rssi_stats;
				     i++) {
					rssi_stats = (wmi_rssi_stats *)temp;
					wma_update_rssi_stats(wma, rssi_stats);
					temp += sizeof(wmi_rssi_stats);
				}
			}
		}
	}

	congestion_stats = (wmi_congestion_stats *) param_buf->congestion_stats;
	if (congestion_stats) {
		if (((congestion_stats->tlv_header & 0xFFFF0000) >> 16 ==
			  WMITLV_TAG_STRUC_wmi_congestion_stats) &&
			  ((congestion_stats->tlv_header & 0x0000FFFF) ==
			  WMITLV_GET_STRUCT_TLVLEN(wmi_congestion_stats))) {
			mac = cds_get_context(QDF_MODULE_ID_PE);
			if (!mac) {
				WMA_LOGE("%s: Invalid mac", __func__);
				return -EINVAL;
			}
			if (!mac->sme.congestion_cb) {
				WMA_LOGE("%s: Callback not registered",
					__func__);
				return -EINVAL;
			}
			WMA_LOGI("%s: congestion %d", __func__,
				congestion_stats->congestion);
			mac->sme.congestion_cb(mac->hHdd,
				congestion_stats->congestion,
				congestion_stats->vdev_id);
		}
	}

	for (i = 0; i < wma->max_bssid; i++) {
		node = &wma->interfaces[i];
		if (node->fw_stats_set & FW_PEER_STATS_SET)
			wma_post_stats(wma, node);
	}

	return 0;
}

/**
 * wma_send_link_speed() - send link speed to SME
 * @link_speed: link speed
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_send_link_speed(uint32_t link_speed)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx;
	tSirLinkSpeedInfo *ls_ind;

	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac_ctx) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ls_ind = (tSirLinkSpeedInfo *)qdf_mem_malloc(sizeof(tSirLinkSpeedInfo));
	if (!ls_ind) {
		WMA_LOGE("%s: Memory allocation failed.", __func__);
		qdf_status = QDF_STATUS_E_NOMEM;
	} else {
		ls_ind->estLinkSpeed = link_speed;
		if (mac_ctx->sme.pLinkSpeedIndCb)
			mac_ctx->sme.pLinkSpeedIndCb(ls_ind,
					mac_ctx->sme.pLinkSpeedCbContext);
		else
			WMA_LOGD("%s: pLinkSpeedIndCb is null", __func__);
		qdf_mem_free(ls_ind);

	}
	return qdf_status;
}

/**
 * wma_link_speed_event_handler() - link speed event handler
 * @handle: wma handle
 * @cmd_param_info: event data
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_link_speed_event_handler(void *handle, uint8_t *cmd_param_info,
				 uint32_t len)
{
	WMI_PEER_ESTIMATED_LINKSPEED_EVENTID_param_tlvs *param_buf;
	wmi_peer_estimated_linkspeed_event_fixed_param *event;
	QDF_STATUS qdf_status;

	param_buf = (WMI_PEER_ESTIMATED_LINKSPEED_EVENTID_param_tlvs *)
							 cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid linkspeed event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	qdf_status = wma_send_link_speed(event->est_linkspeed_kbps);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		return -EINVAL;
	return 0;
}

/**
 * wma_wni_cfg_dnld() - cfg download request
 * @handle: wma handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_wni_cfg_dnld(tp_wma_handle wma_handle)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	void *mac = cds_get_context(QDF_MODULE_ID_PE);

	WMA_LOGD("%s: Enter", __func__);

	if (NULL == mac) {
		WMA_LOGE("%s: Invalid context", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	process_cfg_download_req(mac);

	WMA_LOGD("%s: Exit", __func__);
	return qdf_status;
}

/**
 * wma_unified_debug_print_event_handler() - debug print event handler
 * @handle: wma handle
 * @datap: data pointer
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_unified_debug_print_event_handler(void *handle, uint8_t *datap,
					  uint32_t len)
{
	WMI_DEBUG_PRINT_EVENTID_param_tlvs *param_buf;
	uint8_t *data;
	uint32_t datalen;

	param_buf = (WMI_DEBUG_PRINT_EVENTID_param_tlvs *) datap;
	if (!param_buf) {
		WMA_LOGE("Get NULL point message from FW");
		return -ENOMEM;
	}
	data = param_buf->data;
	datalen = param_buf->num_data;

#ifdef BIG_ENDIAN_HOST
	{
		char dbgbuf[500] = { 0 };

		memcpy(dbgbuf, data, datalen);
		SWAPME(dbgbuf, datalen);
		WMA_LOGD("FIRMWARE:%s", dbgbuf);
		return 0;
	}
#else
	WMA_LOGD("FIRMWARE:%s", data);
	return 0;
#endif /* BIG_ENDIAN_HOST */
}

/**
 * wma_check_scan_in_progress() - check scan is progress or not
 * @handle: wma handle
 *
 * Return: true/false
 */
bool wma_check_scan_in_progress(WMA_HANDLE handle)
{
	tp_wma_handle wma = handle;

	return qdf_atomic_read(&wma->num_pending_scans) > 0;
}

/**
 * wma_is_sap_active() - check sap is active or not
 * @handle: wma handle
 *
 * Return: true/false
 */
bool wma_is_sap_active(tp_wma_handle wma_handle)
{
	int i;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (!wma_is_vdev_up(i))
			continue;
		if (wma_handle->interfaces[i].type == WMI_VDEV_TYPE_AP &&
		    wma_handle->interfaces[i].sub_type == 0)
			return true;
	}
	return false;
}

/**
 * wma_is_p2p_go_active() - check p2p go is active or not
 * @handle: wma handle
 *
 * Return: true/false
 */
bool wma_is_p2p_go_active(tp_wma_handle wma_handle)
{
	int i;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (!wma_is_vdev_up(i))
			continue;
		if (wma_handle->interfaces[i].type == WMI_VDEV_TYPE_AP &&
		    wma_handle->interfaces[i].sub_type ==
		    WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO)
			return true;
	}
	return false;
}

/**
 * wma_is_p2p_cli_active() - check p2p cli is active or not
 * @handle: wma handle
 *
 * Return: true/false
 */
bool wma_is_p2p_cli_active(tp_wma_handle wma_handle)
{
	int i;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (!wma_is_vdev_up(i))
			continue;
		if (wma_handle->interfaces[i].type == WMI_VDEV_TYPE_STA &&
		    wma_handle->interfaces[i].sub_type ==
		    WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT)
			return true;
	}
	return false;
}

/**
 * wma_is_sta_active() - check sta is active or not
 * @handle: wma handle
 *
 * Return: true/false
 */
bool wma_is_sta_active(tp_wma_handle wma_handle)
{
	int i;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (!wma_is_vdev_up(i))
			continue;
		if (wma_handle->interfaces[i].type == WMI_VDEV_TYPE_STA &&
		    wma_handle->interfaces[i].sub_type == 0)
			return true;
		if (wma_handle->interfaces[i].type == WMI_VDEV_TYPE_IBSS)
			return true;
	}
	return false;
}

/**
 * wma_peer_phymode() - get phymode
 * @nw_type: nw type
 * @sta_type: sta type
 * @is_ht: is ht supported
 * @ch_width: supported channel width
 * @is_vht: is vht supported
 * @is_he: is HE supported
 *
 * Return: WLAN_PHY_MODE
 */
WLAN_PHY_MODE wma_peer_phymode(tSirNwType nw_type, uint8_t sta_type,
			       uint8_t is_ht, uint8_t ch_width,
			       uint8_t is_vht, bool is_he)
{
	WLAN_PHY_MODE phymode = MODE_UNKNOWN;

	switch (nw_type) {
	case eSIR_11B_NW_TYPE:
		phymode = MODE_11B;
		if (is_ht || is_vht || is_he)
			WMA_LOGE("HT/VHT is enabled with 11B NW type");
		break;
	case eSIR_11G_NW_TYPE:
		if (!(is_ht || is_vht || is_he)) {
			phymode = MODE_11G;
			break;
		}
		if (CH_WIDTH_40MHZ < ch_width)
			WMA_LOGE("80/160 MHz BW sent in 11G, configured 40MHz");
		if (ch_width)
			phymode = (is_he) ? MODE_11AX_HE40_2G : (is_vht) ?
					MODE_11AC_VHT40_2G : MODE_11NG_HT40;
		else
			phymode = (is_he) ? MODE_11AX_HE20_2G : (is_vht) ?
					MODE_11AC_VHT20_2G : MODE_11NG_HT20;
		break;
	case eSIR_11A_NW_TYPE:
		if (!(is_ht || is_vht || is_he)) {
			phymode = MODE_11A;
			break;
		}
		if (is_he) {
			if (ch_width == CH_WIDTH_160MHZ)
				phymode = MODE_11AX_HE160;
			else if (ch_width == CH_WIDTH_80P80MHZ)
				phymode = MODE_11AX_HE80_80;
			else if (ch_width == CH_WIDTH_80MHZ)
				phymode = MODE_11AX_HE80;
			else
				phymode = (ch_width) ?
					  MODE_11AX_HE40 : MODE_11AX_HE20;
		} else if (is_vht) {
			if (ch_width == CH_WIDTH_160MHZ)
				phymode = MODE_11AC_VHT160;
			else if (ch_width == CH_WIDTH_80P80MHZ)
				phymode = MODE_11AC_VHT80_80;
			else if (ch_width == CH_WIDTH_80MHZ)
				phymode = MODE_11AC_VHT80;
			else
				phymode = (ch_width) ?
					  MODE_11AC_VHT40 : MODE_11AC_VHT20;
		} else
			phymode = (ch_width) ? MODE_11NA_HT40 : MODE_11NA_HT20;
		break;
	default:
		WMA_LOGE("%s: Invalid nw type %d", __func__, nw_type);
		break;
	}
	WMA_LOGD(FL("nw_type %d is_ht %d ch_width %d is_vht %d is_he %d phymode %d"),
		 nw_type, is_ht, ch_width, is_vht, is_he, phymode);

	return phymode;
}

/**
 * wma_txrx_fw_stats_reset() - reset txrx fw statistics
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: 0 for success or return error
 */
int32_t wma_txrx_fw_stats_reset(tp_wma_handle wma_handle,
				uint8_t vdev_id, uint32_t value)
{
	struct ol_txrx_stats_req req;
	struct cdp_vdev *vdev;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		return -EINVAL;
	}

	vdev = wma_find_vdev_by_id(wma_handle, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		return -EINVAL;
	}
	qdf_mem_zero(&req, sizeof(req));
	req.stats_type_reset_mask = value;
	cdp_fw_stats_get(soc, vdev, &req, false, false);

	return 0;
}

#ifdef HELIUMPLUS
#define SET_UPLOAD_MASK(_mask, _rate_info)	\
	((_mask) = 1 << (_rate_info ## _V2))
#else  /* !HELIUMPLUS */
#define SET_UPLOAD_MASK(_mask, _rate_info)	\
	((_mask) = 1 << (_rate_info))
#endif

#ifdef HELIUMPLUS
static bool wma_is_valid_fw_stats_cmd(uint32_t value)
{
	if (value > (HTT_DBG_NUM_STATS + 1) ||
		value == (HTT_DBG_STATS_RX_RATE_INFO + 1) ||
		value == (HTT_DBG_STATS_TX_RATE_INFO + 1) ||
		value == (HTT_DBG_STATS_TXBF_MUSU_NDPA_PKT + 1)) {
		WMA_LOGE("%s: Not supported", __func__);
		return false;
	}
	return true;
}
#else
static bool wma_is_valid_fw_stats_cmd(uint32_t value)
{
	if (value > (HTT_DBG_NUM_STATS + 1) ||
		value == (HTT_DBG_STATS_RX_RATE_INFO_V2 + 1) ||
		value == (HTT_DBG_STATS_TX_RATE_INFO_V2 + 1) ||
		value == (HTT_DBG_STATS_TXBF_MUSU_NDPA_PKT + 1)) {
		WMA_LOGE("%s: Not supported", __func__);
		return false;
	}
	return true;
}
#endif

/**
 * wma_set_txrx_fw_stats_level() - set txrx fw stats level
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: 0 for success or return error
 */
int32_t wma_set_txrx_fw_stats_level(tp_wma_handle wma_handle,
				    uint8_t vdev_id, uint32_t value)
{
	struct ol_txrx_stats_req req;
	struct cdp_vdev *vdev;
	uint32_t l_up_mask;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		return -EINVAL;
	}

	vdev = wma_find_vdev_by_id(wma_handle, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		return -EINVAL;
	}

	if (wma_is_valid_fw_stats_cmd(value) == false)
		return -EINVAL;

	qdf_mem_zero(&req, sizeof(req));
	req.print.verbose = 1;

	/* TODO: Need to check how to avoid mem leak*/
	l_up_mask = 1 << (value - 1);
	req.stats_type_upload_mask = l_up_mask;

	cdp_fw_stats_get(soc, vdev, &req, false, true);

	return 0;
}

/**
 * wma_get_stats_rsp_buf() - fill get stats response buffer
 * @get_stats_param: get stats parameters
 *
 * Return: stats response buffer
 */
static tAniGetPEStatsRsp *wma_get_stats_rsp_buf
			(tAniGetPEStatsReq *get_stats_param)
{
	tAniGetPEStatsRsp *stats_rsp_params;
	uint32_t len, temp_mask;

	len = sizeof(tAniGetPEStatsRsp);
	temp_mask = get_stats_param->statsMask;

	if (temp_mask & (1 << eCsrSummaryStats))
		len += sizeof(tCsrSummaryStatsInfo);

	if (temp_mask & (1 << eCsrGlobalClassAStats))
		len += sizeof(tCsrGlobalClassAStatsInfo);

	if (temp_mask & (1 << eCsrGlobalClassDStats))
		len += sizeof(tCsrGlobalClassDStatsInfo);

	if (temp_mask & (1 << csr_per_chain_rssi_stats))
		len += sizeof(struct csr_per_chain_rssi_stats_info);

	stats_rsp_params = qdf_mem_malloc(len);
	if (!stats_rsp_params) {
		WMA_LOGE("memory allocation failed for tAniGetPEStatsRsp");
		QDF_ASSERT(0);
		return NULL;
	}

	stats_rsp_params->staId = get_stats_param->staId;
	stats_rsp_params->statsMask = get_stats_param->statsMask;
	stats_rsp_params->msgType = WMA_GET_STATISTICS_RSP;
	stats_rsp_params->msgLen = len - sizeof(tAniGetPEStatsRsp);
	stats_rsp_params->rc = QDF_STATUS_SUCCESS;
	return stats_rsp_params;
}

/**
 * wma_get_stats_req() - get stats request
 * @handle: wma handle
 * @get_stats_param: stats params
 *
 * Return: none
 */
void wma_get_stats_req(WMA_HANDLE handle,
		       tAniGetPEStatsReq *get_stats_param)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct wma_txrx_node *node;
	struct pe_stats_req  cmd = {0};
	tAniGetPEStatsRsp *pGetPEStatsRspParams;


	WMA_LOGD("%s: Enter", __func__);
	node = &wma_handle->interfaces[get_stats_param->sessionId];
	if (node->stats_rsp) {
		pGetPEStatsRspParams = node->stats_rsp;
		if (pGetPEStatsRspParams->staId == get_stats_param->staId &&
		    pGetPEStatsRspParams->statsMask ==
		    get_stats_param->statsMask) {
			WMA_LOGD("Stats for staId %d with stats mask %d is pending.. ignore new request",
				 get_stats_param->staId,
				 get_stats_param->statsMask);
			goto end;
		} else {
			qdf_mem_free(node->stats_rsp);
			node->stats_rsp = NULL;
			node->fw_stats_set = 0;
		}
	}

	pGetPEStatsRspParams = wma_get_stats_rsp_buf(get_stats_param);
	if (!pGetPEStatsRspParams)
		goto end;

	node->fw_stats_set = 0;
	if (node->stats_rsp) {
		WMA_LOGD(FL("stats_rsp is not null, prev_value: %p"),
			node->stats_rsp);
		qdf_mem_free(node->stats_rsp);
		node->stats_rsp = NULL;
	}
	node->stats_rsp = pGetPEStatsRspParams;
	WMA_LOGD("stats_rsp allocated: %p, sta_id: %d, mask: %d, vdev_id: %d",
		node->stats_rsp, node->stats_rsp->staId,
		node->stats_rsp->statsMask, get_stats_param->sessionId);

	cmd.session_id = get_stats_param->sessionId;
	if (wmi_unified_get_stats_cmd(wma_handle->wmi_handle, &cmd,
				 node->bssid)) {

		WMA_LOGE("%s: Failed to send WMI_REQUEST_STATS_CMDID",
			 __func__);
		goto failed;
	}

	goto end;
failed:

	pGetPEStatsRspParams->rc = QDF_STATUS_E_FAILURE;
	/* send response to UMAC */
	wma_send_msg(wma_handle, WMA_GET_STATISTICS_RSP, pGetPEStatsRspParams,
		     0);
	node->stats_rsp = NULL;
end:
	qdf_mem_free(get_stats_param);
	WMA_LOGD("%s: Exit", __func__);
}

/**
 * wma_get_cca_stats() - send request to fw to get CCA
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
QDF_STATUS wma_get_cca_stats(tp_wma_handle wma_handle,
				uint8_t vdev_id)
{
	if (wmi_unified_congestion_request_cmd(wma_handle->wmi_handle,
			vdev_id)) {
		WMA_LOGE("Failed to congestion request to fw");
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_beacon_buffer_by_vdev_id() - get the beacon buffer from vdev ID
 * @vdev_id: vdev id
 * @buffer_size: size of buffer
 *
 * Return: none
 */
void *wma_get_beacon_buffer_by_vdev_id(uint8_t vdev_id, uint32_t *buffer_size)
{
	tp_wma_handle wma;
	struct beacon_info *beacon;
	uint8_t *buf;
	uint32_t buf_size;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return NULL;
	}

	if (vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %u", __func__, vdev_id);
		return NULL;
	}

	if (!wma_is_vdev_in_ap_mode(wma, vdev_id)) {
		WMA_LOGE("%s: vdevid %d is not in AP mode", __func__, vdev_id);
		return NULL;
	}

	beacon = wma->interfaces[vdev_id].beacon;

	if (!beacon) {
		WMA_LOGE("%s: beacon invalid", __func__);
		return NULL;
	}

	qdf_spin_lock_bh(&beacon->lock);

	buf_size = qdf_nbuf_len(beacon->buf);
	buf = qdf_mem_malloc(buf_size);

	if (!buf) {
		qdf_spin_unlock_bh(&beacon->lock);
		WMA_LOGE("%s: alloc failed for beacon buf", __func__);
		return NULL;
	}

	qdf_mem_copy(buf, qdf_nbuf_data(beacon->buf), buf_size);

	qdf_spin_unlock_bh(&beacon->lock);

	if (buffer_size)
		*buffer_size = buf_size;

	return buf;
}

/**
 * wma_get_vdev_address_by_vdev_id() - lookup MAC address from vdev ID
 * @vdev_id: vdev id
 *
 * Return: mac address
 */
uint8_t *wma_get_vdev_address_by_vdev_id(uint8_t vdev_id)
{
	tp_wma_handle wma;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return NULL;
	}

	if (vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %u", __func__, vdev_id);
		return NULL;
	}

	return wma->interfaces[vdev_id].addr;
}

QDF_STATUS wma_get_connection_info(uint8_t vdev_id,
		struct policy_mgr_vdev_entry_info *conn_table_entry)
{
	struct wma_txrx_node *wma_conn_table_entry;

	wma_conn_table_entry = wma_get_interface_by_vdev_id(vdev_id);
	if (NULL == wma_conn_table_entry) {
		WMA_LOGE("%s: can't find vdev_id %d in WMA table", __func__, vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	conn_table_entry->chan_width = wma_conn_table_entry->chan_width;
	conn_table_entry->mac_id = wma_conn_table_entry->mac_id;
	conn_table_entry->mhz = wma_conn_table_entry->mhz;
	conn_table_entry->sub_type = wma_conn_table_entry->sub_type;
	conn_table_entry->type = wma_conn_table_entry->type;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_interface_by_vdev_id() - lookup interface entry using vdev ID
 * @vdev_id: vdev id
 *
 * Return: entry from vdev table
 */
struct wma_txrx_node  *wma_get_interface_by_vdev_id(uint8_t vdev_id)
{
	tp_wma_handle wma;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return NULL;
	}

	if (vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %u", __func__, vdev_id);
		return NULL;
	}

	return &wma->interfaces[vdev_id];
}

#if defined(QCA_WIFI_FTM)
/**
 * wma_utf_rsp() - utf response
 * @wma_handle: wma handle
 * @payload: payload
 * @len: length of payload
 *
 * Return: 0 for success or error code
 */
static int wma_utf_rsp(tp_wma_handle wma_handle, uint8_t **payload,
		       uint32_t *len)
{
	int ret = -1;
	uint32_t payload_len;

	payload_len = wma_handle->utf_event_info.length;
	if (payload_len) {
		ret = 0;

		/*
		 * The first 4 bytes holds the payload size
		 * and the actual payload sits next to it
		 */
		*payload = (uint8_t *) qdf_mem_malloc((uint32_t) payload_len
						      + sizeof(A_UINT32));
		*(A_UINT32 *) &(*payload[0]) =
			wma_handle->utf_event_info.length;
		memcpy(*payload + sizeof(A_UINT32),
		       wma_handle->utf_event_info.data, payload_len);
		wma_handle->utf_event_info.length = 0;
		*len = payload_len;
	}

	return ret;
}

/**
 * wma_post_ftm_response() - post ftm response to upper layer
 * @wma_handle: wma handle
 *
 * Return: none
 */
static void wma_post_ftm_response(tp_wma_handle wma_handle)
{
	int ret;
	uint8_t *payload;
	uint32_t data_len;
	struct scheduler_msg msg = { 0 };
	QDF_STATUS status;

	ret = wma_utf_rsp(wma_handle, &payload, &data_len);

	if (ret)
		return;

	sys_build_message_header(SYS_MSG_ID_FTM_RSP, &msg);
	msg.bodyptr = payload;
	msg.bodyval = 0;

	status = scheduler_post_msg(QDF_MODULE_ID_SYS, &msg);

	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("failed to post ftm response to SYS");
		qdf_mem_free(payload);
	}
}

/**
 * wma_process_utf_event() - process utf event
 * @handle: wma handle
 * @datap: data buffer
 * @dataplen: data length
 *
 * Return: 0 for success or error code
 */
static int
wma_process_utf_event(WMA_HANDLE handle, uint8_t *datap, uint32_t dataplen)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct seg_hdr_info segHdrInfo;
	uint8_t totalNumOfSegments, currentSeq;
	WMI_PDEV_UTF_EVENTID_param_tlvs *param_buf;
	uint8_t *data;
	uint32_t datalen;

	param_buf = (WMI_PDEV_UTF_EVENTID_param_tlvs *) datap;
	if (!param_buf) {
		WMA_LOGE("Get NULL point message from FW");
		return -EINVAL;
	}
	data = param_buf->data;
	datalen = param_buf->num_data;

	segHdrInfo = *(struct seg_hdr_info *) &(data[0]);

	wma_handle->utf_event_info.currentSeq = (segHdrInfo.segmentInfo & 0xF);

	currentSeq = (segHdrInfo.segmentInfo & 0xF);
	totalNumOfSegments = (segHdrInfo.segmentInfo >> 4) & 0xF;

	datalen = datalen - sizeof(segHdrInfo);

	if (currentSeq == 0) {
		wma_handle->utf_event_info.expectedSeq = 0;
		wma_handle->utf_event_info.offset = 0;
	} else {
		if (wma_handle->utf_event_info.expectedSeq != currentSeq)
			WMA_LOGE("Mismatch in expecting seq expected Seq %d got seq %d",
				 wma_handle->utf_event_info.expectedSeq,
				 currentSeq);
	}

	memcpy(&wma_handle->utf_event_info.
	       data[wma_handle->utf_event_info.offset],
	       &data[sizeof(segHdrInfo)], datalen);
	wma_handle->utf_event_info.offset =
		wma_handle->utf_event_info.offset + datalen;
	wma_handle->utf_event_info.expectedSeq++;

	if (wma_handle->utf_event_info.expectedSeq == totalNumOfSegments) {
		if (wma_handle->utf_event_info.offset != segHdrInfo.len)
			WMA_LOGE("All segs received total len mismatch.. len %zu total len %d",
				 wma_handle->utf_event_info.offset,
				 segHdrInfo.len);

		wma_handle->utf_event_info.length =
			wma_handle->utf_event_info.offset;
	}

	wma_post_ftm_response(wma_handle);

	return 0;
}

/**
 * wma_utf_detach() - utf detach
 * @wma_handle: wma handle
 *
 * Return: none
 */
void wma_utf_detach(tp_wma_handle wma_handle)
{
	if (wma_handle->utf_event_info.data) {
		qdf_mem_free(wma_handle->utf_event_info.data);
		wma_handle->utf_event_info.data = NULL;
		wma_handle->utf_event_info.length = 0;
		wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
						     WMI_PDEV_UTF_EVENTID);
	}
}

/**
 * wma_utf_attach() - utf attach
 * @wma_handle: wma handle
 *
 * Return: none
 */
void wma_utf_attach(tp_wma_handle wma_handle)
{
	int ret;

	wma_handle->utf_event_info.data = (unsigned char *)
					  qdf_mem_malloc(MAX_UTF_EVENT_LENGTH);
	wma_handle->utf_event_info.length = 0;

	ret = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						 WMI_PDEV_UTF_EVENTID,
						 wma_process_utf_event,
						 WMA_RX_SERIALIZER_CTX);

	if (ret)
		WMA_LOGE("%s: Failed to register UTF event callback", __func__);
}

/**
 * wma_utf_cmd() - utf command
 * @wma_handle: wma handle
 * @data: data
 * @len: length
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS wma_utf_cmd(tp_wma_handle wma_handle, uint8_t *data,
			      uint16_t len)
{
	struct pdev_utf_params param = {0};

	wma_handle->utf_event_info.length = 0;
	param.utf_payload = data;
	param.len = len;

	return wmi_unified_pdev_utf_cmd_send(wma_handle->wmi_handle, &param,
						WMA_WILDCARD_PDEV_ID);
}

/**
 * wma_process_ftm_command() - process ftm command
 * @wma_handle: wma handle
 * @msg_buffer: message buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wma_process_ftm_command(tp_wma_handle wma_handle,
			struct ar6k_testmode_cmd_data *msg_buffer)
{
	uint8_t *data = NULL;
	uint16_t len = 0;
	int ret;

	if (!msg_buffer)
		return QDF_STATUS_E_INVAL;

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE) {
		WMA_LOGE("FTM command issued in non-FTM mode");
		qdf_mem_free(msg_buffer->data);
		qdf_mem_free(msg_buffer);
		return QDF_STATUS_E_NOSUPPORT;
	}

	data = msg_buffer->data;
	len = msg_buffer->len;

	ret = wma_utf_cmd(wma_handle, data, len);

	qdf_mem_free(msg_buffer->data);
	qdf_mem_free(msg_buffer);

	if (ret)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_WIFI_FTM */

/**
 * wma_get_wcnss_software_version() - get wcnss software version
 * @p_cds_gctx: cds context
 * @pVersion: version pointer
 * @versionBufferSize: buffer size
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_get_wcnss_software_version(void *p_cds_gctx,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		return QDF_STATUS_E_FAULT;
	}

	snprintf(pVersion, versionBufferSize, "%x",
		 (unsigned int)wma_handle->target_fw_version);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_get_mac_id_of_vdev() - Get MAC id corresponding to a vdev
 * @vdev_id: VDEV whose MAC ID is required
 *
 * Get MAC id corresponding to a vdev id from the WMA structure
 *
 * Return: Negative value on failure and MAC id on success
 */
int8_t wma_get_mac_id_of_vdev(uint32_t vdev_id)
{
	tp_wma_handle wma;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return -EINVAL;
	}

	if (wma->interfaces)
		return wma->interfaces[vdev_id].mac_id;

	return -EINVAL;
}

/**
 * wma_update_intf_hw_mode_params() - Update WMA params
 * @vdev_id: VDEV id whose params needs to be updated
 * @mac_id: MAC id to be updated
 * @cfgd_hw_mode_index: HW mode index from which Tx and Rx SS will be updated
 *
 * Updates the MAC id, tx spatial stream, rx spatial stream in WMA
 *
 * Return: None
 */
void wma_update_intf_hw_mode_params(uint32_t vdev_id, uint32_t mac_id,
				    uint32_t cfgd_hw_mode_index)
{
	tp_wma_handle wma;
	uint32_t param;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return;
	}

	if (!wma->interfaces) {
		WMA_LOGE("%s: Interface is NULL", __func__);
		return;
	}

	if (cfgd_hw_mode_index > wma->num_dbs_hw_modes) {
		WMA_LOGE("%s: Invalid index", __func__);
		return;
	}

	param = wma->hw_mode.hw_mode_list[cfgd_hw_mode_index];
	wma->interfaces[vdev_id].mac_id = mac_id;
	if (mac_id == 0) {
		wma->interfaces[vdev_id].tx_streams =
			WMA_HW_MODE_MAC0_TX_STREAMS_GET(param);
		wma->interfaces[vdev_id].rx_streams =
			WMA_HW_MODE_MAC0_RX_STREAMS_GET(param);
	} else {
		wma->interfaces[vdev_id].tx_streams =
			WMA_HW_MODE_MAC1_TX_STREAMS_GET(param);
		wma->interfaces[vdev_id].rx_streams =
			WMA_HW_MODE_MAC1_RX_STREAMS_GET(param);
	}
}

/**
 * wma_get_vht_ch_width - return vht channel width
 *
 * Return: return vht channel width
 */
uint32_t wma_get_vht_ch_width(void)
{
	uint32_t fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
	tp_wma_handle wm_hdl = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wm_hdl)
		return fw_ch_wd;

	if (wm_hdl->vht_cap_info & WMI_VHT_CAP_CH_WIDTH_80P80_160MHZ)
		fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ;
	else if (wm_hdl->vht_cap_info & WMI_VHT_CAP_CH_WIDTH_160MHZ)
		fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;

	return fw_ch_wd;
}

/**
 * wma_get_num_of_setbits_from_bitmask() - to get num of setbits from bitmask
 * @mask: given bitmask
 *
 * This helper function should return number of setbits from bitmask
 *
 * Return: number of setbits from bitmask
 */
uint32_t wma_get_num_of_setbits_from_bitmask(uint32_t mask)
{
	uint32_t num_of_setbits = 0;

	while (mask) {
		mask &= (mask - 1);
		num_of_setbits++;
	}
	return num_of_setbits;
}

/**
 * wma_config_debug_module_cmd - set debug log config
 * @wmi_handle: wmi layer handle
 * @param: debug log parameter
 * @val: debug log value
 * @module_id_bitmap: debug module id bitmap
 * @bitmap_len:  debug module bitmap length
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wma_config_debug_module_cmd(wmi_unified_t wmi_handle, A_UINT32 param,
			    A_UINT32 val, A_UINT32 *module_id_bitmap,
			    A_UINT32 bitmap_len)
{
	struct dbglog_params dbg_param;

	dbg_param.param = param;
	dbg_param.val = val;
	dbg_param.module_id_bitmap = module_id_bitmap;
	dbg_param.bitmap_len = bitmap_len;

	return wmi_unified_dbglog_cmd_send(wmi_handle, &dbg_param);
}

/**
 * wma_is_p2p_lo_capable() - if driver is capable of p2p listen offload
 *
 * This function checks if driver is capable of p2p listen offload
 *    true: capable of p2p offload
 *    false: not capable
 *
 * Return: true - capable, false - not capable
 */
bool wma_is_p2p_lo_capable(void)
{
	tp_wma_handle wma;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
			WMI_SERVICE_P2P_LISTEN_OFFLOAD_SUPPORT))
		return true;

	return false;
}

bool wma_is_vdev_up(uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	tp_wma_handle wma = (tp_wma_handle)cds_get_context(QDF_MODULE_ID_WMA);
	enum wlan_vdev_state state = WLAN_VDEV_S_INIT;

	if (!wma) {
		WMA_LOGE("%s: WMA context is invald!", __func__);
		return false;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(wma->psoc, vdev_id,
			WLAN_LEGACY_WMA_ID);
	if (vdev) {
		wlan_vdev_obj_lock(vdev);
		state = wlan_vdev_mlme_get_state(vdev);
		wlan_vdev_obj_unlock(vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
	}
	return (state == WLAN_VDEV_S_RUN) ? true : false;
}

void wma_acquire_wmi_resp_wakelock(t_wma_handle *wma, uint32_t msec)
{
	cds_host_diag_log_work(&wma->wmi_cmd_rsp_wake_lock,
			       msec,
			       WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
	qdf_wake_lock_timeout_acquire(&wma->wmi_cmd_rsp_wake_lock, msec);
	qdf_runtime_pm_prevent_suspend(wma->wmi_cmd_rsp_runtime_lock);
}

void wma_release_wmi_resp_wakelock(t_wma_handle *wma)
{
	qdf_wake_lock_release(&wma->wmi_cmd_rsp_wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP);
	qdf_runtime_pm_allow_suspend(wma->wmi_cmd_rsp_runtime_lock);
}

QDF_STATUS
wma_send_vdev_start_to_fw(t_wma_handle *wma, struct vdev_start_params *params)
{
	QDF_STATUS status;

	wma_acquire_wmi_resp_wakelock(wma, WMA_VDEV_START_REQUEST_TIMEOUT);
	status = wmi_unified_vdev_start_send(wma->wmi_handle, params);
	if (QDF_IS_STATUS_ERROR(status))
		wma_release_wmi_resp_wakelock(wma);

	return status;
}

QDF_STATUS wma_send_vdev_stop_to_fw(t_wma_handle *wma, uint8_t vdev_id)
{
	QDF_STATUS status;

	wma_acquire_wmi_resp_wakelock(wma, WMA_VDEV_STOP_REQUEST_TIMEOUT);
	status = wmi_unified_vdev_stop_send(wma->wmi_handle, vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		wma_release_wmi_resp_wakelock(wma);

	return status;
}

bool wma_is_service_enabled(WMI_SERVICE service_type)
{
	tp_wma_handle wma;
	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	return WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap, service_type);
}
