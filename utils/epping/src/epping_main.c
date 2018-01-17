/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

/*========================================================================

   \file  epping_main.c

   \brief WLAN End Point Ping test tool implementation

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cds_api.h>
#include <cds_sched.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <wni_api.h>
#include <wlan_ptt_sock_svc.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <linux/rtnetlink.h>
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include "bmi.h"
#include "ol_fw.h"
#include "ol_if_athvar.h"
#include "hif.h"
#include "epping_main.h"
#include "epping_internal.h"
#include "wlan_policy_mgr_api.h"

#ifdef TIMER_MANAGER
#define TIMER_MANAGER_STR " +TIMER_MANAGER"
#else
#define TIMER_MANAGER_STR ""
#endif

#ifdef MEMORY_DEBUG
#define MEMORY_DEBUG_STR " +MEMORY_DEBUG"
#else
#define MEMORY_DEBUG_STR ""
#endif

#ifdef HIF_SDIO
#define WLAN_WAIT_TIME_WLANSTART 10000
#else
#define WLAN_WAIT_TIME_WLANSTART 2000
#endif

#ifdef WLAN_FEATURE_EPPING
static struct epping_context *g_epping_ctx;

/**
 * epping_open(): End point ping driver open Function
 *
 * This function is called by HDD to open epping module
 *
 *
 * return - 0 for success, negative for failure
 */
int epping_open(void)
{
	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH, "%s: Enter", __func__);

	g_epping_ctx = qdf_mem_malloc(sizeof(*g_epping_ctx));

	if (g_epping_ctx == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_ERROR,
				"%s: cannot alloc epping context", __func__);
		return -ENOMEM;
	}

	g_epping_ctx->con_mode = cds_get_conparam();
	return 0;
}

/**
 * epping_disable(): End point ping driver disable Function
 *
 * This is the driver disable function - called by HDD to
 * disable epping module
 *
 * return: none
 */
void epping_disable(void)
{
	epping_context_t *pEpping_ctx;
	struct hif_opaque_softc *hif_ctx;
	HTC_HANDLE htc_handle;

	pEpping_ctx = g_epping_ctx;
	if (pEpping_ctx == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: error: pEpping_ctx  = NULL", __func__);
		return;
	}

	if (pEpping_ctx->epping_adapter) {
		epping_destroy_adapter(pEpping_ctx->epping_adapter);
		pEpping_ctx->epping_adapter = NULL;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (hif_ctx == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: error: hif_ctx = NULL", __func__);
		return;
	}
	hif_disable_isr(hif_ctx);
	hif_reset_soc(hif_ctx);

	htc_handle = cds_get_context(QDF_MODULE_ID_HTC);
	if (htc_handle == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: error: htc_handle = NULL", __func__);
		return;
	}
	htc_stop(htc_handle);
	epping_cookie_cleanup(pEpping_ctx);
	htc_destroy(htc_handle);
}

/**
 * epping_close(): End point ping driver close Function
 *
 * This is the driver close function - called by HDD to close epping module
 *
 * return: none
 */
void epping_close(void)
{
	epping_context_t *to_free;


	if (g_epping_ctx == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: error: g_epping_ctx  = NULL", __func__);
		return;
	}

	to_free = g_epping_ctx;
	g_epping_ctx = NULL;
	qdf_mem_free(to_free);
}

/**
 * epping_target_suspend_acknowledge() - process wow ack/nack from fw
 * @context: htc_init_info->context
 * @wow_nack: true when wow is rejected
 */
static void epping_target_suspend_acknowledge(void *context, bool wow_nack)
{
	if (NULL == g_epping_ctx) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: epping_ctx is NULL", __func__);
		return;
	}
	/* EPPING_TODO: do we need wow_nack? */
	g_epping_ctx->wow_nack = wow_nack;
}

/**
 * epping_update_ol_config - API to update ol configuration parameters
 *
 * Return: void
 */
static void epping_update_ol_config(void)
{
	struct ol_config_info cfg;
	struct ol_context *ol_ctx = cds_get_context(QDF_MODULE_ID_BMI);

	if (!ol_ctx)
		return;

	cfg.enable_self_recovery = 0;
	cfg.enable_uart_print = 0;
	cfg.enable_fw_log = 0;
	cfg.enable_ramdump_collection = 0;
	cfg.enable_lpass_support = 0;

	ol_init_ini_config(ol_ctx, &cfg);
}
/**
 * epping_enable(): End point ping driver enable Function
 *
 * This is the driver enable function - called by HDD to enable
 * epping module
 *
 * return - 0 : success, negative: error
 */
int epping_enable(struct device *parent_dev)
{
	int ret = 0;
	epping_context_t *pEpping_ctx = NULL;
	cds_context_type *p_cds_context = NULL;
	qdf_device_t qdf_ctx;
	struct htc_init_info htcInfo;
	struct hif_opaque_softc *scn;
	tSirMacAddr adapter_macAddr;
	struct hif_target_info *tgt_info;
	struct ol_context *ol_ctx;

	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH, "%s: Enter", __func__);

	p_cds_context = cds_get_global_context();

	if (p_cds_context == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: Failed cds_get_global_context", __func__);
		ret = -1;
		return ret;
	}

	pEpping_ctx = g_epping_ctx;
	if (pEpping_ctx == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: Failed to get pEpping_ctx", __func__);
		ret = -1;
		return ret;
	}
	pEpping_ctx->parent_dev = (void *)parent_dev;
	epping_get_dummy_mac_addr(adapter_macAddr);

	/* Initialize the timer module */
	qdf_timer_module_init();

	scn = cds_get_context(QDF_MODULE_ID_HIF);
	if (!scn) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: scn is null!", __func__);
		return A_ERROR;
	}

	tgt_info = hif_get_target_info_handle(scn);

	/* store target type and target version info in hdd ctx */
	pEpping_ctx->target_type = tgt_info->target_type;

	ol_ctx = cds_get_context(QDF_MODULE_ID_BMI);
	if (!ol_ctx) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: ol_ctx is NULL", __func__);
		return A_ERROR;
	}

	epping_update_ol_config();
#ifndef FEATURE_BMI_2
	/* Initialize BMI and Download firmware */
	if (bmi_download_firmware(ol_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: BMI failed to download target", __func__);
		bmi_cleanup(ol_ctx);
		return A_ERROR;
	}
#endif
	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
		   "%s: bmi_download_firmware done", __func__);

	htcInfo.pContext = ol_ctx;
	htcInfo.TargetFailure = ol_target_failure;
	htcInfo.TargetSendSuspendComplete = epping_target_suspend_acknowledge;
	qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	/* Create HTC */
	p_cds_context->htc_ctx = htc_create(scn, &htcInfo, qdf_ctx,
					    cds_get_conparam());
	if (!p_cds_context->htc_ctx) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: Failed to Create HTC", __func__);
		bmi_cleanup(ol_ctx);
		return A_ERROR;
	}
	pEpping_ctx->HTCHandle =
		cds_get_context(QDF_MODULE_ID_HTC);
	if (pEpping_ctx->HTCHandle == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: HTCHandle is NULL", __func__);
		return A_ERROR;
	}

	if (bmi_done(ol_ctx)) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: Failed to complete BMI phase", __func__);
		goto error_end;
	}

	/* start HIF */
	if (htc_wait_target(pEpping_ctx->HTCHandle) != QDF_STATUS_SUCCESS) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: htc_wait_target error", __func__);
		goto error_end;
	}
	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH, "%s: HTC ready", __func__);

	ret = epping_connect_service(pEpping_ctx);
	if (ret != 0) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: htc_wait_targetdone", __func__);
		goto error_end;
	}
	if (htc_start(pEpping_ctx->HTCHandle) != QDF_STATUS_SUCCESS) {
		goto error_end;
	}
	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH, "%s: HTC started", __func__);

	/* init the tx cookie resource */
	ret = epping_cookie_init(pEpping_ctx);
	if (ret == 0) {
		pEpping_ctx->epping_adapter = epping_add_adapter(pEpping_ctx,
								 adapter_macAddr,
								 QDF_STA_MODE);
	}
	if (ret < 0 || pEpping_ctx->epping_adapter == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: epping_add_adaptererror error", __func__);
		htc_stop(pEpping_ctx->HTCHandle);
		epping_cookie_cleanup(pEpping_ctx);
		goto error_end;
	}

	EPPING_LOG(QDF_TRACE_LEVEL_INFO_HIGH, "%s: Exit", __func__);
	return ret;

error_end:
	htc_destroy(p_cds_context->htc_ctx);
	p_cds_context->htc_ctx = NULL;
	bmi_cleanup(ol_ctx);
	return A_ERROR;
}
#endif
