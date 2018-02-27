/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

			s a p F s m . C

   OVERVIEW:

   This software unit holds the implementation of the WLAN SAP Finite
   State Machine modules

   DEPENDENCIES:

   Are listed for each API below.
   ===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "sap_internal.h"
#include <wlan_dfs_tgt_api.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_dfs_public_struct.h>
#include <wlan_reg_services_api.h>
/* Pick up the SME API definitions */
#include "sme_api.h"
/* Pick up the PMC API definitions */
#include "cds_utils.h"
#include "cds_ieee80211_common_i.h"
#include "cds_reg_service.h"
#include "qdf_util.h"
#include "wlan_policy_mgr_api.h"
#include "cfg_api.h"
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_utility.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <wlan_scan_ucfg_api.h>
#include "wlan_reg_services_api.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  External declarations for global context
 * -------------------------------------------------------------------------*/
#ifdef FEATURE_WLAN_CH_AVOID
extern sapSafeChannelType safe_channels[];
#endif /* FEATURE_WLAN_CH_AVOID */

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
#ifdef SOFTAP_CHANNEL_RANGE
static QDF_STATUS sap_get_channel_list(struct sap_context *sapContext,
				    uint8_t **channelList,
				    uint8_t *numberOfChannels);
#endif

/*==========================================================================
   FUNCTION    sapStopDfsCacTimer

   DESCRIPTION
    Function to sttop the DFS CAC timer when SAP is stopped
   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext: SAP Context
   RETURN VALUE
    DFS Timer start status
   SIDE EFFECTS
   ============================================================================*/

static int sap_stop_dfs_cac_timer(struct sap_context *sapContext);

/*==========================================================================
   FUNCTION    sapStartDfsCacTimer

   DESCRIPTION
    Function to start the DFS CAC timer when SAP is started on DFS Channel
   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext: SAP Context
   RETURN VALUE
    DFS Timer start status
   SIDE EFFECTS
   ============================================================================*/

static int sap_start_dfs_cac_timer(struct sap_context *sapContext);

/** sap_hdd_event_to_string() - convert hdd event to string
 * @event: eSapHddEvent event type
 *
 * This function converts eSapHddEvent into string
 *
 * Return: string for the @event.
 */
static uint8_t *sap_hdd_event_to_string(eSapHddEvent event)
{
	switch (event) {
	CASE_RETURN_STRING(eSAP_START_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_STOP_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_STA_ASSOC_IND);
	CASE_RETURN_STRING(eSAP_STA_ASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_REASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_DISASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_SET_KEY_EVENT);
	CASE_RETURN_STRING(eSAP_STA_MIC_FAILURE_EVENT);
	CASE_RETURN_STRING(eSAP_ASSOC_STA_CALLBACK_EVENT);
	CASE_RETURN_STRING(eSAP_WPS_PBC_PROBE_REQ_EVENT);
	CASE_RETURN_STRING(eSAP_DISCONNECT_ALL_P2P_CLIENT);
	CASE_RETURN_STRING(eSAP_MAC_TRIG_STOP_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_UNKNOWN_STA_JOIN);
	CASE_RETURN_STRING(eSAP_MAX_ASSOC_EXCEEDED);
	CASE_RETURN_STRING(eSAP_CHANNEL_CHANGE_EVENT);
	CASE_RETURN_STRING(eSAP_DFS_CAC_START);
	CASE_RETURN_STRING(eSAP_DFS_CAC_INTERRUPTED);
	CASE_RETURN_STRING(eSAP_DFS_CAC_END);
	CASE_RETURN_STRING(eSAP_DFS_PRE_CAC_END);
	CASE_RETURN_STRING(eSAP_DFS_RADAR_DETECT);
	CASE_RETURN_STRING(eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC);
	CASE_RETURN_STRING(eSAP_DFS_NO_AVAILABLE_CHANNEL);
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	CASE_RETURN_STRING(eSAP_ACS_SCAN_SUCCESS_EVENT);
#endif
	CASE_RETURN_STRING(eSAP_ACS_CHANNEL_SELECTED);
	CASE_RETURN_STRING(eSAP_ECSA_CHANGE_CHAN_IND);
	default:
		return "eSAP_HDD_EVENT_UNKNOWN";
	}
}

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/*==========================================================================
   FUNCTION    sap_event_init

   DESCRIPTION
    Function for initializing sWLAN_SAPEvent structure

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapEvent    : State machine event

   RETURN VALUE

    None

   SIDE EFFECTS
   ============================================================================*/
static inline void sap_event_init(ptWLAN_SAPEvent sapEvent)
{
	sapEvent->event = eSAP_MAC_SCAN_COMPLETE;
	sapEvent->params = 0;
	sapEvent->u1 = 0;
	sapEvent->u2 = 0;
}

/**
 * sap_random_channel_sel() - This function randomly pick up an available
 * channel
 * @sap_ctx: sap context.
 *
 * This function first eliminates invalid channel, then selects random channel
 * using following algorithm:
 *
 * Return: channel number picked
 **/
static uint8_t sap_random_channel_sel(struct sap_context *sap_ctx)
{
	uint8_t ch;
	uint8_t ch_wd;
	struct wlan_objmgr_pdev *pdev = NULL;
	tHalHandle hal;
	struct ch_params *ch_params;
	uint32_t hw_mode;
	tpAniSirGlobal mac_ctx;
	struct dfs_acs_info acs_info = {0};

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("null hal"));
		return 0;
	}

	mac_ctx = PMAC_STRUCT(hal);
	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("null mac_ctx"));
		return 0;
	}

	pdev = mac_ctx->pdev;
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("null pdev"));
		return 0;
	}

	ch_params = &mac_ctx->sap.SapDfsInfo.new_ch_params;
	if (mac_ctx->sap.SapDfsInfo.orig_chanWidth == 0) {
		ch_wd = sap_ctx->ch_width_orig;
		mac_ctx->sap.SapDfsInfo.orig_chanWidth = ch_wd;
	} else {
		ch_wd = mac_ctx->sap.SapDfsInfo.orig_chanWidth;
	}

	ch_params->ch_width = ch_wd;
	if (sap_ctx->acs_cfg) {
		acs_info.acs_mode = sap_ctx->acs_cfg->acs_mode;
		acs_info.start_ch = sap_ctx->acs_cfg->start_ch;
		acs_info.end_ch = sap_ctx->acs_cfg->end_ch;
	} else {
		acs_info.acs_mode = false;
	}
	if (QDF_IS_STATUS_ERROR(utils_dfs_get_random_channel(
	    pdev, 0, ch_params, &hw_mode, &ch, &acs_info))) {
		/* No available channel found */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("No available channel found!!!"));
		sap_signal_hdd_event(sap_ctx, NULL,
				eSAP_DFS_NO_AVAILABLE_CHANNEL,
				(void *)eSAP_STATUS_SUCCESS);
		return 0;
	}
	mac_ctx->sap.SapDfsInfo.new_chanWidth = ch_params->ch_width;
	sap_ctx->ch_params.ch_width = ch_params->ch_width;
	sap_ctx->ch_params.sec_ch_offset = ch_params->sec_ch_offset;
	sap_ctx->ch_params.center_freq_seg0 = ch_params->center_freq_seg0;
	sap_ctx->ch_params.center_freq_seg1 = ch_params->center_freq_seg1;
	return ch;
}

/**
 * sap_is_channel_bonding_etsi_weather_channel() - check weather chan bonding.
 * @sap_ctx: sap context
 *
 * Check if the current SAP operating channel is bonded to weather radar
 * channel in ETSI domain.
 *
 * Return: True if bonded to weather channel in ETSI
 */
static bool
sap_is_channel_bonding_etsi_weather_channel(struct sap_context *sap_ctx)
{
	if (IS_CH_BONDING_WITH_WEATHER_CH(sap_ctx->channel) &&
	    (sap_ctx->ch_params.ch_width != CH_WIDTH_20MHZ))
		return true;

	return false;
}

/*
 * sap_get_bonding_channels() - get bonding channels from primary channel.
 * @sapContext: Handle to SAP context.
 * @channel: Channel to get bonded channels.
 * @channels: Bonded channel list
 * @size: Max bonded channels
 * @chanBondState: The channel bonding mode of the passed channel.
 *
 * Return: Number of sub channels
 */
static uint8_t sap_get_bonding_channels(struct sap_context *sapContext,
					uint8_t channel,
					uint8_t *channels, uint8_t size,
					ePhyChanBondState chanBondState)
{
	tHalHandle hHal = CDS_GET_HAL_CB();
	tpAniSirGlobal pMac;
	uint8_t numChannel;

	if (channels == NULL)
		return 0;

	if (size < MAX_BONDED_CHANNELS)
		return 0;

	if (NULL != hHal)
		pMac = PMAC_STRUCT(hHal);
	else
		return 0;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  FL("cbmode: %d, channel: %d"), chanBondState, channel);

	switch (chanBondState) {
	case PHY_SINGLE_CHANNEL_CENTERED:
		numChannel = 1;
		channels[0] = channel;
		break;
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		numChannel = 2;
		channels[0] = channel - 4;
		channels[1] = channel;
		break;
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		numChannel = 2;
		channels[0] = channel;
		channels[1] = channel + 4;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
		numChannel = 4;
		channels[0] = channel;
		channels[1] = channel + 4;
		channels[2] = channel + 8;
		channels[3] = channel + 12;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
		numChannel = 4;
		channels[0] = channel - 4;
		channels[1] = channel;
		channels[2] = channel + 4;
		channels[3] = channel + 8;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
		numChannel = 4;
		channels[0] = channel - 8;
		channels[1] = channel - 4;
		channels[2] = channel;
		channels[3] = channel + 4;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
		numChannel = 4;
		channels[0] = channel - 12;
		channels[1] = channel - 8;
		channels[2] = channel - 4;
		channels[3] = channel;
		break;
	default:
		numChannel = 1;
		channels[0] = channel;
		break;
	}

	return numChannel;
}

/**
 * sap_ch_params_to_bonding_channels() - get bonding channels from channel param
 * @ch_params: channel params ( bw, pri and sec channel info)
 * @channels: bonded channel list
 *
 * Return: Number of sub channels
 */
static uint8_t sap_ch_params_to_bonding_channels(
		struct ch_params *ch_params,
		uint8_t *channels)
{
	uint8_t center_chan = ch_params->center_freq_seg0;
	uint8_t nchannels = 0;

	switch (ch_params->ch_width) {
	case CH_WIDTH_160MHZ:
		nchannels = 8;
		center_chan = ch_params->center_freq_seg1;
		channels[0] = center_chan - 14;
		channels[1] = center_chan - 10;
		channels[2] = center_chan - 6;
		channels[3] = center_chan - 2;
		channels[4] = center_chan + 2;
		channels[5] = center_chan + 6;
		channels[6] = center_chan + 10;
		channels[7] = center_chan + 14;
		break;
	case CH_WIDTH_80P80MHZ:
		nchannels = 8;
		channels[0] = center_chan - 6;
		channels[1] = center_chan - 2;
		channels[2] = center_chan + 2;
		channels[3] = center_chan + 6;

		center_chan = ch_params->center_freq_seg1;
		channels[4] = center_chan - 6;
		channels[5] = center_chan - 2;
		channels[6] = center_chan + 2;
		channels[7] = center_chan + 6;
		break;
	case CH_WIDTH_80MHZ:
		nchannels = 4;
		channels[0] = center_chan - 6;
		channels[1] = center_chan - 2;
		channels[2] = center_chan + 2;
		channels[3] = center_chan + 6;
		break;
	case CH_WIDTH_40MHZ:
		nchannels = 2;
		channels[0] = center_chan - 2;
		channels[1] = center_chan + 2;
		break;
	default:
		nchannels = 1;
		channels[0] = center_chan;
		break;
	}

	return nchannels;
}

/**
 * sap_get_cac_dur_dfs_region() - get cac duration and dfs region.
 * @sap_ctxt: sap context
 * @cac_duration_ms: pointer to cac duration
 * @dfs_region: pointer to dfs region
 *
 * Get cac duration and dfs region.
 *
 * Return: None
 */
static void sap_get_cac_dur_dfs_region(struct sap_context *sap_ctx,
		uint32_t *cac_duration_ms,
		uint32_t *dfs_region)
{
	int i;
	uint8_t channels[MAX_BONDED_CHANNELS];
	uint8_t num_channels;
	struct ch_params *ch_params = &sap_ctx->ch_params;
	tHalHandle hal = NULL;
	tpAniSirGlobal mac = NULL;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: null sap_ctx", __func__);
		return;
	}

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: null hal", __func__);
		return;
	}

	mac = PMAC_STRUCT(hal);
	wlan_reg_get_dfs_region(mac->pdev, dfs_region);
	if (mac->sap.SapDfsInfo.ignore_cac) {
		*cac_duration_ms = 0;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: ignore_cac is set", __func__);
		return;
	}
	*cac_duration_ms = DEFAULT_CAC_TIMEOUT;

	if (*dfs_region != DFS_ETSI_REG) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			 FL("sapdfs: defult cac duration"));
		return;
	}

	if (sap_is_channel_bonding_etsi_weather_channel(sap_ctx)) {
		*cac_duration_ms = ETSI_WEATHER_CH_CAC_TIMEOUT;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("sapdfs: bonding_etsi_weather_channel"));
		return;
	}

	qdf_mem_zero(channels, sizeof(channels));
	num_channels = sap_ch_params_to_bonding_channels(ch_params, channels);
	for (i = 0; i < num_channels; i++) {
		if (IS_ETSI_WEATHER_CH(channels[i])) {
			*cac_duration_ms = ETSI_WEATHER_CH_CAC_TIMEOUT;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("sapdfs: ch=%d is etsi weather channel"),
				  channels[i]);
			return;
		}
	}

}

void sap_dfs_set_current_channel(void *ctx)
{
	struct sap_context *sap_ctx = ctx;
	uint32_t ic_flags = 0;
	uint16_t ic_flagext = 0;
	uint8_t ic_ieee = sap_ctx->channel;
	uint16_t ic_freq = utils_dfs_chan_to_freq(sap_ctx->channel);
	uint8_t vht_seg0 = sap_ctx->csr_roamProfile.ch_params.center_freq_seg0;
	uint8_t vht_seg1 = sap_ctx->csr_roamProfile.ch_params.center_freq_seg1;
	struct wlan_objmgr_pdev *pdev;
	tpAniSirGlobal mac_ctx;
	tHalHandle hal;
	uint32_t use_nol = 0;
	int error;

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("null hal"));
		return;
	}

	mac_ctx = PMAC_STRUCT(hal);
	pdev = mac_ctx->pdev;
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("null pdev"));
		return;
	}

	switch (sap_ctx->csr_roamProfile.ch_params.ch_width) {
	case CH_WIDTH_20MHZ:
		ic_flags |= IEEE80211_CHAN_VHT20;
		break;
	case CH_WIDTH_40MHZ:
		ic_flags |= IEEE80211_CHAN_VHT40PLUS;
		break;
	case CH_WIDTH_80MHZ:
		ic_flags |= IEEE80211_CHAN_VHT80;
		break;
	case CH_WIDTH_80P80MHZ:
		ic_flags |= IEEE80211_CHAN_VHT80_80;
		break;
	case CH_WIDTH_160MHZ:
		ic_flags |= IEEE80211_CHAN_VHT160;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid channel width=%d"),
			  sap_ctx->csr_roamProfile.ch_params.ch_width);
		return;
	}

	if (WLAN_REG_IS_24GHZ_CH(sap_ctx->channel))
		ic_flags |= IEEE80211_CHAN_2GHZ;
	else
		ic_flags |= IEEE80211_CHAN_5GHZ;

	if (wlan_reg_is_dfs_ch(pdev, sap_ctx->channel))
		ic_flagext |= IEEE80211_CHAN_DFS;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		  FL("freq=%d, channel=%d, seg0=%d, seg1=%d, flags=0x%x, ext flags=0x%x"),
		  ic_freq, ic_ieee, vht_seg0, vht_seg1, ic_flags, ic_flagext);

	tgt_dfs_set_current_channel(pdev, ic_freq, ic_flags,
			ic_flagext, ic_ieee, vht_seg0, vht_seg1);

	if (wlan_reg_is_dfs_ch(pdev, sap_ctx->channel)) {
		tgt_dfs_get_radars(pdev);
		if (sap_ctx->csr_roamProfile.disableDFSChSwitch)
			tgt_dfs_control(pdev, DFS_SET_USENOL, &use_nol,
					sizeof(uint32_t), NULL, NULL, &error);
	}
}

/*
 * FUNCTION  sap_dfs_is_w53_invalid
 *
 * DESCRIPTION Checks if the passed channel is W53 and returns if
 *             SAP W53 opearation is allowed.
 *
 * DEPENDENCIES PARAMETERS
 * IN hHAL : HAL pointer
 * channelID: Channel Number to be verified
 *
 * RETURN VALUE  : bool
 *                 true: If W53 operation is disabled
 *                 false: If W53 operation is enabled
 *
 * SIDE EFFECTS
 */
bool sap_dfs_is_w53_invalid(tHalHandle hHal, uint8_t channelID)
{
	tpAniSirGlobal pMac;

	pMac = PMAC_STRUCT(hHal);
	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid pMac"));
		return false;
	}

	/*
	 * Check for JAPAN W53 Channel operation capability
	 */
	if (true == pMac->sap.SapDfsInfo.is_dfs_w53_disabled &&
	    true == IS_CHAN_JAPAN_W53(channelID)) {
		return true;
	}

	return false;
}

/*
 * FUNCTION  sap_dfs_is_channel_in_preferred_location
 *
 * DESCRIPTION Checks if the passed channel is in accordance with preferred
 *          Channel location settings.
 *
 * DEPENDENCIES PARAMETERS
 * IN hHAL : HAL pointer
 * channelID: Channel Number to be verified
 *
 * RETURN VALUE  :bool
 *        true:If Channel location is same as the preferred location
 *        false:If Channel location is not same as the preferred location
 *
 * SIDE EFFECTS
 */
bool sap_dfs_is_channel_in_preferred_location(tHalHandle hHal, uint8_t channelID)
{
	tpAniSirGlobal pMac;

	pMac = PMAC_STRUCT(hHal);
	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid pMac"));
		return true;
	}
	if ((SAP_CHAN_PREFERRED_INDOOR ==
	     pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location) &&
	    (true == IS_CHAN_JAPAN_OUTDOOR(channelID))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("CHAN=%d is Outdoor so invalid,preferred Indoor only"),
			  channelID);
		return false;
	} else if ((SAP_CHAN_PREFERRED_OUTDOOR ==
		    pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location)
		   && (true == IS_CHAN_JAPAN_INDOOR(channelID))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("CHAN=%d is Indoor so invalid,preferred Outdoor only"),
			  channelID);
		return false;
	}

	return true;
}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * sap_check_in_avoid_ch_list() - checks if given channel present is channel
 * avoidance list
 *
 * @sap_ctx:        sap context.
 * @channel:        channel to be checked in sap_ctx's avoid ch list
 *
 * sap_ctx contains sap_avoid_ch_info strcut containing the list of channels on
 * which MDM device's AP with MCC was detected. This function checks if given
 * channel is present in that list.
 *
 * Return: true, if channel was present, false othersie.
 */
bool sap_check_in_avoid_ch_list(struct sap_context *sap_ctx, uint8_t channel)
{
	uint8_t i = 0;
	struct sap_avoid_channels_info *ie_info =
		&sap_ctx->sap_detected_avoid_ch_ie;
	for (i = 0; i < sizeof(ie_info->channels); i++)
		if (ie_info->channels[i] == channel)
			return true;
	return false;
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

/**
 * sap_dfs_is_channel_in_nol_list() - given bonded channel is available
 * @sap_context: Handle to SAP context.
 * @channel_number: Channel on which availability should be checked.
 * @chan_bondState: The channel bonding mode of the passed channel.
 *
 * This function Checks if a given bonded channel is available or
 * usable for DFS operation.
 *
 * Return: false if channel is available, true if channel is in NOL.
 */
bool
sap_dfs_is_channel_in_nol_list(struct sap_context *sap_context,
			       uint8_t channel_number,
			       ePhyChanBondState chan_bondState)
{
	int i;
	tHalHandle h_hal = CDS_GET_HAL_CB();
	tpAniSirGlobal mac_ctx;
	uint8_t channels[MAX_BONDED_CHANNELS];
	uint8_t num_channels;
	struct wlan_objmgr_pdev *pdev = NULL;
	enum channel_state ch_state;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid h_hal"));
		return false;
	} else {
		mac_ctx = PMAC_STRUCT(h_hal);
	}

	/* get the bonded channels */
	if (channel_number == sap_context->channel && chan_bondState >=
						PHY_CHANNEL_BONDING_STATE_MAX)
		num_channels = sap_ch_params_to_bonding_channels(
					&sap_context->ch_params, channels);
	else
		num_channels = sap_get_bonding_channels(sap_context,
					channel_number, channels,
					MAX_BONDED_CHANNELS, chan_bondState);

	pdev = mac_ctx->pdev;
	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("null pdev"));
		return false;
	}

	/* check for NOL, first on will break the loop */
	for (i = 0; i < num_channels; i++) {
		ch_state = wlan_reg_get_channel_state(pdev, channels[i]);
		if (CHANNEL_STATE_ENABLE != ch_state &&
		    CHANNEL_STATE_DFS != ch_state) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid ch num=%d, ch state=%d"),
				  channels[i], ch_state);
			return true;
		}
	} /* loop for bonded channels */

	return false;
}

uint8_t sap_select_default_oper_chan(struct sap_acs_cfg *acs_cfg)
{
	uint8_t channel;

	if (NULL == acs_cfg) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"ACS config invalid!");
		QDF_BUG(0);
		return 0;
	}

	if (acs_cfg->hw_mode == eCSR_DOT11_MODE_11a) {
		channel = SAP_DEFAULT_5GHZ_CHANNEL;
	} else if ((acs_cfg->hw_mode == eCSR_DOT11_MODE_11n) ||
		   (acs_cfg->hw_mode == eCSR_DOT11_MODE_11n_ONLY) ||
		   (acs_cfg->hw_mode == eCSR_DOT11_MODE_11ac) ||
		   (acs_cfg->hw_mode == eCSR_DOT11_MODE_11ac_ONLY) ||
		   (acs_cfg->hw_mode == eCSR_DOT11_MODE_11ax) ||
		   (acs_cfg->hw_mode == eCSR_DOT11_MODE_11ax_ONLY)) {
		if (WLAN_REG_IS_5GHZ_CH(acs_cfg->start_ch))
			channel = SAP_DEFAULT_5GHZ_CHANNEL;
		else
			channel = SAP_DEFAULT_24GHZ_CHANNEL;
	} else {
		channel = SAP_DEFAULT_24GHZ_CHANNEL;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			FL("channel selected to start bss %d"), channel);
	return channel;
}

/**
 * sap_goto_channel_sel - Function for initiating scan request for SME
 * @sap_context: Sap Context value.
 * @sap_event: State machine event
 * @sap_do_acs_pre_start_bss: true, if ACS scan is issued pre start BSS
 *                            false, if ACS scan is issued post start BSS.
 * @check_for_connection_update: true, check and wait for connection update
 *                               false, do not perform connection update
 *
 * Initiates sme scan for ACS to pick a channel.
 *
 * Return: The QDF_STATUS code associated with performing the operation.
 */
QDF_STATUS sap_goto_channel_sel(struct sap_context *sap_context,
	ptWLAN_SAPEvent sap_event,
	bool sap_do_acs_pre_start_bss,
	bool check_for_connection_update)
{

	/* Initiate a SCAN request */
	QDF_STATUS qdf_ret_status;
	/* To be initialised if scan is required */
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx;
	struct scan_start_request *req;
	struct wlan_objmgr_vdev *vdev;
	uint8_t i;

#ifdef SOFTAP_CHANNEL_RANGE
	uint8_t *channel_list = NULL;
	uint8_t num_of_channels = 0;
#endif
	tHalHandle h_hal;
	uint8_t con_ch;

	h_hal = cds_get_context(QDF_MODULE_ID_SME);
	if (NULL == h_hal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  FL("invalid h_hal"));
		return QDF_STATUS_E_FAULT;
	}

	mac_ctx = PMAC_STRUCT(h_hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}
	if (policy_mgr_concurrent_beaconing_sessions_running(mac_ctx->psoc) ||
	   ((sap_context->cc_switch_mode ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION) &&
	   (policy_mgr_mode_specific_connection_count(mac_ctx->psoc,
		PM_SAP_MODE, NULL) ||
	     policy_mgr_mode_specific_connection_count(mac_ctx->psoc,
		PM_P2P_GO_MODE, NULL)))) {
		con_ch =
			sme_get_concurrent_operation_channel(h_hal);
#ifdef FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
		if (con_ch && sap_context->channel == AUTO_CHANNEL_SELECT) {
			sap_context->dfs_ch_disable = true;
		} else if (con_ch && sap_context->channel != con_ch &&
			   wlan_reg_is_dfs_ch(mac_ctx->pdev,
				   sap_context->channel)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("MCC DFS not supported in AP_AP Mode"));
			return QDF_STATUS_E_ABORTED;
		}
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (sap_context->cc_switch_mode !=
					QDF_MCC_TO_SCC_SWITCH_DISABLE &&
					sap_context->channel) {
			/*
			 * For ACS request ,the sapContext->channel is 0,
			 * we skip below overlap checking. When the ACS
			 * finish and SAPBSS start, the sapContext->channel
			 * will not be 0. Then the overlap checking will be
			 * reactivated.If we use sapContext->channel = 0
			 * to perform the overlap checking, an invalid overlap
			 * channel con_ch could becreated. That may cause
			 * SAP start failed.
			 */
			con_ch = sme_check_concurrent_channel_overlap(h_hal,
					sap_context->channel,
					sap_context->csr_roamProfile.phyMode,
					sap_context->cc_switch_mode);
			if (con_ch && !(wlan_reg_is_dfs_ch(mac_ctx->pdev,
						con_ch) &&
			sap_context->cc_switch_mode ==
	QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					"%s: Override ch %d to %d due to CC Intf",
					__func__, sap_context->channel, con_ch);
				sap_context->channel = con_ch;
				wlan_reg_set_channel_params(mac_ctx->pdev,
						sap_context->channel, 0,
						&sap_context->ch_params);
			}
		}
#endif
	}

	if ((policy_mgr_get_concurrency_mode(mac_ctx->psoc) ==
		(QDF_STA_MASK | QDF_SAP_MASK)) ||
	    (policy_mgr_get_concurrency_mode(mac_ctx->psoc) ==
		(QDF_STA_MASK | QDF_P2P_GO_MASK))) {
#ifdef FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
		if (sap_context->channel == AUTO_CHANNEL_SELECT)
			sap_context->dfs_ch_disable = true;
		else if (wlan_reg_is_dfs_ch(mac_ctx->pdev,
					sap_context->channel)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("DFS not supported in STA_AP Mode"));
			return QDF_STATUS_E_ABORTED;
		}
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (sap_context->cc_switch_mode !=
					QDF_MCC_TO_SCC_SWITCH_DISABLE &&
					sap_context->channel) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				FL("check for overlap: chan:%d mode:%d"),
				sap_context->channel,
				sap_context->csr_roamProfile.phyMode);
			con_ch = sme_check_concurrent_channel_overlap(h_hal,
					sap_context->channel,
					sap_context->csr_roamProfile.phyMode,
					sap_context->cc_switch_mode);
			if (sap_context->cc_switch_mode !=
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION) {
				if (QDF_IS_STATUS_ERROR(
					policy_mgr_valid_sap_conc_channel_check(
						mac_ctx->psoc, &con_ch,
						sap_context->channel)))	{
					QDF_TRACE(QDF_MODULE_ID_SAP,
						QDF_TRACE_LEVEL_WARN,
						FL("SAP can't start (no MCC)"));
					return QDF_STATUS_E_ABORTED;
				}
			}
			if (con_ch && !wlan_reg_is_dfs_ch(mac_ctx->pdev,
						con_ch)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					"%s: Override ch %d to %d due to CC Intf",
					__func__, sap_context->channel, con_ch);
				sap_context->channel = con_ch;
				wlan_reg_set_channel_params(mac_ctx->pdev,
						sap_context->channel, 0,
						&sap_context->ch_params);
			}
		}
#endif
	}

	if (sap_context->channel == AUTO_CHANNEL_SELECT) {
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("%s skip_acs_status = %d "), __func__,
			  sap_context->acs_cfg->skip_scan_status);
		if (sap_context->acs_cfg->skip_scan_status !=
						eSAP_SKIP_ACS_SCAN) {
#endif

		req = qdf_mem_malloc(sizeof(*req));
		if (!req) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Failed to allocate memory"));
			return QDF_STATUS_E_NOMEM;
		}
		vdev = wlan_objmgr_get_vdev_by_macaddr_from_psoc(
						mac_ctx->psoc,
						sap_context->self_mac_addr,
						WLAN_LEGACY_SME_ID);
		if (!vdev) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid vdev objmgr"));
			return QDF_STATUS_E_INVAL;
		}

		ucfg_scan_init_default_params(vdev, req);
		req->scan_req.dwell_time_active = 0;
		req->scan_req.scan_id = ucfg_scan_get_scan_id(mac_ctx->psoc);
		req->scan_req.vdev_id = wlan_vdev_get_id(vdev);
		req->scan_req.scan_req_id = sap_context->req_id;
		sap_get_channel_list(sap_context, &channel_list,
				  &num_of_channels);
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (num_of_channels != 0) {
#endif

		req->scan_req.chan_list.num_chan = num_of_channels;
		for (i = 0; i < num_of_channels; i++)
			req->scan_req.chan_list.chan[i].freq =
				wlan_chan_to_freq(channel_list[i]);
		if (sap_context->channelList) {
			qdf_mem_free(sap_context->channelList);
			sap_context->channelList = NULL;
			sap_context->num_of_channel = 0;
		}
		sap_context->channelList = channel_list;
		sap_context->num_of_channel = num_of_channels;
		/* Set requestType to Full scan */

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("calling ucfg_scan_start"));
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (sap_context->acs_cfg->skip_scan_status ==
			eSAP_DO_NEW_ACS_SCAN)
#endif
			sme_scan_flush_result(h_hal);
		sap_context->sap_acs_pre_start_bss = sap_do_acs_pre_start_bss;
		qdf_ret_status = ucfg_scan_start(req);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

		if (QDF_STATUS_SUCCESS != qdf_ret_status) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("scan request  fail %d!!!"),
				  qdf_ret_status);
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("SAP Configuring default channel, Ch=%d"),
				  sap_context->channel);
			sap_context->channel = sap_select_default_oper_chan(
					sap_context->acs_cfg);

#ifdef SOFTAP_CHANNEL_RANGE
			if (sap_context->channelList != NULL) {
				sap_context->channel =
					sap_context->channelList[0];
				qdf_mem_free(sap_context->
					channelList);
				sap_context->channelList = NULL;
				sap_context->num_of_channel = 0;
			}
#endif
			if (true == sap_do_acs_pre_start_bss) {
				/*
				* In case of ACS req before start Bss,
				* return failure so that the calling
				* fucntion can use the default channel.
				*/
				return QDF_STATUS_E_FAILURE;
			} else {
				/* Fill in the event structure */
				sap_event_init(sap_event);
				/* Handle event */
				qdf_status = sap_fsm(sap_context, sap_event);
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				 FL("return sme_ScanReq, scanID=%d, Ch=%d"),
				req->scan_req.scan_id,
				 sap_context->channel);
		}
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		}
	} else {
		sap_context->acs_cfg->skip_scan_status = eSAP_SKIP_ACS_SCAN;
	}

	if (sap_context->acs_cfg->skip_scan_status == eSAP_SKIP_ACS_SCAN) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("## %s SKIPPED ACS SCAN"), __func__);

		if (true == sap_do_acs_pre_start_bss)
			wlansap_pre_start_bss_acs_scan_callback(h_hal,
				sap_context, sap_context->sessionId, 0,
				eCSR_SCAN_SUCCESS);
		else
			wlansap_scan_callback(h_hal, sap_context,
				sap_context->sessionId, 0, eCSR_SCAN_SUCCESS);
	}
#endif
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("for configured channel, Ch= %d"),
			  sap_context->channel);

		if (check_for_connection_update) {
			/* This wait happens in the hostapd context. The event
			 * is set in the MC thread context.
			 */
			qdf_status =
			policy_mgr_update_and_wait_for_connection_update(
					mac_ctx->psoc,
					sap_context->sessionId,
					sap_context->channel,
					SIR_UPDATE_REASON_START_AP);
			if (QDF_IS_STATUS_ERROR(qdf_status))
				return qdf_status;
		}

		if (sap_do_acs_pre_start_bss == true) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				FL("ACS end due to Ch override. Sel Ch = %d"),
							sap_context->channel);
			sap_context->acs_cfg->pri_ch = sap_context->channel;
			sap_context->acs_cfg->ch_width =
						 sap_context->ch_width_orig;
			sap_config_acs_result(h_hal, sap_context, 0);
			return QDF_STATUS_E_CANCELED;
		} else {
			/*
			 * Fill in the event structure
			 * Eventhough scan was not done,
			 * means a user set channel was chosen
			 */
			sap_event_init(sap_event);
			/* Handle event */
			qdf_status = sap_fsm(sap_context, sap_event);
		}
	}

	/*
	 * If scan failed, get default channel and advance state
	 * machine as success with default channel
	 *
	 * Have to wait for the call back to be called to get the
	 * channel cannot advance state machine here as said above
	 */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("before exiting sap_goto_channel_sel channel=%d"),
		  sap_context->channel);

	return QDF_STATUS_SUCCESS;
}

/**
 * sap_find_valid_concurrent_session() - to find valid concurrent session
 * @hal: pointer to hal abstration layer
 *
 * This API will check if any valid concurrent SAP session is present
 *
 * Return: pointer to sap context of valid concurrent session
 */
static struct sap_context *sap_find_valid_concurrent_session(tHalHandle hal)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	uint8_t intf = 0;
	struct sap_context *sap_ctx;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona) ||
		     (QDF_P2P_GO_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
		    mac_ctx->sap.sapCtxList[intf].pSapContext != NULL) {
			sap_ctx = mac_ctx->sap.sapCtxList[intf].pSapContext;
			if (sap_ctx->sapsMachine != eSAP_DISCONNECTED)
				return sap_ctx;
		}
	}

	return NULL;
}

static QDF_STATUS sap_clear_global_dfs_param(tHalHandle hal)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL != sap_find_valid_concurrent_session(hal)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  "conc session exists, no need to clear dfs struct");
		return QDF_STATUS_SUCCESS;
	}
	/*
	 * CAC timer will be initiated and started only when SAP starts
	 * on DFS channel and it will be stopped and destroyed
	 * immediately once the radar detected or timedout. So
	 * as per design CAC timer should be destroyed after stop
	 */
	if (mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running) {
		qdf_mc_timer_stop(&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
		mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
		qdf_mc_timer_destroy(
			&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
	}
	mac_ctx->sap.SapDfsInfo.cac_state = eSAP_DFS_DO_NOT_SKIP_CAC;
	sap_cac_reset_notify(hal);
	qdf_mem_zero(&mac_ctx->sap, sizeof(mac_ctx->sap));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sap_set_session_param(tHalHandle hal, struct sap_context *sapctx,
				uint32_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	sapctx->sessionId = session_id;
	sapctx->is_pre_cac_on = false;
	sapctx->pre_cac_complete = false;
	sapctx->chan_before_pre_cac = 0;
	mac_ctx->sap.sapCtxList[sapctx->sessionId].sessionID =
				sapctx->sessionId;
	mac_ctx->sap.sapCtxList[sapctx->sessionId].pSapContext = sapctx;
	mac_ctx->sap.sapCtxList[sapctx->sessionId].sapPersona =
				sapctx->csr_roamProfile.csrPersona;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		"%s: Initializing sapContext = %pK with session = %d", __func__,
		sapctx, session_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sap_clear_session_param(tHalHandle hal, struct sap_context *sapctx,
				uint32_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (sapctx->sessionId >= SAP_MAX_NUM_SESSION)
		return QDF_STATUS_E_FAILURE;

	mac_ctx->sap.sapCtxList[sapctx->sessionId].sessionID =
		CSR_SESSION_ID_INVALID;
	mac_ctx->sap.sapCtxList[sapctx->sessionId].pSapContext = NULL;
	mac_ctx->sap.sapCtxList[sapctx->sessionId].sapPersona =
		QDF_MAX_NO_OF_MODE;
	sap_clear_global_dfs_param(hal);
	sap_free_roam_profile(&sapctx->csr_roamProfile);
	qdf_mem_zero(sapctx, sizeof(*sapctx));
	sapctx->sessionId = CSR_SESSION_ID_INVALID;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		"%s: Initializing State: %d, sapContext value = %pK", __func__,
		sapctx->sapsMachine, sapctx);

	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    sapGotoStarting

   DESCRIPTION
    Function for initiating start bss request for SME

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    bssType     : Type of bss to start, INRA AP
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_goto_starting(struct sap_context *sapContext,
				    ptWLAN_SAPEvent sapEvent,
				    eCsrRoamBssType bssType)
{
	/* tHalHandle */
	tHalHandle hHal = CDS_GET_HAL_CB();
	QDF_STATUS qdf_ret_status;

	/*- - - - - - - - TODO:once configs from hdd available - - - - - - - - -*/
	char key_material[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
		4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, };
	sapContext->key_type = 0x05;
	sapContext->key_length = 32;
	/* Need a key size define */
	qdf_mem_copy(sapContext->key_material, key_material,
		     sizeof(key_material));

	if (NULL == hHal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "In %s, invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, "%s: session: %d",
		  __func__, sapContext->sessionId);

	qdf_ret_status = sme_roam_connect(hHal, sapContext->sessionId,
					  &sapContext->csr_roamProfile,
					  &sapContext->csr_roamId);
	if (QDF_STATUS_SUCCESS != qdf_ret_status)
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to issue sme_roam_connect", __func__);

	return qdf_ret_status;
} /* sapGotoStarting */

/*==========================================================================
   FUNCTION    sapGotoDisconnecting

   DESCRIPTION
    Processing of SAP FSM Disconnecting state

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_goto_disconnecting(struct sap_context *sapContext)
{
	QDF_STATUS qdf_ret_status;
	tHalHandle hHal;

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s, invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_free_roam_profile(&sapContext->csr_roamProfile);
	qdf_ret_status = sme_roam_stop_bss(hHal, sapContext->sessionId);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Error: In %s calling sme_roam_stop_bss status = %d",
			  __func__, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    sapGotoDisconnected

   DESCRIPTION
    Function for setting the SAP FSM to Disconnection state

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_goto_disconnected(struct sap_context *sapContext)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tWLAN_SAPEvent sapEvent;
	/* Processing has to be coded */
	/* Clean up stations from TL etc as AP BSS is shut down then set event */
	sapEvent.event = eSAP_MAC_READY_FOR_CONNECTIONS;        /* hardcoded */
	sapEvent.params = 0;
	sapEvent.u1 = 0;
	sapEvent.u2 = 0;
	/* Handle event */
	qdf_status = sap_fsm(sapContext, &sapEvent);

	return qdf_status;
}

#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
/**
 * sap_handle_acs_scan_event() - handle acs scan event for SAP
 * @sap_context: ptSapContext
 * @sap_event: tSap_Event
 * @status: status of acs scan
 *
 * The function is to handle the eSAP_ACS_SCAN_SUCCESS_EVENT event.
 *
 * Return: void
 */
static void sap_handle_acs_scan_event(struct sap_context *sap_context,
		tSap_Event *sap_event, eSapStatus status)
{
	sap_event->sapHddEventCode = eSAP_ACS_SCAN_SUCCESS_EVENT;
	sap_event->sapevt.sap_acs_scan_comp.status = status;
	sap_event->sapevt.sap_acs_scan_comp.num_of_channels =
			sap_context->num_of_channel;
	sap_event->sapevt.sap_acs_scan_comp.channellist =
			sap_context->channelList;
}
#else
static void sap_handle_acs_scan_event(struct sap_context *sap_context,
		tSap_Event *sap_event, eSapStatus status)
{
}
#endif

/**
 * sap_signal_hdd_event() - send event notification
 * @sap_ctx: Sap Context
 * @csr_roaminfo: Pointer to CSR roam information
 * @sap_hddevent: SAP HDD event
 * @context: to pass the element for future support
 *
 * Function for HDD to send the event notification using callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sap_signal_hdd_event(struct sap_context *sap_ctx,
		struct csr_roam_info *csr_roaminfo, eSapHddEvent sap_hddevent,
		void *context)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tSap_Event sap_ap_event;       /* This now encodes ALL event types */
	tHalHandle hal = CDS_GET_HAL_CB();
	tpAniSirGlobal mac_ctx;
	tSirSmeChanInfo *chaninfo;
	tSap_StationAssocIndication *assoc_ind;
	tSap_StartBssCompleteEvent *bss_complete;
	struct sap_ch_selected_s *acs_selected;
	tSap_StationAssocReassocCompleteEvent *reassoc_complete;
	tSap_StationDisassocCompleteEvent *disassoc_comp;
	tSap_StationSetKeyCompleteEvent *key_complete;
	tSap_StationMICFailureEvent *mic_failure;

	/* Format the Start BSS Complete event to return... */
	if (NULL == sap_ctx->pfnSapEventCallback) {
		return QDF_STATUS_E_FAILURE;
	}
	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid hal"));
		return QDF_STATUS_E_FAILURE;
	}
	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("SAP event callback event = %s"),
		  sap_hdd_event_to_string(sap_hddevent));

	switch (sap_hddevent) {
	case eSAP_STA_ASSOC_IND:
		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		/*  TODO - Indicate the assoc request indication to OS */
		sap_ap_event.sapHddEventCode = eSAP_STA_ASSOC_IND;
		assoc_ind = &sap_ap_event.sapevt.sapAssocIndication;

		qdf_copy_macaddr(&assoc_ind->staMac, &csr_roaminfo->peerMac);
		assoc_ind->staId = csr_roaminfo->staId;
		assoc_ind->status = 0;
		/* Required for indicating the frames to upper layer */
		assoc_ind->beaconLength = csr_roaminfo->beaconLength;
		assoc_ind->beaconPtr = csr_roaminfo->beaconPtr;
		assoc_ind->assocReqLength = csr_roaminfo->assocReqLength;
		assoc_ind->assocReqPtr = csr_roaminfo->assocReqPtr;
		assoc_ind->fWmmEnabled = csr_roaminfo->wmmEnabledSta;
		assoc_ind->ecsa_capable = csr_roaminfo->ecsa_capable;
		if (csr_roaminfo->u.pConnectedProfile != NULL) {
			assoc_ind->negotiatedAuthType =
				csr_roaminfo->u.pConnectedProfile->AuthType;
			assoc_ind->negotiatedUCEncryptionType =
			    csr_roaminfo->u.pConnectedProfile->EncryptionType;
			assoc_ind->negotiatedMCEncryptionType =
			    csr_roaminfo->u.pConnectedProfile->mcEncryptionType;
			assoc_ind->fAuthRequired = csr_roaminfo->fAuthRequired;
		}
		break;
	case eSAP_START_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_START_BSS_EVENT;
		bss_complete = &sap_ap_event.sapevt.sapStartBssCompleteEvent;

		bss_complete->status = (eSapStatus) context;
		bss_complete->staId = sap_ctx->sap_sta_id;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("(eSAP_START_BSS_EVENT): staId = %d"),
			  bss_complete->staId);

		bss_complete->operatingChannel = (uint8_t) sap_ctx->channel;
		bss_complete->sessionId = sap_ctx->sessionId;
		break;
	case eSAP_DFS_CAC_START:
	case eSAP_DFS_CAC_INTERRUPTED:
	case eSAP_DFS_CAC_END:
	case eSAP_DFS_PRE_CAC_END:
	case eSAP_DFS_RADAR_DETECT:
	case eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC:
	case eSAP_DFS_NO_AVAILABLE_CHANNEL:
		sap_ap_event.sapHddEventCode = sap_hddevent;
		sap_ap_event.sapevt.sapStopBssCompleteEvent.status =
			(eSapStatus) context;
		break;
	case eSAP_ACS_SCAN_SUCCESS_EVENT:
		sap_handle_acs_scan_event(sap_ctx, &sap_ap_event,
			(eSapStatus)context);
		break;
	case eSAP_ACS_CHANNEL_SELECTED:
		sap_ap_event.sapHddEventCode = sap_hddevent;
		acs_selected = &sap_ap_event.sapevt.sap_ch_selected;
		if (eSAP_STATUS_SUCCESS == (eSapStatus)context) {
			acs_selected->pri_ch = sap_ctx->acs_cfg->pri_ch;
			acs_selected->ht_sec_ch = sap_ctx->acs_cfg->ht_sec_ch;
			acs_selected->ch_width = sap_ctx->acs_cfg->ch_width;
			acs_selected->vht_seg0_center_ch =
				sap_ctx->acs_cfg->vht_seg0_center_ch;
			acs_selected->vht_seg1_center_ch =
				sap_ctx->acs_cfg->vht_seg1_center_ch;
		} else if (eSAP_STATUS_FAILURE == (eSapStatus)context) {
			acs_selected->pri_ch = 0;
		}
		break;

	case eSAP_STOP_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_STOP_BSS_EVENT;
		sap_ap_event.sapevt.sapStopBssCompleteEvent.status =
			(eSapStatus) context;
		break;

	case eSAP_STA_ASSOC_EVENT:
	case eSAP_STA_REASSOC_EVENT:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		if (eSAP_DISCONNECTING == sap_ctx->sapsMachine) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "SAP is disconnecting, not able to handle any incoming (re)assoc req");
			return QDF_STATUS_E_ABORTED;
		}

		reassoc_complete =
		    &sap_ap_event.sapevt.sapStationAssocReassocCompleteEvent;

		if (csr_roaminfo->fReassocReq)
			sap_ap_event.sapHddEventCode = eSAP_STA_REASSOC_EVENT;
		else
			sap_ap_event.sapHddEventCode = eSAP_STA_ASSOC_EVENT;

		qdf_copy_macaddr(&reassoc_complete->staMac,
				 &csr_roaminfo->peerMac);
		reassoc_complete->staId = csr_roaminfo->staId;
		reassoc_complete->statusCode = csr_roaminfo->statusCode;
		reassoc_complete->iesLen = csr_roaminfo->rsnIELen;
		qdf_mem_copy(reassoc_complete->ies, csr_roaminfo->prsnIE,
			     csr_roaminfo->rsnIELen);

#ifdef FEATURE_WLAN_WAPI
		if (csr_roaminfo->wapiIELen) {
			uint8_t len = reassoc_complete->iesLen;

			reassoc_complete->iesLen += csr_roaminfo->wapiIELen;
			qdf_mem_copy(&reassoc_complete->ies[len],
				     csr_roaminfo->pwapiIE,
				     csr_roaminfo->wapiIELen);
		}
#endif
		if (csr_roaminfo->addIELen) {
			uint8_t len = reassoc_complete->iesLen;

			reassoc_complete->iesLen += csr_roaminfo->addIELen;
			qdf_mem_copy(&reassoc_complete->ies[len],
				     csr_roaminfo->paddIE,
				     csr_roaminfo->addIELen);
			if (wlan_get_vendor_ie_ptr_from_oui(
			    SIR_MAC_P2P_OUI, SIR_MAC_P2P_OUI_SIZE,
			    csr_roaminfo->paddIE, csr_roaminfo->addIELen)) {
				reassoc_complete->staType = eSTA_TYPE_P2P_CLI;
			} else {
				reassoc_complete->staType = eSTA_TYPE_INFRA;
			}
		}

		/* also fill up the channel info from the csr_roamInfo */
		chaninfo = &reassoc_complete->chan_info;

		chaninfo->chan_id = csr_roaminfo->chan_info.chan_id;
		chaninfo->mhz = csr_roaminfo->chan_info.mhz;
		chaninfo->info = csr_roaminfo->chan_info.info;
		chaninfo->band_center_freq1 =
			csr_roaminfo->chan_info.band_center_freq1;
		chaninfo->band_center_freq2 =
			csr_roaminfo->chan_info.band_center_freq2;
		chaninfo->reg_info_1 =
			csr_roaminfo->chan_info.reg_info_1;
		chaninfo->reg_info_2 =
			csr_roaminfo->chan_info.reg_info_2;
		chaninfo->nss = csr_roaminfo->chan_info.nss;
		chaninfo->rate_flags = csr_roaminfo->chan_info.rate_flags;

		reassoc_complete->wmmEnabled = csr_roaminfo->wmmEnabledSta;
		reassoc_complete->status = (eSapStatus) context;
		reassoc_complete->timingMeasCap = csr_roaminfo->timingMeasCap;
		reassoc_complete->ampdu = csr_roaminfo->ampdu;
		reassoc_complete->sgi_enable = csr_roaminfo->sgi_enable;
		reassoc_complete->tx_stbc = csr_roaminfo->tx_stbc;
		reassoc_complete->rx_stbc = csr_roaminfo->rx_stbc;
		reassoc_complete->ch_width = csr_roaminfo->ch_width;
		reassoc_complete->mode = csr_roaminfo->mode;
		reassoc_complete->max_supp_idx = csr_roaminfo->max_supp_idx;
		reassoc_complete->max_ext_idx = csr_roaminfo->max_ext_idx;
		reassoc_complete->max_mcs_idx = csr_roaminfo->max_mcs_idx;
		reassoc_complete->rx_mcs_map = csr_roaminfo->rx_mcs_map;
		reassoc_complete->tx_mcs_map = csr_roaminfo->tx_mcs_map;
		reassoc_complete->ecsa_capable = csr_roaminfo->ecsa_capable;
		if (csr_roaminfo->ht_caps.present)
			reassoc_complete->ht_caps = csr_roaminfo->ht_caps;
		if (csr_roaminfo->vht_caps.present)
			reassoc_complete->vht_caps = csr_roaminfo->vht_caps;

		break;

	case eSAP_STA_DISASSOC_EVENT:
		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		sap_ap_event.sapHddEventCode = eSAP_STA_DISASSOC_EVENT;
		disassoc_comp =
			&sap_ap_event.sapevt.sapStationDisassocCompleteEvent;

		qdf_copy_macaddr(&disassoc_comp->staMac,
				 &csr_roaminfo->peerMac);
		disassoc_comp->staId = csr_roaminfo->staId;
		if (csr_roaminfo->disassoc_reason == eCSR_ROAM_RESULT_FORCED)
			disassoc_comp->reason = eSAP_USR_INITATED_DISASSOC;
		else
			disassoc_comp->reason = eSAP_MAC_INITATED_DISASSOC;

		disassoc_comp->statusCode = csr_roaminfo->statusCode;
		disassoc_comp->status = (eSapStatus) context;
		disassoc_comp->rssi = csr_roaminfo->rssi;
		disassoc_comp->rx_rate = csr_roaminfo->rx_rate;
		disassoc_comp->tx_rate = csr_roaminfo->tx_rate;
		disassoc_comp->reason_code = csr_roaminfo->reasonCode;
		break;

	case eSAP_STA_SET_KEY_EVENT:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		sap_ap_event.sapHddEventCode = eSAP_STA_SET_KEY_EVENT;
		key_complete =
			&sap_ap_event.sapevt.sapStationSetKeyCompleteEvent;
		key_complete->status = (eSapStatus) context;
		qdf_copy_macaddr(&key_complete->peerMacAddr,
				 &csr_roaminfo->peerMac);
		break;

	case eSAP_STA_MIC_FAILURE_EVENT:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		sap_ap_event.sapHddEventCode = eSAP_STA_MIC_FAILURE_EVENT;
		mic_failure = &sap_ap_event.sapevt.sapStationMICFailureEvent;

		qdf_mem_copy(&mic_failure->srcMacAddr,
			     csr_roaminfo->u.pMICFailureInfo->srcMacAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(&mic_failure->staMac.bytes,
			     csr_roaminfo->u.pMICFailureInfo->taMacAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(&mic_failure->dstMacAddr.bytes,
			     csr_roaminfo->u.pMICFailureInfo->dstMacAddr,
			     sizeof(tSirMacAddr));
		mic_failure->multicast =
			csr_roaminfo->u.pMICFailureInfo->multicast;
		mic_failure->IV1 = csr_roaminfo->u.pMICFailureInfo->IV1;
		mic_failure->keyId = csr_roaminfo->u.pMICFailureInfo->keyId;
		qdf_mem_copy(mic_failure->TSC,
			     csr_roaminfo->u.pMICFailureInfo->TSC,
			     SIR_CIPHER_SEQ_CTR_SIZE);
		break;

	case eSAP_ASSOC_STA_CALLBACK_EVENT:
		break;

	case eSAP_WPS_PBC_PROBE_REQ_EVENT:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		sap_ap_event.sapHddEventCode = eSAP_WPS_PBC_PROBE_REQ_EVENT;

		qdf_mem_copy(&sap_ap_event.sapevt.sapPBCProbeReqEvent.
			     WPSPBCProbeReq, csr_roaminfo->u.pWPSPBCProbeReq,
			     sizeof(tSirWPSPBCProbeReq));
		break;

	case eSAP_DISCONNECT_ALL_P2P_CLIENT:
		sap_ap_event.sapHddEventCode = eSAP_DISCONNECT_ALL_P2P_CLIENT;
		sap_ap_event.sapevt.sapActionCnf.actionSendSuccess =
			(eSapStatus) context;
		break;

	case eSAP_MAC_TRIG_STOP_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_MAC_TRIG_STOP_BSS_EVENT;
		sap_ap_event.sapevt.sapActionCnf.actionSendSuccess =
			(eSapStatus) context;
		break;

	case eSAP_UNKNOWN_STA_JOIN:
		sap_ap_event.sapHddEventCode = eSAP_UNKNOWN_STA_JOIN;
		qdf_mem_copy((void *) sap_ap_event.sapevt.sapUnknownSTAJoin.
			     macaddr.bytes, (void *) context,
			     QDF_MAC_ADDR_SIZE);
		break;

	case eSAP_MAX_ASSOC_EXCEEDED:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		sap_ap_event.sapHddEventCode = eSAP_MAX_ASSOC_EXCEEDED;
		qdf_copy_macaddr(&sap_ap_event.sapevt.
				 sapMaxAssocExceeded.macaddr,
				 &csr_roaminfo->peerMac);
		break;

	case eSAP_CHANNEL_CHANGE_EVENT:
		/*
		 * Reconfig ACS result info. For DFS AP-AP Mode Sec AP ACS
		 * follows pri AP
		 */
		sap_ctx->acs_cfg->pri_ch = sap_ctx->channel;
		sap_ctx->acs_cfg->ch_width =
				sap_ctx->csr_roamProfile.ch_params.ch_width;
		sap_config_acs_result(hal, sap_ctx,
			sap_ctx->csr_roamProfile.ch_params.sec_ch_offset);

		sap_ap_event.sapHddEventCode = eSAP_CHANNEL_CHANGE_EVENT;

		acs_selected = &sap_ap_event.sapevt.sap_ch_selected;
		acs_selected->pri_ch = sap_ctx->acs_cfg->pri_ch;
		acs_selected->ht_sec_ch = sap_ctx->acs_cfg->ht_sec_ch;
		acs_selected->ch_width =
			sap_ctx->csr_roamProfile.ch_params.ch_width;
		acs_selected->vht_seg0_center_ch =
			sap_ctx->csr_roamProfile.ch_params.center_freq_seg0;
		acs_selected->vht_seg1_center_ch =
			sap_ctx->csr_roamProfile.ch_params.center_freq_seg1;
		break;

	case eSAP_ECSA_CHANGE_CHAN_IND:

		if (!csr_roaminfo) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid CSR Roam Info"));
			return QDF_STATUS_E_INVAL;
		}
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"In %s, SAP event callback event = %s",
				__func__, "eSAP_ECSA_CHANGE_CHAN_IND");
		sap_ap_event.sapHddEventCode = eSAP_ECSA_CHANGE_CHAN_IND;
		sap_ap_event.sapevt.sap_chan_cng_ind.new_chan =
					   csr_roaminfo->target_channel;
		break;
	case eSAP_DFS_NEXT_CHANNEL_REQ:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"In %s, SAP event callback event = %s",
				__func__, "eSAP_DFS_NEXT_CHANNEL_REQ");
		sap_ap_event.sapHddEventCode = eSAP_DFS_NEXT_CHANNEL_REQ;
		break;
	case eSAP_STOP_BSS_DUE_TO_NO_CHNL:
		sap_ap_event.sapHddEventCode = eSAP_STOP_BSS_DUE_TO_NO_CHNL;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  FL("stopping session_id:%d, bssid:%pM, channel:%d"),
			     sap_ctx->sessionId, sap_ctx->self_mac_addr,
			     sap_ctx->channel);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("SAP Unknown callback event = %d"),
			  sap_hddevent);
		break;
	}
	qdf_status = (*sap_ctx->pfnSapEventCallback)
			(&sap_ap_event, sap_ctx->pUsrContext);

	return qdf_status;

}

/**
 * sap_find_cac_wait_session() - Get context of a SAP session in CAC wait state
 * @handle: Global MAC handle
 *
 * Finds and gets the context of a SAP session in CAC wait state.
 *
 * Return: Valid SAP context on success, else NULL
 */
static struct sap_context *sap_find_cac_wait_session(tHalHandle handle)
{
	tpAniSirGlobal mac = PMAC_STRUCT(handle);
	uint8_t i = 0;
	struct sap_context *sapContext;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			"%s", __func__);

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		sapContext = mac->sap.sapCtxList[i].pSapContext;
		if (((QDF_SAP_MODE == mac->sap.sapCtxList[i].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == mac->sap.sapCtxList[i].sapPersona)) &&
		    (sapContext) &&
		    (sapContext->sapsMachine == eSAP_DFS_CAC_WAIT)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				"%s: found SAP in cac wait state", __func__);
			return sapContext;
		}
		if (sapContext) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
					"sapdfs: mode:%d intf:%d state:%d",
					mac->sap.sapCtxList[i].sapPersona, i,
					sapContext->sapsMachine);
		}
	}

	return NULL;
}

/*==========================================================================
   FUNCTION  sap_cac_reset_notify

   DESCRIPTION Function will be called up on stop bss indication to clean up
   DFS global structure.

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : void.

   SIDE EFFECTS
   ============================================================================*/
void sap_cac_reset_notify(tHalHandle hHal)
{
	uint8_t intf = 0;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		struct sap_context *pSapContext =
			pMac->sap.sapCtxList[intf].pSapContext;
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			pSapContext->isCacStartNotified = false;
			pSapContext->isCacEndNotified = false;
		}
	}
}

/*==========================================================================
   FUNCTION  sap_cac_start_notify

   DESCRIPTION Function will be called to notify eSAP_DFS_CAC_START event
   to HDD

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : QDF_STATUS.

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_cac_start_notify(tHalHandle hHal)
{
	uint8_t intf = 0;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		struct sap_context *pSapContext =
			pMac->sap.sapCtxList[intf].pSapContext;
		tCsrRoamProfile *profile;

		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL &&
		    (false == pSapContext->isCacStartNotified)) {
			/* Don't start CAC for non-dfs channel, its violation */
			profile = &pSapContext->csr_roamProfile;
			if (!wlan_reg_is_dfs_ch(pMac->pdev,
						profile->operationChannel))
				continue;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Signaling eSAP_DFS_CAC_START to HDD for sapctx[%pK]",
				  pSapContext);

			qdf_status = sap_signal_hdd_event(pSapContext, NULL,
							  eSAP_DFS_CAC_START,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacStartNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}
			pSapContext->isCacStartNotified = true;
		}
	}
	return qdf_status;
}

/**
 * wlansap_update_pre_cac_end() - Update pre cac end to upper layer
 * @sap_context: SAP context
 * @mac: Global MAC structure
 * @intf: Interface number
 *
 * Notifies pre cac end to upper layer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlansap_update_pre_cac_end(struct sap_context *sap_context,
		tpAniSirGlobal mac, uint8_t intf)
{
	QDF_STATUS qdf_status;

	sap_context->isCacEndNotified = true;
	mac->sap.SapDfsInfo.sap_radar_found_status = false;
	sap_context->sapsMachine = eSAP_STARTED;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"In %s, pre cac end notify on %d: from state %s => %s",
			__func__, intf, "eSAP_DFS_CAC_WAIT",
			"eSAP_STARTED");

	qdf_status = sap_signal_hdd_event(sap_context,
			NULL, eSAP_DFS_PRE_CAC_END,
			(void *)eSAP_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_ERROR,
				"In %s, pre cac notify failed on intf %d",
				__func__, intf);
		return qdf_status;
	}

	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION  sap_cac_end_notify

   DESCRIPTION Function will be called to notify eSAP_DFS_CAC_END event
   to HDD

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : QDF_STATUS.

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_cac_end_notify(tHalHandle hHal,
				     struct csr_roam_info *roamInfo)
{
	uint8_t intf;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	/*
	 * eSAP_DFS_CHANNEL_CAC_END:
	 * CAC Period elapsed and there was no radar
	 * found so, SAP can continue beaconing.
	 * sap_radar_found_status is set to 0
	 */
	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		struct sap_context *pSapContext =
			pMac->sap.sapCtxList[intf].pSapContext;
		tCsrRoamProfile *profile;

		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL &&
		    (false == pSapContext->isCacEndNotified) &&
		    (pSapContext->sapsMachine == eSAP_DFS_CAC_WAIT)) {
			pSapContext = pMac->sap.sapCtxList[intf].pSapContext;
			/* Don't check CAC for non-dfs channel */
			profile = &pSapContext->csr_roamProfile;
			if (!wlan_reg_is_dfs_ch(pMac->pdev,
						profile->operationChannel))
				continue;

			/* If this is an end notification of a pre cac, the
			 * SAP must not start beaconing and must delete the
			 * temporary interface created for pre cac and switch
			 * the original SAP to the pre CAC channel.
			 */
			if (pSapContext->is_pre_cac_on) {
				qdf_status = wlansap_update_pre_cac_end(
						pSapContext, pMac, intf);
				if (QDF_IS_STATUS_ERROR(qdf_status))
					return qdf_status;
				/* pre CAC is not allowed with any concurrency.
				 * So, we can break from here.
				 */
				break;
			}

			qdf_status = sap_signal_hdd_event(pSapContext, NULL,
							  eSAP_DFS_CAC_END,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacEndNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}
			pSapContext->isCacEndNotified = true;
			pMac->sap.SapDfsInfo.sap_radar_found_status = false;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Start beacon request on sapctx[%pK]",
				  pSapContext);

			/* Start beaconing on the new channel */
			wlansap_start_beacon_req(pSapContext);

			/* Transition from eSAP_STARTING to eSAP_STARTED
			 * (both without substates)
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: channel[%d] from state %s => %s",
				  pSapContext->channel, "eSAP_STARTING",
				  "eSAP_STARTED");

			pSapContext->sapsMachine = eSAP_STARTED;

			/*Action code for transition */
			qdf_status = sap_signal_hdd_event(pSapContext, roamInfo,
							  eSAP_START_BSS_EVENT,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacEndNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}

			/* Transition from eSAP_STARTING to eSAP_STARTED
			 * (both without substates)
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "In %s, from state %s => %s",
				  __func__, "eSAP_DFS_CAC_WAIT",
				  "eSAP_STARTED");
		}
	}
	/*
	 * All APs are done with CAC timer, all APs should start beaconing.
	 * Lets assume AP1 and AP2 started beaconing on DFS channel, Now lets
	 * say AP1 goes down and comes back on same DFS channel. In this case
	 * AP1 shouldn't start CAC timer and start beacon immediately beacause
	 * AP2 is already beaconing on this channel. This case will be handled
	 * by checking against eSAP_DFS_SKIP_CAC while starting the timer.
	 */
	pMac->sap.SapDfsInfo.cac_state = eSAP_DFS_SKIP_CAC;
	return qdf_status;
}

/**
 * sap_fsm_state_disconnected() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_DISCONNECTED"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_disconnected(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_HDD_START_INFRA_BSS) {
		/*
		 * Transition from eSAP_DISCONNECTED to eSAP_CH_SELECT
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("new from state %s => %s: session:%d"),
			  "eSAP_DISCONNECTED", "eSAP_CH_SELECT",
			  sap_ctx->sessionId);

		/* init dfs channel nol */
		sap_init_dfs_channel_nol_list(sap_ctx);

		/* Set SAP device role */
		sap_ctx->sapsMachine = eSAP_CH_SELECT;

		/*
		 * Perform sme_ScanRequest. This scan request is post start bss
		 * request so, set the third to false.
		 */
		qdf_status = sap_goto_channel_sel(sap_ctx, sap_event, false,
						true);
	} else if (msg == eSAP_DFS_CHANNEL_CAC_START) {
		/*
		 * No need of state check here, caller is expected to perform
		 * the checks before sending the event
		 */
		sap_ctx->sapsMachine = eSAP_DFS_CAC_WAIT;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			FL("from state eSAP_DISCONNECTED => SAP_DFS_CAC_WAIT"));
		if (mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running != true) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			    FL("sapdfs: starting dfs cac timer on sapctx[%pK]"),
			    sap_ctx);
			sap_start_dfs_cac_timer(sap_ctx);
		}

		qdf_status = sap_cac_start_notify(hal);
	} else if (msg == eSAP_CHANNEL_SELECTION_RETRY) {
		/* Set SAP device role */
		sap_ctx->sapsMachine = eSAP_CH_SELECT;

		/*
		 * Perform sme_ScanRequest. This scan request is post start bss
		 * request so, set the third to false.
		 */
		qdf_status = sap_goto_channel_sel(sap_ctx, sap_event, false,
					false);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, event msg %d"),
			  "eSAP_DISCONNECTED", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_ch_select() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_CH_SELECT"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_ch_select(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	bool b_leak_chan = false;
	uint8_t temp_chan;

	if (msg == eSAP_MAC_SCAN_COMPLETE) {
		temp_chan = sap_ctx->channel;
		utils_dfs_mark_leaking_ch(mac_ctx->pdev,
					sap_ctx->ch_params.ch_width,
					1, &temp_chan);

		/*
		 * if selelcted channel has leakage to channels
		 * in NOL, the temp_chan will be reset
		 */
		b_leak_chan = (temp_chan != sap_ctx->channel);
		/*
		 * check if channel is in DFS_NOL or if the channel
		 * has leakage to the channels in NOL
		 */
		if (sap_dfs_is_channel_in_nol_list(sap_ctx, sap_ctx->channel,
			PHY_CHANNEL_BONDING_STATE_MAX) || b_leak_chan) {
			uint8_t ch;

			/* find a new available channel */
			ch = sap_random_channel_sel(sap_ctx);
			if (ch == 0) {
				/* No available channel found */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					FL("No available channel found!!!"));
				sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_DFS_NO_AVAILABLE_CHANNEL,
					(void *)eSAP_STATUS_SUCCESS);
				return QDF_STATUS_E_FAULT;
			}

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("channel %d is in NOL, StartBss on new channel %d"),
				  sap_ctx->channel, ch);

			sap_ctx->channel = ch;
			wlan_reg_set_channel_params(mac_ctx->pdev,
					sap_ctx->channel,
					sap_ctx->secondary_ch,
					&sap_ctx->ch_params);
		}
		if (sap_ctx->channel > 14 &&
		    (sap_ctx->csr_roamProfile.phyMode == eCSR_DOT11_MODE_11g ||
		     sap_ctx->csr_roamProfile.phyMode ==
						eCSR_DOT11_MODE_11g_ONLY))
			sap_ctx->csr_roamProfile.phyMode = eCSR_DOT11_MODE_11a;

		/*
		 * when AP2 is started while AP1 is performing ACS, we may not
		 * have the AP1 channel yet.So here after the completion of AP2
		 * ACS check if AP1 ACS resulting channel is DFS and if yes
		 * override AP2 ACS scan result with AP1 DFS channel
		 */
		if (policy_mgr_concurrent_beaconing_sessions_running(
			mac_ctx->psoc)) {
			uint16_t con_ch;

			con_ch = sme_get_concurrent_operation_channel(hal);
			if (con_ch && wlan_reg_is_dfs_ch(mac_ctx->pdev, con_ch))
				sap_ctx->channel = con_ch;
		}

		/*
		 * Transition from eSAP_CH_SELECT to eSAP_STARTING
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_CH_SELECT", "eSAP_STARTING");
		/* Channel selected. Now can sap_goto_starting */
		sap_ctx->sapsMachine = eSAP_STARTING;
		/* Specify the channel */
		sap_ctx->csr_roamProfile.ChannelInfo.numOfChannels =
						1;
		sap_ctx->csr_roamProfile.ChannelInfo.ChannelList =
			&sap_ctx->csr_roamProfile.operationChannel;
		sap_ctx->csr_roamProfile.operationChannel =
			(uint8_t) sap_ctx->channel;
		sap_ctx->csr_roamProfile.ch_params.ch_width =
					sap_ctx->ch_params.ch_width;
		sap_ctx->csr_roamProfile.ch_params.center_freq_seg0 =
				sap_ctx->ch_params.center_freq_seg0;
		sap_ctx->csr_roamProfile.ch_params.center_freq_seg1 =
				sap_ctx->ch_params.center_freq_seg1;
		sap_ctx->csr_roamProfile.ch_params.sec_ch_offset =
				sap_ctx->ch_params.sec_ch_offset;
		sap_get_cac_dur_dfs_region(sap_ctx,
				&sap_ctx->csr_roamProfile.cac_duration_ms,
				&sap_ctx->csr_roamProfile.dfs_regdomain);
		sap_ctx->csr_roamProfile.beacon_tx_rate =
				sap_ctx->beacon_tx_rate;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		    FL("notify hostapd about channel selection: %d"),
		    sap_ctx->channel);
		sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_CHANNEL_CHANGE_EVENT,
					(void *) eSAP_STATUS_SUCCESS);
		sap_dfs_set_current_channel(sap_ctx);
		qdf_status = sap_goto_starting(sap_ctx, sap_event,
					  eCSR_BSS_TYPE_INFRA_AP);
	} else if (msg == eSAP_CHANNEL_SELECTION_FAILED) {
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		FL("Cannot start BSS, ACS Fail"));
		sap_signal_hdd_event(sap_ctx, NULL, eSAP_START_BSS_EVENT,
					(void *)eSAP_STATUS_FAILURE);
	} else if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		sap_signal_hdd_event(sap_ctx, NULL, eSAP_START_BSS_EVENT,
					(void *)eSAP_STATUS_FAILURE);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			"%s: BSS stopped when Ch select in Progress", __func__);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_CH_SELECT", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_dfs_cac_wait() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_DFS_CAC_WAIT"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_dfs_cac_wait(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	struct csr_roam_info *roam_info =
		(struct csr_roam_info *) (sap_event->params);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_DFS_CHANNEL_CAC_START) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_CH_SELECT", "eSAP_DFS_CAC_WAIT");
		if (mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running != true)
			sap_start_dfs_cac_timer(sap_ctx);
		qdf_status = sap_cac_start_notify(hal);
	} else if (msg == eSAP_DFS_CHANNEL_CAC_RADAR_FOUND) {
		uint8_t intf;
		/*
		 * Radar found while performing channel availability
		 * check, need to switch the channel again
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  "ENTERTRED CAC WAIT STATE-->eSAP_DISCONNECTING\n");
		if (mac_ctx->sap.SapDfsInfo.target_channel) {
			wlan_reg_set_channel_params(mac_ctx->pdev,
				mac_ctx->sap.SapDfsInfo.target_channel, 0,
				&sap_ctx->ch_params);
		}

		for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
			struct sap_context *t_sap_ctx;
			tCsrRoamProfile *profile;

			t_sap_ctx = mac_ctx->sap.sapCtxList[intf].pSapContext;
			if (((QDF_SAP_MODE ==
				 mac_ctx->sap.sapCtxList[intf].sapPersona) ||
			     (QDF_P2P_GO_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
			    t_sap_ctx != NULL &&
			    t_sap_ctx->sapsMachine != eSAP_DISCONNECTED) {
				profile = &t_sap_ctx->csr_roamProfile;
				if (!wlan_reg_is_passive_or_disable_ch(
						mac_ctx->pdev,
						profile->operationChannel))
					continue;
				/* SAP to be moved to DISCONNECTING state */
				t_sap_ctx->sapsMachine = eSAP_DISCONNECTING;
				/*
				 * eSAP_DFS_CHANNEL_CAC_RADAR_FOUND:
				 * A Radar is found on current DFS Channel
				 * while in CAC WAIT period So, do a channel
				 * switch to randomly selected  target channel.
				 * Send the Channel change message to SME/PE.
				 * sap_radar_found_status is set to 1
				 */
				wlansap_channel_change_request(
					t_sap_ctx,
					mac_ctx->sap.SapDfsInfo.target_channel);
			}
		}
	} else if (msg == eSAP_DFS_CHANNEL_CAC_END) {
		qdf_status = sap_cac_end_notify(hal, roam_info);
	} else if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		/* Transition from eSAP_DFS_CAC_WAIT to eSAP_DISCONNECTING */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_DFS_CAC_WAIT", "eSAP_DISCONNECTING");

		/*
		 * Stop the CAC timer only in following conditions
		 * single AP: if there is a single AP then stop the timer
		 * mulitple APs: incase of multiple APs, make sure that
		 *               all APs are down.
		 */
		if (NULL == sap_find_valid_concurrent_session(hal)) {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO_MED,
				  FL("sapdfs: no sessions are valid, stopping timer"));
			sap_stop_dfs_cac_timer(sap_ctx);
		}

		sap_ctx->sapsMachine = eSAP_DISCONNECTING;
		qdf_status = sap_goto_disconnecting(sap_ctx);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_DFS_CAC_WAIT", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_starting() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_STARTING"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_starting(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	struct csr_roam_info *roam_info =
		(struct csr_roam_info *) (sap_event->params);
	tSapDfsInfo *sap_dfs_info;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t is_dfs = false;

	if (msg == eSAP_MAC_START_BSS_SUCCESS) {
		/*
		 * Transition from eSAP_STARTING to eSAP_STARTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state channel = %d %s => %s ch_width %d"),
			  sap_ctx->channel, "eSAP_STARTING", "eSAP_STARTED",
			  sap_ctx->ch_params.ch_width);
		sap_ctx->sapsMachine = eSAP_STARTED;

		/* Action code for transition */
		qdf_status = sap_signal_hdd_event(sap_ctx, roam_info,
				eSAP_START_BSS_EVENT,
				(void *) eSAP_STATUS_SUCCESS);

		/*
		 * The upper layers have been informed that AP is up and
		 * running, however, the AP is still not beaconing, until
		 * CAC is done if the operating channel is DFS
		 */
		if (sap_ctx->ch_params.ch_width == CH_WIDTH_160MHZ) {
			is_dfs = true;
		} else if (sap_ctx->ch_params.ch_width == CH_WIDTH_80P80MHZ) {
			if (wlan_reg_get_channel_state(mac_ctx->pdev,
						sap_ctx->channel) ==
			    CHANNEL_STATE_DFS ||
			    wlan_reg_get_channel_state(mac_ctx->pdev,
				    sap_ctx->ch_params.center_freq_seg1 -
				SIR_80MHZ_START_CENTER_CH_DIFF) ==
					CHANNEL_STATE_DFS)
				is_dfs = true;
		} else {
			if (wlan_reg_get_channel_state(mac_ctx->pdev,
						sap_ctx->channel) ==
							CHANNEL_STATE_DFS)
				is_dfs = true;
		}

		if (is_dfs) {
			sap_dfs_info = &mac_ctx->sap.SapDfsInfo;
			if ((false == sap_dfs_info->ignore_cac) &&
			    (eSAP_DFS_DO_NOT_SKIP_CAC ==
					sap_dfs_info->cac_state) &&
			    !sap_ctx->pre_cac_complete) {
				/* Move the device in CAC_WAIT_STATE */
				sap_ctx->sapsMachine = eSAP_DFS_CAC_WAIT;

				/*
				 * Need to stop the OS transmit queues,
				 * so that no traffic can flow down the stack
				 */

				/* Start CAC wait timer */
				if (sap_dfs_info->is_dfs_cac_timer_running !=
									true)
					sap_start_dfs_cac_timer(sap_ctx);
				qdf_status = sap_cac_start_notify(hal);

			} else {
				wlansap_start_beacon_req(sap_ctx);
			}
		}
	} else if (msg == eSAP_MAC_START_FAILS ||
			msg == eSAP_HDD_STOP_INFRA_BSS) {
		/*
		 * Transition from eSAP_STARTING to eSAP_DISCONNECTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTING", "eSAP_DISCONNECTED");

		/* Advance outer statevar */
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
				eSAP_START_BSS_EVENT,
				(void *) eSAP_STATUS_FAILURE);
		qdf_status = sap_goto_disconnected(sap_ctx);
		/* Close the SME session */
	} else if (msg == eSAP_OPERATING_CHANNEL_CHANGED) {
		/* The operating channel has changed, update hostapd */
		sap_ctx->channel =
			(uint8_t) mac_ctx->sap.SapDfsInfo.target_channel;

		sap_ctx->sapsMachine = eSAP_STARTED;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTING", "eSAP_STARTED");

		/* Indicate change in the state to upper layers */
		qdf_status = sap_signal_hdd_event(sap_ctx, roam_info,
				  eSAP_START_BSS_EVENT,
				  (void *)eSAP_STATUS_SUCCESS);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_STARTING", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_started() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 *
 * This function is called for state transition from "eSAP_STARTED"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_started(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		/*
		 * Transition from eSAP_STARTED to eSAP_DISCONNECTING
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTED", "eSAP_DISCONNECTING");
		sap_ctx->sapsMachine = eSAP_DISCONNECTING;
		qdf_status = sap_goto_disconnecting(sap_ctx);
	} else if (eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START == msg) {
		uint8_t intf;
		/*
		 * Radar is seen on the current operating channel
		 * send CSA IE for all associated stations
		 * Request for CSA IE transmission
		 */
		for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
			struct sap_context *temp_sap_ctx;
			tCsrRoamProfile *profile;

			if (((QDF_SAP_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona) ||
			    (QDF_P2P_GO_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
			    mac_ctx->sap.sapCtxList[intf].pSapContext != NULL) {
				temp_sap_ctx =
				    mac_ctx->sap.sapCtxList[intf].pSapContext;
				/*
				 * Radar won't come on non-dfs channel, so
				 * no need to move them
				 */
				profile = &temp_sap_ctx->csr_roamProfile;
				if (!wlan_reg_is_passive_or_disable_ch(
						mac_ctx->pdev,
						profile->operationChannel))
					continue;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_MED,
					  FL("sapdfs: Sending CSAIE for sapctx[%pK]"),
					  temp_sap_ctx);

				qdf_status =
					wlansap_dfs_send_csa_ie_request(temp_sap_ctx);
			}
		}
	} else if (eSAP_CHANNEL_SWITCH_ANNOUNCEMENT_START == msg) {
		enum QDF_OPMODE persona;

		if (!sap_ctx) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid sap_ctx"));
			return qdf_status;
		}

		persona = mac_ctx->sap.sapCtxList[sap_ctx->sessionId].
								sapPersona;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				FL("app trigger chan switch: mode:%d vdev:%d"),
				persona, sap_ctx->sessionId);

		if ((QDF_SAP_MODE == persona) || (QDF_P2P_GO_MODE == persona))
			qdf_status = wlansap_dfs_send_csa_ie_request(sap_ctx);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_STARTED", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_disconnecting() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 *
 * This function is called for state transition from "eSAP_DISCONNECTING"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_disconnecting(struct sap_context *sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_MAC_READY_FOR_CONNECTIONS) {
		/*
		 * Transition from eSAP_DISCONNECTING to eSAP_DISCONNECTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_DISCONNECTING", "eSAP_DISCONNECTED");
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;

		/* Close the SME session */
		qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_STOP_BSS_EVENT,
					(void *)eSAP_STATUS_SUCCESS);
	} else if (msg == eWNI_SME_CHANNEL_CHANGE_REQ) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: Send channel change request on sapctx[%pK]"),
			  sap_ctx);

		sap_get_cac_dur_dfs_region(sap_ctx,
				&sap_ctx->csr_roamProfile.cac_duration_ms,
				&sap_ctx->csr_roamProfile.dfs_regdomain);
		/*
		 * Most likely, radar has been detected and SAP wants to
		 * change the channel
		 */
		qdf_status = wlansap_channel_change_request(sap_ctx,
				mac_ctx->sap.SapDfsInfo.target_channel);

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("Sending DFS eWNI_SME_CHANNEL_CHANGE_REQ"));
	} else if (msg == eWNI_SME_CHANNEL_CHANGE_RSP) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("in state %s, event msg %d result %d"),
			  "eSAP_DISCONNECTING ", msg, sap_event->u2);
		if (sap_event->u2 == eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE)
			qdf_status = sap_goto_disconnecting(sap_ctx);
	} else if ((msg == eSAP_HDD_STOP_INFRA_BSS) &&
			(sap_ctx->is_chan_change_inprogress)) {
		/* stop bss is recieved while processing channel change */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("in state %s, event msg %d result %d"),
			  "eSAP_DISCONNECTING ", msg, sap_event->u2);
		qdf_status = sap_goto_disconnecting(sap_ctx);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_DISCONNECTING", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm() - SAP statem machine entry function
 * @sap_ctx: SAP context
 * @sap_event: SAP event
 *
 * SAP statem machine entry function
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sap_fsm(struct sap_context *sap_ctx, ptWLAN_SAPEvent sap_event)
{
	/*
	 * Retrieve the phy link state machine structure
	 * from the sap_ctx value
	 * state var that keeps track of state machine
	 */
	eSapFsmStates_t state_var = sap_ctx->sapsMachine;
	uint32_t msg = sap_event->event; /* State machine input event message */
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tHalHandle hal = CDS_GET_HAL_CB();
	tpAniSirGlobal mac_ctx;

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid hal"));
		return QDF_STATUS_E_FAILURE;
	}

	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  FL("sap_ctx=%pK, state_var=%d, msg=0x%x"),
		  sap_ctx, state_var, msg);

	switch (state_var) {
	case eSAP_DISCONNECTED:
		qdf_status = sap_fsm_state_disconnected(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_CH_SELECT:
		qdf_status = sap_fsm_state_ch_select(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_DFS_CAC_WAIT:
		qdf_status = sap_fsm_state_dfs_cac_wait(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_STARTING:
		qdf_status = sap_fsm_state_starting(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_STARTED:
		qdf_status = sap_fsm_state_started(sap_ctx, sap_event,
				mac_ctx);
		break;

	case eSAP_DISCONNECTING:
		qdf_status = sap_fsm_state_disconnecting(sap_ctx, sap_event,
				mac_ctx, hal);
		break;
	}
	return qdf_status;
}

eSapStatus
sapconvert_to_csr_profile(tsap_Config_t *pconfig_params, eCsrRoamBssType bssType,
			  tCsrRoamProfile *profile)
{
	/* Create Roam profile for SoftAP to connect */
	profile->BSSType = eCSR_BSS_TYPE_INFRA_AP;
	profile->SSIDs.numOfSSIDs = 1;
	profile->csrPersona = pconfig_params->persona;
	profile->disableDFSChSwitch = pconfig_params->disableDFSChSwitch;

	qdf_mem_zero(profile->SSIDs.SSIDList[0].SSID.ssId,
		     sizeof(profile->SSIDs.SSIDList[0].SSID.ssId));

	/* Flag to not broadcast the SSID information */
	profile->SSIDs.SSIDList[0].ssidHidden =
		pconfig_params->SSIDinfo.ssidHidden;

	profile->SSIDs.SSIDList[0].SSID.length =
		pconfig_params->SSIDinfo.ssid.length;
	qdf_mem_copy(&profile->SSIDs.SSIDList[0].SSID.ssId,
		     pconfig_params->SSIDinfo.ssid.ssId,
		     sizeof(pconfig_params->SSIDinfo.ssid.ssId));

	profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	if (pconfig_params->authType == eSAP_OPEN_SYSTEM) {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	} else if (pconfig_params->authType == eSAP_SHARED_KEY) {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_SHARED_KEY;
	} else {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_AUTOSWITCH;
	}

	profile->AuthType.numEntries = 1;
	profile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	/* Always set the Encryption Type */
	profile->EncryptionType.numEntries = 1;
	profile->EncryptionType.encryptionType[0] =
		pconfig_params->RSNEncryptType;

	profile->mcEncryptionType.numEntries = 1;
	profile->mcEncryptionType.encryptionType[0] =
		pconfig_params->mcRSNEncryptType;

	if (pconfig_params->privacy & eSAP_SHARED_KEY) {
		profile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;
	}

	profile->privacy = pconfig_params->privacy;
	profile->fwdWPSPBCProbeReq = pconfig_params->fwdWPSPBCProbeReq;

	if (pconfig_params->authType == eSAP_SHARED_KEY) {
		profile->csr80211AuthType = eSIR_SHARED_KEY;
	} else if (pconfig_params->authType == eSAP_OPEN_SYSTEM) {
		profile->csr80211AuthType = eSIR_OPEN_SYSTEM;
	} else {
		profile->csr80211AuthType = eSIR_AUTO_SWITCH;
	}

	/* Initialize we are not going to use it */
	profile->pWPAReqIE = NULL;
	profile->nWPAReqIELength = 0;

	if (profile->pRSNReqIE) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  FL("pRSNReqIE already allocated."));
		qdf_mem_free(profile->pRSNReqIE);
		profile->pRSNReqIE = NULL;
	}

	/* set the RSN/WPA IE */
	profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
	if (pconfig_params->RSNWPAReqIELength) {
		profile->pRSNReqIE =
			qdf_mem_malloc(pconfig_params->RSNWPAReqIELength);
		if (NULL == profile->pRSNReqIE) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  " %s Fail to alloc memory", __func__);
			return eSAP_STATUS_FAILURE;
		}
		qdf_mem_copy(profile->pRSNReqIE, pconfig_params->RSNWPAReqIE,
			     pconfig_params->RSNWPAReqIELength);
		profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
	}

	/* set the phyMode to accept anything */
	/* Best means everything because it covers all the things we support */
	/* eCSR_DOT11_MODE_BEST */
	profile->phyMode = pconfig_params->SapHw_mode;

	/* Configure beaconInterval */
	profile->beaconInterval = (uint16_t) pconfig_params->beacon_int;

	/* set DTIM period */
	profile->dtimPeriod = pconfig_params->dtim_period;

	/* set Uapsd enable bit */
	profile->ApUapsdEnable = pconfig_params->UapsdEnable;

	/* Enable protection parameters */
	profile->protEnabled = pconfig_params->protEnabled;
	profile->obssProtEnabled = pconfig_params->obssProtEnabled;
	profile->cfg_protection = pconfig_params->ht_capab;

	/* country code */
	if (pconfig_params->countryCode[0])
		qdf_mem_copy(profile->countryCode, pconfig_params->countryCode,
			     WNI_CFG_COUNTRY_CODE_LEN);
	profile->ieee80211d = pconfig_params->ieee80211d;
	/* wps config info */
	profile->wps_state = pconfig_params->wps_state;

#ifdef WLAN_FEATURE_11W
	/* MFP capable/required */
	profile->MFPCapable = pconfig_params->mfpCapable ? 1 : 0;
	profile->MFPRequired = pconfig_params->mfpRequired ? 1 : 0;
#endif

	if (pconfig_params->probeRespIEsBufferLen > 0 &&
	    pconfig_params->pProbeRespIEsBuffer != NULL) {
		profile->addIeParams.probeRespDataLen =
			pconfig_params->probeRespIEsBufferLen;
		profile->addIeParams.probeRespData_buff =
			pconfig_params->pProbeRespIEsBuffer;
	} else {
		profile->addIeParams.probeRespDataLen = 0;
		profile->addIeParams.probeRespData_buff = NULL;
	}
	/*assoc resp IE */
	if (pconfig_params->assocRespIEsLen > 0 &&
	    pconfig_params->pAssocRespIEsBuffer != NULL) {
		profile->addIeParams.assocRespDataLen =
			pconfig_params->assocRespIEsLen;
		profile->addIeParams.assocRespData_buff =
			pconfig_params->pAssocRespIEsBuffer;
	} else {
		profile->addIeParams.assocRespDataLen = 0;
		profile->addIeParams.assocRespData_buff = NULL;
	}

	if (pconfig_params->probeRespBcnIEsLen > 0 &&
	    pconfig_params->pProbeRespBcnIEsBuffer != NULL) {
		profile->addIeParams.probeRespBCNDataLen =
			pconfig_params->probeRespBcnIEsLen;
		profile->addIeParams.probeRespBCNData_buff =
			pconfig_params->pProbeRespBcnIEsBuffer;
	} else {
		profile->addIeParams.probeRespBCNDataLen = 0;
		profile->addIeParams.probeRespBCNData_buff = NULL;
	}
	profile->sap_dot11mc = pconfig_params->sap_dot11mc;

	if (pconfig_params->supported_rates.numRates) {
		qdf_mem_copy(profile->supported_rates.rate,
				pconfig_params->supported_rates.rate,
				pconfig_params->supported_rates.numRates);
		profile->supported_rates.numRates =
			pconfig_params->supported_rates.numRates;
	}

	if (pconfig_params->extended_rates.numRates) {
		qdf_mem_copy(profile->extended_rates.rate,
				pconfig_params->extended_rates.rate,
				pconfig_params->extended_rates.numRates);
		profile->extended_rates.numRates =
			pconfig_params->extended_rates.numRates;
	}

	profile->chan_switch_hostapd_rate_enabled =
		pconfig_params->chan_switch_hostapd_rate_enabled;

	return eSAP_STATUS_SUCCESS;     /* Success. */
}

void sap_free_roam_profile(tCsrRoamProfile *profile)
{
	if (profile->pRSNReqIE) {
		qdf_mem_free(profile->pRSNReqIE);
		profile->pRSNReqIE = NULL;
	}
}

void sap_sort_mac_list(struct qdf_mac_addr *macList, uint8_t size)
{
	uint8_t outer, inner;
	struct qdf_mac_addr temp;
	int32_t nRes = -1;

	if ((NULL == macList) || (size > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size = %d is more"), size);
		return;
	}

	for (outer = 0; outer < size; outer++) {
		for (inner = 0; inner < size - 1; inner++) {
			nRes =
				qdf_mem_cmp((macList + inner)->bytes,
						 (macList + inner + 1)->bytes,
						 QDF_MAC_ADDR_SIZE);
			if (nRes > 0) {
				qdf_mem_copy(temp.bytes,
					     (macList + inner + 1)->bytes,
					     QDF_MAC_ADDR_SIZE);
				qdf_mem_copy((macList + inner + 1)->bytes,
					     (macList + inner)->bytes,
					     QDF_MAC_ADDR_SIZE);
				qdf_mem_copy((macList + inner)->bytes,
					     temp.bytes, QDF_MAC_ADDR_SIZE);
			}
		}
	}
}

bool
sap_search_mac_list(struct qdf_mac_addr *macList,
		    uint8_t num_mac, uint8_t *peerMac,
		    uint8_t *index)
{
	int32_t nRes = -1;
	int8_t nStart = 0, nEnd, nMiddle;

	nEnd = num_mac - 1;

	if ((NULL == macList) || (num_mac > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		    FL("either buffer is NULL or size = %d is more."), num_mac);
		return false;
	}

	while (nStart <= nEnd) {
		nMiddle = (nStart + nEnd) / 2;
		nRes =
			qdf_mem_cmp(&macList[nMiddle], peerMac,
					 QDF_MAC_ADDR_SIZE);

		if (0 == nRes) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "search SUCC");
			/* "index equals NULL" means the caller does not need the */
			/* index value of the peerMac being searched */
			if (index != NULL) {
				*index = (uint8_t) nMiddle;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_HIGH, "index %d",
					  *index);
			}
			return true;
		}
		if (nRes < 0)
			nStart = nMiddle + 1;
		else
			nEnd = nMiddle - 1;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "search not succ");
	return false;
}

void sap_add_mac_to_acl(struct qdf_mac_addr *macList,
			uint8_t *size, uint8_t *peerMac)
{
	int32_t nRes = -1;
	int i;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "add acl entered");

	if (NULL == macList || *size > MAX_ACL_MAC_ADDRESS) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size = %d is incorrect."),
			*size);
		return;
	}

	for (i = ((*size) - 1); i >= 0; i--) {
		nRes =
			qdf_mem_cmp(&macList[i], peerMac, QDF_MAC_ADDR_SIZE);
		if (nRes > 0) {
			/* Move alphabetically greater mac addresses one index down to allow for insertion
			   of new mac in sorted order */
			qdf_mem_copy((macList + i + 1)->bytes,
				     (macList + i)->bytes, QDF_MAC_ADDR_SIZE);
		} else {
			break;
		}
	}
	/* This should also take care of if the element is the first to be added in the list */
	qdf_mem_copy((macList + i + 1)->bytes, peerMac, QDF_MAC_ADDR_SIZE);
	/* increment the list size */
	(*size)++;
}

void sap_remove_mac_from_acl(struct qdf_mac_addr *macList,
			     uint8_t *size, uint8_t index)
{
	int i;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "remove acl entered");
	/*
	 * Return if the list passed is empty. Ideally this should never happen
	 * since this funcn is always called after sap_search_mac_list to get
	 * the index of the mac addr to be removed and this will only get
	 * called if the search is successful. Still no harm in having the check
	 */
	if ((macList == NULL) || (*size == 0) ||
					(*size > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size %d is incorrect."),
			*size);
		return;
	}
	for (i = index; i < ((*size) - 1); i++) {
		/* Move mac addresses starting from "index" passed one index up to delete the void
		   created by deletion of a mac address in ACL */
		qdf_mem_copy((macList + i)->bytes, (macList + i + 1)->bytes,
			     QDF_MAC_ADDR_SIZE);
	}
	/* The last space should be made empty since all mac addesses moved one step up */
	qdf_mem_zero((macList + (*size) - 1)->bytes, QDF_MAC_ADDR_SIZE);
	/* reduce the list size by 1 */
	(*size)--;
}

void sap_print_acl(struct qdf_mac_addr *macList, uint8_t size)
{
	int i;
	uint8_t *macArray;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "print acl entered");

	if ((NULL == macList) || (size == 0) || (size >= MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, either buffer is NULL or size %d is incorrect.",
			  __func__, size);
		return;
	}

	for (i = 0; i < size; i++) {
		macArray = (macList + i)->bytes;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "** ACL entry %i - " MAC_ADDRESS_STR, i,
			  MAC_ADDR_ARRAY(macArray));
	}
	return;
}

QDF_STATUS sap_is_peer_mac_allowed(struct sap_context *sapContext,
				   uint8_t *peerMac)
{
	if (eSAP_ALLOW_ALL == sapContext->eSapMacAddrAclMode)
		return QDF_STATUS_SUCCESS;

	if (sap_search_mac_list
		    (sapContext->acceptMacList, sapContext->nAcceptMac, peerMac, NULL))
		return QDF_STATUS_SUCCESS;

	if (sap_search_mac_list
		    (sapContext->denyMacList, sapContext->nDenyMac, peerMac, NULL)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR " in deny list",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}
	/* A new station CAN associate, unless in deny list. Less stringent mode */
	if (eSAP_ACCEPT_UNLESS_DENIED == sapContext->eSapMacAddrAclMode)
		return QDF_STATUS_SUCCESS;

	/* A new station CANNOT associate, unless in accept list. More stringent mode */
	if (eSAP_DENY_UNLESS_ACCEPTED == sapContext->eSapMacAddrAclMode) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR
			  " denied, Mac filter mode is eSAP_DENY_UNLESS_ACCEPTED",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}

	/* The new STA is neither in accept list nor in deny list. In this case, deny the association
	 * but send a wifi event notification indicating the mac address being denied
	 */
	if (eSAP_SUPPORT_ACCEPT_AND_DENY == sapContext->eSapMacAddrAclMode) {
		sap_signal_hdd_event(sapContext, NULL, eSAP_UNKNOWN_STA_JOIN,
				     (void *) peerMac);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR
			  " denied, Mac filter mode is eSAP_SUPPORT_ACCEPT_AND_DENY",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef SOFTAP_CHANNEL_RANGE
/**
 * sap_get_channel_list() - get the list of channels
 * @sap_ctx: sap context
 * @ch_list: pointer to channel list array
 * @num_ch: pointer to number of channels.
 *
 * This function populates the list of channels for scanning.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_get_channel_list(struct sap_context *sap_ctx,
				       uint8_t **ch_list,
				       uint8_t *num_ch)
{
	uint8_t loop_count;
	uint8_t *list;
	uint8_t ch_count;
	uint8_t start_ch_num, band_start_ch;
	uint8_t end_ch_num, band_end_ch;
	uint32_t en_lte_coex;
	tHalHandle hal = CDS_GET_HAL_CB();
#ifdef FEATURE_WLAN_CH_AVOID
	uint8_t i;
#endif
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	tSapChSelSpectInfo spect_info_obj = { NULL, 0 };
	uint16_t ch_width;

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid HAL pointer from p_cds_gctx"));
		*num_ch = 0;
		*ch_list = NULL;
		return QDF_STATUS_E_FAULT;
	}

	start_ch_num = sap_ctx->acs_cfg->start_ch;
	end_ch_num = sap_ctx->acs_cfg->end_ch;
	ch_width = sap_ctx->acs_cfg->ch_width;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		  FL("startChannel %d, EndChannel %d, ch_width %d, HW:%d"),
		     start_ch_num, end_ch_num, ch_width,
		     sap_ctx->acs_cfg->hw_mode);

	wlansap_extend_to_acs_range(hal, &start_ch_num, &end_ch_num,
					    &band_start_ch, &band_end_ch);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("expanded startChannel %d,EndChannel %d"),
			  start_ch_num, end_ch_num);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("band_start_ch %d, band_end_ch %d"),
			  band_start_ch, band_end_ch);

	sme_cfg_get_int(hal, WNI_CFG_ENABLE_LTE_COEX, &en_lte_coex);

	/* Check if LTE coex is enabled and 2.4GHz is selected */
	if (en_lte_coex && (band_start_ch == CHAN_ENUM_1) &&
	    (band_end_ch == CHAN_ENUM_14)) {
		/* Set 2.4GHz upper limit to channel 9 for LTE COEX */
		band_end_ch = CHAN_ENUM_9;
	}

	/* Allocate the max number of channel supported */
	list = (uint8_t *) qdf_mem_malloc(NUM_5GHZ_CHANNELS +
						NUM_24GHZ_CHANNELS);
	if (NULL == list) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Unable to allocate channel list"));
		*num_ch = 0;
		*ch_list = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	/* Search for the Active channels in the given range */
	ch_count = 0;
	for (loop_count = band_start_ch; loop_count <= band_end_ch;
	     loop_count++) {
		/* go to next channel if rf_channel is out of range */
		if ((start_ch_num > WLAN_REG_CH_NUM(loop_count)) ||
		    (end_ch_num < WLAN_REG_CH_NUM(loop_count)))
			continue;
		/*
		 * go to next channel if none of these condition pass
		 * - DFS scan enabled and chan not in CHANNEL_STATE_DISABLE
		 * - DFS scan disable but chan in CHANNEL_STATE_ENABLE
		 */
		if (!(((true == mac_ctx->scan.fEnableDFSChnlScan) &&
		      wlan_reg_get_channel_state(mac_ctx->pdev, loop_count)) ||
		    ((false == mac_ctx->scan.fEnableDFSChnlScan) &&
		     (CHANNEL_STATE_ENABLE ==
		      wlan_reg_get_channel_state(mac_ctx->pdev, loop_count)))))
			continue;

		/*
		 * Skip the channels which are not in ACS config from user
		 * space
		 */
		if (SAP_CHANNEL_NOT_SELECTED ==
			sap_channel_in_acs_channel_list(
				WLAN_REG_CH_NUM(loop_count),
				sap_ctx, &spect_info_obj))
			continue;

		/*
		 * If we have any 5Ghz channel in the channel list
		 * and bw is 40/80/160 Mhz then we don't want SAP to
		 * come up in 2.4Ghz as for 40Mhz, 2.4Ghz channel is
		 * not preferred and 80/160Mhz is not allowed for 2.4Ghz
		 * band. So, don't even scan on 2.4Ghz channels if bw is
		 * 40/80/160Mhz and channel list has any 5Ghz channel.
		 */
		if (end_ch_num >= WLAN_REG_CH_NUM(CHAN_ENUM_36) &&
		    ((ch_width == CH_WIDTH_40MHZ) ||
		     (ch_width == CH_WIDTH_80MHZ) ||
		     (ch_width == CH_WIDTH_80P80MHZ) ||
		     (ch_width == CH_WIDTH_160MHZ))) {
			if (WLAN_REG_CH_NUM(loop_count) >=
			    WLAN_REG_CH_NUM(CHAN_ENUM_1) &&
			    WLAN_REG_CH_NUM(loop_count) <=
			    WLAN_REG_CH_NUM(CHAN_ENUM_14))
				continue;
		}

#ifdef FEATURE_WLAN_CH_AVOID
		for (i = 0; i < NUM_CHANNELS; i++) {
			if (safe_channels[i].channelNumber ==
			     WLAN_REG_CH_NUM(loop_count)) {
				/* Check if channel is safe */
				if (true == safe_channels[i].isSafe) {
#endif
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		uint8_t ch;

		ch = WLAN_REG_CH_NUM(loop_count);
		if ((sap_ctx->acs_cfg->skip_scan_status ==
			eSAP_DO_PAR_ACS_SCAN)) {
		    if ((ch >= sap_ctx->acs_cfg->skip_scan_range1_stch &&
			 ch <= sap_ctx->acs_cfg->skip_scan_range1_endch) ||
			(ch >= sap_ctx->acs_cfg->skip_scan_range2_stch &&
			 ch <= sap_ctx->acs_cfg->skip_scan_range2_endch)) {
			list[ch_count] =
				WLAN_REG_CH_NUM(loop_count);
			ch_count++;
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO,
				FL("%d %d added to ACS ch range"),
				ch_count, ch);
		    } else {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO_HIGH,
				FL("%d %d skipped from ACS ch range"),
				ch_count, ch);
		    }
		} else {
			list[ch_count] =
				WLAN_REG_CH_NUM(loop_count);
			ch_count++;
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO,
				FL("%d %d added to ACS ch range"),
				ch_count, ch);
		}
#else
		list[ch_count] = WLAN_REG_CH_NUM(loop_count);
		ch_count++;
#endif
#ifdef FEATURE_WLAN_CH_AVOID
				}
				break;
			}
		}
#endif
	}
	if (0 == ch_count) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		    FL("No active channels present for the current region"));
		/*
		 * LTE COEX: channel range outside the restricted 2.4GHz
		 * band limits
		 */
		if (en_lte_coex && (start_ch_num > band_end_ch))
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
				FL("SAP can't be started as due to LTE COEX"));
	}

	/* return the channel list and number of channels to scan */
	*num_ch = ch_count;
	if (ch_count != 0) {
		*ch_list = list;
	} else {
		*ch_list = NULL;
		qdf_mem_free(list);
	}

	for (loop_count = 0; loop_count < ch_count; loop_count++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			FL("channel number: %d"), list[loop_count]);
	}
	return QDF_STATUS_SUCCESS;
}
#endif

uint8_t sap_indicate_radar(struct sap_context *sap_ctx)
{
	uint8_t target_channel = 0;
	tHalHandle hal;
	tpAniSirGlobal mac;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("null sap_ctx"));
		return 0;
	}

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("null hal"));
		return 0;
	}

	mac = PMAC_STRUCT(hal);

	/*
	 * SAP needs to generate Channel Switch IE
	 * if the radar is found in the STARTED state
	 */
	if (eSAP_STARTED == sap_ctx->sapsMachine)
		mac->sap.SapDfsInfo.csaIERequired = true;

	if (sap_ctx->csr_roamProfile.disableDFSChSwitch)
		return sap_ctx->channel;

	/* set the Radar Found flag in SapDfsInfo */
	mac->sap.SapDfsInfo.sap_radar_found_status = true;

	if (sap_ctx->chan_before_pre_cac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			FL("sapdfs: set chan before pre cac %d as target chan"),
			sap_ctx->chan_before_pre_cac);
		return sap_ctx->chan_before_pre_cac;
	}

	if (sap_ctx->vendor_acs_dfs_lte_enabled && (QDF_STATUS_SUCCESS ==
	    sap_signal_hdd_event(sap_ctx, NULL, eSAP_DFS_NEXT_CHANNEL_REQ,
	    (void *) eSAP_STATUS_SUCCESS)))
		return 0;

	target_channel = sap_random_channel_sel(sap_ctx);
	if (!target_channel)
		sap_signal_hdd_event(sap_ctx, NULL,
		eSAP_DFS_NO_AVAILABLE_CHANNEL, (void *) eSAP_STATUS_SUCCESS);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
		  FL("sapdfs: New selected target channel is [%d]"),
		  target_channel);

	return target_channel;
}

/*
 * CAC timer callback function.
 * Post eSAP_DFS_CHANNEL_CAC_END event to sap_fsm().
 */
void sap_dfs_cac_timer_callback(void *data)
{
	struct sap_context *sapContext;
	tWLAN_SAPEvent sapEvent;
	tHalHandle hHal = (tHalHandle) data;
	tpAniSirGlobal pMac;

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return;
	}
	pMac = PMAC_STRUCT(hHal);
	sapContext = sap_find_cac_wait_session(hHal);
	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: no SAP contexts in wait state", __func__);
		return;
	}

	/*
	 * SAP may not be in CAC wait state, when the timer runs out.
	 * if following flag is set, then timer is in initialized state,
	 * destroy timer here.
	 */
	if (pMac->sap.SapDfsInfo.is_dfs_cac_timer_running == true) {
		if (!sapContext->dfs_cac_offload)
			qdf_mc_timer_destroy(
				&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
		pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = false;
	}

	/*
	 * CAC Complete, post eSAP_DFS_CHANNEL_CAC_END to sap_fsm
	 */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			"sapdfs: Sending eSAP_DFS_CHANNEL_CAC_END for target_channel = %d on sapctx[%pK]",
			sapContext->channel, sapContext);

	sapEvent.event = eSAP_DFS_CHANNEL_CAC_END;
	sapEvent.params = 0;
	sapEvent.u1 = 0;
	sapEvent.u2 = 0;

	sap_fsm(sapContext, &sapEvent);
}

/*
 * Function to stop the DFS CAC Timer
 */
static int sap_stop_dfs_cac_timer(struct sap_context *sapContext)
{
	tHalHandle hHal;
	tpAniSirGlobal pMac;

	if (sapContext == NULL)
		return 0;

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return 0;
	}
	pMac = PMAC_STRUCT(hHal);

	if (sapContext->dfs_cac_offload) {
		pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
		return 0;
	}

	if (QDF_TIMER_STATE_RUNNING !=
	    qdf_mc_timer_get_current_state(&pMac->sap.SapDfsInfo.
					   sap_dfs_cac_timer)) {
		return 0;
	}

	qdf_mc_timer_stop(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
	pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
	qdf_mc_timer_destroy(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);

	return 0;
}


/*
 * Function to start the DFS CAC Timer
 * when SAP is started on a DFS channel
 */
static int sap_start_dfs_cac_timer(struct sap_context *sap_ctx)
{
	QDF_STATUS status;
	uint32_t cac_dur;
	tHalHandle hal = NULL;
	tpAniSirGlobal mac = NULL;
	enum dfs_reg dfs_region;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: null sap_ctx", __func__);
		return 0;
	}

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: null hal", __func__);
		return 0;
	}

	mac = PMAC_STRUCT(hal);
	if (sap_ctx->dfs_cac_offload) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: cac timer offloaded to firmware", __func__);
		mac->sap.SapDfsInfo.is_dfs_cac_timer_running = true;
		return 1;
	}

	sap_get_cac_dur_dfs_region(sap_ctx, &cac_dur, &dfs_region);
	if (0 == cac_dur)
		return 0;

#ifdef QCA_WIFI_NAPIER_EMULATION
	cac_dur = cac_dur / 100;
#endif
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
		  "sapdfs: SAP_DFS_CHANNEL_CAC_START on CH-%d, CAC_DUR-%d sec",
		  sap_ctx->channel, cac_dur / 1000);

	qdf_mc_timer_init(&mac->sap.SapDfsInfo.sap_dfs_cac_timer,
			  QDF_TIMER_TYPE_SW,
			  sap_dfs_cac_timer_callback, (void *)hal);

	/* Start the CAC timer */
	status = qdf_mc_timer_start(&mac->sap.SapDfsInfo.sap_dfs_cac_timer,
			cac_dur);
	if (status == QDF_STATUS_SUCCESS) {
		mac->sap.SapDfsInfo.is_dfs_cac_timer_running = true;
		return 1;
	} else {
		mac->sap.SapDfsInfo.is_dfs_cac_timer_running = false;
		qdf_mc_timer_destroy(&mac->sap.SapDfsInfo.sap_dfs_cac_timer);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to start cac timer", __func__);
		return 0;
	}
}

/*
 * This function initializes the NOL list
 * parameters required to track the radar
 * found DFS channels in the current Reg. Domain .
 */
QDF_STATUS sap_init_dfs_channel_nol_list(struct sap_context *sapContext)
{
	tHalHandle hHal;
	tpAniSirGlobal pMac;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid sapContext pointer on sap_init_dfs_channel_nol_list");
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB();

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	utils_dfs_init_nol(pMac->pdev);

	return QDF_STATUS_SUCCESS;
}

/*
 * This function will calculate how many interfaces
 * have sap persona and returns total number of sap persona.
 */
uint8_t sap_get_total_number_sap_intf(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t intf = 0;
	uint8_t intf_count = 0;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			intf_count++;
		}
	}
	return intf_count;
}

/**
 * is_concurrent_sap_ready_for_channel_change() - to check all saps are ready
 *						  for channel change
 * @hHal: HAL pointer
 * @sapContext: sap context for which this function has been called
 *
 * This function will find the concurrent sap context apart from
 * passed sap context and return its channel change ready status
 *
 *
 * Return: true if other SAP personas are ready to channel switch else false
 */
bool is_concurrent_sap_ready_for_channel_change(tHalHandle hHal,
						struct sap_context *sapContext)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct sap_context *pSapContext;
	uint8_t intf = 0;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			pSapContext =
				pMac->sap.sapCtxList[intf].pSapContext;
			if (pSapContext == sapContext) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("sapCtx matched [%pK]"),
					  sapContext);
				continue;
			} else {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL
						  ("concurrent sapCtx[%pK] didn't matche with [%pK]"),
					  pSapContext, sapContext);
				return pSapContext->is_sap_ready_for_chnl_chng;
			}
		}
	}
	return false;
}

/**
 * sap_is_conc_sap_doing_scc_dfs() - check if conc SAPs are doing SCC DFS
 * @hal: pointer to hal
 * @sap_context: current SAP persona's channel
 *
 * If provided SAP's channel is DFS then Loop through each SAP or GO persona and
 * check if other beaconing entity's channel is same DFS channel. If they are
 * same then concurrent sap is doing SCC DFS.
 *
 * Return: true if two or more beaconing entitity doing SCC DFS else false
 */
bool sap_is_conc_sap_doing_scc_dfs(tHalHandle hal,
				   struct sap_context *given_sapctx)
{
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct sap_context *sap_ctx;
	uint8_t intf = 0, scc_dfs_counter = 0;

	/*
	 * current SAP persona's channel itself is not DFS, so no need to check
	 * what other persona's channel is
	 */
	if (!wlan_reg_is_dfs_ch(mac->pdev,
			given_sapctx->csr_roamProfile.operationChannel)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  FL("skip this loop as provided channel is non-dfs"));
		return false;
	}

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if ((QDF_SAP_MODE != mac->sap.sapCtxList[intf].sapPersona) &&
		    (QDF_P2P_GO_MODE != mac->sap.sapCtxList[intf].sapPersona))
			continue;
		if (!mac->sap.sapCtxList[intf].pSapContext)
			continue;
		sap_ctx = mac->sap.sapCtxList[intf].pSapContext;
		/* if same SAP contexts then skip to next context */
		if (sap_ctx == given_sapctx)
			continue;
		if (given_sapctx->csr_roamProfile.operationChannel ==
				sap_ctx->csr_roamProfile.operationChannel)
			scc_dfs_counter++;
	}

	/* Found atleast two of the beaconing entities doing SCC DFS */
	if (scc_dfs_counter)
		return true;

	return false;
}
