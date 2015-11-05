/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include "ol_txrx_ctrl_api.h"
#include "wlan_tgt_def_config.h"

#include "cdf_nbuf.h"
#include "cdf_types.h"
#include "ol_txrx_api.h"
#include "cdf_memory.h"
#include "ol_txrx_types.h"
#include "ol_txrx_peer_find.h"

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

#include "dfs.h"
#include "wma_internal.h"
#include "cds_concurrency.h"

#include "linux/ieee80211.h"

/* MCS Based rate table */
/* HT MCS parameters with Nss = 1 */
static struct index_data_rate_type supported_mcs_rate_nss1[] = {
	/* MCS  L20   L40   S20  S40 */
	{0, {65, 135, 72, 150} },
	{1, {130, 270, 144, 300} },
	{2, {195, 405, 217, 450} },
	{3, {260, 540, 289, 600} },
	{4, {390, 810, 433, 900} },
	{5, {520, 1080, 578, 1200} },
	{6, {585, 1215, 650, 1350} },
	{7, {650, 1350, 722, 1500} }
};

/* HT MCS parameters with Nss = 2 */
static struct index_data_rate_type supported_mcs_rate_nss2[] = {
	/* MCS  L20    L40   S20   S40 */
	{0, {130, 270, 144, 300} },
	{1, {260, 540, 289, 600} },
	{2, {390, 810, 433, 900} },
	{3, {520, 1080, 578, 1200} },
	{4, {780, 1620, 867, 1800} },
	{5, {1040, 2160, 1156, 2400} },
	{6, {1170, 2430, 1300, 2700} },
	{7, {1300, 2700, 1444, 3000} }
};

#ifdef WLAN_FEATURE_11AC
/* MCS Based VHT rate table */
/* MCS parameters with Nss = 1*/
static struct index_vht_data_rate_type supported_vht_mcs_rate_nss1[] = {
	/* MCS  L80    S80     L40   S40    L20   S40 */
	{0, {293, 325}, {135, 150}, {65, 72} },
	{1, {585, 650}, {270, 300}, {130, 144} },
	{2, {878, 975}, {405, 450}, {195, 217} },
	{3, {1170, 1300}, {540, 600}, {260, 289} },
	{4, {1755, 1950}, {810, 900}, {390, 433} },
	{5, {2340, 2600}, {1080, 1200}, {520, 578} },
	{6, {2633, 2925}, {1215, 1350}, {585, 650} },
	{7, {2925, 3250}, {1350, 1500}, {650, 722} },
	{8, {3510, 3900}, {1620, 1800}, {780, 867} },
	{9, {3900, 4333}, {1800, 2000}, {780, 867} }
};

/*MCS parameters with Nss = 2*/
static struct index_vht_data_rate_type supported_vht_mcs_rate_nss2[] = {
	/* MCS  L80    S80     L40   S40    L20   S40 */
	{0, {585, 650}, {270, 300}, {130, 144} },
	{1, {1170, 1300}, {540, 600}, {260, 289} },
	{2, {1755, 1950}, {810, 900}, {390, 433} },
	{3, {2340, 2600}, {1080, 1200}, {520, 578} },
	{4, {3510, 3900}, {1620, 1800}, {780, 867} },
	{5, {4680, 5200}, {2160, 2400}, {1040, 1156} },
	{6, {5265, 5850}, {2430, 2700}, {1170, 1300} },
	{7, {5850, 6500}, {2700, 3000}, {1300, 1444} },
	{8, {7020, 7800}, {3240, 3600}, {1560, 1733} },
	{9, {7800, 8667}, {3600, 4000}, {1560, 1733} }
};
#endif /* WLAN_FEATURE_11AC */

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
	for (i = 0; i < no_words; i++) {
		*(word_ptr + i) = __cpu_to_le32(*(word_ptr + i));
	}
}

#define SWAPME(x, len) wma_swap_bytes(&x, len);
#endif /* BIG_ENDIAN_HOST */

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
	uint8_t rateFlag = 0, curIdx = 0;
	uint16_t curRate;
	bool found = false;
#ifdef WLAN_FEATURE_11AC
	struct index_vht_data_rate_type *supported_vht_mcs_rate;
#endif /* WLAN_FEATURE_11AC */
	struct index_data_rate_type *supported_mcs_rate;

	WMA_LOGD("%s rate:%d rate_flgs:%d", __func__, maxRate, rate_flags);
#ifdef WLAN_FEATURE_11AC
	supported_vht_mcs_rate = (struct index_vht_data_rate_type *)
				 ((nss == 1) ? &supported_vht_mcs_rate_nss1 :
				  &supported_vht_mcs_rate_nss2);
#endif /* WLAN_FEATURE_11AC */
	supported_mcs_rate = (struct index_data_rate_type *)
			     ((nss == 1) ? &supported_mcs_rate_nss1 : &supported_mcs_rate_nss2);

	*mcsRateFlag = rate_flags;
	*mcsRateFlag &= ~eHAL_TX_RATE_SGI;
#ifdef WLAN_FEATURE_11AC
	if (rate_flags &
	    (eHAL_TX_RATE_VHT20 | eHAL_TX_RATE_VHT40 | eHAL_TX_RATE_VHT80)) {

		if (rate_flags & eHAL_TX_RATE_VHT80) {
			for (curIdx = 0; curIdx < MAX_VHT_MCS_IDX; curIdx++) {
				rateFlag = 0;
				if (curIdx >= 7) {
					if (rate_flags & eHAL_TX_RATE_SGI)
						rateFlag |= 0x1;
				}

				curRate = supported_vht_mcs_rate[curIdx].supported_VHT80_rate[rateFlag];
				if (curRate == maxRate) {
					found = true;
					break;
				}
			}
		}

		if ((found == false) &&
		    ((rate_flags & eHAL_TX_RATE_VHT80) ||
		     (rate_flags & eHAL_TX_RATE_VHT40))) {
			for (curIdx = 0; curIdx < MAX_VHT_MCS_IDX; curIdx++) {
				rateFlag = 0;
				if (curIdx >= 7) {
					if (rate_flags & eHAL_TX_RATE_SGI)
						rateFlag |= 0x1;
				}

				curRate = supported_vht_mcs_rate[curIdx].supported_VHT40_rate[rateFlag];
				if (curRate == maxRate) {
					found = true;
					*mcsRateFlag &= ~eHAL_TX_RATE_VHT80;
					break;
				}
			}
		}

		if ((found == false) &&
		    ((rate_flags & eHAL_TX_RATE_VHT80) ||
		     (rate_flags & eHAL_TX_RATE_VHT40) ||
		     (rate_flags & eHAL_TX_RATE_VHT20))) {
			for (curIdx = 0; curIdx < MAX_VHT_MCS_IDX; curIdx++) {
				rateFlag = 0;
				if (curIdx >= 7) {
					if (rate_flags & eHAL_TX_RATE_SGI)
						rateFlag |= 0x1;
				}

				curRate = supported_vht_mcs_rate[curIdx].supported_VHT20_rate[rateFlag];
				if (curRate == maxRate) {
					found = true;
					*mcsRateFlag &=
						~(eHAL_TX_RATE_VHT80 |
						  eHAL_TX_RATE_VHT40);
					break;
				}
			}
		}
	}
#endif /* WLAN_FEATURE_11AC */
	if ((found == false) &&
	    (rate_flags & (eHAL_TX_RATE_HT40 | eHAL_TX_RATE_HT20))) {
		if (rate_flags & eHAL_TX_RATE_HT40) {
			rateFlag = 0x1;

			for (curIdx = 0; curIdx < MAX_HT_MCS_IDX; curIdx++) {
				if (curIdx == 7) {
					if (rate_flags & eHAL_TX_RATE_SGI)
						rateFlag |= 0x2;
				}

				curRate = supported_mcs_rate[curIdx].supported_rate[rateFlag];
				if (curRate == maxRate) {
					found = true;
					*mcsRateFlag = eHAL_TX_RATE_HT40;
					break;
				}
			}
		}

		if (found == false) {
			rateFlag = 0;
			for (curIdx = 0; curIdx < MAX_HT_MCS_IDX; curIdx++) {
				if (curIdx == 7) {
					if (rate_flags & eHAL_TX_RATE_SGI)
						rateFlag |= 0x2;
				}

				curRate = supported_mcs_rate[curIdx].supported_rate[rateFlag];
				if (curRate == maxRate) {
					found = true;
					*mcsRateFlag = eHAL_TX_RATE_HT20;
					break;
				}
			}
		}
	}

	/*SGI rates are used by firmware only for MCS >= 7 */
	if (found && (curIdx >= 7))
		*mcsRateFlag |= eHAL_TX_RATE_SGI;

	return  found ? curIdx : INVALID_MCS_IDX;
}

/**
 * host_map_smps_mode() - map fw smps mode to tSmpsModeValue
 * @fw_smps_mode: fw smps mode
 *
 * Return: return tSmpsModeValue
 */
tSmpsModeValue host_map_smps_mode(A_UINT32 fw_smps_mode)
{
	tSmpsModeValue smps_mode = SMPS_MODE_DISABLED;
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
	CDF_STATUS status;
	cds_msg_t cds_msg;
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

	stats_ext_event = (tSirStatsExtEvent *) cdf_mem_malloc(alloc_len);
	if (NULL == stats_ext_event) {
		WMA_LOGE("%s: Memory allocation failure", __func__);
		return -ENOMEM;
	}

	buf_ptr += sizeof(wmi_stats_ext_event_fixed_param) + WMI_TLV_HDR_SIZE;

	stats_ext_event->vdev_id = stats_ext_info->vdev_id;
	stats_ext_event->event_data_len = stats_ext_info->data_len;
	cdf_mem_copy(stats_ext_event->event_data,
		     buf_ptr, stats_ext_event->event_data_len);

	cds_msg.type = eWNI_SME_STATS_EXT_EVENT;
	cds_msg.bodyptr = (void *)stats_ext_event;
	cds_msg.bodyval = 0;

	status = cds_mq_post_message(CDS_MQ_ID_SME, &cds_msg);
	if (status != CDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to post stats ext event to SME", __func__);
		cdf_mem_free(stats_ext_event);
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
	CDF_TRACE(CDF_MODULE_ID_WMA, CDF_TRACE_LEVEL_ERROR,
				"Profile data stats\n");
	CDF_TRACE(CDF_MODULE_ID_WMA, CDF_TRACE_LEVEL_ERROR,
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

	for (i = 0; i < entries; i++) {
		if (i == WMI_WLAN_PROFILE_MAX_BIN_CNT)
			break;
		CDF_TRACE(CDF_MODULE_ID_WMA, CDF_TRACE_LEVEL_ERROR,
			"Profile ID: %d\n"
			"Profile Count: %d\n"
			"Profile TOT: %d\n"
			"Profile Min: %d\n"
			"Profile Max: %d\n"
			"Profile hist_intvl: %d\n"
			"Profile hist[0]: %d\n"
			"Profile hist[1]: %d\n"
			"Profile hist[2]: %d\n",
			profile_data[i].id,
			profile_data[i].cnt,
			profile_data[i].tot,
			profile_data[i].min,
			profile_data[i].max,
			profile_data[i].hist_intvl,
			profile_data[i].hist[0],
			profile_data[i].hist[1],
			profile_data[i].hist[2]);
	}

	return 0;
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

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
	uint32_t count, num_rates = 0;
	uint32_t next_res_offset, next_peer_offset, next_rate_offset;
	size_t peer_info_size, peer_stats_size, rate_stats_size;
	size_t link_stats_results_size;

	tpAniSirGlobal pMac = cds_get_context(CDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: Posting Peer Stats event to HDD", __func__);
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

	link_stats_results = cdf_mem_malloc(link_stats_results_size);
	if (NULL == link_stats_results) {
		WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
			 __func__, link_stats_results_size);
		return -ENOMEM;
	}

	WMA_LOGD("Peer stats from FW event buf");
	WMA_LOGD("Fixed Param:");
	WMA_LOGD("request_id %u num_peers %u peer_event_number %u more_data %u",
		 fixed_param->request_id, fixed_param->num_peers,
		 fixed_param->peer_event_number, fixed_param->more_data);

	cdf_mem_zero(link_stats_results, link_stats_results_size);

	link_stats_results->paramId = WMI_LINK_STATS_ALL_PEER;
	link_stats_results->rspId = fixed_param->request_id;
	link_stats_results->ifaceId = 0;
	link_stats_results->num_peers = fixed_param->num_peers;
	link_stats_results->peer_event_number = fixed_param->peer_event_number;
	link_stats_results->moreResultToFollow = fixed_param->more_data;

	cdf_mem_copy(link_stats_results->results,
		     &fixed_param->num_peers, peer_stats_size);

	results = (uint8_t *) link_stats_results->results;
	t_peer_stats = (uint8_t *) peer_stats;
	t_rate_stats = (uint8_t *) rate_stats;
	next_res_offset = peer_stats_size;
	next_peer_offset = WMI_TLV_HDR_SIZE;
	next_rate_offset = WMI_TLV_HDR_SIZE;
	for (count = 0; count < fixed_param->num_peers; count++) {
		WMA_LOGD("Peer Info:");
		WMA_LOGD("peer_type %u capabilities %u num_rates %u",
			 peer_stats->peer_type, peer_stats->capabilities,
			 peer_stats->num_rates);

		cdf_mem_copy(results + next_res_offset,
			     t_peer_stats + next_peer_offset, peer_info_size);
		next_res_offset += peer_info_size;

		/* Copy rate stats associated with this peer */
		for (count = 0; count < peer_stats->num_rates; count++) {
			WMA_LOGD("Rate Stats Info:");
			WMA_LOGD("rate %u bitrate %u tx_mpdu %u rx_mpdu %u "
				 "mpdu_lost %u retries %u retries_short %u "
				 "retries_long %u", rate_stats->rate,
				 rate_stats->bitrate, rate_stats->tx_mpdu,
				 rate_stats->rx_mpdu, rate_stats->mpdu_lost,
				 rate_stats->retries, rate_stats->retries_short,
				 rate_stats->retries_long);
			rate_stats++;

			cdf_mem_copy(results + next_res_offset,
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
	WMA_LOGD("%s: Peer Stats event posted to HDD", __func__);
	cdf_mem_free(link_stats_results);

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
	WMI_RADIO_LINK_STATS_EVENTID_param_tlvs *param_tlvs;
	wmi_radio_link_stats_event_fixed_param *fixed_param;
	wmi_radio_link_stats *radio_stats;
	wmi_channel_stats *channel_stats;
	tSirLLStatsResults *link_stats_results;
	uint8_t *results, *t_radio_stats, *t_channel_stats;
	uint32_t next_res_offset, next_chan_offset, count;
	size_t radio_stats_size, chan_stats_size;
	size_t link_stats_results_size;

	tpAniSirGlobal pMac = cds_get_context(CDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: Posting Radio Stats event to HDD", __func__);
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
				  radio_stats_size + (radio_stats->num_channels * chan_stats_size);

	link_stats_results = cdf_mem_malloc(link_stats_results_size);
	if (NULL == link_stats_results) {
		WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
			 __func__, link_stats_results_size);
		return -ENOMEM;
	}

	WMA_LOGD("Radio stats from FW event buf");
	WMA_LOGD("Fixed Param:");
	WMA_LOGD("request_id %u num_radio %u more_radio_events %u",
		 fixed_param->request_id, fixed_param->num_radio,
		 fixed_param->more_radio_events);

	WMA_LOGD("Radio Info");
	WMA_LOGD("radio_id %u on_time %u tx_time %u rx_time %u on_time_scan %u "
		 "on_time_nbd %u on_time_gscan %u on_time_roam_scan %u "
		 "on_time_pno_scan %u on_time_hs20 %u num_channels %u",
		 radio_stats->radio_id, radio_stats->on_time,
		 radio_stats->tx_time, radio_stats->rx_time,
		 radio_stats->on_time_scan, radio_stats->on_time_nbd,
		 radio_stats->on_time_gscan,
		 radio_stats->on_time_roam_scan,
		 radio_stats->on_time_pno_scan,
		 radio_stats->on_time_hs20, radio_stats->num_channels);

	cdf_mem_zero(link_stats_results, link_stats_results_size);

	link_stats_results->paramId = WMI_LINK_STATS_RADIO;
	link_stats_results->rspId = fixed_param->request_id;
	link_stats_results->ifaceId = 0;
	link_stats_results->num_radio = fixed_param->num_radio;
	link_stats_results->peer_event_number = 0;
	link_stats_results->moreResultToFollow = fixed_param->more_radio_events;

	results = (uint8_t *) link_stats_results->results;
	t_radio_stats = (uint8_t *) radio_stats;
	t_channel_stats = (uint8_t *) channel_stats;

	cdf_mem_copy(results, t_radio_stats + WMI_TLV_HDR_SIZE,
		     radio_stats_size);

	next_res_offset = radio_stats_size;
	next_chan_offset = WMI_TLV_HDR_SIZE;
	WMA_LOGD("Channel Stats Info");
	for (count = 0; count < radio_stats->num_channels; count++) {
		WMA_LOGD("channel_width %u center_freq %u center_freq0 %u "
			 "center_freq1 %u radio_awake_time %u cca_busy_time %u",
			 channel_stats->channel_width,
			 channel_stats->center_freq,
			 channel_stats->center_freq0,
			 channel_stats->center_freq1,
			 channel_stats->radio_awake_time,
			 channel_stats->cca_busy_time);
		channel_stats++;

		cdf_mem_copy(results + next_res_offset,
			     t_channel_stats + next_chan_offset,
			     chan_stats_size);
		next_res_offset += chan_stats_size;
		next_chan_offset += sizeof(*channel_stats);
	}

	/* call hdd callback with Link Layer Statistics
	 * vdev_id/ifacId in link_stats_results will be
	 * used to retrieve the correct HDD context
	 */
	pMac->sme.pLinkLayerStatsIndCallback(pMac->hHdd,
					     WMA_LINK_LAYER_STATS_RESULTS_RSP,
					     link_stats_results);
	WMA_LOGD("%s: Radio Stats event posted to HDD", __func__);
	cdf_mem_free(link_stats_results);

	return 0;
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
					   wma_unified_link_iface_stats_event_handler);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_LINK_STATS_EVENTID,
					   wma_unified_link_peer_stats_event_handler);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_RADIO_LINK_STATS_EVENTID,
					   wma_unified_link_radio_stats_event_handler);

	return;
}


/**
 * wma_process_ll_stats_clear_req() - clear link layer stats
 * @wma: wma handle
 * @clearReq: ll stats clear request command params
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_process_ll_stats_clear_req
	(tp_wma_handle wma, const tpSirLLStatsClearReq clearReq)
{
	wmi_clear_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	if (!clearReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma->wmi_handle, len);

	if (!buf) {
		WMA_LOGE("%s: Failed allocate wmi buffer", __func__);
		return CDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cdf_mem_zero(buf_ptr, len);
	cmd = (wmi_clear_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_clear_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_clear_link_stats_cmd_fixed_param));

	cmd->stop_stats_collection_req = clearReq->stopReq;
	cmd->vdev_id = clearReq->staId;
	cmd->stats_clear_req_mask = clearReq->statsClearReqMask;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(wma->interfaces[clearReq->staId].addr,
				   &cmd->peer_macaddr);

	WMA_LOGD("LINK_LAYER_STATS - Clear Request Params");
	WMA_LOGD("StopReq         : %d", cmd->stop_stats_collection_req);
	WMA_LOGD("Vdev Id         : %d", cmd->vdev_id);
	WMA_LOGD("Clear Stat Mask : %d", cmd->stats_clear_req_mask);
	WMA_LOGD("Peer MAC Addr   : %pM",
		 wma->interfaces[clearReq->staId].addr);

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_CLEAR_LINK_STATS_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send clear link stats req", __func__);
		wmi_buf_free(buf);
		return CDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Clear Link Layer Stats request sent successfully");
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_process_ll_stats_set_req() - link layer stats set request
 * @wma:       wma handle
 * @setReq:  ll stats set request command params
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_process_ll_stats_set_req
	(tp_wma_handle wma, const tpSirLLStatsSetReq setReq)
{
	wmi_start_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	if (!setReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma->wmi_handle, len);

	if (!buf) {
		WMA_LOGE("%s: Failed allocate wmi buffer", __func__);
		return CDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cdf_mem_zero(buf_ptr, len);
	cmd = (wmi_start_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_start_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_link_stats_cmd_fixed_param));

	cmd->mpdu_size_threshold = setReq->mpduSizeThreshold;
	cmd->aggressive_statistics_gathering =
		setReq->aggressiveStatisticsGathering;

	WMA_LOGD("LINK_LAYER_STATS - Start/Set Request Params");
	WMA_LOGD("MPDU Size Thresh : %d", cmd->mpdu_size_threshold);
	WMA_LOGD("Aggressive Gather: %d", cmd->aggressive_statistics_gathering);

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_START_LINK_STATS_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send set link stats request", __func__);
		wmi_buf_free(buf);
		return CDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Set Link Layer Stats request sent successfully");
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_process_ll_stats_get_req() - link layer stats get request
 * @wma:wma handle
 * @getReq:ll stats get request command params
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_process_ll_stats_get_req
	(tp_wma_handle wma, const tpSirLLStatsGetReq getReq)
{
	wmi_request_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	if (!getReq || !wma) {
		WMA_LOGE("%s: input pointer is NULL", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wma->wmi_handle, len);

	if (!buf) {
		WMA_LOGE("%s: Failed allocate wmi buffer", __func__);
		return CDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cdf_mem_zero(buf_ptr, len);
	cmd = (wmi_request_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_link_stats_cmd_fixed_param));

	cmd->request_id = getReq->reqId;
	cmd->stats_type = getReq->paramIdMask;
	cmd->vdev_id = getReq->staId;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(wma->interfaces[getReq->staId].addr,
				   &cmd->peer_macaddr);

	WMA_LOGD("LINK_LAYER_STATS - Get Request Params");
	WMA_LOGD("Request ID      : %d", cmd->request_id);
	WMA_LOGD("Stats Type      : %d", cmd->stats_type);
	WMA_LOGD("Vdev ID         : %d", cmd->vdev_id);
	WMA_LOGD("Peer MAC Addr   : %pM", wma->interfaces[getReq->staId].addr);

	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_REQUEST_LINK_STATS_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send get link stats request", __func__);
		wmi_buf_free(buf);
		return CDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("Get Link Layer Stats request sent successfully");
	return CDF_STATUS_SUCCESS;
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

	tpAniSirGlobal pMac = cds_get_context(CDF_MODULE_ID_PE);

	if (!pMac) {
		WMA_LOGD("%s: NULL pMac ptr. Exiting", __func__);
		return -EINVAL;
	}

	if (!pMac->sme.pLinkLayerStatsIndCallback) {
		WMA_LOGD("%s: HDD callback is null", __func__);
		return -EINVAL;
	}

	WMA_LOGD("%s: Posting Iface Stats event to HDD", __func__);
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

	link_stats_results = cdf_mem_malloc(link_stats_results_size);
	if (!link_stats_results) {
		WMA_LOGD("%s: could not allocate mem for stats results-len %zu",
			 __func__, link_stats_results_size);
		return -ENOMEM;
	}

	WMA_LOGD("Interface stats from FW event buf");
	WMA_LOGD("Fixed Param:");
	WMA_LOGD("request_id %u vdev_id %u",
		 fixed_param->request_id, fixed_param->vdev_id);

	WMA_LOGD("Iface Stats:");
	WMA_LOGD("beacon_rx %u mgmt_rx %u mgmt_action_rx %u mgmt_action_tx %u "
		 "rssi_mgmt %u rssi_data %u rssi_ack %u num_peers %u "
		 "num_peer_events %u num_ac %u roam_state %u"
		 " avg_bcn_spread_offset_high %u"
		 " avg_bcn_spread_offset_low %u"
		 " is leaky_ap %u"
		 " avg_rx_frames_leaked %u"
		 " rx_leak_window %u",
		 link_stats->beacon_rx, link_stats->mgmt_rx,
		 link_stats->mgmt_action_rx, link_stats->mgmt_action_tx,
		 link_stats->rssi_mgmt, link_stats->rssi_data,
		 link_stats->rssi_ack, link_stats->num_peers,
		 link_stats->num_peer_events, link_stats->num_ac,
		 link_stats->roam_state,
		 link_stats->avg_bcn_spread_offset_high,
		 link_stats->avg_bcn_spread_offset_low,
		 link_stats->is_leaky_ap,
		 link_stats->avg_rx_frms_leaked,
		 link_stats->rx_leak_window);

	cdf_mem_zero(link_stats_results, link_stats_results_size);

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

	cdf_mem_copy(results + roaming_offset, &link_stats->roam_state,
		     roaming_size);

	cdf_mem_copy(results + iface_info_size,
		     t_link_stats + WMI_TLV_HDR_SIZE,
		     link_stats_size - iface_info_size -
		     WIFI_AC_MAX * ac_stats_size);

	next_res_offset = link_stats_size - WIFI_AC_MAX * ac_stats_size;
	next_ac_offset = WMI_TLV_HDR_SIZE;

	WMA_LOGD("AC Stats:");
	for (count = 0; count < link_stats->num_ac; count++) {
		WMA_LOGD("ac_type %u tx_mpdu %u rx_mpdu %u tx_mcast %u "
			 "rx_mcast %u rx_ampdu %u tx_ampdu %u mpdu_lost %u "
			 "retries %u retries_short %u retries_long %u "
			 "contention_time_min %u contention_time_max %u "
			 "contention_time_avg %u contention_num_samples %u",
			 ac_stats->ac_type, ac_stats->tx_mpdu,
			 ac_stats->rx_mpdu, ac_stats->tx_mcast,
			 ac_stats->rx_mcast, ac_stats->rx_ampdu,
			 ac_stats->tx_ampdu, ac_stats->mpdu_lost,
			 ac_stats->retries, ac_stats->retries_short,
			 ac_stats->retries_long, ac_stats->contention_time_min,
			 ac_stats->contention_time_max,
			 ac_stats->contention_time_avg,
			 ac_stats->contention_num_samples);
		ac_stats++;

		cdf_mem_copy(results + next_res_offset,
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
	WMA_LOGD("%s: Iface Stats event posted to HDD", __func__);
	cdf_mem_free(link_stats_results);

	return 0;
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
	CDF_STATUS cdf_status;
	tAniGetRssiReq *pGetRssiReq = (tAniGetRssiReq *) wma->pGetRssiReq;
	cds_msg_t sme_msg = { 0 };

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
		}
	}

	WMA_LOGD("vdev id %d beancon snr %d data snr %d",
		 vdev_stats->vdev_id,
		 vdev_stats->vdev_snr.bcn_snr, vdev_stats->vdev_snr.dat_snr);

	if (pGetRssiReq && pGetRssiReq->sessionId == vdev_stats->vdev_id) {
		if ((vdev_stats->vdev_snr.bcn_snr == WMA_TGT_INVALID_SNR) &&
		    (vdev_stats->vdev_snr.dat_snr == WMA_TGT_INVALID_SNR)) {
			/*
			 * Firmware sends invalid snr till it sees
			 * Beacon/Data after connection since after
			 * vdev up fw resets the snr to invalid.
			 * In this duartion Host will return the last know
			 * rssi during connection.
			 */
			WMA_LOGE("Invalid SNR from firmware");

		} else {
			if (vdev_stats->vdev_snr.bcn_snr != WMA_TGT_INVALID_SNR) {
				rssi = vdev_stats->vdev_snr.bcn_snr;
			} else if (vdev_stats->vdev_snr.dat_snr !=
				   WMA_TGT_INVALID_SNR) {
				rssi = vdev_stats->vdev_snr.dat_snr;
			}

			/*
			 * Get the absolute rssi value from the current rssi value
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

		cdf_mem_free(pGetRssiReq);
		wma->pGetRssiReq = NULL;
	}

	if (node->psnr_req) {
		tAniGetSnrReq *p_snr_req = node->psnr_req;

		if (vdev_stats->vdev_snr.bcn_snr != WMA_TGT_INVALID_SNR)
			p_snr_req->snr = vdev_stats->vdev_snr.bcn_snr;
		else if (vdev_stats->vdev_snr.dat_snr != WMA_TGT_INVALID_SNR)
			p_snr_req->snr = vdev_stats->vdev_snr.dat_snr;
		else
			p_snr_req->snr = WMA_TGT_INVALID_SNR;

		sme_msg.type = eWNI_SME_SNR_IND;
		sme_msg.bodyptr = p_snr_req;
		sme_msg.bodyval = 0;

		cdf_status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			WMA_LOGE("%s: Fail to post snr ind msg", __func__);
			cdf_mem_free(p_snr_req);
		}

		node->psnr_req = NULL;
	}
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
	tAniGetPEStatsRsp *stats_rsp_params;

	stats_rsp_params = node->stats_rsp;
	/* send response to UMAC */
	wma_send_msg(wma, WMA_GET_STATISTICS_RSP, (void *)stats_rsp_params, 0);
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
	if (node->stats_rsp) {
		node->fw_stats_set |= FW_PEER_STATS_SET;
		WMA_LOGD("<-- FW PEER STATS received for vdevId:%d", vdev_id);
		stats_rsp_params = (tAniGetPEStatsRsp *) node->stats_rsp;
		stats_buf = (uint8_t *) (stats_rsp_params + 1);
		temp_mask = stats_rsp_params->statsMask;
		if (temp_mask & (1 << eCsrSummaryStats))
			stats_buf += sizeof(tCsrSummaryStatsInfo);

		if (temp_mask & (1 << eCsrGlobalClassAStats)) {
			classa_stats = (tCsrGlobalClassAStatsInfo *) stats_buf;
			WMA_LOGD("peer tx rate:%d", peer_stats->peer_tx_rate);
			/*The linkspeed returned by fw is in kbps so convert
			 *it in to units of 500kbps which is expected by UMAC*/
			if (peer_stats->peer_tx_rate) {
				classa_stats->tx_rate =
					peer_stats->peer_tx_rate / 500;
			}

			classa_stats->tx_rate_flags = node->rate_flags;
			if (!(node->rate_flags & eHAL_TX_RATE_LEGACY)) {
				classa_stats->mcs_index =
					wma_get_mcs_idx((peer_stats->peer_tx_rate /
							 100), node->rate_flags,
							node->nss, &mcsRateFlags);
				/* rx_frag_cnt and promiscuous_rx_frag_cnt
				 * parameter is currently not used. lets use the
				 * same parameter to hold the nss value and mcs
				 * rate flags */
				classa_stats->rx_frag_cnt = node->nss;
				classa_stats->promiscuous_rx_frag_cnt =
					mcsRateFlags;
				WMA_LOGD("Computed mcs_idx:%d mcs_rate_flags:%d",
					classa_stats->mcs_index, mcsRateFlags);
			}
			/* FW returns tx power in intervals of 0.5 dBm
			   Convert it back to intervals of 1 dBm */
			classa_stats->max_pwr =
				roundup(classa_stats->max_pwr, 2) >> 1;
			WMA_LOGD("peer tx rate flags:%d nss:%d max_txpwr:%d",
				 node->rate_flags, node->nss,
				 classa_stats->max_pwr);
		}

		if (node->fw_stats_set & FW_STATS_SET) {
			WMA_LOGD("<--STATS RSP VDEV_ID:%d", vdev_id);
			wma_post_stats(wma, node);
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
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	cds_msg_t sme_msg = { 0 };

	pGetLinkStatus->linkStatus = link_status;
	sme_msg.type = eWNI_SME_LINK_STATUS_IND;
	sme_msg.bodyptr = pGetLinkStatus;
	sme_msg.bodyval = 0;

	cdf_status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		WMA_LOGE("%s: Fail to post link status ind msg", __func__);
		cdf_mem_free(pGetLinkStatus);
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

	event = (wmi_vdev_rate_stats_event_fixed_param *) param_buf->fixed_param;
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

			wma_post_link_status(intr[ht_info->vdevid].plink_status_req,
							 link_status);
			intr[ht_info->vdevid].plink_status_req = NULL;
			link_status = LINK_STATUS_LEGACY;
		}

		ht_info++;
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
	uint8_t i, *temp;


	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGA("%s: Invalid stats event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	temp = (uint8_t *) param_buf->data;

	WMA_LOGD("%s: num_stats: pdev: %u vdev: %u peer %u",
		 __func__, event->num_pdev_stats, event->num_vdev_stats,
		 event->num_peer_stats);
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

	WMA_LOGI("%s: Exit", __func__);
	return 0;
}

/**
 * wma_send_link_speed() - send link speed to SME
 * @link_speed: link speed
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_send_link_speed(uint32_t link_speed)
{
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	cds_msg_t sme_msg = { 0 };
	tSirLinkSpeedInfo *ls_ind =
		(tSirLinkSpeedInfo *) cdf_mem_malloc(sizeof(tSirLinkSpeedInfo));
	if (!ls_ind) {
		WMA_LOGE("%s: Memory allocation failed.", __func__);
		cdf_status = CDF_STATUS_E_NOMEM;
	} else {
		ls_ind->estLinkSpeed = link_speed;
		sme_msg.type = eWNI_SME_LINK_SPEED_IND;
		sme_msg.bodyptr = ls_ind;
		sme_msg.bodyval = 0;

		cdf_status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
		if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
			WMA_LOGE("%s: Fail to post linkspeed ind  msg",
				 __func__);
			cdf_mem_free(ls_ind);
		}
	}
	return cdf_status;
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
	CDF_STATUS cdf_status;

	param_buf =
		(WMI_PEER_ESTIMATED_LINKSPEED_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid linkspeed event", __func__);
		return -EINVAL;
	}
	event = param_buf->fixed_param;
	cdf_status = wma_send_link_speed(event->est_linkspeed_kbps);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		return -EINVAL;
	}
	return 0;
}

/**
 * wma_wni_cfg_dnld() - cfg download request
 * @handle: wma handle
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_wni_cfg_dnld(tp_wma_handle wma_handle)
{
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	void *mac = cds_get_context(CDF_MODULE_ID_PE);

	WMA_LOGD("%s: Enter", __func__);

	if (NULL == mac) {
		WMA_LOGP("%s: Invalid context", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}

	process_cfg_download_req(mac);

	WMA_LOGD("%s: Exit", __func__);
	return cdf_status;
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
	tp_wma_handle wma_handle = handle;
	int i;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (wma_handle->interfaces[i].scan_info.scan_id) {

			WMA_LOGE("%s: scan in progress on interface[%d],scanid = %d",
				__func__, i,
				wma_handle->interfaces[i].scan_info.scan_id);
			return true;
		}
	}
	return false;
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
		if (!wma_handle->interfaces[i].vdev_up)
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
		if (!wma_handle->interfaces[i].vdev_up)
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
		if (!wma_handle->interfaces[i].vdev_up)
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
		if (!wma_handle->interfaces[i].vdev_up)
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
 * @is_cw40: is channel width 40 supported
 * @is_vht: is vht supported
 * @is_cw_vht: is channel width 80 supported
 *
 * Return: WLAN_PHY_MODE
 */
WLAN_PHY_MODE wma_peer_phymode(tSirNwType nw_type, uint8_t sta_type,
			       uint8_t is_ht, uint8_t ch_width,
			       uint8_t is_vht)
{
	WLAN_PHY_MODE phymode = MODE_UNKNOWN;

	switch (nw_type) {
	case eSIR_11B_NW_TYPE:
		phymode = MODE_11B;
		if (is_ht || is_vht)
			WMA_LOGE("HT/VHT is enabled with 11B NW type");
		break;
	case eSIR_11G_NW_TYPE:
		if (!(is_ht || is_vht)) {
			phymode = MODE_11G;
			break;
		}
		if (CH_WIDTH_40MHZ < ch_width)
			WMA_LOGE("80/160 MHz BW sent in 11G, configured 40MHz");
		if (ch_width)
			phymode = (is_vht) ?
				MODE_11AC_VHT40 : MODE_11NG_HT40;
		else
			phymode = (is_vht) ?
				MODE_11AC_VHT20 : MODE_11NG_HT20;
		break;
	case eSIR_11A_NW_TYPE:
		if (!(is_ht || is_vht)) {
			phymode = MODE_11A;
			break;
		}
		if (is_vht) {
#if CONFIG_160MHZ_SUPPORT != 0
			if (ch_width == CH_WIDTH_160MHZ)
				phymode = MODE_11AC_VHT160;
			else if (ch_width == CH_WIDTH_80P80MHZ)
				phymode = MODE_11AC_VHT80_80;
			else
#endif
			if (ch_width == CH_WIDTH_80MHZ)
				phymode = MODE_11AC_VHT80;
			else
				phymode = (ch_width) ?
					  MODE_11AC_VHT40 : MODE_11AC_VHT20;
		} else
			phymode = (ch_width) ? MODE_11NA_HT40 : MODE_11NA_HT20;
		break;
	default:
		WMA_LOGP("%s: Invalid nw type %d", __func__, nw_type);
		break;
	}
	WMA_LOGD("%s: nw_type %d is_ht %d ch_width %d is_vht %d phymode %d",
		  __func__, nw_type, is_ht, ch_width, is_vht, phymode);

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
	ol_txrx_vdev_handle vdev;

	vdev = wma_find_vdev_by_id(wma_handle, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		return -EINVAL;
	}
	cdf_mem_zero(&req, sizeof(req));
	req.stats_type_reset_mask = value;
	ol_txrx_fw_stats_get(vdev, &req);

	return 0;
}

#ifdef HELIUMPLUS
#define SET_UPLOAD_MASK(_mask, _rate_info) 	\
	((_mask) = 1 << (_rate_info ## _V2))
#else  /* !HELIUMPLUS */
#define SET_UPLOAD_MASK(_mask, _rate_info) 	\
	((_mask) = 1 << (_rate_info))
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
	ol_txrx_vdev_handle vdev;
	uint32_t l_up_mask;

	vdev = wma_find_vdev_by_id(wma_handle, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		return -EINVAL;
	}
	cdf_mem_zero(&req, sizeof(req));
	req.print.verbose = 1;

	switch (value) {
	/* txrx_fw_stats 1 */
	case WMA_FW_PHY_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_WAL_PDEV_TXRX;
		break;

	/* txrx_fw_stats 2 */
	case WMA_FW_RX_REORDER_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_RX_REORDER;
		break;

	/* txrx_fw_stats 3 */
	case WMA_FW_RX_RC_STATS:
		SET_UPLOAD_MASK(l_up_mask, HTT_DBG_STATS_RX_RATE_INFO);
		break;

	/* txrx_fw_stats 5 */
	case WMA_FW_TX_CONCISE_STATS:
		req.print.concise = 1;
		/* No break here, since l_up_mask is same for
		 * both WMA_FW_TX_CONCISE_STATS & WMA_FW_TX_PPDU_STATS */

	/* txrx_fw_stats 4 */
	case WMA_FW_TX_PPDU_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_TX_PPDU_LOG;
		break;

	/* txrx_fw_stats 6 */
	case WMA_FW_TX_RC_STATS:
		SET_UPLOAD_MASK(l_up_mask, HTT_DBG_STATS_TX_RATE_INFO);
		break;

	/* txrx_fw_stats 12 */
	/*
	 * This is 1:1 correspondence with WMA defined value
	 * and the f/w bitmask.
	 */
	case WMA_FW_RX_REM_RING_BUF:
		l_up_mask = 1 << HTT_DBG_STATS_RX_REMOTE_RING_BUFFER_INFO;
		break;

	/* txrx_fw_stats 7 */
	case WMA_FW_TXBF_INFO_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_TXBF_INFO;
		break;

	/* txrx_fw_stats 8 */
	case WMA_FW_SND_INFO_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_SND_INFO;
		break;

	/* txrx_fw_stats 9 */
	case WMA_FW_ERROR_INFO_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_ERROR_INFO;
		break;

	/* txrx_fw_stats 10 */
	case WMA_FW_TX_SELFGEN_INFO_STATS:
		l_up_mask = 1 << HTT_DBG_STATS_TX_SELFGEN_INFO;
		break;

	/* txrx_fw_stats 15 */
	/*
	 * This is 1:1 correspondence with WMA defined value
	 * and the f/w bitmask.
	 */
	case WMA_FW_RX_TXBF_MUSU_NDPA:
		l_up_mask = 1 << HTT_DBG_STATS_TXBF_MUSU_NDPA_PKT;
		break;

	default:
		cdf_print("%s %d Invalid value %d\n",
				__func__, __LINE__, value);
		return 0;
	}
	req.stats_type_upload_mask = l_up_mask;

	ol_txrx_fw_stats_get(vdev, &req);

	return 0;
}

/**
 * wmi_crash_inject() - inject fw crash
 * @wma_handle: wma handle
 * @type: type
 * @delay_time_ms: delay time in ms
 *
 * Return: 0 for success or return error
 */
int wmi_crash_inject(wmi_unified_t wmi_handle, uint32_t type,
		     uint32_t delay_time_ms)
{
	int ret = 0;
	WMI_FORCE_FW_HANG_CMD_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed!", __func__);
		return -ENOMEM;
	}

	cmd = (WMI_FORCE_FW_HANG_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_FORCE_FW_HANG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_FORCE_FW_HANG_CMD_fixed_param));
	cmd->type = type;
	cmd->delay_time_ms = delay_time_ms;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_FORCE_FW_HANG_CMDID);
	if (ret < 0) {
		WMA_LOGE("%s: Failed to send set param command, ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
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
	uint32_t len, temp_mask, counter = 0;

	len = sizeof(tAniGetPEStatsRsp);
	temp_mask = get_stats_param->statsMask;

	while (temp_mask) {
		if (temp_mask & 1) {
			switch (counter) {
			case eCsrSummaryStats:
				len += sizeof(tCsrSummaryStatsInfo);
				break;
			case eCsrGlobalClassAStats:
				len += sizeof(tCsrGlobalClassAStatsInfo);
				break;
			case eCsrGlobalClassBStats:
				len += sizeof(tCsrGlobalClassBStatsInfo);
				break;
			case eCsrGlobalClassCStats:
				len += sizeof(tCsrGlobalClassCStatsInfo);
				break;
			case eCsrGlobalClassDStats:
				len += sizeof(tCsrGlobalClassDStatsInfo);
				break;
			case eCsrPerStaStats:
				len += sizeof(tCsrPerStaStatsInfo);
				break;
			}
		}

		counter++;
		temp_mask >>= 1;
	}

	stats_rsp_params = (tAniGetPEStatsRsp *) cdf_mem_malloc(len);
	if (!stats_rsp_params) {
		WMA_LOGE("memory allocation failed for tAniGetPEStatsRsp");
		CDF_ASSERT(0);
		return NULL;
	}

	cdf_mem_zero(stats_rsp_params, len);
	stats_rsp_params->staId = get_stats_param->staId;
	stats_rsp_params->statsMask = get_stats_param->statsMask;
	stats_rsp_params->msgType = WMA_GET_STATISTICS_RSP;
	stats_rsp_params->msgLen = len - sizeof(tAniGetPEStatsRsp);
	stats_rsp_params->rc = CDF_STATUS_SUCCESS;
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
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	tAniGetPEStatsRsp *pGetPEStatsRspParams;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	WMA_LOGD("%s: Enter", __func__);
	node = &wma_handle->interfaces[get_stats_param->sessionId];
	if (node->stats_rsp) {
		pGetPEStatsRspParams = node->stats_rsp;
		if (pGetPEStatsRspParams->staId == get_stats_param->staId &&
		    pGetPEStatsRspParams->statsMask ==
		    get_stats_param->statsMask) {
			WMA_LOGI("Stats for staId %d with stats mask %d "
				 "is pending.... ignore new request",
				 get_stats_param->staId,
				 get_stats_param->statsMask);
			goto end;
		} else {
			cdf_mem_free(node->stats_rsp);
			node->stats_rsp = NULL;
			node->fw_stats_set = 0;
		}
	}

	pGetPEStatsRspParams = wma_get_stats_rsp_buf(get_stats_param);
	if (!pGetPEStatsRspParams)
		goto end;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: Failed to allocate wmi buffer", __func__);
		goto failed;
	}

	node->fw_stats_set = 0;
	node->stats_rsp = pGetPEStatsRspParams;
	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id =
		WMI_REQUEST_PEER_STAT | WMI_REQUEST_PDEV_STAT |
		WMI_REQUEST_VDEV_STAT;
	cmd->vdev_id = get_stats_param->sessionId;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(node->bssid, &cmd->peer_macaddr);
	WMA_LOGD("STATS REQ VDEV_ID:%d-->", cmd->vdev_id);
	if (wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {

		WMA_LOGE("%s: Failed to send WMI_REQUEST_STATS_CMDID",
			 __func__);
		wmi_buf_free(buf);
		goto failed;
	}

	goto end;
failed:

	pGetPEStatsRspParams->rc = CDF_STATUS_E_FAILURE;
	node->stats_rsp = NULL;
	/* send response to UMAC */
	wma_send_msg(wma_handle, WMA_GET_STATISTICS_RSP, pGetPEStatsRspParams,
		     0);
end:
	cdf_mem_free(get_stats_param);
	WMA_LOGD("%s: Exit", __func__);
	return;
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

	wma = cds_get_context(CDF_MODULE_ID_WMA);
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

	cdf_spin_lock_bh(&beacon->lock);

	buf_size = cdf_nbuf_len(beacon->buf);
	buf = cdf_mem_malloc(buf_size);

	if (!buf) {
		cdf_spin_unlock_bh(&beacon->lock);
		WMA_LOGE("%s: alloc failed for beacon buf", __func__);
		return NULL;
	}

	cdf_mem_copy(buf, cdf_nbuf_data(beacon->buf), buf_size);

	cdf_spin_unlock_bh(&beacon->lock);

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

	wma = cds_get_context(CDF_MODULE_ID_WMA);
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

/**
 * wma_get_interface_by_vdev_id() - lookup interface entry using vdev ID
 * @vdev_id: vdev id
 *
 * Return: entry from vdev table
 */
struct wma_txrx_node  *wma_get_interface_by_vdev_id(uint8_t vdev_id)
{
	tp_wma_handle wma;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
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

/**
 * wma_is_vdev_up() - return whether a vdev is up
 * @vdev_id: vdev id
 *
 * Return: true if the vdev is up, false otherwise
 */
bool wma_is_vdev_up(uint8_t vdev_id)
{
	struct wma_txrx_node *vdev = wma_get_interface_by_vdev_id(vdev_id);
	if (vdev)
		return vdev->vdev_up;
	else
		return false;
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
int wma_utf_rsp(tp_wma_handle wma_handle, uint8_t **payload, uint32_t *len)
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
		*payload = (uint8_t *) cdf_mem_malloc((uint32_t) payload_len
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
	cds_msg_t msg = { 0 };
	CDF_STATUS status;

	ret = wma_utf_rsp(wma_handle, &payload, &data_len);

	if (ret) {
		return;
	}

	sys_build_message_header(SYS_MSG_ID_FTM_RSP, &msg);
	msg.bodyptr = payload;
	msg.bodyval = 0;

	status = cds_mq_post_message(CDS_MQ_ID_SYS, &msg);

	if (status != CDF_STATUS_SUCCESS) {
		WMA_LOGE("failed to post ftm response to SYS");
		cdf_mem_free(payload);
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
	SEG_HDR_INFO_STRUCT segHdrInfo;
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

	segHdrInfo = *(SEG_HDR_INFO_STRUCT *) &(data[0]);

	wma_handle->utf_event_info.currentSeq = (segHdrInfo.segmentInfo & 0xF);

	currentSeq = (segHdrInfo.segmentInfo & 0xF);
	totalNumOfSegments = (segHdrInfo.segmentInfo >> 4) & 0xF;

	datalen = datalen - sizeof(segHdrInfo);

	if (currentSeq == 0) {
		wma_handle->utf_event_info.expectedSeq = 0;
		wma_handle->utf_event_info.offset = 0;
	} else {
		if (wma_handle->utf_event_info.expectedSeq != currentSeq)
			WMA_LOGE("Mismatch in expecting seq expected"
				 " Seq %d got seq %d",
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
			WMA_LOGE("All segs received total len mismatch.."
				 " len %zu total len %d",
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
		cdf_mem_free(wma_handle->utf_event_info.data);
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
					  cdf_mem_malloc(MAX_UTF_EVENT_LENGTH);
	wma_handle->utf_event_info.length = 0;

	ret = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						 WMI_PDEV_UTF_EVENTID,
						 wma_process_utf_event);

	if (ret)
		WMA_LOGP("%s: Failed to register UTF event callback", __func__);
}

/**
 * wmi_unified_pdev_utf_cmd() - send utf command to fw
 * @wmi_handle: wmi handle
 * @utf_payload: utf payload
 * @len: length
 *
 * Return: 0 for success or error code
 */
static int
wmi_unified_pdev_utf_cmd(wmi_unified_t wmi_handle, uint8_t *utf_payload,
			 uint16_t len)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	int ret = 0;
	static uint8_t msgref = 1;
	uint8_t segNumber = 0, segInfo, numSegments;
	uint16_t chunk_len, total_bytes;
	uint8_t *bufpos;
	SEG_HDR_INFO_STRUCT segHdrInfo;

	bufpos = utf_payload;
	total_bytes = len;
	ASSERT(total_bytes / MAX_WMI_UTF_LEN ==
	       (uint8_t) (total_bytes / MAX_WMI_UTF_LEN));
	numSegments = (uint8_t) (total_bytes / MAX_WMI_UTF_LEN);

	if (len - (numSegments * MAX_WMI_UTF_LEN))
		numSegments++;

	while (len) {
		if (len > MAX_WMI_UTF_LEN)
			chunk_len = MAX_WMI_UTF_LEN;    /* MAX messsage */
		else
			chunk_len = len;

		buf = wmi_buf_alloc(wmi_handle,
				    (chunk_len + sizeof(segHdrInfo) +
				     WMI_TLV_HDR_SIZE));
		if (!buf) {
			WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
			return -ENOMEM;
		}

		cmd = (uint8_t *) wmi_buf_data(buf);

		segHdrInfo.len = total_bytes;
		segHdrInfo.msgref = msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;
		segHdrInfo.pad = 0;

		WMA_LOGD("%s:segHdrInfo.len = %d, segHdrInfo.msgref = %d,"
			 " segHdrInfo.segmentInfo = %d",
			 __func__, segHdrInfo.len, segHdrInfo.msgref,
			 segHdrInfo.segmentInfo);

		WMA_LOGD("%s:total_bytes %d segNumber %d totalSegments %d"
			 "chunk len %d", __func__, total_bytes, segNumber,
			 numSegments, chunk_len);

		segNumber++;

		WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE,
			       (chunk_len + sizeof(segHdrInfo)));
		cmd += WMI_TLV_HDR_SIZE;
		memcpy(cmd, &segHdrInfo, sizeof(segHdrInfo));   /* 4 bytes */
		memcpy(&cmd[sizeof(segHdrInfo)], bufpos, chunk_len);

		ret = wmi_unified_cmd_send(wmi_handle, buf,
					   (chunk_len + sizeof(segHdrInfo) +
					    WMI_TLV_HDR_SIZE),
					   WMI_PDEV_UTF_CMDID);

		if (ret != EOK) {
			WMA_LOGE("Failed to send WMI_PDEV_UTF_CMDID command");
			wmi_buf_free(buf);
			break;
		}

		len -= chunk_len;
		bufpos += chunk_len;
	}

	msgref++;

	return ret;
}

/**
 * wma_utf_cmd() - utf command
 * @wma_handle: wma handle
 * @data: data
 * @len: length
 *
 * Return: 0 for success or error code
 */
int wma_utf_cmd(tp_wma_handle wma_handle, uint8_t *data, uint16_t len)
{
	wma_handle->utf_event_info.length = 0;
	return wmi_unified_pdev_utf_cmd(wma_handle->wmi_handle, data, len);
}

/**
 * wma_process_ftm_command() - process ftm command
 * @wma_handle: wma handle
 * @msg_buffer: message buffer
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS
wma_process_ftm_command(tp_wma_handle wma_handle,
			struct ar6k_testmode_cmd_data *msg_buffer)
{
	uint8_t *data = NULL;
	uint16_t len = 0;
	int ret;

	if (!msg_buffer)
		return CDF_STATUS_E_INVAL;

	if (cds_get_conparam() != CDF_FTM_MODE) {
		WMA_LOGE("FTM command issued in non-FTM mode");
		cdf_mem_free(msg_buffer->data);
		cdf_mem_free(msg_buffer);
		return CDF_STATUS_E_NOSUPPORT;
	}

	data = msg_buffer->data;
	len = msg_buffer->len;

	ret = wma_utf_cmd(wma_handle, data, len);

	cdf_mem_free(msg_buffer->data);
	cdf_mem_free(msg_buffer);

	if (ret)
		return CDF_STATUS_E_FAILURE;

	return CDF_STATUS_SUCCESS;
}
#endif /* QCA_WIFI_FTM */

/**
 * wma_get_wcnss_software_version() - get wcnss software version
 * @p_cds_gctx: cds context
 * @pVersion: version pointer
 * @versionBufferSize: buffer size
 *
 * Return: CDF_STATUS_SUCCESS for success or error code
 */
CDF_STATUS wma_get_wcnss_software_version(void *p_cds_gctx,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize)
{
	tp_wma_handle wma_handle;
	wma_handle = cds_get_context(CDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		return CDF_STATUS_E_FAULT;
	}

	snprintf(pVersion, versionBufferSize, "%x",
		 (unsigned int)wma_handle->target_fw_version);
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_get_tx_rx_ss_from_config() - Get Tx/Rx spatial stream from HW mode config
 * @mac_ss: Config which indicates the HW mode as per 'hw_mode_ss_config'
 * @tx_ss: Contains the Tx spatial stream
 * @rx_ss: Contains the Rx spatial stream
 *
 * Returns the number of spatial streams of Tx and Rx
 *
 * Return: None
 */
void wma_get_tx_rx_ss_from_config(enum hw_mode_ss_config mac_ss,
				  uint32_t *tx_ss,
				  uint32_t *rx_ss)
{
	switch (mac_ss) {
	case HW_MODE_SS_0x0:
		*tx_ss = 0;
		*rx_ss = 0;
		break;
	case HW_MODE_SS_1x1:
		*tx_ss = 1;
		*rx_ss = 1;
		break;
	case HW_MODE_SS_2x2:
		*tx_ss = 2;
		*rx_ss = 2;
		break;
	case HW_MODE_SS_3x3:
		*tx_ss = 3;
		*rx_ss = 3;
		break;
	case HW_MODE_SS_4x4:
		*tx_ss = 4;
		*rx_ss = 4;
		break;
	default:
		*tx_ss = 0;
		*rx_ss = 0;
	}
}

/**
 * wma_get_matching_hw_mode_index() - Get matching HW mode index
 * @wma: WMA handle
 * @mac0_tx_ss: Number of tx spatial streams of MAC0
 * @mac0_rx_ss: Number of rx spatial streams of MAC0
 * @mac0_bw: Bandwidth of MAC0 of type 'hw_mode_bandwidth'
 * @mac1_tx_ss: Number of tx spatial streams of MAC1
 * @mac1_rx_ss: Number of rx spatial streams of MAC1
 * @mac1_bw: Bandwidth of MAC1 of type 'hw_mode_bandwidth'
 * @dbs: DBS capability of type 'hw_mode_dbs_capab'
 * @dfs: Agile DFS capability of type 'hw_mode_agile_dfs_capab'
 *
 * Fetches the HW mode index corresponding to the HW mode provided
 *
 * Return: Positive hw mode index in case a match is found or a negative
 * value, otherwise
 */
static int8_t wma_get_matching_hw_mode_index(tp_wma_handle wma,
				uint32_t mac0_tx_ss, uint32_t mac0_rx_ss,
				enum hw_mode_bandwidth mac0_bw,
				uint32_t mac1_tx_ss, uint32_t mac1_rx_ss,
				enum hw_mode_bandwidth mac1_bw,
				enum hw_mode_dbs_capab dbs,
				enum hw_mode_agile_dfs_capab dfs)
{
	uint32_t i;
	uint32_t t_mac0_tx_ss, t_mac0_rx_ss, t_mac0_bw;
	uint32_t t_mac1_tx_ss, t_mac1_rx_ss, t_mac1_bw;
	uint32_t dbs_mode, agile_dfs_mode;
	int8_t found = -EINVAL;

	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return found;
	}

	for (i = 0; i < wma->num_dbs_hw_modes; i++) {
		t_mac0_tx_ss = WMI_DBS_HW_MODE_MAC0_TX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac0_tx_ss != mac0_tx_ss)
			continue;

		t_mac0_rx_ss = WMI_DBS_HW_MODE_MAC0_RX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac0_rx_ss != mac0_rx_ss)
			continue;

		t_mac0_bw = WMI_DBS_HW_MODE_MAC0_BANDWIDTH_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac0_bw != mac0_bw)
			continue;

		t_mac1_tx_ss = WMI_DBS_HW_MODE_MAC1_TX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac1_tx_ss != mac1_tx_ss)
			continue;

		t_mac1_rx_ss = WMI_DBS_HW_MODE_MAC1_RX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac1_rx_ss != mac1_rx_ss)
			continue;

		t_mac1_bw = WMI_DBS_HW_MODE_MAC1_BANDWIDTH_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (t_mac1_bw != mac1_bw)
			continue;

		dbs_mode = WMI_DBS_HW_MODE_DBS_MODE_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (dbs_mode != dbs)
			continue;

		agile_dfs_mode = WMI_DBS_HW_MODE_AGILE_DFS_GET(
				wma->hw_mode.hw_mode_list[i]);
		if (agile_dfs_mode != dfs)
			continue;

		found = i;
		WMA_LOGI("%s: hw_mode index %d found",
				__func__, i);
		break;
	}
	return found;
}

/**
 * wma_get_hw_mode_from_dbs_hw_list() - Get hw_mode index
 * @mac0_ss: MAC0 spatial stream configuration
 * @mac0_bw: MAC0 bandwidth configuration
 * @mac1_ss: MAC1 spatial stream configuration
 * @mac1_bw: MAC1 bandwidth configuration
 * @dbs: HW DBS capability
 * @dfs: HW Agile DFS capability
 *
 * Get the HW mode index corresponding to the HW modes spatial stream,
 * bandwidth, DBS and Agile DFS capability
 *
 * Return: Index number if a match is found or -negative value if not found
 */
int8_t wma_get_hw_mode_idx_from_dbs_hw_list(enum hw_mode_ss_config mac0_ss,
					    enum hw_mode_bandwidth mac0_bw,
					    enum hw_mode_ss_config mac1_ss,
					    enum hw_mode_bandwidth mac1_bw,
					    enum hw_mode_dbs_capab dbs,
					    enum hw_mode_agile_dfs_capab dfs)
{
	tp_wma_handle wma;
	uint32_t mac0_tx_ss, mac0_rx_ss;
	uint32_t mac1_tx_ss, mac1_rx_ss;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return -EINVAL;
	}

	wma_get_tx_rx_ss_from_config(mac0_ss, &mac0_tx_ss, &mac0_rx_ss);
	wma_get_tx_rx_ss_from_config(mac1_ss, &mac1_tx_ss, &mac1_rx_ss);

	WMA_LOGI("%s: MAC0: TxSS=%d, RxSS=%d, BW=%d",
		__func__, mac0_tx_ss, mac0_rx_ss, mac0_bw);
	WMA_LOGI("%s: MAC1: TxSS=%d, RxSS=%d, BW=%d",
		__func__, mac1_tx_ss, mac1_rx_ss, mac1_bw);
	WMA_LOGI("%s: DBS capab=%d, Agile DFS capab=%d",
		__func__, dbs, dfs);

	return wma_get_matching_hw_mode_index(wma, mac0_tx_ss, mac0_rx_ss,
						mac0_bw,
						mac1_tx_ss, mac1_rx_ss,
						mac1_bw,
						dbs, dfs);
}

/**
 * wma_get_hw_mode_from_idx() - Get HW mode based on index
 * @idx: HW mode index
 * @hw_mode: HW mode params
 *
 * Fetches the HW mode parameters
 *
 * Return: Success if hw mode is obtained and the hw mode params
 */
CDF_STATUS wma_get_hw_mode_from_idx(uint32_t idx,
		struct sir_hw_mode_params *hw_mode)
{
	tp_wma_handle wma;
	uint32_t param;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	if (idx > wma->num_dbs_hw_modes) {
		WMA_LOGE("%s: Invalid index", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	param = wma->hw_mode.hw_mode_list[idx];

	hw_mode->mac0_tx_ss = WMI_DBS_HW_MODE_MAC0_TX_STREAMS_GET(param);
	hw_mode->mac0_rx_ss = WMI_DBS_HW_MODE_MAC0_RX_STREAMS_GET(param);
	hw_mode->mac0_bw = WMI_DBS_HW_MODE_MAC0_BANDWIDTH_GET(param);
	hw_mode->mac1_tx_ss = WMI_DBS_HW_MODE_MAC1_TX_STREAMS_GET(param);
	hw_mode->mac1_rx_ss = WMI_DBS_HW_MODE_MAC1_RX_STREAMS_GET(param);
	hw_mode->mac1_bw = WMI_DBS_HW_MODE_MAC1_BANDWIDTH_GET(param);
	hw_mode->dbs_cap = WMI_DBS_HW_MODE_DBS_MODE_GET(param);
	hw_mode->agile_dfs_cap = WMI_DBS_HW_MODE_AGILE_DFS_GET(param);

	return CDF_STATUS_SUCCESS;
}

/**
 * wma_get_num_dbs_hw_modes() - Get number of HW mode
 *
 * Fetches the number of DBS HW modes returned by the FW
 *
 * Return: Negative value on error or returns the number of DBS HW modes
 */
int8_t wma_get_num_dbs_hw_modes(void)
{
	tp_wma_handle wma;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return -EINVAL;
	}
	return wma->num_dbs_hw_modes;
}

/**
 * wma_is_hw_dbs_capable() - Check if HW is DBS capable
 *
 * Checks if the HW is DBS capable
 *
 * Return: true if the HW is DBS capable
 */
bool wma_is_hw_dbs_capable(void)
{
	tp_wma_handle wma;
	uint32_t param, i, found = 0;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	if (!wma_is_dbs_enable()) {
		WMA_LOGI("%s: DBS is disabled", __func__);
		return false;
	}

	WMA_LOGI("%s: DBS service bit map: %d", __func__,
		WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
		WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT));

	/* The agreement with FW is that: To know if the target is DBS
	 * capable, DBS needs to be supported both in the HW mode list
	 * and in the service ready event
	 */
	if (!(WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
			WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT)))
		return false;

	for (i = 0; i < wma->num_dbs_hw_modes; i++) {
		param = wma->hw_mode.hw_mode_list[i];
		WMA_LOGI("%s: HW param: %x", __func__, param);
		if (WMI_DBS_HW_MODE_DBS_MODE_GET(param)) {
			WMA_LOGI("%s: HW (%d) is DBS capable", __func__, i);
			found = 1;
			break;
		}
	}

	if (found)
		return true;

	return false;
}

/**
 * wma_is_hw_agile_dfs_capable() - Check if HW is agile DFS capable
 *
 * Checks if the HW is agile DFS capable
 *
 * Return: true if the HW is agile DFS capable
 */
bool wma_is_hw_agile_dfs_capable(void)
{
	tp_wma_handle wma;
	uint32_t param, i, found = 0;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	if (!wma_is_agile_dfs_enable()) {
		WMA_LOGI("%s: Agile DFS is disabled", __func__);
		return false;
	}

	WMA_LOGI("%s: DBS service bit map: %d", __func__,
		WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
		WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT));

	/* The agreement with FW is that to know if the target is Agile DFS
	 * capable, DBS needs to be supported in the service bit map and
	 * Agile DFS needs to be supported in the HW mode list
	 */
	if (!(WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
			WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT)))
		return false;

	for (i = 0; i < wma->num_dbs_hw_modes; i++) {
		param = wma->hw_mode.hw_mode_list[i];
		WMA_LOGI("%s: HW param: %x", __func__, param);
		if (WMI_DBS_HW_MODE_AGILE_DFS_GET(param)) {
			WMA_LOGI("%s: HW %d is agile DFS capable",
				__func__, i);
			found = 1;
			break;
		}
	}

	if (found)
		return true;

	return false;
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

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return -EINVAL;
	}

	if (wma->interfaces)
		return wma->interfaces[vdev_id].mac_id;

	return -EINVAL;
}

/**
 * wma_get_old_and_new_hw_index() - Get the old and new HW index
 * @old_hw_mode_index: Value at this pointer contains the old HW mode index
 * Default value when not configured is WMA_DEFAULT_HW_MODE_INDEX
 * @new_hw_mode_index: Value at this pointer contains the new HW mode index
 * Default value when not configured is WMA_DEFAULT_HW_MODE_INDEX
 *
 * Get the old and new HW index configured in the driver
 *
 * Return: Failure in case the HW mode indices cannot be fetched and Success
 * otherwise. When no HW mode transition has happened the values of
 * old_hw_mode_index and new_hw_mode_index will be the same.
 */
CDF_STATUS wma_get_old_and_new_hw_index(uint32_t *old_hw_mode_index,
				    uint32_t *new_hw_mode_index)
{
	tp_wma_handle wma;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return CDF_STATUS_E_INVAL;
	}

	*old_hw_mode_index = wma->old_hw_mode_index;
	*new_hw_mode_index = wma->new_hw_mode_index;

	return CDF_STATUS_SUCCESS;
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

	wma = cds_get_context(CDF_MODULE_ID_WMA);
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
			WMI_DBS_HW_MODE_MAC0_TX_STREAMS_GET(param);
		wma->interfaces[vdev_id].rx_streams =
			WMI_DBS_HW_MODE_MAC0_RX_STREAMS_GET(param);
	} else {
		wma->interfaces[vdev_id].tx_streams =
			WMI_DBS_HW_MODE_MAC1_TX_STREAMS_GET(param);
		wma->interfaces[vdev_id].rx_streams =
			WMI_DBS_HW_MODE_MAC1_RX_STREAMS_GET(param);
	}
}

/**
 * wma_get_dbs_hw_modes() - Get the DBS HW modes for userspace
 * @one_by_one_dbs: 1x1 DBS capability of HW
 * @two_by_two_dbs: 2x2 DBS capability of HW
 *
 * Provides the DBS HW mode capability such as whether
 * 1x1 DBS, 2x2 DBS is supported by the HW or not.
 *
 * Return: Failure in case of error and 0 on success
 *         one_by_one_dbs/two_by_two_dbs will be false,
 *         if they are not supported.
 *         one_by_one_dbs/two_by_two_dbs will be true,
 *         if they are supported.
 *         false values of one_by_one_dbs/two_by_two_dbs,
 *         indicate DBS is disabled
 */
CDF_STATUS wma_get_dbs_hw_modes(bool *one_by_one_dbs, bool *two_by_two_dbs)
{
	tp_wma_handle wma;
	uint32_t i;
	int8_t found_one_by_one = -EINVAL, found_two_by_two = -EINVAL;
	uint32_t conf1_tx_ss, conf1_rx_ss;
	uint32_t conf2_tx_ss, conf2_rx_ss;

	*one_by_one_dbs = false;
	*two_by_two_dbs = false;

	if (wma_is_hw_dbs_capable() == false) {
		WMA_LOGE("%s: HW is not DBS capable", __func__);
		/* Caller will understand that DBS is disabled */
		return CDF_STATUS_SUCCESS;

	}

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	/* To check 1x1 capability */
	wma_get_tx_rx_ss_from_config(HW_MODE_SS_1x1,
			&conf1_tx_ss, &conf1_rx_ss);
	/* To check 2x2 capability */
	wma_get_tx_rx_ss_from_config(HW_MODE_SS_2x2,
			&conf2_tx_ss, &conf2_rx_ss);

	for (i = 0; i < wma->num_dbs_hw_modes; i++) {
		uint32_t t_conf0_tx_ss, t_conf0_rx_ss;
		uint32_t t_conf1_tx_ss, t_conf1_rx_ss;
		uint32_t dbs_mode;

		t_conf0_tx_ss = WMI_DBS_HW_MODE_MAC0_TX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		t_conf0_rx_ss = WMI_DBS_HW_MODE_MAC0_RX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		t_conf1_tx_ss = WMI_DBS_HW_MODE_MAC1_TX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		t_conf1_rx_ss = WMI_DBS_HW_MODE_MAC1_RX_STREAMS_GET(
				wma->hw_mode.hw_mode_list[i]);
		dbs_mode = WMI_DBS_HW_MODE_DBS_MODE_GET(
				wma->hw_mode.hw_mode_list[i]);

		if (((((t_conf0_tx_ss == conf1_tx_ss) &&
		    (t_conf0_rx_ss == conf1_rx_ss)) ||
		    ((t_conf1_tx_ss == conf1_tx_ss) &&
		    (t_conf1_rx_ss == conf1_rx_ss))) &&
		    (dbs_mode == HW_MODE_DBS)) &&
		    (found_one_by_one < 0)) {
			found_one_by_one = i;
			WMA_LOGI("%s: 1x1 hw_mode index %d found",
					__func__, i);
			/* Once an entry is found, need not check for 1x1
			 * again
			 */
			continue;
		}

		if (((((t_conf0_tx_ss == conf2_tx_ss) &&
		    (t_conf0_rx_ss == conf2_rx_ss)) ||
		    ((t_conf1_tx_ss == conf2_tx_ss) &&
		    (t_conf1_rx_ss == conf2_rx_ss))) &&
		    (dbs_mode == HW_MODE_DBS)) &&
		    (found_two_by_two < 0)) {
			found_two_by_two = i;
			WMA_LOGI("%s: 2x2 hw_mode index %d found",
					__func__, i);
			/* Once an entry is found, need not check for 2x2
			 * again
			 */
			continue;
		}
	}

	if (found_one_by_one >= 0)
		*one_by_one_dbs = true;
	if (found_two_by_two >= 0)
		*two_by_two_dbs = true;

	return CDF_STATUS_SUCCESS;
}

/**
 * wma_get_current_hw_mode() - Get current HW mode params
 * @hw_mode: HW mode parameters
 *
 * Provides the current HW mode parameters if the HW mode is initialized
 * in the driver
 *
 * Return: Success if the current HW mode params are successfully populated
 */
CDF_STATUS wma_get_current_hw_mode(struct sir_hw_mode_params *hw_mode)
{
	CDF_STATUS status;
	uint32_t old_hw_index = 0, new_hw_index = 0;

	WMA_LOGI("%s: Get the current hw mode", __func__);

	status = wma_get_old_and_new_hw_index(&old_hw_index,
			&new_hw_index);
	if (CDF_STATUS_SUCCESS != status) {
		WMA_LOGE("%s: Failed to get HW mode index", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	if (new_hw_index == WMA_DEFAULT_HW_MODE_INDEX) {
		WMA_LOGE("%s: HW mode is not yet initialized", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	status = wma_get_hw_mode_from_idx(new_hw_index, hw_mode);
	if (CDF_STATUS_SUCCESS != status) {
		WMA_LOGE("%s: Failed to get HW mode index", __func__);
		return CDF_STATUS_E_FAILURE;
	}
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_is_dbs_enable() - Check if master DBS control is enabled
 *
 * Checks if the master DBS control is enabled. This will be used
 * to override any other DBS capability
 *
 * Return: True if master DBS control is enabled
 */
bool wma_is_dbs_enable(void)
{
	tp_wma_handle wma;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	WMA_LOGD("%s: DBS=%d", __func__,
	    WMI_DBS_FW_MODE_CFG_DBS_GET(wma->dual_mac_cfg.cur_fw_mode_config));

	if (WMI_DBS_FW_MODE_CFG_DBS_GET(wma->dual_mac_cfg.cur_fw_mode_config))
		return true;

	return false;
}

/**
 * wma_is_agile_dfs_enable() - Check if master Agile DFS control is enabled
 *
 * Checks if the master Agile DFS control is enabled. This will be used
 * to override any other Agile DFS capability
 *
 * Return: True if master Agile DFS control is enabled
 */
bool wma_is_agile_dfs_enable(void)
{
	tp_wma_handle wma;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return false;
	}

	WMA_LOGD("%s: DFS=%d Single mac with DFS=%d", __func__,
			WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(
				wma->dual_mac_cfg.cur_fw_mode_config),
			WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(
				wma->dual_mac_cfg.cur_scan_config));

	if ((WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(
			wma->dual_mac_cfg.cur_fw_mode_config)) &&
			(WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(
					    wma->dual_mac_cfg.cur_scan_config)))
		return true;

	return false;
}

/**
 * wma_get_updated_scan_config() - Get the updated scan configuration
 * @scan_config: Pointer containing the updated scan config
 * @dbs_scan: 0 or 1 indicating if DBS scan needs to be enabled/disabled
 * @dbs_plus_agile_scan: 0 or 1 indicating if DBS plus agile scan needs to be
 * enabled/disabled
 * @single_mac_scan_with_dfs: 0 or 1 indicating if single MAC scan with DFS
 * needs to be enabled/disabled
 *
 * Takes the current scan configuration and set the necessary scan config
 * bits to either 0/1 and provides the updated value to the caller who
 * can use this to pass it on to the FW
 *
 * Return: 0 on success
 */
CDF_STATUS wma_get_updated_scan_config(uint32_t *scan_config,
		bool dbs_scan,
		bool dbs_plus_agile_scan,
		bool single_mac_scan_with_dfs)
{
	tp_wma_handle wma;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return CDF_STATUS_E_FAILURE;
	}
	*scan_config = wma->dual_mac_cfg.cur_scan_config;

	WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_SET(*scan_config, dbs_scan);
	WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_SET(*scan_config,
			dbs_plus_agile_scan);
	WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_SET(*scan_config,
			single_mac_scan_with_dfs);

	WMA_LOGD("%s: *scan_config:%x ", __func__, *scan_config);
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_get_updated_fw_mode_config() - Get the updated fw mode configuration
 * @fw_mode_config: Pointer containing the updated fw mode config
 * @dbs: 0 or 1 indicating if DBS needs to be enabled/disabled
 * @agile_dfs: 0 or 1 indicating if agile DFS needs to be enabled/disabled
 *
 * Takes the current fw mode configuration and set the necessary fw mode config
 * bits to either 0/1 and provides the updated value to the caller who
 * can use this to pass it on to the FW
 *
 * Return: 0 on success
 */
CDF_STATUS wma_get_updated_fw_mode_config(uint32_t *fw_mode_config,
		bool dbs,
		bool agile_dfs)
{
	tp_wma_handle wma;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return CDF_STATUS_E_FAILURE;
	}
	*fw_mode_config = wma->dual_mac_cfg.cur_fw_mode_config;

	WMI_DBS_FW_MODE_CFG_DBS_SET(*fw_mode_config, dbs);
	WMI_DBS_FW_MODE_CFG_AGILE_DFS_SET(*fw_mode_config, agile_dfs);

	WMA_LOGD("%s: *fw_mode_config:%x ", __func__, *fw_mode_config);
	return CDF_STATUS_SUCCESS;
}

/**
 * wma_get_dbs_config() - Get DBS bit
 *
 * Gets the DBS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the DBS bit
 */
bool wma_get_dbs_config(void)
{
	tp_wma_handle wma;
	uint32_t fw_mode_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = wma->dual_mac_cfg.cur_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_DBS_GET(fw_mode_config);
}

/**
 * wma_get_agile_dfs_config() - Get Agile DFS bit
 *
 * Gets the Agile DFS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the Agile DFS bit
 */
bool wma_get_agile_dfs_config(void)
{
	tp_wma_handle wma;
	uint32_t fw_mode_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = wma->dual_mac_cfg.cur_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(fw_mode_config);
}

/**
 * wma_get_dbs_scan_config() - Get DBS scan bit
 *
 * Gets the DBS scan bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the DBS scan bit
 */
bool wma_get_dbs_scan_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.cur_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_GET(scan_config);
}

/**
 * wma_get_dbs_plus_agile_scan_config() - Get DBS plus agile scan bit
 *
 * Gets the DBS plus agile scan bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the DBS plus agile scan bit
 */
bool wma_get_dbs_plus_agile_scan_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.cur_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_GET(scan_config);
}

/**
 * wma_get_single_mac_scan_with_dfs_config() - Get Single MAC scan with DFS bit
 *
 * Gets the Single MAC scan with DFS bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the Single MAC scan with DFS bit
 */
bool wma_get_single_mac_scan_with_dfs_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.cur_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(scan_config);
}

/**
 * wma_is_dual_mac_disabled_in_ini() - Check if dual mac is disabled in INI
 *
 * Checks if the dual mac feature is disabled in INI
 *
 * Return: true if the dual mac feature is disabled from INI
 */
bool wma_is_dual_mac_disabled_in_ini(void)
{
	tpAniSirGlobal mac = cds_get_context(CDF_MODULE_ID_PE);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac pointer", __func__);
		return true;
	}

	if (mac->dual_mac_feature_disable)
		return true;

	return false;
}

/**
 * wma_get_prev_dbs_config() - Get prev DBS bit
 *
 * Gets the previous DBS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the DBS bit
 */
bool wma_get_prev_dbs_config(void)
{
	tp_wma_handle wma;
	uint32_t fw_mode_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = wma->dual_mac_cfg.prev_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_DBS_GET(fw_mode_config);
}

/**
 * wma_get_prev_agile_dfs_config() - Get prev Agile DFS bit
 *
 * Gets the previous Agile DFS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the Agile DFS bit
 */
bool wma_get_prev_agile_dfs_config(void)
{
	tp_wma_handle wma;
	uint32_t fw_mode_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = wma->dual_mac_cfg.prev_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(fw_mode_config);
}

/**
 * wma_get_prev_dbs_scan_config() - Get prev DBS scan bit
 *
 * Gets the previous DBS scan bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the DBS scan bit
 */
bool wma_get_prev_dbs_scan_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.prev_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_GET(scan_config);
}

/**
 * wma_get_prev_dbs_plus_agile_scan_config() - Get prev DBS plus agile scan bit
 *
 * Gets the previous DBS plus agile scan bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the DBS plus agile scan bit
 */
bool wma_get_prev_dbs_plus_agile_scan_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.prev_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_GET(scan_config);
}

/**
 * wma_get_prev_single_mac_scan_with_dfs_config() - Get prev Single MAC scan
 * with DFS bit
 *
 * Gets the previous Single MAC scan with DFS bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the Single MAC scan with DFS bit
 */
bool wma_get_prev_single_mac_scan_with_dfs_config(void)
{
	tp_wma_handle wma;
	uint32_t scan_config;

	if (wma_is_dual_mac_disabled_in_ini())
		return false;

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = wma->dual_mac_cfg.prev_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(scan_config);
}

/**
 * wma_is_scan_simultaneous_capable() - Check if scan parallelization is
 * supported or not
 *
 * currently scan parallelization feature support is dependent on DBS but
 * it can be independent in future.
 *
 * Return: True if master DBS control is enabled
 */
bool wma_is_scan_simultaneous_capable(void)
{
	if (wma_is_hw_dbs_capable())
		return true;

	return false;
}

/**
 * wma_get_vht_ch_width - return vht channel width
 *
 * Return: return vht channel width
 */
uint32_t wma_get_vht_ch_width(void)
{
	uint32_t fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
	tp_wma_handle wm_hdl = cds_get_context(CDF_MODULE_ID_WMA);

	if (NULL == wm_hdl)
		return fw_ch_wd;

	if (wm_hdl->vht_cap_info &
	    IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ)
		fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;
	else if (wm_hdl->vht_cap_info &
		 IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ)
		fw_ch_wd = WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ;

	return fw_ch_wd;
}
