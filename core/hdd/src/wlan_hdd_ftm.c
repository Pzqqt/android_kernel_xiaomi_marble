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

/**
 * DOC: wlan_hdd_ftm.c
 *
 * This file contains the WLAN factory test mode implementation
 */

#include <cds_mq.h>
#include "cds_sched.h"
#include <cds_api.h>
#include "sir_types.h"
#include "cdf_types.h"
#include "sir_api.h"
#include "sir_mac_prot_def.h"
#include "sme_api.h"
#include "mac_init_api.h"
#include "wlan_qct_sys.h"
#include "wlan_hdd_misc.h"
#include "i_cds_packet.h"
#include "cds_reg_service.h"
#include "wlan_hdd_main.h"
#include "qwlan_version.h"
#include "wma_types.h"
#include "cfg_api.h"

#if  defined(QCA_WIFI_FTM)
#include "bmi.h"
#include "ol_fw.h"
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_main.h"
#include "hif.h"
#endif

#define HDD_FTM_WMA_PRE_START_TIMEOUT (30000) /* 30 seconds */

#if  defined(QCA_WIFI_FTM)
#if defined(LINUX_QCMBR)
#define ATH_XIOCTL_UNIFIED_UTF_CMD  0x1000
#define ATH_XIOCTL_UNIFIED_UTF_RSP  0x1001
#define MAX_UTF_LENGTH              1024
typedef struct qcmbr_data_s {
	unsigned int cmd;
	unsigned int length;
	unsigned char buf[MAX_UTF_LENGTH + 4];
	unsigned int copy_to_user;
} qcmbr_data_t;
typedef struct qcmbr_queue_s {
	unsigned char utf_buf[MAX_UTF_LENGTH + 4];
	struct list_head list;
} qcmbr_queue_t;
LIST_HEAD(qcmbr_queue_head);
DEFINE_SPINLOCK(qcmbr_queue_lock);
#endif
#endif

/**
 * wlan_ftm_postmsg() - Post FTM message
 * @cmd_ptr: Pointer to FTM command buffer
 * @cmd_len: Length of command in @cmd_ptr
 *
 * This function is used to send FTM commands to firmware
 *
 * Return: 0 for success, non zero for failure
 */
static uint32_t wlan_ftm_postmsg(uint8_t *cmd_ptr, uint16_t cmd_len)
{
	cds_msg_t ftmMsg;

	ENTER();

	ftmMsg.type = WMA_FTM_CMD_REQ;
	ftmMsg.reserved = 0;
	ftmMsg.bodyptr = (uint8_t *) cmd_ptr;
	ftmMsg.bodyval = 0;

	if (CDF_STATUS_SUCCESS != cds_mq_post_message(CDF_MODULE_ID_WMA,
						      &ftmMsg)) {
		hddLog(CDF_TRACE_LEVEL_ERROR, "%s: : Failed to post Msg to HAL",
		       __func__);

		return CDF_STATUS_E_FAILURE;
	}

	EXIT();
	return CDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_ftm_update_tgt_cfg() - Update target configuration
 * @context: context registered with WMA
 * @param: target configuration
 *
 * This function is registered with WMA via wma_open(), and is
 * invoked via callback when target parameters are received
 * from firmware.
 *
 * Return: None
 */
static void wlan_hdd_ftm_update_tgt_cfg(void *context, void *param)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) context;
	struct wma_tgt_cfg *cfg = (struct wma_tgt_cfg *)param;

	if (!cdf_is_macaddr_zero(&cfg->hw_macaddr)) {
		hdd_update_macaddr(hdd_ctx->config, cfg->hw_macaddr);
	} else {
		hddLog(CDF_TRACE_LEVEL_ERROR,
		       "%s: Invalid MAC passed from target, using MAC from ini file"
		       MAC_ADDRESS_STR, __func__,
		       MAC_ADDR_ARRAY(hdd_ctx->config->intfMacAddr[0].bytes));
	}
}

#ifdef WLAN_FEATURE_LPSS
static inline void hdd_is_lpass_supported(tMacOpenParameters *mac_openParms,
						hdd_context_t *hdd_ctx)
{
	mac_openParms->is_lpass_enabled = hdd_ctx->config->enable_lpass_support;
}
#else
static inline void hdd_is_lpass_supported(tMacOpenParameters *mac_openParms,
						hdd_context_t *hdd_ctx)
{ }
#endif

/**
 * wlan_ftm_cds_open() - Open the CDS Module in FTM mode
 * @p_cds_context: pointer to the global CDS context
 * @hddContextSize: Size of the HDD context to allocate.
 *
 * The wlan_ftm_cds_open() function opens the CDF Scheduler
 * Upon successful initialization:
 * - All CDS submodules should have been initialized
 * - The CDS scheduler should have opened
 * - All the WLAN SW components should have been opened. This includes MAC.
 *
 * Returns:
 *	CDF_STATUS_SUCCESS - Scheduler was successfully initialized and
 *	is ready to be used.
 *	CDF_STATUS_E_RESOURCES - System resources (other than memory)
 *	are unavailable to initialize the scheduler
 *	CDF_STATUS_E_FAILURE - Failure to initialize the scheduler
 */
static CDF_STATUS wlan_ftm_cds_open(v_CONTEXT_t p_cds_context,
				    uint32_t hddContextSize)
{
	CDF_STATUS vStatus = CDF_STATUS_SUCCESS;
	int iter = 0;
	tSirRetStatus sirStatus = eSIR_SUCCESS;
	tMacOpenParameters mac_openParms;
	p_cds_contextType gp_cds_context = (p_cds_contextType) p_cds_context;
#if  defined(QCA_WIFI_FTM)
	cdf_device_t cdf_ctx;
	HTC_INIT_INFO htcInfo;
	void *pHifContext = NULL;
	void *pHtcContext = NULL;
	struct ol_context *ol_ctx;
#endif
	hdd_context_t *hdd_ctx;

	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Opening CDS", __func__);

	if (NULL == gp_cds_context) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Trying to open CDS without a PreOpen", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}

	/* Initialize the probe event */
	if (qdf_event_create(&gp_cds_context->ProbeEvent) != QDF_STATUS_SUCCESS) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Unable to init probeEvent", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}

	if (qdf_event_create(&(gp_cds_context->wmaCompleteEvent)) !=
	    QDF_STATUS_SUCCESS) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Unable to init wmaCompleteEvent", __func__);
		CDF_ASSERT(0);

		goto err_probe_event;
	}

	/* Initialize the free message queue */
	vStatus = cds_mq_init(&gp_cds_context->freeVosMq);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {

		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to initialize CDS free message queue %d",
			  __func__, vStatus);
		CDF_ASSERT(0);
		goto err_wma_complete_event;
	}

	for (iter = 0; iter < CDS_CORE_MAX_MESSAGES; iter++) {
		(gp_cds_context->aMsgWrappers[iter]).pVosMsg =
			&(gp_cds_context->aMsgBuffers[iter]);
		INIT_LIST_HEAD(&gp_cds_context->aMsgWrappers[iter].msgNode);
		cds_mq_put(&gp_cds_context->freeVosMq,
			   &(gp_cds_context->aMsgWrappers[iter]));
	}

	/* Now Open the CDS Scheduler */
	vStatus = cds_sched_open(gp_cds_context, &gp_cds_context->cdf_sched,
				 sizeof(cds_sched_context));

	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to open CDS Scheduler %d", __func__,
			  vStatus);
		CDF_ASSERT(0);
		goto err_msg_queue;
	}
#if  defined(QCA_WIFI_FTM)
	/* Initialize BMI and Download firmware */
	pHifContext = cds_get_context(CDF_MODULE_ID_HIF);
	if (!pHifContext) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: failed to get HIF context", __func__);
		goto err_sched_close;
	}

	ol_ctx = cds_get_context(CDF_MODULE_ID_BMI);
	if (bmi_download_firmware(ol_ctx)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: BMI failed to download target", __func__);
		goto err_bmi_close;
	}

	htcInfo.pContext = ol_ctx;
	htcInfo.TargetFailure = ol_target_failure;
	htcInfo.TargetSendSuspendComplete = wma_target_suspend_acknowledge;
	cdf_ctx = cds_get_context(CDF_MODULE_ID_CDF_DEVICE);

	/* Create HTC */
	gp_cds_context->htc_ctx =
		htc_create(pHifContext, &htcInfo, cdf_ctx);
	if (!gp_cds_context->htc_ctx) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: Failed to Create HTC", __func__);
		goto err_bmi_close;
	}

	if (bmi_done(ol_ctx)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: Failed to complete BMI phase", __func__);
		goto err_htc_close;
	}
#endif /* QCA_WIFI_FTM */

	/*Open the WMA module */
	cdf_mem_set(&mac_openParms, sizeof(mac_openParms), 0);
	mac_openParms.driverType = eDRIVER_TYPE_MFG;

	hdd_ctx = (hdd_context_t *) (gp_cds_context->pHDDContext);
	if ((NULL == hdd_ctx) || (NULL == hdd_ctx->config)) {
		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Hdd Context is Null", __func__);
		CDF_ASSERT(0);
		goto err_htc_close;
	}

	mac_openParms.powersaveOffloadEnabled =
		hdd_ctx->config->enablePowersaveOffload;

	hdd_is_lpass_supported(&mac_openParms, hdd_ctx);

	vStatus = wma_open(gp_cds_context,
			   wlan_hdd_ftm_update_tgt_cfg, NULL, &mac_openParms);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to open WMA module %d", __func__,
			  vStatus);
		CDF_ASSERT(0);
		goto err_htc_close;
	}
#if  defined(QCA_WIFI_FTM)
	hdd_update_config(hdd_ctx);

	pHtcContext = cds_get_context(CDF_MODULE_ID_HTC);
	if (!pHtcContext) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: failed to get HTC context", __func__);
		goto err_wma_close;
	}
	if (htc_wait_target(pHtcContext)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "%s: Failed to complete BMI phase", __func__);
		goto err_wma_close;
	}
#endif

	/* Now proceed to open the MAC */

	/* UMA is supported in hardware for performing the
	 * frame translation 802.11 <-> 802.3
	 */
	mac_openParms.frameTransRequired = 1;

	sirStatus =
		mac_open(&(gp_cds_context->pMACContext),
			 gp_cds_context->pHDDContext,
			 &mac_openParms);

	if (eSIR_SUCCESS != sirStatus) {
		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to open MAC %d", __func__, sirStatus);
		CDF_ASSERT(0);
		goto err_wma_close;
	}
#ifndef QCA_WIFI_FTM
	/* Now proceed to open the SME */
	vStatus = sme_open(gp_cds_context->pMACContext);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		/* Critical Error ...  Cannot proceed further */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to open SME %d", __func__, vStatus);
		goto err_mac_close;
	}

	vStatus = sme_init_chan_list(gp_cds_context->pMACContext,
				     hdd_ctx->reg.alpha2, hdd_ctx->reg.cc_src);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to init sme channel list", __func__);
	} else {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: CDS successfully Opened", __func__);
		return CDF_STATUS_SUCCESS;
	}
#else
	return CDF_STATUS_SUCCESS;
#endif

#ifndef QCA_WIFI_FTM
err_mac_close:
#endif
	mac_close(gp_cds_context->pMACContext);

err_wma_close:
	wma_close(gp_cds_context);

err_htc_close:
#if  defined(QCA_WIFI_FTM)
	if (gp_cds_context->htc_ctx) {
		htc_destroy(gp_cds_context->htc_ctx);
		gp_cds_context->htc_ctx = NULL;
	}

err_bmi_close:
	bmi_cleanup(ol_ctx);
#endif /* QCA_WIFI_FTM */

err_sched_close:
	cds_sched_close(gp_cds_context);
err_msg_queue:
	cds_mq_deinit(&gp_cds_context->freeVosMq);

err_wma_complete_event:
	qdf_event_destroy(&gp_cds_context->wmaCompleteEvent);

err_probe_event:
	qdf_event_destroy(&gp_cds_context->ProbeEvent);

	return CDF_STATUS_E_FAILURE;

} /* wlan_ftm_cds_open() */

/**
 * wlan_ftm_cds_close() - Close the CDF Module in FTM mode
 * @cds_context:  context of cds
 *
 * The wlan_ftm_cds_close() function closes the CDF Module
 *
 * Return: CDF_STATUS_SUCCESS - successfully closed
 */
static CDF_STATUS wlan_ftm_cds_close(v_CONTEXT_t cds_context)
{
	CDF_STATUS cdf_status;
	QDF_STATUS qdf_status;
	p_cds_contextType gp_cds_context = (p_cds_contextType) cds_context;

#ifndef QCA_WIFI_FTM
	cdf_status = sme_close(((p_cds_contextType) cds_context)->pMACContext);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to close SME %d", __func__, cdf_status);
		CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));
	}
#endif

	cdf_status = mac_close(((p_cds_contextType) cds_context)->pMACContext);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to close MAC %d", __func__, cdf_status);
		CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));
	}

	((p_cds_contextType) cds_context)->pMACContext = NULL;


	cdf_status = wma_close(cds_context);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to close WMA %d", __func__, cdf_status);
		CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));
	}
#if  defined(QCA_WIFI_FTM)
	if (gp_cds_context->htc_ctx) {
		htc_stop(gp_cds_context->htc_ctx);
		htc_destroy(gp_cds_context->htc_ctx);
		gp_cds_context->htc_ctx = NULL;
	}
	cdf_status = wma_wmi_service_close(cds_context);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to close wma_wmi_service", __func__);
		CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));
	}

	hif_disable_isr(gp_cds_context->pHIFContext);
#endif

	cds_mq_deinit(&((p_cds_contextType) cds_context)->freeVosMq);

	qdf_status = qdf_event_destroy(&gp_cds_context->ProbeEvent);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to destroy ProbeEvent %d", __func__,
			  qdf_status);
		CDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
	}

	qdf_status = qdf_event_destroy(&gp_cds_context->wmaCompleteEvent);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to destroy wmaCompleteEvent %d", __func__,
			  qdf_status);
		CDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
	}

	return CDF_STATUS_SUCCESS;
}

/**
 * cds_ftm_pre_start() - Pre-start CDS Module in FTM Mode
 * @cds_context: The CDS context
 *
 * The cds_ftm_pre_start() function performs all pre-start activities
 * in FTM mode.
 *
 * Return: CDF_STATUS_SUCCESS if pre-start was successful, an
 *	   appropriate CDF_STATUS_E_* error code otherwise
 */
static CDF_STATUS cds_ftm_pre_start(v_CONTEXT_t cds_context)
{
	CDF_STATUS vStatus = CDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	p_cds_contextType p_cds_context = (p_cds_contextType) cds_context;
#if  defined(QCA_WIFI_FTM)
	p_cds_contextType gp_cds_context =
		cds_get_global_context();
#endif

	CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_INFO, "cds prestart");
	if (NULL == p_cds_context->pWMAContext) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "%s: WMA NULL context", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	/* Reset WMA wait event */
	qdf_event_reset(&p_cds_context->wmaCompleteEvent);

	/*call WMA pre start */
	vStatus = wma_pre_start(p_cds_context);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
			  "Failed to WMA prestart ");
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}

	/* Need to update time out of complete */
	qdf_status = qdf_wait_single_event(&p_cds_context->wmaCompleteEvent,
					HDD_FTM_WMA_PRE_START_TIMEOUT);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		if (qdf_status == QDF_STATUS_E_TIMEOUT) {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				  "%s: Timeout occurred before WMA complete",
				  __func__);
		} else {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				  "%s: wma_pre_start reporting  other error",
				  __func__);
		}
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}
#if  defined(QCA_WIFI_FTM)
	vStatus = htc_start(gp_cds_context->htc_ctx);
	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_FATAL,
			  "Failed to Start HTC");
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}
	wma_wait_for_ready_event(gp_cds_context->pWMAContext);
#endif /* QCA_WIFI_FTM */

	return CDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_ftm_open() - Open HDD in FTM Mode
 * @hdd_ctx: global HDD context
 *
 * The function hdd_wlan_startup calls this function to initialize the
 * FTM specific modules.
 *
 * Return: 0 on success, non-zero on error
 */
int wlan_hdd_ftm_open(hdd_context_t *hdd_ctx)
{
	CDF_STATUS vStatus = CDF_STATUS_SUCCESS;
	p_cds_contextType p_cds_context = NULL;

	CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Opening CDS", __func__);

	p_cds_context = cds_get_global_context();

	if (NULL == p_cds_context) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: Trying to open CDS without a PreOpen", __func__);
		CDF_ASSERT(0);
		goto err_cdf_status_failure;
	}

	vStatus = wlan_ftm_cds_open(p_cds_context, 0);

	if (!CDF_IS_STATUS_SUCCESS(vStatus)) {
		hddLog(CDF_TRACE_LEVEL_FATAL, "%s: cds_open failed", __func__);
		goto err_cdf_status_failure;
	}

	/*
	 * only needed to start WMA, which happens in wlan_hdd_ftm_start()
	 */

	/* Save the hal context in Adapter */
	hdd_ctx->hHal =
		(tHalHandle) cds_get_context(CDF_MODULE_ID_SME);

	if (NULL == hdd_ctx->hHal) {
		hddLog(CDF_TRACE_LEVEL_ERROR, "%s: HAL context is null",
		       __func__);
		goto err_ftm_close;
	}

	return 0;

err_ftm_close:
	wlan_ftm_cds_close(p_cds_context);

err_cdf_status_failure:
	return -EPERM;
}

/**
 * hdd_ftm_service_registration() - Register FTM service
 * @hdd_ctx: global HDD context
 *
 * Return: 0 on success, non-zero on failure
 */
static int hdd_ftm_service_registration(hdd_context_t *hdd_ctx)
{
	hdd_adapter_t *adapter;
	adapter = hdd_open_adapter(hdd_ctx, WLAN_HDD_FTM, "wlan%d",
				    wlan_hdd_get_intf_addr(hdd_ctx), false);
	if (NULL == adapter) {
		hddLog(CDF_TRACE_LEVEL_ERROR, "%s: hdd_open_adapter failed",
		       __func__);
		goto err_adapter_open_failure;
	}

	hdd_ctx->ftm.ftm_state = WLAN_FTM_INITIALIZED;

	return 0;

err_adapter_open_failure:

	return -EPERM;
}

/**
 * wlan_ftm_stop() - Stop HDD in FTM mode
 * @hdd_ctx: pointer to HDD context
 *
 * This function stops the following modules
 * WMA
 *
 * Return: 0 on success, non-zero on failure
 */
static int wlan_ftm_stop(hdd_context_t *hdd_ctx)
{
	if (hdd_ctx->ftm.ftm_state != WLAN_FTM_STARTED) {
		hddLog(LOGP, FL("FTM has not started. No need to stop"));
		return -EPERM;
	}
	wma_stop(hdd_ctx->pcds_context, HAL_STOP_TYPE_RF_KILL);
	return 0;
}

/**
 * wlan_hdd_ftm_close() - Close HDD in FTM mode
 * @hdd_ctx: pointer to HDD context
 *
 * Return: 0 on success, non-zero on failure
 */
int wlan_hdd_ftm_close(hdd_context_t *hdd_ctx)
{
	CDF_STATUS cdf_status;
	v_CONTEXT_t cds_context = hdd_ctx->pcds_context;

	hdd_adapter_t *adapter = hdd_get_adapter(hdd_ctx, WLAN_HDD_FTM);
	ENTER();
	if (adapter == NULL) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_FATAL,
			  "%s:adapter is NULL", __func__);
		return -ENXIO;
	}

	if (WLAN_FTM_STARTED == hdd_ctx->ftm.ftm_state) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_FATAL,
			  "%s: Ftm has been started. stopping ftm", __func__);
		wlan_ftm_stop(hdd_ctx);
	}

	hdd_close_all_adapters(hdd_ctx, false);

	cdf_status = cds_sched_close(cds_context);
	if (!CDF_IS_STATUS_SUCCESS(cdf_status)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to close CDS Scheduler", __func__);
		CDF_ASSERT(CDF_IS_STATUS_SUCCESS(cdf_status));
	}
	/* Close CDS */
	wlan_ftm_cds_close(cds_context);

#if defined(QCA_WIFI_FTM) && defined(LINUX_QCMBR)
	spin_lock_bh(&qcmbr_queue_lock);
	if (!list_empty(&qcmbr_queue_head)) {
		qcmbr_queue_t *msg_buf, *tmp_buf;
		list_for_each_entry_safe(msg_buf, tmp_buf, &qcmbr_queue_head,
					 list) {
			list_del(&msg_buf->list);
			kfree(msg_buf);
		}
	}
	spin_unlock_bh(&qcmbr_queue_lock);
#endif

	return 0;

}


/**
 * hdd_ftm_mc_process_msg() - Process FTM mailbox message
 * @message: FTM response message
 *
 * Process FTM mailbox message
 *
 * Return: void
 */
static void hdd_ftm_mc_process_msg(void *message)
{
	void *data;
	uint32_t data_len;

	if (!message) {
		hdd_err("Message is NULL, nothing to process.");
		return;
	}

	data_len = *((uint32_t *) message);
	data = (uint32_t *) message + 1;

#if defined(LINUX_QCMBR)
	wlanqcmbr_mc_process_msg(message);
#else
#ifdef CONFIG_NL80211_TESTMODE
	wlan_hdd_testmode_rx_event(data, (size_t) data_len);
#endif
#endif
	return;
}

/**
 * wlan_hdd_ftm_start() - Start HDD in FTM mode
 * @hdd_ctx: Global HDD context
 *
 * This function  starts the following modules.
 * 1) WMA Start.
 * 2) HTC Start.
 * 3) MAC Start to download the firmware.
 *
 * Return: 0 for success, non zero for failure
 */
static int wlan_hdd_ftm_start(hdd_context_t *hdd_ctx)
{
	CDF_STATUS vStatus = CDF_STATUS_SUCCESS;
	p_cds_contextType p_cds_context =
		(p_cds_contextType) (hdd_ctx->pcds_context);

	if (WLAN_FTM_STARTED == hdd_ctx->ftm.ftm_state) {
		return 0;
	}

	CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO,
		  "%s: Starting CLD SW", __func__);

	/* We support only one instance for now ... */
	if (p_cds_context == NULL) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: mismatch in context", __func__);
		goto err_status_failure;
	}

	if (p_cds_context->pMACContext == NULL) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: MAC NULL context", __func__);
		goto err_status_failure;
	}

	/* Vos preStart is calling */
	if (!CDF_IS_STATUS_SUCCESS(cds_ftm_pre_start(hdd_ctx->pcds_context))) {
		hddLog(CDF_TRACE_LEVEL_FATAL, "%s: cds_pre_enable failed",
		       __func__);
		goto err_status_failure;
	}

	sme_register_ftm_msg_processor(hdd_ctx->hHal, hdd_ftm_mc_process_msg);

	vStatus = wma_start(p_cds_context);
	if (vStatus != CDF_STATUS_SUCCESS) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to start WMA", __func__);
		goto err_status_failure;
	}

	CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO,
		  "%s: MAC correctly started", __func__);

	if (hdd_ftm_service_registration(hdd_ctx)) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  "%s: failed", __func__);
		goto err_ftm_service_reg;
	}

	hdd_ctx->ftm.ftm_state = WLAN_FTM_STARTED;

	return 0;

err_ftm_service_reg:
	wlan_hdd_ftm_close(hdd_ctx);

err_status_failure:

	return -EPERM;

}

#if  defined(QCA_WIFI_FTM)
/**
 * hdd_ftm_start() - Start HDD in FTM mode
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 for success, non zero for failure
 */
int hdd_ftm_start(hdd_context_t *hdd_ctx)
{
	return wlan_hdd_ftm_start(hdd_ctx);
}
#endif

#if  defined(QCA_WIFI_FTM)
/**
 * hdd_ftm_stop() - Stop HDD in FTM mode
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 for success, non zero for failure
 */
int hdd_ftm_stop(hdd_context_t *hdd_ctx)
{
	return wlan_ftm_stop(hdd_ctx);
}
#endif

#if  defined(QCA_WIFI_FTM)
#if defined(LINUX_QCMBR)
/**
 * wlan_hdd_qcmbr_command() - QCMBR command handler
 * @adapter: adapter upon which the command was received
 * @pqcmbr_data: QCMBR command
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_command(hdd_adapter_t *adapter,
				  qcmbr_data_t *pqcmbr_data)
{
	int ret = 0;
	qcmbr_queue_t *qcmbr_buf = NULL;

	switch (pqcmbr_data->cmd) {
	case ATH_XIOCTL_UNIFIED_UTF_CMD: {
		pqcmbr_data->copy_to_user = 0;
		if (pqcmbr_data->length) {
			if (wlan_hdd_ftm_testmode_cmd(pqcmbr_data->buf,
						      pqcmbr_data->
						      length)
			    != CDF_STATUS_SUCCESS) {
				ret = -EBUSY;
			} else {
				ret = 0;
			}
		}
	}
	break;

	case ATH_XIOCTL_UNIFIED_UTF_RSP: {
		pqcmbr_data->copy_to_user = 1;
		if (!list_empty(&qcmbr_queue_head)) {
			spin_lock_bh(&qcmbr_queue_lock);
			qcmbr_buf = list_first_entry(&qcmbr_queue_head,
						     qcmbr_queue_t,
						     list);
			list_del(&qcmbr_buf->list);
			spin_unlock_bh(&qcmbr_queue_lock);
			ret = 0;
		} else {
			ret = -1;
		}

		if (!ret) {
			memcpy(pqcmbr_data->buf, qcmbr_buf->utf_buf,
			       (MAX_UTF_LENGTH + 4));
			kfree(qcmbr_buf);
		} else {
			ret = -EAGAIN;
		}
	}
	break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
/**
 * wlan_hdd_qcmbr_ioctl() - Compatability-mode QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_compat_ioctl(hdd_adapter_t *adapter,
				       struct ifreq *ifr)
{
	qcmbr_data_t *qcmbr_data;
	int ret = 0;

	qcmbr_data = kzalloc(sizeof(qcmbr_data_t), GFP_KERNEL);
	if (qcmbr_data == NULL)
		return -ENOMEM;

	if (copy_from_user(qcmbr_data, ifr->ifr_data, sizeof(*qcmbr_data))) {
		ret = -EFAULT;
		goto exit;
	}

	ret = wlan_hdd_qcmbr_command(adapter, qcmbr_data);
	if (qcmbr_data->copy_to_user) {
		ret = copy_to_user(ifr->ifr_data, qcmbr_data->buf,
				   (MAX_UTF_LENGTH + 4));
	}

exit:
	kfree(qcmbr_data);
	return ret;
}
#else                           /* CONFIG_COMPAT */
static int wlan_hdd_qcmbr_compat_ioctl(hdd_adapter_t *adapter,
				       struct ifreq *ifr)
{
	return 0;
}
#endif /* CONFIG_COMPAT */

/**
 * wlan_hdd_qcmbr_ioctl() - Standard QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	qcmbr_data_t *qcmbr_data;
	int ret = 0;

	qcmbr_data = kzalloc(sizeof(qcmbr_data_t), GFP_KERNEL);
	if (qcmbr_data == NULL)
		return -ENOMEM;

	if (copy_from_user(qcmbr_data, ifr->ifr_data, sizeof(*qcmbr_data))) {
		ret = -EFAULT;
		goto exit;
	}

	ret = wlan_hdd_qcmbr_command(adapter, qcmbr_data);
	if (qcmbr_data->copy_to_user) {
		ret = copy_to_user(ifr->ifr_data, qcmbr_data->buf,
				   (MAX_UTF_LENGTH + 4));
	}

exit:
	kfree(qcmbr_data);
	return ret;
}

/**
 * wlan_hdd_qcmbr_unified_ioctl() - Unified QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
int wlan_hdd_qcmbr_unified_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	int ret = 0;

	if (is_compat_task()) {
		ret = wlan_hdd_qcmbr_compat_ioctl(adapter, ifr);
	} else {
		ret = wlan_hdd_qcmbr_ioctl(adapter, ifr);
	}

	return ret;
}

/**
 * wlanqcmbr_mc_process_msg() - Process QCMBR response message
 * @message: QCMBR message
 *
 * Return: None
 */
static void wlanqcmbr_mc_process_msg(void *message)
{
	qcmbr_queue_t *qcmbr_buf = NULL;
	uint32_t data_len;

	data_len = *((uint32_t *) message) + sizeof(uint32_t);
	qcmbr_buf = kzalloc(sizeof(qcmbr_queue_t), GFP_KERNEL);
	if (qcmbr_buf != NULL) {
		memcpy(qcmbr_buf->utf_buf, message, data_len);
		spin_lock_bh(&qcmbr_queue_lock);
		list_add_tail(&(qcmbr_buf->list), &qcmbr_queue_head);
		spin_unlock_bh(&qcmbr_queue_lock);
	}
}
#endif /*LINUX_QCMBR */

/**
 * wlan_hdd_ftm_testmode_cmd() - Process FTM testmode command
 * @data: FTM testmode command
 * @len: length of @data
 *
 * Return: CDF_STATUS_SUCCESS on success, CDF_STATUS_E_* on error
 */
CDF_STATUS wlan_hdd_ftm_testmode_cmd(void *data, int len)
{
	struct ar6k_testmode_cmd_data *cmd_data;

	cmd_data = (struct ar6k_testmode_cmd_data *)
		   cdf_mem_malloc(sizeof(*cmd_data));

	if (!cmd_data) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  ("Failed to allocate FTM command data"));
		return CDF_STATUS_E_NOMEM;
	}

	cmd_data->data = cdf_mem_malloc(len);

	if (!cmd_data->data) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_ERROR,
			  ("Failed to allocate FTM command data buffer"));
		cdf_mem_free(cmd_data);
		return CDF_STATUS_E_NOMEM;
	}

	cmd_data->len = len;
	cdf_mem_copy(cmd_data->data, data, len);

	if (wlan_ftm_postmsg((uint8_t *) cmd_data, sizeof(*cmd_data))) {
		cdf_mem_free(cmd_data->data);
		cdf_mem_free(cmd_data);
		return CDF_STATUS_E_FAILURE;
	}

	return CDF_STATUS_SUCCESS;
}
#endif /*QCA_WIFI_FTM */
