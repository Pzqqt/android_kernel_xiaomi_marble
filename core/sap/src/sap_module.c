/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
#include "cdf_trace.h"
#include "cdf_util.h"
/* Pick up the sme callback registration API */
#include "sme_api.h"

/* SAP API header file */

#include "sap_internal.h"
#include "sme_inside.h"
#include "cds_ieee80211_common_i.h"
#include "cds_regdomain_common.h"
#include "cds_concurrency.h"

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
ptSapContext gp_sap_ctx;

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

/**
 * wlansap_open() - WLAN SAP open function call
 * @p_cds_gctx: Pointer to the global cds context; a handle to SAP's
 *
 * Called at driver initialization (cds_open). SAP will initialize
 * all its internal resources and will wait for the call to start to
 * register with the other modules.
 *
 * Return: The result code associated with performing the operation
 *
 * #ifdef WLAN_FEATURE_MBSSID
 *          void *: Pointer to the SAP context
 * #else
 *          CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL ;
 *                              access would cause a page fault
 *          CDF_STATUS_SUCCESS: Success
 *  #endif
 */
#ifdef WLAN_FEATURE_MBSSID
void *
#else
CDF_STATUS
#endif
wlansap_open(void *p_cds_gctx) {
	ptSapContext pSapCtx = NULL;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef WLAN_FEATURE_MBSSID
	/* amically allocate the sapContext */
	pSapCtx = (ptSapContext) cdf_mem_malloc(sizeof(tSapContext));
#else
	if (NULL == p_cds_gctx) {
		CDF_ASSERT(p_cds_gctx);
		return CDF_STATUS_E_NOMEM;
	}
	/*------------------------------------------------------------------------
	     Allocate (and sanity check?!) SAP control block
	   ------------------------------------------------------------------------*/
	cds_alloc_context(p_cds_gctx, CDF_MODULE_ID_SAP, (void **)&pSapCtx,
			  sizeof(tSapContext));
#endif

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
#ifdef WLAN_FEATURE_MBSSID
		return NULL;
#else
		return CDF_STATUS_E_FAULT;
#endif
	}


	/*------------------------------------------------------------------------
	    Clean up SAP control block, initialize all values
	   ------------------------------------------------------------------------*/
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH, "wlansap_open");

	wlansap_clean_cb(pSapCtx, 0); /*do not empty */

	/* Setup the "link back" to the CDS context */
	pSapCtx->p_cds_gctx = p_cds_gctx;

	/* Store a pointer to the SAP context provided by CDS */
	gp_sap_ctx = pSapCtx;

	/*------------------------------------------------------------------------
	    Allocate internal resources
	   ------------------------------------------------------------------------*/

#ifdef WLAN_FEATURE_MBSSID
	return pSapCtx;
#else
	return CDF_STATUS_SUCCESS;
#endif
} /* wlansap_open */

/**
 * wlansap_start() - wlan start SAP.
 * @pCtx: Pointer to the global cds context; a handle to SAP's
 *        control block can be extracted from its context
 *        When MBSSID feature is enabled, SAP context is directly
 *        passed to SAP APIs
 *
 * Called as part of the overall start procedure (cds_enable). SAP will
 * use this call to register with TL as the SAP entity for SAP RSN frames.
 *
 * Return: The result code associated with performing the operation
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault.
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_start(void *pCtx)
{
	ptSapContext pSapCtx = NULL;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_start invoked successfully");
	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
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

	/* Now configure the auth type in the roaming profile. To open. */
	pSapCtx->csr_roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;        /* open is the default */

	if (!CDF_IS_STATUS_SUCCESS(cdf_mutex_init(&pSapCtx->SapGlobalLock))) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "wlansap_start failed init lock");
		return CDF_STATUS_E_FAULT;
	}

	return CDF_STATUS_SUCCESS;
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
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault.
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_stop(void *pCtx)
{
	ptSapContext pSapCtx = NULL;

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_stop invoked successfully ");

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sap_free_roam_profile(&pSapCtx->csr_roamProfile);

	if (!CDF_IS_STATUS_SUCCESS(cdf_mutex_destroy(&pSapCtx->SapGlobalLock))) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "wlansap_stop failed destroy lock");
		return CDF_STATUS_E_FAULT;
	}
	/*------------------------------------------------------------------------
	    Stop SAP (de-register RSN handler!?)
	   ------------------------------------------------------------------------*/

	return CDF_STATUS_SUCCESS;
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
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_close(void *pCtx)
{
	ptSapContext pSapCtx = NULL;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_close invoked");

	pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	/*------------------------------------------------------------------------
	    Cleanup SAP control block.
	   ------------------------------------------------------------------------*/
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_close");
	wlansap_clean_cb(pSapCtx,
			 true); /* empty queues/lists/pkts if any */

#ifdef WLAN_FEATURE_MBSSID
	cdf_mem_free(pSapCtx);
#else
	/*------------------------------------------------------------------------
	    Free SAP context from CDS global
	   ------------------------------------------------------------------------*/
	cds_free_context(pCtx, CDF_MODULE_ID_SAP, pSapCtx);
#endif

	return CDF_STATUS_SUCCESS;
} /* wlansap_close */

/*----------------------------------------------------------------------------
 * Utility Function implementations
 * -------------------------------------------------------------------------*/

/**
 * wlansap_clean_cb() - clean SAP callback function.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * Clear out all fields in the SAP context.
 *
 * Return: The result code associated with performing the operation
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_clean_cb(ptSapContext pSapCtx, uint32_t freeFlag      /* 0 / *do not empty* /); */
			    ) {
	/*------------------------------------------------------------------------
	    Sanity check SAP control block
	   ------------------------------------------------------------------------*/

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	/*------------------------------------------------------------------------
	    Clean up SAP control block, initialize all values
	   ------------------------------------------------------------------------*/
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_clean_cb");

	cdf_mem_zero(pSapCtx, sizeof(tSapContext));

	pSapCtx->p_cds_gctx = NULL;

	pSapCtx->sapsMachine = eSAP_DISCONNECTED;

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Initializing State: %d, sapContext value = %p", __func__,
		  pSapCtx->sapsMachine, pSapCtx);
	pSapCtx->sessionId = 0;
	pSapCtx->channel = 0;
	pSapCtx->isSapSessionOpen = eSAP_FALSE;

	return CDF_STATUS_SUCCESS;
} /* wlansap_clean_cb */

/*==========================================================================
   FUNCTION    wlansap_pmc_full_pwr_req_cb

   DESCRIPTION
    Callback provide to PMC in the pmc_request_full_power API.

   DEPENDENCIES

   PARAMETERS

    IN
    callbackContext:  The user passed in a context to identify
    status:           The cdf_ret_status

   RETURN VALUE
    None

   SIDE EFFECTS
   ============================================================================*/
void
wlansap_pmc_full_pwr_req_cb(void *callbackContext, CDF_STATUS status)
{
	if (CDF_IS_STATUS_SUCCESS(status)) {
		/* If success what else to be handled??? */
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_FATAL,
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
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	return pSapCtx->sapsMachine;
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
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
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
CDF_STATUS
wlansap_set_scan_acs_channel_params(tsap_Config_t *pconfig,
				ptSapContext psap_ctx,
				void *pusr_context)
{
	tHalHandle h_hal = NULL;

	if (NULL == pconfig) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%s: Invalid pconfig passed ", __func__);
		return CDF_STATUS_E_FAULT;
	}

	if (NULL == psap_ctx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%s: Invalid pconfig passed ", __func__);
		return CDF_STATUS_E_FAULT;
	}

	/* Channel selection is auto or configured */
	psap_ctx->channel = pconfig->channel;
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

	/*
	 * Save a copy to SAP context
	 */
	cdf_mem_copy(psap_ctx->csr_roamProfile.BSSIDs.bssid,
		pconfig->self_macaddr.bytes, CDF_MAC_ADDR_SIZE);
	cdf_mem_copy(psap_ctx->self_mac_addr,
		pconfig->self_macaddr.bytes, CDF_MAC_ADDR_SIZE);

	h_hal = (tHalHandle)CDS_GET_HAL_CB(psap_ctx->p_cds_gctx);
	if (NULL == h_hal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
			"%s: Invalid MAC context from pvosGCtx", __func__);
	} else {
		/*
		* If concurrent session is running that is already associated
		* then we just follow that sessions country info (whether
		* present or not doesn't maater as we have to follow whatever
		* STA session does)
		*/
		if ((0 == sme_get_concurrent_operation_channel(h_hal)) &&
			pconfig->ieee80211d) {
			/* Setting the region/country  information */
			sme_set_reg_info(h_hal, pconfig->countryCode);
			sme_apply_channel_power_info_to_fw(h_hal);
		}
	}

	return CDF_STATUS_SUCCESS;
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
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
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
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
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
			hHal,
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
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_start_bss(void *pCtx,     /* pwextCtx */
			     tpWLAN_SAPEventCB pSapEventCallback,
			     tsap_Config_t *pConfig, void *pUsrContext) {
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	tHalHandle hHal;
	tpAniSirGlobal pmac = NULL;

	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);

		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
			  "wlansap_start_bss: sapContext=%p", pSapCtx);

		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		pSapCtx->sapsMachine = eSAP_DISCONNECTED;

		/* Channel selection is auto or configured */
		pSapCtx->channel = pConfig->channel;
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
		/* Set the BSSID to your "self MAC Addr" read the mac address from Configuation ITEM received from HDD */
		pSapCtx->csr_roamProfile.BSSIDs.numOfBSSIDs = 1;
		cdf_mem_copy(pSapCtx->csr_roamProfile.BSSIDs.bssid,
			     pSapCtx->self_mac_addr, sizeof(struct cdf_mac_addr));

		/* Save a copy to SAP context */
		cdf_mem_copy(pSapCtx->csr_roamProfile.BSSIDs.bssid,
			     pConfig->self_macaddr.bytes, CDF_MAC_ADDR_SIZE);
		cdf_mem_copy(pSapCtx->self_mac_addr,
			     pConfig->self_macaddr.bytes, CDF_MAC_ADDR_SIZE);

		/* copy the configuration items to csrProfile */
		sapconvert_to_csr_profile(pConfig, eCSR_BSS_TYPE_INFRA_AP,
				       &pSapCtx->csr_roamProfile);
		hHal = (tHalHandle) CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Invalid MAC context from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		} else {
			/* If concurrent session is running that is already associated */
			/* then we just follow that sessions country info (whether */
			/* present or not doesn't maater as we have to follow whatever */
			/* STA session does) */
			if ((0 == sme_get_concurrent_operation_channel(hHal)) &&
			    pConfig->ieee80211d) {
				/* Setting the region/country  information */
				sme_set_reg_info(hHal, pConfig->countryCode);
				sme_apply_channel_power_info_to_fw(hHal);
			}
		}

		pmac = PMAC_STRUCT(hHal);
		if (NULL == pmac) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Invalid MAC context from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		/*
		 * Copy the DFS Test Mode setting to pmac for
		 * access in lower layers
		 */
		pmac->sap.SapDfsInfo.disable_dfs_ch_switch =
					pConfig->disableDFSChSwitch;

		/* Copy MAC filtering settings to sap context */
		pSapCtx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;
		cdf_mem_copy(pSapCtx->acceptMacList, pConfig->accept_mac,
			     sizeof(pConfig->accept_mac));
		pSapCtx->nAcceptMac = pConfig->num_accept_mac;
		sap_sort_mac_list(pSapCtx->acceptMacList, pSapCtx->nAcceptMac);
		cdf_mem_copy(pSapCtx->denyMacList, pConfig->deny_mac,
			     sizeof(pConfig->deny_mac));
		pSapCtx->nDenyMac = pConfig->num_deny_mac;
		sap_sort_mac_list(pSapCtx->denyMacList, pSapCtx->nDenyMac);
		/* Fill in the event structure for FSM */
		sapEvent.event = eSAP_HDD_START_INFRA_BSS;
		sapEvent.params = 0;    /* pSapPhysLinkCreate */

		/* Store the HDD callback in SAP context */
		pSapCtx->pfnSapEventCallback = pSapEventCallback;

		/* Handle event */
		cdf_status = sap_fsm(pSapCtx, &sapEvent);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "SoftAp role has not been enabled");
	}

	return cdf_status;
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
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_set_mac_acl(void *pCtx,    /* pwextCtx */
			       tsap_Config_t *pConfig) {
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "wlansap_set_mac_acl");

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		/* Copy MAC filtering settings to sap context */
		pSapCtx->eSapMacAddrAclMode = pConfig->SapMacaddr_acl;

		if (eSAP_DENY_UNLESS_ACCEPTED == pSapCtx->eSapMacAddrAclMode) {
			cdf_mem_copy(pSapCtx->acceptMacList,
				     pConfig->accept_mac,
				     sizeof(pConfig->accept_mac));
			pSapCtx->nAcceptMac = pConfig->num_accept_mac;
			sap_sort_mac_list(pSapCtx->acceptMacList,
				       pSapCtx->nAcceptMac);
		} else if (eSAP_ACCEPT_UNLESS_DENIED ==
			   pSapCtx->eSapMacAddrAclMode) {
			cdf_mem_copy(pSapCtx->denyMacList, pConfig->deny_mac,
				     sizeof(pConfig->deny_mac));
			pSapCtx->nDenyMac = pConfig->num_deny_mac;
			sap_sort_mac_list(pSapCtx->denyMacList, pSapCtx->nDenyMac);
		}
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s : SoftAp role has not been enabled", __func__);
		return CDF_STATUS_E_FAULT;
	}

	return cdf_status;
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
 *         CDF_STATUS_E_FAULT: Pointer to SAP cb is NULL;
 *                             access would cause a page fault
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS wlansap_stop_bss(void *pCtx)
{
	tWLAN_SAPEvent sapEvent;        /* State machine event */
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	/*------------------------------------------------------------------------
	    Sanity check
	    Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid Global CDS handle", __func__);
		return CDF_STATUS_E_FAULT;
	}

	pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	/* Fill in the event structure for FSM */
	sapEvent.event = eSAP_HDD_STOP_INFRA_BSS;
	sapEvent.params = 0;

	/* Handle event */
	cdf_status = sap_fsm(pSapCtx, &sapEvent);

	return cdf_status;
}

/**
 * wlansap_get_assoc_stations() - get list of associated stations.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @modId: Module from whom list of associtated stations  is supposed to be
 *         probed. If an invalid module is passed then by default
 *         CDF_MODULE_ID_PE will be probed
 * @pAssocStas: Pointer to list of associated stations that are known to the
 *              module specified in mod parameter
 *
 * This api function is used to probe the list of associated stations from
 * various modules of CORE stack
 * NOTE: The memory for this list will be allocated by the caller of this API
 *
 * Return: The result code associated with performing the operation
 *         CDF_STATUS_SUCCESS: Success
 */
CDF_STATUS
wlansap_get_assoc_stations
	(void *pCtx, CDF_MODULE_ID modId, tpSap_AssocMacAddr pAssocStas) {
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sme_roam_get_associated_stas(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				     pSapCtx->sessionId, modId,
				     pSapCtx->pUsrContext,
				     (void **) pSapCtx->pfnSapEventCallback,
				     (uint8_t *) pAssocStas);

	return CDF_STATUS_SUCCESS;
}

/**
 * wlansap_remove_wps_session_overlap() - remove overlapping wps session.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pRemoveMac: pointer to struct cdf_mac_addr for session MAC address
 *
 * This api function provides for Ap App/HDD to remove an entry from session
 * overlap info.
 *
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 *         CDF_STATUS_E_FAULT:  Session is not dectected.
 *                              The parameter is function not valid.
 */
CDF_STATUS
wlansap_remove_wps_session_overlap(void *pCtx,
				   struct cdf_mac_addr pRemoveMac)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sme_roam_get_wps_session_overlap(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
					 pSapCtx->sessionId, pSapCtx->pUsrContext,
					 (void **) pSapCtx->pfnSapEventCallback,
					 pRemoveMac);

	return CDF_STATUS_SUCCESS;
}

/**
 * wlansap_get_wps_session_overlap() - get overlapping wps session.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api function provides for Ap App/HDD to get WPS session overlap info.
 *
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS wlansap_get_wps_session_overlap(void *pCtx)
{
	struct cdf_mac_addr pRemoveMac = CDF_MAC_ADDR_ZERO_INITIALIZER;

	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sme_roam_get_wps_session_overlap(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
					 pSapCtx->sessionId, pSapCtx->pUsrContext,
					 (void **) pSapCtx->pfnSapEventCallback,
					 pRemoveMac);

	return CDF_STATUS_SUCCESS;
}

/* This routine will set the mode of operation for ACL dynamically*/
CDF_STATUS wlansap_set_mode(void *pCtx, uint32_t mode)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	pSapCtx->eSapMacAddrAclMode = (eSapMacAddrACL) mode;
	return CDF_STATUS_SUCCESS;
}

/* Get ACL Mode */
CDF_STATUS wlansap_get_acl_mode(void *pCtx, eSapMacAddrACL *mode)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	*mode = pSapCtx->eSapMacAddrAclMode;
	return CDF_STATUS_SUCCESS;
}

/* API to get ACL Accept List */
CDF_STATUS
wlansap_get_acl_accept_list(void *pCtx, struct cdf_mac_addr *pAcceptList,
				uint8_t *nAcceptList)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	memcpy((void *)pAcceptList, (void *)pSapCtx->acceptMacList,
	       (pSapCtx->nAcceptMac * CDF_MAC_ADDR_SIZE));
	*nAcceptList = pSapCtx->nAcceptMac;
	return CDF_STATUS_SUCCESS;
}

/* API to get Deny List */
CDF_STATUS
wlansap_get_acl_deny_list(void *pCtx, struct cdf_mac_addr *pDenyList,
			  uint8_t *nDenyList)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	memcpy((void *)pDenyList, (void *)pSapCtx->denyMacList,
	       (pSapCtx->nDenyMac * CDF_MAC_ADDR_SIZE));
	*nDenyList = pSapCtx->nDenyMac;
	return CDF_STATUS_SUCCESS;
}

/* This routine will clear all the entries in accept list as well as deny list  */

CDF_STATUS wlansap_clear_acl(void *pCtx)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);
	uint8_t i;

	if (NULL == pSapCtx) {
		return CDF_STATUS_E_RESOURCES;
	}

	if (pSapCtx->denyMacList != NULL) {
		for (i = 0; i < (pSapCtx->nDenyMac - 1); i++) {
			cdf_mem_zero((pSapCtx->denyMacList + i)->bytes,
				     CDF_MAC_ADDR_SIZE);

		}
	}
	sap_print_acl(pSapCtx->denyMacList, pSapCtx->nDenyMac);
	pSapCtx->nDenyMac = 0;

	if (pSapCtx->acceptMacList != NULL) {
		for (i = 0; i < (pSapCtx->nAcceptMac - 1); i++) {
			cdf_mem_zero((pSapCtx->acceptMacList + i)->bytes,
				     CDF_MAC_ADDR_SIZE);

		}
	}
	sap_print_acl(pSapCtx->acceptMacList, pSapCtx->nAcceptMac);
	pSapCtx->nAcceptMac = 0;

	return CDF_STATUS_SUCCESS;
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

CDF_STATUS
wlansap_modify_acl
	(void *ctx,
	uint8_t *peer_sta_mac, eSapACLType list_type, eSapACLCmdType cmd) {
	eSapBool sta_white_list = eSAP_FALSE, sta_black_list = eSAP_FALSE;
	uint8_t staWLIndex, staBLIndex;
	ptSapContext sap_ctx = CDS_GET_SAP_CB(ctx);

	if (NULL == sap_ctx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP Context", __func__);
		return CDF_STATUS_E_FAULT;
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_LOW,
		  "Modify ACL entered\n" "Before modification of ACL\n"
		  "size of accept and deny lists %d %d", sap_ctx->nAcceptMac,
		  sap_ctx->nDenyMac);
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "*** WHITE LIST ***");
	sap_print_acl(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
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
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "Peer mac " MAC_ADDRESS_STR
			  " found in white and black lists."
			  "Initial lists passed incorrect. Cannot execute this command.",
			  MAC_ADDR_ARRAY(peer_sta_mac));
		return CDF_STATUS_E_FAILURE;

	}
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_LOW,
		  "cmd %d", cmd);

	switch (list_type) {
	case eSAP_WHITE_LIST:
		if (cmd == ADD_STA_TO_ACL) {
			/* error check */
			/* if list is already at max, return failure */
			if (sap_ctx->nAcceptMac == MAX_ACL_MAC_ADDRESS) {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_ERROR,
					  "White list is already maxed out. Cannot accept "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_E_FAILURE;
			}
			if (sta_white_list) {
				/* Do nothing if already present in white list. Just print a warning */
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
					  "MAC address already present in white list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_SUCCESS;
			}
			if (sta_black_list) {
				/* remove it from black list before adding to the white list */
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
					  "STA present in black list so first remove from it");
				sap_remove_mac_from_acl(sap_ctx->
						    denyMacList,
						    &sap_ctx->nDenyMac,
						    staBLIndex);
			}
			CDF_TRACE(CDF_MODULE_ID_SAP,
				  CDF_TRACE_LEVEL_INFO,
				  "... Now add to the white list");
			sap_add_mac_to_acl(sap_ctx->acceptMacList,
					       &sap_ctx->nAcceptMac,
			       peer_sta_mac);
				CDF_TRACE(CDF_MODULE_ID_SAP,
				  CDF_TRACE_LEVEL_INFO_LOW,
				  "size of accept and deny lists %d %d",
				  sap_ctx->nAcceptMac,
				  sap_ctx->nDenyMac);
		} else if (cmd == DELETE_STA_FROM_ACL) {
			if (sta_white_list) {

				struct tagCsrDelStaParams delStaParams;

				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_INFO,
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
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_INFO_LOW,
					  "size of accept and deny lists %d %d",
					  sap_ctx->nAcceptMac,
					  sap_ctx->nDenyMac);
			} else {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
					  "MAC address to be deleted is not present in the white list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_E_FAILURE;
			}
		} else {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "Invalid cmd type passed");
			return CDF_STATUS_E_FAILURE;
		}
		break;

	case eSAP_BLACK_LIST:

		if (cmd == ADD_STA_TO_ACL) {
			struct tagCsrDelStaParams delStaParams;
			/* error check */
			/* if list is already at max, return failure */
			if (sap_ctx->nDenyMac == MAX_ACL_MAC_ADDRESS) {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_ERROR,
					  "Black list is already maxed out. Cannot accept "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_E_FAILURE;
			}
			if (sta_black_list) {
				/* Do nothing if already present in white list */
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
					  "MAC address already present in black list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_SUCCESS;
			}
			if (sta_white_list) {
				/* remove it from white list before adding to the black list */
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
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
			CDF_TRACE(CDF_MODULE_ID_SAP,
				  CDF_TRACE_LEVEL_INFO,
				  "... Now add to black list");
			sap_add_mac_to_acl(sap_ctx->denyMacList,
				       &sap_ctx->nDenyMac, peer_sta_mac);
			CDF_TRACE(CDF_MODULE_ID_SAP,
				  CDF_TRACE_LEVEL_INFO_LOW,
				  "size of accept and deny lists %d %d",
				  sap_ctx->nAcceptMac,
				  sap_ctx->nDenyMac);
		} else if (cmd == DELETE_STA_FROM_ACL) {
			if (sta_black_list) {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_INFO,
					  "Delete from black list");
				sap_remove_mac_from_acl(sap_ctx->denyMacList,
						    &sap_ctx->nDenyMac,
						    staBLIndex);
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_INFO_LOW,
					  "no accept and deny mac %d %d",
					  sap_ctx->nAcceptMac,
					  sap_ctx->nDenyMac);
			} else {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					  CDF_TRACE_LEVEL_WARN,
					  "MAC address to be deleted is not present in the black list "
					  MAC_ADDRESS_STR,
					  MAC_ADDR_ARRAY(peer_sta_mac));
				return CDF_STATUS_E_FAILURE;
			}
		} else {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "Invalid cmd type passed");
			return CDF_STATUS_E_FAILURE;
		}
		break;

	default:
	{
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "Invalid list type passed %d", list_type);
		return CDF_STATUS_E_FAILURE;
	}
	}
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_LOW,
		  "After modification of ACL");
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "*** WHITE LIST ***");
	sap_print_acl(sap_ctx->acceptMacList, sap_ctx->nAcceptMac);
	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "*** BLACK LIST ***");
	sap_print_acl(sap_ctx->denyMacList, sap_ctx->nDenyMac);
	return CDF_STATUS_SUCCESS;
}

/**
 * wlansap_disassoc_sta() - initiate disassociation of station.
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 * @pPeerStaMac: Mac address of the station to disassociate
 *
 * This api function provides for Ap App/HDD initiated disassociation of station
 *
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS wlansap_disassoc_sta(void *pCtx, const uint8_t *pPeerStaMac)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sme_roam_disconnect_sta(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				pSapCtx->sessionId, pPeerStaMac);

	return CDF_STATUS_SUCCESS;
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
 * Return: The CDF_STATUS code associated with performing the operation
 */
CDF_STATUS wlansap_deauth_sta(void *pCtx,
			      struct tagCsrDelStaParams *pDelStaParams)
{
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	CDF_STATUS cdf_status = CDF_STATUS_E_FAULT;
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return cdf_status;
	}

	cdf_ret_status =
		sme_roam_deauth_sta(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				    pSapCtx->sessionId, pDelStaParams);

	if (cdf_ret_status == CDF_STATUS_SUCCESS) {
		cdf_status = CDF_STATUS_SUCCESS;
	}
	return cdf_status;
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
 * Return: The CDF_STATUS code associated with performing the operation
 */
static CDF_STATUS wlansap_update_csa_channel_params(ptSapContext sap_context,
	uint32_t channel)
{
	void *hal;
	tpAniSirGlobal mac_ctx;
	tSmeConfigParams *sme_config;
	uint8_t bw;

	hal = CDS_GET_HAL_CB(sap_context->p_cds_gctx);
	if (!hal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%s: Invalid hal pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	mac_ctx = PMAC_STRUCT(hal);

	sme_config = cdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			FL("memory allocation failed for sme_config"));
		return CDF_STATUS_E_NOMEM;
	}

	sme_get_config_param(mac_ctx, sme_config);
	if (channel <= RF_CHAN_14) {
		/*
		 * currently OBSS scan is done in hostapd, so to avoid
		 * SAP coming up in HT40 on channel switch we are
		 * disabling channel bonding in 2.4Ghz.
		 */
		sme_config->csrConfig.channelBondingMode24GHz =
			eCSR_INI_SINGLE_CHANNEL_CENTERED;
		mac_ctx->sap.SapDfsInfo.new_cbMode = 0;
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

			op_class = cds_regdm_get_opclass_from_channel(
					mac_ctx->scan.countryCodeCurrent,
					channel, bw);
			if (!op_class)
				continue;

			if (bw == BW80) {
				wlansap_update_bw80_cbmode(channel, sme_config);
				mac_ctx->sap.SapDfsInfo.new_cbMode =
				   sme_config->csrConfig.channelBondingMode5GHz;
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
					CH_WIDTH_80MHZ;
			} else if (bw == BW40_HIGH_PRIMARY) {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
					CH_WIDTH_40MHZ;
				sme_config->csrConfig.channelBondingMode5GHz =
				mac_ctx->sap.SapDfsInfo.new_cbMode =
				   eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY;
			} else if (bw == BW40_LOW_PRIMARY) {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
				   CH_WIDTH_40MHZ;
				sme_config->csrConfig.channelBondingMode5GHz =
				mac_ctx->sap.SapDfsInfo.new_cbMode =
				   eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY;
			} else {
				mac_ctx->sap.SapDfsInfo.new_chanWidth =
				   CH_WIDTH_20MHZ;
				sme_config->csrConfig.channelBondingMode5GHz =
				mac_ctx->sap.SapDfsInfo.new_cbMode =
				   eCSR_INI_SINGLE_CHANNEL_CENTERED;
			}
			break;
		}

	}

	sme_update_config(mac_ctx, sme_config);
	cdf_mem_free(sme_config);
	return CDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    wlansap_set_channel_change_with_csa

   DESCRIPTION
      This api function does a channel change to the target channel specified
      through an iwpriv. CSA IE is included in the beacons before doing a
      channel change.

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    p_cds_gctx             : Pointer to cds global context structure
    targetChannel        : New target channel to change to.

   RETURN VALUE
    The CDF_STATUS code associated with performing the operation

    CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS
wlansap_set_channel_change_with_csa(void *p_cds_gctx, uint32_t targetChannel)
{

	ptSapContext sapContext = NULL;
	tWLAN_SAPEvent sapEvent;
	tpAniSirGlobal pMac = NULL;
	void *hHal = NULL;
	bool valid;
	CDF_STATUS status;

	sapContext = CDS_GET_SAP_CB(p_cds_gctx);
	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);

		return CDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/*
	 * Now, validate if the passed channel is valid in the
	 * current regulatory domain.
	 */
	if (sapContext->channel != targetChannel &&
		((cds_get_channel_state(targetChannel) ==
			CHANNEL_STATE_ENABLE) ||
		(cds_get_channel_state(targetChannel) ==
			CHANNEL_STATE_DFS &&
		!cds_concurrent_open_sessions_running()))) {
		/*
		 * validate target channel switch w.r.t various concurrency
		 * rules set.
		 */
		valid = wlan_sap_validate_channel_switch(hHal, targetChannel,
				sapContext);
		if (!valid) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
					FL("Channel switch to %u is not allowed due to concurrent channel interference"),
					targetChannel);
			return CDF_STATUS_E_FAULT;
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
			if (status != CDF_STATUS_SUCCESS)
				return status;

			/*
			 * Copy the requested target channel
			 * to sap context.
			 */
			pMac->sap.SapDfsInfo.target_channel = targetChannel;
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
			 * Post the eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START
			 * to SAP state machine to process the channel
			 * request with CSA IE set in the beacons.
			 */
			sapEvent.event =
				eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START;
			sapEvent.params = 0;
			sapEvent.u1 = 0;
			sapEvent.u2 = 0;

			sap_fsm(sapContext, &sapEvent);

		} else {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Failed to request Channel Change, since"
				  "SAP is not in eSAP_STARTED state", __func__);
			return CDF_STATUS_E_FAULT;
		}

	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Channel = %d is not valid in the current"
			  "regulatory domain", __func__, targetChannel);

		return CDF_STATUS_E_FAULT;
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Posted eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START"
		  "successfully to sap_fsm for Channel = %d",
		  __func__, targetChannel);

	return CDF_STATUS_SUCCESS;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS wlansap_set_counter_measure(void *pCtx, bool bEnable)
{
	ptSapContext pSapCtx = CDS_GET_SAP_CB(pCtx);

	/*------------------------------------------------------------------------
	   Sanity check
	   Extract SAP control block
	   ------------------------------------------------------------------------*/
	if (NULL == pSapCtx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from pCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	sme_roam_tkip_counter_measures(CDS_GET_HAL_CB(pSapCtx->p_cds_gctx),
				       pSapCtx->sessionId, bEnable);

	return CDF_STATUS_SUCCESS;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS wlansap_set_key_sta(void *pCtx, tCsrRoamSetKey *pSetKeyInfo)
{
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	uint32_t roamId = 0xFF;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid HAL pointer from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		cdf_ret_status =
			sme_roam_set_key(hHal, pSapCtx->sessionId, pSetKeyInfo,
					 &roamId);

		if (cdf_ret_status == CDF_STATUS_SUCCESS) {
			cdf_status = CDF_STATUS_SUCCESS;
		} else {
			cdf_status = CDF_STATUS_E_FAULT;
		}
	} else
		cdf_status = CDF_STATUS_E_FAULT;

	return cdf_status;
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
 * Return: CDF_STATUS enumeration
 */

CDF_STATUS
wlan_sap_getstation_ie_information
	(void *ctx, uint32_t *len, uint8_t *buf) {
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	ptSapContext sap_ctx = NULL;
	uint32_t ie_len = 0;
	tCDF_CON_MODE mode;

	mode = cds_get_conparam();
	if (CDF_SAP_MODE != mode) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%d: called in wrong mode ",
			mode);
		return CDF_STATUS_E_FAULT;
	}

	sap_ctx = CDS_GET_SAP_CB(ctx);
	if (NULL == sap_ctx) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			FL("Invalid SAP pointer from pCtx"));
		return CDF_STATUS_E_FAULT;
	}

	if (len) {
		ie_len = *len;
		*len = sap_ctx->nStaWPARSnReqIeLength;
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
			FL("WPAIE len : %x"), *len);
		if ((buf) && (ie_len >= sap_ctx->nStaWPARSnReqIeLength)) {
			cdf_mem_copy(buf,
				sap_ctx->pStaWpaRsnReqIE,
				sap_ctx->nStaWPARSnReqIeLength);
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
				FL("WPAIE: %02x:%02x:%02x:%02x:%02x:%02x"),
				buf[0], buf[1], buf[2], buf[3], buf[4],
				buf[5]);
			cdf_status = CDF_STATUS_SUCCESS;
		}
	}
	return cdf_status;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
CDF_STATUS wlansap_set_wps_ie(void *pCtx, tSap_WPSIE *pSap_WPSIe)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
		  "%s, %d", __func__, __LINE__);

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid HAL pointer from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		if (sap_acquire_global_lock(pSapCtx) == CDF_STATUS_SUCCESS) {
			if (pSap_WPSIe->sapWPSIECode == eSAP_WPS_BEACON_IE) {
				cdf_mem_copy(&pSapCtx->APWPSIEs.SirWPSBeaconIE,
					     &pSap_WPSIe->sapwpsie.
					     sapWPSBeaconIE,
					     sizeof(tSap_WPSBeaconIE));
			} else if (pSap_WPSIe->sapWPSIECode ==
				   eSAP_WPS_PROBE_RSP_IE) {
				cdf_mem_copy(&pSapCtx->APWPSIEs.
					     SirWPSProbeRspIE,
					     &pSap_WPSIe->sapwpsie.
					     sapWPSProbeRspIE,
					     sizeof(tSap_WPSProbeRspIE));
			} else {
				sap_release_global_lock(pSapCtx);
				return CDF_STATUS_E_FAULT;
			}
			sap_release_global_lock(pSapCtx);
			return CDF_STATUS_SUCCESS;
		} else
			return CDF_STATUS_E_FAULT;
	} else
		return CDF_STATUS_E_FAULT;
}

/**
 * wlansap_update_wps_ie() - update WPI IE
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api function provides API for App/HDD to update WPS IE.
 *
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
CDF_STATUS wlansap_update_wps_ie(void *pCtx)
{
	CDF_STATUS cdf_status = CDF_STATUS_E_FAULT;
	ptSapContext pSapCtx = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "%s, %d", __func__, __LINE__);

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid HAL pointer from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status =
			sme_roam_update_apwpsie(hHal, pSapCtx->sessionId,
						&pSapCtx->APWPSIEs);

		if (cdf_ret_status == CDF_STATUS_SUCCESS) {
			cdf_status = CDF_STATUS_SUCCESS;
		} else {
			cdf_status = CDF_STATUS_E_FAULT;
		}

	}

	return cdf_status;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS wlansap_get_wps_state(void *pCtx, bool *bWPSState)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
		  "%s, %d", __func__, __LINE__);

	if (CDF_SAP_MODE == cds_get_conparam()) {

		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid HAL pointer from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		if (sap_acquire_global_lock(pSapCtx) == CDF_STATUS_SUCCESS) {
			if (pSapCtx->APWPSIEs.SirWPSProbeRspIE.
			    FieldPresent &
			    SIR_WPS_PROBRSP_SELECTEDREGISTRA_PRESENT)
				*bWPSState = true;
			else
				*bWPSState = false;

			sap_release_global_lock(pSapCtx);

			return CDF_STATUS_SUCCESS;
		} else
			return CDF_STATUS_E_FAULT;
	} else
		return CDF_STATUS_E_FAULT;

}

CDF_STATUS sap_acquire_global_lock(ptSapContext pSapCtx)
{
	CDF_STATUS cdf_status = CDF_STATUS_E_FAULT;

	if (CDF_IS_STATUS_SUCCESS(cdf_mutex_acquire(&pSapCtx->SapGlobalLock))) {
		cdf_status = CDF_STATUS_SUCCESS;
	}

	return cdf_status;
}

CDF_STATUS sap_release_global_lock(ptSapContext pSapCtx)
{
	CDF_STATUS cdf_status = CDF_STATUS_E_FAULT;

	if (CDF_IS_STATUS_SUCCESS(cdf_mutex_release(&pSapCtx->SapGlobalLock))) {
		cdf_status = CDF_STATUS_SUCCESS;
	}

	return cdf_status;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise
 */
CDF_STATUS wlansap_set_wparsn_ies
	(void *pCtx, uint8_t *pWPARSNIEs, uint32_t WPARSNIEsLen) {
	ptSapContext pSapCtx = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if (NULL == hHal) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid HAL pointer from p_cds_gctx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}

		pSapCtx->APWPARSNIEs.length = (uint16_t) WPARSNIEsLen;
		cdf_mem_copy(pSapCtx->APWPARSNIEs.rsnIEdata, pWPARSNIEs,
			     WPARSNIEsLen);

		cdf_ret_status =
			sme_roam_update_apwparsni_es(hHal, pSapCtx->sessionId,
						     &pSapCtx->APWPARSNIEs);

		if (cdf_ret_status == CDF_STATUS_SUCCESS) {
			return CDF_STATUS_SUCCESS;
		} else {
			return CDF_STATUS_E_FAULT;
		}
	}

	return CDF_STATUS_E_FAULT;
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
 * Return: The CDF_STATUS code associated with performing the operation
*          CDF_STATUS_SUCCESS:  Success and error code otherwise
 */
CDF_STATUS wlansap_send_action(void *pCtx, const uint8_t *pBuf,
	uint32_t len, uint16_t wait, uint16_t channel_freq)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if ((NULL == hHal) || (eSAP_TRUE != pSapCtx->isSapSessionOpen)) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: HAL pointer (%p) NULL OR SME session is not open (%d)",
				  __func__, hHal, pSapCtx->isSapSessionOpen);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status =
			sme_send_action(hHal, pSapCtx->sessionId, pBuf, len, 0,
			0, channel_freq);

		if (CDF_STATUS_SUCCESS == cdf_ret_status) {
			return CDF_STATUS_SUCCESS;
		}
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "Failed to Send Action Frame");

	return CDF_STATUS_E_FAULT;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise
 */
CDF_STATUS wlansap_remain_on_channel(void *pCtx,
	uint8_t channel, uint32_t duration, remainOnChanCallback callback,
	void *pContext, uint32_t *scan_id)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if ((NULL == hHal) || (eSAP_TRUE != pSapCtx->isSapSessionOpen)) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: HAL pointer (%p) NULL OR SME session is not open (%d)",
				  __func__, hHal, pSapCtx->isSapSessionOpen);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status = sme_remain_on_channel(hHal, pSapCtx->sessionId,
					channel, duration, callback, pContext,
					true, scan_id);

		if (CDF_STATUS_SUCCESS == cdf_ret_status) {
			return CDF_STATUS_SUCCESS;
		}
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "Failed to Set Remain on Channel");

	return CDF_STATUS_E_FAULT;
}

/**
 * wlansap_cancel_remain_on_channel() - cancel remain on channel
 * @pCtx: Pointer to the global cds context; a handle to SAP's control block
 *        can be extracted from its context. When MBSSID feature is enabled,
 *        SAP context is directly passed to SAP APIs.
 *
 * This api cancel previous remain on channel request.
 *
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwie
 */
CDF_STATUS wlansap_cancel_remain_on_channel(void *pCtx,
	uint32_t scan_id)
{
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if ((NULL == hHal) ||
			(eSAP_TRUE != pSapCtx->isSapSessionOpen)) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: HAL pointer (%p) NULL OR SME session is not open (%d)",
				  __func__, hHal, pSapCtx->isSapSessionOpen);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status =
			sme_cancel_remain_on_channel(hHal, pSapCtx->sessionId,
			scan_id);

		if (CDF_STATUS_SUCCESS == cdf_ret_status) {
			return CDF_STATUS_SUCCESS;
		}
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "Failed to Cancel Remain on Channel");

	return CDF_STATUS_E_FAULT;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise
 */
CDF_STATUS wlansap_register_mgmt_frame
	(void *pCtx,
	uint16_t frameType, uint8_t *matchData, uint16_t matchLen) {
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if ((NULL == hHal) || (eSAP_TRUE != pSapCtx->isSapSessionOpen)) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: HAL pointer (%p) NULL OR SME session is not open (%d)",
				  __func__, hHal, pSapCtx->isSapSessionOpen);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status = sme_register_mgmt_frame(hHal, pSapCtx->sessionId,
							 frameType, matchData,
							 matchLen);

		if (CDF_STATUS_SUCCESS == cdf_ret_status) {
			return CDF_STATUS_SUCCESS;
		}
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "Failed to Register MGMT frame");

	return CDF_STATUS_E_FAULT;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise
 */
CDF_STATUS wlansap_de_register_mgmt_frame
	(void *pCtx,
	uint16_t frameType, uint8_t *matchData, uint16_t matchLen) {
	ptSapContext pSapCtx = NULL;
	void *hHal = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;

	if (CDF_SAP_MODE == cds_get_conparam()) {
		pSapCtx = CDS_GET_SAP_CB(pCtx);
		if (NULL == pSapCtx) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Invalid SAP pointer from pCtx",
				  __func__);
			return CDF_STATUS_E_FAULT;
		}
		hHal = CDS_GET_HAL_CB(pSapCtx->p_cds_gctx);
		if ((NULL == hHal) || (eSAP_TRUE != pSapCtx->isSapSessionOpen)) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: HAL pointer (%p) NULL OR SME session is not open (%d)",
				  __func__, hHal, pSapCtx->isSapSessionOpen);
			return CDF_STATUS_E_FAULT;
		}

		cdf_ret_status =
			sme_deregister_mgmt_frame(hHal, pSapCtx->sessionId, frameType,
						  matchData, matchLen);

		if (CDF_STATUS_SUCCESS == cdf_ret_status) {
			return CDF_STATUS_SUCCESS;
		}
	}

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
		  "Failed to Deregister MGMT frame");

	return CDF_STATUS_E_FAULT;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS
wlansap_channel_change_request(void *pSapCtx, uint8_t target_channel)
{
	ptSapContext sapContext = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal mac_ctx = NULL;
	eCsrPhyMode phy_mode;
	uint32_t cb_mode;
	uint32_t vht_channel_width;
	chan_params_t ch_params;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	mac_ctx = PMAC_STRUCT(hHal);
	phy_mode = sapContext->csr_roamProfile.phyMode;
	sapContext->csr_roamProfile.ChannelInfo.ChannelList[0] = target_channel;
	/*
	 * We are getting channel bonding mode from sapDfsInfor structure
	 * because we've implemented channel width fallback mechanism for DFS
	 * which will result in channel width changing dynamically.
	 */
	cb_mode = mac_ctx->sap.SapDfsInfo.new_cbMode;
	vht_channel_width = mac_ctx->sap.SapDfsInfo.new_chanWidth;
	ch_params.ch_width = vht_channel_width;
	sme_set_ch_params(hHal, phy_mode, target_channel, 0, &ch_params);
	sapContext->ch_params.ch_width = vht_channel_width;
	/* Update the channel as this will be used to
	 * send event to supplicant
	 */
	sapContext->channel = target_channel;
	sapContext->csr_roamProfile.ch_params.ch_width = vht_channel_width;
	cdf_ret_status = sme_roam_channel_change_req(hHal, sapContext->bssid,
				cb_mode, &sapContext->csr_roamProfile);

	if (cdf_ret_status == CDF_STATUS_SUCCESS) {
		sap_signal_hdd_event(sapContext, NULL,
			eSAP_CHANNEL_CHANGE_EVENT,
			(void *) eSAP_STATUS_SUCCESS);

		return CDF_STATUS_SUCCESS;
	}
	return CDF_STATUS_E_FAULT;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_start_beacon_req(void *pSapCtx)
{
	ptSapContext sapContext = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	uint8_t dfsCacWaitStatus = 0;
	tpAniSirGlobal pMac = NULL;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/* No Radar was found during CAC WAIT, So start Beaconing */
	if (pMac->sap.SapDfsInfo.sap_radar_found_status == false) {
		/* CAC Wait done without any Radar Detection */
		dfsCacWaitStatus = true;
		cdf_ret_status = sme_roam_start_beacon_req(hHal,
							   sapContext->bssid,
							   dfsCacWaitStatus);
		if (cdf_ret_status == CDF_STATUS_SUCCESS) {
			return CDF_STATUS_SUCCESS;
		}
		return CDF_STATUS_E_FAULT;
	}

	return CDF_STATUS_E_FAULT;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_dfs_send_csa_ie_request(void *pSapCtx)
{
	ptSapContext sapContext = NULL;
	CDF_STATUS cdf_ret_status = CDF_STATUS_E_FAILURE;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;
	uint32_t cbmode, vht_ch_width;
	uint8_t ch_bandwidth;
	sapContext = (ptSapContext) pSapCtx;

	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/*
	 * We are getting channel bonding mode from sapDfsInfor structure
	 * because we've implemented channel width fallback mechanism for DFS
	 * which will result in channel width changing dynamically.
	 */
	vht_ch_width = pMac->sap.SapDfsInfo.new_chanWidth;
	cbmode = pMac->sap.SapDfsInfo.new_cbMode;

	if (pMac->sap.SapDfsInfo.target_channel <= 14 ||
		vht_ch_width == eHT_CHANNEL_WIDTH_40MHZ ||
		vht_ch_width == eHT_CHANNEL_WIDTH_20MHZ) {
		switch (cbmode) {
		case eCSR_INI_DOUBLE_CHANNEL_HIGH_PRIMARY:
			ch_bandwidth = BW40_HIGH_PRIMARY;
			break;
		case eCSR_INI_DOUBLE_CHANNEL_LOW_PRIMARY:
			ch_bandwidth = BW40_LOW_PRIMARY;
			break;
		case eCSR_INI_SINGLE_CHANNEL_CENTERED:
		default:
			ch_bandwidth = BW20;
			break;
		}
	} else {
		ch_bandwidth = BW80;
	}

	cdf_ret_status = sme_roam_csa_ie_request(hHal,
				sapContext->bssid,
				pMac->sap.SapDfsInfo.target_channel,
				pMac->sap.SapDfsInfo.csaIERequired,
				ch_bandwidth);

	if (cdf_ret_status == CDF_STATUS_SUCCESS) {
		return CDF_STATUS_SUCCESS;
	}

	return CDF_STATUS_E_FAULT;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_get_dfs_ignore_cac(tHalHandle hHal, uint8_t *pIgnore_cac)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	*pIgnore_cac = pMac->sap.SapDfsInfo.ignore_cac;
	return CDF_STATUS_SUCCESS;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_set_dfs_ignore_cac(tHalHandle hHal, uint8_t ignore_cac)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	pMac->sap.SapDfsInfo.ignore_cac = (ignore_cac >= true) ?
					  true : false;
	return CDF_STATUS_SUCCESS;
}

/**
 * wlansap_set_dfs_restrict_japan_w53() - enable/disable dfS for japan
 * @hHal : HAL pointer
 * @disable_Dfs_JapanW3 :Indicates if Japan W53 is disabled when set to 1
 *                       Indicates if Japan W53 is enabled when set to 0
 *
 * This API is used to enable or disable Japan W53 Band
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success
 */
CDF_STATUS
wlansap_set_dfs_restrict_japan_w53(tHalHandle hHal, uint8_t disable_Dfs_W53)
{
	tpAniSirGlobal pMac = NULL;
	CDF_STATUS status;
	uint8_t dfs_region;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	cds_get_dfs_region(&dfs_region);

	/*
	 * Set the JAPAN W53 restriction only if the current
	 * regulatory domain is JAPAN.
	 */
	if (DFS_MKK4_DOMAIN == dfs_region) {
		pMac->sap.SapDfsInfo.is_dfs_w53_disabled = disable_Dfs_W53;
		CDF_TRACE(CDF_MODULE_ID_SAP,
			  CDF_TRACE_LEVEL_INFO_LOW,
			  FL("sapdfs: SET DFS JAPAN W53 DISABLED = %d"),
			  pMac->sap.SapDfsInfo.is_dfs_w53_disabled);

		status = CDF_STATUS_SUCCESS;
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  FL
				  ("Regdomain not japan, set disable JP W53 not valid"));

		status = CDF_STATUS_E_FAULT;
	}

	return status;
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
CDF_STATUS
wlan_sap_set_channel_avoidance(tHalHandle hal, bool sap_channel_avoidance)
{
	tpAniSirGlobal mac_ctx = NULL;
	if (NULL != hal)
		mac_ctx = PMAC_STRUCT(hal);
	if (mac_ctx == NULL || hal == NULL) {
		CDF_TRACE(CDF_MODULE_ID_SAP,
			  CDF_TRACE_LEVEL_ERROR,
			  FL("hal or mac_ctx pointer NULL"));
		return CDF_STATUS_E_FAULT;
	}
	mac_ctx->sap.sap_channel_avoidance = sap_channel_avoidance;
	return CDF_STATUS_SUCCESS;
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
 * Return: The CDF_STATUS code associated with performing the operation
 *         CDF_STATUS_SUCCESS:  Success and error code otherwise.
 */
CDF_STATUS
wlansap_set_dfs_preferred_channel_location(tHalHandle hHal,
					   uint8_t
					   dfs_Preferred_Channels_location)
{
	tpAniSirGlobal pMac = NULL;
	CDF_STATUS status;
	uint8_t dfs_region;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	cds_get_dfs_region(&dfs_region);

	/*
	 * The Indoor/Outdoor only random channel selection
	 * restriction is currently enforeced only for
	 * JAPAN regulatory domain.
	 */
	if (DFS_MKK4_DOMAIN == dfs_region) {
		pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location =
			dfs_Preferred_Channels_location;
		CDF_TRACE(CDF_MODULE_ID_SAP,
			  CDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("sapdfs:Set Preferred Operating Channel location=%d"),
			  pMac->sap.SapDfsInfo.
			  sap_operating_chan_preferred_location);

		status = CDF_STATUS_SUCCESS;
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  FL
				  ("sapdfs:NOT JAPAN REG, Invalid Set preferred chans location"));

		status = CDF_STATUS_E_FAULT;
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
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_set_dfs_target_chnl(tHalHandle hHal, uint8_t target_channel)
{
	tpAniSirGlobal pMac = NULL;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid hHal pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}
	if (target_channel > 0) {
		pMac->sap.SapDfsInfo.user_provided_target_channel =
			target_channel;
	} else {
		pMac->sap.SapDfsInfo.user_provided_target_channel = 0;
	}

	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
wlansap_update_sap_config_add_ie(tsap_Config_t *pConfig,
				 const uint8_t *pAdditionIEBuffer,
				 uint16_t additionIELength,
				 eUpdateIEsType updateType)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	uint8_t bufferValid = false;
	uint16_t bufferLength = 0;
	uint8_t *pBuffer = NULL;

	if (NULL == pConfig) {
		return CDF_STATUS_E_FAULT;
	}

	if ((pAdditionIEBuffer != NULL) && (additionIELength != 0)) {
		/* initialize the buffer pointer so that pe can copy */
		if (additionIELength > 0) {
			bufferLength = additionIELength;
			pBuffer = cdf_mem_malloc(bufferLength);
			if (NULL == pBuffer) {
				CDF_TRACE(CDF_MODULE_ID_SME,
					  CDF_TRACE_LEVEL_ERROR,
					  FL("Could not allocate the buffer "));
				return CDF_STATUS_E_NOMEM;
			}
			cdf_mem_copy(pBuffer, pAdditionIEBuffer, bufferLength);
			bufferValid = true;
		}
	}

	switch (updateType) {
	case eUPDATE_IE_PROBE_BCN:
		if (bufferValid) {
			pConfig->probeRespBcnIEsLen = bufferLength;
			pConfig->pProbeRespBcnIEsBuffer = pBuffer;
		} else {
			cdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
			pConfig->probeRespBcnIEsLen = 0;
			pConfig->pProbeRespBcnIEsBuffer = NULL;
			CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  FL
					  ("No Probe Resp beacone IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_PROBE_RESP:
		if (bufferValid) {
			pConfig->probeRespIEsBufferLen = bufferLength;
			pConfig->pProbeRespIEsBuffer = pBuffer;
		} else {
			cdf_mem_free(pConfig->pProbeRespIEsBuffer);
			pConfig->probeRespIEsBufferLen = 0;
			pConfig->pProbeRespIEsBuffer = NULL;
			CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  FL
					  ("No Probe Response IE received in set beacon"));
		}
		break;
	case eUPDATE_IE_ASSOC_RESP:
		if (bufferValid) {
			pConfig->assocRespIEsLen = bufferLength;
			pConfig->pAssocRespIEsBuffer = pBuffer;
		} else {
			cdf_mem_free(pConfig->pAssocRespIEsBuffer);
			pConfig->assocRespIEsLen = 0;
			pConfig->pAssocRespIEsBuffer = NULL;
			CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  FL
					  ("No Assoc Response IE received in set beacon"));
		}
		break;
	default:
		CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
			  FL("No matching buffer type %d"), updateType);
		if (pBuffer != NULL)
			cdf_mem_free(pBuffer);
		break;
	}

	return status;
}

CDF_STATUS
wlansap_reset_sap_config_add_ie(tsap_Config_t *pConfig, eUpdateIEsType updateType)
{
	if (NULL == pConfig) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid Config pointer", __func__);
		return CDF_STATUS_E_FAULT;
	}

	switch (updateType) {
	case eUPDATE_IE_ALL:    /*only used to reset */
	case eUPDATE_IE_PROBE_RESP:
		cdf_mem_free(pConfig->pProbeRespIEsBuffer);
		pConfig->probeRespIEsBufferLen = 0;
		pConfig->pProbeRespIEsBuffer = NULL;
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_ASSOC_RESP:
		cdf_mem_free(pConfig->pAssocRespIEsBuffer);
		pConfig->assocRespIEsLen = 0;
		pConfig->pAssocRespIEsBuffer = NULL;
		if (eUPDATE_IE_ALL != updateType)
			break;

	case eUPDATE_IE_PROBE_BCN:
		cdf_mem_free(pConfig->pProbeRespBcnIEsBuffer);
		pConfig->probeRespBcnIEsLen = 0;
		pConfig->pProbeRespBcnIEsBuffer = NULL;
		if (eUPDATE_IE_ALL != updateType)
			break;

	default:
		if (eUPDATE_IE_ALL != updateType)
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  FL("Invalid buffer type %d"), updateType);
		break;
	}
	return CDF_STATUS_SUCCESS;
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
void wlansap_extend_to_acs_range(uint8_t *startChannelNum,
				 uint8_t *endChannelNum,
				 uint8_t *bandStartChannel,
				 uint8_t *bandEndChannel)
{
#define ACS_WLAN_20M_CH_INC 4
#define ACS_2G_EXTEND ACS_WLAN_20M_CH_INC
#define ACS_5G_EXTEND (ACS_WLAN_20M_CH_INC * 3)

	uint8_t tmp_startChannelNum = 0, tmp_endChannelNum = 0;

	if (*startChannelNum <= 14 && *endChannelNum <= 14) {
		*bandStartChannel = RF_CHAN_1;
		*bandEndChannel = RF_CHAN_14;
		tmp_startChannelNum = *startChannelNum > 5 ?
				   (*startChannelNum - ACS_2G_EXTEND) : 1;
		tmp_endChannelNum = (*endChannelNum + ACS_2G_EXTEND) <= 14 ?
				 (*endChannelNum + ACS_2G_EXTEND) : 14;
	} else if (*startChannelNum >= 36 && *endChannelNum >= 36) {
		*bandStartChannel = RF_CHAN_36;
		*bandEndChannel = RF_CHAN_165;
		tmp_startChannelNum = (*startChannelNum - ACS_5G_EXTEND) > 36 ?
				   (*startChannelNum - ACS_5G_EXTEND) : 36;
		tmp_endChannelNum = (*endChannelNum + ACS_5G_EXTEND) <= 165 ?
				 (*endChannelNum + ACS_5G_EXTEND) : 165;
	} else {
		*bandStartChannel = RF_CHAN_1;
		*bandEndChannel = RF_CHAN_165;
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
	if (CDS_IS_DFS_CH(*startChannelNum)) {
		while (!CDS_IS_DFS_CH(tmp_startChannelNum) &&
			tmp_startChannelNum < *startChannelNum)
			tmp_startChannelNum += ACS_WLAN_20M_CH_INC;

		*startChannelNum = tmp_startChannelNum;
	}
	if (CDS_IS_DFS_CH(*endChannelNum)) {
		while (!CDS_IS_DFS_CH(tmp_endChannelNum) &&
				 tmp_endChannelNum > *endChannelNum)
			tmp_endChannelNum -= ACS_WLAN_20M_CH_INC;

		*endChannelNum = tmp_endChannelNum;
	}
}

/*==========================================================================
   FUNCTION    wlansap_get_dfs_nol

   DESCRIPTION
   This API is used to dump the dfs nol
   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   sapContext: Pointer to cds global context structure

   RETURN VALUE
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_get_dfs_nol(void *pSapCtx)
{
	int i = 0;
	ptSapContext sapContext = (ptSapContext) pSapCtx;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;
	uint64_t current_time, found_time, elapsed_time;
	unsigned long left_time;
	tSapDfsNolInfo *dfs_nol = NULL;
	bool bAvailable = false;

	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	if (!pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL is empty", __func__);
		return CDF_STATUS_SUCCESS;
	}

	dfs_nol = pMac->sap.SapDfsInfo.sapDfsChannelNolList;

	if (!dfs_nol) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL context is null", __func__);
		return CDF_STATUS_E_FAULT;
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

			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Channel[%d] is AVAILABLE",
				  __func__, dfs_nol[i].dfs_channel_number);
		} else {

			/* the time left in min */
			left_time = SAP_DFS_NON_OCCUPANCY_PERIOD - elapsed_time;
			left_time = left_time / (60 * 1000 * 1000);

			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Channel[%d] is UNAVAILABLE [%lu min left]",
				  __func__,
				  dfs_nol[i].dfs_channel_number, left_time);
		}
	}

	return CDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    wlansap_set_dfs_nol

   DESCRIPTION
   This API is used to set the dfs nol
   DEPENDENCIES
   NA.

   PARAMETERS
   IN
   sapContext: Pointer to cds global context structure
   conf: set type

   RETURN VALUE
   The CDF_STATUS code associated with performing the operation

   CDF_STATUS_SUCCESS:  Success

   SIDE EFFECTS
   ============================================================================*/
CDF_STATUS wlansap_set_dfs_nol(void *pSapCtx, eSapDfsNolType conf)
{
	int i = 0;
	ptSapContext sapContext = (ptSapContext) pSapCtx;
	void *hHal = NULL;
	tpAniSirGlobal pMac = NULL;

	if (NULL == sapContext) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid SAP pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid HAL pointer from p_cds_gctx", __func__);
		return CDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	if (!pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
			  "%s: DFS NOL is empty", __func__);
		return CDF_STATUS_SUCCESS;
	}

	if (conf == eSAP_DFS_NOL_CLEAR) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: clear the DFS NOL", __func__);

		for (i = 0;
		     i < pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels;
		     i++) {
			if (!pMac->sap.SapDfsInfo.
			    sapDfsChannelNolList[i].dfs_channel_number)
				continue;

			pMac->sap.SapDfsInfo.
			sapDfsChannelNolList[i].radar_status_flag =
				eSAP_DFS_CHANNEL_AVAILABLE;
			pMac->sap.SapDfsInfo.
			sapDfsChannelNolList[i].radar_found_timestamp = 0;
		}
	} else if (conf == eSAP_DFS_NOL_RANDOMIZE) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: Randomize the DFS NOL", __func__);

		/* random 1/0 to decide to put the channel into NOL */
		for (i = 0;
		     i < pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels;
		     i++) {
			uint32_t random_bytes = 0;
			get_random_bytes(&random_bytes, 1);

			if (!pMac->sap.SapDfsInfo.
			    sapDfsChannelNolList[i].dfs_channel_number)
				continue;

			if ((random_bytes + jiffies) % 2) {
				/* mark the channel unavailable */
				pMac->sap.SapDfsInfo.sapDfsChannelNolList[i]
				.radar_status_flag =
					eSAP_DFS_CHANNEL_UNAVAILABLE;

				/* mark the timestamp */
				pMac->sap.SapDfsInfo.sapDfsChannelNolList[i]
				.radar_found_timestamp =
					cds_get_monotonic_boottime();
			} else {
				/* mark the channel available */
				pMac->sap.SapDfsInfo.
				sapDfsChannelNolList[i].radar_status_flag =
					eSAP_DFS_CHANNEL_AVAILABLE;

				/* clear the timestamp */
				pMac->sap.SapDfsInfo.
				sapDfsChannelNolList
				[i].radar_found_timestamp = 0;
			}

			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				  "%s: Set channel[%d] %s",
				  __func__,
				  pMac->sap.SapDfsInfo.sapDfsChannelNolList[i]
				  .dfs_channel_number,
				  (pMac->sap.SapDfsInfo.
				   sapDfsChannelNolList[i].radar_status_flag >
				   eSAP_DFS_CHANNEL_AVAILABLE) ? "UNAVAILABLE" :
				  "AVAILABLE");
		}
	} else {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			  "%s: unsupport type %d", __func__, conf);
	}

	/* set DFS-NOL back to keep it update-to-date in CNSS */
	sap_signal_hdd_event(sapContext, NULL, eSAP_DFS_NOL_SET,
			  (void *) eSAP_STATUS_SUCCESS);

	return CDF_STATUS_SUCCESS;
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
		cdf_set_macaddr_broadcast(&pDelStaParams->peerMacAddr);
	else
		cdf_mem_copy(pDelStaParams->peerMacAddr.bytes, mac,
			     CDF_MAC_ADDR_SIZE);

	if (reason_code == 0)
		pDelStaParams->reason_code = eSIR_MAC_DEAUTH_LEAVING_BSS_REASON;
	else
		pDelStaParams->reason_code = reason_code;

	if (subtype == (SIR_MAC_MGMT_DEAUTH >> 4) ||
	    subtype == (SIR_MAC_MGMT_DISASSOC >> 4))
		pDelStaParams->subtype = subtype;
	else
		pDelStaParams->subtype = (SIR_MAC_MGMT_DEAUTH >> 4);

	CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO,
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
 * Return: The CDF_STATUS code associated with performing the operation.
 */
CDF_STATUS
wlansap_acs_chselect(void *pvos_gctx,
			tpWLAN_SAPEventCB pacs_event_callback,
			tsap_Config_t *pconfig,
			void *pusr_context)
{
	ptSapContext sap_context = NULL;
	tHalHandle h_hal = NULL;
	CDF_STATUS cdf_status = CDF_STATUS_E_FAILURE;
	tpAniSirGlobal pmac = NULL;

	sap_context = CDS_GET_SAP_CB(pvos_gctx);
	if (NULL == sap_context) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%s: Invalid SAP pointer from pvos_gctx", __func__);

		return CDF_STATUS_E_FAULT;
	}

	h_hal = (tHalHandle)CDS_GET_HAL_CB(sap_context->p_cds_gctx);
	if (NULL == h_hal) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"%s: Invalid MAC context from pvosGCtx", __func__);
		return CDF_STATUS_E_FAULT;
	}

	if (sap_context->isSapSessionOpen == eSAP_TRUE) {
		CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_FATAL,
			"%s:SME Session is already opened\n", __func__);
		return CDF_STATUS_E_EXISTS;
	}

	sap_context->sessionId = 0xff;

	pmac = PMAC_STRUCT(h_hal);
	sap_context->acs_cfg = &pconfig->acs_cfg;
	sap_context->ch_width_orig = pconfig->acs_cfg.ch_width;
	sap_context->csr_roamProfile.phyMode = pconfig->acs_cfg.hw_mode;

	if (sap_context->isScanSessionOpen == eSAP_FALSE) {
		uint32_t type, subType;

		/*
		* Now, configure the scan and ACS channel params
		* to issue a scan request.
		*/
		wlansap_set_scan_acs_channel_params(pconfig, sap_context,
						pusr_context);

		if (CDF_STATUS_SUCCESS ==
			cds_get_vdev_types(CDF_STA_MODE, &type, &subType)) {
			/*
			* Open SME Session for scan
			*/
			if (CDF_STATUS_SUCCESS  != sme_open_session(h_hal,
						NULL, sap_context,
						sap_context->self_mac_addr,
						&sap_context->sessionId,
						type, subType)) {
				CDF_TRACE(CDF_MODULE_ID_SAP,
					CDF_TRACE_LEVEL_ERROR,
					"Error: In %s calling sme_OpenSession",
					__func__);
				return CDF_STATUS_E_FAILURE;
			} else {
				sap_context->isScanSessionOpen = eSAP_TRUE;
			}
		}

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
		cdf_status = sap_goto_channel_sel(sap_context, NULL, true);

		if (CDF_STATUS_E_ABORTED == cdf_status) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
			"In %s,DFS not supported in the current operating mode",
			__func__);
			return CDF_STATUS_E_FAILURE;
		} else if (CDF_STATUS_E_CANCELED == cdf_status) {
			/*
			* ERROR is returned when either the SME scan request
			* failed or ACS is overridden due to other constrainst
			* So send selected channel to HDD
			*/
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				FL("Scan Req Failed/ACS Overridden"));
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_ERROR,
				FL("Selected channel = %d"),
				sap_context->channel);
			if (sap_context->isScanSessionOpen == eSAP_TRUE) {
				/* ACS scan not needed so close session */
				tHalHandle h_hal = CDS_GET_HAL_CB(
						sap_context->p_cds_gctx);
				if (h_hal == NULL)
					return CDF_STATUS_E_FAILURE;

				if (sme_close_session(h_hal,
					sap_context->sessionId, NULL, NULL) ==
							 CDF_STATUS_SUCCESS)
					sap_context->isScanSessionOpen =
								eSAP_FALSE;
				else
					CDF_TRACE(CDF_MODULE_ID_SAP,
						CDF_TRACE_LEVEL_ERROR,
						"ACS Scan Session close fail");
				sap_context->sessionId = 0xff;
			}

			return sap_signal_hdd_event(sap_context, NULL,
					eSAP_ACS_CHANNEL_SELECTED,
					(void *) eSAP_STATUS_SUCCESS);
		} else if (CDF_STATUS_SUCCESS == cdf_status) {
			CDF_TRACE(CDF_MODULE_ID_SAP, CDF_TRACE_LEVEL_INFO_HIGH,
			FL("Successfully Issued a Pre Start Bss Scan Request"));
		}
	}
	return cdf_status;
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
