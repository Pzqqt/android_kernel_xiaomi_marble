/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: csr_api_scan.c
 *
 * Implementation for the Common Scan interfaces.
 */

#include "ani_global.h"

#include "csr_inside_api.h"
#include "sme_inside.h"

#include "csr_support.h"

#include "host_diag_core_log.h"
#include "host_diag_core_event.h"

#include "cds_reg_service.h"
#include "wma_types.h"
#include "cds_utils.h"
#include "wma.h"

#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_main.h"
#include "pld_common.h"
#include "csr_internal.h"
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_api.h>
#include <wlan_scan_utils_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_utility.h>
#include "wlan_reg_services_api.h"
#include "sch_api.h"
#include "wlan_blm_api.h"
#include "qdf_crypto.h"
#include <wlan_crypto_global_api.h>
#include "wlan_reg_ucfg_api.h"
#include "wlan_cm_bss_score_param.h"

static void csr_set_cfg_valid_channel_list(struct mac_context *mac,
					   uint32_t *pchan_freq_list,
					   uint8_t NumChannels);

static void csr_save_tx_power_to_cfg(struct mac_context *mac,
				     tDblLinkList *pList,
				     uint32_t cfgId);

static void csr_purge_channel_power(struct mac_context *mac,
				    tDblLinkList *pChannelList);

/* pResult is invalid calling this function. */
void csr_free_scan_result_entry(struct mac_context *mac,
				struct tag_csrscan_result *pResult)
{
	if (pResult->Result.pvIes)
		qdf_mem_free(pResult->Result.pvIes);

	qdf_mem_free(pResult);
}

static QDF_STATUS csr_ll_scan_purge_result(struct mac_context *mac,
					   tDblLinkList *pList)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry;
	struct tag_csrscan_result *bss_desc;

	while ((pEntry = csr_ll_remove_head(pList, LL_ACCESS_NOLOCK)) != NULL) {
		bss_desc = GET_BASE_ADDR(pEntry, struct tag_csrscan_result,
					 Link);
		csr_free_scan_result_entry(mac, bss_desc);
	}

	return status;
}

QDF_STATUS csr_scan_open(struct mac_context *mac_ctx)
{
	csr_ll_open(&mac_ctx->scan.channelPowerInfoList24);
	csr_ll_open(&mac_ctx->scan.channelPowerInfoList5G);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_close(struct mac_context *mac)
{
	csr_purge_channel_power(mac, &mac->scan.channelPowerInfoList24);
	csr_purge_channel_power(mac, &mac->scan.channelPowerInfoList5G);
	csr_ll_close(&mac->scan.channelPowerInfoList24);
	csr_ll_close(&mac->scan.channelPowerInfoList5G);
	wlan_scan_psoc_set_disable(mac->psoc, REASON_SYSTEM_DOWN);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_result_purge(struct mac_context *mac,
				 tScanResultHandle hScanList)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct scan_result_list *pScanList =
				(struct scan_result_list *) hScanList;

	if (pScanList) {
		status = csr_ll_scan_purge_result(mac, &pScanList->List);
		csr_ll_close(&pScanList->List);
		qdf_mem_free(pScanList);
	}
	return status;
}

static void csr_purge_channel_power(struct mac_context *mac,
				    tDblLinkList *pChannelList)
{
	struct csr_channel_powerinfo *pChannelSet;
	tListElem *pEntry;

	/*
	 * Remove the channel sets from the learned list and put them
	 * in the free list
	 */
	csr_ll_lock(pChannelList);
	while ((pEntry = csr_ll_remove_head(pChannelList,
					    LL_ACCESS_NOLOCK)) != NULL) {
		pChannelSet = GET_BASE_ADDR(pEntry,
					struct csr_channel_powerinfo, link);
		if (pChannelSet)
			qdf_mem_free(pChannelSet);
	}
	csr_ll_unlock(pChannelList);
}

/*
 * Save the channelList into the ultimate storage as the final stage of channel
 * Input: pCountryInfo -- the country code (e.g. "USI"), channel list, and power
 * limit are all stored inside this data structure
 */
QDF_STATUS csr_save_to_channel_power2_g_5_g(struct mac_context *mac,
					    uint32_t tableSize,
					    struct pwr_channel_info *channelTable)
{
	uint32_t i = tableSize / sizeof(struct pwr_channel_info);
	struct pwr_channel_info *pChannelInfo;
	struct csr_channel_powerinfo *pChannelSet;
	bool f2GHzInfoFound = false;
	bool f2GListPurged = false, f5GListPurged = false;

	pChannelInfo = channelTable;
	/* atleast 3 bytes have to be remaining  -- from "countryString" */
	while (i--) {
	pChannelSet = qdf_mem_malloc(sizeof(struct csr_channel_powerinfo));
		if (!pChannelSet) {
			pChannelInfo++;
			continue;
		}
		pChannelSet->first_chan_freq = pChannelInfo->first_freq;
		pChannelSet->numChannels = pChannelInfo->num_chan;
		/*
		 * Now set the inter-channel offset based on the frequency band
		 * the channel set lies in
		 */
		if (WLAN_REG_IS_24GHZ_CH_FREQ(pChannelSet->first_chan_freq) &&
		    (pChannelSet->first_chan_freq + 5 * (pChannelSet->numChannels - 1) <=
		     WLAN_REG_MAX_24GHZ_CHAN_FREQ)) {
			pChannelSet->interChannelOffset = 5;
			f2GHzInfoFound = true;
		} else if (WLAN_REG_IS_5GHZ_CH_FREQ(pChannelSet->first_chan_freq) &&
			   (pChannelSet->first_chan_freq + 20 * (pChannelSet->numChannels - 1) <=
			   WLAN_REG_MAX_5GHZ_CHAN_FREQ)) {
			pChannelSet->interChannelOffset = 20;
			f2GHzInfoFound = false;
		} else {
			sme_warn("Invalid Channel freq %d Present in Country IE",
				 pChannelSet->first_chan_freq);
			qdf_mem_free(pChannelSet);
			return QDF_STATUS_E_FAILURE;
		}
		pChannelSet->txPower = pChannelInfo->max_tx_pwr;
		if (f2GHzInfoFound) {
			if (!f2GListPurged) {
				/* purge previous results if found new */
				csr_purge_channel_power(mac,
							&mac->scan.
							channelPowerInfoList24);
				f2GListPurged = true;
			}
			if (CSR_IS_OPERATING_BG_BAND(mac)) {
				/* add to the list of 2.4 GHz channel sets */
				csr_ll_insert_tail(&mac->scan.
						   channelPowerInfoList24,
						   &pChannelSet->link,
						   LL_ACCESS_LOCK);
			} else {
				sme_debug(
					"Adding 11B/G ch in 11A. 1st ch freq %d",
					pChannelSet->first_chan_freq);
				qdf_mem_free(pChannelSet);
			}
		} else {
			/* 5GHz info found */
			if (!f5GListPurged) {
				/* purge previous results if found new */
				csr_purge_channel_power(mac,
							&mac->scan.
							channelPowerInfoList5G);
				f5GListPurged = true;
			}
			if (CSR_IS_OPERATING_A_BAND(mac)) {
				/* add to the list of 5GHz channel sets */
				csr_ll_insert_tail(&mac->scan.
						   channelPowerInfoList5G,
						   &pChannelSet->link,
						   LL_ACCESS_LOCK);
			} else {
				sme_debug(
					"Adding 11A ch in B/G. 1st ch freq %d",
					pChannelSet->first_chan_freq);
				qdf_mem_free(pChannelSet);
			}
		}
		pChannelInfo++; /* move to next entry */
	}
	return QDF_STATUS_SUCCESS;
}

void csr_apply_power2_current(struct mac_context *mac)
{
	sme_debug("Updating Cfg with power settings");
	csr_save_tx_power_to_cfg(mac, &mac->scan.channelPowerInfoList24,
				 BAND_2G);
	csr_save_tx_power_to_cfg(mac, &mac->scan.channelPowerInfoList5G,
				 BAND_5G);
}

void csr_apply_channel_power_info_to_fw(struct mac_context *mac_ctx,
					struct csr_channel *ch_lst)
{
	int i;
	uint8_t num_ch = 0;
	uint8_t tempNumChannels = 0;
	struct csr_channel tmp_ch_lst;

	if (ch_lst->numChannels) {
		tempNumChannels = QDF_MIN(ch_lst->numChannels,
					  CFG_VALID_CHANNEL_LIST_LEN);
		for (i = 0; i < tempNumChannels; i++) {
			tmp_ch_lst.channel_freq_list[num_ch] = ch_lst->channel_freq_list[i];
			num_ch++;
		}
		tmp_ch_lst.numChannels = num_ch;
		/* Store the channel+power info in the global place: Cfg */
		csr_apply_power2_current(mac_ctx);
		csr_set_cfg_valid_channel_list(mac_ctx, tmp_ch_lst.channel_freq_list,
					       tmp_ch_lst.numChannels);
	} else {
		sme_err("11D channel list is empty");
	}
	sch_edca_profile_update_all(mac_ctx);
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void csr_diag_reset_country_information(struct mac_context *mac)
{

	host_log_802_11d_pkt_type *p11dLog;
	int Index;
	uint8_t reg_cc[REG_ALPHA2_LEN + 1];

	WLAN_HOST_DIAG_LOG_ALLOC(p11dLog, host_log_802_11d_pkt_type,
				 LOG_WLAN_80211D_C);
	if (!p11dLog)
		return;

	p11dLog->eventId = WLAN_80211D_EVENT_RESET;
	wlan_reg_read_current_country(mac->psoc, reg_cc);
	qdf_mem_copy(p11dLog->countryCode, reg_cc, 3);
	p11dLog->numChannel = mac->scan.base_channels.numChannels;
	if (p11dLog->numChannel <= HOST_LOG_MAX_NUM_CHANNEL) {
		for (Index = 0;
		     Index < mac->scan.base_channels.numChannels;
		     Index++) {
			p11dLog->Channels[Index] =
				wlan_reg_freq_to_chan(mac->pdev, mac->scan.base_channels.channel_freq_list[Index]);
			p11dLog->TxPwr[Index] =
				mac->scan.defaultPowerTable[Index].tx_power;
		}
	}

	WLAN_HOST_DIAG_LOG_REPORT(p11dLog);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

/**
 * csr_apply_channel_power_info_wrapper() - sends channel info to fw
 * @mac: main MAC data structure
 *
 * This function sends the channel power info to firmware
 *
 * Return: none
 */
void csr_apply_channel_power_info_wrapper(struct mac_context *mac)
{

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_diag_reset_country_information(mac);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	csr_prune_channel_list_for_mode(mac, &mac->scan.base_channels);
	csr_save_channel_power_for_band(mac, false);
	csr_save_channel_power_for_band(mac, true);
	/* apply the channel list, power settings, and the country code. */
	csr_apply_channel_power_info_to_fw(mac, &mac->scan.base_channels);
	/* clear the 11d channel list */
	qdf_mem_zero(&mac->scan.channels11d, sizeof(mac->scan.channels11d));
}

void csr_save_channel_power_for_band(struct mac_context *mac, bool fill_5f)
{
	uint32_t idx, count = 0;
	struct pwr_channel_info *chan_info;
	struct pwr_channel_info *ch_info_start;
	int32_t max_ch_idx;
	bool tmp_bool;
	uint32_t ch_freq = 0;

	max_ch_idx =
		(mac->scan.base_channels.numChannels <
		CFG_VALID_CHANNEL_LIST_LEN) ?
		mac->scan.base_channels.numChannels :
		CFG_VALID_CHANNEL_LIST_LEN;

	chan_info = qdf_mem_malloc(sizeof(struct pwr_channel_info) *
				   CFG_VALID_CHANNEL_LIST_LEN);
	if (!chan_info)
		return;

	ch_info_start = chan_info;
	for (idx = 0; idx < max_ch_idx; idx++) {
		ch_freq = mac->scan.defaultPowerTable[idx].center_freq;
		tmp_bool = (fill_5f && WLAN_REG_IS_5GHZ_CH_FREQ(ch_freq)) ||
			(!fill_5f && WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq));
		if (!tmp_bool)
			continue;

		if (count >= CFG_VALID_CHANNEL_LIST_LEN) {
			sme_warn("count: %d exceeded", count);
			break;
		}

		chan_info->first_freq =
			mac->scan.defaultPowerTable[idx].center_freq;
		chan_info->num_chan = 1;
		chan_info->max_tx_pwr =
			mac->scan.defaultPowerTable[idx].tx_power;
		chan_info++;
		count++;
	}
	if (count) {
		csr_save_to_channel_power2_g_5_g(mac,
				count * sizeof(struct pwr_channel_info),
				ch_info_start);
	}
	qdf_mem_free(ch_info_start);
}

bool csr_is_supported_channel(struct mac_context *mac, uint32_t chan_freq)
{
	bool fRet = false;
	uint32_t i;

	for (i = 0; i < mac->scan.base_channels.numChannels; i++) {
		if (chan_freq == mac->scan.base_channels.channel_freq_list[i]) {
			fRet = true;
			break;
		}
	}

	return fRet;
}

tCsrScanResultInfo *csr_scan_result_get_first(struct mac_context *mac,
					      tScanResultHandle hScanResult)
{
	tListElem *pEntry;
	struct tag_csrscan_result *pResult;
	tCsrScanResultInfo *pRet = NULL;
	struct scan_result_list *pResultList =
				(struct scan_result_list *) hScanResult;

	if (pResultList) {
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
		if (pEntry) {
			pResult = GET_BASE_ADDR(pEntry, struct
						tag_csrscan_result, Link);
			pRet = &pResult->Result;
		}
		pResultList->pCurEntry = pEntry;
	}

	return pRet;
}

tCsrScanResultInfo *csr_scan_result_get_next(struct mac_context *mac,
					     tScanResultHandle hScanResult)
{
	tListElem *pEntry = NULL;
	struct tag_csrscan_result *pResult = NULL;
	tCsrScanResultInfo *pRet = NULL;
	struct scan_result_list *pResultList =
				(struct scan_result_list *) hScanResult;

	if (!pResultList)
		return NULL;

	if (!pResultList->pCurEntry)
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
	else
		pEntry = csr_ll_next(&pResultList->List, pResultList->pCurEntry,
				     LL_ACCESS_NOLOCK);

	if (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, struct tag_csrscan_result,
					Link);
		pRet = &pResult->Result;
	}
	pResultList->pCurEntry = pEntry;

	return pRet;
}

static void csr_set_cfg_valid_channel_list(struct mac_context *mac,
					   uint32_t *pchan_freq_list,
					   uint8_t NumChannels)
{
	QDF_STATUS status;
	uint8_t i;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: dump valid channel list(NumChannels(%d))",
		  __func__, NumChannels);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			   pchan_freq_list, NumChannels);
	for (i = 0; i < NumChannels; i++) {
		mac->mlme_cfg->reg.valid_channel_freq_list[i] = pchan_freq_list[i];
	}

	mac->mlme_cfg->reg.valid_channel_list_num = NumChannels;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "Scan offload is enabled, update default chan list");
	/*
	 * disable fcc constraint since new country code
	 * is being set
	 */
	mac->scan.fcc_constraint = false;
	status = csr_update_channel_list(mac);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "failed to update the supported channel list");
	}
}

/*
 * The Tx power limits are saved in the cfg for future usage.
 */
static void csr_save_tx_power_to_cfg(struct mac_context *mac,
				     tDblLinkList *pList,
				     enum band_info band)
{
	tListElem *pEntry;
	uint32_t cbLen = 0, dataLen, tmp_len;
	struct csr_channel_powerinfo *ch_set;
	uint32_t idx, count = 0;
	struct pwr_channel_info *ch_pwr_set;
	uint8_t *p_buf = NULL;

	/* allocate maximum space for all channels */
	dataLen = CFG_VALID_CHANNEL_LIST_LEN * sizeof(struct pwr_channel_info);
	p_buf = qdf_mem_malloc(dataLen);
	if (!p_buf)
		return;

	ch_pwr_set = (struct pwr_channel_info *)(p_buf);
	csr_ll_lock(pList);
	pEntry = csr_ll_peek_head(pList, LL_ACCESS_NOLOCK);
	/*
	 * write the tuples (startChan, numChan, txPower) for each channel found
	 * in the channel power list.
	 */
	while (pEntry) {
		ch_set = GET_BASE_ADDR(pEntry,
				struct csr_channel_powerinfo, link);
		if (ch_set->interChannelOffset != 5) {
			/*
			 * we keep the 5G channel sets internally with an
			 * interchannel offset of 4. Expand these to the right
			 * format. (inter channel offset of 1 is the only option
			 * for the triplets that 11d advertises.
			 */
			tmp_len = cbLen + (ch_set->numChannels *
						sizeof(struct pwr_channel_info));
			if (tmp_len >= dataLen) {
				/*
				 * expanding this entry will overflow our
				 * allocation
				 */
				sme_err(
					"Buffer overflow, start freq %d, num %d, offset %d",
					ch_set->first_chan_freq,
					ch_set->numChannels,
					ch_set->interChannelOffset);
				break;
			}

			for (idx = 0; idx < ch_set->numChannels; idx++) {
				ch_pwr_set->first_freq =
					ch_set->first_chan_freq;
				ch_pwr_set->num_chan = 1;
				ch_pwr_set->max_tx_pwr = ch_set->txPower;
				cbLen += sizeof(struct pwr_channel_info);
				ch_pwr_set++;
				count++;
			}
		} else {
			if (cbLen + sizeof(struct pwr_channel_info) >= dataLen) {
				/* this entry will overflow our allocation */
				sme_err(
					"Buffer overflow, start freq %d, num %d, offset %d",
					ch_set->first_chan_freq,
					ch_set->numChannels,
					ch_set->interChannelOffset);
				break;
			}
			ch_pwr_set->first_freq = ch_set->first_chan_freq;
			ch_pwr_set->num_chan = ch_set->numChannels;
			ch_pwr_set->max_tx_pwr = ch_set->txPower;
			cbLen += sizeof(struct pwr_channel_info);
			ch_pwr_set++;
			count++;
		}
		pEntry = csr_ll_next(pList, pEntry, LL_ACCESS_NOLOCK);
	}
	csr_ll_unlock(pList);
	if (band == BAND_2G) {
		mac->mlme_cfg->power.max_tx_power_24.len =
					sizeof(struct pwr_channel_info) * count;
		if (mac->mlme_cfg->power.max_tx_power_24.len >
						CFG_MAX_TX_POWER_2_4_LEN)
			mac->mlme_cfg->power.max_tx_power_24.len =
						CFG_MAX_TX_POWER_2_4_LEN;
		qdf_mem_copy(mac->mlme_cfg->power.max_tx_power_24.data,
			     (uint8_t *)p_buf,
			     mac->mlme_cfg->power.max_tx_power_24.len);
	}
	if (band == BAND_5G) {
		mac->mlme_cfg->power.max_tx_power_5.len =
					sizeof(struct pwr_channel_info) * count;
		if (mac->mlme_cfg->power.max_tx_power_5.len >
							CFG_MAX_TX_POWER_5_LEN)
			mac->mlme_cfg->power.max_tx_power_5.len =
							CFG_MAX_TX_POWER_5_LEN;
		qdf_mem_copy(mac->mlme_cfg->power.max_tx_power_5.data,
			     (uint8_t *)p_buf,
			     mac->mlme_cfg->power.max_tx_power_5.len);
	}
	qdf_mem_free(p_buf);
}

static void csr_fill_rsn_auth_type(enum csr_akm_type *auth_type, uint32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384))
		*auth_type = eCSR_AUTH_TYPE_FT_FILS_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256))
		*auth_type = eCSR_AUTH_TYPE_FT_FILS_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA384))
		*auth_type = eCSR_AUTH_TYPE_FILS_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FILS_SHA256))
		*auth_type = eCSR_AUTH_TYPE_FILS_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE))
		*auth_type = eCSR_AUTH_TYPE_FT_SAE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE))
		*auth_type = eCSR_AUTH_TYPE_SAE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_DPP))
		*auth_type = eCSR_AUTH_TYPE_DPP_RSN;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OSEN))
		*auth_type = eCSR_AUTH_TYPE_OSEN;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_OWE))
		*auth_type = eCSR_AUTH_TYPE_OWE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X))
		*auth_type = eCSR_AUTH_TYPE_FT_RSN;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_PSK))
		*auth_type = eCSR_AUTH_TYPE_FT_RSN_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		*auth_type = eCSR_AUTH_TYPE_RSN;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		*auth_type = eCSR_AUTH_TYPE_RSN_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		*auth_type = eCSR_AUTH_TYPE_CCKM_RSN;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK_SHA256))
		*auth_type = eCSR_AUTH_TYPE_RSN_PSK_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SHA256))
		*auth_type = eCSR_AUTH_TYPE_RSN_8021X_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B))
		*auth_type = eCSR_AUTH_TYPE_SUITEB_EAP_SHA256;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X_SUITE_B_192))
		*auth_type = eCSR_AUTH_TYPE_SUITEB_EAP_SHA384;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384))
		*auth_type = eCSR_AUTH_TYPE_FT_SUITEB_EAP_SHA384;
	else
		*auth_type = eCSR_AUTH_TYPE_NONE;
}

static void csr_fill_wpa_auth_type(enum csr_akm_type *auth_type, uint32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_IEEE8021X))
		*auth_type = eCSR_AUTH_TYPE_WPA;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK))
		*auth_type = eCSR_AUTH_TYPE_WPA_PSK;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM))
		*auth_type = eCSR_AUTH_TYPE_CCKM_WPA;
	else
		*auth_type = eCSR_AUTH_TYPE_WPA_NONE;
}

static void csr_fill_wapi_auth_type(enum csr_akm_type *auth_type, uint32_t akm)
{
	/* Try the more preferred ones first. */
	if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_CERT))
		*auth_type = eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	else if (QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_WAPI_PSK))
		*auth_type = eCSR_AUTH_TYPE_WAPI_WAI_PSK;
	else
		*auth_type = eCSR_AUTH_TYPE_NONE;
}

void csr_fill_auth_type(enum csr_akm_type *auth_type,
			uint32_t authmodeset, uint32_t akm,
			uint32_t ucastcipherset)
{
	if (!authmodeset) {
		*auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_NONE) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_OPEN)) {
		*auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_AUTO)) {
		if ((QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_40) ||
		     QDF_HAS_PARAM(ucastcipherset, WLAN_CRYPTO_CIPHER_WEP_104)))
			*auth_type = eCSR_AUTH_TYPE_AUTOSWITCH;
		else
			*auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;

		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SHARED)) {
		*auth_type = eCSR_AUTH_TYPE_SHARED_KEY;
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_8021X) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_RSNA) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_CCKM) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_SAE) ||
	    QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_FILS_SK)) {
		csr_fill_rsn_auth_type(auth_type, akm);
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WPA)) {
		csr_fill_wpa_auth_type(auth_type, akm);
		return;
	}

	if (QDF_HAS_PARAM(authmodeset, WLAN_CRYPTO_AUTH_WAPI)) {
		csr_fill_wapi_auth_type(auth_type, akm);
		return;
	}

	*auth_type = eCSR_AUTH_TYPE_OPEN_SYSTEM;
}

void csr_fill_enc_type(eCsrEncryptionType *cipher_type, uint32_t cipherset)
{
	if (!cipherset) {
		*cipher_type = eCSR_ENCRYPT_TYPE_NONE;
		return;
	}
	if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM_256))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES_GCMP_256;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GCM))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES_GCMP;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_OCB) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CCM_256))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_TKIP))
		*cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_CMAC_256))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES_CMAC;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_GCM4) ||
		 QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WAPI_SMS4))
		*cipher_type = eCSR_ENCRYPT_TYPE_WPI;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES_GMAC_128;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_AES_GMAC_256))
		*cipher_type = eCSR_ENCRYPT_TYPE_AES_GMAC_256;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP))
		*cipher_type = eCSR_ENCRYPT_TYPE_WEP40;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_40))
		*cipher_type = eCSR_ENCRYPT_TYPE_WEP40;
	else if (QDF_HAS_PARAM(cipherset, WLAN_CRYPTO_CIPHER_WEP_104))
		*cipher_type = eCSR_ENCRYPT_TYPE_WEP104;
	else
		*cipher_type = eCSR_ENCRYPT_TYPE_NONE;
}

static void csr_fill_neg_crypto_info(struct tag_csrscan_result *bss,
				     struct security_info *sec_info)
{
	if (!sec_info->authmodeset && !sec_info->key_mgmt &&
	    !sec_info->ucastcipherset)
		return;

	csr_fill_enc_type(&bss->ucEncryptionType, sec_info->ucastcipherset);
	csr_fill_enc_type(&bss->mcEncryptionType, sec_info->mcastcipherset);
	csr_fill_auth_type(&bss->authType, sec_info->authmodeset,
			   sec_info->key_mgmt, sec_info->ucastcipherset);
	sme_debug("Authmode %x, AKM %x, Cipher Uc %x Mc %x CSR: Auth %d, Cipher Uc %d Mc %d",
		  sec_info->authmodeset, sec_info->key_mgmt,
		  sec_info->ucastcipherset, sec_info->mcastcipherset,
		  bss->authType, bss->ucEncryptionType, bss->mcEncryptionType);
}

static QDF_STATUS csr_fill_bss_from_scan_entry(struct mac_context *mac_ctx,
					struct scan_cache_entry *scan_entry,
					struct tag_csrscan_result **p_result)
{
	tDot11fBeaconIEs *bcn_ies;
	struct bss_description *bss_desc;
	tCsrScanResultInfo *result_info;
	uint8_t *ie_ptr;
	struct tag_csrscan_result *bss;
	uint32_t bss_len, alloc_len, ie_len;
	QDF_STATUS status;
	enum channel_state ap_channel_state;

	ap_channel_state =
		wlan_reg_get_channel_state_for_freq(
				mac_ctx->pdev,
				scan_entry->channel.chan_freq);
	if (ap_channel_state == CHANNEL_STATE_DISABLE ||
	    ap_channel_state == CHANNEL_STATE_INVALID) {
		sme_err("BSS "QDF_MAC_ADDR_FMT" channel %d invalid, not populating this BSSID",
			QDF_MAC_ADDR_REF(scan_entry->bssid.bytes),
			scan_entry->channel.chan_freq);
		return QDF_STATUS_E_INVAL;
	}

	ie_len = util_scan_entry_ie_len(scan_entry);
	ie_ptr = util_scan_entry_ie_data(scan_entry);

	bss_len = (uint16_t)(offsetof(struct bss_description,
			   ieFields[0]) + ie_len);
	alloc_len = sizeof(struct tag_csrscan_result) + bss_len;
	bss = qdf_mem_malloc(alloc_len);
	if (!bss)
		return QDF_STATUS_E_NOMEM;

	csr_fill_neg_crypto_info(bss, &scan_entry->neg_sec_info);
	bss->bss_score = scan_entry->bss_score;

	result_info = &bss->Result;
	result_info->ssId.length = scan_entry->ssid.length;
	qdf_mem_copy(result_info->ssId.ssId,
		scan_entry->ssid.ssid,
		result_info->ssId.length);
	result_info->timer = scan_entry->hidden_ssid_timestamp;

	bss_desc = &result_info->BssDescriptor;

	wlan_fill_bss_desc_from_scan_entry(mac_ctx, bss_desc, scan_entry);

	status = wlan_get_parsed_bss_description_ies(mac_ctx, bss_desc,
						     &bcn_ies);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(bss);
		return status;
	}
	result_info->pvIes = bcn_ies;

	*p_result = bss;
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS csr_parse_scan_list(struct mac_context *mac_ctx,
				      struct scan_result_list *ret_list,
				      qdf_list_t *scan_list)
{
	struct tag_csrscan_result *pResult = NULL;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;

	qdf_list_peek_front(scan_list, (qdf_list_node_t **) &cur_node);

	while (cur_node) {
		qdf_list_peek_next(scan_list, (qdf_list_node_t *) cur_node,
				  (qdf_list_node_t **) &next_node);
		pResult = NULL;
		csr_fill_bss_from_scan_entry(mac_ctx,
					     cur_node->entry, &pResult);
		if (pResult)
			csr_ll_insert_tail(&ret_list->List, &pResult->Link,
					   LL_ACCESS_NOLOCK);
		cur_node = next_node;
		next_node = NULL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_get_result(struct mac_context *mac_ctx,
			       struct scan_filter *filter,
			       tScanResultHandle *results)
{
	QDF_STATUS status;
	struct scan_result_list *ret_list = NULL;
	qdf_list_t *list = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint32_t num_bss = 0;

	if (results)
		*results = CSR_INVALID_SCANRESULT_HANDLE;

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);
	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	list = ucfg_scan_get_result(pdev, filter);
	if (list) {
		num_bss = qdf_list_size(list);
		sme_debug("num_entries %d", num_bss);
	}

	if (!list || (list && !qdf_list_size(list))) {
		sme_debug("scan list empty");
		if (num_bss)
			status = QDF_STATUS_E_EXISTS;
		else
			status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	ret_list = qdf_mem_malloc(sizeof(struct scan_result_list));
	if (!ret_list) {
		status = QDF_STATUS_E_NOMEM;
		goto error;
	}

	csr_ll_open(&ret_list->List);
	ret_list->pCurEntry = NULL;
	status = csr_parse_scan_list(mac_ctx, ret_list, list);
	if (QDF_IS_STATUS_ERROR(status) || !results)
		/* Fail or No one wants the result. */
		csr_scan_result_purge(mac_ctx, (tScanResultHandle) ret_list);
	else {
		if (!csr_ll_count(&ret_list->List)) {
			/* This mean that there is no match */
			csr_ll_close(&ret_list->List);
			qdf_mem_free(ret_list);
			/*
			 * Do not trigger scan for ssid if the scan entries
			 * are removed either due to rssi reject or assoc
			 * disallowed.
			 */
			if (num_bss)
				status = QDF_STATUS_E_EXISTS;
			else
				status = QDF_STATUS_E_NULL_VALUE;
		} else if (results) {
			*results = ret_list;
		}
	}

error:
	if (list)
		ucfg_scan_purge_results(list);
	if (pdev)
		wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);

	return status;
}

QDF_STATUS csr_scan_get_result_for_bssid(struct mac_context *mac_ctx,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct scan_filter *scan_filter;
	tScanResultHandle filtered_scan_result = NULL;
	tCsrScanResultInfo *scan_result;

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("mac_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	scan_filter = qdf_mem_malloc(sizeof(*scan_filter));
	if (!scan_filter)
		return QDF_STATUS_E_NOMEM;

	scan_filter->num_of_bssid = 1;
	qdf_mem_copy(scan_filter->bssid_list[0].bytes, bssid->bytes,
		     QDF_MAC_ADDR_SIZE);

	status = csr_scan_get_result(mac_ctx, scan_filter,
				&filtered_scan_result);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to get scan result");
		goto free_filter;
	}

	scan_result = csr_scan_result_get_first(mac_ctx, filtered_scan_result);

	if (scan_result) {
		res->pvIes = NULL;
		res->ssId.length = scan_result->ssId.length;
		qdf_mem_copy(&res->ssId.ssId, &scan_result->ssId.ssId,
			res->ssId.length);
		res->timer = scan_result->timer;
		qdf_mem_copy(&res->BssDescriptor, &scan_result->BssDescriptor,
			sizeof(struct bss_description));
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}

	csr_scan_result_purge(mac_ctx, filtered_scan_result);

free_filter:
	if (scan_filter)
		qdf_mem_free(scan_filter);

	return status;
}

static inline QDF_STATUS
csr_flush_scan_results(struct mac_context *mac_ctx,
		       struct scan_filter *filter)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	QDF_STATUS status;

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);
	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	status = ucfg_scan_flush_results(pdev, filter);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
	return status;
}

static inline void csr_flush_bssid(struct mac_context *mac_ctx,
				   uint8_t *bssid)
{
	struct scan_filter *filter;

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		return;

	filter->num_of_bssid = 1;
	qdf_mem_copy(filter->bssid_list[0].bytes,
		     bssid, QDF_MAC_ADDR_SIZE);

	csr_flush_scan_results(mac_ctx, filter);
	sme_debug("Removed BSS entry:"QDF_MAC_ADDR_FMT,
		   QDF_MAC_ADDR_REF(bssid));
	if (filter)
		qdf_mem_free(filter);
}

void csr_remove_bssid_from_scan_list(struct mac_context *mac_ctx,
				     tSirMacAddr bssid)
{
	csr_flush_bssid(mac_ctx, bssid);
}

/**
 * csr_scan_filter_results: filter scan result based
 * on valid channel list number.
 * @mac_ctx: mac context
 *
 * Get scan result from scan list and Check Scan result channel number
 * with 11d channel list if channel number is found in 11d channel list
 * then do not remove scan result entry from scan list
 *
 * return: QDF Status
 */
QDF_STATUS csr_scan_filter_results(struct mac_context *mac_ctx)
{
	uint32_t len = mac_ctx->mlme_cfg->reg.valid_channel_list_num;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint32_t i, valid_chan_len = 0;
	uint32_t ch_freq;
	uint32_t valid_ch_freq_list[CFG_VALID_CHANNEL_LIST_LEN];

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);
	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* This is a temporary conversion till the scm handles freq */
	for (i = 0; i < len; i++) {
		if (wlan_reg_is_dsrc_freq(
			mac_ctx->mlme_cfg->reg.valid_channel_freq_list[i]))
			continue;
		ch_freq = mac_ctx->mlme_cfg->reg.valid_channel_freq_list[i];
		valid_ch_freq_list[valid_chan_len++] = ch_freq;
	}
	sme_debug("No of valid channel %d", valid_chan_len);

	ucfg_scan_filter_valid_channel(pdev, valid_ch_freq_list,
				       valid_chan_len);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);

	return QDF_STATUS_SUCCESS;
}

void csr_update_beacon(struct mac_context *mac)
{
	struct scheduler_msg msg = { 0 };
	QDF_STATUS status;

	msg.type = SIR_LIM_UPDATE_BEACON;
	status = scheduler_post_message(QDF_MODULE_ID_SME, QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (status != QDF_STATUS_SUCCESS)
		sme_err("scheduler_post_message failed, status = %u", status);
}
