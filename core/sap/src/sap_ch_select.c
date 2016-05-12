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

/*===========================================================================

			s a p C h S e l e c t . C
   OVERVIEW:

   This software unit holds the implementation of the WLAN SAP modules
   functions for channel selection.

   DEPENDENCIES:

   Are listed for each API below.
   ===========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "qdf_trace.h"
#include "csr_api.h"
#include "sme_api.h"
#include "sap_ch_select.h"
#include "sap_internal.h"
#ifdef ANI_OS_TYPE_QNX
#include "stdio.h"
#endif
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
#include "lim_utils.h"
#include "parser_api.h"
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

#include "pld_common.h"

/*--------------------------------------------------------------------------
   Function definitions
   --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Defines
   --------------------------------------------------------------------------*/
#define SAP_DEBUG

#define IS_RSSI_VALID(extRssi, rssi) \
	( \
		((extRssi < rssi) ? true : false) \
	)

#define SET_ACS_BAND(acs_band, pSapCtx) \
{ \
	if (pSapCtx->acs_cfg->start_ch <= 14 && \
		pSapCtx->acs_cfg->end_ch <= 14) \
		acs_band = eCSR_DOT11_MODE_11g; \
	else if (pSapCtx->acs_cfg->start_ch >= 14)\
		acs_band = eCSR_DOT11_MODE_11a; \
	else \
		acs_band = eCSR_DOT11_MODE_abg; \
}

#ifdef FEATURE_WLAN_CH_AVOID
sapSafeChannelType safe_channels[NUM_CHANNELS] = {
	{1, true},
	{2, true},
	{3, true},
	{4, true},
	{5, true},
	{6, true},
	{7, true},
	{8, true},
	{9, true},
	{10, true},
	{11, true},
	{12, true},
	{13, true},
	{14, true},
	{36, true},
	{40, true},
	{44, true},
	{48, true},
	{52, true},
	{56, true},
	{60, true},
	{64, true},
	{100, true},
	{104, true},
	{108, true},
	{112, true},
	{116, true},
	{120, true},
	{124, true},
	{128, true},
	{132, true},
	{136, true},
	{140, true},
	{144, true},
	{149, true},
	{153, true},
	{157, true},
	{161, true},
	{165, true},
};
#endif

typedef struct {
	uint16_t chStartNum;
	uint32_t weight;
} sapAcsChannelInfo;

sapAcsChannelInfo acs_ht40_channels5_g[] = {
	{36, SAP_ACS_WEIGHT_MAX},
	{44, SAP_ACS_WEIGHT_MAX},
	{52, SAP_ACS_WEIGHT_MAX},
	{60, SAP_ACS_WEIGHT_MAX},
	{100, SAP_ACS_WEIGHT_MAX},
	{108, SAP_ACS_WEIGHT_MAX},
	{116, SAP_ACS_WEIGHT_MAX},
	{124, SAP_ACS_WEIGHT_MAX},
	{132, SAP_ACS_WEIGHT_MAX},
	{140, SAP_ACS_WEIGHT_MAX},
	{149, SAP_ACS_WEIGHT_MAX},
	{157, SAP_ACS_WEIGHT_MAX},
};

sapAcsChannelInfo acs_ht80_channels[] = {
	{36, SAP_ACS_WEIGHT_MAX},
	{52, SAP_ACS_WEIGHT_MAX},
	{100, SAP_ACS_WEIGHT_MAX},
	{116, SAP_ACS_WEIGHT_MAX},
	{132, SAP_ACS_WEIGHT_MAX},
	{149, SAP_ACS_WEIGHT_MAX},
};

sapAcsChannelInfo acs_vht160_channels[] = {
	{36, SAP_ACS_WEIGHT_MAX},
	{100, SAP_ACS_WEIGHT_MAX},
};

sapAcsChannelInfo acs_ht40_channels24_g[] = {
	{1, SAP_ACS_WEIGHT_MAX},
	{2, SAP_ACS_WEIGHT_MAX},
	{3, SAP_ACS_WEIGHT_MAX},
	{4, SAP_ACS_WEIGHT_MAX},
	{9, SAP_ACS_WEIGHT_MAX},
};

#define CHANNEL_165  165

/* rssi discount for channels in PCL */
#define PCL_RSSI_DISCOUNT 10

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * sap_check_n_add_channel() - checks and add given channel in sap context's
 * avoid_channels_info struct
 * @sap_ctx:           sap context.
 * @new_channel:       channel to be added to sap_ctx's avoid ch info
 *
 * sap_ctx contains sap_avoid_ch_info strcut containing the list of channels on
 * which MDM device's AP with MCC was detected. This function will add channels
 * to that list after checking for duplicates.
 *
 * Return: true: if channel was added or already present
 *   else false: if channel list was already full.
 */
bool
sap_check_n_add_channel(ptSapContext sap_ctx,
			uint8_t new_channel)
{
	uint8_t i = 0;
	struct sap_avoid_channels_info *ie_info =
		&sap_ctx->sap_detected_avoid_ch_ie;

	for (i = 0; i < sizeof(ie_info->channels); i++) {
		if (ie_info->channels[i] == new_channel)
			break;

		if (ie_info->channels[i] == 0) {
			ie_info->channels[i] = new_channel;
			break;
		}
	}
	if (i == sizeof(ie_info->channels))
		return false;
	else
		return true;
}
/**
 * sap_check_n_add_overlapped_chnls() - checks & add overlapped channels
 *                                      to primary channel in 2.4Ghz band.
 * @sap_ctx:           sap context.
 * @primary_chnl:      primary channel to be avoided.
 *
 * sap_ctx contains sap_avoid_ch_info struct containing the list of channels on
 * which MDM device's AP with MCC was detected. This function will add channels
 * to that list after checking for duplicates.
 *
 * Return: true: if channel was added or already present
 *   else false: if channel list was already full.
 */
static bool
sap_check_n_add_overlapped_chnls(ptSapContext sap_ctx, uint8_t primary_channel)
{
	uint8_t i = 0, j = 0, upper_chnl = 0, lower_chnl = 0;
	struct sap_avoid_channels_info *ie_info =
		&sap_ctx->sap_detected_avoid_ch_ie;
	/*
	 * if primary channel less than channel 1 or out of 2g band then
	 * no further process is required. return true in this case.
	 */
	if (primary_channel < CHANNEL_1 || primary_channel > CHANNEL_14)
		return true;

	/* lower channel is one channel right before primary channel */
	lower_chnl = primary_channel - 1;
	/* upper channel is one channel right after primary channel */
	upper_chnl = primary_channel + 1;

	/* lower channel needs to be non-zero, zero is not valid channel */
	if (lower_chnl > (CHANNEL_1 - 1)) {
		for (i = 0; i < sizeof(ie_info->channels); i++) {
			if (ie_info->channels[i] == lower_chnl)
				break;
			if (ie_info->channels[i] == 0) {
				ie_info->channels[i] = lower_chnl;
				break;
			}
		}
	}
	/* upper channel needs to be atleast last channel in 2.4Ghz band */
	if (upper_chnl < (CHANNEL_14 + 1)) {
		for (j = 0; j < sizeof(ie_info->channels); j++) {
			if (ie_info->channels[j] == upper_chnl)
				break;
			if (ie_info->channels[j] == 0) {
				ie_info->channels[j] = upper_chnl;
				break;
			}
		}
	}
	if (i == sizeof(ie_info->channels) || j == sizeof(ie_info->channels))
		return false;
	else
		return true;
}

/**
 * sap_process_avoid_ie() - processes the detected Q2Q IE
 * context's avoid_channels_info struct
 * @hal:                hal handle
 * @sap_ctx:            sap context.
 * @scan_result:        scan results for ACS scan.
 * @spect_info:         spectrum weights array to update
 *
 * Detection of Q2Q IE indicates presence of another MDM device with its AP
 * operating in MCC mode. This function parses the scan results and processes
 * the Q2Q IE if found. It then extracts the channels and populates them in
 * sap_ctx struct. It also increases the weights of those channels so that
 * ACS logic will avoid those channels in its selection algorithm.
 *
 * Return: void
 */
void sap_process_avoid_ie(tHalHandle hal,
			  ptSapContext sap_ctx,
			  tScanResultHandle scan_result,
			  tSapChSelSpectInfo *spect_info)
{
	uint32_t total_ie_len = 0;
	uint8_t *temp_ptr = NULL;
	uint8_t i = 0;
	struct sAvoidChannelIE *avoid_ch_ie;
	tCsrScanResultInfo *node = NULL;
	tpAniSirGlobal mac_ctx = NULL;
	tSapSpectChInfo *spect_ch = NULL;

	mac_ctx = PMAC_STRUCT(hal);
	spect_ch = spect_info->pSpectCh;
	node = sme_scan_result_get_first(hal, scan_result);

	while (node) {
		total_ie_len = (node->BssDescriptor.length +
			sizeof(uint16_t) + sizeof(uint32_t) -
			sizeof(tSirBssDescription));
		temp_ptr = cfg_get_vendor_ie_ptr_from_oui(mac_ctx,
				SIR_MAC_QCOM_VENDOR_OUI,
				SIR_MAC_QCOM_VENDOR_SIZE,
				((uint8_t *)&node->BssDescriptor.ieFields),
				total_ie_len);

		if (temp_ptr) {
			avoid_ch_ie = (struct sAvoidChannelIE *)temp_ptr;
			if (avoid_ch_ie->type != QCOM_VENDOR_IE_MCC_AVOID_CH)
				continue;

			sap_ctx->sap_detected_avoid_ch_ie.present = 1;
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_DEBUG,
				  "Q2Q IE - avoid ch %d",
				  avoid_ch_ie->channel);
			/* add this channel to to_avoid channel list */
			sap_check_n_add_channel(sap_ctx,
					avoid_ch_ie->channel);
			sap_check_n_add_overlapped_chnls(sap_ctx,
					avoid_ch_ie->channel);
			/*
			 * Mark weight of these channel present in IE to MAX
			 * so that ACS logic will to avoid thse channels
			 */
			for (i = 0; i < spect_info->numSpectChans; i++)
				if (spect_ch[i].chNum == avoid_ch_ie->channel) {
					/*
					 * weight is set more than max so that,
					 * in the case of other channels being
					 * assigned max weight due to noise,
					 * they may be preferred over channels
					 * with Q2Q IE.
					 */
					spect_ch[i].weight = SAP_ACS_WEIGHT_MAX + 1;
					spect_ch[i].weight_copy =
						SAP_ACS_WEIGHT_MAX + 1;
					break;
				}
		} /* if (temp_ptr) */
		node = sme_scan_result_get_next(hal, scan_result);
	}
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

#ifdef FEATURE_WLAN_CH_AVOID
/*==========================================================================
   FUNCTION    sap_update_unsafe_channel_list

   DESCRIPTION
    Function  Undate unsafe channel list table

   DEPENDENCIES
    NA.

   IN
    SapContext pointer

   RETURN VALUE
    NULL
   ============================================================================*/
void sap_update_unsafe_channel_list(ptSapContext pSapCtx)
{
	uint16_t i, j;
	uint16_t unsafe_channel_list[NUM_CHANNELS];
	uint16_t unsafe_channel_count = 0;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	/* Flush, default set all channel safe */
	for (i = 0; i < NUM_CHANNELS; i++) {
		safe_channels[i].isSafe = true;
	}

	/* Try to find unsafe channel */
#if defined(FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE)
	for (i = 0; i < NUM_CHANNELS; i++) {
		if (pSapCtx->dfs_ch_disable == true) {
			if (CDS_IS_DFS_CH(safe_channels[i].channelNumber)) {
				safe_channels[i].isSafe = false;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_INFO_HIGH,
					"%s: DFS Ch %d is not safe in"
					" Concurrent mode",
					__func__,
					safe_channels[i].channelNumber);
			}
		}
	}
#endif
	pld_get_wlan_unsafe_channel(qdf_ctx->dev,
				    unsafe_channel_list,
				     &unsafe_channel_count,
				     sizeof(unsafe_channel_list));

	for (i = 0; i < unsafe_channel_count; i++) {
		for (j = 0; j < NUM_CHANNELS; j++) {
			if (safe_channels[j].channelNumber ==
			    unsafe_channel_list[i]) {
				/* Found unsafe channel, update it */
				safe_channels[j].isSafe = false;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("CH %d is not safe"),
					  unsafe_channel_list[i]);
				break;
			}
		}
	}

	return;
}

#endif /* FEATURE_WLAN_CH_AVOID */

/*==========================================================================
   FUNCTION    sap_cleanup_channel_list

   DESCRIPTION
    Function sap_cleanup_channel_list frees up the memory allocated to the channel list.

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    NULL

   RETURN VALUE
    NULL
   ============================================================================*/

void sap_cleanup_channel_list(void *p_cds_gctx)
{
	ptSapContext pSapCtx;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		  "Cleaning up the channel list structure");

	if (NULL == p_cds_gctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "SAP Global Context is NULL");
		return;
	}

	pSapCtx = CDS_GET_SAP_CB(p_cds_gctx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "SAP Context is NULL");
		return;
	}

	pSapCtx->SapChnlList.numChannel = 0;
	if (pSapCtx->SapChnlList.channelList) {
		qdf_mem_free(pSapCtx->SapChnlList.channelList);
		pSapCtx->SapChnlList.channelList = NULL;
	}

	pSapCtx->SapAllChnlList.numChannel = 0;
	if (pSapCtx->SapAllChnlList.channelList) {
		qdf_mem_free(pSapCtx->SapAllChnlList.channelList);
		pSapCtx->SapAllChnlList.channelList = NULL;
	}
}

/**
 * sap_select_preferred_channel_from_channel_list() - to calc best cahnnel
 * @best_chnl: best channel already calculated among all the chanels
 * @sap_ctx: sap context
 * @spectinfo_param: Pointer to tSapChSelSpectInfo structure
 *
 * This function calculates the best channel among the configured channel list.
 * If channel list not configured then returns the best channel calculated
 * among all the channel list.
 *
 * Return: uint8_t best channel
 */
uint8_t sap_select_preferred_channel_from_channel_list(uint8_t best_chnl,
				ptSapContext sap_ctx,
				tSapChSelSpectInfo *spectinfo_param)
{
	uint8_t i = 0;

	/*
	 * If Channel List is not Configured don't do anything
	 * Else return the Best Channel from the Channel List
	 */
	if ((NULL == sap_ctx->acs_cfg->ch_list) ||
		(NULL == spectinfo_param) ||
		(0 == sap_ctx->acs_cfg->ch_list_count))
		return best_chnl;

	if (best_chnl <= 0 || best_chnl > 252)
		return SAP_CHANNEL_NOT_SELECTED;

	/* Select the best channel from allowed list */
	for (i = 0; i < sap_ctx->acs_cfg->ch_list_count; i++) {
		if (sap_ctx->acs_cfg->ch_list[i] == best_chnl) {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO_HIGH,
				"Best channel is: %d",
				best_chnl);
			return best_chnl;
		}
	}

	return SAP_CHANNEL_NOT_SELECTED;
}

/*==========================================================================
   FUNCTION    sap_chan_sel_init

   DESCRIPTION
    Function sap_chan_sel_init allocates the memory, intializes the
    structures used by the channel selection algorithm

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    halHandle          : Pointer to tHalHandle
   *pSpectInfoParams  : Pointer to tSapChSelSpectInfo structure
     pSapCtx           : Pointer to SAP Context

   RETURN VALUE
    bool:  Success or FAIL

   SIDE EFFECTS
   ============================================================================*/
bool sap_chan_sel_init(tHalHandle halHandle,
		       tSapChSelSpectInfo *pSpectInfoParams, ptSapContext pSapCtx)
{
	tSapSpectChInfo *pSpectCh = NULL;
	uint8_t *pChans = NULL;
	uint16_t channelnum = 0;
	tpAniSirGlobal pMac = PMAC_STRUCT(halHandle);
	bool chSafe = true;
#ifdef FEATURE_WLAN_CH_AVOID
	uint16_t i;
#endif
	uint32_t dfs_master_cap_enabled;
	bool include_dfs_ch = true;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH, "In %s",
		  __func__);

	pSpectInfoParams->numSpectChans =
		pMac->scan.base_channels.numChannels;

	/* Allocate memory for weight computation of 2.4GHz */
	pSpectCh =
		(tSapSpectChInfo *) qdf_mem_malloc((pSpectInfoParams->numSpectChans)
						   * sizeof(*pSpectCh));

	if (pSpectCh == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s, QDF_MALLOC_ERR", __func__);
		return eSAP_FALSE;
	}

	qdf_mem_zero(pSpectCh,
		     (pSpectInfoParams->numSpectChans) * sizeof(*pSpectCh));

	/* Initialize the pointers in the DfsParams to the allocated memory */
	pSpectInfoParams->pSpectCh = pSpectCh;

	pChans = pMac->scan.base_channels.channelList;

#if defined(FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE)
	if (pSapCtx->dfs_ch_disable == true)
		include_dfs_ch = false;
#endif
	sme_cfg_get_int(halHandle, WNI_CFG_DFS_MASTER_ENABLED,
			&dfs_master_cap_enabled);
	if (dfs_master_cap_enabled == 0)
		include_dfs_ch = false;

	/* Fill the channel number in the spectrum in the operating freq band */
	for (channelnum = 0;
	     channelnum < pSpectInfoParams->numSpectChans;
	     channelnum++, pChans++, pSpectCh++) {
		chSafe = true;

		/* check if the channel is in NOL blacklist */
		if (sap_dfs_is_channel_in_nol_list(pSapCtx, *pChans,
						   PHY_SINGLE_CHANNEL_CENTERED)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "In %s, Ch %d is in NOL list", __func__,
				  *pChans);
			chSafe = false;
			continue;
		}

		if (include_dfs_ch == false) {
			if (CDS_IS_DFS_CH(*pChans)) {
				chSafe = false;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_HIGH,
					  "In %s, DFS Ch %d not considered for ACS",
					  __func__, *pChans);
				continue;
			}
		}

#ifdef FEATURE_WLAN_CH_AVOID
		for (i = 0; i < NUM_CHANNELS; i++) {
			if ((safe_channels[i].channelNumber == *pChans) &&
			    (false == safe_channels[i].isSafe)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_HIGH,
					  "In %s, Ch %d is not safe", __func__,
					  *pChans);
				chSafe = false;
				break;
			}
		}
#endif /* FEATURE_WLAN_CH_AVOID */

		/* OFDM rates are not supported on channel 14 */
		if (*pChans == 14 &&
		    eCSR_DOT11_MODE_11b != pSapCtx->csr_roamProfile.phyMode) {
			continue;
		}

		if (true == chSafe) {
			pSpectCh->chNum = *pChans;
			pSpectCh->valid = eSAP_TRUE;
			pSpectCh->rssiAgr = SOFTAP_MIN_RSSI;    /* Initialise for all channels */
			pSpectCh->channelWidth = SOFTAP_HT20_CHANNELWIDTH;      /* Initialise 20MHz for all the Channels */
		}
	}
	return eSAP_TRUE;
}

/*==========================================================================
   FUNCTION    sapweight_rssi_count

   DESCRIPTION
    Function weightRssiCount calculates the channel weight due to rssi
    and data count(here number of BSS observed)

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    rssi        : Max signal strength receieved from a BSS for the channel
    count       : Number of BSS observed in the channel

   RETURN VALUE
    uint32_t     : Calculated channel weight based on above two

   SIDE EFFECTS
   ============================================================================*/
uint32_t sapweight_rssi_count(int8_t rssi, uint16_t count)
{
	int32_t rssiWeight = 0;
	int32_t countWeight = 0;
	uint32_t rssicountWeight = 0;

	/* Weight from RSSI */
	rssiWeight = SOFTAP_RSSI_WEIGHT * (rssi - SOFTAP_MIN_RSSI)
		     / (SOFTAP_MAX_RSSI - SOFTAP_MIN_RSSI);

	if (rssiWeight > SOFTAP_RSSI_WEIGHT)
		rssiWeight = SOFTAP_RSSI_WEIGHT;
	else if (rssiWeight < 0)
		rssiWeight = 0;

	/* Weight from data count */
	countWeight = SOFTAP_COUNT_WEIGHT * (count - SOFTAP_MIN_COUNT)
		      / (SOFTAP_MAX_COUNT - SOFTAP_MIN_COUNT);

	if (countWeight > SOFTAP_COUNT_WEIGHT)
		countWeight = SOFTAP_COUNT_WEIGHT;

	rssicountWeight = rssiWeight + countWeight;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, rssiWeight=%d, countWeight=%d, rssicountWeight=%d",
		  __func__, rssiWeight, countWeight, rssicountWeight);

	return rssicountWeight;
}

/**
 * sap_update_rssi_bsscount() - updates bss count and rssi effect.
 *
 * @pSpectCh:     Channel Information
 * @offset:       Channel Offset
 * @sap_24g:      Channel is in 2.4G or 5G
 *
 * sap_update_rssi_bsscount updates bss count and rssi effect based
 * on the channel offset.
 *
 * Return: None.
 */

void sap_update_rssi_bsscount(tSapSpectChInfo *pSpectCh, int32_t offset,
			      bool sap_24g)
{
	tSapSpectChInfo *pExtSpectCh = NULL;
	int32_t rssi, rsssi_effect;

	pExtSpectCh = (pSpectCh + offset);
	if (pExtSpectCh != NULL) {
		++pExtSpectCh->bssCount;
		switch (offset) {
		case -1:
		case 1:
			rsssi_effect = sap_24g ?
			    SAP_24GHZ_FIRST_OVERLAP_CHAN_RSSI_EFFECT_PRIMARY :
			    SAP_SUBBAND1_RSSI_EFFECT_PRIMARY;
			break;
		case -2:
		case 2:
			rsssi_effect = sap_24g ?
			    SAP_24GHZ_SEC_OVERLAP_CHAN_RSSI_EFFECT_PRIMARY :
			    SAP_SUBBAND2_RSSI_EFFECT_PRIMARY;
			break;
		case -3:
		case 3:
			rsssi_effect = sap_24g ?
			    SAP_24GHZ_THIRD_OVERLAP_CHAN_RSSI_EFFECT_PRIMARY :
			    SAP_SUBBAND3_RSSI_EFFECT_PRIMARY;
			break;
		case -4:
		case 4:
			rsssi_effect = sap_24g ?
			    SAP_24GHZ_FOURTH_OVERLAP_CHAN_RSSI_EFFECT_PRIMARY :
			    SAP_SUBBAND4_RSSI_EFFECT_PRIMARY;
			break;
		case -5:
		case 5:
			rsssi_effect = SAP_SUBBAND5_RSSI_EFFECT_PRIMARY;
			break;
		case -6:
		case 6:
			rsssi_effect = SAP_SUBBAND6_RSSI_EFFECT_PRIMARY;
			break;
		case -7:
		case 7:
			rsssi_effect = SAP_SUBBAND7_RSSI_EFFECT_PRIMARY;
			break;
		default:
			rsssi_effect = 0;
			break;
		}

		rssi = pSpectCh->rssiAgr + rsssi_effect;
		if (IS_RSSI_VALID(pExtSpectCh->rssiAgr, rssi))
			pExtSpectCh->rssiAgr = rssi;
		if (pExtSpectCh->rssiAgr < SOFTAP_MIN_RSSI)
			pExtSpectCh->rssiAgr = SOFTAP_MIN_RSSI;
	}
}

/**
 * sap_upd_chan_spec_params() - sap_upd_chan_spec_params
 *                              updates channel parameters obtained from Beacon
 * @pBeaconStruct Beacon strucutre populated by parse_beacon function
 * @channelWidth Channel width
 * @secondaryChannelOffset Secondary Channel Offset
 * @vhtSupport If channel supports VHT
 * @centerFreq Central frequency for the given channel.
 *
 * sap_upd_chan_spec_params updates the spectrum channels based on the
 * pBeaconStruct obtained from Beacon IE
 *
 * Return: NA.
 */

void sap_upd_chan_spec_params(tSirProbeRespBeacon *pBeaconStruct,
			      uint16_t *channelWidth,
			      uint16_t *secondaryChannelOffset,
			      uint16_t *vhtSupport,
			      uint16_t *centerFreq,
			      uint16_t *centerFreq_2)
{
	if (NULL == pBeaconStruct) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("pBeaconStruct is NULL"));
		return;
	}

	if (pBeaconStruct->HTCaps.present && pBeaconStruct->HTInfo.present) {
		*channelWidth = pBeaconStruct->HTCaps.supportedChannelWidthSet;
		*secondaryChannelOffset =
			pBeaconStruct->HTInfo.secondaryChannelOffset;
		if (!pBeaconStruct->VHTOperation.present)
			return;
		*vhtSupport = pBeaconStruct->VHTOperation.present;
		if (pBeaconStruct->VHTOperation.chanWidth) {
			*centerFreq =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg1;
			*centerFreq_2 =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg2;
			 /*
			  * LHS follows tSirMacHTChannelWidth, while RHS follows
			  * WNI_CFG_VHT_CHANNEL_WIDTH_X format hence following
			  * adjustment
			  */
			*channelWidth =
				pBeaconStruct->VHTOperation.chanWidth + 1;

		}
	}
}

/**
 * sap_update_rssi_bsscount_vht_5G() - updates bss count and rssi effect.
 *
 * @pSpectCh:     Channel Information
 * @offset:       Channel Offset
 * @num_ch:       no.of channels
 *
 * sap_update_rssi_bsscount_vht_5G updates bss count and rssi effect based
 * on the channel offset.
 *
 * Return: None.
 */

void sap_update_rssi_bsscount_vht_5G(tSapSpectChInfo *spect_ch, int32_t offset,
			      uint16_t num_ch)
{
	int32_t ch_offset;
	uint16_t i, cnt;

	if (!offset)
		return;
	if (offset > 0)
		cnt = num_ch;
	else
		cnt = num_ch + 1;
	for (i = 0; i < cnt; i++) {
		ch_offset = offset + i;
		if (ch_offset == 0)
			continue;
		sap_update_rssi_bsscount(spect_ch, ch_offset, false);
	}
}
/**
 * sap_interference_rssi_count_5G() - sap_interference_rssi_count
 *                                    considers the Adjacent channel rssi and
 *                                    data count(here number of BSS observed)
 * @spect_ch:        Channel Information
 * @chan_width:      Channel width parsed from beacon IE
 * @sec_chan_offset: Secondary Channel Offset
 * @center_freq:     Central frequency for the given channel.
 * @channel_id:      channel_id
 *
 * sap_interference_rssi_count_5G considers the Adjacent channel rssi
 * and data count(here number of BSS observed)
 *
 * Return: NA.
 */

void sap_interference_rssi_count_5G(tSapSpectChInfo *spect_ch,
				 uint16_t chan_width,
				 uint16_t sec_chan_offset,
				 uint16_t center_freq,
				 uint16_t center_freq_2,
				 uint8_t channel_id)
{
	uint16_t num_ch;
	int32_t offset = 0;
	if (NULL == spect_ch) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("spect_ch is NULL"));
		return;
	}

	/* Updating the received ChannelWidth */
	if (spect_ch->channelWidth != chan_width)
		spect_ch->channelWidth = chan_width;
	/* If received ChannelWidth is other than HT20,
	 * we need to update the extension channel Params as well
	 * chan_width == 0, HT20
	 * chan_width == 1, HT40
	 * chan_width == 2, VHT80
	 * chan_width == 3, VHT160
	 */

	switch (spect_ch->channelWidth) {
	case eHT_CHANNEL_WIDTH_40MHZ:   /* HT40 */
		switch (sec_chan_offset) {
		/* Above the Primary Channel */
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
			sap_update_rssi_bsscount(spect_ch, 1, false);
			return;

		/* Below the Primary channel */
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			sap_update_rssi_bsscount(spect_ch, -1, false);
			return;
		}
		return;
	case eHT_CHANNEL_WIDTH_80MHZ:   /* VHT80 */
		num_ch = 3;
		if ((center_freq - channel_id) == 6) {
			offset = 1;
		} else if ((center_freq - channel_id) == 2) {
			offset = -1;
		} else if ((center_freq - channel_id) == -2) {
			offset = -2;
		} else if ((center_freq - channel_id) == -6) {
			offset = -3;
		}
		break;
	case eHT_CHANNEL_WIDTH_160MHZ:   /* VHT160 */
		num_ch = 7;
		if ((center_freq - channel_id) == 14)
			offset = 1;
		else if ((center_freq - channel_id) == 10)
			offset = -1;
		else if ((center_freq - channel_id) == 6)
			offset = -2;
		else if ((center_freq - channel_id) == 2)
			offset = -3;
		else if ((center_freq - channel_id) == -2)
			offset = -4;
		else if ((center_freq - channel_id) == -6)
			offset = -5;
		else if ((center_freq - channel_id) == -10)
			offset = -6;
		else if ((center_freq - channel_id) == -14)
			offset = -7;
		break;
	default:
		return;
	}
	sap_update_rssi_bsscount_vht_5G(spect_ch, offset, num_ch);
}

/**
 * sap_interference_rssi_count() - sap_interference_rssi_count
 *                                 considers the Adjacent channel rssi
 *                                 and data count(here number of BSS observed)
 * @spect_ch    Channel Information
 *
 * sap_interference_rssi_count considers the Adjacent channel rssi
 * and data count(here number of BSS observed)
 *
 * Return: None.
 */

void sap_interference_rssi_count(tSapSpectChInfo *spect_ch)
{
	if (NULL == spect_ch) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: spect_ch is NULL", __func__);
		return;
	}

	switch (spect_ch->chNum) {
	case CHANNEL_1:
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		sap_update_rssi_bsscount(spect_ch, 4, true);
		break;

	case CHANNEL_2:
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		sap_update_rssi_bsscount(spect_ch, 4, true);
		break;
	case CHANNEL_3:
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		sap_update_rssi_bsscount(spect_ch, 4, true);
		break;
	case CHANNEL_4:
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		sap_update_rssi_bsscount(spect_ch, 4, true);
		break;

	case CHANNEL_5:
	case CHANNEL_6:
	case CHANNEL_7:
	case CHANNEL_8:
	case CHANNEL_9:
	case CHANNEL_10:
		sap_update_rssi_bsscount(spect_ch, -4, true);
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		sap_update_rssi_bsscount(spect_ch, 4, true);
		break;

	case CHANNEL_11:
		sap_update_rssi_bsscount(spect_ch, -4, true);
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		sap_update_rssi_bsscount(spect_ch, 3, true);
		break;

	case CHANNEL_12:
		sap_update_rssi_bsscount(spect_ch, -4, true);
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		sap_update_rssi_bsscount(spect_ch, 2, true);
		break;

	case CHANNEL_13:
		sap_update_rssi_bsscount(spect_ch, -4, true);
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		sap_update_rssi_bsscount(spect_ch, 1, true);
		break;

	case CHANNEL_14:
		sap_update_rssi_bsscount(spect_ch, -4, true);
		sap_update_rssi_bsscount(spect_ch, -3, true);
		sap_update_rssi_bsscount(spect_ch, -2, true);
		sap_update_rssi_bsscount(spect_ch, -1, true);
		break;

	default:
		break;
	}
}

/*==========================================================================
  Function    ch_in_pcl

  Description
   Check if a channel is in the preferred channel list

  Parameters
   sap_ctx   SAP context pointer
   channel   input channel number

  Return Value
   true:    channel is in PCL
   false:   channel is not in PCL
 ==========================================================================*/
bool ch_in_pcl(ptSapContext sap_ctx, uint8_t channel)
{
	uint32_t i;

	for (i = 0; i < sap_ctx->acs_cfg->pcl_ch_count; i++) {
		if (channel == sap_ctx->acs_cfg->pcl_channels[i])
			return true;
	}

	return false;
}

/*==========================================================================
   FUNCTION    sap_compute_spect_weight

   DESCRIPTION
    Main function for computing the weight of each channel in the
    spectrum based on the RSSI value of the BSSes on the channel
    and number of BSS

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    pSpectInfoParams       : Pointer to the tSpectInfoParams structure
    halHandle              : Pointer to HAL handle
    pResult                : Pointer to tScanResultHandle

   RETURN VALUE
    void     : NULL

   SIDE EFFECTS
   ============================================================================*/
void sap_compute_spect_weight(tSapChSelSpectInfo *pSpectInfoParams,
			      tHalHandle halHandle, tScanResultHandle pResult,
				ptSapContext sap_ctx)
{
	int8_t rssi = 0;
	uint8_t chn_num = 0;
	uint8_t channel_id = 0;

	tCsrScanResultInfo *pScanResult;
	tSapSpectChInfo *pSpectCh = pSpectInfoParams->pSpectCh;
	uint32_t operatingBand;
	uint16_t channelWidth;
	uint16_t secondaryChannelOffset;
	uint16_t centerFreq;
	uint16_t centerFreq_2 = 0;
	uint16_t vhtSupport;
	uint32_t ieLen = 0;
	tSirProbeRespBeacon *pBeaconStruct;
	tpAniSirGlobal pMac = (tpAniSirGlobal) halHandle;

	pBeaconStruct = qdf_mem_malloc(sizeof(tSirProbeRespBeacon));
	if (NULL == pBeaconStruct) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Unable to allocate memory in sap_compute_spect_weight\n");
		return;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, Computing spectral weight", __func__);

	/**
	 * Soft AP specific channel weight calculation using DFS formula
	 */
	SET_ACS_BAND(operatingBand, sap_ctx);

	pScanResult = sme_scan_result_get_first(halHandle, pResult);

	while (pScanResult) {
		pSpectCh = pSpectInfoParams->pSpectCh;
		/* Defining the default values, so that any value will hold the default values */
		channelWidth = eHT_CHANNEL_WIDTH_20MHZ;
		secondaryChannelOffset = PHY_SINGLE_CHANNEL_CENTERED;
		vhtSupport = 0;
		centerFreq = 0;

		if (pScanResult->BssDescriptor.ieFields != NULL) {
			ieLen =
				(pScanResult->BssDescriptor.length +
				 sizeof(uint16_t) + sizeof(uint32_t) -
				 sizeof(tSirBssDescription));
			qdf_mem_set((uint8_t *) pBeaconStruct,
				    sizeof(tSirProbeRespBeacon), 0);

			if ((sir_parse_beacon_ie
				     (pMac, pBeaconStruct, (uint8_t *)
				      (pScanResult->BssDescriptor.ieFields),
				      ieLen)) == eSIR_SUCCESS) {
				sap_upd_chan_spec_params(pBeaconStruct,
					&channelWidth, &secondaryChannelOffset,
					&vhtSupport, &centerFreq,
					&centerFreq_2);
			}
		}
		/* Processing for each tCsrScanResultInfo in the tCsrScanResult DLink list */
		for (chn_num = 0; chn_num < pSpectInfoParams->numSpectChans;
		     chn_num++) {

			/*
			 *  if the Beacon has channel ID, use it other wise we will
			 *  rely on the channelIdSelf
			 */
			if (pScanResult->BssDescriptor.channelId == 0)
				channel_id =
					pScanResult->BssDescriptor.channelIdSelf;
			else
				channel_id =
					pScanResult->BssDescriptor.channelId;

			if (pSpectCh && (channel_id == pSpectCh->chNum)) {
				if (pSpectCh->rssiAgr <
				    pScanResult->BssDescriptor.rssi)
					pSpectCh->rssiAgr =
						pScanResult->BssDescriptor.rssi;

				++pSpectCh->bssCount;   /* Increment the count of BSS */

				/*
				 * Connsidering the Extension Channel
				 * only in a channels
				 */
				switch (operatingBand) {
				case eCSR_DOT11_MODE_11a:
					sap_interference_rssi_count_5G(
					    pSpectCh, channelWidth,
					    secondaryChannelOffset,
					    centerFreq,
					    centerFreq_2,
					    channel_id);
					break;

				case eCSR_DOT11_MODE_11g:
					sap_interference_rssi_count(pSpectCh);
					break;

				case eCSR_DOT11_MODE_abg:
					sap_interference_rssi_count_5G(
					    pSpectCh, channelWidth,
					    secondaryChannelOffset,
					    centerFreq,
					    centerFreq_2,
					    channel_id);
					sap_interference_rssi_count(pSpectCh);
					break;
				}

				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_HIGH,
					  "In %s, bssdes.ch_self=%d, bssdes.ch_ID=%d, bssdes.rssi=%d, SpectCh.bssCount=%d, pScanResult=%p, ChannelWidth %d, secondaryChanOffset %d, center frequency %d \n",
					  __func__,
					  pScanResult->BssDescriptor.
					  channelIdSelf,
					  pScanResult->BssDescriptor.channelId,
					  pScanResult->BssDescriptor.rssi,
					  pSpectCh->bssCount, pScanResult,
					  pSpectCh->channelWidth,
					  secondaryChannelOffset, centerFreq);
				pSpectCh++;
				break;
			} else {
				pSpectCh++;
			}
		}

		pScanResult = sme_scan_result_get_next(halHandle, pResult);
	}

	/* Calculate the weights for all channels in the spectrum pSpectCh */
	pSpectCh = pSpectInfoParams->pSpectCh;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, Spectrum Channels Weight", __func__);

	for (chn_num = 0; chn_num < (pSpectInfoParams->numSpectChans);
	     chn_num++) {

		/*
		   rssi : Maximum received signal strength among all BSS on that channel
		   bssCount : Number of BSS on that channel
		 */

		rssi = (int8_t) pSpectCh->rssiAgr;
		if (ch_in_pcl(sap_ctx, chn_num))
			rssi -= PCL_RSSI_DISCOUNT;

		pSpectCh->weight =
			SAPDFS_NORMALISE_1000 * sapweight_rssi_count(rssi,
								     pSpectCh->
								     bssCount);
		pSpectCh->weight_copy = pSpectCh->weight;

		/* ------ Debug Info ------ */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Chan=%d Weight= %d rssiAgr=%d bssCount=%d",
			  __func__, pSpectCh->chNum, pSpectCh->weight,
			  pSpectCh->rssiAgr, pSpectCh->bssCount);
		/* ------ Debug Info ------ */
		pSpectCh++;
	}
	qdf_mem_free(pBeaconStruct);
}

/*==========================================================================
   FUNCTION    sap_chan_sel_exit

   DESCRIPTION
    Exit function for free out the allocated memory, to be called
    at the end of the dfsSelectChannel function

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    pSpectInfoParams       : Pointer to the tSapChSelSpectInfo structure

   RETURN VALUE
    void     : NULL

   SIDE EFFECTS
   ============================================================================*/
void sap_chan_sel_exit(tSapChSelSpectInfo *pSpectInfoParams)
{
	/* Free all the allocated memory */
	qdf_mem_free(pSpectInfoParams->pSpectCh);
}

/*==========================================================================
   FUNCTION    sap_sort_chl_weight

   DESCRIPTION
    Funtion to sort the channels with the least weight first for 20MHz channels

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    pSpectInfoParams       : Pointer to the tSapChSelSpectInfo structure

   RETURN VALUE
    void     : NULL

   SIDE EFFECTS
   ============================================================================*/
void sap_sort_chl_weight(tSapChSelSpectInfo *pSpectInfoParams)
{
	tSapSpectChInfo temp;

	tSapSpectChInfo *pSpectCh = NULL;
	uint32_t i = 0, j = 0, minWeightIndex = 0;

	pSpectCh = pSpectInfoParams->pSpectCh;
	for (i = 0; i < pSpectInfoParams->numSpectChans; i++) {
		minWeightIndex = i;
		for (j = i + 1; j < pSpectInfoParams->numSpectChans; j++) {
			if (pSpectCh[j].weight <
			    pSpectCh[minWeightIndex].weight) {
				minWeightIndex = j;
			}
		}
		if (minWeightIndex != i) {
			qdf_mem_copy(&temp, &pSpectCh[minWeightIndex],
				     sizeof(*pSpectCh));
			qdf_mem_copy(&pSpectCh[minWeightIndex], &pSpectCh[i],
				     sizeof(*pSpectCh));
			qdf_mem_copy(&pSpectCh[i], &temp, sizeof(*pSpectCh));
		}
	}
}

/**
 * sap_sort_chl_weight_ht80() - to sort the channels with the least weight
 * @pSpectInfoParams: Pointer to the tSapChSelSpectInfo structure
 *
 * Funtion to sort the channels with the least weight first for HT80 channels
 *
 * Return: none
 */
void sap_sort_chl_weight_ht80(tSapChSelSpectInfo *pSpectInfoParams)
{
	uint8_t i, j, n;
	tSapSpectChInfo *pSpectInfo;
	uint8_t minIdx;

	pSpectInfo = pSpectInfoParams->pSpectCh;
	/* for each HT80 channel, calculate the combined weight of the
	   four 20MHz weight */
	for (i = 0; i < ARRAY_SIZE(acs_ht80_channels); i++) {
		for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
			if (pSpectInfo[j].chNum ==
					acs_ht80_channels[i].chStartNum)
				break;
		}
		if (j == pSpectInfoParams->numSpectChans)
			continue;

		if (!(((pSpectInfo[j].chNum + 4) == pSpectInfo[j + 1].chNum) &&
			((pSpectInfo[j].chNum + 8) ==
				 pSpectInfo[j + 2].chNum) &&
			((pSpectInfo[j].chNum + 12) ==
				 pSpectInfo[j + 3].chNum))) {
			/*
			 * some channels does not exist in pSectInfo array,
			 * skip this channel and those in the same HT80 width
			 */
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 4;
			if ((pSpectInfo[j].chNum + 4) ==
					pSpectInfo[j + 1].chNum)
				pSpectInfo[j + 1].weight =
					SAP_ACS_WEIGHT_MAX * 4;
			if ((pSpectInfo[j].chNum + 8) ==
					pSpectInfo[j + 2].chNum)
				pSpectInfo[j + 2].weight =
					SAP_ACS_WEIGHT_MAX * 4;
			if ((pSpectInfo[j].chNum + 12) ==
					pSpectInfo[j + 3].chNum)
				pSpectInfo[j + 3].weight =
					SAP_ACS_WEIGHT_MAX * 4;
			continue;
		}
		/*found the channel, add the 4 adjacent channels' weight */
		acs_ht80_channels[i].weight = pSpectInfo[j].weight +
			pSpectInfo[j + 1].weight + pSpectInfo[j + 2].weight +
			pSpectInfo[j + 3].weight;
		/* find best channel among 4 channels as the primary channel */
		if ((pSpectInfo[j].weight + pSpectInfo[j + 1].weight) <
			(pSpectInfo[j + 2].weight + pSpectInfo[j + 3].weight)) {
			/* lower 2 channels are better choice */
			if (pSpectInfo[j].weight < pSpectInfo[j + 1].weight)
				minIdx = 0;
			else
				minIdx = 1;
		} else if (pSpectInfo[j + 2].weight <=
				pSpectInfo[j + 3].weight) {
			/* upper 2 channels are better choice */
			minIdx = 2;
		} else {
			minIdx = 3;
		}

		/*
		 * set all 4 channels to max value first, then reset the
		 * best channel as the selected primary channel, update its
		 * weightage with the combined weight value
		 */
		for (n = 0; n < 4; n++)
			pSpectInfo[j + n].weight = SAP_ACS_WEIGHT_MAX * 4;

		pSpectInfo[j + minIdx].weight = acs_ht80_channels[i].weight;
	}

	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
		if (CHANNEL_165 == pSpectInfo[j].chNum) {
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 4;
			break;
		}
	}

	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < (pSpectInfoParams->numSpectChans); j++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Channel=%d Weight= %d rssi=%d bssCount=%d"),
			pSpectInfo->chNum, pSpectInfo->weight,
			pSpectInfo->rssiAgr, pSpectInfo->bssCount);
		pSpectInfo++;
	}
}

/**
 * sap_sort_chl_weight_vht160() - to sort the channels with the least weight
 * @pSpectInfoParams: Pointer to the tSapChSelSpectInfo structure
 *
 * Funtion to sort the channels with the least weight first for VHT160 channels
 *
 * Return: none
 */
void sap_sort_chl_weight_vht160(tSapChSelSpectInfo *pSpectInfoParams)
{
	uint8_t i, j, n, idx;
	tSapSpectChInfo *pSpectInfo;
	uint8_t minIdx;

	pSpectInfo = pSpectInfoParams->pSpectCh;
	/* for each VHT160 channel, calculate the combined weight of the
	   8 20MHz weight */
	for (i = 0; i < ARRAY_SIZE(acs_vht160_channels); i++) {
		for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
			if (pSpectInfo[j].chNum ==
					acs_vht160_channels[i].chStartNum)
				break;
		}
		if (j == pSpectInfoParams->numSpectChans)
			continue;

		if (!(((pSpectInfo[j].chNum + 4) == pSpectInfo[j + 1].chNum) &&
			((pSpectInfo[j].chNum + 8) ==
				 pSpectInfo[j + 2].chNum) &&
			((pSpectInfo[j].chNum + 12) ==
				 pSpectInfo[j + 3].chNum) &&
			((pSpectInfo[j].chNum + 16) ==
				 pSpectInfo[j + 4].chNum) &&
			((pSpectInfo[j].chNum + 20) ==
				 pSpectInfo[j + 5].chNum) &&
			((pSpectInfo[j].chNum + 24) ==
				 pSpectInfo[j + 6].chNum) &&
			((pSpectInfo[j].chNum + 28) ==
				 pSpectInfo[j + 7].chNum))) {
			/*
			 * some channels does not exist in pSectInfo array,
			 * skip this channel and those in the same VHT160 width
			 */
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 4) ==
					pSpectInfo[j + 1].chNum)
				pSpectInfo[j + 1].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 8) ==
					pSpectInfo[j + 2].chNum)
				pSpectInfo[j + 2].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 12) ==
					pSpectInfo[j + 3].chNum)
				pSpectInfo[j + 3].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 16) ==
					pSpectInfo[j + 4].chNum)
				pSpectInfo[j + 4].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 20) ==
					pSpectInfo[j + 5].chNum)
				pSpectInfo[j + 5].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 24) ==
					pSpectInfo[j + 6].chNum)
				pSpectInfo[j + 6].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			if ((pSpectInfo[j].chNum + 28) ==
					pSpectInfo[j + 7].chNum)
				pSpectInfo[j + 7].weight =
					SAP_ACS_WEIGHT_MAX * 8;
			continue;
		}
		/*found the channel, add the 7 adjacent channels' weight */
		acs_vht160_channels[i].weight = pSpectInfo[j].weight +
			pSpectInfo[j + 1].weight + pSpectInfo[j + 2].weight +
			pSpectInfo[j + 3].weight + pSpectInfo[j + 4].weight +
			pSpectInfo[j + 5].weight + pSpectInfo[j + 6].weight +
			pSpectInfo[j + 7].weight;

		/* find best channel among 8 channels as the primary channel */
		if ((pSpectInfo[j].weight + pSpectInfo[j + 1].weight +
			pSpectInfo[j + 2].weight + pSpectInfo[j + 3].weight) >
			(pSpectInfo[j + 4].weight + pSpectInfo[j + 5].weight +
			pSpectInfo[j + 6].weight + pSpectInfo[j + 7].weight))
			idx = 4;
		else
			idx = 0;
		/* find best channel among 4 channels as the primary channel */
		if ((pSpectInfo[j + idx].weight +
					pSpectInfo[j + idx + 1].weight) <
			(pSpectInfo[j + idx + 2].weight +
			 pSpectInfo[j + idx + 3].weight)) {
			/* lower 2 channels are better choice */
			if (pSpectInfo[j + idx].weight <
					pSpectInfo[j + idx + 1].weight)
				minIdx = 0 + idx;
			else
				minIdx = 1 + idx;
		} else if (pSpectInfo[j + idx + 2].weight <=
				pSpectInfo[j + idx + 3].weight) {
			/* upper 2 channels are better choice */
			minIdx = 2 + idx;
		} else {
			minIdx = 3 + idx;
		}

		/*
		 * set all 8 channels to max value first, then reset the
		 * best channel as the selected primary channel, update its
		 * weightage with the combined weight value
		 */
		for (n = 0; n < 8; n++)
			pSpectInfo[j + n].weight = SAP_ACS_WEIGHT_MAX * 8;

		pSpectInfo[j + minIdx].weight = acs_vht160_channels[i].weight;
	}

	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
		if (CHANNEL_165 == pSpectInfo[j].chNum) {
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 8;
			break;
		}
	}

	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < (pSpectInfoParams->numSpectChans); j++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Channel=%d Weight= %d rssi=%d bssCount=%d"),
			pSpectInfo->chNum, pSpectInfo->weight,
			pSpectInfo->rssiAgr, pSpectInfo->bssCount);
		pSpectInfo++;
	}
}

/**
 * sap_sort_chl_weight_ht40_24_g() - to sort channel with the least weight
 * @pSpectInfoParams: Pointer to the tSapChSelSpectInfo structure
 *
 * Funtion to sort the channels with the least weight first for HT40 channels
 *
 * Return: none
 */
void sap_sort_chl_weight_ht40_24_g(tSapChSelSpectInfo *pSpectInfoParams)
{
	uint8_t i, j;
	tSapSpectChInfo *pSpectInfo;
	uint32_t tmpWeight1, tmpWeight2;

	pSpectInfo = pSpectInfoParams->pSpectCh;
	/*
	 * for each HT40 channel, calculate the combined weight of the
	 * two 20MHz weight
	 */
	for (i = 0; i < ARRAY_SIZE(acs_ht40_channels24_g); i++) {
		for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
			if (pSpectInfo[j].chNum ==
				acs_ht40_channels24_g[i].chStartNum)
				break;
		}
		if (j == pSpectInfoParams->numSpectChans)
			continue;

		if (!((pSpectInfo[j].chNum + 4) == pSpectInfo[j + 4].chNum)) {
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 2;
			continue;
		}
		/*
		 * check if there is another channel combination possiblity
		 * e.g., {1, 5} & {5, 9}
		 */
		if ((pSpectInfo[j + 4].chNum + 4) == pSpectInfo[j + 8].chNum) {
			/* need to compare two channel pairs */
			tmpWeight1 = pSpectInfo[j].weight +
						pSpectInfo[j + 4].weight;
			tmpWeight2 = pSpectInfo[j + 4].weight +
						pSpectInfo[j + 8].weight;
			if (tmpWeight1 <= tmpWeight2) {
				if (pSpectInfo[j].weight <=
						pSpectInfo[j + 4].weight) {
					pSpectInfo[j].weight =
						tmpWeight1;
					pSpectInfo[j + 4].weight =
						SAP_ACS_WEIGHT_MAX * 2;
					pSpectInfo[j + 8].weight =
						SAP_ACS_WEIGHT_MAX * 2;
				} else {
					pSpectInfo[j + 4].weight =
						tmpWeight1;
					/* for secondary channel selection */
					pSpectInfo[j].weight =
						SAP_ACS_WEIGHT_MAX * 2
						- 1;
					pSpectInfo[j + 8].weight =
						SAP_ACS_WEIGHT_MAX * 2;
				}
			} else {
				if (pSpectInfo[j + 4].weight <=
						pSpectInfo[j + 8].weight) {
					pSpectInfo[j + 4].weight =
						tmpWeight2;
					pSpectInfo[j].weight =
						SAP_ACS_WEIGHT_MAX * 2;
					/* for secondary channel selection */
					pSpectInfo[j + 8].weight =
						SAP_ACS_WEIGHT_MAX * 2
						- 1;
				} else {
					pSpectInfo[j + 8].weight =
						tmpWeight2;
					pSpectInfo[j].weight =
						SAP_ACS_WEIGHT_MAX * 2;
					pSpectInfo[j + 4].weight =
						SAP_ACS_WEIGHT_MAX * 2;
				}
			}
		} else {
			tmpWeight1 = pSpectInfo[j].weight +
						pSpectInfo[j + 4].weight;
			if (pSpectInfo[j].weight <=
					pSpectInfo[j + 4].weight) {
				pSpectInfo[j].weight = tmpWeight1;
				pSpectInfo[j + 4].weight =
					SAP_ACS_WEIGHT_MAX * 2;
			} else {
				pSpectInfo[j + 4].weight = tmpWeight1;
				pSpectInfo[j].weight =
					SAP_ACS_WEIGHT_MAX * 2;
			}
		}
	}
}

/*==========================================================================
   FUNCTION    sap_sort_chl_weight_ht40_5_g

   DESCRIPTION
    Funtion to sort the channels with the least weight first for HT40 channels

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    pSpectInfoParams       : Pointer to the tSapChSelSpectInfo structure

   RETURN VALUE
    void     : NULL

   SIDE EFFECTS
   ============================================================================*/
void sap_sort_chl_weight_ht40_5_g(tSapChSelSpectInfo *pSpectInfoParams)
{
	uint8_t i, j;
	tSapSpectChInfo *pSpectInfo;

	pSpectInfo = pSpectInfoParams->pSpectCh;
	/*for each HT40 channel, calculate the combined weight of the
	   two 20MHz weight */
	for (i = 0; i < ARRAY_SIZE(acs_ht40_channels5_g); i++) {
		for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
			if (pSpectInfo[j].chNum ==
			    acs_ht40_channels5_g[i].chStartNum)
				break;
		}
		if (j == pSpectInfoParams->numSpectChans)
			continue;

		/* found the channel, add the two adjacent channels' weight */
		if ((pSpectInfo[j].chNum + 4) == pSpectInfo[j + 1].chNum) {
			acs_ht40_channels5_g[i].weight = pSpectInfo[j].weight +
						      pSpectInfo[j + 1].weight;
			/* select better of the adjact channel as the primary channel */
			if (pSpectInfo[j].weight <= pSpectInfo[j + 1].weight) {
				pSpectInfo[j].weight =
					acs_ht40_channels5_g[i].weight;
				/* mark the adjacent channel's weight as max value so
				   that it will be sorted to the bottom */
				pSpectInfo[j + 1].weight =
					SAP_ACS_WEIGHT_MAX * 2;
			} else {
				pSpectInfo[j + 1].weight =
					acs_ht40_channels5_g[i].weight;
				/* mark the adjacent channel's weight as max value so
				   that it will be sorted to the bottom */
				pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 2;
			}

		} else
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 2;
	}

	/* avoid channel 165 by setting its weight to max */
	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < pSpectInfoParams->numSpectChans; j++) {
		if (CHANNEL_165 == pSpectInfo[j].chNum) {
			pSpectInfo[j].weight = SAP_ACS_WEIGHT_MAX * 2;
			break;
		}
	}

	pSpectInfo = pSpectInfoParams->pSpectCh;
	for (j = 0; j < (pSpectInfoParams->numSpectChans); j++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Channel=%d Weight= %d rssi=%d bssCount=%d",
			  __func__, pSpectInfo->chNum, pSpectInfo->weight,
			  pSpectInfo->rssiAgr, pSpectInfo->bssCount);
		pSpectInfo++;
	}

	sap_sort_chl_weight(pSpectInfoParams);
}

/*==========================================================================
   FUNCTION    sap_sort_chl_weight_all

   DESCRIPTION
    Funtion to sort the channels with the least weight first

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    ptSapContext           : Pointer to the ptSapContext structure
    pSpectInfoParams       : Pointer to the tSapChSelSpectInfo structure

   RETURN VALUE
    void     : NULL

   SIDE EFFECTS
   ============================================================================*/
void sap_sort_chl_weight_all(ptSapContext pSapCtx,
			     tSapChSelSpectInfo *pSpectInfoParams,
			     uint32_t operatingBand)
{
	tSapSpectChInfo *pSpectCh = NULL;
	uint32_t j = 0;
#ifndef SOFTAP_CHANNEL_RANGE
	uint32_t i = 0;
#endif

	pSpectCh = pSpectInfoParams->pSpectCh;
#ifdef SOFTAP_CHANNEL_RANGE

	switch (pSapCtx->acs_cfg->ch_width) {
	case CH_WIDTH_40MHZ:
		if (eCSR_DOT11_MODE_11g == operatingBand)
			sap_sort_chl_weight_ht40_24_g(pSpectInfoParams);
		else if (eCSR_DOT11_MODE_11a == operatingBand)
			sap_sort_chl_weight_ht40_5_g(pSpectInfoParams);
		else {
			sap_sort_chl_weight_ht40_24_g(pSpectInfoParams);
			sap_sort_chl_weight_ht40_5_g(pSpectInfoParams);
		}
		sap_sort_chl_weight(pSpectInfoParams);
		break;

	case CH_WIDTH_80MHZ:
	case CH_WIDTH_80P80MHZ:
		sap_sort_chl_weight_ht80(pSpectInfoParams);
		break;

	case CH_WIDTH_160MHZ:
		sap_sort_chl_weight_vht160(pSpectInfoParams);
		break;
	case CH_WIDTH_20MHZ:
	default:
		/* Sorting the channels as per weights as 20MHz channels */
		sap_sort_chl_weight(pSpectInfoParams);
	}

#else
	/* Sorting the channels as per weights */
	for (i = 0; i < SPECT_24GHZ_CH_COUNT; i++) {
		minWeightIndex = i;
		for (j = i + 1; j < SPECT_24GHZ_CH_COUNT; j++) {
			if (pSpectCh[j].weight <
			    pSpectCh[minWeightIndex].weight) {
				minWeightIndex = j;
			}
		}
		if (minWeightIndex != i) {
			qdf_mem_copy(&temp, &pSpectCh[minWeightIndex],
				     sizeof(*pSpectCh));
			qdf_mem_copy(&pSpectCh[minWeightIndex], &pSpectCh[i],
				     sizeof(*pSpectCh));
			qdf_mem_copy(&pSpectCh[i], &temp, sizeof(*pSpectCh));
		}
	}
#endif

	/* For testing */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, Sorted Spectrum Channels Weight", __func__);
	pSpectCh = pSpectInfoParams->pSpectCh;
	for (j = 0; j < (pSpectInfoParams->numSpectChans); j++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Channel=%d Weight= %d rssi=%d bssCount=%d",
			  __func__, pSpectCh->chNum, pSpectCh->weight,
			  pSpectCh->rssiAgr, pSpectCh->bssCount);
		pSpectCh++;
	}

}

/*==========================================================================
   FUNCTION    sap_filter_over_lap_ch

   DESCRIPTION
    return true if ch is acceptable.
    This function will decide if we will filter over lap channel or not.

   DEPENDENCIES
    shall called after ap start.

   PARAMETERS

   IN
    pSapCtx          : Pointer to ptSapContext.
    chNum             : Filter channel number.

   RETURN VALUE
    bool          : true if channel is accepted.

   SIDE EFFECTS
   ============================================================================*/
bool sap_filter_over_lap_ch(ptSapContext pSapCtx, uint16_t chNum)
{
	if (pSapCtx->enableOverLapCh)
		return eSAP_TRUE;
	else if ((chNum == CHANNEL_1) ||
		 (chNum == CHANNEL_6) || (chNum == CHANNEL_11))
		return eSAP_TRUE;

	return eSAP_FALSE;
}

/**
 * sap_select_channel_no_scan_result() - select SAP channel when no scan results
 * are available.
 * @sap_ctx: Sap context
 *
 * Returns: channel number if success, 0 otherwise
 */
static uint8_t sap_select_channel_no_scan_result(ptSapContext sap_ctx)
{
	enum channel_state ch_type;
	uint32_t start_ch_num, end_ch_num;
#ifdef FEATURE_WLAN_CH_AVOID
	uint8_t i, first_safe_ch_in_range = SAP_CHANNEL_NOT_SELECTED;
#endif
	start_ch_num = sap_ctx->acs_cfg->start_ch;
	end_ch_num = sap_ctx->acs_cfg->end_ch;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("start - end: %d - %d"), start_ch_num, end_ch_num);

#ifndef FEATURE_WLAN_CH_AVOID
	sap_ctx->acs_cfg->pri_ch = start_ch_num;
	sap_ctx->acs_cfg->ht_sec_ch = 0;
	/* pick the first channel in configured range */
	return start_ch_num;
#else

	/* get a channel in PCL and within the range */
	for (i = 0; i < sap_ctx->acs_cfg->pcl_ch_count; i++) {
		if ((sap_ctx->acs_cfg->pcl_channels[i] < start_ch_num) ||
		    (sap_ctx->acs_cfg->pcl_channels[i] > end_ch_num))
			continue;

		first_safe_ch_in_range = sap_ctx->acs_cfg->pcl_channels[i];
		break;
	}

	if (SAP_CHANNEL_NOT_SELECTED != first_safe_ch_in_range)
		return first_safe_ch_in_range;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if ((safe_channels[i].channelNumber < start_ch_num) ||
		    (safe_channels[i].channelNumber > end_ch_num))
			continue;

		ch_type = cds_get_channel_state(safe_channels[i].channelNumber);

		if ((ch_type == CHANNEL_STATE_DISABLE) ||
			(ch_type == CHANNEL_STATE_INVALID))
			continue;

		if (safe_channels[i].isSafe == true) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				FL("channel %d in the configuration is safe"),
				safe_channels[i].channelNumber);
			first_safe_ch_in_range = safe_channels[i].channelNumber;
			break;
		}

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("channel %d in the configuration is unsafe"),
			safe_channels[i].channelNumber);
	}

	/* if no channel selected return SAP_CHANNEL_NOT_SELECTED */
	return first_safe_ch_in_range;
#endif /* !FEATURE_WLAN_CH_AVOID */
}

/**
 * sap_select_channel() - select SAP channel
 * @hal: Pointer to HAL handle
 * @sap_ctx: Sap context
 * @scan_result: Pointer to tScanResultHandle
 *
 * Runs a algorithm to select the best channel to operate in based on BSS
 * rssi and bss count on each channel
 *
 * Returns: channel number if success, 0 otherwise
 */
uint8_t sap_select_channel(tHalHandle hal, ptSapContext sap_ctx,
			   tScanResultHandle scan_result)
{
	/* DFS param object holding all the data req by the algo */
	tSapChSelSpectInfo spect_info_obj = { NULL, 0 };
	tSapChSelSpectInfo *spect_info = &spect_info_obj;
	uint8_t best_ch_num = SAP_CHANNEL_NOT_SELECTED;
#ifdef SOFTAP_CHANNEL_RANGE
	uint8_t count;
	uint32_t start_ch_num, end_ch_num, tmp_ch_num, operating_band = 0;
#endif
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, Running SAP Ch Select", __func__);

#ifdef FEATURE_WLAN_CH_AVOID
	sap_update_unsafe_channel_list(sap_ctx);
#endif

	if (NULL == scan_result) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("No external AP present"));

#ifndef SOFTAP_CHANNEL_RANGE
		return SAP_CHANNEL_NOT_SELECTED;
#else
		return sap_select_channel_no_scan_result(sap_ctx);
#endif
	}

	/* Initialize the structure pointed by spect_info */
	if (sap_chan_sel_init(hal, spect_info, sap_ctx) != eSAP_TRUE) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Ch Select initialization failed"));
		return SAP_CHANNEL_NOT_SELECTED;
	}
	/* Compute the weight of the entire spectrum in the operating band */
	sap_compute_spect_weight(spect_info, hal, scan_result, sap_ctx);

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	/* process avoid channel IE to collect all channels to avoid */
	sap_process_avoid_ie(hal, sap_ctx, scan_result, spect_info);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

#ifdef SOFTAP_CHANNEL_RANGE
	start_ch_num = sap_ctx->acs_cfg->start_ch;
	end_ch_num = sap_ctx->acs_cfg->end_ch;
	SET_ACS_BAND(operating_band, sap_ctx);

	sap_ctx->acsBestChannelInfo.channelNum = 0;
	sap_ctx->acsBestChannelInfo.weight = SAP_ACS_WEIGHT_MAX;

	/* Sort the ch lst as per the computed weights, lesser weight first. */
	sap_sort_chl_weight_all(sap_ctx, spect_info, operating_band);

	/*Loop till get the best channel in the given range */
	for (count = 0; count < spect_info->numSpectChans; count++) {
		if ((start_ch_num > spect_info->pSpectCh[count].chNum) ||
		    (end_ch_num < spect_info->pSpectCh[count].chNum))
			continue;

		if (best_ch_num == SAP_CHANNEL_NOT_SELECTED) {
			best_ch_num = spect_info->pSpectCh[count].chNum;
			/* check if best_ch_num is in preferred channel list */
			best_ch_num =
				sap_select_preferred_channel_from_channel_list(
					best_ch_num, sap_ctx, spect_info);
			/* if not in preferred ch lst, go to nxt best ch */
			if (best_ch_num == SAP_CHANNEL_NOT_SELECTED)
				continue;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
			/*
			 * Weight of the channels(device's AP is operating)
			 * increased to MAX+1 so that they will be choosen only
			 * when there is no other best channel to choose
			 */
			if (sap_check_in_avoid_ch_list(sap_ctx, best_ch_num)) {
				best_ch_num = SAP_CHANNEL_NOT_SELECTED;
				continue;
			}
#endif

			sap_ctx->acsBestChannelInfo.channelNum = best_ch_num;
			sap_ctx->acsBestChannelInfo.weight =
					spect_info->pSpectCh[count].weight_copy;
		}

		if (best_ch_num == SAP_CHANNEL_NOT_SELECTED)
			continue;

		if (operating_band != eCSR_DOT11_MODE_11g)
			continue;

		/* Give preference to Non-overlap channels */
		if (false == sap_filter_over_lap_ch(sap_ctx,
				spect_info->pSpectCh[count].chNum))
			continue;

		tmp_ch_num = spect_info->pSpectCh[count].chNum;
		tmp_ch_num = sap_select_preferred_channel_from_channel_list(
					tmp_ch_num, sap_ctx, spect_info);
		if (tmp_ch_num == SAP_CHANNEL_NOT_SELECTED)
			continue;

		best_ch_num = tmp_ch_num;
		break;
	}
#else
	/* Sort the ch lst as per the computed weights, lesser weight first. */
	sap_sort_chl_weight_all(sap_ctx, hal, spect_info);
	/* Get the first channel in sorted array as best 20M Channel */
	best_ch_num = (uint8_t) spect_info->pSpectCh[0].chNum;
	/* Select Best Channel from Channel List if Configured */
	best_ch_num = sap_select_preferred_channel_from_channel_list(
					best_ch_num, sap_ctx, spect_info);
#endif

	/*
	 * in case the best channel seleted is not in PCL and there is another
	 * channel which has same weightage and is in PCL, choose the one in
	 * PCL
	 */
	for (count = 0; count < spect_info->numSpectChans; count++) {
		if (!ch_in_pcl(sap_ctx, spect_info->pSpectCh[count].chNum) ||
		    (spect_info->pSpectCh[count].weight !=
				sap_ctx->acsBestChannelInfo.weight))
			continue;

		if (sap_select_preferred_channel_from_channel_list(
			spect_info->pSpectCh[count].chNum, sap_ctx, spect_info)
			== SAP_CHANNEL_NOT_SELECTED)
			continue;

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		if (sap_check_in_avoid_ch_list(sap_ctx, best_ch_num))
			continue;
#endif
		best_ch_num = spect_info->pSpectCh[count].chNum;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("change best channel to %d in PCL"), best_ch_num);
		break;
	}

	sap_ctx->acs_cfg->pri_ch = best_ch_num;
	/* determine secondary channel for 2.4G channel 5, 6, 7 in HT40 */
	if ((operating_band != eCSR_DOT11_MODE_11g) ||
	    (sap_ctx->acs_cfg->ch_width != CH_WIDTH_40MHZ))
		goto sap_ch_sel_end;

	if ((best_ch_num >= 5) && (best_ch_num <= 7)) {
		int weight_below, weight_above, i;
		tSapSpectChInfo *pspect_info;

		weight_below = weight_above = SAP_ACS_WEIGHT_MAX;
		pspect_info = spect_info->pSpectCh;
		for (i = 0; i < spect_info->numSpectChans; i++) {
			if (pspect_info[i].chNum == (best_ch_num - 4))
				weight_below = pspect_info[i].weight;
			if (pspect_info[i].chNum == (best_ch_num + 4))
				weight_above = pspect_info[i].weight;
		}

		if (weight_below < weight_above)
			sap_ctx->acs_cfg->ht_sec_ch =
					sap_ctx->acs_cfg->pri_ch - 4;
		else
			sap_ctx->acs_cfg->ht_sec_ch =
					sap_ctx->acs_cfg->pri_ch + 4;
	} else if (best_ch_num >= 1 && best_ch_num <= 4) {
		sap_ctx->acs_cfg->ht_sec_ch = sap_ctx->acs_cfg->pri_ch + 4;
	} else if (best_ch_num >= 8 && best_ch_num <= 13) {
		sap_ctx->acs_cfg->ht_sec_ch = sap_ctx->acs_cfg->pri_ch - 4;
	} else if (best_ch_num == 14) {
		sap_ctx->acs_cfg->ht_sec_ch = 0;
	}
	sap_ctx->secondary_ch = sap_ctx->acs_cfg->ht_sec_ch;

sap_ch_sel_end:
	/* Free all the allocated memory */
	sap_chan_sel_exit(spect_info);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Running SAP Ch select Completed, Ch=%d"), best_ch_num);
	if (best_ch_num > 0 && best_ch_num <= 252)
		return best_ch_num;
	else
		return SAP_CHANNEL_NOT_SELECTED;
}
