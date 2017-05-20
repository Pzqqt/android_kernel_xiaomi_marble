/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
static ptSapContext gp_sap_ctx[SAP_MAX_NUM_SESSION];
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
static QDF_STATUS wlansap_save_context(ptSapContext ctx)
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
QDF_STATUS wlansap_context_get(ptSapContext ctx)
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

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
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
void wlansap_context_put(ptSapContext ctx)
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

/**
 * wlansap_open() - WLAN SAP open function call
 * @p_cds_gctx: Pointer to the global cds context; a handle to SAP's
 *
 * Called at driver initialization (cds_open). SAP will initialize
 * all its internal resources and will wait for the call to start to
 * register with the other modules.
 *
 * Return: Pointer to the SAP context
 */
void *wlansap_open(void *p_cds_gctx)
{
	ptSapContext pSapCtx = NULL;
	QDF_STATUS status;

	/* dynamically allocate the sapContext */
	pSapCtx = (ptSapContext) qdf_mem_malloc(sizeof(tSapContext));

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return NULL;
	}

	/* Clean up SAP control block, initialize all values */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Enter"));

	/* Setup the "link back" to the CDS context */
	pSapCtx->p_cds_gctx = p_cds_gctx;

	/* Save the SAP context pointer */
	status = wlansap_save_context(pSapCtx);
	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: failed to save SAP context", __func__);
		qdf_mem_free(pSapCtx);
		return NULL;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Exit"));

	return pSapCtx;
} /* wlansap_open */

/**
 * wlansap_start() - wlan start SAP.
 * @pCtx: Pointer to the global cds context; a handle to SAP's
 *        control block can be extracted from its context
 *        When MBSSID feature is enabled, SAP context is directly
 *        passed to SAP APIs
 * @mode: Device mode
 * @addr: MAC address of the SAP
 * @session_id: Pointer to the session id
 *
 * Called as part of the overall start procedure (cds_enable). SAP will
 * use this call to register with TL as the SAP entity for SAP RSN frames.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault.
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_start(void *pCtx, enum tQDF_ADAPTER_MODE mode,
			 uint8_t *addr, uint32_t session_id)
{
	ptSapContext pSapCtx = NULL;
	QDF_STATUS qdf_ret_status;
	tHalHandle hal;
	tpAniSirGlobal pmac;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_start invoked successfully");
	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/*------------------------------------------------------------------------
	    For now, presume security is not enabled.
	   -----------------------------------------------------------------------*/
	pSapCtx->ucSecEnabled = WLANSAP_SECURITY_ENABLED_STATE;

	/*------------------------------------------------------------------------
	    Now configure the roaming profile links. To SSID and bssid.
	   ------------------------------------------------------------------------*/
	/* We have room for two SSIDs. */
	pSapCtx->csr_roamProfile.SSIDs.numOfSSIDs = 1;   /* This is true for now. */
	pSapCtx->csr_roamProfile.SSIDs.SSIDList = pSapCtx->SSIDList;     /* Array of two */
	pSapCtx->csr_roamProfile.SSIDs.SSIDList[0].SSID.length = 0;
	pSapCtx->csr_roamProfile.SSIDs.SSIDList[0].handoffPermitted = false;
	pSapCtx->csr_roamProfile.SSIDs.SSIDList[0].ssidHidden =
		pSapCtx->SSIDList[0].ssidHidden;

	pSapCtx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1; /* This is true for now. */
	pSapCtx->csr_roamProfile.BSSIDs.bssid = &pSapCtx->bssid;
	pSapCtx->csr_roamProfile.csrPersona = mode;
	qdf_mem_copy(pSapCtx->self_mac_addr, addr, QDF_MAC_ADDR_SIZE);
	qdf_event_create(&pSapCtx->sap_session_opened_evt);

	/* Now configure the auth type in the roaming profile. To open. */
	pSapCtx->csr_roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;        /* open is the default */

	if (!QDF_IS_STATUS_SUCCESS(qdf_mutex_create(&pSapCtx->SapGlobalLock))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "wlansap_start failed init lock");
		return QDF_STATUS_E_FAULT;
	}

	hal = (tHalHandle) CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid HAL pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}
	pmac = PMAC_STRUCT(hal);
	/*
	 * Anytime when you call sap_open_session, please call
	 * sap_set_session_param to fill sap context parameters
	 */
	qdf_ret_status = sap_open_session(hal, pSapCtx, session_id);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"Error: In %s calling sap_open_session status = %d",
			__func__, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_ret_status = sap_set_session_param(hal, pSapCtx, session_id);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"In %s calling sap_set_session_param status = %d",
			__func__, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}
#ifdef NAPIER_SCAN
	/* Register with scan component */
	pSapCtx->req_id = ucfg_scan_register_requester(pmac->psoc, "SAP",
					sap_scan_event_callback, pSapCtx);
#endif
	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_stop() - stop SAP module.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs
 *
 * Called by cds_disable to stop operation in SAP, before close. SAP will
 * suspend all BT-AMP Protocol Adaption Layer operation and will wait for the
 * close request to clean up its resources.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault.
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_stop(void *pCtx)
{
	ptSapContext pSapCtx = NULL;
	tHalHandle hal;
	tpAniSirGlobal pmac;

	/* Sanity check - Extract SAP control block */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  "wlansap_stop invoked successfully ");

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	hal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	pmac = (tpAniSirGlobal) hal;
	if (NULL == pmac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid MAC context from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	if (QDF_STATUS_SUCCESS !=
			sap_close_session(hal, pSapCtx, NULL, false)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("sap session can't be closed"));
		return QDF_STATUS_E_FAULT;
	}
#ifdef NAPIER_SCAN
	ucfg_scan_unregister_requester(pmac->psoc, pSapCtx->req_id);
#endif
	sap_free_roam_profile(&pSapCtx->csr_roamProfile);

	if (!QDF_IS_STATUS_SUCCESS(qdf_mutex_destroy(&pSapCtx->SapGlobalLock))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "wlansap_stop failed destroy lock");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_close - close SAP module.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * Called by cds_close during general driver close procedure. SAP will clean up
 * all the internal resources.
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_close(void *pCtx)
{
	ptSapContext pSapCtx = NULL;
	tHalHandle hal;

	/* Sanity check - Extract SAP control block */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_close invoked");

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	/* Cleanup SAP control block */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Enter"));
	sap_cleanup_channel_list(pCtx);
	hal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (!hal)
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal is NULL, so lets skip global sap cleanup"));
	else if (pSapCtx->sessionId != CSR_SESSION_ID_INVALID)
		/* empty queues/lists/pkts if any */
		sap_clear_session_param(hal, pSapCtx, pSapCtx->sessionId);
	/*
	 * wlansap_context_put will release actual pSapCtx memory
	 * allocated during wlansap_open
	 */
	wlansap_context_put(pSapCtx);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG, FL("Exit"));

	return QDF_STATUS_SUCCESS;
} /* wlansap_close */

/*==========================================================================
   FUNCTION    wlansap_pmc_full_pwr_req_cb

   DESCRIPTION
    Callback provide to PMC in the pmc_request_full_power API.

   DEPENDENCIES

   PARAMETERS

    IN
    callbackContext:  The user passed in a context to identify
    status : The qdf_ret_status

   RETURN VALUE
    None

   SIDE EFFECTS
   ============================================================================*/
void
wlansap_pmc_full_pwr_req_cb(void *callbackContext, QDF_STATUS status)
{
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* If success what else to be handled??? */
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "wlansap_pmc_full_pwr_req_cb: PMC failed to put the chip in Full power");

	}

} /* wlansap_pmc_full_pwr_req_cb */

/**
 * wlansap_get_state() - get SAP state
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api returns the current SAP state to the caller.
 *
 * Return: uint8_t - the SAP FSM state.
 */
uint8_t wlansap_get_state(void *pCtx)
{
	ptSapContext pSapCtx = NULL;

	pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	return pSapCtx->sapsMachine;
}

bool wlansap_is_channel_in_nol_list(void *p_cds_gctx,
				    uint8_t channelNumber,
				    ePhyChanBondState chanBondState)
{
	ptSapContext pSapCtx = NULL;

	pSapCtx = CDS_GET_SAP_CB(p_cds_gctx);

	if (!pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	return sap_dfs_is_channel_in_nol_list(pSapCtx, channelNumber,
					      chanBondState);
}

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
static QDF_STATUS wlansap_mark_leaking_channel(ptSapContext sap_ctx,
		tSapDfsNolInfo *nol,
		uint8_t *leakage_adjusted_lst,
		uint8_t chan_bw)
{

	return sap_mark_leaking_ch(sap_ctx, chan_bw, nol, 1,
			leakage_adjusted_lst);
}
#else
static QDF_STATUS wlansap_mark_leaking_channel(ptSapContext sap_ctx,
		tSapDfsNolInfo *nol,
		uint8_t *leakage_adjusted_lst,
		uint8_t chan_bw)
{
	return QDF_STATUS_SUCCESS;
}
#endif

bool wlansap_is_channel_leaking_in_nol(void *ctx, uint8_t channel,
		uint8_t chan_bw)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);
	tpAniSirGlobal mac_ctx;
	uint8_t leakage_adjusted_lst[1];
	void *handle = NULL;
	tSapDfsNolInfo *nol;

	leakage_adjusted_lst[0] = channel;
	handle = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	mac_ctx = PMAC_STRUCT(handle);
	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid mac pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	nol = mac_ctx->sap.SapDfsInfo.sapDfsChannelNolList;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("sapdfs: Processing current chan against NOL."));
	if (wlansap_mark_leaking_channel(sap_ctx, nol,
			leakage_adjusted_lst, chan_bw) != QDF_STATUS_SUCCESS) {
		return true;
	}
	if (leakage_adjusted_lst[0] == 0)
		return true;
	return false;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/*==========================================================================
   FUNCTION    wlansap_check_cc_intf

   DESCRIPTION Restart SAP if Concurrent Channel interfering

   DEPENDENCIES NA.

   PARAMETERS
   IN
   Ctx: Pointer to cds Context or Sap Context based on MBSSID

   RETURN VALUE NONE

   SIDE EFFECTS
   ============================================================================*/
uint16_t wlansap_check_cc_intf(void *Ctx)
{
	tHalHandle hHal;
	uint16_t intf_ch;
	ptSapContext pSapCtx = CDS_GET_SAP_CB(Ctx);

	hHal = (tHalHandle) CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid MAC context from p_cds_gctx", __func__);
		return 0;
	}
	intf_ch = sme_check_concurrent_channel_overlap(hHal, 0, 0,
						       pSapCtx->cc_switch_mode);
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
				ptSapContext psap_ctx,
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

	h_hal = (tHalHandle)CDS_GET_HAL_CB(psap_ctx->p_cds_gctx);
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
 * wlan_sap_get_vht_ch_width() - Returns SAP VHT channel width.
 * @ctx:	Pointer to cds Context or Sap Context based on MBSSID
 *
 * This function provides the SAP current VHT channel with.
 *
 * Return: VHT channel width
 */
uint32_t wlan_sap_get_vht_ch_width(void *ctx)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from ctx"));
		return 0;
	}

	return sap_ctx->ch_params.ch_width;
}

/**
 * wlan_sap_set_vht_ch_width() - Sets SAP VHT channel width.
 * @ctx:		Pointer to cds Context or Sap Context based on MBSSID
 * @vht_channel_width:	SAP VHT channel width value.
 *
 * This function sets the SAP current VHT channel with.
 *
 * Return: None
 */
void wlan_sap_set_vht_ch_width(void *ctx, uint32_t vht_channel_width)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from ctx"));
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
		ptSapContext sap_context)
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
		uint16_t sap_ch, ptSapContext sap_context)
{
	return true;
}
#endif
/**
 * wlansap_start_bss() - start BSS
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pQctCommitConfig: Pointer to configuration structure passed down from
 *                    HDD(HostApd for Android)
 * @hdd_SapEventCallback: Callback function in HDD called by SAP to inform HDD
 *                        about SAP results
 * @pUsrContext: Parameter that will be passed back in all the SAP callback
 *               events.
 *
 * This api function provides SAP FSM event eWLAN_SAP_PHYSICAL_LINK_CREATE for
 * starting AP BSS
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_start_bss(void *pCtx,     /* pwextCtx */
			     tpWLAN_SAPEventCB pSapEventCallback,
			     tsap_Config_t *pConfig, void *pUsrContext) {
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	tHalHandle hHal;
	tpAniSirGlobal pmac = NULL;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	pSapCtx = CDS_GET_SAP_CB(pCtx);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_start_bss: sapContext=%p", pSapCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	pSapCtx->sapsMachine = eSAP_DISCONNECTED;

	/* Channel selection is auto or configured */
	pSapCtx->channel = pConfig->channel;
	pSapCtx->dfs_mode = pConfig->acs_dfs_mode;
	pSapCtx->ch_params.ch_width = pConfig->ch_params.ch_width;
	pSapCtx->ch_params.center_freq_seg0 =
		pConfig->ch_params.center_freq_seg0;
	pSapCtx->ch_params.center_freq_seg1 =
		pConfig->ch_params.center_freq_seg1;
	pSapCtx->ch_params.sec_ch_offset =
		pConfig->ch_params.sec_ch_offset;
	pSapCtx->ch_width_orig = pConfig->ch_width_orig;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	pSapCtx->cc_switch_mode = pConfig->cc_switch_mode;
#endif
	pSapCtx->pUsrContext = pUsrContext;
	pSapCtx->enableOverLapCh = pConfig->enOverLapCh;
	pSapCtx->acs_cfg = &pConfig->acs_cfg;
	pSapCtx->dfs_cac_offload = pConfig->dfs_cac_offload;
	pSapCtx->isCacEndNotified = false;
	/* Set the BSSID to your "self MAC Addr" read the mac address
		from Configuation ITEM received from HDD */
	pSapCtx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(pSapCtx->csr_roamProfile.BSSIDs.bssid,
		     pSapCtx->self_mac_addr, sizeof(struct qdf_mac_addr));

	/* Save a copy to SAP context */
	qdf_mem_copy(pSapCtx->csr_roamProfile.BSSIDs.bssid,
		     pConfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(pSapCtx->self_mac_addr,
		     pConfig->self_macaddr.bytes, QDF_MAC_ADDR_SIZE);

	/* copy the configuration items to csrProfile */
	sapconvert_to_csr_profile(pConfig, eCSR_BSS_TYPE_INFRA_AP,
			       &pSapCtx->csr_roamProfile);
	hHal = (tHalHandle) CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid MAC context from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	pmac = PMAC_STRUCT(hHal);
	if (NULL == pmac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid MAC context from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
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
	pmac->sap.sapCtxList[pSapCtx->sessionId].pSapContext = pSapCtx;
	pmac->sap.sapCtxList[pSapCtx->sessionId].sapPersona =
		pSapCtx->csr_roamProfile.csrPersona;
	pmac->sap.sapCtxList[pSapCtx->sessionId].sessionID =
		pSapCtx->sessionId;

	/* Copy MAC filtering settings to sap context */
	pSapCtx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;
	qdf_mem_copy(pSapCtx->acceptMacList, pConfig->accept_mac,
		     sizeof(pConfig->accept_mac));
	pSapCtx->nAcceptMac = pConfig->num_accept_mac;
	sap_sort_mac_list(pSapCtx->acceptMacList, pSapCtx->nAcceptMac);
	qdf_mem_copy(pSapCtx->denyMacList, pConfig->deny_mac,
		     sizeof(pConfig->deny_mac));
	pSapCtx->nDenyMac = pConfig->num_deny_mac;
	sap_sort_mac_list(pSapCtx->denyMacList, pSapCtx->nDenyMac);
	/* Fill in the event structure for FSM */
	sapEvent.event = eSAP_HDD_START_INFRA_BSS;
	sapEvent.params = 0;    /* pSapPhysLinkCreate */

	/* Store the HDD callback in SAP context */
	pSapCtx->pfnSapEventCallback = pSapEventCallback;

	/* Handle event */
	qdf_status = sap_fsm(pSapCtx, &sapEvent);

	return qdf_status;
} /* wlansap_start_bss */

/**
 * wlansap_set_mac_acl() - set MAC list entry in ACL.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pConfig: Pointer to SAP config.
 *
 * This api function provides SAP to set mac list entry in accept list as well
 * as deny list
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_set_mac_acl(void *pCtx,    /* pwextCtx */
			       tsap_Config_t *pConfig) {
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_set_mac_acl");

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	/* Copy MAC filtering settings to sap context */
	pSapCtx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;

	if (eSAP_DENY_UNLESS_ACCEPTED == pSapCtx->eSapMacAddrAclMode) {
		qdf_mem_copy(pSapCtx->acceptMacList,
			     pConfig->accept_mac,
			     sizeof(pConfig->accept_mac));
		pSapCtx->nAcceptMac = pConfig->num_accept_mac;
		sap_sort_mac_list(pSapCtx->acceptMacList,
			       pSapCtx->nAcceptMac);
	} else if (eSAP_ACCEPT_UNLESS_DENIED ==
		   pSapCtx->eSapMacAddrAclMode) {
		qdf_mem_copy(pSapCtx->denyMacList, pConfig->deny_mac,
			     sizeof(pConfig->deny_mac));
		pSapCtx->nDenyMac = pConfig->num_deny_mac;
		sap_sort_mac_list(pSapCtx->denyMacList, pSapCtx->nDenyMac);
	}

	return qdf_status;
} /* wlansap_set_mac_acl */

/**
 * wlansap_stop_bss() - stop BSS.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api function provides SAP FSM event eSAP_HDD_STOP_INFRA_BSS for
 * stopping AP BSS
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS wlansap_stop_bss(void *pCtx)
{
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid Global CDS handle", __func__);
		return QDF_STATUS_E_FAULT;
	}

	pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* Fill in the event structure for FSM */
	sapEvent.event = eSAP_HDD_STOP_INFRA_BSS;
	sapEvent.params = 0;

	/* Handle event */
	qdf_status = sap_fsm(pSapCtx, &sapEvent);

	return qdf_status;
}

/**
 * wlansap_get_assoc_stations() - get list of associated stations.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @modId: Module from whom list of associtated stations  is supposed to be
 *         probed. If an invalid module is passed then by default
 *         QDF_MODULE_ID_PE will be probed
 * @pAssocStas: Pointer to list of associated stations that are known to the
 *              module specified in mod parameter
 *
 * This api function is used to probe the list of associated stations from
 * various modules of CORE stack
 * NOTE: The memory for this list will be allocated by the caller of this API
 *
 * Return: The result code associated with performing the operation
 *         QDF_STATUS_SUCCESS: Success
 */
QDF_STATUS
wlansap_get_assoc_stations
	(void *pCtx, QDF_MODULE_ID modId, tpSap_AssocMacAddr pAssocStas) {
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_get_associated_stas(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				     pSapCtx->sessionId, modId,
				     pSapCtx->pUsrContext,
				     (void **) pSapCtx->pfnSapEventCallback,
				     (uint8_t *) pAssocStas);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_remove_wps_session_overlap() - remove overlapping wps session.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pRemoveMac: pointer to struct qdf_mac_addr for session MAC address
 *
 * This api function provides for Ap App/HDD to remove an entry from session
 * overlap info.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 *         QDF_STATUS_E_FAULT:  Session is not dectected.
 *                              The parameter is function not valid.
 */
QDF_STATUS
wlansap_remove_wps_session_overlap(void *pCtx,
				   struct qdf_mac_addr pRemoveMac)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_get_wps_session_overlap(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
					 pSapCtx->sessionId, pSapCtx->pUsrContext,
					 (void **) pSapCtx->pfnSapEventCallback,
					 pRemoveMac);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_get_wps_session_overlap() - get overlapping wps session.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api function provides for Ap App/HDD to get WPS session overlap info.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_get_wps_session_overlap(void *pCtx)
{
	struct qdf_mac_addr pRemoveMac = QDF_MAC_ADDR_ZERO_INITIALIZER;

	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_get_wps_session_overlap(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
					 pSapCtx->sessionId, pSapCtx->pUsrContext,
					 (void **) pSapCtx->pfnSapEventCallback,
					 pRemoveMac);

	return QDF_STATUS_SUCCESS;
}

/* This routine will set the mode of operation for ACL dynamically*/
QDF_STATUS wlansap_set_mode(void *pCtx, uint32_t mode)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	pSapCtx->eSapMacAddrAclMode = (eSapMacAddrACL) mode;
	return QDF_STATUS_SUCCESS;
}

/* Get ACL Mode */
QDF_STATUS wlansap_get_acl_mode(void *pCtx, eSapMacAddrACL *mode)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	*mode = pSapCtx->eSapMacAddrAclMode;
	return QDF_STATUS_SUCCESS;
}

/* API to get ACL Accept List */
QDF_STATUS
wlansap_get_acl_accept_list(void *pCtx, struct qdf_mac_addr *pAcceptList,
				uint8_t *nAcceptList)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	memcpy((void *)pAcceptList, (void *)pSapCtx->acceptMacList,
	       (pSapCtx->nAcceptMac * QDF_MAC_ADDR_SIZE));
	*nAcceptList = pSapCtx->nAcceptMac;
	return QDF_STATUS_SUCCESS;
}

/* API to get Deny List */
QDF_STATUS
wlansap_get_acl_deny_list(void *pCtx, struct qdf_mac_addr *pDenyList,
			  uint8_t *nDenyList)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	memcpy((void *)pDenyList, (void *)pSapCtx->denyMacList,
	       (pSapCtx->nDenyMac * QDF_MAC_ADDR_SIZE));
	*nDenyList = pSapCtx->nDenyMac;
	return QDF_STATUS_SUCCESS;
}

/* This routine will clear all the entries in accept list as well as deny list  */

QDF_STATUS wlansap_clear_acl(void *pCtx)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	uint8_t i;

	if (NULL == pSapCtx) {
		return QDF_STATUS_E_RESOURCES;
	}

	if (pSapCtx->denyMacList != NULL) {
		for (i = 0; i < (pSapCtx->nDenyMac - 1); i++) {
			qdf_mem_zero((pSapCtx->denyMacList + i)->bytes,
				     QDF_MAC_ADDR_SIZE);

		}
	}
	sap_print_acl(pSapCtx->denyMacList, pSapCtx->nDenyMac);
	pSapCtx->nDenyMac = 0;

	if (pSapCtx->acceptMacList != NULL) {
		for (i = 0; i < (pSapCtx->nAcceptMac - 1); i++) {
			qdf_mem_zero((pSapCtx->acceptMacList + i)->bytes,
				     QDF_MAC_ADDR_SIZE);

		}
	}
	sap_print_acl(pSapCtx->acceptMacList, pSapCtx->nAcceptMac);
	pSapCtx->nAcceptMac = 0;

	return QDF_STATUS_SUCCESS;
}

/*
 * wlansap_modify_acl() -Update ACL entries
 *
 * @ctx: Global context
 * @peer_sta_mac: peer sta mac to be updated.
 * @list_type: white/Black list type.
 * @cmd: command to be executed on ACL.
 *
 * This function is called when a peer needs to be added or deleted from the
 * white/black ACL
 *
 * Return: Status
 */

QDF_STATUS
wlansap_modify_acl
	(void *ctx,
	uint8_t *peer_sta_mac, eSapACLType list_type, eSapACLCmdType cmd) {
	eSapBool sta_white_list = eSAP_FALSE, sta_black_list = eSAP_FALSE;
	uint8_t staWLIndex, staBLIndex;
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

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

	/* the expectation is a mac addr will not be in both the lists at the same time.
	   It is the responsiblity of userspace to ensure this */
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
				sap_remove_mac_from_acl(sap_ctx->
						    denyMacList,
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

				struct tagCsrDelStaParams delStaParams;

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
				wlansap_deauth_sta(ctx, &delStaParams);
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
			struct tagCsrDelStaParams delStaParams;
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
				sap_remove_mac_from_acl(sap_ctx->
						    acceptMacList,
						    &sap_ctx->
						    nAcceptMac,
						    staWLIndex);
			}
			/* If we are adding a client to the black list; if its connected, send deauth */
			wlansap_populate_del_sta_params(peer_sta_mac,
				eCsrForcedDeauthSta,
				(SIR_MAC_MGMT_DEAUTH >> 4),
				&delStaParams);
			wlansap_deauth_sta(ctx, &delStaParams);
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

/**
 * wlansap_disassoc_sta() - initiate disassociation of station.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @p_del_sta_params: pointer to station deletion parameters
 *
 * This api function provides for Ap App/HDD initiated disassociation of station
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_disassoc_sta(void *pCtx,
				struct tagCsrDelStaParams *p_del_sta_params)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_disconnect_sta(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				pSapCtx->sessionId, p_del_sta_params);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_deauth_sta() - Ap App/HDD initiated deauthentication of station
 * @pCtx : Pointer to the global cds context; a handle to SAP's
 *         control block can be extracted from its context
 *         When MBSSID feature is enabled, SAP context is directly
 *         passed to SAP APIs
 * @pDelStaParams : Pointer to parameters of the station to deauthenticate
 *
 * This api function provides for Ap App/HDD initiated deauthentication of
 * station
 *
 * Return: The QDF_STATUS code associated with performing the operation
 */
QDF_STATUS wlansap_deauth_sta(void *pCtx,
			      struct tagCsrDelStaParams *pDelStaParams)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return qdf_status;
	}

	qdf_ret_status =
		sme_roam_deauth_sta(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				    pSapCtx->sessionId, pDelStaParams);

	if (qdf_ret_status == QDF_STATUS_SUCCESS) {
		qdf_status = QDF_STATUS_SUCCESS;
	}
	return qdf_status;
}

/**
 * wlansap_update_bw80_cbmode() - fucntion to update channel bonding mode for
 *                                VHT80 channel.
 * @channel: target channel
 * @sme_config: sme configuration context
 *
 * Return: none
 */
static inline void wlansap_update_bw80_cbmode(uint32_t channel,
		tSmeConfigParams *sme_config)
{
	if (channel == 36 || channel == 52 || channel == 100 ||
		channel == 116 || channel == 149 || channel == 132) {
		sme_config->csrConfig.channelBondingMode5GHz =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW;
	} else if (channel == 40 || channel == 56 || channel == 104 ||
			channel == 120 || channel == 153 || channel == 136) {
		sme_config->csrConfig.channelBondingMode5GHz =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW;
	} else if (channel == 44 || channel == 60 || channel == 108 ||
			channel == 124 || channel == 157 || channel == 140) {
		sme_config->csrConfig.channelBondingMode5GHz =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH;
	} else if (channel == 48 || channel == 64 || channel == 112 ||
		channel == 128 || channel == 144 || channel == 161) {
		sme_config->csrConfig.channelBondingMode5GHz =
			eCSR_INI_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH;
	}
}

/**
 * wlansap_update_csa_channel_params() - fucntion to populate channel width and
 *                                        bonding modes.
 * @sap_context: sap adapter context
 * @channel: target channel
 *
 * Return: The QDF_STATUS code associated with performing the operation
 */
static QDF_STATUS wlansap_update_csa_channel_params(ptSapContext sap_context,
	uint32_t channel)
{
	void *hal;
	tpAniSirGlobal mac_ctx;
	uint8_t bw;

	hal = CDS_GET_HAL_CB(sap_context->p_cds_gctx);
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
 * @p_cds_gctx: Pointer to cds global context structure
 * @targetChannel: Target channel
 * @target_bw: Target bandwidth
 *
 * This api function does a channel change to the target channel specified.
 * CSA IE is included in the beacons before doing a channel change.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlansap_set_channel_change_with_csa(void *p_cds_gctx, uint32_t targetChannel,
					enum phy_ch_width target_bw)
{

	ptSapContext sapContext = NULL;
	tWLAN_SAPEvent sapEvent;
	tpAniSirGlobal pMac = NULL;
	void *hHal = NULL;
	bool valid;
	QDF_STATUS status;

	sapContext = CDS_GET_SAP_CB(p_cds_gctx);
	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);

		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

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
		valid = wlan_sap_validate_channel_switch(hHal, targetChannel,
				sapContext);
		if (!valid) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
					FL("Channel switch to %u is not allowed due to concurrent channel interference"),
					targetChannel);
			return QDF_STATUS_E_FAULT;
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

/**
 * wlansap_set_counter_measure() - set counter measure.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @bEnable: If true than all stations will be disassociated and no more
 *           will be allowed to associate. If false than CORE will come out
 *            of this state.
 *
 * This api function is used to disassociate all the stations and prevent
 * association for any other station.Whenever Authenticator receives 2 mic
 * failures within 60 seconds, Authenticator will enable counter measure at
 * SAP Layer. Authenticator will start the 60 seconds timer. Core stack will
 * not allow any STA to associate till HDD disables counter meassure. Core
 * stack shall kick out all the STA which are currently associated and DIASSOC
 * Event will be propogated to HDD for each STA to clean up the HDD STA table.
 * Once the 60 seconds timer expires, Authenticator will disable the counter
 * meassure at core stack. Now core stack can allow STAs to associate.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_set_counter_measure(void *pCtx, bool bEnable)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sme_roam_tkip_counter_measures(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				       pSapCtx->sessionId, bEnable);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlansap_set_key_sta() - set keys for a stations.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pSetKeyInfo : tCsrRoamSetKey structure for the station
 *
 * This api function provides for Ap App/HDD to set key for a station.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_set_key_sta(void *pCtx, tCsrRoamSetKey *pSetKeyInfo)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	uint32_t roamId = 0xFF;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	qdf_ret_status =
		sme_roam_set_key(hHal, pSapCtx->sessionId, pSetKeyInfo,
				 &roamId);

	if (qdf_ret_status == QDF_STATUS_SUCCESS)
		qdf_status = QDF_STATUS_SUCCESS;
	else
		qdf_status = QDF_STATUS_E_FAULT;

	return qdf_status;
}

/**
 * wlan_sap_getstation_ie_information() - RSNIE Population
 *
 * @ctx: Global context
 * @len: Length of @buf
 * @buf: RSNIE IE data
 *
 *  Populate RSN IE from CSR to HDD context
 *
 * Return: QDF_STATUS enumeration
 */

QDF_STATUS
wlan_sap_getstation_ie_information
	(void *ctx, uint32_t *len, uint8_t *buf) {
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	ptSapContext sap_ctx = NULL;
	uint32_t ie_len = 0;

	sap_ctx = CDS_GET_SAP_CB(ctx);
	if (NULL == sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from pCtx"));
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

/**
 * wlansap_set_wps_ie() - set WPI IE
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pWPSIE: tSap_WPSIE structure that include WPS IEs
 *
 * This api function provides API for App/HDD to set WPS IE.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
QDF_STATUS wlansap_set_wps_ie(void *pCtx, tSap_WPSIE *pSap_WPSIe)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		  "%s, %d", __func__, __LINE__);

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	if (sap_acquire_global_lock(pSapCtx) == QDF_STATUS_SUCCESS) {
		if (pSap_WPSIe->sapWPSIECode == eSAP_WPS_BEACON_IE) {
			qdf_mem_copy(&pSapCtx->APWPSIEs.SirWPSBeaconIE,
				     &pSap_WPSIe->sapwpsie.
				     sapWPSBeaconIE,
				     sizeof(tSap_WPSBeaconIE));
		} else if (pSap_WPSIe->sapWPSIECode ==
			   eSAP_WPS_PROBE_RSP_IE) {
			qdf_mem_copy(&pSapCtx->APWPSIEs.
				     SirWPSProbeRspIE,
				     &pSap_WPSIe->sapwpsie.
				     sapWPSProbeRspIE,
				     sizeof(tSap_WPSProbeRspIE));
		} else {
			sap_release_global_lock(pSapCtx);
			return QDF_STATUS_E_FAULT;
		}
		sap_release_global_lock(pSapCtx);
		return QDF_STATUS_SUCCESS;
	} else
		return QDF_STATUS_E_FAULT;
}

/**
 * wlansap_update_wps_ie() - update WPI IE
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api function provides API for App/HDD to update WPS IE.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
QDF_STATUS wlansap_update_wps_ie(void *pCtx)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;
	ptSapContext pSapCtx = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "%s, %d", __func__, __LINE__);

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status =
		sme_roam_update_apwpsie(hHal, pSapCtx->sessionId,
					&pSapCtx->APWPSIEs);

	if (qdf_ret_status == QDF_STATUS_SUCCESS)
		qdf_status = QDF_STATUS_SUCCESS;
	else
		qdf_status = QDF_STATUS_E_FAULT;

	return qdf_status;
}

/**
 * wlansap_get_wps_state() - get WPS session state
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pbWPSState: Pointer to variable to indicate if device is in
 *              WPS Registration state
 *
 * This api function provides for Ap App/HDD to check if WPS session in process.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success
 */
QDF_STATUS wlansap_get_wps_state(void *pCtx, bool *bWPSState)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	if (sap_acquire_global_lock(pSapCtx) == QDF_STATUS_SUCCESS) {
		if (pSapCtx->APWPSIEs.SirWPSProbeRspIE.
		    FieldPresent &
		    SIR_WPS_PROBRSP_SELECTEDREGISTRA_PRESENT)
			*bWPSState = true;
		else
			*bWPSState = false;

		sap_release_global_lock(pSapCtx);

		return QDF_STATUS_SUCCESS;
	} else
		return QDF_STATUS_E_FAULT;

}

QDF_STATUS sap_acquire_global_lock(ptSapContext pSapCtx)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;

	if (QDF_IS_STATUS_SUCCESS(qdf_mutex_acquire(&pSapCtx->SapGlobalLock))) {
		qdf_status = QDF_STATUS_SUCCESS;
	}

	return qdf_status;
}

QDF_STATUS sap_release_global_lock(ptSapContext pSapCtx)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAULT;

	if (QDF_IS_STATUS_SUCCESS(qdf_mutex_release(&pSapCtx->SapGlobalLock))) {
		qdf_status = QDF_STATUS_SUCCESS;
	}

	return qdf_status;
}

/**
 * wlansap_set_wparsn_ies() - set WPA  RSN IEs
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pWPARSNIEs  : buffer to the WPA/RSN IEs
 * @WPARSNIEsLen: length of WPA/RSN IEs
 *
 * This api function provides for Ap App/HDD to set AP WPA and RSN IE in its
 * beacon and probe response.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_set_wparsn_ies
	(void *pCtx, uint8_t *pWPARSNIEs, uint32_t WPARSNIEsLen) {
	ptSapContext pSapCtx = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	pSapCtx->APWPARSNIEs.length = (uint16_t) WPARSNIEsLen;
	qdf_mem_copy(pSapCtx->APWPARSNIEs.rsnIEdata, pWPARSNIEs,
		     WPARSNIEsLen);

	qdf_ret_status =
		sme_roam_update_apwparsni_es(hHal, pSapCtx->sessionId,
					     &pSapCtx->APWPARSNIEs);

	if (qdf_ret_status == QDF_STATUS_SUCCESS)
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_FAULT;

	return QDF_STATUS_E_FAULT;
}

/**
 * wlansap_send_action() - send action frame
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pBuf: Pointer of the action frame to be transmitted
 * @len: Length of the action frame
 *
 * This api function provides to send action frame sent by upper layer.
 *
 * Return: The QDF_STATUS code associated with performing the operation
*          QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_send_action(void *pCtx, const uint8_t *pBuf,
	uint32_t len, uint16_t wait, uint16_t channel_freq)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("NULL hal pointer"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status =
		sme_send_action(hHal, pSapCtx->sessionId, pBuf, len, 0,
		0, channel_freq);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Send Action Frame");

	return QDF_STATUS_E_FAULT;
}

/**
 * wlansap_remain_on_channel() - set remain on channel
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @channel: Channel on which driver has to listen
 * @duration: Duration for which driver has to listen on specified channel
 * @callback: Callback function to be called once Listen is done.
 * @pContext: Context needs to be called in callback function.
 * @scan_id: scan identifier
 *
 * This api function provides to set Remain On channel on specified channel
 * for specified duration.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_remain_on_channel(void *pCtx,
	uint8_t channel, uint32_t duration, remainOnChanCallback callback,
	void *pContext, uint32_t *scan_id)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("NULL hal pointer"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status = sme_remain_on_channel(hHal, pSapCtx->sessionId,
				channel, duration, callback, pContext,
				true, scan_id);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Set Remain on Channel");

	return QDF_STATUS_E_FAULT;
}

/**
 * wlansap_cancel_remain_on_channel() - cancel remain on channel
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api cancel previous remain on channel request.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwie
 */
QDF_STATUS wlansap_cancel_remain_on_channel(void *pCtx,
	uint32_t scan_id)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("HAL pointer is null"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status =
		sme_cancel_remain_on_channel(hHal, pSapCtx->sessionId,
		scan_id);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Cancel Remain on Channel");

	return QDF_STATUS_E_FAULT;
}

QDF_STATUS wlan_sap_update_next_channel(void *ctx, uint8_t channel,
				       enum phy_ch_width chan_bw)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->dfs_vendor_channel = channel;
	sap_ctx->dfs_vendor_chan_bw = chan_bw;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_sap_set_pre_cac_status() - Set the pre cac status
 * @ctx: SAP context
 * @status: Status of pre cac
 * @handle: Global MAC handle
 *
 * Sets the pre cac status in the MAC context and updates the state
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_pre_cac_status(void *ctx, bool status,
					tHalHandle handle)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);
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

/**
 * wlan_sap_set_chan_before_pre_cac() - Save the channel before pre cac
 * @ctx: SAP context
 * @chan_before_pre_cac: Channel before pre cac
 *
 * Saves the channel that was in use before pre cac operation
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_chan_before_pre_cac(void *ctx,
					uint8_t chan_before_pre_cac)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_ctx->chan_before_pre_cac = chan_before_pre_cac;
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_sap_set_pre_cac_complete_status() - Sets pre cac complete status
 * @ctx: SAP context
 * @status: Status of pre cac complete
 *
 * Sets the status of pre cac i.e., whether pre cac is complete or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sap_set_pre_cac_complete_status(void *ctx, bool status)
{
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

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
		ptSapContext context =
			(ptSapContext) mac->sap.sapCtxList[i].pSapContext;
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
		ptSapContext context =
			(ptSapContext) mac->sap.sapCtxList[i].pSapContext;
		if (context && context->is_pre_cac_on) {
			*vdev_id = i;
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wlansap_register_mgmt_frame() - register management frame
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @frameType: frameType that needs to be registered with PE.
 * @matchData: Data pointer which should be matched after frame type is matched.
 * @matchLen: Length of the matchData
 *
 * HDD use this API to register specified type of frame with CORE stack.
 * On receiving such kind of frame CORE stack should pass this frame to HDD
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_register_mgmt_frame
	(void *pCtx,
	uint16_t frameType, uint8_t *matchData, uint16_t matchLen) {
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal pointer null"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status = sme_register_mgmt_frame(hHal, pSapCtx->sessionId,
						 frameType, matchData,
						 matchLen);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Register MGMT frame");

	return QDF_STATUS_E_FAULT;
}

/**
 * wlansap_de_register_mgmt_frame() - de register management frame
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @frameType: frameType that needs to be De-registered with PE.
 * @matchData: Data pointer which should be matched after frame type is matched.
 * @matchLen: Length of the matchData
 *
 * This API is used to deregister previously registered frame.
 *
 * Return: The QDF_STATUS code associated with performing the operation
 *         QDF_STATUS_SUCCESS:  Success and error code otherwise
 */
QDF_STATUS wlansap_de_register_mgmt_frame
	(void *pCtx,
	uint16_t frameType, uint8_t *matchData, uint16_t matchLen) {
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal pointer null"));
		return QDF_STATUS_E_FAULT;
	}

	qdf_ret_status =
		sme_deregister_mgmt_frame(hHal, pSapCtx->sessionId, frameType,
					  matchData, matchLen);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  "Failed to Deregister MGMT frame");

	return QDF_STATUS_E_FAULT;
}

/*==========================================================================
   FUNCTION   wlansap_channel_change_request

   DESCRIPTION
   This API is used to send an Indication to SME/PE to change the
   current operating channel to a different target channel.

   The Channel change will be issued by SAP under the following
   scenarios.
   1. A radar indication is received  during SAP CAC WAIT STATE and
      channel change is required.
   2. A radar indication is received during SAP STARTED STATE and
      channel change is required.
   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   pSapCtx: Pointer to cds global context structure

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS
wlansap_channel_change_request(void *pSapCtx, uint8_t target_channel)
{
	ptSapContext sapContext = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal mac_ctx = NULL;
	eCsrPhyMode phy_mode;
	struct ch_params *ch_params;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
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
	sapContext->csr_roamProfile.supported_rates.numRates = 0;
	sapContext->csr_roamProfile.extended_rates.numRates = 0;
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

/*==========================================================================

   FUNCTION    wlansap_start_beacon_req
   DESCRIPTION
   This API is used to send an Indication to SME/PE to start
   beaconing on the current operating channel.

   Brief:When SAP is started on DFS channel and when ADD BSS RESP is received
   LIM temporarily holds off Beaconing for SAP to do CAC WAIT. When
   CAC WAIT is done SAP resumes the Beacon Tx by sending a start beacon
   request to LIM.

   DEPENDENCIES
   NA.

   PARAMETERS

   IN
   pSapCtx: Pointer to cds global context structure

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS wlansap_start_beacon_req(void *pSapCtx)
{
	ptSapContext sapContext = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	uint8_t dfsCacWaitStatus = 0;
	tpAniSirGlobal pMac = NULL;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/* No Radar was found during CAC WAIT, So start Beaconing */
	if (pMac->sap.SapDfsInfo.sap_radar_found_status == false) {
		/* CAC Wait done without any Radar Detection */
		dfsCacWaitStatus = true;
		sapContext->pre_cac_complete = false;
		qdf_ret_status = sme_roam_start_beacon_req(hHal,
							   sapContext->bssid,
							   dfsCacWaitStatus);
		if (qdf_ret_status == QDF_STATUS_SUCCESS) {
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_E_FAULT;
}

/*==========================================================================
   FUNCTION    wlansap_dfs_send_csa_ie_request

   DESCRIPTION
   This API is used to send channel switch announcement request to PE
   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   pSapCtx: Pointer to cds global context structure

   RETURN VALUE
   The QDF_STATUS code associated with performing the operation

   QDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS wlansap_dfs_send_csa_ie_request(void *pSapCtx)
{
	ptSapContext sapContext = NULL;
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
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
				sapContext->bssid,
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

	wlan_reg_get_dfs_region(pMac->psoc, &dfs_region);

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

bool sap_is_auto_channel_select(void *pvos_gctx)
{
	ptSapContext sapcontext = CDS_GET_SAP_CB(pvos_gctx);

	if (NULL == sapcontext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid SAP pointer", __func__);
		return 0;
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

	wlan_reg_get_dfs_region(pMac->psoc, &dfs_region);

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
		if (bufferValid) {
			pConfig->probeRespBcnIEsLen = bufferLength;
			pConfig->pProbeRespBcnIEsBuffer = pBuffer;
		} else {
			qdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
			pConfig->probeRespBcnIEsLen = 0;
			pConfig->pProbeRespBcnIEsBuffer = NULL;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("No Probe Resp beacone IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_PROBE_RESP:
		if (bufferValid) {
			pConfig->probeRespIEsBufferLen = bufferLength;
			pConfig->pProbeRespIEsBuffer = pBuffer;
		} else {
			qdf_mem_free(pConfig->pProbeRespIEsBuffer);
			pConfig->probeRespIEsBufferLen = 0;
			pConfig->pProbeRespIEsBuffer = NULL;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				  FL("No Probe Response IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_ASSOC_RESP:
		if (bufferValid) {
			pConfig->assocRespIEsLen = bufferLength;
			pConfig->pAssocRespIEsBuffer = pBuffer;
		} else {
			qdf_mem_free(pConfig->pAssocRespIEsBuffer);
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
		qdf_mem_free(pConfig->pProbeRespIEsBuffer);
		pConfig->probeRespIEsBufferLen = 0;
		pConfig->pProbeRespIEsBuffer = NULL;
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_ASSOC_RESP:
		qdf_mem_free(pConfig->pAssocRespIEsBuffer);
		pConfig->assocRespIEsLen = 0;
		pConfig->pAssocRespIEsBuffer = NULL;
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_PROBE_BCN:
		qdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
		pConfig->probeRespBcnIEsLen = 0;
		pConfig->pProbeRespBcnIEsBuffer = NULL;
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

QDF_STATUS wlan_sap_set_vendor_acs(void *ctx, bool is_vendor_acs)
{
	ptSapContext sap_context = (ptSapContext) ctx;

	if (!sap_context) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return QDF_STATUS_E_FAULT;
	}
	sap_context->vendor_acs_enabled = is_vendor_acs;

	return QDF_STATUS_SUCCESS;
}
/**
 * wlansap_get_dfs_nol() - Get the DFS NOL
 * @pSapCtx: SAP context
 * @nol: Pointer to the NOL
 * @nol_len: Length of the NOL
 *
 * Provides the DFS NOL
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlansap_get_dfs_nol(void *pSapCtx, uint8_t *nol, uint32_t *nol_len)
{
	int i = 0, j = 0;
	ptSapContext sapContext = (ptSapContext) pSapCtx;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;
	uint64_t current_time, found_time, elapsed_time;
	unsigned long left_time;
	tSapDfsNolInfo *dfs_nol = NULL;
	bool bAvailable = false;
	*nol_len = 0;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	if (!pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL is empty", __func__);
		return QDF_STATUS_SUCCESS;
	}

	dfs_nol = pMac->sap.SapDfsInfo.sapDfsChannelNolList;

	if (!dfs_nol) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL context is null", __func__);
		return QDF_STATUS_E_FAULT;
	}

	for (i = 0; i < pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels;
	     i++) {
		if (!dfs_nol[i].dfs_channel_number)
			continue;

		current_time = cds_get_monotonic_boottime();
		found_time = dfs_nol[i].radar_found_timestamp;

		elapsed_time = current_time - found_time;

		/* check if channel is available
		 * if either channel is usable or available, or timer expired 30mins
		 */
		bAvailable =
			((dfs_nol[i].radar_status_flag ==
			  eSAP_DFS_CHANNEL_AVAILABLE)
			 || (dfs_nol[i].radar_status_flag ==
			     eSAP_DFS_CHANNEL_USABLE)
			 || (elapsed_time >= SAP_DFS_NON_OCCUPANCY_PERIOD));

		if (bAvailable) {
			dfs_nol[i].radar_status_flag =
				eSAP_DFS_CHANNEL_AVAILABLE;
			dfs_nol[i].radar_found_timestamp = 0;

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "%s: Channel[%d] is AVAILABLE",
				  __func__, dfs_nol[i].dfs_channel_number);
		} else {

			/* the time left in min */
			left_time = SAP_DFS_NON_OCCUPANCY_PERIOD - elapsed_time;
			left_time = left_time / (60 * 1000 * 1000);

			nol[j++] = dfs_nol[i].dfs_channel_number;
			(*nol_len)++;

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  "%s: Channel[%d] is UNAVAILABLE [%lu min left]",
				  __func__,
				  dfs_nol[i].dfs_channel_number, left_time);
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlansap_set_dfs_nol(void *psap_ctx, eSapDfsNolType conf)
{
	ptSapContext sap_ctx = (ptSapContext) psap_ctx;
	void *hal = NULL;
	tpAniSirGlobal mac = NULL;

	if (!sap_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return QDF_STATUS_E_FAULT;
	}

	mac = PMAC_STRUCT(hal);
	if (!mac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL is empty", __func__);
		return QDF_STATUS_SUCCESS;
	}

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
		dfs_clear_nol_channels(pdev);
	} else if (conf == eSAP_DFS_NOL_RANDOMIZE) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Randomize the DFS NOL", __func__);

	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: unsupport type %d", __func__, conf);
	}

	/* set DFS-NOL back to keep it update-to-date in CNSS */
	sap_signal_hdd_event(sap_ctx, NULL, eSAP_DFS_NOL_SET,
			  (void *) eSAP_STATUS_SUCCESS);

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
				     struct tagCsrDelStaParams *pDelStaParams)
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

/**
 * wlansap_acs_chselect() - Initiates acs channel selection
 * @pvos_gctx:                 Pointer to vos global context structure
 * @pacs_event_callback:       Callback function in hdd called by sap
 *                             to inform hdd about channel section result
 * @pconfig:                   Pointer to configuration structure
 *                             passed down from hdd
 * @pusr_context:              Parameter that will be passed back in all
 *                             the sap callback events.
 *
 * This function serves as an api for hdd to initiate acs scan pre
 * start bss.
 *
 * Return: The QDF_STATUS code associated with performing the operation.
 */
QDF_STATUS
wlansap_acs_chselect(void *pvos_gctx,
			tpWLAN_SAPEventCB pacs_event_callback,
			tsap_Config_t *pconfig,
			void *pusr_context)
{
	ptSapContext sap_context = NULL;
	tHalHandle h_hal = NULL;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pmac = NULL;

	sap_context = CDS_GET_SAP_CB(pvos_gctx);
	if (NULL == sap_context) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid SAP pointer from pvos_gctx", __func__);

		return QDF_STATUS_E_FAULT;
	}

	h_hal = (tHalHandle)CDS_GET_HAL_CB(sap_context->p_cds_gctx);
	if (NULL == h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid MAC context from pvosGCtx", __func__);
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
	qdf_status = sap_goto_channel_sel(sap_context, NULL, true, false);

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
void wlan_sap_enable_phy_error_logs(tHalHandle hal, bool enable_log)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	mac_ctx->sap.enable_dfs_phy_error_logs = enable_log;
}

/**
 * wlansap_get_chan_width() - get sap channel width.
 * @cds_ctx: pointer of global cds context
 *
 * This function get channel width of sap.
 *
 * Return: sap channel width
 */
uint32_t wlansap_get_chan_width(void *cds_ctx)
{
	ptSapContext sapcontext;

	sapcontext = CDS_GET_SAP_CB(cds_ctx);
	return wlan_sap_get_vht_ch_width(sapcontext);
}

/**
 * wlansap_set_tx_leakage_threshold() - set sap tx leakage threshold.
 * @hal: HAL pointer
 * @tx_leakage_threshold: sap tx leakage threshold
 *
 * This function set sap tx leakage threshold.
 *
 * Return: QDF_STATUS.
 */
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
	mac->sap.SapDfsInfo.tx_leakage_threshold = tx_leakage_threshold;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			"%s: leakage_threshold %d", __func__,
			mac->sap.SapDfsInfo.tx_leakage_threshold);
	return QDF_STATUS_SUCCESS;
}

/*
 * wlansap_set_invalid_session() - set session ID to invalid
 * @cds_ctx: pointer of global context
 *
 * This function sets session ID to invalid
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlansap_set_invalid_session(void *cds_ctx)
{
	ptSapContext psapctx;

	psapctx = CDS_GET_SAP_CB(cds_ctx);
	if (NULL == psapctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from pctx"));
		return QDF_STATUS_E_FAILURE;
	}

	psapctx->sessionId = CSR_SESSION_ID_INVALID;

	return QDF_STATUS_SUCCESS;
}
