/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *

    \file csr_api_roam.c

    Implementation for the Common Roaming interfaces.
   ========================================================================== */
#include "ani_global.h"          /* for tpAniSirGlobal */
#include "wma_types.h"
#include "wma_if.h"          /* for STA_INVALID_IDX. */
#include "cds_mq.h"
#include "csr_inside_api.h"
#include "sms_debug.h"
#include "sme_trace.h"
#include "sme_qos_internal.h"
#include "sme_inside.h"
#include "host_diag_core_event.h"
#include "host_diag_core_log.h"
#include "csr_api.h"
#include "csr_internal.h"
#include "cds_reg_service.h"
#include "mac_trace.h"
#include "csr_neighbor_roam.h"
#include "cds_regdomain.h"
#include "cds_utils.h"
#include "sir_types.h"
#include "cfg_api.h"
#include "sme_power_save_api.h"
#include "wma.h"
#include "cds_concurrency.h"
#include "sme_nan_datapath.h"
#include "pld_common.h"

#define MAX_PWR_FCC_CHAN_12 8
#define MAX_PWR_FCC_CHAN_13 2

#define CSR_NUM_IBSS_START_CHANNELS_50      4
#define CSR_NUM_IBSS_START_CHANNELS_24      3
/* 5 seconds, for WPA, WPA2, CCKM */
#define CSR_WAIT_FOR_KEY_TIMEOUT_PERIOD     (15 * QDF_MC_TIMER_TO_SEC_UNIT)
/* 120 seconds, for WPS */
#define CSR_WAIT_FOR_WPS_KEY_TIMEOUT_PERIOD (120 * QDF_MC_TIMER_TO_SEC_UNIT)

/*---------------------------------------------------------------------------
   OBIWAN recommends [8 10]% : pick 9%
   ---------------------------------------------------------------------------*/
#define CSR_VCC_UL_MAC_LOSS_THRESHOLD 9
/*---------------------------------------------------------------------------
   OBIWAN recommends -85dBm
   ---------------------------------------------------------------------------*/
#define CSR_VCC_RSSI_THRESHOLD 80
#define CSR_MIN_GLOBAL_STAT_QUERY_PERIOD   500  /* ms */
#define CSR_MIN_GLOBAL_STAT_QUERY_PERIOD_IN_BMPS 2000   /* ms */
#define CSR_MIN_TL_STAT_QUERY_PERIOD       500  /* ms */
/* Flag to send/do not send disassoc frame over the air */
#define CSR_DONT_SEND_DISASSOC_OVER_THE_AIR 1
#define RSSI_HACK_BMPS (-40)
#define MAX_CB_VALUE_IN_INI (2)

#define MAX_SOCIAL_CHANNELS  3
/* Choose the largest possible value that can be accomodates in 8 bit signed */
/* variable. */
#define SNR_HACK_BMPS                         (127)

static bool b_roam_scan_offload_started;

/*--------------------------------------------------------------------------
   Static Type declarations
   ------------------------------------------------------------------------*/
static tCsrRoamSession csr_roam_roam_session[CSR_ROAM_SESSION_MAX];

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
int diag_auth_type_from_csr_type(eCsrAuthType authType)
{
	int n = AUTH_OPEN;
	switch (authType) {
	case eCSR_AUTH_TYPE_SHARED_KEY:
		n = AUTH_SHARED;
		break;
	case eCSR_AUTH_TYPE_WPA:
		n = AUTH_WPA_EAP;
		break;
	case eCSR_AUTH_TYPE_WPA_PSK:
		n = AUTH_WPA_PSK;
		break;
	case eCSR_AUTH_TYPE_RSN:
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
#endif
		n = AUTH_WPA2_EAP;
		break;
	case eCSR_AUTH_TYPE_RSN_PSK:
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
#endif
		n = AUTH_WPA2_PSK;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		n = AUTH_WAPI_CERT;
		break;
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		n = AUTH_WAPI_PSK;
		break;
#endif /* FEATURE_WLAN_WAPI */
	default:
		break;
	}
	return n;
}

int diag_enc_type_from_csr_type(eCsrEncryptionType encType)
{
	int n = ENC_MODE_OPEN;
	switch (encType) {
	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP40:
		n = ENC_MODE_WEP40;
		break;
	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP104:
		n = ENC_MODE_WEP104;
		break;
	case eCSR_ENCRYPT_TYPE_TKIP:
		n = ENC_MODE_TKIP;
		break;
	case eCSR_ENCRYPT_TYPE_AES:
		n = ENC_MODE_AES;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
		n = ENC_MODE_SMS4;
		break;
#endif /* FEATURE_WLAN_WAPI */
	default:
		break;
	}
	return n;
}
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */
static const uint8_t
 csr_start_ibss_channels50[CSR_NUM_IBSS_START_CHANNELS_50] = { 36, 40, 44, 48 };
static const uint8_t
 csr_start_ibss_channels24[CSR_NUM_IBSS_START_CHANNELS_24] = { 1, 6, 11 };
static void init_config_param(tpAniSirGlobal pMac);
static bool csr_roam_process_results(tpAniSirGlobal pMac, tSmeCmd *pCommand,
				     eCsrRoamCompleteResult Result,
				     void *Context);
static QDF_STATUS csr_roam_start_ibss(tpAniSirGlobal pMac, uint32_t sessionId,
				      tCsrRoamProfile *pProfile,
				      bool *pfSameIbss);
static void csr_roam_update_connected_profile_from_new_bss(tpAniSirGlobal pMac,
							   uint32_t sessionId,
							   tSirSmeNewBssInfo *
							   pNewBss);
static ePhyChanBondState csr_get_cb_mode_from_ies(tpAniSirGlobal pMac,
						  uint8_t primaryChn,
						  tDot11fBeaconIEs *pIes);

static void csr_roaming_state_config_cnf_processor(tpAniSirGlobal pMac,
						   uint32_t result);
QDF_STATUS csr_roam_open(tpAniSirGlobal pMac);
QDF_STATUS csr_roam_close(tpAniSirGlobal pMac);
void csr_roamMICErrorTimerHandler(void *pv);
void csr_roamTKIPCounterMeasureTimerHandler(void *pv);
bool csr_roam_is_same_profile_keys(tpAniSirGlobal pMac,
				   tCsrRoamConnectedProfile *pConnProfile,
				   tCsrRoamProfile *pProfile2);

static QDF_STATUS csr_roam_start_roaming_timer(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       uint32_t interval);
static QDF_STATUS csr_roam_stop_roaming_timer(tpAniSirGlobal pMac,
					      uint32_t sessionId);
static void csr_roam_roaming_timer_handler(void *pv);
QDF_STATUS csr_roam_start_wait_for_key_timer(tpAniSirGlobal pMac, uint32_t interval);
static void csr_roam_wait_for_key_time_out_handler(void *pv);
static QDF_STATUS csr_init11d_info(tpAniSirGlobal pMac, tCsr11dinfo *ps11dinfo);
static QDF_STATUS csr_init_channel_power_list(tpAniSirGlobal pMac,
					      tCsr11dinfo *ps11dinfo);
static QDF_STATUS csr_roam_free_connected_info(tpAniSirGlobal pMac,
					       tCsrRoamConnectedInfo *
					       pConnectedInfo);
QDF_STATUS csr_send_mb_set_context_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					   struct qdf_mac_addr peer_macaddr,
					    uint8_t numKeys,
					   tAniEdType edType, bool fUnicast,
					   tAniKeyDirection aniKeyDirection,
					   uint8_t keyId, uint8_t keyLength,
					   uint8_t *pKey, uint8_t paeRole,
					   uint8_t *pKeyRsc);
void csr_roamStatisticsTimerHandler(void *pv);
void csr_roamStatsGlobalClassDTimerHandler(void *pv);
static void csr_roam_link_up(tpAniSirGlobal pMac, struct qdf_mac_addr bssid);
static void csr_roam_link_down(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_roam_vcc_trigger(tpAniSirGlobal pMac);
QDF_STATUS csr_send_mb_stats_req_msg(tpAniSirGlobal pMac, uint32_t statsMask,
				     uint8_t staId, uint8_t sessionId);
/*
    pStaEntry is no longer invalid upon the return of this function.
 */
static void csr_roam_remove_stat_list_entry(tpAniSirGlobal pMac, tListElem *pEntry);
static eCsrCfgDot11Mode csr_roam_get_phy_mode_band_for_bss(tpAniSirGlobal pMac,
							   tCsrRoamProfile *pProfile,
							   uint8_t operationChn,
							   eCsrBand *pBand);
static QDF_STATUS csr_roam_get_qos_info_from_bss(tpAniSirGlobal pMac,
						 tSirBssDescription *pBssDesc);
tCsrStatsClientReqInfo *csr_roam_insert_entry_into_list(tpAniSirGlobal pMac,
							tDblLinkList *pStaList,
							tCsrStatsClientReqInfo *
							pStaEntry);
void csr_roam_stats_client_timer_handler(void *pv);
tCsrPeStatsReqInfo *csr_roam_check_pe_stats_req_list(tpAniSirGlobal pMac,
						     uint32_t statsMask,
						     uint32_t periodicity,
						     bool *pFound, uint8_t staId,
						     uint8_t sessionId);
void csr_roam_report_statistics(tpAniSirGlobal pMac, uint32_t statsMask,
				tCsrStatsCallback callback, uint8_t staId,
				void *pContext);
void csr_roam_tl_stats_timer_handler(void *pv);
void csr_roam_pe_stats_timer_handler(void *pv);
tListElem *csr_roam_check_client_req_list(tpAniSirGlobal pMac, uint32_t statsMask);
void csr_roam_remove_entry_from_pe_stats_req_list(tpAniSirGlobal pMac,
						  tCsrPeStatsReqInfo *pPeStaEntry);
tListElem *csr_roam_find_in_pe_stats_req_list(tpAniSirGlobal pMac, uint32_t statsMask);
QDF_STATUS csr_roam_dereg_statistics_req(tpAniSirGlobal pMac);
static uint32_t csr_find_ibss_session(tpAniSirGlobal pMac);
static uint32_t csr_find_session_by_type(tpAniSirGlobal,
					enum tQDF_ADAPTER_MODE);
static bool csr_is_conn_allow_2g_band(tpAniSirGlobal pMac, uint32_t chnl);
static bool csr_is_conn_allow_5g_band(tpAniSirGlobal pMac, uint32_t chnl);
static QDF_STATUS csr_roam_start_wds(tpAniSirGlobal pMac, uint32_t sessionId,
				     tCsrRoamProfile *pProfile,
				     tSirBssDescription *pBssDesc);
static void csr_init_session(tpAniSirGlobal pMac, uint32_t sessionId);
static QDF_STATUS csr_roam_issue_set_key_command(tpAniSirGlobal pMac,
						 uint32_t sessionId,
						 tCsrRoamSetKey *pSetKey,
						 uint32_t roamId);
static QDF_STATUS csr_roam_get_qos_info_from_bss(tpAniSirGlobal pMac,
						 tSirBssDescription *pBssDesc);
void csr_roam_reissue_roam_command(tpAniSirGlobal pMac);
static void csr_ser_des_unpack_diassoc_rsp(uint8_t *pBuf,
					   tSirSmeDisassocRsp *pRsp);
void csr_init_operating_classes(tHalHandle hHal);

/* Initialize global variables */
static void csr_roam_init_globals(tpAniSirGlobal pMac)
{
	if (pMac) {
		qdf_mem_zero(&csr_roam_roam_session, sizeof(csr_roam_roam_session));
		pMac->roam.roamSession = csr_roam_roam_session;
	}
	return;
}

static void csr_roam_de_init_globals(tpAniSirGlobal pMac)
{
	if (pMac) {
		pMac->roam.roamSession = NULL;
	}
	return;
}

QDF_STATUS csr_open(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i;

	do {
		/* Initialize CSR Roam Globals */
		csr_roam_init_globals(pMac);
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++)
			csr_roam_state_change(pMac, eCSR_ROAMING_STATE_STOP, i);

		init_config_param(pMac);
		status = csr_scan_open(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		status = csr_roam_open(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		pMac->roam.nextRoamId = 1;      /* Must not be 0 */
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_ll_open(pMac->hHdd,
					 &pMac->roam.statsClientReqList)))
			break;
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_ll_open(pMac->hHdd,
					 &pMac->roam.peStatsReqList)))
			break;
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_ll_open(pMac->hHdd,
					 &pMac->roam.roamCmdPendingList)))
			break;
	} while (0);

	return status;
}

QDF_STATUS csr_init_chan_list(tpAniSirGlobal mac, uint8_t *alpha2)
{
	QDF_STATUS status;

	mac->scan.countryCodeDefault[0] = alpha2[0];
	mac->scan.countryCodeDefault[1] = alpha2[1];
	mac->scan.countryCodeDefault[2] = alpha2[2];

	sms_log(mac, LOGE, FL("init time country code %.2s"),
		mac->scan.countryCodeDefault);

	mac->scan.domainIdDefault = 0;
	mac->scan.domainIdCurrent = 0;

	qdf_mem_copy(mac->scan.countryCodeCurrent,
		     mac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
	qdf_mem_copy(mac->scan.countryCodeElected,
		     mac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
	status = csr_get_channel_and_power_list(mac);
	csr_clear_votes_for_country_info(mac);
	return status;
}

QDF_STATUS csr_set_reg_info(tHalHandle hHal, uint8_t *apCntryCode)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	v_REGDOMAIN_t regId;
	uint8_t cntryCodeLength;
	if (NULL == apCntryCode) {
		sms_log(pMac, LOGE, FL(" Invalid country Code Pointer"));
		return QDF_STATUS_E_FAILURE;
	}
	sms_log(pMac, LOG1, FL(" country Code %.2s"), apCntryCode);

	cntryCodeLength = WNI_CFG_COUNTRY_CODE_LEN;
	status = csr_get_regulatory_domain_for_country(pMac, apCntryCode, &regId,
						       SOURCE_USERSPACE);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE,
			FL("  fail to get regId for country Code %.2s"),
			apCntryCode);
		return status;
	}
	status = wma_set_reg_domain(hHal, regId);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE,
			FL("  fail to get regId for country Code %.2s"),
			apCntryCode);
		return status;
	}
	pMac->scan.domainIdDefault = regId;
	pMac->scan.domainIdCurrent = pMac->scan.domainIdDefault;
	/* Clear CC field */
	qdf_mem_set(pMac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN, 0);

	/* Copy 2 or 3 bytes country code */
	qdf_mem_copy(pMac->scan.countryCodeDefault, apCntryCode,
		     cntryCodeLength);

	/* If 2 bytes country code, 3rd byte must be filled with space */
	if ((WNI_CFG_COUNTRY_CODE_LEN - 1) == cntryCodeLength) {
		qdf_mem_set(pMac->scan.countryCodeDefault + 2, 1, 0x20);
	}
	qdf_mem_copy(pMac->scan.countryCodeCurrent,
		     pMac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
	status = csr_get_channel_and_power_list(pMac);
	return status;
}

QDF_STATUS csr_set_channels(tHalHandle hHal, tCsrConfigParam *pParam)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t index = 0;
	qdf_mem_copy(pParam->Csr11dinfo.countryCode,
		     pMac->scan.countryCodeCurrent, WNI_CFG_COUNTRY_CODE_LEN);
	for (index = 0; index < pMac->scan.base_channels.numChannels;
	     index++) {
		pParam->Csr11dinfo.Channels.channelList[index] =
			pMac->scan.base_channels.channelList[index];
		pParam->Csr11dinfo.ChnPower[index].firstChannel =
			pMac->scan.base_channels.channelList[index];
		pParam->Csr11dinfo.ChnPower[index].numChannels = 1;
		pParam->Csr11dinfo.ChnPower[index].maxtxPower =
			pMac->scan.defaultPowerTable[index].power;
	}
	pParam->Csr11dinfo.Channels.numChannels =
		pMac->scan.base_channels.numChannels;

	return status;
}

QDF_STATUS csr_close(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	csr_roam_close(pMac);
	csr_scan_close(pMac);
	csr_ll_close(&pMac->roam.statsClientReqList);
	csr_ll_close(&pMac->roam.peStatsReqList);
	csr_ll_close(&pMac->roam.roamCmdPendingList);
	if (pMac->sme.saved_scan_cmd) {
		qdf_mem_free(pMac->sme.saved_scan_cmd);
		pMac->sme.saved_scan_cmd = NULL;
	}
	/* DeInit Globals */
	csr_roam_de_init_globals(pMac);
	return status;
}

static int8_t csr_find_channel_pwr(struct channel_power *
					     pdefaultPowerTable,
					     uint8_t ChannelNum)
{
	uint8_t i;
	/* TODO: if defaultPowerTable is guaranteed to be in ascending */
	/* order of channel numbers, we can employ binary search */
	for (i = 0; i < WNI_CFG_VALID_CHANNEL_LIST_LEN; i++) {
		if (pdefaultPowerTable[i].chan_num == ChannelNum)
			return pdefaultPowerTable[i].power;
	}
	/* could not find the channel list in default list */
	/* this should not have occured */
	QDF_ASSERT(0);
	return 0;
}

/**
 * csr_roam_arrange_ch_list() - Updates the channel list modified with greedy
 * order for 5 Ghz preference and DFS channels.
 * @mac_ctx: pointer to mac context.
 * @chan_list:    channel list updated with greedy channel order.
 * @num_channel:  Number of channels in list
 *
 * To allow Early Stop Roaming Scan feature to co-exist with 5G preference,
 * this function moves 5G channels ahead of 2G channels. This function can
 * also move 2G channels, ahead of DFS channel or vice versa. Order is
 * maintained among same category channels
 *
 * Return: None
 */
void csr_roam_arrange_ch_list(tpAniSirGlobal mac_ctx,
			tSirUpdateChanParam *chan_list, uint8_t num_channel)
{
	bool prefer_5g = CSR_IS_ROAM_PREFER_5GHZ(mac_ctx);
	bool prefer_dfs = CSR_IS_DFS_CH_ROAM_ALLOWED(mac_ctx);
	int i, j = 0;
	tSirUpdateChanParam *tmp_list = NULL;

	if (!prefer_5g)
		return;

	tmp_list = (tSirUpdateChanParam *)
		qdf_mem_malloc(sizeof(tSirUpdateChanParam) * num_channel);
	if (tmp_list == NULL) {
		sms_log(mac_ctx, LOGE, FL("Memory allocation failed"));
		return;
	}

	/* Fist copy Non-DFS 5g channels */
	for (i = 0; i < num_channel; i++) {
		if (CDS_IS_CHANNEL_5GHZ(chan_list[i].chanId) &&
			!CDS_IS_DFS_CH(chan_list[i].chanId)) {
			qdf_mem_copy(&tmp_list[j++],
				&chan_list[i], sizeof(tSirUpdateChanParam));
			chan_list[i].chanId = INVALID_CHANNEL_ID;
		}
	}
	if (prefer_dfs) {
		/* next copy DFS channels (remaining channels in 5G) */
		for (i = 0; i < num_channel; i++) {
			if (CDS_IS_CHANNEL_5GHZ(chan_list[i].chanId)) {
				qdf_mem_copy(&tmp_list[j++], &chan_list[i],
					sizeof(tSirUpdateChanParam));
				chan_list[i].chanId = INVALID_CHANNEL_ID;
			}
		}
	} else {
		/* next copy 2G channels */
		for (i = 0; i < num_channel; i++) {
			if (CDS_IS_CHANNEL_24GHZ(chan_list[i].chanId)) {
				qdf_mem_copy(&tmp_list[j++], &chan_list[i],
					sizeof(tSirUpdateChanParam));
				chan_list[i].chanId = INVALID_CHANNEL_ID;
			}
		}
	}
	/* copy rest of the channels in same order to tmp list */
	for (i = 0; i < num_channel; i++) {
		if (chan_list[i].chanId != INVALID_CHANNEL_ID) {
			qdf_mem_copy(&tmp_list[j++], &chan_list[i],
				sizeof(tSirUpdateChanParam));
			chan_list[i].chanId = INVALID_CHANNEL_ID;
		}
	}
	/* copy tmp list to original channel list buffer */
	qdf_mem_copy(chan_list, tmp_list,
				 sizeof(tSirUpdateChanParam) * num_channel);
	qdf_mem_free(tmp_list);
}

/**
 * csr_roam_sort_channel_for_early_stop() - Sort the channels
 * @mac_ctx:        mac global context
 * @chan_list:      Original channel list from the upper layers
 * @num_channel:    Number of original channels
 *
 * For Early stop scan feature, the channel list should be in an order,
 * where-in there is a maximum chance to detect an AP in the initial
 * channels in the list so that the scanning can be stopped early as the
 * feature demands.
 * Below fixed greedy channel list has been provided
 * based on most of the enterprise wifi installations across the globe.
 *
 * Identify all the greedy channels within the channel list from user space.
 * Identify all the non-greedy channels in the user space channel list.
 * Merge greedy channels followed by non-greedy channels back into the
 * chan_list.
 *
 * Return: None
 */
void csr_roam_sort_channel_for_early_stop(tpAniSirGlobal mac_ctx,
			tSirUpdateChanList *chan_list, uint8_t num_channel)
{
	tSirUpdateChanList *chan_list_greedy, *chan_list_non_greedy;
	uint8_t i, j;
	static const uint8_t fixed_greedy_chan_list[] = {1, 6, 11, 36, 48, 40,
		44, 10, 2, 9, 149, 157, 161, 3, 4, 8, 153, 165, 7, 5, 136, 140,
		52, 116, 56, 104, 64, 60, 100, 120, 13, 14, 112, 132, 151, 155};
	uint8_t num_fixed_greedy_chan;
	uint8_t num_greedy_chan = 0;
	uint8_t num_non_greedy_chan = 0;
	uint8_t match_found = false;
	uint32_t buf_size;

	buf_size = sizeof(tSirUpdateChanList) +
		(sizeof(tSirUpdateChanParam) * num_channel);
	chan_list_greedy = qdf_mem_malloc(buf_size);
	chan_list_non_greedy = qdf_mem_malloc(buf_size);
	if (!chan_list_greedy || !chan_list_non_greedy) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Failed to allocate memory for tSirUpdateChanList");
		return;
	}
	qdf_mem_zero(chan_list_greedy, buf_size);
	qdf_mem_zero(chan_list_non_greedy, buf_size);
	/*
	 * fixed_greedy_chan_list is an evaluated channel list based on most of
	 * the enterprise wifi deployments and the order of the channels
	 * determines the highest possibility of finding an AP.
	 * chan_list is the channel list provided by upper layers based on the
	 * regulatory domain.
	 */
	num_fixed_greedy_chan = sizeof(fixed_greedy_chan_list)/sizeof(uint8_t);
	/*
	 * Browse through the chan_list and put all the non-greedy channels
	 * into a seperate list by name chan_list_non_greedy
	 */
	for (i = 0; i < num_channel; i++) {
		for (j = 0; j < num_fixed_greedy_chan; j++) {
			if (chan_list->chanParam[i].chanId ==
					fixed_greedy_chan_list[j]) {
				match_found = true;
				break;
			}
		}
		if (!match_found) {
			qdf_mem_copy(
			  &chan_list_non_greedy->chanParam[num_non_greedy_chan],
			  &chan_list->chanParam[i],
			  sizeof(tSirUpdateChanParam));
			num_non_greedy_chan++;
		} else {
			match_found = false;
		}
	}
	/*
	 * Browse through the fixed_greedy_chan_list and put all the greedy
	 * channels in the chan_list into a seperate list by name
	 * chan_list_greedy
	 */
	for (i = 0; i < num_fixed_greedy_chan; i++) {
		for (j = 0; j < num_channel; j++) {
			if (fixed_greedy_chan_list[i] ==
					chan_list->chanParam[j].chanId) {
				qdf_mem_copy(
				  &chan_list_greedy->chanParam[num_greedy_chan],
				  &chan_list->chanParam[j],
				  sizeof(tSirUpdateChanParam));
				num_greedy_chan++;
				break;
			}
		}
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
		"greedy=%d, non-greedy=%d, tot=%d",
		num_greedy_chan, num_non_greedy_chan, num_channel);
	if ((num_greedy_chan + num_non_greedy_chan) != num_channel) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			"incorrect sorting of channels");
		goto scan_list_sort_error;
	}
	/* Copy the Greedy channels first */
	i = 0;
	qdf_mem_copy(&chan_list->chanParam[i],
		&chan_list_greedy->chanParam[i],
		num_greedy_chan * sizeof(tSirUpdateChanParam));
	/* Copy the remaining Non Greedy channels */
	i = num_greedy_chan;
	j = 0;
	qdf_mem_copy(&chan_list->chanParam[i],
		&chan_list_non_greedy->chanParam[j],
		num_non_greedy_chan * sizeof(tSirUpdateChanParam));

	/* Update channel list for 5g preference and allow DFS roam */
	csr_roam_arrange_ch_list(mac_ctx, chan_list->chanParam, num_channel);
scan_list_sort_error:
	qdf_mem_free(chan_list_greedy);
	qdf_mem_free(chan_list_non_greedy);
}

QDF_STATUS csr_update_channel_list(tpAniSirGlobal pMac)
{
	tSirUpdateChanList *pChanList;
	tCsrScanStruct *pScan = &pMac->scan;
	uint8_t numChan = pScan->base_channels.numChannels;
	uint8_t num_channel = 0;
	uint32_t bufLen;
	cds_msg_t msg;
	uint8_t i, j, social_channel[MAX_SOCIAL_CHANNELS] = { 1, 6, 11 };
	uint8_t channel_state;
	uint16_t unsafe_chan[NUM_CHANNELS];
	uint16_t unsafe_chan_cnt = 0;
	uint16_t cnt = 0;
	uint8_t  channel;
	bool is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
		    &unsafe_chan_cnt,
		    sizeof(unsafe_chan));

	if (CSR_IS_5G_BAND_ONLY(pMac)) {
		for (i = 0; i < MAX_SOCIAL_CHANNELS; i++) {
			if (cds_get_channel_state(social_channel[i])
			    == CHANNEL_STATE_ENABLE)
				numChan++;
		}
	}

	bufLen = sizeof(tSirUpdateChanList) +
		 (sizeof(tSirUpdateChanParam) * (numChan));

	csr_init_operating_classes((tHalHandle) pMac);
	pChanList = (tSirUpdateChanList *) qdf_mem_malloc(bufLen);
	if (!pChanList) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Failed to allocate memory for tSirUpdateChanList");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(pChanList, bufLen);

	for (i = 0; i < pScan->base_channels.numChannels; i++) {
		/* Scan is not performed on DSRC channels*/
		if (pScan->base_channels.channelList[i] >= CDS_MIN_11P_CHANNEL)
			continue;
		channel = pScan->base_channels.channelList[i];

		channel_state =
			cds_get_channel_state(
				pScan->base_channels.channelList[i]);
		if ((CHANNEL_STATE_ENABLE == channel_state) ||
		    pMac->scan.fEnableDFSChnlScan) {
			if ((pMac->roam.configParam.sta_roam_policy.dfs_mode ==
				CSR_STA_ROAM_POLICY_DFS_DISABLED) &&
				(channel_state == CHANNEL_STATE_DFS)) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_INFO,
					FL("skip dfs channel %d"),
					channel);
				continue;
			}
			if (pMac->roam.configParam.sta_roam_policy.
					skip_unsafe_channels &&
					unsafe_chan_cnt) {
				is_unsafe_chan = false;
				for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
					if (unsafe_chan[cnt] == channel) {
						is_unsafe_chan = true;
						break;
					}
				}
				if (is_unsafe_chan) {
					QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_INFO,
					FL("ignoring unsafe channel %d"),
					channel);
					continue;
				}
			}
			pChanList->chanParam[num_channel].chanId =
				pScan->base_channels.channelList[i];
			pChanList->chanParam[num_channel].pwr =
				csr_find_channel_pwr(pScan->defaultPowerTable,
				  pChanList->chanParam[num_channel].chanId);

			if (pScan->fcc_constraint) {
				if (12 == pChanList->chanParam[num_channel].chanId) {
					pChanList->chanParam[num_channel].pwr =
						MAX_PWR_FCC_CHAN_12;
					QDF_TRACE(QDF_MODULE_ID_SME,
						  QDF_TRACE_LEVEL_INFO,
						  "txpow for channel 12 is %d",
						  MAX_PWR_FCC_CHAN_12);
				}
				if (13 == pChanList->chanParam[num_channel].chanId) {
					pChanList->chanParam[num_channel].pwr =
						MAX_PWR_FCC_CHAN_13;
					QDF_TRACE(QDF_MODULE_ID_SME,
						  QDF_TRACE_LEVEL_INFO,
						  "txpow for channel 13 is %d",
						  MAX_PWR_FCC_CHAN_13);
				}
			}


			if (CHANNEL_STATE_ENABLE == channel_state)
				pChanList->chanParam[num_channel].dfsSet =
					false;
			else
				pChanList->chanParam[num_channel].dfsSet =
					true;
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				"channel:%d, pwr=%d, DFS=%d\n",
				pChanList->chanParam[num_channel].chanId,
				pChanList->chanParam[num_channel].pwr,
				pChanList->chanParam[num_channel].dfsSet);
			if (cds_is_5_mhz_enabled())
				pChanList->chanParam[num_channel].quarter_rate
					= 1;
			else if (cds_is_10_mhz_enabled())
				pChanList->chanParam[num_channel].half_rate = 1;
			num_channel++;
		}
	}

	if (CSR_IS_5G_BAND_ONLY(pMac)) {
		for (j = 0; j < MAX_SOCIAL_CHANNELS; j++) {
			if (cds_get_channel_state(social_channel[j])
			    != CHANNEL_STATE_ENABLE)
				continue;
			pChanList->chanParam[num_channel].chanId =
				social_channel[j];
			pChanList->chanParam[num_channel].pwr =
				csr_find_channel_pwr(pScan->defaultPowerTable,
						     social_channel[j]);
			pChanList->chanParam[num_channel].dfsSet = false;
			if (cds_is_5_mhz_enabled())
				pChanList->chanParam[num_channel].quarter_rate
									= 1;
			else if (cds_is_10_mhz_enabled())
				pChanList->chanParam[num_channel].half_rate = 1;
			num_channel++;
		}
	}
	if (pMac->roam.configParam.early_stop_scan_enable)
		csr_roam_sort_channel_for_early_stop(pMac, pChanList,
						     num_channel);
	else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			FL("Early Stop Scan Feature not supported"));

	if ((pMac->roam.configParam.uCfgDot11Mode ==
				eCSR_CFG_DOT11_MODE_AUTO) ||
			(pMac->roam.configParam.uCfgDot11Mode ==
			 eCSR_CFG_DOT11_MODE_11AC) ||
			(pMac->roam.configParam.uCfgDot11Mode ==
			 eCSR_CFG_DOT11_MODE_11AC_ONLY)) {
		pChanList->vht_en = true;
		if (pMac->roam.configParam.enableVhtFor24GHz)
			pChanList->vht_24_en = true;
	}
	if ((pMac->roam.configParam.uCfgDot11Mode ==
				eCSR_CFG_DOT11_MODE_AUTO) ||
			(pMac->roam.configParam.uCfgDot11Mode ==
			 eCSR_CFG_DOT11_MODE_11N) ||
			(pMac->roam.configParam.uCfgDot11Mode ==
			 eCSR_CFG_DOT11_MODE_11N_ONLY)) {
		pChanList->ht_en = true;
	}
	msg.type = WMA_UPDATE_CHAN_LIST_REQ;
	msg.reserved = 0;
	msg.bodyptr = pChanList;
	pChanList->numChan = num_channel;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 NO_SESSION, msg.type));
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: Failed to post msg to WMA", __func__);
		qdf_mem_free(pChanList);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_start(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i;

	do {
		/* save the global cds context */
		pMac->roam.g_cds_context = cds_get_global_context();
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++)
			csr_roam_state_change(pMac, eCSR_ROAMING_STATE_IDLE, i);

		status = csr_roam_start(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;

		pMac->roam.sPendingCommands = 0;
		csr_scan_enable(pMac);
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++)
			status = csr_neighbor_roam_init(pMac, i);
		pMac->roam.tlStatsReqInfo.numClient = 0;
		pMac->roam.tlStatsReqInfo.periodicity = 0;
		pMac->roam.tlStatsReqInfo.timerRunning = false;
		/* init the link quality indication also */
		pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_MIN_IND;
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGW,
				" csr_start: Couldn't Init HO control blk ");
			break;
		}

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "Scan offload is enabled, update default chan list");
		status = csr_update_channel_list(pMac);
	} while (0);
	return status;
}

QDF_STATUS csr_stop(tpAniSirGlobal pMac, tHalStopType stopType)
{
	uint32_t sessionId;

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		csr_roam_close_session(pMac, sessionId, true, NULL, NULL);
	}
	csr_scan_disable(pMac);
	pMac->scan.fCancelIdleScan = false;
	pMac->scan.fRestartIdleScan = false;
	csr_ll_purge(&pMac->roam.roamCmdPendingList, true);

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++)
		csr_neighbor_roam_close(pMac, sessionId);
	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++)
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			csr_scan_flush_result(pMac);

	/* Reset the domain back to the deault */
	pMac->scan.domainIdCurrent = pMac->scan.domainIdDefault;

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_STOP, sessionId);
		pMac->roam.curSubState[sessionId] = eCSR_ROAM_SUBSTATE_NONE;
	}

	/* When HAL resets all the context information
	 * in HAL is lost, so we might need to send the
	 * scan offload request again when it comes
	 * out of reset for scan offload to be functional
	 */
	if (HAL_STOP_TYPE_SYS_RESET == stopType) {
		b_roam_scan_offload_started = false;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_ready(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	/* If the gScanAgingTime is set to '0' then scan results aging timeout
	   based  on timer feature is not enabled */

	status = csr_apply_channel_and_power_list(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"csr_apply_channel_and_power_list failed during csr_ready with status=%d",
			status);
	}
	return status;
}

void csr_set_default_dot11_mode(tpAniSirGlobal pMac)
{
	uint32_t wniDot11mode = 0;
	wniDot11mode =
		csr_translate_to_wni_cfg_dot11_mode(pMac,
						    pMac->roam.configParam.uCfgDot11Mode);
	cfg_set_int(pMac, WNI_CFG_DOT11_MODE, wniDot11mode);
}

void csr_set_global_cfgs(tpAniSirGlobal pMac)
{

	cfg_set_int(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD,
			csr_get_frag_thresh(pMac));
	cfg_set_int(pMac, WNI_CFG_RTS_THRESHOLD, csr_get_rts_thresh(pMac));
	cfg_set_int(pMac, WNI_CFG_11D_ENABLED,
			((pMac->roam.configParam.Is11hSupportEnabled) ? pMac->roam.
			 configParam.Is11dSupportEnabled : pMac->roam.configParam.
			 Is11dSupportEnabled));
	cfg_set_int(pMac, WNI_CFG_11H_ENABLED,
			pMac->roam.configParam.Is11hSupportEnabled);
	/* For now we will just use the 5GHz CB mode ini parameter to decide whether CB supported or not in Probes when there is no session
	 * Once session is established we will use the session related params stored in PE session for CB mode
	 */
	cfg_set_int(pMac, WNI_CFG_CHANNEL_BONDING_MODE,
			!!(pMac->roam.configParam.channelBondingMode5GHz));
	cfg_set_int(pMac, WNI_CFG_HEART_BEAT_THRESHOLD,
			pMac->roam.configParam.HeartbeatThresh24);

	/* Update the operating mode to configured value during initialization, */
	/* So that client can advertise full capabilities in Probe request frame. */
	csr_set_default_dot11_mode(pMac);
}

QDF_STATUS csr_roam_open(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i;
	tCsrRoamSession *pSession;
	do {
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
			pSession = CSR_GET_SESSION(pMac, i);
			pSession->roamingTimerInfo.pMac = pMac;
			pSession->roamingTimerInfo.sessionId =
				CSR_SESSION_ID_INVALID;
		}
		pMac->roam.WaitForKeyTimerInfo.pMac = pMac;
		pMac->roam.WaitForKeyTimerInfo.sessionId =
			CSR_SESSION_ID_INVALID;
		status =
			qdf_mc_timer_init(&pMac->roam.hTimerWaitForKey,
					  QDF_TIMER_TYPE_SW,
					  csr_roam_wait_for_key_time_out_handler,
					  &pMac->roam.WaitForKeyTimerInfo);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL
					("cannot allocate memory for WaitForKey time out timer"));
			break;
		}
		status =
			qdf_mc_timer_init(&pMac->roam.tlStatsReqInfo.hTlStatsTimer,
					  QDF_TIMER_TYPE_SW,
					  csr_roam_tl_stats_timer_handler, pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL
					("cannot allocate memory for summary Statistics timer"));
			return QDF_STATUS_E_FAILURE;
		}
	} while (0);
	return status;
}

QDF_STATUS csr_roam_close(tpAniSirGlobal pMac)
{
	uint32_t sessionId;
	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		csr_roam_close_session(pMac, sessionId, true, NULL, NULL);
	}
	qdf_mc_timer_stop(&pMac->roam.hTimerWaitForKey);
	qdf_mc_timer_destroy(&pMac->roam.hTimerWaitForKey);
	qdf_mc_timer_stop(&pMac->roam.tlStatsReqInfo.hTlStatsTimer);
	qdf_mc_timer_destroy(&pMac->roam.tlStatsReqInfo.hTlStatsTimer);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_start(tpAniSirGlobal pMac)
{
	(void)pMac;
	return QDF_STATUS_SUCCESS;
}

void csr_roam_stop(tpAniSirGlobal pMac, uint32_t sessionId)
{
	csr_roam_stop_roaming_timer(pMac, sessionId);
	/* deregister the clients requesting stats from PE/TL & also stop the corresponding timers */
	csr_roam_dereg_statistics_req(pMac);
}

QDF_STATUS csr_roam_get_connect_state(tpAniSirGlobal pMac, uint32_t sessionId,
				      eCsrConnectState *pState)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	if (CSR_IS_SESSION_VALID(pMac, sessionId) && (NULL != pState)) {
		status = QDF_STATUS_SUCCESS;
		*pState = pMac->roam.roamSession[sessionId].connectState;
	}
	return status;
}

QDF_STATUS csr_roam_copy_connect_profile(tpAniSirGlobal pMac,
			uint32_t sessionId, tCsrRoamConnectedProfile *pProfile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t size = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tCsrRoamConnectedProfile *connected_prof;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	if (!pProfile) {
		sms_log(pMac, LOGE, FL("profile not found"));
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->pConnectBssDesc) {
		size = pSession->pConnectBssDesc->length +
			sizeof(pSession->pConnectBssDesc->length);
		if (size) {
			pProfile->pBssDesc = qdf_mem_malloc(size);
			if (NULL != pProfile->pBssDesc) {
				qdf_mem_copy(pProfile->pBssDesc,
					pSession->pConnectBssDesc,
					size);
				status = QDF_STATUS_SUCCESS;
			} else {
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			pProfile->pBssDesc = NULL;
		}
		connected_prof = &(pSession->connectedProfile);
		pProfile->AuthType = connected_prof->AuthType;
		pProfile->EncryptionType = connected_prof->EncryptionType;
		pProfile->mcEncryptionType = connected_prof->mcEncryptionType;
		pProfile->BSSType = connected_prof->BSSType;
		pProfile->operationChannel = connected_prof->operationChannel;
		pProfile->CBMode = connected_prof->CBMode;
		qdf_mem_copy(&pProfile->bssid, &connected_prof->bssid,
			sizeof(struct qdf_mac_addr));
		qdf_mem_copy(&pProfile->SSID, &connected_prof->SSID,
			sizeof(tSirMacSSid));
		if (connected_prof->MDID.mdiePresent) {
			pProfile->MDID.mdiePresent = 1;
			pProfile->MDID.mobilityDomain =
				connected_prof->MDID.mobilityDomain;
		} else {
			pProfile->MDID.mdiePresent = 0;
			pProfile->MDID.mobilityDomain = 0;
		}
#ifdef FEATURE_WLAN_ESE
		pProfile->isESEAssoc = connected_prof->isESEAssoc;
		if (csr_is_auth_type_ese(connected_prof->AuthType)) {
			qdf_mem_copy(pProfile->eseCckmInfo.krk,
				connected_prof->eseCckmInfo.krk,
				SIR_KRK_KEY_LEN);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			qdf_mem_copy(pProfile->eseCckmInfo.btk,
				connected_prof->eseCckmInfo.btk,
				SIR_BTK_KEY_LEN);
#endif
			pProfile->eseCckmInfo.reassoc_req_num =
				connected_prof->eseCckmInfo.reassoc_req_num;
			pProfile->eseCckmInfo.krk_plumbed =
				connected_prof->eseCckmInfo.krk_plumbed;
		}
#endif
#ifdef WLAN_FEATURE_11W
		pProfile->MFPEnabled = connected_prof->MFPEnabled;
		pProfile->MFPRequired = connected_prof->MFPRequired;
		pProfile->MFPCapable = connected_prof->MFPCapable;
#endif
	}
	return status;
}

QDF_STATUS csr_roam_get_connect_profile(tpAniSirGlobal pMac, uint32_t sessionId,
					tCsrRoamConnectedProfile *pProfile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if ((csr_is_conn_state_connected(pMac, sessionId)) ||
	    (csr_is_conn_state_ibss(pMac, sessionId))) {
		if (pProfile) {
			status =
				csr_roam_copy_connect_profile(pMac, sessionId,
							      pProfile);
		}
	}
	return status;
}

void csr_roam_free_connect_profile(tCsrRoamConnectedProfile *profile)
{
	if (profile->pBssDesc)
		qdf_mem_free(profile->pBssDesc);
	if (profile->pAddIEAssoc)
		qdf_mem_free(profile->pAddIEAssoc);
	qdf_mem_set(profile, sizeof(tCsrRoamConnectedProfile), 0);
	profile->AuthType = eCSR_AUTH_TYPE_UNKNOWN;
}

static QDF_STATUS csr_roam_free_connected_info(tpAniSirGlobal pMac,
					       tCsrRoamConnectedInfo *
					       pConnectedInfo)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	if (pConnectedInfo->pbFrames) {
		qdf_mem_free(pConnectedInfo->pbFrames);
		pConnectedInfo->pbFrames = NULL;
	}
	pConnectedInfo->nBeaconLength = 0;
	pConnectedInfo->nAssocReqLength = 0;
	pConnectedInfo->nAssocRspLength = 0;
	pConnectedInfo->staId = 0;
	pConnectedInfo->nRICRspLength = 0;
#ifdef FEATURE_WLAN_ESE
	pConnectedInfo->nTspecIeLength = 0;
#endif
	return status;
}

void csr_release_command_roam(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	csr_reinit_roam_cmd(pMac, pCommand);
	csr_release_command(pMac, pCommand);
}

void csr_release_command_scan(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	qdf_mc_timer_stop(&pCommand->u.scanCmd.csr_scan_timer);
	qdf_mc_timer_destroy(&pCommand->u.scanCmd.csr_scan_timer);
	csr_reinit_scan_cmd(pMac, pCommand);
	csr_release_command(pMac, pCommand);
}

void csr_release_command_wm_status_change(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	csr_reinit_wm_status_change_cmd(pMac, pCommand);
	csr_release_command(pMac, pCommand);
}

void csr_reinit_set_key_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	qdf_mem_set(&pCommand->u.setKeyCmd, sizeof(tSetKeyCmd), 0);
}

void csr_release_command_set_key(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	csr_reinit_set_key_cmd(pMac, pCommand);
	csr_release_command(pMac, pCommand);
}

/**
 * csr_release_roc_req_cmd() - Release the command
 * @mac_ctx: Global MAC Context
 *
 * Release the remain on channel request command from the queue
 *
 * Return: None
 */
void csr_release_roc_req_cmd(tpAniSirGlobal mac_ctx)
{
	tListElem *entry;
	tSmeCmd *cmd = NULL;

	entry = csr_ll_peek_head(&mac_ctx->sme.smeCmdActiveList,
			LL_ACCESS_LOCK);
	if (entry) {
		cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
		if (eSmeCommandRemainOnChannel == cmd->command) {
			remainOnChanCallback callback =
				cmd->u.remainChlCmd.callback;
			/* process the msg */
			if (callback)
				callback(mac_ctx,
					cmd->u.remainChlCmd.callbackCtx, 0,
					cmd->u.remainChlCmd.scan_id);
			sms_log(mac_ctx, LOGE,
				FL("Remove RoC Request from Active Cmd List"));
			/* Put this cmd back on the available command list */
			if (csr_ll_remove_entry(&mac_ctx->sme.smeCmdActiveList,
						entry, LL_ACCESS_LOCK))
				sme_release_command(mac_ctx, cmd);
		}
	}
}

void csr_abort_command(tpAniSirGlobal pMac, tSmeCmd *pCommand, bool fStopping)
{

	if (eSmeCsrCommandMask & pCommand->command) {
		switch (pCommand->command) {
		case eSmeCommandScan:
			/* We need to inform the requester before dropping the scan command */
			sms_log(pMac, LOGW,
				"%s: Drop scan reason %d callback %p", __func__,
				pCommand->u.scanCmd.reason,
				pCommand->u.scanCmd.callback);
			if (NULL != pCommand->u.scanCmd.callback) {
				sms_log(pMac, LOGW, "%s callback scan requester",
					__func__);
				csr_scan_call_callback(pMac, pCommand,
						       eCSR_SCAN_ABORT);
			}
			csr_release_command_scan(pMac, pCommand);
			break;
		case eSmeCommandRoam:
			csr_release_command_roam(pMac, pCommand);
			break;

		case eSmeCommandWmStatusChange:
			csr_release_command_wm_status_change(pMac, pCommand);
			break;

		case eSmeCommandSetKey:
			csr_release_command_set_key(pMac, pCommand);
			break;

		case eSmeCommandNdpInitiatorRequest:
			csr_release_ndp_initiator_req(pMac, pCommand);
			break;

		case eSmeCommandNdpResponderRequest:
			csr_release_ndp_responder_req(pMac, pCommand);
			break;

		case eSmeCommandNdpDataEndInitiatorRequest:
			csr_release_ndp_data_end_req(pMac, pCommand);
			break;

		default:
			sms_log(pMac, LOGW, " CSR abort standard command %d",
				pCommand->command);
			csr_release_command(pMac, pCommand);
			break;
		}
	}
}

void csr_roam_substate_change(tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate,
			      uint32_t sessionId)
{
	sms_log(pMac, LOG1, FL("CSR RoamSubstate: [ %s <== %s ]"),
		mac_trace_getcsr_roam_sub_state(NewSubstate),
		mac_trace_getcsr_roam_sub_state(pMac->roam.curSubState[sessionId]));
	if (pMac->roam.curSubState[sessionId] == NewSubstate) {
		return;
	}
	pMac->roam.curSubState[sessionId] = NewSubstate;
}

eCsrRoamState csr_roam_state_change(tpAniSirGlobal pMac,
				    eCsrRoamState NewRoamState, uint8_t sessionId)
{
	eCsrRoamState PreviousState;

	sms_log(pMac, LOG1, FL("CSR RoamState[%hu]: [ %s <== %s ]"), sessionId,
		mac_trace_getcsr_roam_state(NewRoamState),
		mac_trace_getcsr_roam_state(pMac->roam.curState[sessionId]));
	PreviousState = pMac->roam.curState[sessionId];

	if (NewRoamState != pMac->roam.curState[sessionId]) {
		/* Whenever we transition OUT of the Roaming state, clear the Roaming substate... */
		if (CSR_IS_ROAM_JOINING(pMac, sessionId)) {
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 sessionId);
		}

		pMac->roam.curState[sessionId] = NewRoamState;
	}
	return PreviousState;
}

void csr_assign_rssi_for_category(tpAniSirGlobal pMac, int8_t bestApRssi,
				  uint8_t catOffset)
{
	int i;
	sms_log(pMac, LOG2, FL("best AP RSSI:%d, cat offset:%d"),
		bestApRssi, catOffset);
	if (catOffset) {
		pMac->roam.configParam.bCatRssiOffset = catOffset;
		for (i = 0; i < CSR_NUM_RSSI_CAT; i++) {
			pMac->roam.configParam.RSSICat[CSR_NUM_RSSI_CAT - i -
						       1] =
				(int)bestApRssi -
				pMac->roam.configParam.nSelect5GHzMargin -
				(int)(i * catOffset);
		}
	}
}

static void init_config_param(tpAniSirGlobal pMac)
{
	int i;
	pMac->roam.configParam.agingCount = CSR_AGING_COUNT;
	pMac->roam.configParam.channelBondingMode24GHz =
		WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
	pMac->roam.configParam.channelBondingMode5GHz =
		WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;

	pMac->roam.configParam.phyMode = eCSR_DOT11_MODE_AUTO;
	pMac->roam.configParam.eBand = eCSR_BAND_ALL;
	pMac->roam.configParam.uCfgDot11Mode = eCSR_CFG_DOT11_MODE_AUTO;
	pMac->roam.configParam.FragmentationThreshold =
		eCSR_DOT11_FRAG_THRESH_DEFAULT;
	pMac->roam.configParam.HeartbeatThresh24 = 40;
	pMac->roam.configParam.HeartbeatThresh50 = 40;
	pMac->roam.configParam.Is11dSupportEnabled = false;
	pMac->roam.configParam.Is11dSupportEnabledOriginal = false;
	pMac->roam.configParam.Is11eSupportEnabled = true;
	pMac->roam.configParam.Is11hSupportEnabled = true;
	pMac->roam.configParam.RTSThreshold = 2346;
	pMac->roam.configParam.shortSlotTime = true;
	pMac->roam.configParam.WMMSupportMode = eCsrRoamWmmAuto;
	pMac->roam.configParam.ProprietaryRatesEnabled = true;
	pMac->roam.configParam.TxRate = eCSR_TX_RATE_AUTO;
	pMac->roam.configParam.scanAgeTimeNCNPS =
		CSR_SCAN_AGING_TIME_NOT_CONNECT_NO_PS;
	pMac->roam.configParam.scanAgeTimeNCPS =
		CSR_SCAN_AGING_TIME_NOT_CONNECT_W_PS;
	pMac->roam.configParam.scanAgeTimeCNPS =
		CSR_SCAN_AGING_TIME_CONNECT_NO_PS;
	pMac->roam.configParam.scanAgeTimeCPS =
		CSR_SCAN_AGING_TIME_CONNECT_W_PS;
	for (i = 0; i < CSR_NUM_RSSI_CAT; i++) {
		pMac->roam.configParam.BssPreferValue[i] = i;
	}
	csr_assign_rssi_for_category(pMac, CSR_BEST_RSSI_VALUE,
			CSR_DEFAULT_RSSI_DB_GAP);
	pMac->roam.configParam.nRoamingTime = CSR_DEFAULT_ROAMING_TIME;
	pMac->roam.configParam.fSupplicantCountryCodeHasPriority = false;
	pMac->roam.configParam.nActiveMaxChnTime = CSR_ACTIVE_MAX_CHANNEL_TIME;
	pMac->roam.configParam.nActiveMinChnTime = CSR_ACTIVE_MIN_CHANNEL_TIME;
	pMac->roam.configParam.nPassiveMaxChnTime =
		CSR_PASSIVE_MAX_CHANNEL_TIME;
	pMac->roam.configParam.nPassiveMinChnTime =
		CSR_PASSIVE_MIN_CHANNEL_TIME;
#ifdef WLAN_AP_STA_CONCURRENCY
	pMac->roam.configParam.nActiveMaxChnTimeConc =
		CSR_ACTIVE_MAX_CHANNEL_TIME_CONC;
	pMac->roam.configParam.nActiveMinChnTimeConc =
		CSR_ACTIVE_MIN_CHANNEL_TIME_CONC;
	pMac->roam.configParam.nPassiveMaxChnTimeConc =
		CSR_PASSIVE_MAX_CHANNEL_TIME_CONC;
	pMac->roam.configParam.nPassiveMinChnTimeConc =
		CSR_PASSIVE_MIN_CHANNEL_TIME_CONC;
	pMac->roam.configParam.nRestTimeConc = CSR_REST_TIME_CONC;
	pMac->roam.configParam.min_rest_time_conc =  CSR_MIN_REST_TIME_CONC;
	pMac->roam.configParam.idle_time_conc = CSR_IDLE_TIME_CONC;
	pMac->roam.configParam.nNumStaChanCombinedConc =
		CSR_NUM_STA_CHAN_COMBINED_CONC;
	pMac->roam.configParam.nNumP2PChanCombinedConc =
		CSR_NUM_P2P_CHAN_COMBINED_CONC;
#endif
	pMac->roam.configParam.nTxPowerCap = CSR_MAX_TX_POWER;
	pMac->roam.configParam.allow_tpc_from_ap = true;
	pMac->roam.configParam.statsReqPeriodicity =
		CSR_MIN_GLOBAL_STAT_QUERY_PERIOD;
	pMac->roam.configParam.statsReqPeriodicityInPS =
		CSR_MIN_GLOBAL_STAT_QUERY_PERIOD_IN_BMPS;
	pMac->roam.configParam.csr11rConfig.IsFTResourceReqSupported = 0;
	pMac->roam.configParam.neighborRoamConfig.nMaxNeighborRetries = 3;
	pMac->roam.configParam.neighborRoamConfig.nNeighborLookupRssiThreshold =
		120;
	pMac->roam.configParam.neighborRoamConfig.nOpportunisticThresholdDiff =
		30;
	pMac->roam.configParam.neighborRoamConfig.nRoamRescanRssiDiff = 5;
	pMac->roam.configParam.neighborRoamConfig.nNeighborScanMinChanTime = 20;
	pMac->roam.configParam.neighborRoamConfig.nNeighborScanMaxChanTime = 40;
	pMac->roam.configParam.neighborRoamConfig.nNeighborScanTimerPeriod =
		200;
	pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.
	numChannels = 3;
	pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.
	channelList[0] = 1;
	pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.
	channelList[1] = 6;
	pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.
	channelList[2] = 11;
	pMac->roam.configParam.neighborRoamConfig.nNeighborResultsRefreshPeriod = 20000;        /* 20 seconds */
	pMac->roam.configParam.neighborRoamConfig.nEmptyScanRefreshPeriod = 0;
	pMac->roam.configParam.neighborRoamConfig.nRoamBmissFirstBcnt = 10;
	pMac->roam.configParam.neighborRoamConfig.nRoamBmissFinalBcnt = 10;
	pMac->roam.configParam.neighborRoamConfig.nRoamBeaconRssiWeight = 14;
	pMac->roam.configParam.nVhtChannelWidth =
		WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ + 1;

	pMac->roam.configParam.addTSWhenACMIsOff = 0;
	pMac->roam.configParam.fScanTwice = false;

	/* Remove this code once SLM_Sessionization is supported */
	/* BMPS_WORKAROUND_NOT_NEEDED */
	pMac->roam.configParam.doBMPSWorkaround = 0;

	pMac->roam.configParam.nInitialDwellTime = 0;
	pMac->roam.configParam.initial_scan_no_dfs_chnl = 0;
}

eCsrBand csr_get_current_band(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.bandCapability;
}

/*
   This function flushes the roam scan cache
 */
QDF_STATUS csr_flush_cfg_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
						   uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	/* Free up the memory first (if required) */
	if (NULL != pNeighborRoamInfo->cfgParams.channelInfo.ChannelList) {
		qdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
		pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels = 0;
	}
	return status;
}

/*
   This function flushes the roam scan cache and creates fresh cache
   based on the input channel list
 */
QDF_STATUS csr_create_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
						uint8_t sessionId,
						const uint8_t *pChannelList,
						const uint8_t numChannels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels = numChannels;

	pNeighborRoamInfo->cfgParams.channelInfo.ChannelList =
		qdf_mem_malloc(pNeighborRoamInfo->cfgParams.channelInfo.
			       numOfChannels);

	if (NULL == pNeighborRoamInfo->cfgParams.channelInfo.ChannelList) {
		sms_log(pMac, LOGE,
			FL("Memory Allocation for CFG Channel List failed"));
		pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels = 0;
		return QDF_STATUS_E_NOMEM;
	}

	/* Update the roam global structure */
	qdf_mem_copy(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList,
		     pChannelList,
		     pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels);
	return status;
}


#ifdef FEATURE_WLAN_ESE
/**
 * csr_create_roam_scan_channel_list() - create roam scan channel list
 * @pMac: Global mac pointer
 * @sessionId: session id
 * @pChannelList: pointer to channel list
 * @numChannels: number of channels
 * @eBand: band enumeration
 *
 * This function modifies the roam scan channel list as per AP neighbor
 * report; AP neighbor report may be empty or may include only other AP
 * channels; in any case, we merge the channel list with the learned occupied
 * channels list.
 * if the band is 2.4G, then make sure channel list contains only 2.4G
 * valid channels if the band is 5G, then make sure channel list contains
 * only 5G valid channels
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS csr_create_roam_scan_channel_list(tpAniSirGlobal pMac,
					     uint8_t sessionId,
					     uint8_t *pChannelList,
					     uint8_t numChannels,
					     const eCsrBand eBand)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo
		= &pMac->roam.neighborRoamInfo[sessionId];
	uint8_t outNumChannels = 0;
	uint8_t inNumChannels = numChannels;
	uint8_t *inPtr = pChannelList;
	uint8_t i = 0;
	uint8_t ChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t tmpChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t mergedOutputNumOfChannels = 0;
	tpCsrChannelInfo currChannelListInfo
		= &pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo;
	/*
	 * Create a Union of occupied channel list learnt by the DUT along
	 * with the Neighbor report Channels. This increases the chances of
	 * the DUT to get a candidate AP while roaming even if the Neighbor
	 * Report is not able to provide sufficient information.
	 */
	if (pMac->scan.occupiedChannels[sessionId].numChannels) {
		csr_neighbor_roam_merge_channel_lists(pMac,
						      &pMac->scan.
						      occupiedChannels[sessionId].
						      channelList[0],
						      pMac->scan.
						      occupiedChannels[sessionId].
						      numChannels, inPtr,
						      inNumChannels,
						      &mergedOutputNumOfChannels);
		inNumChannels = mergedOutputNumOfChannels;
	}
	if (eCSR_BAND_24 == eBand) {
		for (i = 0; i < inNumChannels; i++) {
			if (CDS_IS_CHANNEL_24GHZ(inPtr[i])
			    && csr_roam_is_channel_valid(pMac, inPtr[i])) {
				ChannelList[outNumChannels++] = inPtr[i];
			}
		}
	} else if (eCSR_BAND_5G == eBand) {
		for (i = 0; i < inNumChannels; i++) {
			/* Add 5G Non-DFS channel */
			if (CDS_IS_CHANNEL_5GHZ(inPtr[i]) &&
			    csr_roam_is_channel_valid(pMac, inPtr[i]) &&
			    !CDS_IS_DFS_CH(inPtr[i])) {
				ChannelList[outNumChannels++] = inPtr[i];
			}
		}
	} else if (eCSR_BAND_ALL == eBand) {
		for (i = 0; i < inNumChannels; i++) {
			if (csr_roam_is_channel_valid(pMac, inPtr[i]) &&
			    !CDS_IS_DFS_CH(inPtr[i])) {
				ChannelList[outNumChannels++] = inPtr[i];
			}
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
			  "Invalid band, No operation carried out (Band %d)",
			  eBand);
		return QDF_STATUS_E_INVAL;
	}
	/*
	 * if roaming within band is enabled, then select only the
	 * in band channels .
	 * This is required only if the band capability is set to ALL,
	 * E.g., if band capability is only 2.4G then all the channels in the
	 * list are already filtered for 2.4G channels, hence ignore this check
	 */
	if ((eCSR_BAND_ALL == eBand) && CSR_IS_ROAM_INTRA_BAND_ENABLED(pMac)) {
		csr_neighbor_roam_channels_filter_by_current_band(pMac,
								  sessionId,
								  ChannelList,
								  outNumChannels,
								  tmpChannelList,
								  &outNumChannels);
		qdf_mem_copy(ChannelList, tmpChannelList, outNumChannels);
	}
	/* Prepare final roam scan channel list */
	if (outNumChannels) {
		/* Clear the channel list first */
		if (NULL != currChannelListInfo->ChannelList) {
			qdf_mem_free(currChannelListInfo->ChannelList);
			currChannelListInfo->ChannelList = NULL;
			currChannelListInfo->numOfChannels = 0;
		}
		currChannelListInfo->ChannelList
			= qdf_mem_malloc(outNumChannels * sizeof(uint8_t));
		if (NULL == currChannelListInfo->ChannelList) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
				  "Failed to allocate memory for roam scan channel list");
			currChannelListInfo->numOfChannels = 0;
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(currChannelListInfo->ChannelList,
			     ChannelList, outNumChannels);
	}
	return status;
}

/**
 * csr_roam_is_ese_assoc() - is this ese association
 * @mac_ctx: Global MAC context
 * @session_id: session identifier
 *
 * Returns whether the current association is a ESE assoc or not.
 *
 * Return: true if ese association; false otherwise
 */
bool csr_roam_is_ese_assoc(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	return mac_ctx->roam.neighborRoamInfo[session_id].isESEAssoc;
}


/**
 * csr_roam_is_ese_ini_feature_enabled() - is ese feature enabled
 * @mac_ctx: Global MAC context
 *
 * Return: true if ese feature is enabled; false otherwise
 */
bool csr_roam_is_ese_ini_feature_enabled(tpAniSirGlobal pMac)
{
	return pMac->roam.configParam.isEseIniFeatureEnabled;
}

/**
 * csr_tsm_stats_rsp_processor() - tsm stats response processor
 * @pMac: Global MAC context
 * @pMsg: Message pointer
 *
 * Return: None
 */
void csr_tsm_stats_rsp_processor(tpAniSirGlobal pMac, void *pMsg)
{
	tAniGetTsmStatsRsp *pTsmStatsRsp = (tAniGetTsmStatsRsp *) pMsg;

	if (NULL != pTsmStatsRsp) {
		/*
		 * Get roam Rssi request is backed up and passed back
		 * to the response, Extract the request message
		 * to fetch callback.
		 */
		tpAniGetTsmStatsReq reqBkp
			= (tAniGetTsmStatsReq *) pTsmStatsRsp->tsmStatsReq;

		if (NULL != reqBkp) {
			if (NULL != reqBkp->tsmStatsCallback) {
				((tCsrTsmStatsCallback)
				 (reqBkp->tsmStatsCallback))(pTsmStatsRsp->
							     tsmMetrics,
							     pTsmStatsRsp->
							     staId,
							     reqBkp->
							     pDevContext);
				reqBkp->tsmStatsCallback = NULL;
			}
			qdf_mem_free(reqBkp);
			pTsmStatsRsp->tsmStatsReq = NULL;
		} else {
			sms_log(pMac, LOGE, FL("reqBkp is NULL"));
			if (NULL != reqBkp) {
				qdf_mem_free(reqBkp);
				pTsmStatsRsp->tsmStatsReq = NULL;
			}
		}
	} else {
		sms_log(pMac, LOGE, FL("pTsmStatsRsp is NULL"));
	}
	return;
}

/**
 * csr_send_ese_adjacent_ap_rep_ind() - ese send adjacent ap report
 * @pMac: Global MAC context
 * @pSession: Session pointer
 *
 * Return: None
 */
void csr_send_ese_adjacent_ap_rep_ind(tpAniSirGlobal pMac,
					tCsrRoamSession *pSession)
{
	uint32_t roamTS2 = 0;
	tCsrRoamInfo roamInfo;
	tpPESession pSessionEntry = NULL;
	uint8_t sessionId = CSR_SESSION_ID_INVALID;

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return;
	}

	roamTS2 = qdf_mc_timer_get_system_time();
	roamInfo.tsmRoamDelay = roamTS2 - pSession->roamTS1;
	sms_log(pMac, LOG1, "Bssid(" MAC_ADDRESS_STR ") Roaming Delay(%u ms)",
		MAC_ADDR_ARRAY(pSession->connectedProfile.bssid.bytes),
		roamInfo.tsmRoamDelay);

	pSessionEntry = pe_find_session_by_bssid(pMac,
					 pSession->connectedProfile.bssid.bytes,
					 &sessionId);
	if (NULL == pSessionEntry) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return;
	}

	pSessionEntry->eseContext.tsm.tsmMetrics.RoamingDly
		= roamInfo.tsmRoamDelay;

	csr_roam_call_callback(pMac, pSession->sessionId, &roamInfo,
			       0, eCSR_ROAM_ESE_ADJ_AP_REPORT_IND, 0);
}

/**
 * csr_get_tsm_stats() - get tsm stats
 * @pMac: Global MAC context
 * @callback: TSM stats callback
 * @staId: Station id
 * @bssId: bssid
 * @pContext: pointer to context
 * @p_cds_context: cds context
 * @tid: traffic id
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS csr_get_tsm_stats(tpAniSirGlobal pMac,
			     tCsrTsmStatsCallback callback,
			     uint8_t staId,
			     struct qdf_mac_addr bssId,
			     void *pContext, void *p_cds_context, uint8_t tid)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tAniGetTsmStatsReq *pMsg = NULL;
	pMsg = qdf_mem_malloc(sizeof(tAniGetTsmStatsReq));
	if (!pMsg) {
		sms_log(pMac, LOGE,
			"csr_get_tsm_stats: failed to allocate mem for req");
		return QDF_STATUS_E_NOMEM;
	}
	/* need to initiate a stats request to PE */
	pMsg->msgType = eWNI_SME_GET_TSM_STATS_REQ;
	pMsg->msgLen = (uint16_t) sizeof(tAniGetTsmStatsReq);
	pMsg->staId = staId;
	pMsg->tid = tid;
	qdf_copy_macaddr(&pMsg->bssId, &bssId);
	pMsg->tsmStatsCallback = callback;
	pMsg->pDevContext = pContext;
	pMsg->p_cds_context = p_cds_context;
	status = cds_send_mb_message_to_mac(pMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOG1,
			" csr_get_tsm_stats: failed to send down the rssi req");
		/* pMsg is freed by cds_send_mb_message_to_mac */
		status = QDF_STATUS_E_FAILURE;
	}
	return status;
}

/**
 * csr_fetch_ch_lst_from_received_list() - fetch channel list from received list
 * and update req msg
 * paramters
 * @mac_ctx:            global mac ctx
 * @roam_info:          roam info struct
 * @curr_ch_lst_info:   current channel list info
 * @req_buf:            out param, roam offload scan request packet
 *
 * Return: void
 */
static void
csr_fetch_ch_lst_from_received_list(tpAniSirGlobal mac_ctx,
				    tpCsrNeighborRoamControlInfo roam_info,
				    tpCsrChannelInfo curr_ch_lst_info,
				    tSirRoamOffloadScanReq *req_buf)
{
	uint8_t i = 0;
	uint8_t num_channels = 0;
	uint8_t *ch_lst = NULL;
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool      is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return;
	}
	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
			&unsafe_chan_cnt,
			sizeof(unsafe_chan));

	if (curr_ch_lst_info->numOfChannels == 0)
		return;

	ch_lst = curr_ch_lst_info->ChannelList;
	for (i = 0; i < curr_ch_lst_info->numOfChannels; i++) {
		if ((!mac_ctx->roam.configParam.allowDFSChannelRoam ||
		    (mac_ctx->roam.configParam.sta_roam_policy.dfs_mode ==
			 CSR_STA_ROAM_POLICY_DFS_DISABLED)) &&
		     (CDS_IS_DFS_CH(*ch_lst))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("ignoring dfs channel %d"), *ch_lst);
			ch_lst++;
			continue;
		}

		if (mac_ctx->roam.configParam.sta_roam_policy.
				skip_unsafe_channels &&
				unsafe_chan_cnt) {
			is_unsafe_chan = false;
			for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
				if (unsafe_chan[cnt] == *ch_lst) {
					is_unsafe_chan = true;
					break;
				}
			}
			if (is_unsafe_chan) {
				QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_INFO,
					FL("ignoring unsafe channel %d"),
					*ch_lst);
				ch_lst++;
				continue;
			}
		}
		req_buf->ConnectedNetwork.ChannelCache[num_channels++] =
			*ch_lst;
		ch_lst++;
	}
	req_buf->ConnectedNetwork.ChannelCount = num_channels;
	req_buf->ChannelCacheType = CHANNEL_LIST_DYNAMIC_UPDATE;
}

/**
 * csr_set_cckm_ie() - set CCKM IE
 * @pMac: Global MAC context
 * @sessionId: session identifier
 * @pCckmIe: Pointer to input CCKM IE data
 * @ccKmIeLen: Length of @pCckmIe
 *
 * This function stores the CCKM IE passed by the supplicant
 * in a place holder data structure and this IE will be packed inside
 * reassociation request
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS csr_set_cckm_ie(tpAniSirGlobal pMac, const uint8_t sessionId,
			   const uint8_t *pCckmIe, const uint8_t ccKmIeLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(pSession->suppCckmIeInfo.cckmIe, pCckmIe, ccKmIeLen);
	pSession->suppCckmIeInfo.cckmIeLen = ccKmIeLen;
	return status;
}

/**
 * csr_roam_read_tsf() - read TSF
 * @pMac: Global MAC context
 * @sessionId: session identifier
 * @pTimestamp: output TSF timestamp
 *
 * This function reads the TSF; and also add the time elapsed since
 * last beacon or probe response reception from the hand off AP to arrive at
 * the latest TSF value.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS csr_roam_read_tsf(tpAniSirGlobal pMac, uint8_t *pTimestamp,
			     uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamBSSInfo handoffNode = {{0} };
	uint32_t timer_diff = 0;
	uint32_t timeStamp[2];
	tpSirBssDescription pBssDescription = NULL;
	csr_neighbor_roam_get_handoff_ap_info(pMac, &handoffNode, sessionId);
	pBssDescription = handoffNode.pBssDescription;
	/* Get the time diff in milli seconds */
	timer_diff = qdf_mc_timer_get_system_time() -
				pBssDescription->scanSysTimeMsec;
	/* Convert msec to micro sec timer */
	timer_diff = (uint32_t) (timer_diff * SYSTEM_TIME_MSEC_TO_USEC);
	timeStamp[0] = pBssDescription->timeStamp[0];
	timeStamp[1] = pBssDescription->timeStamp[1];
	update_cckmtsf(&(timeStamp[0]), &(timeStamp[1]), &timer_diff);
	qdf_mem_copy(pTimestamp, (void *)&timeStamp[0], sizeof(uint32_t) * 2);
	return status;
}

#endif /* FEATURE_WLAN_ESE */

/**
 * csr_roam_is_roam_offload_scan_enabled() - is roam offload enabled
 * @mac_ctx: Global MAC context
 *
 * Returns whether firmware based background scan is currently enabled or not.
 *
 * Return: true if roam offload scan enabled; false otherwise
 */
bool csr_roam_is_roam_offload_scan_enabled(tpAniSirGlobal mac_ctx)
{
	return mac_ctx->roam.configParam.isRoamOffloadScanEnabled;
}

QDF_STATUS csr_set_band(tHalHandle hHal, uint8_t sessionId, eCsrBand eBand)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	if (CSR_IS_PHY_MODE_A_ONLY(pMac) && (eBand == eCSR_BAND_24)) {
		/* DOT11 mode configured to 11a only and received
		   request to change the band to 2.4 GHz */
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "failed to set band cfg80211 = %u, band = %u",
			  pMac->roam.configParam.uCfgDot11Mode, eBand);
		return QDF_STATUS_E_INVAL;
	}
	if ((CSR_IS_PHY_MODE_B_ONLY(pMac) ||
	     CSR_IS_PHY_MODE_G_ONLY(pMac)) && (eBand == eCSR_BAND_5G)) {
		/* DOT11 mode configured to 11b/11g only and received
		   request to change the band to 5 GHz */
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "failed to set band dot11mode = %u, band = %u",
			  pMac->roam.configParam.uCfgDot11Mode, eBand);
		return QDF_STATUS_E_INVAL;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "Band changed to %u (0 - ALL, 1 - 2.4 GHZ, 2 - 5GHZ)", eBand);
	pMac->roam.configParam.eBand = eBand;
	pMac->roam.configParam.bandCapability = eBand;

	status = csr_get_channel_and_power_list(pMac);
	if (QDF_STATUS_SUCCESS == status)
		csr_apply_channel_and_power_list(pMac);
	return status;
}

/* The funcns csr_convert_cb_ini_value_to_phy_cb_state and csr_convert_phy_cb_state_to_ini_value have been
 * introduced to convert the ini value to the ENUM used in csr and MAC for CB state
 * Ideally we should have kept the ini value and enum value same and representing the same
 * cb values as in 11n standard i.e.
 * Set to 1 (SCA) if the secondary channel is above the primary channel
 * Set to 3 (SCB) if the secondary channel is below the primary channel
 * Set to 0 (SCN) if no secondary channel is present
 * However, since our driver is already distributed we will keep the ini definition as it is which is:
 * 0 - secondary none
 * 1 - secondary LOW
 * 2 - secondary HIGH
 * and convert to enum value used within the driver in csr_change_default_config_param using this funcn
 * The enum values are as follows:
 * PHY_SINGLE_CHANNEL_CENTERED          = 0
 * PHY_DOUBLE_CHANNEL_LOW_PRIMARY   = 1
 * PHY_DOUBLE_CHANNEL_HIGH_PRIMARY  = 3
 */
ePhyChanBondState csr_convert_cb_ini_value_to_phy_cb_state(uint32_t cbIniValue)
{

	ePhyChanBondState phyCbState;
	switch (cbIniValue) {
	/* secondary none */
	case eCSR_INI_SINGLE_CHANNEL_CENTERED:
		phyCbState = PHY_SINGLE_CHANNEL_CENTERED;
		break;
	/* secondary LOW */
	case eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY:
		phyCbState = PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
		break;
	/* secondary HIGH */
	case eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY:
		phyCbState = PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
		phyCbState =
			PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH;
		break;
	case eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
		phyCbState = PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH;
		break;
	default:
		/* If an invalid value is passed, disable CHANNEL BONDING */
		phyCbState = PHY_SINGLE_CHANNEL_CENTERED;
		break;
	}
	return phyCbState;
}

uint32_t csr_convert_phy_cb_state_to_ini_value(ePhyChanBondState phyCbState)
{

	uint32_t cbIniValue;
	switch (phyCbState) {
	/* secondary none */
	case PHY_SINGLE_CHANNEL_CENTERED:
		cbIniValue = eCSR_INI_SINGLE_CHANNEL_CENTERED;
		break;
	/* secondary LOW */
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		cbIniValue = eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY;
		break;
	/* secondary HIGH */
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		cbIniValue = eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
		cbIniValue =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
		cbIniValue =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
		cbIniValue =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
		cbIniValue = eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
		cbIniValue = eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
		cbIniValue = eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
		cbIniValue = eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH;
		break;
	default:
		/* return some invalid value */
		cbIniValue = eCSR_INI_CHANNEL_BONDING_STATE_MAX;
		break;
	}
	return cbIniValue;
}

QDF_STATUS csr_change_default_config_param(tpAniSirGlobal pMac,
					   tCsrConfigParam *pParam)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (pParam) {
		pMac->roam.configParam.WMMSupportMode = pParam->WMMSupportMode;
		cfg_set_int(pMac, WNI_CFG_WME_ENABLED,
			(pParam->WMMSupportMode == eCsrRoamWmmNoQos) ? 0 : 1);
		pMac->roam.configParam.Is11eSupportEnabled =
			pParam->Is11eSupportEnabled;
		pMac->roam.configParam.FragmentationThreshold =
			pParam->FragmentationThreshold;
		pMac->roam.configParam.Is11dSupportEnabled =
			pParam->Is11dSupportEnabled;
		pMac->roam.configParam.Is11dSupportEnabledOriginal =
			pParam->Is11dSupportEnabled;
		pMac->roam.configParam.Is11hSupportEnabled =
			pParam->Is11hSupportEnabled;

		pMac->roam.configParam.fenableMCCMode = pParam->fEnableMCCMode;
		pMac->roam.configParam.mcc_rts_cts_prot_enable =
						pParam->mcc_rts_cts_prot_enable;
		pMac->roam.configParam.mcc_bcast_prob_resp_enable =
					pParam->mcc_bcast_prob_resp_enable;
		pMac->roam.configParam.fAllowMCCGODiffBI =
			pParam->fAllowMCCGODiffBI;

		/* channelBondingMode5GHz plays a dual role right now
		 * INFRA STA will use this non zero value as CB enabled and SOFTAP will use this non-zero value to determine the secondary channel offset
		 * This is how channelBondingMode5GHz works now and this is kept intact to avoid any cfg.ini change
		 */
		if (pParam->channelBondingMode24GHz > MAX_CB_VALUE_IN_INI) {
			sms_log(pMac, LOGW,
				"Invalid CB value from ini in 2.4GHz band %d, CB DISABLED",
				pParam->channelBondingMode24GHz);
		}
		pMac->roam.configParam.channelBondingMode24GHz =
			csr_convert_cb_ini_value_to_phy_cb_state(pParam->
								 channelBondingMode24GHz);
		if (pParam->channelBondingMode5GHz > MAX_CB_VALUE_IN_INI) {
			sms_log(pMac, LOGW,
				"Invalid CB value from ini in 5GHz band %d, CB DISABLED",
				pParam->channelBondingMode5GHz);
		}
		pMac->roam.configParam.channelBondingMode5GHz =
			csr_convert_cb_ini_value_to_phy_cb_state(pParam->
								 channelBondingMode5GHz);
		pMac->roam.configParam.RTSThreshold = pParam->RTSThreshold;
		pMac->roam.configParam.phyMode = pParam->phyMode;
		pMac->roam.configParam.shortSlotTime = pParam->shortSlotTime;
		pMac->roam.configParam.HeartbeatThresh24 =
			pParam->HeartbeatThresh24;
		pMac->roam.configParam.HeartbeatThresh50 =
			pParam->HeartbeatThresh50;
		pMac->roam.configParam.ProprietaryRatesEnabled =
			pParam->ProprietaryRatesEnabled;
		pMac->roam.configParam.TxRate = pParam->TxRate;
		pMac->roam.configParam.AdHocChannel24 = pParam->AdHocChannel24;
		pMac->roam.configParam.AdHocChannel5G = pParam->AdHocChannel5G;
		pMac->roam.configParam.bandCapability = pParam->bandCapability;
		pMac->roam.configParam.cbChoice = pParam->cbChoice;
		pMac->roam.configParam.neighborRoamConfig.
			delay_before_vdev_stop =
			pParam->neighborRoamConfig.delay_before_vdev_stop;

		/* if HDD passed down non zero values then only update, */
		/* otherwise keep using the defaults */
		if (pParam->initial_scan_no_dfs_chnl) {
			pMac->roam.configParam.initial_scan_no_dfs_chnl =
				pParam->initial_scan_no_dfs_chnl;
		}
		if (pParam->nInitialDwellTime) {
			pMac->roam.configParam.nInitialDwellTime =
				pParam->nInitialDwellTime;
		}
		if (pParam->nActiveMaxChnTime) {
			pMac->roam.configParam.nActiveMaxChnTime =
				pParam->nActiveMaxChnTime;
			cfg_set_int(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME,
				    pParam->nActiveMaxChnTime);
		}
		if (pParam->nActiveMinChnTime) {
			pMac->roam.configParam.nActiveMinChnTime =
				pParam->nActiveMinChnTime;
			cfg_set_int(pMac, WNI_CFG_ACTIVE_MINIMUM_CHANNEL_TIME,
				    pParam->nActiveMinChnTime);
		}
		if (pParam->nPassiveMaxChnTime) {
			pMac->roam.configParam.nPassiveMaxChnTime =
				pParam->nPassiveMaxChnTime;
			cfg_set_int(pMac, WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME,
				    pParam->nPassiveMaxChnTime);
		}
		if (pParam->nPassiveMinChnTime) {
			pMac->roam.configParam.nPassiveMinChnTime =
				pParam->nPassiveMinChnTime;
			cfg_set_int(pMac, WNI_CFG_PASSIVE_MINIMUM_CHANNEL_TIME,
				    pParam->nPassiveMinChnTime);
		}
#ifdef WLAN_AP_STA_CONCURRENCY
		if (pParam->nActiveMaxChnTimeConc) {
			pMac->roam.configParam.nActiveMaxChnTimeConc =
				pParam->nActiveMaxChnTimeConc;
		}
		if (pParam->nActiveMinChnTimeConc) {
			pMac->roam.configParam.nActiveMinChnTimeConc =
				pParam->nActiveMinChnTimeConc;
		}
		if (pParam->nPassiveMaxChnTimeConc) {
			pMac->roam.configParam.nPassiveMaxChnTimeConc =
				pParam->nPassiveMaxChnTimeConc;
		}
		if (pParam->nPassiveMinChnTimeConc) {
			pMac->roam.configParam.nPassiveMinChnTimeConc =
				pParam->nPassiveMinChnTimeConc;
		}
		pMac->roam.configParam.nRestTimeConc = pParam->nRestTimeConc;
		pMac->roam.configParam.min_rest_time_conc =
			pParam->min_rest_time_conc;
		pMac->roam.configParam.idle_time_conc = pParam->idle_time_conc;

		if (pParam->nNumStaChanCombinedConc) {
			pMac->roam.configParam.nNumStaChanCombinedConc =
				pParam->nNumStaChanCombinedConc;
		}
		if (pParam->nNumP2PChanCombinedConc) {
			pMac->roam.configParam.nNumP2PChanCombinedConc =
				pParam->nNumP2PChanCombinedConc;
		}
#endif
		pMac->roam.configParam.eBand = pParam->eBand;
		pMac->roam.configParam.uCfgDot11Mode =
			csr_get_cfg_dot11_mode_from_csr_phy_mode(NULL,
								 pMac->roam.configParam.
								 phyMode,
								 pMac->roam.configParam.
								 ProprietaryRatesEnabled);
		/* if HDD passed down non zero values for age params, then only update, */
		/* otherwise keep using the defaults */
		if (pParam->nScanResultAgeCount) {
			pMac->roam.configParam.agingCount =
				pParam->nScanResultAgeCount;
		}
		if (pParam->scanAgeTimeNCNPS) {
			pMac->roam.configParam.scanAgeTimeNCNPS =
				pParam->scanAgeTimeNCNPS;
		}
		if (pParam->scanAgeTimeNCPS) {
			pMac->roam.configParam.scanAgeTimeNCPS =
				pParam->scanAgeTimeNCPS;
		}
		if (pParam->scanAgeTimeCNPS) {
			pMac->roam.configParam.scanAgeTimeCNPS =
				pParam->scanAgeTimeCNPS;
		}
		if (pParam->scanAgeTimeCPS) {
			pMac->roam.configParam.scanAgeTimeCPS =
				pParam->scanAgeTimeCPS;
		}
		if (pParam->obss_width_interval) {
			pMac->roam.configParam.obss_width_interval =
				pParam->obss_width_interval;
			cfg_set_int(pMac,
				WNI_CFG_OBSS_HT40_SCAN_WIDTH_TRIGGER_INTERVAL,
				pParam->obss_width_interval);
		}
		if (pParam->obss_active_dwelltime) {
			pMac->roam.configParam.obss_active_dwelltime =
				pParam->obss_active_dwelltime;
			cfg_set_int(pMac,
				WNI_CFG_OBSS_HT40_SCAN_ACTIVE_DWELL_TIME,
				pParam->obss_active_dwelltime);
		}
		if (pParam->obss_passive_dwelltime) {
			pMac->roam.configParam.obss_passive_dwelltime =
				pParam->obss_passive_dwelltime;
			cfg_set_int(pMac,
				WNI_CFG_OBSS_HT40_SCAN_PASSIVE_DWELL_TIME,
				pParam->obss_passive_dwelltime);
		}

		pMac->first_scan_bucket_threshold =
			pParam->first_scan_bucket_threshold;
		csr_assign_rssi_for_category(pMac,
			pMac->first_scan_bucket_threshold,
			pParam->bCatRssiOffset);
		pMac->roam.configParam.nRoamingTime = pParam->nRoamingTime;
		pMac->roam.configParam.fSupplicantCountryCodeHasPriority =
			pParam->fSupplicantCountryCodeHasPriority;
		pMac->roam.configParam.vccRssiThreshold =
			pParam->vccRssiThreshold;
		pMac->roam.configParam.vccUlMacLossThreshold =
			pParam->vccUlMacLossThreshold;
		pMac->roam.configParam.statsReqPeriodicity =
			pParam->statsReqPeriodicity;
		pMac->roam.configParam.statsReqPeriodicityInPS =
			pParam->statsReqPeriodicityInPS;
		/* Assign this before calling csr_init11d_info */
		pMac->roam.configParam.nTxPowerCap = pParam->nTxPowerCap;
		pMac->roam.configParam.allow_tpc_from_ap =
				pParam->allow_tpc_from_ap;
		if (csr_is11d_supported(pMac)) {
			status = csr_init11d_info(pMac, &pParam->Csr11dinfo);
		} else {
			pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
		}

		/* Initialize the power + channel information if 11h is enabled.
		   If 11d is enabled this information has already been initialized */
		if (csr_is11h_supported(pMac) && !csr_is11d_supported(pMac)) {
			csr_init_channel_power_list(pMac, &pParam->Csr11dinfo);
		}

		qdf_mem_copy(&pMac->roam.configParam.csr11rConfig,
			     &pParam->csr11rConfig,
			     sizeof(tCsr11rConfigParams));
		sms_log(pMac, LOG1, "IsFTResourceReqSupp = %d",
			pMac->roam.configParam.csr11rConfig.
			IsFTResourceReqSupported);
		pMac->roam.configParam.isFastTransitionEnabled =
			pParam->isFastTransitionEnabled;
		pMac->roam.configParam.RoamRssiDiff = pParam->RoamRssiDiff;
		pMac->roam.configParam.nRoamPrefer5GHz =
			pParam->nRoamPrefer5GHz;
		pMac->roam.configParam.nRoamIntraBand = pParam->nRoamIntraBand;
		pMac->roam.configParam.isWESModeEnabled =
			pParam->isWESModeEnabled;
		pMac->roam.configParam.nProbes = pParam->nProbes;
		pMac->roam.configParam.nRoamScanHomeAwayTime =
			pParam->nRoamScanHomeAwayTime;
		pMac->roam.configParam.isRoamOffloadScanEnabled =
			pParam->isRoamOffloadScanEnabled;
		pMac->roam.configParam.bFastRoamInConIniFeatureEnabled =
			pParam->bFastRoamInConIniFeatureEnabled;
		pMac->roam.configParam.isFastRoamIniFeatureEnabled =
			pParam->isFastRoamIniFeatureEnabled;
		pMac->roam.configParam.MAWCEnabled = pParam->MAWCEnabled;

#ifdef FEATURE_WLAN_ESE
		pMac->roam.configParam.isEseIniFeatureEnabled =
			pParam->isEseIniFeatureEnabled;
#endif
		qdf_mem_copy(&pMac->roam.configParam.neighborRoamConfig,
			     &pParam->neighborRoamConfig,
			     sizeof(tCsrNeighborRoamConfigParams));
		sms_log(pMac, LOG1, "nNeighborScanTimerPerioid = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborScanTimerPeriod);
		sms_log(pMac, LOG1, "nNeighborLookupRssiThreshold = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborLookupRssiThreshold);
		sms_log(pMac, LOG1, "nOpportunisticThresholdDiff = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nOpportunisticThresholdDiff);
		sms_log(pMac, LOG1, "nRoamRescanRssiDiff = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nRoamRescanRssiDiff);
		sms_log(pMac, LOG1, "nNeighborScanMinChanTime = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborScanMinChanTime);
		sms_log(pMac, LOG1, "nNeighborScanMaxChanTime = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborScanMaxChanTime);
		sms_log(pMac, LOG1, "nMaxNeighborRetries = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nMaxNeighborRetries);
		sms_log(pMac, LOG1, "nNeighborResultsRefreshPeriod = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborResultsRefreshPeriod);
		sms_log(pMac, LOG1, "nEmptyScanRefreshPeriod = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nEmptyScanRefreshPeriod);
		{
			int i;
			sms_log(pMac, LOG1,
				FL("Num of Channels in CFG Channel List: %d"),
				pMac->roam.configParam.neighborRoamConfig.
				neighborScanChanList.numChannels);
			for (i = 0;
			     i <
			     pMac->roam.configParam.neighborRoamConfig.
			     neighborScanChanList.numChannels; i++) {
				sms_log(pMac, LOG1, "%d ",
					pMac->roam.configParam.
					neighborRoamConfig.neighborScanChanList.
					channelList[i]);
			}
		}
		sms_log(pMac, LOG1, "nRoamBmissFirstBcnt = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBmissFirstBcnt);
		sms_log(pMac, LOG1, "nRoamBmissFinalBcnt = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBmissFinalBcnt);
		sms_log(pMac, LOG1, "nRoamBeaconRssiWeight = %d",
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBeaconRssiWeight);
		pMac->roam.configParam.addTSWhenACMIsOff =
			pParam->addTSWhenACMIsOff;
		pMac->scan.fValidateList = pParam->fValidateList;
		pMac->scan.fEnableBypass11d = pParam->fEnableBypass11d;
		pMac->scan.fEnableDFSChnlScan = pParam->fEnableDFSChnlScan;
		pMac->scan.scanResultCfgAgingTime = pParam->scanCfgAgingTime;
		pMac->roam.configParam.fScanTwice = pParam->fScanTwice;
		pMac->scan.fFirstScanOnly2GChnl = pParam->fFirstScanOnly2GChnl;
		pMac->scan.max_scan_count = pParam->max_scan_count;
		/* This parameter is not available in cfg and not passed from upper layers. Instead it is initialized here
		 * This paramtere is used in concurrency to determine if there are concurrent active sessions.
		 * Is used as a temporary fix to disconnect all active sessions when BMPS enabled so the active session if Infra STA
		 * will automatically connect back and resume BMPS since resume BMPS is not working when moving from concurrent to
		 * single session
		 */
		/* Remove this code once SLM_Sessionization is supported */
		/* BMPS_WORKAROUND_NOT_NEEDED */
		pMac->roam.configParam.doBMPSWorkaround = 0;

		pMac->roam.configParam.nVhtChannelWidth =
			pParam->nVhtChannelWidth;
		pMac->roam.configParam.enable_txbf_sap_mode =
			pParam->enable_txbf_sap_mode;
		pMac->roam.configParam.enable2x2 = pParam->enable2x2;
		pMac->roam.configParam.enableVhtFor24GHz =
			pParam->enableVhtFor24GHz;
		pMac->roam.configParam.enableVhtpAid = pParam->enableVhtpAid;
		pMac->roam.configParam.enableVhtGid = pParam->enableVhtGid;
		pMac->roam.configParam.enableAmpduPs = pParam->enableAmpduPs;
		pMac->roam.configParam.enableHtSmps = pParam->enableHtSmps;
		pMac->roam.configParam.htSmps = pParam->htSmps;
		pMac->roam.configParam.send_smps_action =
			pParam->send_smps_action;
		pMac->roam.configParam.txLdpcEnable = pParam->enableTxLdpc;
		pMac->roam.configParam.rxLdpcEnable = pParam->enableRxLDPC;
		pMac->roam.configParam.ignore_peer_erp_info =
			pParam->ignore_peer_erp_info;
		pMac->roam.configParam.isAmsduSupportInAMPDU =
			pParam->isAmsduSupportInAMPDU;
		pMac->roam.configParam.nSelect5GHzMargin =
			pParam->nSelect5GHzMargin;
		pMac->roam.configParam.isCoalesingInIBSSAllowed =
			pParam->isCoalesingInIBSSAllowed;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		pMac->roam.configParam.cc_switch_mode = pParam->cc_switch_mode;
#endif
		pMac->roam.configParam.allowDFSChannelRoam =
			pParam->allowDFSChannelRoam;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		pMac->roam.configParam.isRoamOffloadEnabled =
			pParam->isRoamOffloadEnabled;
#endif
		pMac->roam.configParam.obssEnabled = pParam->obssEnabled;
		pMac->roam.configParam.conc_custom_rule1 =
			pParam->conc_custom_rule1;
		pMac->roam.configParam.conc_custom_rule2 =
			pParam->conc_custom_rule2;
		pMac->roam.configParam.is_sta_connection_in_5gz_enabled =
			pParam->is_sta_connection_in_5gz_enabled;
		pMac->roam.configParam.sendDeauthBeforeCon =
			pParam->sendDeauthBeforeCon;

		pMac->enable_dot11p = pParam->enable_dot11p;
		pMac->roam.configParam.early_stop_scan_enable =
			pParam->early_stop_scan_enable;
		pMac->roam.configParam.early_stop_scan_min_threshold =
			pParam->early_stop_scan_min_threshold;
		pMac->roam.configParam.early_stop_scan_max_threshold =
			pParam->early_stop_scan_max_threshold;
		pMac->isCoalesingInIBSSAllowed =
			pParam->isCoalesingInIBSSAllowed;

		pMac->roam.configParam.roam_params.dense_rssi_thresh_offset =
			pParam->roam_dense_rssi_thresh_offset;
		pMac->roam.configParam.roam_params.dense_min_aps_cnt =
			pParam->roam_dense_min_aps;
		pMac->roam.configParam.roam_params.traffic_threshold =
			pParam->roam_dense_traffic_thresh;

		pMac->roam.configParam.scan_adaptive_dwell_mode =
			pParam->scan_adaptive_dwell_mode;
		pMac->roam.configParam.roamscan_adaptive_dwell_mode =
			pParam->roamscan_adaptive_dwell_mode;

		/* update p2p offload status */
		pMac->pnoOffload = pParam->pnoOffload;

		pMac->fEnableDebugLog = pParam->fEnableDebugLog;

		/* update interface configuration */
		pMac->sme.max_intf_count = pParam->max_intf_count;

		pMac->enable5gEBT = pParam->enable5gEBT;
		pMac->sme.enableSelfRecovery = pParam->enableSelfRecovery;

		pMac->f_sta_miracast_mcc_rest_time_val =
			pParam->f_sta_miracast_mcc_rest_time_val;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		pMac->sap.sap_channel_avoidance =
			pParam->sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

		pMac->f_prefer_non_dfs_on_radar =
			pParam->f_prefer_non_dfs_on_radar;

		pMac->sme.ps_global_info.ps_enabled =
			pParam->is_ps_enabled;
		pMac->roam.configParam.ignore_peer_ht_opmode =
			pParam->ignore_peer_ht_opmode;
		pMac->fine_time_meas_cap = pParam->fine_time_meas_cap;
		pMac->dual_mac_feature_disable =
			pParam->dual_mac_feature_disable;
		sme_update_roam_pno_channel_prediction_config(pMac, pParam,
				SME_CONFIG_TO_ROAM_CONFIG);
		pMac->roam.configParam.early_stop_scan_enable =
			pParam->early_stop_scan_enable;
		pMac->roam.configParam.early_stop_scan_min_threshold =
			pParam->early_stop_scan_min_threshold;
		pMac->roam.configParam.early_stop_scan_max_threshold =
			pParam->early_stop_scan_max_threshold;
		pMac->roam.configParam.enable_edca_params =
			pParam->enable_edca_params;
		pMac->roam.configParam.edca_vo_cwmin = pParam->edca_vo_cwmin;
		pMac->roam.configParam.edca_vi_cwmin = pParam->edca_vi_cwmin;
		pMac->roam.configParam.edca_bk_cwmin = pParam->edca_bk_cwmin;
		pMac->roam.configParam.edca_be_cwmin = pParam->edca_be_cwmin;

		pMac->roam.configParam.edca_vo_cwmax = pParam->edca_vo_cwmax;
		pMac->roam.configParam.edca_vi_cwmax = pParam->edca_vi_cwmax;
		pMac->roam.configParam.edca_bk_cwmax = pParam->edca_bk_cwmax;
		pMac->roam.configParam.edca_be_cwmax = pParam->edca_be_cwmax;

		pMac->roam.configParam.edca_vo_aifs = pParam->edca_vo_aifs;
		pMac->roam.configParam.edca_vi_aifs = pParam->edca_vi_aifs;
		pMac->roam.configParam.edca_bk_aifs = pParam->edca_bk_aifs;
		pMac->roam.configParam.edca_be_aifs = pParam->edca_be_aifs;

		pMac->roam.configParam.enable_fatal_event =
			pParam->enable_fatal_event;
		pMac->roam.configParam.sta_roam_policy.dfs_mode =
			pParam->sta_roam_policy_params.dfs_mode;
		pMac->roam.configParam.sta_roam_policy.skip_unsafe_channels =
			pParam->sta_roam_policy_params.skip_unsafe_channels;

	}
	return status;
}

QDF_STATUS csr_get_config_param(tpAniSirGlobal pMac, tCsrConfigParam *pParam)
{
	int i;
	tCsrConfig *cfg_params = &pMac->roam.configParam;

	if (!pParam)
		return QDF_STATUS_E_INVAL;

	pParam->WMMSupportMode = cfg_params->WMMSupportMode;
	pParam->Is11eSupportEnabled = cfg_params->Is11eSupportEnabled;
	pParam->FragmentationThreshold = cfg_params->FragmentationThreshold;
	pParam->Is11dSupportEnabled = cfg_params->Is11dSupportEnabled;
	pParam->Is11dSupportEnabledOriginal =
		cfg_params->Is11dSupportEnabledOriginal;
	pParam->Is11hSupportEnabled = cfg_params->Is11hSupportEnabled;
	pParam->channelBondingMode24GHz = csr_convert_phy_cb_state_to_ini_value(
					cfg_params->channelBondingMode24GHz);
	pParam->channelBondingMode5GHz = csr_convert_phy_cb_state_to_ini_value(
					cfg_params->channelBondingMode5GHz);
	pParam->RTSThreshold = cfg_params->RTSThreshold;
	pParam->phyMode = cfg_params->phyMode;
	pParam->shortSlotTime = cfg_params->shortSlotTime;
	pParam->HeartbeatThresh24 = cfg_params->HeartbeatThresh24;
	pParam->HeartbeatThresh50 = cfg_params->HeartbeatThresh50;
	pParam->ProprietaryRatesEnabled = cfg_params->ProprietaryRatesEnabled;
	pParam->TxRate = cfg_params->TxRate;
	pParam->AdHocChannel24 = cfg_params->AdHocChannel24;
	pParam->AdHocChannel5G = cfg_params->AdHocChannel5G;
	pParam->bandCapability = cfg_params->bandCapability;
	pParam->cbChoice = cfg_params->cbChoice;
	pParam->nActiveMaxChnTime = cfg_params->nActiveMaxChnTime;
	pParam->nActiveMinChnTime = cfg_params->nActiveMinChnTime;
	pParam->nPassiveMaxChnTime = cfg_params->nPassiveMaxChnTime;
	pParam->nPassiveMinChnTime = cfg_params->nPassiveMinChnTime;
#ifdef WLAN_AP_STA_CONCURRENCY
	pParam->nActiveMaxChnTimeConc = cfg_params->nActiveMaxChnTimeConc;
	pParam->nActiveMinChnTimeConc = cfg_params->nActiveMinChnTimeConc;
	pParam->nPassiveMaxChnTimeConc = cfg_params->nPassiveMaxChnTimeConc;
	pParam->nPassiveMinChnTimeConc = cfg_params->nPassiveMinChnTimeConc;
	pParam->nRestTimeConc = cfg_params->nRestTimeConc;
	pParam->min_rest_time_conc = cfg_params->min_rest_time_conc;
	pParam->idle_time_conc = cfg_params->idle_time_conc;
	pParam->nNumStaChanCombinedConc = cfg_params->nNumStaChanCombinedConc;
	pParam->nNumP2PChanCombinedConc = cfg_params->nNumP2PChanCombinedConc;
#endif
	pParam->eBand = cfg_params->eBand;
	pParam->nScanResultAgeCount = cfg_params->agingCount;
	pParam->scanAgeTimeNCNPS = cfg_params->scanAgeTimeNCNPS;
	pParam->scanAgeTimeNCPS = cfg_params->scanAgeTimeNCPS;
	pParam->scanAgeTimeCNPS = cfg_params->scanAgeTimeCNPS;
	pParam->scanAgeTimeCPS = cfg_params->scanAgeTimeCPS;
	pParam->bCatRssiOffset = cfg_params->bCatRssiOffset;
	pParam->nRoamingTime = cfg_params->nRoamingTime;
	pParam->fSupplicantCountryCodeHasPriority =
		cfg_params->fSupplicantCountryCodeHasPriority;
	pParam->vccRssiThreshold = cfg_params->vccRssiThreshold;
	pParam->vccUlMacLossThreshold = cfg_params->vccUlMacLossThreshold;
	pParam->nTxPowerCap = cfg_params->nTxPowerCap;
	pParam->allow_tpc_from_ap = cfg_params->allow_tpc_from_ap;
	pParam->statsReqPeriodicity = cfg_params->statsReqPeriodicity;
	pParam->statsReqPeriodicityInPS = cfg_params->statsReqPeriodicityInPS;
	pParam->addTSWhenACMIsOff = cfg_params->addTSWhenACMIsOff;
	pParam->fValidateList = cfg_params->fValidateList;
	pParam->fEnableBypass11d = pMac->scan.fEnableBypass11d;
	pParam->fEnableDFSChnlScan = pMac->scan.fEnableDFSChnlScan;
	pParam->fScanTwice = cfg_params->fScanTwice;
	pParam->fFirstScanOnly2GChnl = pMac->scan.fFirstScanOnly2GChnl;
	pParam->fEnableMCCMode = cfg_params->fenableMCCMode;
	pParam->fAllowMCCGODiffBI = cfg_params->fAllowMCCGODiffBI;
	pParam->scanCfgAgingTime = pMac->scan.scanResultCfgAgingTime;
	qdf_mem_copy(&pParam->neighborRoamConfig,
		     &cfg_params->neighborRoamConfig,
		     sizeof(tCsrNeighborRoamConfigParams));
	pParam->nVhtChannelWidth = cfg_params->nVhtChannelWidth;
	pParam->enable_txbf_sap_mode =
		cfg_params->enable_txbf_sap_mode;
	pParam->enableVhtFor24GHz = cfg_params->enableVhtFor24GHz;
	pParam->ignore_peer_erp_info = cfg_params->ignore_peer_erp_info;
	pParam->enable2x2 = cfg_params->enable2x2;
	qdf_mem_copy(&cfg_params->csr11rConfig, &pParam->csr11rConfig,
		     sizeof(tCsr11rConfigParams));
	pParam->isFastTransitionEnabled = cfg_params->isFastTransitionEnabled;
	pParam->RoamRssiDiff = cfg_params->RoamRssiDiff;
	pParam->nRoamPrefer5GHz = cfg_params->nRoamPrefer5GHz;
	pParam->nRoamIntraBand = cfg_params->nRoamIntraBand;
	pParam->isWESModeEnabled = cfg_params->isWESModeEnabled;
	pParam->nProbes = cfg_params->nProbes;
	pParam->nRoamScanHomeAwayTime = cfg_params->nRoamScanHomeAwayTime;
	pParam->isRoamOffloadScanEnabled = cfg_params->isRoamOffloadScanEnabled;
	pParam->bFastRoamInConIniFeatureEnabled =
		cfg_params->bFastRoamInConIniFeatureEnabled;
	pParam->isFastRoamIniFeatureEnabled =
		cfg_params->isFastRoamIniFeatureEnabled;
#ifdef FEATURE_WLAN_ESE
	pParam->isEseIniFeatureEnabled = cfg_params->isEseIniFeatureEnabled;
#endif
	qdf_mem_copy(&pParam->neighborRoamConfig,
		     &cfg_params->neighborRoamConfig,
		     sizeof(tCsrNeighborRoamConfigParams));
	sms_log(pMac, LOG1,
		FL("Num of Channels in CFG Channel List: %d"),
		cfg_params->neighborRoamConfig.
		neighborScanChanList.numChannels);
	for (i = 0; i < cfg_params->neighborRoamConfig.
	     neighborScanChanList.numChannels; i++) {
		sms_log(pMac, LOG1, "%d ",
			cfg_params->neighborRoamConfig.
			neighborScanChanList.channelList[i]);
	}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	pParam->cc_switch_mode = cfg_params->cc_switch_mode;
#endif
	pParam->enableTxLdpc = cfg_params->txLdpcEnable;
	pParam->enableRxLDPC = cfg_params->rxLdpcEnable;
	pParam->isAmsduSupportInAMPDU = cfg_params->isAmsduSupportInAMPDU;
	pParam->nSelect5GHzMargin = cfg_params->nSelect5GHzMargin;
	pParam->isCoalesingInIBSSAllowed = cfg_params->isCoalesingInIBSSAllowed;
	pParam->allowDFSChannelRoam = cfg_params->allowDFSChannelRoam;
	pParam->nInitialDwellTime = cfg_params->nInitialDwellTime;
	pParam->initial_scan_no_dfs_chnl = cfg_params->initial_scan_no_dfs_chnl;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	pParam->isRoamOffloadEnabled = cfg_params->isRoamOffloadEnabled;
#endif
	pParam->enable_dot11p = pMac->enable_dot11p;
	csr_set_channels(pMac, pParam);
	pParam->obssEnabled = cfg_params->obssEnabled;
	pParam->roam_dense_rssi_thresh_offset =
			cfg_params->roam_params.dense_rssi_thresh_offset;
	pParam->roam_dense_min_aps =
			cfg_params->roam_params.dense_min_aps_cnt;
	pParam->roam_dense_traffic_thresh =
			cfg_params->roam_params.traffic_threshold;

	pParam->scan_adaptive_dwell_mode =
			cfg_params->scan_adaptive_dwell_mode;
	pParam->roamscan_adaptive_dwell_mode =
			cfg_params->roamscan_adaptive_dwell_mode;
	pParam->conc_custom_rule1 = cfg_params->conc_custom_rule1;
	pParam->conc_custom_rule2 = cfg_params->conc_custom_rule2;
	pParam->is_sta_connection_in_5gz_enabled =
		cfg_params->is_sta_connection_in_5gz_enabled;
	pParam->sendDeauthBeforeCon =
		cfg_params->sendDeauthBeforeCon;
	pParam->max_scan_count = pMac->scan.max_scan_count;
	pParam->first_scan_bucket_threshold =
		pMac->first_scan_bucket_threshold;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	pParam->sap_channel_avoidance = pMac->sap.sap_channel_avoidance;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
	pParam->max_intf_count = pMac->sme.max_intf_count;
	pParam->enableSelfRecovery = pMac->sme.enableSelfRecovery;
	pParam->pnoOffload = pMac->pnoOffload;
	pParam->f_prefer_non_dfs_on_radar =
		pMac->f_prefer_non_dfs_on_radar;
	pParam->fine_time_meas_cap = pMac->fine_time_meas_cap;
	pParam->dual_mac_feature_disable =
		pMac->dual_mac_feature_disable;
	pParam->is_ps_enabled = pMac->sme.ps_global_info.ps_enabled;
	pParam->fEnableDebugLog = pMac->fEnableDebugLog;
	pParam->enable5gEBT = pMac->enable5gEBT;
	pParam->f_sta_miracast_mcc_rest_time_val =
		pMac->f_sta_miracast_mcc_rest_time_val;
	sme_update_roam_pno_channel_prediction_config(pMac, pParam,
			ROAM_CONFIG_TO_SME_CONFIG);
	pParam->early_stop_scan_enable =
		pMac->roam.configParam.early_stop_scan_enable;
	pParam->early_stop_scan_min_threshold =
		pMac->roam.configParam.early_stop_scan_min_threshold;
	pParam->early_stop_scan_max_threshold =
		pMac->roam.configParam.early_stop_scan_max_threshold;
	pParam->obss_width_interval =
		pMac->roam.configParam.obss_width_interval;
	pParam->obss_active_dwelltime =
		pMac->roam.configParam.obss_active_dwelltime;
	pParam->obss_passive_dwelltime =
		pMac->roam.configParam.obss_passive_dwelltime;
	pParam->ignore_peer_ht_opmode =
		pMac->roam.configParam.ignore_peer_ht_opmode;
	pParam->enableHtSmps = pMac->roam.configParam.enableHtSmps;
	pParam->htSmps = pMac->roam.configParam.htSmps;
	pParam->send_smps_action = pMac->roam.configParam.send_smps_action;

	pParam->enable_edca_params =
		pMac->roam.configParam.enable_edca_params;
	pParam->edca_vo_cwmin = pMac->roam.configParam.edca_vo_cwmin;
	pParam->edca_vi_cwmin = pMac->roam.configParam.edca_vi_cwmin;
	pParam->edca_bk_cwmin = pMac->roam.configParam.edca_bk_cwmin;
	pParam->edca_be_cwmin = pMac->roam.configParam.edca_be_cwmin;

	pParam->edca_vo_cwmax = pMac->roam.configParam.edca_vo_cwmax;
	pParam->edca_vi_cwmax = pMac->roam.configParam.edca_vi_cwmax;
	pParam->edca_bk_cwmax = pMac->roam.configParam.edca_bk_cwmax;
	pParam->edca_be_cwmax = pMac->roam.configParam.edca_be_cwmax;

	pParam->edca_vo_aifs = pMac->roam.configParam.edca_vo_aifs;
	pParam->edca_vi_aifs = pMac->roam.configParam.edca_vi_aifs;
	pParam->edca_bk_aifs = pMac->roam.configParam.edca_bk_aifs;
	pParam->edca_be_aifs = pMac->roam.configParam.edca_be_aifs;
	pParam->enable_fatal_event =
		pMac->roam.configParam.enable_fatal_event;
	pParam->sta_roam_policy_params.dfs_mode =
		pMac->roam.configParam.sta_roam_policy.dfs_mode;
	pParam->sta_roam_policy_params.skip_unsafe_channels =
		pMac->roam.configParam.sta_roam_policy.skip_unsafe_channels;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_set_phy_mode(tHalHandle hHal, uint32_t phyMode, eCsrBand eBand,
			    bool *pfRestartNeeded)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	bool fRestartNeeded = false;
	eCsrPhyMode newPhyMode = eCSR_DOT11_MODE_AUTO;
	if (eCSR_BAND_24 == eBand) {
		if (CSR_IS_RADIO_A_ONLY(pMac))
			goto end;
		if (eCSR_DOT11_MODE_11a & phyMode)
			goto end;
	}
	if (eCSR_BAND_5G == eBand) {
		if (CSR_IS_RADIO_BG_ONLY(pMac))
			goto end;
		if ((eCSR_DOT11_MODE_11b & phyMode)
			|| (eCSR_DOT11_MODE_11b_ONLY & phyMode)
			|| (eCSR_DOT11_MODE_11g & phyMode)
			|| (eCSR_DOT11_MODE_11g_ONLY & phyMode))
			goto end;
	}
	if (eCSR_DOT11_MODE_AUTO & phyMode)
		newPhyMode = eCSR_DOT11_MODE_AUTO;
	else {
		/* Check for dual band and higher capability first */
		if (eCSR_DOT11_MODE_11n_ONLY & phyMode) {
			if (eCSR_DOT11_MODE_11n_ONLY != phyMode)
				goto end;
			newPhyMode = eCSR_DOT11_MODE_11n_ONLY;
		} else if (eCSR_DOT11_MODE_11g_ONLY & phyMode) {
			if (eCSR_DOT11_MODE_11g_ONLY != phyMode)
				goto end;
			if (eCSR_BAND_5G == eBand)
				goto end;
			newPhyMode = eCSR_DOT11_MODE_11g_ONLY;
			eBand = eCSR_BAND_24;
		} else if (eCSR_DOT11_MODE_11b_ONLY & phyMode) {
			if (eCSR_DOT11_MODE_11b_ONLY != phyMode)
				goto end;
			if (eCSR_BAND_5G == eBand)
				goto end;
			newPhyMode = eCSR_DOT11_MODE_11b_ONLY;
			eBand = eCSR_BAND_24;
		} else if (eCSR_DOT11_MODE_11n & phyMode) {
			newPhyMode = eCSR_DOT11_MODE_11n;
		} else if (eCSR_DOT11_MODE_abg & phyMode) {
			newPhyMode = eCSR_DOT11_MODE_abg;
		} else if (eCSR_DOT11_MODE_11a & phyMode) {
			if ((eCSR_DOT11_MODE_11g & phyMode)
				|| (eCSR_DOT11_MODE_11b & phyMode)) {
				if (eCSR_BAND_ALL == eBand)
					newPhyMode = eCSR_DOT11_MODE_abg;
				else
					goto end;
			} else {
				newPhyMode = eCSR_DOT11_MODE_11a;
				eBand = eCSR_BAND_5G;
			}
		} else if (eCSR_DOT11_MODE_11g & phyMode) {
			newPhyMode = eCSR_DOT11_MODE_11g;
			eBand = eCSR_BAND_24;
		} else if (eCSR_DOT11_MODE_11b & phyMode) {
			newPhyMode = eCSR_DOT11_MODE_11b;
			eBand = eCSR_BAND_24;
		} else {
			/* We will never be here */
			sms_log(pMac, LOGE,
					FL("can't recognize phymode 0x%08X"),
					phyMode);
			newPhyMode = eCSR_DOT11_MODE_AUTO;
		}
	}
	/* Done validating */
	status = QDF_STATUS_SUCCESS;
	/* Now we need to check whether a restart is needed. */
	if (eBand != pMac->roam.configParam.eBand) {
		fRestartNeeded = true;
		goto end;
	}
	if (newPhyMode != pMac->roam.configParam.phyMode) {
		fRestartNeeded = true;
		goto end;
	}
end:
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->roam.configParam.eBand = eBand;
		pMac->roam.configParam.phyMode = newPhyMode;
		if (pfRestartNeeded)
			*pfRestartNeeded = fRestartNeeded;
	}
	return status;
}

/**
 * csr_prune_ch_list() - prunes the channel list to keep only a type of channels
 * @ch_lst:        existing channel list
 * @is_24_GHz:     indicates if 2.5 GHz or 5 GHz channels are required
 *
 * Return: void
 */
void csr_prune_ch_list(tCsrChannel *ch_lst, bool is_24_GHz)
{
	uint8_t idx = 0, num_channels = 0;
	for ( ; idx < ch_lst->numChannels; idx++) {
		if (is_24_GHz) {
			if (CDS_IS_CHANNEL_24GHZ(ch_lst->channelList[idx])) {
				ch_lst->channelList[num_channels] =
					ch_lst->channelList[idx];
				num_channels++;
			}
		} else {
			if (CDS_IS_CHANNEL_5GHZ(ch_lst->channelList[idx])) {
				ch_lst->channelList[num_channels] =
					ch_lst->channelList[idx];
				num_channels++;
			}
		}
	}
	/*
	 * Cleanup the rest of channels. Note we only need to clean up the
	 * channels if we had to trim the list. Calling qdf_mem_set() with a 0
	 * size is going to throw asserts on the debug builds so let's be a bit
	 * smarter about that. Zero out the reset of the channels only if we
	 * need to. The amount of memory to clear is the number of channesl that
	 * we trimmed (ch_lst->numChannels - num_channels) times the size of a
	 * channel in the structure.
	 */
	if (ch_lst->numChannels > num_channels) {
		qdf_mem_set(&ch_lst->channelList[num_channels],
			    sizeof(ch_lst->channelList[0]) *
			    (ch_lst->numChannels - num_channels), 0);
	}
	ch_lst->numChannels = num_channels;
}

/**
 * csr_prune_channel_list_for_mode() - prunes the channel list
 * @mac_ctx:       global mac context
 * @ch_lst:        existing channel list
 *
 * Prunes the channel list according to band stored in mac_ctx
 *
 * Return: void
 */
void csr_prune_channel_list_for_mode(tpAniSirGlobal mac_ctx,
				     tCsrChannel *ch_lst)
{
	/* for dual band NICs, don't need to trim the channel list.... */
	if (CSR_IS_OPEARTING_DUAL_BAND(mac_ctx))
		return;
	/*
	 * 2.4 GHz band operation requires the channel list to be trimmed to
	 * the 2.4 GHz channels only
	 */
	if (CSR_IS_24_BAND_ONLY(mac_ctx))
		csr_prune_ch_list(ch_lst, true);
	else if (CSR_IS_5G_BAND_ONLY(mac_ctx))
		csr_prune_ch_list(ch_lst, false);
}

#define INFRA_AP_DEFAULT_CHANNEL 6
QDF_STATUS csr_is_valid_channel(tpAniSirGlobal pMac, uint8_t chnNum)
{
	uint8_t index = 0;
	QDF_STATUS status = QDF_STATUS_E_NOSUPPORT;

	/* regulatory check */
	for (index = 0; index < pMac->scan.base_channels.numChannels;
	     index++) {
		if (pMac->scan.base_channels.channelList[index] == chnNum) {
			status = QDF_STATUS_SUCCESS;
			break;
		}
	}

	if (status == QDF_STATUS_SUCCESS) {
		/* dfs nol */
		for (index = 0;
		     index <
		     pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels;
		     index++) {
			tSapDfsNolInfo *dfsChan =
				&pMac->sap.SapDfsInfo.sapDfsChannelNolList[index];
			if ((dfsChan->dfs_channel_number == chnNum)
			    && (dfsChan->radar_status_flag ==
				eSAP_DFS_CHANNEL_UNAVAILABLE)) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("channel %d is in dfs nol"),
					  chnNum);
				status = QDF_STATUS_E_FAILURE;
				break;
			}
		}
	}

	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("channel %d is not available"), chnNum);
	}

	return status;
}

QDF_STATUS csr_get_channel_and_power_list(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t num20MHzChannelsFound = 0;
	QDF_STATUS qdf_status;
	uint8_t Index = 0;

	qdf_status =
		cds_get_channel_list_with_power(pMac->scan.defaultPowerTable,
						&num20MHzChannelsFound);

	if ((QDF_STATUS_SUCCESS != qdf_status) ||
	    (num20MHzChannelsFound == 0)) {
		sms_log(pMac, LOGE, FL("failed to get channels "));
		status = QDF_STATUS_E_FAILURE;
	} else {
		if (num20MHzChannelsFound > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
			num20MHzChannelsFound = WNI_CFG_VALID_CHANNEL_LIST_LEN;
		}
		pMac->scan.numChannelsDefault = num20MHzChannelsFound;
		/* Move the channel list to the global data */
		/* structure -- this will be used as the scan list */
		for (Index = 0; Index < num20MHzChannelsFound; Index++) {
			pMac->scan.base_channels.channelList[Index] =
				pMac->scan.defaultPowerTable[Index].chan_num;
		}
		pMac->scan.base_channels.numChannels =
			num20MHzChannelsFound;
	}
	return status;
}

QDF_STATUS csr_apply_channel_and_power_list(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	csr_prune_channel_list_for_mode(pMac, &pMac->scan.base_channels);
	csr_save_channel_power_for_band(pMac, false);
	csr_save_channel_power_for_band(pMac, true);
	/* Apply the base channel list, power info, and set the Country code... */
	csr_apply_channel_power_info_to_fw(pMac,
					   &pMac->scan.base_channels,
					   pMac->scan.countryCodeCurrent);

	csr_init_operating_classes((tHalHandle) pMac);
	return status;
}

QDF_STATUS csr_change_config_params(tpAniSirGlobal pMac,
				    tCsrUpdateConfigParam *pUpdateConfigParam)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsr11dinfo *ps11dinfo = NULL;
	ps11dinfo = &pUpdateConfigParam->Csr11dinfo;
	status = csr_init11d_info(pMac, ps11dinfo);
	return status;
}

static QDF_STATUS csr_init11d_info(tpAniSirGlobal pMac, tCsr11dinfo *ps11dinfo)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint8_t index;
	uint32_t count = 0;
	tSirMacChanInfo *pChanInfo;
	tSirMacChanInfo *pChanInfoStart;
	bool applyConfig = true;

	pMac->scan.currentCountryRSSI = -128;
	if (!ps11dinfo) {
		return status;
	}
	if (ps11dinfo->Channels.numChannels
	    && (WNI_CFG_VALID_CHANNEL_LIST_LEN >=
		ps11dinfo->Channels.numChannels)) {
		pMac->scan.base_channels.numChannels =
			ps11dinfo->Channels.numChannels;
		qdf_mem_copy(pMac->scan.base_channels.channelList,
			     ps11dinfo->Channels.channelList,
			     ps11dinfo->Channels.numChannels);
	} else {
		/* No change */
		return QDF_STATUS_SUCCESS;
	}
	/* legacy maintenance */

	qdf_mem_copy(pMac->scan.countryCodeDefault, ps11dinfo->countryCode,
		     WNI_CFG_COUNTRY_CODE_LEN);

	/* Tush: at csropen get this initialized with default, during csr reset if this */
	/* already set with some value no need initilaize with default again */
	if (0 == pMac->scan.countryCodeCurrent[0]) {
		qdf_mem_copy(pMac->scan.countryCodeCurrent,
			     ps11dinfo->countryCode, WNI_CFG_COUNTRY_CODE_LEN);
	}
	/* need to add the max power channel list */
	pChanInfo =
		qdf_mem_malloc(sizeof(tSirMacChanInfo) *
			       WNI_CFG_VALID_CHANNEL_LIST_LEN);
	if (pChanInfo != NULL) {
		qdf_mem_set(pChanInfo,
			    sizeof(tSirMacChanInfo) *
			    WNI_CFG_VALID_CHANNEL_LIST_LEN, 0);

		pChanInfoStart = pChanInfo;
		for (index = 0; index < ps11dinfo->Channels.numChannels;
		     index++) {
			pChanInfo->firstChanNum =
				ps11dinfo->ChnPower[index].firstChannel;
			pChanInfo->numChannels =
				ps11dinfo->ChnPower[index].numChannels;
			pChanInfo->maxTxPower =
				QDF_MIN(ps11dinfo->ChnPower[index].maxtxPower,
					pMac->roam.configParam.nTxPowerCap);
			pChanInfo++;
			count++;
		}
		if (count) {
			csr_save_to_channel_power2_g_5_g(pMac,
							 count *
							 sizeof(tSirMacChanInfo),
							 pChanInfoStart);
		}
		qdf_mem_free(pChanInfoStart);
	}
	/* Only apply them to CFG when not in STOP state. Otherwise they will be applied later */
	if (QDF_IS_STATUS_SUCCESS(status)) {
		for (index = 0; index < CSR_ROAM_SESSION_MAX; index++) {
			if ((CSR_IS_SESSION_VALID(pMac, index))
			    && CSR_IS_ROAM_STOP(pMac, index)) {
				applyConfig = false;
			}
		}

		if (true == applyConfig) {
			/* Apply the base channel list, power info, and set the Country code... */
			csr_apply_channel_power_info_to_fw(pMac,
							   &pMac->scan.
							   base_channels,
							   pMac->scan.
							   countryCodeCurrent);
		}
	}
	return status;
}

/* Initialize the Channel + Power List in the local cache and in the CFG */
QDF_STATUS csr_init_channel_power_list(tpAniSirGlobal pMac, tCsr11dinfo *ps11dinfo)
{
	uint8_t index;
	uint32_t count = 0;
	tSirMacChanInfo *pChanInfo;
	tSirMacChanInfo *pChanInfoStart;

	if (!ps11dinfo || !pMac) {
		return QDF_STATUS_E_FAILURE;
	}

	pChanInfo =
		qdf_mem_malloc(sizeof(tSirMacChanInfo) *
			       WNI_CFG_VALID_CHANNEL_LIST_LEN);
	if (pChanInfo != NULL) {
		qdf_mem_set(pChanInfo,
			    sizeof(tSirMacChanInfo) *
			    WNI_CFG_VALID_CHANNEL_LIST_LEN, 0);
		pChanInfoStart = pChanInfo;

		for (index = 0; index < ps11dinfo->Channels.numChannels;
		     index++) {
			pChanInfo->firstChanNum =
				ps11dinfo->ChnPower[index].firstChannel;
			pChanInfo->numChannels =
				ps11dinfo->ChnPower[index].numChannels;
			pChanInfo->maxTxPower =
				QDF_MIN(ps11dinfo->ChnPower[index].maxtxPower,
					pMac->roam.configParam.nTxPowerCap);
			pChanInfo++;
			count++;
		}
		if (count) {
			csr_save_to_channel_power2_g_5_g(pMac,
							 count *
							 sizeof(tSirMacChanInfo),
							 pChanInfoStart);
		}
		qdf_mem_free(pChanInfoStart);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_roam_remove_duplicate_cmd_from_list()- Remove duplicate roam cmd from
 * list
 *
 * @mac_ctx: pointer to global mac
 * @session_id: session id for the cmd
 * @list: pending list from which cmd needs to be removed
 * @command: cmd to be removed, can be NULL
 * @roam_reason: cmd with reason to be removed
 *
 * Remove duplicate command from the pending list.
 *
 * Return: void
 */
static void csr_roam_remove_duplicate_cmd_from_list(tpAniSirGlobal mac_ctx,
			uint32_t session_id, tDblLinkList *list,
			tSmeCmd *command, eCsrRoamReason roam_reason)
{
	tListElem *entry, *next_entry;
	tSmeCmd *dup_cmd;
	tDblLinkList local_list;

	qdf_mem_zero(&local_list, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(mac_ctx->hHdd, &local_list))) {
		sms_log(mac_ctx, LOGE, FL(" failed to open list"));
		return;
	}
	csr_ll_lock(list);
	entry = csr_ll_peek_head(list, LL_ACCESS_NOLOCK);
	while (entry) {
		next_entry = csr_ll_next(list, entry, LL_ACCESS_NOLOCK);
		dup_cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
		/*
		 * Remove the previous command if..
		 * - the new roam command is for the same RoamReason...
		 * - the new roam command is a NewProfileList.
		 * - the new roam command is a Forced Dissoc
		 * - the new roam command is from an 802.11 OID
		 *   (OID_SSID or OID_BSSID).
		 */
		if ((command && (command->sessionId == dup_cmd->sessionId) &&
			((command->command == dup_cmd->command) &&
			/*
			 * This peermac check is requried for Softap/GO
			 * scenarios. for STA scenario below OR check will
			 * suffice as command will always be NULL for
			 * STA scenarios
			 */
			(!qdf_mem_cmp(dup_cmd->u.roamCmd.peerMac,
				command->u.roamCmd.peerMac,
					sizeof(QDF_MAC_ADDR_SIZE))) &&
				((command->u.roamCmd.roamReason ==
					dup_cmd->u.roamCmd.roamReason) ||
				(eCsrForcedDisassoc ==
					command->u.roamCmd.roamReason) ||
				(eCsrHddIssued ==
					command->u.roamCmd.roamReason)))) ||
			/* OR if pCommand is NULL */
			((session_id == dup_cmd->sessionId) &&
			(eSmeCommandRoam == dup_cmd->command) &&
			((eCsrForcedDisassoc == roam_reason) ||
			(eCsrHddIssued == roam_reason)))) {
			sms_log(mac_ctx, LOGW, FL("RoamReason = %d"),
					dup_cmd->u.roamCmd.roamReason);
			/* Remove the roam command from the pending list */
			if (csr_ll_remove_entry(list, entry, LL_ACCESS_NOLOCK))
				csr_ll_insert_tail(&local_list, entry,
							LL_ACCESS_NOLOCK);
		}
		entry = next_entry;
	}
	csr_ll_unlock(list);

	while ((entry = csr_ll_remove_head(&local_list, LL_ACCESS_NOLOCK))) {
		dup_cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
		/* Tell caller that the command is cancelled */
		csr_roam_call_callback(mac_ctx, dup_cmd->sessionId, NULL,
				dup_cmd->u.roamCmd.roamId,
				eCSR_ROAM_CANCELLED, eCSR_ROAM_RESULT_NONE);
		csr_release_command_roam(mac_ctx, dup_cmd);
	}
	csr_ll_close(&local_list);
}

/**
 * csr_roam_remove_duplicate_command()- Remove duplicate roam cmd
 * from pending lists.
 *
 * @mac_ctx: pointer to global mac
 * @session_id: session id for the cmd
 * @command: cmd to be removed, can be null
 * @roam_reason: cmd with reason to be removed
 *
 * Remove duplicate command from the sme and roam pending list.
 *
 * Return: void
 */
void csr_roam_remove_duplicate_command(tpAniSirGlobal mac_ctx,
			uint32_t session_id, tSmeCmd *command,
			eCsrRoamReason roam_reason)
{
	/* Always lock active list before locking pending lists */
	csr_ll_lock(&mac_ctx->sme.smeCmdActiveList);
	csr_roam_remove_duplicate_cmd_from_list(mac_ctx,
		session_id, &mac_ctx->sme.smeCmdPendingList,
		command, roam_reason);
	csr_roam_remove_duplicate_cmd_from_list(mac_ctx,
		session_id, &mac_ctx->roam.roamCmdPendingList,
		command, roam_reason);
	csr_ll_unlock(&mac_ctx->sme.smeCmdActiveList);
}

/**
 * csr_roam_populate_channels() - Helper function to populate channels
 * @beacon_ies: pointer to beacon ie
 * @roam_info: Roaming related information
 * @chan1: center freq 1
 * @chan2: center freq2
 *
 * This function will issue populate chan1 and chan2 based on beacon ie
 *
 * Return: none.
 */
static void csr_roam_populate_channels(tDot11fBeaconIEs *beacon_ies,
			tCsrRoamInfo *roam_info,
			uint8_t *chan1, uint8_t *chan2)
{
	ePhyChanBondState phy_state;
	if (beacon_ies->VHTOperation.present) {
		*chan1 = beacon_ies->VHTOperation.chanCenterFreqSeg1;
		*chan2 = beacon_ies->VHTOperation.chanCenterFreqSeg2;
		roam_info->chan_info.info = MODE_11AC_VHT80;
	} else if (beacon_ies->HTInfo.present) {
		if (beacon_ies->HTInfo.recommendedTxWidthSet ==
			eHT_CHANNEL_WIDTH_40MHZ) {
			phy_state = beacon_ies->HTInfo.secondaryChannelOffset;
			if (phy_state == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
				*chan1 = beacon_ies->HTInfo.primaryChannel +
						CSR_CB_CENTER_CHANNEL_OFFSET;
			else if (phy_state == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
				*chan1 = beacon_ies->HTInfo.primaryChannel -
						CSR_CB_CENTER_CHANNEL_OFFSET;
			else
				*chan1 = beacon_ies->HTInfo.primaryChannel;

			roam_info->chan_info.info = MODE_11NA_HT40;
		} else {
			*chan1 = beacon_ies->HTInfo.primaryChannel;
			roam_info->chan_info.info = MODE_11NA_HT20;
		}
		*chan2 = 0;
	} else {
		*chan1 = 0;
		*chan2 = 0;
		roam_info->chan_info.info = MODE_11A;
	}
}

QDF_STATUS csr_roam_call_callback(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamInfo *pRoamInfo, uint32_t roamId,
				  eRoamCmdStatus u1, eCsrRoamResult u2)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	uint32_t rssi = 0;
	WLAN_HOST_DIAG_EVENT_DEF(connectionStatus,
			host_event_wlan_status_payload_type);
#endif
	tCsrRoamSession *pSession;
	tDot11fBeaconIEs *beacon_ies = NULL;
	uint8_t chan1, chan2;

	if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
		sms_log(pMac, LOGE, "Session ID:%d is not valid", sessionId);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	pSession = CSR_GET_SESSION(pMac, sessionId);

	if (false == pSession->sessionActive) {
		sms_log(pMac, LOG1, "%s Session is not Active", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	sms_log(pMac, LOG4, "Received RoamCmdStatus %d with Roam Result %d", u1,
		u2);

	if (eCSR_ROAM_ASSOCIATION_COMPLETION == u1 &&
			eCSR_ROAM_RESULT_ASSOCIATED == u2 && pRoamInfo) {
		sms_log(pMac, LOGW,
			FL("Assoc complete result=%d status=%d reason=%d"),
			u2, pRoamInfo->statusCode, pRoamInfo->reasonCode);
		beacon_ies = qdf_mem_malloc(sizeof(tDot11fBeaconIEs));
		if ((NULL != beacon_ies) && (NULL != pRoamInfo->pBssDesc)) {
			status = csr_parse_bss_description_ies(
					(tHalHandle) pMac, pRoamInfo->pBssDesc,
					beacon_ies);
			csr_roam_populate_channels(beacon_ies, pRoamInfo,
					&chan1, &chan2);
			if (0 != chan1)
				pRoamInfo->chan_info.band_center_freq1 =
					cds_chan_to_freq(chan1);
			else
				pRoamInfo->chan_info.band_center_freq1 = 0;
			if (0 != chan2)
				pRoamInfo->chan_info.band_center_freq2 =
					cds_chan_to_freq(chan2);
			else
				pRoamInfo->chan_info.band_center_freq2 = 0;
		} else {
			pRoamInfo->chan_info.band_center_freq1 = 0;
			pRoamInfo->chan_info.band_center_freq2 = 0;
			pRoamInfo->chan_info.info = 0;
		}
		pRoamInfo->chan_info.chan_id =
			pRoamInfo->u.pConnectedProfile->operationChannel;
		pRoamInfo->chan_info.mhz =
			cds_chan_to_freq(pRoamInfo->chan_info.chan_id);
		pRoamInfo->chan_info.reg_info_1 =
			(csr_get_cfg_max_tx_power(pMac,
				pRoamInfo->chan_info.chan_id) << 16);
		pRoamInfo->chan_info.reg_info_2 =
			(csr_get_cfg_max_tx_power(pMac,
				pRoamInfo->chan_info.chan_id) << 8);
		qdf_mem_free(beacon_ies);
	} else if ((u1 == eCSR_ROAM_FT_REASSOC_FAILED)
			&& (pSession->bRefAssocStartCnt)) {
		/*
		 * Decrement bRefAssocStartCnt for FT reassoc failure.
		 * Reason: For FT reassoc failures, we first call
		 * csr_roam_call_callback before notifying a failed roam
		 * completion through csr_roam_complete. The latter in
		 * turn calls csr_roam_process_results which tries to
		 * once again call csr_roam_call_callback if bRefAssocStartCnt
		 * is non-zero. Since this is redundant for FT reassoc
		 * failure, decrement bRefAssocStartCnt.
		 */
		pSession->bRefAssocStartCnt--;
	} else if (u1 == eCSR_ROAM_SET_CHANNEL_RSP && u2 ==
				eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS)
		pSession->connectedProfile.operationChannel =
			pRoamInfo->channelChangeRespEvent->newChannelNumber;

	if (NULL != pSession->callback) {
		if (pRoamInfo) {
			pRoamInfo->sessionId = (uint8_t) sessionId;
			/*
			 * the reasonCode will be passed to supplicant by
			 * cfg80211_disconnected. Based on the document,
			 * the reason code passed to supplicant needs to set
			 * to 0 if unknow. eSIR_BEACON_MISSED reason code is not
			 * recognizable so that we set to 0 instead.
			 */
			pRoamInfo->reasonCode =
				(pRoamInfo->reasonCode == eSIR_BEACON_MISSED) ?
				0 : pRoamInfo->reasonCode;
		}
		status = pSession->callback(pSession->pContext, pRoamInfo,
					roamId, u1, u2);
	}
	/*
	 * EVENT_WLAN_STATUS_V2: eCSR_ROAM_ASSOCIATION_COMPLETION,
	 *                    eCSR_ROAM_LOSTLINK,
	 *                    eCSR_ROAM_DISASSOCIATED,
	 */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	qdf_mem_set(&connectionStatus,
			sizeof(host_event_wlan_status_payload_type), 0);

	if ((eCSR_ROAM_ASSOCIATION_COMPLETION == u1)
			&& (eCSR_ROAM_RESULT_ASSOCIATED == u2) && pRoamInfo) {
		connectionStatus.eventId = eCSR_WLAN_STATUS_CONNECT;
		connectionStatus.bssType =
			pRoamInfo->u.pConnectedProfile->BSSType;

		if (NULL != pRoamInfo->pBssDesc) {
			connectionStatus.rssi =
				pRoamInfo->pBssDesc->rssi * (-1);
			connectionStatus.channel =
				pRoamInfo->pBssDesc->channelId;
		}
		if (cfg_set_int(pMac, WNI_CFG_CURRENT_RSSI,
				connectionStatus.rssi) == eSIR_FAILURE)
			sms_log(pMac, LOGE,
				FL("Can't pass WNI_CFG_CURRENT_RSSI to cfg"));

		connectionStatus.qosCapability =
			pRoamInfo->u.pConnectedProfile->qosConnection;
		connectionStatus.authType =
			(uint8_t) diag_auth_type_from_csr_type(
				pRoamInfo->u.pConnectedProfile->AuthType);
		connectionStatus.encryptionType =
			(uint8_t) diag_enc_type_from_csr_type(
				pRoamInfo->u.pConnectedProfile->EncryptionType);
		qdf_mem_copy(connectionStatus.ssid,
				pRoamInfo->u.pConnectedProfile->SSID.ssId,
				pRoamInfo->u.pConnectedProfile->SSID.length);

		connectionStatus.reason = eCSR_REASON_UNSPECIFIED;
		qdf_mem_copy(&pMac->sme.eventPayload, &connectionStatus,
				sizeof(host_event_wlan_status_payload_type));
		WLAN_HOST_DIAG_EVENT_REPORT(&connectionStatus,
				EVENT_WLAN_STATUS_V2);
	}
	if ((eCSR_ROAM_MIC_ERROR_IND == u1)
			|| (eCSR_ROAM_RESULT_MIC_FAILURE == u2)) {
		qdf_mem_copy(&connectionStatus, &pMac->sme.eventPayload,
				sizeof(host_event_wlan_status_payload_type));
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(pMac,
				WNI_CFG_CURRENT_RSSI, &rssi)))
			connectionStatus.rssi = rssi;

		connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
		connectionStatus.reason = eCSR_REASON_MIC_ERROR;
		WLAN_HOST_DIAG_EVENT_REPORT(&connectionStatus,
				EVENT_WLAN_STATUS_V2);
	}
	if (eCSR_ROAM_RESULT_FORCED == u2) {
		qdf_mem_copy(&connectionStatus, &pMac->sme.eventPayload,
				sizeof(host_event_wlan_status_payload_type));
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(pMac,
				WNI_CFG_CURRENT_RSSI, &rssi)))
			connectionStatus.rssi = rssi;

		connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
		connectionStatus.reason = eCSR_REASON_USER_REQUESTED;
		WLAN_HOST_DIAG_EVENT_REPORT(&connectionStatus,
				EVENT_WLAN_STATUS_V2);
	}
	if (eCSR_ROAM_RESULT_DISASSOC_IND == u2) {
		qdf_mem_copy(&connectionStatus, &pMac->sme.eventPayload,
				sizeof(host_event_wlan_status_payload_type));
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(pMac,
				WNI_CFG_CURRENT_RSSI, &rssi)))
			connectionStatus.rssi = rssi;

		connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
		connectionStatus.reason = eCSR_REASON_DISASSOC;
		if (pRoamInfo)
			connectionStatus.reasonDisconnect =
				pRoamInfo->reasonCode;

		WLAN_HOST_DIAG_EVENT_REPORT(&connectionStatus,
				EVENT_WLAN_STATUS_V2);
	}
	if (eCSR_ROAM_RESULT_DEAUTH_IND == u2) {
		qdf_mem_copy(&connectionStatus, &pMac->sme.eventPayload,
				sizeof(host_event_wlan_status_payload_type));
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(pMac,
				WNI_CFG_CURRENT_RSSI, &rssi)))
			connectionStatus.rssi = rssi;

		connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
		connectionStatus.reason = eCSR_REASON_DEAUTH;
		if (pRoamInfo)
			connectionStatus.reasonDisconnect =
				pRoamInfo->reasonCode;
		WLAN_HOST_DIAG_EVENT_REPORT(&connectionStatus,
				EVENT_WLAN_STATUS_V2);
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	return status;
}

/* Returns whether handoff is currently in progress or not */
bool csr_roam_is_handoff_in_progress(tpAniSirGlobal pMac, uint8_t sessionId)
{
	return csr_neighbor_roam_is_handoff_in_progress(pMac, sessionId);
}

QDF_STATUS csr_roam_issue_disassociate(tpAniSirGlobal pMac, uint32_t sessionId,
				       eCsrRoamSubState NewSubstate,
				       bool fMICFailure)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr bssId = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	uint16_t reasonCode;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (fMICFailure) {
		reasonCode = eSIR_MAC_MIC_FAILURE_REASON;
	} else if (NewSubstate == eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF) {
		reasonCode = eSIR_MAC_DISASSOC_DUE_TO_FTHANDOFF_REASON;
	} else if (eCSR_ROAM_SUBSTATE_DISASSOC_STA_HAS_LEFT == NewSubstate) {
		reasonCode = eSIR_MAC_DISASSOC_LEAVING_BSS_REASON;
		NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_FORCED;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL
				  ("set to reason code eSIR_MAC_DISASSOC_LEAVING_BSS_REASON"
				  " and set back NewSubstate"));
	} else {
		reasonCode = eSIR_MAC_UNSPEC_FAILURE_REASON;
	}
	if ((csr_roam_is_handoff_in_progress(pMac, sessionId)) &&
	    (NewSubstate != eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF)) {
		tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
			&pMac->roam.neighborRoamInfo[sessionId];
		qdf_copy_macaddr(&bssId,
			      pNeighborRoamInfo->csrNeighborRoamProfile.BSSIDs.
			      bssid);
	} else if (pSession->pConnectBssDesc) {
		qdf_mem_copy(&bssId.bytes, pSession->pConnectBssDesc->bssId,
			     sizeof(struct qdf_mac_addr));
	}

	sms_log(pMac, LOG2,
		FL("CSR Attempting to Disassociate Bssid=" MAC_ADDRESS_STR
		   " subState = %s reason=%d"), MAC_ADDR_ARRAY(bssId.bytes),
		mac_trace_getcsr_roam_sub_state(NewSubstate), reasonCode);

	csr_roam_substate_change(pMac, NewSubstate, sessionId);

	status = csr_send_mb_disassoc_req_msg(pMac, sessionId, bssId.bytes,
						reasonCode);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_link_down(pMac, sessionId);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		/* no need to tell QoS that we are disassociating, it will be taken care off in assoc req for HO */
		if (eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF != NewSubstate) {
			/* notify QoS module that disassoc happening */
			sme_qos_csr_event_ind(pMac, (uint8_t) sessionId,
					      SME_QOS_CSR_DISCONNECT_REQ, NULL);
		}
#endif
	} else {
		sms_log(pMac, LOGW,
			FL("csr_send_mb_disassoc_req_msg failed with status %d"),
			status);
	}

	return status;
}

/**
 * csr_roam_issue_disassociate_sta_cmd() - disassociate a associated station
 * @sessionId:     Session Id for Soft AP
 * @p_del_sta_params: Pointer to parameters of the station to disassoc
 *
 * CSR function that HDD calls to delete a associated station
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_* on error
 */
QDF_STATUS csr_roam_issue_disassociate_sta_cmd(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       struct tagCsrDelStaParams
					       *p_del_sta_params)

{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;

	do {
		pCommand = csr_get_command_buffer(pMac);
		if (!pCommand) {
			sms_log(pMac, LOGE, FL(" fail to get command buffer"));
			status = QDF_STATUS_E_RESOURCES;
			break;
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.roamReason = eCsrForcedDisassocSta;
		qdf_mem_copy(pCommand->u.roamCmd.peerMac,
				p_del_sta_params->peerMacAddr.bytes,
				sizeof(pCommand->u.roamCmd.peerMac));
		pCommand->u.roamCmd.reason =
			(tSirMacReasonCodes)p_del_sta_params->reason_code;
		status = csr_queue_sme_command(pMac, pCommand, false);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_release_command_roam(pMac, pCommand);
		}
	} while (0);

	return status;
}

/**
 * csr_roam_issue_deauthSta() - delete a associated station
 * @sessionId:     Session Id for Soft AP
 * @pDelStaParams: Pointer to parameters of the station to deauthenticate
 *
 * CSR function that HDD calls to delete a associated station
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_** on error
 */
QDF_STATUS csr_roam_issue_deauth_sta_cmd(tpAniSirGlobal pMac,
		uint32_t sessionId,
		struct tagCsrDelStaParams *pDelStaParams)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;

	do {
		pCommand = csr_get_command_buffer(pMac);
		if (!pCommand) {
			sms_log(pMac, LOGE, FL(" fail to get command buffer"));
			status = QDF_STATUS_E_RESOURCES;
			break;
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.roamReason = eCsrForcedDeauthSta;
		qdf_mem_copy(pCommand->u.roamCmd.peerMac,
			     pDelStaParams->peerMacAddr.bytes,
			     sizeof(tSirMacAddr));
		pCommand->u.roamCmd.reason =
			(tSirMacReasonCodes)pDelStaParams->reason_code;
		status = csr_queue_sme_command(pMac, pCommand, false);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_release_command_roam(pMac, pCommand);
		}
	} while (0);

	return status;
}

static QDF_STATUS
csr_send_mb_tkip_counter_measures_req_msg(tpAniSirGlobal pMac,
					  uint32_t sessionId, bool bEnable,
					  struct qdf_mac_addr *bssId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeTkipCntrMeasReq *pMsg;
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeTkipCntrMeasReq));
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		qdf_mem_set(pMsg, sizeof(tSirSmeTkipCntrMeasReq), 0);
		pMsg->messageType = eWNI_SME_TKIP_CNTR_MEAS_REQ;
		pMsg->length = sizeof(tSirSmeTkipCntrMeasReq);
		pMsg->sessionId = sessionId;
		pMsg->transactionId = 0;
		qdf_copy_macaddr(&pMsg->bssId, bssId);
		pMsg->bEnable = bEnable;
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS
csr_roam_issue_tkip_counter_measures(tpAniSirGlobal pMac, uint32_t sessionId,
				     bool bEnable)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct qdf_mac_addr bssId = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE,
			"csr_roam_issue_tkip_counter_measures:CSR Session not found");
		return status;
	}
	if (pSession->pConnectBssDesc) {
		qdf_mem_copy(bssId.bytes, pSession->pConnectBssDesc->bssId,
			     sizeof(struct qdf_mac_addr));
	} else {
		sms_log(pMac, LOGE,
			"csr_roam_issue_tkip_counter_measures:Connected BSS Description in CSR Session not found");
		return status;
	}
	sms_log(pMac, LOG2,
		"CSR issuing tkip counter measures for Bssid = " MAC_ADDRESS_STR
		", Enable = %d", MAC_ADDR_ARRAY(bssId.bytes), bEnable);
	status =
		csr_send_mb_tkip_counter_measures_req_msg(pMac, sessionId,
							bEnable, &bssId);
	return status;
}

QDF_STATUS
csr_roam_get_associated_stas(tpAniSirGlobal pMac, uint32_t sessionId,
			     QDF_MODULE_ID modId, void *pUsrContext,
			     void *pfnSapEventCallback, uint8_t *pAssocStasBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr bssId = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE,
			"csr_roam_get_associated_stas:CSR Session not found");
		return status;
	}
	if (pSession->pConnectBssDesc) {
		qdf_mem_copy(bssId.bytes, pSession->pConnectBssDesc->bssId,
			     sizeof(struct qdf_mac_addr));
	} else {
		sms_log(pMac, LOGE,
			"csr_roam_get_associated_stas:Connected BSS Description in CSR Session not found");
		return status;
	}
	sms_log(pMac, LOG2,
		"CSR getting associated stations for Bssid = " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(bssId.bytes));
	status =
		csr_send_mb_get_associated_stas_req_msg(pMac, sessionId, modId,
							bssId,
							pUsrContext,
							pfnSapEventCallback,
							pAssocStasBuf);
	return status;
}

QDF_STATUS
csr_roam_get_wps_session_overlap(tpAniSirGlobal pMac, uint32_t sessionId,
				 void *pUsrContext, void *pfnSapEventCallback,
				 struct qdf_mac_addr pRemoveMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr bssId = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE,
			"csr_roam_get_wps_session_overlap:CSR Session not found");
		return status;
	}
	if (pSession->pConnectBssDesc) {
		qdf_mem_copy(bssId.bytes, pSession->pConnectBssDesc->bssId,
			     sizeof(struct qdf_mac_addr));
	} else {
		sms_log(pMac, LOGE,
			"csr_roam_get_wps_session_overlap:Connected BSS Description in CSR Session not found");
		return status;
	}
	sms_log(pMac, LOG2,
		"CSR getting WPS Session Overlap for Bssid = " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(bssId.bytes));

	status = csr_send_mb_get_wpspbc_sessions(pMac, sessionId, bssId,
				pUsrContext, pfnSapEventCallback, pRemoveMac);

	return status;
}

QDF_STATUS csr_roam_issue_deauth(tpAniSirGlobal pMac, uint32_t sessionId,
				 eCsrRoamSubState NewSubstate)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr bssId = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->pConnectBssDesc) {
		qdf_mem_copy(bssId.bytes, pSession->pConnectBssDesc->bssId,
			     sizeof(struct qdf_mac_addr));
	}
	sms_log(pMac, LOG2, "CSR Attempting to Deauth Bssid= " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(bssId.bytes));
	csr_roam_substate_change(pMac, NewSubstate, sessionId);

	status =
		csr_send_mb_deauth_req_msg(pMac, sessionId, bssId.bytes,
					   eSIR_MAC_DEAUTH_LEAVING_BSS_REASON);
	if (QDF_IS_STATUS_SUCCESS(status))
		csr_roam_link_down(pMac, sessionId);
	else {
		sms_log(pMac, LOGE,
			FL
				("csr_send_mb_deauth_req_msg failed with status %d Session ID: %d"
				MAC_ADDRESS_STR), status, sessionId,
			MAC_ADDR_ARRAY(bssId.bytes));
	}

	return status;
}

QDF_STATUS csr_roam_save_connected_bss_desc(tpAniSirGlobal pMac, uint32_t sessionId,
					    tSirBssDescription *pBssDesc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	uint32_t size;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	/* If no BSS description was found in this connection (happens with start IBSS), then */
	/* nix the BSS description that we keep around for the connected BSS) and get out... */
	if (NULL == pBssDesc) {
		csr_free_connect_bss_desc(pMac, sessionId);
	} else {
		size = pBssDesc->length + sizeof(pBssDesc->length);
		if (NULL != pSession->pConnectBssDesc) {
			if (((pSession->pConnectBssDesc->length) +
			     sizeof(pSession->pConnectBssDesc->length)) <
			    size) {
				/* not enough room for the new BSS, pMac->roam.pConnectBssDesc is freed inside */
				csr_free_connect_bss_desc(pMac, sessionId);
			}
		}
		if (NULL == pSession->pConnectBssDesc) {
			pSession->pConnectBssDesc = qdf_mem_malloc(size);
		}
		if (NULL == pSession->pConnectBssDesc)
			status = QDF_STATUS_E_NOMEM;
		else
			qdf_mem_copy(pSession->pConnectBssDesc, pBssDesc, size);
	}
	return status;
}

QDF_STATUS csr_roam_prepare_bss_config(tpAniSirGlobal pMac,
				       tCsrRoamProfile *pProfile,
				       tSirBssDescription *pBssDesc,
				       tBssConfigParam *pBssConfig,
				       tDot11fBeaconIEs *pIes)
{
	eCsrCfgDot11Mode cfgDot11Mode;
	QDF_ASSERT(pIes != NULL);
	if (pIes == NULL)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(&pBssConfig->BssCap, &pBssDesc->capabilityInfo,
		     sizeof(tSirMacCapabilityInfo));
	/* get qos */
	pBssConfig->qosType = csr_get_qo_s_from_bss_desc(pMac, pBssDesc, pIes);
	/* get SSID */
	if (pIes->SSID.present) {
		qdf_mem_copy(&pBssConfig->SSID.ssId, pIes->SSID.ssid,
			     pIes->SSID.num_ssid);
		pBssConfig->SSID.length = pIes->SSID.num_ssid;
	} else
		pBssConfig->SSID.length = 0;
	if (csr_is_nullssid(pBssConfig->SSID.ssId, pBssConfig->SSID.length)) {
		sms_log(pMac, LOGW, FL("BSS desc SSID is a wild card"));
		/* Return failed if profile doesn't have an SSID either. */
		if (pProfile->SSIDs.numOfSSIDs == 0) {
			sms_log(pMac, LOGW,
				FL("BSS desc and profile doesn't have SSID"));
			return QDF_STATUS_E_FAILURE;
		}
	}
	if (CDS_IS_CHANNEL_5GHZ(pBssDesc->channelId))
		pBssConfig->eBand = eCSR_BAND_5G;
	else
		pBssConfig->eBand = eCSR_BAND_24;
		/* phymode */
	if (csr_is_phy_mode_match(pMac, pProfile->phyMode, pBssDesc,
				  pProfile, &cfgDot11Mode, pIes)) {
		pBssConfig->uCfgDot11Mode = cfgDot11Mode;
	} else {
		sms_log(pMac, LOGW, "Can not find match phy mode");
		/* force it */
		if (eCSR_BAND_24 == pBssConfig->eBand)
			pBssConfig->uCfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
		else
			pBssConfig->uCfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
	}
	/* Qos */
	if ((pBssConfig->uCfgDot11Mode != eCSR_CFG_DOT11_MODE_11N) &&
	    (pMac->roam.configParam.WMMSupportMode == eCsrRoamWmmNoQos)) {
		/*
		 * Joining BSS is not 11n capable and WMM is disabled on client.
		 * Disable QoS and WMM
		 */
		pBssConfig->qosType = eCSR_MEDIUM_ACCESS_DCF;
	}

	if (((pBssConfig->uCfgDot11Mode == eCSR_CFG_DOT11_MODE_11N)
	    || (pBssConfig->uCfgDot11Mode == eCSR_CFG_DOT11_MODE_11AC))
		&& ((pBssConfig->qosType != eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP)
		    || (pBssConfig->qosType != eCSR_MEDIUM_ACCESS_11e_HCF)
		    || (pBssConfig->qosType != eCSR_MEDIUM_ACCESS_11e_eDCF))) {
		/* Joining BSS is 11n capable and WMM is disabled on AP. */
		/* Assume all HT AP's are QOS AP's and enable WMM */
		pBssConfig->qosType = eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP;
	}
	/* auth type */
	switch (pProfile->negotiatedAuthType) {
	default:
	case eCSR_AUTH_TYPE_WPA:
	case eCSR_AUTH_TYPE_WPA_PSK:
	case eCSR_AUTH_TYPE_WPA_NONE:
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		pBssConfig->authType = eSIR_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		pBssConfig->authType = eSIR_SHARED_KEY;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		pBssConfig->authType = eSIR_AUTO_SWITCH;
		break;
	}
	/* short slot time */
	if (eCSR_CFG_DOT11_MODE_11B != cfgDot11Mode)
		pBssConfig->uShortSlotTime =
			pMac->roam.configParam.shortSlotTime;
	else
		pBssConfig->uShortSlotTime = 0;

	if (pBssConfig->BssCap.ibss)
		/* We don't support 11h on IBSS */
		pBssConfig->f11hSupport = false;
	else
		pBssConfig->f11hSupport =
			pMac->roam.configParam.Is11hSupportEnabled;
	/* power constraint */
	pBssConfig->uPowerLimit =
		csr_get11h_power_constraint(pMac, &pIes->PowerConstraints);
	/* heartbeat */
	if (CSR_IS_11A_BSS(pBssDesc))
		pBssConfig->uHeartBeatThresh =
			pMac->roam.configParam.HeartbeatThresh50;
	else
		pBssConfig->uHeartBeatThresh =
			pMac->roam.configParam.HeartbeatThresh24;

	/*
	 * Join timeout: if we find a BeaconInterval in the BssDescription,
	 * then set the Join Timeout to be 10 x the BeaconInterval.
	 */
	if (pBssDesc->beaconInterval)
		/* Make sure it is bigger than the minimal */
		pBssConfig->uJoinTimeOut =
			QDF_MAX(10 * pBssDesc->beaconInterval,
				CSR_JOIN_FAILURE_TIMEOUT_MIN);
	else
		pBssConfig->uJoinTimeOut =
			CSR_JOIN_FAILURE_TIMEOUT_DEFAULT;
	/* validate CB */
	pBssConfig->cbMode = csr_get_cb_mode_from_ies(pMac, pBssDesc->channelId,
						      pIes);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_prepare_bss_config_from_profile(tpAniSirGlobal pMac,
							   tCsrRoamProfile *pProfile,
							   tBssConfigParam *
							   pBssConfig,
							   tSirBssDescription *
							   pBssDesc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t operationChannel = 0;
	uint8_t qAPisEnabled = false;
	/* SSID */
	pBssConfig->SSID.length = 0;
	if (pProfile->SSIDs.numOfSSIDs) {
		/* only use the first one */
		qdf_mem_copy(&pBssConfig->SSID,
			     &pProfile->SSIDs.SSIDList[0].SSID,
			     sizeof(tSirMacSSid));
	} else {
		/* SSID must present */
		return QDF_STATUS_E_FAILURE;
	}
	/* Settomg up the capabilities */
	if (csr_is_bss_type_ibss(pProfile->BSSType)) {
		pBssConfig->BssCap.ibss = 1;
	} else {
		pBssConfig->BssCap.ess = 1;
	}
	if (eCSR_ENCRYPT_TYPE_NONE !=
	    pProfile->EncryptionType.encryptionType[0]) {
		pBssConfig->BssCap.privacy = 1;
	}
	pBssConfig->eBand = pMac->roam.configParam.eBand;
	/* phymode */
	if (pProfile->ChannelInfo.ChannelList) {
		operationChannel = pProfile->ChannelInfo.ChannelList[0];
	}
	pBssConfig->uCfgDot11Mode =
		csr_roam_get_phy_mode_band_for_bss(pMac, pProfile, operationChannel,
						   &pBssConfig->eBand);
	/* QOS */
	/* Is this correct to always set to this // *** */
	if (pBssConfig->BssCap.ess == 1) {
		/*For Softap case enable WMM */
		if (CSR_IS_INFRA_AP(pProfile)
		    && (eCsrRoamWmmNoQos !=
			pMac->roam.configParam.WMMSupportMode)) {
			qAPisEnabled = true;
		} else
		if (csr_roam_get_qos_info_from_bss(pMac, pBssDesc) ==
		    QDF_STATUS_SUCCESS) {
			qAPisEnabled = true;
		} else {
			qAPisEnabled = false;
		}
	} else {
		qAPisEnabled = true;
	}
	if ((eCsrRoamWmmNoQos != pMac->roam.configParam.WMMSupportMode &&
	     qAPisEnabled) ||
	    ((eCSR_CFG_DOT11_MODE_11N == pBssConfig->uCfgDot11Mode &&
	      qAPisEnabled))) {
		pBssConfig->qosType = eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP;
	} else {
		pBssConfig->qosType = eCSR_MEDIUM_ACCESS_DCF;
	}

	/* auth type */
	/* Take the preferred Auth type. */
	switch (pProfile->AuthType.authType[0]) {
	default:
	case eCSR_AUTH_TYPE_WPA:
	case eCSR_AUTH_TYPE_WPA_PSK:
	case eCSR_AUTH_TYPE_WPA_NONE:
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		pBssConfig->authType = eSIR_OPEN_SYSTEM;
		break;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		pBssConfig->authType = eSIR_SHARED_KEY;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		pBssConfig->authType = eSIR_AUTO_SWITCH;
		break;
	}
	/* short slot time */
	if (WNI_CFG_PHY_MODE_11B != pBssConfig->uCfgDot11Mode) {
		pBssConfig->uShortSlotTime =
			pMac->roam.configParam.shortSlotTime;
	} else {
		pBssConfig->uShortSlotTime = 0;
	}
	/* power constraint. We don't support 11h on IBSS */
	pBssConfig->f11hSupport = false;
	pBssConfig->uPowerLimit = 0;
	/* heartbeat */
	if (eCSR_BAND_5G == pBssConfig->eBand) {
		pBssConfig->uHeartBeatThresh =
			pMac->roam.configParam.HeartbeatThresh50;
	} else {
		pBssConfig->uHeartBeatThresh =
			pMac->roam.configParam.HeartbeatThresh24;
	}
	/* Join timeout */
	pBssConfig->uJoinTimeOut = CSR_JOIN_FAILURE_TIMEOUT_DEFAULT;

	return status;
}

static QDF_STATUS csr_roam_get_qos_info_from_bss(tpAniSirGlobal pMac,
						 tSirBssDescription *pBssDesc)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tDot11fBeaconIEs *pIes = NULL;

	do {
		if (!QDF_IS_STATUS_SUCCESS(
			csr_get_parsed_bss_description_ies(
				pMac, pBssDesc, &pIes))) {
			/* err msg */
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "csr_roam_get_qos_info_from_bss() failed");
			break;
		}
		/* check if the AP is QAP & it supports APSD */
		if (CSR_IS_QOS_BSS(pIes)) {
			status = QDF_STATUS_SUCCESS;
		}
	} while (0);

	if (NULL != pIes) {
		qdf_mem_free(pIes);
	}

	return status;
}

void csr_set_cfg_privacy(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile,
			 bool fPrivacy)
{
	/*
	 * the only difference between this function and
	 * the csr_set_cfg_privacyFromProfile() is the setting of the privacy
	 * CFG based on the advertised privacy setting from the AP for WPA
	 * associations. See note below in this function...
	 */
	uint32_t PrivacyEnabled = 0, RsnEnabled = 0, WepDefaultKeyId = 0;
	uint32_t WepKeyLength = WNI_CFG_WEP_KEY_LENGTH_5;
	uint32_t Key0Length = 0, Key1Length = 0, Key2Length = 0, Key3Length = 0;

	/* Reserve for the biggest key */
	uint8_t Key0[WNI_CFG_WEP_DEFAULT_KEY_1_LEN];
	uint8_t Key1[WNI_CFG_WEP_DEFAULT_KEY_2_LEN];
	uint8_t Key2[WNI_CFG_WEP_DEFAULT_KEY_3_LEN];
	uint8_t Key3[WNI_CFG_WEP_DEFAULT_KEY_4_LEN];

	switch (pProfile->negotiatedUCEncryptionType) {
	case eCSR_ENCRYPT_TYPE_NONE:
		/* for NO encryption, turn off Privacy and Rsn. */
		PrivacyEnabled = 0;
		RsnEnabled = 0;
		/* clear out the WEP keys that may be hanging around. */
		Key0Length = 0;
		Key1Length = 0;
		Key2Length = 0;
		Key3Length = 0;
		break;

	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP40:

		/* Privacy is ON.  NO RSN for Wep40 static key. */
		PrivacyEnabled = 1;
		RsnEnabled = 0;
		/* Set the Wep default key ID. */
		WepDefaultKeyId = pProfile->Keys.defaultIndex;
		/* Wep key size if 5 bytes (40 bits). */
		WepKeyLength = WNI_CFG_WEP_KEY_LENGTH_5;
		/*
		 * set encryption keys in the CFG database or
		 * clear those that are not present in this profile.
		 */
		if (pProfile->Keys.KeyLength[0]) {
			qdf_mem_copy(Key0,
				pProfile->Keys.KeyMaterial[0],
				WNI_CFG_WEP_KEY_LENGTH_5);
			Key0Length = WNI_CFG_WEP_KEY_LENGTH_5;
		} else {
			Key0Length = 0;
		}

		if (pProfile->Keys.KeyLength[1]) {
			qdf_mem_copy(Key1,
				pProfile->Keys.KeyMaterial[1],
				WNI_CFG_WEP_KEY_LENGTH_5);
			Key1Length = WNI_CFG_WEP_KEY_LENGTH_5;
		} else {
			Key1Length = 0;
		}

		if (pProfile->Keys.KeyLength[2]) {
			qdf_mem_copy(Key2,
				pProfile->Keys.KeyMaterial[2],
				WNI_CFG_WEP_KEY_LENGTH_5);
			Key2Length = WNI_CFG_WEP_KEY_LENGTH_5;
		} else {
			Key2Length = 0;
		}

		if (pProfile->Keys.KeyLength[3]) {
			qdf_mem_copy(Key3,
				pProfile->Keys.KeyMaterial[3],
				WNI_CFG_WEP_KEY_LENGTH_5);
			Key3Length = WNI_CFG_WEP_KEY_LENGTH_5;
		} else {
			Key3Length = 0;
		}
		break;

	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
	case eCSR_ENCRYPT_TYPE_WEP104:

		/* Privacy is ON.  NO RSN for Wep40 static key. */
		PrivacyEnabled = 1;
		RsnEnabled = 0;
		/* Set the Wep default key ID. */
		WepDefaultKeyId = pProfile->Keys.defaultIndex;
		/* Wep key size if 13 bytes (104 bits). */
		WepKeyLength = WNI_CFG_WEP_KEY_LENGTH_13;
		/*
		 * set encryption keys in the CFG database or clear
		 * those that are not present in this profile.
		 */
		if (pProfile->Keys.KeyLength[0]) {
			qdf_mem_copy(Key0,
				pProfile->Keys.KeyMaterial[0],
				WNI_CFG_WEP_KEY_LENGTH_13);
			Key0Length = WNI_CFG_WEP_KEY_LENGTH_13;
		} else {
			Key0Length = 0;
		}

		if (pProfile->Keys.KeyLength[1]) {
			qdf_mem_copy(Key1,
				pProfile->Keys.KeyMaterial[1],
				WNI_CFG_WEP_KEY_LENGTH_13);
			Key1Length = WNI_CFG_WEP_KEY_LENGTH_13;
		} else {
			Key1Length = 0;
		}

		if (pProfile->Keys.KeyLength[2]) {
			qdf_mem_copy(Key2,
				pProfile->Keys.KeyMaterial[2],
				WNI_CFG_WEP_KEY_LENGTH_13);
			Key2Length = WNI_CFG_WEP_KEY_LENGTH_13;
		} else {
			Key2Length = 0;
		}

		if (pProfile->Keys.KeyLength[3]) {
			qdf_mem_copy(Key3,
				pProfile->Keys.KeyMaterial[3],
				WNI_CFG_WEP_KEY_LENGTH_13);
			Key3Length = WNI_CFG_WEP_KEY_LENGTH_13;
		} else {
			Key3Length = 0;
		}
		break;

	case eCSR_ENCRYPT_TYPE_TKIP:
	case eCSR_ENCRYPT_TYPE_AES:
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
#endif /* FEATURE_WLAN_WAPI */
		/*
		 * this is the only difference between this function and
		 * the csr_set_cfg_privacyFromProfile().
		 * (setting of the privacy CFG based on the advertised
		 *  privacy setting from AP for WPA/WAPI associations).
		 */
		PrivacyEnabled = (0 != fPrivacy);
		/* turn on RSN enabled for WPA associations */
		RsnEnabled = 1;
		/* clear static WEP keys that may be hanging around. */
		Key0Length = 0;
		Key1Length = 0;
		Key2Length = 0;
		Key3Length = 0;
		break;
	default:
		PrivacyEnabled = 0;
		RsnEnabled = 0;
		break;
	}

	cfg_set_int(pMac, WNI_CFG_PRIVACY_ENABLED, PrivacyEnabled);
	cfg_set_int(pMac, WNI_CFG_RSN_ENABLED, RsnEnabled);
	cfg_set_str(pMac, WNI_CFG_WEP_DEFAULT_KEY_1, Key0, Key0Length);
	cfg_set_str(pMac, WNI_CFG_WEP_DEFAULT_KEY_2, Key1, Key1Length);
	cfg_set_str(pMac, WNI_CFG_WEP_DEFAULT_KEY_3, Key2, Key2Length);
	cfg_set_str(pMac, WNI_CFG_WEP_DEFAULT_KEY_4, Key3, Key3Length);
	cfg_set_int(pMac, WNI_CFG_WEP_KEY_LENGTH, WepKeyLength);
	cfg_set_int(pMac, WNI_CFG_WEP_DEFAULT_KEYID, WepDefaultKeyId);
}

static void csr_set_cfg_ssid(tpAniSirGlobal pMac, tSirMacSSid *pSSID)
{
	uint32_t len = 0;
	if (pSSID->length <= WNI_CFG_SSID_LEN) {
		len = pSSID->length;
	}
	cfg_set_str(pMac, WNI_CFG_SSID, (uint8_t *) pSSID->ssId, len);
}

QDF_STATUS csr_set_qos_to_cfg(tpAniSirGlobal pMac, uint32_t sessionId,
			      eCsrMediaAccessType qosType)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t QoSEnabled;
	uint32_t WmeEnabled;
	/* set the CFG enable/disable variables based on the qosType being configured... */
	switch (qosType) {
	case eCSR_MEDIUM_ACCESS_WMM_eDCF_802dot1p:
		QoSEnabled = false;
		WmeEnabled = true;
		break;
	case eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP:
		QoSEnabled = false;
		WmeEnabled = true;
		break;
	case eCSR_MEDIUM_ACCESS_WMM_eDCF_NoClassify:
		QoSEnabled = false;
		WmeEnabled = true;
		break;
	case eCSR_MEDIUM_ACCESS_11e_eDCF:
		QoSEnabled = true;
		WmeEnabled = false;
		break;
	case eCSR_MEDIUM_ACCESS_11e_HCF:
		QoSEnabled = true;
		WmeEnabled = false;
		break;
	default:
	case eCSR_MEDIUM_ACCESS_DCF:
		QoSEnabled = false;
		WmeEnabled = false;
		break;
	}
	/* save the WMM setting for later use */
	pMac->roam.roamSession[sessionId].fWMMConnection = (bool) WmeEnabled;
	pMac->roam.roamSession[sessionId].fQOSConnection = (bool) QoSEnabled;
	return status;
}

static QDF_STATUS csr_get_rate_set(tpAniSirGlobal pMac,
				tCsrRoamProfile *pProfile, eCsrPhyMode phyMode,
				tSirBssDescription *pBssDesc,
				tDot11fBeaconIEs *pIes,
				tSirMacRateSet *pOpRateSet,
				tSirMacRateSet *pExRateSet)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int i;
	eCsrCfgDot11Mode cfgDot11Mode;
	uint8_t *pDstRate;
	uint16_t rateBitmap = 0;
	qdf_mem_set(pOpRateSet, sizeof(tSirMacRateSet), 0);
	qdf_mem_set(pExRateSet, sizeof(tSirMacRateSet), 0);
	QDF_ASSERT(pIes != NULL);

	if (NULL == pIes) {
		sms_log(pMac, LOGE, FL("failed to parse BssDesc"));
		return status;
	}

	csr_is_phy_mode_match(pMac, phyMode, pBssDesc, pProfile,
			      &cfgDot11Mode, pIes);
	/*
	 * Originally, we thought that for 11a networks, the 11a rates
	 * are always in the Operational Rate set & for 11b and 11g
	 * networks, the 11b rates appear in the Operational Rate set.
	 * Consequently, in either case, we would blindly put the rates
	 * we support into our Operational Rate set.
	 * (including the basic rates, which we've already verified are
	 * supported earlier in the roaming decision).
	 * However, it turns out that this is not always the case.
	 * Some AP's (e.g. D-Link DI-784) ram 11g rates into the
	 * Operational Rate set too.  Now, we're a little more careful.
	 */
	pDstRate = pOpRateSet->rate;
	if (pIes->SuppRates.present) {
		for (i = 0; i < pIes->SuppRates.num_rates; i++) {
			if (csr_rates_is_dot11_rate_supported(pMac,
				pIes->SuppRates.rates[i]) &&
				!csr_check_rate_bitmap(
					pIes->SuppRates.rates[i],
					rateBitmap)) {
				csr_add_rate_bitmap(pIes->SuppRates.
						    rates[i], &rateBitmap);
				*pDstRate++ = pIes->SuppRates.rates[i];
				pOpRateSet->numRates++;
			}
		}
	}
	/*
	 * If there are Extended Rates in the beacon, we will reflect the
	 * extended rates that we support in our Extended Operational Rate
	 * set.
	 */
	if (pIes->ExtSuppRates.present) {
		pDstRate = pExRateSet->rate;
		for (i = 0; i < pIes->ExtSuppRates.num_rates; i++) {
			if (csr_rates_is_dot11_rate_supported(pMac,
				pIes->ExtSuppRates.rates[i]) &&
				!csr_check_rate_bitmap(
					pIes->ExtSuppRates.rates[i],
					rateBitmap)) {
				*pDstRate++ = pIes->ExtSuppRates.rates[i];
				pExRateSet->numRates++;
			}
		}
	}
	if (pOpRateSet->numRates > 0 || pExRateSet->numRates > 0)
		status = QDF_STATUS_SUCCESS;
	return status;
}

static void csr_set_cfg_rate_set(tpAniSirGlobal pMac, eCsrPhyMode phyMode,
				 tCsrRoamProfile *pProfile,
				 tSirBssDescription *pBssDesc,
				 tDot11fBeaconIEs *pIes)
{
	int i;
	uint8_t *pDstRate;
	eCsrCfgDot11Mode cfgDot11Mode;
	uint8_t OperationalRates[CSR_DOT11_SUPPORTED_RATES_MAX];        /* leave enough room for the max number of rates */
	uint32_t OperationalRatesLength = 0;
	uint8_t ExtendedOperationalRates[CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX];       /* leave enough room for the max number of rates */
	uint32_t ExtendedOperationalRatesLength = 0;
	uint8_t MCSRateIdxSet[SIZE_OF_SUPPORTED_MCS_SET];
	uint32_t MCSRateLength = 0;
	QDF_ASSERT(pIes != NULL);
	if (NULL != pIes) {
		csr_is_phy_mode_match(pMac, phyMode, pBssDesc, pProfile,
				      &cfgDot11Mode, pIes);
		/* Originally, we thought that for 11a networks, the 11a rates are always */
		/* in the Operational Rate set & for 11b and 11g networks, the 11b rates */
		/* appear in the Operational Rate set.  Consequently, in either case, we */
		/* would blindly put the rates we support into our Operational Rate set */
		/* (including the basic rates, which we have already verified are */
		/* supported earlier in the roaming decision). */
		/* However, it turns out that this is not always the case.  Some AP's */
		/* (e.g. D-Link DI-784) ram 11g rates into the Operational Rate set, */
		/* too.  Now, we're a little more careful: */
		pDstRate = OperationalRates;
		if (pIes->SuppRates.present) {
			for (i = 0; i < pIes->SuppRates.num_rates; i++) {
				if (csr_rates_is_dot11_rate_supported
					    (pMac, pIes->SuppRates.rates[i])
				    && (OperationalRatesLength <
					CSR_DOT11_SUPPORTED_RATES_MAX)) {
					*pDstRate++ = pIes->SuppRates.rates[i];
					OperationalRatesLength++;
				}
			}
		}
		if (eCSR_CFG_DOT11_MODE_11G == cfgDot11Mode ||
		    eCSR_CFG_DOT11_MODE_11N == cfgDot11Mode ||
		    eCSR_CFG_DOT11_MODE_ABG == cfgDot11Mode) {
			/* If there are Extended Rates in the beacon, we will reflect those */
			/* extended rates that we support in out Extended Operational Rate */
			/* set: */
			pDstRate = ExtendedOperationalRates;
			if (pIes->ExtSuppRates.present) {
				for (i = 0; i < pIes->ExtSuppRates.num_rates;
				     i++) {
					if (csr_rates_is_dot11_rate_supported
						    (pMac, pIes->ExtSuppRates.rates[i])
					    && (ExtendedOperationalRatesLength <
						CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX)) {
						*pDstRate++ =
							pIes->ExtSuppRates.rates[i];
						ExtendedOperationalRatesLength++;
					}
				}
			}
		}
		/* Enable proprietary MAC features if peer node is Airgo node and STA */
		/* user wants to use them */
		/* For ANI network companions, we need to populate the proprietary rate */
		/* set with any proprietary rates we found in the beacon, only if user */
		/* allows them... */
			/* No proprietary modes... */
		/* Get MCS Rate */
		pDstRate = MCSRateIdxSet;
		if (pIes->HTCaps.present) {
			for (i = 0; i < VALID_MAX_MCS_INDEX; i++) {
				if ((unsigned int)pIes->HTCaps.
				    supportedMCSSet[0] & (1 << i)) {
					MCSRateLength++;
					*pDstRate++ = i;
				}
			}
		}
		/* Set the operational rate set CFG variables... */
		cfg_set_str(pMac, WNI_CFG_OPERATIONAL_RATE_SET,
				OperationalRates, OperationalRatesLength);
		cfg_set_str(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
				ExtendedOperationalRates,
				ExtendedOperationalRatesLength);
		cfg_set_str(pMac, WNI_CFG_CURRENT_MCS_SET, MCSRateIdxSet,
				MCSRateLength);
	} /* Parsing BSSDesc */
	else {
		sms_log(pMac, LOGE, FL("failed to parse BssDesc"));
	}
}

static void csr_set_cfg_rate_set_from_profile(tpAniSirGlobal pMac,
					      tCsrRoamProfile *pProfile)
{
	tSirMacRateSetIE DefaultSupportedRates11a = { SIR_MAC_RATESET_EID,
						      {8,
						       {SIR_MAC_RATE_6,
							SIR_MAC_RATE_9,
							SIR_MAC_RATE_12,
							SIR_MAC_RATE_18,
							SIR_MAC_RATE_24,
							SIR_MAC_RATE_36,
							SIR_MAC_RATE_48,
							SIR_MAC_RATE_54} } };
	tSirMacRateSetIE DefaultSupportedRates11b = { SIR_MAC_RATESET_EID,
						      {4,
						       {SIR_MAC_RATE_1,
							SIR_MAC_RATE_2,
							SIR_MAC_RATE_5_5,
							SIR_MAC_RATE_11} } };

	tSirMacPropRateSet DefaultSupportedPropRates = { 3,
							 {SIR_MAC_RATE_72,
							  SIR_MAC_RATE_96,
							  SIR_MAC_RATE_108} };
	eCsrCfgDot11Mode cfgDot11Mode;
	eCsrBand eBand;
	/* leave enough room for the max number of rates */
	uint8_t OperationalRates[CSR_DOT11_SUPPORTED_RATES_MAX];
	uint32_t OperationalRatesLength = 0;
	/* leave enough room for the max number of rates */
	uint8_t ExtendedOperationalRates[CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX];
	uint32_t ExtendedOperationalRatesLength = 0;
	/* leave enough room for the max number of proprietary rates */
	uint8_t ProprietaryOperationalRates[4];
	uint32_t ProprietaryOperationalRatesLength = 0;
	uint32_t PropRatesEnable = 0;
	uint8_t operationChannel = 0;
	if (pProfile->ChannelInfo.ChannelList) {
		operationChannel = pProfile->ChannelInfo.ChannelList[0];
	}
	cfgDot11Mode =
		csr_roam_get_phy_mode_band_for_bss(pMac, pProfile, operationChannel,
						   &eBand);
	/* For 11a networks, the 11a rates go into the Operational Rate set.  For 11b and 11g */
	/* networks, the 11b rates appear in the Operational Rate set.  In either case, */
	/* we can blindly put the rates we support into our Operational Rate set */
	/* (including the basic rates, which we have already verified are supported */
	/* earlier in the roaming decision). */
	if (eCSR_BAND_5G == eBand) {
		/* 11a rates into the Operational Rate Set. */
		OperationalRatesLength =
			DefaultSupportedRates11a.supportedRateSet.numRates *
			sizeof(*DefaultSupportedRates11a.supportedRateSet.rate);
		qdf_mem_copy(OperationalRates,
			     DefaultSupportedRates11a.supportedRateSet.rate,
			     OperationalRatesLength);

		/* Nothing in the Extended rate set. */
		ExtendedOperationalRatesLength = 0;
		/* populate proprietary rates if user allows them */
		if (pMac->roam.configParam.ProprietaryRatesEnabled) {
			ProprietaryOperationalRatesLength =
				DefaultSupportedPropRates.numPropRates *
				sizeof(*DefaultSupportedPropRates.propRate);
			qdf_mem_copy(ProprietaryOperationalRates,
				     DefaultSupportedPropRates.propRate,
				     ProprietaryOperationalRatesLength);
		} else {
			/* No proprietary modes */
			ProprietaryOperationalRatesLength = 0;
		}
	} else if (eCSR_CFG_DOT11_MODE_11B == cfgDot11Mode) {
		/* 11b rates into the Operational Rate Set. */
		OperationalRatesLength =
			DefaultSupportedRates11b.supportedRateSet.numRates *
			sizeof(*DefaultSupportedRates11b.supportedRateSet.rate);
		qdf_mem_copy(OperationalRates,
			     DefaultSupportedRates11b.supportedRateSet.rate,
			     OperationalRatesLength);
		/* Nothing in the Extended rate set. */
		ExtendedOperationalRatesLength = 0;
		/* No proprietary modes */
		ProprietaryOperationalRatesLength = 0;
	} else {
		/* 11G */

		/* 11b rates into the Operational Rate Set. */
		OperationalRatesLength =
			DefaultSupportedRates11b.supportedRateSet.numRates *
			sizeof(*DefaultSupportedRates11b.supportedRateSet.rate);
		qdf_mem_copy(OperationalRates,
			     DefaultSupportedRates11b.supportedRateSet.rate,
			     OperationalRatesLength);

		/* 11a rates go in the Extended rate set. */
		ExtendedOperationalRatesLength =
			DefaultSupportedRates11a.supportedRateSet.numRates *
			sizeof(*DefaultSupportedRates11a.supportedRateSet.rate);
		qdf_mem_copy(ExtendedOperationalRates,
			     DefaultSupportedRates11a.supportedRateSet.rate,
			     ExtendedOperationalRatesLength);

		/* populate proprietary rates if user allows them */
		if (pMac->roam.configParam.ProprietaryRatesEnabled) {
			ProprietaryOperationalRatesLength =
				DefaultSupportedPropRates.numPropRates *
				sizeof(*DefaultSupportedPropRates.propRate);
			qdf_mem_copy(ProprietaryOperationalRates,
				     DefaultSupportedPropRates.propRate,
				     ProprietaryOperationalRatesLength);
		} else {
			/* No proprietary modes */
			ProprietaryOperationalRatesLength = 0;
		}
	}
	/* set this to 1 if prop. rates need to be advertised in to the IBSS beacon and user wants to use them */
	if (ProprietaryOperationalRatesLength
	    && pMac->roam.configParam.ProprietaryRatesEnabled) {
		PropRatesEnable = 1;
	} else {
		PropRatesEnable = 0;
	}

	/* Set the operational rate set CFG variables... */
	cfg_set_str(pMac, WNI_CFG_OPERATIONAL_RATE_SET, OperationalRates,
			OperationalRatesLength);
	cfg_set_str(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
			ExtendedOperationalRates, ExtendedOperationalRatesLength);
	cfg_set_str(pMac, WNI_CFG_PROPRIETARY_OPERATIONAL_RATE_SET,
			ProprietaryOperationalRates,
			ProprietaryOperationalRatesLength);
}

void csr_roam_ccm_cfg_set_callback(tpAniSirGlobal pMac, int32_t result)
{
	tListElem *pEntry =
		csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	uint32_t sessionId;
	tSmeCmd *pCommand = NULL;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	tCsrRoamSession *pSession = NULL;
#endif
	if (NULL == pEntry) {
		sms_log(pMac, LOGW, "CFG_CNF with active list empty");
		return;
	}
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	sessionId = pCommand->sessionId;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	pSession = &pMac->roam.roamSession[sessionId];
	if (pSession->roam_synch_in_progress) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR3:csr_roam_cfg_set_callback");
	}
#endif

	if (CSR_IS_ROAM_JOINING(pMac, sessionId)
	    && CSR_IS_ROAM_SUBSTATE_CONFIG(pMac, sessionId)) {
		csr_roaming_state_config_cnf_processor(pMac, (uint32_t) result);
	}
}

/* This function is very dump. It is here because PE still need WNI_CFG_PHY_MODE */
uint32_t csr_roam_get_phy_mode_from_dot11_mode(eCsrCfgDot11Mode dot11Mode,
					       eCsrBand band)
{
	if (eCSR_CFG_DOT11_MODE_11B == dot11Mode) {
		return WNI_CFG_PHY_MODE_11B;
	} else {
		if (eCSR_BAND_24 == band)
			return WNI_CFG_PHY_MODE_11G;
	}
	return WNI_CFG_PHY_MODE_11A;
}

ePhyChanBondState csr_get_htcb_state_from_vhtcb_state(ePhyChanBondState aniCBMode)
{
	switch (aniCBMode) {
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
		return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED:
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
		return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_CENTERED_40MHZ_CENTERED:
	default:
		return PHY_SINGLE_CHANNEL_CENTERED;
	}
}

/* pIes may be NULL */
QDF_STATUS csr_roam_set_bss_config_cfg(tpAniSirGlobal pMac, uint32_t sessionId,
				       tCsrRoamProfile *pProfile,
				       tSirBssDescription *pBssDesc,
				       tBssConfigParam *pBssConfig,
				       tDot11fBeaconIEs *pIes, bool resetCountry)
{
	tSirRetStatus status;
	uint32_t cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
	uint8_t channel = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	/* Make sure we have the domain info for the BSS we try to connect to. */
	/* Do we need to worry about sequence for OSs that are not Windows?? */
	if (pBssDesc) {
		if ((QDF_SAP_MODE !=
			csr_get_session_persona(pMac, sessionId)) &&
			csr_learn_11dcountry_information(
					pMac, pBssDesc, pIes, true)) {
			csr_apply_country_information(pMac);
		}
		if ((csr_is11d_supported(pMac)) && pIes) {
			if (!pIes->Country.present) {
				csr_apply_channel_power_info_wrapper(pMac);
			} else {
				/* Let's also update the below to make sure we don't update CC while */
				/* connected to an AP which is advertising some CC */
				qdf_mem_copy(pMac->scan.currentCountryBssid.bytes,
					     pBssDesc->bssId,
					     sizeof(tSirMacAddr));
			}
		}
	}
	/* Qos */
	csr_set_qos_to_cfg(pMac, sessionId, pBssConfig->qosType);
	/* SSID */
	csr_set_cfg_ssid(pMac, &pBssConfig->SSID);

	/* Auth type */
	cfg_set_int(pMac, WNI_CFG_AUTHENTICATION_TYPE, pBssConfig->authType);
	/* encryption type */
	csr_set_cfg_privacy(pMac, pProfile, (bool) pBssConfig->BssCap.privacy);
	/* short slot time */
	cfg_set_int(pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED,
			pBssConfig->uShortSlotTime);
	/* 11d */
	cfg_set_int(pMac, WNI_CFG_11D_ENABLED,
			((pBssConfig->f11hSupport) ? pBssConfig->f11hSupport :
			 pProfile->ieee80211d));
	cfg_set_int(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT,
			pBssConfig->uPowerLimit);
	/* CB */

	if (CSR_IS_INFRA_AP(pProfile) || CSR_IS_IBSS(pProfile)) {
		channel = pProfile->operationChannel;
	} else {
		if (pBssDesc) {
			channel = pBssDesc->channelId;
		}
	}
	if (0 != channel) {
		if (CDS_IS_CHANNEL_24GHZ(channel)) {    /* for now if we are on 2.4 Ghz, CB will be always disabled */
			cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		} else {
			cfgCb = pBssConfig->cbMode;
		}
	}
	/* Rate */
	/* Fixed Rate */
	if (pBssDesc) {
		csr_set_cfg_rate_set(pMac, (eCsrPhyMode) pProfile->phyMode,
				     pProfile, pBssDesc, pIes);
	} else {
		csr_set_cfg_rate_set_from_profile(pMac, pProfile);
	}
	status = cfg_set_int(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT,
			pBssConfig->uJoinTimeOut);
	/* Any roaming related changes should be above this line */
	if (pSession && pSession->roam_synch_in_progress) {
		sms_log(pMac, LOG4, FL("Roam synch is in progress %d"),
			sessionId);
		return QDF_STATUS_SUCCESS;
	}
	/* Make this the last CFG to set. The callback will trigger a join_req */
	/* Join time out */
	csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_CONFIG, sessionId);

	csr_roam_ccm_cfg_set_callback(pMac, status);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_stop_network(tpAniSirGlobal pMac, uint32_t sessionId,
				 tCsrRoamProfile *pProfile,
				 tSirBssDescription *pBssDesc,
				 tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status;
	tBssConfigParam *pBssConfig;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	pBssConfig = qdf_mem_malloc(sizeof(tBssConfigParam));
	if (NULL == pBssConfig)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(pBssConfig, sizeof(tBssConfigParam), 0);
	status = csr_roam_prepare_bss_config(pMac, pProfile, pBssDesc,
			pBssConfig, pIes);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		eCsrRoamSubState substate;
		substate = eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING;
		pSession->bssParams.uCfgDot11Mode = pBssConfig->uCfgDot11Mode;
		/* This will allow to pass cbMode during join req */
		pSession->bssParams.cbMode = pBssConfig->cbMode;
		/* For IBSS, we need to prepare some more information */
		if (csr_is_bss_type_ibss(pProfile->BSSType) ||
				CSR_IS_INFRA_AP(pProfile))
			csr_roam_prepare_bss_params(pMac, sessionId, pProfile,
				pBssDesc, pBssConfig, pIes);

		/*
		 * If we are in an IBSS, then stop the IBSS...
		 * Not worry about WDS connection for now
		 */
		if (csr_is_conn_state_ibss(pMac, sessionId)) {
			status = csr_roam_issue_stop_bss(pMac, sessionId,
					substate);
		} else if (csr_is_conn_state_infra(pMac, sessionId)) {
			/*
			 * the new Bss is an Ibss OR we are roaming from
			 * Infra to Infra across SSIDs
			 * (roaming to a new SSID)...
			 * Not worry about WDS connection for now
			 */
			if (pBssDesc && (csr_is_ibss_bss_desc(pBssDesc) ||
				!csr_is_ssid_equal(pMac,
					pSession->pConnectBssDesc,
					pBssDesc, pIes)))
				status = csr_roam_issue_disassociate(pMac,
						sessionId, substate, false);
			else if (pBssDesc)
				/*
				 * In an infra & going to an infra network with
				 * the same SSID.  This calls for a reassoc seq.
				 * So issue the CFG sets for this new AP. Set
				 * parameters for this Bss.
				 */
				status = csr_roam_set_bss_config_cfg(pMac,
						sessionId, pProfile, pBssDesc,
						pBssConfig, pIes, false);
		} else if (pBssDesc ||
					CSR_IS_INFRA_AP(pProfile)) {
			/*
			 * Neither in IBSS nor in Infra. We can go ahead and set
			 * the cfg for tne new network... nothing to stop.
			 */
			bool is11rRoamingFlag = false;
			is11rRoamingFlag = csr_roam_is11r_assoc(pMac,
							sessionId);
			/* Set parameters for this Bss. */
			status = csr_roam_set_bss_config_cfg(pMac, sessionId,
					pProfile, pBssDesc, pBssConfig, pIes,
					is11rRoamingFlag);
		}
	} /* Success getting BSS config info */
	qdf_mem_free(pBssConfig);
	return status;
}

/**
 * csr_roam_state_for_same_profile() - Determine roam state for same profile
 * @mac_ctx: pointer to mac context
 * @profile: Roam profile
 * @session: Roam session
 * @session_id: session id
 * @ies_local: local ies
 * @bss_descr: bss description
 *
 * This function will determine the roam state for same profile
 *
 * Return: Roaming state.
 */
static eCsrJoinState csr_roam_state_for_same_profile(tpAniSirGlobal mac_ctx,
			tCsrRoamProfile *profile, tCsrRoamSession *session,
			uint32_t session_id, tDot11fBeaconIEs *ies_local,
			tSirBssDescription *bss_descr)
{
	QDF_STATUS status;
	tBssConfigParam bssConfig;
	if (csr_roam_is_same_profile_keys(mac_ctx, &session->connectedProfile,
				profile))
		return eCsrReassocToSelfNoCapChange;
	/* The key changes */
	qdf_mem_set(&bssConfig, sizeof(bssConfig), 0);
	status = csr_roam_prepare_bss_config(mac_ctx, profile, bss_descr,
				&bssConfig, ies_local);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		session->bssParams.uCfgDot11Mode =
				bssConfig.uCfgDot11Mode;
		session->bssParams.cbMode =
				bssConfig.cbMode;
		/* reapply the cfg including keys so reassoc happens. */
		status = csr_roam_set_bss_config_cfg(mac_ctx, session_id,
				profile, bss_descr, &bssConfig,
				ies_local, false);
		if (QDF_IS_STATUS_SUCCESS(status))
			return eCsrContinueRoaming;
	}

	return eCsrStopRoaming;

}

eCsrJoinState csr_roam_join(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrScanResultInfo *pScanResult,
			    tCsrRoamProfile *pProfile)
{
	eCsrJoinState eRoamState = eCsrContinueRoaming;
	tSirBssDescription *pBssDesc = &pScanResult->BssDescriptor;
	tDot11fBeaconIEs *pIesLocal = (tDot11fBeaconIEs *) (pScanResult->pvIes);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return eCsrStopRoaming;
	}

	if (!pIesLocal &&
		!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies(pMac,
				pBssDesc, &pIesLocal))) {
		sms_log(pMac, LOGE, FL("fail to parse IEs"));
		return eCsrStopRoaming;
	}
	if (csr_is_infra_bss_desc(pBssDesc)) {
		/*
		 * If we are connected in infra mode and the join bss descr is
		 * for the same BssID, then we are attempting to join the AP we
		 * are already connected with.  In that case, see if the Bss or
		 * sta capabilities have changed and handle the changes
		 * without disturbing the current association
		 */

		if (csr_is_conn_state_connected_infra(pMac, sessionId) &&
			csr_is_bss_id_equal(pMac,
				pBssDesc, pSession->pConnectBssDesc) &&
			csr_is_ssid_equal(pMac, pSession->pConnectBssDesc,
				pBssDesc, pIesLocal)) {
			/*
			 * Check to see if the Auth type has changed in the
			 * profile.  If so, we don't want to reassociate with
			 * authenticating first.  To force this, stop the
			 * current association (Disassociate) and then re 'Join'
			 * the AP, wihch will force an Authentication (with the
			 * new Auth type) followed by a new Association.
			 */
			if (csr_is_same_profile(pMac,
				&pSession->connectedProfile, pProfile)) {
				sms_log(pMac, LOGW,
					FL("detect same profile"));
				eRoamState =
					csr_roam_state_for_same_profile(pMac,
						pProfile, pSession, sessionId,
						pIesLocal, pBssDesc);
			} else if (!QDF_IS_STATUS_SUCCESS(
					csr_roam_issue_disassociate(pMac,
						sessionId,
						eCSR_ROAM_SUBSTATE_DISASSOC_REQ,
						false))) {
					sms_log(pMac, LOGE,
						FL("fail disassoc session %d"),
						sessionId);
					eRoamState = eCsrStopRoaming;
			}
		} else if (!QDF_IS_STATUS_SUCCESS(csr_roam_stop_network(pMac,
				sessionId, pProfile, pBssDesc, pIesLocal))) {
				/* we used to pre-auth here with open auth
				 * networks but that wasn't working so well.
				 * stop the existing network before attempting
				 * to join the new network... */
				eRoamState = eCsrStopRoaming;
		}
	} else if (!QDF_IS_STATUS_SUCCESS(csr_roam_stop_network(pMac, sessionId,
						pProfile, pBssDesc,
						pIesLocal))) {
			eRoamState = eCsrStopRoaming;
	}

	if (pIesLocal && !pScanResult->pvIes)
		qdf_mem_free(pIesLocal);
	return eRoamState;
}

QDF_STATUS csr_roam_should_roam(tpAniSirGlobal pMac, uint32_t sessionId,
				tSirBssDescription *pBssDesc, uint32_t roamId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo roamInfo;
	qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
	roamInfo.pBssDesc = pBssDesc;
	status =
		csr_roam_call_callback(pMac, sessionId, &roamInfo, roamId,
				       eCSR_ROAM_SHOULD_ROAM, eCSR_ROAM_RESULT_NONE);
	return status;
}

/* In case no matching BSS is found, use whatever default we can find */
static void csr_roam_assign_default_param(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	/* Need to get all negotiated types in place first */
	/* auth type */
	/* Take the preferred Auth type. */
	switch (pCommand->u.roamCmd.roamProfile.AuthType.authType[0]) {
	default:
	case eCSR_AUTH_TYPE_WPA:
	case eCSR_AUTH_TYPE_WPA_PSK:
	case eCSR_AUTH_TYPE_WPA_NONE:
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		pCommand->u.roamCmd.roamProfile.negotiatedAuthType =
			eCSR_AUTH_TYPE_OPEN_SYSTEM;
		break;

	case eCSR_AUTH_TYPE_SHARED_KEY:
		pCommand->u.roamCmd.roamProfile.negotiatedAuthType =
			eCSR_AUTH_TYPE_SHARED_KEY;
		break;

	case eCSR_AUTH_TYPE_AUTOSWITCH:
		pCommand->u.roamCmd.roamProfile.negotiatedAuthType =
			eCSR_AUTH_TYPE_AUTOSWITCH;
		break;
	}
	pCommand->u.roamCmd.roamProfile.negotiatedUCEncryptionType =
		pCommand->u.roamCmd.roamProfile.EncryptionType.encryptionType[0];
	/* In this case, the multicast encryption needs to follow the uncast ones. */
	pCommand->u.roamCmd.roamProfile.negotiatedMCEncryptionType =
		pCommand->u.roamCmd.roamProfile.EncryptionType.encryptionType[0];
}

static void csr_set_abort_roaming_command(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	switch (pCommand->u.roamCmd.roamReason) {
	case eCsrLostLink1:
		pCommand->u.roamCmd.roamReason = eCsrLostLink1Abort;
		break;
	case eCsrLostLink2:
		pCommand->u.roamCmd.roamReason = eCsrLostLink2Abort;
		break;
	case eCsrLostLink3:
		pCommand->u.roamCmd.roamReason = eCsrLostLink3Abort;
		break;
	default:
		sms_log(pMac, LOGE,
			FL(" aborting roaming reason %d not recognized"),
			pCommand->u.roamCmd.roamReason);
		break;
	}
}

/**
 * csr_roam_select_bss() - Handle join scenario based on profile
 * @mac_ctx:             Global MAC Context
 * @roam_bss_entry:      The next BSS to join
 * @csr_result_info:     Result of join
 * @csr_scan_result:     Global scan result
 * @session_id:          SME Session ID
 * @roam_id:             Roaming ID
 * @roam_state:          Current roaming state
 * @bss_list:            BSS List
 *
 * Return: true if the entire BSS list is done, false otherwise.
 */
static bool csr_roam_select_bss(tpAniSirGlobal mac_ctx,
		tListElem *roam_bss_entry, tCsrScanResultInfo **csr_result_info,
		tCsrScanResult **csr_scan_result, uint32_t session_id,
		uint32_t roam_id, eCsrJoinState *roam_state,
		tScanResultList *bss_list)
{
	uint8_t conc_channel = 0;
	bool status = false;
	tCsrScanResult *scan_result = NULL;
	tCsrScanResultInfo *result = NULL;

	while (roam_bss_entry) {
		scan_result = GET_BASE_ADDR(roam_bss_entry, tCsrScanResult,
				Link);
		/*
		 * If concurrency enabled take the
		 * concurrent connected channel first.
		 * Valid multichannel concurrent
		 * sessions exempted
		 */
		result = &scan_result->Result;
		if (cds_concurrent_open_sessions_running() &&
			!csr_is_valid_mc_concurrent_session(mac_ctx,
					session_id, &result->BssDescriptor)) {
			conc_channel = csr_get_concurrent_operation_channel(
					mac_ctx);
			sms_log(mac_ctx, LOG1, FL("csr Conc Channel = %d"),
				conc_channel);
			if ((conc_channel) && (conc_channel ==
				result->BssDescriptor.channelId)) {
				/*
				 * make this 0 because we do not want the below
				 * check to pass as we don't want to connect on
				 * other channel
				 */
				sms_log(mac_ctx, LOG1, FL("Conc chnl match=%d"),
					conc_channel);
				conc_channel = 0;
			}
		}

		/* Ok to roam this */
		if (!conc_channel &&
			QDF_IS_STATUS_SUCCESS(csr_roam_should_roam(mac_ctx,
				session_id, &result->BssDescriptor, roam_id))) {
			status = false;
			break;
		} else {
			*roam_state = eCsrStopRoamingDueToConcurrency;
			status = true;
		}
		roam_bss_entry = csr_ll_next(&bss_list->List, roam_bss_entry,
					LL_ACCESS_LOCK);
	}
	*csr_result_info = result;
	*csr_scan_result = scan_result;
	return status;
}

/**
 * csr_roam_join_handle_profile() - Handle join scenario based on profile
 * @mac_ctx:             Global MAC Context
 * @session_id:          SME Session ID
 * @cmd:                 Command
 * @roam_info_ptr:       Pointed to the roaming info for join
 * @roam_state:          Current roaming state
 * @result:              Result of join
 * @scan_result:         Global scan result
 *
 * Return: None
 */
static void csr_roam_join_handle_profile(tpAniSirGlobal mac_ctx,
		uint32_t session_id, tSmeCmd *cmd, tCsrRoamInfo *roam_info_ptr,
		eCsrJoinState *roam_state, tCsrScanResultInfo *result,
		tCsrScanResult *scan_result)
{
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	uint8_t acm_mask = 0;
#endif
	QDF_STATUS status;
	tCsrRoamSession *session;
	tCsrRoamProfile *profile = &cmd->u.roamCmd.roamProfile;
	tDot11fBeaconIEs *ies_local = NULL;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE, FL("Invalid session id %d"), session_id);
		return;
	}
	session = CSR_GET_SESSION(mac_ctx, session_id);

	/*
	 * We have something to roam, tell HDD when it is infra.
	 * For IBSS, the indication goes back to HDD via eCSR_ROAM_IBSS_IND
	 */
	if (CSR_IS_INFRASTRUCTURE(profile) && roam_info_ptr) {
		if (session->bRefAssocStartCnt) {
			session->bRefAssocStartCnt--;
			roam_info_ptr->pProfile = profile;
			/*
			 * Complete the last assoc attempt as a
			 * new one is about to be tried
			 */
			csr_roam_call_callback(mac_ctx, session_id,
				roam_info_ptr, cmd->u.roamCmd.roamId,
				eCSR_ROAM_ASSOCIATION_COMPLETION,
				eCSR_ROAM_RESULT_NOT_ASSOCIATED);
		}
		/* If roaming has stopped, don't continue the roaming command */
		if (!CSR_IS_ROAMING(session) && CSR_IS_ROAMING_COMMAND(cmd)) {
			/* No need to complete roaming as it already complete */
			sms_log(mac_ctx, LOGW,
				FL(
				"Roam cmd(reason %d)aborted as roam complete"),
				cmd->u.roamCmd.roamReason);
			*roam_state = eCsrStopRoaming;
			csr_set_abort_roaming_command(mac_ctx, cmd);
			return;
		}
		qdf_mem_set(roam_info_ptr, sizeof(tCsrRoamInfo), 0);
		if (!scan_result)
			cmd->u.roamCmd.roamProfile.uapsd_mask = 0;
		else
			ies_local = scan_result->Result.pvIes;

		if (!result) {
			sms_log(mac_ctx, LOGE, FL(" cannot parse IEs"));
			*roam_state = eCsrStopRoaming;
			return;
		} else if (scan_result && !ies_local &&
				(!QDF_IS_STATUS_SUCCESS(
					csr_get_parsed_bss_description_ies(
						mac_ctx, &result->BssDescriptor,
						&ies_local)))) {
			sms_log(mac_ctx, LOGE, FL(" cannot parse IEs"));
			*roam_state = eCsrStopRoaming;
			return;
		}
		roam_info_ptr->pBssDesc = &result->BssDescriptor;
		cmd->u.roamCmd.pLastRoamBss = roam_info_ptr->pBssDesc;
		/* dont put uapsd_mask if BSS doesn't support uAPSD */
		if (scan_result && cmd->u.roamCmd.roamProfile.uapsd_mask
				&& CSR_IS_QOS_BSS(ies_local)
				&& CSR_IS_UAPSD_BSS(ies_local)) {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
			acm_mask = sme_qos_get_acm_mask(mac_ctx,
					&result->BssDescriptor, ies_local);
#endif /* WLAN_MDM_CODE_REDUCTION_OPT */
		} else {
			cmd->u.roamCmd.roamProfile.uapsd_mask = 0;
		}
		if (ies_local && !result->pvIes)
			qdf_mem_free(ies_local);
		roam_info_ptr->pProfile = profile;
		session->bRefAssocStartCnt++;
		csr_roam_call_callback(mac_ctx, session_id, roam_info_ptr,
			cmd->u.roamCmd.roamId, eCSR_ROAM_ASSOCIATION_START,
			eCSR_ROAM_RESULT_NONE);
	}
	if (NULL != cmd->u.roamCmd.pRoamBssEntry) {
		/*
		 * We have BSS
		 * Need to assign these value because
		 * they are used in csr_is_same_profile
		 */
		scan_result = GET_BASE_ADDR(cmd->u.roamCmd.pRoamBssEntry,
				tCsrScanResult, Link);
		/*
		 * The OSEN IE doesn't provide the cipher suite.Therefore set
		 * to constant value of AES
		 */
		if (cmd->u.roamCmd.roamProfile.bOSENAssociation) {
			cmd->u.roamCmd.roamProfile.negotiatedUCEncryptionType =
				eCSR_ENCRYPT_TYPE_AES;
			cmd->u.roamCmd.roamProfile.negotiatedMCEncryptionType =
				eCSR_ENCRYPT_TYPE_AES;
		} else {
			/* Negotiated while building scan result. */
			cmd->u.roamCmd.roamProfile.negotiatedUCEncryptionType =
				scan_result->ucEncryptionType;
			cmd->u.roamCmd.roamProfile.negotiatedMCEncryptionType =
				scan_result->mcEncryptionType;
		}
		cmd->u.roamCmd.roamProfile.negotiatedAuthType =
			scan_result->authType;
		if (CSR_IS_START_IBSS(&cmd->u.roamCmd.roamProfile)) {
			if (csr_is_same_profile(mac_ctx,
				&session->connectedProfile, profile)) {
				*roam_state = eCsrStartIbssSameIbss;
				return;
			}
		}
		if (cmd->u.roamCmd.fReassocToSelfNoCapChange) {
			/* trying to connect to the one already connected */
			cmd->u.roamCmd.fReassocToSelfNoCapChange = false;
			*roam_state = eCsrReassocToSelfNoCapChange;
			return;
		}
		/* Attempt to Join this Bss... */
		*roam_state = csr_roam_join(mac_ctx, session_id,
				&scan_result->Result, profile);
		return;
	}

	/* For an IBSS profile, then we need to start the IBSS. */
	if (CSR_IS_START_IBSS(profile)) {
		bool same_ibss = false;
		/* Attempt to start this IBSS... */
		csr_roam_assign_default_param(mac_ctx, cmd);
		status = csr_roam_start_ibss(mac_ctx, session_id,
				profile, &same_ibss);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			if (same_ibss)
				*roam_state = eCsrStartIbssSameIbss;
			else
				*roam_state = eCsrContinueRoaming;
		} else {
			/* it somehow fail need to stop */
			*roam_state = eCsrStopRoaming;
		}
		return;
	} else if (CSR_IS_INFRA_AP(profile)) {
		/* Attempt to start this WDS... */
		csr_roam_assign_default_param(mac_ctx, cmd);
		/* For AP WDS, we dont have any BSSDescription */
		status = csr_roam_start_wds(mac_ctx, session_id, profile, NULL);
		if (QDF_IS_STATUS_SUCCESS(status))
			*roam_state = eCsrContinueRoaming;
		else
			*roam_state = eCsrStopRoaming;
	} else if (CSR_IS_NDI(profile)) {
		csr_roam_assign_default_param(mac_ctx, cmd);
		status = csr_roam_start_ndi(mac_ctx, session_id, profile);
		if (QDF_IS_STATUS_SUCCESS(status))
			*roam_state = eCsrContinueRoaming;
		else
			*roam_state = eCsrStopRoaming;
	} else {
		/* Nothing we can do */
		sms_log(mac_ctx, LOGW, FL("cannot continue without BSS list"));
		*roam_state = eCsrStopRoaming;
		return;
	}

}
/**
 * csr_roam_join_next_bss() - Pick the next BSS for join
 * @mac_ctx:             Global MAC Context
 * @cmd:                 Command
 * @use_same_bss:        Use Same BSS to Join
 *
 * Return: The Join State
 */
static eCsrJoinState csr_roam_join_next_bss(tpAniSirGlobal mac_ctx,
		tSmeCmd *cmd, bool use_same_bss)
{
	tCsrScanResult *scan_result = NULL;
	eCsrJoinState roam_state = eCsrStopRoaming;
	tScanResultList *bss_list =
		(tScanResultList *) cmd->u.roamCmd.hBSSList;
	bool done = false;
	tCsrRoamInfo roam_info, *roam_info_ptr = NULL;
	uint32_t session_id = cmd->sessionId;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	tCsrRoamProfile *profile = &cmd->u.roamCmd.roamProfile;
	tCsrRoamJoinStatus *join_status;
	tCsrScanResultInfo *result = NULL;

	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), session_id);
		return eCsrStopRoaming;
	}

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	qdf_mem_copy(&roam_info.bssid, &session->joinFailStatusCode.bssId,
			sizeof(tSirMacAddr));
	/*
	 * When handling AP's capability change, continue to associate
	 * to same BSS and make sure pRoamBssEntry is not Null.
	 */
	if ((NULL != bss_list) &&
		((false == use_same_bss) ||
		 (cmd->u.roamCmd.pRoamBssEntry == NULL))) {
		if (cmd->u.roamCmd.pRoamBssEntry == NULL) {
			/* Try the first BSS */
			cmd->u.roamCmd.pLastRoamBss = NULL;
			cmd->u.roamCmd.pRoamBssEntry =
				csr_ll_peek_head(&bss_list->List,
					LL_ACCESS_LOCK);
		} else {
			cmd->u.roamCmd.pRoamBssEntry =
				csr_ll_next(&bss_list->List,
					cmd->u.roamCmd.pRoamBssEntry,
					LL_ACCESS_LOCK);
			/*
			 * Done with all the BSSs.
			 * In this case, will tell HDD the
			 * completion
			 */
			if (NULL == cmd->u.roamCmd.pRoamBssEntry)
				goto end;
			/*
			 * We need to indicate to HDD that we
			 * are done with this one.
			 */
			roam_info.pBssDesc = cmd->u.roamCmd.pLastRoamBss;
			join_status = &session->joinFailStatusCode;
			roam_info.statusCode = join_status->statusCode;
			roam_info.reasonCode = join_status->reasonCode;
			roam_info_ptr = &roam_info;
		}
		done = csr_roam_select_bss(mac_ctx,
				cmd->u.roamCmd.pRoamBssEntry, &result,
				&scan_result, session_id, cmd->u.roamCmd.roamId,
				&roam_state, bss_list);
		if (done)
			goto end;
	}
	if (!roam_info_ptr)
		roam_info_ptr = &roam_info;
	roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;

	csr_roam_join_handle_profile(mac_ctx, session_id, cmd, roam_info_ptr,
		&roam_state, result, scan_result);
end:
	if ((eCsrStopRoaming == roam_state) && CSR_IS_INFRASTRUCTURE(profile) &&
		(session->bRefAssocStartCnt > 0)) {
		/*
		 * Need to indicate association_completion if association_start
		 * has been done
		 */
		session->bRefAssocStartCnt--;
		/*
		 * Complete the last assoc attempte as a
		 * new one is about to be tried
		 */
		roam_info_ptr = &roam_info;
		roam_info_ptr->pProfile = profile;
		csr_roam_call_callback(mac_ctx, session_id,
			roam_info_ptr, cmd->u.roamCmd.roamId,
			eCSR_ROAM_ASSOCIATION_COMPLETION,
			eCSR_ROAM_RESULT_NOT_ASSOCIATED);
	}

	return roam_state;
}

static QDF_STATUS csr_roam(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	eCsrJoinState RoamState;
	eCsrRoamSubState substate;
	uint32_t sessionId = pCommand->sessionId;

	/* Attept to join a Bss... */
	RoamState = csr_roam_join_next_bss(pMac, pCommand, false);

	/* if nothing to join.. */
	if ((eCsrStopRoaming == RoamState) ||
		(eCsrStopRoamingDueToConcurrency == RoamState)) {
		bool fComplete = false;
		/* and if connected in Infrastructure mode... */
		if (csr_is_conn_state_infra(pMac, sessionId)) {
			/* ... then we need to issue a disassociation */
			substate = eCSR_ROAM_SUBSTATE_DISASSOC_NOTHING_TO_JOIN;
			status = csr_roam_issue_disassociate(pMac, sessionId,
					substate, false);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGW,
					FL("fail issuing disassoc status = %d"),
					status);
				/*
				 * roam command is completed by caller in the
				 * failed case
				 */
				fComplete = true;
			}
		} else if (csr_is_conn_state_ibss(pMac, sessionId)) {
			status = csr_roam_issue_stop_bss(pMac, sessionId,
					eCSR_ROAM_SUBSTATE_STOP_BSS_REQ);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGW,
					FL("fail issuing stop bss status = %d"),
					status);
				/*
				 * roam command is completed by caller in the
				 * failed case
				 */
				fComplete = true;
			}
		} else if (csr_is_conn_state_connected_infra_ap(pMac,
					sessionId)) {
			substate = eCSR_ROAM_SUBSTATE_STOP_BSS_REQ;
			status = csr_roam_issue_stop_bss(pMac, sessionId,
						substate);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGW,
					FL("fail issuing stop bss status = %d"),
					status);
				/*
				 * roam command is completed by caller in the
				 * failed case
				 */
				fComplete = true;
			}
		} else {
			fComplete = true;
		}

		if (fComplete) {
			/* otherwise, we can complete the Roam command here. */
			if (eCsrStopRoamingDueToConcurrency == RoamState)
				csr_roam_complete(pMac,
					eCsrJoinFailureDueToConcurrency, NULL);
			else
				csr_roam_complete(pMac,
					eCsrNothingToJoin, NULL);
		}
	} else if (eCsrReassocToSelfNoCapChange == RoamState) {
		csr_roam_complete(pMac, eCsrSilentlyStopRoamingSaveState,
				NULL);
	} else if (eCsrStartIbssSameIbss == RoamState) {
		csr_roam_complete(pMac, eCsrSilentlyStopRoaming, NULL);
	}

	return status;
}

QDF_STATUS csr_process_ft_reassoc_roam_command(tpAniSirGlobal pMac,
					       tSmeCmd *pCommand)
{
	uint32_t sessionId;
	tCsrRoamSession *pSession;
	tCsrScanResult *pScanResult = NULL;
	tSirBssDescription *pBssDesc = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	sessionId = pCommand->sessionId;
	pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (CSR_IS_ROAMING(pSession) && pSession->fCancelRoaming) {
		/* the roaming is cancelled. Simply complete the command */
		sms_log(pMac, LOG1, FL("Roam command canceled"));
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
		return QDF_STATUS_E_FAILURE;
	}
	if (pCommand->u.roamCmd.pRoamBssEntry) {
		pScanResult =
			GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry,
				      tCsrScanResult, Link);
		pBssDesc = &pScanResult->Result.BssDescriptor;
	} else {
		/* the roaming is cancelled. Simply complete the command */
		sms_log(pMac, LOG1, FL("Roam command canceled"));
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
		return QDF_STATUS_E_FAILURE;
	}
	status = csr_roam_issue_reassociate(pMac, sessionId, pBssDesc,
					    (tDot11fBeaconIEs *) (pScanResult->
								  Result.pvIes),
					    &pCommand->u.roamCmd.roamProfile);
	return status;
}

/**
 * csr_roam_trigger_reassociate() - Helper function to trigger reassociate
 * @mac_ctx: pointer to mac context
 * @cmd: sme command
 * @roam_info: Roaming infor structure
 * @session_ptr: session pointer
 * @session_id: session id
 *
 * This function will trigger reassociate.
 *
 * Return: QDF_STATUS for success or failure.
 */
static QDF_STATUS csr_roam_trigger_reassociate(tpAniSirGlobal mac_ctx,
			tSmeCmd *cmd, tCsrRoamInfo *roam_info,
			tCsrRoamSession *session_ptr, uint32_t session_id)
{
	tDot11fBeaconIEs *pIes = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (session_ptr->pConnectBssDesc) {
		status = csr_get_parsed_bss_description_ies(mac_ctx,
				session_ptr->pConnectBssDesc, &pIes);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(mac_ctx, LOGE, FL("fail to parse IEs"));
		} else {
			roam_info->reasonCode =
					eCsrRoamReasonStaCapabilityChanged;
			csr_roam_call_callback(mac_ctx, session_ptr->sessionId,
					roam_info, 0, eCSR_ROAM_ROAMING_START,
					eCSR_ROAM_RESULT_NONE);
			session_ptr->roamingReason = eCsrReassocRoaming;
			roam_info->pBssDesc = session_ptr->pConnectBssDesc;
			roam_info->pProfile = &cmd->u.roamCmd.roamProfile;
			session_ptr->bRefAssocStartCnt++;
			csr_roam_call_callback(mac_ctx, session_id, roam_info,
				cmd->u.roamCmd.roamId,
				eCSR_ROAM_ASSOCIATION_START,
				eCSR_ROAM_RESULT_NONE);

			sms_log(mac_ctx, LOG1,
				FL("calling csr_roam_issue_reassociate"));
			status = csr_roam_issue_reassociate(mac_ctx, session_id,
					session_ptr->pConnectBssDesc, pIes,
					&cmd->u.roamCmd.roamProfile);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(mac_ctx, LOGE, FL("failed status %d"),
					status);
				csr_release_command_roam(mac_ctx, cmd);
			}

			qdf_mem_free(pIes);
			pIes = NULL;
		}
	} else {
		sms_log(mac_ctx, LOGE, FL
			("reassoc to same AP failed as connected BSS is NULL"));
		status = QDF_STATUS_E_FAILURE;
	}
	return status;
}

QDF_STATUS csr_roam_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo roamInfo;
	uint32_t sessionId = pCommand->sessionId;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	sms_log(pMac, LOG1, FL("Roam Reason : %d, sessionId: %d"),
		pCommand->u.roamCmd.roamReason, sessionId);

	switch (pCommand->u.roamCmd.roamReason) {
	case eCsrForcedDisassoc:
		if (eCSR_ROAMING_STATE_IDLE == pMac->roam.curState[sessionId]) {
			sms_log(pMac, LOGE,
				FL("Ignore eCsrForcedDisassoc cmd on roam state"
				   " %d"), eCSR_ROAMING_STATE_IDLE);
			return QDF_STATUS_E_FAILURE;
		}

		status = csr_roam_process_disassoc_deauth(pMac, pCommand,
				true, false);
		csr_free_roam_profile(pMac, sessionId);
		break;
	case eCsrSmeIssuedDisassocForHandoff:
		/* Not to free pMac->roam.pCurRoamProfile (via
		 * csr_free_roam_profile) because its needed after disconnect */
		status = csr_roam_process_disassoc_deauth(pMac, pCommand,
				true, false);

		break;
	case eCsrForcedDisassocMICFailure:
		status = csr_roam_process_disassoc_deauth(pMac, pCommand,
				true, true);
		csr_free_roam_profile(pMac, sessionId);
		break;
	case eCsrForcedDeauth:
		status = csr_roam_process_disassoc_deauth(pMac, pCommand,
				false, false);
		csr_free_roam_profile(pMac, sessionId);
		break;
	case eCsrHddIssuedReassocToSameAP:
	case eCsrSmeIssuedReassocToSameAP:
		status = csr_roam_trigger_reassociate(pMac, pCommand, &roamInfo,
				pSession, sessionId);
		break;
	case eCsrCapsChange:
		sms_log(pMac, LOGE, FL("received eCsrCapsChange "));
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING,
				sessionId);
		status = csr_roam_issue_disassociate(pMac, sessionId,
				eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING,
				false);
		break;
	case eCsrSmeIssuedFTReassoc:
		sms_log(pMac, LOG1, FL("received FT Reassoc Req "));
		status = csr_process_ft_reassoc_roam_command(pMac, pCommand);
		break;

	case eCsrStopBss:
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING,
				sessionId);
		status = csr_roam_issue_stop_bss(pMac, sessionId,
				eCSR_ROAM_SUBSTATE_STOP_BSS_REQ);
		break;

	case eCsrForcedDisassocSta:
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING,
				sessionId);
		csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_DISASSOC_REQ,
				sessionId);
		status = csr_send_mb_disassoc_req_msg(pMac, sessionId,
				pCommand->u.roamCmd.peerMac,
				pCommand->u.roamCmd.reason);
		break;

	case eCsrForcedDeauthSta:
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING,
				sessionId);
		csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_DEAUTH_REQ,
				sessionId);
		status = csr_send_mb_deauth_req_msg(pMac, sessionId,
				pCommand->u.roamCmd.peerMac,
				pCommand->u.roamCmd.reason);
		break;

	case eCsrPerformPreauth:
		sms_log(pMac, LOG1, FL("Attempting FT PreAuth Req"));
		status = csr_roam_issue_ft_preauth_req(pMac, sessionId,
				pCommand->u.roamCmd.pLastRoamBss);
		break;
	default:
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING,
				sessionId);

		if (pCommand->u.roamCmd.fUpdateCurRoamProfile) {
			/* Remember the roaming profile */
			csr_free_roam_profile(pMac, sessionId);
			pSession->pCurRoamProfile =
					qdf_mem_malloc(sizeof(tCsrRoamProfile));
			if (NULL != pSession->pCurRoamProfile) {
				qdf_mem_set(pSession->pCurRoamProfile,
					sizeof(tCsrRoamProfile), 0);
				csr_roam_copy_profile(pMac,
					pSession->pCurRoamProfile,
					&pCommand->u.roamCmd.roamProfile);
			}
		}
		/*
		 * At this point original uapsd_mask is saved in
		 * pCurRoamProfile. uapsd_mask in the pCommand may change from
		 * this point on. Attempt to roam with the new scan results
		 * (if we need to..)
		 */
		status = csr_roam(pMac, pCommand);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sms_log(pMac, LOGW,
				FL("csr_roam() failed with status = 0x%08X"),
				status);
		break;
	}
	return status;
}

void csr_reinit_roam_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	if (pCommand->u.roamCmd.fReleaseBssList) {
		csr_scan_result_purge(pMac, pCommand->u.roamCmd.hBSSList);
		pCommand->u.roamCmd.fReleaseBssList = false;
		pCommand->u.roamCmd.hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
	}
	if (pCommand->u.roamCmd.fReleaseProfile) {
		csr_release_profile(pMac, &pCommand->u.roamCmd.roamProfile);
		pCommand->u.roamCmd.fReleaseProfile = false;
	}
	pCommand->u.roamCmd.pRoamBssEntry = NULL;
	/* Because u.roamCmd is union and share with scanCmd and StatusChange */
	qdf_mem_set(&pCommand->u.roamCmd, sizeof(tRoamCmd), 0);
}

void csr_reinit_wm_status_change_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	qdf_mem_set(&pCommand->u.wmStatusChangeCmd, sizeof(tWmStatusChangeCmd),
		    0);
}

void csr_roam_complete(tpAniSirGlobal pMac, eCsrRoamCompleteResult Result,
		       void *Context)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;
	bool fReleaseCommand = true;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Roam Completion ...", __func__);
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		/* If the head of the queue is Active and it is a ROAM command, remove */
		/* and put this on the Free queue. */
		if (eSmeCommandRoam == pCommand->command) {
			/* we need to process the result first before removing it from active list because state changes */
			/* still happening insides roamQProcessRoamResults so no other roam command should be issued */
			fReleaseCommand =
				csr_roam_process_results(pMac, pCommand, Result,
							 Context);
			if (fReleaseCommand) {
				if (csr_ll_remove_entry
					    (&pMac->sme.smeCmdActiveList, pEntry,
					    LL_ACCESS_LOCK)) {
					csr_release_command_roam(pMac, pCommand);
				} else {
					sms_log(pMac, LOGE,
						" **********csr_roam_complete fail to release command reason %d",
						pCommand->u.roamCmd.roamReason);
				}
			} else {
				sms_log(pMac, LOGE,
					" **********csr_roam_complete fail to release command reason %d",
					pCommand->u.roamCmd.roamReason);
			}
		} else {
			sms_log(pMac, LOGW,
				"CSR: Roam Completion called but ROAM command is not ACTIVE ...");
		}
	} else {
		sms_log(pMac, LOGW,
			"CSR: Roam Completion called but NO commands are ACTIVE ...");
	}
	if (fReleaseCommand) {
		sme_process_pending_queue(pMac);
	}
}

void csr_reset_pmkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}
	qdf_mem_set(&(pSession->PmkidCandidateInfo[0]),
		    sizeof(tPmkidCandidateInfo) * CSR_MAX_PMKID_ALLOWED, 0);
	pSession->NumPmkidCandidate = 0;
}

#ifdef FEATURE_WLAN_WAPI
void csr_reset_bkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}
	qdf_mem_set(&(pSession->BkidCandidateInfo[0]),
		    sizeof(tBkidCandidateInfo) * CSR_MAX_BKID_ALLOWED, 0);
	pSession->NumBkidCandidate = 0;
}
#endif /* FEATURE_WLAN_WAPI */
extern uint8_t csr_wpa_oui[][CSR_WPA_OUI_SIZE];

/**
 * csr_roam_save_params() - Helper function to save params
 * @mac_ctx: pointer to mac context
 * @session_ptr: Session pointer
 * @auth_type: auth type
 * @ie_ptr: pointer to ie
 * @ie_local: pointr to local ie
 *
 * This function will save params to session
 *
 * Return: none.
 */
static QDF_STATUS csr_roam_save_params(tpAniSirGlobal mac_ctx,
				tCsrRoamSession *session_ptr,
				eCsrAuthType auth_type,
				tDot11fBeaconIEs *ie_ptr,
				tDot11fBeaconIEs *ie_local)
{
	uint32_t nIeLen;
	uint8_t *pIeBuf;

	if ((eCSR_AUTH_TYPE_RSN == auth_type) ||
		(eCSR_AUTH_TYPE_FT_RSN == auth_type) ||
		(eCSR_AUTH_TYPE_FT_RSN_PSK == auth_type) ||
#if defined WLAN_FEATURE_11W
		(eCSR_AUTH_TYPE_RSN_PSK_SHA256 == auth_type) ||
		(eCSR_AUTH_TYPE_RSN_8021X_SHA256 == auth_type) ||
#endif
		(eCSR_AUTH_TYPE_RSN_PSK == auth_type)) {
		if (ie_local->RSN.present) {
			tDot11fIERSN *rsnie = &ie_local->RSN;
			/*
			 * Calculate the actual length
			 * version + gp_cipher_suite + pwise_cipher_suite_count
			 * + akm_suite_count + reserved + pwise_cipher_suites
			 */
			nIeLen = 8 + 2 + 2
				+ (rsnie->pwise_cipher_suite_count * 4)
				+ (rsnie->akm_suite_count * 4);
			if (rsnie->pmkid_count)
				/* pmkid */
				nIeLen += 2 + rsnie->pmkid_count * 4;

			/* nIeLen doesn't count EID and length fields */
			session_ptr->pWpaRsnRspIE = qdf_mem_malloc(nIeLen + 2);
			if (NULL == session_ptr->pWpaRsnRspIE)
				return QDF_STATUS_E_NOMEM;

			qdf_mem_set(session_ptr->pWpaRsnRspIE,
					nIeLen + 2, 0);
			session_ptr->pWpaRsnRspIE[0] = DOT11F_EID_RSN;
			session_ptr->pWpaRsnRspIE[1] = (uint8_t) nIeLen;
			/* copy upto akm_suites */
			pIeBuf = session_ptr->pWpaRsnRspIE + 2;
			qdf_mem_copy(pIeBuf, &rsnie->version,
					sizeof(rsnie->version));
			pIeBuf += sizeof(rsnie->version);
			qdf_mem_copy(pIeBuf, &rsnie->gp_cipher_suite,
				sizeof(rsnie->gp_cipher_suite));
			pIeBuf += sizeof(rsnie->gp_cipher_suite);
			qdf_mem_copy(pIeBuf, &rsnie->pwise_cipher_suite_count,
				sizeof(rsnie->pwise_cipher_suite_count));
			pIeBuf += sizeof(rsnie->pwise_cipher_suite_count);
			if (rsnie->pwise_cipher_suite_count) {
				/* copy pwise_cipher_suites */
				qdf_mem_copy(pIeBuf, rsnie->pwise_cipher_suites,
					rsnie->pwise_cipher_suite_count * 4);
				pIeBuf += rsnie->pwise_cipher_suite_count * 4;
			}
			qdf_mem_copy(pIeBuf, &rsnie->akm_suite_count, 2);
			pIeBuf += 2;
			if (rsnie->akm_suite_count) {
				/* copy akm_suites */
				qdf_mem_copy(pIeBuf, rsnie->akm_suites,
					rsnie->akm_suite_count * 4);
				pIeBuf += rsnie->akm_suite_count * 4;
			}
			/* copy the rest */
			qdf_mem_copy(pIeBuf, rsnie->akm_suites +
				rsnie->akm_suite_count * 4,
				2 + rsnie->pmkid_count * 4);
			session_ptr->nWpaRsnRspIeLength = nIeLen + 2;
		}
	} else if ((eCSR_AUTH_TYPE_WPA == auth_type) ||
			(eCSR_AUTH_TYPE_WPA_PSK == auth_type)) {
		if (ie_local->WPA.present) {
			tDot11fIEWPA *wpaie = &ie_local->WPA;
			/* Calculate the actual length wpaie */
			nIeLen = 12 + 2 /* auth_suite_count */
				+ wpaie->unicast_cipher_count * 4
				+ wpaie->auth_suite_count * 4;

			/* The WPA capabilities follows the Auth Suite
			 * (two octects)-- this field is optional, and
			 * we always "send" zero, so just remove it.  This is
			 * consistent with our assumptions in the frames
			 * compiler; nIeLen doesn't count EID & length fields */
			session_ptr->pWpaRsnRspIE = qdf_mem_malloc(nIeLen + 2);
			if (NULL == session_ptr->pWpaRsnRspIE)
				return QDF_STATUS_E_NOMEM;
			session_ptr->pWpaRsnRspIE[0] = DOT11F_EID_WPA;
			session_ptr->pWpaRsnRspIE[1] = (uint8_t) nIeLen;
			pIeBuf = session_ptr->pWpaRsnRspIE + 2;
			/* Copy WPA OUI */
			qdf_mem_copy(pIeBuf, &csr_wpa_oui[1], 4);
			pIeBuf += 4;
			qdf_mem_copy(pIeBuf, &wpaie->version,
				8 + wpaie->unicast_cipher_count * 4);
			pIeBuf += 8 + wpaie->unicast_cipher_count * 4;
			qdf_mem_copy(pIeBuf, &wpaie->auth_suite_count,
				2 + wpaie->auth_suite_count * 4);
			pIeBuf += wpaie->auth_suite_count * 4;
			session_ptr->nWpaRsnRspIeLength = nIeLen + 2;
		}
	}
#ifdef FEATURE_WLAN_WAPI
	else if ((eCSR_AUTH_TYPE_WAPI_WAI_PSK == auth_type) ||
			(eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE ==
			 auth_type)) {
		if (ie_local->WAPI.present) {
			tDot11fIEWAPI *wapi_ie = &ie_local->WAPI;
			/* Calculate the actual length of wapi ie*/
			nIeLen = 4 + 2 /* pwise_cipher_suite_count */
				+ wapi_ie->akm_suite_count * 4
				+ wapi_ie->unicast_cipher_suite_count * 4
				+ 6;  /* gp_cipher_suite + preauth + reserved */

			if (wapi_ie->bkid_count)
				nIeLen += 2 + wapi_ie->bkid_count * 4;

			/* nIeLen doesn't count EID and length fields */
			session_ptr->pWapiRspIE =
				qdf_mem_malloc(nIeLen + 2);
			if (NULL == session_ptr->pWapiRspIE)
				return QDF_STATUS_E_NOMEM;
			session_ptr->pWapiRspIE[0] = DOT11F_EID_WAPI;
			session_ptr->pWapiRspIE[1] = (uint8_t) nIeLen;
			pIeBuf = session_ptr->pWapiRspIE + 2;
			/* copy upto akm_suite_count */
			qdf_mem_copy(pIeBuf, &wapi_ie->version, 2);
			pIeBuf += 4;
			if (wapi_ie->akm_suite_count) {
				/* copy akm_suites */
				qdf_mem_copy(pIeBuf,
					wapi_ie->akm_suites,
					wapi_ie->akm_suite_count * 4);
				pIeBuf += wapi_ie->akm_suite_count * 4;
			}
			qdf_mem_copy(pIeBuf,
				&wapi_ie->unicast_cipher_suite_count, 2);
			pIeBuf += 2;
			if (wapi_ie->unicast_cipher_suite_count) {
				uint16_t suite_size =
					wapi_ie->unicast_cipher_suite_count * 4;
				/* copy pwise_cipher_suites */
				qdf_mem_copy(pIeBuf,
					wapi_ie->unicast_cipher_suites,
					suite_size);
				pIeBuf += suite_size;
			}
			/* gp_cipher_suite */
			qdf_mem_copy(pIeBuf,
				wapi_ie->multicast_cipher_suite, 4);
			pIeBuf += 4;
			/* preauth + reserved */
			qdf_mem_copy(pIeBuf,
				wapi_ie->multicast_cipher_suite + 4, 2);
			pIeBuf += 2;
			if (wapi_ie->bkid_count) {
				/* bkid_count */
				qdf_mem_copy(pIeBuf, &wapi_ie->bkid_count, 2);
				pIeBuf += 2;
				/* copy akm_suites */
				qdf_mem_copy(pIeBuf, wapi_ie->bkid,
					wapi_ie->bkid_count * 4);
				pIeBuf += wapi_ie->bkid_count * 4;
			}
			session_ptr->nWapiRspIeLength = nIeLen + 2;
		}
	}
#endif /* FEATURE_WLAN_WAPI */
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS csr_roam_save_security_rsp_ie(tpAniSirGlobal pMac,
						uint32_t sessionId,
						eCsrAuthType authType,
						tSirBssDescription *pSirBssDesc,
						tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tDot11fBeaconIEs *pIesLocal = pIes;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if ((eCSR_AUTH_TYPE_WPA == authType) ||
		(eCSR_AUTH_TYPE_WPA_PSK == authType) ||
		(eCSR_AUTH_TYPE_RSN == authType) ||
		(eCSR_AUTH_TYPE_RSN_PSK == authType)
		|| (eCSR_AUTH_TYPE_FT_RSN == authType) ||
		(eCSR_AUTH_TYPE_FT_RSN_PSK == authType)
#ifdef FEATURE_WLAN_WAPI
		|| (eCSR_AUTH_TYPE_WAPI_WAI_PSK == authType) ||
		(eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE == authType)
#endif /* FEATURE_WLAN_WAPI */
#ifdef WLAN_FEATURE_11W
		|| (eCSR_AUTH_TYPE_RSN_PSK_SHA256 == authType) ||
		(eCSR_AUTH_TYPE_RSN_8021X_SHA256 == authType)
#endif /* FEATURE_WLAN_WAPI */
	) {
		if (!pIesLocal && !QDF_IS_STATUS_SUCCESS
				(csr_get_parsed_bss_description_ies(pMac,
				pSirBssDesc, &pIesLocal)))
			sms_log(pMac, LOGE, FL(" cannot parse IEs"));
		if (pIesLocal) {
			status = csr_roam_save_params(pMac, pSession, authType,
					pIes, pIesLocal);
			if (!pIes)
				/* locally allocated */
				qdf_mem_free(pIesLocal);
		}
	}
	return status;
}

/* Returns whether the current association is a 11r assoc or not */
bool csr_roam_is11r_assoc(tpAniSirGlobal pMac, uint8_t sessionId)
{
	return csr_neighbor_roam_is11r_assoc(pMac, sessionId);
}

/* Returns whether "Legacy Fast Roaming" is currently enabled...or not */
bool csr_roam_is_fast_roam_enabled(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = NULL;

	if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
		pSession = CSR_GET_SESSION(pMac, sessionId);
		if (NULL != pSession->pCurRoamProfile) {
			if (pSession->pCurRoamProfile->csrPersona !=
			    QDF_STA_MODE) {
				return false;
			}
		}
	}
	if (true == CSR_IS_FASTROAM_IN_CONCURRENCY_INI_FEATURE_ENABLED(pMac)) {
		return pMac->roam.configParam.isFastRoamIniFeatureEnabled;
	} else {
		return pMac->roam.configParam.isFastRoamIniFeatureEnabled &&
			(!csr_is_concurrent_session_running(pMac));
	}
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
eCsrPhyMode csr_roamdot11mode_to_phymode(uint8_t dot11mode)
{
	eCsrPhyMode phymode = eCSR_DOT11_MODE_abg;

	switch (dot11mode) {
	case WNI_CFG_DOT11_MODE_ALL:
		phymode = eCSR_DOT11_MODE_abg;
		break;
	case WNI_CFG_DOT11_MODE_11A:
		phymode = eCSR_DOT11_MODE_11a;
		break;
	case WNI_CFG_DOT11_MODE_11B:
		phymode = eCSR_DOT11_MODE_11b;
		break;
	case WNI_CFG_DOT11_MODE_11G:
		phymode = eCSR_DOT11_MODE_11g;
		break;
	case WNI_CFG_DOT11_MODE_11N:
		phymode = eCSR_DOT11_MODE_11n;
		break;
	case WNI_CFG_DOT11_MODE_11G_ONLY:
		phymode = eCSR_DOT11_MODE_11g_ONLY;
		break;
	case WNI_CFG_DOT11_MODE_11N_ONLY:
		phymode = eCSR_DOT11_MODE_11n_ONLY;
		break;
	case WNI_CFG_DOT11_MODE_11AC:
		phymode = eCSR_DOT11_MODE_11ac;
		break;
	case WNI_CFG_DOT11_MODE_11AC_ONLY:
		phymode = eCSR_DOT11_MODE_11ac_ONLY;
		break;
	default:
		break;
	}

	return phymode;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void csr_roam_synch_clean_up (tpAniSirGlobal mac, uint8_t session_id)
{
	cds_msg_t msg;
	struct roam_offload_synch_fail *roam_offload_failed = NULL;
	tCsrRoamSession *session = &mac->roam.roamSession[session_id];

	/* Clean up the roam synch in progress for LFR3 */
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "%s: Roam Synch Failed, Clean Up", __func__);
	session->roam_synch_in_progress = false;

	roam_offload_failed = qdf_mem_malloc(
				sizeof(struct roam_offload_synch_fail));
	if (NULL == roam_offload_failed) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: unable to allocate memory for roam synch fail" ,
			  __func__);
		return;
	}

	roam_offload_failed->session_id = session_id;
	msg.type     = WMA_ROAM_OFFLOAD_SYNCH_FAIL;
	msg.reserved = 0;
	msg.bodyptr  = roam_offload_failed;
	if (!QDF_IS_STATUS_SUCCESS(cds_mq_post_message(QDF_MODULE_ID_WMA,
						       &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"%s: Unable to post WMA_ROAM_OFFLOAD_SYNCH_FAIL to WMA",
			__func__);
		qdf_mem_free(roam_offload_failed);
	}
}
#endif

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * csr_roam_copy_ht_profile() - Copy from src to dst
 * @dst_profile:          Destination HT profile
 * @src_profile:          Source HT profile
 *
 * Copy the HT profile from the given source to destination
 *
 * Return: None
 */
static void csr_roam_copy_ht_profile(tCsrRoamHTProfile *dst_profile,
		tSirSmeHTProfile *src_profile)
{
	dst_profile->phymode =
		csr_roamdot11mode_to_phymode(src_profile->dot11mode);
	dst_profile->htCapability = src_profile->htCapability;
	dst_profile->htSupportedChannelWidthSet =
		src_profile->htSupportedChannelWidthSet;
	dst_profile->htRecommendedTxWidthSet =
		src_profile->htRecommendedTxWidthSet;
	dst_profile->htSecondaryChannelOffset =
		src_profile->htSecondaryChannelOffset;
	dst_profile->vhtCapability = src_profile->vhtCapability;
	dst_profile->apCenterChan = src_profile->apCenterChan;
	dst_profile->apChanWidth = src_profile->apChanWidth;
}
#endif

/**
 * csr_roam_process_results_default() - Process the result for start bss
 * @mac_ctx:          Global MAC Context
 * @cmd:              Command to be processed
 * @context:          Additional data in context of the cmd
 *
 * Return: None
 */
static void csr_roam_process_results_default(tpAniSirGlobal mac_ctx,
		     tSmeCmd *cmd, void *context, eCsrRoamCompleteResult res)
{
	uint32_t session_id = cmd->sessionId;
	tCsrRoamSession *session;
	tCsrRoamInfo roam_info;
	QDF_STATUS status;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE, FL("Invalid session id %d"), session_id);
		return;
	}
	session = CSR_GET_SESSION(mac_ctx, session_id);

	sms_log(mac_ctx, LOGW, FL("receives no association indication"));
	sms_log(mac_ctx, LOG1, FL("Assoc ref count %d"),
			session->bRefAssocStartCnt);
	if (CSR_IS_INFRASTRUCTURE(&session->connectedProfile)
		|| CSR_IS_ROAM_SUBSTATE_STOP_BSS_REQ(mac_ctx, session_id)) {
		/*
		 * do not free for the other profiles as we need
		 * to send down stop BSS later
		 */
		csr_free_connect_bss_desc(mac_ctx, session_id);
		csr_roam_free_connect_profile(&session->connectedProfile);
		csr_roam_free_connected_info(mac_ctx, &session->connectedInfo);
		csr_set_default_dot11_mode(mac_ctx);
	}

	switch (cmd->u.roamCmd.roamReason) {
	/*
	 * If this transition is because of an 802.11 OID, then we
	 * transition back to INIT state so we sit waiting for more
	 * OIDs to be issued and we don't start the IDLE timer.
	 */
	case eCsrSmeIssuedFTReassoc:
	case eCsrSmeIssuedAssocToSimilarAP:
	case eCsrHddIssued:
	case eCsrSmeIssuedDisassocForHandoff:
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_IDLE,
			session_id);
		qdf_mem_set(&roam_info, sizeof(tCsrRoamInfo), 0);
		roam_info.pBssDesc = cmd->u.roamCmd.pLastRoamBss;
		roam_info.pProfile = &cmd->u.roamCmd.roamProfile;
		roam_info.statusCode = session->joinFailStatusCode.statusCode;
		roam_info.reasonCode = session->joinFailStatusCode.reasonCode;
		qdf_mem_copy(&roam_info.bssid,
			&session->joinFailStatusCode.bssId,
			sizeof(struct qdf_mac_addr));

		/*
		 * If Join fails while Handoff is in progress, indicate
		 * disassociated event to supplicant to reconnect
		 */
		if (csr_roam_is_handoff_in_progress(mac_ctx, session_id)) {
			csr_neighbor_roam_indicate_connect(mac_ctx,
				(uint8_t)session_id, QDF_STATUS_E_FAILURE);
		}
		if (session->bRefAssocStartCnt > 0) {
			session->bRefAssocStartCnt--;
			if (eCsrJoinFailureDueToConcurrency == res)
				csr_roam_call_callback(mac_ctx, session_id,
					&roam_info, cmd->u.roamCmd.roamId,
					eCSR_ROAM_ASSOCIATION_COMPLETION,
					eCSR_ROAM_RESULT_ASSOC_FAIL_CON_CHANNEL);
			else
				csr_roam_call_callback(mac_ctx, session_id,
					&roam_info, cmd->u.roamCmd.roamId,
					eCSR_ROAM_ASSOCIATION_COMPLETION,
					eCSR_ROAM_RESULT_FAILURE);
		} else {
			/*
			 * bRefAssocStartCnt is not incremented when
			 * eRoamState == eCsrStopRoamingDueToConcurrency
			 * in csr_roam_join_next_bss API. so handle this in
			 * else case by sending assoc failure
			 */
			csr_roam_call_callback(mac_ctx, session_id,
				&roam_info, cmd->u.scanCmd.roamId,
				eCSR_ROAM_ASSOCIATION_FAILURE,
				eCSR_ROAM_RESULT_FAILURE);
		}
		sms_log(mac_ctx, LOG1, FL("roam(reason %d) failed"),
			cmd->u.roamCmd.roamReason);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		sme_qos_update_hand_off((uint8_t) session_id, false);
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id,
			SME_QOS_CSR_DISCONNECT_IND, NULL);
#endif
		csr_roam_completion(mac_ctx, session_id, NULL, cmd,
			eCSR_ROAM_RESULT_FAILURE, false);
		break;
	case eCsrHddIssuedReassocToSameAP:
	case eCsrSmeIssuedReassocToSameAP:
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_IDLE,
			session_id);

		csr_roam_call_callback(mac_ctx, session_id, NULL,
			cmd->u.roamCmd.roamId, eCSR_ROAM_DISASSOCIATED,
			eCSR_ROAM_RESULT_FORCED);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id,
			SME_QOS_CSR_DISCONNECT_IND, NULL);
#endif
		csr_roam_completion(mac_ctx, session_id, NULL, cmd,
			eCSR_ROAM_RESULT_FAILURE, false);
		break;
	case eCsrForcedDisassoc:
	case eCsrForcedDeauth:
	case eCsrSmeIssuedIbssJoinFailure:
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_IDLE,
			session_id);

		if (eCsrSmeIssuedIbssJoinFailure == cmd->u.roamCmd.roamReason)
			/* notify HDD that IBSS join failed */
			csr_roam_call_callback(mac_ctx, session_id, NULL, 0,
				eCSR_ROAM_IBSS_IND,
				eCSR_ROAM_RESULT_IBSS_JOIN_FAILED);
		else
			csr_roam_call_callback(mac_ctx, session_id, NULL,
				cmd->u.roamCmd.roamId, eCSR_ROAM_DISASSOCIATED,
				eCSR_ROAM_RESULT_FORCED);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id,
				SME_QOS_CSR_DISCONNECT_IND,
				NULL);
#endif
		csr_roam_link_down(mac_ctx, session_id);

		if (mac_ctx->roam.deauthRspStatus == eSIR_SME_DEAUTH_STATUS) {
			sms_log(mac_ctx, LOGW,
				FL("FW still in connected state"));
			break;
		}
		break;
	case eCsrForcedIbssLeave:
		csr_roam_call_callback(mac_ctx, session_id, NULL,
			cmd->u.roamCmd.roamId, eCSR_ROAM_IBSS_LEAVE,
			eCSR_ROAM_RESULT_IBSS_STOP);
		session->connectState = eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED;
		break;
	case eCsrForcedDisassocMICFailure:
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_IDLE,
			session_id);

		csr_roam_call_callback(mac_ctx, session_id, NULL,
			cmd->u.roamCmd.roamId, eCSR_ROAM_DISASSOCIATED,
			eCSR_ROAM_RESULT_MIC_FAILURE);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id,
			SME_QOS_CSR_DISCONNECT_REQ, NULL);
#endif
		break;
	case eCsrStopBss:
		csr_roam_call_callback(mac_ctx, session_id, NULL,
			cmd->u.roamCmd.roamId, eCSR_ROAM_INFRA_IND,
			eCSR_ROAM_RESULT_INFRA_STOPPED);
		break;
	case eCsrForcedDisassocSta:
	case eCsrForcedDeauthSta:
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_JOINED,
			session_id);
		session = CSR_GET_SESSION(mac_ctx, session_id);
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id) &&
			CSR_IS_INFRA_AP(&session->connectedProfile)) {
			roam_info.u.pConnectedProfile =
				&session->connectedProfile;
			qdf_mem_copy(roam_info.peerMac.bytes,
					cmd->u.roamCmd.peerMac,
					sizeof(tSirMacAddr));
			roam_info.reasonCode = eCSR_ROAM_RESULT_FORCED;
			roam_info.statusCode = eSIR_SME_SUCCESS;
			status = csr_roam_call_callback(mac_ctx, session_id,
					&roam_info, cmd->u.roamCmd.roamId,
					eCSR_ROAM_LOSTLINK,
					eCSR_ROAM_RESULT_FORCED);
		}
		break;
	case eCsrLostLink1:
		/* if lost link roam1 failed, then issue lost link Scan2 ... */
		csr_scan_request_lost_link2(mac_ctx, session_id);
		break;
	case eCsrLostLink2:
		/* if lost link roam2 failed, then issue lost link scan3 ... */
		csr_scan_request_lost_link3(mac_ctx, session_id);
		break;
	case eCsrLostLink3:
	default:
		csr_roam_state_change(mac_ctx,
			eCSR_ROAMING_STATE_IDLE, session_id);

		/* We are done with one round of lostlink roaming here */
		csr_scan_handle_failed_lostlink3(mac_ctx, session_id);
		break;
	}
}

/**
 * csr_roam_process_start_bss_success() - Process the result for start bss
 * @mac_ctx:          Global MAC Context
 * @cmd:              Command to be processed
 * @context:          Additional data in context of the cmd
 *
 * Return: None
 */
static void csr_roam_process_start_bss_success(tpAniSirGlobal mac_ctx,
		     tSmeCmd *cmd, void *context)
{
	uint32_t session_id = cmd->sessionId;
	tCsrRoamProfile *profile = &cmd->u.roamCmd.roamProfile;
	tCsrRoamSession *session;
	tSirBssDescription *bss_desc = NULL;
	tCsrRoamInfo roam_info;
	tSirSmeStartBssRsp *start_bss_rsp = NULL;
	tCsrScanResult *scan_res = NULL;
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;
	tDot11fBeaconIEs *ies_ptr = NULL;
	tSirMacAddr bcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	QDF_STATUS status;
	host_log_ibss_pkt_type *ibss_log;
	uint32_t bi;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	tSirSmeHTProfile *src_profile = NULL;
	tCsrRoamHTProfile *dst_profile = NULL;
#endif

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE, FL("Invalid session id %d"), session_id);
		return;
	}
	session = CSR_GET_SESSION(mac_ctx, session_id);

	/*
	 * on the StartBss Response, LIM is returning the Bss Description that
	 * we are beaconing.  Add this Bss Description to our scan results and
	 * chain the Profile to this Bss Description.  On a Start BSS, there was
	 * no detected Bss description (no partner) so we issued the Start Bss
	 * to start the Ibss without any Bss description.  Lim was kind enough
	 * to return the Bss Description that we start beaconing for the newly
	 * started Ibss.
	 */
	sms_log(mac_ctx, LOG2, FL("receives start BSS ok indication"));
	status = QDF_STATUS_E_FAILURE;
	start_bss_rsp = (tSirSmeStartBssRsp *) context;
	qdf_mem_set(&roam_info, sizeof(tCsrRoamInfo), 0);
	if (CSR_IS_IBSS(profile))
		session->connectState = eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED;
	else if (CSR_IS_INFRA_AP(profile))
		session->connectState =
			eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED;
	else if (CSR_IS_NDI(profile))
		session->connectState = eCSR_CONNECT_STATE_TYPE_NDI_STARTED;
	else
		session->connectState = eCSR_ASSOC_STATE_TYPE_WDS_DISCONNECTED;

	bss_desc = &start_bss_rsp->bssDescription;
	if (CSR_IS_NDI(profile)) {
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_JOINED,
			session_id);
		csr_roam_save_ndi_connected_info(mac_ctx, session_id, profile,
						bss_desc);
		roam_info.u.pConnectedProfile = &session->connectedProfile;
		qdf_mem_copy(&roam_info.bssid, &bss_desc->bssId,
			    sizeof(struct qdf_mac_addr));
	} else {
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_JOINED,
				session_id);
		if (!QDF_IS_STATUS_SUCCESS
			(csr_get_parsed_bss_description_ies(mac_ctx, bss_desc,
							    &ies_ptr))) {
			sms_log(mac_ctx, LOGW, FL("cannot parse IBSS IEs"));
			roam_info.pBssDesc = bss_desc;
			csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId, eCSR_ROAM_IBSS_IND,
				eCSR_ROAM_RESULT_IBSS_START_FAILED);
			return;
		}
	}
	if (!CSR_IS_INFRA_AP(profile)) {
		scan_res =
			csr_scan_append_bss_description(mac_ctx,
					bss_desc, ies_ptr, false,
					session_id);
	}
	csr_roam_save_connected_bss_desc(mac_ctx, session_id, bss_desc);
	csr_roam_free_connect_profile(&session->connectedProfile);
	csr_roam_free_connected_info(mac_ctx, &session->connectedInfo);
	csr_roam_save_connected_infomation(mac_ctx, session_id,
			profile, bss_desc, ies_ptr);
	qdf_mem_copy(&roam_info.bssid, &bss_desc->bssId,
			sizeof(struct qdf_mac_addr));
	/* We are done with the IEs so free it */
	qdf_mem_free(ies_ptr);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	WLAN_HOST_DIAG_LOG_ALLOC(ibss_log,
		host_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
	if (ibss_log) {
		if (CSR_INVALID_SCANRESULT_HANDLE ==
				cmd->u.roamCmd.hBSSList) {
			/*
			 * We start the IBSS (didn't find any
			 * matched IBSS out there)
			 */
			ibss_log->eventId =
				WLAN_IBSS_EVENT_START_IBSS_RSP;
		} else {
			ibss_log->eventId =
				WLAN_IBSS_EVENT_JOIN_IBSS_RSP;
		}
		if (bss_desc) {
			qdf_mem_copy(ibss_log->bssid.bytes,
				bss_desc->bssId, QDF_MAC_ADDR_SIZE);
			ibss_log->operatingChannel =
				bss_desc->channelId;
		}
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(
					mac_ctx,
					WNI_CFG_BEACON_INTERVAL,
					&bi)))
			/* U8 is not enough for BI */
			ibss_log->beaconInterval = (uint8_t) bi;
		WLAN_HOST_DIAG_LOG_REPORT(ibss_log);
	}
#endif
	/*
	 * Only set context for non-WDS_STA. We don't even need it for
	 * WDS_AP. But since the encryption.
	 * is WPA2-PSK so it won't matter.
	 */
	if (CSR_IS_ENC_TYPE_STATIC(profile->negotiatedUCEncryptionType)
			&& session->pCurRoamProfile
			&& !CSR_IS_INFRA_AP(session->pCurRoamProfile)) {
		/*
		 * Issue the set Context request to LIM to establish
		 * the Broadcast STA context for the Ibss. In Rome IBSS
		 * case, dummy key installation will break proper BSS
		 * key installation, so skip it.
		 */
		if (!CSR_IS_IBSS(session->pCurRoamProfile)) {
			/* NO keys. these key parameters don't matter */
			csr_roam_issue_set_context_req(mac_ctx,
				session_id,
				profile->negotiatedMCEncryptionType,
				bss_desc, &bcast_mac, false,
				false, eSIR_TX_RX, 0, 0, NULL, 0);
		}
	}
	/*
	 * Only tell upper layer is we start the BSS because Vista doesn't like
	 * multiple connection indications. If we don't start the BSS ourself,
	 * handler of eSIR_SME_JOINED_NEW_BSS will trigger the connection start
	 * indication in Vista
	 */
	if (!CSR_IS_JOIN_TO_IBSS(profile)) {
		roam_status = eCSR_ROAM_IBSS_IND;
		roam_result = eCSR_ROAM_RESULT_IBSS_STARTED;
		if (CSR_IS_INFRA_AP(profile)) {
			roam_status = eCSR_ROAM_INFRA_IND;
			roam_result = eCSR_ROAM_RESULT_INFRA_STARTED;
		}
		roam_info.staId = (uint8_t) start_bss_rsp->staId;
		if (CSR_IS_NDI(profile)) {
			csr_roam_update_ndp_return_params(mac_ctx,
							eCsrStartBssSuccess,
							&roam_status,
							&roam_result,
							&roam_info);
		}
		/*
		 * Only tell upper layer is we start the BSS because Vista
		 * doesn't like multiple connection indications. If we don't
		 * start the BSS ourself, handler of eSIR_SME_JOINED_NEW_BSS
		 * will trigger the connection start indication in Vista
		 */
		roam_info.statusCode = session->joinFailStatusCode.statusCode;
		roam_info.reasonCode = session->joinFailStatusCode.reasonCode;
		/* We start the IBSS (didn't find any matched IBSS out there) */
		roam_info.pBssDesc = bss_desc;
		if (bss_desc)
			qdf_mem_copy(roam_info.bssid.bytes, bss_desc->bssId,
				sizeof(struct qdf_mac_addr));
		if (!IS_FEATURE_SUPPORTED_BY_FW(SLM_SESSIONIZATION) &&
				(csr_is_concurrent_session_running(mac_ctx))) {
			mac_ctx->roam.configParam.doBMPSWorkaround = 1;
		}
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		dst_profile = &session->connectedProfile.HTProfile;
		src_profile = &start_bss_rsp->HTProfile;
		if (mac_ctx->roam.configParam.cc_switch_mode
				!= QDF_MCC_TO_SCC_SWITCH_DISABLE)
			csr_roam_copy_ht_profile(dst_profile, src_profile);
#endif
		csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				roam_status, roam_result);
	}

}

/**
 * csr_roam_process_join_res() - Process the Join results
 * @mac_ctx:          Global MAC Context
 * @result:           Result after the command was processed
 * @cmd:              Command to be processed
 * @context:          Additional data in context of the cmd
 *
 * Process the join results which are obtained in a succesful join
 *
 * Return: None
 */
static void csr_roam_process_join_res(tpAniSirGlobal mac_ctx,
	eCsrRoamCompleteResult res, tSmeCmd *cmd, void *context)
{
	tSirMacAddr bcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	sme_QosAssocInfo assoc_info;
	uint32_t key_timeout_interval = 0;
	uint8_t acm_mask = 0;   /* HDD needs ACM mask in assoc rsp callback */
	uint32_t session_id = cmd->sessionId;
	tCsrRoamProfile *profile = &cmd->u.roamCmd.roamProfile;
	tCsrRoamSession *session;
	tSirBssDescription *bss_desc = NULL;
	tCsrScanResult *scan_res = NULL;
	sme_qos_csr_event_indType ind_qos;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	tSirSmeHTProfile *src_profile = NULL;
	tCsrRoamHTProfile *dst_profile = NULL;
#endif
	tCsrRoamConnectedProfile *conn_profile = NULL;
	tDot11fBeaconIEs *ies_ptr = NULL;
	tCsrRoamInfo roam_info;
	struct ps_global_info *ps_global_info = &mac_ctx->sme.ps_global_info;
	tSirSmeJoinRsp *join_rsp = (tSirSmeJoinRsp *) context;
	uint32_t len;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE, FL("Invalid session id %d"), session_id);
		return;
	}
	session = CSR_GET_SESSION(mac_ctx, session_id);

	conn_profile = &session->connectedProfile;
	if (eCsrReassocSuccess == res) {
		roam_info.reassoc = true;
		ind_qos = SME_QOS_CSR_REASSOC_COMPLETE;
	} else {
		roam_info.reassoc = false;
		ind_qos = SME_QOS_CSR_ASSOC_COMPLETE;
	}
	sms_log(mac_ctx, LOGW, FL("receives association indication"));
	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	/* always free the memory here */
	if (session->pWpaRsnRspIE) {
		session->nWpaRsnRspIeLength = 0;
		qdf_mem_free(session->pWpaRsnRspIE);
		session->pWpaRsnRspIE = NULL;
	}
#ifdef FEATURE_WLAN_WAPI
	if (session->pWapiRspIE) {
		session->nWapiRspIeLength = 0;
		qdf_mem_free(session->pWapiRspIE);
		session->pWapiRspIE = NULL;
	}
#endif /* FEATURE_WLAN_WAPI */
#ifdef FEATURE_WLAN_BTAMP_UT_RF
	session->maxRetryCount = 0;
	csr_roam_stop_join_retry_timer(mac_ctx, session_id);
#endif
	/*
	 * Reset remain_in_power_active_till_dhcp as
	 * it might have been set by last failed secured connection.
	 * It should be set only for secured connection.
	 */
	ps_global_info->remain_in_power_active_till_dhcp = false;
	if (CSR_IS_INFRASTRUCTURE(profile))
		session->connectState = eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED;
	else
		session->connectState = eCSR_ASSOC_STATE_TYPE_WDS_CONNECTED;
	/*
	 * Use the last connected bssdesc for reassoc-ing to the same AP.
	 * NOTE: What to do when reassoc to a different AP???
	 */
	if ((eCsrHddIssuedReassocToSameAP == cmd->u.roamCmd.roamReason)
		|| (eCsrSmeIssuedReassocToSameAP ==
			cmd->u.roamCmd.roamReason)) {
		bss_desc = session->pConnectBssDesc;
		if (bss_desc)
			qdf_mem_copy(&roam_info.bssid, &bss_desc->bssId,
					sizeof(struct qdf_mac_addr));
	} else {
		if (cmd->u.roamCmd.pRoamBssEntry) {
			scan_res = GET_BASE_ADDR(cmd->u.roamCmd.pRoamBssEntry,
					tCsrScanResult, Link);
			if (scan_res != NULL) {
				bss_desc = &scan_res->Result.BssDescriptor;
				ies_ptr = (tDot11fBeaconIEs *)
					(scan_res->Result.pvIes);
				qdf_mem_copy(&roam_info.bssid, &bss_desc->bssId,
					sizeof(struct qdf_mac_addr));
			}
		}
	}
	if (bss_desc) {
		roam_info.staId = STA_INVALID_IDX;
		csr_roam_save_connected_infomation(mac_ctx, session_id,
			profile, bss_desc, ies_ptr);
		/* Save WPA/RSN IE */
		csr_roam_save_security_rsp_ie(mac_ctx, session_id,
			profile->negotiatedAuthType, bss_desc, ies_ptr);
#ifdef FEATURE_WLAN_ESE
		roam_info.isESEAssoc = conn_profile->isESEAssoc;
#endif

		/*
		 * csr_roam_state_change also affects sub-state.
		 * Hence, csr_roam_state_change happens first and then
		 * substate change.
		 * Moving even save profile above so that below
		 * mentioned conditon is also met.
		 * JEZ100225: Moved to after saving the profile.
		 * Fix needed in main/latest
		 */
		csr_roam_state_change(mac_ctx,
			eCSR_ROAMING_STATE_JOINED, session_id);

		/*
		 * Make sure the Set Context is issued before link
		 * indication to NDIS.  After link indication is
		 * made to NDIS, frames could start flowing.
		 * If we have not set context with LIM, the frames
		 * will be dropped for the security context may not
		 * be set properly.
		 *
		 * this was causing issues in the 2c_wlan_wep WHQL test
		 * when the SetContext was issued after the link
		 * indication. (Link Indication happens in the
		 * profFSMSetConnectedInfra call).
		 *
		 * this reordering was done on titan_prod_usb branch
		 * and is being replicated here.
		 */

		if (CSR_IS_ENC_TYPE_STATIC
			(profile->negotiatedUCEncryptionType) &&
			!profile->bWPSAssociation) {
			/*
			 * Issue the set Context request to LIM to establish
			 * the Unicast STA context
			 */
			if (!QDF_IS_STATUS_SUCCESS(
				csr_roam_issue_set_context_req(mac_ctx,
					session_id,
					profile->negotiatedUCEncryptionType,
					bss_desc, &(bss_desc->bssId),
					false, true,
					eSIR_TX_RX, 0, 0, NULL, 0))) {
				/* NO keys. these key parameters don't matter */
				sms_log(mac_ctx, LOGE,
					FL("Set context for unicast fail"));
				csr_roam_substate_change(mac_ctx,
					eCSR_ROAM_SUBSTATE_NONE, session_id);
			}
			/*
			 * Issue the set Context request to LIM
			 * to establish the Broadcast STA context
			 * NO keys. these key parameters don't matter
			 */
			csr_roam_issue_set_context_req(mac_ctx, session_id,
				profile->negotiatedMCEncryptionType,
				bss_desc, &bcast_mac, false, false,
				eSIR_TX_RX, 0, 0, NULL, 0);
		} else {
			/* Need to wait for supplicant authtication */
			roam_info.fAuthRequired = true;
			/*
			 * Set the substate to WaitForKey in case
			 * authentiation is needed
			 */
			csr_roam_substate_change(mac_ctx,
					eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY,
					session_id);

			/*
			 * Set remain_in_power_active_till_dhcp to make
			 * sure we wait for until keys are set before
			 * going into BMPS.
			 */
			ps_global_info->remain_in_power_active_till_dhcp
				= true;

			if (profile->bWPSAssociation)
				key_timeout_interval =
					CSR_WAIT_FOR_WPS_KEY_TIMEOUT_PERIOD;
			else
				key_timeout_interval =
					CSR_WAIT_FOR_KEY_TIMEOUT_PERIOD;

			/* Save session_id in case of timeout */
			mac_ctx->roam.WaitForKeyTimerInfo.sessionId =
				(uint8_t) session_id;
			/*
			 * This time should be long enough for the rest
			 * of the process plus setting key
			 */
			if (!QDF_IS_STATUS_SUCCESS
					(csr_roam_start_wait_for_key_timer(
					   mac_ctx, key_timeout_interval))
			   ) {
				/* Reset state so nothing is blocked. */
				sms_log(mac_ctx, LOGE, FL
						("Failed preauth timer start"));
				csr_roam_substate_change(mac_ctx,
						eCSR_ROAM_SUBSTATE_NONE,
						session_id);
			}
		}

		assoc_info.pBssDesc = bss_desc;       /* could be NULL */
		assoc_info.pProfile = profile;
		if (context) {
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			if (session->roam_synch_in_progress)
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_DEBUG,
					FL("LFR3:Clear Connected info"));
#endif
			csr_roam_free_connected_info(mac_ctx,
				&session->connectedInfo);
			len = join_rsp->assocReqLength +
				join_rsp->assocRspLength +
				join_rsp->beaconLength;
			len += join_rsp->parsedRicRspLen;
#ifdef FEATURE_WLAN_ESE
			len += join_rsp->tspecIeLen;
#endif
			if (len) {
				session->connectedInfo.pbFrames =
					qdf_mem_malloc(len);
				if (session->connectedInfo.pbFrames !=
						NULL) {
					qdf_mem_copy(
						session->connectedInfo.pbFrames,
						join_rsp->frames, len);
					session->connectedInfo.nAssocReqLength =
						join_rsp->assocReqLength;
					session->connectedInfo.nAssocRspLength =
						join_rsp->assocRspLength;
					session->connectedInfo.nBeaconLength =
						join_rsp->beaconLength;
					session->connectedInfo.nRICRspLength =
						join_rsp->parsedRicRspLen;
#ifdef FEATURE_WLAN_ESE
					session->connectedInfo.nTspecIeLength =
						join_rsp->tspecIeLen;
#endif
					roam_info.nAssocReqLength =
						join_rsp->assocReqLength;
					roam_info.nAssocRspLength =
						join_rsp->assocRspLength;
					roam_info.nBeaconLength =
						join_rsp->beaconLength;
					roam_info.pbFrames =
						session->connectedInfo.pbFrames;
				}
			}
			if (cmd->u.roamCmd.fReassoc)
				roam_info.fReassocReq =
					roam_info.fReassocRsp = true;
			conn_profile->vht_channel_width =
				join_rsp->vht_channel_width;
			session->connectedInfo.staId =
				(uint8_t) join_rsp->staId;
			roam_info.staId = (uint8_t) join_rsp->staId;
			roam_info.ucastSig = (uint8_t) join_rsp->ucastSig;
			roam_info.bcastSig = (uint8_t) join_rsp->bcastSig;
			roam_info.timingMeasCap = join_rsp->timingMeasCap;
			roam_info.chan_info.nss = join_rsp->nss;
			roam_info.chan_info.rate_flags =
				join_rsp->max_rate_flags;
#ifdef FEATURE_WLAN_TDLS
			roam_info.tdls_prohibited = join_rsp->tdls_prohibited;
			roam_info.tdls_chan_swit_prohibited =
				join_rsp->tdls_chan_swit_prohibited;
			sms_log(mac_ctx, LOG1,
				FL("tdls:prohibit: %d, chan_swit_prohibit: %d"),
				roam_info.tdls_prohibited,
				roam_info.tdls_chan_swit_prohibited);
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
			src_profile = &join_rsp->HTProfile;
			dst_profile = &conn_profile->HTProfile;
			if (mac_ctx->roam.configParam.cc_switch_mode
				!= QDF_MCC_TO_SCC_SWITCH_DISABLE)
				csr_roam_copy_ht_profile(dst_profile,
						src_profile);
#endif
			roam_info.vht_caps = join_rsp->vht_caps;
			roam_info.ht_caps = join_rsp->ht_caps;
			roam_info.hs20vendor_ie = join_rsp->hs20vendor_ie;
			roam_info.ht_operation = join_rsp->ht_operation;
			roam_info.vht_operation = join_rsp->vht_operation;
		} else {
			if (cmd->u.roamCmd.fReassoc) {
				roam_info.fReassocReq =
					roam_info.fReassocRsp = true;
				roam_info.nAssocReqLength =
					session->connectedInfo.nAssocReqLength;
				roam_info.nAssocRspLength =
					session->connectedInfo.nAssocRspLength;
				roam_info.nBeaconLength =
					session->connectedInfo.nBeaconLength;
				roam_info.pbFrames =
					session->connectedInfo.pbFrames;
			}
		}
		/*
		 * Update the staId from the previous connected profile info
		 * as the reassociation is triggred at SME/HDD
		 */

		if ((eCsrHddIssuedReassocToSameAP ==
				cmd->u.roamCmd.roamReason) ||
			(eCsrSmeIssuedReassocToSameAP ==
				cmd->u.roamCmd.roamReason))
			roam_info.staId = session->connectedInfo.staId;

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		/*
		 * Indicate SME-QOS with reassoc success event,
		 * only after copying the frames
		 */
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id, ind_qos,
				&assoc_info);
#endif
		roam_info.pBssDesc = bss_desc;
		roam_info.statusCode =
			session->joinFailStatusCode.statusCode;
		roam_info.reasonCode =
			session->joinFailStatusCode.reasonCode;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		acm_mask = sme_qos_get_acm_mask(mac_ctx, bss_desc, NULL);
#endif
		conn_profile->acm_mask = acm_mask;
		/*
		 * start UAPSD if uapsd_mask is not 0 because HDD will
		 * configure for trigger frame It may be better to let QoS do
		 * this????
		 */
		if (conn_profile->modifyProfileFields.uapsd_mask) {
			sms_log(mac_ctx, LOGE,
				" uapsd_mask (0x%X) set, request UAPSD now",
				conn_profile->modifyProfileFields.uapsd_mask);
			sme_ps_start_uapsd(mac_ctx, session_id,
				NULL, NULL);
		}
		conn_profile->dot11Mode = session->bssParams.uCfgDot11Mode;
		roam_info.u.pConnectedProfile = conn_profile;

		if (session->bRefAssocStartCnt > 0) {
			session->bRefAssocStartCnt--;
			if (!IS_FEATURE_SUPPORTED_BY_FW
				(SLM_SESSIONIZATION) &&
				(csr_is_concurrent_session_running(mac_ctx))) {
				mac_ctx->roam.configParam.doBMPSWorkaround = 1;
			}
			csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				eCSR_ROAM_ASSOCIATION_COMPLETION,
				eCSR_ROAM_RESULT_ASSOCIATED);
		}

		csr_roam_completion(mac_ctx, session_id, NULL, cmd,
				eCSR_ROAM_RESULT_NONE, true);
		csr_reset_pmkid_candidate_list(mac_ctx, session_id);
#ifdef FEATURE_WLAN_WAPI
		csr_reset_bkid_candidate_list(mac_ctx, session_id);
#endif
	} else {
		sms_log(mac_ctx, LOGW,
			"Roam command doesn't have a BSS desc");
	}
	/* Not to signal link up because keys are yet to be set.
	 * The linkup function will overwrite the sub-state that
	 * we need to keep at this point.
	 */
	if (!CSR_IS_WAIT_FOR_KEY(mac_ctx, session_id)) {
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (session->roam_synch_in_progress) {
			QDF_TRACE(QDF_MODULE_ID_SME,
				QDF_TRACE_LEVEL_DEBUG,
				FL
				("NO CSR_IS_WAIT_FOR_KEY -> csr_roam_link_up"));
		}
#endif
		csr_roam_link_up(mac_ctx, conn_profile->bssid);
	}
}

/**
 * csr_roam_process_results() - Process the Roam Results
 * @mac_ctx:      Global MAC Context
 * @cmd:          Command that has been processed
 * @res:          Results available after processing the command
 * @context:      Context
 *
 * Process the available results and make an appropriate decision
 *
 * Return: true if the command can be released, else not.
 */
static bool csr_roam_process_results(tpAniSirGlobal mac_ctx, tSmeCmd *cmd,
				     eCsrRoamCompleteResult res, void *context)
{
	bool release_cmd = true;
	tSirBssDescription *bss_desc = NULL;
	tCsrRoamInfo roam_info;
	uint32_t session_id = cmd->sessionId;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	tCsrRoamProfile *profile = &cmd->u.roamCmd.roamProfile;
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;
	host_log_ibss_pkt_type *ibss_log;
	tSirSmeStartBssRsp  *start_bss_rsp = NULL;

	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found "), session_id);
		return false;
	}
	sms_log(mac_ctx, LOG1, FL("Processing ROAM results..."));
	switch (res) {
	case eCsrJoinSuccess:
	case eCsrReassocSuccess:
		csr_roam_process_join_res(mac_ctx, res, cmd, context);
		break;
	case eCsrStartBssSuccess:
		csr_roam_process_start_bss_success(mac_ctx, cmd, context);
		break;
	case eCsrStartBssFailure:
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
		WLAN_HOST_DIAG_LOG_ALLOC(ibss_log,
			host_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
		if (ibss_log) {
			ibss_log->status = WLAN_IBSS_STATUS_FAILURE;
			WLAN_HOST_DIAG_LOG_REPORT(ibss_log);
		}
#endif
		start_bss_rsp = (tSirSmeStartBssRsp *)context;
		qdf_mem_set(&roam_info, sizeof(roam_info), 0);
		roam_status = eCSR_ROAM_IBSS_IND;
		roam_result = eCSR_ROAM_RESULT_IBSS_STARTED;
		if (CSR_IS_INFRA_AP(profile)) {
			roam_status = eCSR_ROAM_INFRA_IND;
			roam_result = eCSR_ROAM_RESULT_INFRA_START_FAILED;
		}
		if (CSR_IS_NDI(profile)) {
			csr_roam_update_ndp_return_params(mac_ctx,
				eCsrStartBssFailure,
				&roam_status, &roam_result, &roam_info);
		}

		if (context) {
			bss_desc = (tSirBssDescription *) context;
		} else {
			bss_desc = NULL;
		}
		roam_info.pBssDesc = bss_desc;
		csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId, roam_status,
				roam_result);
		csr_set_default_dot11_mode(mac_ctx);
		break;
	case eCsrSilentlyStopRoaming:
		/*
		 * We are here because we try to start the same IBSS.
		 * No message to PE. return the roaming state to Joined.
		 */
		sms_log(mac_ctx, LOGW, FL("receives silently stop roam ind"));
		csr_roam_state_change(mac_ctx, eCSR_ROAMING_STATE_JOINED,
			session_id);
		csr_roam_substate_change(mac_ctx, eCSR_ROAM_SUBSTATE_NONE,
			session_id);
		qdf_mem_set(&roam_info, sizeof(tCsrRoamInfo), 0);
		roam_info.pBssDesc = session->pConnectBssDesc;
		if (roam_info.pBssDesc)
			qdf_mem_copy(&roam_info.bssid,
				&roam_info.pBssDesc->bssId,
				sizeof(struct qdf_mac_addr));
		/*
		 * Since there is no change in the current state, simply pass
		 * back no result otherwise HDD may be mistakenly mark to
		 * disconnected state.
		 */
		csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				eCSR_ROAM_IBSS_IND, eCSR_ROAM_RESULT_NONE);
		break;
	case eCsrSilentlyStopRoamingSaveState:
		/* We are here because we try to connect to the same AP */
		/* No message to PE */
		sms_log(mac_ctx, LOGW,
			FL("receives silently stop roaming indication"));
		qdf_mem_set(&roam_info, sizeof(roam_info), 0);

		/* to aviod resetting the substate to NONE */
		mac_ctx->roam.curState[session_id] = eCSR_ROAMING_STATE_JOINED;
		/*
		 * No need to change substate to wai_for_key because there
		 * is no state change
		 */
		roam_info.pBssDesc = session->pConnectBssDesc;
		if (roam_info.pBssDesc)
			qdf_mem_copy(&roam_info.bssid,
				&roam_info.pBssDesc->bssId,
				sizeof(struct qdf_mac_addr));
		roam_info.statusCode = session->joinFailStatusCode.statusCode;
		roam_info.reasonCode = session->joinFailStatusCode.reasonCode;
		roam_info.nBeaconLength = session->connectedInfo.nBeaconLength;
		roam_info.nAssocReqLength =
			session->connectedInfo.nAssocReqLength;
		roam_info.nAssocRspLength =
			session->connectedInfo.nAssocRspLength;
		roam_info.pbFrames = session->connectedInfo.pbFrames;
		roam_info.staId = session->connectedInfo.staId;
		roam_info.u.pConnectedProfile = &session->connectedProfile;
		if (0 == roam_info.staId)
			QDF_ASSERT(0);

		session->bRefAssocStartCnt--;
		csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				eCSR_ROAM_ASSOCIATION_COMPLETION,
				eCSR_ROAM_RESULT_ASSOCIATED);
		csr_roam_completion(mac_ctx, session_id, NULL, cmd,
				eCSR_ROAM_RESULT_ASSOCIATED, true);
		break;
	case eCsrReassocFailure:
		/*
		* Currently Reassoc failure is handled through eCsrJoinFailure
		* Need to revisit for eCsrReassocFailure handling
		*/
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		sme_qos_csr_event_ind(mac_ctx, (uint8_t) session_id,
				SME_QOS_CSR_REASSOC_FAILURE, NULL);
#endif
		break;
	case eCsrStopBssSuccess:
		if (CSR_IS_NDI(profile)) {
			csr_roam_update_ndp_return_params(mac_ctx, res,
				&roam_status, &roam_result, &roam_info);
			csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				roam_status, roam_result);
		}
		break;
	case eCsrStopBssFailure:
		if (CSR_IS_NDI(profile)) {
			csr_roam_update_ndp_return_params(mac_ctx, res,
				&roam_status, &roam_result, &roam_info);
			csr_roam_call_callback(mac_ctx, session_id, &roam_info,
				cmd->u.roamCmd.roamId,
				roam_status, roam_result);
		}
		break;
	case eCsrJoinFailure:
	case eCsrNothingToJoin:
	case eCsrJoinFailureDueToConcurrency:
	default:
		csr_roam_process_results_default(mac_ctx, cmd, context, res);
		break;
	}
	return release_cmd;
}

QDF_STATUS csr_roam_copy_profile(tpAniSirGlobal pMac,
				 tCsrRoamProfile *pDstProfile,
				 tCsrRoamProfile *pSrcProfile)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t size = 0;

	qdf_mem_set(pDstProfile, sizeof(tCsrRoamProfile), 0);
	if (pSrcProfile->BSSIDs.numOfBSSIDs) {
		size = sizeof(struct qdf_mac_addr) * pSrcProfile->BSSIDs.numOfBSSIDs;
		pDstProfile->BSSIDs.bssid = qdf_mem_malloc(size);
		if (NULL == pDstProfile->BSSIDs.bssid) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->BSSIDs.numOfBSSIDs =
			pSrcProfile->BSSIDs.numOfBSSIDs;
		qdf_mem_copy(pDstProfile->BSSIDs.bssid,
			pSrcProfile->BSSIDs.bssid, size);
	}
	if (pSrcProfile->SSIDs.numOfSSIDs) {
		size = sizeof(tCsrSSIDInfo) * pSrcProfile->SSIDs.numOfSSIDs;
		pDstProfile->SSIDs.SSIDList = qdf_mem_malloc(size);
		if (NULL == pDstProfile->SSIDs.SSIDList) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->SSIDs.numOfSSIDs =
			pSrcProfile->SSIDs.numOfSSIDs;
		qdf_mem_copy(pDstProfile->SSIDs.SSIDList,
			pSrcProfile->SSIDs.SSIDList, size);
	}
	if (pSrcProfile->nWPAReqIELength) {
		pDstProfile->pWPAReqIE =
			qdf_mem_malloc(pSrcProfile->nWPAReqIELength);
		if (NULL == pDstProfile->pWPAReqIE) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->nWPAReqIELength =
			pSrcProfile->nWPAReqIELength;
		qdf_mem_copy(pDstProfile->pWPAReqIE, pSrcProfile->pWPAReqIE,
			pSrcProfile->nWPAReqIELength);
	}
	if (pSrcProfile->nRSNReqIELength) {
		pDstProfile->pRSNReqIE =
			qdf_mem_malloc(pSrcProfile->nRSNReqIELength);
		if (NULL == pDstProfile->pRSNReqIE) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->nRSNReqIELength =
			pSrcProfile->nRSNReqIELength;
		qdf_mem_copy(pDstProfile->pRSNReqIE, pSrcProfile->pRSNReqIE,
			pSrcProfile->nRSNReqIELength);
	}
#ifdef FEATURE_WLAN_WAPI
	if (pSrcProfile->nWAPIReqIELength) {
		pDstProfile->pWAPIReqIE =
			qdf_mem_malloc(pSrcProfile->nWAPIReqIELength);
		if (NULL == pDstProfile->pWAPIReqIE) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->nWAPIReqIELength =
			pSrcProfile->nWAPIReqIELength;
		qdf_mem_copy(pDstProfile->pWAPIReqIE, pSrcProfile->pWAPIReqIE,
			pSrcProfile->nWAPIReqIELength);
	}
#endif /* FEATURE_WLAN_WAPI */
	if (pSrcProfile->nAddIEScanLength) {
		pDstProfile->pAddIEScan =
			qdf_mem_malloc(pSrcProfile->nAddIEScanLength);
		if (NULL == pDstProfile->pAddIEScan) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->nAddIEScanLength =
			pSrcProfile->nAddIEScanLength;
		qdf_mem_copy(pDstProfile->pAddIEScan, pSrcProfile->pAddIEScan,
			pSrcProfile->nAddIEScanLength);
	}
	if (pSrcProfile->nAddIEAssocLength) {
		pDstProfile->pAddIEAssoc =
			qdf_mem_malloc(pSrcProfile->nAddIEAssocLength);
		if (NULL == pDstProfile->pAddIEAssoc) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->nAddIEAssocLength =
			pSrcProfile->nAddIEAssocLength;
		qdf_mem_copy(pDstProfile->pAddIEAssoc, pSrcProfile->pAddIEAssoc,
			pSrcProfile->nAddIEAssocLength);
	}
	if (pSrcProfile->ChannelInfo.ChannelList) {
		pDstProfile->ChannelInfo.ChannelList =
			qdf_mem_malloc(pSrcProfile->ChannelInfo.
					numOfChannels);
		if (NULL == pDstProfile->ChannelInfo.ChannelList) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		pDstProfile->ChannelInfo.numOfChannels =
			pSrcProfile->ChannelInfo.numOfChannels;
		qdf_mem_copy(pDstProfile->ChannelInfo.ChannelList,
			pSrcProfile->ChannelInfo.ChannelList,
			pSrcProfile->ChannelInfo.numOfChannels);
	}
	pDstProfile->AuthType = pSrcProfile->AuthType;
	pDstProfile->EncryptionType = pSrcProfile->EncryptionType;
	pDstProfile->mcEncryptionType = pSrcProfile->mcEncryptionType;
	pDstProfile->negotiatedUCEncryptionType =
		pSrcProfile->negotiatedUCEncryptionType;
	pDstProfile->negotiatedMCEncryptionType =
		pSrcProfile->negotiatedMCEncryptionType;
	pDstProfile->negotiatedAuthType = pSrcProfile->negotiatedAuthType;
#ifdef WLAN_FEATURE_11W
	pDstProfile->MFPEnabled = pSrcProfile->MFPEnabled;
	pDstProfile->MFPRequired = pSrcProfile->MFPRequired;
	pDstProfile->MFPCapable = pSrcProfile->MFPCapable;
#endif
	pDstProfile->BSSType = pSrcProfile->BSSType;
	pDstProfile->phyMode = pSrcProfile->phyMode;
	pDstProfile->csrPersona = pSrcProfile->csrPersona;

#ifdef FEATURE_WLAN_WAPI
	if (csr_is_profile_wapi(pSrcProfile))
		if (pDstProfile->phyMode & eCSR_DOT11_MODE_11n)
			pDstProfile->phyMode &= ~eCSR_DOT11_MODE_11n;
#endif /* FEATURE_WLAN_WAPI */
	pDstProfile->CBMode = pSrcProfile->CBMode;
	pDstProfile->ch_params.ch_width = pSrcProfile->ch_params.ch_width;
	pDstProfile->ch_params.center_freq_seg0 =
		pSrcProfile->ch_params.center_freq_seg0;
	pDstProfile->ch_params.center_freq_seg1 =
		pSrcProfile->ch_params.center_freq_seg1;
	pDstProfile->ch_params.sec_ch_offset =
		pSrcProfile->ch_params.sec_ch_offset;
	/*Save the WPS info */
	pDstProfile->bWPSAssociation = pSrcProfile->bWPSAssociation;
	pDstProfile->bOSENAssociation = pSrcProfile->bOSENAssociation;
	pDstProfile->uapsd_mask = pSrcProfile->uapsd_mask;
	pDstProfile->beaconInterval = pSrcProfile->beaconInterval;
	pDstProfile->privacy = pSrcProfile->privacy;
	pDstProfile->fwdWPSPBCProbeReq = pSrcProfile->fwdWPSPBCProbeReq;
	pDstProfile->csr80211AuthType = pSrcProfile->csr80211AuthType;
	pDstProfile->dtimPeriod = pSrcProfile->dtimPeriod;
	pDstProfile->ApUapsdEnable = pSrcProfile->ApUapsdEnable;
	pDstProfile->SSIDs.SSIDList[0].ssidHidden =
		pSrcProfile->SSIDs.SSIDList[0].ssidHidden;
	pDstProfile->protEnabled = pSrcProfile->protEnabled;
	pDstProfile->obssProtEnabled = pSrcProfile->obssProtEnabled;
	pDstProfile->cfg_protection = pSrcProfile->cfg_protection;
	pDstProfile->wps_state = pSrcProfile->wps_state;
	pDstProfile->ieee80211d = pSrcProfile->ieee80211d;
	pDstProfile->sap_dot11mc = pSrcProfile->sap_dot11mc;
	pDstProfile->do_not_roam = pSrcProfile->do_not_roam;
	qdf_mem_copy(&pDstProfile->Keys, &pSrcProfile->Keys,
		sizeof(pDstProfile->Keys));
#ifdef WLAN_FEATURE_11W
	pDstProfile->MFPEnabled = pSrcProfile->MFPEnabled;
	pDstProfile->MFPRequired = pSrcProfile->MFPRequired;
	pDstProfile->MFPCapable = pSrcProfile->MFPCapable;
#endif
	if (pSrcProfile->MDID.mdiePresent) {
		pDstProfile->MDID.mdiePresent = 1;
		pDstProfile->MDID.mobilityDomain =
			pSrcProfile->MDID.mobilityDomain;
	}
	qdf_mem_copy(&pDstProfile->addIeParams, &pSrcProfile->addIeParams,
			sizeof(tSirAddIeParams));
end:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		csr_release_profile(pMac, pDstProfile);
		pDstProfile = NULL;
	}

	return status;
}

QDF_STATUS csr_roam_copy_connected_profile(tpAniSirGlobal pMac,
			uint32_t sessionId, tCsrRoamProfile *pDstProfile)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamConnectedProfile *pSrcProfile =
		&pMac->roam.roamSession[sessionId].connectedProfile;

	qdf_mem_set(pDstProfile, sizeof(tCsrRoamProfile), 0);

	pDstProfile->BSSIDs.bssid = qdf_mem_malloc(sizeof(struct qdf_mac_addr));
	if (NULL == pDstProfile->BSSIDs.bssid) {
		status = QDF_STATUS_E_NOMEM;
		sms_log(pMac, LOGE,
			FL("failed to allocate memory for BSSID "
			MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(pSrcProfile->bssid.bytes));
		goto end;
	}
	pDstProfile->BSSIDs.numOfBSSIDs = 1;
	qdf_copy_macaddr(pDstProfile->BSSIDs.bssid, &pSrcProfile->bssid);

	if (pSrcProfile->SSID.ssId) {
		pDstProfile->SSIDs.SSIDList =
			qdf_mem_malloc(sizeof(tCsrSSIDInfo));
		if (NULL == pDstProfile->SSIDs.SSIDList) {
			status = QDF_STATUS_E_NOMEM;
			sms_log(pMac, LOGE,
				FL("failed to allocate memory for SSID "
				MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(pSrcProfile->bssid.bytes));
			goto end;
		}
		pDstProfile->SSIDs.numOfSSIDs = 1;
		pDstProfile->SSIDs.SSIDList[0].handoffPermitted =
			pSrcProfile->handoffPermitted;
		pDstProfile->SSIDs.SSIDList[0].ssidHidden =
			pSrcProfile->ssidHidden;
		qdf_mem_copy(&pDstProfile->SSIDs.SSIDList[0].SSID,
			&pSrcProfile->SSID, sizeof(tSirMacSSid));
	}
	if (pSrcProfile->nAddIEAssocLength) {
		pDstProfile->pAddIEAssoc =
			qdf_mem_malloc(pSrcProfile->nAddIEAssocLength);
		if (NULL == pDstProfile->pAddIEAssoc) {
			status = QDF_STATUS_E_NOMEM;
			sms_log(pMac, LOGE,
				FL("failed to allocate mem for additional ie"));
			goto end;
		}
		pDstProfile->nAddIEAssocLength = pSrcProfile->nAddIEAssocLength;
		qdf_mem_copy(pDstProfile->pAddIEAssoc, pSrcProfile->pAddIEAssoc,
			pSrcProfile->nAddIEAssocLength);
	}
	pDstProfile->ChannelInfo.ChannelList = qdf_mem_malloc(1);
	if (NULL == pDstProfile->ChannelInfo.ChannelList) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	pDstProfile->ChannelInfo.numOfChannels = 1;
	pDstProfile->ChannelInfo.ChannelList[0] = pSrcProfile->operationChannel;
	pDstProfile->AuthType.numEntries = 1;
	pDstProfile->AuthType.authType[0] = pSrcProfile->AuthType;
	pDstProfile->negotiatedAuthType = pSrcProfile->AuthType;
	pDstProfile->EncryptionType.numEntries = 1;
	pDstProfile->EncryptionType.encryptionType[0] =
		pSrcProfile->EncryptionType;
	pDstProfile->negotiatedUCEncryptionType =
		pSrcProfile->EncryptionType;
	pDstProfile->mcEncryptionType.numEntries = 1;
	pDstProfile->mcEncryptionType.encryptionType[0] =
		pSrcProfile->mcEncryptionType;
	pDstProfile->negotiatedMCEncryptionType =
		pSrcProfile->mcEncryptionType;
	pDstProfile->BSSType = pSrcProfile->BSSType;
	pDstProfile->CBMode = pSrcProfile->CBMode;
	qdf_mem_copy(&pDstProfile->Keys, &pSrcProfile->Keys,
		sizeof(pDstProfile->Keys));
	if (pSrcProfile->MDID.mdiePresent) {
		pDstProfile->MDID.mdiePresent = 1;
		pDstProfile->MDID.mobilityDomain =
			pSrcProfile->MDID.mobilityDomain;
	}
#ifdef WLAN_FEATURE_11W
	pDstProfile->MFPEnabled = pSrcProfile->MFPEnabled;
	pDstProfile->MFPRequired = pSrcProfile->MFPRequired;
	pDstProfile->MFPCapable = pSrcProfile->MFPCapable;
#endif

end:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		csr_release_profile(pMac, pDstProfile);
		pDstProfile = NULL;
	}

	return status;
}

QDF_STATUS csr_roam_issue_connect(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				  tScanResultHandle hBSSList,
				  eCsrRoamReason reason, uint32_t roamId,
				  bool fImediate, bool fClearScan)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;

	pCommand = csr_get_command_buffer(pMac);
	if (NULL == pCommand) {
		sms_log(pMac, LOGE, FL(" fail to get command buffer"));
		status = QDF_STATUS_E_RESOURCES;
	} else {
		if (fClearScan) {
			csr_scan_abort_mac_scan_not_for_connect(pMac, sessionId);
		}
		pCommand->u.roamCmd.fReleaseProfile = false;
		if (NULL == pProfile) {
			/* We can roam now */
			/* Since pProfile is NULL, we need to build our own profile, set everything to default */
			/* We can only support open and no encryption */
			pCommand->u.roamCmd.roamProfile.AuthType.numEntries = 1;
			pCommand->u.roamCmd.roamProfile.AuthType.authType[0] =
				eCSR_AUTH_TYPE_OPEN_SYSTEM;
			pCommand->u.roamCmd.roamProfile.EncryptionType.
			numEntries = 1;
			pCommand->u.roamCmd.roamProfile.EncryptionType.
			encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
			pCommand->u.roamCmd.roamProfile.csrPersona =
				QDF_STA_MODE;
		} else {
			/* make a copy of the profile */
			status =
				csr_roam_copy_profile(pMac,
						      &pCommand->u.roamCmd.roamProfile,
						      pProfile);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				pCommand->u.roamCmd.fReleaseProfile = true;
			}
		}

		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.hBSSList = hBSSList;
		pCommand->u.roamCmd.roamId = roamId;
		pCommand->u.roamCmd.roamReason = reason;
		/* We need to free the BssList when the command is done */
		pCommand->u.roamCmd.fReleaseBssList = true;
		pCommand->u.roamCmd.fUpdateCurRoamProfile = true;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("CSR PERSONA=%d"),
			  pCommand->u.roamCmd.roamProfile.csrPersona);
		status = csr_queue_sme_command(pMac, pCommand, fImediate);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_release_command_roam(pMac, pCommand);
		}
	}

	return status;
}

QDF_STATUS csr_roam_issue_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				  tCsrRoamModifyProfileFields *pMmodProfileFields,
				  eCsrRoamReason reason, uint32_t roamId,
				  bool fImediate)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;

	pCommand = csr_get_command_buffer(pMac);
	if (NULL == pCommand) {
		sms_log(pMac, LOGE, FL(" fail to get command buffer"));
		status = QDF_STATUS_E_RESOURCES;
	} else {
		csr_scan_abort_mac_scan_not_for_connect(pMac, sessionId);
		if (pProfile) {
			/* This is likely trying to reassoc to different profile */
			pCommand->u.roamCmd.fReleaseProfile = false;
			/* make a copy of the profile */
			status =
				csr_roam_copy_profile(pMac,
						      &pCommand->u.roamCmd.roamProfile,
						      pProfile);
			pCommand->u.roamCmd.fUpdateCurRoamProfile = true;
		} else {
			status =
				csr_roam_copy_connected_profile(pMac, sessionId,
								&pCommand->u.roamCmd.
								roamProfile);
			/* how to update WPA/WPA2 info in roamProfile?? */
			pCommand->u.roamCmd.roamProfile.uapsd_mask =
				pMmodProfileFields->uapsd_mask;
		}
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pCommand->u.roamCmd.fReleaseProfile = true;
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.roamId = roamId;
		pCommand->u.roamCmd.roamReason = reason;
		/* We need to free the BssList when the command is done */
		/* For reassoc there is no BSS list, so the bool set to false */
		pCommand->u.roamCmd.hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
		pCommand->u.roamCmd.fReleaseBssList = false;
		pCommand->u.roamCmd.fReassoc = true;
		csr_roam_remove_duplicate_command(pMac, sessionId, pCommand,
						  reason);
		status = csr_queue_sme_command(pMac, pCommand, fImediate);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_roam_completion(pMac, sessionId, NULL, pCommand,
					    eCSR_ROAM_RESULT_FAILURE, false);
			csr_release_command_roam(pMac, pCommand);
		}
	}
	return status;
}

QDF_STATUS csr_dequeue_roam_command(tpAniSirGlobal pMac, eCsrRoamReason reason)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if ((eSmeCommandRoam == pCommand->command) &&
		    (eCsrPerformPreauth == reason)) {
			sms_log(pMac, LOG1, FL("DQ-Command = %d, Reason = %d"),
				pCommand->command,
				pCommand->u.roamCmd.roamReason);
			if (csr_ll_remove_entry
				    (&pMac->sme.smeCmdActiveList, pEntry,
				    LL_ACCESS_LOCK)) {
				csr_release_command_preauth(pMac, pCommand);
			}
		} else if ((eSmeCommandRoam == pCommand->command) &&
			   (eCsrSmeIssuedFTReassoc == reason)) {
			sms_log(pMac, LOG1, FL("DQ-Command = %d, Reason = %d"),
				pCommand->command,
				pCommand->u.roamCmd.roamReason);
			if (csr_ll_remove_entry
				    (&pMac->sme.smeCmdActiveList, pEntry,
				    LL_ACCESS_LOCK)) {
				csr_release_command_roam(pMac, pCommand);
			}
		} else {
			sms_log(pMac, LOGE, FL("Command = %d, Reason = %d "),
				pCommand->command,
				pCommand->u.roamCmd.roamReason);
		}
	} else {
		sms_log(pMac, LOGE,
			FL("pEntry NULL for eWNI_SME_FT_PRE_AUTH_RSP"));
	}
	sme_process_pending_queue(pMac);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_connect(tpAniSirGlobal pMac, uint32_t sessionId,
		tCsrRoamProfile *pProfile,
		uint32_t *pRoamId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tScanResultHandle hBSSList;
	tCsrScanResultFilter *pScanFilter;
	uint32_t roamId = 0;
	bool fCallCallback = false;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tSirBssDescription first_ap_profile;

	if (NULL == pSession) {
		sms_log(pMac, LOGE,
			FL("session does not exist for given sessionId:%d"),
			sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (NULL == pProfile) {
		sms_log(pMac, LOGP, FL("No profile specified"));
		return QDF_STATUS_E_FAILURE;
	}
	/* Initialize the count before proceeding with the Join requests */
	pSession->join_bssid_count = 0;
	sms_log(pMac, LOG1,
		FL("called  BSSType = %s (%d) authtype = %d  encryType = %d"),
		sme_bss_type_to_string(pProfile->BSSType),
		pProfile->BSSType, pProfile->AuthType.authType[0],
		pProfile->EncryptionType.encryptionType[0]);
	/* Reset dhcp_done for the fresh connection */
	pSession->dhcp_done = false;
	csr_roam_cancel_roaming(pMac, sessionId);
	csr_scan_remove_fresh_scan_command(pMac, sessionId);
	/* Only abort the scan if its not used for other roam/connect purpose */
	csr_scan_abort_mac_scan(pMac, sessionId, eCSR_SCAN_ABORT_DEFAULT);
	csr_roam_remove_duplicate_command(pMac, sessionId, NULL, eCsrHddIssued);
	/* Check whether ssid changes */
	if (csr_is_conn_state_connected(pMac, sessionId) &&
		pProfile->SSIDs.numOfSSIDs && !csr_is_ssid_in_list(pMac,
			&pSession->connectedProfile.SSID, &pProfile->SSIDs))
			csr_roam_issue_disassociate_cmd(pMac, sessionId,
					eCSR_DISCONNECT_REASON_UNSPECIFIED);
	/*
	 * If roamSession.connectState is disconnecting that mean
	 * disconnect was received with scan for ssid in progress
	 * and dropped. This state will ensure that connect will
	 * not be issued from scan for ssid completion. Thus
	 * if this fresh connect also issue scan for ssid the connect
	 * command will be dropped assuming disconnect is in progress.
	 * Thus reset connectState here
	 */
	if (eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTING ==
		pMac->roam.roamSession[sessionId].connectState)
			pMac->roam.roamSession[sessionId].connectState =
				eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
#ifdef FEATURE_WLAN_BTAMP_UT_RF
	pSession->maxRetryCount = CSR_JOIN_MAX_RETRY_COUNT;
#endif
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	qdf_mem_set(pScanFilter, sizeof(tCsrScanResultFilter),
			0);
	/* Try to connect to any BSS */
	if (NULL == pProfile) {
		/* No encryption */
		pScanFilter->EncryptionType.numEntries = 1;
		pScanFilter->EncryptionType.encryptionType[0] =
			eCSR_ENCRYPT_TYPE_NONE;
	} else {
		/* Here is the profile we need to connect to */
		status = csr_roam_prepare_filter_from_profile(pMac,
				pProfile, pScanFilter);
	}
	roamId = GET_NEXT_ROAM_ID(&pMac->roam);
	if (pRoamId)
		*pRoamId = roamId;
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_free(pScanFilter);
		goto end;
	}

	/*Save the WPS info */
	if (NULL != pProfile) {
		pScanFilter->bWPSAssociation =
			pProfile->bWPSAssociation;
		pScanFilter->bOSENAssociation =
			pProfile->bOSENAssociation;
	} else {
		pScanFilter->bWPSAssociation = 0;
		pScanFilter->bOSENAssociation = 0;
	}
	if (pProfile && CSR_IS_INFRA_AP(pProfile)) {
		/* This can be started right away */
		status = csr_roam_issue_connect(pMac, sessionId, pProfile, NULL,
				 eCsrHddIssued, roamId, false, false);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL("CSR failed to issue start BSS cmd with status = 0x%08X"),
				status);
			fCallCallback = true;
		} else {
			sms_log(pMac, LOG1,
				FL("Connect request to proceed for sap mode"));
		}

		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
		goto end;
	}
	status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
	sms_log(pMac, LOG1,
		FL("******* csr_scan_get_result Status ****** %d"), status);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* check if set hw mode needs to be done */
		if ((pScanFilter->csrPersona == QDF_STA_MODE) ||
			 (pScanFilter->csrPersona == QDF_P2P_CLIENT_MODE)) {
			csr_get_bssdescr_from_scan_handle(hBSSList,
					&first_ap_profile);
			status = cds_handle_conc_multiport(sessionId,
						first_ap_profile.channelId);
			if ((QDF_IS_STATUS_SUCCESS(status)) &&
				(!csr_wait_for_connection_update(pMac, true))) {
					sms_log(pMac, LOG1,
						FL("conn update error"));
					csr_scan_result_purge(pMac, hBSSList);
					fCallCallback = true;
					goto error;
			} else if (status == QDF_STATUS_E_FAILURE) {
				sms_log(pMac, LOG1, FL("conn update error"));
				csr_scan_result_purge(pMac, hBSSList);
				fCallCallback = true;
				goto error;
			}
		}

		status = csr_roam_issue_connect(pMac, sessionId, pProfile,
				hBSSList, eCsrHddIssued, roamId, false, false);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL("CSR failed to issue connect cmd with status = 0x%08X"),
				status);
			csr_scan_result_purge(pMac, hBSSList);
			fCallCallback = true;
		}
	} else if (NULL != pProfile) {
		/* Check whether it is for start ibss */
		if (CSR_IS_START_IBSS(pProfile) ||
		    CSR_IS_NDI(pProfile)) {
			status = csr_roam_issue_connect(pMac, sessionId,
					pProfile, NULL, eCsrHddIssued,
					roamId, false, false);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL("Failed with status = 0x%08X"),
					status);
				fCallCallback = true;
			}
		} else {
			/* scan for this SSID */
			status = csr_scan_for_ssid(pMac, sessionId, pProfile,
						roamId, true);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL("CSR failed to issue SSID scan cmd with status = 0x%08X"),
					status);
				fCallCallback = true;
			} else {
				sms_log(pMac, LOG1,
					FL("SSID scan requested"));
			}
		}
	} else {
		fCallCallback = true;
	}

error:
	if (NULL != pProfile)
		/*
		 * we need to free memory for filter
		 * if profile exists
		 */
		csr_free_scan_filter(pMac, pScanFilter);

	qdf_mem_free(pScanFilter);
end:
	/* tell the caller if we fail to trigger a join request */
	if (fCallCallback) {
		csr_roam_call_callback(pMac, sessionId, NULL, roamId,
				eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
	}
	return status;
}

/**
 * csr_roam_reassoc() - process reassoc command
 * @mac_ctx:       mac global context
 * @session_id:    session id
 * @profile:       roam profile
 * @mod_fields:    AC info being modified in reassoc
 * @roam_id:       roam id to be populated
 *
 * Return: status of operation
 */
QDF_STATUS
csr_roam_reassoc(tpAniSirGlobal mac_ctx, uint32_t session_id,
		 tCsrRoamProfile *profile,
		 tCsrRoamModifyProfileFields mod_fields,
		 uint32_t *roam_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool fCallCallback = true;
	uint32_t roamId = 0;

	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == profile) {
		sms_log(mac_ctx, LOGP, FL("No profile specified"));
		return QDF_STATUS_E_FAILURE;
	}
	sms_log(mac_ctx, LOG1,
		FL("called  BSSType = %s (%d) authtype = %d  encryType = %d"),
		sme_bss_type_to_string(profile->BSSType),
		profile->BSSType, profile->AuthType.authType[0],
		profile->EncryptionType.encryptionType[0]);
	csr_roam_cancel_roaming(mac_ctx, session_id);
	csr_scan_remove_fresh_scan_command(mac_ctx, session_id);
	csr_scan_abort_mac_scan_not_for_connect(mac_ctx, session_id);
	csr_roam_remove_duplicate_command(mac_ctx, session_id, NULL,
					  eCsrHddIssuedReassocToSameAP);
	if (csr_is_conn_state_connected(mac_ctx, session_id)) {
		if (profile) {
			if (profile->SSIDs.numOfSSIDs &&
			    csr_is_ssid_in_list(mac_ctx,
						&session->connectedProfile.SSID,
						&profile->SSIDs)) {
				fCallCallback = false;
			} else {
				/*
				 * Connected SSID did not match with what is
				 * asked in profile
				 */
				sms_log(mac_ctx, LOG1, FL("SSID mismatch"));
			}
		} else if (qdf_mem_cmp(&mod_fields,
				&session->connectedProfile.modifyProfileFields,
				sizeof(tCsrRoamModifyProfileFields))) {
			fCallCallback = false;
		} else {
			sms_log(mac_ctx, LOG1,
				/*
				 * Either the profile is NULL or none of the
				 * fields in tCsrRoamModifyProfileFields got
				 * modified
				 */
				FL("Profile NULL or nothing to modify."));
		}
	} else {
		sms_log(mac_ctx, LOG1, FL("Not connected! No need to reassoc"));
	}
	if (!fCallCallback) {
		roamId = GET_NEXT_ROAM_ID(&mac_ctx->roam);
		if (roam_id)
			*roam_id = roamId;
		status = csr_roam_issue_reassoc(mac_ctx, session_id, profile,
				&mod_fields, eCsrHddIssuedReassocToSameAP,
				roamId, false);
	} else {
		status = csr_roam_call_callback(mac_ctx, session_id, NULL,
						roamId, eCSR_ROAM_FAILED,
						eCSR_ROAM_RESULT_FAILURE);
	}
	return status;
}

QDF_STATUS csr_roam_join_last_profile(tpAniSirGlobal pMac, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultHandle hBSSList = NULL;
	tCsrScanResultFilter *pScanFilter = NULL;
	uint32_t roamId;
	tCsrRoamProfile *pProfile = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->pCurRoamProfile) {
		csr_scan_abort_mac_scan_not_for_connect(pMac, sessionId);
		/* We have to make a copy of pCurRoamProfile because it
		 * will be free inside csr_roam_issue_connect */
		pProfile = qdf_mem_malloc(sizeof(tCsrRoamProfile));
		if (NULL == pProfile) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		qdf_mem_set(pProfile, sizeof(tCsrRoamProfile), 0);
		status = csr_roam_copy_profile(pMac, pProfile,
			pSession->pCurRoamProfile);
		if (!QDF_IS_STATUS_SUCCESS(status))
			goto end;
		pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
		if (NULL == pScanFilter) {
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		qdf_mem_set(pScanFilter, sizeof(tCsrScanResultFilter), 0);
		status = csr_roam_prepare_filter_from_profile(pMac, pProfile,
					pScanFilter);
		if (!QDF_IS_STATUS_SUCCESS(status))
			goto end;
		roamId = GET_NEXT_ROAM_ID(&pMac->roam);
		status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* we want to put the last connected BSS to the
			 * very beginning, if possible */
			csr_move_bss_to_head_from_bssid(pMac,
				&pSession->connectedProfile.bssid, hBSSList);
			status = csr_roam_issue_connect(pMac, sessionId,
					pProfile, hBSSList, eCsrHddIssued,
					roamId, false, false);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				csr_scan_result_purge(pMac, hBSSList);
				goto end;
			}
		} else {
			/* scan for this SSID only incase AP suppresses SSID */
			status = csr_scan_for_ssid(pMac, sessionId, pProfile,
					roamId, true);
			if (!QDF_IS_STATUS_SUCCESS(status))
				goto end;
		}
	} /* We have a profile */
	else {
		sms_log(pMac, LOGW, FL("cannot find a roaming profile"));
		goto end;
	}
end:
	if (pScanFilter) {
		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
	}
	if (NULL != pProfile) {
		csr_release_profile(pMac, pProfile);
		qdf_mem_free(pProfile);
	}
	return status;
}

QDF_STATUS csr_roam_reconnect(tpAniSirGlobal pMac, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	if (csr_is_conn_state_connected(pMac, sessionId)) {
		status =
			csr_roam_issue_disassociate_cmd(pMac, sessionId,
							eCSR_DISCONNECT_REASON_UNSPECIFIED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = csr_roam_join_last_profile(pMac, sessionId);
		}
	}
	return status;
}

QDF_STATUS csr_roam_connect_to_last_profile(tpAniSirGlobal pMac, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	sms_log(pMac, LOGW, FL("is called"));
	csr_roam_cancel_roaming(pMac, sessionId);
	csr_roam_remove_duplicate_command(pMac, sessionId, NULL, eCsrHddIssued);
	if (csr_is_conn_state_disconnected(pMac, sessionId)) {
		status = csr_roam_join_last_profile(pMac, sessionId);
	}
	return status;
}

QDF_STATUS csr_roam_process_disassoc_deauth(tpAniSirGlobal pMac, tSmeCmd *pCommand,
					    bool fDisassoc, bool fMICFailure)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool fComplete = false;
	eCsrRoamSubState NewSubstate;
	uint32_t sessionId = pCommand->sessionId;

	if (CSR_IS_WAIT_FOR_KEY(pMac, sessionId)) {
		sms_log(pMac, LOG1,
			FL(" Stop Wait for key timer and change substate to"
			   " eCSR_ROAM_SUBSTATE_NONE"));
		csr_roam_stop_wait_for_key_timer(pMac);
		csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE, sessionId);
	}
	/* change state to 'Roaming'... */
	csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING, sessionId);

	if (csr_is_conn_state_ibss(pMac, sessionId)) {
		/* If we are in an IBSS, then stop the IBSS... */
		status =
			csr_roam_issue_stop_bss(pMac, sessionId,
						eCSR_ROAM_SUBSTATE_STOP_BSS_REQ);
		fComplete = (!QDF_IS_STATUS_SUCCESS(status));
	} else if (csr_is_conn_state_infra(pMac, sessionId)) {
		/*
		 * in Infrastructure, we need to disassociate from the
		 * Infrastructure network...
		 */
		NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_FORCED;
		if (eCsrSmeIssuedDisassocForHandoff ==
		    pCommand->u.roamCmd.roamReason) {
			NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF;
		} else
		if ((eCsrForcedDisassoc == pCommand->u.roamCmd.roamReason)
		    && (eSIR_MAC_DISASSOC_LEAVING_BSS_REASON ==
			pCommand->u.roamCmd.reason)) {
			NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_STA_HAS_LEFT;
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  FL
					  ("set to substate eCSR_ROAM_SUBSTATE_DISASSOC_STA_HAS_LEFT"));
		}
		if (eCsrSmeIssuedDisassocForHandoff !=
				pCommand->u.roamCmd.roamReason) {
			/*
			 * If we are in neighbor preauth done state then
			 * on receiving disassoc or deauth we dont roam
			 * instead we just disassoc from current ap and
			 * then go to disconnected state.
			 * This happens for ESE and 11r FT connections ONLY.
			 */
			if (csr_roam_is11r_assoc(pMac, sessionId) &&
				(csr_neighbor_roam_state_preauth_done(pMac,
							sessionId))) {
				csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							pMac, sessionId);
			}
#ifdef FEATURE_WLAN_ESE
			if (csr_roam_is_ese_assoc(pMac, sessionId) &&
				(csr_neighbor_roam_state_preauth_done(pMac,
							sessionId))) {
				csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							pMac, sessionId);
			}
#endif
			if (csr_roam_is_fast_roam_enabled(pMac, sessionId) &&
				(csr_neighbor_roam_state_preauth_done(pMac,
							sessionId))) {
				csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							pMac, sessionId);
			}
		}
		if (fDisassoc) {
			status =
				csr_roam_issue_disassociate(pMac, sessionId,
							    NewSubstate, fMICFailure);
		} else {
			status =
				csr_roam_issue_deauth(pMac, sessionId,
						      eCSR_ROAM_SUBSTATE_DEAUTH_REQ);
		}
		fComplete = (!QDF_IS_STATUS_SUCCESS(status));
	} else {
		/* we got a dis-assoc request while not connected to any peer */
		/* just complete the command */
		fComplete = true;
		status = QDF_STATUS_E_FAILURE;
	}
	if (fComplete) {
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
	}

	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (csr_is_conn_state_infra(pMac, sessionId)) {
			/* Set the state to disconnect here */
			pMac->roam.roamSession[sessionId].connectState =
				eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
		}
	} else {
		sms_log(pMac, LOGW, FL(" failed with status %d"), status);
	}
	return status;
}

/**
 * csr_prepare_disconnect_command() - function to prepare disconnect command
 * @mac: pointer to global mac structure
 * @session_id: sme session index
 * @sme_cmd: pointer to sme command being prepared
 *
 * Function to prepare internal sme disconnect command
 * Return: QDF_STATUS_SUCCESS on success else QDF_STATUS_E_RESOURCES on failure
 */

QDF_STATUS csr_prepare_disconnect_command(tpAniSirGlobal mac,
			uint32_t session_id, tSmeCmd **sme_cmd)
{
	tSmeCmd *command;

	command = csr_get_command_buffer(mac);
	if (!command) {
		sms_log(mac, LOGE, FL("fail to get command buffer"));
		return QDF_STATUS_E_RESOURCES;
	}

	command->command = eSmeCommandRoam;
	command->sessionId = (uint8_t)session_id;
	command->u.roamCmd.roamReason = eCsrForcedDisassoc;

	*sme_cmd = command;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_issue_disassociate_cmd(tpAniSirGlobal pMac, uint32_t sessionId,
					   eCsrRoamDisconnectReason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;
	do {
		pCommand = csr_get_command_buffer(pMac);
		if (!pCommand) {
			sms_log(pMac, LOGE, FL(" fail to get command buffer"));
			status = QDF_STATUS_E_RESOURCES;
			break;
		}
		/* Change the substate in case it is wait-for-key */
		if (CSR_IS_WAIT_FOR_KEY(pMac, sessionId)) {
			csr_roam_stop_wait_for_key_timer(pMac);
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 sessionId);
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		sms_log(pMac, LOG1,
			FL("Disassociate reason: %d, sessionId: %d"),
			reason, sessionId);
		switch (reason) {
		case eCSR_DISCONNECT_REASON_MIC_ERROR:
			pCommand->u.roamCmd.roamReason =
				eCsrForcedDisassocMICFailure;
			break;
		case eCSR_DISCONNECT_REASON_DEAUTH:
			pCommand->u.roamCmd.roamReason = eCsrForcedDeauth;
			break;
		case eCSR_DISCONNECT_REASON_HANDOFF:
			pCommand->u.roamCmd.roamReason =
				eCsrSmeIssuedDisassocForHandoff;
			break;
		case eCSR_DISCONNECT_REASON_UNSPECIFIED:
		case eCSR_DISCONNECT_REASON_DISASSOC:
			pCommand->u.roamCmd.roamReason = eCsrForcedDisassoc;
			break;
		case eCSR_DISCONNECT_REASON_IBSS_JOIN_FAILURE:
			pCommand->u.roamCmd.roamReason =
				eCsrSmeIssuedIbssJoinFailure;
			break;
		case eCSR_DISCONNECT_REASON_IBSS_LEAVE:
			pCommand->u.roamCmd.roamReason = eCsrForcedIbssLeave;
			break;
		case eCSR_DISCONNECT_REASON_STA_HAS_LEFT:
			pCommand->u.roamCmd.roamReason = eCsrForcedDisassoc;
			pCommand->u.roamCmd.reason =
				eSIR_MAC_DISASSOC_LEAVING_BSS_REASON;
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  FL("SME convert to internal reason code eCsrStaHasLeft"));
			break;
		case eCSR_DISCONNECT_REASON_NDI_DELETE:
			pCommand->u.roamCmd.roamReason = eCsrStopBss;
			pCommand->u.roamCmd.roamProfile.BSSType =
				eCSR_BSS_TYPE_NDI;
		default:
			break;
		}
		status = csr_queue_sme_command(pMac, pCommand, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_release_command_roam(pMac, pCommand);
		}
	} while (0);
	return status;
}

QDF_STATUS csr_roam_issue_stop_bss_cmd(tpAniSirGlobal pMac, uint32_t sessionId,
				       bool fHighPriority)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;
	pCommand = csr_get_command_buffer(pMac);
	if (NULL != pCommand) {
		/* Change the substate in case it is wait-for-key */
		if (CSR_IS_WAIT_FOR_KEY(pMac, sessionId)) {
			csr_roam_stop_wait_for_key_timer(pMac);
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 sessionId);
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.roamReason = eCsrStopBss;
		status = csr_queue_sme_command(pMac, pCommand, fHighPriority);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
			csr_release_command_roam(pMac, pCommand);
		}
	} else {
		sms_log(pMac, LOGE, FL(" fail to get command buffer"));
		status = QDF_STATUS_E_RESOURCES;
	}
	return status;
}

QDF_STATUS csr_roam_disconnect_internal(tpAniSirGlobal pMac, uint32_t sessionId,
					eCsrRoamDisconnectReason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
#ifdef FEATURE_WLAN_BTAMP_UT_RF
	/* Stop the retry */
	pSession->maxRetryCount = 0;
	csr_roam_stop_join_retry_timer(pMac, sessionId);
#endif
	/* Not to call cancel roaming here */
	/* Only issue disconnect when necessary */
	if (csr_is_conn_state_connected(pMac, sessionId)
	    || csr_is_bss_type_ibss(pSession->connectedProfile.BSSType)
	    || csr_is_roam_command_waiting_for_session(pMac, sessionId)
	    || CSR_IS_CONN_NDI(&pSession->connectedProfile)) {
		sms_log(pMac, LOG2, FL("called"));
		status = csr_roam_issue_disassociate_cmd(pMac, sessionId,
							 reason);
	} else {
		pMac->roam.roamSession[sessionId].connectState =
			eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTING;
		csr_scan_abort_scan_for_ssid(pMac, sessionId);
		status = QDF_STATUS_CMD_NOT_QUEUED;
		sms_log(pMac, LOG1,
			FL
			(" Disconnect cmd not queued, Roam command is not present"
			" return with status %d"), status);
	}
	return status;
}

QDF_STATUS csr_roam_disconnect(tpAniSirGlobal pMac, uint32_t sessionId,
			       eCsrRoamDisconnectReason reason)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	csr_roam_cancel_roaming(pMac, sessionId);
	csr_roam_remove_duplicate_command(pMac, sessionId, NULL,
					  eCsrForcedDisassoc);

	return csr_roam_disconnect_internal(pMac, sessionId, reason);
}

QDF_STATUS csr_roam_save_connected_infomation(tpAniSirGlobal pMac,
					      uint32_t sessionId,
					      tCsrRoamProfile *pProfile,
					      tSirBssDescription *pSirBssDesc,
					      tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tDot11fBeaconIEs *pIesTemp = pIes;
	uint8_t index;
	tCsrRoamSession *pSession = NULL;
	tCsrRoamConnectedProfile *pConnectProfile = NULL;

	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	pConnectProfile = &pSession->connectedProfile;
	if (pConnectProfile->pAddIEAssoc) {
		qdf_mem_free(pConnectProfile->pAddIEAssoc);
		pConnectProfile->pAddIEAssoc = NULL;
	}
	/*
	 * In case of LFR2.0, the connected profile is copied into a temporary
	 * profile and cleared and then is copied back. This is not needed for
	 * LFR3.0, since the profile is not cleared.
	 */
	if (!pSession->roam_synch_in_progress) {
		qdf_mem_set(&pSession->connectedProfile,
				sizeof(tCsrRoamConnectedProfile), 0);
		pConnectProfile->AuthType = pProfile->negotiatedAuthType;
		pConnectProfile->AuthInfo = pProfile->AuthType;
		pConnectProfile->EncryptionType =
			pProfile->negotiatedUCEncryptionType;
		pConnectProfile->EncryptionInfo = pProfile->EncryptionType;
		pConnectProfile->mcEncryptionType =
			pProfile->negotiatedMCEncryptionType;
		pConnectProfile->mcEncryptionInfo = pProfile->mcEncryptionType;
		pConnectProfile->BSSType = pProfile->BSSType;
		pConnectProfile->modifyProfileFields.uapsd_mask =
			pProfile->uapsd_mask;
		qdf_mem_copy(&pConnectProfile->Keys, &pProfile->Keys,
				sizeof(tCsrKeys));
		if (pProfile->nAddIEAssocLength) {
			pConnectProfile->pAddIEAssoc =
				qdf_mem_malloc(pProfile->nAddIEAssocLength);
			if (NULL == pConnectProfile->pAddIEAssoc)
				status = QDF_STATUS_E_NOMEM;
			else
				status = QDF_STATUS_SUCCESS;
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL("Failed to allocate memory for IE"));
				return QDF_STATUS_E_FAILURE;
			}
			pConnectProfile->nAddIEAssocLength =
				pProfile->nAddIEAssocLength;
			qdf_mem_copy(pConnectProfile->pAddIEAssoc,
					pProfile->pAddIEAssoc,
					pProfile->nAddIEAssocLength);
		}
#ifdef WLAN_FEATURE_11W
		pConnectProfile->MFPEnabled = pProfile->MFPEnabled;
		pConnectProfile->MFPRequired = pProfile->MFPRequired;
		pConnectProfile->MFPCapable = pProfile->MFPCapable;
#endif
	}
	if (pIes)
		pConnectProfile->CBMode = csr_get_cb_mode_from_ies(pMac,
				pSirBssDesc->channelId, pIes);
	else
		sms_log(pMac, LOGE, FL("IE unavailable to derive CB mode"));

	/* Save bssid */
	pConnectProfile->operationChannel = pSirBssDesc->channelId;
	pConnectProfile->beaconInterval = pSirBssDesc->beaconInterval;
	if (!pConnectProfile->beaconInterval) {
		sms_log(pMac, LOGW, FL("ERROR: Beacon interval is ZERO"));
	}
	csr_get_bss_id_bss_desc(pMac, pSirBssDesc, &pConnectProfile->bssid);
	if (pSirBssDesc->mdiePresent) {
		pConnectProfile->MDID.mdiePresent = 1;
		pConnectProfile->MDID.mobilityDomain =
			(pSirBssDesc->mdie[1] << 8) | (pSirBssDesc->mdie[0]);
	}
	if (NULL == pIesTemp) {
		status =
			csr_get_parsed_bss_description_ies(pMac, pSirBssDesc,
							   &pIesTemp);
	}
#ifdef FEATURE_WLAN_ESE
	if ((csr_is_profile_ese(pProfile) ||
	     (QDF_IS_STATUS_SUCCESS(status) && (pIesTemp->ESEVersion.present)
	      && (pProfile->negotiatedAuthType == eCSR_AUTH_TYPE_OPEN_SYSTEM)))
	    && (pMac->roam.configParam.isEseIniFeatureEnabled)) {
		pConnectProfile->isESEAssoc = 1;
	}
#endif
	/* save ssid */
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (pIesTemp->SSID.present) {
			pConnectProfile->SSID.length = pIesTemp->SSID.num_ssid;
			qdf_mem_copy(pConnectProfile->SSID.ssId,
				     pIesTemp->SSID.ssid,
				     pIesTemp->SSID.num_ssid);
		}
		/* Save the bss desc */
		status =
			csr_roam_save_connected_bss_desc(pMac, sessionId, pSirBssDesc);

		if (CSR_IS_QOS_BSS(pIesTemp) || pIesTemp->HTCaps.present) {
			/* Some HT AP's dont send WMM IE so in that case we assume all HT Ap's are Qos Enabled AP's */
			pConnectProfile->qap = true;
		} else {
			pConnectProfile->qap = false;
		}

		if (pIesTemp->ExtCap.present) {
			struct s_ext_cap *p_ext_cap = (struct s_ext_cap *)
							pIesTemp->ExtCap.bytes;
			pConnectProfile->proxyARPService = p_ext_cap->
							    proxy_arp_service;
		}

		if (NULL == pIes) {
			/* Free memory if it allocated locally */
			qdf_mem_free(pIesTemp);
		}
	}
	/* Save Qos connection */
	pConnectProfile->qosConnection =
		pMac->roam.roamSession[sessionId].fWMMConnection;

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		csr_free_connect_bss_desc(pMac, sessionId);
	}
	for (index = 0; index < pProfile->SSIDs.numOfSSIDs; index++) {
		if ((pProfile->SSIDs.SSIDList[index].SSID.length ==
		     pConnectProfile->SSID.length)
		    && (!qdf_mem_cmp(pProfile->SSIDs.SSIDList[index].SSID.
				       ssId, pConnectProfile->SSID.ssId,
				       pConnectProfile->SSID.length))) {
			pConnectProfile->handoffPermitted =
				pProfile->SSIDs.SSIDList[index].handoffPermitted;
			break;
		}
		pConnectProfile->handoffPermitted = false;
	}

	return status;
}


bool is_disconnect_pending(tpAniSirGlobal pmac,
				uint8_t sessionid)
{
	tListElem *entry = NULL;
	tListElem *next_entry = NULL;
	tSmeCmd *command = NULL;
	bool disconnect_cmd_exist = false;

	csr_ll_lock(&pmac->sme.smeCmdPendingList);
	entry = csr_ll_peek_head(&pmac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
	while (entry) {
		next_entry = csr_ll_next(&pmac->sme.smeCmdPendingList,
					entry, LL_ACCESS_NOLOCK);

		command = GET_BASE_ADDR(entry, tSmeCmd, Link);
		if (command && CSR_IS_DISCONNECT_COMMAND(command) &&
				command->sessionId == sessionid){
			disconnect_cmd_exist = true;
			break;
		}
		entry = next_entry;
	}
	csr_ll_unlock(&pmac->sme.smeCmdPendingList);
	return disconnect_cmd_exist;
}

static void csr_roam_join_rsp_processor(tpAniSirGlobal pMac,
					tSirSmeJoinRsp *pSmeJoinRsp)
{
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	tCsrRoamSession *session_ptr;

	if (pSmeJoinRsp) {
		session_ptr = CSR_GET_SESSION(pMac, pSmeJoinRsp->sessionId);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Sme Join Response is NULL"));
		return;
	}
	if (!session_ptr) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("session %d not found"), pSmeJoinRsp->sessionId);
		return;
	}
	/* The head of the active list is the request we sent */
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	}
	if (eSIR_SME_SUCCESS == pSmeJoinRsp->statusCode) {
		if (pCommand
		    && eCsrSmeIssuedAssocToSimilarAP ==
		    pCommand->u.roamCmd.roamReason) {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
			sme_qos_csr_event_ind(pMac, pSmeJoinRsp->sessionId,
					      SME_QOS_CSR_HANDOFF_COMPLETE, NULL);
#endif
		}

		session_ptr->supported_nss_1x1 =
			pSmeJoinRsp->supported_nss_1x1;
		sms_log(pMac, LOG1,
			FL("SME session supported nss: %d"),
			session_ptr->supported_nss_1x1);

		/* *
		 * The join bssid count can be reset as soon as
		 * we are done with the join requests and returning
		 * the response to upper layers
		 * */
		session_ptr->join_bssid_count = 0;
		csr_roam_complete(pMac, eCsrJoinSuccess, (void *)pSmeJoinRsp);
	} else {
		uint32_t roamId = 0;
		bool is_dis_pending;

		/* The head of the active list is the request we sent */
		/* Try to get back the same profile and roam again */
		if (pCommand) {
			roamId = pCommand->u.roamCmd.roamId;
		}
		session_ptr->joinFailStatusCode.statusCode =
			pSmeJoinRsp->statusCode;
		session_ptr->joinFailStatusCode.reasonCode =
			pSmeJoinRsp->protStatusCode;
		sms_log(pMac, LOGW,
			"SmeJoinReq failed with statusCode= 0x%08X [%d]",
			pSmeJoinRsp->statusCode, pSmeJoinRsp->statusCode);
		/* If Join fails while Handoff is in progress, indicate disassociated event to supplicant to reconnect */
		if (csr_roam_is_handoff_in_progress(pMac, pSmeJoinRsp->sessionId)) {
			csr_roam_call_callback(pMac, pSmeJoinRsp->sessionId, NULL,
					       roamId, eCSR_ROAM_DISASSOCIATED,
					       eCSR_ROAM_RESULT_FORCED);
			/* Should indicate neighbor roam algorithm about the connect failure here */
			csr_neighbor_roam_indicate_connect(pMac,
							   pSmeJoinRsp->sessionId,
							   QDF_STATUS_E_FAILURE);
		}
		/*
		 * if userspace has issued disconnection,
		 * driver should not continue connecting
		 */
		is_dis_pending = is_disconnect_pending(pMac, session_ptr->sessionId);
		if (pCommand && (session_ptr->join_bssid_count <
				CSR_MAX_BSSID_COUNT) && !is_dis_pending) {
			csr_roam(pMac, pCommand);
		} else {
			/* ****************************************************
			 * When the upper layers issue a connect command, there
			 * is a roam command with reason eCsrHddIssued that
			 * gets enqueued and an associated timer for the SME
			 * command timeout is started which is currently 120
			 * seconds. This command would be dequeued only upon
			 * succesfull connections. In case of join failures, if
			 * there are too many BSS in the cache, and if we fail
			 * Join requests with all of them, there is a chance of
			 * timing out the above timer.
			 * ***************************************************/
			if (session_ptr->join_bssid_count >=
					CSR_MAX_BSSID_COUNT)
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Excessive Join Req Failures"));

			if (is_dis_pending)
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_ERROR,
					FL("disconnect is pending, complete roam"));

			session_ptr->join_bssid_count = 0;
			csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
		}
	} /*else: ( eSIR_SME_SUCCESS == pSmeJoinRsp->statusCode ) */
}

QDF_STATUS csr_roam_issue_join(tpAniSirGlobal pMac, uint32_t sessionId,
			       tSirBssDescription *pSirBssDesc,
			       tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile,
			       uint32_t roamId)
{
	QDF_STATUS status;
	sms_log(pMac, LOG1, "Attempting to Join Bssid= " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(pSirBssDesc->bssId));

	/* Set the roaming substate to 'join attempt'... */
	csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_JOIN_REQ, sessionId);
	/* attempt to Join this BSS... */
	status =
		csr_send_join_req_msg(pMac, sessionId, pSirBssDesc, pProfile, pIes,
				      eWNI_SME_JOIN_REQ);
	return status;
}

void csr_roam_reissue_roam_command(tpAniSirGlobal pMac)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tCsrRoamInfo roamInfo;
	uint32_t sessionId;
	tCsrRoamSession *pSession;

	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (NULL == pEntry) {
		sms_log(pMac, LOGE,
			FL("Disassoc rsp can't continue, no active CMD"));
		return;
	}
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (eSmeCommandRoam != pCommand->command) {
		sms_log(pMac, LOGW, FL("Active cmd, is not a roaming CMD"));
		return;
	}
	sessionId = pCommand->sessionId;
	pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return;
	}

	if (!pCommand->u.roamCmd.fStopWds) {
		if (pSession->bRefAssocStartCnt > 0) {
			/*
			 * bRefAssocStartCnt was incremented in
			 * csr_roam_join_next_bss when the roam command issued
			 * previously. As part of reissuing the roam command
			 * again csr_roam_join_next_bss is going increment
			 * RefAssocStartCnt. So make sure to decrement the
			 * bRefAssocStartCnt
			 */
			pSession->bRefAssocStartCnt--;
		}
		if (eCsrStopRoaming == csr_roam_join_next_bss(pMac, pCommand,
							      true)) {
			sms_log(pMac, LOGW,
				FL("Failed to reissue join command"));
			csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
		}
		return;
	}
	qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
	roamInfo.pBssDesc = pCommand->u.roamCmd.pLastRoamBss;
	roamInfo.statusCode = pSession->joinFailStatusCode.statusCode;
	roamInfo.reasonCode = pSession->joinFailStatusCode.reasonCode;
	pSession->connectState = eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTED;
	csr_roam_call_callback(pMac, sessionId, &roamInfo,
			       pCommand->u.roamCmd.roamId,
			       eCSR_ROAM_INFRA_IND,
			       eCSR_ROAM_RESULT_INFRA_DISASSOCIATED);

	if (!QDF_IS_STATUS_SUCCESS(csr_roam_issue_stop_bss(pMac, sessionId,
					eCSR_ROAM_SUBSTATE_STOP_BSS_REQ))) {
		sms_log(pMac, LOGE,
			FL("Failed to reissue stop_bss command for WDS"));
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
	}
}

bool csr_is_roam_command_waiting_for_session(tpAniSirGlobal pMac, uint32_t sessionId)
{
	bool fRet = false;
	tListElem *pEntry;
	tSmeCmd *pCommand = NULL;
	/* alwasy lock active list before locking pending list */
	csr_ll_lock(&pMac->sme.smeCmdActiveList);
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if ((eSmeCommandRoam == pCommand->command)
		    && (sessionId == pCommand->sessionId)) {
			fRet = true;
		}
	}
	if (false == fRet) {
		csr_ll_lock(&pMac->sme.smeCmdPendingList);
		pEntry =
			csr_ll_peek_head(&pMac->sme.smeCmdPendingList,
					 LL_ACCESS_NOLOCK);
		while (pEntry) {
			pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			if ((eSmeCommandRoam == pCommand->command)
			    && (sessionId == pCommand->sessionId)) {
				fRet = true;
				break;
			}
			pEntry =
				csr_ll_next(&pMac->sme.smeCmdPendingList, pEntry,
					    LL_ACCESS_NOLOCK);
		}
		csr_ll_unlock(&pMac->sme.smeCmdPendingList);
	}
	if (false == fRet) {
		csr_ll_lock(&pMac->roam.roamCmdPendingList);
		pEntry =
			csr_ll_peek_head(&pMac->roam.roamCmdPendingList,
					 LL_ACCESS_NOLOCK);
		while (pEntry) {
			pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			if ((eSmeCommandRoam == pCommand->command)
			    && (sessionId == pCommand->sessionId)) {
				fRet = true;
				break;
			}
			pEntry =
				csr_ll_next(&pMac->roam.roamCmdPendingList, pEntry,
					    LL_ACCESS_NOLOCK);
		}
		csr_ll_unlock(&pMac->roam.roamCmdPendingList);
	}
	csr_ll_unlock(&pMac->sme.smeCmdActiveList);
	return fRet;
}

bool csr_is_roam_command_waiting(tpAniSirGlobal pMac)
{
	bool fRet = false;
	uint32_t i;
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		fRet = csr_is_roam_command_waiting_for_session(pMac, i);
		if (CSR_IS_SESSION_VALID(pMac, i)
		    && (fRet)) {
			break;
		}
	}
	return fRet;
}

bool csr_is_command_waiting(tpAniSirGlobal pMac)
{
	bool fRet = false;
	/* alwasy lock active list before locking pending list */
	csr_ll_lock(&pMac->sme.smeCmdActiveList);
	fRet = csr_ll_is_list_empty(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK);
	if (false == fRet) {
		fRet =
			csr_ll_is_list_empty(&pMac->sme.smeCmdPendingList,
					     LL_ACCESS_LOCK);
	}
	csr_ll_unlock(&pMac->sme.smeCmdActiveList);
	return fRet;
}

bool csr_is_scan_for_roam_command_active(tpAniSirGlobal pMac)
{
	bool fRet = false;
	tListElem *pEntry;
	tCsrCmd *pCommand;
	/* alwasy lock active list before locking pending list */
	csr_ll_lock(&pMac->sme.smeCmdActiveList);
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tCsrCmd, Link);
		if ((eCsrRoamCommandScan == pCommand->command) &&
		    ((eCsrScanForSsid == pCommand->u.scanCmd.reason) ||
		     (eCsrScanP2PFindPeer == pCommand->u.scanCmd.reason))) {
			fRet = true;
		}
	}
	csr_ll_unlock(&pMac->sme.smeCmdActiveList);
	return fRet;
}

static void
csr_roaming_state_config_cnf_processor(tpAniSirGlobal mac_ctx,
				       uint32_t result)
{
	tListElem *entry = csr_ll_peek_head(&mac_ctx->sme.smeCmdActiveList,
					     LL_ACCESS_LOCK);
	tCsrScanResult *scan_result = NULL;
	tSirBssDescription *bss_desc = NULL;
	tSmeCmd *cmd = NULL;
	uint32_t session_id;
	tCsrRoamSession *session;
	tDot11fBeaconIEs *local_ies = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (NULL == entry) {
		sms_log(mac_ctx, LOGE, FL("CFG_CNF with active list empty"));
		return;
	}
	cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
	session_id = cmd->sessionId;
	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), session_id);
		return;
	}

	if (CSR_IS_ROAMING(session) && session->fCancelRoaming) {
		/* the roaming is cancelled. Simply complete the command */
		sms_log(mac_ctx, LOGW, FL("Roam command canceled"));
		csr_roam_complete(mac_ctx, eCsrNothingToJoin, NULL);
		return;
	}

	/* If the roaming has stopped, not to continue the roaming command */
	if (!CSR_IS_ROAMING(session) && CSR_IS_ROAMING_COMMAND(cmd)) {
		/* No need to complete roaming here as it already completes */
		sms_log(mac_ctx, LOGW,
			FL("Roam cmd (reason %d) aborted(roaming completed)"),
			cmd->u.roamCmd.roamReason);
		csr_set_abort_roaming_command(mac_ctx, cmd);
		csr_roam_complete(mac_ctx, eCsrNothingToJoin, NULL);
		return;
	}

	if (!IS_SIR_STATUS_SUCCESS(result)) {
		/*
		 * In the event the configuration failed, for infra let the roam
		 * processor attempt to join something else...
		 */
		if (cmd->u.roamCmd.pRoamBssEntry
		    && CSR_IS_INFRASTRUCTURE(&cmd->u.roamCmd.roamProfile)) {
			csr_roam(mac_ctx, cmd);
		} else {
			/* We need to complete the command */
			if (csr_is_bss_type_ibss
				    (cmd->u.roamCmd.roamProfile.BSSType)) {
				csr_roam_complete(mac_ctx, eCsrStartBssFailure,
						  NULL);
			} else {
				csr_roam_complete(mac_ctx, eCsrNothingToJoin,
						  NULL);
			}
		}
		return;
	}

	/* we have active entry */
	sms_log(mac_ctx, LOG2, "Cfg sequence complete");
	/*
	 * Successfully set the configuration parameters for the new Bss.
	 * Attempt to join the roaming Bss
	 */
	if (cmd->u.roamCmd.pRoamBssEntry) {
		scan_result = GET_BASE_ADDR(cmd->u.roamCmd.pRoamBssEntry,
					    tCsrScanResult,
					    Link);
		bss_desc = &scan_result->Result.BssDescriptor;
	}
	if (csr_is_bss_type_ibss(cmd->u.roamCmd.roamProfile.BSSType)
	    || CSR_IS_INFRA_AP(&cmd->u.roamCmd.roamProfile)
	    || CSR_IS_NDI(&cmd->u.roamCmd.roamProfile)) {
		if (!QDF_IS_STATUS_SUCCESS(csr_roam_issue_start_bss(mac_ctx,
						session_id, &session->bssParams,
						&cmd->u.roamCmd.roamProfile,
						bss_desc,
						cmd->u.roamCmd.roamId))) {
			sms_log(mac_ctx, LOGE, FL("CSR start BSS failed"));
			/* We need to complete the command */
			csr_roam_complete(mac_ctx, eCsrStartBssFailure, NULL);
		}
		return;
	}

	if (!cmd->u.roamCmd.pRoamBssEntry) {
		sms_log(mac_ctx, LOGE, FL("pRoamBssEntry is NULL"));
		/* We need to complete the command */
		csr_roam_complete(mac_ctx, eCsrJoinFailure, NULL);
		return;
	}

	if (NULL == scan_result) {
		/* If we are roaming TO an Infrastructure BSS... */
		QDF_ASSERT(scan_result != NULL);
		return;
	}

	if (!csr_is_infra_bss_desc(bss_desc)) {
		sms_log(mac_ctx, LOGW,
			FL("found BSSType mismatching the one in BSS descp"));
		return;
	}

	local_ies = (tDot11fBeaconIEs *) scan_result->Result.pvIes;
	if (!local_ies) {
		status = csr_get_parsed_bss_description_ies(mac_ctx, bss_desc,
							    &local_ies);
		if (!QDF_IS_STATUS_SUCCESS(status))
			return;
	}

	if (csr_is_conn_state_connected_infra(mac_ctx, session_id)) {
		if (csr_is_ssid_equal(mac_ctx, session->pConnectBssDesc,
				      bss_desc, local_ies)) {
			cmd->u.roamCmd.fReassoc = true;
			csr_roam_issue_reassociate(mac_ctx, session_id,
						   bss_desc, local_ies,
						   &cmd->u.roamCmd.roamProfile);
		} else {
			/*
			 * otherwise, we have to issue a new Join request to LIM
			 * because we disassociated from the previously
			 * associated AP.
			 */
			status = csr_roam_issue_join(mac_ctx, session_id,
					bss_desc, local_ies,
					&cmd->u.roamCmd.roamProfile,
					cmd->u.roamCmd.roamId);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				/* try something else */
				csr_roam(mac_ctx, cmd);
			}
		}
	} else {
		status = QDF_STATUS_SUCCESS;
		/*
		 * We need to come with other way to figure out that this is
		 * because of HO in BMP The below API will be only available for
		 * Android as it uses a different HO algorithm. Reassoc request
		 * will be used only for ESE and 11r handoff whereas other
		 * legacy roaming should use join request
		 */
		if (csr_roam_is_handoff_in_progress(mac_ctx, session_id)
		    && csr_roam_is11r_assoc(mac_ctx, session_id)) {
			status = csr_roam_issue_reassociate(mac_ctx,
					session_id, bss_desc,
					(tDot11fBeaconIEs *)
					(scan_result->Result.pvIes),
					&cmd->u.roamCmd.roamProfile);
		} else
#ifdef FEATURE_WLAN_ESE
		if (csr_roam_is_handoff_in_progress(mac_ctx, session_id)
		   && csr_roam_is_ese_assoc(mac_ctx, session_id)) {
			/* Now serialize the reassoc command. */
			status = csr_roam_issue_reassociate_cmd(mac_ctx,
								session_id);
		} else
#endif
		if (csr_roam_is_handoff_in_progress(mac_ctx, session_id)
		   && csr_roam_is_fast_roam_enabled(mac_ctx, session_id)) {
			/* Now serialize the reassoc command. */
			status = csr_roam_issue_reassociate_cmd(mac_ctx,
								session_id);
		} else {
			/*
			 * else we are not connected and attempting to Join.
			 * Issue the Join request.
			 */
			status = csr_roam_issue_join(mac_ctx, session_id,
						    bss_desc,
						    (tDot11fBeaconIEs *)
						    (scan_result->Result.pvIes),
						    &cmd->u.roamCmd.roamProfile,
						    cmd->u.roamCmd.roamId);
		}
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			/* try something else */
			csr_roam(mac_ctx, cmd);
		}
	}
	if (!scan_result->Result.pvIes) {
		/* Locally allocated */
		qdf_mem_free(local_ies);
	}
}

static void csr_roam_roaming_state_reassoc_rsp_processor(tpAniSirGlobal pMac,
							 tpSirSmeJoinRsp pSmeJoinRsp)
{
	eCsrRoamCompleteResult result;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[pSmeJoinRsp->sessionId];
	tCsrRoamInfo roamInfo;
	uint32_t roamId = 0;
	tCsrRoamSession *csr_session;

	if (eSIR_SME_SUCCESS == pSmeJoinRsp->statusCode) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  FL("CSR SmeReassocReq Successful"));
		result = eCsrReassocSuccess;
		csr_session = CSR_GET_SESSION(pMac, pSmeJoinRsp->sessionId);
		if (NULL != csr_session) {
			csr_session->supported_nss_1x1 =
				pSmeJoinRsp->supported_nss_1x1;
			sms_log(pMac, LOG1, FL("SME session supported nss: %d"),
				csr_session->supported_nss_1x1);
		}
		/*
		 * Since the neighbor roam algorithm uses reassoc req for
		 * handoff instead of join, we need the response contents while
		 * processing the result in csr_roam_process_results()
		 */
		if (csr_roam_is_handoff_in_progress(pMac, pSmeJoinRsp->sessionId)) {
			/* Need to dig more on indicating events to SME QoS module */
			sme_qos_csr_event_ind(pMac, pSmeJoinRsp->sessionId,
					      SME_QOS_CSR_HANDOFF_COMPLETE, NULL);
			csr_roam_complete(pMac, result, pSmeJoinRsp);
		} else {
			csr_roam_complete(pMac, result, NULL);
		}
	}
	/* Should we handle this similar to handling the join failure? Is it ok
	 * to call csr_roam_complete() with state as CsrJoinFailure */
	else {
		sms_log(pMac, LOGW,
			"CSR SmeReassocReq failed with statusCode= 0x%08X [%d]",
			pSmeJoinRsp->statusCode, pSmeJoinRsp->statusCode);
		result = eCsrReassocFailure;
		cds_flush_logs(WLAN_LOG_TYPE_FATAL,
			WLAN_LOG_INDICATOR_HOST_DRIVER,
			WLAN_LOG_REASON_ROAM_FAIL,
			true, false);
		if ((eSIR_SME_FT_REASSOC_TIMEOUT_FAILURE ==
		     pSmeJoinRsp->statusCode)
		    || (eSIR_SME_FT_REASSOC_FAILURE ==
			pSmeJoinRsp->statusCode)
		    || (eSIR_SME_INVALID_PARAMETERS ==
			pSmeJoinRsp->statusCode)) {
			/* Inform HDD to turn off FT flag in HDD */
			if (pNeighborRoamInfo) {
				qdf_mem_zero(&roamInfo, sizeof(tCsrRoamInfo));
				csr_roam_call_callback(pMac,
						       pSmeJoinRsp->sessionId,
						       &roamInfo, roamId,
						       eCSR_ROAM_FT_REASSOC_FAILED,
						       eSIR_SME_SUCCESS);
				/*
				 * Since the above callback sends a disconnect
				 * to HDD, we should clean-up our state
				 * machine as well to be in sync with the upper
				 * layers. There is no need to send a disassoc
				 * since: 1) we will never reassoc to the current
				 * AP in LFR, and 2) there is no need to issue a
				 * disassoc to the AP with which we were trying
				 * to reassoc.
				 */
				csr_roam_complete(pMac, eCsrJoinFailure, NULL);
				return;
			}
		}
		/* In the event that the Reassociation fails, then we need to Disassociate the current association and keep */
		/* roaming.  Note that we will attempt to Join the AP instead of a Reassoc since we may have attempted a */
		/* 'Reassoc to self', which AP's that don't support Reassoc will force a Disassoc. */
		/* The disassoc rsp message will remove the command from active list */
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_roam_issue_disassociate
				    (pMac, pSmeJoinRsp->sessionId,
				    eCSR_ROAM_SUBSTATE_DISASSOC_REASSOC_FAILURE, false))) {
			csr_roam_complete(pMac, eCsrJoinFailure, NULL);
		}
	}
}

static void csr_roam_roaming_state_stop_bss_rsp_processor(tpAniSirGlobal pMac,
							  tSirSmeRsp *pSmeRsp)
{
	eCsrRoamCompleteResult result_code = eCsrNothingToJoin;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	{
		host_log_ibss_pkt_type *pIbssLog;
		WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
					 LOG_WLAN_IBSS_C);
		if (pIbssLog) {
			pIbssLog->eventId = WLAN_IBSS_EVENT_STOP_RSP;
			if (eSIR_SME_SUCCESS != pSmeRsp->statusCode) {
				pIbssLog->status = WLAN_IBSS_STATUS_FAILURE;
			}
			WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
		}
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	pMac->roam.roamSession[pSmeRsp->sessionId].connectState =
		eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
	if (CSR_IS_ROAM_SUBSTATE_STOP_BSS_REQ(pMac, pSmeRsp->sessionId)) {
		if (CSR_IS_CONN_NDI(pMac->roam.roamSession[pSmeRsp->sessionId].
							pCurRoamProfile)) {
			result_code = eCsrStopBssSuccess;
			if (pSmeRsp->statusCode != eSIR_SME_SUCCESS)
				result_code = eCsrStopBssFailure;
		}
		csr_roam_complete(pMac, result_code, NULL);
	} else
	if (CSR_IS_ROAM_SUBSTATE_DISCONNECT_CONTINUE
		    (pMac, pSmeRsp->sessionId)) {
		csr_roam_reissue_roam_command(pMac);
	}
}

/**
 * csr_dequeue_command() - removes a command from active cmd list
 * @pMac:          mac global context
 *
 * Return: void
 */
static void
csr_dequeue_command(tpAniSirGlobal mac_ctx)
{
	bool fRemoveCmd;
	tSmeCmd *cmd = NULL;
	tListElem *entry = csr_ll_peek_head(&mac_ctx->sme.smeCmdActiveList,
					    LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac_ctx, LOGE, FL("NO commands are active"));
		return;
	}

	cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
	/*
	 * If the head of the queue is Active and it is a given cmd type, remove
	 * and put this on the Free queue.
	 */
	if (eSmeCommandRoam != cmd->command) {
		sms_log(mac_ctx, LOGE, FL("Roam command not active"));
		return;
	}
	/*
	 * we need to process the result first before removing it from active
	 * list because state changes still happening insides
	 * roamQProcessRoamResults so no other roam command should be issued.
	 */
	fRemoveCmd = csr_ll_remove_entry(&mac_ctx->sme.smeCmdActiveList, entry,
					 LL_ACCESS_LOCK);
	if (cmd->u.roamCmd.fReleaseProfile) {
		csr_release_profile(mac_ctx, &cmd->u.roamCmd.roamProfile);
		cmd->u.roamCmd.fReleaseProfile = false;
	}
	if (fRemoveCmd)
		csr_release_command_roam(mac_ctx, cmd);
	else
		sms_log(mac_ctx, LOGE, FL("fail to remove cmd reason %d"),
			cmd->u.roamCmd.roamReason);
}

/**
 * csr_post_roam_failure() - post roam failure back to csr and issues a disassoc
 * @pMac:               mac global context
 * @session_id:         session id
 * @roam_info:          roam info struct
 * @scan_filter:        scan filter to free
 * @cur_roam_profile:   current csr roam profile
 *
 * Return: void
 */
static void
csr_post_roam_failure(tpAniSirGlobal mac_ctx,
		      uint32_t session_id,
		      tCsrRoamInfo *roam_info,
		      tCsrScanResultFilter *scan_filter,
		      tCsrRoamProfile *cur_roam_profile)
{
	QDF_STATUS status;

	if (scan_filter) {
		csr_free_scan_filter(mac_ctx, scan_filter);
		qdf_mem_free(scan_filter);
	}
	if (cur_roam_profile)
		qdf_mem_free(cur_roam_profile);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		csr_roam_synch_clean_up(mac_ctx, session_id);
#endif
	/* Inform the upper layers that the reassoc failed */
	qdf_mem_zero(roam_info, sizeof(tCsrRoamInfo));
	csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
			       eCSR_ROAM_FT_REASSOC_FAILED, eSIR_SME_SUCCESS);
	/*
	 * Issue a disassoc request so that PE/LIM uses this to clean-up the FT
	 * session. Upon success, we would re-enter this routine after receiving
	 * the disassoc response and will fall into the reassoc fail sub-state.
	 * And, eventually call csr_roam_complete which would remove the roam
	 * command from SME active queue.
	 */
	status = csr_roam_issue_disassociate(mac_ctx, session_id,
			eCSR_ROAM_SUBSTATE_DISASSOC_REASSOC_FAILURE, false);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE,
			FL("csr_roam_issue_disassociate failed, status %d"),
			status);
		csr_roam_complete(mac_ctx, eCsrJoinFailure, NULL);
	}
}

/**
 * csr_check_profile_in_scan_cache() - finds if roam profile is present in scan
 * cache or not
 * @pMac:                  mac global context
 * @scan_filter:           out param, scan filter
 * @neighbor_roam_info:    roam info struct
 * @hBSSList:              scan result
 *
 * Return: true if found else false.
 */
static bool
csr_check_profile_in_scan_cache(tpAniSirGlobal mac_ctx,
				tCsrScanResultFilter **scan_filter,
				tpCsrNeighborRoamControlInfo neighbor_roam_info,
				tScanResultHandle *hBSSList)
{
	QDF_STATUS status;
	*scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == *scan_filter) {
		sms_log(mac_ctx, LOGE, FL("alloc for ScanFilter failed."));
		return false;
	}
	qdf_mem_set(*scan_filter, sizeof(tCsrScanResultFilter), 0);
	(*scan_filter)->scan_filter_for_roam = 1;
	status = csr_roam_prepare_filter_from_profile(mac_ctx,
			&neighbor_roam_info->csrNeighborRoamProfile,
			*scan_filter);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE,
			FL("failed to prepare scan filter, status %d"),
			status);
		return false;
	}
	status = csr_scan_get_result(mac_ctx, *scan_filter, hBSSList);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE,
			FL("csr_scan_get_result failed, status %d"),
			status);
		return false;
	}
	return true;
}

void csr_roam_roaming_state_disassoc_rsp_processor(tpAniSirGlobal pMac,
						   tSirSmeDisassocRsp *pSmeRsp)
{
	tScanResultHandle hBSSList;
	tCsrRoamInfo roamInfo;
	tCsrScanResultFilter *pScanFilter = NULL;
	uint32_t roamId = 0;
	tCsrRoamProfile *pCurRoamProfile = NULL;
	QDF_STATUS status;
	uint32_t sessionId;
	tCsrRoamSession *pSession;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	tSirSmeDisassocRsp SmeDisassocRsp;

	csr_ser_des_unpack_diassoc_rsp((uint8_t *) pSmeRsp, &SmeDisassocRsp);
	sessionId = SmeDisassocRsp.sessionId;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG, FL("sessionId %d"),
		  sessionId);

	if (csr_is_conn_state_infra(pMac, sessionId)) {
		pMac->roam.roamSession[sessionId].connectState =
			eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
	}

	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return;
	}

	if (CSR_IS_ROAM_SUBSTATE_DISASSOC_NO_JOIN(pMac, sessionId)) {
		sms_log(pMac, LOG2, "***eCsrNothingToJoin***");
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
	} else if (CSR_IS_ROAM_SUBSTATE_DISASSOC_FORCED(pMac, sessionId) ||
		   CSR_IS_ROAM_SUBSTATE_DISASSOC_REQ(pMac, sessionId)) {
		if (eSIR_SME_SUCCESS == SmeDisassocRsp.statusCode) {
			sms_log(pMac, LOG2,
				FL("CSR force disassociated successful"));
			/*
			 * A callback to HDD will be issued from
			 * csr_roam_complete so no need to do anything here
			 */
		}
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
	} else if (CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac, sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  FL("CSR SmeDisassocReq due to HO on session %d"),
			  sessionId);
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		/*
		 * First ensure if the roam profile is in the scan cache.
		 * If not, post a reassoc failure and disconnect.
		 */
		if (!csr_check_profile_in_scan_cache(pMac, &pScanFilter,
						pNeighborRoamInfo, &hBSSList))
			goto POST_ROAM_FAILURE;

		/*
		 * After ensuring that the roam profile is in the scan result
		 * list, dequeue the command from the active list.
		 */
		csr_dequeue_command(pMac);

		/* notify HDD about handoff and provide the BSSID too */
		roamInfo.reasonCode = eCsrRoamReasonBetterAP;

		qdf_copy_macaddr(&roamInfo.bssid,
			pNeighborRoamInfo->csrNeighborRoamProfile.BSSIDs.bssid);

		csr_roam_call_callback(pMac, sessionId, &roamInfo, 0,
				       eCSR_ROAM_ROAMING_START,
				       eCSR_ROAM_RESULT_NONE);

		/*
		 * Copy the connected profile to apply the same for this
		 * connection as well
		 */
		pCurRoamProfile = qdf_mem_malloc(sizeof(tCsrRoamProfile));
		if (pCurRoamProfile != NULL) {
			/*
			 * notify sub-modules like QoS etc. that handoff
			 * happening
			 */
			sme_qos_csr_event_ind(pMac, sessionId,
					      SME_QOS_CSR_HANDOFF_ASSOC_REQ,
					      NULL);
			qdf_mem_set(pCurRoamProfile, sizeof(tCsrRoamProfile),
				    0);
			csr_roam_copy_profile(pMac, pCurRoamProfile,
					      pSession->pCurRoamProfile);
			/* make sure to put it at the head of the cmd queue */
			status = csr_roam_issue_connect(pMac, sessionId,
					pCurRoamProfile, hBSSList,
					eCsrSmeIssuedAssocToSimilarAP,
					roamId, true, false);
			if (!QDF_IS_STATUS_SUCCESS(status))
				sms_log(pMac, LOGE,
					FL("issue_connect failed. status %d"),
					status);

			csr_release_profile(pMac, pCurRoamProfile);
			qdf_mem_free(pCurRoamProfile);
			csr_free_scan_filter(pMac, pScanFilter);
			qdf_mem_free(pScanFilter);
			return;
		}

POST_ROAM_FAILURE:
		csr_post_roam_failure(pMac, sessionId, &roamInfo,
			      pScanFilter, pCurRoamProfile);
	} /* else if ( CSR_IS_ROAM_SUBSTATE_DISASSOC_HO( pMac ) ) */
	else if (CSR_IS_ROAM_SUBSTATE_REASSOC_FAIL(pMac, sessionId)) {
		/* Disassoc due to Reassoc failure falls into this codepath */
		csr_roam_complete(pMac, eCsrJoinFailure, NULL);
	} else {
		if (eSIR_SME_SUCCESS == SmeDisassocRsp.statusCode) {
			/*
			 * Successfully disassociated from the 'old' Bss.
			 * We get Disassociate response in three conditions.
			 * 1) The case where we are disasociating from an Infra
			 *    Bss to start an IBSS.
			 * 2) When we are disassociating from an Infra Bss to
			 *    join an IBSS or a new infra network.
			 * 3) Where we are doing an Infra to Infra roam between
			 *    networks with different SSIDs.
			 * In all cases, we set the new Bss configuration here
			 * and attempt to join
			 */
			sms_log(pMac, LOG2,
				FL("Disassociated successfully"));
		} else {
			sms_log(pMac, LOGE,
				FL("DisassocReq failed, statusCode= 0x%08X"),
				SmeDisassocRsp.statusCode);
		}
		/* We are not done yet. Get the data and continue roaming */
		csr_roam_reissue_roam_command(pMac);
	}
}

static void csr_roam_roaming_state_deauth_rsp_processor(tpAniSirGlobal pMac,
							tSirSmeDeauthRsp *pSmeRsp)
{
	tSirResultCodes statusCode;
	/* No one is sending eWNI_SME_DEAUTH_REQ to PE. */
	sms_log(pMac, LOGW, FL("is no-op"));
	statusCode = csr_get_de_auth_rsp_status_code(pSmeRsp);
	pMac->roam.deauthRspStatus = statusCode;
	if (CSR_IS_ROAM_SUBSTATE_DEAUTH_REQ(pMac, pSmeRsp->sessionId)) {
		csr_roam_complete(pMac, eCsrNothingToJoin, NULL);
	} else {
		if (eSIR_SME_SUCCESS == statusCode) {
			/* Successfully deauth from the 'old' Bss... */
			/* */
			sms_log(pMac, LOG2,
				"CSR SmeDeauthReq disassociated Successfully");
		} else {
			sms_log(pMac, LOGW,
				"SmeDeauthReq failed with statusCode= 0x%08X",
				statusCode);
		}
		/* We are not done yet. Get the data and continue roaming */
		csr_roam_reissue_roam_command(pMac);
	}
}

static void csr_roam_roaming_state_start_bss_rsp_processor(tpAniSirGlobal pMac,
							   tSirSmeStartBssRsp *
							   pSmeStartBssRsp)
{
	eCsrRoamCompleteResult result;

	if (eSIR_SME_SUCCESS == pSmeStartBssRsp->statusCode) {
		sms_log(pMac, LOGW, "SmeStartBssReq Successful");
		result = eCsrStartBssSuccess;
	} else {
		sms_log(pMac, LOGW,
			"SmeStartBssReq failed with statusCode= 0x%08X",
			pSmeStartBssRsp->statusCode);
		/* Let csr_roam_complete decide what to do */
		result = eCsrStartBssFailure;
	}
	csr_roam_complete(pMac, result, pSmeStartBssRsp);
}

/**
 * csr_roaming_state_msg_processor() - process roaming messages
 * @pMac:       mac global context
 * @pMsgBuf:    message buffer
 *
 * We need to be careful on whether to cast pMsgBuf (pSmeRsp) to other type of
 * strucutres. It depends on how the message is constructed. If the message is
 * sent by lim_send_sme_rsp, the pMsgBuf is only a generic response and can only
 * be used as pointer to tSirSmeRsp. For the messages where sender allocates
 * memory for specific structures, then it can be cast accordingly.
 *
 * Return: status of operation
 */
void csr_roaming_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf)
{
	tSirSmeRsp *pSmeRsp;
	tSmeIbssPeerInd *pIbssPeerInd;
	tCsrRoamInfo roamInfo;
	pSmeRsp = (tSirSmeRsp *) pMsgBuf;
	sms_log(pMac, LOG2, FL("Message %d[0x%04X] received in substate %s"),
		pSmeRsp->messageType, pSmeRsp->messageType,
		mac_trace_getcsr_roam_sub_state(
			pMac->roam.curSubState[pSmeRsp->sessionId]));
	pSmeRsp->messageType = pSmeRsp->messageType;
	pSmeRsp->length = pSmeRsp->length;
	pSmeRsp->statusCode = pSmeRsp->statusCode;

	switch (pSmeRsp->messageType) {

	case eWNI_SME_JOIN_RSP:
		/* in Roaming state, process the Join response message... */
		if (CSR_IS_ROAM_SUBSTATE_JOIN_REQ(pMac, pSmeRsp->sessionId))
			/* We sent a JOIN_REQ */
			csr_roam_join_rsp_processor(pMac,
						    (tSirSmeJoinRsp *) pSmeRsp);
		break;
	case eWNI_SME_REASSOC_RSP:
		/* or the Reassociation response message... */
		if (CSR_IS_ROAM_SUBSTATE_REASSOC_REQ(pMac, pSmeRsp->sessionId))
			csr_roam_roaming_state_reassoc_rsp_processor(pMac,
						(tpSirSmeJoinRsp) pSmeRsp);
		break;
	case eWNI_SME_STOP_BSS_RSP:
		/* or the Stop Bss response message... */
		csr_roam_roaming_state_stop_bss_rsp_processor(pMac, pSmeRsp);
		break;
	case eWNI_SME_DISASSOC_RSP:
		/* or the Disassociate response message... */
		if (CSR_IS_ROAM_SUBSTATE_DISASSOC_REQ(pMac, pSmeRsp->sessionId)
		    || CSR_IS_ROAM_SUBSTATE_DISASSOC_NO_JOIN(pMac,
							pSmeRsp->sessionId)
		    || CSR_IS_ROAM_SUBSTATE_REASSOC_FAIL(pMac,
							pSmeRsp->sessionId)
		    || CSR_IS_ROAM_SUBSTATE_DISASSOC_FORCED(pMac,
							pSmeRsp->sessionId)
		    || CSR_IS_ROAM_SUBSTATE_DISCONNECT_CONTINUE(pMac,
							pSmeRsp->sessionId)
		    || CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac,
							pSmeRsp->sessionId)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  FL("eWNI_SME_DISASSOC_RSP subState = %s"),
				  mac_trace_getcsr_roam_sub_state(
				  pMac->roam.curSubState[pSmeRsp->sessionId]));
			csr_roam_roaming_state_disassoc_rsp_processor(pMac,
						(tSirSmeDisassocRsp *) pSmeRsp);
		}
		break;
	case eWNI_SME_DEAUTH_RSP:
		/* or the Deauthentication response message... */
		if (CSR_IS_ROAM_SUBSTATE_DEAUTH_REQ(pMac, pSmeRsp->sessionId)) {
			csr_remove_cmd_with_session_id_from_pending_list(pMac,
					pSmeRsp->sessionId,
					&pMac->sme.smeCmdPendingList,
					eSmeCommandWmStatusChange);
			csr_remove_cmd_with_session_id_from_pending_list(pMac,
					pSmeRsp->sessionId,
					&pMac->roam.roamCmdPendingList,
					eSmeCommandWmStatusChange);
			csr_roam_roaming_state_deauth_rsp_processor(pMac,
						(tSirSmeDeauthRsp *) pSmeRsp);
		}
		break;
	case eWNI_SME_START_BSS_RSP:
		/* or the Start BSS response message... */
		if (CSR_IS_ROAM_SUBSTATE_START_BSS_REQ(pMac,
						       pSmeRsp->sessionId))
			csr_roam_roaming_state_start_bss_rsp_processor(pMac,
						(tSirSmeStartBssRsp *) pSmeRsp);
		break;
	/* In case CSR issues STOP_BSS, we need to tell HDD about peer departed
	 * becasue PE is removing them
	 */
	case eWNI_SME_IBSS_PEER_DEPARTED_IND:
		pIbssPeerInd = (tSmeIbssPeerInd *) pSmeRsp;
		sms_log(pMac, LOGE,
			FL("Peer departed ntf from LIM in joining state"));
		qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
		roamInfo.staId = (uint8_t) pIbssPeerInd->staId;
		roamInfo.ucastSig = (uint8_t) pIbssPeerInd->ucastSig;
		roamInfo.bcastSig = (uint8_t) pIbssPeerInd->bcastSig;
		qdf_copy_macaddr(&roamInfo.peerMac, &pIbssPeerInd->peer_addr);
		csr_roam_call_callback(pMac, pSmeRsp->sessionId, &roamInfo, 0,
				       eCSR_ROAM_CONNECT_STATUS_UPDATE,
				       eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED);
		break;
	case eWNI_SME_GET_RSSI_REQ:
	{
		tAniGetRssiReq *pGetRssiReq = (tAniGetRssiReq *) pMsgBuf;
		if (NULL != pGetRssiReq->rssiCallback)
			((tCsrRssiCallback) pGetRssiReq->rssiCallback)
				(pGetRssiReq->lastRSSI, pGetRssiReq->staId,
				pGetRssiReq->pDevContext);
		else
			sms_log(pMac, LOGE,
				FL("pGetRssiReq->rssiCallback is NULL"));
	}
	break;
	default:
		sms_log(pMac, LOG1,
			FL("Unexpected message type = %d[0x%X] received in substate %s"),
			pSmeRsp->messageType, pSmeRsp->messageType,
			mac_trace_getcsr_roam_sub_state(
				pMac->roam.curSubState[pSmeRsp->sessionId]));
		/* If we are connected, check the link status change */
		if (!csr_is_conn_state_disconnected(pMac, pSmeRsp->sessionId))
			csr_roam_check_for_link_status_change(pMac, pSmeRsp);
		break;
	}
}

void csr_roam_joined_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf)
{
	tSirSmeRsp *pSirMsg = (tSirSmeRsp *) pMsgBuf;
	switch (pSirMsg->messageType) {
	case eWNI_SME_GET_STATISTICS_RSP:
		sms_log(pMac, LOG2, FL("Stats rsp from PE"));
		csr_roam_stats_rsp_processor(pMac, pSirMsg);
		break;
	case eWNI_SME_UPPER_LAYER_ASSOC_CNF:
	{
		tCsrRoamSession *pSession;
		tSirSmeAssocIndToUpperLayerCnf *pUpperLayerAssocCnf;
		tCsrRoamInfo roamInfo;
		tCsrRoamInfo *pRoamInfo = NULL;
		uint32_t sessionId;
		QDF_STATUS status;
		sms_log(pMac, LOG1,
			FL
				("ASSOCIATION confirmation can be given to upper layer "));
		qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
		pRoamInfo = &roamInfo;
		pUpperLayerAssocCnf =
			(tSirSmeAssocIndToUpperLayerCnf *) pMsgBuf;
		status =
			csr_roam_get_session_id_from_bssid(pMac,
							   (struct qdf_mac_addr *)
							   pUpperLayerAssocCnf->
							   bssId, &sessionId);
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sms_log(pMac, LOGE,
				FL("  session %d not found "),
				sessionId);
			return;
		}

		pRoamInfo->statusCode = eSIR_SME_SUCCESS;               /* send the status code as Success */
		pRoamInfo->u.pConnectedProfile =
			&pSession->connectedProfile;
		pRoamInfo->staId = (uint8_t) pUpperLayerAssocCnf->aid;
		pRoamInfo->rsnIELen =
			(uint8_t) pUpperLayerAssocCnf->rsnIE.length;
		pRoamInfo->prsnIE =
			pUpperLayerAssocCnf->rsnIE.rsnIEdata;
#ifdef FEATURE_WLAN_WAPI
		pRoamInfo->wapiIELen =
			(uint8_t) pUpperLayerAssocCnf->wapiIE.length;
		pRoamInfo->pwapiIE =
			pUpperLayerAssocCnf->wapiIE.wapiIEdata;
#endif
		pRoamInfo->addIELen =
			(uint8_t) pUpperLayerAssocCnf->addIE.length;
		pRoamInfo->paddIE =
			pUpperLayerAssocCnf->addIE.addIEdata;
		qdf_mem_copy(pRoamInfo->peerMac.bytes,
			     pUpperLayerAssocCnf->peerMacAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(&pRoamInfo->bssid,
			     pUpperLayerAssocCnf->bssId,
			     sizeof(struct qdf_mac_addr));
		pRoamInfo->wmmEnabledSta =
			pUpperLayerAssocCnf->wmmEnabledSta;
		pRoamInfo->timingMeasCap =
			pUpperLayerAssocCnf->timingMeasCap;
		qdf_mem_copy(&pRoamInfo->chan_info,
			     &pUpperLayerAssocCnf->chan_info,
			     sizeof(tSirSmeChanInfo));
		if (CSR_IS_INFRA_AP(pRoamInfo->u.pConnectedProfile)) {
			pMac->roam.roamSession[sessionId].connectState =
				eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED;
			pRoamInfo->fReassocReq =
				pUpperLayerAssocCnf->reassocReq;
			status =
				csr_roam_call_callback(pMac, sessionId,
						       pRoamInfo, 0,
						       eCSR_ROAM_INFRA_IND,
						       eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF);
		}
	}
	break;
	default:
		csr_roam_check_for_link_status_change(pMac, pSirMsg);
		break;
	}
}

QDF_STATUS csr_roam_issue_set_context_req(tpAniSirGlobal pMac,
					  uint32_t sessionId,
					  eCsrEncryptionType EncryptType,
					  tSirBssDescription *pBssDescription,
					  tSirMacAddr *bssId, bool addKey,
					  bool fUnicast,
					  tAniKeyDirection aniKeyDirection,
					  uint8_t keyId, uint16_t keyLength,
					  uint8_t *pKey, uint8_t paeRole)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tAniEdType edType;

	if (eCSR_ENCRYPT_TYPE_UNKNOWN == EncryptType) {
		EncryptType = eCSR_ENCRYPT_TYPE_NONE;
	}

	edType = csr_translate_encrypt_type_to_ed_type(EncryptType);

	/*
	 * Allow 0 keys to be set for the non-WPA encrypt types. For WPA encrypt
	 * types, the num keys must be non-zero or LIM will reject the set
	 * context (assumes the SET_CONTEXT does not occur until the keys are
	 * distrubuted).
	 */
	if (CSR_IS_ENC_TYPE_STATIC(EncryptType) || addKey) {
		tCsrRoamSetKey setKey;
		setKey.encType = EncryptType;
		setKey.keyDirection = aniKeyDirection;
		qdf_mem_copy(&setKey.peerMac, bssId, sizeof(struct qdf_mac_addr));
		/* 0 for supplicant */
		setKey.paeRole = paeRole;
		/* Key index */
		setKey.keyId = keyId;
		setKey.keyLength = keyLength;
		if (keyLength) {
			qdf_mem_copy(setKey.Key, pKey, keyLength);
		}
		status = csr_roam_issue_set_key_command(pMac, sessionId,
							&setKey, 0);
	}
	return status;
}

/**
 * csr_update_key_cmd() - update key info in set key command
 * @mac_ctx:         mac global context
 * @session:         roam session
 * @set_key:         input set key command
 * @set_key_cmd:     set key command to update
 * @enqueue_cmd:     indicates if command need to be enqueued to sme
 *
 * This function will validate the key length, adjust if too long. Tt will
 * update bool enqueue_cmd, to false if some error has occured key are local.
 *
 * Return: status of operation
 */
static QDF_STATUS
csr_update_key_cmd(tpAniSirGlobal mac_ctx, tCsrRoamSession *session,
		   tCsrRoamSetKey *set_key, tSmeCmd *set_key_cmd,
		   bool *enqueue_cmd)
{
	switch (set_key->encType) {
	case eCSR_ENCRYPT_TYPE_WEP40:
	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
		/* KeyLength maybe 0 for static WEP */
		if (set_key->keyLength) {
			if (set_key->keyLength < CSR_WEP40_KEY_LEN) {
				sms_log(mac_ctx, LOGW,
					FL("Invalid WEP40 keylength [= %d]"),
					set_key->keyLength);
				*enqueue_cmd = false;
				return QDF_STATUS_E_INVAL;
			}

			set_key_cmd->u.setKeyCmd.keyLength = CSR_WEP40_KEY_LEN;
			qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
				     CSR_WEP40_KEY_LEN);
		}
		*enqueue_cmd = true;
		break;
	case eCSR_ENCRYPT_TYPE_WEP104:
	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
		/* KeyLength maybe 0 for static WEP */
		if (set_key->keyLength) {
			if (set_key->keyLength < CSR_WEP104_KEY_LEN) {
				sms_log(mac_ctx, LOGW,
					FL("Invalid WEP104 keylength [= %d]"),
					set_key->keyLength);
				*enqueue_cmd = false;
				return QDF_STATUS_E_INVAL;
			}

			set_key_cmd->u.setKeyCmd.keyLength = CSR_WEP104_KEY_LEN;
			qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
				     CSR_WEP104_KEY_LEN);
		}
		*enqueue_cmd = true;
		break;
	case eCSR_ENCRYPT_TYPE_TKIP:
		if (set_key->keyLength < CSR_TKIP_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("Invalid TKIP keylength [= %d]"),
				set_key->keyLength);
			*enqueue_cmd = false;
			return QDF_STATUS_E_INVAL;
		}
		set_key_cmd->u.setKeyCmd.keyLength = CSR_TKIP_KEY_LEN;
		qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
			     CSR_TKIP_KEY_LEN);
		*enqueue_cmd = true;
		break;
	case eCSR_ENCRYPT_TYPE_AES:
		if (set_key->keyLength < CSR_AES_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("Invalid AES/CCMP keylength [= %d]"),
				set_key->keyLength);
			*enqueue_cmd = false;
			return QDF_STATUS_E_INVAL;
		}
		set_key_cmd->u.setKeyCmd.keyLength = CSR_AES_KEY_LEN;
		qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
			     CSR_AES_KEY_LEN);
		*enqueue_cmd = true;
		break;
#ifdef FEATURE_WLAN_WAPI
	case eCSR_ENCRYPT_TYPE_WPI:
		if (set_key->keyLength < CSR_WAPI_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("Invalid WAPI keylength [= %d]"),
				set_key->keyLength);
			*enqueue_cmd = false;
			return QDF_STATUS_E_INVAL;
		}
		set_key_cmd->u.setKeyCmd.keyLength = CSR_WAPI_KEY_LEN;
		qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
			     CSR_WAPI_KEY_LEN);
		if (session->pCurRoamProfile) {
			session->pCurRoamProfile->negotiatedUCEncryptionType =
				eCSR_ENCRYPT_TYPE_WPI;
		} else {
			sms_log(mac_ctx, LOGW,
				FL("pCurRoamProfile is NULL."));
			*enqueue_cmd = false;
			return QDF_STATUS_E_INVAL;
		}
		*enqueue_cmd = true;
		break;
#endif /* FEATURE_WLAN_WAPI */
#ifdef FEATURE_WLAN_ESE
	case eCSR_ENCRYPT_TYPE_KRK:
		/* no need to enqueue KRK key request, since they are local */
		*enqueue_cmd = false;
		if (set_key->keyLength < CSR_KRK_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("Invalid KRK keylength [= %d]"),
				set_key->keyLength);
			return QDF_STATUS_E_INVAL;
		}
		qdf_mem_copy(session->eseCckmInfo.krk, set_key->Key,
			     CSR_KRK_KEY_LEN);
		session->eseCckmInfo.reassoc_req_num = 1;
		session->eseCckmInfo.krk_plumbed = true;
		break;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case eCSR_ENCRYPT_TYPE_BTK:
		/* no need to enqueue KRK key request, since they are local */
		*enqueue_cmd = false;
		if (set_key->keyLength < SIR_BTK_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("LFR3:Invalid BTK keylength [= %d]"),
				set_key->keyLength);
			return QDF_STATUS_E_INVAL;
		}
		qdf_mem_copy(session->eseCckmInfo.btk, set_key->Key,
			     SIR_BTK_KEY_LEN);
		break;
#endif
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_11W
	/* Check for 11w BIP */
	case eCSR_ENCRYPT_TYPE_AES_CMAC:
		if (set_key->keyLength < CSR_AES_KEY_LEN) {
			sms_log(mac_ctx, LOGW,
				FL("Invalid AES/CCMP keylength [= %d]"),
				set_key->keyLength);
			*enqueue_cmd = false;
			return QDF_STATUS_E_INVAL;
		}
		set_key_cmd->u.setKeyCmd.keyLength = CSR_AES_KEY_LEN;
		qdf_mem_copy(set_key_cmd->u.setKeyCmd.Key, set_key->Key,
			     CSR_AES_KEY_LEN);
		*enqueue_cmd = true;
		break;
#endif /* WLAN_FEATURE_11W */
	default:
		/* for open security also we want to enqueue command */
		*enqueue_cmd = true;
		return QDF_STATUS_SUCCESS;
	} /* end of switch */
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS csr_roam_issue_set_key_command(tpAniSirGlobal pMac,
						 uint32_t sessionId,
						 tCsrRoamSetKey *pSetKey,
						 uint32_t roamId)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	bool enqueue_cmd = true;
	tSmeCmd *pCommand = NULL;
#if defined(FEATURE_WLAN_ESE) || defined (FEATURE_WLAN_WAPI)
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
#endif /* FEATURE_WLAN_ESE */

	pCommand = csr_get_command_buffer(pMac);
	if (NULL == pCommand) {
		sms_log(pMac, LOGE, FL(" fail to get command buffer"));
		return QDF_STATUS_E_RESOURCES;
	}
	qdf_mem_zero(pCommand, sizeof(tSmeCmd));
	pCommand->command = eSmeCommandSetKey;
	pCommand->sessionId = (uint8_t) sessionId;
	/*
	 * following function will validate the key length, Adjust if too long.
	 * for static WEP the keys are not set thru' SetContextReq
	 *
	 * it will update bool enqueue_cmd, to false if some error has occured
	 * key are local. enqueue sme command only if enqueue_cmd is true
	 * status is indication of success or failure and will be returned to
	 * called of current function if command is not enqueued due to key req
	 * being local
	 */
	status = csr_update_key_cmd(pMac, pSession, pSetKey,
				    pCommand, &enqueue_cmd);
	if (enqueue_cmd) {
		pCommand->u.setKeyCmd.roamId = roamId;
		pCommand->u.setKeyCmd.encType = pSetKey->encType;
		pCommand->u.setKeyCmd.keyDirection = pSetKey->keyDirection;
		qdf_copy_macaddr(&pCommand->u.setKeyCmd.peermac,
				 &pSetKey->peerMac);
		/* 0 for supplicant */
		pCommand->u.setKeyCmd.paeRole = pSetKey->paeRole;
		pCommand->u.setKeyCmd.keyId = pSetKey->keyId;
		qdf_mem_copy(pCommand->u.setKeyCmd.keyRsc, pSetKey->keyRsc,
			     CSR_MAX_RSC_LEN);
		/*
		 * Always put set key to the head of the Q because it is the
		 * only thing to get executed in case of WT_KEY state
		 */

		status = csr_queue_sme_command(pMac, pCommand, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				FL("fail to send message status = %d"), status);
			/* update to false so that command can be freed */
			enqueue_cmd = false;
		}
	}

	/*
	 * Free the command if enqueue_cmd == false:
	 * this means that command was not enqueued because either there has
	 * been a failure, or it is a "local" operation like the set ESE CCKM
	 * KRK key.
	 */
	if (false == enqueue_cmd)
		csr_release_command_set_key(pMac, pCommand);

	return status;
}

QDF_STATUS csr_roam_process_set_key_command(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	QDF_STATUS status;
	uint8_t numKeys = (pCommand->u.setKeyCmd.keyLength) ? 1 : 0;
	tAniEdType edType =
		csr_translate_encrypt_type_to_ed_type(pCommand->u.setKeyCmd.encType);
	bool fUnicast =
		(pCommand->u.setKeyCmd.peermac.bytes[0] == 0xFF) ? false : true;
	uint32_t sessionId = pCommand->sessionId;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	WLAN_HOST_DIAG_EVENT_DEF(setKeyEvent,
				 host_event_wlan_security_payload_type);

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (eSIR_ED_NONE != edType) {
		qdf_mem_set(&setKeyEvent,
			    sizeof(host_event_wlan_security_payload_type), 0);
		if (qdf_is_macaddr_group(&pCommand->u.setKeyCmd.peermac)) {
			setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_BCAST_REQ;
			setKeyEvent.encryptionModeMulticast =
				(uint8_t) diag_enc_type_from_csr_type(pCommand->u.
								      setKeyCmd.encType);
			setKeyEvent.encryptionModeUnicast =
				(uint8_t) diag_enc_type_from_csr_type(pSession->
								      connectedProfile.
								      EncryptionType);
		} else {
			setKeyEvent.eventId =
				WLAN_SECURITY_EVENT_SET_UNICAST_REQ;
			setKeyEvent.encryptionModeUnicast =
				(uint8_t) diag_enc_type_from_csr_type(pCommand->u.
								      setKeyCmd.encType);
			setKeyEvent.encryptionModeMulticast =
				(uint8_t) diag_enc_type_from_csr_type(pSession->
								      connectedProfile.
								      mcEncryptionType);
		}
		qdf_mem_copy(setKeyEvent.bssid,
			     pSession->connectedProfile.bssid.bytes,
			     QDF_MAC_ADDR_SIZE);
		if (CSR_IS_ENC_TYPE_STATIC(pCommand->u.setKeyCmd.encType)) {
			uint32_t defKeyId;
			/* It has to be static WEP here */
			if (IS_SIR_STATUS_SUCCESS
				    (wlan_cfg_get_int
					    (pMac, WNI_CFG_WEP_DEFAULT_KEYID, &defKeyId))) {
				setKeyEvent.keyId = (uint8_t) defKeyId;
			}
		} else {
			setKeyEvent.keyId = pCommand->u.setKeyCmd.keyId;
		}
		setKeyEvent.authMode =
			(uint8_t) diag_auth_type_from_csr_type(pSession->
							       connectedProfile.
							       AuthType);
		WLAN_HOST_DIAG_EVENT_REPORT(&setKeyEvent, EVENT_WLAN_SECURITY);
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	if (csr_is_set_key_allowed(pMac, sessionId)) {
		status = csr_send_mb_set_context_req_msg(pMac, sessionId,
							 pCommand->u.
							 setKeyCmd.peermac,
							 numKeys,
							 edType, fUnicast,
							 pCommand->u.setKeyCmd.
							 keyDirection,
							 pCommand->u.setKeyCmd.keyId,
							 pCommand->u.setKeyCmd.
							 keyLength,
							 pCommand->u.setKeyCmd.Key,
							 pCommand->u.setKeyCmd.
							 paeRole,
							 pCommand->u.setKeyCmd.
							 keyRsc);
	} else {
		sms_log(pMac, LOGW, FL(" cannot process not connected"));
		/* Set this status so the error handling take care of the case. */
		status = QDF_STATUS_CSR_WRONG_STATE;
	}
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("  error status %d"), status);
		csr_roam_call_callback(pMac, sessionId, NULL,
				       pCommand->u.setKeyCmd.roamId,
				       eCSR_ROAM_SET_KEY_COMPLETE,
				       eCSR_ROAM_RESULT_FAILURE);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
		if (eSIR_ED_NONE != edType) {
			if (qdf_is_macaddr_group(
					&pCommand->u.setKeyCmd.peermac)) {
				setKeyEvent.eventId =
					WLAN_SECURITY_EVENT_SET_BCAST_RSP;
			} else {
				setKeyEvent.eventId =
					WLAN_SECURITY_EVENT_SET_UNICAST_RSP;
			}
			setKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;
			WLAN_HOST_DIAG_EVENT_REPORT(&setKeyEvent,
						    EVENT_WLAN_SECURITY);
		}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	}
	return status;
}

QDF_STATUS csr_roam_set_key(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamSetKey *pSetKey, uint32_t roamId)
{
	QDF_STATUS status;

	if (!csr_is_set_key_allowed(pMac, sessionId)) {
		status = QDF_STATUS_CSR_WRONG_STATE;
	} else {
		status =
			csr_roam_issue_set_key_command(pMac, sessionId, pSetKey, roamId);
	}
	return status;
}

/*
   Prepare a filter base on a profile for parsing the scan results.
   Upon successful return, caller MUST call csr_free_scan_filter on
   pScanFilter when it is done with the filter.
 */
QDF_STATUS
csr_roam_prepare_filter_from_profile(tpAniSirGlobal mac_ctx,
				     tCsrRoamProfile *profile,
				     tCsrScanResultFilter *scan_fltr)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t size = 0;
	uint8_t idx = 0;
	tCsrChannelInfo *fltr_ch_info = &scan_fltr->ChannelInfo;
	tCsrChannelInfo *profile_ch_info = &profile->ChannelInfo;
	struct roam_ext_params *roam_params;
	uint8_t i;

	roam_params = &mac_ctx->roam.configParam.roam_params;

	if (profile->BSSIDs.numOfBSSIDs) {
		size = sizeof(struct qdf_mac_addr) * profile->BSSIDs.numOfBSSIDs;
		scan_fltr->BSSIDs.bssid = qdf_mem_malloc(size);
		if (NULL == scan_fltr->BSSIDs.bssid) {
			status = QDF_STATUS_E_NOMEM;
			goto free_filter;
		}
		scan_fltr->BSSIDs.numOfBSSIDs = profile->BSSIDs.numOfBSSIDs;
		qdf_mem_copy(scan_fltr->BSSIDs.bssid,
			     profile->BSSIDs.bssid, size);
	}

	if (profile->SSIDs.numOfSSIDs) {
		scan_fltr->SSIDs.numOfSSIDs = profile->SSIDs.numOfSSIDs;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("No of Allowed List:%d"),
			roam_params->num_ssid_allowed_list);
		if (scan_fltr->scan_filter_for_roam
			&& roam_params->num_ssid_allowed_list) {
			scan_fltr->SSIDs.numOfSSIDs =
				roam_params->num_ssid_allowed_list;
			size = sizeof(tCsrSSIDInfo) *
				scan_fltr->SSIDs.numOfSSIDs;
			scan_fltr->SSIDs.SSIDList = qdf_mem_malloc(size);
			if (NULL == scan_fltr->SSIDs.SSIDList)
				status = QDF_STATUS_E_FAILURE;
			else
				status = QDF_STATUS_SUCCESS;
			if (!QDF_IS_STATUS_SUCCESS(status))
				goto free_filter;
			for  (i = 0;
				i < roam_params->num_ssid_allowed_list;
				i++) {
				qdf_mem_copy((void *)
				    scan_fltr->SSIDs.SSIDList[i].SSID.ssId,
				    roam_params->ssid_allowed_list[i].ssId,
				    roam_params->ssid_allowed_list[i].length);
				scan_fltr->SSIDs.SSIDList[i].SSID.length =
				    roam_params->ssid_allowed_list[i].length;
				scan_fltr->SSIDs.SSIDList[i].handoffPermitted =
					1;
				scan_fltr->SSIDs.SSIDList[i].ssidHidden = 0;
			}
		} else {
			size = sizeof(tCsrSSIDInfo) *
				profile->SSIDs.numOfSSIDs;
			scan_fltr->SSIDs.SSIDList = qdf_mem_malloc(size);
			if (NULL == scan_fltr->SSIDs.SSIDList) {
				status = QDF_STATUS_E_NOMEM;
				goto free_filter;
			}
			qdf_mem_copy(scan_fltr->SSIDs.SSIDList,
					profile->SSIDs.SSIDList, size);
		}
	}

	if (!profile_ch_info->ChannelList
	    || (profile_ch_info->ChannelList[0] == 0)) {
		fltr_ch_info->numOfChannels = 0;
		fltr_ch_info->ChannelList = NULL;
	} else if (profile_ch_info->numOfChannels) {
		fltr_ch_info->numOfChannels = 0;
		fltr_ch_info->ChannelList =
			qdf_mem_malloc(sizeof(*(fltr_ch_info->ChannelList)) *
				       profile_ch_info->numOfChannels);
		if (NULL == fltr_ch_info->ChannelList) {
			status = QDF_STATUS_E_NOMEM;
			goto free_filter;
		}

		for (idx = 0; idx < profile_ch_info->numOfChannels; idx++) {
			if (csr_roam_is_channel_valid(mac_ctx,
				profile_ch_info->ChannelList[idx])) {
				fltr_ch_info->
				ChannelList[fltr_ch_info->numOfChannels]
					= profile_ch_info->ChannelList[idx];
				fltr_ch_info->numOfChannels++;
			} else {
				sms_log(mac_ctx, LOG1,
					FL("Channel (%d) is invalid"),
					profile_ch_info->ChannelList[idx]);
			}
		}
	} else {
		sms_log(mac_ctx, LOGE, FL("Channel list empty"));
		status = QDF_STATUS_E_FAILURE;
		goto free_filter;
	}
	scan_fltr->uapsd_mask = profile->uapsd_mask;
	scan_fltr->authType = profile->AuthType;
	scan_fltr->EncryptionType = profile->EncryptionType;
	scan_fltr->mcEncryptionType = profile->mcEncryptionType;
	scan_fltr->BSSType = profile->BSSType;
	scan_fltr->phyMode = profile->phyMode;
#ifdef FEATURE_WLAN_WAPI
	/*
	 * check if user asked for WAPI with 11n or auto mode, in that
	 * case modify the phymode to 11g
	 */
	if (csr_is_profile_wapi(profile)) {
		if (scan_fltr->phyMode & eCSR_DOT11_MODE_11n)
			scan_fltr->phyMode &= ~eCSR_DOT11_MODE_11n;
		if (scan_fltr->phyMode & eCSR_DOT11_MODE_AUTO)
			scan_fltr->phyMode &= ~eCSR_DOT11_MODE_AUTO;
		if (!scan_fltr->phyMode)
			scan_fltr->phyMode = eCSR_DOT11_MODE_11g;
	}
#endif /* FEATURE_WLAN_WAPI */
	/*Save the WPS info */
	scan_fltr->bWPSAssociation = profile->bWPSAssociation;
	scan_fltr->bOSENAssociation = profile->bOSENAssociation;
	if (profile->countryCode[0]) {
		/*
		 * This causes the matching function to use countryCode as one
		 * of the criteria.
		 */
		qdf_mem_copy(scan_fltr->countryCode, profile->countryCode,
			     WNI_CFG_COUNTRY_CODE_LEN);
	}
	if (profile->MDID.mdiePresent) {
		scan_fltr->MDID.mdiePresent = 1;
		scan_fltr->MDID.mobilityDomain = profile->MDID.mobilityDomain;
	}

#ifdef WLAN_FEATURE_11W
	/* Management Frame Protection */
	scan_fltr->MFPEnabled = profile->MFPEnabled;
	scan_fltr->MFPRequired = profile->MFPRequired;
	scan_fltr->MFPCapable = profile->MFPCapable;
#endif
	scan_fltr->csrPersona = profile->csrPersona;

free_filter:
	if (!QDF_IS_STATUS_SUCCESS(status))
		csr_free_scan_filter(mac_ctx, scan_fltr);

	return status;
}

bool csr_roam_issue_wm_status_change(tpAniSirGlobal pMac, uint32_t sessionId,
				     eCsrRoamWmStatusChangeTypes Type,
				     tSirSmeRsp *pSmeRsp)
{
	bool fCommandQueued = false;
	tSmeCmd *pCommand;
	do {
		/* Validate the type is ok... */
		if ((eCsrDisassociated != Type)
		    && (eCsrDeauthenticated != Type))
			break;
		pCommand = csr_get_command_buffer(pMac);
		if (!pCommand) {
			sms_log(pMac, LOGE, FL(" fail to get command buffer"));
			break;
		}
		/* Change the substate in case it is waiting for key */
		if (CSR_IS_WAIT_FOR_KEY(pMac, sessionId)) {
			csr_roam_stop_wait_for_key_timer(pMac);
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 sessionId);
		}
		pCommand->command = eSmeCommandWmStatusChange;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.wmStatusChangeCmd.Type = Type;
		if (eCsrDisassociated == Type) {
			qdf_mem_copy(&pCommand->u.wmStatusChangeCmd.u.
				     DisassocIndMsg, pSmeRsp,
				     sizeof(pCommand->u.wmStatusChangeCmd.u.
					    DisassocIndMsg));
		} else {
			qdf_mem_copy(&pCommand->u.wmStatusChangeCmd.u.
				     DeauthIndMsg, pSmeRsp,
				     sizeof(pCommand->u.wmStatusChangeCmd.u.
					    DeauthIndMsg));
		}
		if (QDF_IS_STATUS_SUCCESS
			    (csr_queue_sme_command(pMac, pCommand, true))) {
			fCommandQueued = true;
		} else {
			sms_log(pMac, LOGE, FL(" fail to send message "));
			csr_release_command_wm_status_change(pMac, pCommand);
		}

		/* AP has issued Dissac/Deauth, Set the operating mode value to configured value */
		csr_set_default_dot11_mode(pMac);
	} while (0);
	return fCommandQueued;
}

static QDF_STATUS csr_send_snr_request(void *pGetRssiReq)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_STATUS_SUCCESS !=
		wma_send_snr_request(wma_handle, pGetRssiReq)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Failed to Trigger wma stats request");
		return QDF_STATUS_E_FAILURE;
	}

	/* dont send success, otherwise call back
	 * will released with out values */
	return QDF_STATUS_E_BUSY;
}

static void csr_update_rssi(tpAniSirGlobal pMac, void *pMsg)
{
	int8_t rssi = 0;
	tAniGetRssiReq *pGetRssiReq = (tAniGetRssiReq *) pMsg;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	if (pGetRssiReq) {
		if (NULL != pGetRssiReq->p_cds_context) {
			qdf_status = csr_send_snr_request(pGetRssiReq);
		} else {
			sms_log(pMac, LOGE,
				FL("pGetRssiReq->p_cds_context is NULL"));
			return;
		}

		if (NULL != pGetRssiReq->rssiCallback) {
			if (qdf_status != QDF_STATUS_E_BUSY)
				((tCsrRssiCallback) (pGetRssiReq->rssiCallback))
					(rssi, pGetRssiReq->staId,
					pGetRssiReq->pDevContext);
			else
				sms_log(pMac, LOG1,
					FL
						("rssi request is posted. waiting for reply"));
		} else {
			sms_log(pMac, LOGE,
				FL("pGetRssiReq->rssiCallback is NULL"));
			return;
		}
	} else {
		sms_log(pMac, LOGE, FL("pGetRssiReq is NULL"));
	}
	return;

}

static void csr_update_snr(tpAniSirGlobal pMac, void *pMsg)
{
	tAniGetSnrReq *pGetSnrReq = (tAniGetSnrReq *) pMsg;

	if (pGetSnrReq) {
		if (QDF_STATUS_SUCCESS != wma_get_snr(pGetSnrReq)) {
			sms_log(pMac, LOGE, FL("Error in wma_get_snr"));
			return;
		}

	} else {
		sms_log(pMac, LOGE, FL("pGetSnrReq is NULL"));
	}
	return;
}

static QDF_STATUS csr_send_reset_ap_caps_changed(tpAniSirGlobal pMac,
				struct qdf_mac_addr *bssId)
{
	tpSirResetAPCapsChange pMsg;
	uint16_t len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* Create the message and send to lim */
	len = sizeof(tSirResetAPCapsChange);
	pMsg = qdf_mem_malloc(len);
	if (NULL == pMsg)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;

	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_set(pMsg, sizeof(tSirResetAPCapsChange), 0);
		pMsg->messageType = eWNI_SME_RESET_AP_CAPS_CHANGED;
		pMsg->length = len;
		qdf_copy_macaddr(&pMsg->bssId, bssId);
		sms_log(pMac, LOG1,
			FL("CSR reset caps change for Bssid= " MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(pMsg->bssId.bytes));
		status = cds_send_mb_message_to_mac(pMsg);
	} else {
		sms_log(pMac, LOGE, FL("Memory allocation failed"));
	}
	return status;
}

static void
csr_roam_chk_lnk_assoc_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSirSmeAssocInd *pAssocInd;
	tCsrRoamInfo roam_info;
	qdf_mem_set(&roam_info, sizeof(roam_info), 0);

	sms_log(mac_ctx, LOG1, FL("Receive WNI_SME_ASSOC_IND from SME"));
	pAssocInd = (tSirSmeAssocInd *) msg_ptr;
	status = csr_roam_get_session_id_from_bssid(mac_ctx,
				(struct qdf_mac_addr *) pAssocInd->bssId, &sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOG1,
			FL("Couldn't find session_id for given BSSID"));
		return;
	}
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		return;
	}
	roam_info_ptr = &roam_info;
	/* Required for indicating the frames to upper layer */
	roam_info_ptr->assocReqLength = pAssocInd->assocReqLength;
	roam_info_ptr->assocReqPtr = pAssocInd->assocReqPtr;
	roam_info_ptr->beaconPtr = pAssocInd->beaconPtr;
	roam_info_ptr->beaconLength = pAssocInd->beaconLength;
	roam_info_ptr->statusCode = eSIR_SME_SUCCESS;
	roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
	roam_info_ptr->staId = (uint8_t) pAssocInd->staId;
	roam_info_ptr->rsnIELen = (uint8_t) pAssocInd->rsnIE.length;
	roam_info_ptr->prsnIE = pAssocInd->rsnIE.rsnIEdata;
#ifdef FEATURE_WLAN_WAPI
	roam_info_ptr->wapiIELen = (uint8_t) pAssocInd->wapiIE.length;
	roam_info_ptr->pwapiIE = pAssocInd->wapiIE.wapiIEdata;
#endif
	roam_info_ptr->addIELen = (uint8_t) pAssocInd->addIE.length;
	roam_info_ptr->paddIE = pAssocInd->addIE.addIEdata;
	qdf_mem_copy(roam_info_ptr->peerMac.bytes,
		     pAssocInd->peerMacAddr,
		     sizeof(tSirMacAddr));
	qdf_mem_copy(roam_info_ptr->bssid.bytes,
		     pAssocInd->bssId,
		     sizeof(struct qdf_mac_addr));
	roam_info_ptr->wmmEnabledSta = pAssocInd->wmmEnabledSta;
	roam_info_ptr->timingMeasCap = pAssocInd->timingMeasCap;
	qdf_mem_copy(&roam_info_ptr->chan_info,
		     &pAssocInd->chan_info,
		     sizeof(tSirSmeChanInfo));
	if (CSR_IS_INFRA_AP(roam_info_ptr->u.pConnectedProfile)) {
		if (session->pCurRoamProfile &&
		    CSR_IS_ENC_TYPE_STATIC(
			session->pCurRoamProfile->negotiatedUCEncryptionType)) {
			/* NO keys... these key parameters don't matter. */
			csr_roam_issue_set_context_req(mac_ctx, sessionId,
			session->pCurRoamProfile->negotiatedUCEncryptionType,
			session->pConnectBssDesc,
			&(roam_info_ptr->peerMac.bytes),
			false, true, eSIR_TX_RX, 0, 0, NULL, 0);
			roam_info_ptr->fAuthRequired = false;
		} else {
			roam_info_ptr->fAuthRequired = true;
		}
		status = csr_roam_call_callback(mac_ctx, sessionId,
					roam_info_ptr, 0, eCSR_ROAM_INFRA_IND,
					eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND);
		if (!QDF_IS_STATUS_SUCCESS(status))
			/* Refused due to Mac filtering */
			roam_info_ptr->statusCode = eSIR_SME_ASSOC_REFUSED;
	}

	/* Send Association completion message to PE */
	status = csr_send_assoc_cnf_msg(mac_ctx, pAssocInd, status);
	/*
	 * send a message to CSR itself just to avoid the EAPOL frames going
	 * OTA before association response
	 */
	if (CSR_IS_INFRA_AP(roam_info_ptr->u.pConnectedProfile)
	    && (roam_info_ptr->statusCode != eSIR_SME_ASSOC_REFUSED)) {
		roam_info_ptr->fReassocReq = pAssocInd->reassocReq;
		status = csr_send_assoc_ind_to_upper_layer_cnf_msg(mac_ctx,
						pAssocInd, status, sessionId);
	}
}

static void
csr_roam_chk_lnk_disassoc_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSirSmeDisassocInd *pDisassocInd;
	tSmeCmd *cmd;
	tCsrRoamInfo roam_info;

	cmd = qdf_mem_malloc(sizeof(*cmd));
	if (NULL == cmd) {
		sms_log(mac_ctx, LOGE,
			FL("memory allocation failed for size = %zu"),
			sizeof(*cmd));
		return;
	}

	/*
	 * Check if AP dis-associated us because of MIC failure. If so,
	 * then we need to take action immediately and not wait till the
	 * the WmStatusChange requests is pushed and processed
	 */
	pDisassocInd = (tSirSmeDisassocInd *) msg_ptr;
	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	status = csr_roam_get_session_id_from_bssid(mac_ctx,
				&pDisassocInd->bssid, &sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("Session Id not found for BSSID "
			MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(pDisassocInd->bssid.bytes));
		qdf_mem_free(cmd);
		return;
	}

	sms_log(mac_ctx, LOGE,
		FL("DISASSOCIATION Indication from MAC for session %d "),
		sessionId);
	sms_log(mac_ctx, LOGE,
		FL("DISASSOCIATION from peer =" MAC_ADDRESS_STR
		   " " " reason = %d status = %d "),
		MAC_ADDR_ARRAY(pDisassocInd->peer_macaddr.bytes),
		pDisassocInd->reasonCode,
		pDisassocInd->statusCode);
	/*
	 * If we are in neighbor preauth done state then on receiving
	 * disassoc or deauth we dont roam instead we just disassoc
	 * from current ap and then go to disconnected state
	 * This happens for ESE and 11r FT connections ONLY.
	 */
	if (csr_roam_is11r_assoc(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
#ifdef FEATURE_WLAN_ESE
	if (csr_roam_is_ese_assoc(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
#endif
	if (csr_roam_is_fast_roam_enabled(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		qdf_mem_free(cmd);
		return;
	}
	if (csr_is_conn_state_infra(mac_ctx, sessionId))
		session->connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	sme_qos_csr_event_ind(mac_ctx, (uint8_t) sessionId,
			      SME_QOS_CSR_DISCONNECT_IND, NULL);
#endif
	csr_roam_link_down(mac_ctx, sessionId);
	csr_roam_issue_wm_status_change(mac_ctx, sessionId,
					eCsrDisassociated, msg_ptr);
	if (CSR_IS_INFRA_AP(&session->connectedProfile)) {
		roam_info_ptr = &roam_info;
		roam_info_ptr->statusCode = pDisassocInd->statusCode;
		roam_info_ptr->reasonCode = pDisassocInd->reasonCode;
		roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
		roam_info_ptr->staId = (uint8_t) pDisassocInd->staId;
		qdf_copy_macaddr(&roam_info_ptr->peerMac,
				 &pDisassocInd->peer_macaddr);
		qdf_copy_macaddr(&roam_info_ptr->bssid,
				 &pDisassocInd->bssid);
		status = csr_roam_call_callback(mac_ctx, sessionId,
						roam_info_ptr, 0,
						eCSR_ROAM_INFRA_IND,
						eCSR_ROAM_RESULT_DISASSOC_IND);
		/*
		 * STA/P2P client got  disassociated so remove any pending
		 * deauth commands in sme pending list
		 */
		cmd->command = eSmeCommandRoam;
		cmd->sessionId = (uint8_t) sessionId;
		cmd->u.roamCmd.roamReason = eCsrForcedDeauthSta;
		qdf_mem_copy(cmd->u.roamCmd.peerMac,
			     pDisassocInd->peer_macaddr.bytes,
			     QDF_MAC_ADDR_SIZE);
		csr_roam_remove_duplicate_command(mac_ctx, sessionId, cmd,
						  eCsrForcedDeauthSta);
	}
	qdf_mem_free(cmd);
}

static void
csr_roam_chk_lnk_deauth_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSirSmeDeauthInd *pDeauthInd;
	tCsrRoamInfo roam_info;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	sms_log(mac_ctx, LOG1, FL("DEAUTHENTICATION Indication from MAC"));
	pDeauthInd = (tpSirSmeDeauthInd) msg_ptr;
	status = csr_roam_get_session_id_from_bssid(mac_ctx,
						   &pDeauthInd->bssid,
						   &sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return;
	/* If we are in neighbor preauth done state then on receiving
	 * disassoc or deauth we dont roam instead we just disassoc
	 * from current ap and then go to disconnected state
	 * This happens for ESE and 11r FT connections ONLY.
	 */
	if (csr_roam_is11r_assoc(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
#ifdef FEATURE_WLAN_ESE
	if (csr_roam_is_ese_assoc(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
#endif
	if (csr_roam_is_fast_roam_enabled(mac_ctx, sessionId) &&
	    (csr_neighbor_roam_state_preauth_done(mac_ctx, sessionId)))
		csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
							mac_ctx, sessionId);
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		return;
	}

	if (csr_is_conn_state_infra(mac_ctx, sessionId))
		session->connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	sme_qos_csr_event_ind(mac_ctx, (uint8_t) sessionId,
			      SME_QOS_CSR_DISCONNECT_IND, NULL);
#endif
	csr_roam_link_down(mac_ctx, sessionId);
	csr_roam_issue_wm_status_change(mac_ctx, sessionId,
					eCsrDeauthenticated,
					msg_ptr);
	if (CSR_IS_INFRA_AP(&session->connectedProfile)) {
		roam_info_ptr = &roam_info;
		roam_info_ptr->statusCode = pDeauthInd->statusCode;
		roam_info_ptr->reasonCode = pDeauthInd->reasonCode;
		roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
		roam_info_ptr->staId = (uint8_t) pDeauthInd->staId;
		qdf_copy_macaddr(&roam_info_ptr->peerMac,
				 &pDeauthInd->peer_macaddr);
		qdf_copy_macaddr(&roam_info_ptr->bssid,
				 &pDeauthInd->bssid);
		status = csr_roam_call_callback(mac_ctx, sessionId,
						roam_info_ptr, 0,
						eCSR_ROAM_INFRA_IND,
						eCSR_ROAM_RESULT_DEAUTH_IND);
	}
}

static void
csr_roam_chk_lnk_swt_ch_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tpSirSmeSwitchChannelInd pSwitchChnInd;
	tCsrRoamInfo roamInfo;

	/* in case of STA, the SWITCH_CHANNEL originates from its AP */
	sms_log(mac_ctx, LOGW, FL("eWNI_SME_SWITCH_CHL_IND from SME"));
	pSwitchChnInd = (tpSirSmeSwitchChannelInd) msg_ptr;
	/* Update with the new channel id. The channel id is hidden in the
	 * statusCode.
	 */
	status = csr_roam_get_session_id_from_bssid(mac_ctx,
			&pSwitchChnInd->bssid, &sessionId);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		session = CSR_GET_SESSION(mac_ctx, sessionId);
		if (!session) {
			sms_log(mac_ctx, LOGE,
				FL("session %d not found"), sessionId);
			return;
		}
		session->connectedProfile.operationChannel =
			(uint8_t) pSwitchChnInd->newChannelId;
		if (session->pConnectBssDesc) {
			session->pConnectBssDesc->channelId =
				(uint8_t) pSwitchChnInd->newChannelId;
		}

		qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
		roamInfo.chan_info.chan_id = pSwitchChnInd->newChannelId;
		roamInfo.chan_info.ch_width =
				pSwitchChnInd->chan_params.ch_width;
		roamInfo.chan_info.sec_ch_offset =
				pSwitchChnInd->chan_params.sec_ch_offset;
		roamInfo.chan_info.band_center_freq1 =
				pSwitchChnInd->chan_params.center_freq_seg0;
		roamInfo.chan_info.band_center_freq2 =
				pSwitchChnInd->chan_params.center_freq_seg1;
		status = csr_roam_call_callback(mac_ctx, sessionId,
				&roamInfo, 0, eCSR_ROAM_STA_CHANNEL_SWITCH,
				eCSR_ROAM_RESULT_NONE);
	}
}

static void
csr_roam_chk_lnk_deauth_rsp(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSirSmeDeauthRsp *pDeauthRsp = (tSirSmeDeauthRsp *) msg_ptr;
	tCsrRoamInfo roam_info;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	sms_log(mac_ctx, LOGW, FL("eWNI_SME_DEAUTH_RSP from SME"));
	sessionId = pDeauthRsp->sessionId;
	if (!CSR_IS_SESSION_VALID(mac_ctx, sessionId))
		return;
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (CSR_IS_INFRA_AP(&session->connectedProfile)) {
		roam_info_ptr = &roam_info;
		roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
		qdf_copy_macaddr(&roam_info_ptr->peerMac,
				 &pDeauthRsp->peer_macaddr);
		roam_info_ptr->reasonCode = eCSR_ROAM_RESULT_FORCED;
		roam_info_ptr->statusCode = pDeauthRsp->statusCode;
		status = csr_roam_call_callback(mac_ctx, sessionId,
						roam_info_ptr, 0,
						eCSR_ROAM_LOSTLINK,
						eCSR_ROAM_RESULT_FORCED);
	}
}

static void
csr_roam_chk_lnk_disassoc_rsp(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tCsrRoamInfo roam_info;
	/*
	 * session id is invalid here so cant use it to access the array
	 * curSubstate as index
	 */
	tSirSmeDisassocRsp *pDisassocRsp = (tSirSmeDisassocRsp *) msg_ptr;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	sms_log(mac_ctx, LOGW, FL("eWNI_SME_DISASSOC_RSP from SME "));
	sessionId = pDisassocRsp->sessionId;
	if (!CSR_IS_SESSION_VALID(mac_ctx, sessionId))
		return;
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (CSR_IS_INFRA_AP(&session->connectedProfile)) {
		roam_info_ptr = &roam_info;
		roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
		qdf_copy_macaddr(&roam_info_ptr->peerMac,
				 &pDisassocRsp->peer_macaddr);
		roam_info_ptr->reasonCode = eCSR_ROAM_RESULT_FORCED;
		roam_info_ptr->statusCode = pDisassocRsp->statusCode;
		status = csr_roam_call_callback(mac_ctx, sessionId,
						roam_info_ptr, 0,
						eCSR_ROAM_LOSTLINK,
						eCSR_ROAM_RESULT_FORCED);
	}
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void
csr_roam_diag_mic_fail(tpAniSirGlobal mac_ctx, uint32_t sessionId)
{
	WLAN_HOST_DIAG_EVENT_DEF(secEvent,
				 host_event_wlan_security_payload_type);
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		return;
	}
	qdf_mem_set(&secEvent, sizeof(host_event_wlan_security_payload_type),
		    0);
	secEvent.eventId = WLAN_SECURITY_EVENT_MIC_ERROR;
	secEvent.encryptionModeMulticast =
		(uint8_t) diag_enc_type_from_csr_type(
				session->connectedProfile.mcEncryptionType);
	secEvent.encryptionModeUnicast =
		(uint8_t) diag_enc_type_from_csr_type(
				session->connectedProfile.EncryptionType);
	secEvent.authMode =
		(uint8_t) diag_auth_type_from_csr_type(
				session->connectedProfile.AuthType);
	qdf_mem_copy(secEvent.bssid, session->connectedProfile.bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	WLAN_HOST_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

static void
csr_roam_chk_lnk_mic_fail_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tCsrRoamInfo roam_info;
	tpSirSmeMicFailureInd pMicInd = (tpSirSmeMicFailureInd) msg_ptr;
	eCsrRoamResult result = eCSR_ROAM_RESULT_MIC_ERROR_UNICAST;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	status = csr_roam_get_session_id_from_bssid(mac_ctx,
				&pMicInd->bssId, &sessionId);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_set(&roam_info, sizeof(tCsrRoamInfo), 0);
		roam_info.u.pMICFailureInfo = &pMicInd->info;
		roam_info_ptr = &roam_info;
		if (pMicInd->info.multicast)
			result = eCSR_ROAM_RESULT_MIC_ERROR_GROUP;
		else
			result = eCSR_ROAM_RESULT_MIC_ERROR_UNICAST;
		csr_roam_call_callback(mac_ctx, sessionId, roam_info_ptr, 0,
				       eCSR_ROAM_MIC_ERROR_IND, result);
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_roam_diag_mic_fail(mac_ctx, sessionId);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
}

static void
csr_roam_chk_lnk_pbs_probe_req_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo roam_info;
	tpSirSmeProbeReqInd pProbeReqInd = (tpSirSmeProbeReqInd) msg_ptr;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	sms_log(mac_ctx, LOG1, FL("WPS PBC Probe request Indication from SME"));

	status = csr_roam_get_session_id_from_bssid(mac_ctx,
			&pProbeReqInd->bssid, &sessionId);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_set(&roam_info, sizeof(tCsrRoamInfo), 0);
		roam_info.u.pWPSPBCProbeReq = &pProbeReqInd->WPSPBCProbeReq;
		csr_roam_call_callback(mac_ctx, sessionId, &roam_info,
				       0, eCSR_ROAM_WPS_PBC_PROBE_REQ_IND,
				       eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND);
	}
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void
csr_roam_diag_joined_new_bss(tpAniSirGlobal mac_ctx,
				     tSirSmeNewBssInfo *pNewBss)
{
	host_log_ibss_pkt_type *pIbssLog;
	uint32_t bi;
	WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
				 LOG_WLAN_IBSS_C);
	if (!pIbssLog)
		return;
	pIbssLog->eventId = WLAN_IBSS_EVENT_COALESCING;
	if (pNewBss) {
		qdf_copy_macaddr(&pIbssLog->bssid, &pNewBss->bssId);
		if (pNewBss->ssId.length)
			qdf_mem_copy(pIbssLog->ssid, pNewBss->ssId.ssId,
				     pNewBss->ssId.length);
		pIbssLog->operatingChannel = pNewBss->channelNumber;
	}
	if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_int(mac_ctx,
						   WNI_CFG_BEACON_INTERVAL,
						   &bi)))
		/* U8 is not enough for beacon interval */
		pIbssLog->beaconInterval = (uint8_t) bi;
	WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

static void
csr_roam_chk_lnk_wm_status_change_ntf(tpAniSirGlobal mac_ctx,
				      tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSirSmeWmStatusChangeNtf *pStatusChangeMsg;
	tCsrRoamInfo roam_info;
	tSirSmeApNewCaps *pApNewCaps;
	eCsrRoamResult result = eCSR_ROAM_RESULT_NONE;
	tSirMacAddr Broadcastaddr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	tSirSmeNewBssInfo *pNewBss;
	eRoamCmdStatus roamStatus = eCSR_ROAM_FAILED;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	pStatusChangeMsg = (tSirSmeWmStatusChangeNtf *) msg_ptr;
	switch (pStatusChangeMsg->statusChangeCode) {
	case eSIR_SME_IBSS_ACTIVE:
		sessionId = csr_find_ibss_session(mac_ctx);
		if (CSR_SESSION_ID_INVALID == sessionId)
			break;
		session = CSR_GET_SESSION(mac_ctx, sessionId);
		if (!session) {
			sms_log(mac_ctx, LOGE, FL("session %d not found"),
				sessionId);
			return;
		}
		session->connectState = eCSR_ASSOC_STATE_TYPE_IBSS_CONNECTED;
		if (session->pConnectBssDesc) {
			qdf_mem_copy(&roam_info.bssid,
				     session->pConnectBssDesc->bssId,
				     sizeof(struct qdf_mac_addr));
			roam_info.u.pConnectedProfile =
				&session->connectedProfile;
			roam_info_ptr = &roam_info;
		} else {
			sms_log(mac_ctx, LOGE,
				FL("CSR: connected BSS is empty"));
		}
		result = eCSR_ROAM_RESULT_IBSS_CONNECT;
		roamStatus = eCSR_ROAM_CONNECT_STATUS_UPDATE;
		break;

	case eSIR_SME_IBSS_INACTIVE:
		sessionId = csr_find_ibss_session(mac_ctx);
		if (CSR_SESSION_ID_INVALID != sessionId) {
			session = CSR_GET_SESSION(mac_ctx, sessionId);
			if (!session) {
				sms_log(mac_ctx, LOGE,
					FL("session %d not found"), sessionId);
				return;
			}
			session->connectState =
				eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED;
			result = eCSR_ROAM_RESULT_IBSS_INACTIVE;
			roamStatus = eCSR_ROAM_CONNECT_STATUS_UPDATE;
		}
		break;

	case eSIR_SME_JOINED_NEW_BSS:
		/* IBSS coalescing. */
		sms_log(mac_ctx, LOGW,
			FL("CSR: eSIR_SME_JOINED_NEW_BSS received from PE"));
		sessionId = csr_find_ibss_session(mac_ctx);
		if (CSR_SESSION_ID_INVALID == sessionId)
			break;
		session = CSR_GET_SESSION(mac_ctx, sessionId);
		if (!session) {
			sms_log(mac_ctx, LOGE, FL("session %d not found"),
				sessionId);
			return;
		}
		/* update the connection state information */
		pNewBss = &pStatusChangeMsg->statusChangeInfo.newBssInfo;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
		csr_roam_diag_joined_new_bss(mac_ctx, pNewBss);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
		csr_roam_update_connected_profile_from_new_bss(mac_ctx,
							       sessionId,
							       pNewBss);

		if ((eCSR_ENCRYPT_TYPE_NONE ==
		     session->connectedProfile.EncryptionType)) {
			csr_roam_issue_set_context_req(mac_ctx,
			    sessionId,
			    session->connectedProfile.EncryptionType,
			    session->pConnectBssDesc,
			    &Broadcastaddr, false, false,
			    eSIR_TX_RX, 0, 0, NULL, 0);
		}
		result = eCSR_ROAM_RESULT_IBSS_COALESCED;
		roamStatus = eCSR_ROAM_IBSS_IND;
		qdf_mem_copy(&roam_info.bssid, &pNewBss->bssId,
			     sizeof(struct qdf_mac_addr));
		roam_info_ptr = &roam_info;
		/* This BSSID is the real BSSID, save it */
		if (session->pConnectBssDesc)
			qdf_mem_copy(session->pConnectBssDesc->bssId,
				     &pNewBss->bssId, sizeof(struct qdf_mac_addr));
		break;

	/*
	 * detection by LIM that the capabilities of the associated
	 * AP have changed.
	 */
	case eSIR_SME_AP_CAPS_CHANGED:
		pApNewCaps = &pStatusChangeMsg->statusChangeInfo.apNewCaps;
		sms_log(mac_ctx, LOGW,
			FL("CSR handling eSIR_SME_AP_CAPS_CHANGED"));
		status = csr_roam_get_session_id_from_bssid(mac_ctx,
					&pApNewCaps->bssId, &sessionId);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		if (eCSR_ROAMING_STATE_JOINED ==
			mac_ctx->roam.curState[sessionId]
		    && ((eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC
			== mac_ctx->roam.curSubState[sessionId])
		    || (eCSR_ROAM_SUBSTATE_NONE ==
			mac_ctx->roam.curSubState[sessionId])
		    || (eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC
			== mac_ctx->roam.curSubState[sessionId])
		    || (eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC ==
			 mac_ctx->roam.curSubState[sessionId]))) {
			sms_log(mac_ctx, LOGW,
				FL("Calling csr_roam_disconnect_internal"));
			csr_roam_disconnect_internal(mac_ctx, sessionId,
					eCSR_DISCONNECT_REASON_UNSPECIFIED);
		} else {
			sms_log(mac_ctx, LOGW,
				FL("Skipping the new scan as CSR is in state %s and sub-state %s"),
				mac_trace_getcsr_roam_state(
					mac_ctx->roam.curState[sessionId]),
				mac_trace_getcsr_roam_sub_state(
					mac_ctx->roam.curSubState[sessionId]));
			/* We ignore the caps change event if CSR is not in full
			 * connected state. Send one event to PE to reset
			 * limSentCapsChangeNtf Once limSentCapsChangeNtf set
			 * 0, lim can send sub sequent CAPS change event
			 * otherwise lim cannot send any CAPS change events to
			 * SME
			 */
			csr_send_reset_ap_caps_changed(mac_ctx,
						       &pApNewCaps->bssId);
		}
		break;

	default:
		roamStatus = eCSR_ROAM_FAILED;
		result = eCSR_ROAM_RESULT_NONE;
		break;
	} /* end switch on statusChangeCode */
	if (eCSR_ROAM_RESULT_NONE != result) {
		csr_roam_call_callback(mac_ctx, sessionId, roam_info_ptr, 0,
				       roamStatus, result);
	}
}

static void
csr_roam_chk_lnk_ibss_new_peer_ind(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSmeIbssPeerInd *pIbssPeerInd = (tSmeIbssPeerInd *) msg_ptr;
	tCsrRoamInfo roam_info;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	host_log_ibss_pkt_type *pIbssLog;
	WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
				 LOG_WLAN_IBSS_C);
	if (pIbssLog) {
		pIbssLog->eventId = WLAN_IBSS_EVENT_PEER_JOIN;
		qdf_copy_macaddr(&pIbssLog->peer_macaddr,
				 &pIbssPeerInd->peer_addr);
		WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	sessionId = csr_find_ibss_session(mac_ctx);
	if (CSR_SESSION_ID_INVALID == sessionId)
		return;
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		return;
	}
	/*
	 * Issue the set Context request to LIM to establish the Unicast STA
	 * context for the new peer...
	 */
	if (!session->pConnectBssDesc) {
		sms_log(mac_ctx, LOGW, FL("CSR: connected BSS is empty"));
		goto callback_and_free;
	}
	qdf_copy_macaddr(&roam_info.peerMac, &pIbssPeerInd->peer_addr);
	qdf_mem_copy(&roam_info.bssid, session->pConnectBssDesc->bssId,
		     sizeof(struct qdf_mac_addr));
	if (pIbssPeerInd->mesgLen > sizeof(tSmeIbssPeerInd)) {
		roam_info.pbFrames = qdf_mem_malloc((pIbssPeerInd->mesgLen -
					sizeof(tSmeIbssPeerInd)));
		if (NULL == roam_info.pbFrames) {
			status = QDF_STATUS_E_NOMEM;
		} else {
			status = QDF_STATUS_SUCCESS;
			roam_info.nBeaconLength = pIbssPeerInd->mesgLen -
							sizeof(tSmeIbssPeerInd);
			qdf_mem_copy(roam_info.pbFrames,
				((uint8_t *) pIbssPeerInd) +
				sizeof(tSmeIbssPeerInd),
				roam_info.nBeaconLength);
		}
		roam_info.staId = (uint8_t) pIbssPeerInd->staId;
		roam_info.ucastSig = (uint8_t) pIbssPeerInd->ucastSig;
		roam_info.bcastSig = (uint8_t) pIbssPeerInd->bcastSig;
		roam_info.pBssDesc = qdf_mem_malloc(
					session->pConnectBssDesc->length);
		if (NULL == roam_info.pBssDesc) {
			status = QDF_STATUS_E_NOMEM;
			if (roam_info.pbFrames)
				qdf_mem_free(roam_info.pbFrames);
			if (roam_info.pBssDesc)
				qdf_mem_free(roam_info.pBssDesc);
		} else {
			status = QDF_STATUS_SUCCESS;
			qdf_mem_copy(roam_info.pBssDesc,
				     session->pConnectBssDesc,
				     session->pConnectBssDesc->length);
			roam_info_ptr = &roam_info;
		}
	} else {
		roam_info_ptr = &roam_info;
	}
	if ((eCSR_ENCRYPT_TYPE_NONE ==
		session->connectedProfile.EncryptionType)) {
		/* NO keys. these key parameters don't matter */
		csr_roam_issue_set_context_req(mac_ctx, sessionId,
			session->connectedProfile.EncryptionType,
			session->pConnectBssDesc,
			&pIbssPeerInd->peer_addr.bytes,
			false, true, eSIR_TX_RX, 0, 0, NULL, 0);
	}

callback_and_free:
	/* send up the sec type for the new peer */
	if (roam_info_ptr)
		roam_info_ptr->u.pConnectedProfile = &session->connectedProfile;
	csr_roam_call_callback(mac_ctx, sessionId, roam_info_ptr, 0,
			       eCSR_ROAM_CONNECT_STATUS_UPDATE,
			       eCSR_ROAM_RESULT_IBSS_NEW_PEER);
	if (roam_info_ptr) {
		if (roam_info.pbFrames)
			qdf_mem_free(roam_info.pbFrames);
		if (roam_info.pBssDesc)
			qdf_mem_free(roam_info.pBssDesc);
	}
}

static void
csr_roam_chk_lnk_ibss_peer_departed_ind(tpAniSirGlobal mac_ctx,
					tSirSmeRsp *msg_ptr)
{
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	tCsrRoamInfo roam_info;
	tSmeIbssPeerInd *pIbssPeerInd;

	if (NULL == msg_ptr) {
		sms_log(mac_ctx, LOGE, FL("IBSS peer ind. message is NULL"));
		return;
	}
	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	pIbssPeerInd = (tSmeIbssPeerInd *) msg_ptr;
	sessionId = csr_find_ibss_session(mac_ctx);
	if (CSR_SESSION_ID_INVALID != sessionId) {
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
		host_log_ibss_pkt_type *pIbssLog;
		WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
					 LOG_WLAN_IBSS_C);
		if (pIbssLog) {
			pIbssLog->eventId = WLAN_IBSS_EVENT_PEER_LEAVE;
			if (pIbssPeerInd) {
				qdf_copy_macaddr(&pIbssLog->peer_macaddr,
						 &pIbssPeerInd->peer_addr);
			}
			WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
		}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
		sms_log(mac_ctx, LOGW,
			FL("CSR: Peer departed notification from LIM"));
		roam_info.staId = (uint8_t) pIbssPeerInd->staId;
		roam_info.ucastSig = (uint8_t) pIbssPeerInd->ucastSig;
		roam_info.bcastSig = (uint8_t) pIbssPeerInd->bcastSig;
		qdf_copy_macaddr(&roam_info.peerMac, &pIbssPeerInd->peer_addr);
		csr_roam_call_callback(mac_ctx, sessionId, &roam_info, 0,
				       eCSR_ROAM_CONNECT_STATUS_UPDATE,
				       eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED);
	}
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void
csr_roam_diag_set_ctx_rsp(tpAniSirGlobal mac_ctx,
			  tCsrRoamSession *session,
			  tSirSmeSetContextRsp *pRsp)
{
	WLAN_HOST_DIAG_EVENT_DEF(setKeyEvent,
				 host_event_wlan_security_payload_type);
	if (eCSR_ENCRYPT_TYPE_NONE ==
		session->connectedProfile.EncryptionType)
		return;
	qdf_mem_set(&setKeyEvent,
		    sizeof(host_event_wlan_security_payload_type), 0);
	if (qdf_is_macaddr_group(&pRsp->peer_macaddr))
		setKeyEvent.eventId =
			WLAN_SECURITY_EVENT_SET_BCAST_RSP;
	else
		setKeyEvent.eventId =
			WLAN_SECURITY_EVENT_SET_UNICAST_RSP;
	setKeyEvent.encryptionModeMulticast =
		(uint8_t) diag_enc_type_from_csr_type(
				session->connectedProfile.mcEncryptionType);
	setKeyEvent.encryptionModeUnicast =
		(uint8_t) diag_enc_type_from_csr_type(
				session->connectedProfile.EncryptionType);
	qdf_mem_copy(setKeyEvent.bssid, session->connectedProfile.bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	setKeyEvent.authMode =
		(uint8_t) diag_auth_type_from_csr_type(
					session->connectedProfile.AuthType);
	if (eSIR_SME_SUCCESS != pRsp->statusCode)
		setKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;
	WLAN_HOST_DIAG_EVENT_REPORT(&setKeyEvent, EVENT_WLAN_SECURITY);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

static void
csr_roam_chk_lnk_set_ctx_rsp(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	tCsrRoamSession *session;
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	QDF_STATUS status;
	tCsrRoamInfo *roam_info_ptr = NULL;
	tSmeCmd *cmd;
	tCsrRoamInfo roam_info;
	eCsrRoamResult result = eCSR_ROAM_RESULT_NONE;
	tSirSmeSetContextRsp *pRsp = (tSirSmeSetContextRsp *) msg_ptr;
	tListElem *entry;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	entry = csr_ll_peek_head(&mac_ctx->sme.smeCmdActiveList,
				 LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac_ctx, LOGE, FL("CSR: NO commands are ACTIVE ..."));
		goto process_pending_n_exit;
	}

	cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (eSmeCommandSetKey != cmd->command) {
		sms_log(mac_ctx, LOGE, FL("CSR: setkey cmd is not ACTIVE ..."));
		goto process_pending_n_exit;
	}
	sessionId = cmd->sessionId;
	session = CSR_GET_SESSION(mac_ctx, sessionId);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), sessionId);
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_roam_diag_set_ctx_rsp(mac_ctx, session, pRsp);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	if (CSR_IS_WAIT_FOR_KEY(mac_ctx, sessionId)) {
		csr_roam_stop_wait_for_key_timer(mac_ctx);
		/* We are done with authentication, whethere succeed or not */
		csr_roam_substate_change(mac_ctx, eCSR_ROAM_SUBSTATE_NONE,
					 sessionId);
		/* We do it here because this linkup function is not called
		 * after association  when a key needs to be set.
		 */
		if (csr_is_conn_state_connected_infra(mac_ctx, sessionId))
			csr_roam_link_up(mac_ctx,
					 session->connectedProfile.bssid);
	}
	if (eSIR_SME_SUCCESS == pRsp->statusCode) {
		qdf_copy_macaddr(&roam_info.peerMac, &pRsp->peer_macaddr);
		/* Make sure we install the GTK before indicating to HDD as
		 * authenticated. This is to prevent broadcast packets go out
		 * after PTK and before GTK.
		 */
		if (qdf_is_macaddr_broadcast(&pRsp->peer_macaddr)) {
			tpSirSetActiveModeSetBncFilterReq pMsg;
			pMsg = qdf_mem_malloc(
				    sizeof(tSirSetActiveModeSetBncFilterReq));
			pMsg->messageType = eWNI_SME_SET_BCN_FILTER_REQ;
			pMsg->length = sizeof(uint8_t);
			pMsg->seesionId = sessionId;
			status = cds_send_mb_message_to_mac(pMsg);
			/*
			 * OBSS SCAN Indication will be sent to Firmware
			 * to start OBSS Scan
			 */
			if (CSR_IS_CHANNEL_24GHZ(
				session->connectedProfile.operationChannel)
				&& (session->connectState ==
					eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED)
				&& session->pCurRoamProfile
				&& ((QDF_P2P_CLIENT_MODE ==
				     session->pCurRoamProfile->csrPersona)
				|| (QDF_STA_MODE ==
				     session->pCurRoamProfile->csrPersona))) {
				struct sme_obss_ht40_scanind_msg *msg;
				msg = qdf_mem_malloc(sizeof(
					struct sme_obss_ht40_scanind_msg));
				msg->msg_type = eWNI_SME_HT40_OBSS_SCAN_IND;
				msg->length =
				      sizeof(struct sme_obss_ht40_scanind_msg);
				qdf_copy_macaddr(&msg->mac_addr,
					&session->connectedProfile.bssid);
				status = cds_send_mb_message_to_mac(msg);
			}
			result = eCSR_ROAM_RESULT_AUTHENTICATED;
		} else {
			result = eCSR_ROAM_RESULT_NONE;
		}
		roam_info_ptr = &roam_info;
	} else {
		result = eCSR_ROAM_RESULT_FAILURE;
		sms_log(mac_ctx, LOGE,
			FL("CSR: setkey command failed(err=%d) PeerMac "
			MAC_ADDRESS_STR),
			pRsp->statusCode,
			MAC_ADDR_ARRAY(pRsp->peer_macaddr.bytes));
	}
	csr_roam_call_callback(mac_ctx, sessionId, &roam_info,
			       cmd->u.setKeyCmd.roamId,
			       eCSR_ROAM_SET_KEY_COMPLETE, result);
	/* Indicate SME_QOS that the SET_KEY is completed, so that SME_QOS
	 * can go ahead and initiate the TSPEC if any are pending
	 */
	sme_qos_csr_event_ind(mac_ctx, (uint8_t) sessionId,
			      SME_QOS_CSR_SET_KEY_SUCCESS_IND, NULL);
#ifdef FEATURE_WLAN_ESE
	/* Send Adjacent AP repot to new AP. */
	if (result == eCSR_ROAM_RESULT_AUTHENTICATED
	    && session->isPrevApInfoValid
	    && session->connectedProfile.isESEAssoc) {
		csr_send_ese_adjacent_ap_rep_ind(mac_ctx, session);
		session->isPrevApInfoValid = false;
	}
#endif
	if (csr_ll_remove_entry(&mac_ctx->sme.smeCmdActiveList, entry,
				LL_ACCESS_LOCK))
		csr_release_command_set_key(mac_ctx, cmd);

process_pending_n_exit:
	sme_process_pending_queue(mac_ctx);
}


static void
csr_roam_chk_lnk_max_assoc_exceeded(tpAniSirGlobal mac_ctx, tSirSmeRsp *msg_ptr)
{
	uint32_t sessionId = CSR_SESSION_ID_INVALID;
	tSmeMaxAssocInd *pSmeMaxAssocInd;
	tCsrRoamInfo roam_info;

	qdf_mem_set(&roam_info, sizeof(roam_info), 0);
	pSmeMaxAssocInd = (tSmeMaxAssocInd *) msg_ptr;
	sms_log(mac_ctx, LOG1,
		FL("max assoc have been reached, new peer cannot be accepted"));
	sessionId = pSmeMaxAssocInd->sessionId;
	roam_info.sessionId = sessionId;
	qdf_copy_macaddr(&roam_info.peerMac, &pSmeMaxAssocInd->peer_mac);
	csr_roam_call_callback(mac_ctx, sessionId, &roam_info, 0,
			       eCSR_ROAM_INFRA_IND,
			       eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED);
}

void csr_roam_check_for_link_status_change(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg)
{
	if (NULL == pSirMsg) {
		sms_log(pMac, LOGE, FL("pSirMsg is NULL"));
		return;
	}
	switch (pSirMsg->messageType) {
	case eWNI_SME_ASSOC_IND:
		csr_roam_chk_lnk_assoc_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_DISASSOC_IND:
		csr_roam_chk_lnk_disassoc_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_DEAUTH_IND:
		csr_roam_chk_lnk_deauth_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_SWITCH_CHL_IND:
		csr_roam_chk_lnk_swt_ch_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_DEAUTH_RSP:
		csr_roam_chk_lnk_deauth_rsp(pMac, pSirMsg);
		break;
	case eWNI_SME_DISASSOC_RSP:
		csr_roam_chk_lnk_disassoc_rsp(pMac, pSirMsg);
		break;
	case eWNI_SME_MIC_FAILURE_IND:
		csr_roam_chk_lnk_mic_fail_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_WPS_PBC_PROBE_REQ_IND:
		csr_roam_chk_lnk_pbs_probe_req_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_WM_STATUS_CHANGE_NTF:
		csr_roam_chk_lnk_wm_status_change_ntf(pMac, pSirMsg);
		break;
	case eWNI_SME_IBSS_NEW_PEER_IND:
		csr_roam_chk_lnk_ibss_new_peer_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_IBSS_PEER_DEPARTED_IND:
		csr_roam_chk_lnk_ibss_peer_departed_ind(pMac, pSirMsg);
		break;
	case eWNI_SME_SETCONTEXT_RSP:
		csr_roam_chk_lnk_set_ctx_rsp(pMac, pSirMsg);
		break;
	case eWNI_SME_GET_STATISTICS_RSP:
		sms_log(pMac, LOG2, FL("Stats rsp from PE"));
		csr_roam_stats_rsp_processor(pMac, pSirMsg);
		break;
#ifdef FEATURE_WLAN_ESE
	case eWNI_SME_GET_TSM_STATS_RSP:
		sms_log(pMac, LOG2, FL("TSM Stats rsp from PE"));
		csr_tsm_stats_rsp_processor(pMac, pSirMsg);
		break;
#endif /* FEATURE_WLAN_ESE */
	case eWNI_SME_GET_RSSI_REQ:
		sms_log(pMac, LOG2, FL("GetRssiReq from self"));
		csr_update_rssi(pMac, pSirMsg);
		break;
	case eWNI_SME_GET_SNR_REQ:
		sms_log(pMac, LOG2, FL("GetSnrReq from self"));
		csr_update_snr(pMac, pSirMsg);
		break;
	case eWNI_SME_FT_PRE_AUTH_RSP:
		csr_roam_ft_pre_auth_rsp_processor(pMac, (tpSirFTPreAuthRsp) pSirMsg);
		break;
	case eWNI_SME_MAX_ASSOC_EXCEEDED:
		csr_roam_chk_lnk_max_assoc_exceeded(pMac, pSirMsg);
		break;
	case eWNI_SME_CANDIDATE_FOUND_IND:
		sms_log(pMac, LOG2, FL("Candidate found indication from PE"));
		csr_neighbor_roam_candidate_found_ind_hdlr(pMac, pSirMsg);
		break;
	case eWNI_SME_HANDOFF_REQ:
		sms_log(pMac, LOG2, FL("Handoff Req from self"));
		csr_neighbor_roam_handoff_req_hdlr(pMac, pSirMsg);
		break;
	default:
		break;
	} /* end switch on message type */
}

void csr_call_roaming_completion_callback(tpAniSirGlobal pMac,
					  tCsrRoamSession *pSession,
					  tCsrRoamInfo *pRoamInfo, uint32_t roamId,
					  eCsrRoamResult roamResult)
{
	if (pSession) {
		if (pSession->bRefAssocStartCnt) {
			pSession->bRefAssocStartCnt--;

			if (0 != pSession->bRefAssocStartCnt) {
				QDF_ASSERT(pSession->bRefAssocStartCnt == 0);
				return;
			}
			/* Need to call association_completion because there is an assoc_start pending. */
			csr_roam_call_callback(pMac, pSession->sessionId, NULL,
					       roamId,
					       eCSR_ROAM_ASSOCIATION_COMPLETION,
					       eCSR_ROAM_RESULT_FAILURE);
		}
		csr_roam_call_callback(pMac, pSession->sessionId, pRoamInfo,
				       roamId, eCSR_ROAM_ROAMING_COMPLETION,
				       roamResult);
	} else {
		sms_log(pMac, LOGW, FL("  pSession is NULL"));
	}
}

QDF_STATUS csr_roam_start_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
				  eCsrRoamingReason roamingReason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	if (CSR_IS_LOSTLINK_ROAMING(roamingReason) &&
	    (false == pMac->roam.roamSession[sessionId].fCancelRoaming)) {
		status = csr_scan_request_lost_link1(pMac, sessionId);
	}
	return status;
}

/* return a bool to indicate whether roaming completed or continue. */
bool csr_roam_complete_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
			       bool fForce, eCsrRoamResult roamResult)
{
	bool fCompleted = true;
	uint32_t roamTime =
		(uint32_t) (pMac->roam.configParam.nRoamingTime *
			    QDF_TICKS_PER_SECOND);
	uint32_t curTime = (uint32_t) qdf_mc_timer_get_system_ticks();
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return false;
	}
	/* Check whether time is up */
	if (pSession->fCancelRoaming || fForce ||
	    ((curTime - pSession->roamingStartTime) > roamTime) ||
	    eCsrReassocRoaming == pSession->roamingReason ||
	    eCsrDynamicRoaming == pSession->roamingReason) {
		sms_log(pMac, LOGW, FL("  indicates roaming completion"));
		if (pSession->fCancelRoaming
		    && CSR_IS_LOSTLINK_ROAMING(pSession->roamingReason)) {
			/* roaming is cancelled, tell HDD to indicate disconnect */
			/* Because LIM overload deauth_ind for both deauth frame and missed beacon */
			/* we need to use this logic to detinguish it. For missed beacon, LIM set reason */
			/* to be eSIR_BEACON_MISSED */
			if (eSIR_BEACON_MISSED == pSession->roamingStatusCode) {
				roamResult = eCSR_ROAM_RESULT_LOSTLINK;
			} else if (eCsrLostlinkRoamingDisassoc ==
				   pSession->roamingReason) {
				roamResult = eCSR_ROAM_RESULT_DISASSOC_IND;
			} else if (eCsrLostlinkRoamingDeauth ==
				   pSession->roamingReason) {
				roamResult = eCSR_ROAM_RESULT_DEAUTH_IND;
			} else {
				roamResult = eCSR_ROAM_RESULT_LOSTLINK;
			}
		}
		csr_call_roaming_completion_callback(pMac, pSession, NULL, 0,
						     roamResult);
		pSession->roamingReason = eCsrNotRoaming;
	} else {
		pSession->roamResult = roamResult;
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_roam_start_roaming_timer
				    (pMac, sessionId, QDF_MC_TIMER_TO_SEC_UNIT))) {
			csr_call_roaming_completion_callback(pMac, pSession, NULL,
							     0, roamResult);
			pSession->roamingReason = eCsrNotRoaming;
		} else {
			fCompleted = false;
		}
	}
	return fCompleted;
}

void csr_roam_cancel_roaming(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	if (CSR_IS_ROAMING(pSession)) {
		sms_log(pMac, LOGW, "Cancel roaming");
		pSession->fCancelRoaming = true;
		if (CSR_IS_ROAM_JOINING(pMac, sessionId)
		    && CSR_IS_ROAM_SUBSTATE_CONFIG(pMac, sessionId)) {
			/* No need to do anything in here because the handler takes care of it */
		} else {
			eCsrRoamResult roamResult =
				CSR_IS_LOSTLINK_ROAMING(pSession->
							roamingReason) ?
				eCSR_ROAM_RESULT_LOSTLINK : eCSR_ROAM_RESULT_NONE;
			/* Roaming is stopped after here */
			csr_roam_complete_roaming(pMac, sessionId, true,
						  roamResult);
			/* Since CSR may be in lostlink roaming situation, abort all roaming related activities */
			csr_scan_abort_mac_scan(pMac, sessionId,
						eCSR_SCAN_ABORT_DEFAULT);
			csr_roam_stop_roaming_timer(pMac, sessionId);
		}
	}
}

void csr_roam_roaming_timer_handler(void *pv)
{
	tCsrTimerInfo *pInfo = (tCsrTimerInfo *) pv;
	tpAniSirGlobal pMac = pInfo->pMac;
	uint32_t sessionId = pInfo->sessionId;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	if (false == pSession->fCancelRoaming) {
		if (!QDF_IS_STATUS_SUCCESS
			    (csr_roam_start_roaming
				    (pMac, sessionId, pSession->roamingReason))) {
			csr_call_roaming_completion_callback(pMac, pSession, NULL,
							     0,
							     pSession->roamResult);
			pSession->roamingReason = eCsrNotRoaming;
		}
	}
}

QDF_STATUS csr_roam_start_roaming_timer(tpAniSirGlobal pMac, uint32_t sessionId,
					uint32_t interval)
{
	QDF_STATUS status;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sms_log(pMac, LOG1, " csrScanStartRoamingTimer");
	pSession->roamingTimerInfo.sessionId = (uint8_t) sessionId;
	status = qdf_mc_timer_start(&pSession->hTimerRoaming,
				    interval / QDF_MC_TIMER_TO_MS_UNIT);

	return status;
}

QDF_STATUS csr_roam_stop_roaming_timer(tpAniSirGlobal pMac, uint32_t sessionId)
{
	return qdf_mc_timer_stop
			(&pMac->roam.roamSession[sessionId].hTimerRoaming);
}

void csr_roam_wait_for_key_time_out_handler(void *pv)
{
	tCsrTimerInfo *pInfo = (tCsrTimerInfo *) pv;
	tpAniSirGlobal pMac = pInfo->pMac;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, pInfo->sessionId);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (pSession == NULL) {
		sms_log(pMac, LOGE, "%s: session not found", __func__);
		return;
	}

	sms_log(pMac, LOGW,
		FL("WaitForKey timer expired in state=%s sub-state=%s"),
		mac_trace_get_neighbour_roam_state(pMac->roam.
						   neighborRoamInfo[pInfo->sessionId].
						   neighborRoamState),
		mac_trace_getcsr_roam_sub_state(pMac->roam.
						curSubState[pInfo->sessionId]));

	if (CSR_IS_WAIT_FOR_KEY(pMac, pInfo->sessionId)) {
		if (csr_neighbor_roam_is_handoff_in_progress(pMac, pInfo->sessionId)) {
			/*
			 * Enable heartbeat timer when hand-off is in progress
			 * and Key Wait timer expired.
			 */
			sms_log(pMac, LOG2,
				"Enabling HB timer after WaitKey expiry"
				" (nHBCount=%d)",
				pMac->roam.configParam.HeartbeatThresh24);
			cfg_set_int(pMac, WNI_CFG_HEART_BEAT_THRESHOLD,
					pMac->roam.configParam.HeartbeatThresh24);
		}
		sms_log(pMac, LOGE, " SME pre-auth state timeout. ");

		/* Change the substate so command queue is unblocked. */
		if (CSR_ROAM_SESSION_MAX > pInfo->sessionId) {
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 pInfo->sessionId);
		}

		if (csr_is_conn_state_connected_infra(pMac, pInfo->sessionId)) {
			csr_roam_link_up(pMac,
					 pSession->connectedProfile.bssid);
			sme_process_pending_queue(pMac);
			status = sme_acquire_global_lock(&pMac->sme);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				csr_roam_disconnect(pMac, pInfo->sessionId,
					eCSR_DISCONNECT_REASON_UNSPECIFIED);
				sme_release_global_lock(&pMac->sme);
			}
		} else {
			sms_log(pMac, LOGE, "%s: session not found", __func__);
		}
	}

}

QDF_STATUS csr_roam_start_wait_for_key_timer(tpAniSirGlobal pMac, uint32_t interval)
{
	QDF_STATUS status;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[pMac->roam.WaitForKeyTimerInfo.
					     sessionId];
	if (csr_neighbor_roam_is_handoff_in_progress(pMac,
				     pMac->roam.WaitForKeyTimerInfo.
				     sessionId)) {
		/* Disable heartbeat timer when hand-off is in progress */
		sms_log(pMac, LOG2,
			FL("disabling HB timer in state=%s sub-state=%s"),
			mac_trace_get_neighbour_roam_state(
				pNeighborRoamInfo->neighborRoamState),
			mac_trace_getcsr_roam_sub_state(
				pMac->roam.curSubState[pMac->roam.
					WaitForKeyTimerInfo.sessionId]));
		cfg_set_int(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, 0);
	}
	sms_log(pMac, LOG1, " csrScanStartWaitForKeyTimer");
	status = qdf_mc_timer_start(&pMac->roam.hTimerWaitForKey,
				    interval / QDF_MC_TIMER_TO_MS_UNIT);

	return status;
}

QDF_STATUS csr_roam_stop_wait_for_key_timer(tpAniSirGlobal pMac)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[pMac->roam.WaitForKeyTimerInfo.
					     sessionId];

	sms_log(pMac, LOG2,
		FL("WaitForKey timer stopped in state=%s sub-state=%s"),
		mac_trace_get_neighbour_roam_state(pNeighborRoamInfo->
						   neighborRoamState),
		mac_trace_getcsr_roam_sub_state(pMac->roam.
						curSubState[pMac->roam.
							    WaitForKeyTimerInfo.
							    sessionId]));
	if (csr_neighbor_roam_is_handoff_in_progress(pMac,
						     pMac->roam.WaitForKeyTimerInfo.
						     sessionId)) {
		/*
		 * Enable heartbeat timer when hand-off is in progress
		 * and Key Wait timer got stopped for some reason
		 */
		sms_log(pMac, LOG2, "Enabling HB timer after WaitKey stop"
			" (nHBCount=%d)",
			pMac->roam.configParam.HeartbeatThresh24);
		cfg_set_int(pMac, WNI_CFG_HEART_BEAT_THRESHOLD,
				pMac->roam.configParam.HeartbeatThresh24);
	}
	return qdf_mc_timer_stop(&pMac->roam.hTimerWaitForKey);
}

void csr_roam_completion(tpAniSirGlobal pMac, uint32_t sessionId,
			 tCsrRoamInfo *pRoamInfo, tSmeCmd *pCommand,
			 eCsrRoamResult roamResult, bool fSuccess)
{
	eRoamCmdStatus roamStatus = csr_get_roam_complete_status(pMac, sessionId);
	uint32_t roamId = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	if (pCommand) {
		roamId = pCommand->u.roamCmd.roamId;
		if (sessionId != pCommand->sessionId) {
			QDF_ASSERT(sessionId == pCommand->sessionId);
			return;
		}
	}
	if (eCSR_ROAM_ROAMING_COMPLETION == roamStatus) {
		/* if success, force roaming completion */
		csr_roam_complete_roaming(pMac, sessionId, fSuccess, roamResult);
	} else {
		if (pSession->bRefAssocStartCnt != 0) {
			QDF_ASSERT(pSession->bRefAssocStartCnt == 0);
			return;
		}
		sms_log(pMac, LOGW,
			FL
				("  indicates association completion. roamResult = %d"),
			roamResult);
		csr_roam_call_callback(pMac, sessionId, pRoamInfo, roamId,
				       roamStatus, roamResult);
	}
}

QDF_STATUS csr_roam_lost_link(tpAniSirGlobal pMac, uint32_t sessionId,
			      uint32_t type, tSirSmeRsp *pSirMsg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeDeauthInd *pDeauthIndMsg = NULL;
	tSirSmeDisassocInd *pDisassocIndMsg = NULL;
	eCsrRoamResult result = eCSR_ROAM_RESULT_LOSTLINK;
	tCsrRoamInfo *pRoamInfo = NULL;
	tCsrRoamInfo roamInfo;
	bool fToRoam;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	/* Only need to roam for infra station. In this case P2P client will roam as well */
	fToRoam = CSR_IS_INFRASTRUCTURE(&pSession->connectedProfile);
	pSession->fCancelRoaming = false;
	if (eWNI_SME_DISASSOC_IND == type) {
		result = eCSR_ROAM_RESULT_DISASSOC_IND;
		pDisassocIndMsg = (tSirSmeDisassocInd *) pSirMsg;
		pSession->roamingStatusCode = pDisassocIndMsg->statusCode;
		pSession->joinFailStatusCode.reasonCode =
			pDisassocIndMsg->reasonCode;
	} else if (eWNI_SME_DEAUTH_IND == type) {
		result = eCSR_ROAM_RESULT_DEAUTH_IND;
		pDeauthIndMsg = (tSirSmeDeauthInd *) pSirMsg;
		pSession->roamingStatusCode = pDeauthIndMsg->statusCode;
		/* Convert into proper reason code */
		if ((pDeauthIndMsg->reasonCode == eSIR_BEACON_MISSED) ||
				(pDeauthIndMsg->reasonCode ==
				eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON))
			pSession->joinFailStatusCode.reasonCode = 0;
		else
			pSession->joinFailStatusCode.reasonCode =
				pDeauthIndMsg->reasonCode;
		/*
		 * cfg layer expects 0 as reason code if
		 * the driver dosent know the reason code
		 * eSIR_BEACON_MISSED is defined as locally
		 */
	} else {
		sms_log(pMac, LOGW, FL("gets an unknown type (%d)"), type);
		result = eCSR_ROAM_RESULT_NONE;
		pSession->joinFailStatusCode.reasonCode = 1;
	}

	/* call profile lost link routine here */
	if (!CSR_IS_INFRA_AP(&pSession->connectedProfile)) {
		csr_roam_call_callback(pMac, sessionId, NULL, 0,
				       eCSR_ROAM_LOSTLINK_DETECTED, result);
		/*Move the state to Idle after disconnection */
		csr_roam_state_change(pMac, eCSR_ROAMING_STATE_IDLE, sessionId);

	}

	if (eWNI_SME_DISASSOC_IND == type) {
		status = csr_send_mb_disassoc_cnf_msg(pMac, pDisassocIndMsg);
	} else if (eWNI_SME_DEAUTH_IND == type) {
		status = csr_send_mb_deauth_cnf_msg(pMac, pDeauthIndMsg);
	}
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		/* If fail to send confirmation to PE, not to trigger roaming */
		fToRoam = false;
	}
	/* prepare to tell HDD to disconnect */
	qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
	roamInfo.statusCode = (tSirResultCodes) pSession->roamingStatusCode;
	roamInfo.reasonCode = pSession->joinFailStatusCode.reasonCode;
	if (eWNI_SME_DISASSOC_IND == type) {
		/* staMacAddr */
		qdf_copy_macaddr(&roamInfo.peerMac,
				 &pDisassocIndMsg->peer_macaddr);
		roamInfo.staId = (uint8_t) pDisassocIndMsg->staId;
		roamInfo.reasonCode = pDisassocIndMsg->reasonCode;
	} else if (eWNI_SME_DEAUTH_IND == type) {
		/* staMacAddr */
		qdf_copy_macaddr(&roamInfo.peerMac,
				 &pDeauthIndMsg->peer_macaddr);
		roamInfo.staId = (uint8_t) pDeauthIndMsg->staId;
		roamInfo.reasonCode = pDeauthIndMsg->reasonCode;
		roamInfo.rxRssi = pDeauthIndMsg->rssi;
	}
	sms_log(pMac, LOGW, FL("roamInfo.staId: %d"), roamInfo.staId);

	/* See if we can possibly roam.  If so, start the roaming process and notify HDD
	   that we are roaming.  But if we cannot possibly roam, or if we are unable to
	   currently roam, then notify HDD of the lost link */
	if (fToRoam) {
		/* Only remove the connected BSS in infrastructure mode */
		csr_roam_remove_connected_bss_from_scan_cache(pMac,
							      &pSession->
							      connectedProfile);
		/* Not to do anying for lostlink with WDS */
		status = csr_roam_start_roaming(pMac, sessionId,
				(eWNI_SME_DEAUTH_IND == type) ?
				eCsrLostlinkRoamingDeauth :
				eCsrLostlinkRoamingDisassoc);
		if (pMac->roam.configParam.nRoamingTime) {
			status = csr_roam_start_roaming(pMac, sessionId,
					(eWNI_SME_DEAUTH_IND == type) ?
					eCsrLostlinkRoamingDeauth :
					eCsrLostlinkRoamingDisassoc);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
				/* For IBSS, we need to give some more info to HDD */
				if (csr_is_bss_type_ibss
					(pSession->connectedProfile.BSSType)) {
					roamInfo.u.pConnectedProfile =
						&pSession->connectedProfile;
					roamInfo.statusCode =
						(tSirResultCodes) pSession->
						roamingStatusCode;
					roamInfo.reasonCode =
						pSession->joinFailStatusCode.
						reasonCode;
				} else {
					roamInfo.reasonCode =
						eCsrRoamReasonSmeIssuedForLostLink;
				}
				pRoamInfo = &roamInfo;
				pSession->roamingReason =
					(eWNI_SME_DEAUTH_IND ==
					 type) ? eCsrLostlinkRoamingDeauth :
					eCsrLostlinkRoamingDisassoc;
				pSession->roamingStartTime =
					(uint32_t) qdf_mc_timer_get_system_ticks();
				csr_roam_call_callback(pMac, sessionId, pRoamInfo,
						       0, eCSR_ROAM_ROAMING_START,
						       eCSR_ROAM_RESULT_LOSTLINK);
			} else {
				sms_log(pMac, LOGW,
					" %s Fail to start roaming, status = %d",
					__func__, status);
				fToRoam = false;
			}
		} else {
			/* We are told not to roam, indicate lostlink */
			fToRoam = false;
		}
	}
	if (!fToRoam) {
		/* Tell HDD about the lost link */
		if (!CSR_IS_INFRA_AP(&pSession->connectedProfile)) {
			/* Don't call csr_roam_call_callback for GO/SoftAp case as this indication
			 * was already given as part of eWNI_SME_DISASSOC_IND msg handling in
			 * csr_roam_check_for_link_status_change API.
			 */
			csr_roam_call_callback(pMac, sessionId, &roamInfo, 0,
					       eCSR_ROAM_LOSTLINK, result);
		}

	}

	return status;
}

QDF_STATUS csr_roam_lost_link_afterhandoff_failure(tpAniSirGlobal pMac,
						   uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	pSession->fCancelRoaming = false;
	/* Only remove the connected BSS in infrastructure mode */
	csr_roam_remove_connected_bss_from_scan_cache(pMac,
						      &pSession->connectedProfile);
	if (pMac->roam.configParam.nRoamingTime) {
		status = csr_roam_start_roaming(pMac, sessionId,
				pSession->roamingReason);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/*
			 * before starting the lost link logic release
			 * the roam command for handoff
			 */
			pEntry =
				csr_ll_peek_head(&pMac->sme.smeCmdActiveList,
						 LL_ACCESS_LOCK);
			if (pEntry) {
				pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			}
			if (pCommand) {
				if ((eSmeCommandRoam == pCommand->command) &&
				    (eCsrSmeIssuedAssocToSimilarAP ==
				     pCommand->u.roamCmd.roamReason)) {
					if (csr_ll_remove_entry
						    (&pMac->sme.smeCmdActiveList,
						    pEntry, LL_ACCESS_LOCK)) {
						csr_release_command_roam(pMac,
									 pCommand);
					}
				}
			}
			sms_log(pMac, LOGW, "Lost link roaming started ...");
		}
	} else {
		/* We are told not to roam, indicate lostlink */
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

void csr_roam_wm_status_change_complete(tpAniSirGlobal pMac)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (eSmeCommandWmStatusChange == pCommand->command) {
			/* Nothing to process in a Lost Link completion....  It just kicks off a */
			/* roaming sequence. */
			if (csr_ll_remove_entry
				    (&pMac->sme.smeCmdActiveList, pEntry,
				    LL_ACCESS_LOCK)) {
				csr_release_command_wm_status_change(pMac, pCommand);
			} else {
				sms_log(pMac, LOGE,
					" ******csr_roam_wm_status_change_complete fail to release command");
			}

		} else {
			sms_log(pMac, LOGW,
				"CSR: WmStatusChange Completion called but LOST LINK command is not ACTIVE ...");
		}
	} else {
		sms_log(pMac, LOGW,
			"CSR: WmStatusChange Completion called but NO commands are ACTIVE ...");
	}
	sme_process_pending_queue(pMac);
}

void csr_roam_process_wm_status_change_command(tpAniSirGlobal pMac,
					       tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tSirSmeRsp *pSirSmeMsg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, pCommand->sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "),
			pCommand->sessionId);
		return;
	}
	sms_log(pMac, LOG1, FL("session:%d, CmdType : %d"),
		pCommand->sessionId, pCommand->u.wmStatusChangeCmd.Type);

	switch (pCommand->u.wmStatusChangeCmd.Type) {
	case eCsrDisassociated:
		pSirSmeMsg =
			(tSirSmeRsp *) &pCommand->u.wmStatusChangeCmd.u.
			DisassocIndMsg;
		status =
			csr_roam_lost_link(pMac, pCommand->sessionId,
					   eWNI_SME_DISASSOC_IND, pSirSmeMsg);
		break;
	case eCsrDeauthenticated:
		pSirSmeMsg =
			(tSirSmeRsp *) &pCommand->u.wmStatusChangeCmd.u.
			DeauthIndMsg;
		status =
			csr_roam_lost_link(pMac, pCommand->sessionId,
					   eWNI_SME_DEAUTH_IND, pSirSmeMsg);
		break;
	default:
		sms_log(pMac, LOGW, FL("gets an unknown command %d"),
			pCommand->u.wmStatusChangeCmd.Type);
		break;
	}
	/* Lost Link just triggers a roaming sequence.  We can complte the Lost Link */
	/* command here since there is nothing else to do. */
	csr_roam_wm_status_change_complete(pMac);
}


/**
 * csr_compute_mode_and_band() - computes dot11mode
 * @pMac:          mac global context
 * @dot11_mode:    out param, do11 mode calculated
 * @band:          out param, band caclculated
 * @opr_ch:        operating channels
 *
 * This function finds dot11 mode based on current mode, operating channel and
 * fw supported modes.
 *
 * Return: void
 */
static void
csr_compute_mode_and_band(tpAniSirGlobal mac_ctx,
			  eCsrCfgDot11Mode *dot11_mode,
			  eCsrBand *band,
			  uint8_t opr_ch)
{
	bool vht_24_ghz = mac_ctx->roam.configParam.enableVhtFor24GHz;
	switch (mac_ctx->roam.configParam.uCfgDot11Mode) {
	case eCSR_CFG_DOT11_MODE_11A:
		*dot11_mode = eCSR_CFG_DOT11_MODE_11A;
		*band = eCSR_BAND_5G;
		break;
	case eCSR_CFG_DOT11_MODE_11B:
		*dot11_mode = eCSR_CFG_DOT11_MODE_11B;
		*band = eCSR_BAND_24;
		break;
	case eCSR_CFG_DOT11_MODE_11G:
		*dot11_mode = eCSR_CFG_DOT11_MODE_11G;
		*band = eCSR_BAND_24;
		break;
	case eCSR_CFG_DOT11_MODE_11N:
		*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
		*band = CSR_GET_BAND(opr_ch);
		break;
	case eCSR_CFG_DOT11_MODE_11AC:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			/*
			 * If the operating channel is in 2.4 GHz band, check
			 * for INI item to disable VHT operation in 2.4 GHz band
			 */
			if (CDS_IS_CHANNEL_24GHZ(opr_ch) && !vht_24_ghz)
				/* Disable 11AC operation */
				*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
			else
				*dot11_mode = eCSR_CFG_DOT11_MODE_11AC;
		} else {
			*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
		}
		*band = CSR_GET_BAND(opr_ch);
		break;
	case eCSR_CFG_DOT11_MODE_11AC_ONLY:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			/*
			 * If the operating channel is in 2.4 GHz band, check
			 * for INI item to disable VHT operation in 2.4 GHz band
			 */
			if (CDS_IS_CHANNEL_24GHZ(opr_ch) && !vht_24_ghz)
				/* Disable 11AC operation */
				*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
			else
				*dot11_mode = eCSR_CFG_DOT11_MODE_11AC_ONLY;
		} else {
			*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
		}
		*band = CSR_GET_BAND(opr_ch);
		break;
	case eCSR_CFG_DOT11_MODE_AUTO:
		if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			/*
			 * If the operating channel is in 2.4 GHz band,
			 * check for INI item to disable VHT operation
			 * in 2.4 GHz band
			 */
			if (CDS_IS_CHANNEL_24GHZ(opr_ch)
				&& !vht_24_ghz)
				/* Disable 11AC operation */
				*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
			else
				*dot11_mode = eCSR_CFG_DOT11_MODE_11AC;
		} else {
			*dot11_mode = eCSR_CFG_DOT11_MODE_11N;
		}
		*band = CSR_GET_BAND(opr_ch);
		break;
	default:
		/*
		 * Global dot11 Mode setting is 11a/b/g. use the channel number
		 * to determine the Mode setting.
		 */
		if (eCSR_OPERATING_CHANNEL_AUTO == opr_ch) {
			*band = mac_ctx->roam.configParam.eBand;
			if (eCSR_BAND_24 == *band) {
				/*
				 * See reason in else if ( CDS_IS_CHANNEL_24GHZ
				 * (opr_ch) ) to pick 11B
				 */
				*dot11_mode = eCSR_CFG_DOT11_MODE_11B;
			} else {
				/* prefer 5GHz */
				*band = eCSR_BAND_5G;
				*dot11_mode = eCSR_CFG_DOT11_MODE_11A;
			}
		} else if (CDS_IS_CHANNEL_24GHZ(opr_ch)) {
			/*
			 * WiFi tests require IBSS networks to start in 11b mode
			 * without any change to the default parameter settings
			 * on the adapter. We use ACU to start an IBSS through
			 * creation of a startIBSS profile. This startIBSS
			 * profile has Auto MACProtocol and the adapter property
			 * setting for dot11Mode is also AUTO. So in this case,
			 * let's start the IBSS network in 11b mode instead of
			 * 11g mode. So this is for Auto=profile->MacProtocol &&
			 * Auto=Global. dot11Mode && profile->channel is < 14,
			 * then start the IBSS in b mode.
			 *
			 * Note: we used to have this start as an 11g IBSS for
			 * best performance. now to specify that the user will
			 * have to set the do11Mode in the property page to 11g
			 * to force it.
			 */
			*dot11_mode = eCSR_CFG_DOT11_MODE_11B;
			*band = eCSR_BAND_24;
		} else {
			/* else, it's a 5.0GHz channel.  Set mode to 11a. */
			*dot11_mode = eCSR_CFG_DOT11_MODE_11A;
			*band = eCSR_BAND_5G;
		}
		break;
	} /* switch */
}

/**
 * csr_roam_get_phy_mode_band_for_bss() - This function returns band and mode
 * information.
 * @mac_ctx:       mac global context
 * @profile:       bss profile
 * @band:          out param, band caclculated
 * @opr_ch:        operating channels
 *
 * This function finds dot11 mode based on current mode, operating channel and
 * fw supported modes. The only tricky part is that if phyMode is set to 11abg,
 * this function may return eCSR_CFG_DOT11_MODE_11B instead of
 * eCSR_CFG_DOT11_MODE_11G if everything is set to auto-pick.
 *
 * Return: dot11mode
 */
static eCsrCfgDot11Mode
csr_roam_get_phy_mode_band_for_bss(tpAniSirGlobal mac_ctx,
				   tCsrRoamProfile *profile,
				   uint8_t opr_chn,
				   eCsrBand *p_band)
{
	eCsrBand band;
	eCsrCfgDot11Mode curr_mode = mac_ctx->roam.configParam.uCfgDot11Mode;
	eCsrCfgDot11Mode cfg_dot11_mode =
		csr_get_cfg_dot11_mode_from_csr_phy_mode(profile,
			(eCsrPhyMode) profile->phyMode,
			mac_ctx->roam.configParam.ProprietaryRatesEnabled);

	/*
	 * If the global setting for dot11Mode is set to auto/abg, we overwrite
	 * the setting in the profile.
	 */
	if ((!CSR_IS_INFRA_AP(profile)
	    && ((eCSR_CFG_DOT11_MODE_AUTO == curr_mode)
	    || (eCSR_CFG_DOT11_MODE_ABG == curr_mode)))
	    || (eCSR_CFG_DOT11_MODE_AUTO == cfg_dot11_mode)
	    || (eCSR_CFG_DOT11_MODE_ABG == cfg_dot11_mode)) {
		csr_compute_mode_and_band(mac_ctx, &cfg_dot11_mode,
					  &band, opr_chn);
	} /* if( eCSR_CFG_DOT11_MODE_ABG == cfg_dot11_mode ) */
	else {
		/* dot11 mode is set, lets pick the band */
		if (eCSR_OPERATING_CHANNEL_AUTO == opr_chn) {
			/* channel is Auto also. */
			band = mac_ctx->roam.configParam.eBand;
			if (eCSR_BAND_ALL == band) {
				/* prefer 5GHz */
				band = eCSR_BAND_5G;
			}
		} else{
			band = CSR_GET_BAND(opr_chn);
		}
	}
	if (p_band)
		*p_band = band;

	if (opr_chn == 14) {
		sms_log(mac_ctx, LOGE, FL("Switching to Dot11B mode"));
		cfg_dot11_mode = eCSR_CFG_DOT11_MODE_11B;
	}

	if (IS_24G_CH(opr_chn) &&
	   (false == mac_ctx->roam.configParam.enableVhtFor24GHz) &&
	   (eCSR_CFG_DOT11_MODE_11AC == cfg_dot11_mode ||
	    eCSR_CFG_DOT11_MODE_11AC_ONLY == cfg_dot11_mode))
		cfg_dot11_mode = eCSR_CFG_DOT11_MODE_11N;
	/*
	 * Incase of WEP Security encryption type is coming as part of add key.
	 * So while STart BSS dont have information
	 */
	if ((!CSR_IS_11n_ALLOWED(profile->EncryptionType.encryptionType[0])
	    || ((profile->privacy == 1)
		&& (profile->EncryptionType.encryptionType[0] ==
		eCSR_ENCRYPT_TYPE_NONE)))
		&& ((eCSR_CFG_DOT11_MODE_11N == cfg_dot11_mode) ||
	     (eCSR_CFG_DOT11_MODE_11AC == cfg_dot11_mode))) {
		/* We cannot do 11n here */
		if (CDS_IS_CHANNEL_24GHZ(opr_chn))
			cfg_dot11_mode = eCSR_CFG_DOT11_MODE_11G;
		else
			cfg_dot11_mode = eCSR_CFG_DOT11_MODE_11A;
	}
	return cfg_dot11_mode;
}

QDF_STATUS csr_roam_issue_stop_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				   eCsrRoamSubState NewSubstate)
{
	QDF_STATUS status;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	{
		host_log_ibss_pkt_type *pIbssLog;
		WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
					 LOG_WLAN_IBSS_C);
		if (pIbssLog) {
			pIbssLog->eventId = WLAN_IBSS_EVENT_STOP_REQ;
			WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
		}
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	/* Set the roaming substate to 'stop Bss request'... */
	csr_roam_substate_change(pMac, NewSubstate, sessionId);

	/* attempt to stop the Bss (reason code is ignored...) */
	status = csr_send_mb_stop_bss_req_msg(pMac, sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGW,
			FL("csr_send_mb_stop_bss_req_msg failed with status %d"),
			status);
	}
	return status;
}

/* pNumChan is a caller allocated space with the sizeof pChannels */
QDF_STATUS csr_get_cfg_valid_channels(tpAniSirGlobal pMac, uint8_t *pChannels,
				      uint32_t *pNumChan)
{
	uint8_t num_chan_temp = 0;
	int i;

	if (!IS_SIR_STATUS_SUCCESS(wlan_cfg_get_str(pMac,
					WNI_CFG_VALID_CHANNEL_LIST,
					(uint8_t *) pChannels, pNumChan)))
		return QDF_STATUS_E_FAILURE;

	for (i = 0; i < *pNumChan; i++) {
		if (!cds_is_dsrc_channel(cds_chan_to_freq(pChannels[i]))) {
			pChannels[num_chan_temp] = pChannels[i];
			num_chan_temp++;
		}
	}

	*pNumChan = num_chan_temp;
	return QDF_STATUS_SUCCESS;
}

int8_t csr_get_cfg_max_tx_power(tpAniSirGlobal pMac, uint8_t channel)
{
	uint32_t cfgLength = 0;
	uint16_t cfgId = 0;
	int8_t maxTxPwr = 0;
	uint8_t *pCountryInfo = NULL;
	QDF_STATUS status;
	uint8_t count = 0;
	uint8_t firstChannel;
	uint8_t maxChannels;

	if (CDS_IS_CHANNEL_5GHZ(channel)) {
		cfgId = WNI_CFG_MAX_TX_POWER_5;
		cfgLength = WNI_CFG_MAX_TX_POWER_5_LEN;
	} else if (CDS_IS_CHANNEL_24GHZ(channel)) {
		cfgId = WNI_CFG_MAX_TX_POWER_2_4;
		cfgLength = WNI_CFG_MAX_TX_POWER_2_4_LEN;
	} else
		return maxTxPwr;

	pCountryInfo = qdf_mem_malloc(cfgLength);
	if (NULL == pCountryInfo)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("%s: failed to allocate memory, status = %d"),
			  __FUNCTION__, status);
		goto error;
	}
	if (wlan_cfg_get_str(pMac, cfgId, (uint8_t *)pCountryInfo,
			&cfgLength) != eSIR_SUCCESS) {
		goto error;
	}
	/* Identify the channel and maxtxpower */
	while (count <= (cfgLength - (sizeof(tSirMacChanInfo)))) {
		firstChannel = pCountryInfo[count++];
		maxChannels = pCountryInfo[count++];
		maxTxPwr = pCountryInfo[count++];

		if ((channel >= firstChannel) &&
		    (channel < (firstChannel + maxChannels))) {
			break;
		}
	}

error:
	if (NULL != pCountryInfo)
		qdf_mem_free(pCountryInfo);

	return maxTxPwr;
}

bool csr_roam_is_channel_valid(tpAniSirGlobal pMac, uint8_t channel)
{
	bool fValid = false;
	uint32_t idxValidChannels;
	uint32_t len = sizeof(pMac->roam.validChannelList);

	if (QDF_IS_STATUS_SUCCESS
		    (csr_get_cfg_valid_channels(pMac, pMac->roam.validChannelList, &len))) {
		for (idxValidChannels = 0; (idxValidChannels < len);
		     idxValidChannels++) {
			if (channel ==
			    pMac->roam.validChannelList[idxValidChannels]) {
				fValid = true;
				break;
			}
		}
	}
	pMac->roam.numValidChannels = len;
	return fValid;
}

/* This function check and validate whether the NIC can do CB (40MHz) */
static ePhyChanBondState csr_get_cb_mode_from_ies(tpAniSirGlobal pMac,
						  uint8_t primaryChn,
						  tDot11fBeaconIEs *pIes)
{
	ePhyChanBondState eRet = PHY_SINGLE_CHANNEL_CENTERED;
	uint8_t centerChn;
	uint32_t ChannelBondingMode;
	if (CDS_IS_CHANNEL_24GHZ(primaryChn)) {
		ChannelBondingMode =
			pMac->roam.configParam.channelBondingMode24GHz;
	} else {
		ChannelBondingMode =
			pMac->roam.configParam.channelBondingMode5GHz;
	}

	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE == ChannelBondingMode)
		return PHY_SINGLE_CHANNEL_CENTERED;

	/* Figure what the other side's CB mode */
	if (!(pIes->HTCaps.present && (eHT_CHANNEL_WIDTH_40MHZ ==
		pIes->HTCaps.supportedChannelWidthSet))) {
		return PHY_SINGLE_CHANNEL_CENTERED;
	}

	/* In Case WPA2 and TKIP is the only one cipher suite in Pairwise */
	if ((pIes->RSN.present && (pIes->RSN.pwise_cipher_suite_count == 1) &&
		!memcmp(&(pIes->RSN.pwise_cipher_suites[0][0]),
					"\x00\x0f\xac\x02", 4))
		/* In Case only WPA1 is supported and TKIP is
		 * the only one cipher suite in Unicast.
		 */
		|| (!pIes->RSN.present && (pIes->WPA.present &&
			(pIes->WPA.unicast_cipher_count == 1) &&
			!memcmp(&(pIes->WPA.unicast_ciphers[0][0]),
					"\x00\x50\xf2\x02", 4)))) {
		sms_log(pMac, LOGW,
			" No channel bonding in TKIP mode ");
		return PHY_SINGLE_CHANNEL_CENTERED;
	}

	if (!pIes->HTInfo.present)
		return PHY_SINGLE_CHANNEL_CENTERED;

	/*
	 * This is called during INFRA STA/CLIENT and should use the merged
	 * value of supported channel width and recommended tx width as per
	 * standard
	 */
	sms_log(pMac, LOG1, "scws %u rtws %u sco %u",
		pIes->HTCaps.supportedChannelWidthSet,
		pIes->HTInfo.recommendedTxWidthSet,
		pIes->HTInfo.secondaryChannelOffset);

	if (pIes->HTInfo.recommendedTxWidthSet == eHT_CHANNEL_WIDTH_40MHZ)
		eRet = (ePhyChanBondState)pIes->HTInfo.secondaryChannelOffset;
	else
		eRet = PHY_SINGLE_CHANNEL_CENTERED;

	switch (eRet) {
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		centerChn = primaryChn + CSR_CB_CENTER_CHANNEL_OFFSET;
		break;
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		centerChn = primaryChn - CSR_CB_CENTER_CHANNEL_OFFSET;
		break;
	case PHY_SINGLE_CHANNEL_CENTERED:
	default:
		centerChn = primaryChn;
		break;
	}

	if ((PHY_SINGLE_CHANNEL_CENTERED != eRet) &&
	    (QDF_STATUS_SUCCESS != sme_check_ch_in_band(pMac,
							centerChn - 2, 2))) {
		sms_log(pMac, LOGE,
			"Invalid center channel (%d), disable 40MHz mode",
			centerChn);
		eRet = PHY_SINGLE_CHANNEL_CENTERED;
	}
	return eRet;
}

bool csr_is_encryption_in_list(tpAniSirGlobal pMac,
			       tCsrEncryptionList *pCipherList,
			       eCsrEncryptionType encryptionType)
{
	bool fFound = false;
	uint32_t idx;
	for (idx = 0; idx < pCipherList->numEntries; idx++) {
		if (pCipherList->encryptionType[idx] == encryptionType) {
			fFound = true;
			break;
		}
	}
	return fFound;
}

bool csr_is_auth_in_list(tpAniSirGlobal pMac, tCsrAuthList *pAuthList,
			 eCsrAuthType authType)
{
	bool fFound = false;
	uint32_t idx;
	for (idx = 0; idx < pAuthList->numEntries; idx++) {
		if (pAuthList->authType[idx] == authType) {
			fFound = true;
			break;
		}
	}
	return fFound;
}

bool csr_is_same_profile(tpAniSirGlobal pMac,
			 tCsrRoamConnectedProfile *pProfile1,
			 tCsrRoamProfile *pProfile2)
{
	uint32_t i;
	bool fCheck = false;
	tCsrScanResultFilter *pScanFilter = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!(pProfile1 && pProfile2))
		return fCheck;
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter)
		return fCheck;

	qdf_mem_set(pScanFilter, sizeof(tCsrScanResultFilter), 0);
	status = csr_roam_prepare_filter_from_profile(pMac, pProfile2,
						      pScanFilter);
	if (!(QDF_IS_STATUS_SUCCESS(status)))
		goto free_scan_filter;

	for (i = 0; i < pScanFilter->SSIDs.numOfSSIDs; i++) {
		fCheck = csr_is_ssid_match(pMac,
				pScanFilter->SSIDs.SSIDList[i].SSID.ssId,
				pScanFilter->SSIDs.SSIDList[i].SSID.length,
				pProfile1->SSID.ssId,
				pProfile1->SSID.length,
				false);
		if (fCheck)
			break;
	}
	if (!fCheck)
		goto free_scan_filter;

	if (!csr_is_auth_in_list(pMac, &pProfile2->AuthType,
				 pProfile1->AuthType)
	    || (pProfile2->BSSType != pProfile1->BSSType)
	    || !csr_is_encryption_in_list(pMac, &pProfile2->EncryptionType,
					  pProfile1->EncryptionType)) {
		fCheck = false;
		goto free_scan_filter;
	}
	if (pProfile1->MDID.mdiePresent || pProfile2->MDID.mdiePresent) {
		if (pProfile1->MDID.mobilityDomain
			!= pProfile2->MDID.mobilityDomain) {
			fCheck = false;
			goto free_scan_filter;
		}
	}
	/* Match found */
	fCheck = true;
free_scan_filter:
	csr_free_scan_filter(pMac, pScanFilter);
	qdf_mem_free(pScanFilter);
	return fCheck;
}

bool csr_roam_is_same_profile_keys(tpAniSirGlobal pMac,
				   tCsrRoamConnectedProfile *pConnProfile,
				   tCsrRoamProfile *pProfile2)
{
	bool fCheck = false;
	int i;
	do {
		/* Only check for static WEP */
		if (!csr_is_encryption_in_list
			    (pMac, &pProfile2->EncryptionType,
			    eCSR_ENCRYPT_TYPE_WEP40_STATICKEY)
		    && !csr_is_encryption_in_list(pMac,
				&pProfile2->EncryptionType,
				eCSR_ENCRYPT_TYPE_WEP104_STATICKEY)) {
			fCheck = true;
			break;
		}
		if (!csr_is_encryption_in_list
			    (pMac, &pProfile2->EncryptionType,
			    pConnProfile->EncryptionType))
			break;
		if (pConnProfile->Keys.defaultIndex !=
		    pProfile2->Keys.defaultIndex)
			break;
		for (i = 0; i < CSR_MAX_NUM_KEY; i++) {
			if (pConnProfile->Keys.KeyLength[i] !=
			    pProfile2->Keys.KeyLength[i])
				break;
			if (qdf_mem_cmp(&pConnProfile->Keys.KeyMaterial[i],
					     &pProfile2->Keys.KeyMaterial[i],
					     pProfile2->Keys.KeyLength[i])) {
				break;
			}
		}
		if (i == CSR_MAX_NUM_KEY) {
			fCheck = true;
		}
	} while (0);
	return fCheck;
}

/* IBSS */

uint8_t csr_roam_get_ibss_start_channel_number50(tpAniSirGlobal pMac)
{
	uint8_t channel = 0;
	uint32_t idx;
	uint32_t idxValidChannels;
	bool fFound = false;
	uint32_t len = sizeof(pMac->roam.validChannelList);

	if (eCSR_OPERATING_CHANNEL_ANY != pMac->roam.configParam.AdHocChannel5G) {
		channel = pMac->roam.configParam.AdHocChannel5G;
		if (!csr_roam_is_channel_valid(pMac, channel)) {
			channel = 0;
		}
	}
	if (0 == channel
	    &&
	    QDF_IS_STATUS_SUCCESS(csr_get_cfg_valid_channels
					  (pMac,
					  (uint8_t *) pMac->roam.validChannelList,
					  &len))) {
		for (idx = 0; (idx < CSR_NUM_IBSS_START_CHANNELS_50) && !fFound;
		     idx++) {
			for (idxValidChannels = 0;
			     (idxValidChannels < len) && !fFound;
			     idxValidChannels++) {
				if (csr_start_ibss_channels50[idx] ==
				    pMac->roam.
				    validChannelList[idxValidChannels]) {
					fFound = true;
					channel = csr_start_ibss_channels50[idx];
				}
			}
		}
		/*
		 * this is rare, but if it does happen,
		 * we find anyone in 11a bandwidth and
		 * return the first 11a channel found!
		 */
		if (!fFound) {
			for (idxValidChannels = 0; idxValidChannels < len;
			     idxValidChannels++) {
				if (CDS_IS_CHANNEL_5GHZ(pMac->roam.
					validChannelList[idxValidChannels])) {
					/* the max channel# in 11g is 14 */
					if (idxValidChannels <
					    CSR_NUM_IBSS_START_CHANNELS_50) {
						channel =
						pMac->roam.validChannelList
						[idxValidChannels];
					}
					break;
				}
			}
		}
	} /* if */

	return channel;
}

uint8_t csr_roam_get_ibss_start_channel_number24(tpAniSirGlobal pMac)
{
	uint8_t channel = 1;
	uint32_t idx;
	uint32_t idxValidChannels;
	bool fFound = false;
	uint32_t len = sizeof(pMac->roam.validChannelList);

	if (eCSR_OPERATING_CHANNEL_ANY != pMac->roam.configParam.AdHocChannel24) {
		channel = pMac->roam.configParam.AdHocChannel24;
		if (!csr_roam_is_channel_valid(pMac, channel)) {
			channel = 0;
		}
	}

	if (0 == channel
	    &&
	    QDF_IS_STATUS_SUCCESS(csr_get_cfg_valid_channels
					  (pMac,
					  (uint8_t *) pMac->roam.validChannelList,
					  &len))) {
		for (idx = 0; (idx < CSR_NUM_IBSS_START_CHANNELS_24) && !fFound;
		     idx++) {
			for (idxValidChannels = 0;
			     (idxValidChannels < len) && !fFound;
			     idxValidChannels++) {
				if (csr_start_ibss_channels24[idx] ==
				    pMac->roam.
				    validChannelList[idxValidChannels]) {
					fFound = true;
					channel = csr_start_ibss_channels24[idx];
				}
			}
		}
	}

	return channel;
}

/**
 * csr_populate_basic_rates() - populates OFDM or CCK rates
 * @rates:         rate struct to populate
 * @type:          true: ofdm rates, false: cck rates
 * @masked:        indicates if rates are to be masked with
 *                 CSR_DOT11_BASIC_RATE_MASK
 *
 * This function will populate OFDM or CCK rates
 *
 * Return: void
 */
static void
csr_populate_basic_rates(tSirMacRateSet *rate_set, bool type, bool masked)
{
	uint8_t ofdm_rates[8] = {
		SIR_MAC_RATE_6,
		SIR_MAC_RATE_9,
		SIR_MAC_RATE_12,
		SIR_MAC_RATE_18,
		SIR_MAC_RATE_24,
		SIR_MAC_RATE_36,
		SIR_MAC_RATE_48,
		SIR_MAC_RATE_54
	};
	uint8_t cck_rates[4] = {
		SIR_MAC_RATE_1,
		SIR_MAC_RATE_2,
		SIR_MAC_RATE_5_5,
		SIR_MAC_RATE_11
	};

	if (type == true) {
		rate_set->numRates = 8;
		qdf_mem_copy(rate_set->rate, ofdm_rates, sizeof(ofdm_rates));
		if (masked) {
			rate_set->rate[0] |= CSR_DOT11_BASIC_RATE_MASK;
			rate_set->rate[2] |= CSR_DOT11_BASIC_RATE_MASK;
			rate_set->rate[4] |= CSR_DOT11_BASIC_RATE_MASK;
		}
	} else {
		rate_set->numRates = 4;
		qdf_mem_copy(rate_set->rate, cck_rates, sizeof(cck_rates));
		if (masked) {
			rate_set->rate[0] |= CSR_DOT11_BASIC_RATE_MASK;
			rate_set->rate[1] |= CSR_DOT11_BASIC_RATE_MASK;
			rate_set->rate[2] |= CSR_DOT11_BASIC_RATE_MASK;
			rate_set->rate[3] |= CSR_DOT11_BASIC_RATE_MASK;
		}
	}
}

/**
 * csr_convert_mode_to_nw_type() - convert mode into network type
 * @dot11_mode:    dot11_mode
 * @band:          2.4 or 5 GHz
 *
 * Return: tSirNwType
 */
static tSirNwType
csr_convert_mode_to_nw_type(eCsrCfgDot11Mode dot11_mode, eCsrBand band)
{
	switch (dot11_mode) {
	case eCSR_CFG_DOT11_MODE_11G:
		return eSIR_11G_NW_TYPE;
	case eCSR_CFG_DOT11_MODE_11B:
		return eSIR_11B_NW_TYPE;
	case eCSR_CFG_DOT11_MODE_11A:
		return eSIR_11A_NW_TYPE;
	case eCSR_CFG_DOT11_MODE_11N:
	default:
		/*
		 * Because LIM only verifies it against 11a, 11b or 11g, set
		 * only 11g or 11a here
		 */
		if (eCSR_BAND_24 == band)
			return eSIR_11G_NW_TYPE;
		else
			return eSIR_11A_NW_TYPE;
	}
	return eSIR_DONOT_USE_NW_TYPE;
}

/**
 * csr_roam_get_bss_start_parms() - get bss start param from profile
 * @pMac:          mac global context
 * @pProfile:      roam profile
 * @pParam:        out param, start bss params
 *
 * This function populates start bss param from roam profile
 *
 * Return: void
 */
static void
csr_roam_get_bss_start_parms(tpAniSirGlobal pMac,
			     tCsrRoamProfile *pProfile,
			     tCsrRoamStartBssParams *pParam)
{
	eCsrBand band;
	uint8_t opr_ch = 0;
	tSirNwType nw_type;
	uint8_t tmp_opr_ch = 0;
	tSirMacRateSet *opr_rates = &pParam->operationalRateSet;
	tSirMacRateSet *ext_rates = &pParam->extendedRateSet;

	if (pProfile->ChannelInfo.numOfChannels
	    && pProfile->ChannelInfo.ChannelList) {
		tmp_opr_ch = pProfile->ChannelInfo.ChannelList[0];
	}

	pParam->uCfgDot11Mode = csr_roam_get_phy_mode_band_for_bss(pMac,
					 pProfile, tmp_opr_ch, &band);

	if (((pProfile->csrPersona == QDF_P2P_CLIENT_MODE)
	    || (pProfile->csrPersona == QDF_P2P_GO_MODE))
	    && (pParam->uCfgDot11Mode == eCSR_CFG_DOT11_MODE_11B)) {
		/* This should never happen */
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  FL("For P2P (persona %d) dot11_mode is 11B"),
			  pProfile->csrPersona);
		QDF_ASSERT(0);
	}

	nw_type = csr_convert_mode_to_nw_type(pParam->uCfgDot11Mode, band);
	ext_rates->numRates = 0;

	switch (nw_type) {
	default:
		sms_log(pMac, LOGE, FL("sees an unknown pSirNwType (%d)"),
			nw_type);
	case eSIR_11A_NW_TYPE:
		csr_populate_basic_rates(opr_rates, true, true);
		if (eCSR_OPERATING_CHANNEL_ANY != tmp_opr_ch) {
			opr_ch = tmp_opr_ch;
			break;
		}
		opr_ch = csr_roam_get_ibss_start_channel_number50(pMac);
		if (0 == opr_ch &&
		    CSR_IS_PHY_MODE_DUAL_BAND(pProfile->phyMode) &&
		    CSR_IS_PHY_MODE_DUAL_BAND(pMac->roam.configParam.phyMode)) {
			/*
			 * We could not find a 5G channel by auto pick, let's
			 * try 2.4G channels. We only do this here because
			 * csr_roam_get_phy_mode_band_for_bss always picks 11a
			 * for AUTO
			 */
			nw_type = eSIR_11B_NW_TYPE;
			opr_ch = csr_roam_get_ibss_start_channel_number24(pMac);
			csr_populate_basic_rates(opr_rates, false, true);
		}
		break;
	case eSIR_11B_NW_TYPE:
		csr_populate_basic_rates(opr_rates, false, true);
		if (eCSR_OPERATING_CHANNEL_ANY == tmp_opr_ch)
			opr_ch = csr_roam_get_ibss_start_channel_number24(pMac);
		else
			opr_ch = tmp_opr_ch;
		break;
	case eSIR_11G_NW_TYPE:
		/* For P2P Client and P2P GO, disable 11b rates */
		if ((pProfile->csrPersona == QDF_P2P_CLIENT_MODE)
		    || (pProfile->csrPersona == QDF_P2P_GO_MODE)
		    || (eCSR_CFG_DOT11_MODE_11G_ONLY ==
					pParam->uCfgDot11Mode)) {
			csr_populate_basic_rates(opr_rates, true, true);
		} else {
			csr_populate_basic_rates(opr_rates, false, true);
			csr_populate_basic_rates(ext_rates, true, false);
		}

		if (eCSR_OPERATING_CHANNEL_ANY == tmp_opr_ch)
			opr_ch = csr_roam_get_ibss_start_channel_number24(pMac);
		else
			opr_ch = tmp_opr_ch;
		break;
	}
	pParam->operationChn = opr_ch;
	pParam->sirNwType = nw_type;
	pParam->ch_params.ch_width = pProfile->ch_params.ch_width;
	pParam->ch_params.center_freq_seg0 =
		pProfile->ch_params.center_freq_seg0;
	pParam->ch_params.center_freq_seg1 =
		pProfile->ch_params.center_freq_seg1;
	pParam->ch_params.sec_ch_offset =
		pProfile->ch_params.sec_ch_offset;
}

static void
csr_roam_get_bss_start_parms_from_bss_desc(tpAniSirGlobal pMac,
					   tSirBssDescription *pBssDesc,
					   tDot11fBeaconIEs *pIes,
					   tCsrRoamStartBssParams *pParam)
{
	if (!pParam) {
		sms_log(pMac, LOGE, FL("BSS param's pointer is NULL"));
		return;
	}

	pParam->sirNwType = pBssDesc->nwType;
	pParam->cbMode = PHY_SINGLE_CHANNEL_CENTERED;
	pParam->operationChn = pBssDesc->channelId;
	qdf_mem_copy(&pParam->bssid, pBssDesc->bssId, sizeof(struct qdf_mac_addr));

	if (!pIes) {
		pParam->ssId.length = 0;
		pParam->operationalRateSet.numRates = 0;
		sms_log(pMac, LOGE, FL("IEs struct pointer is NULL"));
		return;
	}

	if (pIes->SuppRates.present) {
		pParam->operationalRateSet.numRates = pIes->SuppRates.num_rates;
		if (pIes->SuppRates.num_rates > SIR_MAC_RATESET_EID_MAX) {
			sms_log(pMac, LOGE,
				FL("num_rates: %d > max val, resetting"),
				pIes->SuppRates.num_rates);
			pIes->SuppRates.num_rates = SIR_MAC_RATESET_EID_MAX;
		}
		qdf_mem_copy(pParam->operationalRateSet.rate,
			     pIes->SuppRates.rates,
			     sizeof(*pIes->SuppRates.rates) *
			     pIes->SuppRates.num_rates);
	}
	if (pIes->ExtSuppRates.present) {
		pParam->extendedRateSet.numRates = pIes->ExtSuppRates.num_rates;
		if (pIes->ExtSuppRates.num_rates > SIR_MAC_RATESET_EID_MAX) {
			sms_log(pMac, LOGE,
				FL("num_rates: %d > max val, resetting"),
				pIes->ExtSuppRates.num_rates);
			pIes->ExtSuppRates.num_rates = SIR_MAC_RATESET_EID_MAX;
		}
		qdf_mem_copy(pParam->extendedRateSet.rate,
			     pIes->ExtSuppRates.rates,
			     sizeof(*pIes->ExtSuppRates.rates) *
			     pIes->ExtSuppRates.num_rates);
	}
	if (pIes->SSID.present) {
		pParam->ssId.length = pIes->SSID.num_ssid;
		qdf_mem_copy(pParam->ssId.ssId, pIes->SSID.ssid,
			     pParam->ssId.length);
	}
	pParam->cbMode = csr_get_cb_mode_from_ies(pMac, pParam->operationChn,
						  pIes);
}

static void csr_roam_determine_max_rate_for_ad_hoc(tpAniSirGlobal pMac,
						   tSirMacRateSet *pSirRateSet)
{
	uint8_t MaxRate = 0;
	uint32_t i;
	uint8_t *pRate;

	pRate = pSirRateSet->rate;
	for (i = 0; i < pSirRateSet->numRates; i++) {
		MaxRate =
			CSR_MAX(MaxRate, (pRate[i] & (~CSR_DOT11_BASIC_RATE_MASK)));
	}

	/* Save the max rate in the connected state information... */

	/* modify LastRates variable as well */

	return;
}

QDF_STATUS csr_roam_issue_start_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				    tCsrRoamStartBssParams *pParam,
				    tCsrRoamProfile *pProfile,
				    tSirBssDescription *pBssDesc, uint32_t roamId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	eCsrBand eBand;
	/* Set the roaming substate to 'Start BSS attempt'... */
	csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_START_BSS_REQ,
				 sessionId);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	/* Need to figure out whether we need to log WDS??? */
	if (CSR_IS_IBSS(pProfile)) {
		host_log_ibss_pkt_type *pIbssLog;
		WLAN_HOST_DIAG_LOG_ALLOC(pIbssLog, host_log_ibss_pkt_type,
					 LOG_WLAN_IBSS_C);
		if (pIbssLog) {
			if (pBssDesc) {
				pIbssLog->eventId =
					WLAN_IBSS_EVENT_JOIN_IBSS_REQ;
				qdf_mem_copy(pIbssLog->bssid.bytes,
					pBssDesc->bssId, QDF_MAC_ADDR_SIZE);
			} else {
				pIbssLog->eventId =
					WLAN_IBSS_EVENT_START_IBSS_REQ;
			}
			qdf_mem_copy(pIbssLog->ssid, pParam->ssId.ssId,
				     pParam->ssId.length);
			if (pProfile->ChannelInfo.numOfChannels == 0) {
				pIbssLog->channelSetting = AUTO_PICK;
			} else {
				pIbssLog->channelSetting = SPECIFIED;
			}
			pIbssLog->operatingChannel = pParam->operationChn;
			WLAN_HOST_DIAG_LOG_REPORT(pIbssLog);
		}
	}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	/* Put RSN information in for Starting BSS */
	pParam->nRSNIELength = (uint16_t) pProfile->nRSNReqIELength;
	pParam->pRSNIE = pProfile->pRSNReqIE;

	pParam->privacy = pProfile->privacy;
	pParam->fwdWPSPBCProbeReq = pProfile->fwdWPSPBCProbeReq;
	pParam->authType = pProfile->csr80211AuthType;
	pParam->beaconInterval = pProfile->beaconInterval;
	pParam->dtimPeriod = pProfile->dtimPeriod;
	pParam->ApUapsdEnable = pProfile->ApUapsdEnable;
	pParam->ssidHidden = pProfile->SSIDs.SSIDList[0].ssidHidden;
	if (CSR_IS_INFRA_AP(pProfile) && (pParam->operationChn != 0)) {
		if (csr_is_valid_channel(pMac, pParam->operationChn) !=
		    QDF_STATUS_SUCCESS) {
			pParam->operationChn = INFRA_AP_DEFAULT_CHANNEL;
		}
	}
	pParam->protEnabled = pProfile->protEnabled;
	pParam->obssProtEnabled = pProfile->obssProtEnabled;
	pParam->ht_protection = pProfile->cfg_protection;
	pParam->wps_state = pProfile->wps_state;

	pParam->uCfgDot11Mode =
		csr_roam_get_phy_mode_band_for_bss(pMac, pProfile,
						   pParam->
						   operationChn,
						   &eBand);
	pParam->bssPersona = pProfile->csrPersona;

#ifdef WLAN_FEATURE_11W
	pParam->mfpCapable = (0 != pProfile->MFPCapable);
	pParam->mfpRequired = (0 != pProfile->MFPRequired);
#endif

	pParam->addIeParams.probeRespDataLen =
		pProfile->addIeParams.probeRespDataLen;
	pParam->addIeParams.probeRespData_buff =
		pProfile->addIeParams.probeRespData_buff;

	pParam->addIeParams.assocRespDataLen =
		pProfile->addIeParams.assocRespDataLen;
	pParam->addIeParams.assocRespData_buff =
		pProfile->addIeParams.assocRespData_buff;

	if (CSR_IS_IBSS(pProfile)) {
		pParam->addIeParams.probeRespBCNDataLen =
			pProfile->nWPAReqIELength;
		pParam->addIeParams.probeRespBCNData_buff = pProfile->pWPAReqIE;
	} else {
		pParam->addIeParams.probeRespBCNDataLen =
			pProfile->addIeParams.probeRespBCNDataLen;
		pParam->addIeParams.probeRespBCNData_buff =
			pProfile->addIeParams.probeRespBCNData_buff;
	}
	pParam->sap_dot11mc = pProfile->sap_dot11mc;

	/* When starting an IBSS, start on the channel from the Profile. */
	status =
		csr_send_mb_start_bss_req_msg(pMac, sessionId, pProfile->BSSType, pParam,
					      pBssDesc);
	return status;
}

void csr_roam_prepare_bss_params(tpAniSirGlobal pMac, uint32_t sessionId,
					tCsrRoamProfile *pProfile,
					tSirBssDescription *pBssDesc,
					tBssConfigParam *pBssConfig,
					tDot11fBeaconIEs *pIes)
{
	uint8_t Channel;
	ePhyChanBondState cbMode = PHY_SINGLE_CHANNEL_CENTERED;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	if (pBssDesc) {
		csr_roam_get_bss_start_parms_from_bss_desc(pMac, pBssDesc, pIes,
							&pSession->bssParams);
		if (CSR_IS_NDI(pProfile)) {
			qdf_copy_macaddr(&pSession->bssParams.bssid,
				&pSession->selfMacAddr);
		}
	} else {
		csr_roam_get_bss_start_parms(pMac, pProfile, &pSession->bssParams);
		/* Use the first SSID */
		if (pProfile->SSIDs.numOfSSIDs)
			qdf_mem_copy(&pSession->bssParams.ssId,
				     pProfile->SSIDs.SSIDList,
				     sizeof(tSirMacSSid));
		if (pProfile->BSSIDs.numOfBSSIDs)
			/* Use the first BSSID */
			qdf_mem_copy(&pSession->bssParams.bssid,
				     pProfile->BSSIDs.bssid,
				     sizeof(struct qdf_mac_addr));
		else
			qdf_mem_set(&pSession->bssParams.bssid,
				    sizeof(struct qdf_mac_addr), 0);
	}
	Channel = pSession->bssParams.operationChn;
	/* Set operating channel in pProfile which will be used */
	/* in csr_roam_set_bss_config_cfg() to determine channel bonding */
	/* mode and will be configured in CFG later */
	pProfile->operationChannel = Channel;

	if (Channel == 0) {
		sms_log(pMac, LOGE,
			"   CSR cannot find a channel to start IBSS");
	} else {

		csr_roam_determine_max_rate_for_ad_hoc(pMac,
						       &pSession->bssParams.
						       operationalRateSet);
		if (CSR_IS_INFRA_AP(pProfile) || CSR_IS_START_IBSS(pProfile)) {
			if (CDS_IS_CHANNEL_24GHZ(Channel)) {
				cbMode =
					pMac->roam.configParam.
					channelBondingMode24GHz;
			} else {
				cbMode =
					pMac->roam.configParam.
					channelBondingMode5GHz;
			}
			sms_log(pMac, LOG1, "## cbMode %d", cbMode);
			pBssConfig->cbMode = cbMode;
			pSession->bssParams.cbMode = cbMode;
		}
	}
}

static QDF_STATUS csr_roam_start_ibss(tpAniSirGlobal pMac, uint32_t sessionId,
				      tCsrRoamProfile *pProfile,
				      bool *pfSameIbss)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool fSameIbss = false;

	if (csr_is_conn_state_ibss(pMac, sessionId)) {
		/* Check if any profile parameter has changed ? If any profile parameter */
		/* has changed then stop old BSS and start a new one with new parameters */
		if (csr_is_same_profile
			    (pMac, &pMac->roam.roamSession[sessionId].connectedProfile,
			    pProfile)) {
			fSameIbss = true;
		} else {
			status =
				csr_roam_issue_stop_bss(pMac, sessionId,
							eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING);
		}
	} else if (csr_is_conn_state_connected_infra(pMac, sessionId)) {
		/* Disassociate from the connected Infrastructure network... */
		status =
			csr_roam_issue_disassociate(pMac, sessionId,
						    eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING,
						    false);
	} else {
		tBssConfigParam *pBssConfig;

		pBssConfig = qdf_mem_malloc(sizeof(tBssConfigParam));
		if (NULL == pBssConfig)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_set(pBssConfig, sizeof(tBssConfigParam), 0);
			/* there is no Bss description before we start an IBSS so we need to adopt */
			/* all Bss configuration parameters from the Profile. */
			status =
				csr_roam_prepare_bss_config_from_profile(pMac, pProfile,
									 pBssConfig,
									 NULL);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				/* save dotMode */
				pMac->roam.roamSession[sessionId].bssParams.
				uCfgDot11Mode = pBssConfig->uCfgDot11Mode;
				/* Prepare some more parameters for this IBSS */
				csr_roam_prepare_bss_params(pMac, sessionId,
							    pProfile, NULL,
							    pBssConfig, NULL);
				status =
					csr_roam_set_bss_config_cfg(pMac, sessionId,
								    pProfile, NULL,
								    pBssConfig, NULL,
								    false);
			}

			qdf_mem_free(pBssConfig);
		} /* Allocate memory */
	}

	if (pfSameIbss) {
		*pfSameIbss = fSameIbss;
	}
	return status;
}

static void csr_roam_update_connected_profile_from_new_bss(tpAniSirGlobal pMac,
							   uint32_t sessionId,
							   tSirSmeNewBssInfo *pNewBss)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	if (pNewBss) {
		/* Set the operating channel. */
		pSession->connectedProfile.operationChannel =
			pNewBss->channelNumber;
		/* move the BSSId from the BSS description into the connected state information. */
		qdf_mem_copy(&pSession->connectedProfile.bssid.bytes,
			     &(pNewBss->bssId), sizeof(struct qdf_mac_addr));
	}
	return;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS csr_roam_set_psk_pmk(tpAniSirGlobal pMac, uint32_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(pSession->psk_pmk, pPSK_PMK, sizeof(pSession->psk_pmk));
	pSession->pmk_len = pmk_len;
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void
csr_roam_diag_set_pmkid(tCsrRoamSession *pSession)
{
	WLAN_HOST_DIAG_EVENT_DEF(secEvent,
				 host_event_wlan_security_payload_type);
	qdf_mem_set(&secEvent,
	    sizeof(host_event_wlan_security_payload_type), 0);
	secEvent.eventId = WLAN_SECURITY_EVENT_PMKID_UPDATE;
	secEvent.encryptionModeMulticast =
		(uint8_t) diag_enc_type_from_csr_type(
			pSession->connectedProfile.mcEncryptionType);
	secEvent.encryptionModeUnicast =
		(uint8_t) diag_enc_type_from_csr_type(
			pSession->connectedProfile.EncryptionType);
	qdf_mem_copy(secEvent.bssid,
		     pSession->connectedProfile.bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	secEvent.authMode = (uint8_t) diag_auth_type_from_csr_type(
				pSession->connectedProfile.AuthType);
	WLAN_HOST_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

QDF_STATUS
csr_roam_set_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId,
			 tPmkidCacheInfo *pPMKIDCache, uint32_t numItems,
			 bool update_entire_cache)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	uint32_t i = 0;
	tPmkidCacheInfo *pmksa;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sms_log(pMac, LOGW, FL("numItems = %d"), numItems);

	if (numItems > CSR_MAX_PMKID_ALLOWED)
		return QDF_STATUS_E_INVAL;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_roam_diag_set_pmkid(pSession);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

	if (update_entire_cache) {
		if (numItems && pPMKIDCache) {
			pSession->NumPmkidCache = (uint16_t) numItems;
			qdf_mem_copy(pSession->PmkidCacheInfo, pPMKIDCache,
				sizeof(tPmkidCacheInfo) * numItems);
			pSession->curr_cache_idx = (uint16_t)numItems;
		}
		return QDF_STATUS_SUCCESS;
	}

	for (i = 0; i < numItems; i++) {
		pmksa = &pPMKIDCache[i];

		/* Delete the entry if present */
		csr_roam_del_pmkid_from_cache(pMac, sessionId,
				pmksa->BSSID.bytes, false);

		/* Add entry to the cache */
		qdf_copy_macaddr(
		    &pSession->PmkidCacheInfo[pSession->curr_cache_idx].BSSID,
		    &pmksa->BSSID);
		qdf_mem_copy(
		    pSession->PmkidCacheInfo[pSession->curr_cache_idx].PMKID,
		    pmksa->PMKID, CSR_RSN_PMKID_SIZE);

		/* Increment the CSR local cache index */
		if (pSession->curr_cache_idx < (CSR_MAX_PMKID_ALLOWED - 1))
			pSession->curr_cache_idx++;
		else
			pSession->curr_cache_idx = 0;

		pSession->NumPmkidCache++;
		if (pSession->NumPmkidCache > CSR_MAX_PMKID_ALLOWED)
			pSession->NumPmkidCache = CSR_MAX_PMKID_ALLOWED;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_del_pmkid_from_cache(tpAniSirGlobal pMac,
					 uint32_t sessionId,
					 const uint8_t *pBSSId,
					 bool flush_cache)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	bool fMatchFound = false;
	uint32_t Index;
	uint32_t curr_idx;
	uint32_t i;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if there are no entries to delete */
	if (0 == pSession->NumPmkidCache) {
		sms_log(pMac, LOG1, FL("No entries to delete/Flush"));
		return QDF_STATUS_SUCCESS;
	}

	if (flush_cache) {
		/* Flush the entire cache */
		qdf_mem_zero(pSession->PmkidCacheInfo,
			     sizeof(tPmkidCacheInfo) * CSR_MAX_PMKID_ALLOWED);
		pSession->NumPmkidCache = 0;
		pSession->curr_cache_idx = 0;
		return QDF_STATUS_SUCCESS;
	}

	/* !flush_cache - so look up in the cache */
	for (Index = 0; Index < CSR_MAX_PMKID_ALLOWED; Index++) {
		if (!qdf_mem_cmp(pSession->PmkidCacheInfo[Index].BSSID.bytes,
				    pBSSId, QDF_MAC_ADDR_SIZE)) {
			fMatchFound = 1;

			/* Clear this - the matched entry */
			qdf_mem_zero(&pSession->PmkidCacheInfo[Index],
				     sizeof(tPmkidCacheInfo));
			break;
		}
	}

	if (Index == CSR_MAX_PMKID_ALLOWED && !fMatchFound) {
		sms_log(pMac, LOG1, FL("No such PMKSA entry exists"
			MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pBSSId));
		return QDF_STATUS_SUCCESS;
	}

	/* Match Found, Readjust the other entries */
	curr_idx = pSession->curr_cache_idx;
	if (Index < curr_idx) {
		for (i = Index; i < (curr_idx - 1); i++) {
			qdf_mem_copy(&pSession->PmkidCacheInfo[i],
				     &pSession->PmkidCacheInfo[i + 1],
				     sizeof(tPmkidCacheInfo));
		}

		pSession->curr_cache_idx--;
		qdf_mem_zero(&pSession->PmkidCacheInfo
			     [pSession->curr_cache_idx],
			     sizeof(tPmkidCacheInfo));
	} else if (Index > curr_idx) {
		for (i = Index; i > (curr_idx); i--) {
			qdf_mem_copy(&pSession->PmkidCacheInfo[i],
				     &pSession->PmkidCacheInfo[i - 1],
				     sizeof(tPmkidCacheInfo));
		}

		qdf_mem_zero(&pSession->PmkidCacheInfo
			     [pSession->curr_cache_idx],
			     sizeof(tPmkidCacheInfo));
	}

	/* Decrement the count since an entry has been deleted */
	pSession->NumPmkidCache--;
	return QDF_STATUS_SUCCESS;
}

uint32_t csr_roam_get_num_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId)
{
	return pMac->roam.roamSession[sessionId].NumPmkidCache;
}

QDF_STATUS csr_roam_get_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId,
				    uint32_t *pNum, tPmkidCacheInfo *pPmkidCache)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tPmkidCacheInfo *pmksa;
	uint16_t i, j;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pNum || !pPmkidCache) {
		sms_log(pMac, LOGE, FL("Either pNum or pPmkidCache is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->NumPmkidCache == 0) {
		*pNum = 0;
		return QDF_STATUS_SUCCESS;
	}

	if (*pNum < pSession->NumPmkidCache) {
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->NumPmkidCache > CSR_MAX_PMKID_ALLOWED) {
		sms_log(pMac, LOGE,
			FL(
			"NumPmkidCache :%d is more than CSR_MAX_PMKID_ALLOWED, resetting to CSR_MAX_PMKID_ALLOWED"),
			pSession->NumPmkidCache);
		pSession->NumPmkidCache = CSR_MAX_PMKID_ALLOWED;
	}

	for (i = 0, j = 0; ((j < pSession->NumPmkidCache) &&
		(i < CSR_MAX_PMKID_ALLOWED)); i++) {
		/* Fill the valid entries */
		pmksa = &pSession->PmkidCacheInfo[i];
		if (!qdf_is_macaddr_zero(&pmksa->BSSID)) {
			qdf_mem_copy(pPmkidCache, pmksa,
				     sizeof(tPmkidCacheInfo));
			pPmkidCache++;
			j++;
		}
	}

	*pNum = pSession->NumPmkidCache;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_get_wpa_rsn_req_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t len;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pLen) {
		len = *pLen;
		*pLen = pSession->nWpaRsnReqIeLength;
		if (pBuf) {
			if (len >= pSession->nWpaRsnReqIeLength) {
				qdf_mem_copy(pBuf, pSession->pWpaRsnReqIE,
					     pSession->nWpaRsnReqIeLength);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	return status;
}

QDF_STATUS csr_roam_get_wpa_rsn_rsp_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t len;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pLen) {
		len = *pLen;
		*pLen = pSession->nWpaRsnRspIeLength;
		if (pBuf) {
			if (len >= pSession->nWpaRsnRspIeLength) {
				qdf_mem_copy(pBuf, pSession->pWpaRsnRspIE,
					     pSession->nWpaRsnRspIeLength);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	return status;
}

#ifdef FEATURE_WLAN_WAPI
QDF_STATUS csr_roam_get_wapi_req_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				    uint32_t *pLen, uint8_t *pBuf)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t len;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pLen) {
		len = *pLen;
		*pLen = pSession->nWapiReqIeLength;
		if (pBuf) {
			if (len >= pSession->nWapiReqIeLength) {
				qdf_mem_copy(pBuf, pSession->pWapiReqIE,
					     pSession->nWapiReqIeLength);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	return status;
}

QDF_STATUS csr_roam_get_wapi_rsp_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				    uint32_t *pLen, uint8_t *pBuf)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t len;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pLen) {
		len = *pLen;
		*pLen = pSession->nWapiRspIeLength;
		if (pBuf) {
			if (len >= pSession->nWapiRspIeLength) {
				qdf_mem_copy(pBuf, pSession->pWapiRspIE,
					     pSession->nWapiRspIeLength);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	return status;
}
#endif /* FEATURE_WLAN_WAPI */
eRoamCmdStatus csr_get_roam_complete_status(tpAniSirGlobal pMac, uint32_t sessionId)
{
	eRoamCmdStatus retStatus = eCSR_ROAM_CONNECT_COMPLETION;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return retStatus;
	}

	if (CSR_IS_ROAMING(pSession)) {
		retStatus = eCSR_ROAM_ROAMING_COMPLETION;
		pSession->fRoaming = false;
	}
	return retStatus;
}

/* This function remove the connected BSS from te cached scan result */
QDF_STATUS
csr_roam_remove_connected_bss_from_scan_cache(tpAniSirGlobal pMac,
					tCsrRoamConnectedProfile *pConnProfile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrScanResultFilter *pScanFilter = NULL;
	tListElem *pEntry;
	tCsrScanResult *pResult;
	tDot11fBeaconIEs *pIes;
	bool fMatch;

	if ((qdf_is_macaddr_zero(&pConnProfile->bssid) ||
	     qdf_is_macaddr_broadcast(&pConnProfile->bssid)))
		return status;
	/*
	 * Prepare the filter. Only fill in the necessary fields. Not all fields
	 * are needed
	 */
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(pScanFilter, sizeof(tCsrScanResultFilter), 0);
	pScanFilter->BSSIDs.bssid = qdf_mem_malloc(sizeof(struct qdf_mac_addr));
	if (NULL == pScanFilter->BSSIDs.bssid) {
		qdf_mem_free(pScanFilter);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(pScanFilter->BSSIDs.bssid,
		     &pConnProfile->bssid, sizeof(struct qdf_mac_addr));
	pScanFilter->BSSIDs.numOfBSSIDs = 1;
	if (!csr_is_nullssid(pConnProfile->SSID.ssId,
			pConnProfile->SSID.length)) {
		pScanFilter->SSIDs.SSIDList = qdf_mem_malloc(
							sizeof(tCsrSSIDInfo));
		if (NULL == pScanFilter->SSIDs.SSIDList) {
			csr_free_scan_filter(pMac, pScanFilter);
			qdf_mem_free(pScanFilter);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(&pScanFilter->SSIDs.SSIDList[0].SSID,
			&pConnProfile->SSID, sizeof(tSirMacSSid));
	}
	pScanFilter->authType.numEntries = 1;
	pScanFilter->authType.authType[0] = pConnProfile->AuthType;
	pScanFilter->BSSType = pConnProfile->BSSType;
	pScanFilter->EncryptionType.numEntries = 1;
	pScanFilter->EncryptionType.encryptionType[0] =
		pConnProfile->EncryptionType;
	pScanFilter->mcEncryptionType.numEntries = 1;
	pScanFilter->mcEncryptionType.encryptionType[0] =
		pConnProfile->mcEncryptionType;
	/* We ignore the channel for now, BSSID should be enough */
	pScanFilter->ChannelInfo.numOfChannels = 0;
	/* Also ignore the following fields */
	pScanFilter->uapsd_mask = 0;
	pScanFilter->bWPSAssociation = false;
	pScanFilter->bOSENAssociation = false;
	pScanFilter->countryCode[0] = 0;
	pScanFilter->phyMode = eCSR_DOT11_MODE_AUTO;
#ifdef WLAN_FEATURE_11W
	pScanFilter->MFPEnabled = pConnProfile->MFPEnabled;
	pScanFilter->MFPRequired = pConnProfile->MFPRequired;
	pScanFilter->MFPCapable = pConnProfile->MFPCapable;
#endif
	csr_ll_lock(&pMac->scan.scanResultList);
	pEntry = csr_ll_peek_head(&pMac->scan.scanResultList, LL_ACCESS_NOLOCK);
	while (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
		pIes = (tDot11fBeaconIEs *) (pResult->Result.pvIes);
		fMatch = csr_match_bss(pMac, &pResult->Result.BssDescriptor,
				       pScanFilter, NULL, NULL, NULL, &pIes);
		/* Release the IEs allocated by csr_match_bss is needed */
		if (!pResult->Result.pvIes) {
			/*
			 * need to free the IEs since it is allocated
			 * by csr_match_bss
			 */
			qdf_mem_free(pIes);
		}
		if (fMatch) {
			/* We found the one */
			if (csr_ll_remove_entry(&pMac->scan.scanResultList,
						pEntry, LL_ACCESS_NOLOCK))
				/* Free the memory */
				csr_free_scan_result_entry(pMac, pResult);
			break;
		}
		pEntry = csr_ll_next(&pMac->scan.scanResultList,
				    pEntry, LL_ACCESS_NOLOCK);
	} /* while */
	csr_ll_unlock(&pMac->scan.scanResultList);
	csr_free_scan_filter(pMac, pScanFilter);
	qdf_mem_free(pScanFilter);
	return status;
}

static QDF_STATUS csr_roam_start_wds(tpAniSirGlobal pMac, uint32_t sessionId,
				     tCsrRoamProfile *pProfile,
				     tSirBssDescription *pBssDesc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tBssConfigParam bssConfig;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (csr_is_conn_state_ibss(pMac, sessionId)) {
		status =
			csr_roam_issue_stop_bss(pMac, sessionId,
						eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING);
	} else if (csr_is_conn_state_connected_infra(pMac, sessionId)) {
		/* Disassociate from the connected Infrastructure network... */
		status =
			csr_roam_issue_disassociate(pMac, sessionId,
						    eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING,
						    false);
	} else {
		/* We don't expect Bt-AMP HDD not to disconnect the last connection first at this time. */
		/* Otherwise we need to add code to handle the */
		/* situation just like IBSS. Though for WDS station, we need to send disassoc to PE first then */
		/* send stop_bss to PE, before we can continue. */

		if (csr_is_conn_state_wds(pMac, sessionId)) {
			QDF_ASSERT(0);
			return QDF_STATUS_E_FAILURE;
		}
		qdf_mem_set(&bssConfig, sizeof(tBssConfigParam), 0);
		/* Assume HDD provide bssid in profile */
		qdf_copy_macaddr(&pSession->bssParams.bssid,
				 pProfile->BSSIDs.bssid);
		/* there is no Bss description before we start an WDS so we need */
		/* to adopt all Bss configuration parameters from the Profile. */
		status =
			csr_roam_prepare_bss_config_from_profile(pMac, pProfile,
								 &bssConfig, pBssDesc);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* Save profile for late use */
			csr_free_roam_profile(pMac, sessionId);
			pSession->pCurRoamProfile =
				qdf_mem_malloc(sizeof(tCsrRoamProfile));
			if (pSession->pCurRoamProfile != NULL) {
				qdf_mem_set(pSession->pCurRoamProfile,
					    sizeof(tCsrRoamProfile), 0);
				csr_roam_copy_profile(pMac,
						      pSession->pCurRoamProfile,
						      pProfile);
			}
			/* Prepare some more parameters for this WDS */
			csr_roam_prepare_bss_params(pMac, sessionId, pProfile, NULL,
						    &bssConfig, NULL);
			status =
				csr_roam_set_bss_config_cfg(pMac, sessionId, pProfile,
							    NULL, &bssConfig, NULL,
							    false);
		}
	}

	return status;
}

/**
 * csr_add_supported_5Ghz_channels()- Add valid 5Ghz channels
 * in Join req.
 * @mac_ctx: pointer to global mac structure
 * @chan_list: Pointer to channel list buffer to populate
 * @num_chan: Pointer to number of channels value to update
 * @supp_chan_ie: Boolean to check if we need to populate as IE
 *
 * This function is called to update valid 5Ghz channels
 * in Join req. If @supp_chan_ie is true, supported channels IE
 * format[chan num 1, num of channels 1, chan num 2, num of
 * channels 2, ..] is populated. Else, @chan_list would be a list
 * of supported channels[chan num 1, chan num 2..]
 *
 * Return: void
 */
static void csr_add_supported_5Ghz_channels(tpAniSirGlobal mac_ctx,
						uint8_t *chan_list,
						uint8_t *num_chnl,
						bool supp_chan_ie)
{
	uint16_t i, j;
	uint32_t size = 0;

	if (!chan_list) {
		sms_log(mac_ctx, LOGE, FL("chan_list buffer NULL"));
		return;
	}

	size = sizeof(mac_ctx->roam.validChannelList);
	if (QDF_IS_STATUS_SUCCESS
		(csr_get_cfg_valid_channels(mac_ctx,
		(uint8_t *) mac_ctx->roam.validChannelList,
				&size))) {
		for (i = 0, j = 0; i < size; i++) {
			/* Only add 5ghz channels.*/
			if (CDS_IS_CHANNEL_5GHZ
					(mac_ctx->roam.validChannelList[i])) {
				chan_list[j]
					= mac_ctx->roam.validChannelList[i];
				j++;

				if (supp_chan_ie) {
					chan_list[j] = 1;
					j++;
				}
			}
		}
		*num_chnl = (uint8_t)j;
	} else {
		sms_log(mac_ctx, LOGE,
			FL("can not find any valid channel"));
		*num_chnl = 0;
	}
}

/**
 * csr_set_ldpc_exception() - to set allow any LDPC exception permitted
 * @mac_ctx: Pointer to mac context
 * @session: Pointer to SME/CSR session
 * @channel: Given channel number where connection will go
 * @usr_cfg_rx_ldpc: User provided RX LDPC setting
 *
 * This API will check if hardware allows LDPC to be enabled for provided
 * channel and user has enabled the RX LDPC selection
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS csr_set_ldpc_exception(tpAniSirGlobal mac_ctx,
			tCsrRoamSession *session, uint8_t channel,
			bool usr_cfg_rx_ldpc)
{
	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"mac_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (!session) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"session is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (usr_cfg_rx_ldpc && wma_is_rx_ldpc_supported_for_channel(channel)) {
		session->htConfig.ht_rx_ldpc = 1;
		session->vht_config.ldpc_coding = 1;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_HIGH,
			"LDPC enable for chnl[%d]", channel);
	} else {
		session->htConfig.ht_rx_ldpc = 0;
		session->vht_config.ldpc_coding = 0;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_HIGH,
			"LDPC disable for chnl[%d]", channel);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * The communication between HDD and LIM is thru mailbox (MB).
 * Both sides will access the data structure "tSirSmeJoinReq".
 * The rule is, while the components of "tSirSmeJoinReq" can be accessed in the
 * regular way like tSirSmeJoinReq.assocType, this guideline stops at component
 * tSirRSNie;
 * any acces to the components after tSirRSNie is forbidden because the space
 * from tSirRSNie is squeezed with the component "tSirBssDescription" and since
 * the size of actual 'tSirBssDescription' varies, the receiving side should
 * keep in mind not to access the components DIRECTLY after tSirRSNie.
 */
QDF_STATUS csr_send_join_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
				 tSirBssDescription *pBssDescription,
				 tCsrRoamProfile *pProfile,
				 tDot11fBeaconIEs *pIes, uint16_t messageType)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t acm_mask = 0, uapsd_mask;
	uint16_t msgLen, ieLen;
	tSirMacRateSet OpRateSet;
	tSirMacRateSet ExRateSet;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	uint32_t dwTmp, ucDot11Mode = 0;
	/* RSN MAX is bigger than WPA MAX */
	uint8_t wpaRsnIE[DOT11F_IE_RSN_MAX_LEN];
	uint8_t txBFCsnValue = 0;
	tSirSmeJoinReq *csr_join_req;
	tSirMacCapabilityInfo *pAP_capabilityInfo;
	tAniBool fTmp;
	int8_t pwrLimit = 0;
	struct ps_global_info *ps_global_info = &pMac->sme.ps_global_info;
	struct ps_params *ps_param = &ps_global_info->ps_params[sessionId];
	uint8_t ese_config = 0;
	tpCsrNeighborRoamControlInfo neigh_roam_info;
	uint32_t value = 0, value1 = 0;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	/* To satisfy klockworks */
	if (NULL == pBssDescription) {
		sms_log(pMac, LOGE, FL(" pBssDescription is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	neigh_roam_info = &pMac->roam.neighborRoamInfo[sessionId];
	if ((eWNI_SME_REASSOC_REQ == messageType) ||
		CDS_IS_CHANNEL_5GHZ(pBssDescription->channelId) ||
		(abs(pBssDescription->rssi) <
		 (neigh_roam_info->cfgParams.neighborLookupThreshold -
		  neigh_roam_info->cfgParams.hi_rssi_scan_rssi_delta))) {
		pSession->disable_hi_rssi = true;
		sms_log(pMac, LOG1,
			FL("Disabling HI_RSSI feature, AP channel=%d, rssi=%d"),
			pBssDescription->channelId, pBssDescription->rssi);
	} else {
		pSession->disable_hi_rssi = false;
	}


	do {
		pSession->joinFailStatusCode.statusCode = eSIR_SME_SUCCESS;
		pSession->joinFailStatusCode.reasonCode = 0;
		qdf_mem_copy(&pSession->joinFailStatusCode.bssId,
		       &pBssDescription->bssId, sizeof(tSirMacAddr));
		/*
		 * the tSirSmeJoinReq which includes a single
		 * bssDescription. it includes a single uint32_t for the
		 * IE fields, but the length field in the bssDescription
		 * needs to be interpreted to determine length of IE fields
		 * So, take the size of the tSirSmeJoinReq, subtract  size of
		 * bssDescription, add the number of bytes indicated by the
		 * length field of the bssDescription, add the size of length
		 * field  because it not included in the lenghth field.
		 */
		msgLen = sizeof(tSirSmeJoinReq) - sizeof(*pBssDescription) +
				pBssDescription->length +
				sizeof(pBssDescription->length) +
				/*
				 * add in the size of the WPA IE that
				 * we may build.
				 */
				sizeof(tCsrWpaIe) + sizeof(tCsrWpaAuthIe) +
				sizeof(uint16_t);
		csr_join_req = qdf_mem_malloc(msgLen);
		if (NULL == csr_join_req)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		qdf_mem_set(csr_join_req, msgLen, 0);
		csr_join_req->messageType = messageType;
		csr_join_req->length = msgLen;
		csr_join_req->sessionId = (uint8_t) sessionId;
		csr_join_req->transactionId = 0;
		if (pIes->SSID.present && pIes->SSID.num_ssid) {
			csr_join_req->ssId.length = pIes->SSID.num_ssid;
			qdf_mem_copy(&csr_join_req->ssId.ssId, pIes->SSID.ssid,
				     pIes->SSID.num_ssid);
		} else
			csr_join_req->ssId.length = 0;
		qdf_mem_copy(&csr_join_req->selfMacAddr, &pSession->selfMacAddr,
			     sizeof(tSirMacAddr));
		sms_log(pMac, LOGE,
			FL("Connecting to ssid:%.*s bssid: "
			MAC_ADDRESS_STR" rssi: %d channel: %d country_code: %c%c"),
			pIes->SSID.num_ssid, pIes->SSID.ssid,
			MAC_ADDR_ARRAY(pBssDescription->bssId),
			pBssDescription->rssi, pBssDescription->channelId,
			pMac->scan.countryCodeCurrent[0],
			pMac->scan.countryCodeCurrent[1]);
		/* bsstype */
		dwTmp = csr_translate_bsstype_to_mac_type
						(pProfile->BSSType);
		csr_join_req->bsstype = dwTmp;
		/* dot11mode */
		ucDot11Mode =
			csr_translate_to_wni_cfg_dot11_mode(pMac,
							    pSession->bssParams.
							    uCfgDot11Mode);
		if (pBssDescription->channelId <= 14
		    && false == pMac->roam.configParam.enableVhtFor24GHz
		    && WNI_CFG_DOT11_MODE_11AC == ucDot11Mode) {
			/* Need to disable VHT operation in 2.4 GHz band */
			ucDot11Mode = WNI_CFG_DOT11_MODE_11N;
		}
		csr_join_req->dot11mode = (uint8_t) ucDot11Mode;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		csr_join_req->cc_switch_mode =
			pMac->roam.configParam.cc_switch_mode;
#endif
		csr_join_req->staPersona = (uint8_t) pProfile->csrPersona;
		csr_join_req->wps_registration = pProfile->bWPSAssociation;
		csr_join_req->cbMode = (uint8_t) pSession->bssParams.cbMode;
		sms_log(pMac, LOG2,
			  FL("CSR PERSONA=%d CSR CbMode %d"),
			  pProfile->csrPersona, pSession->bssParams.cbMode);
		csr_join_req->uapsdPerAcBitmask = pProfile->uapsd_mask;
		status =
			csr_get_rate_set(pMac, pProfile,
					 (eCsrPhyMode) pProfile->phyMode,
					 pBssDescription, pIes, &OpRateSet,
					 &ExRateSet);
		ps_param->uapsd_per_ac_bit_mask =
			pProfile->uapsd_mask;
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* OperationalRateSet */
			if (OpRateSet.numRates) {
				csr_join_req->operationalRateSet.numRates =
					OpRateSet.numRates;
				qdf_mem_copy(&csr_join_req->operationalRateSet.
						rate, OpRateSet.rate,
						OpRateSet.numRates);
			} else
				csr_join_req->operationalRateSet.numRates = 0;

			/* ExtendedRateSet */
			if (ExRateSet.numRates) {
				csr_join_req->extendedRateSet.numRates =
					ExRateSet.numRates;
				qdf_mem_copy(&csr_join_req->extendedRateSet.
						rate, ExRateSet.rate,
						ExRateSet.numRates);
			} else
				csr_join_req->extendedRateSet.numRates = 0;
		} else {
			csr_join_req->operationalRateSet.numRates = 0;
			csr_join_req->extendedRateSet.numRates = 0;
		}
		/* rsnIE */
		if (csr_is_profile_wpa(pProfile)) {
			/* Insert the Wpa IE into the join request */
			ieLen =
				csr_retrieve_wpa_ie(pMac, pProfile,
						pBssDescription, pIes,
						(tCsrWpaIe *) (wpaRsnIE));
		} else if (csr_is_profile_rsn(pProfile)) {
			/* Insert the RSN IE into the join request */
			ieLen =
				csr_retrieve_rsn_ie(pMac, sessionId, pProfile,
						    pBssDescription, pIes,
						    (tCsrRSNIe *) (wpaRsnIE));
		}
#ifdef FEATURE_WLAN_WAPI
		else if (csr_is_profile_wapi(pProfile)) {
			/* Insert the WAPI IE into the join request */
			ieLen =
				csr_retrieve_wapi_ie(pMac, sessionId, pProfile,
						     pBssDescription, pIes,
						     (tCsrWapiIe *) (wpaRsnIE));
		}
#endif /* FEATURE_WLAN_WAPI */
		else {
			ieLen = 0;
		}
		/* remember the IE for future use */
		if (ieLen) {
			if (ieLen > DOT11F_IE_RSN_MAX_LEN) {
				sms_log(pMac, LOGE,
					FL
					(" WPA RSN IE length :%d is more than DOT11F_IE_RSN_MAX_LEN, resetting to %d"),
					ieLen, DOT11F_IE_RSN_MAX_LEN);
				ieLen = DOT11F_IE_RSN_MAX_LEN;
			}
#ifdef FEATURE_WLAN_WAPI
			if (csr_is_profile_wapi(pProfile)) {
				/* Check whether we need to allocate more mem */
				if (ieLen > pSession->nWapiReqIeLength) {
					if (pSession->pWapiReqIE
					    && pSession->nWapiReqIeLength) {
						qdf_mem_free(pSession->
							     pWapiReqIE);
					}
					pSession->pWapiReqIE =
						qdf_mem_malloc(ieLen);
					if (NULL == pSession->pWapiReqIE)
						status = QDF_STATUS_E_NOMEM;
					else
						status = QDF_STATUS_SUCCESS;
					if (!QDF_IS_STATUS_SUCCESS(status))
						break;
				}
				pSession->nWapiReqIeLength = ieLen;
				qdf_mem_copy(pSession->pWapiReqIE, wpaRsnIE,
					     ieLen);
				csr_join_req->rsnIE.length = ieLen;
				qdf_mem_copy(&csr_join_req->rsnIE.rsnIEdata,
						 wpaRsnIE, ieLen);
			} else  /* should be WPA/WPA2 otherwise */
#endif /* FEATURE_WLAN_WAPI */
			{
				/* Check whether we need to allocate more mem */
				if (ieLen > pSession->nWpaRsnReqIeLength) {
					if (pSession->pWpaRsnReqIE
					    && pSession->nWpaRsnReqIeLength) {
						qdf_mem_free(pSession->
							     pWpaRsnReqIE);
					}
					pSession->pWpaRsnReqIE =
						qdf_mem_malloc(ieLen);
					if (NULL == pSession->pWpaRsnReqIE)
						status = QDF_STATUS_E_NOMEM;
					else
						status = QDF_STATUS_SUCCESS;
					if (!QDF_IS_STATUS_SUCCESS(status))
						break;
				}
				pSession->nWpaRsnReqIeLength = ieLen;
				qdf_mem_copy(pSession->pWpaRsnReqIE, wpaRsnIE,
					     ieLen);
				csr_join_req->rsnIE.length = ieLen;
				qdf_mem_copy(&csr_join_req->rsnIE.rsnIEdata,
						 wpaRsnIE, ieLen);
			}
		} else {
			/* free whatever old info */
			pSession->nWpaRsnReqIeLength = 0;
			if (pSession->pWpaRsnReqIE) {
				qdf_mem_free(pSession->pWpaRsnReqIE);
				pSession->pWpaRsnReqIE = NULL;
			}
#ifdef FEATURE_WLAN_WAPI
			pSession->nWapiReqIeLength = 0;
			if (pSession->pWapiReqIE) {
				qdf_mem_free(pSession->pWapiReqIE);
				pSession->pWapiReqIE = NULL;
			}
#endif /* FEATURE_WLAN_WAPI */
			csr_join_req->rsnIE.length = 0;
		}
#ifdef FEATURE_WLAN_ESE
		if (eWNI_SME_JOIN_REQ == messageType)
			csr_join_req->cckmIE.length = 0;
		else if (eWNI_SME_REASSOC_REQ == messageType) {
			/* cckmIE */
			if (csr_is_profile_ese(pProfile)) {
				/* Insert the CCKM IE into the join request */
				ieLen = pSession->suppCckmIeInfo.cckmIeLen;
				qdf_mem_copy((void *)(wpaRsnIE),
						pSession->suppCckmIeInfo.cckmIe,
						ieLen);
			} else
				ieLen = 0;
			/*
			 * If present, copy the IE into the
			 * eWNI_SME_REASSOC_REQ message buffer
			 */
			if (ieLen) {
				/*
				 * Copy the CCKM IE over from the temp
				 * buffer (wpaRsnIE)
				 */
				csr_join_req->cckmIE.length = ieLen;
				qdf_mem_copy(&csr_join_req->cckmIE.cckmIEdata,
						wpaRsnIE, ieLen);
			} else
				csr_join_req->cckmIE.length = 0;
		}
#endif /* FEATURE_WLAN_ESE */
		/* addIEScan */
		if (pProfile->nAddIEScanLength && pProfile->pAddIEScan) {
			ieLen = pProfile->nAddIEScanLength;
			if (ieLen > pSession->nAddIEScanLength) {
				if (pSession->pAddIEScan
					&& pSession->nAddIEScanLength) {
					qdf_mem_free(pSession->pAddIEScan);
				}
				pSession->pAddIEScan = qdf_mem_malloc(ieLen);
				if (NULL == pSession->pAddIEScan)
					status = QDF_STATUS_E_NOMEM;
				else
					status = QDF_STATUS_SUCCESS;
				if (!QDF_IS_STATUS_SUCCESS(status))
					break;
			}
			pSession->nAddIEScanLength = ieLen;
			qdf_mem_copy(pSession->pAddIEScan, pProfile->pAddIEScan,
					ieLen);
			csr_join_req->addIEScan.length = ieLen;
			qdf_mem_copy(&csr_join_req->addIEScan.addIEdata,
					pProfile->pAddIEScan, ieLen);
		} else {
			pSession->nAddIEScanLength = 0;
			if (pSession->pAddIEScan) {
				qdf_mem_free(pSession->pAddIEScan);
				pSession->pAddIEScan = NULL;
			}
			csr_join_req->addIEScan.length = 0;
		}
		/* addIEAssoc */
		if (pProfile->nAddIEAssocLength && pProfile->pAddIEAssoc) {
			ieLen = pProfile->nAddIEAssocLength;
			if (ieLen > pSession->nAddIEAssocLength) {
				if (pSession->pAddIEAssoc
				    && pSession->nAddIEAssocLength) {
					qdf_mem_free(pSession->pAddIEAssoc);
				}
				pSession->pAddIEAssoc = qdf_mem_malloc(ieLen);
				if (NULL == pSession->pAddIEAssoc)
					status = QDF_STATUS_E_NOMEM;
				else
					status = QDF_STATUS_SUCCESS;
				if (!QDF_IS_STATUS_SUCCESS(status))
					break;
			}
			pSession->nAddIEAssocLength = ieLen;
			qdf_mem_copy(pSession->pAddIEAssoc,
				     pProfile->pAddIEAssoc, ieLen);
			csr_join_req->addIEAssoc.length = ieLen;
			qdf_mem_copy(&csr_join_req->addIEAssoc.addIEdata,
					 pProfile->pAddIEAssoc, ieLen);
		} else {
			pSession->nAddIEAssocLength = 0;
			if (pSession->pAddIEAssoc) {
				qdf_mem_free(pSession->pAddIEAssoc);
				pSession->pAddIEAssoc = NULL;
			}
			csr_join_req->addIEAssoc.length = 0;
		}

		if (eWNI_SME_REASSOC_REQ == messageType) {
			/* Unmask any AC in reassoc that is ACM-set */
			uapsd_mask = (uint8_t) pProfile->uapsd_mask;
			if (uapsd_mask && (NULL != pBssDescription)) {
				if (CSR_IS_QOS_BSS(pIes)
						&& CSR_IS_UAPSD_BSS(pIes))
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
					acm_mask =
						sme_qos_get_acm_mask(pMac,
								pBssDescription,
								pIes);
#endif /* WLAN_MDM_CODE_REDUCTION_OPT */
				else
					uapsd_mask = 0;
			}
		}

		csr_join_req->UCEncryptionType =
				csr_translate_encrypt_type_to_ed_type
					(pProfile->negotiatedUCEncryptionType);

		csr_join_req->MCEncryptionType =
				csr_translate_encrypt_type_to_ed_type
					(pProfile->negotiatedMCEncryptionType);
#ifdef WLAN_FEATURE_11W
		if (pProfile->MFPEnabled)
			csr_join_req->MgmtEncryptionType = eSIR_ED_AES_128_CMAC;
		else
			csr_join_req->MgmtEncryptionType = eSIR_ED_NONE;
#endif
		if (pProfile->MFPEnabled &&
			 !(pProfile->MFPRequired) &&
			 ((pIes->RSN.present) &&
			 (!(pIes->RSN.RSN_Cap[0] >> 7) & 0x1)))
			csr_join_req->MgmtEncryptionType = eSIR_ED_NONE;
#ifdef FEATURE_WLAN_ESE
		ese_config =  pMac->roam.configParam.isEseIniFeatureEnabled;
#endif
		pProfile->MDID.mdiePresent = pBssDescription->mdiePresent;
		if (csr_is_profile11r(pProfile)
#ifdef FEATURE_WLAN_ESE
		    &&
		    !((pProfile->negotiatedAuthType ==
		       eCSR_AUTH_TYPE_OPEN_SYSTEM) && (pIes->ESEVersion.present)
		      && (ese_config))
#endif
			)
			csr_join_req->is11Rconnection = true;
		else
			csr_join_req->is11Rconnection = false;
#ifdef FEATURE_WLAN_ESE
		if (true == ese_config)
			csr_join_req->isESEFeatureIniEnabled = true;
		else
			csr_join_req->isESEFeatureIniEnabled = false;

		/* A profile can not be both ESE and 11R. But an 802.11R AP
		 * may be advertising support for ESE as well. So if we are
		 * associating Open or explicitly ESE then we will get ESE.
		 * If we are associating explictly 11R only then we will get
		 * 11R.
		 */
		if ((csr_is_profile_ese(pProfile) ||
			((pIes->ESEVersion.present) &&
			(pProfile->negotiatedAuthType ==
				eCSR_AUTH_TYPE_OPEN_SYSTEM)))
			&& (ese_config))
			csr_join_req->isESEconnection = true;
		else
			csr_join_req->isESEconnection = false;

		if (eWNI_SME_JOIN_REQ == messageType) {
			tESETspecInfo eseTspec;
			/*
			 * ESE-Tspec IEs in the ASSOC request is presently not
			 * supported. so nullify the TSPEC parameters
			 */
			qdf_mem_set(&eseTspec, sizeof(tESETspecInfo), 0);
			qdf_mem_copy(&csr_join_req->eseTspecInfo,
					&eseTspec, sizeof(tESETspecInfo));
		} else if (eWNI_SME_REASSOC_REQ == messageType) {
			if ((csr_is_profile_ese(pProfile) ||
				((pIes->ESEVersion.present)
				&& (pProfile->negotiatedAuthType ==
					eCSR_AUTH_TYPE_OPEN_SYSTEM))) &&
				(ese_config)) {
				tESETspecInfo eseTspec;
				qdf_mem_set(&eseTspec, sizeof(tESETspecInfo),
						0);
				eseTspec.numTspecs =
					sme_qos_ese_retrieve_tspec_info(pMac,
						sessionId,
						(tTspecInfo *) &eseTspec.
							tspec[0]);
				csr_join_req->eseTspecInfo.numTspecs =
					eseTspec.numTspecs;
				if (eseTspec.numTspecs) {
					qdf_mem_copy(&csr_join_req->eseTspecInfo
						.tspec[0],
						&eseTspec.tspec[0],
						(eseTspec.numTspecs *
							sizeof(tTspecInfo)));
				}
			} else {
				tESETspecInfo eseTspec;
				/**
				 * ESE-Tspec IEs in the ASSOC request is
				 * presently not supported. so nullify the TSPEC
				 * parameters
				 */
				qdf_mem_set(&eseTspec, sizeof(tESETspecInfo),
						0);
				qdf_mem_copy(&csr_join_req->eseTspecInfo,
						&eseTspec,
						sizeof(tESETspecInfo));
			}
		}
#endif /* FEATURE_WLAN_ESE */
		if (ese_config
		    || csr_roam_is_fast_roam_enabled(pMac, sessionId)) {
			csr_join_req->isFastTransitionEnabled = true;
		} else {
			csr_join_req->isFastTransitionEnabled = false;
		}
		if (csr_roam_is_fast_roam_enabled(pMac, sessionId))
			csr_join_req->isFastRoamIniFeatureEnabled = true;
		else
			csr_join_req->isFastRoamIniFeatureEnabled = false;

		csr_join_req->txLdpcIniFeatureEnabled =
			(uint8_t) pMac->roam.configParam.txLdpcEnable;

		if ((csr_is11h_supported(pMac))
		    && (CDS_IS_CHANNEL_5GHZ(pBssDescription->channelId))
		    && (pIes->Country.present)
		    && (!pMac->roam.configParam.
			fSupplicantCountryCodeHasPriority)) {
			csr_save_to_channel_power2_g_5_g(pMac,
				pIes->Country.num_triplets *
				sizeof(tSirMacChanInfo),
				(tSirMacChanInfo *)
				(&pIes->Country.triplets[0]));
			csr_apply_power2_current(pMac);
		}
		/*
		 * If RX LDPC has been disabled for 2.4GHz channels and enabled
		 * for 5Ghz for STA like persona here is how to handle those
		 * cases here (by now channel has been decided).
		 */
		if (eSIR_INFRASTRUCTURE_MODE == csr_join_req->bsstype)
			csr_set_ldpc_exception(pMac, pSession,
					pBssDescription->channelId,
					pMac->roam.configParam.rxLdpcEnable);
		qdf_mem_copy(&csr_join_req->htConfig,
				&pSession->htConfig, sizeof(tSirHTConfig));
		qdf_mem_copy(&csr_join_req->vht_config, &pSession->vht_config,
				sizeof(pSession->vht_config));
		sms_log(pMac, LOG2,
			FL("ht capability 0x%x VHT capability 0x%x"),
			(unsigned int)(*(uint32_t *) &csr_join_req->htConfig),
			(unsigned int)(*(uint32_t *) &csr_join_req->vht_config));
		if (wlan_cfg_get_int(pMac, WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
				     &value) != eSIR_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to get SU beamformee capability"));
		if (wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
				&value1) != eSIR_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to get CSN beamformee capability"));

		csr_join_req->vht_config.su_beam_formee = value;
		if (value) {
			txBFCsnValue = (uint8_t)value1;
			if (IS_BSS_VHT_CAPABLE(pIes->VHTCaps) &&
					pIes->VHTCaps.numSoundingDim)
				txBFCsnValue = QDF_MIN(txBFCsnValue,
						pIes->VHTCaps.numSoundingDim);
			else if (IS_BSS_VHT_CAPABLE(pIes->vendor2_ie.VHTCaps)
				&& pIes->vendor2_ie.VHTCaps.numSoundingDim)
				txBFCsnValue = QDF_MIN(txBFCsnValue,
					pIes->vendor2_ie.VHTCaps.numSoundingDim);
		}
		csr_join_req->vht_config.csnof_beamformer_antSup = txBFCsnValue;

		if (wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_MU_BEAMFORMEE_CAP, &value)
				!= eSIR_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to get CSN beamformee capability"));
		csr_join_req->vht_config.mu_beam_formee = (uint8_t) value;

		csr_join_req->enableVhtpAid =
			(uint8_t) pMac->roam.configParam.enableVhtpAid;

		csr_join_req->enableVhtGid =
			(uint8_t) pMac->roam.configParam.enableVhtGid;

		csr_join_req->enableAmpduPs =
			(uint8_t) pMac->roam.configParam.enableAmpduPs;

		csr_join_req->enableHtSmps =
			(uint8_t) pMac->roam.configParam.enableHtSmps;

		csr_join_req->htSmps = (uint8_t) pMac->roam.configParam.htSmps;
		csr_join_req->send_smps_action =
			pMac->roam.configParam.send_smps_action;

		csr_join_req->isAmsduSupportInAMPDU =
			(uint8_t) pMac->roam.configParam.isAmsduSupportInAMPDU;

		if (pMac->roam.roamSession[sessionId].fWMMConnection)
			csr_join_req->isWMEenabled = true;
		else
			csr_join_req->isWMEenabled = false;

		if (pMac->roam.roamSession[sessionId].fQOSConnection)
			csr_join_req->isQosEnabled = true;
		else
			csr_join_req->isQosEnabled = false;

		if (pProfile->bOSENAssociation)
			csr_join_req->isOSENConnection = true;
		else
			csr_join_req->isOSENConnection = false;

		/* Fill rrm config parameters */
		qdf_mem_copy(&csr_join_req->rrm_config,
			     &pMac->rrm.rrmSmeContext.rrmConfig,
			     sizeof(struct rrm_config_param));

		pAP_capabilityInfo =
			(tSirMacCapabilityInfo *)
				&pBssDescription->capabilityInfo;
		/*
		 * tell the target AP my 11H capability only if both AP and STA
		 * support
		 * 11H and the channel being used is 11a
		 */
		if (csr_is11h_supported(pMac) && pAP_capabilityInfo->spectrumMgt
			&& eSIR_11A_NW_TYPE == pBssDescription->nwType) {
			fTmp = (tAniBool) 1;
		} else
			fTmp = (tAniBool) 0;

		csr_join_req->spectrumMgtIndicator = fTmp;
		csr_join_req->powerCap.minTxPower = MIN_TX_PWR_CAP;
		/*
		 * This is required for 11k test VoWiFi Ent: Test 2.
		 * We need the power capabilities for Assoc Req.
		 * This macro is provided by the halPhyCfg.h. We pick our
		 * max and min capability by the halPhy provided macros
		 */
		pwrLimit = csr_get_cfg_max_tx_power(pMac,
					pBssDescription->channelId);
		if (0 != pwrLimit)
			csr_join_req->powerCap.maxTxPower = pwrLimit;
		else
			csr_join_req->powerCap.maxTxPower = MAX_TX_PWR_CAP;

		csr_add_supported_5Ghz_channels(pMac,
				csr_join_req->supportedChannels.channelList,
				&csr_join_req->supportedChannels.numChnl,
				false);

		csr_join_req->uapsdPerAcBitmask = (uint8_t)pProfile->uapsd_mask;
		/* Move the entire BssDescription into the join request. */
		qdf_mem_copy(&csr_join_req->bssDescription, pBssDescription,
				pBssDescription->length +
				sizeof(pBssDescription->length));

		/*
		 * conc_custom_rule1:
		 * If SAP comes up first and STA comes up later then SAP
		 * need to follow STA's channel in 2.4Ghz. In following if
		 * condition we are adding sanity check, just to make sure that
		 * if this rule is enabled then don't allow STA to connect on
		 * 5gz channel and also by this time SAP's channel should be the
		 * same as STA's channel.
		 */
		if (pMac->roam.configParam.conc_custom_rule1) {
			if ((0 ==
			     pMac->
			      roam.configParam.is_sta_connection_in_5gz_enabled)
			     && CDS_IS_CHANNEL_5GHZ(pBssDescription->
							channelId)) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("STA-conn on 5G isn't allowed"));
				status = QDF_STATUS_E_FAILURE;
				break;
			}
			if (!CDS_IS_CHANNEL_5GHZ(pBssDescription->channelId) &&
				(false == csr_is_conn_allow_2g_band(pMac,
						pBssDescription->channelId))) {
				status = QDF_STATUS_E_FAILURE;
				break;
			}
		}

		/*
		 * conc_custom_rule2:
		 * If P2PGO comes up first and STA comes up later then P2PGO
		 * need to follow STA's channel in 5Ghz. In following if
		 * condition we are just adding sanity check to make sure that
		 * by this time P2PGO's channel is same as STA's channel.
		 */
		if (pMac->roam.configParam.conc_custom_rule2) {
			if (!CDS_IS_CHANNEL_24GHZ(pBssDescription->channelId) &&
				(false == csr_is_conn_allow_5g_band(pMac,
				pBssDescription->channelId))) {
					status = QDF_STATUS_E_FAILURE;
					break;
			}
		}
		status = cds_send_mb_message_to_mac(csr_join_req);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			/*
			 * cds_send_mb_message_to_mac would've released the mem
			 * allocated by csr_join_req. Let's make it defensive by
			 * assigning NULL to the pointer.
			 */
			csr_join_req = NULL;
			break;
		} else {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
			if (eWNI_SME_JOIN_REQ == messageType) {
				/* Notify QoS module that join happening */
				pSession->join_bssid_count++;
				QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_DEBUG,
						"BSSID Count = %d",
						pSession->join_bssid_count);
				sme_qos_csr_event_ind(pMac, (uint8_t) sessionId,
						SME_QOS_CSR_JOIN_REQ, NULL);
			} else if (eWNI_SME_REASSOC_REQ == messageType) {
				/* Notify QoS module that reassoc happening */
				sme_qos_csr_event_ind(pMac, (uint8_t) sessionId,
						SME_QOS_CSR_REASSOC_REQ,
						NULL);
			}
#endif
		}
	} while (0);

	/* Clean up the memory in case of any failure */
	if (!QDF_IS_STATUS_SUCCESS(status) && (NULL != csr_join_req))
		qdf_mem_free(csr_join_req);

	return status;
}

/* */
QDF_STATUS csr_send_mb_disassoc_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					tSirMacAddr bssId, uint16_t reasonCode)
{
	tSirSmeDisassocReq *pMsg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!CSR_IS_SESSION_VALID(pMac, sessionId))
		return QDF_STATUS_E_FAILURE;

	pMsg = qdf_mem_malloc(sizeof(tSirSmeDisassocReq));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(pMsg, sizeof(tSirSmeDisassocReq), 0);
	pMsg->messageType = eWNI_SME_DISASSOC_REQ;
	pMsg->length = sizeof(tSirSmeDisassocReq);
	pMsg->sessionId = sessionId;
	pMsg->transactionId = 0;
	if ((pSession->pCurRoamProfile != NULL)
		&& (CSR_IS_INFRA_AP(pSession->pCurRoamProfile))) {
		qdf_mem_copy(&pMsg->bssid.bytes,
			     &pSession->selfMacAddr,
			     QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(&pMsg->peer_macaddr.bytes,
			     bssId,
			     QDF_MAC_ADDR_SIZE);
	} else {
		qdf_mem_copy(&pMsg->bssid.bytes,
			     bssId, QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(&pMsg->peer_macaddr.bytes,
			     bssId, QDF_MAC_ADDR_SIZE);
	}
	pMsg->reasonCode = reasonCode;
	/*
	 * The state will be DISASSOC_HANDOFF only when we are doing
	 * handoff. Here we should not send the disassoc over the air
	 * to the AP
	 */
	if (CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac, sessionId)
	    && csr_roam_is11r_assoc(pMac, sessionId)) {
		/* Set DoNotSendOverTheAir flag to 1 only for handoff case */
		pMsg->doNotSendOverTheAir = CSR_DONT_SEND_DISASSOC_OVER_THE_AIR;
	}
	return cds_send_mb_message_to_mac(pMsg);
}

QDF_STATUS
csr_send_mb_get_associated_stas_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					QDF_MODULE_ID modId,
					struct qdf_mac_addr bssid,
					void *pUsrContext,
					void *pfnSapEventCallback,
					uint8_t *pAssocStasBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeGetAssocSTAsReq *pMsg;

	pMsg = qdf_mem_malloc(sizeof(*pMsg));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;

	pMsg->messageType = eWNI_SME_GET_ASSOC_STAS_REQ;
	qdf_copy_macaddr(&pMsg->bssid, &bssid);
	pMsg->modId = modId;
	qdf_mem_copy(pMsg->pUsrContext, pUsrContext, sizeof(void *));
	qdf_mem_copy(pMsg->pSapEventCallback,
			pfnSapEventCallback, sizeof(void *));
	qdf_mem_copy(pMsg->pAssocStasArray, pAssocStasBuf, sizeof(void *));
	pMsg->length = sizeof(*pMsg);
	status = cds_send_mb_message_to_mac(pMsg);

	return status;
}

QDF_STATUS
csr_send_mb_get_wpspbc_sessions(tpAniSirGlobal pMac, uint32_t sessionId,
				struct qdf_mac_addr bssid, void *pUsrContext,
				void *pfnSapEventCallback,
				struct qdf_mac_addr remove_mac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeGetWPSPBCSessionsReq *pMsg;

	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeGetWPSPBCSessionsReq));
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		qdf_mem_set(pMsg, sizeof(tSirSmeGetWPSPBCSessionsReq), 0);
		pMsg->messageType = eWNI_SME_GET_WPSPBC_SESSION_REQ;
		qdf_mem_copy(pMsg->pUsrContext, pUsrContext, sizeof(void *));
		qdf_mem_copy(pMsg->pSapEventCallback, pfnSapEventCallback,
			     sizeof(void *));
		qdf_copy_macaddr(&pMsg->bssid, &bssid);
		qdf_copy_macaddr(&pMsg->remove_mac, &remove_mac);
		pMsg->length = sizeof(struct sSirSmeGetWPSPBCSessionsReq);
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS csr_send_chng_mcc_beacon_interval(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tpSirChangeBIParams pMsg;
	uint16_t len = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	/* NO need to update the Beacon Params if update beacon parameter flag is not set */
	if (!pMac->roam.roamSession[sessionId].bssParams.updatebeaconInterval)
		return QDF_STATUS_SUCCESS;

	pMac->roam.roamSession[sessionId].bssParams.updatebeaconInterval =
		false;

	/* Create the message and send to lim */
	len = sizeof(tSirChangeBIParams);
	pMsg = qdf_mem_malloc(len);
	if (NULL == pMsg)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_set(pMsg, sizeof(tSirChangeBIParams), 0);
		pMsg->messageType = eWNI_SME_CHNG_MCC_BEACON_INTERVAL;
		pMsg->length = len;

		qdf_copy_macaddr(&pMsg->bssid, &pSession->selfMacAddr);
		sms_log(pMac, LOG1,
			FL("CSR Attempting to change BI for Bssid= "
			   MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pMsg->bssid.bytes));
		pMsg->sessionId = sessionId;
		sms_log(pMac, LOG1, FL("  session %d BeaconInterval %d"),
			sessionId,
			pMac->roam.roamSession[sessionId].bssParams.
			beaconInterval);
		pMsg->beaconInterval =
			pMac->roam.roamSession[sessionId].bssParams.beaconInterval;
		status = cds_send_mb_message_to_mac(pMsg);
	}
	return status;
}

#ifdef QCA_HT_2040_COEX
QDF_STATUS csr_set_ht2040_mode(tpAniSirGlobal pMac, uint32_t sessionId,
			       ePhyChanBondState cbMode, bool obssEnabled)
{
	tpSirSetHT2040Mode pMsg;
	uint16_t len = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	/* Create the message and send to lim */
	len = sizeof(tSirSetHT2040Mode);
	pMsg = qdf_mem_malloc(len);
	if (NULL == pMsg)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_set(pMsg, sizeof(tSirSetHT2040Mode), 0);
		pMsg->messageType = eWNI_SME_SET_HT_2040_MODE;
		pMsg->length = len;

		qdf_copy_macaddr(&pMsg->bssid, &pSession->selfMacAddr);
		sms_log(pMac, LOG1,
			FL("CSR Attempting to set HT20/40 mode for Bssid= "
			   MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pMsg->bssid.bytes));
		pMsg->sessionId = sessionId;
		sms_log(pMac, LOG1, FL("  session %d HT20/40 mode %d"),
			sessionId, cbMode);
		pMsg->cbMode = cbMode;
		pMsg->obssEnabled = obssEnabled;
		status = cds_send_mb_message_to_mac(pMsg);
	}
	return status;
}
#endif

QDF_STATUS csr_send_mb_deauth_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
				      tSirMacAddr bssId, uint16_t reasonCode)
{
	tSirSmeDeauthReq *pMsg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!CSR_IS_SESSION_VALID(pMac, sessionId))
		return QDF_STATUS_E_FAILURE;

	pMsg = qdf_mem_malloc(sizeof(tSirSmeDeauthReq));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(pMsg, sizeof(tSirSmeDeauthReq), 0);
	pMsg->messageType = eWNI_SME_DEAUTH_REQ;
	pMsg->length = sizeof(tSirSmeDeauthReq);
	pMsg->sessionId = sessionId;
	pMsg->transactionId = 0;

	if ((pSession->pCurRoamProfile != NULL)
	     && (CSR_IS_INFRA_AP(pSession->pCurRoamProfile))) {
		qdf_mem_copy(&pMsg->bssid,
			     &pSession->selfMacAddr,
			     QDF_MAC_ADDR_SIZE);
	} else {
		qdf_mem_copy(&pMsg->bssid,
			     bssId, QDF_MAC_ADDR_SIZE);
	}

	/* Set the peer MAC address before sending the message to LIM */
	qdf_mem_copy(&pMsg->peer_macaddr.bytes, bssId, QDF_MAC_ADDR_SIZE);
	pMsg->reasonCode = reasonCode;

	return cds_send_mb_message_to_mac(pMsg);
}

QDF_STATUS csr_send_mb_disassoc_cnf_msg(tpAniSirGlobal pMac,
					tpSirSmeDisassocInd pDisassocInd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeDisassocCnf *pMsg;
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeDisassocCnf));
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		qdf_mem_set(pMsg, sizeof(tSirSmeDisassocCnf), 0);
		pMsg->messageType = eWNI_SME_DISASSOC_CNF;
		pMsg->statusCode = eSIR_SME_SUCCESS;
		pMsg->length = sizeof(tSirSmeDisassocCnf);
		qdf_copy_macaddr(&pMsg->peer_macaddr,
				 &pDisassocInd->peer_macaddr);
		status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_free(pMsg);
			break;
		}

		qdf_copy_macaddr(&pMsg->bssid, &pDisassocInd->bssid);
		status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_free(pMsg);
			break;
		}

		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS csr_send_mb_deauth_cnf_msg(tpAniSirGlobal pMac,
				      tpSirSmeDeauthInd pDeauthInd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeDeauthCnf *pMsg;
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeDeauthCnf));
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		qdf_mem_set(pMsg, sizeof(tSirSmeDeauthCnf), 0);
		pMsg->messageType = eWNI_SME_DEAUTH_CNF;
		pMsg->statusCode = eSIR_SME_SUCCESS;
		pMsg->length = sizeof(tSirSmeDeauthCnf);
		qdf_copy_macaddr(&pMsg->bssid, &pDeauthInd->bssid);
		status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_free(pMsg);
			break;
		}
		qdf_copy_macaddr(&pMsg->peer_macaddr,
				 &pDeauthInd->peer_macaddr);
		status = QDF_STATUS_SUCCESS;
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_free(pMsg);
			break;
		}
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS csr_send_assoc_cnf_msg(tpAniSirGlobal pMac, tpSirSmeAssocInd pAssocInd,
				  QDF_STATUS Halstatus)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeAssocCnf *pMsg;

	sms_log(pMac, LOG1,
		FL("Posting eWNI_SME_ASSOC_CNF to LIM.HalStatus :%d"),
		Halstatus);
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeAssocCnf));
		if (NULL == pMsg)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_set(pMsg, sizeof(tSirSmeAssocCnf), 0);
		pMsg->messageType = eWNI_SME_ASSOC_CNF;
		pMsg->length = sizeof(tSirSmeAssocCnf);
		if (QDF_IS_STATUS_SUCCESS(Halstatus))
			pMsg->statusCode = eSIR_SME_SUCCESS;
		else
			pMsg->statusCode = eSIR_SME_ASSOC_REFUSED;
		/* bssId */
		qdf_mem_copy(pMsg->bssid.bytes, pAssocInd->bssId,
			     QDF_MAC_ADDR_SIZE);
		/* peerMacAddr */
		qdf_mem_copy(pMsg->peer_macaddr.bytes, pAssocInd->peerMacAddr,
			     QDF_MAC_ADDR_SIZE);
		/* aid */
		pMsg->aid = pAssocInd->aid;
		/* alternateBssId */
		qdf_mem_copy(pMsg->alternate_bssid.bytes, pAssocInd->bssId,
			     QDF_MAC_ADDR_SIZE);
		/* alternateChannelId */
		pMsg->alternateChannelId = 11;
		/* pMsg is freed by cds_send_mb_message_to_mac in anycase*/
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS csr_send_assoc_ind_to_upper_layer_cnf_msg(tpAniSirGlobal pMac,
						     tpSirSmeAssocInd pAssocInd,
						     QDF_STATUS Halstatus,
						     uint8_t sessionId)
{
	tSirMsgQ msgQ;
	tSirSmeAssocIndToUpperLayerCnf *pMsg;
	uint8_t *pBuf;
	tSirResultCodes statusCode;
	uint16_t wTmp;
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirSmeAssocIndToUpperLayerCnf));
		if (NULL == pMsg)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_set(pMsg, sizeof(tSirSmeAssocIndToUpperLayerCnf), 0);

		pMsg->messageType = eWNI_SME_UPPER_LAYER_ASSOC_CNF;
		pMsg->length = sizeof(tSirSmeAssocIndToUpperLayerCnf);

		pMsg->sessionId = sessionId;

		pBuf = (uint8_t *) &pMsg->statusCode;
		if (QDF_IS_STATUS_SUCCESS(Halstatus))
			statusCode = eSIR_SME_SUCCESS;
		else
			statusCode = eSIR_SME_ASSOC_REFUSED;
		qdf_mem_copy(pBuf, &statusCode, sizeof(tSirResultCodes));
		pBuf += sizeof(tSirResultCodes);
		/* bssId */
		qdf_mem_copy((tSirMacAddr *) pBuf, pAssocInd->bssId,
			     sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);
		/* peerMacAddr */
		qdf_mem_copy((tSirMacAddr *) pBuf, pAssocInd->peerMacAddr,
			     sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);
		/* StaId */
		wTmp = pAssocInd->staId;
		qdf_mem_copy(pBuf, &wTmp, sizeof(uint16_t));
		pBuf += sizeof(uint16_t);
		/* alternateBssId */
		qdf_mem_copy((tSirMacAddr *) pBuf, pAssocInd->bssId,
			     sizeof(tSirMacAddr));
		pBuf += sizeof(tSirMacAddr);
		/* alternateChannelId */
		*pBuf = 11;
		pBuf += sizeof(uint8_t);
		/* Instead of copying roam Info, we just copy only WmmEnabled, RsnIE information */
		/* Wmm */
		*pBuf = pAssocInd->wmmEnabledSta;
		pBuf += sizeof(uint8_t);
		/* RSN IE */
		qdf_mem_copy((tSirRSNie *) pBuf, &pAssocInd->rsnIE,
			     sizeof(tSirRSNie));
		pBuf += sizeof(tSirRSNie);
#ifdef FEATURE_WLAN_WAPI
		/* WAPI IE */
		qdf_mem_copy((tSirWAPIie *) pBuf, &pAssocInd->wapiIE,
			     sizeof(tSirWAPIie));
		pBuf += sizeof(tSirWAPIie);
#endif
		/* Additional IE */
		qdf_mem_copy((void *)pBuf, &pAssocInd->addIE,
			     sizeof(tSirAddie));
		pBuf += sizeof(tSirAddie);
		/* reassocReq */
		*pBuf = pAssocInd->reassocReq;
		pBuf += sizeof(uint8_t);
		/* timingMeasCap */
		*pBuf = pAssocInd->timingMeasCap;
		pBuf += sizeof(uint8_t);
		qdf_mem_copy((void *)pBuf, &pAssocInd->chan_info,
			     sizeof(tSirSmeChanInfo));
		msgQ.type = eWNI_SME_UPPER_LAYER_ASSOC_CNF;
		msgQ.bodyptr = pMsg;
		msgQ.bodyval = 0;
		sys_process_mmh_msg(pMac, &msgQ);
	} while (0);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_send_mb_set_context_req_msg(tpAniSirGlobal pMac,
					   uint32_t sessionId,
					   struct qdf_mac_addr peer_macaddr,
					   uint8_t numKeys,
					   tAniEdType edType, bool fUnicast,
					   tAniKeyDirection aniKeyDirection,
					   uint8_t keyId, uint8_t keyLength,
					   uint8_t *pKey, uint8_t paeRole,
					   uint8_t *pKeyRsc)
{
	tSirSmeSetContextReq *pMsg;
	uint16_t msgLen;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	sms_log(pMac, LOG1, FL("keylength is %d, Encry type is : %d"),
		keyLength, edType);
	do {
		if ((1 != numKeys) && (0 != numKeys))
			break;
		/*
		 * All of these fields appear in every SET_CONTEXT message.
		 * Below we'll add in the size for each key set. Since we only support
		 * up to one key, we always allocate memory for 1 key.
		 */
		msgLen = sizeof(struct sSirSmeSetContextReq);

		pMsg = qdf_mem_malloc(msgLen);
		if (NULL == pMsg)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_set(pMsg, msgLen, 0);
		pMsg->messageType = eWNI_SME_SETCONTEXT_REQ;
		pMsg->length = msgLen;
		pMsg->sessionId = (uint8_t) sessionId;
		pMsg->transactionId = 0;
		qdf_copy_macaddr(&pMsg->peer_macaddr, &peer_macaddr);
		qdf_copy_macaddr(&pMsg->bssid,
				 &pSession->connectedProfile.bssid);

		/**
		 * Set the pMsg->keyMaterial.length field
		 * (this length is defined as all data that follows the
		 * edType field in the tSirKeyMaterial keyMaterial; field).
		 *
		 * NOTE:  This keyMaterial.length contains the length of a
		 * MAX size key, though the keyLength can be shorter than this
		 * max size.  Is LIM interpreting this ok ?
		 */
		pMsg->keyMaterial.length =
				sizeof(pMsg->keyMaterial.numKeys) +
				(numKeys * sizeof(pMsg->keyMaterial.key));
		pMsg->keyMaterial.edType = edType;
		pMsg->keyMaterial.numKeys = numKeys;
		pMsg->keyMaterial.key[0].keyId = keyId;
		pMsg->keyMaterial.key[0].unicast = fUnicast;
		pMsg->keyMaterial.key[0].keyDirection = aniKeyDirection;
		qdf_mem_copy(pMsg->keyMaterial.key[0].keyRsc,
				pKeyRsc, CSR_MAX_RSC_LEN);
		/* 0 is Supplicant */
		pMsg->keyMaterial.key[0].paeRole = paeRole;
		pMsg->keyMaterial.key[0].keyLength = keyLength;
		if (keyLength && pKey) {
			qdf_mem_copy(pMsg->keyMaterial.key[0].key,
					pKey, keyLength);
			sms_log(pMac, LOG1,
				FL("SME set keyIndx (%d) encType (%d) key"),
				keyId, edType);
			sir_dump_buf(pMac, SIR_SMS_MODULE_ID, LOG1, pKey,
				     keyLength);
		}
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

QDF_STATUS csr_send_mb_start_bss_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					 eCsrRoamBssType bssType,
					 tCsrRoamStartBssParams *pParam,
					 tSirBssDescription *pBssDesc)
{
	tSirSmeStartBssReq *pMsg;
	uint16_t wTmp;
	uint32_t value = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	pSession->joinFailStatusCode.statusCode = eSIR_SME_SUCCESS;
	pSession->joinFailStatusCode.reasonCode = 0;
	pMsg = qdf_mem_malloc(sizeof(tSirSmeStartBssReq));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(pMsg, sizeof(tSirSmeStartBssReq), 0);
	pMsg->messageType = eWNI_SME_START_BSS_REQ;
	pMsg->sessionId = sessionId;
	pMsg->length = sizeof(tSirSmeStartBssReq);
	pMsg->transactionId = 0;
	qdf_copy_macaddr(&pMsg->bssid, &pParam->bssid);
	/* selfMacAddr */
	qdf_copy_macaddr(&pMsg->self_macaddr, &pSession->selfMacAddr);
	/* beaconInterval */
	if (pBssDesc && pBssDesc->beaconInterval)
		wTmp = pBssDesc->beaconInterval;
	else if (pParam->beaconInterval)
		wTmp = pParam->beaconInterval;
	else
		wTmp = WNI_CFG_BEACON_INTERVAL_STADEF;

	csr_validate_mcc_beacon_interval(pMac, pParam->operationChn,
					 &wTmp, sessionId, pParam->bssPersona);
	/* Update the beacon Interval */
	pParam->beaconInterval = wTmp;
	pMsg->beaconInterval = wTmp;
	pMsg->dot11mode =
		csr_translate_to_wni_cfg_dot11_mode(pMac,
						    pParam->uCfgDot11Mode);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	pMsg->cc_switch_mode = pMac->roam.configParam.cc_switch_mode;
#endif
	pMsg->bssType = csr_translate_bsstype_to_mac_type(bssType);
	qdf_mem_copy(&pMsg->ssId, &pParam->ssId, sizeof(pParam->ssId));
	pMsg->channelId = pParam->operationChn;
	/* What should we really do for the cbmode. */
	pMsg->cbMode = (ePhyChanBondState) pParam->cbMode;
	pMsg->vht_channel_width = pParam->ch_params.ch_width;
	pMsg->center_freq_seg0 = pParam->ch_params.center_freq_seg0;
	pMsg->center_freq_seg1 = pParam->ch_params.center_freq_seg1;
	pMsg->sec_ch_offset = pParam->ch_params.sec_ch_offset;
	pMsg->privacy = pParam->privacy;
	pMsg->apUapsdEnable = pParam->ApUapsdEnable;
	pMsg->ssidHidden = pParam->ssidHidden;
	pMsg->fwdWPSPBCProbeReq = (uint8_t) pParam->fwdWPSPBCProbeReq;
	pMsg->protEnabled = (uint8_t) pParam->protEnabled;
	pMsg->obssProtEnabled = (uint8_t) pParam->obssProtEnabled;
	/* set cfg related to protection */
	pMsg->ht_capab = pParam->ht_protection;
	pMsg->authType = pParam->authType;
	pMsg->dtimPeriod = pParam->dtimPeriod;
	pMsg->wps_state = pParam->wps_state;
	pMsg->isCoalesingInIBSSAllowed = pMac->isCoalesingInIBSSAllowed;
	pMsg->bssPersona = pParam->bssPersona;
	pMsg->txLdpcIniFeatureEnabled = pMac->roam.configParam.txLdpcEnable;

	if (wlan_cfg_get_int(pMac, WNI_CFG_VHT_SU_BEAMFORMEE_CAP, &value)
					!= eSIR_SUCCESS)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  FL("could not get SU beam formee capability"));
	pMsg->vht_config.su_beam_formee =
		(uint8_t)value &&
		(uint8_t)pMac->roam.configParam.enable_txbf_sap_mode;
	if (wlan_cfg_get_int(pMac,
			WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
			&value) != eSIR_SUCCESS)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to get CSN beamformee capability"));
	pMsg->vht_config.csnof_beamformer_antSup = (uint8_t)value;

#ifdef WLAN_FEATURE_11W
	pMsg->pmfCapable = pParam->mfpCapable;
	pMsg->pmfRequired = pParam->mfpRequired;
#endif

	if (pParam->nRSNIELength > sizeof(pMsg->rsnIE.rsnIEdata)) {
		qdf_mem_free(pMsg);
		return QDF_STATUS_E_INVAL;
	}
	pMsg->rsnIE.length = pParam->nRSNIELength;
	qdf_mem_copy(pMsg->rsnIE.rsnIEdata,
		     pParam->pRSNIE,
		     pParam->nRSNIELength);
	pMsg->nwType = (tSirNwType)pParam->sirNwType;
	qdf_mem_copy(&pMsg->operationalRateSet,
		     &pParam->operationalRateSet,
		     sizeof(tSirMacRateSet));
	qdf_mem_copy(&pMsg->extendedRateSet,
		     &pParam->extendedRateSet,
		     sizeof(tSirMacRateSet));
	if (eSIR_IBSS_MODE == pMsg->bssType)
		csr_set_ldpc_exception(pMac, pSession,
				pMsg->channelId,
				pMac->roam.configParam.rxLdpcEnable);
	qdf_mem_copy(&pMsg->vht_config,
		     &pSession->vht_config,
		     sizeof(pSession->vht_config));
	qdf_mem_copy(&pMsg->htConfig,
		     &pSession->htConfig,
		     sizeof(tSirHTConfig));
	sms_log(pMac, LOG2, FL("ht capability 0x%x VHT capability 0x%x"),
			 (uint32_t)(*(uint32_t *) &pMsg->htConfig),
			 (uint32_t)(*(uint32_t *) &pMsg->vht_config));
	qdf_mem_copy(&pMsg->addIeParams,
		     &pParam->addIeParams,
		     sizeof(pParam->addIeParams));
	pMsg->obssEnabled = pMac->roam.configParam.obssEnabled;
	pMsg->sap_dot11mc = pParam->sap_dot11mc;

	return cds_send_mb_message_to_mac(pMsg);
}

QDF_STATUS csr_send_mb_stop_bss_req_msg(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tSirSmeStopBssReq *pMsg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	pMsg = qdf_mem_malloc(sizeof(tSirSmeStopBssReq));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_set(pMsg, sizeof(tSirSmeStopBssReq), 0);
	pMsg->messageType = eWNI_SME_STOP_BSS_REQ;
	pMsg->sessionId = sessionId;
	pMsg->length = sizeof(tSirSmeStopBssReq);
	pMsg->transactionId = 0;
	pMsg->reasonCode = 0;
	qdf_copy_macaddr(&pMsg->bssid, &pSession->connectedProfile.bssid);
	return cds_send_mb_message_to_mac(pMsg);
}

QDF_STATUS csr_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
		       tCsrRoamModifyProfileFields *pModProfileFields,
		       uint32_t *pRoamId, bool fForce)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t roamId = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if ((csr_is_conn_state_connected(pMac, sessionId)) &&
	    (fForce || (qdf_mem_cmp(&pModProfileFields,
				     &pSession->connectedProfile.
				     modifyProfileFields,
				     sizeof(tCsrRoamModifyProfileFields))))) {
		roamId = GET_NEXT_ROAM_ID(&pMac->roam);
		if (pRoamId) {
			*pRoamId = roamId;
		}

		status =
			csr_roam_issue_reassoc(pMac, sessionId, NULL,
					       pModProfileFields,
					       eCsrSmeIssuedReassocToSameAP,
					       roamId, false);
	}
	return status;
}

static QDF_STATUS csr_roam_session_opened(tpAniSirGlobal pMac,
					  uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo roamInfo;
	qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
	status = csr_roam_call_callback(pMac, sessionId, &roamInfo, 0,
					eCSR_ROAM_SESSION_OPENED,
					eCSR_ROAM_RESULT_NONE);
	return status;
}

QDF_STATUS csr_process_add_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg)
{
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	struct add_sta_self_params *rsp;
	struct send_extcap_ie *msg;
	QDF_STATUS status;

	if (pMsg == NULL) {
		sms_log(pMac, LOGE, "in %s msg ptr is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (!pEntry) {
		sms_log(pMac, LOGE, "in %s NO commands are ACTIVE ...",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (eSmeCommandAddStaSession != pCommand->command) {
		sms_log(pMac, LOGE, "in %s Cmd not in active list ...",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}
	rsp = (struct add_sta_self_params *) pMsg;
	sms_log(pMac, LOG1, "Add Sta self rsp status = %d", rsp->status);

	if (QDF_STATUS_SUCCESS == rsp->status &&
		(WMI_VDEV_TYPE_STA == rsp->type ||
		(WMI_VDEV_TYPE_AP == rsp->type &&
		 WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE == rsp->sub_type))) {
		sms_log(pMac, LOG1, FL("send SET IE msg to PE"));
		msg = qdf_mem_malloc(sizeof(*msg));
		if (NULL == msg) {
			sms_log(pMac, LOGE, FL("Memory allocation failed"));
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_set(msg, sizeof(*msg), 0);
		msg->msg_type = eWNI_SME_SET_IE_REQ;
		msg->session_id = rsp->session_id;
		msg->length = sizeof(*msg);
		status = cds_send_mb_message_to_mac(msg);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sms_log(pMac, LOGE,
				FL("Failed to send down the set IE req "));
	}

	csr_roam_session_opened(pMac, pCommand->sessionId);
	/* Remove this command out of the active list */
	if (csr_ll_remove_entry(&pMac->sme.smeCmdActiveList, pEntry,
		 LL_ACCESS_LOCK)) {
	/* Now put this command back on the avilable command list */
		csr_release_command(pMac, pCommand);
	}
	sme_process_pending_queue(pMac);
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_get_vdev_type_nss() - gets the nss value based on vdev type
 * @mac_ctx: Pointer to Global MAC structure
 * @dev_mode: current device operating mode.
 * @nss2g: Pointer to the 2G Nss parameter.
 * @nss5g: Pointer to the 5G Nss parameter.
 *
 * Fills the 2G and 5G Nss values based on device mode.
 *
 * Return: None
 */
void csr_get_vdev_type_nss(tpAniSirGlobal mac_ctx,
		enum tQDF_ADAPTER_MODE dev_mode,
		uint8_t *nss_2g, uint8_t *nss_5g)
{
	switch (dev_mode) {
	case QDF_STA_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.sta;
		*nss_5g = mac_ctx->vdev_type_nss_5g.sta;
		break;
	case QDF_SAP_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.sap;
		*nss_5g = mac_ctx->vdev_type_nss_5g.sap;
		break;
	case QDF_P2P_CLIENT_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.p2p_cli;
		*nss_5g = mac_ctx->vdev_type_nss_5g.p2p_cli;
		break;
	case QDF_P2P_GO_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.p2p_go;
		*nss_5g = mac_ctx->vdev_type_nss_5g.p2p_go;
		break;
	case QDF_P2P_DEVICE_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.p2p_dev;
		*nss_5g = mac_ctx->vdev_type_nss_5g.p2p_dev;
		break;
	case QDF_IBSS_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.ibss;
		*nss_5g = mac_ctx->vdev_type_nss_5g.ibss;
		break;
	case QDF_OCB_MODE:
		*nss_2g = mac_ctx->vdev_type_nss_2g.ocb;
		*nss_5g = mac_ctx->vdev_type_nss_5g.ocb;
		break;
	default:
		*nss_2g = 1;
		*nss_5g = 1;
		sms_log(mac_ctx, LOGE, FL("Unknown device mode"));
		break;
	}
	sms_log(mac_ctx, LOG1, FL("mode - %d: nss_2g - %d, 5g - %d"),
			dev_mode, *nss_2g, *nss_5g);
}
QDF_STATUS csr_issue_add_sta_for_session_req(tpAniSirGlobal pMac, uint32_t sessionId,
					     tSirMacAddr sessionMacAddr,
					     uint32_t type, uint32_t subType)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;
	pCommand = csr_get_command_buffer(pMac);
	if (NULL == pCommand) {
		status = QDF_STATUS_E_RESOURCES;
	} else {
		pCommand->command = eSmeCommandAddStaSession;
		pCommand->sessionId = (uint8_t) sessionId;
		qdf_mem_copy(pCommand->u.addStaSessionCmd.selfMacAddr,
			     sessionMacAddr, sizeof(tSirMacAddr));
		pCommand->u.addStaSessionCmd.currDeviceMode =
			pMac->sme.currDeviceMode;
		pCommand->u.addStaSessionCmd.type = type;
		pCommand->u.addStaSessionCmd.subType = subType;
		status = csr_queue_sme_command(pMac, pCommand, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			/* Should be panic?? */
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
		}
	}
	return status;
}

QDF_STATUS csr_process_add_sta_session_command(tpAniSirGlobal pMac,
					       tSmeCmd *pCommand)
{
	tAddStaForSessionCmd *pAddStaReq =
		&pCommand->u.addStaSessionCmd;
	uint8_t sessionId = pCommand->sessionId;
	struct add_sta_self_params *add_sta_self_req;
	uint8_t nss_2g;
	uint8_t nss_5g;
	QDF_STATUS status = QDF_STATUS_E_NOMEM;
	tSirMsgQ msg;

	add_sta_self_req = qdf_mem_malloc(sizeof(struct add_sta_self_params));
	if (NULL == add_sta_self_req) {
		sms_log(pMac, LOGP,
			FL
			("Unable to allocate memory for tAddSelfStaParams"));
		return status;
	}

	csr_get_vdev_type_nss(pMac, pAddStaReq->currDeviceMode,
			&nss_2g, &nss_5g);
	qdf_mem_copy(add_sta_self_req->self_mac_addr, pAddStaReq->selfMacAddr,
			sizeof(tSirMacAddr));
	add_sta_self_req->curr_device_mode = pAddStaReq->currDeviceMode;
	add_sta_self_req->session_id = sessionId;
	add_sta_self_req->type = pAddStaReq->type;
	add_sta_self_req->sub_type = pAddStaReq->subType;
	add_sta_self_req->nss_2g = nss_2g;
	add_sta_self_req->nss_5g = nss_5g;

	msg.type = WMA_ADD_STA_SELF_REQ;
	msg.reserved = 0;
	msg.bodyptr = add_sta_self_req;
	msg.bodyval = 0;

	sms_log(pMac, LOG1,
		 FL
		 ("Send WMA_ADD_STA_SELF_REQ for selfMac=" MAC_ADDRESS_STR),
		 MAC_ADDR_ARRAY(add_sta_self_req->self_mac_addr));
	status = wma_post_ctrl_msg(pMac, &msg);

	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGP, FL("wma_post_ctrl_msg failed"));
		qdf_mem_free(add_sta_self_req);
		add_sta_self_req = NULL;
	}
	return status;
}

QDF_STATUS csr_roam_open_session(tpAniSirGlobal pMac,
				 csr_roam_completeCallback callback,
				 void *pContext,
				 uint8_t *pSelfMacAddr, uint8_t *pbSessionId,
				 uint32_t type, uint32_t subType)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i, value = 0;
	union {
		uint16_t nCfgValue16;
		tSirMacHTCapabilityInfo htCapInfo;
	} uHTCapabilityInfo;
	uint32_t nCfgValue = 0;
	tCsrRoamSession *pSession;
	*pbSessionId = CSR_SESSION_ID_INVALID;

	for (i = 0; i < pMac->sme.max_intf_count; i++) {
		sms_log(pMac, LOG1, FL("session:%d active:%d"), i,
			pMac->roam.roamSession[i].sessionActive);
		if (!CSR_IS_SESSION_VALID(pMac, i)) {
			pSession = CSR_GET_SESSION(pMac, i);
			if (!pSession) {
				sms_log(pMac, LOGE,
					FL
						("Session does not exist for interface %d"),
					i);
				break;
			}
			status = QDF_STATUS_SUCCESS;
			pSession->sessionActive = true;
			pSession->sessionId = (uint8_t) i;

			/* Initialize FT related data structures only in STA mode */
			sme_ft_open(pMac, pSession->sessionId);

			pSession->callback = callback;
			pSession->pContext = pContext;
			qdf_mem_copy(&pSession->selfMacAddr, pSelfMacAddr,
				     sizeof(struct qdf_mac_addr));
			*pbSessionId = (uint8_t) i;
			status =
				qdf_mc_timer_init(&pSession->hTimerRoaming,
						  QDF_TIMER_TYPE_SW,
						  csr_roam_roaming_timer_handler,
						  &pSession->roamingTimerInfo);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL
						("cannot allocate memory for Roaming timer"));
				break;
			}
			/* get the HT capability info */
			if (wlan_cfg_get_int(pMac, WNI_CFG_HT_CAP_INFO, &value)
					!= eSIR_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_QDF,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: could not get HT capability info",
					  __func__);
				break;
			}
#ifdef FEATURE_WLAN_BTAMP_UT_RF
			status = qdf_mc_timer_init(&pSession->hTimerJoinRetry,
						   QDF_TIMER_TYPE_SW,
						   csr_roam_join_retry_timer_handler,
						   &pSession->
						   joinRetryTimerInfo);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL
						("cannot allocate memory for join retry timer"));
				break;
			}
#endif
			uHTCapabilityInfo.nCfgValue16 = 0xFFFF & value;
			pSession->htConfig.ht_rx_ldpc =
				uHTCapabilityInfo.htCapInfo.advCodingCap;
			pSession->htConfig.ht_tx_stbc =
				uHTCapabilityInfo.htCapInfo.txSTBC;
			pSession->htConfig.ht_rx_stbc =
				uHTCapabilityInfo.htCapInfo.rxSTBC;
			pSession->htConfig.ht_sgi20 =
				uHTCapabilityInfo.htCapInfo.shortGI20MHz;
			pSession->htConfig.ht_sgi40 =
				uHTCapabilityInfo.htCapInfo.shortGI40MHz;

			wlan_cfg_get_int(pMac, WNI_CFG_VHT_MAX_MPDU_LENGTH,
					 &nCfgValue);
			pSession->vht_config.max_mpdu_len = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac,
					 WNI_CFG_VHT_SUPPORTED_CHAN_WIDTH_SET,
					 &nCfgValue);
			pSession->vht_config.supported_channel_widthset =
								nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_LDPC_CODING_CAP,
					 &nCfgValue);
			pSession->vht_config.ldpc_coding = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_SHORT_GI_80MHZ,
					 &nCfgValue);
			pSession->vht_config.shortgi80 = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ,
				&nCfgValue);
			pSession->vht_config.shortgi160and80plus80 = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_TXSTBC, &nCfgValue);
			pSession->vht_config.tx_stbc = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_RXSTBC, &nCfgValue);
			pSession->vht_config.rx_stbc = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_SU_BEAMFORMER_CAP,
						&nCfgValue);
			pSession->vht_config.su_beam_former = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_SU_BEAMFORMEE_CAP,
						&nCfgValue);
			pSession->vht_config.su_beam_formee = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_CSN_BEAMFORMEE_ANT_SUPPORTED,
				&nCfgValue);
			pSession->vht_config.csnof_beamformer_antSup =
								nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac,
					WNI_CFG_VHT_NUM_SOUNDING_DIMENSIONS,
					&nCfgValue);
			pSession->vht_config.num_soundingdim = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_MU_BEAMFORMER_CAP,
						&nCfgValue);
			pSession->vht_config.mu_beam_former = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_MU_BEAMFORMEE_CAP,
						&nCfgValue);
			pSession->vht_config.mu_beam_formee = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_TXOP_PS,
					 &nCfgValue);
			pSession->vht_config.vht_txops = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_HTC_VHTC_CAP,
					 &nCfgValue);
			pSession->vht_config.htc_vhtcap = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_RX_ANT_PATTERN,
					 &nCfgValue);
			pSession->vht_config.rx_antpattern = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_TX_ANT_PATTERN,
					 &nCfgValue);
			pSession->vht_config.tx_antpattern = nCfgValue;

			nCfgValue = 0;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_AMPDU_LEN_EXPONENT,
					 &nCfgValue);
			pSession->vht_config.max_ampdu_lenexp = nCfgValue;

			status = csr_issue_add_sta_for_session_req(pMac, i,
								pSelfMacAddr,
								type, subType);
			break;
		}
	}
	if (pMac->sme.max_intf_count == i) {
		/* No session is available */
		sms_log(pMac, LOGE,
			"%s: Reached max interfaces: %d! Session creation will fail",
			__func__, pMac->sme.max_intf_count);
		status = QDF_STATUS_E_RESOURCES;
	}
	return status;
}

QDF_STATUS csr_process_del_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	struct del_sta_self_params *rsp;
	uint8_t sessionId;

	if (pMsg == NULL) {
		sms_log(pMac, LOGE, FL("msg ptr is NULL"));
		return status;
	}
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (!pEntry) {
		sms_log(pMac, LOGE, FL("NO commands are ACTIVE ..."));
		return status;
	}
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	sessionId = pCommand->sessionId;
	if (eSmeCommandDelStaSession != pCommand->command) {
		sms_log(pMac, LOGE, FL("NO Del sta session command ACTIVE"));
		return status;
	}
	rsp = (struct del_sta_self_params *) pMsg;
	sms_log(pMac, LOG1, FL("Del Sta rsp status = %d"), rsp->status);
	/* This session is done. */
	csr_cleanup_session(pMac, sessionId);
	if (pCommand->u.delStaSessionCmd.callback) {
		status = sme_release_global_lock(&pMac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sms_log(pMac, LOG1, FL("Failed to Release Lock"));
		else {
			pCommand->u.delStaSessionCmd.
				callback(pCommand->u.delStaSessionCmd.pContext);
			status = sme_acquire_global_lock(&pMac->sme);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOG1, FL("Failed to get Lock"));
				return status;
			}
		}
	}
	/* Remove this command out of the active list */
	if (csr_ll_remove_entry(&pMac->sme.smeCmdActiveList, pEntry,
		LL_ACCESS_LOCK)) {
		/* Now put this command back on the avilable command list */
		csr_release_command(pMac, pCommand);
	}
	sme_process_pending_queue(pMac);
	status = QDF_STATUS_SUCCESS;
	return status;
}


QDF_STATUS csr_issue_del_sta_for_session_req(tpAniSirGlobal pMac, uint32_t sessionId,
					     tSirMacAddr sessionMacAddr,
					     csr_roamSessionCloseCallback callback,
					     void *pContext)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand;
	pCommand = csr_get_command_buffer(pMac);
	if (NULL == pCommand) {
		status = QDF_STATUS_E_RESOURCES;
	} else {
		pCommand->command = eSmeCommandDelStaSession;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.delStaSessionCmd.callback = callback;
		pCommand->u.delStaSessionCmd.pContext = pContext;
		qdf_mem_copy(pCommand->u.delStaSessionCmd.selfMacAddr,
			     sessionMacAddr, sizeof(tSirMacAddr));
		status = csr_queue_sme_command(pMac, pCommand, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			/* Should be panic?? */
			sms_log(pMac, LOGE,
				FL(" fail to send message status = %d"), status);
		}
	}
	return status;
}

QDF_STATUS csr_process_del_sta_session_command(tpAniSirGlobal pMac,
					       tSmeCmd *pCommand)
{
	struct del_sta_self_params *del_sta_self_req;
	tSirMsgQ msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	del_sta_self_req = qdf_mem_malloc(sizeof(struct del_sta_self_params));
	if (NULL == del_sta_self_req) {
		sms_log(pMac, LOGP,
			FL(" mem alloc failed for tDelStaSelfParams"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(del_sta_self_req->self_mac_addr,
		pCommand->u.delStaSessionCmd.selfMacAddr,
		sizeof(tSirMacAddr));

	del_sta_self_req->session_id = pCommand->sessionId;
	msg.type = WMA_DEL_STA_SELF_REQ;
	msg.reserved = 0;
	msg.bodyptr = del_sta_self_req;
	msg.bodyval = 0;

	sms_log(pMac, LOG1,
		FL("sending WMA_DEL_STA_SELF_REQ"));
	status = wma_post_ctrl_msg(pMac, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGP, FL("wma_post_ctrl_msg failed"));
		qdf_mem_free(del_sta_self_req);
	}
	return status;
}

static void purge_csr_session_cmd_list(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tDblLinkList *pList = &pMac->roam.roamCmdPendingList;
	tListElem *pEntry, *pNext;
	tSmeCmd *pCommand;
	tDblLinkList localList;

	qdf_mem_zero(&localList, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(pMac->hHdd, &localList))) {
		sms_log(pMac, LOGE, FL(" failed to open list"));
		return;
	}
	csr_ll_lock(pList);
	pEntry = csr_ll_peek_head(pList, LL_ACCESS_NOLOCK);
	while (pEntry != NULL) {
		pNext = csr_ll_next(pList, pEntry, LL_ACCESS_NOLOCK);
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (pCommand->sessionId == sessionId) {
			if (csr_ll_remove_entry(pList, pEntry, LL_ACCESS_NOLOCK)) {
				csr_ll_insert_tail(&localList, pEntry,
						   LL_ACCESS_NOLOCK);
			}
		}
		pEntry = pNext;
	}
	csr_ll_unlock(pList);

	while ((pEntry = csr_ll_remove_head(&localList, LL_ACCESS_NOLOCK))) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		csr_abort_command(pMac, pCommand, true);
	}
	csr_ll_close(&localList);
}

void csr_cleanup_session(tpAniSirGlobal pMac, uint32_t sessionId)
{
	if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
		tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

		csr_roam_stop(pMac, sessionId);

		/* Clean up FT related data structures */
		sme_ft_close(pMac, sessionId);
		csr_free_connect_bss_desc(pMac, sessionId);
		csr_roam_free_connect_profile(&pSession->connectedProfile);
		csr_roam_free_connected_info(pMac, &pSession->connectedInfo);
		qdf_mc_timer_destroy(&pSession->hTimerRoaming);
#ifdef FEATURE_WLAN_BTAMP_UT_RF
		qdf_mc_timer_destroy(&pSession->hTimerJoinRetry);
#endif
		purge_sme_session_cmd_list(pMac, sessionId,
					   &pMac->sme.smeCmdPendingList);
		purge_sme_session_cmd_list(pMac, sessionId,
						   &pMac->sme.
						   smeScanCmdPendingList);

		purge_csr_session_cmd_list(pMac, sessionId);
		csr_init_session(pMac, sessionId);
	}
}

QDF_STATUS csr_roam_close_session(tpAniSirGlobal pMac, uint32_t sessionId,
				  bool fSync,
				  csr_roamSessionCloseCallback callback,
				  void *pContext)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
		tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
		if (fSync) {
			csr_cleanup_session(pMac, sessionId);
		} else {
			purge_sme_session_cmd_list(pMac, sessionId,
						   &pMac->sme.smeCmdPendingList);
			purge_sme_session_cmd_list(pMac, sessionId,
					   &pMac->sme.smeScanCmdPendingList);

			purge_csr_session_cmd_list(pMac, sessionId);
			status = csr_issue_del_sta_for_session_req(pMac,
						 sessionId,
						 pSession->selfMacAddr.bytes,
						 callback, pContext);
		}
	} else {
		status = QDF_STATUS_E_INVAL;
	}
	return status;
}

static void csr_init_session(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}

	pSession->sessionActive = false;
	pSession->sessionId = CSR_SESSION_ID_INVALID;
	pSession->callback = NULL;
	pSession->pContext = NULL;
	pSession->connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
	csr_free_roam_profile(pMac, sessionId);
	csr_roam_free_connect_profile(&pSession->connectedProfile);
	csr_roam_free_connected_info(pMac, &pSession->connectedInfo);
	csr_free_connect_bss_desc(pMac, sessionId);
	csr_scan_enable(pMac);
	qdf_mem_set(&pSession->selfMacAddr, sizeof(struct qdf_mac_addr), 0);
	if (pSession->pWpaRsnReqIE) {
		qdf_mem_free(pSession->pWpaRsnReqIE);
		pSession->pWpaRsnReqIE = NULL;
	}
	pSession->nWpaRsnReqIeLength = 0;
	if (pSession->pWpaRsnRspIE) {
		qdf_mem_free(pSession->pWpaRsnRspIE);
		pSession->pWpaRsnRspIE = NULL;
	}
	pSession->nWpaRsnRspIeLength = 0;
#ifdef FEATURE_WLAN_WAPI
	if (pSession->pWapiReqIE) {
		qdf_mem_free(pSession->pWapiReqIE);
		pSession->pWapiReqIE = NULL;
	}
	pSession->nWapiReqIeLength = 0;
	if (pSession->pWapiRspIE) {
		qdf_mem_free(pSession->pWapiRspIE);
		pSession->pWapiRspIE = NULL;
	}
	pSession->nWapiRspIeLength = 0;
#endif /* FEATURE_WLAN_WAPI */
	if (pSession->pAddIEScan) {
		qdf_mem_free(pSession->pAddIEScan);
		pSession->pAddIEScan = NULL;
	}
	pSession->nAddIEScanLength = 0;
	if (pSession->pAddIEAssoc) {
		qdf_mem_free(pSession->pAddIEAssoc);
		pSession->pAddIEAssoc = NULL;
	}
	pSession->nAddIEAssocLength = 0;
}

QDF_STATUS csr_roam_get_session_id_from_bssid(tpAniSirGlobal pMac,
					      struct qdf_mac_addr *bssid,
					      uint32_t *pSessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t i;
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		if (CSR_IS_SESSION_VALID(pMac, i)) {
			if (qdf_is_macaddr_equal(bssid,
				    &pMac->roam.roamSession[i].connectedProfile.
				    bssid)) {
				/* Found it */
				status = QDF_STATUS_SUCCESS;
				*pSessionId = i;
				break;
			}
		}
	}
	return status;
}

/* This function assumes that we only support one IBSS session. We cannot use BSSID to identify */
/* session because for IBSS, the bssid changes. */
static uint32_t csr_find_ibss_session(tpAniSirGlobal pMac)
{
	uint32_t i, nRet = CSR_SESSION_ID_INVALID;
	tCsrRoamSession *pSession;
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		if (CSR_IS_SESSION_VALID(pMac, i)) {
			pSession = CSR_GET_SESSION(pMac, i);
			if (pSession->pCurRoamProfile
			    &&
			    (csr_is_bss_type_ibss
				     (pSession->connectedProfile.BSSType))) {
				/* Found it */
				nRet = i;
				break;
			}
		}
	}
	return nRet;
}

static void csr_roam_link_up(tpAniSirGlobal pMac, struct qdf_mac_addr bssid)
{
	uint32_t sessionId = 0;

	/*
	 * Update the current BSS info in ho control block based on connected
	 * profile info from pmac global structure
	 */

	sms_log(pMac, LOGW,
		" csr_roam_link_up: WLAN link UP with AP= " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(bssid.bytes));
	/* Check for user misconfig of RSSI trigger threshold */
	pMac->roam.configParam.vccRssiThreshold =
		(0 == pMac->roam.configParam.vccRssiThreshold) ?
		CSR_VCC_RSSI_THRESHOLD :
		pMac->roam.configParam.vccRssiThreshold;
	pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
	/* Check for user misconfig of UL MAC Loss trigger threshold */
	pMac->roam.configParam.vccUlMacLossThreshold =
		(0 == pMac->roam.configParam.vccUlMacLossThreshold) ?
		CSR_VCC_UL_MAC_LOSS_THRESHOLD : pMac->roam.configParam.
		vccUlMacLossThreshold;
	/* Indicate the neighbor roal algorithm about the connect indication */
	csr_roam_get_session_id_from_bssid(pMac, &bssid,
					   &sessionId);
	csr_neighbor_roam_indicate_connect(pMac, sessionId,
					   QDF_STATUS_SUCCESS);
}

static void csr_roam_link_down(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "), sessionId);
		return;
	}
	/* Only to handle the case for Handover on infra link */
	if (eCSR_BSS_TYPE_INFRASTRUCTURE != pSession->connectedProfile.BSSType) {
		return;
	}
	/*
	 * Incase of station mode, immediately stop data transmission whenever
	 * link down is detected.
	 */
	if (csr_roam_is_sta_mode(pMac, sessionId)
	    && !CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac, sessionId)
	    && !csr_roam_is11r_assoc(pMac, sessionId)) {
		sms_log(pMac, LOG1, FL("Inform Link lost for session %d"),
			sessionId);
		csr_roam_call_callback(pMac, sessionId, NULL, 0,
				       eCSR_ROAM_LOSTLINK,
				       eCSR_ROAM_RESULT_LOSTLINK);
	}
	/* deregister the clients requesting stats from PE/TL & also stop the corresponding timers */
	csr_roam_dereg_statistics_req(pMac);
	pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
	/* Indicate the neighbor roal algorithm about the disconnect indication */
	csr_neighbor_roam_indicate_disconnect(pMac, sessionId);

	/* Remove this code once SLM_Sessionization is supported */
	/* BMPS_WORKAROUND_NOT_NEEDED */
	if (!IS_FEATURE_SUPPORTED_BY_FW(SLM_SESSIONIZATION) &&
	    csr_is_infra_ap_started(pMac) &&
	    pMac->roam.configParam.doBMPSWorkaround) {
		pMac->roam.configParam.doBMPSWorkaround = 0;
	}

}

void csr_roam_tl_stats_timer_handler(void *pv)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(pv);
	QDF_STATUS status;
	pMac->roam.tlStatsReqInfo.timerRunning = false;

	sms_log(pMac, LOG1,
		FL
			(" TL stat timer is no-op. It needs to support multiple stations"));

	if (!pMac->roam.tlStatsReqInfo.timerRunning) {
		if (pMac->roam.tlStatsReqInfo.periodicity) {
			/* start timer */
			status =
				qdf_mc_timer_start(&pMac->roam.tlStatsReqInfo.
						   hTlStatsTimer,
						   pMac->roam.tlStatsReqInfo.
						   periodicity);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_tl_stats_timer_handler:cannot start TlStatsTimer timer"));
				return;
			}
			pMac->roam.tlStatsReqInfo.timerRunning = true;
		}
	}
}

void csr_roam_pe_stats_timer_handler(void *pv)
{
	tCsrPeStatsReqInfo *pPeStatsReqListEntry = (tCsrPeStatsReqInfo *) pv;
	QDF_STATUS status;
	tpAniSirGlobal pMac = pPeStatsReqListEntry->pMac;
	QDF_STATUS qdf_status;
	pPeStatsReqListEntry->timerRunning = false;
	if (pPeStatsReqListEntry->timerStopFailed == true) {
		/* If we entered here, meaning the timer could not be successfully */
		/* stopped in csr_roam_remove_entry_from_pe_stats_req_list(). So do it here. */

		/* Destroy the timer */
		qdf_status =
			qdf_mc_timer_destroy(&pPeStatsReqListEntry->hPeStatsTimer);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			sms_log(pMac, LOGE,
				FL
					("csr_roam_pe_stats_timer_handler:failed to destroy hPeStatsTimer timer"));
		}
		/* Free the entry */
		qdf_mem_free(pPeStatsReqListEntry);
		pPeStatsReqListEntry = NULL;
	} else {
		if (!pPeStatsReqListEntry->rspPending) {
			status =
				csr_send_mb_stats_req_msg(pMac,
							  pPeStatsReqListEntry->
							  statsMask & ~(1 <<
									eCsrGlobalClassDStats),
							  pPeStatsReqListEntry->staId,
							  pPeStatsReqListEntry->
							  sessionId);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_pe_stats_timer_handler:failed to send down stats req to PE"));
			} else {
				pPeStatsReqListEntry->rspPending = true;
			}
		}
		/* send down a req */
		if (pPeStatsReqListEntry->periodicity &&
		    (QDF_TIMER_STATE_STOPPED ==
		     qdf_mc_timer_get_current_state(&pPeStatsReqListEntry->
						    hPeStatsTimer))) {
			if (pPeStatsReqListEntry->periodicity <
					pMac->roam.configParam.
					statsReqPeriodicityInPS) {
				pPeStatsReqListEntry->periodicity =
					pMac->roam.configParam.
					statsReqPeriodicityInPS;
			}
			/* start timer */
			qdf_status =
				qdf_mc_timer_start(&pPeStatsReqListEntry->
						   hPeStatsTimer,
						   pPeStatsReqListEntry->
						   periodicity);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_pe_stats_timer_handler:cannot start hPeStatsTimer timer"));
				return;
			}
			pPeStatsReqListEntry->timerRunning = true;

		}

	}
}

void csr_roam_stats_client_timer_handler(void *pv)
{
	tCsrStatsClientReqInfo *pStaEntry = (tCsrStatsClientReqInfo *) pv;
	if (QDF_TIMER_STATE_STOPPED ==
	    qdf_mc_timer_get_current_state(&pStaEntry->timer)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("roam stats client timer is stopped"));
	}
}

QDF_STATUS csr_send_mb_stats_req_msg(tpAniSirGlobal pMac, uint32_t statsMask,
				     uint8_t staId, uint8_t sessionId)
{
	tAniGetPEStatsReq *pMsg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	pMsg = qdf_mem_malloc(sizeof(tAniGetPEStatsReq));
	if (NULL == pMsg) {
		sms_log(pMac, LOGE, FL("Failed to allocate mem for stats req "));
		return QDF_STATUS_E_NOMEM;
	}
	/* need to initiate a stats request to PE */
	pMsg->msgType = eWNI_SME_GET_STATISTICS_REQ;
	pMsg->msgLen = (uint16_t) sizeof(tAniGetPEStatsReq);
	pMsg->staId = staId;
	pMsg->statsMask = statsMask;
	pMsg->sessionId = sessionId;
	status = cds_send_mb_message_to_mac(pMsg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOG1, FL("Failed to send down the stats req "));
	}
	return status;
}

/**
 * csr_update_stats() - updates correct stats struct in mac_ctx
 * @mac:             mac global context
 * @stats_type:      stats type
 * @sme_stats_rsp:   stats rsp msg packet
 * @stats:           input stats data buffer to fill in mac_ctx struct
 * @length:          out param - stats length
 *
 * This function fills corresponding stats struct in mac_cts based on stats type
 * passed
 *
 * Return: void
 */
static void
csr_update_stats(tpAniSirGlobal mac, uint8_t stats_type,
		 tAniGetPEStatsRsp *sme_stats_rsp,
		 uint8_t **stats, uint32_t *length)
{
	switch (stats_type) {
	case eCsrSummaryStats:
		sms_log(mac, LOG2, FL("summary stats"));
		qdf_mem_copy((uint8_t *) &mac->roam.summaryStatsInfo, *stats,
			     sizeof(tCsrSummaryStatsInfo));
		*stats += sizeof(tCsrSummaryStatsInfo);
		*length -= sizeof(tCsrSummaryStatsInfo);
		break;
	case eCsrGlobalClassAStats:
		sms_log(mac, LOG2, FL("ClassA stats"));
		qdf_mem_copy((uint8_t *) &mac->roam.classAStatsInfo, *stats,
			     sizeof(tCsrGlobalClassAStatsInfo));
		*stats += sizeof(tCsrGlobalClassAStatsInfo);
		*length -= sizeof(tCsrGlobalClassAStatsInfo);
		break;
	case eCsrGlobalClassBStats:
		sms_log(mac, LOG2, FL("ClassB stats"));
		qdf_mem_copy((uint8_t *) &mac->roam.classBStatsInfo, *stats,
			     sizeof(tCsrGlobalClassBStatsInfo));
		*stats += sizeof(tCsrGlobalClassBStatsInfo);
		*length -= sizeof(tCsrGlobalClassBStatsInfo);
		break;
	case eCsrGlobalClassCStats:
		sms_log(mac, LOG2, FL("ClassC stats"));
		qdf_mem_copy((uint8_t *) &mac->roam.classCStatsInfo, *stats,
			     sizeof(tCsrGlobalClassCStatsInfo));
		*stats += sizeof(tCsrGlobalClassCStatsInfo);
		*length -= sizeof(tCsrGlobalClassCStatsInfo);
		break;
	case eCsrPerStaStats:
		sms_log(mac, LOG2, FL("PerSta stats"));
		if (CSR_MAX_STA > sme_stats_rsp->staId) {
			qdf_mem_copy(
				&mac->roam.perStaStatsInfo[sme_stats_rsp->staId],
				*stats, sizeof(tCsrPerStaStatsInfo));
		} else {
			sms_log(mac, LOGE, FL("out bound staId:%d. failed to copy PerSta stats"),
				sme_stats_rsp->staId);
			QDF_ASSERT(0);
		}
		*stats += sizeof(tCsrPerStaStatsInfo);
		*length -= sizeof(tCsrPerStaStatsInfo);
		break;
	case csr_per_chain_rssi_stats:
		sms_log(mac, LOG2,
			FL("csrRoamStatsRspProcessor:Per Chain RSSI stats"));
		qdf_mem_copy((uint8_t *)&mac->roam.per_chain_rssi_stats,
			*stats, sizeof(struct csr_per_chain_rssi_stats_info));
		*stats += sizeof(struct csr_per_chain_rssi_stats_info);
		*length -= sizeof(struct csr_per_chain_rssi_stats_info);
		break;
	default:
		sms_log(mac, LOGW, FL("unknown stats type"));
		break;
	}
}

/**
 * csr_roam_stats_rsp_processor() - processes stats rsp msg
 * @pMac             mac global context
 * @pSirMsg:         incoming message
 *
 * Return: void
 */
void csr_roam_stats_rsp_processor(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg)
{
	tAniGetPEStatsRsp *pSmeStatsRsp;
	tListElem *pEntry = NULL;
	tCsrStatsClientReqInfo *pTempStaEntry = NULL;
	tCsrPeStatsReqInfo *pPeStaEntry = NULL;
	uint32_t tempMask = 0;
	uint8_t counter = 0;
	uint8_t *pStats = NULL;
	uint32_t length = 0;
	void *p_cds_gctx;
	int8_t rssi = 0, snr = 0;
	uint32_t *pRssi = NULL, *pSnr = NULL;
	uint32_t linkCapacity;
	pSmeStatsRsp = (tAniGetPEStatsRsp *) pSirMsg;

	if (pSmeStatsRsp->rc) {
		sms_log(pMac, LOGW, FL("stats rsp from PE shows failure"));
		goto post_update;
	}
	tempMask = pSmeStatsRsp->statsMask;
	pStats = ((uint8_t *) &pSmeStatsRsp->statsMask) +
		sizeof(pSmeStatsRsp->statsMask);
	/*
	 * subtract all statistics from this length, and after processing the
	 * entire 'stat' part of the message, if the length is not zero, then
	 * rssi is piggy packed in this 'stats' message.
	 */
	length = pSmeStatsRsp->msgLen - sizeof(tAniGetPEStatsRsp);
	/* new stats info from PE, fill up the stats strucutres in PMAC */
	while (tempMask) {
		if (tempMask & 1) {
			csr_update_stats(pMac, counter, pSmeStatsRsp,
					 &pStats, &length);
		}
		tempMask >>= 1;
		counter++;
	}
	p_cds_gctx = cds_get_global_context();
	if (length != 0) {
		pRssi = (uint32_t *) pStats;
		rssi = (int8_t) *pRssi;
		pStats += sizeof(uint32_t);
		length -= sizeof(uint32_t);
	} else {
		/* If riva is not sending rssi, continue to use the hack */
		rssi = RSSI_HACK_BMPS;
	}

	if (length != 0) {
		linkCapacity = *(uint32_t *) pStats;
		pStats += sizeof(uint32_t);
		length -= sizeof(uint32_t);
	} else {
		linkCapacity = 0;
	}

	if (length != 0) {
		pSnr = (uint32_t *) pStats;
		snr = (int8_t) *pSnr;
	} else {
		snr = SNR_HACK_BMPS;
	}

post_update:
	/* make sure to update the pe stats req list */
	pEntry = csr_roam_find_in_pe_stats_req_list(pMac, pSmeStatsRsp->statsMask);
	if (pEntry) {
		pPeStaEntry = GET_BASE_ADDR(pEntry, tCsrPeStatsReqInfo, link);
		pPeStaEntry->rspPending = false;

	}
	/* check the one timer cases */
	pEntry = csr_roam_check_client_req_list(pMac, pSmeStatsRsp->statsMask);
	if (pEntry) {
		pTempStaEntry =
			GET_BASE_ADDR(pEntry, tCsrStatsClientReqInfo, link);
		if (pTempStaEntry->timerExpired) {
			/* send up the stats report */
			csr_roam_report_statistics(pMac, pTempStaEntry->statsMask,
						   pTempStaEntry->callback,
						   pTempStaEntry->staId,
						   pTempStaEntry->pContext);
			/* also remove from the client list */
			csr_roam_remove_stat_list_entry(pMac, pEntry);
			pTempStaEntry = NULL;
		}
	}
}

tListElem *csr_roam_find_in_pe_stats_req_list(tpAniSirGlobal pMac, uint32_t statsMask)
{
	tListElem *pEntry = NULL;
	tCsrPeStatsReqInfo *pTempStaEntry = NULL;
	pEntry = csr_ll_peek_head(&pMac->roam.peStatsReqList, LL_ACCESS_LOCK);
	if (!pEntry) {
		/* list empty */
		sms_log(pMac, LOG2,
			"csr_roam_find_in_pe_stats_req_list: List empty, no request to PE");
		return NULL;
	}
	while (pEntry) {
		pTempStaEntry = GET_BASE_ADDR(pEntry, tCsrPeStatsReqInfo, link);
		if (pTempStaEntry->statsMask == statsMask) {
			sms_log(pMac, LOG3,
				"csr_roam_find_in_pe_stats_req_list: match found");
			break;
		}
		pEntry =
			csr_ll_next(&pMac->roam.peStatsReqList, pEntry,
				    LL_ACCESS_NOLOCK);
	}
	return pEntry;
}

tListElem *csr_roam_checkn_update_client_req_list(tpAniSirGlobal pMac,
						  tCsrStatsClientReqInfo *pStaEntry,
						  bool update)
{
	tListElem *pEntry;
	tCsrStatsClientReqInfo *pTempStaEntry;
	pEntry = csr_ll_peek_head(&pMac->roam.statsClientReqList, LL_ACCESS_LOCK);
	if (!pEntry) {
		/* list empty */
		sms_log(pMac, LOG2,
			"csr_roam_checkn_update_client_req_list: List empty, no request from "
			"upper layer client(s)");
		return NULL;
	}
	while (pEntry) {
		pTempStaEntry =
			GET_BASE_ADDR(pEntry, tCsrStatsClientReqInfo, link);
		if ((pTempStaEntry->requesterId == pStaEntry->requesterId)
		    && (pTempStaEntry->statsMask == pStaEntry->statsMask)) {
			sms_log(pMac, LOG3,
				"csr_roam_checkn_update_client_req_list: match found");
			if (update) {
				pTempStaEntry->periodicity =
					pStaEntry->periodicity;
				pTempStaEntry->callback = pStaEntry->callback;
				pTempStaEntry->pContext = pStaEntry->pContext;
			}
			break;
		}
		pEntry =
			csr_ll_next(&pMac->roam.statsClientReqList, pEntry,
				    LL_ACCESS_NOLOCK);
	}
	return pEntry;
}

tListElem *csr_roam_check_client_req_list(tpAniSirGlobal pMac, uint32_t statsMask)
{
	tListElem *pEntry;
	tCsrStatsClientReqInfo *pTempStaEntry;
	pEntry = csr_ll_peek_head(&pMac->roam.statsClientReqList, LL_ACCESS_LOCK);
	if (!pEntry) {
		/* list empty */
		sms_log(pMac, LOG2,
			"csr_roam_check_client_req_list: List empty, no request from "
			"upper layer client(s)");
		return NULL;
	}
	while (pEntry) {
		pTempStaEntry =
			GET_BASE_ADDR(pEntry, tCsrStatsClientReqInfo, link);
		if ((pTempStaEntry->
		     statsMask & ~(1 << eCsrGlobalClassDStats)) == statsMask) {
			sms_log(pMac, LOG3,
				"csr_roam_check_client_req_list: match found");
			break;
		}
		pEntry =
			csr_ll_next(&pMac->roam.statsClientReqList, pEntry,
				    LL_ACCESS_NOLOCK);
	}
	return pEntry;
}

void csr_roam_vcc_trigger(tpAniSirGlobal pMac)
{
	eCsrRoamLinkQualityInd newVccLinkQuality;
	uint32_t ul_mac_loss = 0;
	uint32_t ul_mac_loss_trigger_threshold;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/*-------------------------------------------------------------------------
	   Link quality is currently binary based on OBIWAN recommended triggers
	   Check for a change in link quality and notify client if necessary
	   -------------------------------------------------------------------------*/
	ul_mac_loss_trigger_threshold =
		pMac->roam.configParam.vccUlMacLossThreshold;
	if (0 == ul_mac_loss_trigger_threshold) {
		QDF_ASSERT(ul_mac_loss_trigger_threshold != 0);
		return;
	}
	sms_log(pMac, LOGW, "csr_roam_vcc_trigger: UL_MAC_LOSS_THRESHOLD is %d",
		ul_mac_loss_trigger_threshold);
	if (ul_mac_loss_trigger_threshold < ul_mac_loss) {
		sms_log(pMac, LOGW, "csr_roam_vcc_trigger: link quality is POOR ");
		newVccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
	} else {
		sms_log(pMac, LOGW, "csr_roam_vcc_trigger: link quality is GOOD");
		newVccLinkQuality = eCSR_ROAM_LINK_QUAL_GOOD_IND;
	}
	sms_log(pMac, LOGW,
		"csr_roam_vcc_trigger: link qual : *** UL_MAC_LOSS %d *** ",
		ul_mac_loss);
	if (newVccLinkQuality != pMac->roam.vccLinkQuality) {
		sms_log(pMac, LOGW,
			"csr_roam_vcc_trigger: link quality changed: trigger necessary");
		if (NULL != pMac->roam.linkQualityIndInfo.callback) {
			sms_log(pMac, LOGW,
				"csr_roam_vcc_trigger: link quality indication %d",
				newVccLinkQuality);

			/* we now invoke the callback once to notify client of initial value   */
			pMac->roam.linkQualityIndInfo.
			callback(newVccLinkQuality,
				 pMac->roam.linkQualityIndInfo.context);
			/* event: EVENT_WLAN_VCC */
		}
	}
	pMac->roam.vccLinkQuality = newVccLinkQuality;

}

tCsrStatsClientReqInfo *csr_roam_insert_entry_into_list(tpAniSirGlobal pMac,
							tDblLinkList *pStaList,
							tCsrStatsClientReqInfo *
							pStaEntry)
{
	tCsrStatsClientReqInfo *pNewStaEntry = NULL;
	/* if same entity requested for same set of stats with different periodicity & */
	/* callback update it */
	if (NULL == csr_roam_checkn_update_client_req_list(pMac, pStaEntry, true)) {

		pNewStaEntry = qdf_mem_malloc(sizeof(tCsrStatsClientReqInfo));
		if (NULL == pNewStaEntry) {
			sms_log(pMac, LOGW,
				"csr_roam_insert_entry_into_list: couldn't allocate memory for the "
				"entry");
			return NULL;
		}

		pNewStaEntry->callback = pStaEntry->callback;
		pNewStaEntry->pContext = pStaEntry->pContext;
		pNewStaEntry->periodicity = pStaEntry->periodicity;
		pNewStaEntry->requesterId = pStaEntry->requesterId;
		pNewStaEntry->statsMask = pStaEntry->statsMask;
		pNewStaEntry->pPeStaEntry = pStaEntry->pPeStaEntry;
		pNewStaEntry->pMac = pStaEntry->pMac;
		pNewStaEntry->staId = pStaEntry->staId;
		pNewStaEntry->timerExpired = pStaEntry->timerExpired;

		csr_ll_insert_tail(pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK);
	}
	return pNewStaEntry;
}

tCsrPeStatsReqInfo *csr_roam_insert_entry_into_pe_stats_req_list(tpAniSirGlobal pMac,
								 tDblLinkList *
								 pStaList,
								 tCsrPeStatsReqInfo *
								 pStaEntry)
{
	tCsrPeStatsReqInfo *pNewStaEntry = NULL;
	pNewStaEntry = qdf_mem_malloc(sizeof(tCsrPeStatsReqInfo));
	if (NULL == pNewStaEntry) {
		sms_log(pMac, LOGW,
			"csr_roam_insert_entry_into_pe_stats_req_list: couldn't allocate memory for the "
			"entry");
		return NULL;
	}

	pNewStaEntry->hPeStatsTimer = pStaEntry->hPeStatsTimer;
	pNewStaEntry->numClient = pStaEntry->numClient;
	pNewStaEntry->periodicity = pStaEntry->periodicity;
	pNewStaEntry->statsMask = pStaEntry->statsMask;
	pNewStaEntry->pMac = pStaEntry->pMac;
	pNewStaEntry->staId = pStaEntry->staId;
	pNewStaEntry->timerRunning = pStaEntry->timerRunning;
	pNewStaEntry->rspPending = pStaEntry->rspPending;

	csr_ll_insert_tail(pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK);
	return pNewStaEntry;
}

QDF_STATUS csr_get_rssi(tpAniSirGlobal pMac,
			tCsrRssiCallback callback,
			uint8_t staId,
			struct qdf_mac_addr bssId,
			int8_t lastRSSI, void *pContext, void *p_cds_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	cds_msg_t msg;
	uint32_t sessionId;

	tAniGetRssiReq *pMsg;
	sms_log(pMac, LOG2, FL("called"));

	status = csr_roam_get_session_id_from_bssid(pMac, &bssId, &sessionId);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		callback(lastRSSI, staId, pContext);
		sms_log(pMac, LOGE, FL("Failed to get SessionId"));
		return QDF_STATUS_E_FAILURE;
	}

	pMsg = qdf_mem_malloc(sizeof(tAniGetRssiReq));
	if (NULL == pMsg) {
		sms_log(pMac, LOGE,
			" csr_get_rssi: failed to allocate mem for req ");
		return QDF_STATUS_E_NOMEM;
	}

	pMsg->msgType = eWNI_SME_GET_RSSI_REQ;
	pMsg->msgLen = (uint16_t) sizeof(tAniGetRssiReq);
	pMsg->sessionId = sessionId;
	pMsg->staId = staId;
	pMsg->rssiCallback = callback;
	pMsg->pDevContext = pContext;
	pMsg->p_cds_context = p_cds_context;
	/*
	 * store RSSI at time of calling, so that if RSSI request cannot
	 * be sent to firmware, this value can be used to return immediately
	 */
	pMsg->lastRSSI = lastRSSI;
	msg.type = eWNI_SME_GET_RSSI_REQ;
	msg.bodyptr = pMsg;
	msg.reserved = 0;
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
		sms_log(pMac, LOGE, " csr_get_rssi failed to post msg to self ");
		qdf_mem_free((void *)pMsg);
		status = QDF_STATUS_E_FAILURE;
	}
	sms_log(pMac, LOG2, FL("returned"));
	return status;
}

QDF_STATUS csr_get_snr(tpAniSirGlobal pMac,
		       tCsrSnrCallback callback,
		       uint8_t staId, struct qdf_mac_addr bssId, void *pContext)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	cds_msg_t msg;
	uint32_t sessionId;

	tAniGetSnrReq *pMsg;

	sms_log(pMac, LOG2, FL("called"));

	pMsg = (tAniGetSnrReq *) qdf_mem_malloc(sizeof(tAniGetSnrReq));
	if (NULL == pMsg) {
		sms_log(pMac, LOGE, "%s: failed to allocate mem for req",
			__func__);
		return QDF_STATUS_E_NOMEM;
	}

	csr_roam_get_session_id_from_bssid(pMac, &bssId, &sessionId);

	pMsg->msgType = eWNI_SME_GET_SNR_REQ;
	pMsg->msgLen = (uint16_t) sizeof(tAniGetSnrReq);
	pMsg->sessionId = sessionId;
	pMsg->staId = staId;
	pMsg->snrCallback = callback;
	pMsg->pDevContext = pContext;
	msg.type = eWNI_SME_GET_SNR_REQ;
	msg.bodyptr = pMsg;
	msg.reserved = 0;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
		sms_log(pMac, LOGE, "%s failed to post msg to self", __func__);
		qdf_mem_free((void *)pMsg);
		status = QDF_STATUS_E_FAILURE;
	}

	sms_log(pMac, LOG2, FL("returned"));
	return status;
}

/**
 * csr_deregister_client_request() - deregisters a get stats request
 * @mac_ctx:       mac global context
 * @sta_entry:     stats request entry
 *
 * Return: status of operation
 */
static QDF_STATUS
csr_deregister_client_request(tpAniSirGlobal mac_ctx,
			      tCsrStatsClientReqInfo *sta_entry)
{
	QDF_STATUS status;
	tListElem *entry = NULL;
	tCsrStatsClientReqInfo *ptr_sta_entry = NULL;

	entry = csr_roam_checkn_update_client_req_list(mac_ctx, sta_entry,
						      false);
	if (!entry) {
		sms_log(mac_ctx, LOGW,
			FL("callback is empty in the request & couldn't find any existing request in statsClientReqList"));
		return QDF_STATUS_E_FAILURE;
	}
	/* clean up & return */
	ptr_sta_entry = GET_BASE_ADDR(entry, tCsrStatsClientReqInfo, link);
	if (NULL != ptr_sta_entry->pPeStaEntry) {
		ptr_sta_entry->pPeStaEntry->numClient--;
		/* check if we need to delete the entry from peStatsReqList */
		if (!ptr_sta_entry->pPeStaEntry->numClient)
			csr_roam_remove_entry_from_pe_stats_req_list(mac_ctx,
						ptr_sta_entry->pPeStaEntry);
	}
	/* check if we need to stop the tl stats timer too */
	mac_ctx->roam.tlStatsReqInfo.numClient--;
	if (!mac_ctx->roam.tlStatsReqInfo.numClient) {
		if (mac_ctx->roam.tlStatsReqInfo.timerRunning) {
			status = qdf_mc_timer_stop(
				&mac_ctx->roam.tlStatsReqInfo.hTlStatsTimer);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(mac_ctx, LOGE,
					FL("cannot stop TlStatsTimer timer"));
				return status;
			}
		}
		mac_ctx->roam.tlStatsReqInfo.periodicity = 0;
		mac_ctx->roam.tlStatsReqInfo.timerRunning = false;
	}
	qdf_mc_timer_stop(&ptr_sta_entry->timer);
	/* Destroy the qdf timer... */
	status = qdf_mc_timer_destroy(&ptr_sta_entry->timer);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sms_log(mac_ctx, LOGE,
			FL("failed to destroy Client req timer"));

	csr_roam_remove_stat_list_entry(mac_ctx, entry);
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_insert_stats_request_to_list() - inserts request to existing list
 * @mac_ctx:       mac global context
 * @sta_entry:     stats request entry
 * @periodicity:   periodicity of stats
 *
 * Return: status of operation
 */
static QDF_STATUS
csr_insert_stats_request_to_list(tpAniSirGlobal mac_ctx,
				 tCsrStatsClientReqInfo *sta_entry,
				 uint32_t periodicity)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrStatsClientReqInfo *ptr_sta_entry = csr_roam_insert_entry_into_list(
				mac_ctx, &mac_ctx->roam.statsClientReqList,
				sta_entry);
	if (!ptr_sta_entry) {
		sms_log(mac_ctx, LOGW,
			FL("Failed to insert req in statsClientReqList"));
		return QDF_STATUS_E_FAILURE;
	}
	/* Init & start timer if needed */
	ptr_sta_entry->periodicity = periodicity;
	if (ptr_sta_entry->periodicity) {
		status = qdf_mc_timer_init(&ptr_sta_entry->timer,
					QDF_TIMER_TYPE_SW,
					csr_roam_stats_client_timer_handler,
					ptr_sta_entry);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(mac_ctx, LOGE,
				FL("cannot init StatsClient timer"));
			return QDF_STATUS_E_FAILURE;
		}
		status = qdf_mc_timer_start(&ptr_sta_entry->timer,
					    ptr_sta_entry->periodicity);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(mac_ctx, LOGE,
				FL("cannot start StatsClient timer"));
			return QDF_STATUS_E_FAILURE;
		}
	}
	return status;
}

/**
 * csr_get_statistics_from_tl() - fetch stats from tl layer
 * @mac_ctx:       mac global context
 * @cache:         indicate if cached stats are required
 * @staId:         station id
 * @periodicity:   periodicity of stats
 *
 * Return: status of operation
 */
static QDF_STATUS
csr_get_statistics_from_tl(tpAniSirGlobal mac_ctx,
			   bool cache,
			   uint8_t staId,
			   uint32_t periodicity)
{
	QDF_STATUS status;

	if (cache && mac_ctx->roam.tlStatsReqInfo.numClient) {
		sms_log(mac_ctx, LOGE, FL("Looking for cached stats from TL"));
		mac_ctx->roam.tlStatsReqInfo.numClient++;
		return QDF_STATUS_SUCCESS;
	}

	/* update periodicity */
	if (mac_ctx->roam.tlStatsReqInfo.periodicity)
		mac_ctx->roam.tlStatsReqInfo.periodicity =
		    QDF_MIN(periodicity,
			    mac_ctx->roam.tlStatsReqInfo.periodicity);
	else
		mac_ctx->roam.tlStatsReqInfo.periodicity = periodicity;

	if (mac_ctx->roam.tlStatsReqInfo.periodicity
	    < CSR_MIN_TL_STAT_QUERY_PERIOD) {
		mac_ctx->roam.tlStatsReqInfo.periodicity =
		CSR_MIN_TL_STAT_QUERY_PERIOD;
	}

	if (!mac_ctx->roam.tlStatsReqInfo.timerRunning) {

		if (mac_ctx->roam.tlStatsReqInfo.periodicity) {
			/* start timer */
			status = qdf_mc_timer_start(
				&mac_ctx->roam.tlStatsReqInfo.hTlStatsTimer,
				mac_ctx->roam.tlStatsReqInfo.periodicity);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(mac_ctx, LOGE,
					FL("cannot start TlStatsTimer timer"));
				return QDF_STATUS_E_FAILURE;
			}
			mac_ctx->roam.tlStatsReqInfo.timerRunning = true;
		}
	}
	mac_ctx->roam.tlStatsReqInfo.numClient++;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_get_statistics(tpAniSirGlobal pMac,
			      eCsrStatsRequesterType requesterId,
			      uint32_t statsMask,
			      tCsrStatsCallback callback,
			      uint32_t periodicity,
			      bool cache,
			      uint8_t staId,
			      void *pContext,
			      uint8_t sessionId)
{
	tCsrStatsClientReqInfo staEntry;
	tCsrPeStatsReqInfo *pPeStaEntry = NULL;
	bool found = false;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool insertInClientList = false;
	uint32_t temp_mask = 0;

	if (csr_is_all_session_disconnected(pMac))
		return QDF_STATUS_E_FAILURE;

	if (csr_neighbor_middle_of_roaming(pMac, sessionId)) {
		sms_log(pMac, LOG1, FL("in the middle of roaming states"));
		return QDF_STATUS_E_FAILURE;
	}

	if ((!statsMask) && (!callback)) {
		sms_log(pMac, LOGW,
			FL("statsMask & callback empty in the request"));
		return QDF_STATUS_E_FAILURE;
	}
	/* for the search list method for deregister */
	staEntry.requesterId = requesterId;
	staEntry.statsMask = statsMask;
	/* requester wants to deregister or just an error */
	if ((statsMask) && (!callback))
		return csr_deregister_client_request(pMac, &staEntry);

	if (cache && !periodicity) {
		/* return the cached stats */
		csr_roam_report_statistics(pMac, statsMask, callback, staId,
					   pContext);
		return QDF_STATUS_SUCCESS;
	}
	/* add the request in the client req list */
	staEntry.callback = callback;
	staEntry.pContext = pContext;
	staEntry.periodicity = periodicity;
	staEntry.pPeStaEntry = NULL;
	staEntry.staId = staId;
	staEntry.pMac = pMac;
	staEntry.timerExpired = false;
	staEntry.sessionId = sessionId;

	/* if periodic report requested with non cached result from PE/TL */
	if (periodicity) {
		/* if looking for stats from PE */
		temp_mask = statsMask & ~(1 << eCsrGlobalClassDStats);
		if (temp_mask) {
			/* check if same req made already & waiting for rsp */
			pPeStaEntry = csr_roam_check_pe_stats_req_list(pMac,
					temp_mask, periodicity, &found,
					staId, sessionId);
			if (!pPeStaEntry)
				/* bail out, maxed out on num of req for PE */
				return QDF_STATUS_E_FAILURE;
			else
				staEntry.pPeStaEntry = pPeStaEntry;
		}
		/*
		 * request stats from TL rightaway if requested by client,
		 * update tlStatsReqInfo if needed
		 */
		temp_mask = statsMask & (1 << eCsrGlobalClassDStats);
		if (temp_mask) {
			status = csr_get_statistics_from_tl(pMac, cache, staId,
							    periodicity);
			if (!QDF_IS_STATUS_SUCCESS(status))
				return status;
		}
		insertInClientList = true;
	}
	/* if one time report requested with non cached result from PE/TL */
	else if (!cache && !periodicity) {
		temp_mask = statsMask & ~(1 << eCsrGlobalClassDStats);
		if (temp_mask) {
			/* send down a req */
			status = csr_send_mb_stats_req_msg(pMac,
						temp_mask, staId, sessionId);
			if (!QDF_IS_STATUS_SUCCESS(status))
				sms_log(pMac, LOGE,
					FL("failed to send down stats req"));
			/*
			 * so that when the stats rsp comes back from PE we
			 * respond to upper layer right away
			 */
			staEntry.timerExpired = true;
			insertInClientList = true;
		}
		/* if looking for stats from TL only */
		if (!insertInClientList) {
			/* return the stats */
			csr_roam_report_statistics(pMac, statsMask, callback,
						   staId, pContext);
			return QDF_STATUS_SUCCESS;
		}
	}
	if (insertInClientList)
		return csr_insert_stats_request_to_list(pMac, &staEntry,
							periodicity);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * csr_roam_set_key_mgmt_offload() - enable/disable key mgmt offload
 * @mac_ctx: mac context.
 * @session_id: Session Identifier
 * @roam_key_mgmt_offload_enabled: key mgmt enable/disable flag
 * @okc_enabled: Opportunistic key caching enable/disable flag
 *
 * Return: QDF_STATUS_SUCCESS - CSR updated config successfully.
 * Other status means CSR is failed to update.
 */

QDF_STATUS csr_roam_set_key_mgmt_offload(tpAniSirGlobal mac_ctx,
					 uint32_t session_id,
					 bool roam_key_mgmt_offload_enabled,
					 bool okc_enabled)
{
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), session_id);
		return QDF_STATUS_E_FAILURE;
	}
	session->RoamKeyMgmtOffloadEnabled = roam_key_mgmt_offload_enabled;
	session->okc_enabled = okc_enabled;
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_update_roam_scan_offload_request() - updates req msg with roam offload
 * paramters
 * @pMac:          mac global context
 * @req_buf:       out param, roam offload scan request packet
 * @session:       roam session
 *
 * Return: void
 */
static void
csr_update_roam_scan_offload_request(tpAniSirGlobal mac_ctx,
				     tSirRoamOffloadScanReq *req_buf,
				     tCsrRoamSession *session)
{
	qdf_mem_copy(req_buf->PSK_PMK, session->psk_pmk,
		     sizeof(req_buf->PSK_PMK));
	req_buf->pmk_len = session->pmk_len;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "LFR3: PMK Length = %d", req_buf->pmk_len);
	req_buf->R0KH_ID_Length = session->ftSmeContext.r0kh_id_len;
	qdf_mem_copy(req_buf->R0KH_ID,
		     session->ftSmeContext.r0kh_id,
		     req_buf->R0KH_ID_Length);
	req_buf->Prefer5GHz = mac_ctx->roam.configParam.nRoamPrefer5GHz;
	req_buf->RoamRssiCatGap = mac_ctx->roam.configParam.bCatRssiOffset;
	req_buf->Select5GHzMargin = mac_ctx->roam.configParam.nSelect5GHzMargin;
	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
			     (uint32_t *) &req_buf->ReassocFailureTimeout)
	    != eSIR_SUCCESS) {
		sms_log(mac_ctx, LOGE,
			FL("could not retrieve ReassocFailureTimeout value"));
		req_buf->ReassocFailureTimeout =
			DEFAULT_REASSOC_FAILURE_TIMEOUT;
	}
#ifdef FEATURE_WLAN_ESE
	if (csr_is_auth_type_ese(req_buf->ConnectedNetwork.authentication)) {
		qdf_mem_copy(req_buf->KRK, session->eseCckmInfo.krk,
			     SIR_KRK_KEY_LEN);
		qdf_mem_copy(req_buf->BTK, session->eseCckmInfo.btk,
			     SIR_BTK_KEY_LEN);
	}
#endif
	req_buf->AcUapsd.acbe_uapsd =
		SIR_UAPSD_GET(ACBE, mac_ctx->lim.gUapsdPerAcBitmask);
	req_buf->AcUapsd.acbk_uapsd =
		SIR_UAPSD_GET(ACBK, mac_ctx->lim.gUapsdPerAcBitmask);
	req_buf->AcUapsd.acvi_uapsd =
		SIR_UAPSD_GET(ACVI, mac_ctx->lim.gUapsdPerAcBitmask);
	req_buf->AcUapsd.acvo_uapsd =
		SIR_UAPSD_GET(ACVO, mac_ctx->lim.gUapsdPerAcBitmask);
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * csr_check_band_channel_match() - check if passed band and channel match
 * paramters
 * @band:       band to match with channel
 * @channel:    channel to match with band
 *
 * Return: bool if match else false
 */
static bool
csr_check_band_channel_match(eCsrBand band, uint8_t channel)
{
	if (eCSR_BAND_ALL == band)
		return true;

	if (eCSR_BAND_24 == band && CDS_IS_CHANNEL_24GHZ(channel))
		return true;

	if (eCSR_BAND_5G == band && CDS_IS_CHANNEL_5GHZ(channel))
		return true;

	return false;
}

/**
 * csr_fetch_ch_lst_from_ini() - fetch channel list from ini and update req msg
 * paramters
 * @mac_ctx:      global mac ctx
 * @roam_info:    roam info struct
 * @req_buf:      out param, roam offload scan request packet
 *
 * Return: result of operation
 */
static QDF_STATUS
csr_fetch_ch_lst_from_ini(tpAniSirGlobal mac_ctx,
			  tpCsrNeighborRoamControlInfo roam_info,
			  tSirRoamOffloadScanReq *req_buf)
{
	eCsrBand band;
	uint8_t i = 0;
	uint8_t num_channels = 0;
	uint8_t *ch_lst = roam_info->cfgParams.channelInfo.ChannelList;
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool      is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
			&unsafe_chan_cnt,
			 sizeof(unsafe_chan));

	/*
	 * The INI channels need to be filtered with respect to the current band
	 * that is supported.
	 */
	band = mac_ctx->roam.configParam.bandCapability;
	if ((eCSR_BAND_24 != band) && (eCSR_BAND_5G != band)
	    && (eCSR_BAND_ALL != band)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid band(%d), roam scan offload req aborted"),
			  band);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < roam_info->cfgParams.channelInfo.numOfChannels; i++) {
		if (!csr_check_band_channel_match(band, *ch_lst))
			continue;
		/* Allow DFS channels only if the DFS roaming is enabled */
		if ((!mac_ctx->roam.configParam.allowDFSChannelRoam ||
		    (mac_ctx->roam.configParam.sta_roam_policy.dfs_mode ==
			 CSR_STA_ROAM_POLICY_DFS_DISABLED)) &&
		     (CDS_IS_DFS_CH(*ch_lst))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("ignoring dfs channel %d"), *ch_lst);
			ch_lst++;
			continue;
		}

		if (mac_ctx->roam.configParam.sta_roam_policy.
				skip_unsafe_channels &&
				unsafe_chan_cnt) {
			is_unsafe_chan = false;
			for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
				if (unsafe_chan[cnt] == *ch_lst) {
					is_unsafe_chan = true;
					break;
				}
			}
			if (is_unsafe_chan) {
				QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_INFO,
					FL("ignoring unsafe channel %d"),
					*ch_lst);
				ch_lst++;
				continue;
			}
		}
		req_buf->ConnectedNetwork.ChannelCache[num_channels++] =
			*ch_lst;
		ch_lst++;

	}
	req_buf->ConnectedNetwork.ChannelCount = num_channels;
	req_buf->ChannelCacheType = CHANNEL_LIST_STATIC;
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_fetch_ch_lst_from_occupied_lst() - fetch channel list from occupied list
 * and update req msg
 * paramters
 * @mac_ctx:      global mac ctx
 * @session_id:   session id
 * @reason:       reason to roam
 * @req_buf:      out param, roam offload scan request packet
 * @roam_info:    roam info struct
 *
 * Return: void
 */
static void
csr_fetch_ch_lst_from_occupied_lst(tpAniSirGlobal mac_ctx,
				   uint8_t session_id,
				   uint8_t reason,
				   tSirRoamOffloadScanReq *req_buf,
				   tpCsrNeighborRoamControlInfo roam_info)
{
	uint8_t i = 0;
	uint8_t num_channels = 0;
	uint8_t *ch_lst =
		mac_ctx->scan.occupiedChannels[session_id].channelList;
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool      is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"Num of channels before filtering=%d",
		mac_ctx->scan.occupiedChannels[session_id].numChannels);
	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
			&unsafe_chan_cnt,
			 sizeof(unsafe_chan));
	for (i = 0; i < mac_ctx->scan.occupiedChannels[session_id].numChannels;
	     i++) {
		if ((!mac_ctx->roam.configParam.allowDFSChannelRoam ||
		    (mac_ctx->roam.configParam.sta_roam_policy.dfs_mode ==
			 CSR_STA_ROAM_POLICY_DFS_DISABLED)) &&
		     (CDS_IS_DFS_CH(*ch_lst))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("ignoring dfs channel %d"), *ch_lst);
			ch_lst++;
			continue;
		}

		if (mac_ctx->roam.configParam.sta_roam_policy.
				skip_unsafe_channels &&
				unsafe_chan_cnt) {
			is_unsafe_chan = false;
			for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
				if (unsafe_chan[cnt] == *ch_lst) {
					is_unsafe_chan = true;
					break;
				}
			}
			if (is_unsafe_chan) {
				QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_INFO,
					FL("ignoring unsafe channel %d"),
					*ch_lst);
				ch_lst++;
				continue;
			}
		}
		req_buf->ConnectedNetwork.ChannelCache[num_channels++] =
			*ch_lst;
		if (*ch_lst)
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"DFSRoam=%d, ChnlState=%d, Chnl=%d, num_ch=%d",
				mac_ctx->roam.configParam.allowDFSChannelRoam,
				cds_get_channel_state(*ch_lst), *ch_lst,
				num_channels);
		ch_lst++;
	}
	req_buf->ConnectedNetwork.ChannelCount = num_channels;
	/*
	 * If the profile changes as to what it was earlier, inform the FW
	 * through FLUSH as ChannelCacheType in which case, the FW will flush
	 * the occupied channels for the earlier profile and try to learn them
	 * afresh
	 */
	if (reason == REASON_FLUSH_CHANNEL_LIST)
		req_buf->ChannelCacheType = CHANNEL_LIST_DYNAMIC_FLUSH;
	else {
		if (csr_neighbor_roam_is_new_connected_profile(mac_ctx,
							       session_id))
			req_buf->ChannelCacheType = CHANNEL_LIST_DYNAMIC_INIT;
		else
			req_buf->ChannelCacheType = CHANNEL_LIST_DYNAMIC_UPDATE;
	}
}

/**
 * csr_fetch_valid_ch_lst() - fetch channel list from valid channel list and
 * update req msg
 * paramters
 * @mac_ctx:            global mac ctx
 * @req_buf:            out param, roam offload scan request packet
 *
 * Return: void
 */
static QDF_STATUS
csr_fetch_valid_ch_lst(tpAniSirGlobal mac_ctx,
		       tSirRoamOffloadScanReq *req_buf)
{
	QDF_STATUS status;
	uint32_t host_channels = 0;
	uint8_t *ch_lst = NULL;
	uint8_t i = 0, num_channels = 0;
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool      is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
			&unsafe_chan_cnt,
			sizeof(unsafe_chan));

	host_channels = sizeof(mac_ctx->roam.validChannelList);
	status = csr_get_cfg_valid_channels(mac_ctx,
					    mac_ctx->roam.validChannelList,
					    &host_channels);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to get the valid channel list"));
		return status;
	}
	ch_lst = mac_ctx->roam.validChannelList;
	mac_ctx->roam.numValidChannels = host_channels;
	for (i = 0; i < mac_ctx->roam.numValidChannels; i++) {
		ch_lst++;
		if ((!mac_ctx->roam.configParam.allowDFSChannelRoam ||
		    (mac_ctx->roam.configParam.sta_roam_policy.dfs_mode ==
			 CSR_STA_ROAM_POLICY_DFS_DISABLED)) &&
		     (CDS_IS_DFS_CH(*ch_lst))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("ignoring dfs channel %d"), *ch_lst);
			ch_lst++;
			continue;
		}

		if (mac_ctx->roam.configParam.
				sta_roam_policy.skip_unsafe_channels &&
				unsafe_chan_cnt) {
			is_unsafe_chan = false;
			for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
				if (unsafe_chan[cnt] == *ch_lst) {
					is_unsafe_chan = true;
					break;
				}
			}
			if (is_unsafe_chan) {
				QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_INFO,
					FL("ignoring unsafe channel %d"),
					*ch_lst);
				ch_lst++;
				continue;
			}
		}
		req_buf->ConnectedNetwork.ChannelCache[num_channels++] =
			*ch_lst;
		ch_lst++;
	}
	req_buf->ValidChannelCount = num_channels;
	return status;
}

/**
 * csr_create_roam_scan_offload_request() - init roam offload scan request
 *
 * paramters
 * @mac_ctx:      global mac ctx
 * @command:      roam scan offload command input
 * @session_id:   session id
 * @reason:       reason to roam
 * @session:      roam session
 * @roam_info:    roam info struct
 *
 * Return: roam offload scan request packet buffer
 */
static tSirRoamOffloadScanReq *
csr_create_roam_scan_offload_request(tpAniSirGlobal mac_ctx,
				     uint8_t command,
				     uint8_t session_id,
				     uint8_t reason,
				     tCsrRoamSession *session,
				     tpCsrNeighborRoamControlInfo roam_info)
{
	QDF_STATUS status;
	uint8_t i, j, dot11_mode;
	bool ese_neighbor_list_recvd = false;
	uint8_t ch_cache_str[128] = { 0 };
	tSirRoamOffloadScanReq *req_buf = NULL;
	tpCsrChannelInfo curr_ch_lst_info =
		&roam_info->roamChannelInfo.currentChannelListInfo;
#ifdef FEATURE_WLAN_ESE
	/*
	 * this flag will be true if connection is ESE and no neighbor
	 * list received or if the connection is not ESE
	 */
	ese_neighbor_list_recvd = ((roam_info->isESEAssoc)
		&& (roam_info->roamChannelInfo.IAPPNeighborListReceived
		    == false))
		|| (roam_info->isESEAssoc == false);
#endif /* FEATURE_WLAN_ESE */

	req_buf = qdf_mem_malloc(sizeof(tSirRoamOffloadScanReq));
	if (NULL == req_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Mem alloc for roam scan offload req failed."));
		return NULL;
	}
	qdf_mem_zero(req_buf, sizeof(tSirRoamOffloadScanReq));
	req_buf->Command = command;
	/*
	 * If command is STOP, then pass down ScanOffloadEnabled as Zero. This
	 * will handle the case of host driver reloads, but Riva still up and
	 * running
	 */
	if (command == ROAM_SCAN_OFFLOAD_STOP) {
		/*
		 * clear the roaming parameters that are per connection.
		 * For a new connection, they have to be programmed again.
		 */
		if (csr_neighbor_middle_of_roaming((tHalHandle)mac_ctx,
				session_id))
			req_buf->middle_of_roaming = 1;
		else
			csr_roam_reset_roam_params(mac_ctx);
		req_buf->RoamScanOffloadEnabled = 0;
	} else {
		req_buf->RoamScanOffloadEnabled =
			mac_ctx->roam.configParam.isRoamOffloadScanEnabled;
	}
	qdf_mem_copy(req_buf->ConnectedNetwork.currAPbssid,
		     roam_info->currAPbssid.bytes, sizeof(struct qdf_mac_addr));
	req_buf->ConnectedNetwork.ssId.length =
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.SSID.length;
	qdf_mem_copy(req_buf->ConnectedNetwork.ssId.ssId,
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.SSID.ssId,
		req_buf->ConnectedNetwork.ssId.length);
	req_buf->ConnectedNetwork.authentication =
		mac_ctx->roam.roamSession[session_id].connectedProfile.AuthType;
	req_buf->ConnectedNetwork.encryption =
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.EncryptionType;
	req_buf->ConnectedNetwork.mcencryption =
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.mcEncryptionType;
#ifdef WLAN_FEATURE_11W
	req_buf->ConnectedNetwork.mfp_enabled =
	    mac_ctx->roam.roamSession[session_id].connectedProfile.MFPEnabled;
#endif
	req_buf->delay_before_vdev_stop =
		roam_info->cfgParams.delay_before_vdev_stop;
	req_buf->OpportunisticScanThresholdDiff =
		roam_info->cfgParams.nOpportunisticThresholdDiff;
	req_buf->RoamRescanRssiDiff =
		roam_info->cfgParams.nRoamRescanRssiDiff;
	req_buf->RoamRssiDiff = mac_ctx->roam.configParam.RoamRssiDiff;
	req_buf->reason = reason;
	req_buf->NeighborScanTimerPeriod =
		roam_info->cfgParams.neighborScanPeriod;
	req_buf->NeighborRoamScanRefreshPeriod =
		roam_info->cfgParams.neighborResultsRefreshPeriod;
	req_buf->NeighborScanChannelMinTime =
		roam_info->cfgParams.minChannelScanTime;
	req_buf->NeighborScanChannelMaxTime =
		roam_info->cfgParams.maxChannelScanTime;
	req_buf->EmptyRefreshScanPeriod =
		roam_info->cfgParams.emptyScanRefreshPeriod;
	req_buf->RoamBmissFirstBcnt =
		roam_info->cfgParams.nRoamBmissFirstBcnt;
	req_buf->RoamBmissFinalBcnt =
		roam_info->cfgParams.nRoamBmissFinalBcnt;
	req_buf->RoamBeaconRssiWeight =
		roam_info->cfgParams.nRoamBeaconRssiWeight;
	/* MAWC feature */
	req_buf->MAWCEnabled = mac_ctx->roam.configParam.MAWCEnabled;
#ifdef FEATURE_WLAN_ESE
	req_buf->IsESEAssoc =
		csr_roam_is_ese_assoc(mac_ctx, session_id) &&
		((req_buf->ConnectedNetwork.authentication ==
			eCSR_AUTH_TYPE_OPEN_SYSTEM)  ||
		(csr_is_auth_type_ese(req_buf->
			ConnectedNetwork.authentication)));
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("LFR3:IsEseAssoc=%d\n"), req_buf->IsESEAssoc);
#endif
	if (ese_neighbor_list_recvd || curr_ch_lst_info->numOfChannels == 0) {
		/*
		 * Retrieve the Channel Cache either from ini or from the
		 * occupied channels list. Give Preference to INI Channels
		 */
		if (roam_info->cfgParams.channelInfo.numOfChannels) {
			status = csr_fetch_ch_lst_from_ini(mac_ctx, roam_info,
							   req_buf);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				qdf_mem_free(req_buf);
				return NULL;
			}
		} else {
			csr_fetch_ch_lst_from_occupied_lst(mac_ctx, session_id,
						reason, req_buf, roam_info);
		}
	}
#ifdef FEATURE_WLAN_ESE
	else {
		/*
		 * If ESE is enabled, and a neighbor Report is received,then
		 * Ignore the INI Channels or the Occupied Channel List.
		 * Consider the channels in the neighbor list sent by the ESE AP
		 */
		 csr_fetch_ch_lst_from_received_list(mac_ctx, roam_info,
						curr_ch_lst_info, req_buf);
	}
#endif
	for (i = 0, j = 0; i < req_buf->ConnectedNetwork.ChannelCount; i++) {
		if (j < sizeof(ch_cache_str)) {
			j += snprintf(ch_cache_str + j,
				      sizeof(ch_cache_str) - j, " %d",
				      req_buf->ConnectedNetwork.
				      ChannelCache[i]);
		} else
			break;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  FL("ChnlCacheType:%d, No of Chnls:%d,Channels: %s"),
		  req_buf->ChannelCacheType,
		  req_buf->ConnectedNetwork.ChannelCount, ch_cache_str);

	/* Maintain the Valid Channels List */
	status = csr_fetch_valid_ch_lst(mac_ctx, req_buf);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_free(req_buf);
		return NULL;
	}

	req_buf->MDID.mdiePresent =
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.MDID.mdiePresent;
	req_buf->MDID.mobilityDomain =
		mac_ctx->roam.roamSession[session_id].
		connectedProfile.MDID.mobilityDomain;
	req_buf->sessionId = session_id;
	req_buf->nProbes = mac_ctx->roam.configParam.nProbes;
	req_buf->HomeAwayTime = mac_ctx->roam.configParam.nRoamScanHomeAwayTime;

	/*
	 * Home Away Time should be at least equal to (MaxDwell time + (2*RFS)),
	 * where RFS is the RF Switching time. It is twice RFS to consider the
	 * time to go off channel and return to the home channel.
	 */
	if (req_buf->HomeAwayTime < (req_buf->NeighborScanChannelMaxTime +
	     (2 * CSR_ROAM_SCAN_CHANNEL_SWITCH_TIME))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
			  FL("Invalid config, Home away time(%d) is less than (twice RF switching time + channel max time)(%d). Hence enforcing home away time to disable (0)"),
			  req_buf->HomeAwayTime,
			  (req_buf->NeighborScanChannelMaxTime +
			   (2 * CSR_ROAM_SCAN_CHANNEL_SWITCH_TIME)));
		req_buf->HomeAwayTime = 0;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  FL("HomeAwayTime:%d"), req_buf->HomeAwayTime);

	/*Prepare a probe request for 2.4GHz band and one for 5GHz band */
	dot11_mode = (uint8_t) csr_translate_to_wni_cfg_dot11_mode(mac_ctx,
				csr_find_best_phy_mode(mac_ctx,
					mac_ctx->roam.configParam.phyMode));
	req_buf->allowDFSChannelRoam =
	mac_ctx->roam.configParam.allowDFSChannelRoam;
	req_buf->early_stop_scan_enable =
		mac_ctx->roam.configParam.early_stop_scan_enable;
	req_buf->early_stop_scan_min_threshold =
		mac_ctx->roam.configParam.early_stop_scan_min_threshold;
	req_buf->early_stop_scan_max_threshold =
		mac_ctx->roam.configParam.early_stop_scan_max_threshold;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  FL("EarlyStopFeature Enable=%d, MinThresh=%d, MaxThresh=%d"),
		  req_buf->early_stop_scan_enable,
		  req_buf->early_stop_scan_min_threshold,
		  req_buf->early_stop_scan_max_threshold);
	req_buf->roamscan_adaptive_dwell_mode =
		mac_ctx->roam.configParam.roamscan_adaptive_dwell_mode;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	req_buf->RoamOffloadEnabled = csr_roamIsRoamOffloadEnabled(mac_ctx);
	req_buf->RoamKeyMgmtOffloadEnabled = session->RoamKeyMgmtOffloadEnabled;
	req_buf->okc_enabled = session->okc_enabled;
	/* Roam Offload piggybacks upon the Roam Scan offload command. */
	if (req_buf->RoamOffloadEnabled)
		csr_update_roam_scan_offload_request(mac_ctx, req_buf, session);
	qdf_mem_copy(&req_buf->roam_params,
		&mac_ctx->roam.configParam.roam_params,
		sizeof(req_buf->roam_params));
#endif
	return req_buf;
}
/**
 * check_allowed_ssid_list() - Check the WhiteList
 * @req_buffer:      Buffer which contains the connected profile SSID.
 * @roam_params:     Buffer which contains the whitelist SSID's.
 *
 * Check if the connected profile SSID exists in the whitelist.
 * It is assumed that the framework provides this also in the whitelist.
 * If it exists there is no issue. Otherwise add it to the list.
 *
 * Return: None
 */
static void check_allowed_ssid_list(tSirRoamOffloadScanReq *req_buffer,
		struct roam_ext_params *roam_params)
{
	int i = 0;
	bool match = false;
	for (i = 0; i < roam_params->num_ssid_allowed_list; i++) {
		if ((roam_params->ssid_allowed_list[i].length ==
			req_buffer->ConnectedNetwork.ssId.length) &&
			(!qdf_mem_cmp(roam_params->ssid_allowed_list[i].ssId,
				req_buffer->ConnectedNetwork.ssId.ssId,
				roam_params->ssid_allowed_list[i].length))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"Whitelist contains connected profile SSID");
			match = true;
			break;
		}
	}
	if (!match) {
		if (roam_params->num_ssid_allowed_list >=
				MAX_SSID_ALLOWED_LIST) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"Whitelist is FULL. Cannot Add another entry");
			return;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"Adding Connected profile SSID to whitelist");
		/* i is the next available index to add the entry.*/
		i = roam_params->num_ssid_allowed_list;
		qdf_mem_copy(roam_params->ssid_allowed_list[i].ssId,
				req_buffer->ConnectedNetwork.ssId.ssId,
				req_buffer->ConnectedNetwork.ssId.length);
		roam_params->ssid_allowed_list[i].length =
			req_buffer->ConnectedNetwork.ssId.length;
		roam_params->num_ssid_allowed_list++;
	}
}

/*
 * Below Table describe whether RSO command can be send down to fimrware or not.
 * Host check it on the basis of previous RSO command sent down to firmware.
 *||==========================================================================||
 *|| New cmd        |            LAST SENT COMMAND --->                       ||
 *||====|=====================================================================||
 *||    V           | START | STOP | RESTART | UPDATE_CFG| ABORT_SCAN         ||
 *|| -------------------------------------------------------------------------||
 *|| RSO_START      | NO    | YES  |  NO     | NO        | NO                 ||
 *|| RSO_STOP       | YES   | YES  |  YES    | YES       | YES                ||
 *|| RSO_RESTART    | YES   | YES  |  NO     | YES       | YES                ||
 *|| RSO_UPDATE_CFG | YES   | NO   |  YES    | YES       | YES                ||
 *|| RSO_ABORT_SCAN | YES   | NO   |  YES    | YES       | YES                ||
 *||==========================================================================||
 **/
#define RSO_START_BIT       (1<<ROAM_SCAN_OFFLOAD_START)
#define RSO_STOP_BIT        (1<<ROAM_SCAN_OFFLOAD_STOP)
#define RSO_RESTART_BIT     (1<<ROAM_SCAN_OFFLOAD_RESTART)
#define RSO_UPDATE_CFG_BIT  (1<<ROAM_SCAN_OFFLOAD_UPDATE_CFG)
#define RSO_ABORT_SCAN_BIT  (1<<ROAM_SCAN_OFFLOAD_ABORT_SCAN)
#define RSO_START_ALLOW_MASK   (RSO_STOP_BIT)
#define RSO_STOP_ALLOW_MASK    (RSO_UPDATE_CFG_BIT | RSO_RESTART_BIT | \
		RSO_STOP_BIT | RSO_START_BIT | RSO_ABORT_SCAN_BIT)
#define RSO_RESTART_ALLOW_MASK (RSO_UPDATE_CFG_BIT | RSO_START_BIT | \
		RSO_ABORT_SCAN_BIT | RSO_RESTART_BIT)
#define RSO_UPDATE_CFG_ALLOW_MASK  (RSO_UPDATE_CFG_BIT | RSO_STOP_BIT | \
		RSO_START_BIT | RSO_ABORT_SCAN_BIT)
#define RSO_ABORT_SCAN_ALLOW_MASK (RSO_START_BIT | RSO_RESTART_BIT | \
		RSO_UPDATE_CFG_BIT | RSO_ABORT_SCAN_BIT)

bool csr_is_RSO_cmd_allowed(tpAniSirGlobal mac_ctx, uint8_t command,
		uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo neigh_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	uint8_t desiredMask = 0;
	bool ret_val;

	switch (command) {
	case ROAM_SCAN_OFFLOAD_START:
		desiredMask = RSO_START_ALLOW_MASK;
		break;
	case ROAM_SCAN_OFFLOAD_STOP:
		desiredMask = RSO_STOP_ALLOW_MASK;
		break;
	case ROAM_SCAN_OFFLOAD_RESTART:
		desiredMask = RSO_RESTART_ALLOW_MASK;
		break;
	case ROAM_SCAN_OFFLOAD_UPDATE_CFG:
		desiredMask = RSO_UPDATE_CFG_ALLOW_MASK;
		break;
	case ROAM_SCAN_OFFLOAD_ABORT_SCAN:
		desiredMask = RSO_ABORT_SCAN_ALLOW_MASK;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Wrong RSO command %d, not allowed"), command);
		return 0;/*Cmd Not allowed*/
	}
	ret_val = desiredMask & (1 << neigh_roam_info->last_sent_cmd);
	return ret_val;
}

/*
 * csr_roam_send_rso_cmd() - API to send RSO command to PE
 * @mac_ctx: Pointer to global MAC structure
 * @session_id: Session ID
 * @request_buf: Pointer to tSirRoamOffloadScanReq
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_roam_send_rso_cmd(tpAniSirGlobal mac_ctx, uint8_t session_id,
					tSirRoamOffloadScanReq *request_buf)
{
	QDF_STATUS status;
	request_buf->message_type = eWNI_SME_ROAM_SCAN_OFFLOAD_REQ;
	request_buf->length = sizeof(*request_buf);

	status = cds_send_mb_message_to_mac(request_buf);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE, FL("Send RSO from CSR failed"));
		return status;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_append_assoc_ies() - Append specific IE to assoc IE's buffer
 * @mac_ctx: Pointer to global mac context
 * @req_buf: Pointer to Roam offload scan request
 * @ie_id: IE ID to be appended
 * @ie_len: IE length to be appended
 * @ie_data: IE data to be appended
 *
 * Return: None
 */
static void csr_append_assoc_ies(tpAniSirGlobal mac_ctx,
				tSirRoamOffloadScanReq *req_buf, uint8_t ie_id,
				uint8_t ie_len, uint8_t *ie_data)
{
	tSirAddie *assoc_ie = &req_buf->assoc_ie;
	if ((SIR_MAC_MAX_ADD_IE_LENGTH - assoc_ie->length) < ie_len) {
		sms_log(mac_ctx, LOGE, "Appending IE(id:%d) fails", ie_id);
		return;
	}

	sms_log(mac_ctx, LOG1, "Appended IE(Id:%d) to RSO", ie_id);
	assoc_ie->addIEdata[assoc_ie->length] = ie_id;
	assoc_ie->addIEdata[assoc_ie->length + 1] = ie_len;
	qdf_mem_copy(&assoc_ie->addIEdata[assoc_ie->length + 2],
						ie_data, ie_len);
	assoc_ie->length += (ie_len + 2);
}

/**
 * csr_update_driver_assoc_ies() - Append driver built IE's to assoc IE's
 * @mac_ctx: Pointer to global mac structure
 * @session: pointer to CSR session
 * @req_buf: Pointer to Roam offload scan request
 *
 * Return: None
 */
static void csr_update_driver_assoc_ies(tpAniSirGlobal mac_ctx,
					tCsrRoamSession *session,
					tSirRoamOffloadScanReq *req_buf)
{
	bool power_caps_populated = false;
	uint32_t csr_11henable = WNI_CFG_11H_ENABLED_STADEF;
	uint8_t *rrm_cap_ie_data
			= (uint8_t *) &mac_ctx->rrm.rrmPEContext.rrmEnabledCaps;
	uint8_t power_cap_ie_data[DOT11F_IE_POWERCAPS_MAX_LEN]
			= {MIN_TX_PWR_CAP, MAX_TX_PWR_CAP};
	uint8_t max_tx_pwr_cap
			= csr_get_cfg_max_tx_power(mac_ctx,
				session->pConnectBssDesc->channelId);

	uint8_t supp_chan_ie[DOT11F_IE_SUPPCHANNELS_MAX_LEN], supp_chan_ie_len;
	uint8_t ese_ie[DOT11F_IE_ESEVERSION_MAX_LEN]
			= { 0x0, 0x40, 0x96, 0x3, ESE_VERSION_SUPPORTED};

	if (max_tx_pwr_cap)
		power_cap_ie_data[1] = max_tx_pwr_cap;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_11H_ENABLED, &csr_11henable);

	if (csr_11henable && csr_is11h_supported(mac_ctx)) {
		/* Append power cap IE */
		csr_append_assoc_ies(mac_ctx, req_buf, IEEE80211_ELEMID_PWRCAP,
					DOT11F_IE_POWERCAPS_MAX_LEN,
					power_cap_ie_data);
		power_caps_populated = true;

		/* Append Supported channels IE */
		csr_add_supported_5Ghz_channels(mac_ctx, supp_chan_ie,
					&supp_chan_ie_len, true);

		csr_append_assoc_ies(mac_ctx, req_buf,
					IEEE80211_ELEMID_SUPPCHAN,
					supp_chan_ie_len, supp_chan_ie);
	}

	/* Append ESE version IE if isEseIniFeatureEnabled INI is enabled */
	if (mac_ctx->roam.configParam.isEseIniFeatureEnabled)
		csr_append_assoc_ies(mac_ctx, req_buf, IEEE80211_ELEMID_VENDOR,
					DOT11F_IE_ESEVERSION_MAX_LEN,
					ese_ie);

	if (mac_ctx->rrm.rrmPEContext.rrmEnable) {
		/* Append RRM IE */
		csr_append_assoc_ies(mac_ctx, req_buf, IEEE80211_ELEMID_RRM,
					DOT11F_IE_RRMENABLEDCAP_MAX_LEN,
					rrm_cap_ie_data);
		if (!power_caps_populated)
			/* Append Power cap IE if not appended already */
			csr_append_assoc_ies(mac_ctx, req_buf,
					IEEE80211_ELEMID_PWRCAP,
					DOT11F_IE_POWERCAPS_MAX_LEN,
					power_cap_ie_data);
	}
}

/**
 * csr_roam_offload_scan() - populates roam offload scan request and sends to
 * WMA
 *
 * paramters
 * @mac_ctx:      global mac ctx
 * @session_id:   session id
 * @command:      roam scan offload command input
 * @reason:       reason to roam
 *
 * Return: result of operation
 */
QDF_STATUS
csr_roam_offload_scan(tpAniSirGlobal mac_ctx, uint8_t session_id,
		      uint8_t command, uint8_t reason)
{
	uint8_t *state = NULL;
	tSirRoamOffloadScanReq *req_buf;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	tpCsrNeighborRoamControlInfo roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	struct roam_ext_params *roam_params_dst;
	struct roam_ext_params *roam_params_src;
	uint8_t i;
	uint8_t op_channel;

	if (NULL == session) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("session is null"));
		return QDF_STATUS_E_FAILURE;
	}

	if ((ROAM_SCAN_OFFLOAD_START == command) && session->pCurRoamProfile &&
	    session->pCurRoamProfile->do_not_roam) {
		sms_log(mac_ctx, LOGE,
			FL("Supplicant disabled driver roaming"));
		return QDF_STATUS_E_FAILURE;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session->roam_synch_in_progress
	    && (ROAM_SCAN_OFFLOAD_STOP == command)) {
		/*
		 * When roam synch is in progress for propagation, there is no
		 * need to send down the STOP command since the firmware is not
		 * expecting any WMI commands when the roam synch is in progress
		 */
		b_roam_scan_offload_started = false;
		return QDF_STATUS_SUCCESS;
	}
#endif
	if (0 == csr_roam_is_roam_offload_scan_enabled(mac_ctx)) {
		sms_log(mac_ctx, LOGE, "isRoamOffloadScanEnabled not set");
		return QDF_STATUS_E_FAILURE;
	}
	if (!csr_is_RSO_cmd_allowed(mac_ctx, command, session_id) &&
			reason != REASON_ROAM_SET_BLACKLIST_BSSID) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("RSO out-of-sync command %d lastSentCmd %d"),
			command, roam_info->last_sent_cmd);
		return QDF_STATUS_E_FAILURE;
	}

	if ((true == b_roam_scan_offload_started)
	    && (ROAM_SCAN_OFFLOAD_START == command)) {
		sms_log(mac_ctx, LOGE, "Roam Scan Offload is already started");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * The Dynamic Config Items Update may happen even if the state is in
	 * INIT. It is important to ensure that the command is passed down to
	 * the FW only if the Infra Station is in a connected state.A connected
	 * station could also be in a PREAUTH or REASSOC states.So, consider not
	 * sending the command down in INIT state. We also have to ensure that
	 * if there is a STOP command we always have to inform Riva,
	 * irrespective of whichever state we are in
	 */

	if ((roam_info->neighborRoamState ==
	     eCSR_NEIGHBOR_ROAM_STATE_INIT) &&
	    (command != ROAM_SCAN_OFFLOAD_STOP) &&
	    (reason != REASON_ROAM_SET_BLACKLIST_BSSID)) {
		state = mac_trace_get_neighbour_roam_state(
				roam_info->neighborRoamState);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  FL("Scan Command not sent to FW state=%s and cmd=%d"),
			  state,  command);
		return QDF_STATUS_E_FAILURE;
	}

	req_buf = csr_create_roam_scan_offload_request(mac_ctx, command,
						       session_id, reason,
						       session, roam_info);
	if (!req_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to create req packet"));
		return QDF_STATUS_E_FAILURE;
	}
	roam_params_dst = &req_buf->roam_params;
	roam_params_src = &mac_ctx->roam.configParam.roam_params;
	if (reason == REASON_ROAM_SET_SSID_ALLOWED)
		check_allowed_ssid_list(req_buf, roam_params_src);
	/*
	 * Configure the lookup threshold either from INI or from framework.
	 * If both are present, give higher priority to the one from framework.
	 */
	if (roam_params_src->alert_rssi_threshold)
		req_buf->LookupThreshold =
			roam_params_src->alert_rssi_threshold;
	else
		req_buf->LookupThreshold =
			(int8_t)roam_info->cfgParams.neighborLookupThreshold *
			(-1);
	qdf_mem_copy(roam_params_dst, roam_params_src,
		sizeof(struct roam_ext_params));
	/*
	 * rssi_diff which is updated via framework is equivalent to the
	 * INI RoamRssiDiff parameter and hence should be updated.
	 */
	if (roam_params_src->rssi_diff)
		req_buf->RoamRssiDiff = roam_params_src->rssi_diff;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"num_bssid_avoid_list: %d, num_ssid_allowed_list: %d, "
		"num_bssid_favored: %d, raise_rssi_thresh_5g: %d, "
		"drop_rssi_thresh_5g: %d, raise_rssi_type_5g: %d, "
		"raise_factor_5g: %d, drop_rssi_type_5g: %d, "
		"drop_factor_5g: %d, max_raise_rssi_5g: %d, "
		"max_drop_rssi_5g: %d, rssi_diff: %d, alert_rssi_threshold:%d",
		roam_params_dst->num_bssid_avoid_list,
		roam_params_dst->num_ssid_allowed_list,
		roam_params_dst->num_bssid_favored,
		roam_params_dst->raise_rssi_thresh_5g,
		roam_params_dst->drop_rssi_thresh_5g,
		roam_params_dst->raise_rssi_type_5g,
		roam_params_dst->raise_factor_5g,
		roam_params_dst->drop_rssi_type_5g,
		roam_params_dst->drop_factor_5g,
		roam_params_dst->max_raise_rssi_5g,
		roam_params_dst->max_drop_rssi_5g,
		req_buf->RoamRssiDiff, roam_params_dst->alert_rssi_threshold);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"dense_rssi_thresh_offset: %d, dense_min_aps_cnt:%d, traffic_threshold:%d",
		roam_params_dst->dense_rssi_thresh_offset,
		roam_params_dst->dense_min_aps_cnt,
		roam_params_dst->traffic_threshold);

	/* Set initial dense roam status */
	if (mac_ctx->scan.roam_candidate_count[session_id] >
	    roam_params_dst->dense_min_aps_cnt)
		roam_params_dst->initial_dense_status = true;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		 "initial_dense_status:%d, candidate count:%d",
		 roam_params_dst->initial_dense_status,
		 mac_ctx->scan.roam_candidate_count[session_id]);

	for (i = 0; i < roam_params_dst->num_bssid_avoid_list; i++) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"Blacklist Bssid: ("MAC_ADDRESS_STR")",
			MAC_ADDR_ARRAY(roam_params_dst->bssid_avoid_list[i].bytes));
	}
	for (i = 0; i < roam_params_dst->num_ssid_allowed_list; i++) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"Whitelist: %.*s",
			roam_params_dst->ssid_allowed_list[i].length,
			roam_params_dst->ssid_allowed_list[i].ssId);
	}
	for (i = 0; i < roam_params_dst->num_bssid_favored; i++) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"Preferred Bssid: ("MAC_ADDRESS_STR") score: %d",
			MAC_ADDR_ARRAY(roam_params_dst->bssid_favored[i].bytes),
			roam_params_dst->bssid_favored_factor[i]);
	}

	op_channel = session->connectedProfile.operationChannel;
	req_buf->hi_rssi_scan_max_count =
		roam_info->cfgParams.hi_rssi_scan_max_count;
	req_buf->hi_rssi_scan_delay =
		roam_info->cfgParams.hi_rssi_scan_delay;
	req_buf->hi_rssi_scan_rssi_ub =
		roam_info->cfgParams.hi_rssi_scan_rssi_ub;
	/*
	 * If the current operation channel is 5G frequency band, then
	 * there is no need to enable the HI_RSSI feature. This feature
	 * is useful only if we are connected to a 2.4 GHz AP and we wish
	 * to connect to a better 5GHz AP is available.
	 */
	if (session->disable_hi_rssi)
		req_buf->hi_rssi_scan_rssi_delta = 0;
	else
		req_buf->hi_rssi_scan_rssi_delta =
			roam_info->cfgParams.hi_rssi_scan_rssi_delta;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"hi_rssi:delta=%d, max_count=%d, delay=%d, ub=%d",
			req_buf->hi_rssi_scan_rssi_delta,
			req_buf->hi_rssi_scan_max_count,
			req_buf->hi_rssi_scan_delay,
			req_buf->hi_rssi_scan_rssi_ub);

	if (command != ROAM_SCAN_OFFLOAD_STOP) {
		qdf_mem_copy(req_buf->assoc_ie.addIEdata,
				session->pAddIEAssoc,
				session->nAddIEAssocLength);
		csr_update_driver_assoc_ies(mac_ctx, session, req_buf);
	}

	if (!QDF_IS_STATUS_SUCCESS(
		csr_roam_send_rso_cmd(mac_ctx, session_id, req_buf))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post message to PE",
			  __func__);
		return QDF_STATUS_E_FAILURE;
	} else {
		if (ROAM_SCAN_OFFLOAD_START == command)
			b_roam_scan_offload_started = true;
		else if (ROAM_SCAN_OFFLOAD_STOP == command)
			b_roam_scan_offload_started = false;
	}
	/* update the last sent cmd */
	roam_info->last_sent_cmd = command;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "Roam Scan Offload Command %d, Reason %d", command, reason);
	return status;
}

QDF_STATUS csr_roam_offload_scan_rsp_hdlr(tpAniSirGlobal pMac,
					  tpSirRoamOffloadScanRsp scanOffloadRsp)
{
	switch (scanOffloadRsp->reason) {
	case 0:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "Rsp for Roam Scan Offload with failure status");
		break;
	case REASON_OS_REQUESTED_ROAMING_NOW:
		csr_neighbor_roam_proceed_with_handoff_req(pMac,
							   scanOffloadRsp->sessionId);
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "Rsp for Roam Scan Offload with reason %d",
			  scanOffloadRsp->reason);
	}
	return QDF_STATUS_SUCCESS;
}
#endif

tCsrPeStatsReqInfo *csr_roam_check_pe_stats_req_list(tpAniSirGlobal pMac,
						     uint32_t statsMask,
						     uint32_t periodicity,
						     bool *pFound,
						     uint8_t staId, uint8_t sessionId)
{
	bool found = false;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrPeStatsReqInfo staEntry;
	tCsrPeStatsReqInfo *pTempStaEntry = NULL;
	tListElem *pStaEntry = NULL;
	QDF_STATUS qdf_status;
	*pFound = false;

	pStaEntry = csr_roam_find_in_pe_stats_req_list(pMac, statsMask);
	if (pStaEntry) {
		pTempStaEntry =
			GET_BASE_ADDR(pStaEntry, tCsrPeStatsReqInfo, link);
		if (pTempStaEntry->periodicity) {
			pTempStaEntry->periodicity =
				QDF_MIN(periodicity,
					pTempStaEntry->periodicity);
		} else {
			pTempStaEntry->periodicity = periodicity;
		}
		pTempStaEntry->numClient++;
		found = true;
	} else {
		qdf_mem_set(&staEntry, sizeof(tCsrPeStatsReqInfo), 0);
		staEntry.numClient = 1;
		staEntry.periodicity = periodicity;
		staEntry.pMac = pMac;
		staEntry.rspPending = false;
		staEntry.staId = staId;
		staEntry.statsMask = statsMask;
		staEntry.timerRunning = false;
		staEntry.sessionId = sessionId;
		pTempStaEntry =
			csr_roam_insert_entry_into_pe_stats_req_list(pMac,
								     &pMac->roam.
								     peStatsReqList,
								     &staEntry);
		if (!pTempStaEntry) {
			/* msg */
			sms_log(pMac, LOGW,
				"csr_roam_check_pe_stats_req_list: Failed to insert req in peStatsReqList");
			return NULL;
		}
	}
	pTempStaEntry->periodicity =
		pMac->roam.configParam.statsReqPeriodicityInPS;

	if (!pTempStaEntry->timerRunning) {
		/* send down a req in case of one time req, for periodic ones wait for timer to expire */
		if (!pTempStaEntry->rspPending && !pTempStaEntry->periodicity) {
			status = csr_send_mb_stats_req_msg(pMac,
							   statsMask & ~(1 <<
									 eCsrGlobalClassDStats),
							   staId, sessionId);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_check_pe_stats_req_list:failed to send down stats req to PE"));
			} else {
				pTempStaEntry->rspPending = true;
			}
		}
		if (pTempStaEntry->periodicity) {
			if (!found) {

				qdf_status =
					qdf_mc_timer_init(&pTempStaEntry->
							  hPeStatsTimer,
							  QDF_TIMER_TYPE_SW,
							  csr_roam_pe_stats_timer_handler,
							  pTempStaEntry);
				if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
					sms_log(pMac, LOGE,
						FL
							("csr_roam_check_pe_stats_req_list:cannot init hPeStatsTimer timer"));
					return NULL;
				}
			}
			/* start timer */
			sms_log(pMac, LOG1,
				"csr_roam_check_pe_stats_req_list:peStatsTimer period %d",
				pTempStaEntry->periodicity);
			qdf_status =
				qdf_mc_timer_start(&pTempStaEntry->hPeStatsTimer,
						   pTempStaEntry->periodicity);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_check_pe_stats_req_list:cannot start hPeStatsTimer timer"));
				return NULL;
			}
			pTempStaEntry->timerRunning = true;
		}
	}
	*pFound = found;
	return pTempStaEntry;
}

/*
    pStaEntry is no longer invalid upon the return of this function.
 */
static void csr_roam_remove_stat_list_entry(tpAniSirGlobal pMac, tListElem *pEntry)
{
	if (pEntry) {
		if (csr_ll_remove_entry
			    (&pMac->roam.statsClientReqList, pEntry, LL_ACCESS_LOCK)) {
			qdf_mem_free(GET_BASE_ADDR
					     (pEntry, tCsrStatsClientReqInfo, link));
		}
	}
}

void csr_roam_remove_entry_from_pe_stats_req_list(tpAniSirGlobal pMac,
						tCsrPeStatsReqInfo *pPeStaEntry)
{
	tListElem *pEntry;
	tCsrPeStatsReqInfo *pTempStaEntry;
	QDF_STATUS qdf_status;
	pEntry = csr_ll_peek_head(&pMac->roam.peStatsReqList, LL_ACCESS_LOCK);
	if (!pEntry) {
		sms_log(pMac, LOGE, FL(" List empty, no stats req for PE"));
		return;
	}
	while (pEntry) {
		pTempStaEntry = GET_BASE_ADDR(pEntry, tCsrPeStatsReqInfo, link);
		if (NULL == pTempStaEntry
			|| (pTempStaEntry->statsMask !=
				pPeStaEntry->statsMask)) {
			pEntry = csr_ll_next(&pMac->roam.peStatsReqList, pEntry,
					LL_ACCESS_NOLOCK);
			continue;
		}
		sms_log(pMac, LOGW, FL("Match found"));
		if (pTempStaEntry->timerRunning) {
			qdf_status = qdf_mc_timer_stop(
					&pTempStaEntry->hPeStatsTimer);
			/*
			 * If we are not able to stop the timer here, just
			 * remove the entry from the linked list. Destroy the
			 * timer object and free the memory in the timer CB
			 */
			if (qdf_status == QDF_STATUS_SUCCESS) {
				/* the timer is successfully stopped */
				pTempStaEntry->timerRunning = false;
				/* Destroy the timer */
				qdf_status = qdf_mc_timer_destroy(
						&pTempStaEntry->hPeStatsTimer);
			} else {
				/*
				 * the timer could not be stopped. Hence destroy
				 * and free the memory for the PE stat entry in
				 * the timer CB.
				 */
				pTempStaEntry->timerStopFailed = true;
			}
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				sms_log(pMac, LOGE,
					FL("failed to stop/destroy timer"));
			}
		}

		if (csr_ll_remove_entry(&pMac->roam.peStatsReqList, pEntry,
					LL_ACCESS_LOCK)) {
			/*
			 * Only free the memory if we could stop the timer
			 * successfully
			 */
			if (!pTempStaEntry->timerStopFailed) {
				qdf_mem_free(pTempStaEntry);
				pTempStaEntry = NULL;
			}
			break;
		}
		pEntry = csr_ll_next(&pMac->roam.peStatsReqList, pEntry,
				     LL_ACCESS_NOLOCK);
	} /* end of while loop */
	return;
}

void csr_roam_report_statistics(tpAniSirGlobal pMac, uint32_t statsMask,
				tCsrStatsCallback callback, uint8_t staId,
				void *pContext)
{
	uint8_t stats[500];
	uint8_t *pStats = NULL;
	uint32_t tempMask = 0;
	uint8_t counter = 0;
	if (!callback) {
		sms_log(pMac, LOGE, FL("Cannot report callback NULL"));
		return;
	}
	if (!statsMask) {
		sms_log(pMac, LOGE, FL("Cannot report statsMask is 0"));
		return;
	}
	pStats = stats;
	tempMask = statsMask;
	while (tempMask) {
		if (tempMask & 1) {
			/* new stats info from PE, fill up the stats strucutres in PMAC */
			switch (counter) {
			case eCsrSummaryStats:
				sms_log(pMac, LOG2, FL("Summary stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     summaryStatsInfo,
					     sizeof(tCsrSummaryStatsInfo));
				pStats += sizeof(tCsrSummaryStatsInfo);
				break;
			case eCsrGlobalClassAStats:
				sms_log(pMac, LOG2, FL("ClassA stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     classAStatsInfo,
					     sizeof(tCsrGlobalClassAStatsInfo));
				pStats += sizeof(tCsrGlobalClassAStatsInfo);
				break;
			case eCsrGlobalClassBStats:
				sms_log(pMac, LOG2, FL("ClassB stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     classBStatsInfo,
					     sizeof(tCsrGlobalClassBStatsInfo));
				pStats += sizeof(tCsrGlobalClassBStatsInfo);
				break;
			case eCsrGlobalClassCStats:
				sms_log(pMac, LOG2, FL("ClassC stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     classCStatsInfo,
					     sizeof(tCsrGlobalClassCStatsInfo));
				pStats += sizeof(tCsrGlobalClassCStatsInfo);
				break;
			case eCsrGlobalClassDStats:
				sms_log(pMac, LOG2, FL("ClassD stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     classDStatsInfo,
					     sizeof(tCsrGlobalClassDStatsInfo));
				pStats += sizeof(tCsrGlobalClassDStatsInfo);
				break;
			case eCsrPerStaStats:
				sms_log(pMac, LOG2, FL("PerSta stats"));
				qdf_mem_copy(pStats,
					     (uint8_t *) &pMac->roam.
					     perStaStatsInfo[staId],
					     sizeof(tCsrPerStaStatsInfo));
				pStats += sizeof(tCsrPerStaStatsInfo);
				break;
			case csr_per_chain_rssi_stats:
				sms_log(pMac, LOG2, FL("Per Chain RSSI stats"));
				qdf_mem_copy(pStats,
				  (uint8_t *)&pMac->roam.per_chain_rssi_stats,
				  sizeof(struct csr_per_chain_rssi_stats_info));
				pStats += sizeof(
					struct csr_per_chain_rssi_stats_info);
				break;
			default:
				sms_log(pMac, LOGE,
					FL("Unknown stats type and counter %d"),
					counter);
				break;
			}
		}
		tempMask >>= 1;
		counter++;
	}
	callback(stats, pContext);
}

QDF_STATUS csr_roam_dereg_statistics_req(tpAniSirGlobal pMac)
{
	tListElem *pEntry = NULL;
	tListElem *pPrevEntry = NULL;
	tCsrStatsClientReqInfo *pTempStaEntry = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status;
	pEntry = csr_ll_peek_head(&pMac->roam.statsClientReqList, LL_ACCESS_LOCK);
	if (!pEntry) {
		/* list empty */
		sms_log(pMac, LOGW,
			"csr_roam_dereg_statistics_req: List empty, no request from "
			"upper layer client(s)");
		return status;
	}
	while (pEntry) {
		if (pPrevEntry) {
			pTempStaEntry =
				GET_BASE_ADDR(pPrevEntry, tCsrStatsClientReqInfo,
					      link);
			/* send up the stats report */
			csr_roam_report_statistics(pMac, pTempStaEntry->statsMask,
						   pTempStaEntry->callback,
						   pTempStaEntry->staId,
						   pTempStaEntry->pContext);
			csr_roam_remove_stat_list_entry(pMac, pPrevEntry);
		}
		pTempStaEntry =
			GET_BASE_ADDR(pEntry, tCsrStatsClientReqInfo, link);
		if (pTempStaEntry->pPeStaEntry) {
			/* pPeStaEntry can be NULL */
			pTempStaEntry->pPeStaEntry->numClient--;
			/* check if we need to delete the entry from peStatsReqList too */
			if (!pTempStaEntry->pPeStaEntry->numClient) {
				csr_roam_remove_entry_from_pe_stats_req_list(pMac,
									     pTempStaEntry->
									     pPeStaEntry);
			}
		}
		/* check if we need to stop the tl stats timer too */
		pMac->roam.tlStatsReqInfo.numClient--;
		if (!pMac->roam.tlStatsReqInfo.numClient) {
			if (pMac->roam.tlStatsReqInfo.timerRunning) {
				status =
					qdf_mc_timer_stop(&pMac->roam.
							  tlStatsReqInfo.
							  hTlStatsTimer);
				if (!QDF_IS_STATUS_SUCCESS(status)) {
					sms_log(pMac, LOGE,
						FL
							("csr_roam_dereg_statistics_req:cannot stop TlStatsTimer timer"));
					/* we will continue */
				}
			}
			pMac->roam.tlStatsReqInfo.periodicity = 0;
			pMac->roam.tlStatsReqInfo.timerRunning = false;
		}
		if (pTempStaEntry->periodicity) {
			/* While creating StaEntry in csr_get_statistics, */
			/* Initializing and starting timer only when periodicity is set. */
			/* So Stop and Destroy timer only when periodicity is set. */

			qdf_mc_timer_stop(&pTempStaEntry->timer);
			/* Destroy the qdf timer... */
			qdf_status =
				qdf_mc_timer_destroy(&pTempStaEntry->timer);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				sms_log(pMac, LOGE,
					FL
						("csr_roam_dereg_statistics_req:failed to destroy Client req timer"));
			}
		}

		pPrevEntry = pEntry;
		pEntry =
			csr_ll_next(&pMac->roam.statsClientReqList, pEntry,
				    LL_ACCESS_NOLOCK);
	}
	/* the last one */
	if (pPrevEntry) {
		pTempStaEntry =
			GET_BASE_ADDR(pPrevEntry, tCsrStatsClientReqInfo, link);
		/* send up the stats report */
		csr_roam_report_statistics(pMac, pTempStaEntry->statsMask,
					   pTempStaEntry->callback,
					   pTempStaEntry->staId,
					   pTempStaEntry->pContext);
		csr_roam_remove_stat_list_entry(pMac, pPrevEntry);
	}
	return status;

}

tSmeCmd *csr_get_command_buffer(tpAniSirGlobal pMac)
{
	tSmeCmd *pCmd = sme_get_command_buffer(pMac);
	if (pCmd) {
		pMac->roam.sPendingCommands++;
	}
	return pCmd;
}

void csr_release_command(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	if (pMac->roam.sPendingCommands > 0) {
		/* All command allocated through csr_get_command_buffer need to */
		/* decrement the pending count when releasing. */
		pMac->roam.sPendingCommands--;
		sme_release_command(pMac, pCommand);
	} else {
		sms_log(pMac, LOGE, FL("no pending commands"));
		QDF_ASSERT(0);
	}
}

/* Return SUCCESS is the command is queued, failed */
QDF_STATUS csr_queue_sme_command(tpAniSirGlobal pMac, tSmeCmd *pCommand,
				 bool fHighPriority)
{
	bool fNoCmdPending;

	if (!SME_IS_START(pMac)) {
		sms_log(pMac, LOGE, FL("Sme in stop state"));
		QDF_ASSERT(0);
		return QDF_STATUS_E_PERM;
	}

	if ((eSmeCommandScan == pCommand->command) && pMac->scan.fDropScanCmd) {
		sms_log(pMac, LOGW, FL(" drop scan (scan reason %d) command"),
			pCommand->u.scanCmd.reason);
		return QDF_STATUS_CSR_WRONG_STATE;
	}

	if ((pCommand->command == eSmeCommandScan)
	    || (pCommand->command == eSmeCommandRemainOnChannel)) {
		sms_log(pMac, LOGW,
		FL("scan pending list count %d scan_id %d"),
			pMac->sme.smeScanCmdPendingList.Count,
			pCommand->u.scanCmd.scanID);
		csr_ll_insert_tail(&pMac->sme.smeScanCmdPendingList,
				   &pCommand->Link, LL_ACCESS_LOCK);
		/* process the command queue... */
		sme_process_pending_queue(pMac);
		return QDF_STATUS_SUCCESS;
	}
	/* Make sure roamCmdPendingList is not empty first */
	fNoCmdPending =
		csr_ll_is_list_empty(&pMac->roam.roamCmdPendingList, false);
	if (fNoCmdPending) {
		sme_push_command(pMac, pCommand, fHighPriority);
	} else {
		/* no list lock is needed since SME lock is held */
		if (!fHighPriority) {
			csr_ll_insert_tail(&pMac->roam.roamCmdPendingList,
					&pCommand->Link, false);
		} else {
			csr_ll_insert_head(&pMac->roam.roamCmdPendingList,
					&pCommand->Link, false);
		}
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_roam_update_apwpsie(tpAniSirGlobal pMac, uint32_t sessionId,
				   tSirAPWPSIEs *pAPWPSIES)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirUpdateAPWPSIEsReq *pMsg;

	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL == pSession) {
		sms_log(pMac, LOGE,
			FL("Session does not exist for session id %d"),
			sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	pMsg = qdf_mem_malloc(sizeof(*pMsg));
	if (NULL == pMsg)
		return QDF_STATUS_E_NOMEM;

	pMsg->messageType = eWNI_SME_UPDATE_APWPSIE_REQ;
	pMsg->transactionId = 0;
	qdf_copy_macaddr(&pMsg->bssid, &pSession->selfMacAddr);
	pMsg->sessionId = sessionId;
	qdf_mem_copy(&pMsg->APWPSIEs, pAPWPSIES, sizeof(tSirAPWPSIEs));
	pMsg->length = sizeof(*pMsg);
	status = cds_send_mb_message_to_mac(pMsg);

	return status;
}

QDF_STATUS csr_roam_update_wparsni_es(tpAniSirGlobal pMac, uint32_t sessionId,
				      tSirRSNie *pAPSirRSNie)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirUpdateAPWPARSNIEsReq *pMsg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL == pSession) {
		sms_log(pMac, LOGE,
			FL("  Session does not exist for session id %d"),
			sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	do {
		pMsg = qdf_mem_malloc(sizeof(tSirUpdateAPWPARSNIEsReq));
		if (NULL == pMsg)
			return QDF_STATUS_E_NOMEM;
		qdf_mem_set(pMsg, sizeof(tSirUpdateAPWPARSNIEsReq), 0);
		pMsg->messageType = eWNI_SME_SET_APWPARSNIEs_REQ;
		pMsg->transactionId = 0;
		qdf_copy_macaddr(&pMsg->bssid, &pSession->selfMacAddr);
		pMsg->sessionId = sessionId;
		qdf_mem_copy(&pMsg->APWPARSNIEs, pAPSirRSNie,
			     sizeof(tSirRSNie));
		pMsg->length = sizeof(struct sSirUpdateAPWPARSNIEsReq);
		status = cds_send_mb_message_to_mac(pMsg);
	} while (0);
	return status;
}

/*
   pBuf points to the beginning of the message
   LIM packs disassoc rsp as below,
      messageType - 2 bytes
      messageLength - 2 bytes
      sessionId - 1 byte
      transactionId - 2 bytes (uint16_t)
      reasonCode - 4 bytes (sizeof(tSirResultCodes))
      peerMacAddr - 6 bytes
      The rest is conditionally defined of (WNI_POLARIS_FW_PRODUCT == AP) and not used
 */
static void csr_ser_des_unpack_diassoc_rsp(uint8_t *pBuf, tSirSmeDisassocRsp *pRsp)
{
	if (pBuf && pRsp) {
		pBuf += 4;      /* skip type and length */
		pRsp->sessionId = *pBuf++;
		qdf_get_u16(pBuf, (uint16_t *) &pRsp->transactionId);
		pBuf += 2;
		qdf_get_u32(pBuf, (uint32_t *) &pRsp->statusCode);
		pBuf += 4;
		qdf_mem_copy(pRsp->peer_macaddr.bytes, pBuf, QDF_MAC_ADDR_SIZE);
	}
}

/* Returns whether a session is in QDF_STA_MODE...or not */
bool csr_roam_is_sta_mode(tpAniSirGlobal pMac, uint32_t sessionId)
{
	tCsrRoamSession *pSession = NULL;
	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!pSession) {
		sms_log(pMac, LOGE, FL(" %s: session %d not found "), __func__,
			sessionId);
		return false;
	}
	if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
		sms_log(pMac, LOGE, FL(" %s: Inactive session"), __func__);
		return false;
	}
	if (eCSR_BSS_TYPE_INFRASTRUCTURE != pSession->connectedProfile.BSSType) {
		return false;
	}
	/* There is a possibility that the above check may fail,because
	 * P2P CLI also uses the same BSSType (eCSR_BSS_TYPE_INFRASTRUCTURE)
	 * when it is connected.So,we may sneak through the above check even
	 * if we are not a STA mode INFRA station. So, if we sneak through
	 * the above condition, we can use the following check if we are
	 * really in STA Mode.*/

	if (NULL != pSession->pCurRoamProfile) {
		if (pSession->pCurRoamProfile->csrPersona == QDF_STA_MODE) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}

QDF_STATUS csr_handoff_request(tpAniSirGlobal pMac,
			       uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	cds_msg_t msg;

	tAniHandoffReq *pMsg;
	pMsg = qdf_mem_malloc(sizeof(tAniHandoffReq));
	if (NULL == pMsg) {
		sms_log(pMac, LOGE,
			" csr_handoff_request: failed to allocate mem for req ");
		return QDF_STATUS_E_NOMEM;
	}
	pMsg->msgType = eWNI_SME_HANDOFF_REQ;
	pMsg->msgLen = (uint16_t) sizeof(tAniHandoffReq);
	pMsg->sessionId = sessionId;
	pMsg->channel = pHandoffInfo->channel;
	pMsg->handoff_src = pHandoffInfo->src;
	qdf_mem_copy(pMsg->bssid, pHandoffInfo->bssid.bytes, QDF_MAC_ADDR_SIZE);
	msg.type = eWNI_SME_HANDOFF_REQ;
	msg.bodyptr = pMsg;
	msg.reserved = 0;
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
		sms_log(pMac, LOGE,
			" csr_handoff_request failed to post msg to self ");
		qdf_mem_free((void *)pMsg);
		status = QDF_STATUS_E_FAILURE;
	}
	return status;
}

/**
 * csr_roam_channel_change_req() - Post channel change request to LIM
 * @pMac: mac context
 * @bssid: SAP bssid
 * @ch_params: channel information
 * @profile: CSR profile
 *
 * This API is primarily used to post Channel Change Req for SAP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_roam_channel_change_req(tpAniSirGlobal pMac,
				       struct qdf_mac_addr bssid,
				       struct ch_params_s *ch_params,
				       tCsrRoamProfile *profile)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirChanChangeRequest *pMsg;
	tCsrRoamStartBssParams param;

	csr_roam_get_bss_start_parms(pMac, profile, &param);
	pMsg = qdf_mem_malloc(sizeof(tSirChanChangeRequest));
	if (!pMsg) {
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_set((void *)pMsg, sizeof(tSirChanChangeRequest), 0);

	pMsg->messageType = eWNI_SME_CHANNEL_CHANGE_REQ;
	pMsg->messageLen = sizeof(tSirChanChangeRequest);
	pMsg->targetChannel = profile->ChannelInfo.ChannelList[0];
	pMsg->sec_ch_offset = ch_params->sec_ch_offset;
	pMsg->ch_width = profile->ch_params.ch_width;
	pMsg->dot11mode = csr_translate_to_wni_cfg_dot11_mode(pMac,
					pMac->roam.configParam.uCfgDot11Mode);
	if (IS_24G_CH(pMsg->targetChannel) &&
	   (false == pMac->roam.configParam.enableVhtFor24GHz) &&
	   (WNI_CFG_DOT11_MODE_11AC == pMsg->dot11mode ||
	    WNI_CFG_DOT11_MODE_11AC_ONLY == pMsg->dot11mode))
		pMsg->dot11mode = WNI_CFG_DOT11_MODE_11N;

	pMsg->center_freq_seg_0 = ch_params->center_freq_seg0;
	pMsg->center_freq_seg_1 = ch_params->center_freq_seg1;
	qdf_mem_copy(pMsg->bssid, bssid.bytes, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(&pMsg->operational_rateset,
		&param.operationalRateSet, sizeof(pMsg->operational_rateset));
	qdf_mem_copy(&pMsg->extended_rateset,
		&param.extendedRateSet, sizeof(pMsg->extended_rateset));
	status = cds_send_mb_message_to_mac(pMsg);

	return status;
}

/*
 * Post Beacon Tx Start request to LIM
 * immediately after SAP CAC WAIT is
 * completed without any RADAR indications.
 */
QDF_STATUS csr_roam_start_beacon_req(tpAniSirGlobal pMac,
				     struct qdf_mac_addr bssid,
				     uint8_t dfsCacWaitStatus)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirStartBeaconIndication *pMsg;

	pMsg = qdf_mem_malloc(sizeof(tSirStartBeaconIndication));

	if (!pMsg) {
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_set((void *)pMsg, sizeof(tSirStartBeaconIndication), 0);
	pMsg->messageType = eWNI_SME_START_BEACON_REQ;
	pMsg->messageLen = sizeof(tSirStartBeaconIndication);
	pMsg->beaconStartStatus = dfsCacWaitStatus;
	qdf_mem_copy(pMsg->bssid, bssid.bytes, QDF_MAC_ADDR_SIZE);

	status = cds_send_mb_message_to_mac(pMsg);

	return status;
}

/*----------------------------------------------------------------------------
   \fn csr_roam_modify_add_ies
   \brief  This function sends msg to modify the additional IE buffers in PE
   \param  pMac - pMac global structure
   \param  pModifyIE - pointer to tSirModifyIE structure
   \param  updateType - Type of buffer
   \- return Success or failure
   -----------------------------------------------------------------------------*/
QDF_STATUS
csr_roam_modify_add_ies(tpAniSirGlobal pMac,
			 tSirModifyIE *pModifyIE, eUpdateIEsType updateType)
{
	tpSirModifyIEsInd pModifyAddIEInd = NULL;
	uint8_t *pLocalBuffer = NULL;
	QDF_STATUS status;

	/* following buffer will be freed by consumer (PE) */
	pLocalBuffer = qdf_mem_malloc(pModifyIE->ieBufferlength);

	if (NULL == pLocalBuffer) {
		sms_log(pMac, LOGE, FL("Memory Allocation Failure!!!"));
		return QDF_STATUS_E_NOMEM;
	}

	pModifyAddIEInd = qdf_mem_malloc(sizeof(tSirModifyIEsInd));
	if (NULL == pModifyAddIEInd) {
		sms_log(pMac, LOGE, FL("Memory Allocation Failure!!!"));
		qdf_mem_free(pLocalBuffer);
		return QDF_STATUS_E_NOMEM;
	}

	/*copy the IE buffer */
	qdf_mem_copy(pLocalBuffer, pModifyIE->pIEBuffer,
		     pModifyIE->ieBufferlength);
	qdf_mem_zero(pModifyAddIEInd, sizeof(tSirModifyIEsInd));

	pModifyAddIEInd->msgType = eWNI_SME_MODIFY_ADDITIONAL_IES;
	pModifyAddIEInd->msgLen = sizeof(tSirModifyIEsInd);

	qdf_copy_macaddr(&pModifyAddIEInd->modifyIE.bssid, &pModifyIE->bssid);

	pModifyAddIEInd->modifyIE.smeSessionId = pModifyIE->smeSessionId;
	pModifyAddIEInd->modifyIE.notify = pModifyIE->notify;
	pModifyAddIEInd->modifyIE.ieID = pModifyIE->ieID;
	pModifyAddIEInd->modifyIE.ieIDLen = pModifyIE->ieIDLen;
	pModifyAddIEInd->modifyIE.pIEBuffer = pLocalBuffer;
	pModifyAddIEInd->modifyIE.ieBufferlength = pModifyIE->ieBufferlength;
	pModifyAddIEInd->modifyIE.oui_length = pModifyIE->oui_length;

	pModifyAddIEInd->updateType = updateType;

	status = cds_send_mb_message_to_mac(pModifyAddIEInd);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			FL("Failed to send eWNI_SME_UPDATE_ADDTIONAL_IES msg"
			   "!!! status %d"), status);
		qdf_mem_free(pLocalBuffer);
	}
	return status;
}

/*----------------------------------------------------------------------------
   \fn csr_roam_update_add_ies
   \brief  This function sends msg to updates the additional IE buffers in PE
   \param  pMac - pMac global structure
   \param  sessionId - SME session id
   \param  bssid - BSSID
   \param  additionIEBuffer - buffer containing addition IE from hostapd
   \param  length - length of buffer
   \param  updateType - Type of buffer
   \param  append - append or replace completely
   \- return Success or failure
   -----------------------------------------------------------------------------*/
QDF_STATUS
csr_roam_update_add_ies(tpAniSirGlobal pMac,
			 tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType)
{
	tpSirUpdateIEsInd pUpdateAddIEs = NULL;
	uint8_t *pLocalBuffer = NULL;
	QDF_STATUS status;

	if (pUpdateIE->ieBufferlength != 0) {
		/* Following buffer will be freed by consumer (PE) */
		pLocalBuffer = qdf_mem_malloc(pUpdateIE->ieBufferlength);
		if (NULL == pLocalBuffer) {
			sms_log(pMac, LOGE, FL("Memory Allocation Failure!!!"));
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(pLocalBuffer, pUpdateIE->pAdditionIEBuffer,
			     pUpdateIE->ieBufferlength);
	}

	pUpdateAddIEs = qdf_mem_malloc(sizeof(tSirUpdateIEsInd));
	if (NULL == pUpdateAddIEs) {
		sms_log(pMac, LOGE, FL("Memory Allocation Failure!!!"));
		if (pLocalBuffer != NULL) {
			qdf_mem_free(pLocalBuffer);
		}
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(pUpdateAddIEs, sizeof(tSirUpdateIEsInd));

	pUpdateAddIEs->msgType = eWNI_SME_UPDATE_ADDITIONAL_IES;
	pUpdateAddIEs->msgLen = sizeof(tSirUpdateIEsInd);

	qdf_copy_macaddr(&pUpdateAddIEs->updateIE.bssid, &pUpdateIE->bssid);

	pUpdateAddIEs->updateIE.smeSessionId = pUpdateIE->smeSessionId;
	pUpdateAddIEs->updateIE.append = pUpdateIE->append;
	pUpdateAddIEs->updateIE.notify = pUpdateIE->notify;
	pUpdateAddIEs->updateIE.ieBufferlength = pUpdateIE->ieBufferlength;
	pUpdateAddIEs->updateIE.pAdditionIEBuffer = pLocalBuffer;

	pUpdateAddIEs->updateType = updateType;

	status = cds_send_mb_message_to_mac(pUpdateAddIEs);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			FL("Failed to send eWNI_SME_UPDATE_ADDTIONAL_IES msg"
			   "!!! status %d"), status);
		qdf_mem_free(pLocalBuffer);
	}
	return status;
}

/**
 * csr_send_ext_change_channel()- function to post send ECSA
 * action frame to lim.
 * @mac_ctx: pointer to global mac structure
 * @channel: new channel to switch
 * @session_id: senssion it should be sent on.
 *
 * This function is called to post ECSA frame to lim.
 *
 * Return: success if msg posted to LIM else return failure
 */
QDF_STATUS csr_send_ext_change_channel(tpAniSirGlobal mac_ctx, uint32_t channel,
					uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sir_sme_ext_cng_chan_req *msg;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (NULL == msg)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_zero(msg, sizeof(*msg));
	msg->message_type = eWNI_SME_EXT_CHANGE_CHANNEL;
	msg->length = sizeof(*msg);
	msg->new_channel = channel;
	msg->session_id = session_id;
	status = cds_send_mb_message_to_mac(msg);
	return status;
}

/**
 * csr_roam_send_chan_sw_ie_request() - Request to transmit CSA IE
 * @mac_ctx:        Global MAC context
 * @bssid:          BSSID
 * @target_channel: Channel on which to send the IE
 * @csa_ie_reqd:    Include/Exclude CSA IE.
 * @ch_params:  operating Channel related information
 *
 * This function sends request to transmit channel switch announcement
 * IE to lower layers
 *
 * Return: success or failure
 **/
QDF_STATUS csr_roam_send_chan_sw_ie_request(tpAniSirGlobal mac_ctx,
					    struct qdf_mac_addr bssid,
					    uint8_t target_channel,
					    uint8_t csa_ie_reqd,
					    struct ch_params_s *ch_params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirDfsCsaIeRequest *msg;

	msg = qdf_mem_malloc(sizeof(tSirDfsCsaIeRequest));
	if (!msg) {
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_set((void *)msg, sizeof(tSirDfsCsaIeRequest), 0);
	msg->msgType = eWNI_SME_DFS_BEACON_CHAN_SW_IE_REQ;
	msg->msgLen = sizeof(tSirDfsCsaIeRequest);

	msg->targetChannel = target_channel;
	msg->csaIeRequired = csa_ie_reqd;
	qdf_mem_copy(msg->bssid, bssid.bytes, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(&msg->ch_params, ch_params, sizeof(struct ch_params_s));

	status = cds_send_mb_message_to_mac(msg);

	return status;
}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
/**
 * csr_roaming_report_diag_event() - Diag events for LFR3
 * @mac_ctx:              MAC context
 * @roam_synch_ind_ptr:   Roam Synch Indication Pointer
 * @reason:               Reason for this event to happen
 *
 * The major events in the host for LFR3 roaming such as
 * roam synch indication, roam synch completion and
 * roam synch handoff fail will be indicated to the
 * diag framework using this API.
 *
 * Return: None
 */
void csr_roaming_report_diag_event(tpAniSirGlobal mac_ctx,
		roam_offload_synch_ind *roam_synch_ind_ptr,
		eCsrDiagWlanStatusEventReason reason)
{
	WLAN_HOST_DIAG_EVENT_DEF(roam_connection,
		host_event_wlan_status_payload_type);
	qdf_mem_set(&roam_connection,
		sizeof(host_event_wlan_status_payload_type), 0);
	switch (reason) {
	case eCSR_REASON_ROAM_SYNCH_IND:
		roam_connection.eventId = eCSR_WLAN_STATUS_CONNECT;
		if (roam_synch_ind_ptr) {
			roam_connection.rssi = roam_synch_ind_ptr->rssi;
			roam_connection.channel =
				cds_freq_to_chan(roam_synch_ind_ptr->chan_freq);
		}
		break;
	case eCSR_REASON_ROAM_SYNCH_CNF:
		roam_connection.eventId = eCSR_WLAN_STATUS_CONNECT;
		break;
	case eCSR_REASON_ROAM_HO_FAIL:
		roam_connection.eventId = eCSR_WLAN_STATUS_DISCONNECT;
		break;
	default:
		sms_log(mac_ctx, LOGE,
			FL("LFR3: Unsupported reason %d"), reason);
		return;
	}
	roam_connection.reason = reason;
	WLAN_HOST_DIAG_EVENT_REPORT(&roam_connection, EVENT_WLAN_STATUS_V2);
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*----------------------------------------------------------------------------
* fn csr_process_ho_fail_ind
* brief  This function will process the Hand Off Failure indication
*        received from the firmware. It will trigger a disconnect on
*        the session which the firmware reported a hand off failure
* param  pMac global structure
* param  pMsgBuf - Contains the session ID for which the handler should apply
* --------------------------------------------------------------------------*/
void csr_process_ho_fail_ind(tpAniSirGlobal pMac, void *pMsgBuf)
{
	tSirSmeHOFailureInd *pSmeHOFailInd = (tSirSmeHOFailureInd *) pMsgBuf;
	uint32_t sessionId;

	if (pSmeHOFailInd)
		sessionId = pSmeHOFailInd->sessionId;
	else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "LFR3: Hand-Off Failure Ind is NULL");
		return;
	}
	/* Roaming is supported only on Infra STA Mode. */
	if (!csr_roam_is_sta_mode(pMac, sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "LFR3:HO Fail cannot be handled for session %d",
			  sessionId);
		return;
	}
	cds_set_connection_in_progress(false);
	csr_roam_synch_clean_up(pMac, sessionId);
	csr_roaming_report_diag_event(pMac, NULL,
			eCSR_REASON_ROAM_HO_FAIL);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "LFR3:Issue Disconnect on session %d", sessionId);
	csr_roam_disconnect(pMac, sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

/**
 * csr_update_op_class_array() - update op class for each band
 * @mac_ctx:          mac global context
 * @op_classes:       out param, operating class array to update
 * @channel_info:     channel info
 * @ch_name:          channel band name to display in debug messages
 * @i:                out param, stores number of operating classes
 *
 * Return: void
 */
static void
csr_update_op_class_array(tpAniSirGlobal mac_ctx,
			  uint8_t *op_classes,
			  tCsrChannel *channel_info,
			  char *ch_name,
			  uint8_t *i)
{
	uint8_t j = 0, idx = 0, class = 0;
	bool found = false;
	uint8_t num_channels = channel_info->numChannels;
	uint8_t ch_bandwidth;

	sms_log(mac_ctx, LOG1,
		FL("Num of %s channels,  %d"),
		ch_name, num_channels);

	for (idx = 0; idx < num_channels
		&& *i < (CDS_MAX_SUPP_OPER_CLASSES - 1); idx++) {
		for (ch_bandwidth = BW20; ch_bandwidth < BWALL;
			ch_bandwidth++) {
			class = cds_reg_dmn_get_opclass_from_channel(
					mac_ctx->scan.countryCodeCurrent,
					channel_info->channelList[idx],
					ch_bandwidth);
			sms_log(mac_ctx, LOG4, FL("for chan %d, op class: %d"),
				channel_info->channelList[idx], class);

			found = false;
			for (j = 0; j < CDS_MAX_SUPP_OPER_CLASSES - 1;
				j++) {
				if (op_classes[j] == class) {
					found = true;
					break;
				}
			}

			if (!found) {
				op_classes[*i] = class;
				*i = *i + 1;
			}
		}
	}
}

/**
 * csr_update_op_class_array() - update op class for all bands
 * @hHal:          global hal context
 *
 * Return: void
 */
void csr_init_operating_classes(tHalHandle hHal)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t swap = 0;
	uint8_t numClasses = 0;
	uint8_t opClasses[CDS_MAX_SUPP_OPER_CLASSES] = {0,};
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOG1, FL("Current Country = %c%c"),
		pMac->scan.countryCodeCurrent[0],
		pMac->scan.countryCodeCurrent[1]);

	csr_update_op_class_array(pMac, opClasses,
				  &pMac->scan.base_channels, "20MHz", &i);
	numClasses = i;

	/* As per spec the operating classes should be in ascending order.
	 * Bubble sort is fine since we don't have many classes
	 */
	for (i = 0; i < (numClasses - 1); i++) {
		for (j = 0; j < (numClasses - i - 1); j++) {
			/* For decreasing order use < */
			if (opClasses[j] > opClasses[j + 1]) {
				swap = opClasses[j];
				opClasses[j] = opClasses[j + 1];
				opClasses[j + 1] = swap;
			}
		}
	}

	sms_log(pMac, LOG1, FL("Number of unique supported op classes %d"),
		numClasses);
	for (i = 0; i < numClasses; i++) {
		sms_log(pMac, LOG1, FL("supported opClasses[%d] = %d"), i,
			opClasses[i]);
	}

	/* Set the ordered list of op classes in regdomain
	 * for use by other modules
	 */
	cds_reg_dmn_set_curr_opclasses(numClasses, &opClasses[0]);
}

/**
 * csr_find_session_by_type() - This function will find given session type from
 * all sessions.
 * @mac_ctx: pointer to mac context.
 * @type:    session type
 *
 * Return: session id for give session type.
 **/
static uint32_t
csr_find_session_by_type(tpAniSirGlobal mac_ctx, enum tQDF_ADAPTER_MODE type)
{
	uint32_t i, session_id = CSR_SESSION_ID_INVALID;
	tCsrRoamSession *session_ptr;
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		if (!CSR_IS_SESSION_VALID(mac_ctx, i))
			continue;

		session_ptr = CSR_GET_SESSION(mac_ctx, i);
		if (type == session_ptr->bssParams.bssPersona) {
			session_id = i;
			break;
		}
	}
	return session_id;
}
/**
 * csr_is_conn_allow_2g_band() - This function will check if station's conn
 * is allowed in 2.4Ghz band.
 * @mac_ctx: pointer to mac context.
 * @chnl: station's channel.
 *
 * This function will check if station's connection is allowed in 5Ghz band
 * after comparing it with SAP's operating channel. If SAP's operating
 * channel and Station's channel is different than this function will return
 * false else true.
 *
 * Return: true or false.
 **/
static bool csr_is_conn_allow_2g_band(tpAniSirGlobal mac_ctx, uint32_t chnl)
{
	uint32_t sap_session_id;
	tCsrRoamSession *sap_session;

	if (0 == chnl) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("channel is zero, connection not allowed"));

		return false;
	}

	sap_session_id = csr_find_session_by_type(mac_ctx, QDF_SAP_MODE);
	if (CSR_SESSION_ID_INVALID != sap_session_id) {
		sap_session = CSR_GET_SESSION(mac_ctx, sap_session_id);
		if ((0 != sap_session->bssParams.operationChn) &&
				(sap_session->bssParams.operationChn != chnl)) {

			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL
				("Can't allow STA to connect, chnls not same"));
			return false;
		}
	}
	return true;
}

/**
 * csr_is_conn_allow_5g_band() - This function will check if station's conn
 * is allowed in 5Ghz band.
 * @mac_ctx: pointer to mac context.
 * @chnl: station's channel.
 *
 * This function will check if station's connection is allowed in 5Ghz band
 * after comparing it with P2PGO's operating channel. If P2PGO's operating
 * channel and Station's channel is different than this function will return
 * false else true.
 *
 * Return: true or false.
 **/
static bool csr_is_conn_allow_5g_band(tpAniSirGlobal mac_ctx, uint32_t chnl)
{
	uint32_t p2pgo_session_id;
	tCsrRoamSession *p2pgo_session;

	if (0 == chnl) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL
				("channel is zero, connection not allowed"));
		return false;
	}

	p2pgo_session_id = csr_find_session_by_type(mac_ctx, QDF_P2P_GO_MODE);
	if (CSR_SESSION_ID_INVALID != p2pgo_session_id) {
		p2pgo_session = CSR_GET_SESSION(mac_ctx, p2pgo_session_id);
		if ((0 != p2pgo_session->bssParams.operationChn) &&
				(eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED !=
				 p2pgo_session->connectState) &&
				(p2pgo_session->bssParams.operationChn !=
				 chnl)) {

			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL
				("Can't allow STA to connect, chnls not same"));
			return false;
		}
	}
	return true;
}

/**
 * csr_clear_joinreq_param() - This function will clear station's params
 * for stored join request to csr.
 * @hal_handle: pointer to hal context.
 * @session_id: station's session id.
 *
 * This function will clear station's allocated memory for cached join
 * request.
 *
 * Return: true or false based on function's overall success.
 **/
bool csr_clear_joinreq_param(tpAniSirGlobal mac_ctx,
		uint32_t session_id)
{
	tCsrRoamSession *sta_session;
	tScanResultList *bss_list;

	if (NULL == mac_ctx)
		return false;

	sta_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == sta_session)
		return false;

	/* Release the memory allocated by previous join request */
	bss_list =
		(tScanResultList *)&sta_session->stored_roam_profile.
		bsslist_handle;
	if (NULL != bss_list) {
		csr_scan_result_purge(mac_ctx,
			sta_session->stored_roam_profile.bsslist_handle);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			FL("bss list is released for session %d"), session_id);
		sta_session->stored_roam_profile.bsslist_handle = NULL;
	}
	sta_session->stored_roam_profile.bsslist_handle = NULL;
	csr_release_profile(mac_ctx, &sta_session->stored_roam_profile.profile);
	sta_session->stored_roam_profile.reason = 0;
	sta_session->stored_roam_profile.roam_id = 0;
	sta_session->stored_roam_profile.imediate_flag = false;
	sta_session->stored_roam_profile.clear_flag = false;
	return true;
}

/**
 * csr_store_joinreq_param() - This function will store station's join
 * request to that station's session.
 * @mac_ctx: pointer to mac context.
 * @profile: pointer to station's roam profile.
 * @scan_cache: pointer to station's scan cache.
 * @roam_id: reference to roam_id variable being passed.
 * @session_id: station's session id.
 *
 * This function will store station's join request to one of the
 * csr structure and add it to station's session.
 *
 * Return: true or false based on function's overall success.
 **/
bool csr_store_joinreq_param(tpAniSirGlobal mac_ctx,
		tCsrRoamProfile *profile,
		tScanResultHandle scan_cache,
		uint32_t *roam_id,
		uint32_t session_id)
{
	tCsrRoamSession *sta_session;

	if (NULL == mac_ctx)
		return false;

	sta_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == sta_session)
		return false;

	sta_session->stored_roam_profile.session_id = session_id;
	csr_roam_copy_profile(mac_ctx,
			&sta_session->stored_roam_profile.profile, profile);
	/* new bsslist_handle's memory will be relased later */
	sta_session->stored_roam_profile.bsslist_handle = scan_cache;
	sta_session->stored_roam_profile.reason = eCsrHddIssued;
	sta_session->stored_roam_profile.roam_id = *roam_id;
	sta_session->stored_roam_profile.imediate_flag = false;
	sta_session->stored_roam_profile.clear_flag = false;

	return true;
}

/**
 * csr_issue_stored_joinreq() - This function will issues station's stored
 * the join request.
 * @mac_ctx: pointer to mac context.
 * @roam_id: reference to roam_id variable being passed.
 * @session_id: station's session id.
 *
 * This function will issue station's stored join request, from this point
 * onwards the flow will be just like normal connect request.
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE.
 **/
QDF_STATUS csr_issue_stored_joinreq(tpAniSirGlobal mac_ctx,
		uint32_t *roam_id,
		uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *sta_session;
	uint32_t new_roam_id;

	sta_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == sta_session)
		return QDF_STATUS_E_FAILURE;
	new_roam_id = GET_NEXT_ROAM_ID(&mac_ctx->roam);
	*roam_id = new_roam_id;
	status = csr_roam_issue_connect(mac_ctx,
			sta_session->stored_roam_profile.session_id,
			&sta_session->stored_roam_profile.profile,
			sta_session->stored_roam_profile.bsslist_handle,
			sta_session->stored_roam_profile.reason,
			new_roam_id,
			sta_session->stored_roam_profile.imediate_flag,
			sta_session->stored_roam_profile.clear_flag);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL
			("CSR failed issuing connect cmd with status = 0x%08X"),
				status);
		csr_clear_joinreq_param(mac_ctx, session_id);
	}
	return status;
}

/**
 * csr_process_set_hw_mode() - Set HW mode command to PE
 * @mac: Globacl MAC pointer
 * @command: Command received from SME
 *
 * Posts the set HW mode command to PE. This message passing
 * through PE is required for PE's internal management
 *
 * Return: None
 */
void csr_process_set_hw_mode(tpAniSirGlobal mac, tSmeCmd *command)
{
	uint32_t len;
	struct s_sir_set_hw_mode *cmd;
	QDF_STATUS status;
	tSirMsgQ msg;
	struct sir_set_hw_mode_resp *param;

	/* Setting HW mode is for the entire system.
	 * So, no need to check session
	 */

	if (!command) {
		sms_log(mac, LOGE, FL("Set HW mode param is NULL"));
		goto fail;
	}

	len = sizeof(*cmd);
	cmd = qdf_mem_malloc(len);
	if (!cmd) {
		sms_log(mac, LOGE, FL("Memory allocation failed"));
		/* Probably the fail response will also fail during malloc.
		 * Still proceeding to send response!
		 */
		goto fail;
	}

	/* For hidden SSID case, if there is any scan command pending
	 * it needs to be cleared before issuing set HW mode
	 */
	if (command->u.set_hw_mode_cmd.reason == SIR_UPDATE_REASON_HIDDEN_STA) {
		sms_log(mac, LOGE, FL("clear any pending scan command"));
		status = csr_scan_abort_mac_scan_not_for_connect(mac,
				command->u.set_hw_mode_cmd.session_id);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(mac, LOGE, FL("Failed to clear scan cmd"));
			goto fail;
		}
	}

	qdf_mem_set(cmd, len, 0);

	cmd->messageType = eWNI_SME_SET_HW_MODE_REQ;
	cmd->length = len;
	cmd->set_hw.hw_mode_index = command->u.set_hw_mode_cmd.hw_mode_index;
	cmd->set_hw.reason = command->u.set_hw_mode_cmd.reason;
	/*
	 * Below callback and context info are not needed for PE as of now.
	 * Storing the passed value in the same s_sir_set_hw_mode format.
	 */
	cmd->set_hw.set_hw_mode_cb = command->u.set_hw_mode_cmd.set_hw_mode_cb;

	sms_log(mac, LOG1,
		FL("Posting set hw mode req to PE session:%d reason:%d"),
		command->u.set_hw_mode_cmd.session_id,
		command->u.set_hw_mode_cmd.reason);

	status = cds_send_mb_message_to_mac(cmd);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac, LOGE, FL("Posting to PE failed"));
		return;
	}
	return;
fail:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		sms_log(mac, LOGE,
			FL("Malloc fail: Fail to send response to SME"));
		return;
	}
	sms_log(mac, LOGE, FL("Sending set HW fail response to SME"));
	param->status = SET_HW_MODE_STATUS_ECANCELED;
	param->cfgd_hw_mode_index = 0;
	param->num_vdev_mac_entries = 0;
	msg.type = eWNI_SME_SET_HW_MODE_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	sys_process_mmh_msg(mac, &msg);
}

/**
 * csr_process_set_dual_mac_config() - Set HW mode command to PE
 * @mac: Global MAC pointer
 * @command: Command received from SME
 *
 * Posts the set dual mac config command to PE.
 *
 * Return: None
 */
void csr_process_set_dual_mac_config(tpAniSirGlobal mac, tSmeCmd *command)
{
	uint32_t len;
	struct sir_set_dual_mac_cfg *cmd;
	QDF_STATUS status;
	tSirMsgQ msg;
	struct sir_dual_mac_config_resp *param;

	/* Setting MAC configuration is for the entire system.
	 * So, no need to check session
	 */

	if (!command) {
		sms_log(mac, LOGE, FL("Set HW mode param is NULL"));
		goto fail;
	}

	len = sizeof(*cmd);
	cmd = qdf_mem_malloc(len);
	if (!cmd) {
		sms_log(mac, LOGE, FL("Memory allocation failed"));
		/* Probably the fail response will also fail during malloc.
		 * Still proceeding to send response!
		 */
		goto fail;
	}

	cmd->message_type = eWNI_SME_SET_DUAL_MAC_CFG_REQ;
	cmd->length = len;
	cmd->set_dual_mac.scan_config = command->u.set_dual_mac_cmd.scan_config;
	cmd->set_dual_mac.fw_mode_config =
		command->u.set_dual_mac_cmd.fw_mode_config;
	/*
	 * Below callback and context info are not needed for PE as of now.
	 * Storing the passed value in the same sir_set_dual_mac_cfg format.
	 */
	cmd->set_dual_mac.set_dual_mac_cb =
		command->u.set_dual_mac_cmd.set_dual_mac_cb;

	sms_log(mac, LOG1,
		FL("Posting eWNI_SME_SET_DUAL_MAC_CFG_REQ to PE: %x %x"),
		cmd->set_dual_mac.scan_config,
		cmd->set_dual_mac.fw_mode_config);

	status = cds_send_mb_message_to_mac(cmd);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac, LOGE, FL("Posting to PE failed"));
		return;
	}
	return;
fail:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		sms_log(mac, LOGE,
			FL("Malloc fail: Fail to send response to SME"));
		return;
	}
	sms_log(mac, LOGE, FL("Sending set dual mac fail response to SME"));
	param->status = SET_HW_MODE_STATUS_ECANCELED;
	msg.type = eWNI_SME_SET_DUAL_MAC_CFG_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	sys_process_mmh_msg(mac, &msg);
}

/**
 * csr_process_set_antenna_mode() - Set antenna mode command to
 * PE
 * @mac: Global MAC pointer
 * @command: Command received from SME
 *
 * Posts the set dual mac config command to PE.
 *
 * Return: None
 */
void csr_process_set_antenna_mode(tpAniSirGlobal mac, tSmeCmd *command)
{
	uint32_t len;
	struct sir_set_antenna_mode *cmd;
	QDF_STATUS status;
	tSirMsgQ msg;
	struct sir_antenna_mode_resp *param;

	/* Setting MAC configuration is for the entire system.
	 * So, no need to check session
	 */

	if (!command) {
		sms_log(mac, LOGE, FL("Set antenna mode param is NULL"));
		goto fail;
	}

	len = sizeof(*cmd);
	cmd = qdf_mem_malloc(len);
	if (!cmd) {
		sms_log(mac, LOGE, FL("Memory allocation failed"));
		goto fail;
	}

	cmd->message_type = eWNI_SME_SET_ANTENNA_MODE_REQ;
	cmd->length = len;
	cmd->set_antenna_mode = command->u.set_antenna_mode_cmd;

	sms_log(mac, LOG1,
		FL("Posting eWNI_SME_SET_ANTENNA_MODE_REQ to PE: %d %d"),
		cmd->set_antenna_mode.num_rx_chains,
		cmd->set_antenna_mode.num_tx_chains);

	status = cds_send_mb_message_to_mac(cmd);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac, LOGE, FL("Posting to PE failed"));
		/*
		 * cds_send_mb_message_to_mac would've released the mem
		 * allocated by cmd.
		 */
		goto fail;
	}

	return;
fail:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		sms_log(mac, LOGE,
			FL("Malloc fail: Fail to send response to SME"));
		return;
	}
	sms_log(mac, LOGE, FL("Sending set dual mac fail response to SME"));
	param->status = SET_ANTENNA_MODE_STATUS_ECANCELED;
	msg.type = eWNI_SME_SET_ANTENNA_MODE_RESP;
	msg.bodyptr = param;
	msg.bodyval = 0;
	sys_process_mmh_msg(mac, &msg);
}

/**
 * csr_process_nss_update_req() - Update nss command to PE
 * @mac: Globacl MAC pointer
 * @command: Command received from SME
 *
 * Posts the nss update command to PE. This message passing
 * through PE is required for PE's internal management
 *
 * Return: None
 */
void csr_process_nss_update_req(tpAniSirGlobal mac, tSmeCmd *command)
{
	uint32_t len;
	struct sir_nss_update_request *msg;
	QDF_STATUS status;
	tSirMsgQ msg_return;
	struct sir_beacon_tx_complete_rsp *param;
	tCsrRoamSession *session;

	if (!command) {
		sms_log(mac, LOGE, FL("nss update param is NULL"));
		return;
	}

	if (!CSR_IS_SESSION_VALID(mac, command->sessionId)) {
		sms_log(mac, LOGE, FL("Invalid session id %d"),
			command->sessionId);
		return;
	}
	session = CSR_GET_SESSION(mac, command->sessionId);

	len = sizeof(*msg);
	msg = qdf_mem_malloc(len);
	if (!msg) {
		sms_log(mac, LOGE, FL("Memory allocation failed"));
		/* Probably the fail response is also fail during malloc.
		 * Still proceeding to send response!
		 */
		goto fail;
	}

	qdf_mem_set((void *)msg, sizeof(*msg), 0);
	msg->msgType = eWNI_SME_NSS_UPDATE_REQ;
	msg->msgLen = sizeof(*msg);

	msg->new_nss = command->u.nss_update_cmd.new_nss;
	msg->vdev_id = command->u.nss_update_cmd.session_id;

	sms_log(mac, LOG1,
		FL("Posting eWNI_SME_NSS_UPDATE_REQ to PE"));

	status = cds_send_mb_message_to_mac(msg);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac, LOGE, FL("Posting to PE failed"));
		return;
	}
	return;
fail:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		sms_log(mac, LOGE,
			FL("Malloc fail: Fail to send response to SME"));
		return;
	}
	sms_log(mac, LOGE, FL("Sending nss update fail response to SME"));
	param->tx_status = QDF_STATUS_E_FAILURE;
	param->session_id = command->u.nss_update_cmd.session_id;
	msg_return.type = eWNI_SME_NSS_UPDATE_RSP;
	msg_return.bodyptr = param;
	msg_return.bodyval = 0;
	sys_process_mmh_msg(mac, &msg_return);
}
#ifdef FEATURE_WLAN_TDLS
/**
 * csr_roam_fill_tdls_info() - Fill TDLS information
 * @roam_info: Roaming information buffer
 * @join_rsp: Join response which has TDLS info
 *
 * Return: None
 */
void csr_roam_fill_tdls_info(tCsrRoamInfo *roam_info, tpSirSmeJoinRsp join_rsp)
{
	roam_info->tdls_prohibited = join_rsp->tdls_prohibited;
	roam_info->tdls_chan_swit_prohibited =
		join_rsp->tdls_chan_swit_prohibited;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * csr_roam_synch_callback() - SME level callback for roam synch propagation
 * @mac_ctx: MAC Context
 * @roam_synch_data: Roam synch data buffer pointer
 * @bss_desc: BSS descriptor pointer
 * @reason: Reason for calling the callback
 *
 * This callback is registered with WMA and used after roaming happens in
 * firmware and the call to this routine completes the roam synch
 * propagation at both CSR and HDD levels. The HDD level propagation
 * is achieved through the already defined callback for assoc completion
 * handler.
 *
 * Return: None.
 */
void csr_roam_synch_callback(tpAniSirGlobal mac_ctx,
		roam_offload_synch_ind *roam_synch_data,
		tpSirBssDescription  bss_desc, enum sir_roam_op_code reason)
{
	uint8_t session_id = roam_synch_data->roamedVdevId;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	tDot11fBeaconIEs *ies_local = NULL;
	struct ps_global_info *ps_global_info = &mac_ctx->sme.ps_global_info;
	tCsrRoamInfo *roam_info;
	tCsrRoamConnectedProfile *conn_profile = NULL;
	sme_QosAssocInfo assoc_info;
	struct qdf_mac_addr bcast_mac = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	tpAddBssParams add_bss_params;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint16_t len;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	tSirSmeHTProfile *src_profile = NULL;
	tCsrRoamHTProfile *dst_profile = NULL;
#endif

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("LFR3: Locking failed, bailing out"));
		return;
	}
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("LFR3: Session not found"));
		sme_release_global_lock(&mac_ctx->sme);
		return;
	}
	switch (reason) {
	case SIR_ROAMING_DEREGISTER_STA:
		csr_roam_call_callback(mac_ctx, session_id, NULL, 0,
				eCSR_ROAM_FT_START, eSIR_SME_SUCCESS);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	case SIR_ROAMING_START:
		csr_roam_call_callback(mac_ctx, session_id, NULL, 0,
				eCSR_ROAM_START, eSIR_SME_SUCCESS);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	case SIR_ROAMING_ABORT:
		csr_roam_call_callback(mac_ctx, session_id, NULL, 0,
				eCSR_ROAM_ABORT, eSIR_SME_SUCCESS);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	case SIR_ROAM_SYNCH_PROPAGATION:
		break;
	case SIR_ROAM_SYNCH_COMPLETE:
		/*
		 * Following operations need to be done once roam sync
		 * completion is sent to FW, hence called here:
		 * 1) Firmware has already updated DBS policy. Update connection
		 *    table in the host driver.
		 * 2) Force SCC switch if needed
		 * 3) Set connection in progress = false
		 */
		/* first update connection info from wma interface */
		cds_update_connection_info(session_id);
		/* then update remaining parameters from roam sync ctx */
		sms_log(mac_ctx, LOGE, FL("Update DBS hw mode"));
		cds_hw_mode_transition_cb(
			roam_synch_data->hw_mode_trans_ind.old_hw_mode_index,
			roam_synch_data->hw_mode_trans_ind.new_hw_mode_index,
			roam_synch_data->hw_mode_trans_ind.num_vdev_mac_entries,
			roam_synch_data->hw_mode_trans_ind.vdev_mac_map);
		cds_set_connection_in_progress(false);
		session->roam_synch_in_progress = false;
		cds_check_concurrent_intf_and_restart_sap(session->pContext);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	default:
		sms_log(mac_ctx, LOGE, FL("LFR3: callback reason %d"), reason);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	}
	session->roam_synch_in_progress = true;
	session->roam_synch_data = roam_synch_data;
	if (!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies(mac_ctx,
			bss_desc, &ies_local))) {
		sms_log(mac_ctx, LOGE, FL("LFR3: fail to parse IEs"));
		session->roam_synch_in_progress = false;
		sme_release_global_lock(&mac_ctx->sme);
		return;
	}
	conn_profile = &session->connectedProfile;
	csr_roam_stop_network(mac_ctx, session_id,
		session->pCurRoamProfile,
		bss_desc,
		ies_local);
	ps_global_info->remain_in_power_active_till_dhcp = false;
	session->connectState = eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED;
	roam_info = qdf_mem_malloc(sizeof(tCsrRoamInfo));
	if (NULL == roam_info) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("LFR3: Mem Alloc failed for roam info"));
		session->roam_synch_in_progress = false;
		sme_release_global_lock(&mac_ctx->sme);
		return;
	}
	csr_scan_save_roam_offload_ap_to_scan_cache(mac_ctx, roam_synch_data,
			bss_desc);
	qdf_mem_zero(roam_info, sizeof(tCsrRoamInfo));
	roam_info->sessionId = session_id;
	csr_roam_call_callback(mac_ctx, roam_synch_data->roamedVdevId,
		roam_info, 0, eCSR_ROAM_TDLS_STATUS_UPDATE,
		eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND);
	qdf_mem_copy(&roam_info->bssid.bytes, &bss_desc->bssId,
			sizeof(struct qdf_mac_addr));
	csr_roam_save_connected_infomation(mac_ctx, session_id,
			session->pCurRoamProfile,
			bss_desc,
			ies_local);
	csr_roam_save_security_rsp_ie(mac_ctx, session_id,
			session->pCurRoamProfile->negotiatedAuthType,
			bss_desc, ies_local);
	roam_info->isESEAssoc = conn_profile->isESEAssoc;
	if (CSR_IS_ENC_TYPE_STATIC
		(session->pCurRoamProfile->negotiatedUCEncryptionType) &&
		!session->pCurRoamProfile->bWPSAssociation) {
		if (!QDF_IS_STATUS_SUCCESS(
			csr_roam_issue_set_context_req(mac_ctx,
				session_id,
				session->pCurRoamProfile->negotiatedUCEncryptionType,
				bss_desc,
				&(bss_desc->bssId),
				false, true,
				eSIR_TX_RX, 0, 0, NULL, 0))) {
			/* NO keys. these key parameters don't matter */
			sms_log(mac_ctx, LOGE,
					FL("Set context for unicast fail"));
			csr_roam_substate_change(mac_ctx,
					eCSR_ROAM_SUBSTATE_NONE, session_id);
		}
		csr_roam_issue_set_context_req(mac_ctx, session_id,
			session->pCurRoamProfile->negotiatedMCEncryptionType,
			bss_desc,
			&bcast_mac.bytes, false, false,
			eSIR_TX_RX, 0, 0, NULL, 0);
	}
	if ((roam_synch_data->authStatus
				== CSR_ROAM_AUTH_STATUS_AUTHENTICATED)) {
		QDF_TRACE(QDF_MODULE_ID_SME,
				QDF_TRACE_LEVEL_DEBUG,
				FL("LFR3:Don't start waitforkey timer"));
		csr_roam_substate_change(mac_ctx,
				eCSR_ROAM_SUBSTATE_NONE, session_id);
	} else {
		roam_info->fAuthRequired = true;
		csr_roam_substate_change(mac_ctx,
				eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY,
				session_id);

		ps_global_info->remain_in_power_active_till_dhcp = true;
		mac_ctx->roam.WaitForKeyTimerInfo.sessionId = session_id;
		if (!QDF_IS_STATUS_SUCCESS(csr_roam_start_wait_for_key_timer(
				mac_ctx, CSR_WAIT_FOR_KEY_TIMEOUT_PERIOD))
		   ) {
			sms_log(mac_ctx, LOGE, FL
					("Failed wait for key timer start"));
			csr_roam_substate_change(mac_ctx,
					eCSR_ROAM_SUBSTATE_NONE,
					session_id);
		}
	}
	roam_info->nBeaconLength = 0;
	roam_info->nAssocReqLength = roam_synch_data->reassoc_req_length -
		SIR_MAC_HDR_LEN_3A - SIR_MAC_REASSOC_SSID_OFFSET;
	roam_info->nAssocRspLength = roam_synch_data->reassocRespLength -
		SIR_MAC_HDR_LEN_3A;
	roam_info->pbFrames = qdf_mem_malloc(roam_info->nBeaconLength +
		roam_info->nAssocReqLength + roam_info->nAssocRspLength);
	if (NULL == roam_info->pbFrames) {
		sms_log(mac_ctx, LOGE, FL("no memory available"));
		session->roam_synch_in_progress = false;
		if (roam_info)
			qdf_mem_free(roam_info);
		sme_release_global_lock(&mac_ctx->sme);
		return;
	}
	qdf_mem_zero(roam_info->pbFrames, roam_info->nBeaconLength +
		roam_info->nAssocReqLength + roam_info->nAssocRspLength);
	qdf_mem_copy(roam_info->pbFrames,
			(uint8_t *)roam_synch_data +
			roam_synch_data->reassoc_req_offset +
			SIR_MAC_HDR_LEN_3A + SIR_MAC_REASSOC_SSID_OFFSET,
			roam_info->nAssocReqLength);
	qdf_mem_copy(roam_info->pbFrames + roam_info->nAssocReqLength,
			(uint8_t *)roam_synch_data +
			roam_synch_data->reassocRespOffset +
			SIR_MAC_HDR_LEN_3A,
			roam_info->nAssocRspLength);

	QDF_TRACE(QDF_MODULE_ID_SME,
			QDF_TRACE_LEVEL_DEBUG,
			FL("LFR3:Clear Connected info"));
	csr_roam_free_connected_info(mac_ctx,
			&session->connectedInfo);
	len = roam_synch_data->join_rsp->parsedRicRspLen;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("LFR3: RIC length - %d"), len);
	if (len) {
		session->connectedInfo.pbFrames =
			qdf_mem_malloc(len);
		if (session->connectedInfo.pbFrames != NULL) {
			qdf_mem_copy(session->connectedInfo.pbFrames,
				roam_synch_data->join_rsp->frames, len);
			session->connectedInfo.nRICRspLength =
				roam_synch_data->join_rsp->parsedRicRspLen;
		}
	}
	conn_profile->vht_channel_width =
		roam_synch_data->join_rsp->vht_channel_width;
	add_bss_params = (tpAddBssParams)roam_synch_data->add_bss_params;
	session->connectedInfo.staId = add_bss_params->staContext.staIdx;
	roam_info->staId = session->connectedInfo.staId;
	roam_info->ucastSig =
		(uint8_t) roam_synch_data->join_rsp->ucastSig;
	roam_info->bcastSig =
		(uint8_t) roam_synch_data->join_rsp->bcastSig;
	roam_info->timingMeasCap =
		roam_synch_data->join_rsp->timingMeasCap;
	roam_info->chan_info.nss = roam_synch_data->join_rsp->nss;
	roam_info->chan_info.rate_flags =
		roam_synch_data->join_rsp->max_rate_flags;
	csr_roam_fill_tdls_info(roam_info, roam_synch_data->join_rsp);
	sms_log(mac_ctx, LOG1,
		FL("tdls:prohibit: %d, chan_swit_prohibit: %d"),
		roam_info->tdls_prohibited,
		roam_info->tdls_chan_swit_prohibited);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	src_profile = &roam_synch_data->join_rsp->HTProfile;
	dst_profile = &conn_profile->HTProfile;
	if (mac_ctx->roam.configParam.cc_switch_mode
			!= QDF_MCC_TO_SCC_SWITCH_DISABLE)
		csr_roam_copy_ht_profile(dst_profile,
				src_profile);
#endif
	assoc_info.pBssDesc = bss_desc;
	roam_info->statusCode = eSIR_SME_SUCCESS;
	roam_info->reasonCode = eSIR_SME_SUCCESS;
	assoc_info.pProfile = session->pCurRoamProfile;
	mac_ctx->roam.roamSession[session_id].connectState =
		eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
	sme_qos_csr_event_ind(mac_ctx, session_id,
		SME_QOS_CSR_HANDOFF_ASSOC_REQ, NULL);
	sme_qos_csr_event_ind(mac_ctx, session_id,
		SME_QOS_CSR_REASSOC_REQ, NULL);
	sme_qos_csr_event_ind(mac_ctx, session_id,
		SME_QOS_CSR_HANDOFF_COMPLETE, NULL);
	mac_ctx->roam.roamSession[session_id].connectState =
		eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED;
	sme_qos_csr_event_ind(mac_ctx, session_id,
		SME_QOS_CSR_REASSOC_COMPLETE, &assoc_info);
	roam_info->pBssDesc = bss_desc;
	conn_profile->acm_mask = sme_qos_get_acm_mask(mac_ctx,
			bss_desc, NULL);
	if (conn_profile->modifyProfileFields.uapsd_mask) {
		sms_log(mac_ctx, LOG1,
				" uapsd_mask (0x%X) set, request UAPSD now",
				conn_profile->modifyProfileFields.uapsd_mask);
		sme_ps_start_uapsd(mac_ctx, session_id,
				NULL, NULL);
	}
	conn_profile->dot11Mode = session->bssParams.uCfgDot11Mode;
	roam_info->u.pConnectedProfile = conn_profile;

	if (!IS_FEATURE_SUPPORTED_BY_FW
			(SLM_SESSIONIZATION) &&
			(csr_is_concurrent_session_running(mac_ctx))) {
		mac_ctx->roam.configParam.doBMPSWorkaround = 1;
	}
	roam_info->roamSynchInProgress = true;
	roam_info->synchAuthStatus = roam_synch_data->authStatus;
	qdf_mem_copy(roam_info->kck, roam_synch_data->kck, SIR_KCK_KEY_LEN);
	qdf_mem_copy(roam_info->kek, roam_synch_data->kek, SIR_KEK_KEY_LEN);
	qdf_mem_copy(roam_info->replay_ctr, roam_synch_data->replay_ctr,
			SIR_REPLAY_CTR_LEN);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		FL("LFR3: Copy KCK, KEK and Replay Ctr"));
	roam_info->subnet_change_status =
		CSR_GET_SUBNET_STATUS(roam_synch_data->roamReason);
	csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
		eCSR_ROAM_ASSOCIATION_COMPLETION, eCSR_ROAM_RESULT_ASSOCIATED);
	csr_reset_pmkid_candidate_list(mac_ctx, session_id);
#ifdef FEATURE_WLAN_WAPI
	csr_reset_bkid_candidate_list(mac_ctx, session_id);
#endif
	if (!CSR_IS_WAIT_FOR_KEY(mac_ctx, session_id)) {
		QDF_TRACE(QDF_MODULE_ID_SME,
				QDF_TRACE_LEVEL_DEBUG,
				FL
				("NO CSR_IS_WAIT_FOR_KEY -> csr_roam_link_up"));
		csr_roam_link_up(mac_ctx, conn_profile->bssid);
	}

	session->fRoaming = false;
	session->roam_synch_in_progress = false;
	qdf_mem_free(roam_info->pbFrames);
	qdf_mem_free(roam_info);
	sme_release_global_lock(&mac_ctx->sme);
}
#endif
