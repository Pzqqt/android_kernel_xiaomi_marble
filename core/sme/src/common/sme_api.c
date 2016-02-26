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

/**=========================================================================

   \file  smeApi.c

   \brief Definitions for SME APIs

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/

#include "sms_debug.h"
#include "sme_api.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "csr_internal.h"
#include "wma_types.h"
#include "wma_if.h"
#include "qdf_trace.h"
#include "sme_trace.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_utils.h"
#include "sap_api.h"
#include "mac_trace.h"
#ifdef WLAN_FEATURE_NAN
#include "nan_api.h"
#endif
#include "cds_regdomain.h"
#include "cfg_api.h"
#include "sme_power_save_api.h"
#include "wma.h"
#include "sch_api.h"
#include "sme_nan_datapath.h"

extern tSirRetStatus u_mac_post_ctrl_msg(void *pSirGlobal, tSirMbMsg *pMb);

#define LOG_SIZE 256

static tSelfRecoveryStats g_self_recovery_stats;
/* TxMB Functions */
extern QDF_STATUS pmc_prepare_command(tpAniSirGlobal pMac, uint32_t sessionId,
				      eSmeCommandType cmdType, void *pvParam,
				      uint32_t size, tSmeCmd **ppCmd);
extern void pmc_release_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
extern void qos_release_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
extern QDF_STATUS p2p_process_remain_on_channel_cmd(tpAniSirGlobal pMac,
						    tSmeCmd *p2pRemainonChn);
extern QDF_STATUS sme_remain_on_chn_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);
extern QDF_STATUS sme_remain_on_chn_ready(tHalHandle hHal, uint8_t *pMsg);
extern QDF_STATUS sme_send_action_cnf(tHalHandle hHal, uint8_t *pMsg);

static QDF_STATUS init_sme_cmd_list(tpAniSirGlobal pMac);
static void sme_abort_command(tpAniSirGlobal pMac, tSmeCmd *pCommand,
			      bool fStopping);

eCsrPhyMode sme_get_phy_mode(tHalHandle hHal);

QDF_STATUS sme_handle_change_country_code(tpAniSirGlobal pMac, void *pMsgBuf);

void sme_disconnect_connected_sessions(tpAniSirGlobal pMac);

QDF_STATUS sme_handle_generic_change_country_code(tpAniSirGlobal pMac,
						  void *pMsgBuf);

QDF_STATUS sme_process_nss_update_resp(tpAniSirGlobal mac, uint8_t *msg);

#ifdef FEATURE_WLAN_ESE
bool csr_is_supported_channel(tpAniSirGlobal pMac, uint8_t channelId);
#endif

#ifdef WLAN_FEATURE_11W
QDF_STATUS sme_unprotected_mgmt_frm_ind(tHalHandle hHal,
					tpSirSmeUnprotMgmtFrameInd pSmeMgmtFrm);
#endif

/* Message processor for events from DFS */
QDF_STATUS dfs_msg_processor(tpAniSirGlobal pMac,
			     uint16_t msg_type, void *pMsgBuf);

/* Channel Change Response Indication Handler */
QDF_STATUS sme_process_channel_change_resp(tpAniSirGlobal pMac,
					   uint16_t msg_type, void *pMsgBuf);

/* Internal SME APIs */
QDF_STATUS sme_acquire_global_lock(tSmeStruct *psSme)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psSme) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_acquire(&psSme->lkSmeGlobalLock))) {
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

QDF_STATUS sme_release_global_lock(tSmeStruct *psSme)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psSme) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_release(&psSme->lkSmeGlobalLock))) {
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/**
 * sme_process_set_hw_mode_resp() - Process set HW mode response
 * @mac: Global MAC pointer
 * @msg: HW mode response
 *
 * Processes the HW mode response and invokes the HDD callback
 * to process further
 */
static QDF_STATUS sme_process_set_hw_mode_resp(tpAniSirGlobal mac, uint8_t *msg)
{
	tListElem *entry = NULL;
	tSmeCmd *command = NULL;
	bool found;
	hw_mode_cb callback = NULL;
	struct sir_set_hw_mode_resp *param;
	enum sir_conn_update_reason reason;
	tSmeCmd *saved_cmd;

	sms_log(mac, LOG1, FL("%s"), __func__);
	param = (struct sir_set_hw_mode_resp *)msg;
	if (!param) {
		sms_log(mac, LOGE, FL("HW mode resp param is NULL"));
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_ll_peek_head(&mac->sme.smeCmdActiveList,
			LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac, LOGE, FL("No cmd found in active list"));
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sms_log(mac, LOGE, FL("Base address is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_hw_mode != command->command) {
		sms_log(mac, LOGE, FL("Command mismatch!"));
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.set_hw_mode_cmd.set_hw_mode_cb;
	reason = command->u.set_hw_mode_cmd.reason;

	sms_log(mac, LOG1, FL("reason:%d session:%d"),
		command->u.set_hw_mode_cmd.reason,
		command->u.set_hw_mode_cmd.session_id);

	if (!callback) {
		sms_log(mac, LOGE, FL("Callback does not exist"));
		goto end;
	}

	if (!param) {
		sms_log(mac, LOGE,
			FL("Callback failed since HW mode params is NULL"));
		goto end;
	}

	/* Irrespective of the reason for which the hw mode change request
	 * was issued, the policy manager connection table needs to be updated
	 * with the new vdev-mac id mapping, tx/rx spatial streams etc., if the
	 * set hw mode was successful.
	 */
	callback(param->status,
			param->cfgd_hw_mode_index,
			param->num_vdev_mac_entries,
			param->vdev_mac_map);

	if (reason == SIR_UPDATE_REASON_HIDDEN_STA) {
		/* In the case of hidden SSID, connection update
		 * (set hw mode) is done after the scan with reason
		 * code eCsrScanForSsid completes. The connect/failure
		 * needs to be handled after the response of set hw
		 * mode
		 */
		saved_cmd = (tSmeCmd *)mac->sme.saved_scan_cmd;
		if (!saved_cmd) {
			sms_log(mac, LOGP,
					FL("saved cmd is NULL, Check this"));
			goto end;
		}
		if (param->status == SET_HW_MODE_STATUS_OK) {
			sms_log(mac, LOG1,
					FL("search for ssid success"));
			csr_scan_handle_search_for_ssid(mac,
					saved_cmd);
		} else {
			sms_log(mac, LOG1,
					FL("search for ssid failure"));
			csr_scan_handle_search_for_ssid_failure(mac,
					saved_cmd);
		}
		if (saved_cmd->u.roamCmd.pRoamBssEntry)
			qdf_mem_free(
					saved_cmd->u.roamCmd.pRoamBssEntry);
		if (saved_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList)
			qdf_mem_free(saved_cmd->u.scanCmd.u.
					scanRequest.SSIDs.SSIDList);
		if (saved_cmd->u.scanCmd.pToRoamProfile)
			qdf_mem_free(saved_cmd->u.scanCmd.
					pToRoamProfile);
		if (saved_cmd) {
			qdf_mem_free(saved_cmd);
			saved_cmd = NULL;
			mac->sme.saved_scan_cmd = NULL;
		}
	}

end:
	found = csr_ll_remove_entry(&mac->sme.smeCmdActiveList, entry,
			LL_ACCESS_LOCK);
	if (found) {
		/* Now put this command back on the avilable command list */
		sme_release_command(mac, command);
	}
	sme_process_pending_queue(mac);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_process_hw_mode_trans_ind() - Process HW mode transition indication
 * @mac: Global MAC pointer
 * @msg: HW mode transition response
 *
 * Processes the HW mode transition indication and invoke the HDD callback
 * to process further
 */
static QDF_STATUS sme_process_hw_mode_trans_ind(tpAniSirGlobal mac,
						uint8_t *msg)
{
	hw_mode_transition_cb callback = NULL;
	struct sir_hw_mode_trans_ind *param;

	param = (struct sir_hw_mode_trans_ind *)msg;
	if (!param) {
		sms_log(mac, LOGE, FL("HW mode trans ind param is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	callback = mac->sme.sme_hw_mode_trans_cb;
	if (callback) {
		sms_log(mac, LOGE, FL("Calling registered callback..."));
		callback(param->old_hw_mode_index,
			param->new_hw_mode_index,
			param->num_vdev_mac_entries,
			param->vdev_mac_map);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * free_sme_cmds() - This function frees memory allocated for SME commands
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function frees memory allocated for SME commands
 *
 * @Return: void
 */
static void free_sme_cmds(tpAniSirGlobal mac_ctx)
{
	uint32_t idx;
	if (NULL == mac_ctx->sme.pSmeCmdBufAddr)
		return;

	for (idx = 0; idx < mac_ctx->sme.totalSmeCmd; idx++)
		qdf_mem_free(mac_ctx->sme.pSmeCmdBufAddr[idx]);

	qdf_mem_free(mac_ctx->sme.pSmeCmdBufAddr);
	mac_ctx->sme.pSmeCmdBufAddr = NULL;
}

static QDF_STATUS init_sme_cmd_list(tpAniSirGlobal pMac)
{
	QDF_STATUS status;
	tSmeCmd *pCmd;
	uint32_t cmd_idx;
	QDF_STATUS qdf_status;
	qdf_mc_timer_t *cmdTimeoutTimer = NULL;
	uint32_t sme_cmd_ptr_ary_sz;

	pMac->sme.totalSmeCmd = SME_TOTAL_COMMAND;

	status = csr_ll_open(pMac->hHdd, &pMac->sme.smeCmdActiveList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	status = csr_ll_open(pMac->hHdd, &pMac->sme.smeCmdPendingList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	status = csr_ll_open(pMac->hHdd, &pMac->sme.smeScanCmdActiveList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	status = csr_ll_open(pMac->hHdd, &pMac->sme.smeScanCmdPendingList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	status = csr_ll_open(pMac->hHdd, &pMac->sme.smeCmdFreeList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	/* following pointer contains array of pointers for tSmeCmd* */
	sme_cmd_ptr_ary_sz = sizeof(void *) * pMac->sme.totalSmeCmd;
	pMac->sme.pSmeCmdBufAddr = qdf_mem_malloc(sme_cmd_ptr_ary_sz);
	if (NULL == pMac->sme.pSmeCmdBufAddr) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	status = QDF_STATUS_SUCCESS;
	qdf_mem_set(pMac->sme.pSmeCmdBufAddr, sme_cmd_ptr_ary_sz, 0);
	for (cmd_idx = 0; cmd_idx < pMac->sme.totalSmeCmd; cmd_idx++) {
		/*
		 * Since total size of all commands together can be huge chunk
		 * of memory, allocate SME cmd individually. These SME CMDs are
		 * moved between pending and active queues. And these freeing of
		 * these queues just manipulates the list but does not actually
		 * frees SME CMD pointers. Hence store each SME CMD address in
		 * the array, sme.pSmeCmdBufAddr. This will later facilitate
		 * freeing up of all SME CMDs with just a for loop.
		 */
		pMac->sme.pSmeCmdBufAddr[cmd_idx] =
						qdf_mem_malloc(sizeof(tSmeCmd));
		if (NULL == pMac->sme.pSmeCmdBufAddr[cmd_idx]) {
			status = QDF_STATUS_E_NOMEM;
			free_sme_cmds(pMac);
			goto end;
		}
		pCmd = (tSmeCmd *)pMac->sme.pSmeCmdBufAddr[cmd_idx];
		csr_ll_insert_tail(&pMac->sme.smeCmdFreeList,
				&pCmd->Link, LL_ACCESS_LOCK);
	}

	/* This timer is only to debug the active list command timeout */

	cmdTimeoutTimer =
		(qdf_mc_timer_t *) qdf_mem_malloc(sizeof(qdf_mc_timer_t));
	if (cmdTimeoutTimer) {
		pMac->sme.smeCmdActiveList.cmdTimeoutTimer = cmdTimeoutTimer;
		qdf_status =
			qdf_mc_timer_init(pMac->sme.smeCmdActiveList.
					  cmdTimeoutTimer, QDF_TIMER_TYPE_SW,
					  active_list_cmd_timeout_handle, (void *)pMac);

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "Init Timer fail for active list command process time out");
			qdf_mem_free(pMac->sme.smeCmdActiveList.
				     cmdTimeoutTimer);
			pMac->sme.smeCmdActiveList.cmdTimeoutTimer = NULL;
		} else {
			pMac->sme.smeCmdActiveList.cmdTimeoutDuration =
				CSR_ACTIVE_LIST_CMD_TIMEOUT_VALUE;
		}
	}

end:
	if (!QDF_IS_STATUS_SUCCESS(status))
		sms_log(pMac, LOGE, "failed to initialize sme command list:%d\n",
			status);

	return status;
}

void sme_release_command(tpAniSirGlobal pMac, tSmeCmd *pCmd)
{
	pCmd->command = eSmeNoCommand;
	csr_ll_insert_tail(&pMac->sme.smeCmdFreeList, &pCmd->Link, LL_ACCESS_LOCK);
}

static void sme_release_cmd_list(tpAniSirGlobal pMac, tDblLinkList *pList)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;

	while ((pEntry = csr_ll_remove_head(pList, LL_ACCESS_LOCK)) != NULL) {
		/* TODO: base on command type to call release functions */
		/* reinitialize different command types so they can be reused */
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		sme_abort_command(pMac, pCommand, true);
	}
}

static void purge_sme_cmd_list(tpAniSirGlobal pMac)
{
	/* release any out standing commands back to free command list */
	sme_release_cmd_list(pMac, &pMac->sme.smeCmdPendingList);
	sme_release_cmd_list(pMac, &pMac->sme.smeCmdActiveList);
	sme_release_cmd_list(pMac, &pMac->sme.smeScanCmdPendingList);
	sme_release_cmd_list(pMac, &pMac->sme.smeScanCmdActiveList);
}

void purge_sme_session_cmd_list(tpAniSirGlobal pMac, uint32_t sessionId,
				tDblLinkList *pList)
{
	/* release any out standing commands back to free command list */
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
		sme_abort_command(pMac, pCommand, true);
	}
	csr_ll_close(&localList);
}

static QDF_STATUS free_sme_cmd_list(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	purge_sme_cmd_list(pMac);
	csr_ll_close(&pMac->sme.smeCmdPendingList);
	csr_ll_close(&pMac->sme.smeCmdActiveList);
	csr_ll_close(&pMac->sme.smeScanCmdPendingList);
	csr_ll_close(&pMac->sme.smeScanCmdActiveList);
	csr_ll_close(&pMac->sme.smeCmdFreeList);

	/*destroy active list command time out timer */
	qdf_mc_timer_destroy(pMac->sme.smeCmdActiveList.cmdTimeoutTimer);
	qdf_mem_free(pMac->sme.smeCmdActiveList.cmdTimeoutTimer);
	pMac->sme.smeCmdActiveList.cmdTimeoutTimer = NULL;

	status = qdf_mutex_acquire(&pMac->sme.lkSmeGlobalLock);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE,
			FL("Failed to acquire the lock status = %d"), status);
		goto done;
	}

	free_sme_cmds(pMac);

	status = qdf_mutex_release(&pMac->sme.lkSmeGlobalLock);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE,
			FL("Failed to release the lock status = %d"), status);
	}
done:
	return status;
}

void dump_csr_command_info(tpAniSirGlobal pMac, tSmeCmd *pCmd)
{
	switch (pCmd->command) {
	case eSmeCommandScan:
		sms_log(pMac, LOGE, " scan command reason is %d",
			pCmd->u.scanCmd.reason);
		break;

	case eSmeCommandRoam:
		sms_log(pMac, LOGE, " roam command reason is %d",
			pCmd->u.roamCmd.roamReason);
		break;

	case eSmeCommandWmStatusChange:
		sms_log(pMac, LOGE, " WMStatusChange command type is %d",
			pCmd->u.wmStatusChangeCmd.Type);
		break;

	case eSmeCommandSetKey:
		sms_log(pMac, LOGE, " setKey command auth(%d) enc(%d)",
			pCmd->u.setKeyCmd.authType, pCmd->u.setKeyCmd.encType);
		break;

	default:
		sms_log(pMac, LOGE, " default: Unhandled command %d",
			pCmd->command);
		break;
	}
}

tSmeCmd *sme_get_command_buffer(tpAniSirGlobal pMac)
{
	tSmeCmd *pRetCmd = NULL, *pTempCmd = NULL;
	tListElem *pEntry;
	static int sme_command_queue_full;

	pEntry = csr_ll_remove_head(&pMac->sme.smeCmdFreeList, LL_ACCESS_LOCK);

	/* If we can get another MS Msg buffer, then we are ok.  Just link */
	/* the entry onto the linked list.  (We are using the linked list */
	/* to keep track of tfhe message buffers). */
	if (pEntry) {
		pRetCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		/* reset when free list is available */
		sme_command_queue_full = 0;
	} else {
		int idx = 1;

		/* Cannot change pRetCmd here since it needs to return later. */
		pEntry =
			csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
		if (pEntry) {
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		}
		sms_log(pMac, LOGE,
			"Out of command buffer.... command (0x%X) stuck",
			(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
		if (pTempCmd) {
			if (eSmeCsrCommandMask & pTempCmd->command) {
				/* CSR command is stuck. See what the reason code is for that command */
				dump_csr_command_info(pMac, pTempCmd);
			}
		} /* if(pTempCmd) */

		/* dump what is in the pending queue */
		csr_ll_lock(&pMac->sme.smeCmdPendingList);
		pEntry =
			csr_ll_peek_head(&pMac->sme.smeCmdPendingList,
					 LL_ACCESS_NOLOCK);
		while (pEntry && !sme_command_queue_full) {
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			/* Print only 1st five commands from pending queue. */
			if (idx <= 5)
				sms_log(pMac, LOGE,
					"Out of command buffer.... SME pending command #%d (0x%X)",
					idx, pTempCmd->command);
			idx++;
			if (eSmeCsrCommandMask & pTempCmd->command) {
				/* CSR command is stuck. See what the reason code is for that command */
				dump_csr_command_info(pMac, pTempCmd);
			}
			pEntry =
				csr_ll_next(&pMac->sme.smeCmdPendingList, pEntry,
					    LL_ACCESS_NOLOCK);
		}
		csr_ll_unlock(&pMac->sme.smeCmdPendingList);

		idx = 1;
		/* There may be some more command in CSR's own pending queue */
		csr_ll_lock(&pMac->roam.roamCmdPendingList);
		pEntry =
			csr_ll_peek_head(&pMac->roam.roamCmdPendingList,
					 LL_ACCESS_NOLOCK);
		while (pEntry  && !sme_command_queue_full) {
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			/* Print only 1st five commands from CSR pending queue */
			if (idx <= 5)
				sms_log(pMac, LOGE,
					"Out of command buffer.... "
					"CSR roamCmdPendingList command #%d (0x%X)",
					idx, pTempCmd->command);
			idx++;
			dump_csr_command_info(pMac, pTempCmd);
			pEntry =
				csr_ll_next(&pMac->roam.roamCmdPendingList, pEntry,
					    LL_ACCESS_NOLOCK);
		}

		/* Increment static variable so that it prints
		 * pending command only once
		 */
		sme_command_queue_full++;
		csr_ll_unlock(&pMac->roam.roamCmdPendingList);

		if (pMac->roam.configParam.enable_fatal_event)
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF,
				false,
				pMac->sme.enableSelfRecovery ? true : false);
		else if (pMac->sme.enableSelfRecovery)
			cds_trigger_recovery();
		else
			QDF_BUG(0);
	}

	/* memset to zero */
	if (pRetCmd) {
		qdf_mem_set((uint8_t *)&pRetCmd->command,
			    sizeof(pRetCmd->command), 0);
		qdf_mem_set((uint8_t *)&pRetCmd->sessionId,
			    sizeof(pRetCmd->sessionId), 0);
		qdf_mem_set((uint8_t *)&pRetCmd->u, sizeof(pRetCmd->u), 0);
	}

	return pRetCmd;
}

void sme_push_command(tpAniSirGlobal pMac, tSmeCmd *pCmd, bool fHighPriority)
{
	if (!SME_IS_START(pMac)) {
		sms_log(pMac, LOGE, FL("Sme in stop state"));
		QDF_ASSERT(0);
		return;
	}

	if (fHighPriority) {
		csr_ll_insert_head(&pMac->sme.smeCmdPendingList, &pCmd->Link,
				   LL_ACCESS_LOCK);
	} else {
		csr_ll_insert_tail(&pMac->sme.smeCmdPendingList, &pCmd->Link,
				   LL_ACCESS_LOCK);
	}

	/* process the command queue... */
	sme_process_pending_queue(pMac);

	return;
}

/* For commands that need to do extra cleanup. */
static void sme_abort_command(tpAniSirGlobal pMac, tSmeCmd *pCommand,
			      bool fStopping)
{
	if (eSmePmcCommandMask & pCommand->command) {
		sms_log(pMac, LOG1,
				"No need to process PMC commands");
		return;
	}
	if (eSmeCsrCommandMask & pCommand->command) {
		csr_abort_command(pMac, pCommand, fStopping);
		return;
	}
	switch (pCommand->command) {
	case eSmeCommandRemainOnChannel:
		if (NULL != pCommand->u.remainChlCmd.callback) {
			remainOnChanCallback callback =
				pCommand->u.remainChlCmd.callback;
			/* process the msg */
			if (callback) {
				callback(pMac, pCommand->u.remainChlCmd.
					callbackCtx, eCSR_SCAN_ABORT,
					pCommand->u.remainChlCmd.scan_id);
			}
		}
		sme_release_command(pMac, pCommand);
		break;
	default:
		sme_release_command(pMac, pCommand);
		break;
	}

}

tListElem *csr_get_cmd_to_process(tpAniSirGlobal pMac, tDblLinkList *pList,
				  uint8_t sessionId, bool fInterlocked)
{
	tListElem *pCurEntry = NULL;
	tSmeCmd *pCommand;

	/* Go through the list and return the command whose session id is not
	 * matching with the current ongoing scan cmd sessionId */
	pCurEntry = csr_ll_peek_head(pList, LL_ACCESS_LOCK);
	while (pCurEntry) {
		pCommand = GET_BASE_ADDR(pCurEntry, tSmeCmd, Link);
		if (pCommand->sessionId != sessionId) {
			sms_log(pMac, LOG1,
				"selected the command with different sessionId");
			return pCurEntry;
		}

		pCurEntry = csr_ll_next(pList, pCurEntry, fInterlocked);
	}

	sms_log(pMac, LOG1, "No command pending with different sessionId");
	return NULL;
}

bool sme_process_scan_queue(tpAniSirGlobal pMac)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tListElem *pSmeEntry = NULL;
	tSmeCmd *pSmeCommand = NULL;
	bool status = true;

	if ((!csr_ll_is_list_empty(&pMac->sme.smeCmdActiveList,
				   LL_ACCESS_LOCK))) {
		pSmeEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList,
					LL_ACCESS_LOCK);
		if (pSmeEntry)
			pSmeCommand = GET_BASE_ADDR(pSmeEntry, tSmeCmd, Link);
	}
	csr_ll_lock(&pMac->sme.smeScanCmdActiveList);
	if (csr_ll_is_list_empty(&pMac->sme.smeScanCmdPendingList,
				 LL_ACCESS_LOCK))
		goto end;
	pEntry = csr_ll_peek_head(&pMac->sme.smeScanCmdPendingList,
				  LL_ACCESS_LOCK);
	if (!pEntry)
		goto end;

	sms_log(pMac, LOGE,
		FL("scan_count in active scanlist %d "),
		pMac->sme.smeScanCmdActiveList.Count);

	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (pSmeCommand != NULL) {
		/*
		 * if scan is running on one interface and SME receives
		 * the next command on the same interface then
		 * dont the allow the command to be queued to
		 * smeCmdPendingList. If next scan is allowed on
		 * the same interface the CSR state machine will
		 * get screwed up.
		 */
		if (pSmeCommand->sessionId == pCommand->sessionId) {
			status = false;
			goto end;
		}
	}
	/*
	 * We cannot execute any command in wait-for-key state until setKey is
	 * through.
	 */
	if (CSR_IS_WAIT_FOR_KEY(pMac, pCommand->sessionId)) {
		if (!CSR_IS_SET_KEY_COMMAND(pCommand)) {
			sms_log(pMac, LOGE,
				FL("Can't process cmd(%d), waiting for key"),
				pCommand->command);
			status = false;
			goto end;
		}
	}
	if (csr_ll_remove_entry(&pMac->sme.smeScanCmdPendingList, pEntry,
				LL_ACCESS_LOCK)) {
		csr_ll_insert_head(&pMac->sme.smeScanCmdActiveList,
				   &pCommand->Link, LL_ACCESS_NOLOCK);
		switch (pCommand->command) {
		case eSmeCommandScan:
			sms_log(pMac, LOG1, FL("Processing scan offload cmd."));
			qdf_mc_timer_start(&pCommand->u.scanCmd.csr_scan_timer,
				CSR_ACTIVE_SCAN_LIST_CMD_TIMEOUT);
			csr_process_scan_command(pMac, pCommand);
			break;
		case eSmeCommandRemainOnChannel:
			sms_log(pMac, LOG1,
				FL("Processing remain on channel offload cmd"));
			p2p_process_remain_on_channel_cmd(pMac, pCommand);
			break;
		default:
			sms_log(pMac, LOGE,
				FL("Wrong cmd enqueued to ScanCmdPendingList"));
			pEntry = csr_ll_remove_head(
					&pMac->sme.smeScanCmdActiveList,
					LL_ACCESS_NOLOCK);
			pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			sme_release_command(pMac, pCommand);
			break;
		}
	}
end:
	csr_ll_unlock(&pMac->sme.smeScanCmdActiveList);
	return status;
}

/**
 * sme_process_command() - processes SME commnd
 * @mac_ctx:       mac global context
 *
 * This function is called by sme_process_pending_queue() in a while loop
 *
 * Return: true indicates that caller function can proceed to next cmd
 *         false otherwise.
 */
bool sme_process_command(tpAniSirGlobal pMac)
{
	bool fContinue = false;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tListElem *pSmeEntry;
	tSmeCmd *pSmeCommand;

	/*
	 * if the ActiveList is empty, then nothing is active so we can process
	 * a pending command...
	 * alwasy lock active list before locking pending list
	 */
	csr_ll_lock(&pMac->sme.smeCmdActiveList);
	if (!csr_ll_is_list_empty(&pMac->sme.smeCmdActiveList,
				 LL_ACCESS_NOLOCK)) {
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		goto process_scan_q;
	}

	if (csr_ll_is_list_empty(&pMac->sme.smeCmdPendingList,
				 LL_ACCESS_LOCK)) {
		/* No command waiting */
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		goto process_scan_q;
	}

	/*
	 * If scan command is pending in the smeScanCmdActive list then pick the
	 * command from smeCmdPendingList which is not matching with the scan
	 * command session id. At any point of time only one command will be
	 * allowed on a single session.
	 */
	if (!csr_ll_is_list_empty(
			&pMac->sme.smeScanCmdActiveList, LL_ACCESS_LOCK)) {
		pSmeEntry = csr_ll_peek_head(&pMac->sme.smeScanCmdActiveList,
					     LL_ACCESS_LOCK);
		if (pSmeEntry) {
			pSmeCommand = GET_BASE_ADDR(pSmeEntry, tSmeCmd, Link);
			pEntry = csr_get_cmd_to_process(pMac,
					&pMac->sme.smeCmdPendingList,
					pSmeCommand->sessionId,
					LL_ACCESS_LOCK);
			goto sme_process_cmd;
		}
	}

	/* Peek the command */
	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdPendingList, LL_ACCESS_LOCK);
sme_process_cmd:
	if (!pEntry) {
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		goto process_scan_q;
	}
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	/*
	 * Allow only disconnect command in wait-for-key state until setKey is
	 * through.
	 */
	if (CSR_IS_WAIT_FOR_KEY(pMac, pCommand->sessionId)
	    && !CSR_IS_DISCONNECT_COMMAND(pCommand)
	    && !CSR_IS_SET_KEY_COMMAND(pCommand)) {
		if (CSR_IS_CLOSE_SESSION_COMMAND(pCommand)) {
			tSmeCmd *sme_cmd = NULL;

			sms_log(pMac, LOGE,
				FL("SessionId %d: close session command issued while waiting for key, issue disconnect first"),
				pCommand->sessionId);
			status = csr_prepare_disconnect_command(pMac,
					pCommand->sessionId, &sme_cmd);
			if (status == QDF_STATUS_SUCCESS && sme_cmd) {
				csr_ll_lock(&pMac->sme.smeCmdPendingList);
				csr_ll_insert_head(&pMac->sme.smeCmdPendingList,
					&sme_cmd->Link, LL_ACCESS_NOLOCK);
				pEntry = csr_ll_peek_head(
						&pMac->sme.smeCmdPendingList,
						LL_ACCESS_NOLOCK);
				csr_ll_unlock(&pMac->sme.smeCmdPendingList);
				goto sme_process_cmd;
			}
		}

		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		sms_log(pMac, LOGE,
			FL("SessionId %d: Can't process cmd(%d), waiting for key"),
			pCommand->sessionId, pCommand->command);
		fContinue = false;
		goto process_scan_q;
	}

	if (!csr_ll_remove_entry(&pMac->sme.smeCmdPendingList, pEntry,
				LL_ACCESS_LOCK)) {
		/* This is odd. Some one else pull off the command. */
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		goto process_scan_q;
	}
	/* we can reuse the pCommand. Insert the command onto the ActiveList */
	csr_ll_insert_head(&pMac->sme.smeCmdActiveList, &pCommand->Link,
			   LL_ACCESS_NOLOCK);
	/* .... and process the command. */
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_COMMAND,
			 pCommand->sessionId, pCommand->command));

	switch (pCommand->command) {
	case eSmeCommandScan:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_process_scan_command(pMac, pCommand);
		break;
	case eSmeCommandRoam:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_roam_process_command(pMac, pCommand);
		if (!QDF_IS_STATUS_SUCCESS(status)
		    && csr_ll_remove_entry(&pMac->sme.smeCmdActiveList,
				&pCommand->Link, LL_ACCESS_LOCK))
			csr_release_command_roam(pMac, pCommand);
		break;
	case eSmeCommandWmStatusChange:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_roam_process_wm_status_change_command(pMac, pCommand);
		break;
	case eSmeCommandSetKey:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_roam_process_set_key_command(pMac, pCommand);
		if (!QDF_IS_STATUS_SUCCESS(status)
		    && csr_ll_remove_entry(&pMac->sme.smeCmdActiveList,
				&pCommand->Link, LL_ACCESS_LOCK)) {
			csr_release_command_set_key(pMac, pCommand);
		}
		break;
	case eSmeCommandAddStaSession:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_add_sta_session_command(pMac, pCommand);
		break;
	case eSmeCommandNdpInitiatorRequest:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		if (csr_process_ndp_initiator_request(pMac, pCommand) !=
			QDF_STATUS_SUCCESS)
			if (csr_ll_remove_entry(
				&pMac->sme.smeCmdActiveList,
				&pCommand->Link, LL_ACCESS_LOCK))
				csr_release_command(pMac, pCommand);
		break;
	case eSmeCommandNdpResponderRequest:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_process_ndp_responder_request(pMac, pCommand);
		if (status != QDF_STATUS_SUCCESS) {
			if (csr_ll_remove_entry(&pMac->sme.smeCmdActiveList,
					&pCommand->Link, LL_ACCESS_LOCK))
				csr_release_command(pMac, pCommand);
		}
		break;
	case eSmeCommandNdpDataEndInitiatorRequest:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_process_ndp_data_end_request(pMac, pCommand);
		if (status != QDF_STATUS_SUCCESS) {
			if (csr_ll_remove_entry(&pMac->sme.smeCmdActiveList,
					&pCommand->Link, LL_ACCESS_LOCK))
				csr_release_command(pMac, pCommand);
		}
		break;
	case eSmeCommandDelStaSession:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_del_sta_session_command(pMac, pCommand);
		break;
	case eSmeCommandRemainOnChannel:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		p2p_process_remain_on_channel_cmd(pMac, pCommand);
		break;
	/*
	 * Treat standby differently here because caller may not be able
	 * to handle the failure so we do our best here
	 */
	case eSmeCommandEnterStandby:
		break;
	case eSmeCommandAddTs:
	case eSmeCommandDelTs:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		fContinue = qos_process_command(pMac, pCommand);
		if (fContinue && csr_ll_remove_entry(
			&pMac->sme.smeCmdActiveList,
			&pCommand->Link, LL_ACCESS_NOLOCK)) {
			/* The command failed, remove it */
			qos_release_command(pMac, pCommand);
		}
#endif
		break;
#ifdef FEATURE_WLAN_TDLS
	case eSmeCommandTdlsSendMgmt:
	case eSmeCommandTdlsAddPeer:
	case eSmeCommandTdlsDelPeer:
	case eSmeCommandTdlsLinkEstablish:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("sending TDLS Command 0x%x to PE"),
			  pCommand->command);
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		status = csr_tdls_process_cmd(pMac, pCommand);
		break;
#endif
	case e_sme_command_set_hw_mode:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_set_hw_mode(pMac, pCommand);
		break;
	case e_sme_command_nss_update:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_nss_update_req(pMac, pCommand);
		break;
	case e_sme_command_set_dual_mac_config:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_set_dual_mac_config(pMac, pCommand);
		break;
	case e_sme_command_set_antenna_mode:
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		csr_process_set_antenna_mode(pMac, pCommand);
		break;
	default:
		/* something is wrong */
		/* remove it from the active list */
		sms_log(pMac, LOGE, FL("unknown command %d"),
			pCommand->command);
		pEntry = csr_ll_remove_head(&pMac->sme.smeCmdActiveList,
					    LL_ACCESS_NOLOCK);
		csr_ll_unlock(&pMac->sme.smeCmdActiveList);
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		sme_release_command(pMac, pCommand);
		status = QDF_STATUS_E_FAILURE;
		break;
	}
	if (!QDF_IS_STATUS_SUCCESS(status))
		fContinue = true;
process_scan_q:
	if (!(sme_process_scan_queue(pMac)))
		fContinue = false;
	return fContinue;
}

void sme_process_pending_queue(tpAniSirGlobal pMac)
{
	while (sme_process_command(pMac))
	;
}

bool sme_command_pending(tpAniSirGlobal pMac)
{
	return !csr_ll_is_list_empty(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK)
		|| !csr_ll_is_list_empty(&pMac->sme.smeCmdPendingList,
					 LL_ACCESS_NOLOCK);
}

/* Global APIs */

/**
 * sme_open() - Initialze all SME modules and put them at idle state
 * @hHal:       The handle returned by mac_open
 *
 * The function initializes each module inside SME, PMC, CSR, etc. Upon
 * successfully return, all modules are at idle state ready to start.
 * smeOpen must be called before any other SME APIs can be involved.
 * smeOpen must be called after mac_open.
 *
 * Return: QDF_STATUS_SUCCESS - SME is successfully initialized.
 *         Other status means SME is failed to be initialized
 */
QDF_STATUS sme_open(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->sme.state = SME_STATE_STOP;
	pMac->sme.currDeviceMode = QDF_STA_MODE;
	if (!QDF_IS_STATUS_SUCCESS(qdf_mutex_create(
					&pMac->sme.lkSmeGlobalLock))) {
		sms_log(pMac, LOGE, FL("sme_open failed init lock"));
		return  QDF_STATUS_E_FAILURE;
	}
	status = csr_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("csr_open failed, status=%d"), status);
		return status;
	}

	status = sme_ps_open(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
		FL("sme_ps_open failed during initialization with status=%d"),
			status);
		return status;
	}
#ifdef FEATURE_WLAN_TDLS
	pMac->is_tdls_power_save_prohibited = 0;
#endif

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("Qos open, status=%d"), status);
		return status;
	}
#endif
	status = init_sme_cmd_list(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	status = rrm_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("rrm_open failed, status=%d"), status);
		return status;
	}
	sme_p2p_open(pMac);
	sme_trace_init(pMac);
	return status;
}

/*
 * sme_init_chan_list, triggers channel setup based on country code.
 */
QDF_STATUS sme_init_chan_list(tHalHandle hal, uint8_t *alpha2,
			      enum country_src cc_src)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);

	if ((cc_src == SOURCE_USERSPACE) &&
	    (pmac->roam.configParam.fSupplicantCountryCodeHasPriority)) {
		pmac->roam.configParam.Is11dSupportEnabled = false;
	}

	return csr_init_chan_list(pmac, alpha2);
}

/*--------------------------------------------------------------------------

   \brief sme_set11dinfo() - Set the 11d information about valid channels
   and there power using information from nvRAM
   This function is called only for AP.

   This is a synchronous call

   \param hHal - The handle returned by mac_open.
   \Param pSmeConfigParams - a pointer to a caller allocated object of
   typedef struct _smeConfigParams.

   \return QDF_STATUS_SUCCESS - SME update the config parameters successfully.

   Other status means SME is failed to update the config parameters.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_set11dinfo(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SET_11DINFO, NO_SESSION, 0));
	if (NULL == pSmeConfigParams) {
		sms_log(pMac, LOGE,
			"Empty config param structure for SME, nothing to update");
		return status;
	}

	status = csr_set_channels(hHal, &pSmeConfigParams->csrConfig);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"csr_change_default_config_param failed with status=%d",
			status);
	}
	return status;
}

/**
 * sme_set_scan_disable() - Dynamically enable/disable scan
 * @h_hal: Handle to HAL
 *
 * This command gives the user an option to dynamically
 * enable or disable scans.
 *
 * Return: None
 */
void sme_set_scan_disable(tHalHandle h_hal, int value)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(h_hal);
	mac_ctx->lim.scan_disabled = value;
	sms_log(mac_ctx, LOG1, FL("value=%d"), value);
}
/*--------------------------------------------------------------------------

   \brief sme_get_soft_ap_domain() - Get the current regulatory domain of softAp.

   This is a synchronous call

   \param hHal - The handle returned by HostapdAdapter.
   \Param v_REGDOMAIN_t - The current Regulatory Domain requested for SoftAp.

   \return QDF_STATUS_SUCCESS - SME successfully completed the request.

   Other status means, failed to get the current regulatory domain.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_get_soft_ap_domain(tHalHandle hHal, v_REGDOMAIN_t *domainIdSoftAp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_GET_SOFTAP_DOMAIN,
			 NO_SESSION, 0));
	if (NULL == domainIdSoftAp) {
		sms_log(pMac, LOGE, "Uninitialized domain Id");
		return status;
	}

	*domainIdSoftAp = pMac->scan.domainIdCurrent;
	status = QDF_STATUS_SUCCESS;

	return status;
}

QDF_STATUS sme_set_reg_info(tHalHandle hHal, uint8_t *apCntryCode)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SET_REGINFO, NO_SESSION, 0));
	if (NULL == apCntryCode) {
		sms_log(pMac, LOGE, "Empty Country Code, nothing to update");
		return status;
	}

	status = csr_set_reg_info(hHal, apCntryCode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, "csr_set_reg_info failed with status=%d",
			status);
	}
	return status;
}

/**
 * sme_update_fine_time_measurement_capab() - Update the FTM capabitlies from
 * incoming val
 * @hal: Handle for Hal layer
 * @val: New FTM capability value
 *
 * Return: None
 */
void sme_update_fine_time_measurement_capab(tHalHandle hal, uint32_t val)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	mac_ctx->fine_time_meas_cap = val;

	if (val == 0) {
		mac_ctx->rrm.rrmPEContext.rrmEnabledCaps.fine_time_meas_rpt = 0;
		((tpRRMCaps)mac_ctx->rrm.rrmSmeContext.
			rrmConfig.rm_capability)->fine_time_meas_rpt = 0;
	} else {
		mac_ctx->rrm.rrmPEContext.rrmEnabledCaps.fine_time_meas_rpt = 1;
		((tpRRMCaps)mac_ctx->rrm.rrmSmeContext.
			rrmConfig.rm_capability)->fine_time_meas_rpt = 1;
	}
}

/*--------------------------------------------------------------------------

   \brief sme_update_config() - Change configurations for all SME moduels

   The function updates some configuration for modules in SME, CSR, etc
   during SMEs close open sequence.

   Modules inside SME apply the new configuration at the next transaction.

   This is a synchronous call

   \param hHal - The handle returned by mac_open.
   \Param pSmeConfigParams - a pointer to a caller allocated object of
   typedef struct _smeConfigParams.

   \return QDF_STATUS_SUCCESS - SME update the config parameters successfully.

   Other status means SME is failed to update the config parameters.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_update_config(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_UPDATE_CONFIG, NO_SESSION,
			 0));
	if (NULL == pSmeConfigParams) {
		sms_log(pMac, LOGE,
			"Empty config param structure for SME, nothing to update");
		return status;
	}

	status =
		csr_change_default_config_param(pMac, &pSmeConfigParams->csrConfig);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"csr_change_default_config_param failed with status=%d",
			status);
	}
	status =
		rrm_change_default_config_param(hHal, &pSmeConfigParams->rrmConfig);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"rrm_change_default_config_param failed with status=%d",
			status);
	}
	/* For SOC, CFG is set before start */
	/* We don't want to apply global CFG in connect state because that may cause some side affect */
	if (csr_is_all_session_disconnected(pMac)) {
		csr_set_global_cfgs(pMac);
	}

	/*
	 * If scan offload is enabled then lim has allow the sending of
	 * scan request to firmware even in powersave mode. The firmware has
	 * to take care of exiting from power save mode
	 */
	status = sme_cfg_set_int(hHal, WNI_CFG_SCAN_IN_POWERSAVE, true);

	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Could not pass on WNI_CFG_SCAN_IN_POWERSAVE to CFG");
	}
	return status;
}

/**
 * sme_update_roam_params() - Store/Update the roaming params
 * @hal:                      Handle for Hal layer
 * @session_id:               SME Session ID
 * @roam_params_src:          The source buffer to copy
 * @update_param:             Type of parameter to be updated
 *
 * Return: Return the status of the updation.
 */
QDF_STATUS sme_update_roam_params(tHalHandle hal,
	uint8_t session_id, struct roam_ext_params roam_params_src,
	int update_param)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct roam_ext_params *roam_params_dst;
	uint8_t i;

	roam_params_dst = &mac_ctx->roam.configParam.roam_params;
	switch (update_param) {
	case REASON_ROAM_EXT_SCAN_PARAMS_CHANGED:
		roam_params_dst->raise_rssi_thresh_5g =
			roam_params_src.raise_rssi_thresh_5g;
		roam_params_dst->drop_rssi_thresh_5g =
			roam_params_src.drop_rssi_thresh_5g;
		roam_params_dst->raise_factor_5g =
			roam_params_src.raise_factor_5g;
		roam_params_dst->drop_factor_5g =
			roam_params_src.drop_factor_5g;
		roam_params_dst->max_raise_rssi_5g =
			roam_params_src.max_raise_rssi_5g;
		roam_params_dst->max_drop_rssi_5g =
			roam_params_src.max_drop_rssi_5g;
		roam_params_dst->alert_rssi_threshold =
			roam_params_src.alert_rssi_threshold;
		roam_params_dst->is_5g_pref_enabled = true;
		break;
	case REASON_ROAM_SET_SSID_ALLOWED:
		qdf_mem_set(&roam_params_dst->ssid_allowed_list, 0,
				sizeof(tSirMacSSid) * MAX_SSID_ALLOWED_LIST);
		roam_params_dst->num_ssid_allowed_list =
			roam_params_src.num_ssid_allowed_list;
		for (i = 0; i < roam_params_dst->num_ssid_allowed_list; i++) {
			roam_params_dst->ssid_allowed_list[i].length =
				roam_params_src.ssid_allowed_list[i].length;
			qdf_mem_copy(roam_params_dst->ssid_allowed_list[i].ssId,
				roam_params_src.ssid_allowed_list[i].ssId,
				roam_params_dst->ssid_allowed_list[i].length);
		}
		break;
	case REASON_ROAM_SET_FAVORED_BSSID:
		qdf_mem_set(&roam_params_dst->bssid_favored, 0,
			sizeof(tSirMacAddr) * MAX_BSSID_FAVORED);
		roam_params_dst->num_bssid_favored =
			roam_params_src.num_bssid_favored;
		for (i = 0; i < roam_params_dst->num_bssid_favored; i++) {
			qdf_mem_copy(&roam_params_dst->bssid_favored[i],
				&roam_params_src.bssid_favored[i],
				sizeof(tSirMacAddr));
			roam_params_dst->bssid_favored_factor[i] =
				roam_params_src.bssid_favored_factor[i];
		}
		break;
	case REASON_ROAM_SET_BLACKLIST_BSSID:
		qdf_mem_set(&roam_params_dst->bssid_avoid_list, 0,
			QDF_MAC_ADDR_SIZE * MAX_BSSID_AVOID_LIST);
		roam_params_dst->num_bssid_avoid_list =
			roam_params_src.num_bssid_avoid_list;
		for (i = 0; i < roam_params_dst->num_bssid_avoid_list; i++) {
			qdf_copy_macaddr(&roam_params_dst->bssid_avoid_list[i],
					&roam_params_src.bssid_avoid_list[i]);
		}
		break;
	case REASON_ROAM_GOOD_RSSI_CHANGED:
		roam_params_dst->good_rssi_roam =
			roam_params_src.good_rssi_roam;
		break;
	default:
		break;
	}
	csr_roam_offload_scan(mac_ctx, session_id, ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			update_param);
	return 0;
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
void sme_process_get_gtk_info_rsp(tHalHandle hHal,
				  tpSirGtkOffloadGetInfoRspParams
				  pGtkOffloadGetInfoRsp)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return;
	}
	if (pMac->sme.gtk_offload_get_info_cb == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD callback is null", __func__);
		return;
	}
	pMac->sme.gtk_offload_get_info_cb(
			pMac->sme.gtk_offload_get_info_cb_context,
			pGtkOffloadGetInfoRsp);
}
#endif

/*--------------------------------------------------------------------------

   \fn    - sme_process_ready_to_suspend
   \brief - On getting ready to suspend indication, this function calls
	    callback registered (HDD callbacks) with SME to inform
	    ready to suspend indication.

   \param hHal - Handle returned by mac_open.
	  pReadyToSuspend - Parameter received along with ready to suspend
			    indication from WMA.

   \return None

   \sa

   --------------------------------------------------------------------------*/
void sme_process_ready_to_suspend(tHalHandle hHal,
				  tpSirReadyToSuspendInd pReadyToSuspend)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return;
	}

	if (NULL != pMac->readyToSuspendCallback) {
		pMac->readyToSuspendCallback(pMac->readyToSuspendContext,
					     pReadyToSuspend->suspended);
		pMac->readyToSuspendCallback = NULL;
	}
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT

/**
 * sme_process_ready_to_ext_wow() - inform ready to ExtWoW indication.
 * @hHal - Handle returned by mac_open.
 * @pReadyToExtWoW - Parameter received along with ready to Ext WoW
 *		     indication from WMA.
 *
 * On getting ready to Ext WoW indication, this function calls callback
 * registered (HDD callback)with SME to inform ready to ExtWoW indication.
 *
 * Return: None
 */
void sme_process_ready_to_ext_wow(tHalHandle hHal,
				   tpSirReadyToExtWoWInd pReadyToExtWoW)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return;
	}

	if (NULL != pMac->readyToExtWoWCallback) {
		pMac->readyToExtWoWCallback(pMac->readyToExtWoWContext,
					    pReadyToExtWoW->status);
		pMac->readyToExtWoWCallback = NULL;
		pMac->readyToExtWoWContext = NULL;
	}

}
#endif

/* ---------------------------------------------------------------------------
    \fn sme_change_config_params
    \brief The SME API exposed for HDD to provide config params to SME during
    SMEs stop -> start sequence.

    If HDD changed the domain that will cause a reset. This function will
    provide the new set of 11d information for the new domain. Currrently this
    API provides info regarding 11d only at reset but we can extend this for
    other params (PMC, QoS) which needs to be initialized again at reset.

    This is a synchronous call

    \param hHal - The handle returned by mac_open.

    \Param
    pUpdateConfigParam - a pointer to a structure (tCsrUpdateConfigParam) that
		currently provides 11d related information like Country code,
		Regulatory domain, valid channel list, Tx power per channel, a
		list with active/passive scan allowed per valid channel.

    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_change_config_params(tHalHandle hHal,
				    tCsrUpdateConfigParam *pUpdateConfigParam)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pUpdateConfigParam) {
		sms_log(pMac, LOGE,
			"Empty config param structure for SME, nothing to reset");
		return status;
	}

	status = csr_change_config_params(pMac, pUpdateConfigParam);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, "csrUpdateConfigParam failed with status=%d",
			status);
	}

	return status;

}

/*--------------------------------------------------------------------------

   \brief sme_hdd_ready_ind() - SME sends eWNI_SME_SYS_READY_IND to PE to inform
   that the NIC is ready tio run.

   The function is called by HDD at the end of initialization stage so PE/HAL can
   enable the NIC to running state.

   This is a synchronous call
   \param hHal - The handle returned by mac_open.

   \return QDF_STATUS_SUCCESS - eWNI_SME_SYS_READY_IND is sent to PE
				successfully.

   Other status means SME failed to send the message to PE.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_hdd_ready_ind(tHalHandle hHal)
{
	tSirSmeReadyReq Msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_HDDREADYIND, NO_SESSION, 0));
	do {

		Msg.messageType = eWNI_SME_SYS_READY_IND;
		Msg.length = sizeof(tSirSmeReadyReq);
		Msg.add_bssdescr_cb = csr_scan_process_single_bssdescr;
		Msg.csr_roam_synch_cb = csr_roam_synch_callback;


		if (eSIR_FAILURE != u_mac_post_ctrl_msg(hHal, (tSirMbMsg *) &Msg)) {
			status = QDF_STATUS_SUCCESS;
		} else {
			sms_log(pMac, LOGE,
				"u_mac_post_ctrl_msg failed to send eWNI_SME_SYS_READY_IND");
			break;
		}

		status = csr_ready(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				"csr_ready failed with status=%d",
				status);
			break;
		}

		if (QDF_STATUS_SUCCESS != rrm_ready(hHal)) {
			status = QDF_STATUS_E_FAILURE;
			sms_log(pMac, LOGE, "rrm_ready failed");
			break;
		}
		pMac->sme.state = SME_STATE_READY;
	} while (0);

	return status;
}

/*--------------------------------------------------------------------------

   \brief sme_start() - Put all SME modules at ready state.

   The function starts each module in SME, PMC, CSR, etc. . Upon
   successfully return, all modules are ready to run.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.

   \return QDF_STATUS_SUCCESS - SME is ready.

   Other status means SME is failed to start
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_start(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		status = csr_start(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(pMac, LOGE,
				"csr_start failed during smeStart with status=%d",
				status);
			break;
		}
		pMac->sme.state = SME_STATE_START;
	} while (0);

	return status;
}

/**
 * sme_handle_scan_req() - Scan request handler
 * @mac_ctx: MAC global context
 * @msg: message buffer
 *
 * Scan request message from upper layer is handled as
 * part of this API
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_handle_scan_req(tpAniSirGlobal mac_ctx,
					void *msg)
{
	struct ani_scan_req *scan_msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint16_t session_id;
	csr_scan_completeCallback callback;

	scan_msg = msg;
	session_id = scan_msg->session_id;
	callback = scan_msg->callback;
	status = csr_scan_request(mac_ctx, session_id,
		scan_msg->scan_param,
		callback, scan_msg->ctx);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE,
			FL("scan request failed. session_id %d"), session_id);
	}
	csr_scan_free_request(mac_ctx, scan_msg->scan_param);
	qdf_mem_free(scan_msg->scan_param);
	return status;
}

/**
 * sme_handle_roc_req() - Roc request handler
 * @mac_ctx: MAC global context
 * @msg: message buffer
 *
 * Roc request message from upper layer is handled as
 * part of this API
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_handle_roc_req(tHalHandle hal,
					void *msg)
{
	struct ani_roc_req *roc_msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	remainOnChanCallback callback;

	if (msg == NULL) {
		sms_log(mac_ctx, LOGE, FL("ROC request is NULL"));
		return status;
	}

	roc_msg = msg;
	callback = roc_msg->callback;
	status = p2p_remain_on_channel(hal, roc_msg->session_id,
		roc_msg->channel, roc_msg->duration, callback,
		roc_msg->ctx, roc_msg->is_p2pprobe_allowed,
		roc_msg->scan_id);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE,
			FL("scan request failed. session_id %d scan_id %d"),
			roc_msg->session_id, roc_msg->scan_id);
	}
	return status;
}

#ifdef WLAN_FEATURE_11W
/*------------------------------------------------------------------
 *
 * Handle the unprotected management frame indication from LIM and
 * forward it to HDD.
 *
 *------------------------------------------------------------------*/

QDF_STATUS sme_unprotected_mgmt_frm_ind(tHalHandle hHal,
					tpSirSmeUnprotMgmtFrameInd pSmeMgmtFrm)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo pRoamInfo = { 0 };
	uint32_t SessionId = pSmeMgmtFrm->sessionId;

	pRoamInfo.nFrameLength = pSmeMgmtFrm->frameLen;
	pRoamInfo.pbFrames = pSmeMgmtFrm->frameBuf;
	pRoamInfo.frameType = pSmeMgmtFrm->frameType;

	/* forward the mgmt frame to HDD */
	csr_roam_call_callback(pMac, SessionId, &pRoamInfo, 0,
			       eCSR_ROAM_UNPROT_MGMT_FRAME_IND, 0);

	return status;
}
#endif

/*------------------------------------------------------------------
 *
 * Handle the DFS Radar Event and indicate it to the SAP
 *
 *------------------------------------------------------------------*/
QDF_STATUS dfs_msg_processor(tpAniSirGlobal pMac, uint16_t msgType, void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo roamInfo = { 0 };
	tSirSmeDfsEventInd *dfs_event;
	tSirSmeCSAIeTxCompleteRsp *csaIeTxCompleteRsp;
	uint32_t sessionId = 0;
	eRoamCmdStatus roamStatus;
	eCsrRoamResult roamResult;
	int i;

	switch (msgType) {
	case eWNI_SME_DFS_RADAR_FOUND:
	{
		/* Radar found !! */
		dfs_event = (tSirSmeDfsEventInd *) pMsgBuf;
		if (NULL == dfs_event) {
			sms_log(pMac, LOGE,
				"%s: pMsg is NULL for eWNI_SME_DFS_RADAR_FOUND message",
				__func__);
			return QDF_STATUS_E_FAILURE;
		}
		sessionId = dfs_event->sessionId;
		roamInfo.dfs_event.sessionId = sessionId;
		roamInfo.dfs_event.chan_list.nchannels =
			dfs_event->chan_list.nchannels;
		for (i = 0; i < dfs_event->chan_list.nchannels; i++) {
			roamInfo.dfs_event.chan_list.channels[i] =
				dfs_event->chan_list.channels[i];
		}

		roamInfo.dfs_event.dfs_radar_status =
			dfs_event->dfs_radar_status;
		roamInfo.dfs_event.use_nol = dfs_event->use_nol;

		roamStatus = eCSR_ROAM_DFS_RADAR_IND;
		roamResult = eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
			  "sapdfs: Radar indication event occurred");
		break;
	}
	case eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND:
	{
		csaIeTxCompleteRsp =
			(tSirSmeCSAIeTxCompleteRsp *) pMsgBuf;
		if (NULL == csaIeTxCompleteRsp) {
			sms_log(pMac, LOGE,
				"%s: pMsg is NULL for eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND",
				__func__);
			return QDF_STATUS_E_FAILURE;
		}
		sessionId = csaIeTxCompleteRsp->sessionId;
		roamStatus = eCSR_ROAM_DFS_CHAN_SW_NOTIFY;
		roamResult = eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
			  "sapdfs: Received eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND for session id [%d]",
			  sessionId);
		break;
	}
	default:
	{
		sms_log(pMac, LOG1, "%s: Invalid DFS message = 0x%x",
			__func__, msgType);
		status = QDF_STATUS_E_FAILURE;
		return status;
	}
	}

	/* Indicate Radar Event to SAP */
	csr_roam_call_callback(pMac, sessionId, &roamInfo, 0,
			       roamStatus, roamResult);
	return status;
}

/**
 * sme_extended_change_channel_ind()- function to indicate ECSA
 * action frame is received in lim to SAP
 * @mac_ctx:  pointer to global mac structure
 * @msg_buf: contain new channel and session id.
 *
 * This function is called to post ECSA action frame
 * receive event to SAP.
 *
 * Return: success if msg indicated to SAP else return failure
 */
static QDF_STATUS sme_extended_change_channel_ind(tpAniSirGlobal mac_ctx,
						void *msg_buf)
{
	struct sir_sme_ext_cng_chan_ind *ext_chan_ind;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t session_id = 0;
	tCsrRoamInfo roamInfo = {0};
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;


	ext_chan_ind = msg_buf;
	if (NULL == ext_chan_ind) {
		sms_log(mac_ctx, LOGE,
			FL("pMsg is NULL for eWNI_SME_EXT_CHANGE_CHANNEL_IND"));
		return QDF_STATUS_E_FAILURE;
	}
	session_id = ext_chan_ind->session_id;
	roamInfo.target_channel = ext_chan_ind->new_channel;
	roam_status = eCSR_ROAM_EXT_CHG_CHNL_IND;
	roam_result = eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND;
	sms_log(mac_ctx, LOG1,
		FL("sapdfs: Received eWNI_SME_EXT_CHANGE_CHANNEL_IND for session id [%d]"),
		session_id);

	/* Indicate Ext Channel Change event to SAP */
	csr_roam_call_callback(mac_ctx, session_id, &roamInfo, 0,
					roam_status, roam_result);
	return status;
}

/**
 * sme_process_fw_mem_dump_rsp - process fw memory dump response from WMA
 *
 * @mac_ctx: pointer to MAC handle.
 * @msg: pointer to received SME msg.
 *
 * This function process the received SME message and calls the corresponding
 * callback which was already registered with SME.
 *
 * Return: None
 */
#ifdef WLAN_FEATURE_MEMDUMP
static void sme_process_fw_mem_dump_rsp(tpAniSirGlobal mac_ctx, cds_msg_t *msg)
{
	if (msg->bodyptr) {
		if (mac_ctx->sme.fw_dump_callback)
			mac_ctx->sme.fw_dump_callback(mac_ctx->hHdd,
				(struct fw_dump_rsp *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
	}
}
#else
static void sme_process_fw_mem_dump_rsp(tpAniSirGlobal mac_ctx, cds_msg_t *msg)
{
}
#endif

#ifdef FEATURE_WLAN_ESE
/**
 * sme_update_is_ese_feature_enabled() - enable/disable ESE support at runtime
 * @hHal: HAL handle
 * @sessionId: session id
 * @isEseIniFeatureEnabled: ese ini enabled
 *
 * It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 * isEseIniFeatureEnabled. This is a synchronous call
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_update_is_ese_feature_enabled(tHalHandle hHal,
			uint8_t sessionId, const bool isEseIniFeatureEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (pMac->roam.configParam.isEseIniFeatureEnabled ==
	    isEseIniFeatureEnabled) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: ESE Mode is already enabled or disabled, nothing to do (returning) old(%d) new(%d)",
			  __func__,
			  pMac->roam.configParam.isEseIniFeatureEnabled,
			  isEseIniFeatureEnabled);
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s: EseEnabled is changed from %d to %d", __func__,
		  pMac->roam.configParam.isEseIniFeatureEnabled,
		  isEseIniFeatureEnabled);
	pMac->roam.configParam.isEseIniFeatureEnabled = isEseIniFeatureEnabled;
	csr_neighbor_roam_update_fast_roaming_enabled(
			pMac, sessionId, isEseIniFeatureEnabled);

	if (true == isEseIniFeatureEnabled)
		sme_update_fast_transition_enabled(hHal, true);

	if (pMac->roam.configParam.isRoamOffloadScanEnabled)
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_ESE_INI_CFG_CHANGED);

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_set_plm_request() - set plm request
 * @hHal: HAL handle
 * @pPlmReq: Pointer to input plm request
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_plm_request(tHalHandle hHal, tpSirPlmReq pPlmReq)
{
	QDF_STATUS status;
	bool ret = false;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t ch_list[WNI_CFG_VALID_CHANNEL_LIST] = { 0 };
	uint8_t count, valid_count = 0;
	cds_msg_t msg;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, pPlmReq->sessionId);

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("session %d not found"),
			pPlmReq->sessionId);
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pSession->sessionActive) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid Sessionid"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pPlmReq->enable)
		goto send_plm_start;
	/* validating channel numbers */
	for (count = 0; count < pPlmReq->plmNumCh; count++) {
		ret = csr_is_supported_channel(pMac, pPlmReq->plmChList[count]);
		if (ret && pPlmReq->plmChList[count] > 14) {
			if (CHANNEL_STATE_DFS == cds_get_channel_state(
						pPlmReq->plmChList[count])) {
				/* DFS channel is provided, no PLM bursts can be
				 * transmitted. Ignoring these channels.
				 */
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_INFO,
					  FL("DFS channel %d ignored for PLM"),
					  pPlmReq->plmChList[count]);
				continue;
			}
		} else if (!ret) {
			/* Not supported, ignore the channel */
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  FL("Unsupported channel %d ignored for PLM"),
				  pPlmReq->plmChList[count]);
			continue;
		}
		ch_list[valid_count] = pPlmReq->plmChList[count];
		valid_count++;
	} /* End of for () */

	/* Copying back the valid channel list to plm struct */
	qdf_mem_set((void *)pPlmReq->plmChList,
		    pPlmReq->plmNumCh, 0);
	if (valid_count)
		qdf_mem_copy(pPlmReq->plmChList, ch_list,
			     valid_count);
	/* All are invalid channels, FW need to send the PLM
	 *  report with "incapable" bit set.
	 */
	pPlmReq->plmNumCh = valid_count;

send_plm_start:
	/* PLM START */
	msg.type = WMA_SET_PLM_REQ;
	msg.reserved = 0;
	msg.bodyptr = pPlmReq;

	if (!QDF_IS_STATUS_SUCCESS(cds_mq_post_message(QDF_MODULE_ID_WMA,
						       &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post WMA_SET_PLM_REQ to WMA"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	sme_release_global_lock(&pMac->sme);
	return status;
}

/**
 * sme_tsm_ie_ind() - sme tsm ie indication
 * @hHal: HAL handle
 * @pSmeTsmIeInd: Pointer to tsm ie indication
 *
 * Handle the tsm ie indication from  LIM and forward it to HDD.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS sme_tsm_ie_ind(tHalHandle hHal, tSirSmeTsmIEInd *pSmeTsmIeInd)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo pRoamInfo = { 0 };
	uint32_t SessionId = pSmeTsmIeInd->sessionId;
	pRoamInfo.tsmIe.tsid = pSmeTsmIeInd->tsmIe.tsid;
	pRoamInfo.tsmIe.state = pSmeTsmIeInd->tsmIe.state;
	pRoamInfo.tsmIe.msmt_interval = pSmeTsmIeInd->tsmIe.msmt_interval;
	/* forward the tsm ie information to HDD */
	csr_roam_call_callback(pMac,
			       SessionId, &pRoamInfo, 0, eCSR_ROAM_TSM_IE_IND, 0);
	return status;
}

/**
 * sme_set_cckm_ie() - set cckm ie
 * @hHal: HAL handle
 * @sessionId: session id
 * @pCckmIe: Pointer to CCKM Ie
 * @cckmIeLen: Length of @pCckmIe
 *
 * Function to store the CCKM IE passed from supplicant and use
 * it while packing reassociation request.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_cckm_ie(tHalHandle hHal, uint8_t sessionId,
			   uint8_t *pCckmIe, uint8_t cckmIeLen)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_set_cckm_ie(pMac, sessionId, pCckmIe, cckmIeLen);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_set_ese_beacon_request() - set ese beacon request
 * @hHal: HAL handle
 * @sessionId: session id
 * @pEseBcnReq: Ese beacon report
 *
 * function to set ESE beacon request parameters
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_ese_beacon_request(tHalHandle hHal, const uint8_t sessionId,
				      const tCsrEseBeaconReq *pEseBcnReq)
{
	QDF_STATUS status = eSIR_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tpSirBeaconReportReqInd pSmeBcnReportReq = NULL;
	tCsrEseBeaconReqParams *pBeaconReq = NULL;
	uint8_t counter = 0;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;

	if (pSmeRrmContext->eseBcnReqInProgress == true) {
		sms_log(pMac, LOGE,
			"A Beacon Report Req is already in progress");
		return QDF_STATUS_E_RESOURCES;
	}

	/* Store the info in RRM context */
	qdf_mem_copy(&pSmeRrmContext->eseBcnReqInfo, pEseBcnReq,
		     sizeof(tCsrEseBeaconReq));

	/* Prepare the request to send to SME. */
	pSmeBcnReportReq = qdf_mem_malloc(sizeof(tSirBeaconReportReqInd));
	if (NULL == pSmeBcnReportReq) {
		sms_log(pMac, LOGP,
			"Memory Allocation Failure!!! ESE  BcnReq Ind to SME");
		return QDF_STATUS_E_NOMEM;
	}

	pSmeRrmContext->eseBcnReqInProgress = true;

	sms_log(pMac, LOGE, "Sending Beacon Report Req to SME");
	qdf_mem_zero(pSmeBcnReportReq, sizeof(tSirBeaconReportReqInd));

	pSmeBcnReportReq->messageType = eWNI_SME_BEACON_REPORT_REQ_IND;
	pSmeBcnReportReq->length = sizeof(tSirBeaconReportReqInd);
	qdf_mem_copy(pSmeBcnReportReq->bssId,
		     pSession->connectedProfile.bssid.bytes,
		     sizeof(tSirMacAddr));
	pSmeBcnReportReq->channelInfo.channelNum = 255;
	pSmeBcnReportReq->channelList.numChannels = pEseBcnReq->numBcnReqIe;
	pSmeBcnReportReq->msgSource = eRRM_MSG_SOURCE_ESE_UPLOAD;

	for (counter = 0; counter < pEseBcnReq->numBcnReqIe; counter++) {
		pBeaconReq =
			(tCsrEseBeaconReqParams *) &pEseBcnReq->bcnReq[counter];
		pSmeBcnReportReq->fMeasurementtype[counter] =
			pBeaconReq->scanMode;
		pSmeBcnReportReq->measurementDuration[counter] =
			SYS_TU_TO_MS(pBeaconReq->measurementDuration);
		pSmeBcnReportReq->channelList.channelNumber[counter] =
			pBeaconReq->channel;
	}

	status = sme_rrm_process_beacon_report_req_ind(pMac, pSmeBcnReportReq);

	if (status != QDF_STATUS_SUCCESS)
		pSmeRrmContext->eseBcnReqInProgress = false;

	return status;
}

/**
 * sme_get_tsm_stats() - SME get tsm stats
 * @hHal: HAL handle
 * @callback: SME sends back the requested stats using the callback
 * @staId: The station ID for which the stats is requested for
 * @bssId: bssid
 * @pContext: user context to be passed back along with the callback
 * @p_cds_context: CDS context
 * @tid: Traffic id
 *
 * API register a callback to get TSM Stats.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_tsm_stats(tHalHandle hHal,
			     tCsrTsmStatsCallback callback,
			     uint8_t staId, struct qdf_mac_addr bssId,
			     void *pContext, void *p_cds_context, uint8_t tid)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_tsm_stats(pMac, callback,
					   staId, bssId, pContext,
					   p_cds_context, tid);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_set_ese_roam_scan_channel_list() - To set ese roam scan channel list
 * @hHal: pointer HAL handle returned by mac_open
 * @sessionId: sme session id
 * @pChannelList: Output channel list
 * @numChannels: Output number of channels
 *
 * This routine is called to set ese roam scan channel list.
 * This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_ese_roam_scan_channel_list(tHalHandle hHal,
					      uint8_t sessionId,
					      uint8_t *pChannelList,
					      uint8_t numChannels)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo
		= &pMac->roam.neighborRoamInfo[sessionId];
	tpCsrChannelInfo curchnl_list_info
		= &pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo;
	uint8_t oldChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[128] = { 0 };
	uint8_t i = 0, j = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		if (pMac->roam.configParam.isRoamOffloadScanEnabled)
			csr_roam_offload_scan(pMac, sessionId,
					ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					REASON_CHANNEL_LIST_CHANGED);
		return status;
	}
	if (NULL != curchnl_list_info->ChannelList) {
		for (i = 0; i < curchnl_list_info->numOfChannels; i++) {
			j += snprintf(oldChannelList + j,
				sizeof(oldChannelList) - j, "%d",
				curchnl_list_info->ChannelList[i]);
		}
	}
	status = csr_create_roam_scan_channel_list(pMac, sessionId,
				pChannelList, numChannels,
				csr_get_current_band(hHal));
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (NULL != curchnl_list_info->ChannelList) {
			j = 0;
			for (i = 0; i < curchnl_list_info->numOfChannels; i++) {
				j += snprintf(newChannelList + j,
					sizeof(newChannelList) - j, "%d",
					curchnl_list_info->ChannelList[i]);
			}
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"ESE roam scan chnl list successfully set to %s-old value is %s-roam state is %d",
			newChannelList, oldChannelList,
			pNeighborRoamInfo->neighborRoamState);
	}
	sme_release_global_lock(&pMac->sme);
	if (pMac->roam.configParam.isRoamOffloadScanEnabled)
		csr_roam_offload_scan(pMac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_CHANNEL_LIST_CHANGED);
	return status;
}

#endif /* FEATURE_WLAN_ESE */

QDF_STATUS sme_ibss_peer_info_response_handleer(tHalHandle hHal,
						tpSirIbssGetPeerInfoRspParams
						pIbssPeerInfoParams)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (pMac->sme.peerInfoParams.peerInfoCbk == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD callback is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	pMac->sme.peerInfoParams.peerInfoCbk(pMac->sme.peerInfoParams.pUserData,
					     &pIbssPeerInfoParams->
					     ibssPeerInfoRspParams);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_process_dual_mac_config_resp() - Process set Dual mac config response
 * @mac: Global MAC pointer
 * @msg: Dual mac config response
 *
 * Processes the dual mac configuration response and invokes the HDD callback
 * to process further
 */
static QDF_STATUS sme_process_dual_mac_config_resp(tpAniSirGlobal mac,
		uint8_t *msg)
{
	tListElem *entry = NULL;
	tSmeCmd *command = NULL;
	bool found;
	dual_mac_cb callback = NULL;
	struct sir_dual_mac_config_resp *param;

	param = (struct sir_dual_mac_config_resp *)msg;
	if (!param) {
		sms_log(mac, LOGE, FL("Dual mac config resp param is NULL"));
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_ll_peek_head(&mac->sme.smeCmdActiveList,
			LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac, LOGE, FL("No cmd found in active list"));
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sms_log(mac, LOGE, FL("Base address is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_dual_mac_config != command->command) {
		sms_log(mac, LOGE, FL("Command mismatch!"));
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.set_dual_mac_cmd.set_dual_mac_cb;
	if (callback) {
		if (!param) {
			sms_log(mac, LOGE,
				FL("Callback failed-Dual mac config is NULL"));
		} else {
			sms_log(mac, LOG1,
				FL("Calling HDD callback for Dual mac config"));
			callback(param->status,
				command->u.set_dual_mac_cmd.scan_config,
				command->u.set_dual_mac_cmd.fw_mode_config);
		}
	} else {
		sms_log(mac, LOGE, FL("Callback does not exist"));
	}

	found = csr_ll_remove_entry(&mac->sme.smeCmdActiveList, entry,
			LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the available command list */
		sme_release_command(mac, command);

	sme_process_pending_queue(mac);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_process_antenna_mode_resp() - Process set antenna mode
 * response
 * @mac: Global MAC pointer
 * @msg: antenna mode response
 *
 * Processes the antenna mode response and invokes the HDD
 * callback to process further
 */
static QDF_STATUS sme_process_antenna_mode_resp(tpAniSirGlobal mac,
		uint8_t *msg)
{
	tListElem *entry;
	tSmeCmd *command;
	bool found;
	antenna_mode_cb callback;
	struct sir_antenna_mode_resp *param;

	param = (struct sir_antenna_mode_resp *)msg;
	if (!param) {
		sms_log(mac, LOGE, FL("set antenna mode resp is NULL"));
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_ll_peek_head(&mac->sme.smeCmdActiveList,
			LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac, LOGE, FL("No cmd found in active list"));
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sms_log(mac, LOGE, FL("Base address is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_antenna_mode != command->command) {
		sms_log(mac, LOGE, FL("Command mismatch!"));
		return QDF_STATUS_E_FAILURE;
	}

	callback =
		command->u.set_antenna_mode_cmd.set_antenna_mode_resp;
	if (callback) {
		if (!param) {
			sms_log(mac, LOGE,
				FL("Set antenna mode call back is NULL"));
		} else {
			sms_log(mac, LOG1,
				FL("HDD callback for set antenna mode"));
			callback(param->status);
		}
	} else {
		sms_log(mac, LOGE, FL("Callback does not exist"));
	}

	found = csr_ll_remove_entry(&mac->sme.smeCmdActiveList, entry,
			LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the available command list */
		sme_release_command(mac, command);

	sme_process_pending_queue(mac);
	return QDF_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------

   \brief sme_process_msg() - The main message processor for SME.

   The function is called by a message dispatcher when to process a message
   targeted for SME.

   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \param pMsg - A pointer to a caller allocated object of tSirMsgQ.

   \return QDF_STATUS_SUCCESS - SME successfully process the message.

   Other status means SME failed to process the message to HAL.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_process_msg(tHalHandle hHal, cds_msg_t *pMsg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (pMsg == NULL) {
		sms_log(pMac, LOGE, "Empty message for SME");
		return status;
	}
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGW, FL("Locking failed, bailing out"));
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		return status;
	}
	if (!SME_IS_START(pMac)) {
		sms_log(pMac, LOGW, FL("message type %d in stop state ignored"),
			pMsg->type);
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		goto release_lock;
	}
	switch (pMsg->type) {
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case eWNI_SME_HO_FAIL_IND:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("LFR3: Rcvd eWNI_SME_HO_FAIL_IND"));
		csr_process_ho_fail_ind(pMac, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_PMC_SMPS_STATE_IND:
		break;
	case WNI_CFG_SET_CNF:
	case WNI_CFG_DNLD_CNF:
	case WNI_CFG_GET_RSP:
	case WNI_CFG_ADD_GRP_ADDR_CNF:
	case WNI_CFG_DEL_GRP_ADDR_CNF:
		break;
	case eWNI_SME_ADDTS_RSP:
	case eWNI_SME_DELTS_RSP:
	case eWNI_SME_DELTS_IND:
	case eWNI_SME_FT_AGGR_QOS_RSP:
		/* QoS */
		if (pMsg->bodyptr) {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
			status = sme_qos_msg_processor(pMac, pMsg->type,
						       pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
#endif
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_NEIGHBOR_REPORT_IND:
	case eWNI_SME_BEACON_REPORT_REQ_IND:
		if (pMsg->bodyptr) {
			status = sme_rrm_msg_processor(pMac, pMsg->type,
						       pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_ADD_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_add_sta_session_rsp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_DEL_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_del_sta_session_rsp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_REMAIN_ON_CHN_RSP:
		if (pMsg->bodyptr) {
			status = sme_remain_on_chn_rsp(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_REMAIN_ON_CHN_RDY_IND:
		if (pMsg->bodyptr) {
			status = sme_remain_on_chn_ready(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_ACTION_FRAME_SEND_CNF:
		if (pMsg->bodyptr) {
			status = sme_send_action_cnf(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#ifdef FEATURE_WLAN_SCAN_PNO
	case eWNI_SME_PREF_NETWORK_FOUND_IND:
		if (pMsg->bodyptr) {
			status = sme_preferred_network_found_ind((void *)pMac,
								 pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif /* FEATURE_WLAN_SCAN_PNO */
	case eWNI_SME_CHANGE_COUNTRY_CODE:
		if (pMsg->bodyptr) {
			status = sme_handle_change_country_code((void *)pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE:
		if (pMsg->bodyptr) {
			status = sme_handle_generic_change_country_code(
						(void *)pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_SCAN_CMD:
		if (pMsg->bodyptr) {
			status = sme_handle_scan_req(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_ROC_CMD:
		if (pMsg->bodyptr) {
			status = sme_handle_roc_req(hHal, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#ifdef FEATURE_WLAN_TDLS
	/*
	 * command rescived from PE, SME tdls msg processor shall be called
	 * to process commands recieved from PE
	 */
	case eWNI_SME_TDLS_SEND_MGMT_RSP:
	case eWNI_SME_TDLS_ADD_STA_RSP:
	case eWNI_SME_TDLS_DEL_STA_RSP:
	case eWNI_SME_TDLS_DEL_STA_IND:
	case eWNI_SME_TDLS_DEL_ALL_PEER_IND:
	case eWNI_SME_MGMT_FRM_TX_COMPLETION_IND:
	case eWNI_SME_TDLS_LINK_ESTABLISH_RSP:
	case eWNI_SME_TDLS_SHOULD_DISCOVER:
	case eWNI_SME_TDLS_SHOULD_TEARDOWN:
	case eWNI_SME_TDLS_PEER_DISCONNECTED:
		if (pMsg->bodyptr) {
			status = tdls_msg_processor(pMac, pMsg->type,
						    pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif
#ifdef WLAN_FEATURE_11W
	case eWNI_SME_UNPROT_MGMT_FRM_IND:
		if (pMsg->bodyptr) {
			sme_unprotected_mgmt_frm_ind(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_ESE
	case eWNI_SME_TSM_IE_IND:
		if (pMsg->bodyptr) {
			sme_tsm_ie_ind(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif /* FEATURE_WLAN_ESE */
	case eWNI_SME_ROAM_SCAN_OFFLOAD_RSP:
		status = csr_roam_offload_scan_rsp_hdlr((void *)pMac,
							pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	case eWNI_PMC_GTK_OFFLOAD_GETINFO_RSP:
		if (pMsg->bodyptr) {
			sme_process_get_gtk_info_rsp(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_LPHB
	/* LPHB timeout indication arrived, send IND to client */
	case eWNI_SME_LPHB_IND:
		if (pMac->sme.pLphbIndCb)
			pMac->sme.pLphbIndCb(pMac->hHdd, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif /* FEATURE_WLAN_LPHB */
	case eWNI_SME_IBSS_PEER_INFO_RSP:
		if (pMsg->bodyptr) {
			sme_ibss_peer_info_response_handleer(pMac,
							     pMsg->
							     bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_READY_TO_SUSPEND_IND:
		if (pMsg->bodyptr) {
			sme_process_ready_to_suspend(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	case eWNI_SME_READY_TO_EXTWOW_IND:
		if (pMsg->bodyptr) {
			sme_process_ready_to_ext_wow(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_CH_AVOID
	/* channel avoid message arrived, send IND to client */
	case eWNI_SME_CH_AVOID_IND:
		if (pMac->sme.pChAvoidNotificationCb) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  FL("CH avoid notification"));
			pMac->sme.pChAvoidNotificationCb(pMac->hHdd,
							 pMsg->bodyptr);
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif /* FEATURE_WLAN_CH_AVOID */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case eWNI_SME_AUTO_SHUTDOWN_IND:
		if (pMac->sme.pAutoShutdownNotificationCb) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  FL("Auto shutdown notification"));
			pMac->sme.pAutoShutdownNotificationCb();
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_SME_DFS_RADAR_FOUND:
	case eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND:
		status = dfs_msg_processor(pMac, pMsg->type, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_CHANNEL_CHANGE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_channel_change_resp(pMac,
								 pMsg->type,
								 pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#ifdef WLAN_FEATURE_STATS_EXT
	case eWNI_SME_STATS_EXT_EVENT:
		if (pMsg->bodyptr) {
			status = sme_stats_ext_event(hHal, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
#endif
	case eWNI_SME_LINK_SPEED_IND:
		if (pMac->sme.pLinkSpeedIndCb)
			pMac->sme.pLinkSpeedIndCb(pMsg->bodyptr,
						pMac->sme.pLinkSpeedCbContext);
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_CSA_OFFLOAD_EVENT:
		if (pMsg->bodyptr) {
			csr_scan_flush_bss_entry(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		}
		break;
	case eWNI_SME_TSF_EVENT:
		if (pMac->sme.get_tsf_cb) {
			pMac->sme.get_tsf_cb(pMac->sme.get_tsf_cxt,
					(struct stsf *)pMsg->bodyptr);
		}
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		break;
#ifdef WLAN_FEATURE_NAN
	case eWNI_SME_NAN_EVENT:
		if (pMsg->bodyptr) {
			sme_nan_event(hHal, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		}
		break;
#endif /* WLAN_FEATURE_NAN */
	case eWNI_SME_LINK_STATUS_IND:
	{
		tAniGetLinkStatus *pLinkStatus =
			(tAniGetLinkStatus *) pMsg->bodyptr;
		if (pLinkStatus) {
			if (pMac->sme.linkStatusCallback) {
				pMac->sme.linkStatusCallback(
					pLinkStatus->linkStatus,
					pMac->sme.linkStatusContext);
			}
			pMac->sme.linkStatusCallback = NULL;
			pMac->sme.linkStatusContext = NULL;
			qdf_mem_free(pLinkStatus);
		}
		break;
	}
	case eWNI_SME_MSG_GET_TEMPERATURE_IND:
		if (pMac->sme.pGetTemperatureCb) {
			pMac->sme.pGetTemperatureCb(pMsg->bodyval,
					pMac->sme.pTemperatureCbContext);
		}
		break;
	case eWNI_SME_SNR_IND:
	{
		tAniGetSnrReq *pSnrReq = (tAniGetSnrReq *) pMsg->bodyptr;
		if (pSnrReq) {
			if (pSnrReq->snrCallback) {
				((tCsrSnrCallback)
				 (pSnrReq->snrCallback))
					(pSnrReq->snr, pSnrReq->staId,
					pSnrReq->pDevContext);
			}
			qdf_mem_free(pSnrReq);
		}
		break;
	}
#ifdef FEATURE_WLAN_EXTSCAN
	case eWNI_SME_EXTSCAN_FULL_SCAN_RESULT_IND:
		if (pMac->sme.pExtScanIndCb)
			pMac->sme.pExtScanIndCb(pMac->hHdd,
					eSIR_EXTSCAN_FULL_SCAN_RESULT_IND,
					pMsg->bodyptr);
		else
			sms_log(pMac, LOGE,
				FL("callback not registered to process %d"),
				pMsg->type);

		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_EPNO_NETWORK_FOUND_IND:
		if (pMac->sme.pExtScanIndCb)
			pMac->sme.pExtScanIndCb(pMac->hHdd,
					eSIR_EPNO_NETWORK_FOUND_IND,
					pMsg->bodyptr);
		else
			sms_log(pMac, LOGE,
				FL("callback not registered to process %d"),
				pMsg->type);

		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_SME_FW_DUMP_IND:
		sme_process_fw_mem_dump_rsp(pMac, pMsg);
		break;
	case eWNI_SME_SET_HW_MODE_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_set_hw_mode_resp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_HW_MODE_TRANS_IND:
		if (pMsg->bodyptr) {
			status = sme_process_hw_mode_trans_ind(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_NSS_UPDATE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_nss_update_resp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
		}
		break;
	case eWNI_SME_OCB_SET_CONFIG_RSP:
		if (pMac->sme.ocb_set_config_callback) {
			pMac->sme.ocb_set_config_callback(
				pMac->sme.ocb_set_config_context,
				pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL(
				"Message error. The callback is NULL."));
		}
		pMac->sme.ocb_set_config_callback = NULL;
		pMac->sme.ocb_set_config_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_OCB_GET_TSF_TIMER_RSP:
		if (pMac->sme.ocb_get_tsf_timer_callback) {
			pMac->sme.ocb_get_tsf_timer_callback(
				pMac->sme.ocb_get_tsf_timer_context,
				pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL(
				"Message error. The callback is NULL."));
		}
		pMac->sme.ocb_get_tsf_timer_callback = NULL;
		pMac->sme.ocb_get_tsf_timer_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_GET_STATS_RSP:
		if (pMac->sme.dcc_get_stats_callback) {
			pMac->sme.dcc_get_stats_callback(
				pMac->sme.dcc_get_stats_context,
				pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL(
				"Message error. The callback is NULL."));
		}
		pMac->sme.dcc_get_stats_callback = NULL;
		pMac->sme.dcc_get_stats_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_UPDATE_NDL_RSP:
		if (pMac->sme.dcc_update_ndl_callback) {
			pMac->sme.dcc_update_ndl_callback(
				pMac->sme.dcc_update_ndl_context,
				pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL(
				"Message error. The callback is NULL."));
		}
		pMac->sme.dcc_update_ndl_callback = NULL;
		pMac->sme.dcc_update_ndl_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_STATS_EVENT:
		if (pMac->sme.dcc_stats_event_callback) {
			pMac->sme.dcc_stats_event_callback(
				pMac->sme.dcc_stats_event_context,
				pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL(
				"Message error. The callback is NULL."));
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_SET_DUAL_MAC_CFG_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_dual_mac_config_resp(pMac,
					pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
					pMsg->type);
		}
	case eWNI_SME_SET_THERMAL_LEVEL_IND:
		if (pMac->sme.set_thermal_level_cb)
			pMac->sme.set_thermal_level_cb(pMac->hHdd,
								pMsg->bodyval);
		break;
	case eWNI_SME_EXT_CHANGE_CHANNEL_IND:
		 status = sme_extended_change_channel_ind(pMac, pMsg->bodyptr);
		 qdf_mem_free(pMsg->bodyptr);
		 break;
	case eWNI_SME_SET_ANTENNA_MODE_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_antenna_mode_resp(pMac,
					pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sms_log(pMac, LOGE, FL("Empty message for %d"),
					pMsg->type);
		}
		break;
	case eWNI_SME_NDP_CONFIRM_IND:
	case eWNI_SME_NDP_NEW_PEER_IND:
	case eWNI_SME_NDP_INITIATOR_RSP:
	case eWNI_SME_NDP_INDICATION:
	case eWNI_SME_NDP_RESPONDER_RSP:
	case eWNI_SME_NDP_END_RSP:
	case eWNI_SME_NDP_END_IND:
	case eWNI_SME_NDP_PEER_DEPARTED_IND:
		sme_ndp_msg_processor(pMac, pMsg);
		break;
	default:

		if ((pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN)
		    && (pMsg->type <= eWNI_SME_MSG_TYPES_END)) {
			/* CSR */
			if (pMsg->bodyptr) {
				status = csr_msg_processor(hHal, pMsg->bodyptr);
				qdf_mem_free(pMsg->bodyptr);
			} else {
				sms_log(pMac, LOGE, FL("Empty message for %d"),
				pMsg->type);
			}
		} else {
			sms_log(pMac, LOGW, FL("Unknown message type %d"),
				pMsg->type);
			if (pMsg->bodyptr)
				qdf_mem_free(pMsg->bodyptr);
		}
	} /* switch */
release_lock:
	sme_release_global_lock(&pMac->sme);
	return status;
}

/**
 * sme_process_nss_update_resp() - Process nss update response
 * @mac: Global MAC pointer
 * @msg: nss update response
 *
 * Processes the nss update response and invokes the HDD
 * callback to process further
 */
QDF_STATUS sme_process_nss_update_resp(tpAniSirGlobal mac, uint8_t *msg)
{
	tListElem *entry = NULL;
	tSmeCmd *command = NULL;
	bool found;
	nss_update_cb callback = NULL;
	struct sir_beacon_tx_complete_rsp *param;

	param = (struct sir_beacon_tx_complete_rsp *)msg;
	if (!param) {
		sms_log(mac, LOGE, FL("nss update resp param is NULL"));
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_ll_peek_head(&mac->sme.smeCmdActiveList,
			LL_ACCESS_LOCK);
	if (!entry) {
		sms_log(mac, LOGE, FL("No cmd found in active list"));
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sms_log(mac, LOGE, FL("Base address is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_nss_update != command->command) {
		sms_log(mac, LOGE, FL("Command mismatch!"));
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.nss_update_cmd.nss_update_cb;
	if (callback) {
		if (!param) {
			sms_log(mac, LOGE,
				FL("Callback failed since nss update params is NULL"));
		} else {
			sms_log(mac, LOGE,
				FL("Calling HDD callback for nss update response"));
			callback(command->u.nss_update_cmd.context,
				param->tx_status,
				param->session_id,
				command->u.nss_update_cmd.next_action,
				command->u.nss_update_cmd.reason);
		}
	} else {
		sms_log(mac, LOGE, FL("Callback does not exisit"));
	}

	found = csr_ll_remove_entry(&mac->sme.smeCmdActiveList, entry,
			LL_ACCESS_LOCK);
	if (found) {
		/* Now put this command back on the avilable command list */
		sme_release_command(mac, command);
	}
	sme_process_pending_queue(mac);
	return QDF_STATUS_SUCCESS;
}

/* No need to hold the global lock here because this function can only be called */
/* after sme_stop. */
void sme_free_msg(tHalHandle hHal, cds_msg_t *pMsg)
{
	if (pMsg) {
		if (pMsg->bodyptr) {
			qdf_mem_free(pMsg->bodyptr);
		}
	}

}

/*--------------------------------------------------------------------------

   \brief sme_stop() - Stop all SME modules and put them at idle state

   The function stops each module in SME, PMC, CSR, etc. . Upon
   return, all modules are at idle state ready to start.

   This is a synchronous call
   \param hHal - The handle returned by mac_open
   \param tHalStopType - reason for stopping

   \return QDF_STATUS_SUCCESS - SME is stopped.

   Other status means SME is failed to stop but caller should still
   consider SME is stopped.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_stop(tHalHandle hHal, tHalStopType stopType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS fail_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	p2p_stop(hHal);

	status = csr_stop(pMac, stopType);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"csr_stop failed during smeStop with status=%d", status);
		fail_status = status;
	}

	purge_sme_cmd_list(pMac);

	if (!QDF_IS_STATUS_SUCCESS(fail_status)) {
		status = fail_status;
	}

	pMac->sme.state = SME_STATE_STOP;

	return status;
}

/*--------------------------------------------------------------------------

   \brief sme_close() - Release all SME modules and their resources.

   The function release each module in SME, PMC, CSR, etc. . Upon
   return, all modules are at closed state.

   No SME APIs can be involved after smeClose except smeOpen.
   smeClose must be called before mac_close.
   This is a synchronous call
   \param hHal - The handle returned by mac_open

   \return QDF_STATUS_SUCCESS - SME is successfully close.

   Other status means SME is failed to be closed but caller still cannot
   call any other SME functions except smeOpen.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_close(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS fail_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (!pMac)
		return QDF_STATUS_E_FAILURE;

	/* Note: pSession will be invalid from here on, do not access */
	status = csr_close(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"csr_close failed during sme close with status=%d",
			status);
		fail_status = status;
	}
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_close(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"Qos close failed during sme close with status=%d",
			status);
		fail_status = status;
	}
#endif
	status = sme_ps_close(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
				"sme_ps_close failed during smeClose status=%d",
				status);
		fail_status = status;
	}

	status = rrm_close(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE,
			"RRM close failed during sme close with status=%d",
			status);
		fail_status = status;
	}

	sme_p2p_close(hHal);

	free_sme_cmd_list(pMac);

	if (!QDF_IS_STATUS_SUCCESS
		    (qdf_mutex_destroy(&pMac->sme.lkSmeGlobalLock))) {
		fail_status = QDF_STATUS_E_FAILURE;
	}

	if (!QDF_IS_STATUS_SUCCESS(fail_status)) {
		status = fail_status;
	}

	pMac->sme.state = SME_STATE_STOP;

	return status;
}

/**
 * sme_scan_request() - wrapper function to Request a 11d or full scan from CSR.
 * @hal:          hal global context
 * @session_id:   session id
 * @scan_req:     scan req
 * @callback:     a callback function that scan calls upon finish, will not
 *                be called if csr_scan_request returns error
 * @ctx:          a pointer passed in for the callback
 *
 * This is a wrapper function to Request a 11d or full scan from CSR. This is
 * an asynchronous call
 *
 * Return: Status of operation
 */
QDF_STATUS sme_scan_request(tHalHandle hal, uint8_t session_id,
		tCsrScanRequest *scan_req,
		csr_scan_completeCallback callback, void *ctx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct ani_scan_req *scan_msg;
	cds_msg_t msg;
	uint32_t scan_req_id, scan_count;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
		 TRACE_CODE_SME_RX_HDD_MSG_SCAN_REQ, session_id,
		 scan_req->scanType));

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE, FL("Invalid session id:%d"),
				session_id);
		return status;
	}

	if (!mac_ctx->scan.fScanEnable) {
		sms_log(mac_ctx, LOGE, FL("fScanEnable false"));
		return status;
	}

	scan_count = csr_ll_count(&mac_ctx->sme.smeScanCmdActiveList);
	if (scan_count >= mac_ctx->scan.max_scan_count) {
		sms_log(mac_ctx, LOGE, FL("Max scan reached"));
		return QDF_STATUS_E_FAILURE;
	}
	wma_get_scan_id(&scan_req_id);
	scan_req->scan_id = scan_req_id;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("Unable to acquire lock"));
		return status;
	}
	sms_log(mac_ctx, LOG1, FL(" called"));
	scan_msg = qdf_mem_malloc(sizeof(struct ani_scan_req));
	if (NULL == scan_msg) {
		sms_log(mac_ctx, LOGE,
			" scan_req: failed to allocate mem for msg");
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_NOMEM;
	}
	scan_msg->msg_type = eWNI_SME_SCAN_CMD;
	scan_msg->msg_len = (uint16_t) sizeof(struct ani_scan_req);
	scan_msg->session_id = session_id;
	scan_msg->callback = callback;
	scan_msg->ctx = ctx;
	scan_msg->scan_param = qdf_mem_malloc(sizeof(tCsrScanRequest));
	if (NULL == scan_msg->scan_param) {
		sms_log(mac_ctx, LOGE,
			"scan_req:failed to allocate mem for scanreq");
		sme_release_global_lock(&mac_ctx->sme);
		qdf_mem_free(scan_msg);
		return QDF_STATUS_E_NOMEM;
	}
	csr_scan_copy_request(mac_ctx, scan_msg->scan_param, scan_req);
	msg.type = eWNI_SME_SCAN_CMD;
	msg.bodyptr = scan_msg;
	msg.reserved = 0;
	msg.bodyval = 0;
	if (QDF_STATUS_SUCCESS !=
		cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
		sms_log(mac_ctx, LOGE,
			" sme_scan_req failed to post msg");
		csr_scan_free_request(mac_ctx, scan_msg->scan_param);
		qdf_mem_free(scan_msg->scan_param);
		qdf_mem_free(scan_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac_ctx->sme);
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_get_result
    \brief a wrapper function to request scan results from CSR.
    This is a synchronous call
    \param pFilter - If pFilter is NULL, all cached results are returned
    \param phResult - an object for the result.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_scan_get_result(tHalHandle hHal, uint8_t sessionId,
			       tCsrScanResultFilter *pFilter,
			       tScanResultHandle *phResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_GET_RESULTS, sessionId,
			 0));
	sms_log(pMac, LOG2, FL("enter"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_get_result(hHal, pFilter, phResult);
		sme_release_global_lock(&pMac->sme);
	}
	sms_log(pMac, LOG2, FL("exit status %d"), status);

	return status;
}

/**
 * sme_get_ap_channel_from_scan_cache() - a wrapper function to get AP's
 *                                        channel id from CSR by filtering the
 *                                        result which matches our roam profile.
 * @profile: SAP adapter
 * @ap_chnl_id: pointer to channel id of SAP. Fill the value after finding the
 *              best ap from scan cache.
 *
 * This function is written to get AP's channel id from CSR by filtering
 * the result which matches our roam profile. This is a synchronous call.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS sme_get_ap_channel_from_scan_cache(tHalHandle hal_handle,
					tCsrRoamProfile *profile,
					tScanResultHandle *scan_cache,
					uint8_t *ap_chnl_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	tCsrScanResultFilter *scan_filter = NULL;
	tScanResultHandle filtered_scan_result = NULL;
	tSirBssDescription first_ap_profile;

	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("mac_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == scan_filter) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("scan_filter mem alloc failed"));
		return QDF_STATUS_E_FAILURE;
	} else {
		qdf_mem_set(scan_filter, sizeof(tCsrScanResultFilter), 0);
		qdf_mem_set(&first_ap_profile, sizeof(tSirBssDescription), 0);

		if (NULL == profile) {
			scan_filter->EncryptionType.numEntries = 1;
			scan_filter->EncryptionType.encryptionType[0]
				= eCSR_ENCRYPT_TYPE_NONE;
		} else {
			/* Here is the profile we need to connect to */
			status = csr_roam_prepare_filter_from_profile(mac_ctx,
					profile,
					scan_filter);
		}

		if (QDF_STATUS_SUCCESS == status) {
			/* Save the WPS info */
			if (NULL != profile) {
				scan_filter->bWPSAssociation =
						 profile->bWPSAssociation;
				scan_filter->bOSENAssociation =
						 profile->bOSENAssociation;
			} else {
				scan_filter->bWPSAssociation = 0;
				scan_filter->bOSENAssociation = 0;
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Preparing the profile filter failed"));
			qdf_mem_free(scan_filter);
			return QDF_STATUS_E_FAILURE;
		}
	}
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		status = csr_scan_get_result(hal_handle, scan_filter,
					  &filtered_scan_result);
		if (QDF_STATUS_SUCCESS == status) {
			csr_get_bssdescr_from_scan_handle(filtered_scan_result,
					&first_ap_profile);
			*scan_cache = filtered_scan_result;
			if (0 != first_ap_profile.channelId) {
				*ap_chnl_id = first_ap_profile.channelId;
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Found best AP & its on chnl[%d]"),
					  first_ap_profile.channelId);
			} else {
				/*
				 * This means scan result is empty
				 * so set the channel to zero, caller should
				 * take of zero channel id case.
				 */
				*ap_chnl_id = 0;
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("Scan is empty, set chnl to 0"));
				status = QDF_STATUS_E_FAILURE;
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					FL("Failed to get scan get result"));
			status = QDF_STATUS_E_FAILURE;
		}
		csr_free_scan_filter(mac_ctx, scan_filter);
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Aquiring lock failed"));
		csr_free_scan_filter(mac_ctx, scan_filter);
		status = QDF_STATUS_E_FAILURE;
	}
	qdf_mem_free(scan_filter);
	return status;
}

/**
 * sme_store_joinreq_param() - This function will pass station's join
 * request to store to csr.
 * @hal_handle: pointer to hal context.
 * @profile: pointer to station's roam profile.
 * @scan_cache: pointer to station's scan cache.
 * @roam_id: reference to roam_id variable being passed.
 * @session_id: station's session id.
 *
 * This function will pass station's join request further down to csr
 * to store it. this stored parameter will be used later.
 *
 * Return: true or false based on function's overall success.
 **/
bool sme_store_joinreq_param(tHalHandle hal_handle,
		tCsrRoamProfile *profile,
		tScanResultHandle scan_cache,
		uint32_t *roam_id,
		uint32_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	bool ret_status = true;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			TRACE_CODE_SME_RX_HDD_STORE_JOIN_REQ,
			session_id, 0));
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (false == csr_store_joinreq_param(mac_ctx, profile,
					scan_cache, roam_id, session_id)) {
			ret_status = false;
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		ret_status = false;
	}

	return ret_status;
}

/**
 * sme_clear_joinreq_param() - This function will pass station's clear
 * the join request to csr.
 * @hal_handle: pointer to hal context.
 * @session_id: station's session id.
 *
 * This function will pass station's clear join request further down to csr
 * to cleanup.
 *
 * Return: true or false based on function's overall success.
 **/
bool sme_clear_joinreq_param(tHalHandle hal_handle,
		uint32_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	bool ret_status = true;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
				TRACE_CODE_SME_RX_HDD_CLEAR_JOIN_REQ,
				session_id, 0));
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (false == csr_clear_joinreq_param(mac_ctx,
					session_id)) {
			ret_status = false;
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		ret_status = false;
	}

	return ret_status;
}

/**
 * sme_issue_stored_joinreq() - This function will issues station's stored
 * the join request to csr.
 * @hal_handle: pointer to hal context.
 * @roam_id: reference to roam_id variable being passed.
 * @session_id: station's session id.
 *
 * This function will issue station's stored join request further down to csr
 * to proceed forward.
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE.
 **/
QDF_STATUS sme_issue_stored_joinreq(tHalHandle hal_handle,
		uint32_t *roam_id,
		uint32_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS ret_status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
				TRACE_CODE_SME_RX_HDD_ISSUE_JOIN_REQ,
				session_id, 0));
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (QDF_STATUS_SUCCESS != csr_issue_stored_joinreq(mac_ctx,
					roam_id,
					session_id)) {
			ret_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		csr_clear_joinreq_param(mac_ctx, session_id);
		ret_status = QDF_STATUS_E_FAILURE;
	}
	return ret_status;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_flush_result
    \brief a wrapper function to request CSR to clear scan results.
    This is a synchronous call
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_scan_flush_result(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_FLUSH_RESULTS,
			 0, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_flush_result(hHal);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_filter_scan_results
    \brief a wrapper function to request CSR to clear scan results.
    This is a synchronous call
    \param tHalHandle - HAL context handle
    \param sessionId - session id
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_filter_scan_results(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_FLUSH_RESULTS,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_scan_filter_results(pMac);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

QDF_STATUS sme_scan_flush_p2p_result(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_FLUSH_P2PRESULTS,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_flush_selective_result(hHal, true);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_result_get_first
    \brief a wrapper function to request CSR to returns the first element of
	scan result.
    This is a synchronous call
    \param hScanResult - returned from csr_scan_get_result
    \return tCsrScanResultInfo * - NULL if no result
   ---------------------------------------------------------------------------*/
tCsrScanResultInfo *sme_scan_result_get_first(tHalHandle hHal,
					      tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrScanResultInfo *pRet = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_RESULT_GETFIRST,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pRet = csr_scan_result_get_first(pMac, hScanResult);
		sme_release_global_lock(&pMac->sme);
	}

	return pRet;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_result_get_next
    \brief a wrapper function to request CSR to returns the next element of
	scan result. It can be called without calling csr_scan_result_get_first
	first
    This is a synchronous call
    \param hScanResult - returned from csr_scan_get_result
    \return Null if no result or reach the end
   ---------------------------------------------------------------------------*/
tCsrScanResultInfo *sme_scan_result_get_next(tHalHandle hHal,
					     tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrScanResultInfo *pRet = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_RESULT_GETNEXT,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pRet = csr_scan_result_get_next(pMac, hScanResult);
		sme_release_global_lock(&pMac->sme);
	}

	return pRet;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_result_purge
    \brief a wrapper function to request CSR to remove all items(tCsrScanResult)
	in the list and free memory for each item
    This is a synchronous call
    \param hScanResult - returned from csr_scan_get_result. hScanResult is
			 considered gone by
    calling this function and even before this function reutrns.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_scan_result_purge(tHalHandle hHal, tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_RESULT_PURGE,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_result_purge(hHal, hScanResult);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_scan_get_pmkid_candidate_list
    \brief a wrapper function to return the PMKID candidate list
    This is a synchronous call
    \param pPmkidList - caller allocated buffer point to an array of
			tPmkidCandidateInfo
    \param pNumItems - pointer to a variable that has the number of
		       tPmkidCandidateInfo allocated when retruning, this is
		       either the number needed or number of items put into
		       pPmkidList
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough and pNumItems
			 has the number of tPmkidCandidateInfo.
    \Note: pNumItems is a number of tPmkidCandidateInfo,
	   not sizeof(tPmkidCandidateInfo) * something
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_scan_get_pmkid_candidate_list(tHalHandle hHal, uint8_t sessionId,
					     tPmkidCandidateInfo *pPmkidList,
					     uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status =
			csr_scan_get_pmkid_candidate_list(pMac, sessionId,
							  pPmkidList,
							  pNumItems);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

eCsrPhyMode sme_get_phy_mode(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.phyMode;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_channel_bonding_mode5_g
    \brief get the channel bonding mode for 5G band
    \param hHal - HAL handle
    \return channel bonding mode for 5G
   ---------------------------------------------------------------------------*/
uint32_t sme_get_channel_bonding_mode5_g(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeConfigParams smeConfig;

	sme_get_config_param(pMac, &smeConfig);

	return smeConfig.csrConfig.channelBondingMode5GHz;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_channel_bonding_mode24_g
    \brief get the channel bonding mode for 2.4G band
    \param hHal - HAL handle
    \return channel bonding mode for 2.4G
   ---------------------------------------------------------------------------*/
uint32_t sme_get_channel_bonding_mode24_g(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeConfigParams smeConfig;

	sme_get_config_param(pMac, &smeConfig);

	return smeConfig.csrConfig.channelBondingMode24GHz;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_connect
    \brief a wrapper function to request CSR to inititiate an association
    This is an asynchronous call.
    \param sessionId - the sessionId returned by sme_open_session.
    \param pProfile - description of the network to which to connect
    \param hBssListIn - a list of BSS descriptor to roam to. It is returned
			from csr_scan_get_result
    \param pRoamId - to get back the request ID
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_connect(tHalHandle hHal, uint8_t sessionId,
			    tCsrRoamProfile *pProfile, uint32_t *pRoamId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (!pMac) {
		return QDF_STATUS_E_FAILURE;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_CONNECT, sessionId, 0));
	sms_log(pMac, LOG2, FL("enter"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_connect(pMac, sessionId, pProfile,
						 pRoamId);
		} else {
			sms_log(pMac, LOGE, FL("invalid sessionID %d"),
				sessionId);
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		sms_log(pMac, LOGE, FL("sme_acquire_global_lock failed"));
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_phy_mode

    \brief Changes the PhyMode.

    \param hHal - The handle returned by mac_open.

    \param phyMode new phyMode which is to set

    \return QDF_STATUS  SUCCESS.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_set_phy_mode(tHalHandle hHal, eCsrPhyMode phyMode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid context", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pMac->roam.configParam.phyMode = phyMode;
	pMac->roam.configParam.uCfgDot11Mode =
		csr_get_cfg_dot11_mode_from_csr_phy_mode(NULL,
							 pMac->roam.configParam.phyMode,
							 pMac->roam.configParam.
							 ProprietaryRatesEnabled);

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_reassoc
    \brief a wrapper function to request CSR to inititiate a re-association
    \param pProfile - can be NULL to join the currently connected AP. In that
    case modProfileFields should carry the modified field(s) which could trigger
    reassoc
    \param modProfileFields - fields which are part of tCsrRoamConnectedProfile
    that might need modification dynamically once STA is up & running and this
    could trigger a reassoc
    \param pRoamId - to get back the request ID
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_reassoc(tHalHandle hHal, uint8_t sessionId,
			    tCsrRoamProfile *pProfile,
			    tCsrRoamModifyProfileFields modProfileFields,
			    uint32_t *pRoamId, bool fForce)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_REASSOC, sessionId, 0));
	sms_log(pMac, LOG2, FL("enter"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			if ((NULL == pProfile) && (fForce == 1)) {
				status =
					csr_reassoc(pMac, sessionId,
						    &modProfileFields, pRoamId,
						    fForce);
			} else {
				status =
					csr_roam_reassoc(pMac, sessionId, pProfile,
							 modProfileFields, pRoamId);
			}
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_connect_to_last_profile
    \brief a wrapper function to request CSR to disconnect and reconnect with
	the same profile
    This is an asynchronous call.
    \return QDF_STATUS. It returns fail if currently connected
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_connect_to_last_profile(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_GET_CONNECTPROFILE,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = csr_roam_connect_to_last_profile(pMac, sessionId);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_disconnect
    \brief a wrapper function to request CSR to disconnect from a network
    This is an asynchronous call.
    \param reason -- To indicate the reason for disconnecting. Currently, only
		     eCSR_DISCONNECT_REASON_MIC_ERROR is meanful.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_disconnect(tHalHandle hHal, uint8_t sessionId,
			       eCsrRoamDisconnectReason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DISCONNECT, sessionId,
			 reason));
	sms_log(pMac, LOG2, FL("enter"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = csr_roam_disconnect(pMac, sessionId, reason);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_stop_bss
    \brief To stop BSS for Soft AP. This is an asynchronous API.
    \param hHal - Global structure
    \param sessionId - sessionId of SoftAP
    \return QDF_STATUS  SUCCESS  Roam callback will be called to indicate actual results
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_stop_bss(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOG2, FL("enter"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = csr_roam_issue_stop_bss_cmd(pMac, sessionId, true);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_disconnect_sta
    \brief To disassociate a station. This is an asynchronous API.
    \param hHal - Global structure
    \param sessionId - sessionId of SoftAP
    \param pPeerMacAddr - Caller allocated memory filled with peer MAC address (6 bytes)
    \return QDF_STATUS  SUCCESS  Roam callback will be called to indicate actual results
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_disconnect_sta(tHalHandle hHal, uint8_t sessionId,
				   const uint8_t *pPeerMacAddr)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = csr_roam_issue_disassociate_sta_cmd(pMac,
					sessionId, pPeerMacAddr,
					eSIR_MAC_DEAUTH_LEAVING_BSS_REASON);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_roam_deauth_sta() - deauthenticate a station
 * @hHal:          Global structure
 * @sessionId:     SessionId of SoftAP
 * @pDelStaParams: Pointer to parameters of the station to deauthenticate
 *
 * To disassociate a station. This is an asynchronous API.
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS error
 *         code on error. Roam callback will be called to indicate actual
 *         result
 */
QDF_STATUS sme_roam_deauth_sta(tHalHandle hHal, uint8_t sessionId,
			       struct tagCsrDelStaParams *pDelStaParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_DEAUTH_STA,
			 sessionId, pDelStaParams->reason_code));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_issue_deauth_sta_cmd(pMac, sessionId,
							      pDelStaParams);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_tkip_counter_measures
    \brief To start or stop TKIP counter measures. This is an asynchronous API.
    \param sessionId - sessionId of SoftAP
    \param pPeerMacAddr - Caller allocated memory filled with peer MAC address (6 bytes)
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_tkip_counter_measures(tHalHandle hHal, uint8_t sessionId,
					  bool bEnable)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_issue_tkip_counter_measures(pMac, sessionId,
								     bEnable);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_associated_stas
    \brief To probe the list of associated stations from various modules
	 of CORE stack.
    \This is an asynchronous API.
    \param sessionId    - sessionId of SoftAP
    \param modId        - Module from whom list of associtated stations is
			  to be probed. If an invalid module is passed then
			  by default QDF_MODULE_ID_PE will be probed.
    \param pUsrContext  - Opaque HDD context
    \param pfnSapEventCallback  - Sap event callback in HDD
    \param pAssocBuf    - Caller allocated memory to be filled with associatd
			  stations info
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_associated_stas(tHalHandle hHal, uint8_t sessionId,
					QDF_MODULE_ID modId, void *pUsrContext,
					void *pfnSapEventCallback,
					uint8_t *pAssocStasBuf)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_associated_stas(pMac, sessionId, modId,
							     pUsrContext,
							     pfnSapEventCallback,
							     pAssocStasBuf);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_wps_session_overlap
    \brief To get the WPS PBC session overlap information.
    \This is an asynchronous API.
    \param sessionId    - sessionId of SoftAP
    \param pUsrContext  - Opaque HDD context
    \param pfnSapEventCallback  - Sap event callback in HDD
    \pRemoveMac - pointer to Mac address which needs to be removed from session
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_wps_session_overlap(tHalHandle hHal, uint8_t sessionId,
					    void *pUsrContext, void
					    *pfnSapEventCallback,
					    struct qdf_mac_addr pRemoveMac)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_wps_session_overlap(pMac, sessionId,
								 pUsrContext,
								 pfnSapEventCallback,
								 pRemoveMac);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_connect_state
    \brief a wrapper function to request CSR to return the current connect state
	of Roaming
    This is a synchronous call.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_connect_state(tHalHandle hHal, uint8_t sessionId,
				      eCsrConnectState *pState)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_connect_state(pMac, sessionId, pState);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_connect_profile
    \brief a wrapper function to request CSR to return the current connect
	profile. Caller must call csr_roam_free_connect_profile after it is done
	and before reuse for another csr_roam_get_connect_profile call.
    This is a synchronous call.
    \param pProfile - pointer to a caller allocated structure
		      tCsrRoamConnectedProfile
    \return QDF_STATUS. Failure if not connected
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_connect_profile(tHalHandle hHal, uint8_t sessionId,
					tCsrRoamConnectedProfile *pProfile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_GET_CONNECTPROFILE,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_connect_profile(pMac, sessionId, pProfile);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_roam_free_connect_profile - a wrapper function to request CSR to free and
 * reinitialize the profile returned previously by csr_roam_get_connect_profile.
 *
 * @profile - pointer to a caller allocated structure tCsrRoamConnectedProfile
 *
 * Return: none
 */
void sme_roam_free_connect_profile(tCsrRoamConnectedProfile *profile)
{
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_FREE_CONNECTPROFILE,
			 NO_SESSION, 0));
	csr_roam_free_connect_profile(profile);
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_set_pmkid_cache
    \brief a wrapper function to request CSR to return the PMKID candidate list
    This is a synchronous call.
    \param pPMKIDCache - caller allocated buffer point to an array of
			 tPmkidCacheInfo
    \param numItems - a variable that has the number of tPmkidCacheInfo
		      allocated when retruning, this is either the number needed
		      or number of items put into pPMKIDCache
    \param update_entire_cache - this bool value specifies if the entire pmkid
				 cache should be overwritten or should it be
				 updated entry by entry.
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough and pNumItems has the number of
			 tPmkidCacheInfo.
    \Note: pNumItems is a number of tPmkidCacheInfo,
	   not sizeof(tPmkidCacheInfo) * something
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_set_pmkid_cache(tHalHandle hHal, uint8_t sessionId,
				    tPmkidCacheInfo *pPMKIDCache,
				    uint32_t numItems, bool update_entire_cache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_SET_PMKIDCACHE, sessionId,
			 numItems));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_set_pmkid_cache(pMac, sessionId, pPMKIDCache,
							 numItems, update_entire_cache);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

QDF_STATUS sme_roam_del_pmkid_from_cache(tHalHandle hHal, uint8_t sessionId,
					 const uint8_t *pBSSId, bool flush_cache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DEL_PMKIDCACHE,
			 sessionId, flush_cache));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status = csr_roam_del_pmkid_from_cache(pMac, sessionId,
							       pBSSId, flush_cache);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/* ---------------------------------------------------------------------------
 * \fn sme_roam_set_psk_pmk
 * \brief a wrapper function to request CSR to save PSK/PMK
 *  This is a synchronous call.
 * \param hHal - Global structure
 * \param sessionId - SME sessionId
 * \param pPSK_PMK - pointer to an array of Psk[]/Pmk
 * \param pmk_len - Length could be only 16 bytes in case if LEAP
 *                  connections. Need to pass this information to
 *                  firmware.
 * \return QDF_STATUS -status whether PSK/PMK is set or not
 *---------------------------------------------------------------------------
 */
QDF_STATUS sme_roam_set_psk_pmk(tHalHandle hHal, uint8_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_set_psk_pmk(pMac, sessionId, pPSK_PMK,
						     pmk_len);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}
#endif
/* ---------------------------------------------------------------------------
    \fn sme_roam_get_security_req_ie
    \brief a wrapper function to request CSR to return the WPA or RSN or WAPI IE CSR
	passes to PE to JOIN request or START_BSS request
    This is a synchronous call.
    \param pLen - caller allocated memory that has the length of pBuf as input.
		  Upon returned, *pLen has the needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any,
		  upon return
    \param secType - Specifies whether looking for WPA/WPA2/WAPI IE
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_security_req_ie(tHalHandle hHal, uint8_t sessionId,
					uint32_t *pLen, uint8_t *pBuf,
					eCsrSecurityType secType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_wpa_rsn_req_ie(hHal, sessionId, pLen, pBuf);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_security_rsp_ie
    \brief a wrapper function to request CSR to return the WPA or RSN or
	 WAPI IE from the beacon or probe rsp if connected
    This is a synchronous call.
    \param pLen - caller allocated memory that has the length of pBuf as input.
		  Upon returned, *pLen has the needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any,
		  upon return
    \param secType - Specifies whether looking for WPA/WPA2/WAPI IE
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_security_rsp_ie(tHalHandle hHal, uint8_t sessionId,
					uint32_t *pLen, uint8_t *pBuf,
					eCsrSecurityType secType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_wpa_rsn_rsp_ie(pMac, sessionId, pLen, pBuf);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_num_pmkid_cache
    \brief a wrapper function to request CSR to return number of PMKID cache
	entries
    This is a synchronous call.
    \return uint32_t - the number of PMKID cache entries
   ---------------------------------------------------------------------------*/
uint32_t sme_roam_get_num_pmkid_cache(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint32_t numPmkidCache = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			numPmkidCache =
				csr_roam_get_num_pmkid_cache(pMac, sessionId);
			status = QDF_STATUS_SUCCESS;
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return numPmkidCache;
}

/* ---------------------------------------------------------------------------
    \fn sme_roam_get_pmkid_cache
    \brief a wrapper function to request CSR to return PMKID cache from CSR
    This is a synchronous call.
    \param pNum - caller allocated memory that has the space of the number of
		  pBuf tPmkidCacheInfo as input. Upon returned, *pNum has the
		  needed or actually number in tPmkidCacheInfo.
    \param pPmkidCache - Caller allocated memory that contains PMKID cache, if
			 any, upon return
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_get_pmkid_cache(tHalHandle hHal, uint8_t sessionId,
				    uint32_t *pNum, tPmkidCacheInfo *pPmkidCache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_GET_PMKIDCACHE, sessionId,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_get_pmkid_cache(pMac, sessionId, pNum,
							 pPmkidCache);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_config_param
    \brief a wrapper function that HDD calls to get the global settings
	currently maintained by CSR.
    This is a synchronous call.
    \param pParam - caller allocated memory
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_config_param(tHalHandle hHal, tSmeConfigParams *pParam)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_CONFIGPARAM, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_config_param(pMac, &pParam->csrConfig);
		if (status != QDF_STATUS_SUCCESS) {
			sms_log(pMac, LOGE, "%s csr_get_config_param failed",
				__func__);
			sme_release_global_lock(&pMac->sme);
			return status;
		}
		qdf_mem_copy(&pParam->rrmConfig,
				&pMac->rrm.rrmSmeContext.rrmConfig,
				sizeof(pMac->rrm.rrmSmeContext.rrmConfig));
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_cfg_set_int() - Sets the cfg parameter value.
 * @hal:	Handle to hal.
 * @cfg_id:	Configuration parameter ID.
 * @value: 	value to be saved in the cfg parameter.
 *
 * This function sets the string value in cfg parameter.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_cfg_set_int(tHalHandle hal, uint16_t cfg_id, uint32_t value)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (eSIR_SUCCESS != cfg_set_int(pmac, cfg_id, value))
		status = QDF_STATUS_E_FAILURE;

	return status;
}

/**
 * sme_cfg_set_str() - Sets the cfg parameter string.
 * @hal:	Handle to hal.
 * @cfg_id:	Configuration parameter ID.
 * @str:	Pointer to the string buffer.
 * @length:	Length of the string.
 *
 * This function sets the string value in cfg parameter.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_cfg_set_str(tHalHandle hal, uint16_t cfg_id, uint8_t *str,
			   uint32_t length)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (eSIR_SUCCESS != cfg_set_str(pmac, cfg_id, str, length))
		status = QDF_STATUS_E_FAILURE;

	return status;
}

/**
 * sme_cfg_get_int() -  Gets the cfg parameter value.
 * @hal:	Handle to hal.
 * @cfg_id:	Configuration parameter ID.
 * @cfg_value:	Pointer to variable in which cfg value
 * 		will be saved.
 *
 * This function gets the value of the cfg parameter.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_cfg_get_int(tHalHandle hal, uint16_t cfg_id, uint32_t *cfg_value)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (eSIR_SUCCESS != wlan_cfg_get_int(pmac, cfg_id, cfg_value))
		status = QDF_STATUS_E_FAILURE;

	return status;
}

/**
 * sme_cfg_get_str() - Gets the cfg parameter string.
 * @hal:	Handle to hal.
 * @cfg_id:	Configuration parameter ID.
 * @str:	Pointer to the string buffer.
 * @length:	Pointer to length of the string.
 *
 * This function gets the string value of the cfg parameter.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_cfg_get_str(tHalHandle hal, uint16_t cfg_id, uint8_t *str,
			   uint32_t *length)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (eSIR_SUCCESS != wlan_cfg_get_str(pmac, cfg_id, str, length))
		status = QDF_STATUS_E_INVAL;

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_modify_profile_fields
    \brief HDD or SME - QOS calls this function to get the current values of
    connected profile fields, changing which can cause reassoc.
    This function must be called after CFG is downloaded and STA is in connected
    state. Also, make sure to call this function to get the current profile
    fields before calling the reassoc. So that pModifyProfileFields will have
    all the latest values plus the one(s) has been updated as part of reassoc
    request.
    \param pModifyProfileFields - pointer to the connected profile fields
    changing which can cause reassoc

    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_get_modify_profile_fields(tHalHandle hHal, uint8_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_MODPROFFIELDS, sessionId,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_get_modify_profile_fields(pMac, sessionId,
							      pModifyProfileFields);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn  sme_set_dhcp_till_power_active_flag
    \brief  Sets/Clears DHCP related flag to disable/enable auto PS
    \param  hal - The handle returned by mac_open.
   ---------------------------------------------------------------------------*/
void sme_set_dhcp_till_power_active_flag(tHalHandle hal, uint8_t flag)
{
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct ps_global_info *ps_global_info = &mac->sme.ps_global_info;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
				TRACE_CODE_SME_RX_HDD_SET_DHCP_FLAG, NO_SESSION,
				flag));
	/* Set/Clear the DHCP flag which will disable/enable auto PS */
	ps_global_info->remain_in_power_active_till_dhcp = flag;
}

/* ---------------------------------------------------------------------------
    \fn sme_register11d_scan_done_callback
    \brief  Register a routine of type csr_scan_completeCallback which is
	called whenever an 11d scan is done
    \param  hHal - The handle returned by mac_open.
    \param  callback -  11d scan complete routine to be registered
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_register11d_scan_done_callback(tHalHandle hHal,
					      csr_scan_completeCallback callback)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->scan.callback11dScanDone = callback;

	return status;
}

/**
 * sme_deregister11d_scan_done_callback() - De-register scandone callback
 * @h_hal: Handler return by mac_open
 *
 * This function De-registers the scandone callback  to SME
 *
 * Return: None
 */
void sme_deregister11d_scan_done_callback(tHalHandle h_hal)
{
	tpAniSirGlobal pmac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("hHal is not valid"));
		return;
	}

	pmac = PMAC_STRUCT(h_hal);
	pmac->scan.callback11dScanDone = NULL;
}


#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * sme_register_oem_data_rsp_callback() - Register a routine of
 *                                        type send_oem_data_rsp_msg
 * @h_hal:                                Handle returned by mac_open.
 * @callback:                             Callback to send response
 *                                        to oem application.
 *
 * sme_oem_data_rsp_callback is used to register sme_send_oem_data_rsp_msg
 * callback function.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS sme_register_oem_data_rsp_callback(tHalHandle h_hal,
				sme_send_oem_data_rsp_msg callback)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pmac = PMAC_STRUCT(h_hal);

	pmac->sme.oem_data_rsp_callback = callback;

	return status;

}

/**
 * sme_deregister_oem_data_rsp_callback() - De-register OEM datarsp callback
 * @h_hal: Handler return by mac_open
 * This function De-registers the OEM data response callback  to SME
 *
 * Return: None
 */
void  sme_deregister_oem_data_rsp_callback(tHalHandle h_hal)
{
	tpAniSirGlobal pmac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("hHal is not valid"));
		return;
	}
	pmac = PMAC_STRUCT(h_hal);

	pmac->sme.oem_data_rsp_callback = NULL;
}
#endif

/**
 * sme_oem_update_capability() - update UMAC's oem related capability.
 * @hal: Handle returned by mac_open
 * @oem_cap: pointer to oem_capability
 *
 * This function updates OEM capability to UMAC. Currently RTT
 * related capabilities are updated. More capabilities can be
 * added in future.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_update_capability(tHalHandle hal,
				     struct sme_oem_capability *cap)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	uint8_t *bytes;

	bytes = pmac->rrm.rrmSmeContext.rrmConfig.rm_capability;

	if (cap->ftm_rr)
		bytes[4] |= RM_CAP_FTM_RANGE_REPORT;
	if (cap->lci_capability)
		bytes[4] |= RM_CAP_CIVIC_LOC_MEASUREMENT;

	return status;
}

/**
 * sme_oem_get_capability() - get oem capability
 * @hal: Handle returned by mac_open
 * @oem_cap: pointer to oem_capability
 *
 * This function is used to get the OEM capability from UMAC.
 * Currently RTT related capabilities are received. More
 * capabilities can be added in future.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_get_capability(tHalHandle hal,
				  struct sme_oem_capability *cap)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	uint8_t *bytes;

	bytes = pmac->rrm.rrmSmeContext.rrmConfig.rm_capability;

	cap->ftm_rr = bytes[4] & RM_CAP_FTM_RANGE_REPORT;
	cap->lci_capability = bytes[4] & RM_CAP_CIVIC_LOC_MEASUREMENT;

	return status;
}

/**
 * sme_register_ftm_msg_processor() - registers hdd ftm message processor
 * function to MAC/SYS
 *
 * @hal:        hal handle
 * @callback:   hdd function that has to be registered
 *
 * Return: void
 */
void sme_register_ftm_msg_processor(tHalHandle hal,
				    hdd_ftm_msg_processor callback)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (mac_ctx == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("mac ctx is NULL"));
		return;
	}

	mac_ctx->ftm_msg_processor_callback = callback;
	return;
}

/**
 * sme_wow_add_pattern() - add a wow pattern in fw
 * @hHal: handle returned by mac_open
 * @pattern: pointer to input pattern
 *
 * Add a pattern for Pattern Byte Matching in WoW mode. Firmware will
 * do a pattern match on these patterns when WoW is enabled during system
 * suspend.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_wow_add_pattern(tHalHandle hal,
			struct wow_add_pattern *pattern,
			uint8_t session_id)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	struct wow_add_pattern *ptrn;
	tSirRetStatus ret_code = eSIR_SUCCESS;
	tSirMsgQ msg_q;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			TRACE_CODE_SME_RX_HDD_WOWL_ADDBCAST_PATTERN, session_id,
			0));
	ptrn = qdf_mem_malloc(sizeof(*ptrn));
	if (NULL == ptrn) {
		sms_log(pMac, LOGP,
			FL("Fail to allocate memory for WoWLAN Add Bcast Pattern "));
		return QDF_STATUS_E_NOMEM;
	}
	(void)qdf_mem_copy(ptrn, pattern, sizeof(*ptrn));

	msg_q.type = WMA_WOW_ADD_PTRN;
	msg_q.reserved = 0;
	msg_q.bodyptr = ptrn;
	msg_q.bodyval = 0;

	sms_log(pMac, LOG1, FL("Sending WMA_WOWL_ADD_BCAST_PTRN to HAL"));
	ret_code = wma_post_ctrl_msg(pMac, &msg_q);
	if (eSIR_SUCCESS != ret_code) {
		sms_log(pMac, LOGE,
			FL("Posting WMA_WOWL_ADD_BCAST_PTRN failed, reason=%X"),
			ret_code);
	}
	return ret_code;
}

/**
 * sme_wow_delete_pattern() - delete user configured wow pattern in target
 * @hHal: handle returned by mac_open.
 * @pattern: pointer to delete pattern parameter
 * @sessionId: session id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_wow_delete_pattern(tHalHandle hal,
		struct wow_delete_pattern *pattern, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	struct wow_delete_pattern *delete_ptrn;
	tSirRetStatus ret_code = eSIR_SUCCESS;
	tSirMsgQ msg_q;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_WOWL_DELBCAST_PATTERN, sessionId,
			 0));
	delete_ptrn = qdf_mem_malloc(sizeof(*delete_ptrn));
	if (NULL == delete_ptrn) {
		sms_log(pMac, LOGP,
			FL("Fail to allocate memory for WoWLAN Delete Bcast Pattern "));
		return QDF_STATUS_E_NOMEM;
	}
	(void)qdf_mem_copy(delete_ptrn, pattern, sizeof(*delete_ptrn));
	msg_q.type = WMA_WOW_DEL_PTRN;
	msg_q.reserved = 0;
	msg_q.bodyptr = delete_ptrn;
	msg_q.bodyval = 0;

	sms_log(pMac, LOG1, FL("Sending WMA_WOWL_DEL_BCAST_PTRN"));

	ret_code = wma_post_ctrl_msg(pMac, &msg_q);
	if (eSIR_SUCCESS != ret_code) {
		sms_log(pMac, LOGE,
			FL("Posting WMA_WOWL_DEL_BCAST_PTRN failed, reason=%X"),
			ret_code);
	}
	return ret_code;
}

/**
 * sme_enter_wowl(): SME API exposed to HDD to request enabling of WOWL mode.
 * @hal_ctx - The handle returned by mac_open.
 * @enter_wowl_callback_routine -  Callback routine provided by HDD.
 *		Used for success/failure notification by SME
 * @enter_wowl_callback_context - A cookie passed by HDD, that is passed
 *		back to HDD at the time of callback.
 * @wake_reason_ind_cb -  Callback routine provided by HDD.
 *		Used for Wake Reason Indication by SME
 * @wake_reason_ind_cb_ctx - A cookie passed by HDD, that is passed
 *		back to HDD at the time of callback.
 *
 * WoWLAN works on top of BMPS mode.
 * If the device is not in BMPS mode,
 * SME will will cache the information that
 * WOWL has been enabled and attempt to put the device
 * in BMPS. On entry into BMPS, SME will enable the
 * WOWL mode.
 * Note 1: If we exit BMPS mode (someone requests full power),
 * we will NOT resume WOWL when we go back to BMPS again.
 * Request for full power (while in WOWL mode) means disable
 * WOWL and go to full power.
 * Note 2: Both UAPSD and WOWL work on top of BMPS.
 * On entry into BMPS, SME will give priority to UAPSD and
 * enable only UAPSD if both UAPSD and WOWL are required.
 * Currently there is no requirement or use case to support
 * UAPSD and WOWL at the same time.
 *
 * Return: QDF_STATUS
 *	QDF_STATUS_SUCCESS  Device is already in WoWLAN mode
 *	QDF_STATUS_E_FAILURE  Device cannot enter WoWLAN mode.
 *	QDF_STATUS_PMC_PENDING  Request accepted. SME will enable
 *			WOWL after BMPS mode is entered.
 */
QDF_STATUS sme_enter_wowl(tHalHandle hal_ctx,
		void (*enter_wowl_callback_routine)(void
			*callback_context,
			QDF_STATUS status),
		void *enter_wowl_callback_context,
#ifdef WLAN_WAKEUP_EVENTS
		void (*wakeIndicationCB)(void *callback_context,
			tpSirWakeReasonInd
			wake_reason_ind),
		void *wakeIndicationCBContext,
#endif /* WLAN_WAKEUP_EVENTS */
		tpSirSmeWowlEnterParams wowl_enter_params,
		uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_ctx);
	struct ps_global_info *ps_global_info = &mac_ctx->sme.ps_global_info;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			TRACE_CODE_SME_RX_HDD_ENTER_WOWL, session_id, 0));

	/* cache the WOWL information */
	ps_global_info->ps_params[session_id].wowl_enter_params =
		*wowl_enter_params;
	ps_global_info->ps_params[session_id].enter_wowl_callback_routine =
		enter_wowl_callback_routine;
	ps_global_info->ps_params[session_id].enter_wowl_callback_context =
		enter_wowl_callback_context;
#ifdef WLAN_WAKEUP_EVENTS
	/* Cache the Wake Reason Indication callback information */
	ps_global_info->ps_params[session_id].wake_reason_ind_cb =
		wakeIndicationCB;
	ps_global_info->ps_params[session_id].wake_reason_ind_cb_ctx =
		wakeIndicationCBContext;
#endif /* WLAN_WAKEUP_EVENTS */

	status = sme_ps_process_command(mac_ctx, session_id, SME_PS_WOWL_ENTER);
	return status;
}
/**
 *sme_exit_wowl(): SME API exposed to HDD to request exit from WoWLAN mode.
 * @hal_ctx - The handle returned by mac_open.
 * @wowl_exit_params - Carries info on which smesession
 *			wowl exit is requested.
 *
 * SME will initiate exit from WoWLAN mode and device will be
 * put in BMPS mode.
 * Return QDF_STATUS
 *	QDF_STATUS_E_FAILURE  Device cannot exit WoWLAN mode.
 *	QDF_STATUS_SUCCESS  Request accepted to exit WoWLAN mode.
 */
QDF_STATUS sme_exit_wowl(tHalHandle hal_ctx,
		tpSirSmeWowlExitParams wowl_exit_params)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_ctx);
	uint8_t session_id;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			TRACE_CODE_SME_RX_HDD_EXIT_WOWL, NO_SESSION, 0));
	session_id = wowl_exit_params->sessionId;
	status = sme_ps_process_command(mac_ctx, session_id, SME_PS_WOWL_EXIT);
	return status;
}

/**
 * sme_roam_set_key() - To set encryption key.
 * @hal:           hal global context
 * @session_id:    session id
 * @set_key:       pointer to a caller allocated object of tCsrSetContextInfo
 * @ptr_roam_id:       Upon success return, this is the id caller can use to
 *                 identify the request in roamcallback
 *
 * This function should be called only when connected. This is an asynchronous
 * API.
 *
 * Return: Status of operation
 */
QDF_STATUS sme_roam_set_key(tHalHandle hal,  uint8_t session_id,
			    tCsrRoamSetKey *set_key, uint32_t *ptr_roam_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	uint32_t roam_id;
	uint32_t i;
	tCsrRoamSession *session = NULL;
	struct ps_global_info *ps_global_info = &mac_ctx->sme.ps_global_info;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_HDD_SET_KEY,
			 session_id, 0));
	if (set_key->keyLength > CSR_MAX_KEY_LEN) {
		sms_log(mac_ctx, LOGE, FL("Invalid key length %d"),
			set_key->keyLength);
		return QDF_STATUS_E_FAILURE;
	}
	/*Once Setkey is done, we can go in BMPS */
	if (set_key->keyLength)
		ps_global_info->remain_in_power_active_till_dhcp = false;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	roam_id = GET_NEXT_ROAM_ID(&mac_ctx->roam);
	if (ptr_roam_id)
		*ptr_roam_id = roam_id;

	sms_log(mac_ctx, LOG2, FL("keyLength %d"), set_key->keyLength);
	for (i = 0; i < set_key->keyLength; i++)
		sms_log(mac_ctx, LOG2, FL("%02x"), set_key->Key[i]);

	sms_log(mac_ctx, LOG2, "\n session_id=%d roam_id=%d", session_id,
		roam_id);
	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sms_log(mac_ctx, LOGE, FL("session %d not found"), session_id);
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_FAILURE;
	}
	if (CSR_IS_INFRA_AP(&session->connectedProfile)
	    && set_key->keyDirection == eSIR_TX_DEFAULT) {
		if ((eCSR_ENCRYPT_TYPE_WEP40 == set_key->encType)
		    || (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY ==
			set_key->encType)) {
			session->pCurRoamProfile->negotiatedUCEncryptionType =
				eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
		}
		if ((eCSR_ENCRYPT_TYPE_WEP104 == set_key->encType)
		    || (eCSR_ENCRYPT_TYPE_WEP104_STATICKEY ==
			set_key->encType)) {
			session->pCurRoamProfile->negotiatedUCEncryptionType =
				eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
		}
	}
	status = csr_roam_set_key(mac_ctx, session_id, set_key, roam_id);
	sme_release_global_lock(&mac_ctx->sme);
	return status;
}

/**
 * sme_roam_set_default_key_index - To set default wep key idx
 * @hal: pointer to hal handler
 * @session_id: session id
 * @default_idx: default wep key index
 *
 * This function prepares a message and post to WMA to set wep default
 * key index
 *
 * Return: Success:QDF_STATUS_SUCCESS Failure: Error value
 */
QDF_STATUS sme_roam_set_default_key_index(tHalHandle hal, uint8_t session_id,
					  uint8_t default_idx)
{
	cds_msg_t msg;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct wep_update_default_key_idx *update_key;

	update_key = qdf_mem_malloc(sizeof(*update_key));
	if (!update_key) {
		sms_log(mac_ctx, LOGE,
			FL("Failed to allocate memory for update key"));
		return QDF_STATUS_E_NOMEM;
	}

	update_key->session_id = session_id;
	update_key->default_idx = default_idx;

	msg.type = WMA_UPDATE_WEP_DEFAULT_KEY;
	msg.reserved = 0;
	msg.bodyptr = (void *)update_key;

	if (QDF_STATUS_SUCCESS !=
	    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		sms_log(mac_ctx, LOGE,
			FL("Failed to post WMA_UPDATE_WEP_DEFAULT_KEY to WMA"));
		qdf_mem_free(update_key);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/* ---------------------------------------------------------------------------
    \fn sme_get_rssi
    \brief a wrapper function that client calls to register a callback to get
	RSSI

    \param hHal - HAL handle for device
    \param callback - SME sends back the requested stats using the callback
    \param staId -    The station ID for which the stats is requested for
    \param bssid - The bssid of the connected session
    \param lastRSSI - RSSI value at time of request. In case fw cannot provide
		      RSSI, do not hold up but return this value.
    \param pContext - user context to be passed back along with the callback
    \param p_cds_context - cds context
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_rssi(tHalHandle hHal,
			tCsrRssiCallback callback,
			uint8_t staId, struct qdf_mac_addr bssId, int8_t lastRSSI,
			void *pContext, void *p_cds_context)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_RSSI, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_rssi(pMac, callback,
				      staId, bssId, lastRSSI,
				      pContext, p_cds_context);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_snr
    \brief a wrapper function that client calls to register a callback to
	get SNR

    \param callback - SME sends back the requested stats using the callback
    \param staId - The station ID for which the stats is requested for
    \param pContext - user context to be passed back along with the callback
    \param p_cds_context - cds context
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_snr(tHalHandle hHal,
		       tCsrSnrCallback callback,
		       uint8_t staId, struct qdf_mac_addr bssId, void *pContext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_snr(pMac, callback, staId, bssId, pContext);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_statistics
    \brief a wrapper function that client calls to register a callback to get
    different PHY level statistics from CSR.

    \param requesterId - different client requesting for statistics,
	HDD, UMA/GAN etc
    \param statsMask - The different category/categories of stats requester
	is looking for
    \param callback - SME sends back the requested stats using the callback
    \param periodicity - If requester needs periodic update in millisec, 0 means
			 it's an one time request
    \param cache - If requester is happy with cached stats
    \param staId - The station ID for which the stats is requested for
    \param pContext - user context to be passed back along with the callback
    \param sessionId - sme session interface
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_statistics(tHalHandle hHal,
			      eCsrStatsRequesterType requesterId,
			      uint32_t statsMask, tCsrStatsCallback callback,
			      uint32_t periodicity, bool cache, uint8_t staId,
			      void *pContext, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_STATS, NO_SESSION,
			 periodicity));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status =
			csr_get_statistics(pMac, requesterId, statsMask, callback,
					   periodicity, cache, staId, pContext,
					   sessionId);
		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

QDF_STATUS sme_get_link_status(tHalHandle hHal,
			       tCsrLinkStatusCallback callback,
			       void *pContext, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tAniGetLinkStatus *pMsg;
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMsg = qdf_mem_malloc(sizeof(tAniGetLinkStatus));
		if (NULL == pMsg) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to allocate memory for link status",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		pMsg->msgType = WMA_LINK_STATUS_GET_REQ;
		pMsg->msgLen = (uint16_t) sizeof(tAniGetLinkStatus);
		pMsg->sessionId = sessionId;
		pMac->sme.linkStatusContext = pContext;
		pMac->sme.linkStatusCallback = callback;

		cds_message.type = WMA_LINK_STATUS_GET_REQ;
		cds_message.bodyptr = pMsg;
		cds_message.reserved = 0;

		if (!QDF_IS_STATUS_SUCCESS
			    (cds_mq_post_message(QDF_MODULE_ID_WMA, &cds_message))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post LINK STATUS MSG fail", __func__);
			qdf_mem_free(pMsg);
			pMac->sme.linkStatusContext = NULL;
			pMac->sme.linkStatusCallback = NULL;
			status = QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_get_country_code

    \brief To return the current country code. If no country code is applied,
	default country code is used to fill the buffer.
	If 11d supported is turned off, an error is return and the last
	applied/default country code is used.
	This is a synchronous API.

    \param pBuf - pointer to a caller allocated buffer for returned country code.

    \param pbLen  For input, this parameter indicates how big is the buffer.
		  Upon return, this parameter has the number of bytes for
		  country. If pBuf doesn't have enough space, this function
		  returns fail status and this parameter contains the number
		  that is needed.

    \return QDF_STATUS  SUCCESS.

			FAILURE or RESOURCES  The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_get_country_code(tHalHandle hHal, uint8_t *pBuf, uint8_t *pbLen)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_CNTRYCODE, NO_SESSION, 0));

	return csr_get_country_code(pMac, pBuf, pbLen);
}

/**
 * sme_apply_channel_power_info_to_fw() - sends channel info to fw
 * @hHal: hal handle
 *
 * This function sends the channel power info to firmware
 *
 * Return: none
 */
void sme_apply_channel_power_info_to_fw(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	csr_apply_channel_power_info_wrapper(pMac);
}

/* some support functions */
bool sme_is11d_supported(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return csr_is11d_supported(pMac);
}

bool sme_is11h_supported(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return csr_is11h_supported(pMac);
}

bool sme_is_wmm_supported(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return csr_is_wmm_supported(pMac);
}

/* ---------------------------------------------------------------------------

    \fn sme_change_country_code

    \brief Change Country code from upperlayer during WLAN driver operation.
	This is a synchronous API.

    \param hHal - The handle returned by mac_open.

    \param pCountry New Country Code String

    \param sendRegHint If we want to send reg hint to nl80211

    \return QDF_STATUS  SUCCESS.

			FAILURE or RESOURCES  The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_change_country_code(tHalHandle hHal,
				   tSmeChangeCountryCallback callback,
				   uint8_t *pCountry,
				   void *pContext,
				   void *p_cds_context,
				   tAniBool countryFromUserSpace,
				   tAniBool sendRegHint)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg;
	tAniChangeCountryCodeReq *pMsg;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CHANGE_CNTRYCODE, NO_SESSION,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOG1, FL(" called"));

		if ((pMac->roam.configParam.Is11dSupportEnabledOriginal == true)
		    && (!pMac->roam.configParam.
			fSupplicantCountryCodeHasPriority)) {

			sms_log(pMac, LOGW,
				"Set Country Code Fail since the STA is associated and userspace does not have priority ");

			sme_release_global_lock(&pMac->sme);
			status = QDF_STATUS_E_FAILURE;
			return status;
		}

		pMsg = qdf_mem_malloc(sizeof(tAniChangeCountryCodeReq));
		if (NULL == pMsg) {
			sms_log(pMac, LOGE,
				" csrChangeCountryCode: failed to allocate mem for req");
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		pMsg->msgType = eWNI_SME_CHANGE_COUNTRY_CODE;
		pMsg->msgLen = (uint16_t) sizeof(tAniChangeCountryCodeReq);
		qdf_mem_copy(pMsg->countryCode, pCountry, 3);
		pMsg->countryFromUserSpace = countryFromUserSpace;
		pMsg->sendRegHint = sendRegHint;
		pMsg->changeCCCallback = callback;
		pMsg->pDevContext = pContext;
		pMsg->p_cds_context = p_cds_context;

		msg.type = eWNI_SME_CHANGE_COUNTRY_CODE;
		msg.bodyptr = pMsg;
		msg.reserved = 0;

		if (QDF_STATUS_SUCCESS !=
		    cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
			sms_log(pMac, LOGE,
				" sme_change_country_code failed to post msg to self ");
			qdf_mem_free((void *)pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sms_log(pMac, LOG1, FL(" returned"));
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------

    \fn sme_generic_change_country_code

    \brief Change Country code from upperlayer during WLAN driver operation.
	This is a synchronous API.

    \param hHal - The handle returned by mac_open.

    \param pCountry New Country Code String

    \param reg_domain regulatory domain

    \return QDF_STATUS  SUCCESS.

			FAILURE or RESOURCES  The API finished and failed.

   -----------------------------------------------------------------------------*/
QDF_STATUS sme_generic_change_country_code(tHalHandle hHal,
					   uint8_t *pCountry)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg;
	tAniGenericChangeCountryCodeReq *pMsg;

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOG1, FL(" called"));
		pMsg = qdf_mem_malloc(sizeof(tAniGenericChangeCountryCodeReq));

		if (NULL == pMsg) {
			sms_log(pMac, LOGE,
				" sme_generic_change_country_code: failed to allocate mem for req");
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		pMsg->msgType = eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE;
		pMsg->msgLen =
			(uint16_t) sizeof(tAniGenericChangeCountryCodeReq);
		qdf_mem_copy(pMsg->countryCode, pCountry, 2);
		pMsg->countryCode[2] = ' ';

		msg.type = eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE;
		msg.bodyptr = pMsg;
		msg.reserved = 0;

		if (QDF_STATUS_SUCCESS !=
		    cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
			sms_log(pMac, LOGE,
				"sme_generic_change_country_code failed to post msg to self");
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sms_log(pMac, LOG1, FL(" returned"));
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_dhcp_start_ind

    \brief API to signal the FW about the DHCP Start event.

    \param hHal - HAL handle for device.

    \param device_mode - mode(AP,SAP etc) of the device.

    \param macAddr - MAC address of the adapter.

    \param sessionId - session ID.

    \return QDF_STATUS  SUCCESS.

			FAILURE or RESOURCES  The API finished and failed.
   --------------------------------------------------------------------------*/
QDF_STATUS sme_dhcp_start_ind(tHalHandle hHal,
			      uint8_t device_mode,
			      uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tAniDHCPInd *pMsg;
	tCsrRoamSession *pSession;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sms_log(pMac, LOGE, FL("session %d not found "),
				sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		pMsg = (tAniDHCPInd *) qdf_mem_malloc(sizeof(tAniDHCPInd));
		if (NULL == pMsg) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to allocate memory for dhcp start",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		pMsg->msgType = WMA_DHCP_START_IND;
		pMsg->msgLen = (uint16_t) sizeof(tAniDHCPInd);
		pMsg->device_mode = device_mode;
		qdf_mem_copy(pMsg->adapterMacAddr.bytes, macAddr,
			     QDF_MAC_ADDR_SIZE);
		qdf_copy_macaddr(&pMsg->peerMacAddr,
				 &pSession->connectedProfile.bssid);

		cds_message.type = WMA_DHCP_START_IND;
		cds_message.bodyptr = pMsg;
		cds_message.reserved = 0;

		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post DHCP Start MSG fail", __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_dhcp_stop_ind

    \brief API to signal the FW about the DHCP complete event.

    \param hHal - HAL handle for device.

    \param device_mode - mode(AP, SAP etc) of the device.

    \param macAddr - MAC address of the adapter.

    \param sessionId - session ID.

    \return QDF_STATUS  SUCCESS.
			FAILURE or RESOURCES  The API finished and failed.
   --------------------------------------------------------------------------*/
QDF_STATUS sme_dhcp_stop_ind(tHalHandle hHal,
			     uint8_t device_mode,
			     uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tAniDHCPInd *pMsg;
	tCsrRoamSession *pSession;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sms_log(pMac, LOGE, FL("session %d not found "),
				sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		pMsg = (tAniDHCPInd *) qdf_mem_malloc(sizeof(tAniDHCPInd));
		if (NULL == pMsg) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to allocate memory for dhcp stop",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		pMsg->msgType = WMA_DHCP_STOP_IND;
		pMsg->msgLen = (uint16_t) sizeof(tAniDHCPInd);
		pMsg->device_mode = device_mode;
		qdf_mem_copy(pMsg->adapterMacAddr.bytes, macAddr,
			     QDF_MAC_ADDR_SIZE);
		qdf_copy_macaddr(&pMsg->peerMacAddr,
				 &pSession->connectedProfile.bssid);

		cds_message.type = WMA_DHCP_STOP_IND;
		cds_message.bodyptr = pMsg;
		cds_message.reserved = 0;

		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post DHCP Stop MSG fail", __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*---------------------------------------------------------------------------

    \fn sme_TXFailMonitorStopInd

    \brief API to signal the FW to start monitoring TX failures

    \return QDF_STATUS  SUCCESS.

			FAILURE or RESOURCES  The API finished and failed.
   --------------------------------------------------------------------------*/
QDF_STATUS sme_tx_fail_monitor_start_stop_ind(tHalHandle hHal, uint8_t tx_fail_count,
					      void *txFailIndCallback)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tAniTXFailMonitorInd *pMsg;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pMsg = (tAniTXFailMonitorInd *)
		       qdf_mem_malloc(sizeof(tAniTXFailMonitorInd));
		if (NULL == pMsg) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Failed to allocate memory", __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		pMsg->msgType = WMA_TX_FAIL_MONITOR_IND;
		pMsg->msgLen = (uint16_t) sizeof(tAniTXFailMonitorInd);

		/* tx_fail_count = 0 should disable the Monitoring in FW */
		pMsg->tx_fail_count = tx_fail_count;
		pMsg->txFailIndCallback = txFailIndCallback;

		cds_message.type = WMA_TX_FAIL_MONITOR_IND;
		cds_message.bodyptr = pMsg;
		cds_message.reserved = 0;

		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post TX Fail monitor Start MSG fail",
				  __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_cfg_privacy
    \brief  API to set configure privacy parameters
    \param  hHal - The handle returned by mac_open.
    \param  pProfile - Pointer CSR Roam profile.
    \param  fPrivacy - This parameter indicates status of privacy

    \return void
   ---------------------------------------------------------------------------*/
void sme_set_cfg_privacy(tHalHandle hHal,
			 tCsrRoamProfile *pProfile, bool fPrivacy)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_CFGPRIVACY, NO_SESSION, 0));
	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		csr_set_cfg_privacy(pMac, pProfile, fPrivacy);
		sme_release_global_lock(&pMac->sme);
	}
}

/* ---------------------------------------------------------------------------
    \fn sme_neighbor_report_request
    \brief  API to request neighbor report.
    \param  hHal - The handle returned by mac_open.
    \param  pRrmNeighborReq - Pointer to a caller allocated object of type
			      tRrmNeighborReq. Caller owns the memory and is
			      responsible for freeing it.
    \return QDF_STATUS
	    QDF_STATUS_E_FAILURE - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_neighbor_report_request(tHalHandle hHal, uint8_t sessionId,
				       tpRrmNeighborReq pRrmNeighborReq,
				       tpRrmNeighborRspCallbackInfo callbackInfo)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_NEIGHBOR_REPORTREQ, NO_SESSION,
			 0));

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		status =
			sme_rrm_neighbor_report_request(hHal, sessionId,
							pRrmNeighborReq, callbackInfo);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

void sms_log(tpAniSirGlobal pMac, uint32_t loglevel, const char *pString, ...)
{
#ifdef WLAN_DEBUG
	/* Verify against current log level */
	if (loglevel >
	    pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE(SIR_SMS_MODULE_ID)])
		return;
	else {
		va_list marker;

		va_start(marker, pString);      /* Initialize variable arguments. */

		log_debug(pMac, SIR_SMS_MODULE_ID, loglevel, pString, marker);

		va_end(marker); /* Reset variable arguments.      */
	}
#endif
}

/* ---------------------------------------------------------------------------
    \fn sme_get_wcnss_wlan_compiled_version
    \brief  This API returns the version of the WCNSS WLAN API with
	which the HOST driver was built
    \param  hHal - The handle returned by mac_open.
    \param  pVersion - Points to the Version structure to be filled
    \return QDF_STATUS
	    QDF_STATUS_E_INVAL - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_wcnss_wlan_compiled_version(tHalHandle hHal,
					       tSirVersionType *pVersion)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		if (pVersion != NULL)
			status = QDF_STATUS_SUCCESS;
		else
			status = QDF_STATUS_E_INVAL;

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_wcnss_wlan_reported_version
    \brief  This API returns the version of the WCNSS WLAN API with
	which the WCNSS driver reports it was built
    \param  hHal - The handle returned by mac_open.
    \param  pVersion - Points to the Version structure to be filled
    \return QDF_STATUS
	    QDF_STATUS_E_INVAL - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_wcnss_wlan_reported_version(tHalHandle hHal,
					       tSirVersionType *pVersion)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		if (pVersion != NULL)
			status = QDF_STATUS_SUCCESS;
		else
			status = QDF_STATUS_E_INVAL;

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_wcnss_software_version
    \brief  This API returns the version string of the WCNSS driver
    \param  hHal - The handle returned by mac_open.
    \param  pVersion - Points to the Version string buffer to be filled
    \param  versionBufferSize - THe size of the Version string buffer
    \return QDF_STATUS
	    QDF_STATUS_E_INVAL - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_wcnss_software_version(tHalHandle hHal,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	v_CONTEXT_t cds_context = cds_get_global_context();

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		if (pVersion != NULL) {
			status =
				wma_get_wcnss_software_version(cds_context,
							    pVersion,
							    versionBufferSize);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_wcnss_hardware_version
    \brief  This API returns the version string of the WCNSS hardware
    \param  hHal - The handle returned by mac_open.
    \param  pVersion - Points to the Version string buffer to be filled
    \param  versionBufferSize - THe size of the Version string buffer
    \return QDF_STATUS
	    QDF_STATUS_E_INVAL - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_get_wcnss_hardware_version(tHalHandle hHal,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		if (pVersion != NULL)
			status = QDF_STATUS_SUCCESS;
		else
			status = QDF_STATUS_E_INVAL;

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

#ifdef FEATURE_WLAN_WAPI

/* ---------------------------------------------------------------------------
    \fn sme_scan_get_bkid_candidate_list
    \brief a wrapper function to return the BKID candidate list
    \param pBkidList - caller allocated buffer point to an array of
		       tBkidCandidateInfo
    \param pNumItems - pointer to a variable that has the number of
		       tBkidCandidateInfo allocated when retruning, this is
		       either the number needed or number of items put into
		       pPmkidList
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not
			 big enough and pNumItems
			 has the number of tBkidCandidateInfo.
    \Note: pNumItems is a number of tBkidCandidateInfo,
	   not sizeof(tBkidCandidateInfo) * something
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_scan_get_bkid_candidate_list(tHalHandle hHal, uint32_t sessionId,
					    tBkidCandidateInfo *pBkidList,
					    uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status =
			csr_scan_get_bkid_candidate_list(pMac, sessionId, pBkidList,
							 pNumItems);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif /* FEATURE_WLAN_WAPI */

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * sme_oem_data_req() - send oem data request to WMA
 * @hal: HAL handle
 * @hdd_oem_req: OEM data request from HDD
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_data_req(tHalHandle hal, struct oem_data_req *hdd_oem_req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct oem_data_req *oem_data_req;
	void *wma_handle;

	sms_log(mac_ctx, LOG1, FL("enter"));
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sms_log(mac_ctx, LOGE, FL("wma_handle is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	oem_data_req = qdf_mem_malloc(sizeof(*oem_data_req));
	if (!oem_data_req) {
		sms_log(mac_ctx, LOGE, FL("mem alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	oem_data_req->data_len = hdd_oem_req->data_len;
	oem_data_req->data = qdf_mem_malloc(oem_data_req->data_len);
	if (!oem_data_req->data) {
		sms_log(mac_ctx, LOGE, FL("mem alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(oem_data_req->data, hdd_oem_req->data,
		     oem_data_req->data_len);

	status = wma_start_oem_data_req(wma_handle, oem_data_req);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("Post oem data request msg fail"));
	} else {
		sms_log(mac_ctx, LOG1,
			FL("OEM request(length: %d) sent to WMA"),
			oem_data_req->data_len);
	}

	if (oem_data_req->data_len)
		qdf_mem_free(oem_data_req->data);
	qdf_mem_free(oem_data_req);

	sms_log(mac_ctx, LOG1, FL("exit"));
	return status;
}
#endif /*FEATURE_OEM_DATA_SUPPORT */

/*--------------------------------------------------------------------------

   \brief sme_open_session() - Open a session for scan/roam operation.

   This is a synchronous API.

   \param hHal - The handle returned by mac_open.
   \param callback - A pointer to the function caller specifies for
		     roam/connect status indication
   \param pContext - The context passed with callback
   \param pSelfMacAddr - Caller allocated memory filled with self MAC address
			 (6 bytes)
   \param pbSessionId - pointer to a caller allocated buffer for returned session ID

   \return QDF_STATUS_SUCCESS - session is opened. sessionId returned.

		Other status means SME is failed to open the session.
		QDF_STATUS_E_RESOURCES - no more session available.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_open_session(tHalHandle hHal, csr_roam_completeCallback callback,
			    void *pContext,
			    uint8_t *pSelfMacAddr, uint8_t *pbSessionId,
			    uint32_t type, uint32_t subType)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: type=%d, subType=%d addr:%pM",
		  __func__, type, subType, pSelfMacAddr);

	if (NULL == pbSessionId) {
		status = QDF_STATUS_E_INVAL;
	} else {
		status = sme_acquire_global_lock(&pMac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = csr_roam_open_session(pMac, callback, pContext,
						       pSelfMacAddr,
						       pbSessionId, type,
						       subType);

			sme_release_global_lock(&pMac->sme);
		}
	}
	if (NULL != pbSessionId)
		MTRACE(qdf_trace(QDF_MODULE_ID_SME,
				 TRACE_CODE_SME_RX_HDD_OPEN_SESSION,
				 *pbSessionId, 0));

	return status;
}

/*--------------------------------------------------------------------------

   \brief sme_close_session() - Open a session for scan/roam operation.

   This is a synchronous API.

   \param hHal - The handle returned by mac_open.

   \param sessionId - A previous opened session's ID.

   \return QDF_STATUS_SUCCESS - session is closed.

	Other status means SME is failed to open the session.
	QDF_STATUS_E_INVAL - session is not opened.
   \sa

   --------------------------------------------------------------------------*/
QDF_STATUS sme_close_session(tHalHandle hHal, uint8_t sessionId,
			     csr_roamSessionCloseCallback callback,
			     void *pContext)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CLOSE_SESSION, sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_close_session(pMac, sessionId, false,
						callback, pContext);

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_roam_update_apwpsie

    \brief To update AP's WPS IE. This function should be called after SME AP session is created
    This is an asynchronous API.

    \param pAPWPSIES - pointer to a caller allocated object of tSirAPWPSIEs

    \return QDF_STATUS  SUCCESS 

			FAILURE or RESOURCES  The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_update_apwpsie(tHalHandle hHal, uint8_t sessionId,
				   tSirAPWPSIEs *pAPWPSIES)
{

	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		status = csr_roam_update_apwpsie(pMac, sessionId, pAPWPSIES);

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_roam_update_apwparsni_es

    \brief To update AP's WPA/RSN IEs. This function should be called after SME AP session is created
    This is an asynchronous API.

    \param pAPSirRSNie - pointer to a caller allocated object of tSirRSNie with WPS/RSN IEs

    \return QDF_STATUS  SUCCESS 

			FAILURE or RESOURCES  The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_roam_update_apwparsni_es(tHalHandle hHal, uint8_t sessionId,
					tSirRSNie *pAPSirRSNie)
{

	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		status = csr_roam_update_wparsni_es(pMac, sessionId, pAPSirRSNie);

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_change_mcc_beacon_interval

    \brief To update P2P-GO beaconInterval. This function should be called after
    disassociating all the station is done
    This is an asynchronous API.

    \param

    \return QDF_STATUS  SUCCESS
			FAILURE or RESOURCES
			The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_change_mcc_beacon_interval(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOG1, FL("Update Beacon PARAMS "));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_send_chng_mcc_beacon_interval(pMac, sessionId);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_set_host_offload(): API to set the host offload feature.
 * @hHal: The handle returned by mac_open.
 * @sessionId: Session Identifier
 * @request: Pointer to the offload request.
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_host_offload(tHalHandle hHal, uint8_t sessionId,
				tpSirHostOffloadReq request)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_HOSTOFFLOAD, sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
#ifdef WLAN_NS_OFFLOAD
		if (SIR_IPV6_NS_OFFLOAD == request->offloadType) {
			status = sme_set_ps_ns_offload(hHal, request,
					sessionId);
		} else
#endif /* WLAN_NS_OFFLOAD */
		{
			status = sme_set_ps_host_offload(hHal, request,
					sessionId);
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/**
 * sme_set_gtk_offload(): API to set GTK offload information.
 * @hHal: The handle returned by mac_open.
 * @sessionId: Session Identifier
 * @pGtkOffload: Pointer to the GTK offload request..
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_gtk_offload(tHalHandle hHal,
		tpSirGtkOffloadParams pGtkOffload,
			       uint8_t sessionId)
{
	tpSirGtkOffloadParams request_buf;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: KeyReplayCounter: %lld", __func__,
		  pGtkOffload->ullKeyReplayCounter);

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Session not found ", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  FL("Not able to allocate memory for GTK offload request"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&pGtkOffload->bssid,
			 &pSession->connectedProfile.bssid);

	*request_buf = *pGtkOffload;

	msg.type = WMA_GTK_OFFLOAD_REQ;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (!QDF_IS_STATUS_SUCCESS
		    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post SIR_HAL_SET_GTK_OFFLOAD message to HAL"));
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_get_gtk_offload(): API to get GTK offload information
 * @hHal: The handle returned by mac_open.
 * @callback_routine: callback_routine.
 * @sessionId: Session Identifier.
 * callback_context: callback_context.
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_gtk_offload(tHalHandle hHal,
			       gtk_offload_get_info_callback callback_routine,
			       void *callback_context, uint8_t session_id)
{
	tpSirGtkOffloadGetInfoRspParams request_buf;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, session_id);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "%s: Entered",
		  __func__);

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Session not found", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			 FL("Not able to allocate memory for Get GTK offload request"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&request_buf->bssid,
			 &pSession->connectedProfile.bssid);

	msg.type = WMA_GTK_OFFLOAD_GETINFO_REQ;
	msg.reserved = 0;
	msg.bodyptr = request_buf;

	/* Cache the Get GTK Offload callback information */
	if (NULL != pMac->sme.gtk_offload_get_info_cb) {

		/* Do we need to check if the callback is in use? */
		/* Because we are not sending the same message again
		 * when it is pending,
		 * the only case when the callback is not NULL is that
		 * the previous message was timed out or failed.
		 * So, it will be safe to set the callback in this case.
		 */
	}

	pMac->sme.gtk_offload_get_info_cb = callback_routine;
	pMac->sme.gtk_offload_get_info_cb_context = callback_context;

	if (!QDF_IS_STATUS_SUCCESS
		    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post WMA_GTK_OFFLOAD_GETINFO_REQ message to WMA"));
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_GTK_OFFLOAD */

/* ---------------------------------------------------------------------------
    \fn sme_set_keep_alive
    \brief  API to set the Keep Alive feature.
    \param  hHal - The handle returned by mac_open.
    \param  request -  Pointer to the Keep Alive request.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_keep_alive(tHalHandle hHal, uint8_t session_id,
			      tpSirKeepAliveReq request)
{
	tpSirKeepAliveReq request_buf;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, session_id);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_LOW,
			FL("WMA_SET_KEEP_ALIVE message"));

	if (pSession == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Session not Found"));
		return QDF_STATUS_E_FAILURE;
	}
	request_buf = qdf_mem_malloc(sizeof(tSirKeepAliveReq));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to allocate memory for keep alive request"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&request->bssid, &pSession->connectedProfile.bssid);
	qdf_mem_copy(request_buf, request, sizeof(tSirKeepAliveReq));

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_LOW,
			"buff TP %d input TP %d ", request_buf->timePeriod,
		  request->timePeriod);
	request_buf->sessionId = session_id;

	msg.type = WMA_SET_KEEP_ALIVE;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (QDF_STATUS_SUCCESS !=
			cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Not able to post WMA_SET_KEEP_ALIVE message to WMA"));
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_SCAN_PNO
/* ---------------------------------------------------------------------------
    \fn sme_set_preferred_network_list
    \brief  API to set the Preferred Network List Offload feature.
    \param  hHal - The handle returned by mac_open.
    \param  request -  Pointer to the offload request.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_preferred_network_list(tHalHandle hHal,
		tpSirPNOScanReq request,
		uint8_t sessionId,
		void (*callback_routine)(void *callback_context,
			tSirPrefNetworkFoundInd
			*pPrefNetworkFoundInd),
		void *callback_context)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_PREF_NET_LIST,
			 sessionId, request->ucNetworksCount));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sme_set_ps_preferred_network_list(hHal, request, sessionId,
				callback_routine, callback_context);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

#endif /* FEATURE_WLAN_SCAN_PNO */

/* ---------------------------------------------------------------------------
    \fn sme_abort_mac_scan
    \brief  API to cancel MAC scan.
    \param  hHal - The handle returned by mac_open.
    \param  sessionId - sessionId on which we need to abort scan.
    \param  reason - Reason to abort the scan.
    \return QDF_STATUS
	    QDF_STATUS_E_FAILURE - failure
	    QDF_STATUS_SUCCESS  success
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_abort_mac_scan(tHalHandle hHal, uint8_t sessionId,
			      eCsrAbortReason reason)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ABORT_MACSCAN, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_abort_mac_scan(pMac, sessionId, reason);

		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ----------------------------------------------------------------------------
    \fn sme_get_operation_channel
    \brief API to get current channel on which STA is parked
    this function gives channel information only of infra station or IBSS station
    \param hHal, pointer to memory location and sessionId
    \returns QDF_STATUS_SUCCESS
	     QDF_STATUS_E_FAILURE
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_get_operation_channel(tHalHandle hHal, uint32_t *pChannel,
				     uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession;

	if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if ((pSession->connectedProfile.BSSType ==
		     eCSR_BSS_TYPE_INFRASTRUCTURE)
		    || (pSession->connectedProfile.BSSType ==
			eCSR_BSS_TYPE_IBSS)
		    || (pSession->connectedProfile.BSSType ==
			eCSR_BSS_TYPE_INFRA_AP)
		    || (pSession->connectedProfile.BSSType ==
			eCSR_BSS_TYPE_START_IBSS)) {
			*pChannel = pSession->connectedProfile.operationChannel;
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_FAILURE;
} /* sme_get_operation_channel ends here */

/**
 * sme_register_mgmt_frame_ind_callback() - Register a callback for
 * management frame indication to PE.
 *
 * @hal: hal pointer
 * @callback: callback pointer to be registered
 *
 * This function is used to register a callback for management
 * frame indication to PE.
 *
 * Return: Success if msg is posted to PE else Failure.
 */
QDF_STATUS sme_register_mgmt_frame_ind_callback(tHalHandle hal,
				sir_mgmt_frame_ind_callback callback)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct sir_sme_mgmt_frame_cb_req *msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	sms_log(mac_ctx, LOG1, FL(": ENTER"));

	if (QDF_STATUS_SUCCESS ==
			sme_acquire_global_lock(&mac_ctx->sme)) {
		msg = qdf_mem_malloc(sizeof(*msg));
		if (NULL == msg) {
			sms_log(mac_ctx, LOGE,
				FL("Not able to allocate memory for eWNI_SME_REGISTER_MGMT_FRAME_CB"));
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_set(msg, sizeof(*msg), 0);
		msg->message_type = eWNI_SME_REGISTER_MGMT_FRAME_CB;
		msg->length          = sizeof(*msg);

		msg->callback = callback;
		status = cds_send_mb_message_to_mac(msg);
		sme_release_global_lock(&mac_ctx->sme);
		return status;
	}
	return QDF_STATUS_E_FAILURE;
}

/* ---------------------------------------------------------------------------

    \fn sme_RegisterMgtFrame

    \brief To register managment frame of specified type and subtype.
    \param frameType - type of the frame that needs to be passed to HDD.
    \param matchData - data which needs to be matched before passing frame
		       to HDD.
    \param matchDataLen - Length of matched data.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_register_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
				   uint16_t frameType, uint8_t *matchData,
				   uint16_t matchLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_REGISTER_MGMTFR, sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tSirRegisterMgmtFrame *pMsg;
		uint16_t len;
		tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sms_log(pMac, LOGE, FL("  session %d not found "),
				sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession->sessionActive) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s Invalid Sessionid", __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		len = sizeof(tSirRegisterMgmtFrame) + matchLen;

		pMsg = qdf_mem_malloc(len);
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else {
			qdf_mem_set(pMsg, len, 0);
			pMsg->messageType = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
			pMsg->length = len;
			pMsg->sessionId = sessionId;
			pMsg->registerFrame = true;
			pMsg->frameType = frameType;
			pMsg->matchLen = matchLen;
			qdf_mem_copy(pMsg->matchData, matchData, matchLen);
			status = cds_send_mb_message_to_mac(pMsg);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_DeregisterMgtFrame

    \brief To De-register managment frame of specified type and subtype.
    \param frameType - type of the frame that needs to be passed to HDD.
    \param matchData - data which needs to be matched before passing frame
		       to HDD.
    \param matchDataLen - Length of matched data.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS sme_deregister_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
				     uint16_t frameType, uint8_t *matchData,
				     uint16_t matchLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_DEREGISTER_MGMTFR, sessionId,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tSirRegisterMgmtFrame *pMsg;
		uint16_t len;
		tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sms_log(pMac, LOGE, FL("  session %d not found "),
				sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession->sessionActive) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s Invalid Sessionid", __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		len = sizeof(tSirRegisterMgmtFrame) + matchLen;

		pMsg = qdf_mem_malloc(len);
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else {
			qdf_mem_set(pMsg, len, 0);
			pMsg->messageType = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
			pMsg->length = len;
			pMsg->registerFrame = false;
			pMsg->frameType = frameType;
			pMsg->matchLen = matchLen;
			qdf_mem_copy(pMsg->matchData, matchData, matchLen);
			status = cds_send_mb_message_to_mac(pMsg);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_remain_on_channel - API to request remain on channel for 'x' duration
 *
 * @hHal: pointer to MAC handle
 * @session_id: Session identifier
 * @channel: channel information
 * @duration: duration in ms
 * @callback: HDD registered callback to process reaminOnChannelRsp
 * @context: HDD Callback param
 * @scan_id: scan identifier
 *
 * This function process the roc request and generates scan identifier.s
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_remain_on_channel(tHalHandle hHal, uint8_t session_id,
				 uint8_t channel, uint32_t duration,
				 remainOnChanCallback callback,
				 void *pContext, uint8_t isP2PProbeReqAllowed,
				 uint32_t *scan_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hHal);
	uint32_t san_req_id, scan_count;
	struct ani_roc_req *roc_msg;
	cds_msg_t msg;


	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_REMAIN_ONCHAN, session_id, 0));

	scan_count = csr_ll_count(&mac_ctx->sme.smeScanCmdActiveList);
	if (scan_count >= mac_ctx->scan.max_scan_count) {
		sms_log(mac_ctx, LOGE, FL("Max scan reached"));
		return QDF_STATUS_E_FAILURE;
	}

	wma_get_scan_id(&san_req_id);
	*scan_id = san_req_id;
	status = sme_acquire_global_lock(&mac_ctx->sme);

	sms_log(mac_ctx, LOG1, FL(" called"));
	roc_msg = qdf_mem_malloc(sizeof(struct ani_roc_req));
	if (NULL == roc_msg) {
		sms_log(mac_ctx, LOGE,
			" scan_req: failed to allocate mem for msg");
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_NOMEM;
	}
	roc_msg->msg_type = eWNI_SME_ROC_CMD;
	roc_msg->msg_len = (uint16_t) sizeof(struct ani_roc_req);
	roc_msg->session_id = session_id;
	roc_msg->callback = callback;
	roc_msg->duration = duration;
	roc_msg->channel = channel;
	roc_msg->is_p2pprobe_allowed = isP2PProbeReqAllowed;
	roc_msg->ctx = pContext;
	roc_msg->scan_id = *scan_id;
	msg.type = eWNI_SME_ROC_CMD;
	msg.bodyptr = roc_msg;
	msg.reserved = 0;
	msg.bodyval = 0;
	if (QDF_STATUS_SUCCESS !=
		cds_mq_post_message(CDS_MQ_ID_SME, &msg)) {
		sms_log(mac_ctx, LOGE,
			" sme_scan_req failed to post msg");
		qdf_mem_free(roc_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac_ctx->sme);
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_report_probe_req
    \brief  API to enable/disable forwarding of probeReq to apps in p2p.
    \param  hHal - The handle returned by mac_open.
    \param  falg: to set the Probe request forarding to wpa_supplicant in listen state in p2p
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/

#ifndef WLAN_FEATURE_CONCURRENT_P2P
QDF_STATUS sme_report_probe_req(tHalHandle hHal, uint8_t flag)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	do {
		/* acquire the lock for the sme object */
		status = sme_acquire_global_lock(&pMac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* call set in context */
			pMac->p2pContext.probeReqForwarding = flag;
			/* release the lock for the sme object */
			sme_release_global_lock(&pMac->sme);
		}
	} while (0);

	sms_log(pMac, LOGW, "exiting function %s", __func__);

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_p2p_ie
    \brief  API to set the P2p Ie in p2p context
    \param  hHal - The handle returned by mac_open.
    \param  p2pIe -  Ptr to p2pIe from HDD.
    \param p2pIeLength: length of p2pIe
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/

QDF_STATUS sme_update_p2p_ie(tHalHandle hHal, void *p2pIe, uint32_t p2pIeLength)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_P2P_IE, NO_SESSION, 0));
	/* acquire the lock for the sme object */
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (NULL != pMac->p2pContext.probeRspIe) {
			qdf_mem_free(pMac->p2pContext.probeRspIe);
			pMac->p2pContext.probeRspIeLength = 0;
		}

		pMac->p2pContext.probeRspIe = qdf_mem_malloc(p2pIeLength);
		if (NULL == pMac->p2pContext.probeRspIe) {
			sms_log(pMac, LOGE, "%s: Unable to allocate P2P IE",
				__func__);
			pMac->p2pContext.probeRspIeLength = 0;
			status = QDF_STATUS_E_NOMEM;
		} else {
			pMac->p2pContext.probeRspIeLength = p2pIeLength;

			sir_dump_buf(pMac, SIR_LIM_MODULE_ID, LOG2,
				     pMac->p2pContext.probeRspIe,
				     pMac->p2pContext.probeRspIeLength);
			qdf_mem_copy((uint8_t *) pMac->p2pContext.probeRspIe,
				     p2pIe, p2pIeLength);
		}

		/* release the lock for the sme object */
		sme_release_global_lock(&pMac->sme);
	}

	sms_log(pMac, LOG2, "exiting function %s", __func__);

	return status;
}
#endif

/* ---------------------------------------------------------------------------
    \fn sme_send_action
    \brief  API to send action frame from supplicant.
    \param  hHal - The handle returned by mac_open.
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/

QDF_STATUS sme_send_action(tHalHandle hHal, uint8_t sessionId,
			   const uint8_t *pBuf, uint32_t len,
			   uint16_t wait, bool noack,
			   uint16_t channel_freq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SEND_ACTION, sessionId, 0));
	/* acquire the lock for the sme object */
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		p2p_send_action(hHal, sessionId, pBuf, len, wait, noack,
		channel_freq);
		/* release the lock for the sme object */
		sme_release_global_lock(&pMac->sme);
	}

	sms_log(pMac, LOGW, "exiting function %s", __func__);

	return status;
}

QDF_STATUS sme_cancel_remain_on_channel(tHalHandle hHal,
	uint8_t sessionId, uint32_t scan_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CANCEL_REMAIN_ONCHAN, sessionId,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = p2p_cancel_remain_on_channel(hHal, sessionId, scan_id);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* Power Save Related */
QDF_STATUS sme_p2p_set_ps(tHalHandle hHal, tP2pPsConfig *data)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = p2p_set_ps(hHal, data);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------

   \fn    sme_configure_rxp_filter

   \brief
    SME will pass this request to lower mac to set/reset the filter on RXP for
    multicast & broadcast traffic.

   \param

    hHal - The handle returned by mac_open.

    filterMask- Currently the API takes a 1 or 0 (set or reset) as filter.
    Basically to enable/disable the filter (to filter "all" mcbc traffic) based
    on this param. In future we can use this as a mask to set various types of
    filters as suggested below:
    FILTER_ALL_MULTICAST:
    FILTER_ALL_BROADCAST:
    FILTER_ALL_MULTICAST_BROADCAST:

   \return QDF_STATUS

   --------------------------------------------------------------------------- */
QDF_STATUS sme_configure_rxp_filter(tHalHandle hHal,
				    tpSirWlanSetRxpFilters wlanRxpFilterParam)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_RXPFIL, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		cds_message.bodyptr = wlanRxpFilterParam;
		cds_message.type = WMA_CFG_RXP_FILTER_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------

   \fn    sme_configure_suspend_ind

   \brief
    SME will pass this request to lower mac to Indicate that the wlan needs to
    be suspended

   \param

    hHal - The handle returned by mac_open.

    wlanSuspendParam- Depicts the wlan suspend params

    csr_readyToSuspendCallback - Callback to be called when ready to suspend
				 event is received.
	callback_context  - Context associated with csr_readyToSuspendCallback.

   \return QDF_STATUS

   --------------------------------------------------------------------------- */
QDF_STATUS sme_configure_suspend_ind(tHalHandle hHal,
				     uint32_t conn_state_mask,
				     csr_readyToSuspendCallback callback,
				     void *callback_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_SUSPENDIND, NO_SESSION,
			 0));

	pMac->readyToSuspendCallback = callback;
	pMac->readyToSuspendContext = callback_context;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		cds_message.bodyval = conn_state_mask;
		cds_message.type = WMA_WLAN_SUSPEND_IND;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			pMac->readyToSuspendCallback = NULL;
			pMac->readyToSuspendContext = NULL;
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

   \fn    sme_configure_resume_req

   \brief
    SME will pass this request to lower mac to Indicate that the wlan needs to
    be Resumed

   \param

    hHal - The handle returned by mac_open.

    wlanResumeParam- Depicts the wlan resume params

   \return QDF_STATUS

   --------------------------------------------------------------------------- */
QDF_STATUS sme_configure_resume_req(tHalHandle hHal,
				    tpSirWlanResumeParam wlanResumeParam)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_RESUMEREQ, NO_SESSION,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		cds_message.bodyptr = wlanResumeParam;
		cds_message.type = WMA_WLAN_RESUME_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/**
 * sme_configure_ext_wow() - configure Extr WoW
 * @hHal - The handle returned by mac_open.
 * @wlanExtParams - Depicts the wlan Ext params.
 * @callback - ext_wow callback to be registered.
 * @callback_context - ext_wow callback context
 *
 * SME will pass this request to lower mac to configure Extr WoW
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_ext_wow(tHalHandle hHal,
				  tpSirExtWoWParams wlanExtParams,
				  csr_readyToExtWoWCallback callback,
				  void *callback_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tpSirExtWoWParams MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_EXTWOW, NO_SESSION, 0));

	pMac->readyToExtWoWCallback = callback;
	pMac->readyToExtWoWContext = callback_context;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanExtParams, sizeof(*MsgPtr));
		cds_message.bodyptr = MsgPtr;
		cds_message.type = WMA_WLAN_EXT_WOW;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			pMac->readyToExtWoWCallback = NULL;
			pMac->readyToExtWoWContext = NULL;
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		pMac->readyToExtWoWCallback = NULL;
		pMac->readyToExtWoWContext = NULL;
		qdf_mem_free(MsgPtr);
	}

	return status;
}

/* ---------------------------------------------------------------------------

   \fn    sme_configure_app_type1_params

   \brief
   SME will pass this request to lower mac to configure Indoor WoW parameters.

   \param

    hHal - The handle returned by mac_open.

    wlanAppType1Params- Depicts the wlan App Type 1(Indoor) params

   \return QDF_STATUS

   --------------------------------------------------------------------------- */
QDF_STATUS sme_configure_app_type1_params(tHalHandle hHal,
					  tpSirAppType1Params wlanAppType1Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tpSirAppType1Params MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_APP_TYPE1, NO_SESSION,
			 0));

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanAppType1Params, sizeof(*MsgPtr));
		cds_message.bodyptr = MsgPtr;
		cds_message.type = WMA_WLAN_SET_APP_TYPE1_PARAMS;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		qdf_mem_free(MsgPtr);
	}

	return status;
}

/* ---------------------------------------------------------------------------

   \fn    sme_configure_app_type2_params

   \brief
   SME will pass this request to lower mac to configure Indoor WoW parameters.

   \param

    hHal - The handle returned by mac_open.

    wlanAppType2Params- Depicts the wlan App Type 2 (Outdoor) params

   \return QDF_STATUS

   --------------------------------------------------------------------------- */
QDF_STATUS sme_configure_app_type2_params(tHalHandle hHal,
					  tpSirAppType2Params wlanAppType2Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tpSirAppType2Params MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_APP_TYPE2, NO_SESSION,
			 0));

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanAppType2Params, sizeof(*MsgPtr));
		cds_message.bodyptr = MsgPtr;
		cds_message.type = WMA_WLAN_SET_APP_TYPE2_PARAMS;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		qdf_mem_free(MsgPtr);
	}

	return status;
}
#endif

/* ---------------------------------------------------------------------------

    \fn sme_get_infra_session_id

    \brief To get the session ID for infra session, if connected
    This is a synchronous API.

    \param hHal - The handle returned by mac_open.

    \return sessionid, -1 if infra session is not connected

   -------------------------------------------------------------------------------*/
int8_t sme_get_infra_session_id(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int8_t sessionid = -1;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		sessionid = csr_get_infra_session_id(pMac);

		sme_release_global_lock(&pMac->sme);
	}

	return sessionid;
}

/* ---------------------------------------------------------------------------

    \fn sme_get_infra_operation_channel

    \brief To get the operating channel for infra session, if connected
    This is a synchronous API.

    \param hHal - The handle returned by mac_open.
    \param sessionId - the sessionId returned by sme_open_session.

    \return operating channel, 0 if infra session is not connected

   -------------------------------------------------------------------------------*/
uint8_t sme_get_infra_operation_channel(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t channel = 0;
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		channel = csr_get_infra_operation_channel(pMac, sessionId);

		sme_release_global_lock(&pMac->sme);
	}

	return channel;
}

/* This routine will return poerating channel on which other BSS is operating to be used for concurrency mode. */
/* If other BSS is not up or not connected it will return 0 */
uint8_t sme_get_concurrent_operation_channel(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t channel = 0;
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		channel = csr_get_concurrent_operation_channel(pMac);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_HIGH, "%s: "
			  " Other Concurrent Channel = %d", __func__, channel);
		sme_release_global_lock(&pMac->sme);
	}

	return channel;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t sme_check_concurrent_channel_overlap(tHalHandle hHal, uint16_t sap_ch,
					      eCsrPhyMode sapPhyMode,
					      uint8_t cc_switch_mode)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint16_t channel = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		channel =
			csr_check_concurrent_channel_overlap(pMac, sap_ch, sapPhyMode,
							     cc_switch_mode);
		sme_release_global_lock(&pMac->sme);
	}

	return channel;
}
#endif

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * sme_update_roam_pno_channel_prediction_config() - Update PNO config
 * @csr_config:      config from SME context
 * @hal:             Global Hal handle
 * @copy_from_to:    Used to specify the source and destination
 *
 * Copy the PNO channel prediction configuration parameters from
 * SME context to MAC context or vice-versa
 *
 * Return: None
 */
void sme_update_roam_pno_channel_prediction_config(
		tHalHandle hal, tCsrConfigParam *csr_config,
		uint8_t copy_from_to)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	if (copy_from_to == SME_CONFIG_TO_ROAM_CONFIG) {
		mac_ctx->roam.configParam.pno_channel_prediction =
			csr_config->pno_channel_prediction;
		mac_ctx->roam.configParam.top_k_num_of_channels =
			csr_config->top_k_num_of_channels;
		mac_ctx->roam.configParam.stationary_thresh =
			csr_config->stationary_thresh;
		mac_ctx->roam.configParam.channel_prediction_full_scan =
			csr_config->channel_prediction_full_scan;
		mac_ctx->roam.configParam.pnoscan_adaptive_dwell_mode =
			csr_config->pnoscan_adaptive_dwell_mode;
	} else if (copy_from_to == ROAM_CONFIG_TO_SME_CONFIG) {
		csr_config->pno_channel_prediction =
			mac_ctx->roam.configParam.pno_channel_prediction;
		csr_config->top_k_num_of_channels =
			mac_ctx->roam.configParam.top_k_num_of_channels;
		csr_config->stationary_thresh =
			mac_ctx->roam.configParam.stationary_thresh;
		csr_config->channel_prediction_full_scan =
			mac_ctx->roam.configParam.channel_prediction_full_scan;
		csr_config->pnoscan_adaptive_dwell_mode =
			mac_ctx->roam.configParam.pnoscan_adaptive_dwell_mode;
	}

}
/******************************************************************************
*
* Name: sme_preferred_network_found_ind
*
* Description:
*    Invoke Preferred Network Found Indication
*
* Parameters:
*    hHal - HAL handle for device
*    pMsg - found network description
*
* Returns: QDF_STATUS
*
******************************************************************************/
QDF_STATUS sme_preferred_network_found_ind(tHalHandle hHal, void *pMsg)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirPrefNetworkFoundInd *pPrefNetworkFoundInd =
		(tSirPrefNetworkFoundInd *) pMsg;
	uint8_t dumpSsId[SIR_MAC_MAX_SSID_LENGTH + 1];
	uint8_t ssIdLength = 0;

	if (NULL == pMsg) {
		sms_log(pMac, LOGE, "in %s msg ptr is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (pMac->pnoOffload) {
		/* Call Preferred Network Found Indication callback routine. */
		if (pMac->sme.pref_netw_found_cb != NULL) {
			pMac->sme.pref_netw_found_cb(pMac->sme.
					preferred_network_found_ind_cb_ctx,
					pPrefNetworkFoundInd);
		}
		return status;
	}

	if (pPrefNetworkFoundInd->ssId.length > 0) {
		ssIdLength = CSR_MIN(SIR_MAC_MAX_SSID_LENGTH,
				     pPrefNetworkFoundInd->ssId.length);
		qdf_mem_copy(dumpSsId, pPrefNetworkFoundInd->ssId.ssId,
			     ssIdLength);
		dumpSsId[ssIdLength] = 0;
		sms_log(pMac, LOG2, "%s:SSID=%s frame length %d",
			__func__, dumpSsId, pPrefNetworkFoundInd->frameLength);

		/* Flush scan results, So as to avoid indication/updation of
		 * stale entries, which may not have aged out during APPS collapse
		 */
		sme_scan_flush_result(hHal);

		/* Save the frame to scan result */
		if (pPrefNetworkFoundInd->mesgLen >
		    sizeof(tSirPrefNetworkFoundInd)) {
			/* we may have a frame */
			status = csr_scan_save_preferred_network_found(pMac,
					pPrefNetworkFoundInd);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(pMac, LOGE,
					FL(" fail to save preferred network"));
			}
		} else {
			sms_log(pMac, LOGE,
				FL(" not enough data length %d needed %zu"),
				pPrefNetworkFoundInd->mesgLen,
				sizeof(tSirPrefNetworkFoundInd));
		}

		/* Call Preferred Netowrk Found Indication callback routine. */
		if (QDF_IS_STATUS_SUCCESS(status)
		    && (pMac->sme.pref_netw_found_cb != NULL)) {
			pMac->sme.pref_netw_found_cb(pMac->sme.
					preferred_network_found_ind_cb_ctx,
					pPrefNetworkFoundInd);
		}
	} else {
		sms_log(pMac, LOGE, "%s: callback failed - SSID is NULL",
			__func__);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

#endif /* FEATURE_WLAN_SCAN_PNO */

/**
 * sme_set_tsfcb() - Set callback for TSF capture
 * @h_hal: Handler return by mac_open
 * @cb_fn: Callback function pointer
 * @db_ctx: Callback data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_tsfcb(tHalHandle h_hal,
	int (*cb_fn)(void *cb_ctx, struct stsf *ptsf), void *cb_ctx)
{
	tpAniSirGlobal mac = PMAC_STRUCT(h_hal);
	QDF_STATUS status;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.get_tsf_cb = cb_fn;
		mac->sme.get_tsf_cxt = cb_ctx;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_reset_tsfcb() - Reset callback for TSF capture
 * @h_hal: Handler return by mac_open
 *
 * This function reset the tsf capture callback to SME
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_reset_tsfcb(tHalHandle h_hal)
{
	tpAniSirGlobal mac;
	QDF_STATUS status;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("h_hal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.get_tsf_cb = NULL;
		mac->sme.get_tsf_cxt = NULL;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_TSF
/*
 * sme_set_tsf_gpio() - set gpio pin that be toggled when capture tef
 * @h_hal: Handler return by mac_open
 * @pinvalue: gpio pin id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_tsf_gpio(tHalHandle h_hal, uint32_t pinvalue)
{
	QDF_STATUS status;
	cds_msg_t tsf_msg = {0};
	tpAniSirGlobal mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tsf_msg.type = WMA_TSF_GPIO_PIN;
		tsf_msg.reserved = 0;
		tsf_msg.bodyval = pinvalue;

		status = cds_mq_post_message(CDS_MQ_ID_WMA, &tsf_msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sms_log(mac, LOGE, "Unable to post WMA_TSF_GPIO_PIN");
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}
#endif

QDF_STATUS sme_get_cfg_valid_channels(tHalHandle hHal, uint8_t *aValidChannels,
				      uint32_t *len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_cfg_valid_channels(pMac, aValidChannels, len);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_handle_change_country_code

    \brief Change Country code, Reg Domain and channel list

    \details Country Code Priority
    If Supplicant country code is priority than 11d is disabled.
    If 11D is enabled, we update the country code after every scan.
    Hence when Supplicant country code is priority, we don't need 11D info.
    Country code from Supplicant is set as current courtry code.
    User can send reset command XX (instead of country code) to reset the
    country code to default values.
    If 11D is priority,
    Than Supplicant country code code is set to default code. But 11D code is set as current country code

    \param pMac - The handle returned by mac_open.
    \param pMsgBuf - MSG Buffer

    \return QDF_STATUS

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_handle_change_country_code(tpAniSirGlobal pMac, void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tAniChangeCountryCodeReq *pMsg;
	v_REGDOMAIN_t domainIdIoctl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	static uint8_t default_country[CDS_COUNTRY_CODE_LEN + 1];
	pMsg = (tAniChangeCountryCodeReq *) pMsgBuf;

	/*
	 * if the reset Supplicant country code command is triggered,
	 * enable 11D, reset the country code and return
	 */
	if (true !=
	    qdf_mem_cmp(pMsg->countryCode, SME_INVALID_COUNTRY_CODE, 2)) {
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;

		qdf_status = cds_read_default_country(default_country);

		/* read the country code and use it */
		if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_copy(pMsg->countryCode,
				     default_country,
				     WNI_CFG_COUNTRY_CODE_LEN);
		} else {
			status = QDF_STATUS_E_FAILURE;
			return status;
		}
		/*
		 * Update the 11d country to default country so that when
		 * callback is received for this default country, driver will
		 * not disable the 11d taking it as valid country by user.
		 */
		sms_log(pMac, LOG1,
			FL("Set default country code (%c%c) as invalid country received"),
			pMsg->countryCode[0], pMsg->countryCode[1]);
			qdf_mem_copy(pMac->scan.countryCode11d,
			pMsg->countryCode,
			WNI_CFG_COUNTRY_CODE_LEN);
	} else {
		/* if Supplicant country code has priority, disable 11d */
		if (pMac->roam.configParam.fSupplicantCountryCodeHasPriority &&
		    pMsg->countryFromUserSpace) {
			pMac->roam.configParam.Is11dSupportEnabled = false;
		}
	}

	if (pMac->roam.configParam.Is11dSupportEnabled)
		return QDF_STATUS_SUCCESS;

	/* Set Current Country code and Current Regulatory domain */
	status = csr_set_country_code(pMac, pMsg->countryCode);
	if (QDF_STATUS_SUCCESS != status) {
		/* Supplicant country code failed. So give 11D priority */
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;
		sms_log(pMac, LOGE, "Set Country Code Fail %d", status);
		return status;
	}

	/* overwrite the defualt country code */
	qdf_mem_copy(pMac->scan.countryCodeDefault,
		     pMac->scan.countryCodeCurrent, WNI_CFG_COUNTRY_CODE_LEN);

	/* Get Domain ID from country code */
	status = csr_get_regulatory_domain_for_country(pMac,
						       pMac->scan.countryCodeCurrent,
						       (v_REGDOMAIN_t *) &
						       domainIdIoctl,
						       SOURCE_QUERY);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE, FL("  fail to get regId %d"), domainIdIoctl);
		return status;
	} else if (REGDOMAIN_WORLD == domainIdIoctl) {
		/* Supplicant country code is invalid, so we are on world mode now. So
		   give 11D chance to update */
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;
		sms_log(pMac, LOG1, FL("Country Code unrecognized by driver"));
	}

	status = wma_set_reg_domain(pMac, domainIdIoctl);

	if (status != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE, FL("  fail to set regId %d"), domainIdIoctl);
		return status;
	} else {
		/* if 11d has priority, clear currentCountryBssid & countryCode11d to get */
		/* set again if we find AP with 11d info during scan */
		if (!pMac->roam.configParam.fSupplicantCountryCodeHasPriority) {
			sms_log(pMac, LOGW,
				FL
					("Clearing currentCountryBssid, countryCode11d"));
			qdf_mem_zero(&pMac->scan.currentCountryBssid,
				     sizeof(struct qdf_mac_addr));
			qdf_mem_zero(pMac->scan.countryCode11d,
				     sizeof(pMac->scan.countryCode11d));
		}
	}

	if (pMsg->changeCCCallback) {
		((tSmeChangeCountryCallback) (pMsg->changeCCCallback))((void *)
								       pMsg->
								       pDevContext);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_handle_generic_change_country_code() - handles country ch req
 * @mac_ctx:    mac global context
 * @msg:        request msg packet
 *
 * If Supplicant country code is priority than 11d is disabled.
 * If 11D is enabled, we update the country code after every scan.
 * Hence when Supplicant country code is priority, we don't need 11D info.
 * Country code from Supplicant is set as current country code.
 *
 * Return: status of operation
 */
QDF_STATUS
sme_handle_generic_change_country_code(tpAniSirGlobal mac_ctx,
				       void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	v_REGDOMAIN_t reg_domain_id = 0;
	bool is_11d_country = false;
	bool supplicant_priority =
		mac_ctx->roam.configParam.fSupplicantCountryCodeHasPriority;
	tAniGenericChangeCountryCodeReq *msg = pMsgBuf;

	sms_log(mac_ctx, LOG1, FL(" called"));

	if (memcmp(msg->countryCode, mac_ctx->scan.countryCode11d,
		   CDS_COUNTRY_CODE_LEN) == 0) {
		is_11d_country = true;
	}
	/* Set the country code given by userspace when 11dOriginal is false
	 * when 11doriginal is True,is_11d_country =0 and
	 * fSupplicantCountryCodeHasPriority = 0, then revert the country code,
	 * and return failure
	 */
	if (mac_ctx->roam.configParam.Is11dSupportEnabledOriginal == true
	    && !is_11d_country && !supplicant_priority) {
		sms_log(mac_ctx, LOGW,
			FL("Incorrect country req, nullify this"));

		/* we have got a request for a country that should not have been
		 * added since the STA is associated; nullify this request. If
		 * both countryCode11d[0] and countryCode11d[1] are zero, revert
		 * it to World domain to avoid from causing cfg80211 call trace.
		 */
		if ((mac_ctx->scan.countryCode11d[0] == 0)
		    && (mac_ctx->scan.countryCode11d[1] == 0))
			status = csr_get_regulatory_domain_for_country(mac_ctx,
					"00", (v_REGDOMAIN_t *) &reg_domain_id,
					SOURCE_11D);
		else
			status = csr_get_regulatory_domain_for_country(mac_ctx,
					mac_ctx->scan.countryCode11d,
					(v_REGDOMAIN_t *) &reg_domain_id,
					SOURCE_11D);

		return QDF_STATUS_E_FAILURE;
	}

	/* if Supplicant country code has priority, disable 11d */
	if (!is_11d_country && supplicant_priority)
		mac_ctx->roam.configParam.Is11dSupportEnabled = false;
	qdf_mem_copy(mac_ctx->scan.countryCodeCurrent, msg->countryCode,
		     WNI_CFG_COUNTRY_CODE_LEN);
	status = wma_set_reg_domain(mac_ctx, reg_domain_id);
	if (false == is_11d_country) {
		/* overwrite the defualt country code */
		qdf_mem_copy(mac_ctx->scan.countryCodeDefault,
			     mac_ctx->scan.countryCodeCurrent,
			     WNI_CFG_COUNTRY_CODE_LEN);
		/* set to default domain ID */
		mac_ctx->scan.domainIdDefault = mac_ctx->scan.domainIdCurrent;
	}
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac_ctx, LOGE, FL("fail to set regId %d"),
			reg_domain_id);
		return status;
	} else {
		/* if 11d has priority, clear currentCountryBssid &
		 * countryCode11d to get set again if we find AP with 11d info
		 * during scan
		 */
		if (!supplicant_priority && (false == is_11d_country)) {
			sms_log(mac_ctx, LOGW,
				FL("Clearing currentCountryBssid, countryCode11d"));
			qdf_mem_zero(&mac_ctx->scan.currentCountryBssid,
				     sizeof(struct qdf_mac_addr));
			qdf_mem_zero(mac_ctx->scan.countryCode11d,
				     sizeof(mac_ctx->scan.countryCode11d));
		}
	}
	/* get the channels based on new cc */
	status = csr_get_channel_and_power_list(mac_ctx);

	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac_ctx, LOGE, FL("fail to get Channels"));
		return status;
	}
	/* reset info based on new cc, and we are done */
	csr_apply_channel_power_info_wrapper(mac_ctx);

	/* Country code  Changed, Purge Only scan result
	 * which does not have channel number belong to 11d
	 * channel list
	 */
	csr_scan_filter_results(mac_ctx);
	/* Do active scans after the country is set by User hints or
	 * Country IE
	 */
	mac_ctx->scan.curScanType = eSIR_ACTIVE_SCAN;
	sme_disconnect_connected_sessions(mac_ctx);
	sms_log(mac_ctx, LOG1, FL(" returned"));
	return QDF_STATUS_SUCCESS;
}

static bool
sme_search_in_base_ch_lst(tpAniSirGlobal mac_ctx, uint8_t curr_ch)
{
	uint8_t i;
	tCsrChannel *ch_lst_info;
	ch_lst_info = &mac_ctx->scan.base_channels;
	for (i = 0; i < ch_lst_info->numChannels; i++) {
		if (ch_lst_info->channelList[i] == curr_ch)
			return true;
	}

	return false;
}
/**
 * sme_disconnect_connected_sessions() - Disconnect STA and P2P client session
 * if channel is not supported
 * @mac_ctx:          mac global context
 *
 * If new country code does not support the channel on which STA/P2P client
 * is connetced, it sends the disconnect to the AP/P2P GO
 *
 * Return: void
 */
void sme_disconnect_connected_sessions(tpAniSirGlobal mac_ctx)
{
	uint8_t session_id, found = false;
	uint8_t curr_ch;

	for (session_id = 0; session_id < CSR_ROAM_SESSION_MAX; session_id++) {
		if (!csr_is_session_client_and_connected(mac_ctx, session_id))
			continue;
		found = false;
		/* Session is connected.Check the channel */
		curr_ch = csr_get_infra_operation_channel(mac_ctx,
							  session_id);
		sms_log(mac_ctx, LOGW,
			FL("Current Operating channel : %d, session :%d"),
			curr_ch, session_id);
		found = sme_search_in_base_ch_lst(mac_ctx, curr_ch);
		if (!found) {
			sms_log(mac_ctx, LOGW, FL("Disconnect Session :%d"),
				session_id);
			csr_roam_disconnect(mac_ctx, session_id,
					    eCSR_DISCONNECT_REASON_UNSPECIFIED);
		}
	}
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
QDF_STATUS sme_8023_multicast_list(tHalHandle hHal, uint8_t sessionId,
				   tpSirRcvFltMcAddrList pMulticastAddrs)
{
	tpSirRcvFltMcAddrList request_buf;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = NULL;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "%s: "
		  "ulMulticastAddrCnt=%d, multicastAddr[0]=%p", __func__,
		  pMulticastAddrs->ulMulticastAddrCnt,
		  pMulticastAddrs->multicastAddr[0].bytes);

	/* Find the connected Infra / P2P_client connected session */
	if (CSR_IS_SESSION_VALID(pMac, sessionId) &&
			(csr_is_conn_state_infra(pMac, sessionId) ||
			csr_is_ndi_started(pMac, sessionId))) {
		pSession = CSR_GET_SESSION(pMac, sessionId);
	}

	if (pSession == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
			  "%s: Unable to find the session Id: %d", __func__,
			  sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(tSirRcvFltMcAddrList));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to "
			  "allocate memory for 8023 Multicast List request",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (!csr_is_conn_state_connected_infra(pMac, sessionId) &&
			!csr_is_ndi_started(pMac, sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Request ignored, session %d is not connected or started",
			__func__, sessionId);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(request_buf, pMulticastAddrs,
		     sizeof(tSirRcvFltMcAddrList));

	qdf_copy_macaddr(&request_buf->self_macaddr, &pSession->selfMacAddr);
	qdf_copy_macaddr(&request_buf->bssid,
			 &pSession->connectedProfile.bssid);

	msg.type = WMA_8023_MULTICAST_LIST_REQ;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to "
			  "post WMA_8023_MULTICAST_LIST message to WMA",
			  __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_receive_filter_set_filter(tHalHandle hHal,
					 tpSirRcvPktFilterCfgType pRcvPktFilterCfg,
					 uint8_t sessionId)
{
	tpSirRcvPktFilterCfgType request_buf;
	int32_t allocSize;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	uint8_t idx = 0;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "%s: filterType=%d, "
		  "filterId = %d", __func__,
		  pRcvPktFilterCfg->filterType, pRcvPktFilterCfg->filterId);

	allocSize = sizeof(tSirRcvPktFilterCfgType);

	request_buf = qdf_mem_malloc(allocSize);

	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to "
			  "allocate memory for Receive Filter Set Filter request",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Session Not found ", __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_copy_macaddr(&pRcvPktFilterCfg->self_macaddr,
			 &pSession->selfMacAddr);
	qdf_copy_macaddr(&pRcvPktFilterCfg->bssid,
			 &pSession->connectedProfile.bssid);
	qdf_mem_copy(request_buf, pRcvPktFilterCfg, allocSize);

	msg.type = WMA_RECEIVE_FILTER_SET_FILTER_REQ;
	msg.reserved = 0;
	msg.bodyptr = request_buf;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "Pkt Flt Req : "
		  "FT %d FID %d ",
		  request_buf->filterType, request_buf->filterId);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "Pkt Flt Req : "
		  "params %d CT %d",
		  request_buf->numFieldParams, request_buf->coalesceTime);

	for (idx = 0; idx < request_buf->numFieldParams; idx++) {

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "Proto %d Comp Flag %d ",
			  request_buf->paramsData[idx].protocolLayer,
			  request_buf->paramsData[idx].cmpFlag);

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "Data Offset %d Data Len %d",
			  request_buf->paramsData[idx].dataOffset,
			  request_buf->paramsData[idx].dataLength);

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "CData: %d:%d:%d:%d:%d:%d",
			  request_buf->paramsData[idx].compareData[0],
			  request_buf->paramsData[idx].compareData[1],
			  request_buf->paramsData[idx].compareData[2],
			  request_buf->paramsData[idx].compareData[3],
			  request_buf->paramsData[idx].compareData[4],
			  request_buf->paramsData[idx].compareData[5]);

		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "MData: %d:%d:%d:%d:%d:%d",
			  request_buf->paramsData[idx].dataMask[0],
			  request_buf->paramsData[idx].dataMask[1],
			  request_buf->paramsData[idx].dataMask[2],
			  request_buf->paramsData[idx].dataMask[3],
			  request_buf->paramsData[idx].dataMask[4],
			  request_buf->paramsData[idx].dataMask[5]);

	}

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post "
			  "WMA_RECEIVE_FILTER_SET_FILTER message to WMA",
			  __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_receive_filter_clear_filter(tHalHandle hHal,
					   tpSirRcvFltPktClearParam
					   pRcvFltPktClearParam, uint8_t sessionId)
{
	tpSirRcvFltPktClearParam request_buf;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "%s: filterId = %d",
		  __func__, pRcvFltPktClearParam->filterId);

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Session Not found", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(tSirRcvFltPktClearParam));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for Receive Filter "
			  "Clear Filter request", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&pRcvFltPktClearParam->self_macaddr,
			 &pSession->selfMacAddr);
	qdf_copy_macaddr(&pRcvFltPktClearParam->bssid,
			 &pSession->connectedProfile.bssid);

	qdf_mem_copy(request_buf, pRcvFltPktClearParam,
		     sizeof(tSirRcvFltPktClearParam));

	msg.type = WMA_RECEIVE_FILTER_CLEAR_FILTER_REQ;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post "
			  "WMA_RECEIVE_FILTER_CLEAR_FILTER message to WMA",
			  __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_PACKET_FILTERING */

/* ---------------------------------------------------------------------------

    \fn sme_is_channel_valid

    \brief To check if the channel is valid for currently established domain
    This is a synchronous API.

    \param hHal - The handle returned by mac_open.
    \param channel - channel to verify

    \return true/false, true if channel is valid

   -------------------------------------------------------------------------------*/
bool sme_is_channel_valid(tHalHandle hHal, uint8_t channel)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	bool valid = false;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		valid = csr_roam_is_channel_valid(pMac, channel);

		sme_release_global_lock(&pMac->sme);
	}

	return valid;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_freq_band
    \brief  Used to set frequency band.
    \param  hHal
    \param  sessionId - Session Identifier
    \eBand  band value to be configured
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_freq_band(tHalHandle hHal, uint8_t sessionId, eCsrBand eBand)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_set_band(hHal, sessionId, eBand);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_freq_band
    \brief  Used to get the current band settings.
    \param  hHal
    \pBand  pointer to hold band value
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_get_freq_band(tHalHandle hHal, eCsrBand *pBand)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		*pBand = csr_get_current_band(hHal);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_max_tx_power_per_band

    \brief Set the Maximum Transmit Power specific to band dynamically.
    Note: this setting will not persist over reboots.

    \param band
    \param power to set in dB
  \- return QDF_STATUS

   ----------------------------------------------------------------------------*/
QDF_STATUS sme_set_max_tx_power_per_band(eCsrBand band, int8_t dB)
{
	cds_msg_t msg;
	tpMaxTxPowerPerBandParams pMaxTxPowerPerBandParams = NULL;

	pMaxTxPowerPerBandParams =
		qdf_mem_malloc(sizeof(tMaxTxPowerPerBandParams));
	if (NULL == pMaxTxPowerPerBandParams) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s:Not able to allocate memory for pMaxTxPowerPerBandParams",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	pMaxTxPowerPerBandParams->power = dB;
	pMaxTxPowerPerBandParams->bandInfo = band;

	msg.type = WMA_SET_MAX_TX_POWER_PER_BAND_REQ;
	msg.reserved = 0;
	msg.bodyptr = pMaxTxPowerPerBandParams;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s:Not able to post WMA_SET_MAX_TX_POWER_PER_BAND_REQ",
			  __func__);
		qdf_mem_free(pMaxTxPowerPerBandParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_max_tx_power

    \brief Set the Maximum Transmit Power dynamically. Note: this setting will
    not persist over reboots.

    \param  hHal
    \param pBssid  BSSID to set the power cap for
    \param pBssid  pSelfMacAddress self MAC Address
    \param pBssid  power to set in dB
   \- return QDF_STATUS

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_set_max_tx_power(tHalHandle hHal, struct qdf_mac_addr pBssid,
				struct qdf_mac_addr pSelfMacAddress, int8_t dB)
{
	cds_msg_t msg;
	tpMaxTxPowerParams pMaxTxParams = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_MAXTXPOW, NO_SESSION, 0));
	pMaxTxParams = qdf_mem_malloc(sizeof(tMaxTxPowerParams));
	if (NULL == pMaxTxParams) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for pMaxTxParams",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&pMaxTxParams->bssId, &pBssid);
	qdf_copy_macaddr(&pMaxTxParams->selfStaMacAddr, &pSelfMacAddress);
	pMaxTxParams->power = dB;

	msg.type = WMA_SET_MAX_TX_POWER_REQ;
	msg.reserved = 0;
	msg.bodyptr = pMaxTxParams;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_SET_MAX_TX_POWER_REQ message to WMA",
			  __func__);
		qdf_mem_free(pMaxTxParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_custom_mac_addr

    \brief Set the customer Mac Address.

    \param customMacAddr  customer MAC Address
   \- return QDF_STATUS

   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_custom_mac_addr(tSirMacAddr customMacAddr)
{
	cds_msg_t msg;
	tSirMacAddr *pBaseMacAddr;

	pBaseMacAddr = qdf_mem_malloc(sizeof(tSirMacAddr));
	if (NULL == pBaseMacAddr) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to allocate memory for pBaseMacAddr"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(*pBaseMacAddr, customMacAddr, sizeof(tSirMacAddr));

	msg.type = SIR_HAL_SET_BASE_MACADDR_IND;
	msg.reserved = 0;
	msg.bodyptr = pBaseMacAddr;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL
				  ("Not able to post SIR_HAL_SET_BASE_MACADDR_IND message to WMA"));
		qdf_mem_free(pBaseMacAddr);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ----------------------------------------------------------------------------
   \fn sme_set_tx_power
   \brief Set Transmit Power dynamically.
   \param  hHal
   \param sessionId  Target Session ID
   \pBSSId BSSID
   \dev_mode dev_mode such as station, P2PGO, SAP
   \param dBm  power to set
   \- return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_tx_power(tHalHandle hHal, uint8_t sessionId,
			   struct qdf_mac_addr pBSSId,
			   enum tQDF_ADAPTER_MODE dev_mode, int dBm)
{
	cds_msg_t msg;
	tpMaxTxPowerParams pTxParams = NULL;
	int8_t power = (int8_t) dBm;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_TXPOW, sessionId, 0));

	/* make sure there is no overflow */
	if ((int)power != dBm) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: error, invalid power = %d", __func__, dBm);
		return QDF_STATUS_E_FAILURE;
	}

	pTxParams = qdf_mem_malloc(sizeof(tMaxTxPowerParams));
	if (NULL == pTxParams) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for pTxParams",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&pTxParams->bssId, &pBSSId);
	pTxParams->power = power;       /* unit is dBm */
	pTxParams->dev_mode = dev_mode;
	msg.type = WMA_SET_TX_POWER_REQ;
	msg.reserved = 0;
	msg.bodyptr = pTxParams;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to post WMA_SET_TX_POWER_REQ to WMA",
			  __func__);
		qdf_mem_free(pTxParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------

    \fn sme_hide_ssid

    \brief hide/show SSID dynamically. Note: this setting will
    not persist over reboots.

    \param hHal
    \param sessionId
    \param ssidHidden 0 - Broadcast SSID, 1 - Disable broadcast SSID
   \- return QDF_STATUS

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_hide_ssid(tHalHandle hHal, uint8_t sessionId, uint8_t ssidHidden)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint16_t len;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tpSirUpdateParams pMsg;
		tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sms_log(pMac, LOGE, FL("  session %d not found "),
				sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!pSession->sessionActive)
			QDF_ASSERT(0);

		/* Create the message and send to lim */
		len = sizeof(tSirUpdateParams);
		pMsg = qdf_mem_malloc(len);
		if (NULL == pMsg)
			status = QDF_STATUS_E_NOMEM;
		else {
			qdf_mem_set(pMsg, sizeof(tSirUpdateParams), 0);
			pMsg->messageType = eWNI_SME_HIDE_SSID_REQ;
			pMsg->length = len;
			/* Data starts from here */
			pMsg->sessionId = sessionId;
			pMsg->ssidHidden = ssidHidden;
			status = cds_send_mb_message_to_mac(pMsg);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_tm_level
    \brief  Set Thermal Mitigation Level to RIVA
    \param  hHal - The handle returned by mac_open.
    \param  newTMLevel - new Thermal Mitigation Level
    \param  tmMode - Thermal Mitigation handle mode, default 0
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_tm_level(tHalHandle hHal, uint16_t newTMLevel, uint16_t tmMode)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tAniSetTmLevelReq *setTmLevelReq = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_TMLEVEL, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		setTmLevelReq =
			(tAniSetTmLevelReq *)
			qdf_mem_malloc(sizeof(tAniSetTmLevelReq));
		if (NULL == setTmLevelReq) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to allocate memory for sme_set_tm_level",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		setTmLevelReq->tmMode = tmMode;
		setTmLevelReq->newTmLevel = newTMLevel;

		/* serialize the req through MC thread */
		cds_message.bodyptr = setTmLevelReq;
		cds_message.type = WMA_SET_TM_LEVEL_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Set TM Level MSG fail", __func__);
			qdf_mem_free(setTmLevelReq);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*---------------------------------------------------------------------------

   \brief sme_feature_caps_exchange() - SME interface to exchange capabilities between
   Host and FW.

   \param  hHal - HAL handle for device

   \return NONE

   ---------------------------------------------------------------------------*/
void sme_feature_caps_exchange(tHalHandle hHal)
{
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_HDD_CAPS_EXCH, NO_SESSION,
		       0));
}

/*---------------------------------------------------------------------------

   \brief sme_disable_feature_capablity() - SME interface to disable Active mode offload capablity
   in Host.

   \param  hHal - HAL handle for device

   \return NONE

   ---------------------------------------------------------------------------*/
void sme_disable_feature_capablity(uint8_t feature_index)
{
}

/* ---------------------------------------------------------------------------
    \fn     sme_reset_power_values_for5_g
    \brief  Reset the power values for 5G band with default power values.
    \param  hHal - HAL handle for device
   \- return NONE
    -------------------------------------------------------------------------*/
void sme_reset_power_values_for5_g(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_RESET_PW5G, NO_SESSION, 0));
	csr_save_channel_power_for_band(pMac, true);
	csr_apply_power2_current(pMac);    /* Store the channel+power info in the global place: Cfg */
}

/* ---------------------------------------------------------------------------
    \fn sme_update_roam_prefer5_g_hz
    \brief  enable/disable Roam prefer 5G runtime option
	    This function is called through dynamic setConfig callback function
	    to configure the Roam prefer 5G runtime option
    \param  hHal - HAL handle for device
    \param  nRoamPrefer5GHz Enable/Disable Roam prefer 5G runtime option
   \- return Success or failure
    -------------------------------------------------------------------------*/

QDF_STATUS sme_update_roam_prefer5_g_hz(tHalHandle hHal, bool nRoamPrefer5GHz)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_RP5G, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: gRoamPrefer5GHz is changed from %d to %d",
			  __func__, pMac->roam.configParam.nRoamPrefer5GHz,
			  nRoamPrefer5GHz);
		pMac->roam.configParam.nRoamPrefer5GHz = nRoamPrefer5GHz;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_roam_intra_band
    \brief  enable/disable Intra band roaming
	    This function is called through dynamic setConfig callback function
	    to configure the intra band roaming
    \param  hHal - HAL handle for device
    \param  nRoamIntraBand Enable/Disable Intra band roaming
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_intra_band(tHalHandle hHal, const bool nRoamIntraBand)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_ROAMIBAND, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: gRoamIntraBand is changed from %d to %d",
			  __func__, pMac->roam.configParam.nRoamIntraBand,
			  nRoamIntraBand);
		pMac->roam.configParam.nRoamIntraBand = nRoamIntraBand;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_roam_scan_n_probes
    \brief  function to update roam scan N probes
	    This function is called through dynamic setConfig callback function
	    to update roam scan N probes
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nProbes number of probe requests to be sent out
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_update_roam_scan_n_probes(tHalHandle hHal, uint8_t sessionId,
					 const uint8_t nProbes)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_ROAM_SCAN_N_PROBES,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: gRoamScanNProbes is changed from %d to %d",
			  __func__, pMac->roam.configParam.nProbes, nProbes);
		pMac->roam.configParam.nProbes = nProbes;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_NPROBES_CHANGED);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_roam_scan_home_away_time
    \brief  function to update roam scan Home away time
	    This function is called through dynamic setConfig callback function
	    to update roam scan home away time
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nRoamScanAwayTime Scan home away time
    \param  bSendOffloadCmd If true then send offload command to firmware
			    If false then command is not sent to firmware
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_update_roam_scan_home_away_time(tHalHandle hHal,
					       uint8_t sessionId,
					       const uint16_t nRoamScanHomeAwayTime,
					       const bool bSendOffloadCmd)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_ROAM_SCAN_HOME_AWAY_TIME,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: gRoamScanHomeAwayTime is changed from %d to %d",
			  __func__,
			  pMac->roam.configParam.nRoamScanHomeAwayTime,
			  nRoamScanHomeAwayTime);
		pMac->roam.configParam.nRoamScanHomeAwayTime =
			nRoamScanHomeAwayTime;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled && bSendOffloadCmd) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_HOME_AWAY_TIME_CHANGED);
	}
	return status;
}

/**
 * sme_ext_change_channel()- function to post send ECSA
 * action frame to csr.
 * @hHal: Hal context
 * @channel: new channel to switch
 * @session_id: senssion it should be sent on.
 *
 * This function is called to post ECSA frame to csr.
 *
 * Return: success if msg is sent else return failure
 */
QDF_STATUS sme_ext_change_channel(tHalHandle h_hal, uint32_t channel,
						uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx  = PMAC_STRUCT(h_hal);
	uint8_t channel_state;

	sms_log(mac_ctx, LOGE, FL(" Set Channel %d "), channel);
	channel_state =
		cds_get_channel_state(channel);

	if (CHANNEL_STATE_DISABLE == channel_state) {
		sms_log(mac_ctx, LOGE, FL(" Invalid channel %d "), channel);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);

	if (QDF_STATUS_SUCCESS == status) {
		/* update the channel list to the firmware */
		status = csr_send_ext_change_channel(mac_ctx,
						channel, session_id);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_roam_intra_band
    \brief  get Intra band roaming
    \param  hHal - HAL handle for device
   \- return Success or failure
    -------------------------------------------------------------------------*/
bool sme_get_roam_intra_band(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_ROAMIBAND, NO_SESSION, 0));
	return pMac->roam.configParam.nRoamIntraBand;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_roam_scan_n_probes
    \brief  get N Probes
    \param  hHal - HAL handle for device
   \- return Success or failure
    -------------------------------------------------------------------------*/
uint8_t sme_get_roam_scan_n_probes(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.nProbes;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_roam_scan_home_away_time
    \brief  get Roam scan home away time
    \param  hHal - HAL handle for device
   \- return Success or failure
    -------------------------------------------------------------------------*/
uint16_t sme_get_roam_scan_home_away_time(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.nRoamScanHomeAwayTime;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_roam_rssi_diff
    \brief  Update RoamRssiDiff
	    This function is called through dynamic setConfig callback function
	    to configure RoamRssiDiff
	    Usage: adb shell iwpriv wlan0 setConfig RoamRssiDiff=[0 .. 125]
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  RoamRssiDiff - minimum rssi difference between potential
	    candidate and current AP.
   \- return Success or failure
    -------------------------------------------------------------------------*/

QDF_STATUS sme_update_roam_rssi_diff(tHalHandle hHal, uint8_t sessionId,
				     uint8_t RoamRssiDiff)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam rssi diff to %d - old value is %d - roam state is %s",
			  RoamRssiDiff,
			  pMac->roam.configParam.RoamRssiDiff,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pMac->roam.configParam.RoamRssiDiff = RoamRssiDiff;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_RSSI_DIFF_CHANGED);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_fast_transition_enabled() - enable/disable Fast Transition
	support at runtime
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   isFastTransitionEnabled.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update isFastTransitionEnabled config
	successfully.
	Other status means SME is failed to update isFastTransitionEnabled.
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_update_fast_transition_enabled(tHalHandle hHal,
					      bool isFastTransitionEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_FTENABLED, NO_SESSION,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: FastTransitionEnabled is changed from %d to %d",
			  __func__,
			  pMac->roam.configParam.isFastTransitionEnabled,
			  isFastTransitionEnabled);
		pMac->roam.configParam.isFastTransitionEnabled =
			isFastTransitionEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_wes_mode
    \brief  Update WES Mode
	    This function is called through dynamic setConfig callback function
	    to configure isWESModeEnabled
    \param  hHal - HAL handle for device
    \param  isWESModeEnabled - WES mode
    \param  sessionId - Session Identifier
    \return QDF_STATUS_SUCCESS - SME update isWESModeEnabled config successfully.
	    Other status means SME is failed to update isWESModeEnabled.
    -------------------------------------------------------------------------*/

QDF_STATUS sme_update_wes_mode(tHalHandle hHal, bool isWESModeEnabled,
			       uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set WES Mode to %d - old value is %d - roam state is %s",
			  isWESModeEnabled,
			  pMac->roam.configParam.isWESModeEnabled,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pMac->roam.configParam.isWESModeEnabled = isWESModeEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_roam_scan_control
    \brief  Set roam scan control
	    This function is called to set roam scan control
	    if roam scan control is set to 0, roaming scan cache is cleared
	    any value other than 0 is treated as invalid value
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \return QDF_STATUS_SUCCESS - SME update config successfully.
	    Other status means SME failure to update
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_scan_control(tHalHandle hHal, uint8_t sessionId,
				     bool roamScanControl)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_SCANCTRL, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan control to %d - old value is %d - roam state is %s",
			  roamScanControl,
			  pMac->roam.configParam.nRoamScanControl,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pMac->roam.configParam.nRoamScanControl = roamScanControl;
		if (0 == roamScanControl) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "LFR runtime successfully cleared roam scan cache");
			csr_flush_cfg_bg_scan_roam_channel_list(pMac, sessionId);
			if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
				csr_roam_offload_scan(pMac, sessionId,
						      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
						      REASON_FLUSH_CHANNEL_LIST);
			}
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_is_fast_roam_ini_feature_enabled() - enable/disable LFR
	support at runtime
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   isFastRoamIniFeatureEnabled.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \param  sessionId - Session Identifier
   \return QDF_STATUS_SUCCESS - SME update isFastRoamIniFeatureEnabled config
	successfully.
	Other status means SME is failed to update isFastRoamIniFeatureEnabled.
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_update_is_fast_roam_ini_feature_enabled
	(tHalHandle hHal,
	uint8_t sessionId, const bool isFastRoamIniFeatureEnabled) {
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (pMac->roam.configParam.isFastRoamIniFeatureEnabled ==
	    isFastRoamIniFeatureEnabled) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: FastRoam is already enabled or disabled, nothing to do (returning) old(%d) new(%d)",
			  __func__,
			  pMac->roam.configParam.isFastRoamIniFeatureEnabled,
			  isFastRoamIniFeatureEnabled);
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: FastRoamEnabled is changed from %d to %d", __func__,
		  pMac->roam.configParam.isFastRoamIniFeatureEnabled,
		  isFastRoamIniFeatureEnabled);
	pMac->roam.configParam.isFastRoamIniFeatureEnabled =
		isFastRoamIniFeatureEnabled;
	csr_neighbor_roam_update_fast_roaming_enabled(pMac, sessionId,
						      isFastRoamIniFeatureEnabled);

	return QDF_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
   \brief sme_update_is_mawc_ini_feature_enabled() -
   Enable/disable LFR MAWC support at runtime
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   isMAWCIniFeatureEnabled.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update MAWCEnabled config successfully.
	Other status means SME is failed to update MAWCEnabled.
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_update_is_mawc_ini_feature_enabled(tHalHandle hHal,
						  const bool MAWCEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: MAWCEnabled is changed from %d to %d", __func__,
			  pMac->roam.configParam.MAWCEnabled, MAWCEnabled);
		pMac->roam.configParam.MAWCEnabled = MAWCEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/*--------------------------------------------------------------------------
   \brief sme_stop_roaming() - Stop roaming for a given sessionId
   This is a synchronous call
   \param hHal      - The handle returned by mac_open
   \param  sessionId - Session Identifier
   \return QDF_STATUS_SUCCESS on success
	   Other status on failure
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_stop_roaming(tHalHandle hHal, uint8_t sessionId, uint8_t reason)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(pMac, sessionId, ROAM_SCAN_OFFLOAD_STOP,
				      reason);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_start_roaming() - Start roaming for a given sessionId
   This is a synchronous call
   \param hHal      - The handle returned by mac_open
   \param  sessionId - Session Identifier
   \return QDF_STATUS_SUCCESS on success
	Other status on failure
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_start_roaming(tHalHandle hHal, uint8_t sessionId, uint8_t reason)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(pMac, sessionId, ROAM_SCAN_OFFLOAD_START,
				      reason);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_enable_fast_roam_in_concurrency() - enable/disable LFR if
	Concurrent session exists
   This is a synchronuous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS
	Other status means SME is failed
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_update_enable_fast_roam_in_concurrency(tHalHandle hHal,
						      bool
						      bFastRoamInConIniFeatureEnabled)
{

	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->roam.configParam.bFastRoamInConIniFeatureEnabled =
			bFastRoamInConIniFeatureEnabled;
		if (0 == pMac->roam.configParam.isRoamOffloadScanEnabled) {
			pMac->roam.configParam.bFastRoamInConIniFeatureEnabled =
				0;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_config_fw_rssi_monitoring() - enable/disable firmware RSSI
	Monitoring at runtime
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   fEnableFwRssiMonitoring.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update fEnableFwRssiMonitoring.
	config successfully.
	Other status means SME is failed to update fEnableFwRssiMonitoring.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_update_config_fw_rssi_monitoring(tHalHandle hHal,
						bool fEnableFwRssiMonitoring)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;

	if (sme_cfg_set_int (hHal, WNI_CFG_PS_ENABLE_RSSI_MONITOR,
						fEnableFwRssiMonitoring) == QDF_STATUS_E_FAILURE) {
		qdf_ret_status = QDF_STATUS_E_FAILURE;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Could not pass on WNI_CFG_PS_RSSI_MONITOR to CFG");
	}

	return qdf_ret_status;
}

/* ---------------------------------------------------------------------------
    \fn     sme_set_roam_opportunistic_scan_threshold_diff
    \brief  Update Opportunistic Scan threshold diff
	This function is called through dynamic setConfig callback function
	to configure  nOpportunisticThresholdDiff
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nOpportunisticThresholdDiff - Opportunistic Scan threshold diff
    \return QDF_STATUS_SUCCESS - SME update nOpportunisticThresholdDiff config
	    successfully.
	    else SME is failed to update nOpportunisticThresholdDiff.
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_opportunistic_scan_threshold_diff(tHalHandle hHal,
							  uint8_t sessionId,
							  const uint8_t
							  nOpportunisticThresholdDiff)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac, sessionId,
				nOpportunisticThresholdDiff,
				REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nOpportunisticThresholdDiff =
				nOpportunisticThresholdDiff;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \fn    sme_get_roam_opportunistic_scan_threshold_diff()
   \brief gets Opportunistic Scan threshold diff
	This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return uint8_t - nOpportunisticThresholdDiff
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_roam_opportunistic_scan_threshold_diff(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.
	       nOpportunisticThresholdDiff;
}

/* ---------------------------------------------------------------------------
    \fn     sme_set_roam_rescan_rssi_diff
    \brief  Update roam rescan rssi diff
	    This function is called through dynamic setConfig callback function
	    to configure  nRoamRescanRssiDiff
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nRoamRescanRssiDiff - roam rescan rssi diff
    \return QDF_STATUS_SUCCESS - SME update nRoamRescanRssiDiff config
	    successfully.
	    else SME is failed to update nRoamRescanRssiDiff.
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_rescan_rssi_diff(tHalHandle hHal,
					 uint8_t sessionId,
					 const uint8_t nRoamRescanRssiDiff)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac, sessionId,
				nRoamRescanRssiDiff,
				REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nRoamRescanRssiDiff = nRoamRescanRssiDiff;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \fn    sme_get_roam_rescan_rssi_diff
   \brief gets roam rescan rssi diff
	  This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return int8_t - nRoamRescanRssiDiff
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_roam_rescan_rssi_diff(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.nRoamRescanRssiDiff;
}

/* ---------------------------------------------------------------------------
    \fn     sme_set_roam_bmiss_first_bcnt
    \brief  Update Roam count for first beacon miss
	    This function is called through dynamic setConfig callback function
	    to configure nRoamBmissFirstBcnt
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nRoamBmissFirstBcnt - Roam first bmiss count
    \return QDF_STATUS_SUCCESS - SME update nRoamBmissFirstBcnt
	    successfully.
	    else SME is failed to update nRoamBmissFirstBcnt
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_bmiss_first_bcnt(tHalHandle hHal,
					 uint8_t sessionId,
					 const uint8_t nRoamBmissFirstBcnt)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac, sessionId,
				nRoamBmissFirstBcnt,
				REASON_ROAM_BMISS_FIRST_BCNT_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBmissFirstBcnt = nRoamBmissFirstBcnt;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_roam_bmiss_first_bcnt
    \brief  get neighbor roam beacon miss first count
    \param hHal - The handle returned by mac_open.
    \return uint8_t - neighbor roam beacon miss first count
    -------------------------------------------------------------------------*/
uint8_t sme_get_roam_bmiss_first_bcnt(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.nRoamBmissFirstBcnt;
}

/* ---------------------------------------------------------------------------
    \fn     sme_set_roam_bmiss_final_bcnt
    \brief  Update Roam count for final beacon miss
	    This function is called through dynamic setConfig callback function
	    to configure nRoamBmissFinalBcnt
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nRoamBmissFinalBcnt - Roam final bmiss count
    \return QDF_STATUS_SUCCESS - SME update nRoamBmissFinalBcnt
	    successfully.
	    else SME is failed to update nRoamBmissFinalBcnt
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_bmiss_final_bcnt(tHalHandle hHal,
					 uint8_t sessionId,
					 const uint8_t nRoamBmissFinalBcnt)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac, sessionId,
				nRoamBmissFinalBcnt,
				REASON_ROAM_BMISS_FINAL_BCNT_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBmissFinalBcnt = nRoamBmissFinalBcnt;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \fn    sme_get_roam_bmiss_final_bcnt
   \brief gets Roam count for final beacon miss
	  This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return uint8_t - nRoamBmissFinalBcnt
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_roam_bmiss_final_bcnt(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.nRoamBmissFinalBcnt;
}

/* ---------------------------------------------------------------------------
    \fn     sme_set_roam_beacon_rssi_weight
    \brief  Update Roam beacon rssi weight
	    This function is called through dynamic setConfig callback function
	    to configure nRoamBeaconRssiWeight
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nRoamBeaconRssiWeight - Roam beacon rssi weight
    \return QDF_STATUS_SUCCESS - SME update nRoamBeaconRssiWeight config
	    successfully.
	    else SME is failed to update nRoamBeaconRssiWeight
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_roam_beacon_rssi_weight(tHalHandle hHal,
					   uint8_t sessionId,
					   const uint8_t nRoamBeaconRssiWeight)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac, sessionId,
				nRoamBeaconRssiWeight,
				REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nRoamBeaconRssiWeight = nRoamBeaconRssiWeight;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \fn    sme_get_roam_beacon_rssi_weight
   \brief gets Roam beacon rssi weight
	  This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return uint8_t - nRoamBeaconRssiWeight
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_roam_beacon_rssi_weight(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.nRoamBeaconRssiWeight;
}

/*--------------------------------------------------------------------------
   \brief sme_set_neighbor_lookup_rssi_threshold() - update neighbor lookup
	rssi threshold
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \param  sessionId - Session Identifier
   \return QDF_STATUS_SUCCESS - SME update config successful.
	   Other status means SME is failed to update
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_set_neighbor_lookup_rssi_threshold
	(tHalHandle hHal, uint8_t sessionId, uint8_t neighborLookupRssiThreshold) {
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(pMac,
				sessionId, neighborLookupRssiThreshold,
				REASON_LOOKUP_THRESH_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			pMac->roam.configParam.neighborRoamConfig.
			nNeighborLookupRssiThreshold =
				neighborLookupRssiThreshold;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
  \brief sme_set_delay_before_vdev_stop() - update delay before VDEV_STOP
  This is a synchronous call
  \param  hal - The handle returned by macOpen.
  \param  session_id - Session Identifier
  \param  delay_before_vdev_stop - value to be set
  \return QDF_STATUS_SUCCESS - SME update config successful.
	  Other status means SME is failed to update
  \sa
  --------------------------------------------------------------------------*/
QDF_STATUS sme_set_delay_before_vdev_stop(tHalHandle hal,
					  uint8_t session_id,
					  uint8_t delay_before_vdev_stop)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME,
			QDF_TRACE_LEVEL_DEBUG,
			FL("LFR param delay_before_vdev_stop changed from %d to %d"),
			pMac->roam.configParam.neighborRoamConfig.
			delay_before_vdev_stop,
			delay_before_vdev_stop);
		pMac->roam.neighborRoamInfo[session_id].cfgParams.
			delay_before_vdev_stop = delay_before_vdev_stop;
		pMac->roam.configParam.neighborRoamConfig.
			delay_before_vdev_stop = delay_before_vdev_stop;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_get_neighbor_lookup_rssi_threshold() - get neighbor lookup
	rssi threshold
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update config successful.
	   Other status means SME is failed to update
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_neighbor_lookup_rssi_threshold(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.
	       nNeighborLookupRssiThreshold;
}

/*--------------------------------------------------------------------------
   \brief sme_set_neighbor_scan_refresh_period() - set neighbor scan results
	refresh period
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \param  sessionId - Session Identifier
   \return QDF_STATUS_SUCCESS - SME update config successful.
	   Other status means SME is failed to update
   \sa
   --------------------------------------------------------------------------*/
QDF_STATUS sme_set_neighbor_scan_refresh_period
	(tHalHandle hHal,
	uint8_t sessionId, uint16_t neighborScanResultsRefreshPeriod) {
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamConfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamConfig =
			&pMac->roam.configParam.neighborRoamConfig;
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan refresh period to %d- old value is %d - roam state is %s",
			  neighborScanResultsRefreshPeriod,
			  pMac->roam.configParam.neighborRoamConfig.
			  nNeighborResultsRefreshPeriod,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pNeighborRoamConfig->nNeighborResultsRefreshPeriod =
			neighborScanResultsRefreshPeriod;
		pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod =
			neighborScanResultsRefreshPeriod;

		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_NEIGHBOR_SCAN_REFRESH_PERIOD_CHANGED);
	}
	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_roam_scan_offload_enabled() - enable/disable roam scan
	offload feaure
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   gRoamScanOffloadEnabled.
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update config successfully.
	   Other status means SME is failed to update.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_update_roam_scan_offload_enabled(tHalHandle hHal,
						bool nRoamScanOffloadEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  FL
				  ("gRoamScanOffloadEnabled is changed from %d to %d"),
			  pMac->roam.configParam.isRoamOffloadScanEnabled,
			  nRoamScanOffloadEnabled);
		pMac->roam.configParam.isRoamOffloadScanEnabled =
			nRoamScanOffloadEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_get_neighbor_scan_refresh_period() - get neighbor scan results
	refresh period
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return uint16_t - Neighbor scan results refresh period value
   \sa
   --------------------------------------------------------------------------*/
uint16_t sme_get_neighbor_scan_refresh_period(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.
	       nNeighborResultsRefreshPeriod;
}

/*--------------------------------------------------------------------------
   \brief sme_get_empty_scan_refresh_period() - get empty scan refresh period
   This is a synchronuous call
   \param hHal - The handle returned by mac_open.
   \return QDF_STATUS_SUCCESS - SME update config successful.
	   Other status means SME is failed to update
   \sa
   --------------------------------------------------------------------------*/
uint16_t sme_get_empty_scan_refresh_period(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.neighborRoamConfig.
	       nEmptyScanRefreshPeriod;
}

/* ---------------------------------------------------------------------------
    \fn sme_update_empty_scan_refresh_period
    \brief  Update nEmptyScanRefreshPeriod
	    This function is called through dynamic setConfig callback function
	    to configure nEmptyScanRefreshPeriod
	    Usage: adb shell iwpriv wlan0 setConfig
			nEmptyScanRefreshPeriod=[0 .. 60]
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nEmptyScanRefreshPeriod - scan period following empty scan results.
   \- return Success or failure
    -------------------------------------------------------------------------*/

QDF_STATUS sme_update_empty_scan_refresh_period(tHalHandle hHal, uint8_t sessionId,
						uint16_t nEmptyScanRefreshPeriod)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamConfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamConfig =
			&pMac->roam.configParam.neighborRoamConfig;
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan period to %d -old value is %d - roam state is %s",
			  nEmptyScanRefreshPeriod,
			  pMac->roam.configParam.neighborRoamConfig.
			  nEmptyScanRefreshPeriod,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pNeighborRoamConfig->nEmptyScanRefreshPeriod =
			nEmptyScanRefreshPeriod;
		pNeighborRoamInfo->cfgParams.emptyScanRefreshPeriod =
			nEmptyScanRefreshPeriod;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_EMPTY_SCAN_REF_PERIOD_CHANGED);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_neighbor_scan_min_chan_time
    \brief  Update nNeighborScanMinChanTime
	    This function is called through dynamic setConfig callback function
	    to configure gNeighborScanChannelMinTime
	    Usage: adb shell iwpriv wlan0 setConfig
			gNeighborScanChannelMinTime=[0 .. 60]
    \param  hHal - HAL handle for device
    \param  nNeighborScanMinChanTime - Channel minimum dwell time
    \param  sessionId - Session Identifier
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_neighbor_scan_min_chan_time(tHalHandle hHal,
					       const uint16_t
					       nNeighborScanMinChanTime,
					       uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set channel min dwell time to %d - old value is %d - roam state is %s",
			  nNeighborScanMinChanTime,
			  pMac->roam.configParam.neighborRoamConfig.
			  nNeighborScanMinChanTime,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));

		pMac->roam.configParam.neighborRoamConfig.
		nNeighborScanMinChanTime = nNeighborScanMinChanTime;
		pMac->roam.neighborRoamInfo[sessionId].cfgParams.
		minChannelScanTime = nNeighborScanMinChanTime;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_neighbor_scan_max_chan_time
    \brief  Update nNeighborScanMaxChanTime
	    This function is called through dynamic setConfig callback function
	    to configure gNeighborScanChannelMaxTime
	    Usage: adb shell iwpriv wlan0 setConfig
			gNeighborScanChannelMaxTime=[0 .. 60]
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nNeighborScanMinChanTime - Channel maximum dwell time
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_neighbor_scan_max_chan_time(tHalHandle hHal, uint8_t sessionId,
					       const uint16_t
					       nNeighborScanMaxChanTime)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamConfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamConfig =
			&pMac->roam.configParam.neighborRoamConfig;
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set channel max dwell time to %d - old value is %d - roam state is %s",
			  nNeighborScanMaxChanTime,
			  pMac->roam.configParam.neighborRoamConfig.
			  nNeighborScanMaxChanTime,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pNeighborRoamConfig->nNeighborScanMaxChanTime =
			nNeighborScanMaxChanTime;
		pNeighborRoamInfo->cfgParams.maxChannelScanTime =
			nNeighborScanMaxChanTime;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_SCAN_CH_TIME_CHANGED);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_neighbor_scan_min_chan_time
    \brief  get neighbor scan min channel time
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint16_t - channel min time value
    -------------------------------------------------------------------------*/
uint16_t sme_get_neighbor_scan_min_chan_time(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       minChannelScanTime;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_neighbor_roam_state
    \brief  get neighbor roam state
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint32_t - neighbor roam state
    -------------------------------------------------------------------------*/
uint32_t sme_get_neighbor_roam_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.neighborRoamInfo[sessionId].neighborRoamState;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_current_roam_state
    \brief  get current roam state
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint32_t - current roam state
    -------------------------------------------------------------------------*/
uint32_t sme_get_current_roam_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.curState[sessionId];
}

/* ---------------------------------------------------------------------------
    \fn sme_get_current_roam_sub_state
    \brief  get neighbor roam sub state
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint32_t - current roam sub state
    -------------------------------------------------------------------------*/
uint32_t sme_get_current_roam_sub_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.curSubState[sessionId];
}

/* ---------------------------------------------------------------------------
    \fn sme_get_lim_sme_state
    \brief  get Lim Sme state
    \param hHal - The handle returned by mac_open.
    \return uint32_t - Lim Sme state
    -------------------------------------------------------------------------*/
uint32_t sme_get_lim_sme_state(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->lim.gLimSmeState;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_lim_mlm_state
    \brief  get Lim Mlm state
    \param hHal - The handle returned by mac_open.
    \return uint32_t - Lim Mlm state
    -------------------------------------------------------------------------*/
uint32_t sme_get_lim_mlm_state(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->lim.gLimMlmState;
}

/* ---------------------------------------------------------------------------
    \fn sme_is_lim_session_valid
    \brief  is Lim session valid
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return bool - true or false
    -------------------------------------------------------------------------*/
bool sme_is_lim_session_valid(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId > pMac->lim.maxBssId)
		return false;

	return pMac->lim.gpSession[sessionId].valid;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_lim_sme_session_state
    \brief  get Lim Sme session state
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint32_t - Lim Sme session state
    -------------------------------------------------------------------------*/
uint32_t sme_get_lim_sme_session_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->lim.gpSession[sessionId].limSmeState;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_lim_mlm_session_state
    \brief  get Lim Mlm session state
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint32_t - Lim Mlm session state
    -------------------------------------------------------------------------*/
uint32_t sme_get_lim_mlm_session_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->lim.gpSession[sessionId].limMlmState;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_neighbor_scan_max_chan_time
    \brief  get neighbor scan max channel time
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint16_t - channel max time value
    -------------------------------------------------------------------------*/
uint16_t sme_get_neighbor_scan_max_chan_time(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       maxChannelScanTime;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_neighbor_scan_period
    \brief  Update nNeighborScanPeriod
	    This function is called through dynamic setConfig callback function
	    to configure nNeighborScanPeriod
	    Usage: adb shell iwpriv wlan0 setConfig
			nNeighborScanPeriod=[0 .. 1000]
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  nNeighborScanPeriod - neighbor scan period
   \- return Success or failure
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId,
					const uint16_t nNeighborScanPeriod)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamConfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamConfig =
			&pMac->roam.configParam.neighborRoamConfig;
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set neighbor scan period to %d"
			  " - old value is %d - roam state is %s",
			  nNeighborScanPeriod,
			  pMac->roam.configParam.neighborRoamConfig.
			  nNeighborScanTimerPeriod,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pNeighborRoamConfig->nNeighborScanTimerPeriod =
			nNeighborScanPeriod;
		pNeighborRoamInfo->cfgParams.neighborScanPeriod =
			nNeighborScanPeriod;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_SCAN_HOME_TIME_CHANGED);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_neighbor_scan_period
    \brief  get neighbor scan period
    \param hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return uint16_t - neighbor scan period
    -------------------------------------------------------------------------*/
uint16_t sme_get_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       neighborScanPeriod;
}



/*--------------------------------------------------------------------------
   \brief sme_get_roam_rssi_diff() - get Roam rssi diff
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return uint16_t - Rssi diff value
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_roam_rssi_diff(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.RoamRssiDiff;
}

/**
 * sme_change_roam_scan_channel_list() - to change scan channel list
 * @hHal: pointer HAL handle returned by mac_open
 * @sessionId: sme session id
 * @pChannelList: Output channel list
 * @numChannels: Output number of channels
 *
 * This routine is called to Change roam scan channel list.
 * This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_change_roam_scan_channel_list(tHalHandle hHal, uint8_t sessionId,
					     uint8_t *pChannelList,
					     uint8_t numChannels)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	uint8_t oldChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t i = 0, j = 0;
	tCsrChannelInfo *chan_info;


	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		if (pMac->roam.configParam.isRoamOffloadScanEnabled)
			csr_roam_offload_scan(pMac, sessionId,
					ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					REASON_CHANNEL_LIST_CHANGED);
		return status;
	}
	chan_info = &pNeighborRoamInfo->cfgParams.channelInfo;

	if (NULL != chan_info->ChannelList) {
		for (i = 0; i < chan_info->numOfChannels; i++) {
			if (j < sizeof(oldChannelList))
				j += snprintf(oldChannelList + j,
					sizeof(oldChannelList) -
					j, "%d",
					chan_info->ChannelList[i]);
			else
				break;
		}
	}
	csr_flush_cfg_bg_scan_roam_channel_list(pMac, sessionId);
	csr_create_bg_scan_roam_channel_list(pMac, sessionId, pChannelList,
			numChannels);
	sme_set_roam_scan_control(hHal, sessionId, 1);
	if (NULL != chan_info->ChannelList) {
		j = 0;
		for (i = 0; i < chan_info->numOfChannels; i++) {
			if (j < sizeof(newChannelList))
				j += snprintf(newChannelList + j,
					sizeof(newChannelList) -
					j, " %d",
					chan_info->ChannelList[i]);
			else
				break;
		}
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("LFR runtime successfully set roam scan channels to %s - old value is %s - roam state is %d"),
			newChannelList, oldChannelList,
			pMac->roam.neighborRoamInfo[sessionId].neighborRoamState);
	sme_release_global_lock(&pMac->sme);

	if (pMac->roam.configParam.isRoamOffloadScanEnabled)
		csr_roam_offload_scan(pMac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_CHANNEL_LIST_CHANGED);
	return status;
}

/**
 * sme_get_roam_scan_channel_list() - To get roam scan channel list
 * @hHal: HAL pointer
 * @pChannelList: Output channel list
 * @pNumChannels: Output number of channels
 * @sessionId: Session Identifier
 *
 * To get roam scan channel list This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_roam_scan_channel_list(tHalHandle hHal,
			uint8_t *pChannelList, uint8_t *pNumChannels,
			uint8_t sessionId)
{
	int i = 0;
	uint8_t *pOutPtr = pChannelList;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	if (NULL == pNeighborRoamInfo->cfgParams.channelInfo.ChannelList) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
			FL("Roam Scan channel list is NOT yet initialized"));
		*pNumChannels = 0;
		sme_release_global_lock(&pMac->sme);
		return status;
	}

	*pNumChannels = pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels;
	for (i = 0; i < (*pNumChannels); i++) {
		pOutPtr[i] =
			pNeighborRoamInfo->cfgParams.channelInfo.ChannelList[i];
	}
	pOutPtr[i] = '\0';
	sme_release_global_lock(&pMac->sme);
	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_get_is_ese_feature_enabled() - get ESE feature enabled or not
   This is a synchronuous call
   \param hHal - The handle returned by mac_open.
   \return true (1) - if the ESE feature is enabled
	   false (0) - if feature is disabled (compile or runtime)
   \sa
   --------------------------------------------------------------------------*/
bool sme_get_is_ese_feature_enabled(tHalHandle hHal)
{
#ifdef FEATURE_WLAN_ESE
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return csr_roam_is_ese_ini_feature_enabled(pMac);
#else
	return false;
#endif
}

/*--------------------------------------------------------------------------
   \brief sme_get_wes_mode() - get WES Mode
   This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return uint8_t - WES Mode Enabled(1)/Disabled(0)
   \sa
   --------------------------------------------------------------------------*/
bool sme_get_wes_mode(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.isWESModeEnabled;
}

/*--------------------------------------------------------------------------
   \brief sme_get_roam_scan_control() - get scan control
   This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return bool - Enabled(1)/Disabled(0)
   \sa
   --------------------------------------------------------------------------*/
bool sme_get_roam_scan_control(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.nRoamScanControl;
}

/*--------------------------------------------------------------------------
   \brief sme_get_is_lfr_feature_enabled() - get LFR feature enabled or not
   This is a synchronuous call
   \param hHal - The handle returned by mac_open.
   \return true (1) - if the feature is enabled
	   false (0) - if feature is disabled (compile or runtime)
   \sa
   --------------------------------------------------------------------------*/
bool sme_get_is_lfr_feature_enabled(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.isFastRoamIniFeatureEnabled;
}

/*--------------------------------------------------------------------------
   \brief sme_get_is_ft_feature_enabled() - get FT feature enabled or not
   This is a synchronuous call
   \param hHal - The handle returned by mac_open.
   \return true (1) - if the feature is enabled
	   false (0) - if feature is disabled (compile or runtime)
   \sa
   --------------------------------------------------------------------------*/
bool sme_get_is_ft_feature_enabled(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.isFastTransitionEnabled;
}

/* ---------------------------------------------------------------------------
    \fn sme_is_feature_supported_by_fw
    \brief  Check if an feature is enabled by FW

    \param  feattEnumValue - Enumeration value from placeHolderInCapBitmap
   \- return 1/0 (true/false)
    -------------------------------------------------------------------------*/
uint8_t sme_is_feature_supported_by_fw(uint8_t featEnumValue)
{
	return IS_FEATURE_SUPPORTED_BY_FW(featEnumValue);
}

#ifdef FEATURE_WLAN_TDLS

/* ---------------------------------------------------------------------------
    \fn sme_send_tdls_link_establish_params
    \brief  API to send TDLS Peer Link Establishment Parameters.

    \param  peerMac - peer's Mac Adress.
    \param  tdlsLinkEstablishParams - TDLS Peer Link Establishment Parameters
   \- return QDF_STATUS_SUCCES
    -------------------------------------------------------------------------*/
QDF_STATUS sme_send_tdls_link_establish_params(tHalHandle hHal,
					       uint8_t sessionId,
					       const tSirMacAddr peerMac,
					       tCsrTdlsLinkEstablishParams *
					       tdlsLinkEstablishParams)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_LINK_ESTABLISH_PARAM,
			 sessionId,
			 tdlsLinkEstablishParams->isOffChannelSupported));
	status = sme_acquire_global_lock(&pMac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
	    status = csr_tdls_send_link_establish_params(hHal, sessionId,
				peerMac, tdlsLinkEstablishParams);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_send_tdls_mgmt_frame
    \brief  API to send TDLS management frames.

    \param  peerMac - peer's Mac Adress.
    \param frame_type - Type of TDLS mgmt frame to be sent.
    \param dialog - dialog token used in the frame.
    \param status - status to be incuded in the frame.
    \param peerCapability - peer cpabilities
    \param buf - additional IEs to be included
    \param len - lenght of additional Ies
    \param responder - Tdls request type
   \- return QDF_STATUS_SUCCES
    -------------------------------------------------------------------------*/
QDF_STATUS sme_send_tdls_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
				    const tSirMacAddr peerMac,
				    uint8_t frame_type,
				    uint8_t dialog, uint16_t statusCode,
				    uint32_t peerCapability, uint8_t *buf,
				    uint8_t len, uint8_t responder)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrTdlsSendMgmt sendTdlsReq = { {0} };
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_SEND_MGMT_FRAME,
			 sessionId, statusCode));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_copy(sendTdlsReq.peerMac, peerMac, sizeof(tSirMacAddr));
		sendTdlsReq.frameType = frame_type;
		sendTdlsReq.buf = buf;
		sendTdlsReq.len = len;
		sendTdlsReq.dialog = dialog;
		sendTdlsReq.statusCode = statusCode;
		sendTdlsReq.responder = responder;
		sendTdlsReq.peerCapability = peerCapability;

		status = csr_tdls_send_mgmt_req(hHal, sessionId, &sendTdlsReq);

		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/* ---------------------------------------------------------------------------
    \fn sme_change_tdls_peer_sta
    \brief  API to Update TDLS peer sta parameters.

    \param  peerMac - peer's Mac Adress.
    \param  staParams - Peer Station Parameters
   \- return QDF_STATUS_SUCCES
    -------------------------------------------------------------------------*/
QDF_STATUS sme_change_tdls_peer_sta(tHalHandle hHal, uint8_t sessionId,
				    const tSirMacAddr peerMac,
				    tCsrStaParams *pstaParams)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pstaParams) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s :pstaParams is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_CHANGE_PEER_STA,
			 sessionId, pstaParams->capability));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_tdls_change_peer_sta(hHal, sessionId, peerMac,
						  pstaParams);

		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/* ---------------------------------------------------------------------------
    \fn sme_add_tdls_peer_sta
    \brief  API to Add TDLS peer sta entry.

    \param  peerMac - peer's Mac Adress.
   \- return QDF_STATUS_SUCCES
    -------------------------------------------------------------------------*/
QDF_STATUS sme_add_tdls_peer_sta(tHalHandle hHal, uint8_t sessionId,
				 const tSirMacAddr peerMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_ADD_PEER_STA,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_tdls_add_peer_sta(hHal, sessionId, peerMac);

		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/* ---------------------------------------------------------------------------
    \fn sme_delete_tdls_peer_sta
    \brief  API to Delete TDLS peer sta entry.

    \param  peerMac - peer's Mac Adress.
   \- return QDF_STATUS_SUCCES
    -------------------------------------------------------------------------*/
QDF_STATUS sme_delete_tdls_peer_sta(tHalHandle hHal, uint8_t sessionId,
				    const tSirMacAddr peerMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_DEL_PEER_STA,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_tdls_del_peer_sta(hHal, sessionId, peerMac);

		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/* ---------------------------------------------------------------------------
    \fn sme_set_tdls_power_save_prohibited
    \API to set/reset the is_tdls_power_save_prohibited.

   \- return void
   -------------------------------------------------------------------------*/
void sme_set_tdls_power_save_prohibited(tHalHandle hHal, uint32_t sessionId,
		bool val)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct ps_global_info *ps_global_info = &pMac->sme.ps_global_info;
	struct ps_params *ps_param = &ps_global_info->ps_params[sessionId];
	ps_param->is_tdls_power_save_prohibited = val;
	return;
}

/* ---------------------------------------------------------------------------
   \fn    sme_update_fw_tdls_state

   \brief
    SME will send message to WMA to set TDLS state in f/w

   \param

    hHal - The handle returned by mac_open

    psmeTdlsParams - TDLS state info to update in f/w

    useSmeLock - Need to acquire SME Global Lock before state update or not

   \return QDF_STATUS
   --------------------------------------------------------------------------- */
QDF_STATUS sme_update_fw_tdls_state(tHalHandle hHal, void *psmeTdlsParams,
				    bool useSmeLock)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = NULL;
	cds_msg_t cds_message;

	/* only acquire sme global lock before state update if asked to */
	if (useSmeLock) {
		pMac = PMAC_STRUCT(hHal);
		if (NULL == pMac)
			return QDF_STATUS_E_FAILURE;

		status = sme_acquire_global_lock(&pMac->sme);
		if (QDF_STATUS_SUCCESS != status)
			return status;
	}

	/* serialize the req through MC thread */
	cds_message.bodyptr = psmeTdlsParams;
	cds_message.type = WMA_UPDATE_FW_TDLS_STATE;
	qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		status = QDF_STATUS_E_FAILURE;

	/* release the lock if it was acquired */
	if (useSmeLock)
		sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_update_tdls_peer_state() - to update the state of TDLS peer
 * @hHal: The handle returned by mac_open
 * @peerStateParams: TDLS Peer state info to update in f/w
 *
 * SME will send message to WMA to set TDLS Peer state in f/w
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_tdls_peer_state(tHalHandle hHal,
				      tSmeTdlsPeerStateParams *peerStateParams)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tTdlsPeerStateParams *pTdlsPeerStateParams = NULL;
	tTdlsPeerCapParams *peer_cap = NULL;
	cds_msg_t cds_message;
	uint8_t num;
	uint8_t peer_chan_len;
	uint8_t chanId;
	uint8_t i;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	pTdlsPeerStateParams = qdf_mem_malloc(sizeof(*pTdlsPeerStateParams));
	if (NULL == pTdlsPeerStateParams) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("failed to allocate mem for tdls peer state param"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(pTdlsPeerStateParams, sizeof(*pTdlsPeerStateParams));
	qdf_mem_copy(&pTdlsPeerStateParams->peerMacAddr,
			&peerStateParams->peerMacAddr, sizeof(tSirMacAddr));
	pTdlsPeerStateParams->vdevId = peerStateParams->vdevId;
	pTdlsPeerStateParams->peerState = peerStateParams->peerState;

	switch (peerStateParams->peerState) {
	case eSME_TDLS_PEER_STATE_PEERING:
		pTdlsPeerStateParams->peerState =
			WMA_TDLS_PEER_STATE_PEERING;
		break;

	case eSME_TDLS_PEER_STATE_CONNECTED:
		pTdlsPeerStateParams->peerState =
			WMA_TDLS_PEER_STATE_CONNECTED;
		break;

	case eSME_TDLS_PEER_STATE_TEARDOWN:
		pTdlsPeerStateParams->peerState =
			WMA_TDLS_PEER_STATE_TEARDOWN;
		break;

	case eSME_TDLS_PEER_ADD_MAC_ADDR:
		pTdlsPeerStateParams->peerState = WMA_TDLS_PEER_ADD_MAC_ADDR;
		break;

	case eSME_TDLS_PEER_REMOVE_MAC_ADDR:
		pTdlsPeerStateParams->peerState = WMA_TDLS_PEER_REMOVE_MAC_ADDR;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("invalid peer state param (%d)"),
			peerStateParams->peerState);
		goto error_return;
	}
	peer_cap = &(pTdlsPeerStateParams->peerCap);
	peer_cap->isPeerResponder =
		peerStateParams->peerCap.isPeerResponder;
	peer_cap->peerUapsdQueue =
		peerStateParams->peerCap.peerUapsdQueue;
	peer_cap->peerMaxSp =
		peerStateParams->peerCap.peerMaxSp;
	peer_cap->peerBuffStaSupport =
		peerStateParams->peerCap.peerBuffStaSupport;
	peer_cap->peerOffChanSupport =
		peerStateParams->peerCap.peerOffChanSupport;
	peer_cap->peerCurrOperClass =
		peerStateParams->peerCap.peerCurrOperClass;
	peer_cap->selfCurrOperClass =
		peerStateParams->peerCap.selfCurrOperClass;

	num = 0;
	peer_chan_len = peerStateParams->peerCap.peerChanLen;

	if (peer_chan_len >= 0 &&
	    peer_chan_len <= SME_TDLS_MAX_SUPP_CHANNELS) {
		for (i = 0; i < peerStateParams->peerCap.peerChanLen; i++) {
			chanId = peerStateParams->peerCap.peerChan[i];
			if (csr_roam_is_channel_valid(pMac, chanId) &&
			    !(cds_get_channel_state(chanId) ==
			      CHANNEL_STATE_DFS) &&
			    !cds_is_dsrc_channel(cds_chan_to_freq(chanId))) {
				peer_cap->peerChan[num].chanId = chanId;
				peer_cap->peerChan[num].pwr =
					csr_get_cfg_max_tx_power(pMac, chanId);
				peer_cap->peerChan[num].dfsSet = false;
			num++;
			}
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("invalid peer channel len (%d)"),
			peer_chan_len);
		goto error_return;
	}

	peer_cap->peerChanLen = num;
	peer_cap->peerOperClassLen =
		peerStateParams->peerCap.peerOperClassLen;
	for (i = 0; i < HAL_TDLS_MAX_SUPP_OPER_CLASSES; i++) {
		peer_cap->peerOperClass[i] =
			peerStateParams->peerCap.peerOperClass[i];
	}

	peer_cap->prefOffChanNum =
		peerStateParams->peerCap.prefOffChanNum;
	peer_cap->prefOffChanBandwidth =
		peerStateParams->peerCap.prefOffChanBandwidth;
	peer_cap->opClassForPrefOffChan =
		peerStateParams->peerCap.opClassForPrefOffChan;

	cds_message.type = WMA_UPDATE_TDLS_PEER_STATE;
	cds_message.reserved = 0;
	cds_message.bodyptr = pTdlsPeerStateParams;

	qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("cds_mq_post_message failed "));
		goto error_return;
	} else {
		goto success_return;
	}

error_return:
	status = QDF_STATUS_E_FAILURE;
	qdf_mem_free(pTdlsPeerStateParams);
success_return:
	sme_release_global_lock(&pMac->sme);
	return status;
}

/**
 * sme_send_tdls_chan_switch_req() - send tdls channel switch request
 * @hal: UMAC handler
 * @ch_switch_params: Pointer to the chan switch parameter structure
 *
 * API to set tdls channel switch parameters.
 *
 * Return: QDF_STATUS_SUCCESS on success; another QDF_STATUS_** code otherwise
 */
QDF_STATUS sme_send_tdls_chan_switch_req(tHalHandle hal,
	sme_tdls_chan_switch_params *ch_switch_params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	tdls_chan_switch_params *chan_switch_params = NULL;
	cds_msg_t cds_message;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_TDLS_CHAN_SWITCH_REQ,
			 NO_SESSION, ch_switch_params->tdls_off_channel));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS != status)
		return status;
	chan_switch_params = qdf_mem_malloc(sizeof(*chan_switch_params));
	if (NULL == chan_switch_params) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("fail to alloc mem for tdls chan switch param"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_zero(chan_switch_params, sizeof(*chan_switch_params));

	switch (ch_switch_params->tdls_off_ch_mode) {
	case ENABLE_CHANSWITCH:
		chan_switch_params->tdls_sw_mode = WMA_TDLS_ENABLE_OFFCHANNEL;
		break;

	case DISABLE_CHANSWITCH:
		chan_switch_params->tdls_sw_mode = WMA_TDLS_DISABLE_OFFCHANNEL;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("invalid off channel command (%d)"),
			ch_switch_params->tdls_off_ch_mode);
		qdf_mem_free(chan_switch_params);
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&chan_switch_params->peer_mac_addr,
		&ch_switch_params->peer_mac_addr, sizeof(tSirMacAddr));
	chan_switch_params->vdev_id = ch_switch_params->vdev_id;
	chan_switch_params->tdls_off_ch = ch_switch_params->tdls_off_channel;
	chan_switch_params->tdls_off_ch_bw_offset =
		ch_switch_params->tdls_off_ch_bw_offset;
	chan_switch_params->is_responder = ch_switch_params->is_responder;
	chan_switch_params->oper_class = ch_switch_params->opclass;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		FL("Country Code=%s, Req offset=%d, Selected Operate Class=%d"),
		mac->scan.countryCodeCurrent,
		chan_switch_params->tdls_off_ch_bw_offset,
		chan_switch_params->oper_class);

	cds_message.type = WMA_TDLS_SET_OFFCHAN_MODE;
	cds_message.reserved = 0;
	cds_message.bodyptr = chan_switch_params;

	qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Message Post failed status=%d"),
			qdf_status);
		qdf_mem_free(chan_switch_params);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return status;
}
#endif /* FEATURE_WLAN_TDLS */

QDF_STATUS sme_get_link_speed(tHalHandle hHal, tSirLinkSpeedInfo *lsReq,
			      void *plsContext,
			      void (*pCallbackfn)(tSirLinkSpeedInfo *indParam,
						  void *pContext))
{

	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if ((NULL == pCallbackfn) &&
		    (NULL == pMac->sme.pLinkSpeedIndCb)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Indication Call back did not registered",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		} else if (NULL != pCallbackfn) {
			pMac->sme.pLinkSpeedCbContext = plsContext;
			pMac->sme.pLinkSpeedIndCb = pCallbackfn;
		}
		/* serialize the req through MC thread */
		cds_message.bodyptr = lsReq;
		cds_message.type = WMA_GET_LINK_SPEED;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Link Speed msg fail", __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}


/*
 * SME API to enable/disable WLAN driver initiated SSR
 */
void sme_update_enable_ssr(tHalHandle hHal, bool enableSSR)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "SSR level is changed %d", enableSSR);
		/* not serializing this messsage, as this is only going
		 * to set a variable in WMA/WDI
		 */
		WMA_SetEnableSSR(enableSSR);
		sme_release_global_lock(&pMac->sme);
	}
	return;
}

QDF_STATUS sme_check_ch_in_band(tpAniSirGlobal mac_ctx, uint8_t start_ch,
		uint8_t ch_cnt)
{
	uint8_t i;
	for (i = 0; i < ch_cnt; i++) {
		if (QDF_STATUS_SUCCESS != csr_is_valid_channel(mac_ctx,
					(start_ch + i*4)))
			return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/*convert the ini value to the ENUM used in csr and MAC for CB state*/
ePhyChanBondState sme_get_cb_phy_state_from_cb_ini_value(uint32_t cb_ini_value)
{
	return csr_convert_cb_ini_value_to_phy_cb_state(cb_ini_value);
}

/*--------------------------------------------------------------------------

   \brief sme_set_curr_device_mode() - Sets the current operating device mode.
   \param hHal - The handle returned by mac_open.
   \param currDeviceMode - Current operating device mode.
   --------------------------------------------------------------------------*/

void sme_set_curr_device_mode(tHalHandle hHal,
				enum tQDF_ADAPTER_MODE currDeviceMode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	pMac->sme.currDeviceMode = currDeviceMode;
	return;
}

/*--------------------------------------------------------------------------
   \brief sme_handoff_request() - a wrapper function to Request a handoff
   from CSR.
   This is a synchronous call
   \param hHal - The handle returned by mac_open
   \param  sessionId - Session Identifier
   \param pHandoffInfo - info provided by HDD with the handoff request (namely:
   BSSID, channel etc.)
   \return QDF_STATUS_SUCCESS - SME passed the request to CSR successfully.
	   Other status means SME is failed to send the request.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_handoff_request(tHalHandle hHal,
			       uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: invoked", __func__);
		status = csr_handoff_request(pMac, sessionId, pHandoffInfo);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

#ifdef IPA_OFFLOAD
/* ---------------------------------------------------------------------------
   \fn sme_ipa_offload_enable_disable
   \brief  API to enable/disable IPA offload
   \param  hal - The handle returned by macOpen.
   \param  session_id - Session Identifier
   \param  request -  Pointer to the offload request.
   \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_ipa_offload_enable_disable(tHalHandle hal, uint8_t session_id,
		struct sir_ipa_offload_enable_disable *request)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sir_ipa_offload_enable_disable *request_buf;
	cds_msg_t msg;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		request_buf = qdf_mem_malloc(sizeof(*request_buf));
		if (NULL == request_buf) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					FL("Not able to allocate memory for \
					IPA_OFFLOAD_ENABLE_DISABLE"));
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		request_buf->offload_type = request->offload_type;
		request_buf->vdev_id = request->vdev_id;
		request_buf->enable = request->enable;

		msg.type     = WMA_IPA_OFFLOAD_ENABLE_DISABLE;
		msg.reserved = 0;
		msg.bodyptr  = request_buf;
		if (!QDF_IS_STATUS_SUCCESS(
				cds_mq_post_message(CDS_MQ_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					FL("Not able to post WMA_IPA_OFFLOAD_\
					ENABLE_DISABLE message to WMA"));
			qdf_mem_free(request_buf);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&pMac->sme);
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */

/*
 * SME API to check if there is any infra station or
 * P2P client is connected
 */
QDF_STATUS sme_is_sta_p2p_client_connected(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	if (csr_is_infra_connected(pMac)) {
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

#ifdef FEATURE_WLAN_LPHB
/* ---------------------------------------------------------------------------
    \fn sme_lphb_config_req
    \API to make configuration LPHB within FW.
    \param hHal - The handle returned by mac_open
    \param lphdReq - LPHB request argument by client
    \param pCallbackfn - LPHB timeout notification callback function pointer
   \- return Configuration message posting status, SUCCESS or Fail
    -------------------------------------------------------------------------*/
QDF_STATUS sme_lphb_config_req
	(tHalHandle hHal,
	tSirLPHBReq *lphdReq,
	void (*pCallbackfn)(void *pHddCtx, tSirLPHBInd * indParam)
	) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_LPHB_CONFIG_REQ,
			 NO_SESSION, lphdReq->cmd));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if ((LPHB_SET_EN_PARAMS_INDID == lphdReq->cmd) &&
		    (NULL == pCallbackfn) && (NULL == pMac->sme.pLphbIndCb)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Indication Call back did not registered",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		} else if (NULL != pCallbackfn) {
			pMac->sme.pLphbIndCb = pCallbackfn;
		}

		/* serialize the req through MC thread */
		cds_message.bodyptr = lphdReq;
		cds_message.type = WMA_LPHB_CONF_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Config LPHB MSG fail", __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif /* FEATURE_WLAN_LPHB */
/*--------------------------------------------------------------------------
   \brief sme_enable_disable_split_scan() - a wrapper function to set the split
					    scan parameter.
   This is a synchronous call
   \param hHal - The handle returned by mac_open
   \return NONE.
   \sa
   --------------------------------------------------------------------------*/
void sme_enable_disable_split_scan(tHalHandle hHal, uint8_t nNumStaChan,
				   uint8_t nNumP2PChan)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->roam.configParam.nNumStaChanCombinedConc = nNumStaChan;
	pMac->roam.configParam.nNumP2PChanCombinedConc = nNumP2PChan;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s: SCAN nNumStaChanCombinedConc : %d,"
		  "nNumP2PChanCombinedConc : %d ",
		  __func__, nNumStaChan, nNumP2PChan);

	return;

}

/**
 * sme_add_periodic_tx_ptrn() - Add Periodic TX Pattern
 * @hal: global hal handle
 * @addPeriodicTxPtrnParams: request message
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
sme_add_periodic_tx_ptrn(tHalHandle hal,
			 struct sSirAddPeriodicTxPtrn *addPeriodicTxPtrnParams)
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(hal);
	struct sSirAddPeriodicTxPtrn *req_msg;
	cds_msg_t msg;

	sms_log(mac, LOG1, FL("enter"));

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sms_log(mac, LOGE, FL("memory allocation failed"));
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *addPeriodicTxPtrnParams;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	msg.bodyptr = req_msg;
	msg.type    = WMA_ADD_PERIODIC_TX_PTRN_IND;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_del_periodic_tx_ptrn() - Delete Periodic TX Pattern
 * @hal: global hal handle
 * @delPeriodicTxPtrnParams: request message
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
sme_del_periodic_tx_ptrn(tHalHandle hal,
			 struct sSirDelPeriodicTxPtrn *delPeriodicTxPtrnParams)
{
	QDF_STATUS status    = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac   = PMAC_STRUCT(hal);
	struct sSirDelPeriodicTxPtrn *req_msg;
	cds_msg_t msg;

	sms_log(mac, LOG1, FL("enter"));

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sms_log(mac, LOGE, FL("memory allocation failed"));
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *delPeriodicTxPtrnParams;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	msg.bodyptr = req_msg;
	msg.type    = WMA_DEL_PERIODIC_TX_PTRN_IND;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
  * sme_enable_rmc() - enables RMC
  * @hHal : Pointer to global HAL handle
  * @sessionId : Session ID
  *
  * Return: QDF_STATUS
  */
QDF_STATUS sme_enable_rmc(tHalHandle hHal, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	sms_log(pMac, LOG1, FL("enable RMC"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cds_message.bodyptr = NULL;
		cds_message.type = WMA_RMC_ENABLE_IND;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: failed to post message to WMA",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
  * sme_disable_rmc() - disables RMC
  * @hHal : Pointer to global HAL handle
  * @sessionId : Session ID
  *
  * Return: QDF_STATUS
  */
QDF_STATUS sme_disable_rmc(tHalHandle hHal, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	sms_log(pMac, LOG1, FL("disable RMC"));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cds_message.bodyptr = NULL;
		cds_message.type = WMA_RMC_DISABLE_IND;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: failed to post message to WMA",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
  * sme_send_rmc_action_period() - sends RMC action period param to target
  * @hHal : Pointer to global HAL handle
  * @sessionId : Session ID
  *
  * Return: QDF_STATUS
  */
QDF_STATUS sme_send_rmc_action_period(tHalHandle hHal, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		cds_message.bodyptr = NULL;
		cds_message.type = WMA_RMC_ACTION_PERIOD_IND;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: failed to post message to WMA",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
  * sme_request_ibss_peer_info() -  request ibss peer info
  * @hHal : Pointer to global HAL handle
  * @pUserData : Pointer to user data
  * @peerInfoCbk : Peer info callback
  * @allPeerInfoReqd : All peer info required or not
  * @staIdx : sta index
  *
  * Return:  QDF_STATUS
  */
QDF_STATUS sme_request_ibss_peer_info(tHalHandle hHal, void *pUserData,
				      pIbssPeerInfoCb peerInfoCbk,
				      bool allPeerInfoReqd, uint8_t staIdx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tSirIbssGetPeerInfoReqParams *pIbssInfoReqParams;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pMac->sme.peerInfoParams.peerInfoCbk = peerInfoCbk;
		pMac->sme.peerInfoParams.pUserData = pUserData;

		pIbssInfoReqParams = (tSirIbssGetPeerInfoReqParams *)
				     qdf_mem_malloc(sizeof(tSirIbssGetPeerInfoReqParams));
		if (NULL == pIbssInfoReqParams) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to allocate memory for dhcp start",
				  __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		pIbssInfoReqParams->allPeerInfoReqd = allPeerInfoReqd;
		pIbssInfoReqParams->staIdx = staIdx;

		cds_message.type = WMA_GET_IBSS_PEER_INFO_REQ;
		cds_message.bodyptr = pIbssInfoReqParams;
		cds_message.reserved = 0;

		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post WMA_GET_IBSS_PEER_INFO_REQ MSG failed",
				  __func__);
			qdf_mem_free(pIbssInfoReqParams);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return qdf_status;
}

/* ---------------------------------------------------------------------------
    \fn sme_send_cesium_enable_ind
    \brief  Used to send proprietary cesium enable indication to fw
    \param  hHal
    \param  sessionId
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_send_cesium_enable_ind(tHalHandle hHal, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		cds_message.bodyptr = NULL;
		cds_message.type = WMA_IBSS_CESIUM_ENABLE_IND;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: failed to post message to WMA",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

void sme_get_command_q_status(tHalHandle hHal)
{
	tSmeCmd *pTempCmd = NULL;
	tListElem *pEntry;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: pMac is NULL", __func__);
		return;
	}

	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	}
	sms_log(pMac, LOGE, "Currently smeCmdActiveList has command (0x%X)",
		(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
	if (pTempCmd) {
		if (eSmeCsrCommandMask & pTempCmd->command) {
			/* CSR command is stuck. See what the reason code is for that command */
			dump_csr_command_info(pMac, pTempCmd);
		}
	} /* if(pTempCmd) */

	sms_log(pMac, LOGE, "Currently smeCmdPendingList has %d commands",
		csr_ll_count(&pMac->sme.smeCmdPendingList));

	sms_log(pMac, LOGE, "Currently roamCmdPendingList has %d commands",
		csr_ll_count(&pMac->roam.roamCmdPendingList));

	return;
}

/**
 * sme_set_dot11p_config() - API to set the 802.11p config
 * @hHal:           The handle returned by macOpen
 * @enable_dot11p:  802.11p config param
 */
void sme_set_dot11p_config(tHalHandle hHal, bool enable_dot11p)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	pMac->enable_dot11p = enable_dot11p;
}

/**
 * copy_sir_ocb_config() - Performs deep copy of an OCB configuration
 * @src: the source configuration
 *
 * Return: pointer to the copied OCB configuration
 */
static struct sir_ocb_config *sme_copy_sir_ocb_config(
	struct sir_ocb_config *src)
{
	struct sir_ocb_config *dst;
	uint32_t length;
	void *cursor;

	length = sizeof(*src) +
		src->channel_count * sizeof(*src->channels) +
		src->schedule_size * sizeof(*src->schedule) +
		src->dcc_ndl_chan_list_len +
		src->dcc_ndl_active_state_list_len;

	dst = qdf_mem_malloc(length);
	if (!dst)
		return NULL;

	*dst = *src;

	cursor = dst;
	cursor += sizeof(*dst);
	dst->channels = cursor;
	cursor += src->channel_count * sizeof(*src->channels);
	qdf_mem_copy(dst->channels, src->channels,
		     src->channel_count * sizeof(*src->channels));
	dst->schedule = cursor;
	cursor += src->schedule_size * sizeof(*src->schedule);
	qdf_mem_copy(dst->schedule, src->schedule,
		     src->schedule_size * sizeof(*src->schedule));
	dst->dcc_ndl_chan_list = cursor;
	cursor += src->dcc_ndl_chan_list_len;
	qdf_mem_copy(dst->dcc_ndl_chan_list, src->dcc_ndl_chan_list,
		     src->dcc_ndl_chan_list_len);
	dst->dcc_ndl_active_state_list = cursor;
	cursor += src->dcc_ndl_active_state_list_len;
	qdf_mem_copy(dst->dcc_ndl_active_state_list,
		     src->dcc_ndl_active_state_list,
		     src->dcc_ndl_active_state_list_len);
	return dst;
}

/**
 * sme_ocb_set_config() - Set the OCB configuration
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 * @config: the OCB configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_ocb_set_config(tHalHandle hHal, void *context,
			      ocb_callback callback,
			      struct sir_ocb_config *config)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_ocb_config *msg_body;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	/*
	 * Check if there is a pending request and return an error if one
	 * exists
	 */
	if (pMac->sme.ocb_set_config_callback) {
		status = QDF_STATUS_E_BUSY;
		goto end;
	}

	msg_body = sme_copy_sir_ocb_config(config);

	if (!msg_body) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	msg.type = WMA_OCB_SET_CONFIG_CMD;
	msg.bodyptr = msg_body;

	/* Set the request callback and context */
	pMac->sme.ocb_set_config_callback = callback;
	pMac->sme.ocb_set_config_context = context;

	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		      FL("Error posting message to WDA: %d"), status);
		pMac->sme.ocb_set_config_callback = callback;
		pMac->sme.ocb_set_config_context = context;
		qdf_mem_free(msg_body);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_ocb_set_utc_time() - Set the OCB UTC time
 * @hHal: reference to the HAL
 * @utc: the UTC time struct
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_ocb_set_utc_time(tHalHandle hHal, struct sir_ocb_utc *utc)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_ocb_utc *sme_utc;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	sme_utc = qdf_mem_malloc(sizeof(*sme_utc));
	if (!sme_utc) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Malloc failed"));
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	*sme_utc = *utc;

	msg.type = WMA_OCB_SET_UTC_TIME_CMD;
	msg.reserved = 0;
	msg.bodyptr = sme_utc;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post message to WDA"));
		qdf_mem_free(utc);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_ocb_start_timing_advert() - Start sending timing advert frames
 * @hHal: reference to the HAL
 * @timing_advert: the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_ocb_start_timing_advert(tHalHandle hHal,
	struct sir_ocb_timing_advert *timing_advert)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	void *buf;
	struct sir_ocb_timing_advert *sme_timing_advert;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	buf = qdf_mem_malloc(sizeof(*sme_timing_advert) +
			     timing_advert->template_length);
	if (!buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to allocate memory for start TA"));
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	sme_timing_advert = (struct sir_ocb_timing_advert *)buf;
	*sme_timing_advert = *timing_advert;
	sme_timing_advert->template_value = buf + sizeof(*sme_timing_advert);
	qdf_mem_copy(sme_timing_advert->template_value,
	timing_advert->template_value, timing_advert->template_length);

	msg.type = WMA_OCB_START_TIMING_ADVERT_CMD;
	msg.reserved = 0;
	msg.bodyptr = buf;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post msg to WDA"));
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_ocb_stop_timing_advert() - Stop sending timing advert frames on a channel
 * @hHal: reference to the HAL
 * @timing_advert: the timing advertisement struct
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_ocb_stop_timing_advert(tHalHandle hHal,
	struct sir_ocb_timing_advert *timing_advert)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_ocb_timing_advert *sme_timing_advert;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	sme_timing_advert = qdf_mem_malloc(sizeof(*timing_advert));
	if (!sme_timing_advert) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to allocate memory for stop TA"));
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	*sme_timing_advert = *timing_advert;

	msg.type = WMA_OCB_STOP_TIMING_ADVERT_CMD;
	msg.reserved = 0;
	msg.bodyptr = sme_timing_advert;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post msg to WDA"));
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_ocb_gen_timing_advert_frame() - generate TA frame and populate the buffer
 * @hal_handle: reference to the HAL
 * @self_addr: the self MAC address
 * @buf: the buffer that will contain the frame
 * @timestamp_offset: return for the offset of the timestamp field
 * @time_value_offset: return for the time_value field in the TA IE
 *
 * Return: the length of the buffer.
 */
int sme_ocb_gen_timing_advert_frame(tHalHandle hal_handle,
				    tSirMacAddr self_addr, uint8_t **buf,
				    uint32_t *timestamp_offset,
				    uint32_t *time_value_offset)
{
	int template_length;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);

	template_length = sch_gen_timing_advert_frame(mac_ctx, self_addr, buf,
						  timestamp_offset,
						  time_value_offset);
	return template_length;
}
/**
 * sme_ocb_get_tsf_timer() - Get the TSF timer value
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 * @request: the TSF timer request
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_ocb_get_tsf_timer(tHalHandle hHal, void *context,
				 ocb_callback callback,
				 struct sir_ocb_get_tsf_timer *request)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_ocb_get_tsf_timer *msg_body;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	/* Allocate memory for the WMI request, and copy the parameter */
	msg_body = qdf_mem_malloc(sizeof(*msg_body));
	if (!msg_body) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	*msg_body = *request;

	msg.type = WMA_OCB_GET_TSF_TIMER_CMD;
	msg.bodyptr = msg_body;

	/* Set the request callback and the context */
	pMac->sme.ocb_get_tsf_timer_callback = callback;
	pMac->sme.ocb_get_tsf_timer_context = context;

	/* Post the message to WDA */
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Error posting message to WDA: %d"), status);
		pMac->sme.ocb_get_tsf_timer_callback = NULL;
		pMac->sme.ocb_get_tsf_timer_context = NULL;
		qdf_mem_free(msg_body);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_dcc_get_stats() - Get the DCC stats
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 * @request: the get DCC stats request
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_dcc_get_stats(tHalHandle hHal, void *context,
			     ocb_callback callback,
			     struct sir_dcc_get_stats *request)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_dcc_get_stats *msg_body;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	/* Allocate memory for the WMI request, and copy the parameter */
	msg_body = qdf_mem_malloc(sizeof(*msg_body) +
				  request->request_array_len);
	if (!msg_body) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	*msg_body = *request;
	msg_body->request_array = (void *)msg_body + sizeof(*msg_body);
	qdf_mem_copy(msg_body->request_array, request->request_array,
		     request->request_array_len);

	msg.type = WMA_DCC_GET_STATS_CMD;
	msg.bodyptr = msg_body;

	/* Set the request callback and context */
	pMac->sme.dcc_get_stats_callback = callback;
	pMac->sme.dcc_get_stats_context = context;

	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Error posting message to WDA: %d"), status);
		pMac->sme.dcc_get_stats_callback = callback;
		pMac->sme.dcc_get_stats_context = context;
		qdf_mem_free(msg_body);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_dcc_clear_stats() - Clear the DCC stats
 * @hHal: reference to the HAL
 * @vdev_id: vdev id for OCB interface
 * @dcc_stats_bitmap: the entries in the stats to clear
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_dcc_clear_stats(tHalHandle hHal, uint32_t vdev_id,
			       uint32_t dcc_stats_bitmap)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_dcc_clear_stats *request;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	request = qdf_mem_malloc(sizeof(struct sir_dcc_clear_stats));
	if (!request) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to allocate memory"));
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	qdf_mem_zero(request, sizeof(*request));
	request->vdev_id = vdev_id;
	request->dcc_stats_bitmap = dcc_stats_bitmap;

	msg.type = WMA_DCC_CLEAR_STATS_CMD;
	msg.bodyptr = request;

	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post msg to WDA"));
		qdf_mem_free(request);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_dcc_update_ndl() - Update the DCC settings
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 * @request: the update DCC request
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_dcc_update_ndl(tHalHandle hHal, void *context,
			      ocb_callback callback,
			      struct sir_dcc_update_ndl *request)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg = {0};
	struct sir_dcc_update_ndl *msg_body;

	/* Lock the SME structure */
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	/* Allocate memory for the WMI request, and copy the parameter */
	msg_body = qdf_mem_malloc(sizeof(*msg_body) +
				  request->dcc_ndl_chan_list_len +
				  request->dcc_ndl_active_state_list_len);
	if (!msg_body) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to allocate memory"));
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	*msg_body = *request;

	msg_body->dcc_ndl_chan_list = (void *)msg_body + sizeof(*msg_body);
	msg_body->dcc_ndl_active_state_list = msg_body->dcc_ndl_chan_list +
		request->dcc_ndl_chan_list_len;
	qdf_mem_copy(msg_body->dcc_ndl_chan_list, request->dcc_ndl_chan_list,
		     request->dcc_ndl_active_state_list_len);
	qdf_mem_copy(msg_body->dcc_ndl_active_state_list,
		     request->dcc_ndl_active_state_list,
		     request->dcc_ndl_active_state_list_len);

	msg.type = WMA_DCC_UPDATE_NDL_CMD;
	msg.bodyptr = msg_body;

	/* Set the request callback and the context */
	pMac->sme.dcc_update_ndl_callback = callback;
	pMac->sme.dcc_update_ndl_context = context;

	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Error posting message to WDA: %d"), status);
		pMac->sme.dcc_update_ndl_callback = NULL;
		pMac->sme.dcc_update_ndl_context = NULL;
		qdf_mem_free(msg_body);
		goto end;
	}

end:
	sme_release_global_lock(&pMac->sme);

	return status;
}

/**
 * sme_register_for_dcc_stats_event() - Register for the periodic DCC stats
 *                                      event
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS sme_register_for_dcc_stats_event(tHalHandle hHal, void *context,
					    ocb_callback callback)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = sme_acquire_global_lock(&pMac->sme);
	pMac->sme.dcc_stats_event_callback = callback;
	pMac->sme.dcc_stats_event_context = context;
	sme_release_global_lock(&pMac->sme);

	return 0;
}

/**
 * sme_deregister_for_dcc_stats_event() - De-Register for the periodic DCC stats
 *					  event
 * @h_hal: Hal Handle
 *
 * This function de-registers the DCC perioc stats callback
 *
 * Return: QDF_STATUS Enumeration
 */
QDF_STATUS sme_deregister_for_dcc_stats_event(tHalHandle h_hal)
{
	tpAniSirGlobal mac;
	QDF_STATUS status;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("h_hal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to acquire global lock"));
		return status;
	}
	mac->sme.dcc_stats_event_callback = NULL;
	mac->sme.dcc_stats_event_context = NULL;
	sme_release_global_lock(&mac->sme);

	return status;
}

void sme_get_recovery_stats(tHalHandle hHal)
{
	uint8_t i;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "Self Recovery Stats");
	for (i = 0; i < MAX_ACTIVE_CMD_STATS; i++) {
		if (eSmeNoCommand !=
		    g_self_recovery_stats.activeCmdStats[i].command) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "timestamp %llu: command 0x%0X: reason %d: session %d",
				  g_self_recovery_stats.activeCmdStats[i].
				  timestamp,
				  g_self_recovery_stats.activeCmdStats[i].command,
				  g_self_recovery_stats.activeCmdStats[i].reason,
				  g_self_recovery_stats.activeCmdStats[i].
				  sessionId);
		}
	}
}

/**
 * sme_save_active_cmd_stats() - To save active command stats
 * @hHal: HAL context
 *
 * This routine is to save active command stats
 *
 * Return: None
 */
void sme_save_active_cmd_stats(tHalHandle hHal)
{
	tSmeCmd *pTempCmd = NULL;
	tListElem *pEntry;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t statidx = 0;
	tActiveCmdStats *actv_cmd_stat = NULL;

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("pMac is NULL"));
		return;
	}

	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry)
		pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

	if (!pTempCmd)
		return;

	if (eSmeCsrCommandMask & pTempCmd->command) {
		statidx = g_self_recovery_stats.cmdStatsIndx;
		actv_cmd_stat = &g_self_recovery_stats.activeCmdStats[statidx];
		actv_cmd_stat->command = pTempCmd->command;
		actv_cmd_stat->sessionId = pTempCmd->sessionId;
		actv_cmd_stat->timestamp = cds_get_monotonic_boottime();
		if (eSmeCommandRoam == pTempCmd->command)
			actv_cmd_stat->reason = pTempCmd->u.roamCmd.roamReason;
		else if (eSmeCommandScan == pTempCmd->command)
			actv_cmd_stat->reason = pTempCmd->u.scanCmd.reason;
		else
			actv_cmd_stat->reason = 0xFF;

		g_self_recovery_stats.cmdStatsIndx =
			((g_self_recovery_stats.cmdStatsIndx + 1) &
				(MAX_ACTIVE_CMD_STATS - 1));
	}
	return;
}

void active_list_cmd_timeout_handle(void *userData)
{
	tHalHandle hal = (tHalHandle)userData;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: mac_ctx is null", __func__);
		return;
	}
	/* Return if no cmd pending in active list as
	 * in this case we should not be here.
	 */
	if (0 == csr_ll_count(&mac_ctx->sme.smeCmdActiveList))
		return;
	sms_log(mac_ctx, LOGE,
		FL("Active List command timeout Cmd List Count %d"),
		  csr_ll_count(&mac_ctx->sme.smeCmdActiveList));
	sme_get_command_q_status(hal);

	if (mac_ctx->roam.configParam.enable_fatal_event)
		cds_flush_logs(WLAN_LOG_TYPE_FATAL,
			WLAN_LOG_INDICATOR_HOST_DRIVER,
			WLAN_LOG_REASON_SME_COMMAND_STUCK,
			false,
			mac_ctx->sme.enableSelfRecovery ? true : false);
	else
		qdf_trace_dump_all(mac_ctx, 0, 0, 500, 0);

	if (mac_ctx->sme.enableSelfRecovery) {
		sme_save_active_cmd_stats(hal);
		cds_trigger_recovery();
	} else {
		if (!mac_ctx->roam.configParam.enable_fatal_event &&
		   !(cds_is_load_or_unload_in_progress() ||
		    cds_is_driver_recovering()))
			QDF_BUG(0);
	}
}

QDF_STATUS sme_notify_modem_power_state(tHalHandle hHal, uint32_t value)
{
	cds_msg_t msg;
	tpSirModemPowerStateInd request_buf;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(tSirModemPowerStateInd));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for MODEM POWER STATE IND",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	request_buf->param = value;

	msg.type = WMA_MODEM_POWER_STATE_IND;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (!QDF_IS_STATUS_SUCCESS
		    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_MODEM_POWER_STATE_IND message"
			  " to WMA", __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_HT_2040_COEX
QDF_STATUS sme_notify_ht2040_mode(tHalHandle hHal, uint16_t staId,
				  struct qdf_mac_addr macAddrSTA,
				  uint8_t sessionId,
				  uint8_t channel_type)
{
	cds_msg_t msg;
	tUpdateVHTOpMode *pHtOpMode = NULL;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		return QDF_STATUS_E_FAILURE;
	}

	pHtOpMode = qdf_mem_malloc(sizeof(tUpdateVHTOpMode));
	if (NULL == pHtOpMode) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for setting OP mode",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	switch (channel_type) {
	case eHT_CHAN_HT20:
		pHtOpMode->opMode = eHT_CHANNEL_WIDTH_20MHZ;
		break;

	case eHT_CHAN_HT40MINUS:
	case eHT_CHAN_HT40PLUS:
		pHtOpMode->opMode = eHT_CHANNEL_WIDTH_40MHZ;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid OP mode", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pHtOpMode->staId = staId,
	qdf_mem_copy(pHtOpMode->peer_mac, macAddrSTA.bytes,
		     sizeof(tSirMacAddr));
	pHtOpMode->smesessionId = sessionId;

	msg.type = WMA_UPDATE_OP_MODE;
	msg.reserved = 0;
	msg.bodyptr = pHtOpMode;
	if (!QDF_IS_STATUS_SUCCESS
		    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_UPDATE_OP_MODE message"
			  " to WMA", __func__);
		qdf_mem_free(pHtOpMode);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s: Notifed FW about OP mode: %d for staId=%d",
		  __func__, pHtOpMode->opMode, staId);

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_ht2040_mode

    \brief To update HT Operation beacon IE.

    \param

    \return QDF_STATUS  SUCCESS
			FAILURE or RESOURCES
			The API finished and failed.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_set_ht2040_mode(tHalHandle hHal, uint8_t sessionId,
			       uint8_t channel_type, bool obssEnabled)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	ePhyChanBondState cbMode;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s: Update HT operation beacon IE, channel_type=%d",
		  __func__, channel_type);

	switch (channel_type) {
	case eHT_CHAN_HT20:
		cbMode = PHY_SINGLE_CHANNEL_CENTERED;
		break;
	case eHT_CHAN_HT40MINUS:
		cbMode = PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
		break;
	case eHT_CHAN_HT40PLUS:
		cbMode = PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s:Error!!! Invalid HT20/40 mode !", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_set_ht2040_mode(pMac, sessionId,
					     cbMode, obssEnabled);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn sme_set_phy_cb_mode24_g

    \brief Changes PHY channel bonding mode

    \param hHal - The handle returned by mac_open.

    \param cbMode new channel bonding mode which is to set

    \return QDF_STATUS  SUCCESS.

   -------------------------------------------------------------------------------*/
QDF_STATUS sme_set_phy_cb_mode24_g(tHalHandle hHal, ePhyChanBondState phyCBMode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid context", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	pMac->roam.configParam.channelBondingMode24GHz = phyCBMode;

	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * SME API to enable/disable idle mode powersave
 * This should be called only if powersave offload
 * is enabled
 */
QDF_STATUS sme_set_idle_powersave_config(void *cds_context,
				tHalHandle hHal, uint32_t value)
{
	void *wmaContext = cds_get_context(QDF_MODULE_ID_WMA);
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == wmaContext) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: wmaContext is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  " Idle Ps Set Value %d", value);

	pMac->imps_enabled = false;
	if (QDF_STATUS_SUCCESS != wma_set_idle_ps_config(wmaContext, value)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  " Failed to Set Idle Ps Value %d", value);
		return QDF_STATUS_E_FAILURE;
	}
	if (value)
		pMac->imps_enabled = true;
	return QDF_STATUS_SUCCESS;
}

int16_t sme_get_ht_config(tHalHandle hHal, uint8_t session_id, uint16_t ht_capab)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, session_id);

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: pSession is NULL", __func__);
		return -EIO;
	}
	switch (ht_capab) {
	case WNI_CFG_HT_CAP_INFO_ADVANCE_CODING:
		return pSession->htConfig.ht_rx_ldpc;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		return pSession->htConfig.ht_tx_stbc;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		return pSession->htConfig.ht_rx_stbc;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
		return pSession->htConfig.ht_sgi20;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		return pSession->htConfig.ht_sgi40;
	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "invalid ht capability");
		return -EIO;
	}
}

int sme_update_ht_config(tHalHandle hHal, uint8_t sessionId, uint16_t htCapab,
			 int value)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (NULL == pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: pSession is NULL", __func__);
		return -EIO;
	}

	if (QDF_STATUS_SUCCESS != wma_set_htconfig(sessionId, htCapab, value)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Failed to set ht capability in target");
		return -EIO;
	}

	switch (htCapab) {
	case WNI_CFG_HT_CAP_INFO_ADVANCE_CODING:
		pSession->htConfig.ht_rx_ldpc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		pSession->htConfig.ht_tx_stbc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		pSession->htConfig.ht_rx_stbc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
		pSession->htConfig.ht_sgi20 = value;
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		pSession->htConfig.ht_sgi40 = value;
		break;
	}

	return 0;
}

#define HT20_SHORT_GI_MCS7_RATE 722
/* ---------------------------------------------------------------------------
    \fn sme_send_rate_update_ind
    \brief  API to Update rate
    \param  hHal - The handle returned by mac_open
    \param  rateUpdateParams - Pointer to rate update params
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_send_rate_update_ind(tHalHandle hHal,
				    tSirRateUpdateInd *rateUpdateParams)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status;
	cds_msg_t msg;
	tSirRateUpdateInd *rate_upd = qdf_mem_malloc(sizeof(tSirRateUpdateInd));

	if (rate_upd == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Rate update struct alloc failed");
		return QDF_STATUS_E_FAILURE;
	}
	*rate_upd = *rateUpdateParams;

	if (rate_upd->mcastDataRate24GHz == HT20_SHORT_GI_MCS7_RATE)
		rate_upd->mcastDataRate24GHzTxFlag =
			eHAL_TX_RATE_HT20 | eHAL_TX_RATE_SGI;
	else if (rate_upd->reliableMcastDataRate ==
		 HT20_SHORT_GI_MCS7_RATE)
		rate_upd->reliableMcastDataRateTxFlag =
			eHAL_TX_RATE_HT20 | eHAL_TX_RATE_SGI;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		msg.type = WMA_RATE_UPDATE_IND;
		msg.bodyptr = rate_upd;

		if (!QDF_IS_STATUS_SUCCESS
			    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able "
				  "to post WMA_SET_RMC_RATE_IND to WMA!",
				  __func__);

			sme_release_global_lock(&pMac->sme);
			qdf_mem_free(rate_upd);
			return QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_SUCCESS;
	}

	return status;
}

/**
 * sme_get_reg_info() - To get registration info
 * @hHal: HAL context
 * @chanId: channel id
 * @regInfo1: first reg info to fill
 * @regInfo2: second reg info to fill
 *
 * This routine will give you reg info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_reg_info(tHalHandle hHal, uint8_t chanId,
			    uint32_t *regInfo1, uint32_t *regInfo2)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status;
	uint8_t i;
	bool found = false;

	status = sme_acquire_global_lock(&pMac->sme);
	*regInfo1 = 0;
	*regInfo2 = 0;
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	for (i = 0; i < WNI_CFG_VALID_CHANNEL_LIST_LEN; i++) {
		if (pMac->scan.defaultPowerTable[i].chan_num == chanId) {
			SME_SET_CHANNEL_REG_POWER(*regInfo1,
				pMac->scan.defaultPowerTable[i].power);

			SME_SET_CHANNEL_MAX_TX_POWER(*regInfo2,
				pMac->scan.defaultPowerTable[i].power);
			found = true;
			break;
		}
	}
	if (!found)
		status = QDF_STATUS_E_FAILURE;

	sme_release_global_lock(&pMac->sme);
	return status;
}

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/* ---------------------------------------------------------------------------
    \fn sme_auto_shutdown_cb
    \brief  Used to plug in callback function for receiving auto shutdown evt
    \param  hHal
    \param  pCallbackfn : callback function pointer should be plugged in
   \- return QDF_STATUS
   -------------------------------------------------------------------------*/
QDF_STATUS sme_set_auto_shutdown_cb(tHalHandle hHal, void (*pCallbackfn)(void)
				    ) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "%s: Plug in Auto shutdown event callback", __func__);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (NULL != pCallbackfn) {
			pMac->sme.pAutoShutdownNotificationCb = pCallbackfn;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_auto_shutdown_timer
    \API to set auto shutdown timer value in FW.
    \param hHal - The handle returned by mac_open
    \param timer_val - The auto shutdown timer value to be set
   \- return Configuration message posting status, SUCCESS or Fail
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_auto_shutdown_timer(tHalHandle hHal, uint32_t timer_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSirAutoShutdownCmdParams *auto_sh_cmd;
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		auto_sh_cmd = (tSirAutoShutdownCmdParams *)
			      qdf_mem_malloc(sizeof(tSirAutoShutdownCmdParams));
		if (auto_sh_cmd == NULL) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s Request Buffer Alloc Fail", __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		auto_sh_cmd->timer_val = timer_val;

		/* serialize the req through MC thread */
		cds_message.bodyptr = auto_sh_cmd;
		cds_message.type = WMA_SET_AUTO_SHUTDOWN_TIMER_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Auto shutdown MSG fail", __func__);
			qdf_mem_free(auto_sh_cmd);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: Posted Auto shutdown MSG", __func__);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif

#ifdef FEATURE_WLAN_CH_AVOID
/* ---------------------------------------------------------------------------
    \fn sme_add_ch_avoid_callback
    \brief  Used to plug in callback function
	    Which notify channel may not be used with SAP or P2PGO mode.
	    Notification come from FW.
    \param  hHal
    \param  pCallbackfn : callback function pointer should be plugged in
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_add_ch_avoid_callback
	(tHalHandle hHal, void (*pCallbackfn)(void *pAdapter, void *indParam)
	) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "%s: Plug in CH AVOID CB", __func__);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (NULL != pCallbackfn) {
			pMac->sme.pChAvoidNotificationCb = pCallbackfn;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ch_avoid_update_req
    \API to request channel avoidance update from FW.
    \param hHal - The handle returned by mac_open
    \param update_type - The udpate_type parameter of this request call
   \- return Configuration message posting status, SUCCESS or Fail
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ch_avoid_update_req(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSirChAvoidUpdateReq *cauReq;
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		cauReq = (tSirChAvoidUpdateReq *)
			 qdf_mem_malloc(sizeof(tSirChAvoidUpdateReq));
		if (NULL == cauReq) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s Request Buffer Alloc Fail", __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		cauReq->reserved_param = 0;

		/* serialize the req through MC thread */
		cds_message.bodyptr = cauReq;
		cds_message.type = WMA_CH_AVOID_UPDATE_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Ch Avoid Update MSG fail",
				  __func__);
			qdf_mem_free(cauReq);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: Posted Ch Avoid Update MSG", __func__);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif /* FEATURE_WLAN_CH_AVOID */

/**
 * sme_set_miracast() - Function to set miracast value to UMAC
 * @hal:                Handle returned by macOpen
 * @filter_type:        0-Disabled, 1-Source, 2-sink
 *
 * This function passes down the value of miracast set by
 * framework to UMAC
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
QDF_STATUS sme_set_miracast(tHalHandle hal, uint8_t filter_type)
{
	cds_msg_t msg;
	uint32_t *val;
	tpAniSirGlobal mac_ptr = PMAC_STRUCT(hal);

	val = qdf_mem_malloc(sizeof(*val));
	if (NULL == val || NULL == mac_ptr) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid pointer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*val = filter_type;

	msg.type = SIR_HAL_SET_MIRACAST;
	msg.reserved = 0;
	msg.bodyptr = val;

	if (!QDF_IS_STATUS_SUCCESS(
				cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		    "%s: Not able to post WDA_SET_MAS_ENABLE_DISABLE to WMA!",
		    __func__);
		qdf_mem_free(val);
		return QDF_STATUS_E_FAILURE;
	}

	mac_ptr->sme.miracast_value = filter_type;
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_set_mas() - Function to set MAS value to UMAC
 * @val:	1-Enable, 0-Disable
 *
 * This function passes down the value of MAS to the UMAC. A
 * value of 1 will enable MAS and a value of 0 will disable MAS
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
QDF_STATUS sme_set_mas(uint32_t val)
{
	cds_msg_t msg;
	uint32_t *ptr_val;

	ptr_val = qdf_mem_malloc(sizeof(*ptr_val));
	if (NULL == ptr_val) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: could not allocate ptr_val", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*ptr_val = val;

	msg.type = SIR_HAL_SET_MAS;
	msg.reserved = 0;
	msg.bodyptr = ptr_val;

	if (!QDF_IS_STATUS_SUCCESS(
				cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		    "%s: Not able to post WDA_SET_MAS_ENABLE_DISABLE to WMA!",
		    __func__);
		qdf_mem_free(ptr_val);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_roam_channel_change_req() - Channel change to new target channel
 * @hHal: handle returned by mac_open
 * @bssid: mac address of BSS
 * @ch_params: target channel information
 * @profile: CSR profile
 *
 * API to Indicate Channel change to new target channel
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_roam_channel_change_req(tHalHandle hHal,
				       struct qdf_mac_addr bssid,
				       struct ch_params_s *ch_params,
				       tCsrRoamProfile *profile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		status = csr_roam_channel_change_req(pMac, bssid, ch_params,
				profile);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* -------------------------------------------------------------------------
   \fn sme_process_channel_change_resp
   \brief API to Indicate Channel change response message to SAP.
   \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_process_channel_change_resp(tpAniSirGlobal pMac,
					   uint16_t msg_type, void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo proam_info = { 0 };
	eCsrRoamResult roamResult;
	tpSwitchChannelParams pChnlParams = (tpSwitchChannelParams) pMsgBuf;
	uint32_t SessionId = pChnlParams->peSessionId;

	proam_info.channelChangeRespEvent =
		(tSirChanChangeResponse *)
		qdf_mem_malloc(sizeof(tSirChanChangeResponse));
	if (NULL == proam_info.channelChangeRespEvent) {
		status = QDF_STATUS_E_NOMEM;
		sms_log(pMac, LOGE,
			"Channel Change Event Allocation Failed: %d\n", status);
		return status;
	}
	if (msg_type == eWNI_SME_CHANNEL_CHANGE_RSP) {
		proam_info.channelChangeRespEvent->sessionId = SessionId;
		proam_info.channelChangeRespEvent->newChannelNumber =
			pChnlParams->channelNumber;
		proam_info.channelChangeRespEvent->secondaryChannelOffset =
			pChnlParams->ch_width;

		if (pChnlParams->status == QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Received success eWNI_SME_CHANNEL_CHANGE_RSP for sessionId[%d]",
				  SessionId);
			proam_info.channelChangeRespEvent->channelChangeStatus =
				1;
			roamResult = eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS;
		} else {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Received failure eWNI_SME_CHANNEL_CHANGE_RSP for sessionId[%d]",
				  SessionId);
			proam_info.channelChangeRespEvent->channelChangeStatus =
				0;
			roamResult = eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE;
		}

		csr_roam_call_callback(pMac, SessionId, &proam_info, 0,
				       eCSR_ROAM_SET_CHANNEL_RSP, roamResult);

	} else {
		status = QDF_STATUS_E_FAILURE;
		sms_log(pMac, LOGE, "Invalid Channel Change Resp Message: %d\n",
			status);
	}
	qdf_mem_free(proam_info.channelChangeRespEvent);

	return status;
}

/* -------------------------------------------------------------------------
   \fn sme_roam_start_beacon_req
   \brief API to Indicate LIM to start Beacon Tx
   \after SAP CAC Wait is completed.
   \param hHal - The handle returned by mac_open
   \param sessionId - session ID
   \param dfsCacWaitStatus - CAC WAIT status flag
   \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_roam_start_beacon_req(tHalHandle hHal, struct qdf_mac_addr bssid,
				     uint8_t dfsCacWaitStatus)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	status = sme_acquire_global_lock(&pMac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_start_beacon_req(pMac, bssid, dfsCacWaitStatus);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_roam_csa_ie_request() - request CSA IE transmission from PE
 * @hHal: handle returned by mac_open
 * @bssid: SAP bssid
 * @targetChannel: target channel information
 * @csaIeReqd: CSA IE Request
 * @ch_params: channel information
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_roam_csa_ie_request(tHalHandle hHal, struct qdf_mac_addr bssid,
				uint8_t targetChannel, uint8_t csaIeReqd,
				struct ch_params_s *ch_params)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_send_chan_sw_ie_request(pMac, bssid,
				targetChannel, csaIeReqd, ch_params);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_init_thermal_info
    \brief  SME API to initialize the thermal mitigation parameters
    \param  hHal
    \param  thermalParam : thermal mitigation parameters
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_init_thermal_info(tHalHandle hHal, tSmeThermalParams thermalParam)
{
	t_thermal_mgmt *pWmaParam;
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pWmaParam = (t_thermal_mgmt *) qdf_mem_malloc(sizeof(t_thermal_mgmt));
	if (NULL == pWmaParam) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: could not allocate tThermalMgmt", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero((void *)pWmaParam, sizeof(t_thermal_mgmt));

	pWmaParam->thermalMgmtEnabled = thermalParam.smeThermalMgmtEnabled;
	pWmaParam->throttlePeriod = thermalParam.smeThrottlePeriod;

	pWmaParam->throttle_duty_cycle_tbl[0] =
		thermalParam.sme_throttle_duty_cycle_tbl[0];
	pWmaParam->throttle_duty_cycle_tbl[1] =
		thermalParam.sme_throttle_duty_cycle_tbl[1];
	pWmaParam->throttle_duty_cycle_tbl[2] =
		thermalParam.sme_throttle_duty_cycle_tbl[2];
	pWmaParam->throttle_duty_cycle_tbl[3] =
		thermalParam.sme_throttle_duty_cycle_tbl[3];

	pWmaParam->thermalLevels[0].minTempThreshold =
		thermalParam.smeThermalLevels[0].smeMinTempThreshold;
	pWmaParam->thermalLevels[0].maxTempThreshold =
		thermalParam.smeThermalLevels[0].smeMaxTempThreshold;
	pWmaParam->thermalLevels[1].minTempThreshold =
		thermalParam.smeThermalLevels[1].smeMinTempThreshold;
	pWmaParam->thermalLevels[1].maxTempThreshold =
		thermalParam.smeThermalLevels[1].smeMaxTempThreshold;
	pWmaParam->thermalLevels[2].minTempThreshold =
		thermalParam.smeThermalLevels[2].smeMinTempThreshold;
	pWmaParam->thermalLevels[2].maxTempThreshold =
		thermalParam.smeThermalLevels[2].smeMaxTempThreshold;
	pWmaParam->thermalLevels[3].minTempThreshold =
		thermalParam.smeThermalLevels[3].smeMinTempThreshold;
	pWmaParam->thermalLevels[3].maxTempThreshold =
		thermalParam.smeThermalLevels[3].smeMaxTempThreshold;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		msg.type = WMA_INIT_THERMAL_INFO_CMD;
		msg.bodyptr = pWmaParam;

		if (!QDF_IS_STATUS_SUCCESS
			    (cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post WMA_SET_THERMAL_INFO_CMD to WMA!",
				  __func__);
			qdf_mem_free(pWmaParam);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_SUCCESS;
	}
	qdf_mem_free(pWmaParam);
	return QDF_STATUS_E_FAILURE;
}

/**
 * sme_add_set_thermal_level_callback() - Plug in set thermal level callback
 * @hal:	Handle returned by macOpen
 * @callback:	sme_set_thermal_level_callback
 *
 * Plug in set thermal level callback
 *
 * Return: none
 */
void sme_add_set_thermal_level_callback(tHalHandle hal,
		sme_set_thermal_level_callback callback)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);

	pMac->sme.set_thermal_level_cb = callback;
}

/**
 * sme_set_thermal_level() - SME API to set the thermal mitigation level
 * @hal:         Handler to HAL
 * @level:       Thermal mitigation level
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_level(tHalHandle hal, uint8_t level)
{
	cds_msg_t msg;
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_THERMAL_LEVEL;
		msg.bodyval = level;

		qdf_status =  cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				   "%s: Not able to post WMA_SET_THERMAL_LEVEL to WMA!",
				   __func__);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

/* ---------------------------------------------------------------------------
   \fn sme_txpower_limit
   \brief SME API to set txpower limits
   \param hHal
   \param psmetx : power limits for 2g/5g
   \- return QDF_STATUS
   -------------------------------------------------------------------------*/
QDF_STATUS sme_txpower_limit(tHalHandle hHal, tSirTxPowerLimit *psmetx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	cds_msg_t cds_message;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSirTxPowerLimit *tx_power_limit;

	tx_power_limit = qdf_mem_malloc(sizeof(*tx_power_limit));
	if (!tx_power_limit) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Memory allocation for TxPowerLimit failed!",
			  __func__);
		return QDF_STATUS_E_FAILURE;
	}

	*tx_power_limit = *psmetx;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cds_message.type = WMA_TX_POWER_LIMIT;
		cds_message.reserved = 0;
		cds_message.bodyptr = tx_power_limit;

		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_TX_POWER_LIMIT",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(tx_power_limit);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

QDF_STATUS sme_update_connect_debug(tHalHandle hHal, uint32_t set_value)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	pMac->fEnableDebugLog = set_value;
	return status;
}

/* ---------------------------------------------------------------------------
   \fn    sme_ap_disable_intra_bss_fwd

   \brief
    SME will send message to WMA to set Intra BSS in txrx

   \param

    hHal - The handle returned by mac_open

    sessionId - session id ( vdev id)

    disablefwd - bool value that indicate disable intrabss fwd disable

   \return QDF_STATUS
   --------------------------------------------------------------------------- */
QDF_STATUS sme_ap_disable_intra_bss_fwd(tHalHandle hHal, uint8_t sessionId,
					bool disablefwd)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	int status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	cds_msg_t cds_message;
	tpDisableIntraBssFwd pSapDisableIntraFwd = NULL;

	/* Prepare the request to send to SME. */
	pSapDisableIntraFwd = qdf_mem_malloc(sizeof(tDisableIntraBssFwd));
	if (NULL == pSapDisableIntraFwd) {
		sms_log(pMac, LOGP, "Memory Allocation Failure!!! %s", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(pSapDisableIntraFwd, sizeof(tDisableIntraBssFwd));

	pSapDisableIntraFwd->sessionId = sessionId;
	pSapDisableIntraFwd->disableintrabssfwd = disablefwd;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		cds_message.bodyptr = pSapDisableIntraFwd;
		cds_message.type = WMA_SET_SAP_INTRABSS_DIS;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(pSapDisableIntraFwd);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_STATS_EXT

/******************************************************************************
   \fn sme_stats_ext_register_callback

   \brief
   a function called to register the callback that send vendor event for stats
   ext

   \param callback - callback to be registered
******************************************************************************/
void sme_stats_ext_register_callback(tHalHandle hHal, StatsExtCallback callback)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->sme.StatsExtCallback = callback;
}

/**
 * sme_stats_ext_deregister_callback() - De-register ext stats callback
 * @h_hal: Hal Handle
 *
 * This function is called to  de initialize the HDD NAN feature.  Currently
 * the only operation required is to de-register a callback with SME.
 *
 * Return: None
 */
void sme_stats_ext_deregister_callback(tHalHandle h_hal)
{
	tpAniSirGlobal pmac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("hHal is not valid"));
		return;
	}

	pmac = PMAC_STRUCT(h_hal);
	pmac->sme.StatsExtCallback = NULL;
}


/******************************************************************************
   \fn sme_stats_ext_request

   \brief
   a function called when HDD receives STATS EXT vendor command from userspace

   \param sessionID - vdevID for the stats ext request

   \param input - Stats Ext Request structure ptr

   \return QDF_STATUS
******************************************************************************/
QDF_STATUS sme_stats_ext_request(uint8_t session_id, tpStatsExtRequestReq input)
{
	cds_msg_t msg;
	tpStatsExtRequest data;
	size_t data_len;

	data_len = sizeof(tStatsExtRequest) + input->request_data_len;
	data = qdf_mem_malloc(data_len);

	if (data == NULL) {
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(data, data_len);
	data->vdev_id = session_id;
	data->request_data_len = input->request_data_len;
	if (input->request_data_len) {
		qdf_mem_copy(data->request_data,
			     input->request_data, input->request_data_len);
	}

	msg.type = WMA_STATS_EXT_REQUEST;
	msg.reserved = 0;
	msg.bodyptr = data;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_STATS_EXT_REQUEST message to WMA",
			  __func__);
		qdf_mem_free(data);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/******************************************************************************
   \fn sme_stats_ext_event

   \brief
   a callback function called when SME received eWNI_SME_STATS_EXT_EVENT
   response from WMA

   \param hHal - HAL handle for device
   \param pMsg - Message body passed from WMA; includes NAN header
   \return QDF_STATUS
******************************************************************************/
QDF_STATUS sme_stats_ext_event(tHalHandle hHal, void *pMsg)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pMsg) {
		sms_log(pMac, LOGE, "in %s msg ptr is NULL", __func__);
		status = QDF_STATUS_E_FAILURE;
	} else {
		sms_log(pMac, LOG2, "SME: entering %s", __func__);

		if (pMac->sme.StatsExtCallback) {
			pMac->sme.StatsExtCallback(pMac->hHdd,
						   (tpStatsExtEvent) pMsg);
		}
	}

	return status;
}

#endif

/* ---------------------------------------------------------------------------
    \fn sme_update_dfs_scan_mode
    \brief  Update DFS roam scan mode
	    This function is called through dynamic setConfig callback function
	    to configure allowDFSChannelRoam.
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
    \param  allowDFSChannelRoam - DFS roaming scan mode 0 (disable),
	    1 (passive), 2 (active)
    \return QDF_STATUS_SUCCESS - SME update DFS roaming scan config
	    successfully.
	    Other status means SME failed to update DFS roaming scan config.
    \sa
    -------------------------------------------------------------------------*/
QDF_STATUS sme_update_dfs_scan_mode(tHalHandle hHal, uint8_t sessionId,
				    uint8_t allowDFSChannelRoam)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set AllowDFSChannelRoam Mode to "
			  "%d - old value is %d - roam state is %s",
			  allowDFSChannelRoam,
			  pMac->roam.configParam.allowDFSChannelRoam,
			  mac_trace_get_neighbour_roam_state(pMac->roam.
							     neighborRoamInfo
							     [sessionId].
							     neighborRoamState));
		pMac->roam.configParam.allowDFSChannelRoam =
			allowDFSChannelRoam;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_ROAM_DFS_SCAN_MODE_CHANGED);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_get_dfs_scan_mode() - get DFS roam scan mode
	  This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \return DFS roaming scan mode 0 (disable), 1 (passive), 2 (active)
   \sa
   --------------------------------------------------------------------------*/
uint8_t sme_get_dfs_scan_mode(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	return pMac->roam.configParam.allowDFSChannelRoam;
}

/*----------------------------------------------------------------------------
   \fn  sme_modify_add_ie
   \brief  This function sends msg to updates the additional IE buffers in PE
   \param  hHal - global structure
   \param  pModifyIE - pointer to tModifyIE structure
   \param  updateType - type of buffer
   \- return Success or failure
   -----------------------------------------------------------------------------*/
QDF_STATUS sme_modify_add_ie(tHalHandle hHal,
			     tSirModifyIE *pModifyIE, eUpdateIEsType updateType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	status = sme_acquire_global_lock(&pMac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_modify_add_ies(pMac, pModifyIE, updateType);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*----------------------------------------------------------------------------
   \fn  sme_update_add_ie
   \brief  This function sends msg to updates the additional IE buffers in PE
   \param  hHal - global structure
   \param  pUpdateIE - pointer to structure tUpdateIE
   \param  updateType - type of buffer
   \- return Success or failure
   -----------------------------------------------------------------------------*/
QDF_STATUS sme_update_add_ie(tHalHandle hHal,
			     tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	status = sme_acquire_global_lock(&pMac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_update_add_ies(pMac, pUpdateIE, updateType);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_sta_in_middle_of_roaming
    \brief  This function returns true if STA is in the middle of roaming state
    \param  hHal - HAL handle for device
    \param  sessionId - Session Identifier
   \- return true or false
    -------------------------------------------------------------------------*/
bool sme_sta_in_middle_of_roaming(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool ret = false;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		ret = csr_neighbor_middle_of_roaming(hHal, sessionId);
		sme_release_global_lock(&pMac->sme);
	}
	return ret;
}


/**
 * sme_update_dsc_pto_up_mapping()
 * @hHal: HAL context
 * @dscpmapping: pointer to DSCP mapping structure
 * @sessionId: SME session id
 *
 * This routine is called to update dscp mapping
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_dsc_pto_up_mapping(tHalHandle hHal,
					 sme_QosWmmUpType *dscpmapping,
					 uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i, j, peSessionId;
	tCsrRoamSession *pCsrSession = NULL;
	tpPESession pSession = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	pCsrSession = CSR_GET_SESSION(pMac, sessionId);
	if (pCsrSession == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Session lookup fails for CSR session"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}
	if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid session Id %u"), sessionId);
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	pSession = pe_find_session_by_bssid(pMac,
				pCsrSession->connectedProfile.bssid.bytes,
				&peSessionId);

	if (pSession == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL(" Session lookup fails for BSSID"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pSession->QosMapSet.present) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
				FL("QOS Mapping IE not present"));
		sme_release_global_lock(&pMac->sme);
		return QDF_STATUS_E_FAILURE;
	}
	for (i = 0; i < SME_QOS_WMM_UP_MAX; i++) {
		for (j = pSession->QosMapSet.dscp_range[i][0];
			j <= pSession->QosMapSet.dscp_range[i][1];
			j++) {
			if ((pSession->QosMapSet.dscp_range[i][0] == 255)
				&& (pSession->QosMapSet.dscp_range[i][1] ==
							255)) {
				dscpmapping[j] = 0;
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_ERROR,
					FL("User Priority %d isn't used"), i);
				break;
			} else {
				dscpmapping[j] = i;
			}
		}
	}
	for (i = 0; i < pSession->QosMapSet.num_dscp_exceptions; i++)
		if (pSession->QosMapSet.dscp_exceptions[i][0] != 255)
			dscpmapping[pSession->QosMapSet.dscp_exceptions[i][0]] =
				pSession->QosMapSet.dscp_exceptions[i][1];

	sme_release_global_lock(&pMac->sme);
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_abort_roam_scan
    \brief  API to abort current roam scan cycle by roam scan offload module.
    \param  hHal - The handle returned by mac_open.
    \param  sessionId - Session Identifier
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/

QDF_STATUS sme_abort_roam_scan(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	sms_log(pMac, LOGW, "entering function %s", __func__);
	if (pMac->roam.configParam.isRoamOffloadScanEnabled) {
		/* acquire the lock for the sme object */
		status = sme_acquire_global_lock(&pMac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_ABORT_SCAN,
					      REASON_ROAM_ABORT_ROAM_SCAN);
			/* release the lock for the sme object */
			sme_release_global_lock(&pMac->sme);
		}
	}

	return status;
}

#ifdef FEATURE_WLAN_EXTSCAN
/**
 * sme_get_valid_channels_by_band() - to fetch valid channels filtered by band
 * @hHal: HAL context
 * @wifiBand: RF band information
 * @aValidChannels: output array to store channel info
 * @pNumChannels: output number of channels
 *
 *  SME API to fetch all valid channels filtered by band
 *
 *  Return: QDF_STATUS
 */
QDF_STATUS sme_get_valid_channels_by_band(tHalHandle hHal,
					  uint8_t wifiBand,
					  uint32_t *aValidChannels,
					  uint8_t *pNumChannels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t chanList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t numChannels = 0;
	uint8_t i = 0;
	uint32_t totValidChannels = WNI_CFG_VALID_CHANNEL_LIST_LEN;

	if (!aValidChannels || !pNumChannels) {
		sms_log(pMac, QDF_TRACE_LEVEL_ERROR,
			FL("Output channel list/NumChannels is NULL"));
		return QDF_STATUS_E_INVAL;
	}

	if ((wifiBand < WIFI_BAND_UNSPECIFIED) || (wifiBand >= WIFI_BAND_MAX)) {
		sms_log(pMac, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid wifiBand (%d)"), wifiBand);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_get_cfg_valid_channels(hHal, &chanList[0],
			&totValidChannels);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, QDF_TRACE_LEVEL_ERROR,
			FL("Fail to get valid channel list (err=%d)"), status);
		return status;
	}

	switch (wifiBand) {
	case WIFI_BAND_UNSPECIFIED:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("Unspec Band, return all (%d) valid channels"),
			totValidChannels);
		numChannels = totValidChannels;
		for (i = 0; i < totValidChannels; i++) {
			aValidChannels[i] = cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_BG:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_BG (2.4 GHz)"));
		for (i = 0; i < totValidChannels; i++) {
			if (CDS_IS_CHANNEL_24GHZ(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	case WIFI_BAND_A:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_A (5 GHz without DFS)"));
		for (i = 0; i < totValidChannels; i++) {
			if (CDS_IS_CHANNEL_5GHZ(chanList[i]) &&
			    !CDS_IS_DFS_CH(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	case WIFI_BAND_ABG:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_ABG (2.4 GHz + 5 GHz; no DFS)"));
		for (i = 0; i < totValidChannels; i++) {
			if ((CDS_IS_CHANNEL_24GHZ(chanList[i]) ||
			     CDS_IS_CHANNEL_5GHZ(chanList[i])) &&
			    !CDS_IS_DFS_CH(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	case WIFI_BAND_A_DFS_ONLY:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_A_DFS (5 GHz DFS only)"));
		for (i = 0; i < totValidChannels; i++) {
			if (CDS_IS_CHANNEL_5GHZ(chanList[i]) &&
			    CDS_IS_DFS_CH(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	case WIFI_BAND_A_WITH_DFS:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_A_WITH_DFS (5 GHz with DFS)"));
		for (i = 0; i < totValidChannels; i++) {
			if (CDS_IS_CHANNEL_5GHZ(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	case WIFI_BAND_ABG_WITH_DFS:
		sms_log(pMac, QDF_TRACE_LEVEL_INFO,
			FL("WIFI_BAND_ABG_WITH_DFS (2.4 GHz+5 GHz with DFS)"));
		for (i = 0; i < totValidChannels; i++) {
			if (CDS_IS_CHANNEL_24GHZ(chanList[i]) ||
			    CDS_IS_CHANNEL_5GHZ(chanList[i])) {
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
			}
		}
		break;

	default:
		sms_log(pMac, QDF_TRACE_LEVEL_ERROR,
			FL("Unknown wifiBand (%d))"), wifiBand);
		return QDF_STATUS_E_INVAL;
		break;
	}
	*pNumChannels = numChannels;

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ext_scan_get_capabilities
    \brief  SME API to fetch extscan capabilities
    \param  hHal
    \param  pReq: extscan capabilities structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ext_scan_get_capabilities(tHalHandle hHal,
					 tSirGetExtScanCapabilitiesReqParams *
					 pReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pReq;
		cds_message.type = WMA_EXTSCAN_GET_CAPABILITIES_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ext_scan_start
    \brief  SME API to issue extscan start
    \param  hHal
    \param  pStartCmd: extscan start structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ext_scan_start(tHalHandle hHal,
			      tSirWifiScanCmdReqParams *pStartCmd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pStartCmd;
		cds_message.type = WMA_EXTSCAN_START_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ext_scan_stop
    \brief  SME API to issue extscan stop
    \param  hHal
    \param  pStopReq: extscan stop structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ext_scan_stop(tHalHandle hHal, tSirExtScanStopReqParams *pStopReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pStopReq;
		cds_message.type = WMA_EXTSCAN_STOP_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_bss_hotlist
    \brief  SME API to set BSSID hotlist
    \param  hHal
    \param  pSetHotListReq: extscan set hotlist structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_bss_hotlist(tHalHandle hHal,
			       tSirExtScanSetBssidHotListReqParams *
			       pSetHotListReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pSetHotListReq;
		cds_message.type = WMA_EXTSCAN_SET_BSSID_HOTLIST_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_reset_bss_hotlist
    \brief  SME API to reset BSSID hotlist
    \param  hHal
    \param  pSetHotListReq: extscan set hotlist structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_reset_bss_hotlist(tHalHandle hHal,
				 tSirExtScanResetBssidHotlistReqParams *
				 pResetReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pResetReq;
		cds_message.type = WMA_EXTSCAN_RESET_BSSID_HOTLIST_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_send_wisa_params(): Pass WISA mode to WMA
 * @hal: HAL context
 * @wisa_params: pointer to WISA params struct
 * @sessionId: SME session id
 *
 * Pass WISA params to WMA
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_wisa_params(tHalHandle hal,
				struct sir_wisa_params *wisa_params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct sir_wisa_params *cds_msg_wisa_params;

	cds_msg_wisa_params = qdf_mem_malloc(sizeof(struct sir_wisa_params));
	if (!cds_msg_wisa_params)
		return QDF_STATUS_E_NOMEM;

	*cds_msg_wisa_params = *wisa_params;
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cds_message.bodyptr = cds_msg_wisa_params;
		cds_message.type = WMA_SET_WISA_PARAMS;
		status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_significant_change
    \brief  SME API to set significant change
    \param  hHal
    \param  pSetSignificantChangeReq: extscan set significant change structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_significant_change(tHalHandle hHal,
				      tSirExtScanSetSigChangeReqParams *
				      pSetSignificantChangeReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pSetSignificantChangeReq;
		cds_message.type = WMA_EXTSCAN_SET_SIGNF_CHANGE_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_reset_significant_change
    \brief  SME API to reset significant change
    \param  hHal
    \param  pResetReq: extscan reset significant change structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_reset_significant_change(tHalHandle hHal,
					tSirExtScanResetSignificantChangeReqParams
					*pResetReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pResetReq;
		cds_message.type = WMA_EXTSCAN_RESET_SIGNF_CHANGE_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_get_cached_results
    \brief  SME API to get cached results
    \param  hHal
    \param  pCachedResultsReq: extscan get cached results structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_get_cached_results(tHalHandle hHal,
				  tSirExtScanGetCachedResultsReqParams *
				  pCachedResultsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pCachedResultsReq;
		cds_message.type = WMA_EXTSCAN_GET_CACHED_RESULTS_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_set_epno_list() - set epno network list
 * @hal: global hal handle
 * @input: request message
 *
 * This function constructs the cds message and fill in message type,
 * bodyptr with %input and posts it to WDA queue.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_epno_list(tHalHandle hal,
				struct wifi_epno_params *input)
{
	QDF_STATUS status    = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac   = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct wifi_epno_params *req_msg;
	int len, i;

	sms_log(mac, LOG1, FL("enter"));
	len = sizeof(*req_msg) +
		(input->num_networks * sizeof(struct wifi_epno_network));
	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(req_msg, len);
	req_msg->num_networks = input->num_networks;
	req_msg->request_id = input->request_id;
	req_msg->session_id = input->session_id;
	for (i = 0; i < req_msg->num_networks; i++) {
		req_msg->networks[i].rssi_threshold =
				input->networks[i].rssi_threshold;
		req_msg->networks[i].flags = input->networks[i].flags;
		req_msg->networks[i].auth_bit_field =
				input->networks[i].auth_bit_field;
		req_msg->networks[i].ssid.length =
				input->networks[i].ssid.length;
		qdf_mem_copy(req_msg->networks[i].ssid.ssId,
				input->networks[i].ssid.ssId,
				req_msg->networks[i].ssid.length);
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = WMA_SET_EPNO_LIST_REQ;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_set_passpoint_list() - set passpoint network list
 * @hal: global hal handle
 * @input: request message
 *
 * This function constructs the cds message and fill in message type,
 * bodyptr with @input and posts it to WDA queue.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_passpoint_list(tHalHandle hal,
				struct wifi_passpoint_req *input)
{
	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct wifi_passpoint_req *req_msg;
	int len, i;

	sms_log(mac, LOG1, FL("enter"));
	len = sizeof(*req_msg) +
		(input->num_networks * sizeof(struct wifi_passpoint_network));
	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(req_msg, len);
	req_msg->num_networks = input->num_networks;
	req_msg->request_id = input->request_id;
	req_msg->session_id = input->session_id;
	for (i = 0; i < req_msg->num_networks; i++) {
		req_msg->networks[i].id =
				input->networks[i].id;
		qdf_mem_copy(req_msg->networks[i].realm,
				input->networks[i].realm,
				strlen(input->networks[i].realm) + 1);
		qdf_mem_copy(req_msg->networks[i].plmn,
				input->networks[i].plmn,
				SIR_PASSPOINT_PLMN_LEN);
		qdf_mem_copy(req_msg->networks[i].roaming_consortium_ids,
			     input->networks[i].roaming_consortium_ids,
			sizeof(req_msg->networks[i].roaming_consortium_ids));
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = WMA_SET_PASSPOINT_LIST_REQ;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_reset_passpoint_list() - reset passpoint network list
 * @hHal: global hal handle
 * @input: request message
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_reset_passpoint_list(tHalHandle hal,
				    struct wifi_passpoint_req *input)
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct wifi_passpoint_req *req_msg;

	sms_log(mac, LOG1, FL("enter"));
	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(req_msg, sizeof(*req_msg));
	req_msg->request_id = input->request_id;
	req_msg->session_id = input->session_id;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = WMA_RESET_PASSPOINT_LIST_REQ;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_set_ssid_hotlist() - Set the SSID hotlist
 * @hal: SME handle
 * @request: set ssid hotlist request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_set_ssid_hotlist(tHalHandle hal,
		     struct sir_set_ssid_hotlist_request *request)
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct sir_set_ssid_hotlist_request *set_req;

	set_req = qdf_mem_malloc(sizeof(*set_req));
	if (!set_req) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_FAILURE;
	}

	*set_req = *request;
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = set_req;
		cds_message.type    = WMA_EXTSCAN_SET_SSID_HOTLIST_REQ;
		status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		sme_release_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_free(set_req);
			status = QDF_STATUS_E_FAILURE;
		}
	} else {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(set_req);
		status = QDF_STATUS_E_FAILURE;
	}
	return status;
}

QDF_STATUS sme_ext_scan_register_callback(tHalHandle hHal,
					  void (*pExtScanIndCb)(void *,
								const uint16_t,
								void *))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->sme.pExtScanIndCb = pExtScanIndCb;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}
#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

/* ---------------------------------------------------------------------------
    \fn sme_ll_stats_clear_req
    \brief  SME API to clear Link Layer Statistics
    \param  hHal
    \param  pclearStatsReq: Link Layer clear stats request params structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ll_stats_clear_req(tHalHandle hHal,
				  tSirLLStatsClearReq *pclearStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tSirLLStatsClearReq *clear_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "staId = %u", pclearStatsReq->staId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "statsClearReqMask = 0x%X",
		  pclearStatsReq->statsClearReqMask);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "stopReq = %u", pclearStatsReq->stopReq);

	clear_stats_req = qdf_mem_malloc(sizeof(*clear_stats_req));

	if (!clear_stats_req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for WMA_LL_STATS_CLEAR_REQ",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*clear_stats_req = *pclearStatsReq;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = clear_stats_req;
		cds_message.type = WMA_LINK_LAYER_STATS_CLEAR_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_CLEAR_REQ",
				  __func__);
			qdf_mem_free(clear_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR, "%s: "
			  "sme_acquire_global_lock error", __func__);
		qdf_mem_free(clear_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ll_stats_set_req
    \brief  SME API to set the Link Layer Statistics
    \param  hHal
    \param  psetStatsReq: Link Layer set stats request params structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ll_stats_set_req(tHalHandle hHal, tSirLLStatsSetReq *psetStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tSirLLStatsSetReq *set_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s:  MPDU Size = %u", __func__,
		  psetStatsReq->mpduSizeThreshold);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  " Aggressive Stats Collections = %u",
		  psetStatsReq->aggressiveStatisticsGathering);

	set_stats_req = qdf_mem_malloc(sizeof(*set_stats_req));

	if (!set_stats_req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for WMA_LL_STATS_SET_REQ",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*set_stats_req = *psetStatsReq;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = set_stats_req;
		cds_message.type = WMA_LINK_LAYER_STATS_SET_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_SET_REQ",
				  __func__);
			qdf_mem_free(set_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR, "%s: "
			  "sme_acquire_global_lock error", __func__);
		qdf_mem_free(set_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_ll_stats_get_req
    \brief  SME API to get the Link Layer Statistics
    \param  hHal
    \param  pgetStatsReq: Link Layer get stats request params structure
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_ll_stats_get_req(tHalHandle hHal, tSirLLStatsGetReq *pgetStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tSirLLStatsGetReq *get_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "reqId = %u", pgetStatsReq->reqId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "staId = %u", pgetStatsReq->staId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "Stats Type = %u", pgetStatsReq->paramIdMask);

	get_stats_req = qdf_mem_malloc(sizeof(*get_stats_req));

	if (!get_stats_req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for WMA_LL_STATS_GET_REQ",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*get_stats_req = *pgetStatsReq;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = get_stats_req;
		cds_message.type = WMA_LINK_LAYER_STATS_GET_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_GET_REQ",
				  __func__);

			qdf_mem_free(get_stats_req);
			status = QDF_STATUS_E_FAILURE;

		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR, "%s: "
			  "sme_acquire_global_lock error", __func__);
		qdf_mem_free(get_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_link_layer_stats_ind_cb
    \brief  SME API to trigger the stats are available  after get request
    \param  hHal
    \param callback_routine - HDD callback which needs to be invoked after
	   getting status notification from FW
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_link_layer_stats_ind_cb
	(tHalHandle hHal,
	void (*callback_routine)(void *callbackCtx, int indType, void *pRsp)
	) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->sme.pLinkLayerStatsIndCallback = callback_routine;
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR, "%s: "
			  "sme_acquire_global_lock error", __func__);
	}

	return status;
}

/**
 * sme_reset_link_layer_stats_ind_cb() - SME API to reset link layer stats
 *					 indication
 * @h_hal: Hal Handle
 *
 * This function reset's the link layer stats indication
 *
 * Return: QDF_STATUS Enumeration
 */

QDF_STATUS sme_reset_link_layer_stats_ind_cb(tHalHandle h_hal)
{
	QDF_STATUS status;
	tpAniSirGlobal pmac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("hHal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	pmac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&pmac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pmac->sme.pLinkLayerStatsIndCallback = NULL;
		sme_release_global_lock(&pmac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR, "%s: sme_acquire_global_lock error",
			  __func__);
	}

	return status;
}


#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * sme_fw_mem_dump_register_cb() - Register fw memory dump callback
 *
 * @hal - MAC global handle
 * @callback_routine - callback routine from HDD
 *
 * This API is invoked by HDD to register its callback in SME
 *
 * Return: QDF_STATUS
 */
#ifdef WLAN_FEATURE_MEMDUMP
QDF_STATUS sme_fw_mem_dump_register_cb(tHalHandle hal,
		void (*callback_routine)(void *cb_context,
					 struct fw_dump_rsp *rsp))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&pmac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pmac->sme.fw_dump_callback = callback_routine;
		sme_release_global_lock(&pmac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("sme_acquire_global_lock error"));
	}

	return status;
}
#else
QDF_STATUS sme_fw_mem_dump_register_cb(tHalHandle hal,
		void (*callback_routine)(void *cb_context,
					 struct fw_dump_rsp *rsp))
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_MEMDUMP */

/**
 * sme_fw_mem_dump_unregister_cb() - Unregister fw memory dump callback
 *
 * @hHal - MAC global handle
 *
 * This API is invoked by HDD to unregister its callback in SME
 *
 * Return: QDF_STATUS
 */
#ifdef WLAN_FEATURE_MEMDUMP
QDF_STATUS sme_fw_mem_dump_unregister_cb(tHalHandle hal)
{
	QDF_STATUS status;
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&pmac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pmac->sme.fw_dump_callback = NULL;
		sme_release_global_lock(&pmac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("sme_acquire_global_lock error"));
	}

	return status;
}
#else
QDF_STATUS sme_fw_mem_dump_unregister_cb(tHalHandle hal)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_MEMDUMP */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*--------------------------------------------------------------------------
   \brief sme_update_roam_offload_enabled() - enable/disable roam offload feaure
   It is used at in the REG_DYNAMIC_VARIABLE macro definition of
   \param hHal - The handle returned by mac_open.
   \param nRoamOffloadEnabled - The bool to update with
   \return QDF_STATUS_SUCCESS - SME update config successfully.
	   Other status means SME is failed to update.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_update_roam_offload_enabled(tHalHandle hHal,
					   bool nRoamOffloadEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  "%s: LFR3:gRoamOffloadEnabled is changed from %d to %d",
			  __func__, pMac->roam.configParam.isRoamOffloadEnabled,
			  nRoamOffloadEnabled);
		pMac->roam.configParam.isRoamOffloadEnabled =
			nRoamOffloadEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*--------------------------------------------------------------------------
   \brief sme_update_roam_key_mgmt_offload_enabled() - enable/disable key mgmt
	offload
	This is a synchronous call
   \param hHal - The handle returned by mac_open.
   \param  sessionId - Session Identifier
   \param nRoamKeyMgmtOffloadEnabled - The bool to update with
   \return QDF_STATUS_SUCCESS - SME update config successfully.
	   Other status means SME is failed to update.
   \sa
   --------------------------------------------------------------------------*/

QDF_STATUS sme_update_roam_key_mgmt_offload_enabled(tHalHandle hHal,
						    uint8_t sessionId,
						    bool nRoamKeyMgmtOffloadEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  "%s: LFR3: KeyMgmtOffloadEnabled changed to %d",
				  __func__, nRoamKeyMgmtOffloadEnabled);
			status = csr_roam_set_key_mgmt_offload(pMac,
							       sessionId,
							       nRoamKeyMgmtOffloadEnabled);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif

/* ---------------------------------------------------------------------------
   \fn sme_get_temperature
   \brief SME API to get the pdev temperature
   \param hHal
   \param temperature context
   \param pCallbackfn: callback fn with response (temperature)
   \- return QDF_STATUS
   -------------------------------------------------------------------------*/
QDF_STATUS sme_get_temperature(tHalHandle hHal,
			       void *tempContext,
			       void (*pCallbackfn)(int temperature,
						   void *pContext))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if ((NULL == pCallbackfn) &&
		    (NULL == pMac->sme.pGetTemperatureCb)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL
					  ("Indication Call back did not registered"));
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		} else if (NULL != pCallbackfn) {
			pMac->sme.pTemperatureCbContext = tempContext;
			pMac->sme.pGetTemperatureCb = pCallbackfn;
		}
		/* serialize the req through MC thread */
		cds_message.bodyptr = NULL;
		cds_message.type = WMA_GET_TEMPERATURE_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("Post Get Temperature msg fail"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_set_scanning_mac_oui
    \brief  SME API to set scanning mac oui
    \param  hHal
    \param  pScanMacOui: Scanning Mac Oui (input 3 bytes)
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_scanning_mac_oui(tHalHandle hHal, tSirScanMacOui *pScanMacOui)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = pScanMacOui;
		cds_message.type = WMA_SET_SCAN_MAC_OUI_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("Msg post Set Scan Mac OUI failed"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef DHCP_SERVER_OFFLOAD
/* ---------------------------------------------------------------------------
    \fn sme_set_dhcp_srv_offload
    \brief  SME API to set DHCP server offload info
    \param  hHal
    \param  pDhcpSrvInfo : DHCP server offload info struct
   \- return QDF_STATUS
    -------------------------------------------------------------------------*/
QDF_STATUS sme_set_dhcp_srv_offload(tHalHandle hHal,
				    tSirDhcpSrvOffloadInfo *pDhcpSrvInfo)
{
	cds_msg_t cds_message;
	tSirDhcpSrvOffloadInfo *pSmeDhcpSrvInfo;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pSmeDhcpSrvInfo = qdf_mem_malloc(sizeof(*pSmeDhcpSrvInfo));

	if (!pSmeDhcpSrvInfo) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for WMA_SET_DHCP_SERVER_OFFLOAD_CMD",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*pSmeDhcpSrvInfo = *pDhcpSrvInfo;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* serialize the req through MC thread */
		cds_message.type = WMA_SET_DHCP_SERVER_OFFLOAD_CMD;
		cds_message.bodyptr = pSmeDhcpSrvInfo;

		if (!QDF_IS_STATUS_SUCCESS
			    (cds_mq_post_message(QDF_MODULE_ID_WMA, &cds_message))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post WMA_SET_DHCP_SERVER_OFFLOAD_CMD to WMA!",
				  __func__);
			qdf_mem_free(pSmeDhcpSrvInfo);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_acquire_global_lock error!", __func__);
		qdf_mem_free(pSmeDhcpSrvInfo);
	}

	return status;
}
#endif /* DHCP_SERVER_OFFLOAD */

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
/* ---------------------------------------------------------------------------
    \fn sme_set_led_flashing
    \brief  API to set the Led flashing parameters.
    \param  hHal - The handle returned by mac_open.
    \param  x0, x1 -  led flashing parameters
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS sme_set_led_flashing(tHalHandle hHal, uint8_t type,
				uint32_t x0, uint32_t x1)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t cds_message;
	tSirLedFlashingReq *ledflashing;

	ledflashing = qdf_mem_malloc(sizeof(*ledflashing));
	if (!ledflashing) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL
				  ("Not able to allocate memory for WMA_LED_TIMING_REQ"));
		return QDF_STATUS_E_NOMEM;
	}

	ledflashing->pattern_id = type;
	ledflashing->led_x0 = x0;
	ledflashing->led_x1 = x1;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		cds_message.bodyptr = ledflashing;
		cds_message.type = WMA_LED_FLASHING_REQ;
		qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}
#endif

/**
 *  sme_handle_dfS_chan_scan() - handle DFS channel configuration
 *  @h_hal:         corestack handler
 *  @dfs_flag:      flag indicating dfs channel enable/disable
 *  Return:         QDF_STATUS
 */
QDF_STATUS sme_handle_dfs_chan_scan(tHalHandle h_hal, uint8_t dfs_flag)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		mac->scan.fEnableDFSChnlScan = dfs_flag;

		/* update the channel list to the firmware */
		status = csr_update_channel_list(mac);

		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * sme_validate_sap_channel_switch() - validate target channel switch w.r.t
 *      concurreny rules set to avoid channel interference.
 * @hal - Hal context
 * @sap_ch - channel to switch
 * @sap_phy_mode - phy mode of SAP
 * @cc_switch_mode - concurreny switch mode
 * @session_id - sme session id.
 *
 * Return: true if there is no channel interference else return false
 */
bool sme_validate_sap_channel_switch(tHalHandle hal,
	uint16_t sap_ch, eCsrPhyMode sap_phy_mode, uint8_t cc_switch_mode,
	uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	tCsrRoamSession *session = CSR_GET_SESSION(mac, session_id);
	uint16_t intf_channel = 0;

	if (!session)
		return false;

	session->ch_switch_in_progress = true;
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		intf_channel = csr_check_concurrent_channel_overlap(mac, sap_ch,
						sap_phy_mode,
						cc_switch_mode);
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("sme_acquire_global_lock error!"));
		session->ch_switch_in_progress = false;
		return false;
	}

	session->ch_switch_in_progress = false;
	return (intf_channel == 0) ? true : false;
}
#endif

/**
 * sme_configure_stats_avg_factor() - function to config avg. stats factor
 * @hal: hal
 * @session_id: session ID
 * @stats_avg_factor: average stats factor
 *
 * This function configures the stats avg factor in firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_stats_avg_factor(tHalHandle hal, uint8_t session_id,
					  uint16_t stats_avg_factor)
{
	cds_msg_t msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(hal);
	struct sir_stats_avg_factor *stats_factor;

	stats_factor = qdf_mem_malloc(sizeof(*stats_factor));

	if (!stats_factor) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for SIR_HAL_CONFIG_STATS_FACTOR",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		stats_factor->vdev_id = session_id;
		stats_factor->stats_avg_factor = stats_avg_factor;

		/* serialize the req through MC thread */
		msg.type     = SIR_HAL_CONFIG_STATS_FACTOR;
		msg.bodyptr  = stats_factor;

		if (!QDF_IS_STATUS_SUCCESS(
			    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post SIR_HAL_CONFIG_STATS_FACTOR to WMA!",
				  __func__);
			qdf_mem_free(stats_factor);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_acquire_global_lock error!",
			  __func__);
		qdf_mem_free(stats_factor);
	}

	return status;
}

/**
 * sme_configure_guard_time() - function to configure guard time
 * @hal: hal
 * @session_id: session id
 * @guard_time: guard time
 *
 * This function configures the guard time in firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_guard_time(tHalHandle hal, uint8_t session_id,
				    uint32_t guard_time)
{
	cds_msg_t msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(hal);
	struct sir_guard_time_request *g_time;

	g_time = qdf_mem_malloc(sizeof(*g_time));

	if (!g_time) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for SIR_HAL_CONFIG_GUARD_TIME",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		g_time->vdev_id = session_id;
		g_time->guard_time = guard_time;

		/* serialize the req through MC thread */
		msg.type     = SIR_HAL_CONFIG_GUARD_TIME;
		msg.bodyptr  = g_time;

		if (!QDF_IS_STATUS_SUCCESS(
			    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post SIR_HAL_CONFIG_GUARD_TIME to WMA!",
				  __func__);
			qdf_mem_free(g_time);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_acquire_global_lock error!",
			  __func__);
		qdf_mem_free(g_time);
	}

	return status;
}

/**
 * sme_configure_modulated_dtim() - function to configure modulated dtim
 * @h_hal: SME API to enable/disable modulated DTIM instantaneously
 * @session_id: session ID
 * @modulated_dtim: modulated dtim value
 *
 * This function configures the modulated dtim in firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_modulated_dtim(tHalHandle h_hal, uint8_t session_id,
					uint32_t modulated_dtim)
{
	cds_msg_t msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(h_hal);
	wma_cli_set_cmd_t *iwcmd;

	iwcmd = qdf_mem_malloc(sizeof(*iwcmd));
	if (NULL == iwcmd) {
		QDF_TRACE(QDF_MODULE_ID_SME,
			  QDF_TRACE_LEVEL_FATAL,
			  "%s: qdf_mem_malloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		qdf_mem_zero((void *)iwcmd, sizeof(*iwcmd));
		iwcmd->param_value = modulated_dtim;
		iwcmd->param_vdev_id = session_id;
		iwcmd->param_id = GEN_PARAM_MODULATED_DTIM;
		iwcmd->param_vp_dev = GEN_CMD;
		msg.type = WMA_CLI_SET_CMD;
		msg.reserved = 0;
		msg.bodyptr = (void *)iwcmd;

		if (!QDF_IS_STATUS_SUCCESS(
			    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post GEN_PARAM_DYNAMIC_DTIM to WMA!",
				  __func__);
			qdf_mem_free(iwcmd);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_acquire_global_lock error!",
			  __func__);
		qdf_mem_free(iwcmd);
	}

	return status;
}

/*
 * sme_wifi_start_logger() - Send the start/stop logging command to WMA
 * to either start/stop logging
 * @hal: HAL context
 * @start_log: Structure containing the wifi start logger params
 *
 * This function sends the start/stop logging command to WMA
 *
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_wifi_start_logger(tHalHandle hal,
		struct sir_wifi_start_log start_log)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac   = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct sir_wifi_start_log *req_msg;
	uint32_t len;

	len = sizeof(*req_msg);
	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(req_msg, len);

	req_msg->verbose_level = start_log.verbose_level;
	req_msg->flag = start_log.flag;
	req_msg->ring_id = start_log.ring_id;
	req_msg->ini_triggered = start_log.ini_triggered;
	req_msg->user_triggered = start_log.user_triggered;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = SIR_HAL_START_STOP_LOGGING;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

/**
 * sme_neighbor_middle_of_roaming() - Function to know if
 * STA is in the middle of roaming states
 * @hal:                Handle returned by macOpen
 * @sessionId: sessionId of the STA session
 *
 * This function is a wrapper to call
 * csr_neighbor_middle_of_roaming to know STA is in the
 * middle of roaming states
 *
 * Return: True or False
 *
 */
bool sme_neighbor_middle_of_roaming(tHalHandle hHal, uint8_t sessionId)
{
	return csr_neighbor_middle_of_roaming(PMAC_STRUCT(hHal), sessionId);
}

/*
 * sme_send_flush_logs_cmd_to_fw() - Flush FW logs
 * @mac: MAC handle
 *
 * This function is used to send the command that will
 * be used to flush the logs in the firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_flush_logs_cmd_to_fw(tpAniSirGlobal mac)
{
	QDF_STATUS status;
	cds_msg_t message;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = NULL;
	message.type    = SIR_HAL_FLUSH_LOG_TO_FW;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_enable_uapsd_for_ac() - enable uapsd for access catagory requerst to WMA
 * @cds_ctx: cds context
 * @sta_id: station id
 * @ac: access catagory
 * @tid: tid value
 * @pri: user priority
 * @srvc_int: service interval
 * @sus_int: suspend interval
 * @dir: tspec direction
 * @psb: PSB value
 * @sessionId: session id
 * @delay_interval: delay interval
 *
 * Return: QDF status
 */
QDF_STATUS sme_enable_uapsd_for_ac(void *cds_ctx, uint8_t sta_id,
				   sme_ac_enum_type ac, uint8_t tid,
				   uint8_t pri, uint32_t srvc_int,
				   uint32_t sus_int,
				   sme_tspec_dir_type dir,
				   uint8_t psb, uint32_t sessionId,
				   uint32_t delay_interval)
{
	void *wma_handle;
	t_wma_trigger_uapsd_params uapsd_params;
	enum uapsd_ac access_category;

	if (!psb) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"No need to configure auto trigger:psb is 0");
		return QDF_STATUS_SUCCESS;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					"wma_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	switch (ac) {
	case SME_AC_BK:
		access_category = UAPSD_BK;
		break;
	case SME_AC_BE:
		access_category = UAPSD_BE;
		break;
	case SME_AC_VI:
		access_category = UAPSD_VI;
		break;
	case SME_AC_VO:
		access_category = UAPSD_VO;
		break;
	default:
		return QDF_STATUS_E_FAILURE;
	}

	uapsd_params.wmm_ac = access_category;
	uapsd_params.user_priority = pri;
	uapsd_params.service_interval = srvc_int;
	uapsd_params.delay_interval = delay_interval;
	uapsd_params.suspend_interval = sus_int;

	if (QDF_STATUS_SUCCESS !=
	    wma_trigger_uapsd_params(wma_handle, sessionId, &uapsd_params)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Failed to Trigger Uapsd params for sessionId %d",
			    sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_disable_uapsd_for_ac() - disable uapsed access catagory request to WMA
 * @cds_ctx: cds context
 * @sta_id: station id
 * @ac: access catagory
 * @sessionId: session id
 *
 * Return: QDF status
 */
QDF_STATUS sme_disable_uapsd_for_ac(void *cds_ctx, uint8_t sta_id,
				       sme_ac_enum_type ac,
				       uint32_t sessionId)
{
	void *wma_handle;
	enum uapsd_ac access_category;

	switch (ac) {
	case SME_AC_BK:
		access_category = UAPSD_BK;
		break;
	case SME_AC_BE:
		access_category = UAPSD_BE;
		break;
	case SME_AC_VI:
		access_category = UAPSD_VI;
		break;
	case SME_AC_VO:
		access_category = UAPSD_VO;
		break;
	default:
		return QDF_STATUS_E_FAILURE;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (QDF_STATUS_SUCCESS !=
	    wma_disable_uapsd_per_ac(wma_handle, sessionId, access_category)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Failed to disable uapsd for ac %d for sessionId %d",
			    ac, sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_update_nss() - SME API to change the number for spatial streams
 * (1 or 2)
 * @hal: Handle returned by mac open
 * @nss: Number of spatial streams
 *
 * This function is used to update the number of spatial streams supported.
 *
 * Return: Success upon successfully changing nss else failure
 *
 */
QDF_STATUS sme_update_nss(tHalHandle h_hal, uint8_t nss)
{
	QDF_STATUS status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(h_hal);
	uint32_t i, value = 0;
	union {
		uint16_t cfg_value16;
		tSirMacHTCapabilityInfo ht_cap_info;
	} uHTCapabilityInfo;
	tCsrRoamSession *csr_session;

	status = sme_acquire_global_lock(&mac_ctx->sme);

	if (QDF_STATUS_SUCCESS == status) {
		mac_ctx->roam.configParam.enable2x2 = (nss == 1) ? 0 : 1;

		/* get the HT capability info*/
		sme_cfg_get_int(mac_ctx, WNI_CFG_HT_CAP_INFO, &value);
		uHTCapabilityInfo.cfg_value16 = (0xFFFF & value);

		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
			if (CSR_IS_SESSION_VALID(mac_ctx, i)) {
				csr_session = &mac_ctx->roam.roamSession[i];
				csr_session->htConfig.ht_tx_stbc =
					uHTCapabilityInfo.ht_cap_info.txSTBC;
			}
		}

		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}

/**
 * sme_update_user_configured_nss() - sets the nss based on user request
 * @hal: Pointer to HAL
 * @nss: number of streams
 *
 * Return: None
 */
void sme_update_user_configured_nss(tHalHandle hal, uint8_t nss)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	mac_ctx->user_configured_nss = nss;
}

/**
 * sme_set_rssi_threshold_breached_cb() - set rssi threshold breached callback
 * @h_hal: global hal handle
 * @cb: callback function pointer
 *
 * This function stores the rssi threshold breached callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_rssi_threshold_breached_cb(tHalHandle h_hal,
				void (*cb)(void *, struct rssi_breach_event *))
{
	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("hHal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		return status;
	}

	mac->sme.rssi_threshold_breached_cb = cb;
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_set_rssi_threshold_breached_cb() - Reset rssi threshold breached callback
 * @hal: global hal handle
 *
 * This function de-registers the rssi threshold breached callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_reset_rssi_threshold_breached_cb(tHalHandle hal)
{
	QDF_STATUS status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		return status;
	}

	mac->sme.rssi_threshold_breached_cb = NULL;
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_is_any_session_in_connected_state() - SME wrapper API to
 * check if any session is in connected state or not.
 *
 * @hal: Handle returned by mac open
 *
 * This function is used to check if any valid sme session is in
 * connected state or not.
 *
 * Return: true if any session is connected, else false.
 *
 */
bool sme_is_any_session_in_connected_state(tHalHandle h_hal)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(h_hal);
	QDF_STATUS status;
	bool ret = false;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		ret = csr_is_any_session_in_connect_state(mac_ctx);
		sme_release_global_lock(&mac_ctx->sme);
	}
	return ret;
}

/**
 * sme_set_rssi_monitoring() - set rssi monitoring
 * @hal: global hal handle
 * @input: request message
 *
 * This function constructs the vos message and fill in message type,
 * bodyptr with @input and posts it to WDA queue.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_rssi_monitoring(tHalHandle hal,
				struct rssi_monitor_req *input)
{
	QDF_STATUS status     = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac    = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct rssi_monitor_req *req_msg;

	sms_log(mac, LOG1, FL("enter"));
	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sms_log(mac, LOGE, FL("memory allocation failed"));
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *input;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = WMA_SET_RSSI_MONITOR_REQ;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
			FL("cds_mq_post_message failed!(err=%d)"),
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

/**
 * sme_fw_mem_dump() - Get FW memory dump
 * @hHal: hal handle
 * @recvd_req: received memory dump request.
 *
 * This API is invoked by HDD to indicate FW to start
 * dumping firmware memory.
 *
 * Return: QDF_STATUS
 */
#ifdef WLAN_FEATURE_MEMDUMP
QDF_STATUS sme_fw_mem_dump(tHalHandle hHal, void *recvd_req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	cds_msg_t msg;
	struct fw_dump_req *send_req;
	struct fw_dump_seg_req seg_req;
	int loop;

	send_req = qdf_mem_malloc(sizeof(*send_req));
	if (!send_req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Memory allocation failed for WDA_FW_MEM_DUMP"));
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(send_req, recvd_req, sizeof(*send_req));

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  FL("request_id:%d num_seg:%d"),
		  send_req->request_id, send_req->num_seg);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  FL("Segment Information"));
	for (loop = 0; loop < send_req->num_seg; loop++) {
		seg_req = send_req->segment[loop];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("seg_number:%d"), loop);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("seg_id:%d start_addr_lo:0x%x start_addr_hi:0x%x"),
			  seg_req.seg_id, seg_req.seg_start_addr_lo,
			  seg_req.seg_start_addr_hi);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			  FL("seg_length:%d dst_addr_lo:0x%x dst_addr_hi:0x%x"),
			  seg_req.seg_length, seg_req.dst_addr_lo,
			  seg_req.dst_addr_hi);
	}

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		msg.bodyptr = send_req;
		msg.type = WMA_FW_MEM_DUMP_REQ;
		msg.reserved = 0;

		qdf_status = cds_mq_post_message(QDF_MODULE_ID_WMA, &msg);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("Not able to post WMA_FW_MEM_DUMP"));
			qdf_mem_free(send_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Failed to acquire SME Global Lock"));
		qdf_mem_free(send_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}
#else
QDF_STATUS sme_fw_mem_dump(tHalHandle hHal, void *recvd_req)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_MEMDUMP */

/*
 * sme_pdev_set_pcl() - Send WMI_PDEV_SET_PCL_CMDID to the WMA
 * @hal: Handle returned by macOpen
 * @msg: PCL channel list and length structure
 *
 * Sends the command to WMA to send WMI_PDEV_SET_PCL_CMDID to FW
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_pdev_set_pcl(tHalHandle hal,
		struct sir_pcl_list msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac   = PMAC_STRUCT(hal);
	cds_msg_t cds_message;
	struct wmi_pcl_chan_weights *req_msg;
	uint32_t len, i;

	len = sizeof(*req_msg);

	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sms_log(mac, LOGE, FL("qdf_mem_malloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(req_msg, len);

	for (i = 0; i < msg.pcl_len; i++) {
		req_msg->pcl_list[i] =  msg.pcl_list[i];
		req_msg->weight_list[i] =  msg.weight_list[i];
	}

	req_msg->pcl_len = msg.pcl_len;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac, LOGE,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	cds_message.bodyptr = req_msg;
	cds_message.type    = SIR_HAL_PDEV_SET_PCL_TO_FW;
	status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE,
				FL("cds_mq_post_message failed!(err=%d)"),
				status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

/*
 * sme_pdev_set_hw_mode() - Send WMI_PDEV_SET_HW_MODE_CMDID to the WMA
 * @hal: Handle returned by macOpen
 * @msg: HW mode structure containing hw mode and callback details
 *
 * Sends the command to CSR to send WMI_PDEV_SET_HW_MODE_CMDID to FW
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_pdev_set_hw_mode(tHalHandle hal,
		struct sir_hw_mode msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	tSmeCmd *cmd = NULL;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE, FL("Failed to acquire lock"));
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = sme_get_command_buffer(mac);
	if (!cmd) {
		sms_log(mac, LOGE, FL("Get command buffer failed"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_hw_mode;
	cmd->u.set_hw_mode_cmd.hw_mode_index = msg.hw_mode_index;
	cmd->u.set_hw_mode_cmd.set_hw_mode_cb = msg.set_hw_mode_cb;
	cmd->u.set_hw_mode_cmd.reason = msg.reason;
	cmd->u.set_hw_mode_cmd.session_id = msg.session_id;

	sms_log(mac, LOG1,
		FL("Queuing set hw mode to CSR, session:%d reason:%d"),
		cmd->u.set_hw_mode_cmd.session_id,
		cmd->u.set_hw_mode_cmd.reason);
	csr_queue_sme_command(mac, cmd, false);

	sme_release_global_lock(&mac->sme);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_register_hw_mode_trans_cb() - HW mode transition callback registration
 * @hal: Handle returned by macOpen
 * @callback: HDD callback to be registered
 *
 * Registers the HDD callback with SME. This callback will be invoked when
 * HW mode transition event is received from the FW
 *
 * Return: None
 */
void sme_register_hw_mode_trans_cb(tHalHandle hal,
			hw_mode_transition_cb callback)
{
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	mac->sme.sme_hw_mode_trans_cb = callback;
	sms_log(mac, LOG1, FL("HW mode transition callback registered"));
}

/**
 * sme_nss_update_request() - Send beacon templete update to FW with new
 * nss value
 * @hal: Handle returned by macOpen
 * @vdev_id: the session id
 * @new_nss: the new nss value
 * @cback: hdd callback
 * @next_action: next action to happen at policy mgr after beacon update
 *
 * Sends the command to CSR to send to PE
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_nss_update_request(tHalHandle hHal, uint32_t vdev_id,
				uint8_t  new_nss, void *cback, uint8_t next_action,
				void *hdd_context,
				enum sir_conn_update_reason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac = PMAC_STRUCT(hHal);
	tSmeCmd *cmd = NULL;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cmd = sme_get_command_buffer(mac);
		if (!cmd) {
			sms_log(mac, LOGE, FL("Get command buffer failed"));
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NULL_VALUE;
		}
		cmd->command = e_sme_command_nss_update;
		/* Sessionized modules may require this info */
		cmd->sessionId = vdev_id;
		cmd->u.nss_update_cmd.new_nss = new_nss;
		cmd->u.nss_update_cmd.session_id = vdev_id;
		cmd->u.nss_update_cmd.nss_update_cb = cback;
		cmd->u.nss_update_cmd.context = hdd_context;
		cmd->u.nss_update_cmd.next_action = next_action;
		cmd->u.nss_update_cmd.reason = reason;

		sms_log(mac, LOG1, FL("Queuing e_sme_command_nss_update to CSR"));
		csr_queue_sme_command(mac, cmd, false);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_soc_set_dual_mac_config() - Set dual mac configurations
 * @hal: Handle returned by macOpen
 * @msg: Structure containing the dual mac config parameters
 *
 * Queues configuration information to CSR to configure
 * WLAN firmware for the dual MAC features
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_soc_set_dual_mac_config(tHalHandle hal,
		struct sir_dual_mac_config msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	tSmeCmd *cmd;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE, FL("Failed to acquire lock"));
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = sme_get_command_buffer(mac);
	if (!cmd) {
		sms_log(mac, LOGE, FL("Get command buffer failed"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_dual_mac_config;
	cmd->u.set_dual_mac_cmd.scan_config = msg.scan_config;
	cmd->u.set_dual_mac_cmd.fw_mode_config = msg.fw_mode_config;
	cmd->u.set_dual_mac_cmd.set_dual_mac_cb = msg.set_dual_mac_cb;

	sms_log(mac, LOG1,
		FL("Queuing e_sme_command_set_dual_mac_config to CSR: %x %x"),
		cmd->u.set_dual_mac_cmd.scan_config,
		cmd->u.set_dual_mac_cmd.fw_mode_config);
	csr_queue_sme_command(mac, cmd, false);

	sme_release_global_lock(&mac->sme);
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_LFR_SUBNET_DETECTION
/**
 * sme_gateway_param_update() - to update gateway parameters with WMA
 * @Hal: hal handle
 * @gw_params: request parameters from HDD
 *
 * Return: QDF_STATUS
 *
 * This routine will update gateway parameters to WMA
 */
QDF_STATUS sme_gateway_param_update(tHalHandle Hal,
			      struct gateway_param_update_req *gw_params)
{
	QDF_STATUS qdf_status;
	cds_msg_t cds_message;
	struct gateway_param_update_req *request_buf;

	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to allocate memory for gw param update request"));
		return QDF_STATUS_E_NOMEM;
	}

	*request_buf = *gw_params;

	cds_message.type = WMA_GW_PARAM_UPDATE_REQ;
	cds_message.reserved = 0;
	cds_message.bodyptr = request_buf;
	qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &cds_message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post WMA_GW_PARAM_UPDATE_REQ message to HAL"));
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_LFR_SUBNET_DETECTION */

/**
 * sme_soc_set_antenna_mode() - set antenna mode
 * @hal: Handle returned by macOpen
 * @msg: Structure containing the antenna mode parameters
 *
 * Send the command to CSR to send
 * WMI_SOC_SET_ANTENNA_MODE_CMDID to FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_soc_set_antenna_mode(tHalHandle hal,
				struct sir_antenna_mode_param *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	tSmeCmd *cmd;

	if (NULL == msg) {
		sms_log(mac, LOGE, FL("antenna mode mesg is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac, LOGE, FL("Failed to acquire lock"));
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = sme_get_command_buffer(mac);
	if (!cmd) {
		sme_release_global_lock(&mac->sme);
		sms_log(mac, LOGE, FL("Get command buffer failed"));
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_antenna_mode;
	cmd->u.set_antenna_mode_cmd = *msg;

	sms_log(mac, LOG1,
		FL("Queuing e_sme_command_set_antenna_mode to CSR: %d %d"),
		cmd->u.set_antenna_mode_cmd.num_rx_chains,
		cmd->u.set_antenna_mode_cmd.num_tx_chains);

	csr_queue_sme_command(mac, cmd, false);
	sme_release_global_lock(&mac->sme);

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_set_peer_authorized() - call peer authorized callback
 * @peer_addr: peer mac address
 * @auth_cb: auth callback
 * @vdev_id: vdev id
 *
 * Return: QDF Status
 */
QDF_STATUS sme_set_peer_authorized(uint8_t *peer_addr,
				   sme_peer_authorized_fp auth_cb,
				   uint32_t vdev_id)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	wma_set_peer_authorized_cb(wma_handle, auth_cb);
	return wma_set_peer_param(wma_handle, peer_addr, WMI_PEER_AUTHORIZE,
				  1, vdev_id);
}

/*
 * sme_handle_set_fcc_channel() - set spec. tx power for non-fcc channel
 * @hal: HAL pointer
 * @fcc_constraint: flag to enable/disable the constraint
 * @scan_pending: whether there is pending scan
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_handle_set_fcc_channel(tHalHandle hal, bool fcc_constraint,
				      bool scan_pending)
{
	QDF_STATUS status;
	tpAniSirGlobal mac_ptr  = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac_ptr->sme);

	if (QDF_STATUS_SUCCESS == status) {

		if (fcc_constraint != mac_ptr->scan.fcc_constraint) {
			mac_ptr->scan.fcc_constraint = fcc_constraint;
			if (scan_pending)
				mac_ptr->scan.defer_update_channel_list = true;
			else
				status = csr_update_channel_list(mac_ptr);
			/* update the channel list in firmware */
			status = csr_update_channel_list(mac_ptr);
		}

		sme_release_global_lock(&mac_ptr->sme);
	}

	return status;
}
/**
 * sme_setdef_dot11mode() - Updates pMac with default dot11mode
 * @hal: Global MAC pointer
 *
 * Return: NULL.
 */
void sme_setdef_dot11mode(tHalHandle hal)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	csr_set_default_dot11_mode(mac_ctx);
}

/**
 * sme_update_roam_scan_hi_rssi_scan_params() - update high rssi scan
 *         params
 * @hal_handle - The handle returned by macOpen.
 * @session_id - Session Identifier
 * @notify_id - Identifies 1 of the 4 parameters to be modified
 * @val New value of the parameter
 *
 * Return: QDF_STATUS - SME update config successful.
 *         Other status means SME failed to update
 */

QDF_STATUS sme_update_roam_scan_hi_rssi_scan_params(tHalHandle hal_handle,
	uint8_t session_id,
	uint32_t notify_id,
	int32_t val)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	tCsrNeighborRoamConfig *nr_config = NULL;
	tpCsrNeighborRoamControlInfo nr_info = NULL;
	uint32_t reason = 0;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		nr_config = &mac_ctx->roam.configParam.neighborRoamConfig;
		nr_info   = &mac_ctx->roam.neighborRoamInfo[session_id];
		switch (notify_id) {
		case eCSR_HI_RSSI_SCAN_MAXCOUNT_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				"%s: gRoamScanHirssiMaxCount %d => %d",
				__func__, nr_config->nhi_rssi_scan_max_count,
				val);
			nr_config->nhi_rssi_scan_max_count = val;
			nr_info->cfgParams.hi_rssi_scan_max_count = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_MAXCOUNT_CHANGED;
		break;

		case eCSR_HI_RSSI_SCAN_RSSI_DELTA_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("gRoamScanHiRssiDelta %d => %d"),
				nr_config->nhi_rssi_scan_rssi_delta,
				val);
			nr_config->nhi_rssi_scan_rssi_delta = val;
			nr_info->cfgParams.hi_rssi_scan_rssi_delta = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_DELTA_CHANGED;
			break;

		case eCSR_HI_RSSI_SCAN_DELAY_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("gRoamScanHiRssiDelay %d => %d"),
				nr_config->nhi_rssi_scan_delay,
				val);
			nr_config->nhi_rssi_scan_delay = val;
			nr_info->cfgParams.hi_rssi_scan_delay = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_DELAY_CHANGED;
			break;

		case eCSR_HI_RSSI_SCAN_RSSI_UB_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("gRoamScanHiRssiUpperBound %d => %d"),
				nr_config->nhi_rssi_scan_rssi_ub,
				val);
			nr_config->nhi_rssi_scan_rssi_ub = val;
			nr_info->cfgParams.hi_rssi_scan_rssi_ub = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_UB_CHANGED;
			break;

		default:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				FL("invalid parameter notify_id %d"),
				notify_id);
			status = QDF_STATUS_E_INVAL;
			break;
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
	if (mac_ctx->roam.configParam.isRoamOffloadScanEnabled &&
		status == QDF_STATUS_SUCCESS) {
		csr_roam_offload_scan(mac_ctx, session_id,
			ROAM_SCAN_OFFLOAD_UPDATE_CFG, reason);
	}

	return status;
}

/**
 * sme_update_tgt_services() - update the target services config.
 * @hal: HAL pointer.
 * @cfg: wma_tgt_services parameters.
 *
 * update the target services config.
 *
 * Return: None.
 */
void sme_update_tgt_services(tHalHandle hal, struct wma_tgt_services *cfg)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	mac_ctx->lteCoexAntShare = cfg->lte_coex_ant_share;
	mac_ctx->per_band_chainmask_supp = cfg->per_band_chainmask_supp;
	mac_ctx->beacon_offload = cfg->beacon_offload;
	mac_ctx->pmf_offload = cfg->pmf_offload;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		FL("mac_ctx->pmf_offload: %d"), mac_ctx->pmf_offload);

	return;
}

/**
 * sme_is_session_id_valid() - Check if the session id is valid
 * @hal: Pointer to HAL
 * @session_id: Session id
 *
 * Checks if the session id is valid or not
 *
 * Return: True is the session id is valid, false otherwise
 */
bool sme_is_session_id_valid(tHalHandle hal, uint32_t session_id)
{
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	if (!mac) {
		/* Using QDF_TRACE since mac is not available for sms_log */
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: null mac pointer", __func__);
		return false;
	}

	if (CSR_IS_SESSION_VALID(mac, session_id)) {
		return true;
	} else {
		sms_log(mac, LOGE,
			FL("invalid session id:%d"), session_id);
		return false;
	}
}

#ifdef FEATURE_WLAN_TDLS

/**
 * sme_get_opclass() - determine operating class
 * @hal: Pointer to HAL
 * @channel: channel id
 * @bw_offset: bandwidth offset
 * @opclass: pointer to operating class
 *
 * Function will determine operating class from regdm_get_opclass_from_channel
 *
 * Return: none
 */
void sme_get_opclass(tHalHandle hal, uint8_t channel, uint8_t bw_offset,
		uint8_t *opclass)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	/* redgm opclass table contains opclass for 40MHz low primary,
	 * 40MHz high primary and 20MHz. No support for 80MHz yet. So
	 * first we will check if bit for 40MHz is set and if so find
	 * matching opclass either with low primary or high primary
	 * (a channel would never be in both) and then search for opclass
	 * matching 20MHz, else for any BW.
	 */
	if (bw_offset & (1 << BW_40_OFFSET_BIT)) {
		*opclass = cds_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BW40_LOW_PRIMARY);
		if (!(*opclass)) {
			*opclass = cds_reg_dmn_get_opclass_from_channel(
					mac_ctx->scan.countryCodeCurrent,
					channel, BW40_HIGH_PRIMARY);
		}
	} else if (bw_offset & (1 << BW_20_OFFSET_BIT)) {
		*opclass = cds_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BW20);
	} else {
		*opclass = cds_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BWALL);
	}
}
#endif

#ifdef FEATURE_GREEN_AP
/**
 * sme_send_egap_conf_params() - set the enhanced green ap configuration params
 * @enable: enable/disable the enhanced green ap feature
 * @inactivity_time: inactivity timeout value
 * @wait_time: wait timeout value
 * @flag: feature flag in bitmasp
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_send_egap_conf_params(uint32_t enable, uint32_t inactivity_time,
				     uint32_t wait_time, uint32_t flags)
{
	cds_msg_t message;
	QDF_STATUS status;
	struct egap_conf_params *egap_params;

	egap_params = qdf_mem_malloc(sizeof(*egap_params));
	if (NULL == egap_params) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: fail to alloc egap_params", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	egap_params->enable = enable;
	egap_params->inactivity_time = inactivity_time;
	egap_params->wait_time = wait_time;
	egap_params->flags = flags;

	message.type = WMA_SET_EGAP_CONF_PARAMS;
	message.bodyptr = egap_params;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WMA!", __func__);

		qdf_mem_free(egap_params);
	}
	return status;
}
#endif

/**
 * sme_ht40_stop_obss_scan() - ht40 obss stop scan
 * @hal: mac handel
 * @vdev_id: vdev identifier
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_ht40_stop_obss_scan(tHalHandle hal, uint32_t vdev_id)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wma_ht40_stop_obss_scan(wma_handle, vdev_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_update_mimo_power_save() - Update MIMO power save
 * configuration
 * @hal: The handle returned by macOpen
 * @is_ht_smps_enabled: enable/disable ht smps
 * @ht_smps_mode: smps mode disabled/static/dynamic
 * @send_smps_action: flag to send smps force mode command
 * to FW
 *
 * Return: QDF_STATUS if SME update mimo power save
 * configuration sucsess else failue status
 */
QDF_STATUS sme_update_mimo_power_save(tHalHandle hal,
				      uint8_t is_ht_smps_enabled,
				      uint8_t ht_smps_mode,
				      bool send_smps_action)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	sms_log(mac_ctx, LOG1,
		"SMPS enable: %d mode: %d send action: %d",
		is_ht_smps_enabled, ht_smps_mode,
		send_smps_action);
	mac_ctx->roam.configParam.enableHtSmps =
		is_ht_smps_enabled;
	mac_ctx->roam.configParam.htSmps = ht_smps_mode;
	mac_ctx->roam.configParam.send_smps_action =
		send_smps_action;

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_is_sta_smps_allowed() - check if the supported nss for
 * the session is greater than 1x1 to enable sta SMPS
 * @hal: The handle returned by macOpen
 * @session_id: session id
 *
 * Return: bool returns true if supported nss is greater than
 * 1x1 else false
 */
bool sme_is_sta_smps_allowed(tHalHandle hal, uint8_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	tCsrRoamSession *csr_session;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE,
			"CSR session not valid: %d",
			session_id);
		return false;
	}

	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == csr_session) {
		sms_log(mac_ctx, LOGE,
			"SME session not valid: %d",
			session_id);
		return false;
	}

	return (csr_session->supported_nss_1x1 == true) ? false : true;
}

/**
 * sme_add_beacon_filter() - set the beacon filter configuration
 * @hal: The handle returned by macOpen
 * @session_id: session id
 * @ie_map: bitwise array of IEs
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_add_beacon_filter(tHalHandle hal,
				 uint32_t session_id,
				 uint32_t *ie_map)
{
	cds_msg_t message;
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE,
			"CSR session not valid: %d",
			session_id);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param = qdf_mem_malloc(sizeof(*filter_param));
	if (NULL == filter_param) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: fail to alloc filter_param", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param->vdev_id = session_id;

	qdf_mem_copy(filter_param->ie_map, ie_map,
			BCN_FLT_MAX_ELEMS_IE_LIST * sizeof(uint32_t));

	message.type = WMA_ADD_BCN_FILTER_CMDID;
	message.bodyptr = filter_param;
	qdf_status = cds_mq_post_message(QDF_MODULE_ID_WMA,
					&message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WDA!",
			__func__);

		qdf_mem_free(filter_param);
	}
	return qdf_status;
}

/**
 * sme_remove_beacon_filter() - set the beacon filter configuration
 * @hal: The handle returned by macOpen
 * @session_id: session id
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_remove_beacon_filter(tHalHandle hal, uint32_t session_id)
{
	cds_msg_t message;
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE,
			"CSR session not valid: %d",
			session_id);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param = qdf_mem_malloc(sizeof(*filter_param));
	if (NULL == filter_param) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: fail to alloc filter_param", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param->vdev_id = session_id;

	message.type = WMA_REMOVE_BCN_FILTER_CMDID;
	message.bodyptr = filter_param;
	qdf_status = cds_mq_post_message(QDF_MODULE_ID_WMA,
					&message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WDA!",
			__func__);

		qdf_mem_free(filter_param);
	}
	return qdf_status;
}

/**
 * sme_get_bpf_offload_capabilities() - Get length for BPF offload
 * @hal: Global HAL handle
 * This function constructs the cds message and fill in message type,
 * post the same to WDA.
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_bpf_offload_capabilities(tHalHandle hal)
{
	QDF_STATUS          status     = QDF_STATUS_SUCCESS;
	tpAniSirGlobal      mac_ctx      = PMAC_STRUCT(hal);
	cds_msg_t           cds_msg;

	sms_log(mac_ctx, LOG1, FL("enter"));

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		cds_msg.bodyptr = NULL;
		cds_msg.type = WDA_BPF_GET_CAPABILITIES_REQ;
		status = cds_mq_post_message(QDF_MODULE_ID_WMA, &cds_msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					FL("Post bpf get offload msg fail"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock error"));
	}
	sms_log(mac_ctx, LOG1, FL("exit"));
	return status;
}


/**
 * sme_set_bpf_instructions() - Set BPF bpf filter instructions.
 * @hal: HAL handle
 * @bpf_set_offload: struct to set bpf filter instructions.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_bpf_instructions(tHalHandle hal,
				    struct sir_bpf_set_offload *req)
{
	QDF_STATUS          status     = QDF_STATUS_SUCCESS;
	tpAniSirGlobal      mac_ctx    = PMAC_STRUCT(hal);
	cds_msg_t           cds_msg;
	struct sir_bpf_set_offload *set_offload;

	set_offload = qdf_mem_malloc(sizeof(*set_offload));

	if (NULL == set_offload) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Failed to alloc set_offload"));
		return QDF_STATUS_E_NOMEM;
	}

	set_offload->session_id = req->session_id;
	set_offload->filter_id = req->filter_id;
	set_offload->current_offset = req->current_offset;
	set_offload->total_length = req->total_length;
	set_offload->current_length = req->current_length;
	if (set_offload->total_length) {
		set_offload->program = qdf_mem_malloc(sizeof(uint8_t) *
						req->current_length);
		if (NULL == set_offload->program) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Failed to alloc instruction memory"));
			qdf_mem_free(set_offload);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(set_offload->program, req->program,
				set_offload->current_length);
	}
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		cds_msg.bodyptr = set_offload;
		cds_msg.type = WDA_BPF_SET_INSTRUCTIONS_REQ;
		status = cds_mq_post_message(QDF_MODULE_ID_WMA, &cds_msg);

		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Post BPF set offload msg fail"));
			status = QDF_STATUS_E_FAILURE;
			if (set_offload->total_length)
				qdf_mem_free(set_offload->program);
			qdf_mem_free(set_offload);
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("sme_acquire_global_lock failed"));
		if (set_offload->total_length)
			qdf_mem_free(set_offload->program);
		qdf_mem_free(set_offload);
	}
	return status;
}

/**
 * sme_bpf_offload_register_callback() - Register get bpf offload callbacK
 *
 * @hal - MAC global handle
 * @callback_routine - callback routine from HDD
 *
 * This API is invoked by HDD to register its callback in SME
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_bpf_offload_register_callback(tHalHandle hal,
				void (*pbpf_get_offload_cb)(void *context,
					struct sir_bpf_get_offload *))
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.pbpf_get_offload_cb = pbpf_get_offload_cb;
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed"));
	}
	return status;
}

/**
 * sme_get_wni_dot11_mode() - return configured wni dot11mode
 * @hal: hal pointer
 *
 * Return: wni dot11 mode.
 */
uint32_t sme_get_wni_dot11_mode(tHalHandle hal)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	return csr_translate_to_wni_cfg_dot11_mode(mac_ctx,
		mac_ctx->roam.configParam.uCfgDot11Mode);
}

/**
 * sme_bpf_offload_deregister_callback() - Register get bpf offload callbacK
 *
 * @h_hal - MAC global handle
 * @callback_routine - callback routine from HDD
 *
 * This API is invoked by HDD to de-register its callback in SME
 *
 * Return: QDF_STATUS Enumeration
 */
QDF_STATUS sme_bpf_offload_deregister_callback(tHalHandle h_hal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac;

	if (!h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("hHal is not valid"));
		return QDF_STATUS_E_INVAL;
	}

	mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.pbpf_get_offload_cb = NULL;
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed"));
	}
	return status;
}


/**
 * sme_create_mon_session() - post message to create PE session for monitormode
 * operation
 * @hal_handle: Handle to the HAL
 * @bssid: pointer to bssid
 *
 * Return: QDF_STATUS_SUCCESS on success, non-zero error code on failure.
 */
QDF_STATUS sme_create_mon_session(tHalHandle hal_handle, tSirMacAddr bss_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sir_create_session *msg;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (NULL != msg) {
		msg->type = eWNI_SME_MON_INIT_SESSION;
		msg->msg_len = sizeof(*msg);
		qdf_mem_copy(msg->bss_id.bytes, bss_id, QDF_MAC_ADDR_SIZE);
		status = cds_send_mb_message_to_mac(msg);
	}
	return status;
}

/**
 * sme_set_adaptive_dwelltime_config() - Update Adaptive dwelltime configuration
 * @hal: The handle returned by macOpen
 * @params: adaptive_dwelltime_params config
 *
 * Return: QDF_STATUS if adaptive dwell time update
 * configuration sucsess else failure status
 */
QDF_STATUS sme_set_adaptive_dwelltime_config(tHalHandle hal,
			struct adaptive_dwelltime_params *params)
{
	cds_msg_t message;
	QDF_STATUS status;
	struct adaptive_dwelltime_params *dwelltime_params;

	dwelltime_params = qdf_mem_malloc(sizeof(*dwelltime_params));
	if (NULL == dwelltime_params) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: fail to alloc dwelltime_params", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	dwelltime_params->is_enabled = params->is_enabled;
	dwelltime_params->dwelltime_mode = params->dwelltime_mode;
	dwelltime_params->lpf_weight = params->lpf_weight;
	dwelltime_params->passive_mon_intval = params->passive_mon_intval;
	dwelltime_params->wifi_act_threshold = params->wifi_act_threshold;

	message.type = WMA_SET_ADAPT_DWELLTIME_CONF_PARAMS;
	message.bodyptr = dwelltime_params;
	status = cds_mq_post_message(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WMA!", __func__);

		qdf_mem_free(dwelltime_params);
	}
	return status;
}
/**
 * sme_set_pdev_ht_vht_ies() - sends the set pdev IE req
 * @hal: Pointer to HAL
 * @enable2x2: 1x1 or 2x2 mode.
 *
 * Sends the set pdev IE req with Nss value.
 *
 * Return: None
 */
void sme_set_pdev_ht_vht_ies(tHalHandle hal, bool enable2x2)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct sir_set_ht_vht_cfg *ht_vht_cfg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!mac_ctx->per_band_chainmask_supp)
		return;

	if (!((mac_ctx->roam.configParam.uCfgDot11Mode ==
					eCSR_CFG_DOT11_MODE_AUTO) ||
				(mac_ctx->roam.configParam.uCfgDot11Mode ==
				 eCSR_CFG_DOT11_MODE_11N) ||
				(mac_ctx->roam.configParam.uCfgDot11Mode ==
				 eCSR_CFG_DOT11_MODE_11N_ONLY) ||
				(mac_ctx->roam.configParam.uCfgDot11Mode ==
				 eCSR_CFG_DOT11_MODE_11AC) ||
				(mac_ctx->roam.configParam.uCfgDot11Mode ==
				 eCSR_CFG_DOT11_MODE_11AC_ONLY)))
		return;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		ht_vht_cfg = qdf_mem_malloc(sizeof(*ht_vht_cfg));
		if (NULL == ht_vht_cfg) {
			sms_log(mac_ctx, LOGE,
					"%s: mem alloc failed for ht_vht_cfg",
					__func__);
			sme_release_global_lock(&mac_ctx->sme);
			return;
		}

		ht_vht_cfg->pdev_id = 0;
		if (enable2x2)
			ht_vht_cfg->nss = 2;
		else
			ht_vht_cfg->nss = 1;
		ht_vht_cfg->dot11mode =
			(uint8_t)csr_translate_to_wni_cfg_dot11_mode(mac_ctx,
				mac_ctx->roam.configParam.uCfgDot11Mode);

		ht_vht_cfg->msg_type = eWNI_SME_PDEV_SET_HT_VHT_IE;
		ht_vht_cfg->len = sizeof(*ht_vht_cfg);
		sms_log(mac_ctx, LOG1,
				FL("SET_HT_VHT_IE with nss %d, dot11mode %d"),
				ht_vht_cfg->nss,
				ht_vht_cfg->dot11mode);
		status = cds_send_mb_message_to_mac(ht_vht_cfg);
		if (QDF_STATUS_SUCCESS != status) {
			sms_log(mac_ctx, LOGE, FL(
					"Send SME_PDEV_SET_HT_VHT_IE fail"));
			qdf_mem_free(ht_vht_cfg);
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
	return;
}

/**
 * sme_update_vdev_type_nss() - sets the nss per vdev type
 * @hal: Pointer to HAL
 * @max_supp_nss: max_supported Nss
 * @band: 5G or 2.4G band
 *
 * Sets the per band Nss for each vdev type based on INI and configured
 * chain mask value.
 *
 * Return: None
 */
void sme_update_vdev_type_nss(tHalHandle hal, uint8_t max_supp_nss,
		uint32_t vdev_type_nss, eCsrBand band)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct vdev_type_nss *vdev_nss;

	if (eCSR_BAND_5G == band)
		vdev_nss = &mac_ctx->vdev_type_nss_5g;
	else
		vdev_nss = &mac_ctx->vdev_type_nss_2g;

	vdev_nss->sta = QDF_MIN(max_supp_nss, CFG_STA_NSS(vdev_type_nss));
	vdev_nss->sap = QDF_MIN(max_supp_nss, CFG_SAP_NSS(vdev_type_nss));
	vdev_nss->p2p_go = QDF_MIN(max_supp_nss,
				CFG_P2P_GO_NSS(vdev_type_nss));
	vdev_nss->p2p_cli = QDF_MIN(max_supp_nss,
				CFG_P2P_CLI_NSS(vdev_type_nss));
	vdev_nss->p2p_dev = QDF_MIN(max_supp_nss,
				CFG_P2P_DEV_NSS(vdev_type_nss));
	vdev_nss->ibss = QDF_MIN(max_supp_nss, CFG_IBSS_NSS(vdev_type_nss));
	vdev_nss->tdls = QDF_MIN(max_supp_nss, CFG_TDLS_NSS(vdev_type_nss));
	vdev_nss->ocb = QDF_MIN(max_supp_nss, CFG_OCB_NSS(vdev_type_nss));

	sms_log(mac_ctx, LOG1,
	"band %d NSS:sta %d sap %d cli %d go %d dev %d ibss %d tdls %d ocb %d",
		band, vdev_nss->sta, vdev_nss->sap, vdev_nss->p2p_cli,
		vdev_nss->p2p_go, vdev_nss->p2p_dev, vdev_nss->ibss,
		vdev_nss->tdls, vdev_nss->ocb);
}

/**
 * sme_register_p2p_lo_event() - Register for the p2p lo event
 * @hHal: reference to the HAL
 * @context: the context of the call
 * @callback: the callback to hdd
 *
 * This function registers the callback function for P2P listen
 * offload stop event.
 *
 * Return: none
 */
void sme_register_p2p_lo_event(tHalHandle hHal, void *context,
					p2p_lo_callback callback)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = sme_acquire_global_lock(&pMac->sme);
	pMac->sme.p2p_lo_event_callback = callback;
	pMac->sme.p2p_lo_event_context = context;
	sme_release_global_lock(&pMac->sme);
}

/**
 * sme_process_mac_pwr_dbg_cmd() - enable mac pwr debugging
 * @hal: The handle returned by macOpen
 * @session_id: session id
 * @dbg_args: args for mac pwr debug command
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_process_mac_pwr_dbg_cmd(tHalHandle hal, uint32_t session_id,
				       struct sir_mac_pwr_dbg_cmd*
				       dbg_args)
{
	cds_msg_t message;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct sir_mac_pwr_dbg_cmd *req;
	int i;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sms_log(mac_ctx, LOGE,
			"CSR session not valid: %d",
			session_id);
		return QDF_STATUS_E_FAILURE;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (NULL == req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: fail to alloc mac_pwr_dbg_args", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	req->module_id = dbg_args->module_id;
	req->pdev_id = dbg_args->pdev_id;
	req->num_args = dbg_args->num_args;
	for (i = 0; i < req->num_args; i++)
		req->args[i] = dbg_args->args[i];

	message.type = SIR_HAL_POWER_DBG_CMD;
	message.bodyptr = req;

	if (!QDF_IS_STATUS_SUCCESS(cds_mq_post_message
				(QDF_MODULE_ID_WMA, &message))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post msg to WDA!",
			  __func__);
		qdf_mem_free(req);
	}
	return QDF_STATUS_SUCCESS;
}
/**
 * sme_get_vdev_type_nss() - gets the nss per vdev type
 * @hal: Pointer to HAL
 * @dev_mode: connection type.
 * @nss2g: Pointer to the 2G Nss parameter.
 * @nss5g: Pointer to the 5G Nss parameter.
 *
 * Fills the 2G and 5G Nss values based on connection type.
 *
 * Return: None
 */
void sme_get_vdev_type_nss(tHalHandle hal, enum tQDF_ADAPTER_MODE dev_mode,
		uint8_t *nss_2g, uint8_t *nss_5g)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	csr_get_vdev_type_nss(mac_ctx, dev_mode, nss_2g, nss_5g);
}
