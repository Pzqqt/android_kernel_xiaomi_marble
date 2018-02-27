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

/**
 * ===========================================================================
 *                     sapModule.C
 *  OVERVIEW:
 *  This software unit holds the implementation of the WLAN SAP modules
 *  functions providing EXTERNAL APIs. It is also where the global SAP module
 *  context gets initialised
 *  DEPENDENCIES:
 *  Are listed for each API below.
 * ===========================================================================
 */

/* $Header$ */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "qdf_trace.h"
#include "qdf_util.h"
#include "qdf_atomic.h"
/* Pick up the sme callback registration API */
#include "sme_api.h"

/* SAP API header file */

#include "sap_internal.h"
#include "sme_inside.h"
#include "cds_ieee80211_common_i.h"
#include "cds_regdomain.h"
#include "wlan_policy_mgr_api.h"
#include <wlan_scan_ucfg_api.h>
#include "wlan_reg_services_api.h"
#include <wlan_dfs_utils_api.h>
#include <wlan_reg_ucfg_api.h>

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
 *  External declarations for global context
 * -------------------------------------------------------------------------*/
/*  No!  Get this from CDS. */
/*  The main per-Physical Link (per WLAN association) context. */
static struct sap_context *gp_sap_ctx[SAP_MAX_NUM_SESSION];
static qdf_atomic_t sap_ctx_ref_count[SAP_MAX_NUM_SESSION];

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
static qdf_mutex_t sap_context_lock;

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/**
 * wlansap_global_init() - Initialize SAP globals
 *
 * Initializes the SAP global data structures
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_global_init(void)
{
	uint32_t i;

	if (QDF_IS_STATUS_ERROR(qdf_mutex_create(&sap_context_lock))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "failed to init sap_context_lock");
		return QDF_STATUS_E_FAULT;
	}

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		gp_sap_ctx[i] = NULL;
		qdf_atomic_init(&sap_ctx_ref_count[i]);
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			"%s: sap global context initialized", __func__);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_global_deinit() - De-initialize SAP globals
 *
 * De-initializes the SAP global data structures
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_global_deinit(void)
{
	uint32_t i;

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		if (gp_sap_ctx[i]) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				"we could be leaking context:%d", i);
		}
		gp_sap_ctx[i] = NULL;
		qdf_atomic_init(&sap_ctx_ref_count[i]);
	}

	if (QDF_IS_STATUS_ERROR(qdf_mutex_destroy(&sap_context_lock))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				"failed to destroy sap_context_lock");
		return QDF_STATUS_E_FAULT;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			"%s: sap global context deinitialized", __func__);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_save_context() - Save the context in global SAP context
 * @ctx: SAP context to be stored
 *
 * Stores the given SAP context in the global SAP context array
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlansap_save_context(struct sap_context *ctx)
{
	uint32_t i;

	qdf_mutex_acquire(&sap_context_lock);
	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		if (gp_sap_ctx[i] == NULL) {
			gp_sap_ctx[i] = ctx;
			qdf_atomic_inc(&sap_ctx_ref_count[i]);
			qdf_mutex_release(&sap_context_lock);
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
				"%s: sap context saved at index: %d",
				__func__, i);
			return QDF_STATUS_SUCCESS;
		}
	}
	qdf_mutex_release(&sap_context_lock);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		"%s: failed to save sap context", __func__);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wlansap_context_get() - Verify SAP context and increment ref count
 * @ctx: Context to be checked
 *
 * Verifies the SAP context and increments the reference count maintained for
 * the corresponding SAP context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_context_get(struct sap_context *ctx)
{
	uint32_t i;

	qdf_mutex_acquire(&sap_context_lock);
	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		if (ctx && (gp_sap_ctx[i] == ctx)) {
			qdf_atomic_inc(&sap_ctx_ref_count[i]);
			qdf_mutex_release(&sap_context_lock);
			return QDF_STATUS_SUCCESS;
		}
	}
	qdf_mutex_release(&sap_context_lock);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			"%s: sap session is not valid", __func__);
	return QDF_STATUS_E_FAILURE;
}

/**
 * wlansap_context_put() - Check the reference count and free SAP context
 * @ctx: SAP context to be checked and freed
 *
 * Checks the reference count and frees the SAP context
 *
 * Return: None
 */
void wlansap_context_put(struct sap_context *ctx)
{
	uint32_t i;

	if (!ctx)
		return;

	qdf_mutex_acquire(&sap_context_lock);
	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		if (gp_sap_ctx[i] == ctx) {
			if (qdf_atomic_dec_and_test(&sap_ctx_ref_count[i])) {
				qdf_mem_free(ctx);
				gp_sap_ctx[i] = NULL;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_DEBUG,
					"%s: sap session freed: %d",
					__func__, i);
			}
			qdf_mutex_release(&sap_context_lock);
			return;
		}
	}
	qdf_mutex_release(&sap_context_lock);
}

struct sap_context *sap_create_ctx(void)
{
	struct sap_context *sap_ctx;
	QDF_STATUS status;

	/* dynamically allocate the sapContext */
	sap_ctx = qdf_mem_malloc(sizeof(*sap_ctx));

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return NULL;
	}

	/* Clean up SAP control block, initialize all values */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Enter"));

	/* Save the SAP context pointer */
	status = wlansap_save_context(sap_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: failed to save SAP context", __func__);
		qdf_mem_free(sap_ctx);
		return NULL;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Exit"));

	return sap_ctx;
} /* sap_create_ctx */

QDF_STATUS sap_init_ctx(struct sap_context *sap_ctx,
			 enum QDF_OPMODE mode,
			 uint8_t *addr, uint32_t session_id)
{
	QDF_STATUS qdf_ret_status;
	tHalHandle hal;
	tpAniSirGlobal pmac;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_start invoked successfully");

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/*------------------------------------------------------------------------
	    For now, presume security is not enabled.
	   -----------------------------------------------------------------------*/
	sap_ctx->ucSecEnabled = WLANSAP_SECURITY_ENABLED_STATE;

	/*------------------------------------------------------------------------
	    Now configure the roaming profile links. To SSID and bssid.
	   ------------------------------------------------------------------------*/
	/* We have room for two SSIDs. */
	sap_ctx->csr_roamProfile.SSIDs.numOfSSIDs = 1;   /* This is true for now. */
	sap_ctx->csr_roamProfile.SSIDs.SSIDList = sap_ctx->SSIDList;     /* Array of two */
	sap_ctx->csr_roamProfile.SSIDs.SSIDList[0].SSID.length = 0;
	sap_ctx->csr_roamProfile.SSIDs.SSIDList[0].handoffPermitted = false;
	sap_ctx->csr_roamProfile.SSIDs.SSIDList[0].ssidHidden =
		sap_ctx->SSIDList[0].ssidHidden;

	sap_ctx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1; /* This is true for now. */
	sap_ctx->csr_roamProfile.BSSIDs.bssid = &sap_ctx->bssid;
	sap_ctx->csr_roamProfile.csrPersona = mode;
	qdf_mem_copy(sap_ctx->self_mac_addr, addr, QDF_MAC_ADDR_SIZE);

	/* Now configure the auth type in the roaming profile. To open. */
	sap_ctx->csr_roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;        /* open is the default */

	hal = (tHalHandle) CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid HAL pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}
	pmac = PMAC_STRUCT(hal);
	qdf_ret_status = sap_set_session_param(hal, sap_ctx, session_id);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"In %s calling sap_set_session_param status = %d",
			__func__, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}
	/* Register with scan component */
	sap_ctx->req_id = ucfg_scan_register_requester(pmac->psoc, "SAP",
					sap_scan_event_callback, sap_ctx);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sap_deinit_ctx(struct sap_context *sap_ctx)
{
	tHalHandle hal;
	tpAniSirGlobal pmac;

	/* Sanity check - Extract SAP control block */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  "wlansap_stop invoked successfully ");

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	hal = CDS_GET_HAL_CB();
	pmac = (tpAniSirGlobal) hal;
	if (NULL == pmac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid MAC context", __func__);
		return QDF_STATUS_E_FAULT;
	}
	ucfg_scan_unregister_requester(pmac->psoc, sap_ctx->req_id);
	sap_free_roam_profile(&sap_ctx->csr_roamProfile);
	if (sap_ctx->sessionId != CSR_SESSION_ID_INVALID) {
		/* empty queues/lists/pkts if any */
		sap_clear_session_param(hal, sap_ctx, sap_ctx->sessionId);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sap_destroy_ctx(struct sap_context *sap_ctx)
{
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "sap_destroy_ctx invoked");

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	/* Cleanup SAP control block */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Enter"));
	/*
	 * wlansap_context_put will release actual sap_ctx memory
	 * allocated during sap_create_ctx
	 */
	wlansap_context_put(sap_ctx);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Exit"));

	return QDF_STATUS_SUCCESS;
} /* sap_destroy_ctx */

bool wlansap_is_channel_in_nol_list(struct sap_context *sap_ctx,
				    uint8_t channelNumber,
				    ePhyChanBondState chanBondState)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	return sap_dfs_is_channel_in_nol_list(sap_ctx, channelNumber,
					      chanBondState);
}

static QDF_STATUS wlansap_mark_leaking_channel(struct wlan_objmgr_pdev *pdev,
		uint8_t *leakage_adjusted_lst,
		uint8_t chan_bw)
{

	return utils_dfs_mark_leaking_ch(pdev, chan_bw, 1,
			leakage_adjusted_lst);
}

bool wlansap_is_channel_leaking_in_nol(struct sap_context *sap_ctx,
				       uint8_t channel,
				       uint8_t chan_bw)
{
	tpAniSirGlobal mac_ctx;
	uint8_t leakage_adjusted_lst[1];
	void *handle = NULL;

	leakage_adjusted_lst[0] = channel;
	handle = CDS_GET_HAL_CB();
	mac_ctx = PMAC_STRUCT(handle);
	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid mac pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	if (QDF_IS_STATUS_ERROR(wlansap_mark_leaking_channel(mac_ctx->pdev,
			leakage_adjusted_lst, chan_bw)))
		return true;

	if (!leakage_adjusted_lst[0])
		return true;

	return false;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t wlansap_check_cc_intf(struct sap_context *sap_ctx)
{
	tHalHandle hHal;
	uint16_t intf_ch;

	hHal = (tHalHandle) CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid MAC context from p_cds_gctx", __func__);
		return 0;
	}
	intf_ch = sme_check_concurrent_channel_overlap(hHal, sap_ctx->channel,
					     sap_ctx->csr_roamProfile.phyMode,
						       sap_ctx->cc_switch_mode);
	return intf_ch;
}
#endif

 /**
  * wlansap_set_scan_acs_channel_params() - Config scan and channel parameters.
  * pconfig:                                Pointer to the SAP config
  * psap_ctx:                               Pointer to the SAP Context.
  * pusr_context:                           Parameter that will be passed
  *                                         back in all the SAP callback events.
  *
  * This api function is used to copy Scan and Channel parameters from sap
  * config to sap context.
  *
  * Return:                                 The result code associated with
  *                                         performing the operation
  */
static QDF_STATUS
wlansap_set_scan_acs_channel_params(tsap_Config_t *pconfig,
				struct sap_context *psap_ctx,
				void *pusr_context)
{
	tHalHandle h_hal = NULL;
	tpAniSirGlobal pmac;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pconfig) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid pconfig passed ", __func__);
		return QDF_STATUS_E_FAULT;
	}

	if (NULL == psap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid pconfig passed ", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* Channel selection is auto or configured */
	psap_ctx->channel = pconfig->channel;
	psap_ctx->dfs_mode = pconfig->acs_dfs_mode;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	psap_ctx->cc_switch_mode = pconfig->cc_switch_mode;
#endif
	psap_ctx->auto_channel_select_weight =
		 pconfig->auto_channel_select_weight;
	psap_ctx->pUsrContext = pusr_context;
	psap_ctx->enableOverLapCh = pconfig->enOverLapCh;
	psap_ctx->acs_cfg = &pconfig->acs_cfg;
	psap_ctx->ch_width_orig = pconfig->acs_cfg.ch_width;
	psap_ctx->secondary_ch = pconfig->sec_ch;

	/*
	 * Set the BSSID to your "self MAC Addr" read
	 * the mac address from Configuation ITEM received
	 * from HDD
	 */
	psap_ctx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1;

	/* Save a copy to SAP context */
	qdf_mem_copy(psap_ctx->csr_roamProfile.BSSIDs.bssid,
		pconfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(psap_ctx->self_mac_addr,
		pconfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);

	h_hal = (tHalHandle)CDS_GET_HAL_CB();
	if (NULL == h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s: Invalid MAC context from pvosGCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pmac = PMAC_STRUCT(h_hal);
	/*
	 * If concurrent session is running that is already associated
	 * then we just follow that sessions country info (whether
	 * present or not doesn't maater as we have to follow whatever
	 * STA session does)
	 */
	if ((0 == sme_get_concurrent_operation_channel(h_hal)) &&
			pconfig->ieee80211d) {
		/* Setting the region/country  information */
		status = ucfg_reg_set_country(pmac->pdev,
					pconfig->countryCode);
		if (QDF_IS_STATUS_ERROR(status))
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to set country"));
	}

	return status;
}

/**
 * wlan_sap_get_roam_profile() - Returns sap roam profile.
 * @sap_ctx:	Pointer to Sap Context.
 *
 * This function provides the SAP roam profile.
 *
 * Return: SAP RoamProfile
 */
tCsrRoamProfile *wlan_sap_get_roam_profile(struct sap_context *sap_ctx)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from ctx"));
		return NULL;
	}
	return &sap_ctx->csr_roamProfile;
}

eCsrPhyMode wlan_sap_get_phymode(struct sap_context *sap_ctx)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from ctx"));
		return 0;
	}
	return sap_ctx->csr_roamProfile.phyMode;
}

uint32_t wlan_sap_get_vht_ch_width(struct sap_context *sap_ctx)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer"));
		return 0;
	}

	return sap_ctx->ch_params.ch_width;
}

void wlan_sap_set_vht_ch_width(struct sap_context *sap_ctx,
			       uint32_t vht_channel_width)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer"));
		return;
	}

	sap_ctx->ch_params.ch_width = vht_channel_width;
}

/**
 * wlan_sap_validate_channel_switch() - validate target channel switch w.r.t
 *      concurreny rules set to avoid channel interference.
 * @hal - Hal context
 * @sap_ch - channel to switch
 * @sap_context - sap session context
 *
 * Return: true if there is no channel interference else return false
 */
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
static bool wlan_sap_validate_channel_switch(tHalHandle hal, uint16_t sap_ch,
		struct sap_context *sap_context)
{
	return sme_validate_sap_channel_switch(
			hal,
			sap_ch,
			sap_context->csr_roamProfile.phyMode,
			sap_context->cc_switch_mode,
			sap_context->sessionId);
}
#else
static inline bool wlan_sap_validate_channel_switch(tHalHandle hal,
		uint16_t sap_ch, struct sap_context *sap_context)
{
	return true;
}
#endif
QDF_STATUS wlansap_start_bss(struct sap_context *sap_ctx,
			     tpWLAN_SAPEventCB pSapEventCallback,
			     tsap_Config_t *pConfig, void *pUsrContext)
{
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tHalHandle hHal;
	tpAniSirGlobal pmac = NULL;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_start_bss: sapContext=%pK", sap_ctx);

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	sap_ctx->sapsMachine = eSAP_DISCONNECTED;

	/* Channel selection is auto or configured */
	sap_ctx->channel = pConfig->channel;
	sap_ctx->dfs_mode = pConfig->acs_dfs_mode;
	sap_ctx->ch_params.ch_width = pConfig->ch_params.ch_width;
	sap_ctx->ch_params.center_freq_seg0 =
		pConfig->ch_params.center_freq_seg0;
	sap_ctx->ch_params.center_freq_seg1 =
		pConfig->ch_params.center_freq_seg1;
	sap_ctx->ch_params.sec_ch_offset =
		pConfig->ch_params.sec_ch_offset;
	sap_ctx->ch_width_orig = pConfig->ch_width_orig;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	sap_ctx->cc_switch_mode = pConfig->cc_switch_mode;
#endif
	sap_ctx->auto_channel_select_weight =
		 pConfig->auto_channel_select_weight;
	sap_ctx->pUsrContext = pUsrContext;
	sap_ctx->enableOverLapCh = pConfig->enOverLapCh;
	sap_ctx->acs_cfg = &pConfig->acs_cfg;
	sap_ctx->dfs_cac_offload = pConfig->dfs_cac_offload;
	sap_ctx->isCacEndNotified = false;
	sap_ctx->is_chan_change_inprogress = false;
	sap_ctx->stop_bss_in_progress = false;
	/* Set the BSSID to your "self MAC Addr" read the mac address
		from Configuation ITEM received from HDD */
	sap_ctx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(sap_ctx->csr_roamProfile.BSSIDs.bssid,
		     sap_ctx->self_mac_addr, sizeof(struct qdf_mac_addr));

	/* Save a copy to SAP context */
	qdf_mem_copy(sap_ctx->csr_roamProfile.BSSIDs.bssid,
		     pConfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(sap_ctx->self_mac_addr,
		     pConfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);

	/* copy the configuration items to csrProfile */
	sapconvert_to_csr_profile(pConfig, eCSR_BSS_TYPE_INFRA_AP,
			       &sap_ctx->csr_roamProfile);
	hHal = (tHalHandle) CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid MAC context from p_cds_gctx",
			  __func__);
		qdf_status = QDF_STATUS_E_FAULT;
		goto fail;
	}
	pmac = PMAC_STRUCT(hHal);
	if (NULL == pmac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid MAC context from p_cds_gctx",
			  __func__);
		qdf_status = QDF_STATUS_E_FAULT;
		goto fail;
	}
	/* If concurrent session is running that is already associated
	 * then we just follow that sessions country info (whether
	 * present or not doesn't maater as we have to follow whatever
	 * STA session does) */
	if ((0 == sme_get_concurrent_operation_channel(hHal)) &&
			pConfig->ieee80211d) {
		/* Setting the region/country  information */
		qdf_status = ucfg_reg_set_country(pmac->pdev,
					pConfig->countryCode);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to set country"));
	}

	/*
	 * Copy the DFS Test Mode setting to pmac for
	 * access in lower layers
	 */
	pmac->sap.SapDfsInfo.disable_dfs_ch_switch =
				pConfig->disableDFSChSwitch;
	pmac->sap.SapDfsInfo.sap_ch_switch_beacon_cnt =
				pConfig->sap_chanswitch_beacon_cnt;
	pmac->sap.SapDfsInfo.sap_ch_switch_mode =
			pConfig->sap_chanswitch_mode;

	pmac->sap.sapCtxList[sap_ctx->sessionId].pSapContext = sap_ctx;
	pmac->sap.sapCtxList[sap_ctx->sessionId].sapPersona =
		sap_ctx->csr_roamProfile.csrPersona;
	pmac->sap.sapCtxList[sap_ctx->sessionId].sessionID =
		sap_ctx->sessionId;
	pmac->sap.SapDfsInfo.dfs_beacon_tx_enhanced =
		pConfig->dfs_beacon_tx_enhanced;
	pmac->sap.SapDfsInfo.reduced_beacon_interval =
				pConfig->reduced_beacon_interval;

	/* Copy MAC filtering settings to sap context */
	sap_ctx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;
	qdf_mem_copy(sap_ctx->acceptMacList, pConfig->accept_mac,
		     sizeof(pConfig->accept_mac));
	sap_ctx->nAcceptMac = pConfig->num_accept_mac;
	sap_sort_mac_list(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	qdf_mem_copy(sap_ctx->denyMacList, pConfig->deny_mac,
		     sizeof(pConfig->deny_mac));
	sap_ctx->nDenyMac = pConfig->num_deny_mac;
	sap_sort_mac_list(sap_ctx->denyMacList, sap_ctx->nDenyMac);
	sap_ctx->beacon_tx_rate = pConfig->beacon_tx_rate;

	/* Fill in the event structure for FSM */
	sapEvent.event = eSAP_HDD_START_INFRA_BSS;
	sapEvent.params = 0;    /* pSapPhysLinkCreate */

	/* Store the HDD callback in SAP context */
	sap_ctx->pfnSapEventCallback = pSapEventCallback;

	/* Handle event */
	qdf_status = sap_fsm(sap_ctx, &sapEvent);
fail:
	if (QDF_IS_STATUS_ERROR(qdf_status))
		sap_free_roam_profile(&sap_ctx->csr_roamProfile);

	return qdf_status;
} /* wlansap_start_bss */

QDF_STATUS wlansap_set_mac_acl(struct sap_context *sap_ctx,
			       tsap_Config_t *pConfig)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_set_mac_acl");

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	/* Copy MAC filtering settings to sap context */
	sap_ctx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;

	if (eSAP_DENY_UNLESS_ACCEPTED == sap_ctx->eSapMacAddrAclMode) {
		qdf_mem_copy(sap_ctx->acceptMacList,
			     pConfig->accept_mac,
			     sizeof(pConfig->accept_mac));
		sap_ctx->nAcceptMac = pConfig->num_accept_mac;
		sap_sort_mac_list(sap_ctx->acceptMacList,
			       sap_ctx->nAcceptMac);
	} else if (eSAP_ACCEPT_UNLESS_DENIED == sap_ctx->eSapMacAddrAclMode) {
		qdf_mem_copy(sap_ctx->denyMacList, pConfig->deny_mac,
			     sizeof(pConfig->deny_mac));
		sap_ctx->nDenyMac = pConfig->num_deny_mac;
		sap_sort_mac_list(sap_ctx->denyMacList, sap_ctx->nDenyMac);
	}

	return qdf_status;
} /* wlansap_set_mac_acl */

void wlansap_set_stop_bss_inprogress(struct sap_context *sap_ctx,
					bool in_progress)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from ctx", __func__);
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Set stop_bss_in_progress to %d",
		  __func__, in_progress);
	sap_ctx->stop_bss_in_progress = in_progress;
}

QDF_STATUS wlansap_stop_bss(struct sap_context *sap_ctx)
{
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	QDF_STATUS qdf_status;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* Fill in the event structure for FSM */
	sapEvent.event = eSAP_HDD_STOP_INFRA_BSS;
	sapEvent.params = 0;

	/* Handle event */
	qdf_status = sap_fsm(sap_ctx, &sapEvent);

	return qdf_status;
}

/* This routine will set the mode of operation for ACL dynamically*/
QDF_STATUS wlansap_set_acl_mode(struct sap_context *sap_ctx,
				eSapMacAddrACL mode)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->eSapMacAddrAclMode = mode;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_get_acl_mode(struct sap_context *sap_ctx,
				eSapMacAddrACL *mode)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	*mode = sap_ctx->eSapMacAddrAclMode;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_get_acl_accept_list(struct sap_context *sap_ctx,
				       struct qdf_mac_addr *pAcceptList,
				       uint8_t *nAcceptList)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	memcpy(pAcceptList, sap_ctx->acceptMacList,
	       (sap_ctx->nAcceptMac * QDF_MAC_ADDR_SIZE));
	*nAcceptList = sap_ctx->nAcceptMac;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_get_acl_deny_list(struct sap_context *sap_ctx,
				     struct qdf_mac_addr *pDenyList,
				     uint8_t *nDenyList)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	memcpy(pDenyList, sap_ctx->denyMacList,
	       (sap_ctx->nDenyMac * QDF_MAC_ADDR_SIZE));
	*nDenyList = sap_ctx->nDenyMac;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_clear_acl(struct sap_context *sap_ctx)
{
	uint8_t i;

	if (NULL == sap_ctx) {
		return QDF_STATUS_E_RESOURCES;
	}

	for (i = 0; i < (sap_ctx->nDenyMac - 1); i++) {
		qdf_mem_zero((sap_ctx->denyMacList + i)->bytes,
			     QDF_MAC_ADDR_SIZE);
	}

	sap_print_acl(sap_ctx->denyMacList, sap_ctx->nDenyMac);
	sap_ctx->nDenyMac = 0;

	for (i = 0; i < (sap_ctx->nAcceptMac - 1); i++) {
		qdf_mem_zero((sap_ctx->acceptMacList + i)->bytes,
			     QDF_MAC_ADDR_SIZE);
	}

	sap_print_acl(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	sap_ctx->nAcceptMac = 0;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_modify_acl(struct sap_context *sap_ctx,
			      uint8_t *peer_sta_mac,
			      eSapACLType list_type, eSapACLCmdType cmd)
{
	bool sta_white_list = false, sta_black_list = false;
	uint8_t staWLIndex, staBLIndex;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP Context", __func__);
		return QDF_STATUS_E_FAULT;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Modify ACL entered\n" "Before modification of ACL\n"
		  "size of accept and deny lists %d %d", sap_ctx->nAcceptMac,
		  sap_ctx->nDenyMac);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "*** WHITE LIST ***");
	sap_print_acl(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "*** BLACK LIST ***");
	sap_print_acl(sap_ctx->denyMacList, sap_ctx->nDenyMac);

	/* the expectation is a mac addr will not be in both the lists
	 * at the same time. It is the responsiblity of userspace to
	 * ensure this
	 */
	sta_white_list =
		sap_search_mac_list(sap_ctx->acceptMacList, sap_ctx->nAcceptMac,
				 peer_sta_mac, &staWLIndex);
	sta_black_list =
		sap_search_mac_list(sap_ctx->denyMacList, sap_ctx->nDenyMac,
				 peer_sta_mac, &staBLIndex);

	if (sta_white_list && sta_black_list) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Peer mac " MAC_ADDRESS_STR
			  " found in white and black lists."
			  "Initial lists passed incorrect. Cannot execute this command.",
			  MAC_ADDR_ARRAY(peer_sta_mac));
		return QDF_STATUS_E_FAILURE;

	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  "cmd %d", cmd);

	switch (list_type) {
	case eSAP_WHITE_LIST:
		if (cmd == ADD_STA_TO_ACL) {
			/* error check */
			/* if list is already at max, return failure */
			if (sap_ctx->nAcceptMac == MAX_ACL_MAC_ADDRESS) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "White list is already maxed out. Cannot accept "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_E_FAILURE;
			}
			if (sta_white_list) {
				/* Do nothing if already present in white list. Just print a warning */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "MAC address already present in white list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_SUCCESS;
			}
			if (sta_black_list) {
				/* remove it from black list before adding to the white list */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "STA present in black list so first remove from it");
				sap_remove_mac_from_acl(sap_ctx->denyMacList,
						    &sap_ctx->nDenyMac,
						    staBLIndex);
			}
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO,
				  "... Now add to the white list");
			sap_add_mac_to_acl(sap_ctx->acceptMacList,
					       &sap_ctx->nAcceptMac,
			       peer_sta_mac);
				QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO_LOW,
				  "size of accept and deny lists %d %d",
				  sap_ctx->nAcceptMac,
				  sap_ctx->nDenyMac);
		} else if (cmd == DELETE_STA_FROM_ACL) {
			if (sta_white_list) {

				struct csr_del_sta_params delStaParams;

				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO,
					  "Delete from white list");
				sap_remove_mac_from_acl(sap_ctx->acceptMacList,
						    &sap_ctx->nAcceptMac,
						    staWLIndex);
				/* If a client is deleted from white list and it is connected, send deauth */
				wlansap_populate_del_sta_params(peer_sta_mac,
					eCsrForcedDeauthSta,
					(SIR_MAC_MGMT_DEAUTH >> 4),
					&delStaParams);
				wlansap_deauth_sta(sap_ctx, &delStaParams);
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  "size of accept and deny lists %d %d",
					  sap_ctx->nAcceptMac,
					  sap_ctx->nDenyMac);
			} else {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "MAC address to be deleted is not present in the white list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "Invalid cmd type passed");
			return QDF_STATUS_E_FAILURE;
		}
		break;

	case eSAP_BLACK_LIST:

		if (cmd == ADD_STA_TO_ACL) {
			struct csr_del_sta_params delStaParams;
			/* error check */
			/* if list is already at max, return failure */
			if (sap_ctx->nDenyMac == MAX_ACL_MAC_ADDRESS) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "Black list is already maxed out. Cannot accept "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_E_FAILURE;
			}
			if (sta_black_list) {
				/* Do nothing if already present in white list */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "MAC address already present in black list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_SUCCESS;
			}
			if (sta_white_list) {
				/* remove it from white list before adding to the black list */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "Present in white list so first remove from it");
				sap_remove_mac_from_acl(sap_ctx->acceptMacList,
						    &sap_ctx->nAcceptMac,
						    staWLIndex);
			}
			/* If we are adding a client to the black list; if its connected, send deauth */
			wlansap_populate_del_sta_params(peer_sta_mac,
				eCsrForcedDeauthSta,
				(SIR_MAC_MGMT_DEAUTH >> 4),
				&delStaParams);
			wlansap_deauth_sta(sap_ctx, &delStaParams);
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO,
				  "... Now add to black list");
			sap_add_mac_to_acl(sap_ctx->denyMacList,
				       &sap_ctx->nDenyMac, peer_sta_mac);
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO_LOW,
				  "size of accept and deny lists %d %d",
				  sap_ctx->nAcceptMac,
				  sap_ctx->nDenyMac);
		} else if (cmd == DELETE_STA_FROM_ACL) {
			if (sta_black_list) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO,
					  "Delete from black list");
				sap_remove_mac_from_acl(sap_ctx->denyMacList,
						    &sap_ctx->nDenyMac,
						    staBLIndex);
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  "no accept and deny mac %d %d",
					  sap_ctx->nAcceptMac,
					  sap_ctx->nDenyMac);
			} else {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_WARN,
					  "MAC address to be deleted is not present in the black list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "Invalid cmd type passed");
			return QDF_STATUS_E_FAILURE;
		}
		break;

	default:
	{
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid list type passed %d", list_type);
		return QDF_STATUS_E_FAILURE;
	}
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  "After modification of ACL");
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "*** WHITE LIST ***");
	sap_print_acl(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "*** BLACK LIST ***");
	sap_print_acl(sap_ctx->denyMacList, sap_ctx->nDenyMac);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_disassoc_sta(struct sap_context *sap_ctx,
				struct csr_del_sta_params *p_del_sta_params)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_disconnect_sta(CDS_GET_HAL_CB(),
				sap_ctx->sessionId, p_del_sta_params);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_deauth_sta(struct sap_context *sap_ctx,
			      struct csr_del_sta_params *pDelStaParams)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return qdf_status;
	}

	qdf_ret_status =
		sme_roam_deauth_sta(CDS_GET_HAL_CB(),
				    sap_ctx->sessionId, pDelStaParams);

	if (qdf_ret_status == QDF_STATUS_SUCCESS) {
		qdf_status = QDF_STATUS_SUCCESS;
	}
	return qdf_status;
}

/**
 * wlansap_update_csa_channel_params() - fucntion to populate channel width and
 *                                        bonding modes.
 * @sap_context: sap adapter context
 * @channel: target channel
 *
 * Return: The QDF_STATUS code associated with performing the operation
 */
static QDF_STATUS
wlansap_update_csa_channel_params(struct sap_context *sap_context,
				  uint32_t channel)
{
	void *hal;
	tpAniSirGlobal mac_ctx;
	uint8_t bw;

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid hal pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	mac_ctx = PMAC_STRUCT(hal);

	if (channel <= CHAN_ENUM_14) {
		/*
		 * currently OBSS scan is done in hostapd, so to avoid
		 * SAP coming up in HT40 on channel switch we are
		 * disabling channel bonding in 2.4Ghz.
		 */
		mac_ctx->sap.SapDfsInfo.new_chanWidth = 0;

	} else {

		if (sap_context->ch_width_orig >= CH_WIDTH_80MHZ)
			bw = BW80;
		else if (sap_context->ch_width_orig == CH_WIDTH_40MHZ)
			bw = BW40_HIGH_PRIMARY;
		else
			bw = BW20;

		for (; bw >= BW20; bw--) {
			uint16_t op_class;

			op_class = wlan_reg_dmn_get_opclass_from_channel(
					mac_ctx->scan.countryCodeCurrent,
					channel, bw);
			if (!op_class)
				continue;

			if (bw == BW80) {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
					CH_WIDTH_80MHZ;
			} else if (bw == BW40_HIGH_PRIMARY) {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
					CH_WIDTH_40MHZ;
			} else if (bw == BW40_LOW_PRIMARY) {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
				   CH_WIDTH_40MHZ;
			} else {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
				   CH_WIDTH_20MHZ;
			}
			break;
		}

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_set_channel_change_with_csa() - Set channel change with CSA
 * @sapContext: Pointer to SAP context
 * @targetChannel: Target channel
 * @target_bw: Target bandwidth
 * @strict: if true switch to the requested channel always,
 *        SCC/MCC check will be ignored,
 *        fail otherwise
 *
 * This api function does a channel change to the target channel specified.
 * CSA IE is included in the beacons before doing a channel change.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_set_channel_change_with_csa(struct sap_context *sapContext,
					       uint32_t targetChannel,
					       enum phy_ch_width target_bw,
					       bool strict)
{

	tWLAN_SAPEvent sapEvent;
	tpAniSirGlobal pMac = NULL;
	void *hHal = NULL;
	bool valid;
	QDF_STATUS status;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);

		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	if (strict && !policy_mgr_is_safe_channel(pMac->psoc, targetChannel)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%u is unsafe channel", targetChannel);
		return QDF_STATUS_E_FAULT;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		"%s: sap chan:%d target:%d conn on 5GHz:%d",
		__func__, sapContext->channel, targetChannel,
		policy_mgr_is_any_mode_active_on_band_along_with_session(
			pMac->psoc, sapContext->sessionId, POLICY_MGR_BAND_5));

	/*
	 * Now, validate if the passed channel is valid in the
	 * current regulatory domain.
	 */
	if (sapContext->channel != targetChannel &&
		((wlan_reg_get_channel_state(pMac->pdev, targetChannel) ==
			CHANNEL_STATE_ENABLE) ||
		(wlan_reg_get_channel_state(pMac->pdev, targetChannel) ==
			CHANNEL_STATE_DFS &&
		!policy_mgr_is_any_mode_active_on_band_along_with_session(
			pMac->psoc, sapContext->sessionId,
			POLICY_MGR_BAND_5)))) {
		/*
		 * validate target channel switch w.r.t various concurrency
		 * rules set.
		 */
		if (!strict) {
			valid = wlan_sap_validate_channel_switch(hHal,
				targetChannel, sapContext);
			if (!valid) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Channel switch to %u is not allowed due to concurrent channel interference"),
					  targetChannel);
				return QDF_STATUS_E_FAULT;
			}
		}
		/*
		 * Post a CSA IE request to SAP state machine with
		 * target channel information and also CSA IE required
		 * flag set in sapContext only, if SAP is in eSAP_STARTED
		 * state.
		 */
		if (eSAP_STARTED == sapContext->sapsMachine) {
			status = wlansap_update_csa_channel_params(sapContext,
					targetChannel);
			if (status != QDF_STATUS_SUCCESS)
				return status;

			/*
			 * Copy the requested target channel
			 * to sap context.
			 */
			pMac->sap.SapDfsInfo.target_channel = targetChannel;
			pMac->sap.SapDfsInfo.new_ch_params.ch_width =
				pMac->sap.SapDfsInfo.new_chanWidth;

			/* By this time, the best bandwidth is calculated for
			 * the given target channel. Now, if there was a
			 * request from user to move to a selected bandwidth,
			 * we can see if it can be honored.
			 *
			 * Ex1: BW80 was selected for the target channel and
			 * user wants BW40, it can be allowed
			 * Ex2: BW40 was selected for the target channel and
			 * user wants BW80, it cannot be allowed for the given
			 * target channel.
			 *
			 * So, the MIN of the selected channel bandwidth and
			 * user input is used for the bandwidth
			 */
			if (target_bw != CH_WIDTH_MAX) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_INFO,
					"%s: target bw:%d new width:%d",
					__func__, target_bw,
					pMac->sap.SapDfsInfo.
					new_ch_params.ch_width);
				pMac->sap.SapDfsInfo.new_ch_params.ch_width =
					pMac->sap.SapDfsInfo.new_chanWidth =
					QDF_MIN(pMac->sap.SapDfsInfo.
							new_ch_params.ch_width,
							target_bw);
			}
			wlan_reg_set_channel_params(pMac->pdev, targetChannel,
				0, &pMac->sap.SapDfsInfo.new_ch_params);
			/*
			 * Set the CSA IE required flag.
			 */
			pMac->sap.SapDfsInfo.csaIERequired = true;

			/*
			 * Set the radar found status to allow the channel
			 * change to happen same as in the case of a radar
			 * detection. Since, this will allow SAP to be in
			 * correct state and also resume the netif queues
			 * that were suspended in HDD before the channel
			 * request was issued.
			 */
			pMac->sap.SapDfsInfo.sap_radar_found_status = true;
			pMac->sap.SapDfsInfo.cac_state =
					eSAP_DFS_DO_NOT_SKIP_CAC;
			sap_cac_reset_notify(hHal);

			/*
			 * Post the eSAP_CHANNEL_SWITCH_ANNOUNCEMENT_START
			 * to SAP state machine to process the channel
			 * request with CSA IE set in the beacons.
			 */
			sapEvent.event =
				eSAP_CHANNEL_SWITCH_ANNOUNCEMENT_START;
			sapEvent.params = 0;
			sapEvent.u1 = 0;
			sapEvent.u2 = 0;

			sap_fsm(sapContext, &sapEvent);

		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "%s: Failed to request Channel Change, since"
				  "SAP is not in eSAP_STARTED state", __func__);
			return QDF_STATUS_E_FAULT;
		}

	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Channel = %d is not valid in the current"
			  "regulatory domain", __func__, targetChannel);

		return QDF_STATUS_E_FAULT;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Posted eSAP_CHANNEL_SWITCH_ANNOUNCEMENT_START successfully to sap_fsm for Channel = %d",
		  __func__, targetChannel);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_set_key_sta(struct sap_context *sap_ctx,
			       tCsrRoamSetKey *pSetKeyInfo)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	uint32_t roamId = INVALID_ROAM_ID;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	qdf_ret_status =
		sme_roam_set_key(hHal, sap_ctx->sessionId, pSetKeyInfo,
				 &roamId);

	if (qdf_ret_status == QDF_STATUS_SUCCESS)
		qdf_status = QDF_STATUS_SUCCESS;
	else
		qdf_status = QDF_STATUS_E_FAULT;

	return qdf_status;
}

QDF_STATUS wlan_sap_getstation_ie_information(struct sap_context *sap_ctx,
					      uint32_t *len, uint8_t *buf)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint32_t ie_len = 0;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer"));
		return QDF_STATUS_E_FAULT;
	}

	if (len) {
		ie_len = *len;
		*len = sap_ctx->nStaWPARSnReqIeLength;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			FL("WPAIE len : %x"), *len);
		if ((buf) && (ie_len >= sap_ctx->nStaWPARSnReqIeLength)) {
			qdf_mem_copy(buf,
				sap_ctx->pStaWpaRsnReqIE,
				sap_ctx->nStaWPARSnReqIeLength);
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				FL("WPAIE: %02x:%02x:%02x:%02x:%02x:%02x"),
				buf[0], buf[1], buf[2], buf[3], buf[4],
				buf[5]);
			qdf_status = QDF_STATUS_SUCCESS;
		}
	}
	return qdf_status;
}

QDF_STATUS wlan_sap_update_next_channel(struct sap_context *sap_ctx,
					uint8_t channel,
					enum phy_ch_width chan_bw)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->dfs_vendor_channel = channel;
	sap_ctx->dfs_vendor_chan_bw = chan_bw;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_sap_set_pre_cac_status(struct sap_context *sap_ctx,
				       bool status, tHalHandle handle)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(handle);

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid mac pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->is_pre_cac_on = status;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		"%s: is_pre_cac_on:%d", __func__, sap_ctx->is_pre_cac_on);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_sap_set_chan_before_pre_cac(struct sap_context *sap_ctx,
					    uint8_t chan_before_pre_cac)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->chan_before_pre_cac = chan_before_pre_cac;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_sap_set_pre_cac_complete_status(struct sap_context *sap_ctx,
						bool status)
{
	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->pre_cac_complete = status;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			"%s: pre cac complete status:%d session:%d",
			__func__, status, sap_ctx->sessionId);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_sap_is_pre_cac_active() - Checks if pre cac in in progress
 * @handle: Global MAC handle
 *
 * Checks if pre cac is in progress in any of the SAP contexts
 *
 * Return: True is pre cac is active, false otherwise
 */
bool wlan_sap_is_pre_cac_active(tHalHandle handle)
{
	tpAniSirGlobal mac = NULL;
	int i;

	mac = PMAC_STRUCT(handle);
	if (!mac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s: Invalid mac context", __func__);
		return false;
	}

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		struct sap_context *context =
			mac->sap.sapCtxList[i].pSapContext;
		if (context && context->is_pre_cac_on)
			return true;
	}
	return false;
}

/**
 * wlan_sap_get_pre_cac_vdev_id() - Get vdev id of the pre cac interface
 * @handle: Global handle
 * @vdev_id: vdev id
 *
 * Fetches the vdev id of the pre cac interface
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_get_pre_cac_vdev_id(tHalHandle handle, uint8_t *vdev_id)
{
	tpAniSirGlobal mac = NULL;
	uint8_t i;

	mac = PMAC_STRUCT(handle);
	if (!mac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			"%s: Invalid mac context", __func__);
		return QDF_STATUS_E_FAULT;
	}

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		struct sap_context *context =
			mac->sap.sapCtxList[i].pSapContext;
		if (context && context->is_pre_cac_on) {
			*vdev_id = i;
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlansap_register_mgmt_frame(struct sap_context *sap_ctx,
				       uint16_t frameType,
				       uint8_t *matchData,
				       uint16_t matchLen)
{
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal pointer null"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status = sme_register_mgmt_frame(hHal, sap_ctx->sessionId,
						 frameType, matchData,
						 matchLen);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Register MGMT frame");

	return QDF_STATUS_E_FAULT;
}

QDF_STATUS wlansap_de_register_mgmt_frame(struct sap_context *sap_ctx,
					  uint16_t frameType,
					  uint8_t *matchData,
					  uint16_t matchLen)
{
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal pointer null"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status =
		sme_deregister_mgmt_frame(hHal, sap_ctx->sessionId, frameType,
					  matchData, matchLen);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Deregister MGMT frame");

	return QDF_STATUS_E_FAULT;
}

QDF_STATUS wlansap_channel_change_request(struct sap_context *sapContext,
					  uint8_t target_channel)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal mac_ctx = NULL;
	eCsrPhyMode phy_mode;
	struct ch_params *ch_params;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	mac_ctx = PMAC_STRUCT(hHal);
	phy_mode = sapContext->csr_roamProfile.phyMode;

	if (sapContext->csr_roamProfile.ChannelInfo.numOfChannels == 0 ||
	    sapContext->csr_roamProfile.ChannelInfo.ChannelList == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid channel list"));
		return QDF_STATUS_E_FAULT;
	}
	sapContext->csr_roamProfile.ChannelInfo.ChannelList[0] = target_channel;
	/*
	 * We are getting channel bonding mode from sapDfsInfor structure
	 * because we've implemented channel width fallback mechanism for DFS
	 * which will result in channel width changing dynamically.
	 */
	ch_params = &mac_ctx->sap.SapDfsInfo.new_ch_params;
	wlan_reg_set_channel_params(mac_ctx->pdev, target_channel,
			0, ch_params);
	sapContext->ch_params.ch_width = ch_params->ch_width;
	/* Update the channel as this will be used to
	 * send event to supplicant
	 */
	sapContext->channel = target_channel;
	sapContext->csr_roamProfile.ch_params.ch_width = ch_params->ch_width;
	sapContext->csr_roamProfile.ch_params.sec_ch_offset =
						ch_params->sec_ch_offset;
	sapContext->csr_roamProfile.ch_params.center_freq_seg0 =
						ch_params->center_freq_seg0;
	sapContext->csr_roamProfile.ch_params.center_freq_seg1 =
						ch_params->center_freq_seg1;
	sap_dfs_set_current_channel(sapContext);

	qdf_ret_status = sme_roam_channel_change_req(hHal, sapContext->bssid,
				ch_params, &sapContext->csr_roamProfile);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		"%s: chan:%d width:%d offset:%d seg0:%d seg1:%d",
		__func__, sapContext->channel, ch_params->ch_width,
		ch_params->sec_ch_offset, ch_params->center_freq_seg0,
		ch_params->center_freq_seg1);

	if (qdf_ret_status == QDF_STATUS_SUCCESS) {
		sap_signal_hdd_event(sapContext, NULL,
			eSAP_CHANNEL_CHANGE_EVENT,
			(void *) eSAP_STATUS_SUCCESS);

		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAULT;
}

QDF_STATUS wlansap_start_beacon_req(struct sap_context *sap_ctx)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	uint8_t dfsCacWaitStatus = 0;
	tpAniSirGlobal pMac = NULL;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/* No Radar was found during CAC WAIT, So start Beaconing */
	if (pMac->sap.SapDfsInfo.sap_radar_found_status == false) {
		/* CAC Wait done without any Radar Detection */
		dfsCacWaitStatus = true;
		sap_ctx->pre_cac_complete = false;
		qdf_ret_status = sme_roam_start_beacon_req(hHal,
							   sap_ctx->bssid,
							   dfsCacWaitStatus);
		if (qdf_ret_status == QDF_STATUS_SUCCESS) {
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_E_FAULT;
}

QDF_STATUS wlansap_dfs_send_csa_ie_request(struct sap_context *sap_ctx)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;

	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB();
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	pMac->sap.SapDfsInfo.new_ch_params.ch_width =
				pMac->sap.SapDfsInfo.new_chanWidth;
	wlan_reg_set_channel_params(pMac->pdev,
			pMac->sap.SapDfsInfo.target_channel,
			0, &pMac->sap.SapDfsInfo.new_ch_params);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			"%s: chan:%d req:%d width:%d off:%d",
			__func__, pMac->sap.SapDfsInfo.target_channel,
			pMac->sap.SapDfsInfo.csaIERequired,
			pMac->sap.SapDfsInfo.new_ch_params.ch_width,
			pMac->sap.SapDfsInfo.new_ch_params.sec_ch_offset);

	qdf_ret_status = sme_roam_csa_ie_request(hHal,
				sap_ctx->bssid,
				pMac->sap.SapDfsInfo.target_channel,
				pMac->sap.SapDfsInfo.csaIERequired,
				&pMac->sap.SapDfsInfo.new_ch_params);

	if (qdf_ret_status == QDF_STATUS_SUCCESS) {
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAULT;
}

/*==========================================================================
   FUNCTION    wlansap_get_dfs_ignore_cac

   DESCRIPTION
   This API is used to get the value of ignore_cac value

   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   hHal : HAL pointer
   pIgnore_cac : pointer to ignore_cac variable

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS wlansap_get_dfs_ignore_cac(tHalHandle hHal, uint8_t *pIgnore_cac)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	*pIgnore_cac = pMac->sap.SapDfsInfo.ignore_cac;
	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    wlansap_set_dfs_ignore_cac

   DESCRIPTION
   This API is used to Set the value of ignore_cac value

   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   hHal : HAL pointer
   ignore_cac : value to set for ignore_cac variable in DFS global structure.

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS wlansap_set_dfs_ignore_cac(tHalHandle hHal, uint8_t ignore_cac)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	pMac->sap.SapDfsInfo.ignore_cac = (ignore_cac >= true) ?
					  true : false;
	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_set_dfs_restrict_japan_w53() - enable/disable dfS for japan
 * @hHal : HAL pointer
 * @disable_Dfs_JapanW3 :Indicates if Japan W53 is disabled when set to 1
 *                       Indicates if Japan W53 is enabled when set to 0
 *
 * This API is used to enable or disable Japan W53 Band
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS
wlansap_set_dfs_restrict_japan_w53(tHalHandle hHal, uint8_t disable_Dfs_W53)
{
	tpAniSirGlobal pMac = NULL;
	QDF_STATUS status;
	enum dfs_reg dfs_region;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	wlan_reg_get_dfs_region(pMac->pdev, &dfs_region);

	/*
	 * Set the JAPAN W53 restriction only if the current
	 * regulatory domain is JAPAN.
	 */
	if (DFS_MKK_REG == dfs_region) {
		pMac->sap.SapDfsInfo.is_dfs_w53_disabled = disable_Dfs_W53;
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_INFO_LOW,
			  FL("sapdfs: SET DFS JAPAN W53 DISABLED = %d"),
			  pMac->sap.SapDfsInfo.is_dfs_w53_disabled);

		status = QDF_STATUS_SUCCESS;
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL
			 ("Regdomain not japan, set disable JP W53 not valid"));

		status = QDF_STATUS_E_FAULT;
	}

	return status;
}

bool sap_is_auto_channel_select(struct sap_context *sapcontext)
{
	if (NULL == sapcontext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid SAP pointer", __func__);
		return false;
	}
	return sapcontext->channel == AUTO_CHANNEL_SELECT;
}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * wlan_sap_set_channel_avoidance() - sets sap mcc channel avoidance ini param
 * @hal:                        hal handle
 * @sap_channel_avoidance:      ini parameter value
 *
 * sets sap mcc channel avoidance ini param, to be called in sap_start
 *
 * Return: success of failure of operation
 */
QDF_STATUS
wlan_sap_set_channel_avoidance(tHalHandle hal, bool sap_channel_avoidance)
{
	tpAniSirGlobal mac_ctx = NULL;

	if (NULL != hal)
		mac_ctx = PMAC_STRUCT(hal);
	if (mac_ctx == NULL || hal == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_ERROR,
			  FL("hal or mac_ctx pointer NULL"));
		return QDF_STATUS_E_FAULT;
	}
	mac_ctx->sap.sap_channel_avoidance = sap_channel_avoidance;
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

/**
 * wlansap_set_dfs_preferred_channel_location() - set dfs preferred channel
 * @hHal : HAL pointer
 * @dfs_Preferred_Channels_location :
 *       0 - Indicates No preferred channel location restrictions
 *       1 - Indicates SAP Indoor Channels operation only.
 *       2 - Indicates SAP Outdoor Channels operation only.
 *
 * This API is used to set sap preferred channels location
 * to resetrict the DFS random channel selection algorithm
 * either Indoor/Outdoor channels only.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
QDF_STATUS
wlansap_set_dfs_preferred_channel_location(tHalHandle hHal,
					   uint8_t
					   dfs_Preferred_Channels_location)
{
	tpAniSirGlobal pMac = NULL;
	QDF_STATUS status;
	enum dfs_reg dfs_region;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	wlan_reg_get_dfs_region(pMac->pdev, &dfs_region);

	/*
	 * The Indoor/Outdoor only random channel selection
	 * restriction is currently enforeced only for
	 * JAPAN regulatory domain.
	 */
	if (DFS_MKK_REG == dfs_region) {
		pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location =
			dfs_Preferred_Channels_location;
		QDF_TRACE(QDF_MODULE_ID_SAP,
			  QDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("sapdfs:Set Preferred Operating Channel location=%d"),
			  pMac->sap.SapDfsInfo.
			  sap_operating_chan_preferred_location);

		status = QDF_STATUS_SUCCESS;
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL
				  ("sapdfs:NOT JAPAN REG, Invalid Set preferred chans location"));

		status = QDF_STATUS_E_FAULT;
	}

	return status;
}

/*==========================================================================
   FUNCTION    wlansap_set_dfs_target_chnl

   DESCRIPTION
   This API is used to set next target chnl as provided channel.
   you can provide any valid channel to this API.

   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   hHal : HAL pointer
   target_channel : target channel to be set

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS wlansap_set_dfs_target_chnl(tHalHandle hHal, uint8_t target_channel)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	if (target_channel > 0) {
		pMac->sap.SapDfsInfo.user_provided_target_channel =
			target_channel;
	} else {
		pMac->sap.SapDfsInfo.user_provided_target_channel = 0;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlansap_update_sap_config_add_ie(tsap_Config_t *pConfig,
				 const uint8_t *pAdditionIEBuffer,
				 uint16_t additionIELength,
				 eUpdateIEsType updateType)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t bufferValid = false;
	uint16_t bufferLength = 0;
	uint8_t *pBuffer = NULL;

	if (NULL == pConfig) {
		return QDF_STATUS_E_FAULT;
	}

	if ((pAdditionIEBuffer != NULL) && (additionIELength != 0)) {
		/* initialize the buffer pointer so that pe can copy */
		if (additionIELength > 0) {
			bufferLength = additionIELength;
			pBuffer = qdf_mem_malloc(bufferLength);
			if (NULL == pBuffer) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Could not allocate the buffer "));
				return QDF_STATUS_E_NOMEM;
			}
			qdf_mem_copy(pBuffer, pAdditionIEBuffer, bufferLength);
			bufferValid = true;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("update_type: %d"), updateType);
			qdf_trace_hex_dump(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO, pBuffer, bufferLength);
		}
	}

	switch (updateType) {
	case eUPDATE_IE_PROBE_BCN:
		if (pConfig->pProbeRespBcnIEsBuffer)
			qdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
		if (bufferValid) {
			pConfig->probeRespBcnIEsLen = bufferLength;
			pConfig->pProbeRespBcnIEsBuffer = pBuffer;
		} else {
			pConfig->probeRespBcnIEsLen = 0;
			pConfig->pProbeRespBcnIEsBuffer = NULL;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("No Probe Resp beacone IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_PROBE_RESP:
		if (pConfig->pProbeRespIEsBuffer)
			qdf_mem_free(pConfig->pProbeRespIEsBuffer);
		if (bufferValid) {
			pConfig->probeRespIEsBufferLen = bufferLength;
			pConfig->pProbeRespIEsBuffer = pBuffer;
		} else {
			pConfig->probeRespIEsBufferLen = 0;
			pConfig->pProbeRespIEsBuffer = NULL;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("No Probe Response IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_ASSOC_RESP:
		if (pConfig->pAssocRespIEsBuffer)
			qdf_mem_free(pConfig->pAssocRespIEsBuffer);
		if (bufferValid) {
			pConfig->assocRespIEsLen = bufferLength;
			pConfig->pAssocRespIEsBuffer = pBuffer;
		} else {
			pConfig->assocRespIEsLen = 0;
			pConfig->pAssocRespIEsBuffer = NULL;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("No Assoc Response IE received in set beacon"));
		}
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("No matching buffer type %d"), updateType);
		if (pBuffer != NULL)
			qdf_mem_free(pBuffer);
		break;
	}

	return status;
}

QDF_STATUS
wlansap_reset_sap_config_add_ie(tsap_Config_t *pConfig, eUpdateIEsType updateType)
{
	if (NULL == pConfig) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid Config pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	switch (updateType) {
	case eUPDATE_IE_ALL:    /*only used to reset */
	case eUPDATE_IE_PROBE_RESP:
		if (pConfig->pProbeRespIEsBuffer) {
			qdf_mem_free(pConfig->pProbeRespIEsBuffer);
			pConfig->probeRespIEsBufferLen = 0;
			pConfig->pProbeRespIEsBuffer = NULL;
		}
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_ASSOC_RESP:
		if (pConfig->pAssocRespIEsBuffer) {
			qdf_mem_free(pConfig->pAssocRespIEsBuffer);
			pConfig->assocRespIEsLen = 0;
			pConfig->pAssocRespIEsBuffer = NULL;
		}
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_PROBE_BCN:
		if (pConfig->pProbeRespBcnIEsBuffer) {
			qdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
			pConfig->probeRespBcnIEsLen = 0;
			pConfig->pProbeRespBcnIEsBuffer = NULL;
		}
		if (eUPDATE_IE_ALL != updateType)
			break;

	default:
		if (eUPDATE_IE_ALL != updateType)
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Invalid buffer type %d"), updateType);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION  wlansap_extend_to_acs_range

   DESCRIPTION Function extends give channel range to consider ACS chan bonding

   DEPENDENCIES PARAMETERS

   IN /OUT
   *startChannelNum : ACS extend start ch
   *endChannelNum   : ACS extended End ch
   *bandStartChannel: Band start ch
   *bandEndChannel  : Band end ch

   RETURN VALUE NONE

   SIDE EFFECTS
   ============================================================================*/
void wlansap_extend_to_acs_range(tHalHandle hal, uint8_t *startChannelNum,
		uint8_t *endChannelNum, uint8_t *bandStartChannel,
		uint8_t *bandEndChannel)
{
#define ACS_WLAN_20M_CH_INC 4
#define ACS_2G_EXTEND ACS_WLAN_20M_CH_INC
#define ACS_5G_EXTEND (ACS_WLAN_20M_CH_INC * 3)

	uint8_t tmp_startChannelNum = 0, tmp_endChannelNum = 0;
	tpAniSirGlobal mac_ctx;

	mac_ctx = PMAC_STRUCT(hal);
	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid mac_ctx", __func__);
		return;
	}
	if (*startChannelNum <= 14 && *endChannelNum <= 14) {
		*bandStartChannel = CHAN_ENUM_1;
		*bandEndChannel = CHAN_ENUM_14;
		tmp_startChannelNum = *startChannelNum > 5 ?
				   (*startChannelNum - ACS_2G_EXTEND) : 1;
		tmp_endChannelNum = (*endChannelNum + ACS_2G_EXTEND) <= 14 ?
				 (*endChannelNum + ACS_2G_EXTEND) : 14;
	} else if (*startChannelNum >= 36 && *endChannelNum >= 36) {
		*bandStartChannel = CHAN_ENUM_36;
		*bandEndChannel = CHAN_ENUM_165;
		tmp_startChannelNum = (*startChannelNum - ACS_5G_EXTEND) > 36 ?
				   (*startChannelNum - ACS_5G_EXTEND) : 36;
		tmp_endChannelNum = (*endChannelNum + ACS_5G_EXTEND) <= 165 ?
				 (*endChannelNum + ACS_5G_EXTEND) : 165;
	} else {
		*bandStartChannel = CHAN_ENUM_1;
		*bandEndChannel = CHAN_ENUM_165;
		tmp_startChannelNum = *startChannelNum > 5 ?
			(*startChannelNum - ACS_2G_EXTEND) : 1;
		tmp_endChannelNum = (*endChannelNum + ACS_5G_EXTEND) <= 165 ?
			(*endChannelNum + ACS_5G_EXTEND) : 165;
	}

	/* Note if the ACS range include only DFS channels, do not cross range
	* Active scanning in adjacent non DFS channels results in transmission
	* spikes in DFS specturm channels which is due to emission spill.
	* Remove the active channels from extend ACS range for DFS only range
	*/
	if (wlan_reg_is_dfs_ch(mac_ctx->pdev, *startChannelNum)) {
		while (!wlan_reg_is_dfs_ch(mac_ctx->pdev,
					tmp_startChannelNum) &&
			tmp_startChannelNum < *startChannelNum)
			tmp_startChannelNum += ACS_WLAN_20M_CH_INC;

		*startChannelNum = tmp_startChannelNum;
	}
	if (wlan_reg_is_dfs_ch(mac_ctx->pdev, *endChannelNum)) {
		while (!wlan_reg_is_dfs_ch(mac_ctx->pdev,
					tmp_endChannelNum) &&
				 tmp_endChannelNum > *endChannelNum)
			tmp_endChannelNum -= ACS_WLAN_20M_CH_INC;

		*endChannelNum = tmp_endChannelNum;
	}
}

QDF_STATUS wlan_sap_set_vendor_acs(struct sap_context *sap_context,
				   bool is_vendor_acs)
{
	if (!sap_context) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	sap_context->vendor_acs_dfs_lte_enabled = is_vendor_acs;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_set_dfs_nol(struct sap_context *sap_ctx,
			       eSapDfsNolType conf)
{
	void *hal = NULL;
	tpAniSirGlobal mac = NULL;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	mac = PMAC_STRUCT(hal);

	if (conf == eSAP_DFS_NOL_CLEAR) {
		struct wlan_objmgr_pdev *pdev;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: clear the DFS NOL", __func__);

		pdev = mac->pdev;
		if (!pdev) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "%s: null pdev", __func__);
			return QDF_STATUS_E_FAULT;
		}
		utils_dfs_clear_nol_channels(pdev);
	} else if (conf == eSAP_DFS_NOL_RANDOMIZE) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Randomize the DFS NOL", __func__);

	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: unsupport type %d", __func__, conf);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_populate_del_sta_params() - populate delete station parameter
 * @mac:           Pointer to peer mac address.
 * @reason_code:   Reason code for the disassoc/deauth.
 * @subtype:       Subtype points to either disassoc/deauth frame.
 * @pDelStaParams: Address where parameters to be populated.
 *
 * This API is used to populate delete station parameter structure
 *
 * Return: none
 */

void wlansap_populate_del_sta_params(const uint8_t *mac,
				     uint16_t reason_code,
				     uint8_t subtype,
				     struct csr_del_sta_params *pDelStaParams)
{
	if (NULL == mac)
		qdf_set_macaddr_broadcast(&pDelStaParams->peerMacAddr);
	else
		qdf_mem_copy(pDelStaParams->peerMacAddr.bytes, mac,
			     QDF_MAC_ADDR_SIZE);

	if (reason_code == 0)
		pDelStaParams->reason_code = eSIR_MAC_DEAUTH_LEAVING_BSS_REASON;
	else
		pDelStaParams->reason_code = reason_code;

	if (subtype == (SIR_MAC_MGMT_DEAUTH >> 4) ||
	    subtype == (SIR_MAC_MGMT_DISASSOC >> 4))
		pDelStaParams->subtype = subtype;
	else
		pDelStaParams->subtype = (SIR_MAC_MGMT_DEAUTH >> 4);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  FL(
		     "Delete STA with RC:%hu subtype:%hhu MAC::"
		     MAC_ADDRESS_STR),
		  pDelStaParams->reason_code, pDelStaParams->subtype,
		  MAC_ADDR_ARRAY(pDelStaParams->peerMacAddr.bytes));
}

QDF_STATUS wlansap_acs_chselect(struct sap_context *sap_context,
				tpWLAN_SAPEventCB pacs_event_callback,
				tsap_Config_t *pconfig,
				void *pusr_context)
{
	tHalHandle h_hal = NULL;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pmac = NULL;
	tWLAN_SAPEvent sapEvent; /* State machine event */

	if (NULL == sap_context) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid SAP pointer", __func__);

		return QDF_STATUS_E_FAULT;
	}

	h_hal = (tHalHandle)CDS_GET_HAL_CB();
	if (NULL == h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid MAC context", __func__);
		return QDF_STATUS_E_FAULT;
	}

	pmac = PMAC_STRUCT(h_hal);
	sap_context->acs_cfg = &pconfig->acs_cfg;
	sap_context->ch_width_orig = pconfig->acs_cfg.ch_width;
	sap_context->csr_roamProfile.phyMode = pconfig->acs_cfg.hw_mode;

	/*
	 * Now, configure the scan and ACS channel params
	 * to issue a scan request.
	 */
	wlansap_set_scan_acs_channel_params(pconfig, sap_context,
						pusr_context);

	/*
	 * Copy the HDD callback function to report the
	 * ACS result after scan in SAP context callback function.
	 */
	sap_context->pfnSapEventCallback = pacs_event_callback;
	/*
	 * init dfs channel nol
	 */
	sap_init_dfs_channel_nol_list(sap_context);

	/*
	 * Issue the scan request. This scan request is
	 * issued before the start BSS is done so
	 *
	 * 1. No need to pass the second parameter
	 * as the SAP state machine is not started yet
	 * and there is no need for any event posting.
	 *
	 * 2. Set third parameter to TRUE to indicate the
	 * channel selection function to register a
	 * different scan callback fucntion to process
	 * the results pre start BSS.
	 */
	qdf_status = sap_goto_channel_sel(sap_context, &sapEvent, true, false);

	if (QDF_STATUS_E_ABORTED == qdf_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"In %s,DFS not supported in the current operating mode",
			__func__);
		return QDF_STATUS_E_FAILURE;
	} else if (QDF_STATUS_E_CANCELED == qdf_status) {
		/*
		* ERROR is returned when either the SME scan request
		* failed or ACS is overridden due to other constrainst
		* So send selected channel to HDD
		*/
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Scan Req Failed/ACS Overridden"));
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Selected channel = %d"),
			sap_context->channel);

		return sap_signal_hdd_event(sap_context, NULL,
				eSAP_ACS_CHANNEL_SELECTED,
				(void *) eSAP_STATUS_SUCCESS);
	} else if (QDF_STATUS_SUCCESS == qdf_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("Successfully Issued a Pre Start Bss Scan Request"));
	}
	return qdf_status;
}

/**
 * wlan_sap_enable_phy_error_logs() - Enable DFS phy error logs
 * @hal:        global hal handle
 * @enable_log: value to set
 *
 * Since the frequency of DFS phy error is very high, enabling logs for them
 * all the times can cause crash and will also create lot of useless logs
 * causing difficulties in debugging other issue. This function will be called
 * from iwpriv cmd to eanble such logs temporarily.
 *
 * Return: void
 */
void wlan_sap_enable_phy_error_logs(tHalHandle hal, uint32_t enable_log)
{
	int error;

	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	mac_ctx->sap.enable_dfs_phy_error_logs = !!enable_log;
	tgt_dfs_control(mac_ctx->pdev, DFS_SET_DEBUG_LEVEL, &enable_log,
			sizeof(uint32_t), NULL, NULL, &error);
}

uint32_t wlansap_get_chan_width(struct sap_context *sap_ctx)
{
	return wlan_sap_get_vht_ch_width(sap_ctx);
}

QDF_STATUS wlansap_set_tx_leakage_threshold(tHalHandle hal,
			uint16_t tx_leakage_threshold)
{
	tpAniSirGlobal mac;

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid hal pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	mac = PMAC_STRUCT(hal);
	tgt_dfs_set_tx_leakage_threshold(mac->pdev, tx_leakage_threshold);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			"%s: leakage_threshold %d", __func__,
			tx_leakage_threshold);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_set_invalid_session(struct sap_context *sap_ctx)
{
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer"));
		return QDF_STATUS_E_FAILURE;
	}

	sap_ctx->sessionId = CSR_SESSION_ID_INVALID;

	return QDF_STATUS_SUCCESS;
}

void wlansap_cleanup_cac_timer(struct sap_context *sap_ctx)
{
	tHalHandle hal;
	tpAniSirGlobal pmac;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP context"));
		return;
	}

	hal = CDS_GET_HAL_CB();
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid hal pointer"));
		return;
	}

	pmac = PMAC_STRUCT(hal);
	if (pmac->sap.SapDfsInfo.is_dfs_cac_timer_running) {
		qdf_mc_timer_stop(&pmac->sap.SapDfsInfo.
				  sap_dfs_cac_timer);
		pmac->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
		qdf_mc_timer_destroy(
			&pmac->sap.SapDfsInfo.sap_dfs_cac_timer);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("sapdfs, force cleanup running dfs cac timer"));
	}
}
