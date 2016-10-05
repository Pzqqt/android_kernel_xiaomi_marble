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

			s a p A p i L i n k C n t l . C

   OVERVIEW:

   This software unit holds the implementation of the WLAN SAP modules
   Link Control functions.

   The functions externalized by this module are to be called ONLY by other
   WLAN modules (HDD)

   DEPENDENCIES:

   Are listed for each API below.
   ===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "qdf_trace.h"
/* Pick up the CSR callback definition */
#include "csr_api.h"
#include "ani_global.h"
#include "csr_inside_api.h"
#include "sme_api.h"
/* SAP Internal API header file */
#include "sap_internal.h"
#include "cds_concurrency.h"
#include "wma.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define SAP_DEBUG

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/*
 * wlansap_scan_callback() - Callback for Scan (scan results) Events
 *
 * @hal_handle  : tHalHandle passed in with the scan request
 * @ctx   : The second context pass in for the caller (sapContext)
 * @scanID      : scanID got after the scan
 *
 *  Callback for Scan (scan results) Events
 *
 * Return: Status
 */
QDF_STATUS wlansap_scan_callback(tHalHandle hal_handle,
				 void *ctx,   /* Opaque SAP handle */
				 uint8_t session_id,
				 uint32_t scan_id, eCsrScanStatus scan_status) {
	tScanResultHandle result = NULL;
	QDF_STATUS get_result_status = QDF_STATUS_E_FAILURE;
	ptSapContext sap_ctx = (ptSapContext) ctx;
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	uint8_t operChannel = 0;
	QDF_STATUS sap_sm_status;

#ifdef SOFTAP_CHANNEL_RANGE
	uint32_t event;
#endif

	if (NULL == hal_handle) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
					"In %s invalid hHal", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (sap_ctx->sapsMachine == eSAP_DISCONNECTED) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				"In %s BSS already stopped", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	switch (scan_status) {
	case eCSR_SCAN_SUCCESS:
		/* sapScanCompleteCallback with eCSR_SCAN_SUCCESS */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, CSR scanStatus = %s (%d)", __func__,
			  "eCSR_SCAN_SUCCESS", scan_status);

		/**
		 * Get scan results, Run channel selection algorithm,
		 * select channel and keep in sap_ctx->Channel
		 */
		get_result_status =
			sme_scan_get_result(hal_handle, sap_ctx->sessionId, NULL,
					    &result);

		event = eSAP_MAC_SCAN_COMPLETE;

		if ((get_result_status != QDF_STATUS_SUCCESS)
		    && (get_result_status != QDF_STATUS_E_NULL_VALUE)) {
			/* No scan results */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "In %s, Get scan result failed! ret = %d",
				  __func__, get_result_status);
			break;
		}
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (scan_id != 0) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Sending ACS Scan skip event", __func__);
			sap_signal_hdd_event(sap_ctx, NULL,
					  eSAP_ACS_SCAN_SUCCESS_EVENT,
					  (void *) eSAP_STATUS_SUCCESS);
		} else
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: ACS scan id: %d (skipped ACS SCAN)",
				  __func__, scan_id);
#endif
		operChannel = sap_select_channel(hal_handle, sap_ctx, result);
		if (!operChannel) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"No channel was selected from preferred channel for Operating channel");

			operChannel = sap_ctx->acs_cfg->start_ch;

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"Selecting operating channel as starting channel from preferred channel list: %d",
				operChannel);
		}
		sme_scan_result_purge(hal_handle, result);
		break;

	default:
		event = eSAP_CHANNEL_SELECTION_FAILED;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, CSR scanStatus = %s (%d)", __func__,
			  "eCSR_SCAN_ABORT/FAILURE", scan_status);
	}

	if (operChannel == SAP_CHANNEL_NOT_SELECTED)
#ifdef SOFTAP_CHANNEL_RANGE
	{
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s: No suitable channel selected due to DFS, LTE"
			"COEX and concurrent mode restrictions", __func__);

		sap_ctx->sapsMachine = eSAP_CH_SELECT;
		event = eSAP_CHANNEL_SELECTION_FAILED;
	}
#else
		sap_ctx->channel = SAP_DEFAULT_24GHZ_CHANNEL;
#endif
	else {
		sap_ctx->channel = operChannel;
	}

	sap_ctx->ch_params.ch_width = sap_ctx->acs_cfg->ch_width;
	cds_set_channel_params(sap_ctx->channel,
		sap_ctx->secondary_ch,
		&sap_ctx->ch_params);
#ifdef SOFTAP_CHANNEL_RANGE
	if (sap_ctx->channelList != NULL) {
		/* Always free up the memory for channel selection whatever
		 * the result */
		qdf_mem_free(sap_ctx->channelList);
		sap_ctx->channelList = NULL;
	}
#endif

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "In %s, Channel selected = %d", __func__,
		  sap_ctx->channel);

	/* Fill in the event structure */
	sapEvent.event = event;
	/* pCsrRoamInfo; */
	sapEvent.params = 0;
	/* roamstatus */
	sapEvent.u1 = scan_status;
	/* roamResult */
	sapEvent.u2 = 0;

	/* Handle event */
	sap_sm_status = sap_fsm(sap_ctx, &sapEvent);

	return sap_sm_status;
} /* wlansap_scan_callback */


/**
 * sap_config_acs_result : Generate ACS result params based on ch constraints
 * @sap_ctx: pointer to SAP context data struct
 * @hal: HAL Handle pointer
 *
 * This function calculates the ACS result params: ht sec channel, vht channel
 * information and channel bonding based on selected ACS channel.
 *
 * Return: None
 */

void sap_config_acs_result(tHalHandle hal, ptSapContext sap_ctx,
							uint32_t sec_ch)
{
	uint32_t channel = sap_ctx->acs_cfg->pri_ch;
	struct ch_params_s ch_params = {0};

	ch_params.ch_width = sap_ctx->acs_cfg->ch_width;
	cds_set_channel_params(channel, sec_ch, &ch_params);
	sap_ctx->acs_cfg->ch_width = ch_params.ch_width;
	if (sap_ctx->acs_cfg->ch_width > CH_WIDTH_40MHZ)
		sap_ctx->acs_cfg->vht_seg0_center_ch =
						ch_params.center_freq_seg0;
	else
		sap_ctx->acs_cfg->vht_seg0_center_ch = 0;

	if (sap_ctx->acs_cfg->ch_width == CH_WIDTH_80P80MHZ)
		sap_ctx->acs_cfg->vht_seg1_center_ch =
						ch_params.center_freq_seg1;
	else
		sap_ctx->acs_cfg->vht_seg1_center_ch = 0;

	if (ch_params.sec_ch_offset == PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
		sap_ctx->acs_cfg->ht_sec_ch = sap_ctx->acs_cfg->pri_ch - 4;
	else if (ch_params.sec_ch_offset == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
		sap_ctx->acs_cfg->ht_sec_ch = sap_ctx->acs_cfg->pri_ch + 4;
	else
		sap_ctx->acs_cfg->ht_sec_ch = 0;
}

/**
 * sap_hdd_signal_event_handler() - routine to inform hostapd via callback
 *
 * ctx: pointer to sap context which was passed to callback
 *
 * this routine will be registered as callback to sme_close_session, so upon
 * closure of sap session it notifies the hostapd
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_hdd_signal_event_handler(void *ctx)
{
	ptSapContext sap_ctx = (struct sSapContext *)ctx;
	QDF_STATUS status;
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("sap context is not valid"));
		return QDF_STATUS_E_FAILURE;
	}
	status = sap_signal_hdd_event(sap_ctx, NULL,
			sap_ctx->sap_state,
			(void *) sap_ctx->sap_status);
	return status;
}

/**
 *
 * wlansap_pre_start_bss_acs_scan_callback() - callback for scan results
 *
 * hal_handle:    the hal_handle passed in with the scan request
 * pcontext:      the second context pass in for the caller, opaque sap Handle.
 * scanid:        scan id passed
 * sessionid:     session identifier
 * status:        status of scan -success, failure or abort
 *
 * Api for scan callback. This function is invoked as a result of scan
 * completion and reports the scan results.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 */
QDF_STATUS
wlansap_pre_start_bss_acs_scan_callback(tHalHandle hal_handle, void *pcontext,
					uint8_t sessionid, uint32_t scanid,
					eCsrScanStatus scan_status)
{
	tScanResultHandle presult = NULL;
	QDF_STATUS scan_get_result_status = QDF_STATUS_E_FAILURE;
	ptSapContext sap_ctx = (ptSapContext)pcontext;
	uint8_t oper_channel = 0;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (eCSR_SCAN_SUCCESS != scan_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("CSR scan_status = eCSR_SCAN_ABORT/FAILURE (%d), choose default channel"),
			scan_status);
		sap_ctx->channel =
			sap_select_default_oper_chan(hal_handle,
					sap_ctx->acs_cfg->hw_mode);
		sap_ctx->sap_state = eSAP_ACS_CHANNEL_SELECTED;
		sap_ctx->sap_status = eSAP_STATUS_SUCCESS;
		goto close_session;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("CSR scan_status = eCSR_SCAN_SUCCESS (%d)"), scan_status);
	/*
	* Now do
	* 1. Get scan results
	* 2. Run channel selection algorithm
	* select channel and store in pSapContext->Channel
	*/
	scan_get_result_status = sme_scan_get_result(hal_handle,
					sap_ctx->sessionId,
					NULL, &presult);
	if ((scan_get_result_status != QDF_STATUS_SUCCESS) &&
		(scan_get_result_status != QDF_STATUS_E_NULL_VALUE)) {
		/*
		* No scan results So, set the operation channel not selected
		* to allow the default channel to be set when reporting to HDD
		*/
		oper_channel = SAP_CHANNEL_NOT_SELECTED;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Get scan result failed! ret = %d"),
		scan_get_result_status);
	} else {
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (scanid != 0) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("Sending ACS Scan skip event"));
			sap_signal_hdd_event(sap_ctx, NULL,
					     eSAP_ACS_SCAN_SUCCESS_EVENT,
					     (void *) eSAP_STATUS_SUCCESS);
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("ACS scanid: %d (skipped ACS SCAN)"),
				  scanid);
		}
#endif
		oper_channel = sap_select_channel(hal_handle, sap_ctx, presult);
		if (!oper_channel) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"No channel was selected from preferred channel for Operating channel");

			oper_channel = sap_ctx->acs_cfg->start_ch;

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"Selecting operating channel as starting channel from preferred channel list: %d",
				oper_channel);
		}
		sme_scan_result_purge(hal_handle, presult);
	}

	if (oper_channel == SAP_CHANNEL_NOT_SELECTED) {
#ifdef SOFTAP_CHANNEL_RANGE
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("No suitable channel selected"));

		sap_ctx->sap_state = eSAP_ACS_CHANNEL_SELECTED;
		sap_ctx->sap_status = eSAP_STATUS_FAILURE;
		goto close_session;
	} else {
#else
		sap_ctx->channel =
			sap_select_default_oper_chan(hal_handle,
				sap_ctx->acs_cfg->hw_mode);
	} else {
#endif
		/* Valid Channel Found from scan results. */
		sap_ctx->acs_cfg->pri_ch = oper_channel;
		sap_ctx->channel = oper_channel;
	}
	sap_config_acs_result(hal_handle, sap_ctx,
			sap_ctx->acs_cfg->ht_sec_ch);

#ifdef SOFTAP_CHANNEL_RANGE
	if (sap_ctx->channelList != NULL) {
		/*
		* Always free up the memory for
		* channel selection whatever
		* the result
		*/
		qdf_mem_free(sap_ctx->channelList);
		sap_ctx->channelList = NULL;
	}
#endif
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Channel selected = %d"), sap_ctx->channel);
	sap_ctx->sap_state = eSAP_ACS_CHANNEL_SELECTED;
	sap_ctx->sap_status = eSAP_STATUS_SUCCESS;
close_session:
	sap_hdd_signal_event_handler(sap_ctx);
	return status;
}

/**
 * wlansap_roam_process_ch_change_success() - handles the case for
 * eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS in function wlansap_roam_callback()
 *
 * @mac_ctx:        mac global context
 * @sap_ctx:        sap context
 * @csr_roam_info:  raom info struct
 * @ret_status:     update return status
 *
 * Return: void
 */
static void
wlansap_roam_process_ch_change_success(tpAniSirGlobal mac_ctx,
				      ptSapContext sap_ctx,
				      tCsrRoamInfo *csr_roam_info,
				      QDF_STATUS *ret_status)
{
	tWLAN_SAPEvent sap_event;
	QDF_STATUS qdf_status;
	bool is_ch_dfs = false;
	/*
	 * Channel change is successful. If the new channel is a DFS channel,
	 * then we will to perform channel availability check for 60 seconds
	 */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
		  FL("sapdfs: changing target channel to [%d]"),
		  mac_ctx->sap.SapDfsInfo.target_channel);
	sap_ctx->channel = mac_ctx->sap.SapDfsInfo.target_channel;
	/* Identify if this is channel change in radar detected state */
	if (eSAP_DISCONNECTING != sap_ctx->sapsMachine)
		return;

	if (sap_ctx->ch_params.ch_width == CH_WIDTH_160MHZ) {
		is_ch_dfs = true;
	} else if (sap_ctx->ch_params.ch_width == CH_WIDTH_80P80MHZ) {
		if (cds_get_channel_state(sap_ctx->channel) ==
						CHANNEL_STATE_DFS ||
		    cds_get_channel_state(sap_ctx->ch_params.center_freq_seg1 -
					  SIR_80MHZ_START_CENTER_CH_DIFF) ==
							CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	} else {
		if (cds_get_channel_state(sap_ctx->channel) ==
						CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	}

	/* check if currently selected channel is a DFS channel */
	if (is_ch_dfs && sap_ctx->pre_cac_complete) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED, FL(
		    "sapdfs: eSAP_DISCONNECTING => eSAP_STARTING, on pre cac"));
		/* Start beaconing on the new pre cac channel */
		wlansap_start_beacon_req(sap_ctx);
		sap_ctx->sapsMachine = eSAP_STARTING;
		mac_ctx->sap.SapDfsInfo.sap_radar_found_status = false;
		sap_event.event = eSAP_MAC_START_BSS_SUCCESS;
		sap_event.params = csr_roam_info;
		sap_event.u1 = eCSR_ROAM_INFRA_IND;
		sap_event.u2 = eCSR_ROAM_RESULT_INFRA_STARTED;
	} else if (is_ch_dfs) {
		if ((false == mac_ctx->sap.SapDfsInfo.ignore_cac)
		    && (eSAP_DFS_DO_NOT_SKIP_CAC ==
			mac_ctx->sap.SapDfsInfo.cac_state)) {
			sap_ctx->sapsMachine = eSAP_DISCONNECTED;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  FL("sapdfs: from state eSAP_DISCONNECTING => DISCONNECTED with ignore cac false on sapctx[%p]"),
				  sap_ctx);
			/* DFS Channel */
			sap_event.event = eSAP_DFS_CHANNEL_CAC_START;
			sap_event.params = csr_roam_info;
			sap_event.u1 = 0;
			sap_event.u2 = 0;
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO_MED,
				  FL("sapdfs: from state eSAP_DISCONNECTING => eSAP_STARTING with ignore cac true on sapctx[%p]"),
				  sap_ctx);

			/* Start beaconing on the new channel */
			wlansap_start_beacon_req(sap_ctx);
			sap_ctx->sapsMachine = eSAP_STARTING;
			mac_ctx->sap.SapDfsInfo.sap_radar_found_status = false;
			sap_event.event = eSAP_MAC_START_BSS_SUCCESS;
			sap_event.params = csr_roam_info;
			sap_event.u1 = eCSR_ROAM_INFRA_IND;
			sap_event.u2 = eCSR_ROAM_RESULT_INFRA_STARTED;
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: from state eSAP_DISCONNECTING => eSAP_STARTING on sapctx[%p]"),
			  sap_ctx);
		/* non-DFS channel */
		sap_ctx->sapsMachine = eSAP_STARTING;
		mac_ctx->sap.SapDfsInfo.sap_radar_found_status = false;
		sap_event.event = eSAP_MAC_START_BSS_SUCCESS;
		sap_event.params = csr_roam_info;
		sap_event.u1 = eCSR_ROAM_INFRA_IND;
		sap_event.u2 = eCSR_ROAM_RESULT_INFRA_STARTED;
	}

	/* Handle the event */
	qdf_status = sap_fsm(sap_ctx, &sap_event);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		*ret_status = QDF_STATUS_E_FAILURE;
}

/**
 * wlansap_roam_process_dfs_chansw_update() - handles the case for
 * eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS in wlansap_roam_callback()
 *
 * @hal:           hal global context
 * @sap_ctx:       sap context
 * @ret_status:    update return status
 *
 * Return: void
 */
static void
wlansap_roam_process_dfs_chansw_update(tHalHandle hHal,
					    ptSapContext sap_ctx,
					    QDF_STATUS *ret_status)
{
	tWLAN_SAPEvent sap_event;
	uint8_t intf;
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hHal);
	uint8_t dfs_beacon_start_req = 0;

	if (sap_ctx->csr_roamProfile.disableDFSChSwitch) {
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_ERROR,
			  FL("sapdfs: DFS channel switch disabled"));
		/*
		 * Send a beacon start request to PE. CSA IE required flag from
		 * beacon template will be cleared by now. A new beacon template
		 * with no CSA IE will be sent to firmware.
		 */
		dfs_beacon_start_req = eSAP_TRUE;
		sap_ctx->pre_cac_complete = false;
		*ret_status = sme_roam_start_beacon_req(hHal, sap_ctx->bssid,
							dfs_beacon_start_req);
		return;
	}
	/*
	 * Irrespective of whether the channel switch IE was sent out
	 * successfully or not, SAP should still vacate the channel immediately
	 */
	if (eSAP_STARTED != sap_ctx->sapsMachine) {
		/* Further actions to be taken here */
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_WARN,
			  FL("eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND received in (%d) state"),
			  sap_ctx->sapsMachine);
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
		  FL("sapdfs: from state eSAP_STARTED => eSAP_DISCONNECTING"));
	/* SAP to be moved to DISCONNECTING state */
	sap_ctx->sapsMachine = eSAP_DISCONNECTING;
	/*
	 * The associated stations have been informed to move to a different
	 * channel. However, the AP may not always select the advertised channel
	 * for operation if the radar is seen. In that case, the stations will
	 * experience link-loss and return back through scanning if they wish to
	 */

	/*
	 * Send channel change request. From spec it is required that the AP
	 * should continue to operate in the same mode as it is operating
	 * currently. For e.g. 20/40/80 MHz operation
	 */
	if (mac_ctx->sap.SapDfsInfo.target_channel)
		cds_set_channel_params(mac_ctx->sap.SapDfsInfo.target_channel,
			0, &sap_ctx->ch_params);

	/*
	 * Fetch the number of SAP interfaces. If the number of sap Interface
	 * more than one then we will make is_sap_ready_for_chnl_chng to true
	 * for that sapctx. If there is only one SAP interface then process
	 * immediately
	 */
	if (sap_get_total_number_sap_intf(hHal) <= 1) {
		/* Send channel switch request */
		sap_event.event = eWNI_SME_CHANNEL_CHANGE_REQ;
		sap_event.params = 0;
		sap_event.u1 = 0;
		sap_event.u2 = 0;
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: Posting event eWNI_SME_CHANNEL_CHANGE_REQ to sapFSM"));
		/* Handle event */
		qdf_status = sap_fsm(sap_ctx, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			*ret_status = QDF_STATUS_E_FAILURE;
		return;
	}

	sap_ctx->is_sap_ready_for_chnl_chng = true;
	/*
	 * now check if the con-current sap interface is ready
	 * for channel change. If yes then we issue channel change for
	 * both the SAPs. If no then simply return success & we will
	 * issue channel change when second AP's 5 CSA beacon Tx is
	 * completed.
	 */
	if (false ==
	    is_concurrent_sap_ready_for_channel_change(hHal, sap_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: sapctx[%p] ready but not concurrent sap"),
			  sap_ctx);
		*ret_status = QDF_STATUS_SUCCESS;
		return;
	}

	/* Issue channel change req for each sapctx */
	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		ptSapContext pSapContext;
		if (!((QDF_SAP_MODE == mac_ctx->sap.sapCtxList[intf].sapPersona)
		    && (mac_ctx->sap.sapCtxList[intf].pSapContext != NULL)))
			continue;

		pSapContext = mac_ctx->sap.sapCtxList[intf].pSapContext;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs:issue chnl change for sapctx[%p]"),
			  pSapContext);
		/* Send channel switch request */
		sap_event.event = eWNI_SME_CHANNEL_CHANGE_REQ;
		sap_event.params = 0;
		sap_event.u1 = 0;
		sap_event.u2 = 0;
		/* Handle event */
		qdf_status = sap_fsm(pSapContext, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("post chnl chng req failed, sap[%p]"),
				  sap_ctx);
			*ret_status = QDF_STATUS_E_FAILURE;
		} else {
			pSapContext->is_sap_ready_for_chnl_chng = false;
		}
	}
	return;
}

/**
 * wlansap_roam_process_dfs_radar_found() - handles the case for
 * eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND in wlansap_roam_callback()
 *
 * @mac_ctx:       mac global context
 * @sap_ctx:       sap context
 * @ret_status:    update return status
 *
 * Return: result of operation
 */
static void
wlansap_roam_process_dfs_radar_found(tpAniSirGlobal mac_ctx,
				     ptSapContext sap_ctx,
				     QDF_STATUS *ret_status)
{
	QDF_STATUS qdf_status;
	tWLAN_SAPEvent sap_event;
	if (eSAP_DFS_CAC_WAIT == sap_ctx->sapsMachine) {
		if (sap_ctx->csr_roamProfile.disableDFSChSwitch) {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_ERROR,
				"sapdfs: DFS channel switch disabled");
			return;
		}
		if (false == mac_ctx->sap.SapDfsInfo.sap_radar_found_status) {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_ERROR,
				"sapdfs: sap_radar_found_status is false");
			return;
		}
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs:Posting event eSAP_DFS_CHANNEL_CAC_RADAR_FOUND"));
		/*
		 * If Radar is found, while in DFS CAC WAIT State then post stop
		 * and destroy the CAC timer and post a
		 * eSAP_DFS_CHANNEL_CAC_RADAR_FOUND  to sapFsm.
		 */
		qdf_mc_timer_stop(&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
		qdf_mc_timer_destroy(&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
		mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running = false;

		/*
		 * User space is already indicated the CAC start and if
		 * CAC end on this channel is not indicated, the user
		 * space will be in some undefined state (e.g., UI frozen)
		 */
		qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
				eSAP_DFS_CAC_INTERRUPTED,
				(void *) eSAP_STATUS_SUCCESS);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to send CAC end"));
			/* Want to still proceed and try to switch channel.
			 * Lets try not to be on the DFS channel
			 */
		}

		sap_event.event = eSAP_DFS_CHANNEL_CAC_RADAR_FOUND;
		sap_event.params = 0;
		sap_event.u1 = 0;
		sap_event.u2 = 0;
		qdf_status = sap_fsm(sap_ctx, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			*ret_status = QDF_STATUS_E_FAILURE;
		return;
	}
	if (eSAP_STARTED == sap_ctx->sapsMachine) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs:Posting event eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START"));

		/*
		 * Radar found on the operating channel in STARTED state,
		 * new operating channel has already been selected. Send
		 * request to SME-->PE for sending CSA IE
		 */
		sap_event.event = eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START;
		sap_event.params = 0;
		sap_event.u1 = 0;
		sap_event.u2 = 0;
		qdf_status = sap_fsm(sap_ctx, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			*ret_status = QDF_STATUS_E_FAILURE;
		return;
	}
	/* Further actions to be taken here */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  FL("eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND received in (%d) state"),
		  sap_ctx->sapsMachine);

	return;
}

/**
 * wlansap_roam_process_infra_assoc_ind() - handles the case for
 * eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND in wlansap_roam_callback()
 *
 * @sap_ctx:       sap context
 * @roam_result:   roam result
 * @csr_roam_info: roam info struct
 * @ret_status:    update return status
 *
 * Return: result of operation
 */
static void
wlansap_roam_process_infra_assoc_ind(ptSapContext sap_ctx,
				     eCsrRoamResult roam_result,
				     tCsrRoamInfo *csr_roam_info,
				     QDF_STATUS *ret_status)
{
	QDF_STATUS qdf_status;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("CSR roam_result = eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND (%d)"),
		  roam_result);
	sap_ctx->nStaWPARSnReqIeLength = csr_roam_info->rsnIELen;
	if (sap_ctx->nStaWPARSnReqIeLength)
		qdf_mem_copy(sap_ctx->pStaWpaRsnReqIE, csr_roam_info->prsnIE,
			     sap_ctx->nStaWPARSnReqIeLength);
#ifdef FEATURE_WLAN_WAPI
	sap_ctx->nStaWAPIReqIeLength = csr_roam_info->wapiIELen;
	if (sap_ctx->nStaWAPIReqIeLength)
		qdf_mem_copy(sap_ctx->pStaWapiReqIE, csr_roam_info->pwapiIE,
			     sap_ctx->nStaWAPIReqIeLength);
#endif
	sap_ctx->nStaAddIeLength = csr_roam_info->addIELen;
	if (sap_ctx->nStaAddIeLength)
		qdf_mem_copy(sap_ctx->pStaAddIE, csr_roam_info->paddIE,
			     sap_ctx->nStaAddIeLength);
	sap_ctx->SapQosCfg.WmmIsEnabled = csr_roam_info->wmmEnabledSta;
	/* MAC filtering */
	qdf_status = sap_is_peer_mac_allowed(sap_ctx,
				     (uint8_t *) csr_roam_info->peerMac.bytes);

	if (QDF_STATUS_SUCCESS == qdf_status) {
		qdf_status = sap_signal_hdd_event(sap_ctx,
				csr_roam_info, eSAP_STA_ASSOC_IND,
				(void *) eSAP_STATUS_SUCCESS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("CSR roam_result = (%d) MAC ("MAC_ADDRESS_STR") fail"),
				  roam_result, MAC_ADDR_ARRAY(
					csr_roam_info->peerMac.bytes));
		*ret_status = QDF_STATUS_E_FAILURE;
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
			  FL("CSR roam_result = (%d) MAC ("MAC_ADDRESS_STR") not allowed"),
			  roam_result,
			  MAC_ADDR_ARRAY(csr_roam_info->peerMac.bytes));
		*ret_status = QDF_STATUS_E_FAILURE;
	}
	return;
}

/**
 * wlansap_roam_callback() - Callback for Roam (connection status) Events
 * @ctx             : pContext passed in with the roam request
 * @csr_roam_info   : Pointer to a tCsrRoamInfo
 * @roamId          : to identify the callback related roam request.
 *                    0 means unsolicit
 * @roam_status     : Flag indicating the status of the callback
 * @roam_result     : Result
 *
 * This function finds dot11 mode based on current mode, operating channel and
 * fw supported modes.
 *
 * Return: Status of operation
 */
QDF_STATUS
wlansap_roam_callback(void *ctx, tCsrRoamInfo *csr_roam_info, uint32_t roamId,
		      eRoamCmdStatus roam_status, eCsrRoamResult roam_result)
{
	/* sap_ctx value */
	ptSapContext sap_ctx;
	/* State machine event */
	tWLAN_SAPEvent sap_event;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	tHalHandle hal;
	tpAniSirGlobal mac_ctx = NULL;
	uint8_t intf;

	if (QDF_IS_STATUS_ERROR(wlansap_context_get((ptSapContext)ctx)))
		return QDF_STATUS_E_FAILURE;

	sap_ctx = (ptSapContext) ctx;
	hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid handle"));
		return QDF_STATUS_E_NOMEM;
	}

	mac_ctx = PMAC_STRUCT(hal);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Before switch on roam_status = %d"), roam_status);
	switch (roam_status) {
	case eCSR_ROAM_SESSION_OPENED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Session %d opened successfully"),
			  sap_ctx->sessionId);
		sap_ctx->isSapSessionOpen = eSAP_TRUE;
		qdf_event_set(&sap_ctx->sap_session_opened_evt);
		break;
	case eCSR_ROAM_INFRA_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_INFRA_IND (%d)"),
			  roam_status);
		if (roam_result == eCSR_ROAM_RESULT_INFRA_START_FAILED) {
			/* Fill in the event structure */
			sap_event.event = eSAP_MAC_START_FAILS;
			sap_event.params = csr_roam_info;
			sap_event.u1 = roam_status;
			sap_event.u2 = roam_result;
			/* Handle event */
			qdf_status = sap_fsm(sap_ctx, &sap_event);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status))
				qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_LOSTLINK:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_LOSTLINK (%d)"),
			  roam_status);
		break;
	case eCSR_ROAM_MIC_ERROR_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_MIC_ERROR_IND (%d)"),
			  roam_status);
		break;
	case eCSR_ROAM_SET_KEY_COMPLETE:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_SET_KEY_COMPLETE (%d)"),
			  roam_status);
		if (roam_result == eCSR_ROAM_RESULT_FAILURE)
			sap_signal_hdd_event(sap_ctx, csr_roam_info,
					     eSAP_STA_SET_KEY_EVENT,
					     (void *) eSAP_STATUS_FAILURE);
		break;
	case eCSR_ROAM_ASSOCIATION_COMPLETION:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_ASSOCIATION_COMPLETION (%d)"),
			  roam_status);
		if (roam_result == eCSR_ROAM_RESULT_FAILURE)
			sap_signal_hdd_event(sap_ctx, csr_roam_info,
					     eSAP_STA_REASSOC_EVENT,
					     (void *) eSAP_STATUS_FAILURE);
		break;
	case eCSR_ROAM_DISASSOCIATED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_DISASSOCIATED (%d)"),
			  roam_status);
		if (roam_result == eCSR_ROAM_RESULT_MIC_FAILURE)
			sap_signal_hdd_event(sap_ctx, csr_roam_info,
					     eSAP_STA_MIC_FAILURE_EVENT,
					     (void *) eSAP_STATUS_FAILURE);
		break;
	case eCSR_ROAM_WPS_PBC_PROBE_REQ_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_status = eCSR_ROAM_WPS_PBC_PROBE_REQ_IND (%d)"),
			  roam_status);
		break;
	case eCSR_ROAM_REMAIN_CHAN_READY:
		/* roamId contains scan identifier */
		sap_ctx->roc_ind_scan_id = csr_roam_info->roc_scan_id;
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				     eSAP_REMAIN_CHAN_READY,
				     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_DISCONNECT_ALL_P2P_CLIENTS:
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				     eSAP_DISCONNECT_ALL_P2P_CLIENT,
				     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_SEND_P2P_STOP_BSS:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Received stopbss"));
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				     eSAP_MAC_TRIG_STOP_BSS_EVENT,
				     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_DFS_RADAR_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Received Radar Indication"));

		if (sap_ctx->is_pre_cac_on) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				FL("sapdfs: Radar detect on pre cac:%d"),
				sap_ctx->sessionId);
			qdf_mc_timer_stop(
				&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
			qdf_mc_timer_destroy(
				&mac_ctx->sap.SapDfsInfo.sap_dfs_cac_timer);
			mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running =
				false;
			sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC,
					(void *) eSAP_STATUS_SUCCESS);
			break;
		}

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: Indicate eSAP_DFS_RADAR_DETECT to HDD"));
		sap_signal_hdd_event(sap_ctx, NULL, eSAP_DFS_RADAR_DETECT,
				     (void *) eSAP_STATUS_SUCCESS);
		/* sync to latest DFS-NOL */
		sap_signal_hdd_event(sap_ctx, NULL, eSAP_DFS_NOL_GET,
				    (void *) eSAP_STATUS_SUCCESS);
		mac_ctx->sap.SapDfsInfo.target_channel =
		    sap_indicate_radar(sap_ctx, &csr_roam_info->dfs_event);
		/* if there is an assigned next channel hopping */
		if (0 < mac_ctx->sap.SapDfsInfo.user_provided_target_channel) {
			mac_ctx->sap.SapDfsInfo.target_channel =
			   mac_ctx->sap.SapDfsInfo.user_provided_target_channel;
			mac_ctx->sap.SapDfsInfo.user_provided_target_channel =
			   0;
		}
		if (mac_ctx->sap.SapDfsInfo.target_channel != 0) {
			mac_ctx->sap.SapDfsInfo.cac_state =
				eSAP_DFS_DO_NOT_SKIP_CAC;
			sap_cac_reset_notify(hal);
			/* set DFS-NOL back to keep it update-to-date in CNSS */
			sap_signal_hdd_event(sap_ctx, NULL, eSAP_DFS_NOL_SET,
					     (void *) eSAP_STATUS_SUCCESS);
			break;
		}
		/* Issue stopbss for each sapctx */
		for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
			ptSapContext pSapContext;
			if (((QDF_SAP_MODE ==
			    mac_ctx->sap.sapCtxList[intf].sapPersona) ||
			    (QDF_P2P_GO_MODE ==
			    mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
			    mac_ctx->sap.sapCtxList[intf].pSapContext !=
			    NULL) {
				pSapContext =
				    mac_ctx->sap.sapCtxList[intf].pSapContext;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("sapdfs: no available channel for sapctx[%p], StopBss"),
					  pSapContext);
				wlansap_stop_bss(pSapContext);
			}
		}
		break;
	case eCSR_ROAM_DFS_CHAN_SW_NOTIFY:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Received Chan Sw Update Notification"));
		break;
	case eCSR_ROAM_SET_CHANNEL_RSP:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("Received set channel response"));
		break;
	case eCSR_ROAM_EXT_CHG_CHNL_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				FL("Received set channel Indication"));
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("CSR roam_status not handled roam_status = %s (%d)"),
			  get_e_roam_cmd_status_str(roam_status),
			  roam_status);
		break;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Before switch on roam_result = %d"), roam_result);

	switch (roam_result) {
	case eCSR_ROAM_RESULT_INFRA_ASSOCIATION_IND:
		wlansap_roam_process_infra_assoc_ind(sap_ctx, roam_result,
						csr_roam_info, &qdf_ret_status);
		break;
	case eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF (%d)"),
			  roam_result);
		sap_ctx->nStaWPARSnReqIeLength = csr_roam_info->rsnIELen;
		if (sap_ctx->nStaWPARSnReqIeLength)
			qdf_mem_copy(sap_ctx->pStaWpaRsnReqIE,
				     csr_roam_info->prsnIE,
				     sap_ctx->nStaWPARSnReqIeLength);

		sap_ctx->nStaAddIeLength = csr_roam_info->addIELen;
		if (sap_ctx->nStaAddIeLength)
			qdf_mem_copy(sap_ctx->pStaAddIE,
				     csr_roam_info->paddIE,
				     sap_ctx->nStaAddIeLength);

		sap_ctx->SapQosCfg.WmmIsEnabled =
			csr_roam_info->wmmEnabledSta;
		/* Fill in the event structure */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
					eSAP_STA_ASSOC_EVENT,
					(void *) eSAP_STATUS_SUCCESS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_DISASSOC_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_DISASSOC_IND (%d)"),
			  roam_result);
		/* Fill in the event structure */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
					eSAP_STA_DISASSOC_EVENT,
					(void *) eSAP_STATUS_SUCCESS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_DEAUTH_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_DEAUTH_IND (%d)"),
			  roam_result);
		/*
		 * Fill in the event structure
		 * we use the same event inorder to inform HDD to disassociate
		 * the station
		 */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
					eSAP_STA_DISASSOC_EVENT,
					(void *) eSAP_STATUS_SUCCESS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_MIC_ERROR_GROUP:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_MIC_ERROR_GROUP (%d)"),
			  roam_result);
		/*
		 * Fill in the event structure
		 * TODO: support for group key MIC failure event to be handled
		 */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
						eSAP_STA_MIC_FAILURE_EVENT,
						(void *) NULL);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_MIC_ERROR_UNICAST:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_MIC_ERROR_UNICAST (%d)"),
			  roam_result);
		/*
		 * Fill in the event structure
		 * TODO: support for unicast key MIC failure event to be handled
		 */
		qdf_status =
			sap_signal_hdd_event(sap_ctx, csr_roam_info,
					  eSAP_STA_MIC_FAILURE_EVENT,
					  (void *) NULL);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		}
		break;
	case eCSR_ROAM_RESULT_AUTHENTICATED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_AUTHENTICATED (%d)"),
			  roam_result);
		/* Fill in the event structure */
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				  eSAP_STA_SET_KEY_EVENT,
				  (void *) eSAP_STATUS_SUCCESS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_ASSOCIATED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_ASSOCIATED (%d)"),
			  roam_result);
		/* Fill in the event structure */
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				     eSAP_STA_REASSOC_EVENT,
				     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_RESULT_INFRA_STARTED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_INFRA_STARTED (%d)"),
			  roam_result);
		/*
		 * In the current implementation, hostapd is not aware that
		 * drive will support DFS. Hence, driver should inform
		 * eSAP_MAC_START_BSS_SUCCESS to upper layers and then perform
		 * CAC underneath
		 */
		sap_event.event = eSAP_MAC_START_BSS_SUCCESS;
		sap_event.params = csr_roam_info;
		sap_event.u1 = roam_status;
		sap_event.u2 = roam_result;
		qdf_status = sap_fsm(sap_ctx, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_INFRA_STOPPED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_INFRA_STOPPED (%d)"),
			  roam_result);
		/* Fill in the event structure */
		sap_event.event = eSAP_MAC_READY_FOR_CONNECTIONS;
		sap_event.params = csr_roam_info;
		sap_event.u1 = roam_status;
		sap_event.u2 = roam_result;
		/* Handle event */
		qdf_status = sap_fsm(sap_ctx, &sap_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_WPS_PBC_PROBE_REQ_IND (%d)"),
			  roam_result);
		/*
		 * Fill in the event structure
		 * TODO: support for group key MIC failure event to be handled
		 */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
						eSAP_WPS_PBC_PROBE_REQ_EVENT,
						(void *) NULL);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	case eCSR_ROAM_RESULT_FORCED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_FORCED (%d)"),
			  roam_result);
		/*
		 * This event can be used to inform hdd about user triggered
		 * disassoc event
		 * Fill in the event structure
		 */
		sap_signal_hdd_event(sap_ctx, csr_roam_info,
				     eSAP_STA_DISASSOC_EVENT,
				     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_RESULT_NONE:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_NONE (%d)"),
			  roam_result);
		/*
		 * This event can be used to inform hdd about user triggered
		 * disassoc event
		 * Fill in the event structure
		 */
		if (roam_status == eCSR_ROAM_SET_KEY_COMPLETE)
			sap_signal_hdd_event(sap_ctx, csr_roam_info,
					     eSAP_STA_SET_KEY_EVENT,
					     (void *) eSAP_STATUS_SUCCESS);
		break;
	case eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("CSR roam_result = eCSR_ROAM_RESULT_MAX_ASSOC_EXCEEDED (%d)"),
			  roam_result);
		/* Fill in the event structure */
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
						  eSAP_MAX_ASSOC_EXCEEDED,
						  NULL);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;

		break;
	case eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND:
		wlansap_roam_process_dfs_radar_found(mac_ctx, sap_ctx,
						&qdf_ret_status);
		break;
	case eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS:
		wlansap_roam_process_dfs_chansw_update(hal, sap_ctx,
				&qdf_ret_status);
		break;
	case eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS:
		wlansap_roam_process_ch_change_success(mac_ctx, sap_ctx,
						csr_roam_info, &qdf_ret_status);
		break;
	case eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE:
		/*
		 * This is much more serious issue, we have to vacate the
		 * channel due to the presence of radar but our channel change
		 * failed, stop the BSS operation completely and inform hostapd
		 */
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		/* Inform cfg80211 and hostapd that BSS is not alive anymore */
		break;
	case eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND:
		qdf_status = sap_signal_hdd_event(sap_ctx, csr_roam_info,
				   eSAP_ECSA_CHANGE_CHAN_IND, NULL);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			qdf_ret_status = QDF_STATUS_E_FAILURE;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("CSR roam_result = %s (%d) not handled"),
			  get_e_csr_roam_result_str(roam_result),
			  roam_result);
		break;
	}
	wlansap_context_put(sap_ctx);
	return qdf_ret_status;
}
