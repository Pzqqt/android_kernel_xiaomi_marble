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

/*
 * DOC: smeApi.c
 *
 * Definitions for SME APIs
 */

/* Include Files */
#include <sir_common.h>
#include <ani_global.h>
#include "sme_api.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "csr_internal.h"
#include "wma_types.h"
#include "wma_if.h"
#include "wma_fips_api.h"
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
#include "csr_api.h"
#include "wlan_reg_services_api.h"
#include <wlan_scan_ucfg_api.h>
#include "wlan_reg_ucfg_api.h"
#include "ol_txrx.h"
#include "wifi_pos_api.h"
#include "net/cfg80211.h"
#include <qca_vendor.h>
#include <wlan_spectral_utils_api.h>

static tSelfRecoveryStats g_self_recovery_stats;

static QDF_STATUS init_sme_cmd_list(tpAniSirGlobal pMac);

static QDF_STATUS sme_handle_change_country_code(tpAniSirGlobal pMac,
						void *pMsgBuf);

static void sme_disconnect_connected_sessions(tpAniSirGlobal pMac);

static QDF_STATUS sme_handle_generic_change_country_code(tpAniSirGlobal pMac,
						  void *pMsgBuf);

static QDF_STATUS sme_process_nss_update_resp(tpAniSirGlobal mac, uint8_t *msg);

#ifdef WLAN_FEATURE_11W
QDF_STATUS sme_unprotected_mgmt_frm_ind(tHalHandle hHal,
					tpSirSmeUnprotMgmtFrameInd pSmeMgmtFrm);
#endif

/* Channel Change Response Indication Handler */
static QDF_STATUS sme_process_channel_change_resp(tpAniSirGlobal pMac,
					   uint16_t msg_type, void *pMsgBuf);

/* Internal SME APIs */
QDF_STATUS sme_acquire_global_lock(tSmeStruct *psSme)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psSme) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_acquire(&psSme->lkSmeGlobalLock)))
			status = QDF_STATUS_SUCCESS;
	}

	return status;
}

QDF_STATUS sme_release_global_lock(tSmeStruct *psSme)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (psSme) {
		if (QDF_IS_STATUS_SUCCESS
			    (qdf_mutex_release(&psSme->lkSmeGlobalLock)))
			status = QDF_STATUS_SUCCESS;
	}

	return status;
}

tpAniSirGlobal sme_get_mac_context(void)
{
	tpAniSirGlobal mac_ctx;
	tHalHandle h_hal;

	h_hal = cds_get_context(QDF_MODULE_ID_SME);
	if (NULL == h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
		FL("invalid h_hal"));
		return NULL;
	}

	mac_ctx = PMAC_STRUCT(h_hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		FL("Invalid MAC context"));
		return NULL;
	}

	return mac_ctx;
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
	tListElem *entry;
	tSmeCmd *command = NULL;
	bool found;
	policy_mgr_pdev_set_hw_mode_cback callback = NULL;
	struct sir_set_hw_mode_resp *param;
	enum sir_conn_update_reason reason;
	struct csr_roam_session *session;
	uint32_t session_id;

	param = (struct sir_set_hw_mode_resp *)msg;
	if (!param) {
		sme_err("HW mode resp param is NULL");
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (!entry) {
		sme_err("No cmd found in active list");
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sme_err("Base address is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_hw_mode != command->command) {
		sme_err("Command mismatch!");
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.set_hw_mode_cmd.set_hw_mode_cb;
	reason = command->u.set_hw_mode_cmd.reason;
	session_id = command->u.set_hw_mode_cmd.session_id;

	sme_debug("reason: %d session: %d",
		command->u.set_hw_mode_cmd.reason,
		command->u.set_hw_mode_cmd.session_id);

	if (!callback) {
		sme_err("Callback does not exist");
		goto end;
	}

	if (!param) {
		sme_err("Callback failed since HW mode params is NULL");
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
			param->vdev_mac_map,
			command->u.set_hw_mode_cmd.next_action,
			command->u.set_hw_mode_cmd.reason,
			command->u.set_hw_mode_cmd.session_id,
			command->u.set_hw_mode_cmd.context);
	if (!CSR_IS_SESSION_VALID(mac, session_id)) {
		sme_err("session %d is invalid", session_id);
		goto end;
	}
	session = CSR_GET_SESSION(mac, session_id);
	if (reason == SIR_UPDATE_REASON_HIDDEN_STA) {
		/* In the case of hidden SSID, connection update
		 * (set hw mode) is done after the scan with reason
		 * code eCsrScanForSsid completes. The connect/failure
		 * needs to be handled after the response of set hw
		 * mode
		 */
		if (param->status == SET_HW_MODE_STATUS_OK) {
			sme_debug("search for ssid success");
			csr_scan_handle_search_for_ssid(mac,
					session_id);
		} else {
			sme_debug("search for ssid failure");
			csr_scan_handle_search_for_ssid_failure(mac,
					session_id);
		}
		csr_saved_scan_cmd_free_fields(mac, session);
	}

end:
	found = csr_nonscan_active_ll_remove_entry(mac, entry,
			LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the avilable command list */
		csr_release_command(mac, command);

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
	struct sir_hw_mode_trans_ind *param;

	param = (struct sir_hw_mode_trans_ind *)msg;
	if (!param) {
		sme_err("HW mode trans ind param is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_hw_mode_transition_cb(param->old_hw_mode_index,
		param->new_hw_mode_index,
		param->num_vdev_mac_entries,
		param->vdev_mac_map, mac->psoc);

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
	uint32_t sme_cmd_ptr_ary_sz;

	pMac->sme.totalSmeCmd = SME_TOTAL_COMMAND;


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

end:
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("Failed to initialize sme command list: %d", status);

	return status;
}

void sme_release_command(tpAniSirGlobal mac_ctx, tSmeCmd *sme_cmd)
{
	sme_cmd->command = eSmeNoCommand;
	csr_ll_insert_tail(&mac_ctx->sme.smeCmdFreeList, &sme_cmd->Link,
			   LL_ACCESS_LOCK);
}

static QDF_STATUS free_sme_cmd_list(tpAniSirGlobal pMac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	csr_ll_close(&pMac->sme.smeCmdFreeList);

	status = qdf_mutex_acquire(&pMac->sme.lkSmeGlobalLock);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("Failed to acquire the lock status: %d", status);
		goto done;
	}

	free_sme_cmds(pMac);

	status = qdf_mutex_release(&pMac->sme.lkSmeGlobalLock);
	if (status != QDF_STATUS_SUCCESS)
		sme_err("Failed to release the lock status: %d", status);
done:
	return status;
}

static void dump_csr_command_info(tpAniSirGlobal pMac, tSmeCmd *pCmd)
{
	switch (pCmd->command) {
	case eSmeCommandRoam:
		sme_debug("roam command reason is %d",
			pCmd->u.roamCmd.roamReason);
		break;

	case eSmeCommandWmStatusChange:
		sme_debug("WMStatusChange command type is %d",
			pCmd->u.wmStatusChangeCmd.Type);
		break;

	default:
		sme_debug("default: Unhandled command %d",
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
		pEntry = csr_nonscan_active_ll_peek_head(pMac, LL_ACCESS_LOCK);
		if (pEntry)
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

		sme_err("Out of command buffer.... command (0x%X) stuck",
			(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
		if (pTempCmd) {
			if (eSmeCsrCommandMask & pTempCmd->command)
				/* CSR command is stuck. See what the reason
				 * code is for that command
				 */
				dump_csr_command_info(pMac, pTempCmd);
		} /* if(pTempCmd) */

		/* dump what is in the pending queue */
		csr_nonscan_pending_ll_lock(pMac);
		pEntry =
			csr_nonscan_pending_ll_peek_head(pMac,
					 LL_ACCESS_NOLOCK);
		while (pEntry && !sme_command_queue_full) {
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
			/* Print only 1st five commands from pending queue. */
			if (idx <= 5)
				sme_err("Out of command buffer.... SME pending command #%d (0x%X)",
					idx, pTempCmd->command);
			idx++;
			if (eSmeCsrCommandMask & pTempCmd->command)
				/* CSR command is stuck. See what the reason
				 * code is for that command
				 */
				dump_csr_command_info(pMac, pTempCmd);
			pEntry = csr_nonscan_pending_ll_next(pMac, pEntry,
					    LL_ACCESS_NOLOCK);
		}
		csr_nonscan_pending_ll_unlock(pMac);

		if (pMac->roam.configParam.enable_fatal_event)
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF,
				false,
				pMac->sme.enableSelfRecovery ? true : false);
		else
			cds_trigger_recovery(QDF_GET_MSG_BUFF_FAILURE);
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

/**
 * sme_ser_handle_active_cmd() - handle command activation callback from
 *					new serialization module
 * @cmd: pointer to new serialization command
 *
 * This API is to handle command activation callback from new serialization
 * callback
 *
 * Return: QDF_STATUS_SUCCESS
 */
static
QDF_STATUS sme_ser_handle_active_cmd(struct wlan_serialization_command *cmd)
{
	tSmeCmd *sme_cmd;
	tHalHandle hal;
	tpAniSirGlobal mac_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool do_continue;

	if (!cmd) {
		sme_err("No serialization command found");
		return QDF_STATUS_E_FAILURE;
	}

	hal = cds_get_context(QDF_MODULE_ID_SME);
	mac_ctx = PMAC_STRUCT(hal);
	if (!mac_ctx) {
		sme_err("No mac_ctx found");
		return QDF_STATUS_E_FAILURE;
	}
	sme_cmd = cmd->umac_cmd;
	if (!sme_cmd) {
		sme_err("No SME command found");
		return QDF_STATUS_E_FAILURE;
	}

	switch (sme_cmd->command) {
	case eSmeCommandRoam:
		status = csr_roam_process_command(mac_ctx, sme_cmd);
		break;
	case eSmeCommandWmStatusChange:
		csr_roam_process_wm_status_change_command(mac_ctx,
					sme_cmd);
		break;
	/*
	 * Treat standby differently here because caller may not be able
	 * to handle the failure so we do our best here
	 */
	case eSmeCommandEnterStandby:
		break;
	case eSmeCommandAddTs:
	case eSmeCommandDelTs:
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
		do_continue = qos_process_command(mac_ctx, sme_cmd);
		if (do_continue)
			status = QDF_STATUS_E_FAILURE;
#endif
		break;
	case e_sme_command_set_hw_mode:
		csr_process_set_hw_mode(mac_ctx, sme_cmd);
		break;
	case e_sme_command_nss_update:
		csr_process_nss_update_req(mac_ctx, sme_cmd);
		break;
	case e_sme_command_set_dual_mac_config:
		csr_process_set_dual_mac_config(mac_ctx, sme_cmd);
		break;
	case e_sme_command_set_antenna_mode:
		csr_process_set_antenna_mode(mac_ctx, sme_cmd);
		break;
	default:
		/* something is wrong */
		sme_err("unknown command %d", sme_cmd->command);
		status = QDF_STATUS_E_FAILURE;
		break;
	}
	return status;
}

QDF_STATUS sme_ser_cmd_callback(void *buf,
				enum wlan_serialization_cb_reason reason)
{
	struct wlan_serialization_command *cmd = buf;
	tHalHandle hal;
	tpAniSirGlobal mac_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *sme_cmd;

	hal = cds_get_context(QDF_MODULE_ID_SME);
	mac_ctx = PMAC_STRUCT(hal);
	if (!mac_ctx) {
		sme_err("mac_ctx is null");
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * Do not acquire lock here as sme global lock is already acquired in
	 * caller or MC thread context
	 */
	if (!cmd) {
		sme_err("serialization command is null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		sme_debug("WLAN_SER_CB_ACTIVATE_CMD callback");
		status = sme_ser_handle_active_cmd(cmd);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		sme_debug("WLAN_SER_CB_CANCEL_CMD callback");
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		sme_debug("WLAN_SER_CB_RELEASE_MEM_CMD callback");
		sme_cmd = cmd->umac_cmd;
		csr_release_command_buffer(mac_ctx, sme_cmd);
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		sme_debug("WLAN_SER_CB_ACTIVE_CMD_TIMEOUT callback");
		break;
	default:
		sme_debug("STOP: unknown reason code");
		return QDF_STATUS_E_FAILURE;
	}
	return status;
}

/**
 * sme_get_sessionid_from_activelist() - gets session id
 * @mac: mac context
 *
 * This function is used to get session id from sme command
 * active list
 *
 * Return: returns session id
 */
static uint32_t sme_get_sessionid_from_activelist(tpAniSirGlobal mac)
{
	tListElem *entry;
	tSmeCmd *command;
	uint32_t session_id = CSR_SESSION_ID_INVALID;

	entry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (entry) {
		command = GET_BASE_ADDR(entry, tSmeCmd, Link);
		session_id = command->sessionId;
	}

	return session_id;
}

/**
 * sme_state_info_dump() - prints state information of sme layer
 * @buf: buffer pointer
 * @size: size of buffer to be filled
 *
 * This function is used to dump state information of sme layer
 *
 * Return: None
 */
static void sme_state_info_dump(char **buf_ptr, uint16_t *size)
{
	uint32_t session_id, active_session_id;
	tHalHandle hal;
	tpAniSirGlobal mac;
	uint16_t len = 0;
	char *buf = *buf_ptr;
	eCsrConnectState connect_state;

	hal = cds_get_context(QDF_MODULE_ID_SME);
	if (hal == NULL) {
		QDF_ASSERT(0);
		return;
	}

	mac = PMAC_STRUCT(hal);

	active_session_id = sme_get_sessionid_from_activelist(mac);
	if (active_session_id != CSR_SESSION_ID_INVALID) {
		len += qdf_scnprintf(buf + len, *size - len,
			"\n active command sessionid %d", active_session_id);
	}

	for (session_id = 0; session_id < CSR_ROAM_SESSION_MAX; session_id++) {
		if (CSR_IS_SESSION_VALID(mac, session_id)) {
			connect_state =
				mac->roam.roamSession[session_id].connectState;
			if ((eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED ==
			     connect_state)
			    || (eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED ==
				connect_state)) {
				len += qdf_scnprintf(buf + len, *size - len,
					"\n NeighborRoamState: %d",
					mac->roam.neighborRoamInfo[session_id].
						neighborRoamState);
				len += qdf_scnprintf(buf + len, *size - len,
					"\n RoamState: %d", mac->roam.
						curState[session_id]);
				len += qdf_scnprintf(buf + len, *size - len,
					"\n RoamSubState: %d", mac->roam.
						curSubState[session_id]);
				len += qdf_scnprintf(buf + len, *size - len,
					"\n ConnectState: %d",
					connect_state);
			}
		}
	}

	*size -= len;
	*buf_ptr += len;
}

/**
 * sme_register_debug_callback() - registration function sme layer
 * to print sme state information
 *
 * Return: None
 */
static void sme_register_debug_callback(void)
{
	qdf_register_debug_callback(QDF_MODULE_ID_SME, &sme_state_info_dump);
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
		sme_err("sme_open failed init lock");
		return  QDF_STATUS_E_FAILURE;
	}
	status = csr_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_open failed, status: %d", status);
		return status;
	}

	status = sme_ps_open(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_ps_open failed with status: %d", status);
		return status;
	}

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Qos open, status: %d", status);
		return status;
	}
#endif
	status = init_sme_cmd_list(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	status = rrm_open(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("rrm_open failed, status: %d", status);
		return status;
	}
	sme_trace_init(pMac);
	sme_register_debug_callback();
	wlan_serialization_legacy_init_callback();

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

/*
 * sme_set11dinfo() - Set the 11d information about valid channels
 *  and there power using information from nvRAM
 *  This function is called only for AP.
 *
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * pSmeConfigParams - a pointer to a caller allocated object of
 *  typedef struct _smeConfigParams.
 *
 * Return QDF_STATUS_SUCCESS - SME update the config parameters successfully.
 *
 *  Other status means SME is failed to update the config parameters.
 */

QDF_STATUS sme_set11dinfo(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SET_11DINFO, NO_SESSION, 0));
	if (NULL == pSmeConfigParams) {
		sme_err("SME config params empty");
		return status;
	}

	status = csr_set_channels(hHal, &pSmeConfigParams->csrConfig);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("csr_set_channels failed with status: %d", status);

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

	sme_info("scan disable %d", value);
	ucfg_scan_set_enable(mac_ctx->psoc, !value);
}
/*
 * sme_get_soft_ap_domain() - Get the current regulatory domain of softAp.
 * This is a synchronous call
 *
 * hHal - The handle returned by HostapdAdapter.
 * v_REGDOMAIN_t - The current Regulatory Domain requested for SoftAp.
 * Return QDF_STATUS_SUCCESS - SME successfully completed the request.
 *  Other status means, failed to get the current regulatory domain.
 */

QDF_STATUS sme_get_soft_ap_domain(tHalHandle hHal, v_REGDOMAIN_t
				*domainIdSoftAp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_GET_SOFTAP_DOMAIN,
			 NO_SESSION, 0));
	if (NULL == domainIdSoftAp) {
		sme_err("Uninitialized domain Id");
		return status;
	}

	*domainIdSoftAp = pMac->scan.domainIdCurrent;
	status = QDF_STATUS_SUCCESS;

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
void sme_update_fine_time_measurement_capab(tHalHandle hal, uint8_t session_id,
						uint32_t val)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	ucfg_wifi_pos_set_ftm_cap(mac_ctx->psoc, val);

	if (!val) {
		mac_ctx->rrm.rrmPEContext.rrmEnabledCaps.fine_time_meas_rpt = 0;
		((tpRRMCaps)mac_ctx->rrm.rrmSmeContext.
			rrmConfig.rm_capability)->fine_time_meas_rpt = 0;
	} else {
		mac_ctx->rrm.rrmPEContext.rrmEnabledCaps.fine_time_meas_rpt = 1;
		((tpRRMCaps)mac_ctx->rrm.rrmSmeContext.
			rrmConfig.rm_capability)->fine_time_meas_rpt = 1;
	}

	/* Inform this RRM IE change to FW */
	csr_roam_offload_scan(mac_ctx, session_id,
			ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			REASON_CONNECT_IES_CHANGED);
}

/*
 * sme_update_config() - Change configurations for all SME moduels
 * The function updates some configuration for modules in SME, CSR, etc
 *  during SMEs close open sequence.
 * Modules inside SME apply the new configuration at the next transaction.
 * This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * pSmeConfigParams - a pointer to a caller allocated object of
 *  typedef struct _smeConfigParams.
 * Return QDF_STATUS_SUCCESS - SME update the config parameters successfully.
 *  Other status means SME is failed to update the config parameters.
 */
QDF_STATUS sme_update_config(tHalHandle hHal, tpSmeConfigParams
				pSmeConfigParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_UPDATE_CONFIG, NO_SESSION,
			 0));
	if (NULL == pSmeConfigParams) {
		sme_err("SME config params empty");
		return status;
	}

	status = csr_change_default_config_param(pMac, &pSmeConfigParams->
						csrConfig);

	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("csr_change_default_config_param failed status: %d",
			status);

	status = rrm_change_default_config_param(hHal, &pSmeConfigParams->
						rrmConfig);

	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("rrm_change_default_config_param failed status: %d",
			status);

	/* For SOC, CFG is set before start We don't want to apply global CFG
	 * in connect state because that may cause some side affect
	 */
	if (csr_is_all_session_disconnected(pMac))
		csr_set_global_cfgs(pMac);

	/*
	 * If scan offload is enabled then lim has allow the sending of
	 * scan request to firmware even in powersave mode. The firmware has
	 * to take care of exiting from power save mode
	 */
	status = sme_cfg_set_int(hHal, WNI_CFG_SCAN_IN_POWERSAVE, true);

	if (QDF_STATUS_SUCCESS != status)
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Could not pass on WNI_CFG_SCAN_IN_POWERSAVE to CFG");

	pMac->snr_monitor_enabled = pSmeConfigParams->snr_monitor_enabled;

	return status;
}

/**
 * sme_update_scan_roam_params() - Update the scan roaming params
 * @mac_ctx: mac ctx
 *
 * Return: void.
 */
static void sme_update_scan_roam_params(tpAniSirGlobal mac_ctx)
{
	struct roam_filter_params scan_params = {0};
	struct roam_ext_params *roam_params_src;
	uint8_t i;
	QDF_STATUS status;

	roam_params_src = &mac_ctx->roam.configParam.roam_params;

	scan_params.num_bssid_avoid_list =
		roam_params_src->num_bssid_avoid_list;

	if (scan_params.num_bssid_avoid_list >
	   MAX_AVOID_LIST_BSSID)
		scan_params.num_bssid_avoid_list =
			MAX_AVOID_LIST_BSSID;

	for (i = 0; i < scan_params.num_bssid_avoid_list; i++) {
		qdf_copy_macaddr(&scan_params.bssid_avoid_list[i],
				&roam_params_src->bssid_avoid_list[i]);
	}

	status = ucfg_scan_update_roam_params(mac_ctx->psoc, &scan_params);
	if (QDF_IS_STATUS_ERROR(status))
		sme_err("ailed to update scan roam params with status=%d",
			status);
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
	uint8_t session_id, struct roam_ext_params *roam_params_src,
	int update_param)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct roam_ext_params *roam_params_dst;
	uint8_t i;

	roam_params_dst = &mac_ctx->roam.configParam.roam_params;
	switch (update_param) {
	case REASON_ROAM_EXT_SCAN_PARAMS_CHANGED:
		roam_params_dst->raise_rssi_thresh_5g =
			roam_params_src->raise_rssi_thresh_5g;
		roam_params_dst->drop_rssi_thresh_5g =
			roam_params_src->drop_rssi_thresh_5g;
		roam_params_dst->raise_factor_5g =
			roam_params_src->raise_factor_5g;
		roam_params_dst->drop_factor_5g =
			roam_params_src->drop_factor_5g;
		roam_params_dst->max_raise_rssi_5g =
			roam_params_src->max_raise_rssi_5g;
		roam_params_dst->max_drop_rssi_5g =
			roam_params_src->max_drop_rssi_5g;
		roam_params_dst->alert_rssi_threshold =
			roam_params_src->alert_rssi_threshold;
		roam_params_dst->is_5g_pref_enabled = true;
		break;
	case REASON_ROAM_SET_SSID_ALLOWED:
		qdf_mem_set(&roam_params_dst->ssid_allowed_list, 0,
				sizeof(tSirMacSSid) * MAX_SSID_ALLOWED_LIST);
		roam_params_dst->num_ssid_allowed_list =
			roam_params_src->num_ssid_allowed_list;
		for (i = 0; i < roam_params_dst->num_ssid_allowed_list; i++) {
			roam_params_dst->ssid_allowed_list[i].length =
				roam_params_src->ssid_allowed_list[i].length;
			qdf_mem_copy(roam_params_dst->ssid_allowed_list[i].ssId,
				roam_params_src->ssid_allowed_list[i].ssId,
				roam_params_dst->ssid_allowed_list[i].length);
		}
		break;
	case REASON_ROAM_SET_FAVORED_BSSID:
		qdf_mem_set(&roam_params_dst->bssid_favored, 0,
			sizeof(tSirMacAddr) * MAX_BSSID_FAVORED);
		roam_params_dst->num_bssid_favored =
			roam_params_src->num_bssid_favored;
		for (i = 0; i < roam_params_dst->num_bssid_favored; i++) {
			qdf_mem_copy(&roam_params_dst->bssid_favored[i],
				&roam_params_src->bssid_favored[i],
				sizeof(tSirMacAddr));
			roam_params_dst->bssid_favored_factor[i] =
				roam_params_src->bssid_favored_factor[i];
		}
		break;
	case REASON_ROAM_SET_BLACKLIST_BSSID:
		qdf_mem_set(&roam_params_dst->bssid_avoid_list, 0,
			QDF_MAC_ADDR_SIZE * MAX_BSSID_AVOID_LIST);
		roam_params_dst->num_bssid_avoid_list =
			roam_params_src->num_bssid_avoid_list;
		for (i = 0; i < roam_params_dst->num_bssid_avoid_list; i++) {
			qdf_copy_macaddr(&roam_params_dst->bssid_avoid_list[i],
					&roam_params_src->bssid_avoid_list[i]);
		}
		break;
	case REASON_ROAM_GOOD_RSSI_CHANGED:
		roam_params_dst->good_rssi_roam =
			roam_params_src->good_rssi_roam;
		break;
	default:
		break;
	}
	csr_roam_offload_scan(mac_ctx, session_id, ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			update_param);

	sme_update_scan_roam_params(mac_ctx);

	return 0;
}

/*
 * sme_process_ready_to_suspend() -
 * On getting ready to suspend indication, this function calls
 * callback registered (HDD callbacks) with SME to inform ready
 * to suspend indication.
 *
 * hHal - Handle returned by mac_open.
 * pReadyToSuspend - Parameter received along with ready to suspend
 *			    indication from WMA.
 * Return: None
 */
static void sme_process_ready_to_suspend(tHalHandle hHal,
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
static void sme_process_ready_to_ext_wow(tHalHandle hHal,
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

/*
 * sme_hdd_ready_ind() - SME sends eWNI_SME_SYS_READY_IND to PE to inform
 *  that the NIC is ready tio run.
 * The function is called by HDD at the end of initialization stage so PE/HAL
 * can enable the NIC to running state.
 * This is a synchronous call
 *
 * @hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - eWNI_SME_SYS_READY_IND is sent to PE
 *				successfully.
 * Other status means SME failed to send the message to PE.
 */
QDF_STATUS sme_hdd_ready_ind(tHalHandle hHal)
{
	tSirSmeReadyReq *msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_HDDREADYIND, NO_SESSION, 0));
	do {

		msg = qdf_mem_malloc(sizeof(*msg));
		if (!msg) {
			sme_err("Memory allocation failed! for msg");
			return QDF_STATUS_E_NOMEM;
		}
		msg->messageType = eWNI_SME_SYS_READY_IND;
		msg->length = sizeof(*msg);
		msg->csr_roam_synch_cb = csr_roam_synch_callback;
		msg->sme_msg_cb = sme_process_msg_callback;

		if (eSIR_FAILURE != u_mac_post_ctrl_msg(hHal, (tSirMbMsg *)
							msg)) {
			status = QDF_STATUS_SUCCESS;
		} else {
			sme_err("u_mac_post_ctrl_msg failed to send eWNI_SME_SYS_READY_IND");
			break;
		}

		status = csr_ready(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("csr_ready failed with status: %d", status);
			break;
		}

		pMac->sme.state = SME_STATE_READY;
	} while (0);

	return status;
}

QDF_STATUS sme_get_valid_channels(uint8_t *chan_list, uint32_t *list_len)
{
	tpAniSirGlobal mac_ctx = sme_get_mac_context();
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	if (eSIR_SUCCESS != wlan_cfg_get_str(mac_ctx,
			WNI_CFG_VALID_CHANNEL_LIST, chan_list, list_len))
		status = QDF_STATUS_E_INVAL;

	return status;
}

/*
 * sme_start() - Put all SME modules at ready state.
 *  The function starts each module in SME, PMC, CSR, etc. . Upon
 *  successfully return, all modules are ready to run.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME is ready.
 * Other status means SME is failed to start
 */
QDF_STATUS sme_start(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct policy_mgr_sme_cbacks sme_cbacks;
	struct spectral_legacy_cbacks spectral_cb;

	do {
		status = csr_start(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("csr_start failed status: %d", status);
			break;
		}
		sme_cbacks.sme_get_nss_for_vdev = sme_get_vdev_type_nss;
		sme_cbacks.sme_get_valid_channels = sme_get_valid_channels;
		sme_cbacks.sme_nss_update_request = sme_nss_update_request;
		sme_cbacks.sme_pdev_set_hw_mode = sme_pdev_set_hw_mode;
		sme_cbacks.sme_pdev_set_pcl = sme_pdev_set_pcl;
		sme_cbacks.sme_soc_set_dual_mac_config =
			sme_soc_set_dual_mac_config;
		sme_cbacks.sme_change_mcc_beacon_interval =
			sme_change_mcc_beacon_interval;
		sme_cbacks.sme_get_ap_channel_from_scan =
			sme_get_ap_channel_from_scan;
		sme_cbacks.sme_scan_result_purge = sme_scan_result_purge;
		status = policy_mgr_register_sme_cb(pMac->psoc, &sme_cbacks);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Failed to register sme cb with Policy Manager: %d",
				status);
			break;
		}
		spectral_cb.vdev_get_chan_freq = sme_get_oper_chan_freq;
		spectral_cb.vdev_get_ch_width = sme_get_oper_ch_width;
		spectral_cb.vdev_get_sec20chan_freq_mhz = sme_get_sec20chan_freq_mhz;
		spectral_register_legacy_cb(pMac->psoc, &spectral_cb);
		pMac->sme.state = SME_STATE_START;

		/* START RRM */
		status = rrm_start(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Failed to start RRM");
			break;
		}
	} while (0);
	return status;
}

static QDF_STATUS dfs_msg_processor(tpAniSirGlobal mac,
		struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info roam_info = { 0 };
	tSirSmeCSAIeTxCompleteRsp *csa_ie_tx_complete_rsp;
	uint32_t session_id = 0;
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;

	switch (msg->type) {
	case eWNI_SME_DFS_RADAR_FOUND:
	{
		session_id = msg->bodyval;
		roam_status = eCSR_ROAM_DFS_RADAR_IND;
		roam_result = eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "sapdfs: Radar indication event occurred");
		break;
	}
	case eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND:
	{
		csa_ie_tx_complete_rsp =
			(tSirSmeCSAIeTxCompleteRsp *) msg->bodyptr;
		if (!csa_ie_tx_complete_rsp) {
			sme_err("eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND null msg");
			return QDF_STATUS_E_FAILURE;
		}
		session_id = csa_ie_tx_complete_rsp->sessionId;
		roam_status = eCSR_ROAM_DFS_CHAN_SW_NOTIFY;
		roam_result = eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_SUCCESS;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND session=%d",
			  session_id);
		break;
	}
	case eWNI_SME_DFS_CAC_COMPLETE:
	{
		session_id = msg->bodyval;
		roam_status = eCSR_ROAM_CAC_COMPLETE_IND;
		roam_result = eCSR_ROAM_RESULT_CAC_END_IND;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "sapdfs: Received eWNI_SME_DFS_CAC_COMPLETE vdevid%d",
			  session_id);
		break;
	}
	default:
	{
		sme_err("Invalid DFS message: 0x%x", msg->type);
		status = QDF_STATUS_E_FAILURE;
		return status;
	}
	}

	/* Indicate Radar Event to SAP */
	csr_roam_call_callback(mac, session_id, &roam_info, 0,
			       roam_status, roam_result);
	return status;
}


#ifdef WLAN_FEATURE_11W
/*
 * Handle the unprotected management frame indication from LIM and
 * forward it to HDD.
 */
QDF_STATUS sme_unprotected_mgmt_frm_ind(tHalHandle hHal,
					tpSirSmeUnprotMgmtFrameInd pSmeMgmtFrm)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info roam_info = { 0 };
	uint32_t SessionId = pSmeMgmtFrm->sessionId;

	roam_info.nFrameLength = pSmeMgmtFrm->frameLen;
	roam_info.pbFrames = pSmeMgmtFrm->frameBuf;
	roam_info.frameType = pSmeMgmtFrm->frameType;

	/* forward the mgmt frame to HDD */
	csr_roam_call_callback(pMac, SessionId, &roam_info, 0,
			       eCSR_ROAM_UNPROT_MGMT_FRAME_IND, 0);

	return status;
}
#endif

QDF_STATUS sme_update_new_channel_event(tHalHandle hal, uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct csr_roam_info *roamInfo;
	eRoamCmdStatus roamStatus;
	eCsrRoamResult roamResult;

	roamInfo = qdf_mem_malloc(sizeof(*roamInfo));
	if (!roamInfo) {
		sme_err("mem alloc failed for roam info");
		return QDF_STATUS_E_FAILURE;
	}
	roamInfo->dfs_event.sessionId = session_id;

	roamStatus = eCSR_ROAM_CHANNEL_COMPLETE_IND;
	roamResult = eCSR_ROAM_RESULT_DFS_RADAR_FOUND_IND;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "sapdfs: Updated new channel event");

	/* Indicate channel Event to SAP */
	csr_roam_call_callback(mac, session_id, roamInfo, 0,
			       roamStatus, roamResult);

	qdf_mem_free(roamInfo);
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
	struct csr_roam_info roamInfo = {0};
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;

	ext_chan_ind = msg_buf;
	if (NULL == ext_chan_ind) {
		sme_err("ext_chan_ind is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	session_id = ext_chan_ind->session_id;
	roamInfo.target_channel = ext_chan_ind->new_channel;
	roam_status = eCSR_ROAM_EXT_CHG_CHNL_IND;
	roam_result = eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND;
	sme_debug("sapdfs: Received eWNI_SME_EXT_CHANGE_CHANNEL_IND for session id [%d]",
		 session_id);

	/* Indicate Ext Channel Change event to SAP */
	csr_roam_call_callback(mac_ctx, session_id, &roamInfo, 0,
					roam_status, roam_result);
	return status;
}

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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: ESE Mode is already enabled or disabled, nothing to do (returning) old(%d) new(%d)",
			  __func__,
			  pMac->roam.configParam.isEseIniFeatureEnabled,
			  isEseIniFeatureEnabled);
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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
	uint8_t ch_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t count, valid_count = 0;
	struct scheduler_msg msg = {0};
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac,
					pPlmReq->sessionId);

	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	if (!pSession) {
		sme_err("session %d not found",	pPlmReq->sessionId);
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
			if (CHANNEL_STATE_DFS == wlan_reg_get_channel_state(
						pMac->pdev,
						pPlmReq->plmChList[count])) {
				/* DFS channel is provided, no PLM bursts can be
				 * transmitted. Ignoring these channels.
				 */
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_DEBUG,
					  FL("DFS channel %d ignored for PLM"),
					  pPlmReq->plmChList[count]);
				continue;
			}
		} else if (!ret) {
			/* Not supported, ignore the channel */
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_msg(QDF_MODULE_ID_WMA,
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
	struct csr_roam_info roam_info = { 0 };
	uint32_t SessionId = pSmeTsmIeInd->sessionId;

	roam_info.tsmIe.tsid = pSmeTsmIeInd->tsmIe.tsid;
	roam_info.tsmIe.state = pSmeTsmIeInd->tsmIe.state;
	roam_info.tsmIe.msmt_interval = pSmeTsmIeInd->tsmIe.msmt_interval;
	/* forward the tsm ie information to HDD */
	csr_roam_call_callback(pMac, SessionId, &roam_info, 0,
			       eCSR_ROAM_TSM_IE_IND, 0);
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
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tpSirBeaconReportReqInd pSmeBcnReportReq = NULL;
	tCsrEseBeaconReqParams *pBeaconReq = NULL;
	uint8_t counter = 0;
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);
	tpRrmSMEContext pSmeRrmContext = &pMac->rrm.rrmSmeContext;

	if (pSmeRrmContext->eseBcnReqInProgress == true) {
		sme_err("A Beacon Report Req is already in progress");
		return QDF_STATUS_E_RESOURCES;
	}

	/* Store the info in RRM context */
	qdf_mem_copy(&pSmeRrmContext->eseBcnReqInfo, pEseBcnReq,
		     sizeof(tCsrEseBeaconReq));

	/* Prepare the request to send to SME. */
	pSmeBcnReportReq = qdf_mem_malloc(sizeof(tSirBeaconReportReqInd));
	if (NULL == pSmeBcnReportReq) {
		sme_err("Memory Allocation Failure!!! ESE  BcnReq Ind to SME");
		return QDF_STATUS_E_NOMEM;
	}

	pSmeRrmContext->eseBcnReqInProgress = true;

	sme_debug("Sending Beacon Report Req to SME");

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

	qdf_mem_free(pSmeBcnReportReq);

	return status;
}

/**
 * sme_get_tsm_stats() - SME get tsm stats
 * @hHal: HAL handle
 * @callback: SME sends back the requested stats using the callback
 * @staId: The station ID for which the stats is requested for
 * @bssId: bssid
 * @pContext: user context to be passed back along with the callback
 * @tid: Traffic id
 *
 * API register a callback to get TSM Stats.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_tsm_stats(tHalHandle hHal,
			     tCsrTsmStatsCallback callback,
			     uint8_t staId, struct qdf_mac_addr bssId,
			     void *pContext, uint8_t tid)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_tsm_stats(pMac, callback,
					   staId, bssId, pContext,
					   tid);
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
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	tpCsrChannelInfo curchnl_list_info = NULL;
	uint8_t oldChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[128] = { 0 };
	uint8_t i = 0, j = 0;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
	curchnl_list_info =
		&pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo;

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

static
QDF_STATUS sme_ibss_peer_info_response_handler(tHalHandle hHal,
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
		sme_err("Dual mac config resp param is NULL");
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */
	}

	entry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (!entry) {
		sme_err("No cmd found in active list");
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sme_err("Base address is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_dual_mac_config != command->command) {
		sme_err("Command mismatch!");
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.set_dual_mac_cmd.set_dual_mac_cb;
	if (callback) {
		if (!param) {
			sme_err("Callback failed-Dual mac config is NULL");
		} else {
			sme_debug("Calling HDD callback for Dual mac config");
			callback(param->status,
				command->u.set_dual_mac_cmd.scan_config,
				command->u.set_dual_mac_cmd.fw_mode_config);
		}
	} else {
		sme_err("Callback does not exist");
	}

	found = csr_nonscan_active_ll_remove_entry(mac, entry, LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the available command list */
		csr_release_command(mac, command);

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
	if (!param)
		sme_err("set antenna mode resp is NULL");
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */

	entry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (!entry) {
		sme_err("No cmd found in active list");
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sme_err("Base address is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_set_antenna_mode != command->command) {
		sme_err("Command mismatch!");
		return QDF_STATUS_E_FAILURE;
	}

	callback =
		command->u.set_antenna_mode_cmd.set_antenna_mode_resp;
	if (callback) {
		if (!param)
			sme_err("Set antenna mode call back is NULL");
		else
			callback(param->status);
	} else {
		sme_err("Callback does not exist");
	}

	found = csr_nonscan_active_ll_remove_entry(mac, entry, LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the available command list */
		csr_release_command(mac, command);

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_process_msg() - The main message processor for SME.
 *  The function is called by a message dispatcher when to process a message
 *  targeted for SME.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * pMsg - A pointer to a caller allocated object of tSirMsgQ.
 * Return QDF_STATUS_SUCCESS - SME successfully process the message.
 * Other status means SME failed to process the message to HAL.
 */
QDF_STATUS sme_process_msg(tHalHandle hHal, struct scheduler_msg *pMsg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct sir_peer_info *peer_stats;
	struct sir_peer_info_resp *peer_info_rsp;

	if (pMsg == NULL) {
		sme_err("Empty message for SME");
		return status;
	}
	status = sme_acquire_global_lock(&pMac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_warn("Locking failed, bailing out");
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		return status;
	}
	if (!SME_IS_START(pMac)) {
		sme_debug("message type %d in stop state ignored", pMsg->type);
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
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_NEIGHBOR_REPORT_IND:
	case eWNI_SME_BEACON_REPORT_REQ_IND:
		if (pMsg->bodyptr) {
			status = sme_rrm_msg_processor(pMac, pMsg->type,
						       pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_ADD_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_add_sta_session_rsp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_DEL_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_del_sta_session_rsp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_CHANGE_COUNTRY_CODE:
		if (pMsg->bodyptr) {
			status = sme_handle_change_country_code((void *)pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE:
		if (pMsg->bodyptr) {
			status = sme_handle_generic_change_country_code(
						(void *)pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;

#ifdef WLAN_FEATURE_11W
	case eWNI_SME_UNPROT_MGMT_FRM_IND:
		if (pMsg->bodyptr) {
			sme_unprotected_mgmt_frm_ind(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_ESE
	case eWNI_SME_TSM_IE_IND:
		if (pMsg->bodyptr) {
			sme_tsm_ie_ind(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif /* FEATURE_WLAN_ESE */
	case eWNI_SME_ROAM_SCAN_OFFLOAD_RSP:
		status = csr_roam_offload_scan_rsp_hdlr((void *)pMac,
							pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_IBSS_PEER_INFO_RSP:
		if (pMsg->bodyptr) {
			sme_ibss_peer_info_response_handler(pMac,
							    pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_READY_TO_SUSPEND_IND:
		if (pMsg->bodyptr) {
			sme_process_ready_to_suspend(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	case eWNI_SME_READY_TO_EXTWOW_IND:
		if (pMsg->bodyptr) {
			sme_process_ready_to_ext_wow(pMac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case eWNI_SME_AUTO_SHUTDOWN_IND:
		if (pMac->sme.pAutoShutdownNotificationCb) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  FL("Auto shutdown notification"));
			pMac->sme.pAutoShutdownNotificationCb();
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_SME_DFS_RADAR_FOUND:
	case eWNI_SME_DFS_CAC_COMPLETE:
	case eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND:
		status = dfs_msg_processor(pMac, pMsg);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_CHANNEL_CHANGE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_channel_change_resp(pMac,
								 pMsg->type,
								 pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#ifdef WLAN_FEATURE_STATS_EXT
	case eWNI_SME_STATS_EXT_EVENT:
		if (pMsg->bodyptr) {
			status = sme_stats_ext_event(hHal, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif
	case eWNI_SME_GET_PEER_INFO_IND:
		if (pMac->sme.pget_peer_info_ind_cb)
			pMac->sme.pget_peer_info_ind_cb(pMsg->bodyptr,
				pMac->sme.pget_peer_info_cb_context);
		if (pMsg->bodyptr) {
			peer_info_rsp = (struct sir_peer_info_resp *)
							(pMsg->bodyptr);
			peer_stats = (struct sir_peer_info *)
							(peer_info_rsp->info);
			if (peer_stats) {
				pMac->peer_rssi = peer_stats[0].rssi;
				pMac->peer_txrate = peer_stats[0].tx_rate;
				pMac->peer_rxrate = peer_stats[0].rx_rate;
			}
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_GET_PEER_INFO_EXT_IND:
		if (pMac->sme.pget_peer_info_ext_ind_cb)
			pMac->sme.pget_peer_info_ext_ind_cb(pMsg->bodyptr,
				pMac->sme.pget_peer_info_ext_cb_context);
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
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_WMA_MSG,
				 NO_SESSION, pMsg->type));
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
			if (pMac->sme.linkStatusCallback)
				pMac->sme.linkStatusCallback(
					pLinkStatus->linkStatus,
					pMac->sme.linkStatusContext);

			pMac->sme.linkStatusCallback = NULL;
			pMac->sme.linkStatusContext = NULL;
			qdf_mem_free(pLinkStatus);
		}
		break;
	}
	case eWNI_SME_MSG_GET_TEMPERATURE_IND:
		if (pMac->sme.pGetTemperatureCb)
			pMac->sme.pGetTemperatureCb(pMsg->bodyval,
					pMac->sme.pTemperatureCbContext);
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
			sme_err("callback not registered to process: %d",
				pMsg->type);

		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_EPNO_NETWORK_FOUND_IND:
		if (pMac->sme.pExtScanIndCb)
			pMac->sme.pExtScanIndCb(pMac->hHdd,
					eSIR_EPNO_NETWORK_FOUND_IND,
					pMsg->bodyptr);
		else
			sme_err("callback not registered to process: %d",
				pMsg->type);

		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_SME_SET_HW_MODE_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_set_hw_mode_resp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_HW_MODE_TRANS_IND:
		if (pMsg->bodyptr) {
			status = sme_process_hw_mode_trans_ind(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_NSS_UPDATE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_nss_update_resp(pMac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_OCB_SET_CONFIG_RSP:
		if (pMac->sme.ocb_set_config_callback)
			pMac->sme.ocb_set_config_callback(
				pMac->sme.ocb_set_config_context,
				pMsg->bodyptr);
		else
			sme_err("No callback for Msg type: %d", pMsg->type);
		pMac->sme.ocb_set_config_callback = NULL;
		pMac->sme.ocb_set_config_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_OCB_GET_TSF_TIMER_RSP:
		if (pMac->sme.ocb_get_tsf_timer_callback)
			pMac->sme.ocb_get_tsf_timer_callback(
				pMac->sme.ocb_get_tsf_timer_context,
				pMsg->bodyptr);
		else
			sme_err("No callback for Msg type: %d", pMsg->type);
		pMac->sme.ocb_get_tsf_timer_callback = NULL;
		pMac->sme.ocb_get_tsf_timer_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_GET_STATS_RSP:
		if (pMac->sme.dcc_get_stats_callback)
			pMac->sme.dcc_get_stats_callback(
				pMac->sme.dcc_get_stats_context,
				pMsg->bodyptr);
		else
			sme_err("No callback for Msg type: %d", pMsg->type);
		pMac->sme.dcc_get_stats_callback = NULL;
		pMac->sme.dcc_get_stats_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_UPDATE_NDL_RSP:
		if (pMac->sme.dcc_update_ndl_callback)
			pMac->sme.dcc_update_ndl_callback(
				pMac->sme.dcc_update_ndl_context,
				pMsg->bodyptr);
		else
			sme_err("No callback for Msg type: %d", pMsg->type);
		pMac->sme.dcc_update_ndl_callback = NULL;
		pMac->sme.dcc_update_ndl_context = NULL;
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_DCC_STATS_EVENT:
		if (pMac->sme.dcc_stats_event_callback)
			pMac->sme.dcc_stats_event_callback(
				pMac->sme.dcc_stats_event_context,
				pMsg->bodyptr);
		else
			sme_err("No callback for Msg type: %d", pMsg->type);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_SET_DUAL_MAC_CFG_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_dual_mac_config_resp(pMac,
					pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
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
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_LOST_LINK_INFO_IND:
		if (pMac->sme.lost_link_info_cb)
			pMac->sme.lost_link_info_cb(pMac->hHdd,
				(struct sir_lost_link_info *)pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_RSO_CMD_STATUS_IND:
		if (pMac->sme.rso_cmd_status_cb)
			pMac->sme.rso_cmd_status_cb(pMac->hHdd, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWMI_SME_LL_STATS_IND:
		if (pMac->sme.link_layer_stats_ext_cb)
			pMac->sme.link_layer_stats_ext_cb(pMac->hHdd,
							  pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_BT_ACTIVITY_INFO_IND:
		if (pMac->sme.bt_activity_info_cb)
			pMac->sme.bt_activity_info_cb(pMac->hHdd,
						      pMsg->bodyval);
		break;
	default:

		if ((pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN)
		    && (pMsg->type <= eWNI_SME_MSG_TYPES_END)) {
			/* CSR */
			if (pMsg->bodyptr) {
				status = csr_msg_processor(hHal, pMsg->bodyptr);
				qdf_mem_free(pMsg->bodyptr);
			} else
				sme_err("Empty message for: %d", pMsg->type);
		} else {
			sme_warn("Unknown message type: %d", pMsg->type);
			if (pMsg->bodyptr)
				qdf_mem_free(pMsg->bodyptr);
		}
	} /* switch */
release_lock:
	sme_release_global_lock(&pMac->sme);
	return status;
}

QDF_STATUS sme_mc_process_handler(struct scheduler_msg *msg)
{
	tpAniSirGlobal mac_ctx = cds_get_context(QDF_MODULE_ID_SME);

	if (mac_ctx == NULL) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return sme_process_msg((tHalHandle)mac_ctx, msg);
}

/**
 * sme_process_nss_update_resp() - Process nss update response
 * @mac: Global MAC pointer
 * @msg: nss update response
 *
 * Processes the nss update response and invokes the HDD
 * callback to process further
 */
static QDF_STATUS sme_process_nss_update_resp(tpAniSirGlobal mac, uint8_t *msg)
{
	tListElem *entry = NULL;
	tSmeCmd *command = NULL;
	bool found;
	policy_mgr_nss_update_cback callback = NULL;
	struct sir_beacon_tx_complete_rsp *param;

	param = (struct sir_beacon_tx_complete_rsp *)msg;
	if (!param)
		sme_err("nss update resp param is NULL");
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */

	entry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (!entry) {
		sme_err("No cmd found in active list");
		return QDF_STATUS_E_FAILURE;
	}

	command = GET_BASE_ADDR(entry, tSmeCmd, Link);
	if (!command) {
		sme_err("Base address is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (e_sme_command_nss_update != command->command) {
		sme_err("Command mismatch!");
		return QDF_STATUS_E_FAILURE;
	}

	callback = command->u.nss_update_cmd.nss_update_cb;
	if (callback) {
		if (!param)
			sme_err("Callback failed since nss update params is NULL");
		else
			callback(command->u.nss_update_cmd.context,
				param->tx_status,
				param->session_id,
				command->u.nss_update_cmd.next_action,
				command->u.nss_update_cmd.reason);
	} else {
		sme_err("Callback does not exisit");
	}

	found = csr_nonscan_active_ll_remove_entry(mac, entry, LL_ACCESS_LOCK);
	if (found) {
		/* Now put this command back on the avilable command list */
		csr_release_command(mac, command);
	}

	return QDF_STATUS_SUCCESS;
}

/* No need to hold the global lock here because this function can only be
 * called after sme_stop.
 */
void sme_free_msg(tHalHandle hHal, struct scheduler_msg *pMsg)
{
	if (pMsg) {
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
	}

}

/*
 * sme_stop() - Stop all SME modules and put them at idle state
 *
 *  The function stops each module in SME, PMC, CSR, etc. . Upon
 *  return, all modules are at idle state ready to start.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * tHalStopType - reason for stopping
 * Return QDF_STATUS_SUCCESS - SME is stopped.
 *  Other status means SME is failed to stop but caller should still
 *  consider SME is stopped.
 */
QDF_STATUS sme_stop(tHalHandle hHal, tHalStopType stopType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS fail_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = rrm_stop(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("rrm_stop failed with status: %d", status);
	}

	status = csr_stop(pMac, stopType);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_stop failed with status: %d", status);
		fail_status = status;
	}

	if (!QDF_IS_STATUS_SUCCESS(fail_status))
		status = fail_status;

	pMac->sme.state = SME_STATE_STOP;

	return status;
}

/*
 * sme_close() - Release all SME modules and their resources.
 * The function release each module in SME, PMC, CSR, etc. . Upon
 *  return, all modules are at closed state.
 *
 *  No SME APIs can be involved after smeClose except smeOpen.
 *  smeClose must be called before mac_close.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return QDF_STATUS_SUCCESS - SME is successfully close.
 *
 *  Other status means SME is failed to be closed but caller still cannot
 *  call any other SME functions except smeOpen.
 */
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
		sme_err("csr_close failed with status: %d", status);
		fail_status = status;
	}
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_close(pMac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Qos close failed with status: %d", status);
		fail_status = status;
	}
#endif
	status = sme_ps_close(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_ps_close failed status: %d", status);
		fail_status = status;
	}

	status = rrm_close(hHal);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("RRM close failed with status: %d", status);
		fail_status = status;
	}

	free_sme_cmd_list(pMac);

	if (!QDF_IS_STATUS_SUCCESS
		    (qdf_mutex_destroy(&pMac->sme.lkSmeGlobalLock)))
		fail_status = QDF_STATUS_E_FAILURE;

	if (!QDF_IS_STATUS_SUCCESS(fail_status))
		status = fail_status;

	pMac->sme.state = SME_STATE_STOP;

	return status;
}

/**
 * sme_remove_bssid_from_scan_list() - wrapper to remove the bssid from
 * scan list
 * @hal: hal context.
 * @bssid: bssid to be removed
 *
 * This function remove the given bssid from scan list.
 *
 * Return: QDF status.
 */
QDF_STATUS sme_remove_bssid_from_scan_list(tHalHandle hal,
	tSirMacAddr bssid)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_remove_bssid_from_scan_list(mac_ctx, bssid);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}


/*
 * sme_scan_get_result
 * A wrapper function to request scan results from CSR.
 * This is a synchronous call
 *
 * pFilter - If pFilter is NULL, all cached results are returned
 * phResult - an object for the result.
 * Return QDF_STATUS
 */
QDF_STATUS sme_scan_get_result(tHalHandle hHal, uint8_t sessionId,
			       tCsrScanResultFilter *pFilter,
			       tScanResultHandle *phResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_GET_RESULTS, sessionId,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_get_result(hHal, pFilter, phResult);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

QDF_STATUS sme_scan_get_result_for_bssid(tHalHandle hal_handle,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_get_result_for_bssid(hal_handle, bssid, res);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

/**
 * sme_get_ap_channel_from_scan() - a wrapper function to get
 *				  AP's channel id from
 *				  CSR by filtering the
 *				  result which matches
 *				  our roam profile.
 * @profile: SAP profile
 * @ap_chnl_id: pointer to channel id of SAP. Fill the value after finding the
 *              best ap from scan cache.
 *
 * This function is written to get AP's channel id from CSR by filtering
 * the result which matches our roam profile. This is a synchronous call.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS sme_get_ap_channel_from_scan(void *profile,
					tScanResultHandle *scan_cache,
					uint8_t *ap_chnl_id)
{
	return sme_get_ap_channel_from_scan_cache((tCsrRoamProfile *)
						  profile,
						  scan_cache,
						  ap_chnl_id);
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
QDF_STATUS sme_get_ap_channel_from_scan_cache(
	tCsrRoamProfile *profile, tScanResultHandle *scan_cache,
	uint8_t *ap_chnl_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = sme_get_mac_context();
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
	}
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
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		status = csr_scan_get_result(mac_ctx, scan_filter,
					  &filtered_scan_result);
		if (QDF_STATUS_SUCCESS == status) {
			csr_get_bssdescr_from_scan_handle(filtered_scan_result,
					&first_ap_profile);
			*scan_cache = filtered_scan_result;
			if (0 != first_ap_profile.channelId) {
				*ap_chnl_id = first_ap_profile.channelId;
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_DEBUG,
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
					scan_cache, roam_id, session_id))
			ret_status = false;
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
					session_id))
			ret_status = false;
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

/*
 * sme_scan_flush_result() -
 * A wrapper function to request CSR to clear scan results.
 * This is a synchronous call
 *
 * Return QDF_STATUS
 */
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

/*
 * sme_filter_scan_results() -
 * A wrapper function to request CSR to clear scan results.
 *   This is a synchronous call
 *
 * tHalHandle - HAL context handle
 * sessionId - session id
 * Return QDF_STATUS
 */
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

/*
 * sme_scan_result_get_first() -
 * A wrapper function to request CSR to returns the first element of
 * scan result.
 * This is a synchronous call
 *
 * hScanResult - returned from csr_scan_get_result
 * Return tCsrScanResultInfo * - NULL if no result
 */
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

/*
 * sme_scan_result_get_next() -
 * A wrapper function to request CSR to returns the next element of
 * scan result. It can be called without calling csr_scan_result_get_first first
 *   This is a synchronous call
 *
 * hScanResult - returned from csr_scan_get_result
 * Return Null if no result or reach the end
 */
tCsrScanResultInfo *sme_scan_result_get_next(tHalHandle hHal,
					     tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrScanResultInfo *pRet = NULL;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pRet = csr_scan_result_get_next(pMac, hScanResult);
		sme_release_global_lock(&pMac->sme);
	}

	return pRet;
}

/*
 * sme_scan_result_purge() -
 * A wrapper function to request CSR to remove all items(tCsrScanResult)
 * in the list and free memory for each item
 *   This is a synchronous call
 *
 * hScanResult - returned from csr_scan_get_result. hScanResult is
 *	considered gone by
 *  calling this function and even before this function reutrns.
 * Return QDF_STATUS
 */
QDF_STATUS sme_scan_result_purge(tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = sme_get_mac_context();

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_RESULT_PURGE,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_result_purge(mac_ctx, hScanResult);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

/*
 * sme_scan_get_pmkid_candidate_list() -
 * A wrapper function to return the PMKID candidate list
 *   This is a synchronous call
 *
 * pPmkidList - caller allocated buffer point to an array of
 *			tPmkidCandidateInfo
 * pNumItems - pointer to a variable that has the number of
 *		       tPmkidCandidateInfo allocated when retruning, this is
 *		       either the number needed or number of items put into
 *		       pPmkidList
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough and pNumItems
 *			 has the number of tPmkidCandidateInfo.
 *  \Note: pNumItems is a number of tPmkidCandidateInfo,
 *	   not sizeof(tPmkidCandidateInfo) * something
 */
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

/*
 * sme_get_channel_bonding_mode5_g() - get the channel bonding mode for 5G band
 *
 * hHal - HAL handle
 * mode - channel bonding mode
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_channel_bonding_mode5_g(tHalHandle hHal, uint32_t *mode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeConfigParams *smeConfig;

	if (!mode) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: invalid mode", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	smeConfig = qdf_mem_malloc(sizeof(*smeConfig));
	if (!smeConfig) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: failed to alloc smeConfig", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (sme_get_config_param(pMac, smeConfig) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: sme_get_config_param failed", __func__);
		qdf_mem_free(smeConfig);
		return QDF_STATUS_E_FAILURE;
	}

	*mode = smeConfig->csrConfig.channelBondingMode5GHz;
	qdf_mem_free(smeConfig);

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_get_channel_bonding_mode24_g() - get the channel bonding mode for 2.4G
 * band
 *
 * hHal - HAL handle
 * mode - channel bonding mode
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_channel_bonding_mode24_g(tHalHandle hHal, uint32_t *mode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeConfigParams *smeConfig;

	if (!mode) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: invalid mode", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	smeConfig = qdf_mem_malloc(sizeof(*smeConfig));
	if (!smeConfig) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: failed to alloc smeConfig", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	if (sme_get_config_param(pMac, smeConfig) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: sme_get_config_param failed", __func__);
		qdf_mem_free(smeConfig);
		return QDF_STATUS_E_FAILURE;
	}

	*mode = smeConfig->csrConfig.channelBondingMode24GHz;
	qdf_mem_free(smeConfig);

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_roam_connect() -
 * A wrapper function to request CSR to inititiate an association
 *   This is an asynchronous call.
 *
 * sessionId - the sessionId returned by sme_open_session.
 * pProfile - description of the network to which to connect
 * hBssListIn - a list of BSS descriptor to roam to. It is returned
 *			from csr_scan_get_result
 * pRoamId - to get back the request ID
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_connect(tHalHandle hHal, uint8_t sessionId,
			    tCsrRoamProfile *pProfile, uint32_t *pRoamId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (!pMac)
		return QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_CONNECT, sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			status =
				csr_roam_connect(pMac, sessionId, pProfile,
						 pRoamId);
		} else {
			sme_err("Invalid sessionID: %d", sessionId);
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		sme_err("sme_acquire_global_lock failed");
	}

	return status;
}

/*
 * sme_set_phy_mode() -
 * Changes the PhyMode.
 *
 * hHal - The handle returned by mac_open.
 * phyMode new phyMode which is to set
 * Return QDF_STATUS  SUCCESS.
 */
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

/*
 * sme_roam_reassoc() -
 * A wrapper function to request CSR to inititiate a re-association
 *
 * pProfile - can be NULL to join the currently connected AP. In that
 * case modProfileFields should carry the modified field(s) which could trigger
 * reassoc
 * modProfileFields - fields which are part of tCsrRoamConnectedProfile
 *   that might need modification dynamically once STA is up & running and this
 *   could trigger a reassoc
 * pRoamId - to get back the request ID
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_reassoc(tHalHandle hHal, uint8_t sessionId,
			    tCsrRoamProfile *pProfile,
			    tCsrRoamModifyProfileFields modProfileFields,
			    uint32_t *pRoamId, bool fForce)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_REASSOC, sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			if ((NULL == pProfile) && (fForce == 1))
				status = csr_reassoc(pMac, sessionId,
						     &modProfileFields, pRoamId,
						     fForce);
			else
				status = csr_roam_reassoc(pMac, sessionId,
						pProfile,
						modProfileFields, pRoamId);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_connect_to_last_profile() -
 * A wrapper function to request CSR to disconnect and reconnect with
 *	the same profile
 *   This is an asynchronous call.
 *
 * Return QDF_STATUS. It returns fail if currently connected
 */
QDF_STATUS sme_roam_connect_to_last_profile(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_GET_CONNECTPROFILE,
			 sessionId, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_connect_to_last_profile(pMac,
								sessionId);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_disconnect() -
 * A wrapper function to request CSR to disconnect from a network
 *   This is an asynchronous call.
 *
 * reason -- To indicate the reason for disconnecting. Currently, only
 *		     eCSR_DISCONNECT_REASON_MIC_ERROR is meanful.
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_disconnect(tHalHandle hHal, uint8_t sessionId,
			       eCsrRoamDisconnectReason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DISCONNECT, sessionId,
			 reason));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_disconnect(pMac, sessionId, reason);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/* sme_dhcp_done_ind() - send dhcp done ind
 * @hal: hal context
 * @session_id: session id
 *
 * Return: void.
 */
void sme_dhcp_done_ind(tHalHandle hal, uint8_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct csr_roam_session *session;

	if (!mac_ctx)
		return;

	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sme_err("Session: %d not found", session_id);
		return;
	}
	session->dhcp_done = true;
}

/*
 * sme_roam_stop_bss() -
 * To stop BSS for Soft AP. This is an asynchronous API.
 *
 * hHal - Global structure
 * sessionId - sessionId of SoftAP
 * Return QDF_STATUS  SUCCESS  Roam callback will be called to indicate
 * actual results
 */
QDF_STATUS sme_roam_stop_bss(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_issue_stop_bss_cmd(pMac, sessionId,
							false);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_roam_disconnect_sta() - disassociate a station
 * @hHal:          Global structure
 * @sessionId:     SessionId of SoftAP
 * @p_del_sta_params: Pointer to parameters of the station to disassoc
 *
 * To disassociate a station. This is an asynchronous API.
 *
 * Return: QDF_STATUS_SUCCESS on success.Roam callback will
 *         be called to indicate actual result.
 */
QDF_STATUS sme_roam_disconnect_sta(tHalHandle hHal, uint8_t sessionId,
				   struct csr_del_sta_params *p_del_sta_params)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_issue_disassociate_sta_cmd(pMac,
					sessionId, p_del_sta_params);
		else
			status = QDF_STATUS_E_INVAL;
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
			       struct csr_del_sta_params *pDelStaParams)
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status =
				csr_roam_issue_deauth_sta_cmd(pMac, sessionId,
							      pDelStaParams);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_get_associated_stas() -
 * To probe the list of associated stations from various modules
 *	 of CORE stack.
 * This is an asynchronous API.
 *
 * sessionId    - sessionId of SoftAP
 * modId        - Module from whom list of associtated stations is
 *			  to be probed. If an invalid module is passed then
 *			  by default QDF_MODULE_ID_PE will be probed.
 * pUsrContext  - Opaque HDD context
 * pfnSapEventCallback  - Sap event callback in HDD
 * pAssocBuf    - Caller allocated memory to be filled with associatd
 *			  stations info
 * Return QDF_STATUS
 */
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status =
				csr_roam_get_associated_stas(pMac, sessionId,
							modId,
							pUsrContext,
							pfnSapEventCallback,
							pAssocStasBuf);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_get_connect_state() -
 * A wrapper function to request CSR to return the current connect state
 *	of Roaming
 * This is a synchronous call.
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_get_connect_state(tHalHandle hHal, uint8_t sessionId,
				      eCsrConnectState *pState)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_get_connect_state(pMac, sessionId,
							pState);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_get_connect_profile() -
 * A wrapper function to request CSR to return the current connect
 * profile. Caller must call csr_roam_free_connect_profile after it is done
 * and before reuse for another csr_roam_get_connect_profile call.
 * This is a synchronous call.
 *
 * pProfile - pointer to a caller allocated structure
 *		      tCsrRoamConnectedProfile
 * eturn QDF_STATUS. Failure if not connected
 */
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_get_connect_profile(pMac, sessionId,
							pProfile);
		else
			status = QDF_STATUS_E_INVAL;
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

/*
 * sme_roam_set_pmkid_cache() -
 * A wrapper function to request CSR to return the PMKID candidate list
 * This is a synchronous call.

 * pPMKIDCache - caller allocated buffer point to an array of
 *			 tPmkidCacheInfo
 * numItems - a variable that has the number of tPmkidCacheInfo
 *		      allocated when retruning, this is either the number needed
 *		      or number of items put into pPMKIDCache
 * update_entire_cache - this bool value specifies if the entire pmkid
 *				 cache should be overwritten or should it be
 *				 updated entry by entry.
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough and pNumItems has the number of
 *			 tPmkidCacheInfo.
 *   \Note: pNumItems is a number of tPmkidCacheInfo,
 *	   not sizeof(tPmkidCacheInfo) * something
 */
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_set_pmkid_cache(pMac, sessionId,
						pPMKIDCache,
						numItems, update_entire_cache);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

QDF_STATUS sme_roam_del_pmkid_from_cache(tHalHandle hHal, uint8_t sessionId,
					 tPmkidCacheInfo *pmksa,
					 bool flush_cache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DEL_PMKIDCACHE,
			 sessionId, flush_cache));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_del_pmkid_from_cache(pMac, sessionId,
						       pmksa, flush_cache);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*
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
 */
QDF_STATUS sme_roam_set_psk_pmk(tHalHandle hHal, uint8_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_set_psk_pmk(pMac, sessionId, pPSK_PMK,
						     pmk_len);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}
#endif
/*
 * sme_roam_get_security_req_ie() -
 * A wrapper function to request CSR to return the WPA or RSN or WAPI IE CSR
 * passes to PE to JOIN request or START_BSS request
 * This is a synchronous call.
 *
 * pLen - caller allocated memory that has the length of pBuf as input.
 *		  Upon returned, *pLen has the needed or IE length in pBuf.
 * pBuf - Caller allocated memory that contain the IE field, if any,
 *		  upon return
 * secType - Specifies whether looking for WPA/WPA2/WAPI IE
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough
 */
QDF_STATUS sme_roam_get_security_req_ie(tHalHandle hHal, uint8_t sessionId,
					uint32_t *pLen, uint8_t *pBuf,
					eCsrSecurityType secType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_get_wpa_rsn_req_ie(hHal, sessionId,
								pLen, pBuf);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_roam_get_security_rsp_ie() -
 * A wrapper function to request CSR to return the WPA or RSN or
 * WAPI IE from the beacon or probe rsp if connected
 * This is a synchronous call.
 *
 * pLen - caller allocated memory that has the length of pBuf as input.
 *		  Upon returned, *pLen has the needed or IE length in pBuf.
 * pBuf - Caller allocated memory that contain the IE field, if any,
 *		  upon return
 * secType - Specifies whether looking for WPA/WPA2/WAPI IE
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough
 */
QDF_STATUS sme_roam_get_security_rsp_ie(tHalHandle hHal, uint8_t sessionId,
					uint32_t *pLen, uint8_t *pBuf,
					eCsrSecurityType secType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_get_wpa_rsn_rsp_ie(pMac, sessionId,
							pLen, pBuf);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/*
 * sme_roam_get_num_pmkid_cache() -
 * A wrapper function to request CSR to return number of PMKID cache
 *	entries
 *   This is a synchronous call.
 *
 * Return uint32_t - the number of PMKID cache entries
 */
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
		} else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return numPmkidCache;
}

/*
 * sme_roam_get_pmkid_cache() -
 * A wrapper function to request CSR to return PMKID cache from CSR
 * This is a synchronous call.
 *
 * pNum - caller allocated memory that has the space of the number of
 *		  pBuf tPmkidCacheInfo as input. Upon returned, *pNum has the
 *		  needed or actually number in tPmkidCacheInfo.
 * pPmkidCache - Caller allocated memory that contains PMKID cache, if
 *			 any, upon return
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough
 */
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_roam_get_pmkid_cache(pMac, sessionId, pNum,
							 pPmkidCache);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_get_config_param() -
 * A wrapper function that HDD calls to get the global settings
 *	currently maintained by CSR.
 * This is a synchronous call.
 *
 * pParam - caller allocated memory
 * Return QDF_STATUS
 */
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
			sme_err("csr_get_config_param failed");
			sme_release_global_lock(&pMac->sme);
			return status;
		}
		qdf_mem_copy(&pParam->rrmConfig,
				&pMac->rrm.rrmSmeContext.rrmConfig,
				sizeof(pMac->rrm.rrmSmeContext.rrmConfig));
		pParam->snr_monitor_enabled = pMac->snr_monitor_enabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_cfg_set_int() - Sets the cfg parameter value.
 * @hal:	Handle to hal.
 * @cfg_id:	Configuration parameter ID.
 * @value:	value to be saved in the cfg parameter.
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
 *		will be saved.
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

/*
 * sme_get_modify_profile_fields() -
 * HDD or SME - QOS calls this function to get the current values of
 * connected profile fields, changing which can cause reassoc.
 * This function must be called after CFG is downloaded and STA is in connected
 * state. Also, make sure to call this function to get the current profile
 * fields before calling the reassoc. So that pModifyProfileFields will have
 * all the latest values plus the one(s) has been updated as part of reassoc
 * request.
 *
 * pModifyProfileFields - pointer to the connected profile fields
 *   changing which can cause reassoc
 * Return QDF_STATUS
 */
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
		if (CSR_IS_SESSION_VALID(pMac, sessionId))
			status = csr_get_modify_profile_fields(pMac, sessionId,
							  pModifyProfileFields);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_set_dhcp_till_power_active_flag() -
 * Sets/Clears DHCP related flag to disable/enable auto PS
 *
 * hal - The handle returned by mac_open.
 */
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

/*
 * sme_register11d_scan_done_callback() -
 * Register a routine of type csr_scan_completeCallback which is
 *	called whenever an 11d scan is done
 *
 * hHal - The handle returned by mac_open.
 * callback -  11d scan complete routine to be registered
 * Return QDF_STATUS
 */
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
#endif

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
	struct csr_roam_session *session = NULL;
	struct ps_global_info *ps_global_info = &mac_ctx->sme.ps_global_info;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_HDD_SET_KEY,
			 session_id, 0));
	if (set_key->keyLength > CSR_MAX_KEY_LEN) {
		sme_err("Invalid key length: %d", set_key->keyLength);
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

	sme_debug("keyLength: %d", set_key->keyLength);

	sme_debug("Session_id: %d roam_id: %d", session_id, roam_id);
	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sme_err("session %d not found", session_id);
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
	struct scheduler_msg msg = {0};
	struct wep_update_default_key_idx *update_key;

	update_key = qdf_mem_malloc(sizeof(*update_key));
	if (!update_key) {
		sme_err("Failed to allocate memory for update key");
		return QDF_STATUS_E_NOMEM;
	}

	update_key->session_id = session_id;
	update_key->default_idx = default_idx;

	msg.type = WMA_UPDATE_WEP_DEFAULT_KEY;
	msg.reserved = 0;
	msg.bodyptr = (void *)update_key;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Failed to post WMA_UPDATE_WEP_DEFAULT_KEY to WMA");
		qdf_mem_free(update_key);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * sme_get_rssi() - API to retrieve current RSSI
 * @hHal: HAL handle for device
 * @callback: SME sends back the requested stats using the callback
 * @staId: The station ID for which the RSSI is requested for
 * @bssid: The bssid of the connected session
 * @lastRSSI: RSSI value at time of request. In case fw cannot provide
 *		      RSSI, do not hold up but return this value.
 * @pContext: user context to be passed back along with the callback
 *
 * A wrapper function that client calls to register a callback to get RSSI
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_rssi(tHalHandle hHal,
			tCsrRssiCallback callback, uint8_t staId,
			struct qdf_mac_addr bssId, int8_t lastRSSI,
			void *pContext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_RSSI, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_rssi(pMac, callback,
				      staId, bssId, lastRSSI,
				      pContext);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_get_snr() -
 * A wrapper function that client calls to register a callback to get SNR
 *
 * callback - SME sends back the requested stats using the callback
 * staId - The station ID for which the stats is requested for
 * pContext - user context to be passed back along with the callback
 * p_cds_context - cds context
 *   \return QDF_STATUS
 */
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

/*
 * sme_get_statistics() -
 * A wrapper function that client calls to register a callback to get
 *   different PHY level statistics from CSR.
 *
 * requesterId - different client requesting for statistics,
 *	HDD, UMA/GAN etc
 * statsMask - The different category/categories of stats requester
 *	is looking for
 * callback - SME sends back the requested stats using the callback
 * periodicity - If requester needs periodic update in millisec, 0 means
 *			 it's an one time request
 * cache - If requester is happy with cached stats
 * staId - The station ID for which the stats is requested for
 * pContext - user context to be passed back along with the callback
 * sessionId - sme session interface
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_statistics(tHalHandle hHal,
			      eCsrStatsRequesterType requesterId,
			      uint32_t statsMask, tCsrStatsCallback callback,
			      uint8_t staId, void *pContext, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_statistics(pMac, requesterId, statsMask,
					    callback, staId, pContext,
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
	struct scheduler_msg message = {0};

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

		message.type = WMA_LINK_STATUS_GET_REQ;
		message.bodyptr = pMsg;
		message.reserved = 0;

		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message))) {
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

/*
 * sme_get_country_code() -
 * To return the current country code. If no country code is applied,
 * default country code is used to fill the buffer.
 * If 11d supported is turned off, an error is return and the last
 * applied/default country code is used.
 * This is a synchronous API.
 *
 * pBuf - pointer to a caller allocated buffer for returned country code.
 * pbLen  For input, this parameter indicates how big is the buffer.
 *		  Upon return, this parameter has the number of bytes for
 *		  country. If pBuf doesn't have enough space, this function
 *		  returns fail status and this parameter contains the number
 *		  that is needed.
 *
 * Return QDF_STATUS  SUCCESS.
 *
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_get_country_code(tHalHandle hHal, uint8_t *pBuf, uint8_t *pbLen)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_CNTRYCODE, NO_SESSION, 0));

	return csr_get_country_code(pMac, pBuf, pbLen);
}

/* some support functions */
bool sme_is11d_supported(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return wlan_reg_11d_enabled_on_host(pMac->psoc);
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

/*
 * sme_change_country_code() -
 * Change Country code from upperlayer during WLAN driver operation.
 * This is a synchronous API.
 *
 * hHal - The handle returned by mac_open.
 * pCountry New Country Code String
 * sendRegHint If we want to send reg hint to nl80211
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_change_country_code(tHalHandle hHal,
				   tSmeChangeCountryCallback callback,
				   uint8_t *pCountry,
				   void *pContext,
				   void *p_cds_context,
				   bool countryFromUserSpace,
				   bool sendRegHint)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg msg = {0};
	tAniChangeCountryCodeReq *pMsg;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CHANGE_CNTRYCODE, NO_SESSION,
			 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if ((pMac->roam.configParam.Is11dSupportEnabledOriginal == true)
		    && (!pMac->roam.configParam.
			fSupplicantCountryCodeHasPriority)) {

			sme_warn("Set Country Code Fail since the STA is associated and userspace does not have priority");

			sme_release_global_lock(&pMac->sme);
			status = QDF_STATUS_E_FAILURE;
			return status;
		}

		pMsg = qdf_mem_malloc(sizeof(tAniChangeCountryCodeReq));
		if (NULL == pMsg) {
			sme_err("csrChangeCountryCode: failed to allocate mem for req");
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
		    scheduler_post_msg(QDF_MODULE_ID_SME, &msg)) {
			sme_err("sme_change_country_code failed to post msg to self");
			qdf_mem_free((void *)pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_generic_change_country_code() -
 * Change Country code from upperlayer during WLAN driver operation.
 * This is a synchronous API.
 *
 * hHal - The handle returned by mac_open.
 * pCountry New Country Code String
 * reg_domain regulatory domain
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_generic_change_country_code(tHalHandle hHal,
					   uint8_t *pCountry)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg msg = {0};
	tAniGenericChangeCountryCodeReq *pMsg;

	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: pMac is null", __func__);
		return status;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMsg = qdf_mem_malloc(sizeof(tAniGenericChangeCountryCodeReq));

		if (NULL == pMsg) {
			sme_err("sme_generic_change_country_code: failed to allocate mem for req");
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
		    scheduler_post_msg(QDF_MODULE_ID_SME, &msg)) {
			sme_err("sme_generic_change_country_code failed to post msg to self");
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_dhcp_start_ind() -
 * API to signal the FW about the DHCP Start event.
 *
 * hHal - HAL handle for device.
 * device_mode - mode(AP,SAP etc) of the device.
 * macAddr - MAC address of the adapter.
 * sessionId - session ID.
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_dhcp_start_ind(tHalHandle hHal,
			      uint8_t device_mode,
			      uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	tAniDHCPInd *pMsg;
	struct csr_roam_session *pSession;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sme_err("Session: %d not found", sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		pSession->dhcp_done = false;

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

		message.type = WMA_DHCP_START_IND;
		message.bodyptr = pMsg;
		message.reserved = 0;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 sessionId, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_dhcp_stop_ind() -
 * API to signal the FW about the DHCP complete event.
 *
 * hHal - HAL handle for device.
 * device_mode - mode(AP, SAP etc) of the device.
 * macAddr - MAC address of the adapter.
 * sessionId - session ID.
 * Return QDF_STATUS  SUCCESS.
 *			FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_dhcp_stop_ind(tHalHandle hHal,
			     uint8_t device_mode,
			     uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	tAniDHCPInd *pMsg;
	struct csr_roam_session *pSession;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(pMac, sessionId);

		if (!pSession) {
			sme_err("Session: %d not found", sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		pSession->dhcp_done = true;

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

		message.type = WMA_DHCP_STOP_IND;
		message.bodyptr = pMsg;
		message.reserved = 0;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 sessionId, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_TXFailMonitorStopInd() -
 * API to signal the FW to start monitoring TX failures
 *
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_tx_fail_monitor_start_stop_ind(tHalHandle hHal, uint8_t
						tx_fail_count,
					      void *txFailIndCallback)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
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

		message.type = WMA_TX_FAIL_MONITOR_IND;
		message.bodyptr = pMsg;
		message.reserved = 0;

		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_set_cfg_privacy() -
 * API to set configure privacy parameters
 *
 * hHal - The handle returned by mac_open.
 * pProfile - Pointer CSR Roam profile.
 * fPrivacy - This parameter indicates status of privacy
 * Return void
 */
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

/*
 * sme_neighbor_report_request() -
 * API to request neighbor report.
 *
 * hHal - The handle returned by mac_open.
 * pRrmNeighborReq - Pointer to a caller allocated object of type
 *			      tRrmNeighborReq. Caller owns the memory and is
 *			      responsible for freeing it.
 * Return QDF_STATUS
 *	    QDF_STATUS_E_FAILURE - failure
 *	    QDF_STATUS_SUCCESS  success
 */
QDF_STATUS sme_neighbor_report_request(tHalHandle hHal, uint8_t sessionId,
				       tpRrmNeighborReq pRrmNeighborReq,
				      tpRrmNeighborRspCallbackInfo callbackInfo)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_NEIGHBOR_REPORTREQ, NO_SESSION,
			 0));

	if (pRrmNeighborReq->neighbor_report_offload) {
		status = csr_invoke_neighbor_report_request(sessionId,
							    pRrmNeighborReq,
							    false);
		return status;
	}

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		status =
			sme_rrm_neighbor_report_request(hHal, sessionId,
						pRrmNeighborReq, callbackInfo);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_get_wcnss_wlan_compiled_version() -
 * This API returns the version of the WCNSS WLAN API with
 *	which the HOST driver was built
 *
 * hHal - The handle returned by mac_open.
 * pVersion - Points to the Version structure to be filled
 * Return QDF_STATUS
 *	    QDF_STATUS_E_INVAL - failure
 *	    QDF_STATUS_SUCCESS  success
 */
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

/*
 * sme_get_wcnss_wlan_reported_version() -
 * This API returns the version of the WCNSS WLAN API with
 *	which the WCNSS driver reports it was built
 * hHal - The handle returned by mac_open.
 * pVersion - Points to the Version structure to be filled
 * Return QDF_STATUS
 *	    QDF_STATUS_E_INVAL - failure
 *	    QDF_STATUS_SUCCESS  success
 */
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

/*
 * sme_get_wcnss_software_version() -
 * This API returns the version string of the WCNSS driver
 *
 * hHal - The handle returned by mac_open.
 * pVersion - Points to the Version string buffer to be filled
 * versionBufferSize - THe size of the Version string buffer
 * Return QDF_STATUS
 *	    QDF_STATUS_E_INVAL - failure
 *	    QDF_STATUS_SUCCESS  success
 */
QDF_STATUS sme_get_wcnss_software_version(tHalHandle hHal,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		if (pVersion != NULL)
			status =
				wma_get_wcnss_software_version(
							    pVersion,
							    versionBufferSize);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_get_wcnss_hardware_version() -
 * This API returns the version string of the WCNSS hardware
 *
 * hHal - The handle returned by mac_open.
 * pVersion - Points to the Version string buffer to be filled
 * versionBufferSize - THe size of the Version string buffer
 * Return QDF_STATUS
 *	    QDF_STATUS_E_INVAL - failure
 *	    QDF_STATUS_SUCCESS  success
 */
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

/*
 * sme_scan_get_bkid_candidate_list() -
 * A wrapper function to return the BKID candidate list
 *
 * pBkidList - caller allocated buffer point to an array of
 *		       tBkidCandidateInfo
 * pNumItems - pointer to a variable that has the number of
 *		       tBkidCandidateInfo allocated when retruning, this is
 *		       either the number needed or number of items put into
 *		       pPmkidList
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough and pNumItems
 *			 has the number of tBkidCandidateInfo.
 * Note: pNumItems is a number of tBkidCandidateInfo,
 *	   not sizeof(tBkidCandidateInfo) * something
 */
QDF_STATUS sme_scan_get_bkid_candidate_list(tHalHandle hHal, uint32_t sessionId,
					    tBkidCandidateInfo *pBkidList,
					    uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status =
			csr_scan_get_bkid_candidate_list(pMac, sessionId,
							pBkidList, pNumItems);
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
	struct oem_data_req *oem_data_req;
	void *wma_handle;

	SME_ENTER();
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sme_err("wma_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	oem_data_req = qdf_mem_malloc(sizeof(*oem_data_req));
	if (!oem_data_req) {
		sme_err("mem alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	oem_data_req->data_len = hdd_oem_req->data_len;
	oem_data_req->data = qdf_mem_malloc(oem_data_req->data_len);
	if (!oem_data_req->data) {
		sme_err("mem alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(oem_data_req->data, hdd_oem_req->data,
		     oem_data_req->data_len);

	status = wma_start_oem_data_req(wma_handle, oem_data_req);

	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("Post oem data request msg fail");
	else
		sme_debug("OEM request(length: %d) sent to WMA",
			  oem_data_req->data_len);

	if (oem_data_req->data_len)
		qdf_mem_free(oem_data_req->data);
	qdf_mem_free(oem_data_req);

	SME_EXIT();
	return status;
}
#endif /*FEATURE_OEM_DATA_SUPPORT */

QDF_STATUS sme_open_session(tHalHandle hal, struct sme_session_params *params)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct cdp_pdev *pdev;
	ol_txrx_peer_handle peer;
	uint8_t peer_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: type=%d, session_id %d subType=%d addr:%pM",
		  __func__, params->type_of_persona,
		  params->sme_session_id, params->subtype_of_persona,
		  params->self_mac_addr);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to get pdev handler", __func__);
		return status;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	peer = cdp_peer_find_by_addr(soc, pdev, params->self_mac_addr,
				     &peer_id);
	if (peer) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Peer=%d exist with same MAC",
			  __func__, peer_id);
		status = QDF_STATUS_E_INVAL;
	} else {
		status = csr_roam_open_session(mac_ctx, params);
	}
	sme_release_global_lock(&mac_ctx->sme);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_HDD_OPEN_SESSION,
			 params->sme_session_id, 0));

	return status;
}

QDF_STATUS sme_close_session(tHalHandle hal, uint8_t session_id)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CLOSE_SESSION, session_id, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_close_session(pMac, session_id, false);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_change_mcc_beacon_interval() -
 * To update P2P-GO beaconInterval. This function should be called after
 *    disassociating all the station is done
 *   This is an asynchronous API.
 *
 * @sessionId: Session Identifier
 * Return QDF_STATUS  SUCCESS
 *			FAILURE or RESOURCES
 *			The API finished and failed.
 */
QDF_STATUS sme_change_mcc_beacon_interval(uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = sme_get_mac_context();

	if (!mac_ctx) {
		sme_err("mac_ctx is NULL");
		return status;
	}
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_send_chng_mcc_beacon_interval(mac_ctx,
							   sessionId);
		sme_release_global_lock(&mac_ctx->sme);
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

/*
 * sme_set_keep_alive() -
 * API to set the Keep Alive feature.
 *
 * hHal - The handle returned by mac_open.
 * request -  Pointer to the Keep Alive request.
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_keep_alive(tHalHandle hHal, uint8_t session_id,
			      tpSirKeepAliveReq request)
{
	tpSirKeepAliveReq request_buf;
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, session_id);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("WMA_SET_KEEP_ALIVE message"));

	if (pSession == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Session not Found"));
		return QDF_STATUS_E_FAILURE;
	}
	request_buf = qdf_mem_malloc(sizeof(tSirKeepAliveReq));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to allocate memory for keep alive request");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&request->bssid, &pSession->connectedProfile.bssid);
	qdf_mem_copy(request_buf, request, sizeof(tSirKeepAliveReq));

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"buff TP %d input TP %d ", request_buf->timePeriod,
		  request->timePeriod);
	request_buf->sessionId = session_id;

	msg.type = WMA_SET_KEEP_ALIVE;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 session_id, msg.type));
	if (QDF_STATUS_SUCCESS !=
			scheduler_post_msg(QDF_MODULE_ID_WMA, &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to post WMA_SET_KEEP_ALIVE message to WMA");
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_get_operation_channel() -
 * API to get current channel on which STA is parked his function gives
 * channel information only of infra station or IBSS station
 *
 * hHal, pointer to memory location and sessionId
 * Returns QDF_STATUS_SUCCESS
 *	     QDF_STATUS_E_FAILURE
 */
QDF_STATUS sme_get_operation_channel(tHalHandle hHal, uint32_t *pChannel,
				     uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct csr_roam_session *pSession;

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

	if (QDF_STATUS_SUCCESS ==
			sme_acquire_global_lock(&mac_ctx->sme)) {
		msg = qdf_mem_malloc(sizeof(*msg));
		if (NULL == msg) {
			sme_err("Not able to allocate memory");
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		msg->message_type = eWNI_SME_REGISTER_MGMT_FRAME_CB;
		msg->length          = sizeof(*msg);

		msg->callback = callback;
		status = umac_send_mb_message_to_mac(msg);
		sme_release_global_lock(&mac_ctx->sme);
		return status;
	}
	return QDF_STATUS_E_FAILURE;
}

/*
 * sme_RegisterMgtFrame() -
 * To register managment frame of specified type and subtype.
 *
 * frameType - type of the frame that needs to be passed to HDD.
 * matchData - data which needs to be matched before passing frame
 *		       to HDD.
 * matchDataLen - Length of matched data.
 * Return QDF_STATUS
 */
QDF_STATUS sme_register_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
				   uint16_t frameType, uint8_t *matchData,
				   uint16_t matchLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tSirRegisterMgmtFrame *pMsg;
		uint16_t len;
		struct csr_roam_session *pSession = CSR_GET_SESSION(pMac,
							sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sme_err("Session %d not found",	sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) &&
						!pSession->sessionActive) {
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
			pMsg->messageType = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
			pMsg->length = len;
			pMsg->sessionId = sessionId;
			pMsg->registerFrame = true;
			pMsg->frameType = frameType;
			pMsg->matchLen = matchLen;
			qdf_mem_copy(pMsg->matchData, matchData, matchLen);
			status = umac_send_mb_message_to_mac(pMsg);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_DeregisterMgtFrame() -
 * To De-register managment frame of specified type and subtype.
 *
 * frameType - type of the frame that needs to be passed to HDD.
 * matchData - data which needs to be matched before passing frame
 *		       to HDD.
 * matchDataLen - Length of matched data.
 * Return QDF_STATUS
 */
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
		struct csr_roam_session *pSession = CSR_GET_SESSION(pMac,
							sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sme_err("Session %d not found",	sessionId);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) &&
						!pSession->sessionActive) {
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
			pMsg->messageType = eWNI_SME_REGISTER_MGMT_FRAME_REQ;
			pMsg->length = len;
			pMsg->registerFrame = false;
			pMsg->frameType = frameType;
			pMsg->matchLen = matchLen;
			qdf_mem_copy(pMsg->matchData, matchData, matchLen);
			status = umac_send_mb_message_to_mac(pMsg);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/**
 * sme_prepare_mgmt_tx() - Prepares mgmt frame
 * @hal: The handle returned by mac_open
 * @session_id: session id
 * @buf: pointer to frame
 * @len: frame length
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_prepare_mgmt_tx(tHalHandle hal, uint8_t session_id,
			   const uint8_t *buf, uint32_t len)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sir_mgmt_msg *msg;
	uint16_t msg_len;
	struct scheduler_msg sch_msg = {0};

	sme_debug("prepares auth frame");

	msg_len = sizeof(*msg) + len;
	msg = qdf_mem_malloc(msg_len);
	if (msg == NULL) {
		status = QDF_STATUS_E_NOMEM;
	} else {
		msg->type = eWNI_SME_SEND_MGMT_FRAME_TX;
		msg->msg_len = msg_len;
		msg->session_id = session_id;
		msg->data = (uint8_t *)msg + sizeof(*msg);
		qdf_mem_copy(msg->data, buf, len);

		sch_msg.type = eWNI_SME_SEND_MGMT_FRAME_TX;
		sch_msg.bodyptr = msg;
		status = scheduler_post_msg(QDF_MODULE_ID_PE, &sch_msg);
	}
	return status;
}

QDF_STATUS sme_send_mgmt_tx(tHalHandle hal, uint8_t session_id,
			   const uint8_t *buf, uint32_t len)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SEND_MGMT_TX, session_id, 0));

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = sme_prepare_mgmt_tx(hal, session_id, buf, len);
		sme_release_global_lock(&mac->sme);
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
	struct scheduler_msg message = {0};
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
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_EXT_WOW;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_configure_app_type1_params() -
 * SME will pass this request to lower mac to configure Indoor WoW parameters.
 *
 * hHal - The handle returned by mac_open.
 * wlanAppType1Params- Depicts the wlan App Type 1(Indoor) params
 * Return QDF_STATUS
 */
QDF_STATUS sme_configure_app_type1_params(tHalHandle hHal,
					  tpSirAppType1Params wlanAppType1Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
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
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_SET_APP_TYPE1_PARAMS;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_configure_app_type2_params() -
 * SME will pass this request to lower mac to configure Indoor WoW parameters.
 *
 * hHal - The handle returned by mac_open.
 * wlanAppType2Params- Depicts the wlan App Type 2 (Outdoor) params
 * Return QDF_STATUS
 */
QDF_STATUS sme_configure_app_type2_params(tHalHandle hHal,
					 tpSirAppType2Params wlanAppType2Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
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
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_SET_APP_TYPE2_PARAMS;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_get_infra_session_id
 * To get the session ID for infra session, if connected
 *   This is a synchronous API.
 *
 * hHal - The handle returned by mac_open.
 * sessionid, -1 if infra session is not connected
 */
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

/*
 * sme_get_infra_operation_channel() -
 * To get the operating channel for infra session, if connected
 *   This is a synchronous API.
 *
 * hHal - The handle returned by mac_open.
 * sessionId - the sessionId returned by sme_open_session.
 * Return operating channel, 0 if infra session is not connected
 */
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

/* This routine will return poerating channel on which other BSS is operating
 * to be used for concurrency mode. If other BSS is not up or not connected it
 * will return 0
 */
uint8_t sme_get_concurrent_operation_channel(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t channel = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		channel = csr_get_concurrent_operation_channel(pMac);
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			"%s: Other Concurrent Channel: %d", __func__, channel);
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
			csr_check_concurrent_channel_overlap(pMac, sap_ch,
								sapPhyMode,
							     cc_switch_mode);
		sme_release_global_lock(&pMac->sme);
	}

	return channel;
}
#endif

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
	struct scheduler_msg tsf_msg = {0};
	tpAniSirGlobal mac = PMAC_STRUCT(h_hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tsf_msg.type = WMA_TSF_GPIO_PIN;
		tsf_msg.reserved = 0;
		tsf_msg.bodyval = pinvalue;

		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &tsf_msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Unable to post WMA_TSF_GPIO_PIN");
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}
#endif

QDF_STATUS sme_get_cfg_valid_channels(uint8_t *aValidChannels,
				      uint32_t *len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac_ctx = sme_get_mac_context();

	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_cfg_valid_channels(mac_ctx,
			aValidChannels, len);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

static uint8_t *sme_reg_hint_to_str(const enum country_src src)
{
	switch (src) {
	case SOURCE_CORE:
		return "WORLD MODE";

	case SOURCE_DRIVER:
		return "BDF file";

	case SOURCE_USERSPACE:
		return "user-space";

	case SOURCE_11D:
		return "802.11D IEs in beacons";

	default:
		return "unknown";
	}
}

void sme_set_cc_src(tHalHandle hHal, enum country_src cc_src)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hHal);

	mac_ctx->reg_hint_src = cc_src;

	sme_debug("Country source is %s",
		  sme_reg_hint_to_str(cc_src));
}
/*
 * sme_handle_change_country_code() -
 *  Change Country code, Reg Domain and channel list
 *
 * Details Country Code Priority
 * If Supplicant country code is priority than 11d is disabled.
 * If 11D is enabled, we update the country code after every scan.
 * Hence when Supplicant country code is priority, we don't need 11D info.
 * Country code from Supplicant is set as current courtry code.
 * User can send reset command XX (instead of country code) to reset the
 * country code to default values. If 11D is priority,
 * Than Supplicant country code code is set to default code. But 11D code
 * is set as current country code
 *
 * pMac - The handle returned by mac_open.
 * pMsgBuf - MSG Buffer
 * Return QDF_STATUS
 */
static QDF_STATUS sme_handle_change_country_code(tpAniSirGlobal pMac,
						void *pMsgBuf)
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
	if (!qdf_mem_cmp(pMsg->countryCode, SME_INVALID_COUNTRY_CODE, 2)) {
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;

		qdf_status = ucfg_reg_get_default_country(pMac->psoc,
							  default_country);

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
		sme_debug(
			"Set default country code (%c%c) as invalid country received",
			pMsg->countryCode[0], pMsg->countryCode[1]);
			qdf_mem_copy(pMac->scan.countryCode11d,
			pMsg->countryCode,
			WNI_CFG_COUNTRY_CODE_LEN);
	} else {
		/* if Supplicant country code has priority, disable 11d */
		if (pMac->roam.configParam.fSupplicantCountryCodeHasPriority &&
		    pMsg->countryFromUserSpace)
			pMac->roam.configParam.Is11dSupportEnabled = false;
	}

	if (pMac->roam.configParam.Is11dSupportEnabled)
		return QDF_STATUS_SUCCESS;

	/* Set Current Country code and Current Regulatory domain */
	status = csr_set_country_code(pMac, pMsg->countryCode);
	if (QDF_STATUS_SUCCESS != status) {
		/* Supplicant country code failed. So give 11D priority */
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;
		sme_err("Set Country Code Fail %d", status);
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
		sme_err("Fail to get regId %d", domainIdIoctl);
		return status;
	} else if (REGDOMAIN_WORLD == domainIdIoctl) {
		/* Supplicant country code is invalid, so we are on world mode
		 * now. So give 11D chance to update
		 */
		pMac->roam.configParam.Is11dSupportEnabled =
			pMac->roam.configParam.Is11dSupportEnabledOriginal;
		sme_warn("Country Code unrecognized by driver");
	}

	if (domainIdIoctl >= REGDOMAIN_COUNT) {
		sme_err("Invalid regId %d", domainIdIoctl);
		return QDF_STATUS_E_FAILURE;
	} else {
		/* if 11d has priority, clear currentCountryBssid & countryCode11d to get */
		/* set again if we find AP with 11d info during scan */
		if (!pMac->roam.configParam.fSupplicantCountryCodeHasPriority) {
			sme_warn("Clearing currentCountryBssid, countryCode11d");
			qdf_mem_zero(&pMac->scan.currentCountryBssid,
				     sizeof(struct qdf_mac_addr));
			qdf_mem_zero(pMac->scan.countryCode11d,
				     sizeof(pMac->scan.countryCode11d));
		}
	}

	if (pMsg->changeCCCallback)
		((tSmeChangeCountryCallback) (pMsg->changeCCCallback))((void *)
							pMsg->pDevContext);

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
static QDF_STATUS
sme_handle_generic_change_country_code(tpAniSirGlobal mac_ctx,
				       void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	v_REGDOMAIN_t reg_domain_id = 0;
	bool user_ctry_priority =
		mac_ctx->roam.configParam.fSupplicantCountryCodeHasPriority;
	tAniGenericChangeCountryCodeReq *msg = pMsgBuf;

	if (SOURCE_11D != mac_ctx->reg_hint_src) {
		if (SOURCE_DRIVER != mac_ctx->reg_hint_src) {
			if (user_ctry_priority)
				mac_ctx->roam.configParam.Is11dSupportEnabled =
					false;
			else {
				if (mac_ctx->roam.configParam.
					Is11dSupportEnabled &&
					mac_ctx->scan.countryCode11d[0] != 0) {

					sme_debug("restore 11d");

					status =
					csr_get_regulatory_domain_for_country(
						mac_ctx,
						mac_ctx->scan.countryCode11d,
						&reg_domain_id,
						SOURCE_11D);
					return QDF_STATUS_E_FAILURE;
				}
			}
		}
	} else {
		/* if kernel gets invalid country code; it
		 *  resets the country code to world
		 */
		if (('0' != msg->countryCode[0]) ||
		    ('0' != msg->countryCode[1]))
			qdf_mem_copy(mac_ctx->scan.countryCode11d,
				     msg->countryCode,
				     WNI_CFG_COUNTRY_CODE_LEN);
	}

	qdf_mem_copy(mac_ctx->scan.countryCodeCurrent,
		     msg->countryCode,
		     WNI_CFG_COUNTRY_CODE_LEN);

	/* get the channels based on new cc */
	status = csr_get_channel_and_power_list(mac_ctx);

	if (status != QDF_STATUS_SUCCESS) {
		sme_err("fail to get Channels");
		return status;
	}

	/* reset info based on new cc, and we are done */
	csr_apply_channel_power_info_wrapper(mac_ctx);

	csr_scan_filter_results(mac_ctx);

	/* scans after the country is set by User hints or
	 * Country IE
	 */
	mac_ctx->scan.curScanType = eSIR_ACTIVE_SCAN;

	mac_ctx->reg_hint_src = SOURCE_UNKNOWN;

	sme_disconnect_connected_sessions(mac_ctx);

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_update_channel_list() - Update configured channel list to fwr
 * This is a synchronous API.
 *
 * @mac_ctx - The handle returned by mac_open.
 *
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS
sme_update_channel_list(tpAniSirGlobal mac_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Update umac channel (enable/disable) from cds channels */
		status = csr_get_channel_and_power_list(mac_ctx);
		if (status != QDF_STATUS_SUCCESS) {
			sme_err("fail to get Channels");
			sme_release_global_lock(&mac_ctx->sme);
			return status;
		}

		csr_apply_channel_power_info_wrapper(mac_ctx);
		csr_scan_filter_results(mac_ctx);
		sme_disconnect_connected_sessions(mac_ctx);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

static bool
sme_search_in_base_ch_lst(tpAniSirGlobal mac_ctx, uint8_t curr_ch)
{
	uint8_t i;
	struct csr_channel *ch_lst_info;

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
static void sme_disconnect_connected_sessions(tpAniSirGlobal mac_ctx)
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
		sme_debug("Current Operating channel : %d, session :%d",
			  curr_ch, session_id);
		found = sme_search_in_base_ch_lst(mac_ctx, curr_ch);
		if (!found) {
			sme_debug("Disconnect Session: %d", session_id);
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
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct csr_roam_session *pSession = NULL;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"%s: ulMulticastAddrCnt: %d, multicastAddr[0]: %pK", __func__,
		  pMulticastAddrs->ulMulticastAddrCnt,
		  pMulticastAddrs->multicastAddr[0].bytes);

	/* Find the connected Infra / P2P_client connected session */
	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (!CSR_IS_SESSION_VALID(pMac, sessionId) ||
			(!csr_is_conn_state_infra(pMac, sessionId) &&
			 !csr_is_ndi_started(pMac, sessionId))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Unable to find the session Id: %d", __func__,
			  sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(tSirRcvFltMcAddrList));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for 8023 Multicast List request",
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
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 sessionId, msg.type));
	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_8023_MULTICAST_LIST message to WMA",
			  __func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_PACKET_FILTERING */

/*
 * sme_is_channel_valid() -
 * To check if the channel is valid for currently established domain
 *   This is a synchronous API.
 *
 * hHal - The handle returned by mac_open.
 * channel - channel to verify
 * Return true/false, true if channel is valid
 */
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

/*
 * sme_set_freq_band() -
 *  Used to set frequency band.
 *
 * hHal
 * sessionId - Session Identifier
 * band value to be configured
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_freq_band(tHalHandle hHal, uint8_t sessionId,
			     enum band_info eBand)
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

/*
 * sme_get_freq_band() -
 * Used to get the current band settings.
 *
 * hHal
 * pBand  pointer to hold band value
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_freq_band(tHalHandle hHal, enum band_info *pBand)
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

/*
 * sme_set_max_tx_power_per_band() -
 * Set the Maximum Transmit Power specific to band dynamically.
 *   Note: this setting will not persist over reboots.
 *
 * band
 * power to set in dB
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_max_tx_power_per_band(enum band_info band, int8_t dB)
{
	struct scheduler_msg msg = {0};
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
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 NO_SESSION, msg.type));
	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s:Not able to post WMA_SET_MAX_TX_POWER_PER_BAND_REQ",
			  __func__);
		qdf_mem_free(pMaxTxPowerPerBandParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_set_max_tx_power() -
 * Set the Maximum Transmit Power dynamically. Note: this setting will
 *   not persist over reboots.
 *
 * hHal
 * pBssid  BSSID to set the power cap for
 * pBssid  pSelfMacAddress self MAC Address
 * pBssid  power to set in dB
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_max_tx_power(tHalHandle hHal, struct qdf_mac_addr pBssid,
				struct qdf_mac_addr pSelfMacAddress, int8_t dB)
{
	struct scheduler_msg msg = {0};
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

	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_SET_MAX_TX_POWER_REQ message to WMA",
			  __func__);
		qdf_mem_free(pMaxTxParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_set_custom_mac_addr() -
 * Set the customer Mac Address.
 *
 * customMacAddr  customer MAC Address
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_custom_mac_addr(tSirMacAddr customMacAddr)
{
	struct scheduler_msg msg = {0};
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

	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to post SIR_HAL_SET_BASE_MACADDR_IND message to WMA");
		qdf_mem_free(pBaseMacAddr);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_set_tx_power() -
 * Set Transmit Power dynamically.
 *
 * hHal
 * sessionId  Target Session ID
 * BSSID
 * dev_mode dev_mode such as station, P2PGO, SAP
 * dBm  power to set
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_tx_power(tHalHandle hHal, uint8_t sessionId,
			   struct qdf_mac_addr pBSSId,
			   enum QDF_OPMODE dev_mode, int dBm)
{
	struct scheduler_msg msg = {0};
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

	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to post WMA_SET_TX_POWER_REQ to WMA",
			  __func__);
		qdf_mem_free(pTxParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_update_fils_setting(tHalHandle hal, uint8_t session_id,
				   uint8_t param_val)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);

	pMac->roam.configParam.is_fils_enabled = !param_val;

	pMac->roam.configParam.enable_bcast_probe_rsp = !param_val;
	status = wma_cli_set_command((int)session_id,
			(int)WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE,
			!param_val, VDEV_CMD);
	if (status)
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to set enable bcast probe setting",
			__func__);

	return status;
}

QDF_STATUS sme_update_session_param(tHalHandle hal, uint8_t session_id,
			uint32_t param_type, uint32_t param_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	uint16_t len;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		struct sir_update_session_param *msg;
		struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx,
							session_id);

		if (!session) {
			sme_err("Session: %d not found", session_id);
			sme_release_global_lock(&mac_ctx->sme);
			return status;
		}

		if (param_type == SIR_PARAM_IGNORE_ASSOC_DISALLOWED)
			mac_ctx->ignore_assoc_disallowed = param_val;

		if (!session->sessionActive)
			QDF_ASSERT(0);

		len = sizeof(*msg);
		msg = qdf_mem_malloc(len);
		if (!msg)
			status = QDF_STATUS_E_NOMEM;
		else {
			msg->message_type = eWNI_SME_SESSION_UPDATE_PARAM;
			msg->length = len;
			msg->session_id = session_id;
			msg->param_type = param_type;
			msg->param_val = param_val;
			status = umac_send_mb_message_to_mac(msg);
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}

/*
 * sme_set_tm_level() -
 * Set Thermal Mitigation Level to RIVA
 *
 * hHal - The handle returned by mac_open.
 * newTMLevel - new Thermal Mitigation Level
 * tmMode - Thermal Mitigation handle mode, default 0
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_tm_level(tHalHandle hHal, uint16_t newTMLevel, uint16_t
			tmMode)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
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
		message.bodyptr = setTmLevelReq;
		message.type = WMA_SET_TM_LEVEL_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_feature_caps_exchange() - SME interface to exchange capabilities between
 *  Host and FW.
 *
 * hHal - HAL handle for device
 * Return NONE
 */
void sme_feature_caps_exchange(tHalHandle hHal)
{
	MTRACE(qdf_trace
		       (QDF_MODULE_ID_SME, TRACE_CODE_SME_RX_HDD_CAPS_EXCH,
			NO_SESSION, 0));
}

/*
 * sme_disable_feature_capablity() - SME interface to disable Active mode
 * offload capablity in Host.
 *
 * hHal - HAL handle for device
 * Return NONE
 */
void sme_disable_feature_capablity(uint8_t feature_index)
{
}

/*
 * sme_reset_power_values_for5_g
 * Reset the power values for 5G band with default power values.
 *
 * hHal - HAL handle for device
 * Return NONE
 */
void sme_reset_power_values_for5_g(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_RESET_PW5G, NO_SESSION, 0));
	csr_save_channel_power_for_band(pMac, true);
	/* Store the channel+power info in the global place: Cfg */
	csr_apply_power2_current(pMac);
}

/*
 * sme_update_roam_prefer5_g_hz() -
 *  Enable/disable Roam prefer 5G runtime option
 *	    This function is called through dynamic setConfig callback function
 *	    to configure the Roam prefer 5G runtime option
 *
 * hHal - HAL handle for device
 * nRoamPrefer5GHz Enable/Disable Roam prefer 5G runtime option
 * Return Success or failure
 */

QDF_STATUS sme_update_roam_prefer5_g_hz(tHalHandle hHal,
					bool nRoamPrefer5GHz)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_RP5G, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: gRoamPrefer5GHz is changed from %d to %d",
			  __func__, pMac->roam.configParam.nRoamPrefer5GHz,
			  nRoamPrefer5GHz);
		pMac->roam.configParam.nRoamPrefer5GHz = nRoamPrefer5GHz;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_set_roam_intra_band() -
 * enable/disable Intra band roaming
 *	    This function is called through dynamic setConfig callback function
 *	    to configure the intra band roaming
 * hHal - HAL handle for device
 * nRoamIntraBand Enable/Disable Intra band roaming
 * Return Success or failure
 */
QDF_STATUS sme_set_roam_intra_band(tHalHandle hHal, const bool nRoamIntraBand)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_ROAMIBAND, NO_SESSION, 0));
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: gRoamIntraBand is changed from %d to %d",
			  __func__, pMac->roam.configParam.nRoamIntraBand,
			  nRoamIntraBand);
		pMac->roam.configParam.nRoamIntraBand = nRoamIntraBand;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_update_roam_scan_n_probes() -
 * Function to update roam scan N probes
 *	    This function is called through dynamic setConfig callback function
 *	    to update roam scan N probes
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nProbes number of probe requests to be sent out
 * Return Success or failure
 */
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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

/*
 * sme_update_roam_scan_home_away_time() -
 *  Function to update roam scan Home away time
 *	    This function is called through dynamic setConfig callback function
 *	    to update roam scan home away time
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nRoamScanAwayTime Scan home away time
 * bSendOffloadCmd If true then send offload command to firmware
 *			    If false then command is not sent to firmware
 * Return Success or failure
 */
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: gRoamScanHomeAwayTime is changed from %d to %d",
			  __func__,
			  pMac->roam.configParam.nRoamScanHomeAwayTime,
			  nRoamScanHomeAwayTime);
		pMac->roam.configParam.nRoamScanHomeAwayTime =
			nRoamScanHomeAwayTime;
		sme_release_global_lock(&pMac->sme);
	}
	if (pMac->roam.configParam.isRoamOffloadScanEnabled &&
					bSendOffloadCmd) {
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

	sme_err("Set Channel: %d", channel);
	channel_state =
		wlan_reg_get_channel_state(mac_ctx->pdev, channel);

	if (CHANNEL_STATE_DISABLE == channel_state) {
		sme_err("Invalid channel: %d", channel);
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

/*
 * sme_get_roam_intra_band() -
 * get Intra band roaming
 *
 * hHal - HAL handle for device
 * Return Success or failure
 */
bool sme_get_roam_intra_band(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_ROAMIBAND, NO_SESSION, 0));
	return pMac->roam.configParam.nRoamIntraBand;
}

/*
 * sme_get_roam_scan_n_probes() -
 * get N Probes
 *
 * hHal - HAL handle for device
 * Return Success or failure
 */
uint8_t sme_get_roam_scan_n_probes(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.nProbes;
}

/*
 * sme_get_roam_scan_home_away_time() -
 * get Roam scan home away time
 *
 * hHal - HAL handle for device
 * Return Success or failure
 */
uint16_t sme_get_roam_scan_home_away_time(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.nRoamScanHomeAwayTime;
}

/*
 * sme_update_roam_rssi_diff() -
 * Update RoamRssiDiff
 *	    This function is called through dynamic setConfig callback function
 *	    to configure RoamRssiDiff
 *	    Usage: adb shell iwpriv wlan0 setConfig RoamRssiDiff=[0 .. 125]
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * RoamRssiDiff - minimum rssi difference between potential
 *	    candidate and current AP.
 * Return Success or failure
 */

QDF_STATUS sme_update_roam_rssi_diff(tHalHandle hHal, uint8_t sessionId,
				     uint8_t RoamRssiDiff)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

	if (pMac->roam.configParam.isRoamOffloadScanEnabled)
		csr_roam_offload_scan(pMac, sessionId,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_RSSI_DIFF_CHANGED);
	return status;
}

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS sme_update_fils_config(tHalHandle hal, uint8_t session_id,
				  tCsrRoamProfile *src_profile)
{
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
			&mac->roam.neighborRoamInfo[session_id];

	if (session_id >= CSR_ROAM_SESSION_MAX) {
		sme_err("Invalid sme session id: %d", session_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!src_profile) {
		sme_err("src roam profile NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!mac->roam.configParam.isFastRoamIniFeatureEnabled ||
	    (neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)) {
		sme_info("Fast roam is disabled or not connected(%d)",
				neighbor_roam_info->neighborRoamState);
		return QDF_STATUS_E_PERM;
	}

	csr_update_fils_config(mac, session_id, src_profile);
	if (csr_roamIsRoamOffloadEnabled(mac)) {
		sme_debug("Updating fils config to fw");
		csr_roam_offload_scan(mac, session_id,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      REASON_FILS_PARAMS_CHANGED);
	} else {
		sme_info("LFR3 not enabled");
		return QDF_STATUS_E_INVAL;
	}

	return status;
}

void sme_send_hlp_ie_info(tHalHandle hal, uint8_t session_id,
			  tCsrRoamProfile *profile, uint32_t if_addr)
{
	int i;
	struct scheduler_msg msg;
	QDF_STATUS status;
	struct hlp_params *params;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct csr_roam_session *session = CSR_GET_SESSION(mac, session_id);
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
				&mac->roam.neighborRoamInfo[session_id];

	if (!session) {
		sme_err("session NULL");
		return;
	}

	if (!mac->roam.configParam.isFastRoamIniFeatureEnabled ||
	    (neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)) {
		sme_debug("Fast roam is disabled or not connected(%d)",
				neighbor_roam_info->neighborRoamState);
		return;
	}

	params = qdf_mem_malloc(sizeof(*params));
	if (!params) {
		sme_err("Mem alloc for HLP IE fails");
		return;
	}
	if ((profile->hlp_ie_len +
	     SIR_IPV4_ADDR_LEN) > FILS_MAX_HLP_DATA_LEN) {
		sme_err("HLP IE len exceeds %d",
				profile->hlp_ie_len);
		qdf_mem_free(params);
		return;
	}

	params->vdev_id = session_id;
	params->hlp_ie_len = profile->hlp_ie_len + SIR_IPV4_ADDR_LEN;

	for (i = 0; i < SIR_IPV4_ADDR_LEN; i++)
		params->hlp_ie[i] = (if_addr >> (i * 8)) & 0xFF;

	qdf_mem_copy(params->hlp_ie + SIR_IPV4_ADDR_LEN,
		     profile->hlp_ie, profile->hlp_ie_len);

	msg.type = SIR_HAL_HLP_IE_INFO;
	msg.reserved = 0;
	msg.bodyptr = params;
	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme lock acquire fails");
		qdf_mem_free(params);
		return;
	}

	if (!QDF_IS_STATUS_SUCCESS
			(scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
		sme_err("Not able to post WMA_HLP_IE_INFO message to HAL");
		sme_release_global_lock(&mac->sme);
		qdf_mem_free(params);
		return;
	}

	sme_release_global_lock(&mac->sme);
}

void sme_free_join_rsp_fils_params(struct csr_roam_info *roam_info)
{
	struct fils_join_rsp_params *roam_fils_params;

	if (!roam_info) {
		sme_err("FILS Roam Info NULL");
		return;
	}

	roam_fils_params = roam_info->fils_join_rsp;
	if (!roam_fils_params) {
		sme_err("FILS Roam Param NULL");
		return;
	}

	if (roam_fils_params->fils_pmk)
		qdf_mem_free(roam_fils_params->fils_pmk);

	qdf_mem_free(roam_fils_params);

	roam_info->fils_join_rsp = NULL;
}

#else
inline void sme_send_hlp_ie_info(tHalHandle hal, uint8_t session_id,
			  tCsrRoamProfile *profile, uint32_t if_addr)
{}
#endif

/*
 * sme_update_fast_transition_enabled() - enable/disable Fast Transition
 *	support at runtime
 *  It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 *  isFastTransitionEnabled.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update isFastTransitionEnabled config
 *	successfully.
 *	Other status means SME is failed to update isFastTransitionEnabled.
 */
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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

/*
 * sme_update_wes_mode() -
 * Update WES Mode
 *	    This function is called through dynamic setConfig callback function
 *	    to configure isWESModeEnabled
 *
 * hHal - HAL handle for device
 * isWESModeEnabled - WES mode
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update isWESModeEnabled config successfully.
 *	    Other status means SME is failed to update isWESModeEnabled.
 */

QDF_STATUS sme_update_wes_mode(tHalHandle hHal, bool isWESModeEnabled,
			       uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

/*
 * sme_set_roam_scan_control() -
 * Set roam scan control
 *	    This function is called to set roam scan control
 *	    if roam scan control is set to 0, roaming scan cache is cleared
 *	    any value other than 0 is treated as invalid value
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successfully.
 *	    Other status means SME failure to update
 */
QDF_STATUS sme_set_roam_scan_control(tHalHandle hHal, uint8_t sessionId,
				     bool roamScanControl)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_SCANCTRL, NO_SESSION, 0));

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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
			csr_flush_cfg_bg_scan_roam_channel_list(pMac,
								sessionId);
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

/*
 * sme_update_is_fast_roam_ini_feature_enabled() - enable/disable LFR
 *	support at runtime
 * It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 * isFastRoamIniFeatureEnabled.
 * This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update isFastRoamIniFeatureEnabled config
 *	successfully.
 * Other status means SME is failed to update isFastRoamIniFeatureEnabled.
 */
QDF_STATUS sme_update_is_fast_roam_ini_feature_enabled(tHalHandle hHal,
		uint8_t sessionId, const bool isFastRoamIniFeatureEnabled)
{
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

/**
 * sme_config_fast_roaming() - enable/disable LFR support at runtime
 * @hal - The handle returned by macOpen.
 * @session_id - Session Identifier
 * @is_fast_roam_enabled - flag to enable/disable roaming
 *
 * When Supplicant issues enabled/disable fast roaming on the basis
 * of the Bssid modification in network block (e.g. AutoJoin mode N/W block)
 *
 * Return: QDF_STATUS
 */

QDF_STATUS sme_config_fast_roaming(tHalHandle hal, uint8_t session_id,
				   const bool is_fast_roam_enabled)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);
	QDF_STATUS status;

	/*
	 * supplicant_disabled_roaming flag is set to true in
	 * wlan_hdd_cfg80211_connect_start when supplicant initiate connect
	 * request with BSSID. This flag is reset when supplicant sends
	 * vendor command to enable roaming after association.
	 *
	 * This request from wpa_supplicant will be skipped in this function
	 * if roaming is disabled using driver command or INI and
	 * supplicant_disabled_roaming flag remains set. So make sure to set
	 * supplicant_disabled_roaming flag as per wpa_supplicant even if roam
	 * request from wpa_supplicant ignored.
	 */
	if (session && session->pCurRoamProfile)
		session->pCurRoamProfile->supplicant_disabled_roaming =
			!is_fast_roam_enabled;

	if (!mac_ctx->roam.configParam.isFastRoamIniFeatureEnabled) {
		sme_debug("Fast roam is disabled through ini");
		if (!is_fast_roam_enabled)
			return QDF_STATUS_SUCCESS;
		return  QDF_STATUS_E_FAILURE;
	}

	status = csr_neighbor_roam_update_fast_roaming_enabled(mac_ctx,
					 session_id, is_fast_roam_enabled);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("update fast roaming failed. status: %d", status);
		return  QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_update_is_mawc_ini_feature_enabled() -
 *  Enable/disable LFR MAWC support at runtime
 *  It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 *  isMAWCIniFeatureEnabled.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update MAWCEnabled config successfully.
 *	Other status means SME is failed to update MAWCEnabled.
 */
QDF_STATUS sme_update_is_mawc_ini_feature_enabled(tHalHandle hHal,
						  const bool MAWCEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: MAWCEnabled is changed from %d to %d", __func__,
			  pMac->roam.configParam.csr_mawc_config.mawc_enabled,
			  MAWCEnabled);
		pMac->roam.configParam.csr_mawc_config.mawc_enabled =
			MAWCEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;

}

/**
 * sme_stop_roaming() - Stop roaming for a given sessionId
 *  This is a synchronous call
 *
 * @hHal      - The handle returned by mac_open
 * @sessionId - Session Identifier
 *
 * Return QDF_STATUS_SUCCESS on success
 *	   Other status on failure
 */
QDF_STATUS sme_stop_roaming(tHalHandle hal, uint8_t session_id, uint8_t reason)
{
	struct scheduler_msg wma_msg = {0};
	tSirRetStatus status;
	tSirRoamOffloadScanReq *req;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	tpCsrNeighborRoamControlInfo roam_info;
	struct csr_roam_session *session;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("incorrect session/vdev ID");
		return QDF_STATUS_E_INVAL;
	}

	session = CSR_GET_SESSION(mac_ctx, session_id);

	roam_info = &mac_ctx->roam.neighborRoamInfo[session_id];
	if (!roam_info->b_roam_scan_offload_started) {
		sme_debug("Roaming already disabled for session %d", session_id);
		return QDF_STATUS_SUCCESS;
	}
	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		sme_err("failed to allocated memory");
		return QDF_STATUS_E_NOMEM;
	}

	req->Command = ROAM_SCAN_OFFLOAD_STOP;
	if (reason == eCsrForcedDisassoc)
		req->reason = REASON_ROAM_STOP_ALL;
	else
		req->reason = REASON_SME_ISSUED;
	req->sessionId = session_id;
	if (csr_neighbor_middle_of_roaming(mac_ctx, session_id))
		req->middle_of_roaming = 1;
	else
		csr_roam_reset_roam_params(mac_ctx);

	wma_msg.type = WMA_ROAM_SCAN_OFFLOAD_REQ;
	wma_msg.bodyptr = req;

	status = wma_post_ctrl_msg(mac_ctx, &wma_msg);
	if (eSIR_SUCCESS != status) {
		sme_err("WMA_ROAM_SCAN_OFFLOAD_REQ failed, session_id: %d",
			session_id);
		qdf_mem_free(req);
		return QDF_STATUS_E_FAULT;
	}
	roam_info->b_roam_scan_offload_started = false;
	roam_info->last_sent_cmd = ROAM_SCAN_OFFLOAD_STOP;

	return QDF_STATUS_SUCCESS;
}

void sme_indicate_disconnect_inprogress(tHalHandle hal, uint8_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_session *session;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
			session = CSR_GET_SESSION(mac_ctx, session_id);
			if (session)
				session->discon_in_progress = true;
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
}

/*
 * sme_start_roaming() - Start roaming for a given sessionId
 *  This is a synchronous call
 *
 * hHal      - The handle returned by mac_open
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS on success
 *	Other status on failure
 */
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

/*
 * sme_update_enable_fast_roam_in_concurrency() - enable/disable LFR if
 *	Concurrent session exists
 *  This is a synchronuous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS
 *	Other status means SME is failed
 */
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

/*
 * sme_update_config_fw_rssi_monitoring() - enable/disable firmware RSSI
 *	Monitoring at runtime
 *  It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 *  fEnableFwRssiMonitoring.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update fEnableFwRssiMonitoring.
 *	config successfully.
 * Other status means SME is failed to update fEnableFwRssiMonitoring.
 */
QDF_STATUS sme_update_config_fw_rssi_monitoring(tHalHandle hHal,
						bool fEnableFwRssiMonitoring)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;

	if (sme_cfg_set_int (hHal, WNI_CFG_PS_ENABLE_RSSI_MONITOR,
						fEnableFwRssiMonitoring) ==
						QDF_STATUS_E_FAILURE) {
		qdf_ret_status = QDF_STATUS_E_FAILURE;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Could not pass on WNI_CFG_PS_RSSI_MONITOR to CFG");
	}

	return qdf_ret_status;
}

/*
 * sme_set_roam_opportunistic_scan_threshold_diff() -
 * Update Opportunistic Scan threshold diff
 *	This function is called through dynamic setConfig callback function
 *	to configure  nOpportunisticThresholdDiff
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nOpportunisticThresholdDiff - Opportunistic Scan threshold diff
 * Return QDF_STATUS_SUCCESS - SME update nOpportunisticThresholdDiff config
 *	    successfully.
 *	    else SME is failed to update nOpportunisticThresholdDiff.
 */
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

/*
 * sme_get_roam_opportunistic_scan_threshold_diff()
 * gets Opportunistic Scan threshold diff
 * This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return uint8_t - nOpportunisticThresholdDiff
 */
uint8_t sme_get_roam_opportunistic_scan_threshold_diff(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.
	       nOpportunisticThresholdDiff;
}

/*
 * sme_set_roam_rescan_rssi_diff() - Update roam rescan rssi diff
 * This function is called through dynamic setConfig callback function
 * to configure  nRoamRescanRssiDiff
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nRoamRescanRssiDiff - roam rescan rssi diff
 * Return QDF_STATUS_SUCCESS - SME update nRoamRescanRssiDiff config
 *	    successfully.
 * else SME is failed to update nRoamRescanRssiDiff.
 */
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

/*
 * sme_get_roam_rescan_rssi_diff()
 * gets roam rescan rssi diff
 *	  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return int8_t - nRoamRescanRssiDiff
 */
uint8_t sme_get_roam_rescan_rssi_diff(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.nRoamRescanRssiDiff;
}

/*
 * sme_set_roam_bmiss_first_bcnt() -
 * Update Roam count for first beacon miss
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nRoamBmissFirstBcnt
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nRoamBmissFirstBcnt - Roam first bmiss count
 * Return QDF_STATUS_SUCCESS - SME update nRoamBmissFirstBcnt
 *	    successfully.
 * else SME is failed to update nRoamBmissFirstBcnt
 */
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

/*
 * sme_get_roam_bmiss_first_bcnt() -
 * get neighbor roam beacon miss first count
 *
 * hHal - The handle returned by mac_open.
 * Return uint8_t - neighbor roam beacon miss first count
 */
uint8_t sme_get_roam_bmiss_first_bcnt(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.nRoamBmissFirstBcnt;
}

/*
 * sme_set_roam_bmiss_final_bcnt() -
 * Update Roam count for final beacon miss
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nRoamBmissFinalBcnt
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nRoamBmissFinalBcnt - Roam final bmiss count
 * Return QDF_STATUS_SUCCESS - SME update nRoamBmissFinalBcnt
 *	    successfully.
 * else SME is failed to update nRoamBmissFinalBcnt
 */
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

/*
 * sme_get_roam_bmiss_final_bcnt() -
 * gets Roam count for final beacon miss
 *	  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return uint8_t - nRoamBmissFinalBcnt
 */
uint8_t sme_get_roam_bmiss_final_bcnt(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.nRoamBmissFinalBcnt;
}

/*
 * sme_set_roam_beacon_rssi_weight() -
 * Update Roam beacon rssi weight
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nRoamBeaconRssiWeight
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nRoamBeaconRssiWeight - Roam beacon rssi weight
 * Return QDF_STATUS_SUCCESS - SME update nRoamBeaconRssiWeight config
 *	    successfully.
 * else SME is failed to update nRoamBeaconRssiWeight
 */
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

/*
 * sme_get_roam_beacon_rssi_weight() -
 * gets Roam beacon rssi weight
 *	  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return uint8_t - nRoamBeaconRssiWeight
 */
uint8_t sme_get_roam_beacon_rssi_weight(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.nRoamBeaconRssiWeight;
}

/*
 * sme_set_neighbor_lookup_rssi_threshold() - update neighbor lookup
 *	rssi threshold
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
QDF_STATUS sme_set_neighbor_lookup_rssi_threshold(tHalHandle hHal,
			uint8_t sessionId, uint8_t neighborLookupRssiThreshold)
{
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

/*
 * sme_set_delay_before_vdev_stop() - update delay before VDEV_STOP
 *  This is a synchronous call
 *
 * hal - The handle returned by macOpen.
 * session_id - Session Identifier
 * delay_before_vdev_stop - value to be set
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	  Other status means SME is failed to update
 */
QDF_STATUS sme_set_delay_before_vdev_stop(tHalHandle hal,
					  uint8_t session_id,
					  uint8_t delay_before_vdev_stop)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (session_id >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), session_id);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"LFR param delay_before_vdev_stop changed from %d to %d",
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

/*
 * sme_get_neighbor_lookup_rssi_threshold() - get neighbor lookup
 *	rssi threshold
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
uint8_t sme_get_neighbor_lookup_rssi_threshold(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.
	       nNeighborLookupRssiThreshold;
}

/*
 * sme_set_neighbor_scan_refresh_period() - set neighbor scan results
 *	refresh period
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
QDF_STATUS sme_set_neighbor_scan_refresh_period(tHalHandle hHal,
		uint8_t sessionId, uint16_t neighborScanResultsRefreshPeriod)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_neighbor_roamconfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

/*
 * sme_update_roam_scan_offload_enabled() - enable/disable roam scan
 *	offload feaure
 *  It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 *  gRoamScanOffloadEnabled.
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update config successfully.
 *	   Other status means SME is failed to update.
 */
QDF_STATUS sme_update_roam_scan_offload_enabled(tHalHandle hHal,
						bool nRoamScanOffloadEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "gRoamScanOffloadEnabled is changed from %d to %d",
			  pMac->roam.configParam.isRoamOffloadScanEnabled,
			  nRoamScanOffloadEnabled);
		pMac->roam.configParam.isRoamOffloadScanEnabled =
			nRoamScanOffloadEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_get_neighbor_scan_refresh_period() - get neighbor scan results
 *	refresh period
 *  This is a synchronous call
 *
 *  \param hHal - The handle returned by mac_open.
 *  \return uint16_t - Neighbor scan results refresh period value
 */
uint16_t sme_get_neighbor_scan_refresh_period(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.
	       nNeighborResultsRefreshPeriod;
}

/*
 * sme_get_empty_scan_refresh_period() - get empty scan refresh period
 * This is a synchronuous call
 *
 * hHal - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
uint16_t sme_get_empty_scan_refresh_period(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.neighborRoamConfig.
	       nEmptyScanRefreshPeriod;
}

/*
 * sme_update_empty_scan_refresh_period
 * Update nEmptyScanRefreshPeriod
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nEmptyScanRefreshPeriod
 *	    Usage: adb shell iwpriv wlan0 setConfig
 *			nEmptyScanRefreshPeriod=[0 .. 60]
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nEmptyScanRefreshPeriod - scan period following empty scan results.
 * Return Success or failure
 */

QDF_STATUS sme_update_empty_scan_refresh_period(tHalHandle hHal, uint8_t
						sessionId, uint16_t
						nEmptyScanRefreshPeriod)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_neighbor_roamconfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

/*
 * sme_set_neighbor_scan_min_chan_time() -
 * Update nNeighborScanMinChanTime
 *	    This function is called through dynamic setConfig callback function
 *	    to configure gNeighborScanChannelMinTime
 *	    Usage: adb shell iwpriv wlan0 setConfig
 *			gNeighborScanChannelMinTime=[0 .. 60]
 *
 * hHal - HAL handle for device
 * nNeighborScanMinChanTime - Channel minimum dwell time
 * sessionId - Session Identifier
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_min_chan_time(tHalHandle hHal,
					       const uint16_t
					       nNeighborScanMinChanTime,
					       uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

/*
 * sme_set_neighbor_scan_max_chan_time() -
 * Update nNeighborScanMaxChanTime
 *	    This function is called through dynamic setConfig callback function
 *	    to configure gNeighborScanChannelMaxTime
 *	    Usage: adb shell iwpriv wlan0 setConfig
 *			gNeighborScanChannelMaxTime=[0 .. 60]
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nNeighborScanMinChanTime - Channel maximum dwell time
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_max_chan_time(tHalHandle hHal, uint8_t
						sessionId,
					       const uint16_t
					       nNeighborScanMaxChanTime)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_neighbor_roamconfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

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

/*
 * sme_get_neighbor_scan_min_chan_time() -
 * get neighbor scan min channel time
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - channel min time value
 */
uint16_t sme_get_neighbor_scan_min_chan_time(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       minChannelScanTime;
}

/*
 * sme_get_neighbor_roam_state() -
 * get neighbor roam state
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - neighbor roam state
 */
uint32_t sme_get_neighbor_roam_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return pMac->roam.neighborRoamInfo[sessionId].neighborRoamState;
}

/*
 * sme_get_current_roam_state() -
 * get current roam state
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - current roam state
 */
uint32_t sme_get_current_roam_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.curState[sessionId];
}

/*
 * sme_get_current_roam_sub_state() -
 *   \brief  get neighbor roam sub state
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - current roam sub state
 */
uint32_t sme_get_current_roam_sub_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.curSubState[sessionId];
}

/*
 * sme_get_lim_sme_state() -
 *   get Lim Sme state
 *
 * hHal - The handle returned by mac_open.
 * Return uint32_t - Lim Sme state
 */
uint32_t sme_get_lim_sme_state(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->lim.gLimSmeState;
}

/*
 * sme_get_lim_mlm_state() -
 *   get Lim Mlm state
 *
 * hHal - The handle returned by mac_open.
 * Return uint32_t - Lim Mlm state
 */
uint32_t sme_get_lim_mlm_state(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->lim.gLimMlmState;
}

/*
 * sme_is_lim_session_valid() -
 *  is Lim session valid
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return bool - true or false
 */
bool sme_is_lim_session_valid(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId > pMac->lim.maxBssId)
		return false;

	return pMac->lim.gpSession[sessionId].valid;
}

/*
 * sme_get_lim_sme_session_state() -
 *   get Lim Sme session state
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - Lim Sme session state
 */
uint32_t sme_get_lim_sme_session_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->lim.gpSession[sessionId].limSmeState;
}

/*
 * sme_get_lim_mlm_session_state() -
 *   \brief  get Lim Mlm session state
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - Lim Mlm session state
 */
uint32_t sme_get_lim_mlm_session_state(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->lim.gpSession[sessionId].limMlmState;
}

/*
 * sme_get_neighbor_scan_max_chan_time() -
 *   get neighbor scan max channel time
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - channel max time value
 */
uint16_t sme_get_neighbor_scan_max_chan_time(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       maxChannelScanTime;
}

/*
 * sme_set_neighbor_scan_period() -
 *  Update nNeighborScanPeriod
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nNeighborScanPeriod
 *	    Usage: adb shell iwpriv wlan0 setConfig
 *			nNeighborScanPeriod=[0 .. 1000]
 *
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * nNeighborScanPeriod - neighbor scan period
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId,
					const uint16_t nNeighborScanPeriod)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_neighbor_roamconfig *pNeighborRoamConfig = NULL;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamConfig =
			&pMac->roam.configParam.neighborRoamConfig;
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set neighbor scan period to %d - old value is %d - roam state is %s",
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

/*
 * sme_get_neighbor_scan_period() -
 *   get neighbor scan period
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - neighbor scan period
 */
uint16_t sme_get_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return pMac->roam.neighborRoamInfo[sessionId].cfgParams.
	       neighborScanPeriod;
}

/**
 * sme_set_neighbor_scan_min_period() - Update neighbor_scan_min_period
 *          This function is called through dynamic setConfig callback function
 *          to configure neighbor_scan_min_period
 *
 * @hal - HAL handle for device
 * @session_id - Session Identifier
 * @neighbor_scan_min_period - neighbor scan min period
 *
 * Return - QDF_STATUS
 */
QDF_STATUS sme_set_neighbor_scan_min_period(tHalHandle hal,
					    uint8_t session_id,
					    const uint16_t
					    neighbor_scan_min_period)
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_neighbor_roamconfig *p_neighbor_roam_config = NULL;
	tpCsrNeighborRoamControlInfo p_neighbor_roam_info = NULL;

	if (session_id >= CSR_ROAM_SESSION_MAX) {
		sme_err("Invalid sme session id: %d", session_id);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&pmac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		p_neighbor_roam_config =
				&pmac->roam.configParam.neighborRoamConfig;
		p_neighbor_roam_info = &pmac->
				roam.neighborRoamInfo[session_id];
		sme_debug("LFR:set neighbor scan min period, old:%d, "
				"new: %d, state: %s",
				pmac->roam.configParam.neighborRoamConfig.
				neighbor_scan_min_timer_period,
				neighbor_scan_min_period,
				mac_trace_get_neighbour_roam_state(pmac->roam.
				neighborRoamInfo[session_id].
				neighborRoamState));
		p_neighbor_roam_config->neighbor_scan_min_timer_period =
				neighbor_scan_min_period;
		p_neighbor_roam_info->cfgParams.neighbor_scan_min_period =
				neighbor_scan_min_period;
		sme_release_global_lock(&pmac->sme);
	}

	return status;
}

/*
 * sme_get_roam_rssi_diff() - get Roam rssi diff
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return uint16_t - Rssi diff value
 */
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
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	uint8_t oldChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t i = 0, j = 0;
	tCsrChannelInfo *chan_info;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
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
		"LFR runtime successfully set roam scan channels to %s - old value is %s - roam state is %d",
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
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[sessionId];
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
	for (i = 0; i < (*pNumChannels); i++)
		pOutPtr[i] =
			pNeighborRoamInfo->cfgParams.channelInfo.ChannelList[i];

	pOutPtr[i] = '\0';
	sme_release_global_lock(&pMac->sme);
	return status;
}

/*
 * sme_get_is_ese_feature_enabled() - get ESE feature enabled or not
 *  This is a synchronuous call
 *
 * hHal - The handle returned by mac_open.
 * Return true (1) - if the ESE feature is enabled
 *	  false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_ese_feature_enabled(tHalHandle hHal)
{
#ifdef FEATURE_WLAN_ESE
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return csr_roam_is_ese_ini_feature_enabled(pMac);
#else
	return false;
#endif
}

/*
 * sme_get_wes_mode() - get WES Mode
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * Return uint8_t - WES Mode Enabled(1)/Disabled(0)
 */
bool sme_get_wes_mode(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.isWESModeEnabled;
}

/*
 * sme_get_roam_scan_control() - get scan control
 *  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return bool - Enabled(1)/Disabled(0)
 */
bool sme_get_roam_scan_control(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.nRoamScanControl;
}

/*
 * sme_get_is_lfr_feature_enabled() - get LFR feature enabled or not
 *  This is a synchronuous call
 * hHal - The handle returned by mac_open.
 * Return true (1) - if the feature is enabled
 *	  false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_lfr_feature_enabled(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.isFastRoamIniFeatureEnabled;
}

/*
 * sme_get_is_ft_feature_enabled() - get FT feature enabled or not
 *  This is a synchronuous call
 *
 * hHal - The handle returned by mac_open.
 * Return true (1) - if the feature is enabled
 *	   false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_ft_feature_enabled(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.isFastTransitionEnabled;
}

/**
 * sme_is_feature_supported_by_fw() - check if feature is supported by FW
 * @feature: enum value of requested feature.
 *
 * Retrun: 1 if supported; 0 otherwise
 */
bool sme_is_feature_supported_by_fw(enum cap_bitmap feature)
{
	return IS_FEATURE_SUPPORTED_BY_FW(feature);
}

QDF_STATUS sme_get_link_speed(tHalHandle hHal, tSirLinkSpeedInfo *lsReq,
			      void *plsContext,
			      void (*pCallbackfn)(tSirLinkSpeedInfo *indParam,
						  void *pContext))
{

	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac;
	tSirLinkSpeedInfo *req;
	void *wma_handle;

	if (!hHal || !pCallbackfn || !lsReq) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid parameter"));
		return QDF_STATUS_E_FAILURE;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pMac = PMAC_STRUCT(hHal);
	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		sme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}
	*req = *lsReq;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("Failed to acquire global lock");
		qdf_mem_free(req);
		return QDF_STATUS_E_FAILURE;
	}

	pMac->sme.pLinkSpeedCbContext = plsContext;
	pMac->sme.pLinkSpeedIndCb = pCallbackfn;
	status = wma_get_link_speed(wma_handle, req);
	sme_release_global_lock(&pMac->sme);

	return status;
}

QDF_STATUS sme_get_peer_stats(tpAniSirGlobal mac,
			      struct sir_peer_info_req req)
{
	QDF_STATUS qdf_status;
	struct scheduler_msg message = {0};

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		sme_debug("Failed to get Lock");
		return qdf_status;
	}
	/* serialize the req through MC thread */
	message.bodyptr = qdf_mem_malloc(sizeof(req));
	if (NULL == message.bodyptr) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Memory allocation failed.", __func__);
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(message.bodyptr, &req, sizeof(req));
	message.type = WMA_GET_PEER_INFO;
	message.reserved = 0;
	qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Post get peer info msg fail", __func__);
		qdf_mem_free(message.bodyptr);
		qdf_status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return qdf_status;
}

QDF_STATUS sme_get_peer_info(tHalHandle hal, struct sir_peer_info_req req,
			void *context,
			void (*callbackfn)(struct sir_peer_info_resp *param,
						void *pcontext))
{

	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (NULL == callbackfn) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Indication Call back is NULL",
				__func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		mac->sme.pget_peer_info_ind_cb = callbackfn;
		mac->sme.pget_peer_info_cb_context = context;

		/* serialize the req through MC thread */
		message.bodyptr = qdf_mem_malloc(sizeof(req));
		if (NULL == message.bodyptr) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Memory allocation failed.", __func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(message.bodyptr, &req, sizeof(req));
		message.type = WMA_GET_PEER_INFO;
		message.reserved = 0;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Post get peer info msg fail", __func__);
			qdf_mem_free(message.bodyptr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

QDF_STATUS sme_get_peer_info_ext(tHalHandle hal,
		struct sir_peer_info_ext_req *req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_ext_resp *param,
			void *pcontext))
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (NULL == callbackfn) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Indication Call back is NULL",
				__func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		mac->sme.pget_peer_info_ext_ind_cb = callbackfn;
		mac->sme.pget_peer_info_ext_cb_context = context;

		/* serialize the req through MC thread */
		message.bodyptr =
			qdf_mem_malloc(sizeof(struct sir_peer_info_ext_req));
		if (NULL == message.bodyptr) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Memory allocation failed.", __func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(message.bodyptr,
				req,
				sizeof(struct sir_peer_info_ext_req));
		message.type = WMA_GET_PEER_INFO_EXT;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Post get rssi msg fail", __func__);
			qdf_mem_free(message.bodyptr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
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
		sme_debug("SSR level is changed %d", enableSSR);
		/* not serializing this messsage, as this is only going
		 * to set a variable in WMA/WDI
		 */
		WMA_SetEnableSSR(enableSSR);
		sme_release_global_lock(&pMac->sme);
	}
}

/*convert the ini value to the ENUM used in csr and MAC for CB state*/
ePhyChanBondState sme_get_cb_phy_state_from_cb_ini_value(uint32_t cb_ini_value)
{
	return csr_convert_cb_ini_value_to_phy_cb_state(cb_ini_value);
}

/*
 * sme_set_curr_device_mode() - Sets the current operating device mode.
 *
 * hHal - The handle returned by mac_open.
 * currDeviceMode - Current operating device mode.
 */
void sme_set_curr_device_mode(tHalHandle hHal,
				enum QDF_OPMODE currDeviceMode)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->sme.currDeviceMode = currDeviceMode;
}

/*
 * sme_handoff_request() - a wrapper function to Request a handoff from CSR.
 *   This is a synchronous call
 *
 * hHal - The handle returned by mac_open
 * sessionId - Session Identifier
 * pHandoffInfo - info provided by HDD with the handoff request (namely:
 * BSSID, channel etc.)
 * Return QDF_STATUS_SUCCESS - SME passed the request to CSR successfully.
 *	   Other status means SME is failed to send the request.
 */

QDF_STATUS sme_handoff_request(tHalHandle hHal,
			       uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: invoked", __func__);
		status = csr_handoff_request(pMac, sessionId, pHandoffInfo);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

#ifdef IPA_OFFLOAD
/*
 * sme_ipa_offload_enable_disable() -
 *  API to enable/disable IPA offload
 *
 * hal - The handle returned by macOpen.
 * session_id - Session Identifier
 * request -  Pointer to the offload request.
 * Return QDF_STATUS
 */
QDF_STATUS sme_ipa_offload_enable_disable(tHalHandle hal, uint8_t session_id,
		struct sir_ipa_offload_enable_disable *request)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sir_ipa_offload_enable_disable *request_buf;
	struct scheduler_msg msg = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		request_buf = qdf_mem_malloc(sizeof(*request_buf));
		if (NULL == request_buf) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"Not able to allocate memory for IPA_OFFLOAD_ENABLE_DISABLE");
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
				scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"Not able to post WMA_IPA_OFFLOAD_ENABLE_DISABLE message to WMA");
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

	if (csr_is_infra_connected(pMac))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
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
	struct scheduler_msg msg = {0};

	SME_ENTER();

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sme_err("memory allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *addPeriodicTxPtrnParams;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	msg.bodyptr = req_msg;
	msg.type    = WMA_ADD_PERIODIC_TX_PTRN_IND;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 NO_SESSION, msg.type));
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
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
	struct scheduler_msg msg = {0};

	SME_ENTER();

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sme_err("memory allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *delPeriodicTxPtrnParams;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	msg.bodyptr = req_msg;
	msg.type    = WMA_DEL_PERIODIC_TX_PTRN_IND;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 NO_SESSION, msg.type));
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

/*
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
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	SME_ENTER();

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_ENABLE_IND;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
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
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	SME_ENTER();

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_DISABLE_IND;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
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
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_ACTION_PERIOD_IND;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
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
	struct scheduler_msg message = {0};
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

		message.type = WMA_GET_IBSS_PEER_INFO_REQ;
		message.bodyptr = pIbssInfoReqParams;
		message.reserved = 0;

		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_send_cesium_enable_ind() -
 *  Used to send proprietary cesium enable indication to fw
 *
 * hHal
 * sessionId
 * Return QDF_STATUS
 */
QDF_STATUS sme_send_cesium_enable_ind(tHalHandle hHal, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		message.bodyptr = NULL;
		message.type = WMA_IBSS_CESIUM_ENABLE_IND;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

QDF_STATUS sme_set_wlm_latency_level(tHalHandle hal, uint16_t session_id,
				     uint16_t latency_level)
{
	QDF_STATUS status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct wlm_latency_level_param params;
	void *wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!mac_ctx->roam.configParam.wlm_latency_enable) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: WLM latency level setting is disabled",
			   __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (!wma) {
		sme_err("wma is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	params.wlm_latency_level = latency_level;
	params.wlm_latency_flags =
		mac_ctx->roam.configParam.wlm_latency_flags[latency_level];
	params.vdev_id = session_id;

	status = wma_set_wlm_latency_level(wma, &params);
	if (!QDF_IS_STATUS_SUCCESS(status))
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to set latency level",
			  __func__);

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

	pEntry = csr_nonscan_active_ll_peek_head(pMac, LL_ACCESS_LOCK);
	if (pEntry)
		pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

	sme_err("Currently smeCmdActiveList has command (0x%X)",
		(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
	if (pTempCmd) {
		if (eSmeCsrCommandMask & pTempCmd->command)
			/* CSR command is stuck. See what the reason code is
			 * for that command
			 */
			dump_csr_command_info(pMac, pTempCmd);
	} /* if(pTempCmd) */

	sme_err("Currently smeCmdPendingList has %d commands",
		csr_nonscan_pending_ll_count(pMac));

}
/**
 * sme_set_prefer_80MHz_over_160MHz() - API to set sta_prefer_80MHz_over_160MHz
 * @hal:           The handle returned by macOpen
 * @sta_prefer_80MHz_over_160MHz: sta_prefer_80MHz_over_160MHz config param
 */
void sme_set_prefer_80MHz_over_160MHz(tHalHandle hal,
		bool sta_prefer_80MHz_over_160MHz)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	mac_ctx->sta_prefer_80MHz_over_160MHz = sta_prefer_80MHz_over_160MHz;
}

#ifdef WLAN_FEATURE_DSRC
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
	struct scheduler_msg msg = {0};
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

	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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

	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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

	request->vdev_id = vdev_id;
	request->dcc_stats_bitmap = dcc_stats_bitmap;

	msg.type = WMA_DCC_CLEAR_STATS_CMD;
	msg.bodyptr = request;

	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
	struct scheduler_msg msg = {0};
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

	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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
#endif

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

QDF_STATUS sme_notify_modem_power_state(tHalHandle hHal, uint32_t value)
{
	struct scheduler_msg msg = {0};
	tpSirModemPowerStateInd request_buf;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac)
		return QDF_STATUS_E_FAILURE;

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
		    (scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_MODEM_POWER_STATE_IND message to WMA",
			__func__);
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
	struct scheduler_msg msg = {0};
	tUpdateVHTOpMode *pHtOpMode = NULL;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (NULL == pMac)
		return QDF_STATUS_E_FAILURE;

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
		    (scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_UPDATE_OP_MODE message to WMA",
			__func__);
		qdf_mem_free(pHtOpMode);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Notifed FW about OP mode: %d for staId=%d",
		  __func__, pHtOpMode->opMode, staId);

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_set_ht2040_mode() -
 *  To update HT Operation beacon IE.
 *
 * Return QDF_STATUS  SUCCESS
 *			FAILURE or RESOURCES
 *			The API finished and failed.
 */
QDF_STATUS sme_set_ht2040_mode(tHalHandle hHal, uint8_t sessionId,
			       uint8_t channel_type, bool obssEnabled)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	ePhyChanBondState cbMode;
	struct csr_roam_session *session = CSR_GET_SESSION(pMac, sessionId);

	if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
		sme_err("Session not valid for session id %d", sessionId);
		return QDF_STATUS_E_INVAL;
	}
	session = CSR_GET_SESSION(pMac, sessionId);
	sme_debug("Update HT operation beacon IE, channel_type=%d cur cbmode %d",
		channel_type, session->bssParams.cbMode);

	switch (channel_type) {
	case eHT_CHAN_HT20:
		if (!session->bssParams.cbMode)
			return QDF_STATUS_SUCCESS;
		cbMode = PHY_SINGLE_CHANNEL_CENTERED;
		break;
	case eHT_CHAN_HT40MINUS:
		if (session->bssParams.cbMode)
			return QDF_STATUS_SUCCESS;
		cbMode = PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
		break;
	case eHT_CHAN_HT40PLUS:
		if (session->bssParams.cbMode)
			return QDF_STATUS_SUCCESS;
		cbMode = PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
		break;
	default:
		sme_err("Error!!! Invalid HT20/40 mode !");
		return QDF_STATUS_E_FAILURE;
	}
	session->bssParams.cbMode = cbMode;
	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_set_ht2040_mode(pMac, sessionId,
					     cbMode, obssEnabled);
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#endif

/*
 * SME API to enable/disable idle mode powersave
 * This should be called only if powersave offload
 * is enabled
 */
QDF_STATUS sme_set_idle_powersave_config(bool value)
{
	void *wmaContext = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wmaContext) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: wmaContext is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  " Idle Ps Set Value %d", value);

	if (QDF_STATUS_SUCCESS != wma_set_idle_ps_config(wmaContext, value)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  " Failed to Set Idle Ps Value %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

int16_t sme_get_ht_config(tHalHandle hHal, uint8_t session_id,
			uint16_t ht_capab)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, session_id);

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
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);

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
		value = value ? 1 : 0; /* HT SGI can be only 1 or 0 */
		pSession->htConfig.ht_sgi20 = value;
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		value = value ? 1 : 0; /* HT SGI can be only 1 or 0 */
		pSession->htConfig.ht_sgi40 = value;
		break;
	}

	csr_roam_update_config(pMac, sessionId, htCapab, value);
	return 0;
}

int sme_set_addba_accept(tHalHandle hal, uint8_t session_id, int value)
{
	struct sme_addba_accept *addba_accept;
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	addba_accept = qdf_mem_malloc(sizeof(*addba_accept));
	if (!addba_accept) {
		sme_err("mem alloc failed for addba_accept");
		return -EIO;
	}
	addba_accept->session_id = session_id;
	addba_accept->addba_accept = value;
	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = eWNI_SME_SET_ADDBA_ACCEPT;
	msg.reserved = 0;
	msg.bodyptr = addba_accept;
	status = scheduler_post_msg(QDF_MODULE_ID_PE, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("Not able to post addba reject");
		qdf_mem_free(addba_accept);
		return -EIO;
	}
	return 0;
}
#define HT20_SHORT_GI_MCS7_RATE 722
/*
 * sme_send_rate_update_ind() -
 *  API to Update rate
 *
 * hHal - The handle returned by mac_open
 * rateUpdateParams - Pointer to rate update params
 * Return QDF_STATUS
 */
QDF_STATUS sme_send_rate_update_ind(tHalHandle hHal,
				    tSirRateUpdateInd *rateUpdateParams)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status;
	struct scheduler_msg msg = {0};
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
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, msg.type));
		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post WMA_SET_RMC_RATE_IND to WMA!",
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
 * sme_update_access_policy_vendor_ie() - update vendor ie and access policy.
 * @hal: Pointer to the mac context
 * @session_id: sme session id
 * @vendor_ie: vendor ie
 * @access_policy: vendor ie access policy
 *
 * This function updates the vendor ie and access policy to lim.
 *
 * Return: success or failure.
 */
QDF_STATUS sme_update_access_policy_vendor_ie(tHalHandle hal,
		uint8_t session_id, uint8_t *vendor_ie, int access_policy)
{
	struct sme_update_access_policy_vendor_ie *msg;
	uint16_t msg_len;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	msg_len  = sizeof(*msg);

	msg = qdf_mem_malloc(msg_len);
	if (!msg) {
		sme_err("failed to allocate memory for sme_update_access_policy_vendor_ie");
		return QDF_STATUS_E_FAILURE;
	}

	msg->msg_type = (uint16_t)eWNI_SME_UPDATE_ACCESS_POLICY_VENDOR_IE;
	msg->length = (uint16_t)msg_len;

	qdf_mem_copy(&msg->ie[0], vendor_ie, sizeof(msg->ie));

	msg->sme_session_id = session_id;
	msg->access_policy = access_policy;

	sme_debug("sme_session_id: %hu, access_policy: %d", session_id,
		  access_policy);

	status = umac_send_mb_message_to_mac(msg);

	return status;
}

/**
 * sme_update_short_retry_limit_threshold() - update short frame retry limit TH
 * @hal: Handle returned by mac_open
 * @session_id: Session ID on which short frame retry limit needs to be
 * updated to FW
 * @short_limit_count_th: Retry count TH to retry short frame.
 *
 * This function is used to configure count to retry short frame.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_short_retry_limit_threshold(tHalHandle hal_handle,
		struct sme_short_retry_limit *short_retry_limit_th)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sme_short_retry_limit *srl;
	struct scheduler_msg msg = {0};

	srl = qdf_mem_malloc(sizeof(*srl));
	if (NULL == srl) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: fail to alloc short retry limit", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	sme_debug("session_id %d short retry limit count: %d",
		short_retry_limit_th->session_id,
		short_retry_limit_th->short_retry_limit);

	srl->session_id = short_retry_limit_th->session_id;
	srl->short_retry_limit = short_retry_limit_th->short_retry_limit;

	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = SIR_HAL_SHORT_RETRY_LIMIT_CNT;
	msg.reserved = 0;
	msg.bodyptr = srl;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post short retry limit count to WDA"));
			qdf_mem_free(srl);
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * sme_update_long_retry_limit_threshold() - update long retry limit TH
 * @hal: Handle returned by mac_open
 * @session_id: Session ID on which long frames retry TH needs to be updated
 * to FW
 * @long_limit_count_th: Retry count to retry long frame.
 *
 * This function is used to configure TH to retry long frame.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_long_retry_limit_threshold(tHalHandle hal_handle,
		struct sme_long_retry_limit  *long_retry_limit_th)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sme_long_retry_limit *lrl;
	struct scheduler_msg msg = {0};

	lrl = qdf_mem_malloc(sizeof(*lrl));
	if (NULL == lrl) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		"%s: fail to alloc long retry limit", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	sme_debug("session_id %d long retry limit count: %d",
		long_retry_limit_th->session_id,
		long_retry_limit_th->long_retry_limit);

	lrl->session_id = long_retry_limit_th->session_id;
	lrl->long_retry_limit = long_retry_limit_th->long_retry_limit;

	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = SIR_HAL_LONG_RETRY_LIMIT_CNT;
	msg.reserved = 0;
	msg.bodyptr = lrl;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);

	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post long retry limit count to WDA"));
		qdf_mem_free(lrl);
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * sme_update_sta_inactivity_timeout(): Update sta_inactivity_timeout to FW
 * @hal: Handle returned by mac_open
 * @session_id: Session ID on which sta_inactivity_timeout needs
 * to be updated to FW
 * @sta_inactivity_timeout: sta inactivity timeout.
 *
 * If a station does not send anything in sta_inactivity_timeout seconds, an
 * empty data frame is sent to it in order to verify whether it is
 * still in range. If this frame is not ACKed, the station will be
 * disassociated and then deauthenticated.
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_update_sta_inactivity_timeout(tHalHandle hal_handle,
		 struct sme_sta_inactivity_timeout  *sta_inactivity_timer)
{
	struct sme_sta_inactivity_timeout *inactivity_time;
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	inactivity_time = qdf_mem_malloc(sizeof(*inactivity_time));
	if (NULL == inactivity_time) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: fail to alloc inactivity_time", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("sta_inactivity_timeout: %d"),
			sta_inactivity_timer->sta_inactivity_timeout);
	inactivity_time->session_id = sta_inactivity_timer->session_id;
	inactivity_time->sta_inactivity_timeout =
		sta_inactivity_timer->sta_inactivity_timeout;

	wma_update_sta_inactivity_timeout(wma_handle,
				inactivity_time);
	return QDF_STATUS_SUCCESS;
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
				pMac->scan.defaultPowerTable[i].tx_power);

			SME_SET_CHANNEL_MAX_TX_POWER(*regInfo2,
				pMac->scan.defaultPowerTable[i].tx_power);
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
/*
 * sme_auto_shutdown_cb() -
 *   Used to plug in callback function for receiving auto shutdown evt
 *
 * hHal
 * pCallbackfn : callback function pointer should be plugged in
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_auto_shutdown_cb(tHalHandle hHal, void (*pCallbackfn)(void)
				    ) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "%s: Plug in Auto shutdown event callback", __func__);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (NULL != pCallbackfn)
			pMac->sme.pAutoShutdownNotificationCb = pCallbackfn;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/*
 * sme_set_auto_shutdown_timer() -
 *  API to set auto shutdown timer value in FW.
 *
 * hHal - The handle returned by mac_open
 * timer_val - The auto shutdown timer value to be set
 * Return Configuration message posting status, SUCCESS or Fail
 */
QDF_STATUS sme_set_auto_shutdown_timer(tHalHandle hHal, uint32_t timer_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSirAutoShutdownCmdParams *auto_sh_cmd;
	struct scheduler_msg message = {0};

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
		message.bodyptr = auto_sh_cmd;
		message.type = WMA_SET_AUTO_SHUTDOWN_TIMER_REQ;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Auto shutdown MSG fail", __func__);
			qdf_mem_free(auto_sh_cmd);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Posted Auto shutdown MSG", __func__);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif

#ifdef FEATURE_WLAN_CH_AVOID
/*
 * sme_ch_avoid_update_req() -
 *   API to request channel avoidance update from FW.
 *
 * hHal - The handle returned by mac_open
 * update_type - The udpate_type parameter of this request call
 * Return Configuration message posting status, SUCCESS or Fail
 */
QDF_STATUS sme_ch_avoid_update_req(tHalHandle hHal)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSirChAvoidUpdateReq *cauReq;
	struct scheduler_msg message = {0};

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
		message.bodyptr = cauReq;
		message.type = WMA_CH_AVOID_UPDATE_REQ;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Ch Avoid Update MSG fail",
				  __func__);
			qdf_mem_free(cauReq);
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Posted Ch Avoid Update MSG", __func__);
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}
#endif

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
	struct scheduler_msg msg = {0};
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
				scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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
	struct scheduler_msg msg = {0};
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
				scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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
				       struct ch_params *ch_params,
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

/*
 * sme_process_channel_change_resp() -
 * API to Indicate Channel change response message to SAP.
 *
 * Return QDF_STATUS
 */
static QDF_STATUS sme_process_channel_change_resp(tpAniSirGlobal pMac,
					   uint16_t msg_type, void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info proam_info = { 0 };
	eCsrRoamResult roamResult;
	tpSwitchChannelParams pChnlParams = (tpSwitchChannelParams) pMsgBuf;
	uint32_t SessionId = pChnlParams->peSessionId;

	proam_info.channelChangeRespEvent =
		(tSirChanChangeResponse *)
		qdf_mem_malloc(sizeof(tSirChanChangeResponse));
	if (NULL == proam_info.channelChangeRespEvent) {
		status = QDF_STATUS_E_NOMEM;
		sme_err("Channel Change Event Allocation Failed: %d\n", status);
		return status;
	}
	if (msg_type == eWNI_SME_CHANNEL_CHANGE_RSP) {
		proam_info.channelChangeRespEvent->sessionId = SessionId;
		proam_info.channelChangeRespEvent->newChannelNumber =
			pChnlParams->channelNumber;

		if (pChnlParams->status == QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "sapdfs: Received success eWNI_SME_CHANNEL_CHANGE_RSP for sessionId[%d]",
				  SessionId);
			proam_info.channelChangeRespEvent->channelChangeStatus =
				1;
			roamResult = eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS;
		} else {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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
		sme_err("Invalid Channel Change Resp Message: %d",
			status);
	}
	qdf_mem_free(proam_info.channelChangeRespEvent);

	return status;
}

/*
 * sme_roam_start_beacon_req() -
 * API to Indicate LIM to start Beacon Tx after SAP CAC Wait is completed.
 *
 * hHal - The handle returned by mac_open
 * sessionId - session ID
 * dfsCacWaitStatus - CAC WAIT status flag
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_start_beacon_req(tHalHandle hHal, struct qdf_mac_addr bssid,
				     uint8_t dfsCacWaitStatus)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_start_beacon_req(pMac, bssid,
						dfsCacWaitStatus);
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
				struct ch_params *ch_params)
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

/*
 * sme_init_thermal_info() -
 * SME API to initialize the thermal mitigation parameters
 *
 * hHal
 * thermalParam : thermal mitigation parameters
 * Return QDF_STATUS
 */
QDF_STATUS sme_init_thermal_info(tHalHandle hHal, tSmeThermalParams
				thermalParam)
{
	t_thermal_mgmt *pWmaParam;
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pWmaParam = (t_thermal_mgmt *) qdf_mem_malloc(sizeof(t_thermal_mgmt));
	if (NULL == pWmaParam) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: could not allocate tThermalMgmt", __func__);
		return QDF_STATUS_E_NOMEM;
	}

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
			    (scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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
	struct scheduler_msg msg = {0};
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&pMac->sme)) {
		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_THERMAL_LEVEL;
		msg.bodyval = level;

		qdf_status =  scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
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

/*
 * sme_txpower_limit() -
 * SME API to set txpower limits
 *
 * hHal
 * psmetx : power limits for 2g/5g
 * Return QDF_STATUS
 */
QDF_STATUS sme_txpower_limit(tHalHandle hHal, tSirTxPowerLimit *psmetx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct scheduler_msg message = {0};
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
		message.type = WMA_TX_POWER_LIMIT;
		message.reserved = 0;
		message.bodyptr = tx_power_limit;

		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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

/*
 * sme_ap_disable_intra_bss_fwd() -
 * SME will send message to WMA to set Intra BSS in txrx
 *
 * hHal - The handle returned by mac_open
 * sessionId - session id ( vdev id)
 * disablefwd - bool value that indicate disable intrabss fwd disable
 * Return QDF_STATUS
 */
QDF_STATUS sme_ap_disable_intra_bss_fwd(tHalHandle hHal, uint8_t sessionId,
					bool disablefwd)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	int status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct scheduler_msg message = {0};
	tpDisableIntraBssFwd pSapDisableIntraFwd = NULL;

	/* Prepare the request to send to SME. */
	pSapDisableIntraFwd = qdf_mem_malloc(sizeof(tDisableIntraBssFwd));
	if (NULL == pSapDisableIntraFwd) {
		sme_err("Memory Allocation Failure!!!");
		return QDF_STATUS_E_NOMEM;
	}

	pSapDisableIntraFwd->sessionId = sessionId;
	pSapDisableIntraFwd->disableintrabssfwd = disablefwd;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		message.bodyptr = pSapDisableIntraFwd;
		message.type = WMA_SET_SAP_INTRABSS_DIS;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(pSapDisableIntraFwd);
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_STATS_EXT

/*
 * sme_stats_ext_register_callback() -
 * This function called to register the callback that send vendor event for
 * stats ext
 *
 * callback - callback to be registered
 */
void sme_stats_ext_register_callback(tHalHandle hHal, StatsExtCallback callback)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pMac->sme.StatsExtCallback = callback;
}

void sme_stats_ext2_register_callback(tHalHandle hal_handle,
	void (*stats_ext2_cb)(void *, struct sir_sme_rx_aggr_hole_ind *))
{
	tpAniSirGlobal pmac = PMAC_STRUCT(hal_handle);

	pmac->sme.stats_ext2_cb = stats_ext2_cb;
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


/*
 * sme_stats_ext_request() -
 * Function called when HDD receives STATS EXT vendor command from userspace
 *
 * sessionID - vdevID for the stats ext request
 * input - Stats Ext Request structure ptr
 * Return QDF_STATUS
 */
QDF_STATUS sme_stats_ext_request(uint8_t session_id, tpStatsExtRequestReq input)
{
	struct scheduler_msg msg = {0};
	tpStatsExtRequest data;
	size_t data_len;

	data_len = sizeof(tStatsExtRequest) + input->request_data_len;
	data = qdf_mem_malloc(data_len);

	if (data == NULL)
		return QDF_STATUS_E_NOMEM;

	data->vdev_id = session_id;
	data->request_data_len = input->request_data_len;
	if (input->request_data_len)
		qdf_mem_copy(data->request_data,
			     input->request_data, input->request_data_len);

	msg.type = WMA_STATS_EXT_REQUEST;
	msg.reserved = 0;
	msg.bodyptr = data;

	if (QDF_STATUS_SUCCESS != scheduler_post_msg(QDF_MODULE_ID_WMA,
						      &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_STATS_EXT_REQUEST message to WMA",
			  __func__);
		qdf_mem_free(data);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_stats_ext_event() -
 * This callback function called when SME received eWNI_SME_STATS_EXT_EVENT
 *  response from WMA
 *
 * hHal - HAL handle for device
 * pMsg - Message body passed from WMA; includes NAN header
 * Return QDF_STATUS
 */
QDF_STATUS sme_stats_ext_event(tHalHandle hHal, void *pMsg)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == pMsg) {
		sme_err("pMsg is NULL in sme_stats_ext_event");
		status = QDF_STATUS_E_FAILURE;
	} else {
		if (pMac->sme.StatsExtCallback)
			pMac->sme.StatsExtCallback(pMac->hHdd,
						   (tpStatsExtEvent) pMsg);
	}

	return status;
}

#endif

/*
 * sme_update_dfs_scan_mode() -
 * Update DFS roam scan mode
 *	    This function is called through dynamic setConfig callback function
 *	    to configure allowDFSChannelRoam.
 * hHal - HAL handle for device
 * sessionId - Session Identifier
 * allowDFSChannelRoam - DFS roaming scan mode 0 (disable),
 *	    1 (passive), 2 (active)
 * Return QDF_STATUS_SUCCESS - SME update DFS roaming scan config
 *	    successfully.
 *	  Other status means SME failed to update DFS roaming scan config.
 */
QDF_STATUS sme_update_dfs_scan_mode(tHalHandle hHal, uint8_t sessionId,
				    uint8_t allowDFSChannelRoam)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set AllowDFSChannelRoam Mode to %d - old value is %d - roam state is %s",
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

/*
 * sme_get_dfs_scan_mode() - get DFS roam scan mode
 *	  This is a synchronous call
 *
 * hHal - The handle returned by mac_open.
 * Return DFS roaming scan mode 0 (disable), 1 (passive), 2 (active)
 */
uint8_t sme_get_dfs_scan_mode(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	return pMac->roam.configParam.allowDFSChannelRoam;
}

/*
 * sme_modify_add_ie() -
 * This function sends msg to updates the additional IE buffers in PE
 *
 * hHal - global structure
 * pModifyIE - pointer to tModifyIE structure
 * updateType - type of buffer
 * Return Success or failure
 */
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

/*
 * sme_update_add_ie() -
 * This function sends msg to updates the additional IE buffers in PE
 *
 * hHal - global structure
 * pUpdateIE - pointer to structure tUpdateIE
 * updateType - type of buffer
 * Return Success or failure
 */
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
					 enum sme_qos_wmmuptype *dscpmapping,
					 uint8_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i, j, peSessionId;
	struct csr_roam_session *pCsrSession = NULL;
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
		sme_debug("QOS Mapping IE not present");
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
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_DEBUG,
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

/*
 * sme_abort_roam_scan() -
 * API to abort current roam scan cycle by roam scan offload module.
 *
 * hHal - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS
 */

QDF_STATUS sme_abort_roam_scan(tHalHandle hHal, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

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
	uint8_t numChannels = 0;
	uint8_t i = 0;
	uint32_t totValidChannels = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hHal);

	if (!aValidChannels || !pNumChannels) {
		sme_err("Output channel list/NumChannels is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (wifiBand >= WIFI_BAND_MAX) {
		sme_err("Invalid wifiBand: %d", wifiBand);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_get_cfg_valid_channels(&chanList[0],
			&totValidChannels);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Fail to get valid channel list (err=%d)", status);
		return status;
	}

	switch (wifiBand) {
	case WIFI_BAND_UNSPECIFIED:
		sme_debug("Unspec Band, return all %d valid channels",
			  totValidChannels);
		numChannels = totValidChannels;
		for (i = 0; i < totValidChannels; i++)
			aValidChannels[i] = cds_chan_to_freq(chanList[i]);
		break;

	case WIFI_BAND_BG:
		sme_debug("WIFI_BAND_BG (2.4 GHz)");
		for (i = 0; i < totValidChannels; i++) {
			if (WLAN_REG_IS_24GHZ_CH(chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_A:
		sme_debug("WIFI_BAND_A (5 GHz without DFS)");
		for (i = 0; i < totValidChannels; i++) {
			if (WLAN_REG_IS_5GHZ_CH(chanList[i]) &&
			    !wlan_reg_is_dfs_ch(mac_ctx->pdev, chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_ABG:
		sme_debug("WIFI_BAND_ABG (2.4 GHz + 5 GHz; no DFS)");
		for (i = 0; i < totValidChannels; i++) {
			if ((WLAN_REG_IS_24GHZ_CH(chanList[i]) ||
			     WLAN_REG_IS_5GHZ_CH(chanList[i])) &&
			    !wlan_reg_is_dfs_ch(mac_ctx->pdev, chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_A_DFS_ONLY:
		sme_debug("WIFI_BAND_A_DFS (5 GHz DFS only)");
		for (i = 0; i < totValidChannels; i++) {
			if (WLAN_REG_IS_5GHZ_CH(chanList[i]) &&
			    wlan_reg_is_dfs_ch(mac_ctx->pdev, chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_A_WITH_DFS:
		sme_debug("WIFI_BAND_A_WITH_DFS (5 GHz with DFS)");
		for (i = 0; i < totValidChannels; i++) {
			if (WLAN_REG_IS_5GHZ_CH(chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	case WIFI_BAND_ABG_WITH_DFS:
		sme_debug("WIFI_BAND_ABG_WITH_DFS (2.4 GHz+5 GHz with DFS)");
		for (i = 0; i < totValidChannels; i++) {
			if (WLAN_REG_IS_24GHZ_CH(chanList[i]) ||
			    WLAN_REG_IS_5GHZ_CH(chanList[i]))
				aValidChannels[numChannels++] =
					cds_chan_to_freq(chanList[i]);
		}
		break;

	default:
		sme_err("Unknown wifiBand: %d", wifiBand);
		return QDF_STATUS_E_INVAL;
	}
	*pNumChannels = numChannels;

	return status;
}

/*
 * sme_ext_scan_get_capabilities() -
 * SME API to fetch extscan capabilities
 *
 * hHal
 * pReq: extscan capabilities structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ext_scan_get_capabilities(tHalHandle hHal,
					 tSirGetExtScanCapabilitiesReqParams *
					 pReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pReq;
		message.type = WMA_EXTSCAN_GET_CAPABILITIES_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_ext_scan_start() -
 * SME API to issue extscan start
 *
 * hHal
 * pStartCmd: extscan start structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ext_scan_start(tHalHandle hHal,
			      tSirWifiScanCmdReqParams *pStartCmd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pStartCmd;
		message.type = WMA_EXTSCAN_START_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_ext_scan_stop() -
 * SME API to issue extscan stop
 *
 * hHal
 * pStopReq: extscan stop structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ext_scan_stop(tHalHandle hHal, tSirExtScanStopReqParams
			*pStopReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pStopReq;
		message.type = WMA_EXTSCAN_STOP_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_set_bss_hotlist() -
 * SME API to set BSSID hotlist
 *
 * hHal
 * pSetHotListReq: extscan set hotlist structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_bss_hotlist(tHalHandle hHal,
			       tSirExtScanSetBssidHotListReqParams *
			       pSetHotListReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pSetHotListReq;
		message.type = WMA_EXTSCAN_SET_BSSID_HOTLIST_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_reset_bss_hotlist() -
 * SME API to reset BSSID hotlist
 *
 * hHal
 * pSetHotListReq: extscan set hotlist structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_reset_bss_hotlist(tHalHandle hHal,
				 tSirExtScanResetBssidHotlistReqParams *
				 pResetReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pResetReq;
		message.type = WMA_EXTSCAN_RESET_BSSID_HOTLIST_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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
	struct scheduler_msg message = {0};
	struct sir_wisa_params *cds_msg_wisa_params;

	cds_msg_wisa_params = qdf_mem_malloc(sizeof(struct sir_wisa_params));
	if (!cds_msg_wisa_params)
		return QDF_STATUS_E_NOMEM;

	*cds_msg_wisa_params = *wisa_params;
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.bodyptr = cds_msg_wisa_params;
		message.type = WMA_SET_WISA_PARAMS;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_set_significant_change() -
 * SME API to set significant change
 *
 * hHal
 * pSetSignificantChangeReq: extscan set significant change structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_significant_change(tHalHandle hHal,
				      tSirExtScanSetSigChangeReqParams *
				      pSetSignificantChangeReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pSetSignificantChangeReq;
		message.type = WMA_EXTSCAN_SET_SIGNF_CHANGE_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_reset_significant_change
 * SME API to reset significant change
 *
 * hHal
 * pResetReq: extscan reset significant change structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_reset_significant_change(tHalHandle hHal,
				tSirExtScanResetSignificantChangeReqParams
					*pResetReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pResetReq;
		message.type = WMA_EXTSCAN_RESET_SIGNF_CHANGE_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_get_cached_results() -
 * SME API to get cached results
 *
 * hHal
 * pCachedResultsReq: extscan get cached results structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_cached_results(tHalHandle hHal,
				  tSirExtScanGetCachedResultsReqParams *
				  pCachedResultsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = pCachedResultsReq;
		message.type = WMA_EXTSCAN_GET_CACHED_RESULTS_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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
	struct scheduler_msg message = {0};
	struct wifi_epno_params *req_msg;
	int len, i;

	SME_ENTER();
	len = sizeof(*req_msg) +
		(input->num_networks * sizeof(struct wifi_epno_network));

	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sme_err("qdf_mem_malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	req_msg->num_networks = input->num_networks;
	req_msg->request_id = input->request_id;
	req_msg->session_id = input->session_id;

	/* Fill only when num_networks are non zero */
	if (req_msg->num_networks) {
		req_msg->min_5ghz_rssi = input->min_5ghz_rssi;
		req_msg->min_24ghz_rssi = input->min_24ghz_rssi;
		req_msg->initial_score_max = input->initial_score_max;
		req_msg->same_network_bonus = input->same_network_bonus;
		req_msg->secure_bonus = input->secure_bonus;
		req_msg->band_5ghz_bonus = input->band_5ghz_bonus;
		req_msg->current_connection_bonus =
			input->current_connection_bonus;

		for (i = 0; i < req_msg->num_networks; i++) {
			req_msg->networks[i].flags = input->networks[i].flags;
			req_msg->networks[i].auth_bit_field =
					input->networks[i].auth_bit_field;
			req_msg->networks[i].ssid.length =
					input->networks[i].ssid.length;
			qdf_mem_copy(req_msg->networks[i].ssid.ssId,
					input->networks[i].ssid.ssId,
					req_msg->networks[i].ssid.length);
		}
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = WMA_SET_EPNO_LIST_REQ;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
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
	struct scheduler_msg message = {0};
	struct wifi_passpoint_req *req_msg;
	int len, i;

	SME_ENTER();
	len = sizeof(*req_msg) +
		(input->num_networks * sizeof(struct wifi_passpoint_network));
	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sme_err("qdf_mem_malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

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
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = WMA_SET_PASSPOINT_LIST_REQ;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
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
	struct scheduler_msg message = {0};
	struct wifi_passpoint_req *req_msg;

	SME_ENTER();
	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sme_err("qdf_mem_malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	req_msg->request_id = input->request_id;
	req_msg->session_id = input->session_id;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = WMA_RESET_PASSPOINT_LIST_REQ;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
			status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
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

/*
 * sme_ll_stats_clear_req() -
 * SME API to clear Link Layer Statistics
 *
 * hHal
 * pclearStatsReq: Link Layer clear stats request params structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ll_stats_clear_req(tHalHandle hHal,
				  tSirLLStatsClearReq *pclearStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	tSirLLStatsClearReq *clear_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "staId = %u", pclearStatsReq->staId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "statsClearReqMask = 0x%X",
		  pclearStatsReq->statsClearReqMask);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "stopReq = %u", pclearStatsReq->stopReq);
	if (!sme_is_session_id_valid(hHal, pclearStatsReq->staId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid staId %d",
			  __func__, pclearStatsReq->staId);
		return QDF_STATUS_E_INVAL;
	}

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
		message.bodyptr = clear_stats_req;
		message.type = WMA_LINK_LAYER_STATS_CLEAR_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_CLEAR_REQ",
				  __func__);
			qdf_mem_free(clear_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);
		qdf_mem_free(clear_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/*
 * sme_ll_stats_set_req() -
 * SME API to set the Link Layer Statistics
 *
 * hHal
 * psetStatsReq: Link Layer set stats request params structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ll_stats_set_req(tHalHandle hHal, tSirLLStatsSetReq
				*psetStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	tSirLLStatsSetReq *set_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s:  MPDU Size = %u", __func__,
		  psetStatsReq->mpduSizeThreshold);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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
		message.bodyptr = set_stats_req;
		message.type = WMA_LINK_LAYER_STATS_SET_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_SET_REQ",
				  __func__);
			qdf_mem_free(set_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);
		qdf_mem_free(set_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/*
 * sme_ll_stats_get_req() -
 * SME API to get the Link Layer Statistics
 *
 * hHal
 * pgetStatsReq: Link Layer get stats request params structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ll_stats_get_req(tHalHandle hHal, tSirLLStatsGetReq
				*pgetStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	tSirLLStatsGetReq *get_stats_req;

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
		message.bodyptr = get_stats_req;
		message.type = WMA_LINK_LAYER_STATS_GET_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_GET_REQ",
				  __func__);

			qdf_mem_free(get_stats_req);
			status = QDF_STATUS_E_FAILURE;

		}
		sme_release_global_lock(&pMac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);
		qdf_mem_free(get_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/*
 * sme_set_link_layer_stats_ind_cb() -
 * SME API to trigger the stats are available  after get request
 *
 * hHal
 * callback_routine - HDD callback which needs to be invoked after
	   getting status notification from FW
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_link_layer_stats_ind_cb(tHalHandle hHal,
	void (*callback_routine)(void *callbackCtx, int indType, void *pRsp))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->sme.pLinkLayerStatsIndCallback = callback_routine;
		sme_release_global_lock(&pMac->sme);
	} else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);

	return status;
}

/**
 * sme_set_link_layer_ext_cb() - Register callback for link layer statistics
 * @hal: Mac global handle
 * @ll_stats_ext_cb: HDD callback which needs to be invoked after getting
 *                   status notification from FW
 *
 * Return: eHalStatus
 */
QDF_STATUS sme_set_link_layer_ext_cb(tHalHandle hal, void (*ll_stats_ext_cb)
				(tHddHandle callback_ctx, tSirLLStatsResults
				*rsp))
{
	QDF_STATUS status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (status == QDF_STATUS_SUCCESS) {
		mac->sme.link_layer_stats_ext_cb = ll_stats_ext_cb;
		sme_release_global_lock(&mac->sme);
	} else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_qcquire_global_lock error", __func__);
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
	} else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);

	return status;
}

/**
 * sme_ll_stats_set_thresh - set threshold for mac counters
 * @hal, hal layer handle
 * @threshold, threshold for mac counters
 *
 * Return: QDF_STATUS Enumeration
 */
QDF_STATUS sme_ll_stats_set_thresh(tHalHandle hal,
				   struct sir_ll_ext_stats_threshold *threshold)
{
	QDF_STATUS status;
	tpAniSirGlobal mac;
	struct scheduler_msg message = {0};
	struct sir_ll_ext_stats_threshold *thresh;

	if (!threshold) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("threshold is not valid"));
		return QDF_STATUS_E_INVAL;
	}

	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("hal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = PMAC_STRUCT(hal);

	thresh = qdf_mem_malloc(sizeof(*thresh));
	if (!thresh) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Fail to alloc mem", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	*thresh = *threshold;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = thresh;
		message.type    = WDA_LINK_LAYER_STATS_SET_THRESHOLD;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WDA_LL_STATS_GET_REQ",
				  __func__);
			qdf_mem_free(thresh);
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("sme_acquire_global_lock error"));
		qdf_mem_free(thresh);
	}
	return status;
}

#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

#ifdef WLAN_POWER_DEBUGFS
/**
 * sme_power_debug_stats_req() - SME API to collect Power debug stats
 * @callback_fn: Pointer to the callback function for Power stats event
 * @power_stats_context: Pointer to context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_power_debug_stats_req(tHalHandle hal, void (*callback_fn)
				(struct power_stats_response *response,
				void *context), void *power_stats_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct scheduler_msg msg = {0};

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (!callback_fn) {
			sme_err("Indication callback did not registered");
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_FAILURE;
		}

		mac_ctx->sme.power_debug_stats_context = power_stats_context;
		mac_ctx->sme.power_stats_resp_callback = callback_fn;
		msg.bodyptr = NULL;
		msg.type = WMA_POWER_DEBUG_STATS_REQ;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sme_err("not able to post WDA_POWER_DEBUG_STATS_REQ");
		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*
 * sme_update_roam_offload_enabled() - enable/disable roam offload feaure
 *  It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 *
 * hHal - The handle returned by mac_open.
 * nRoamOffloadEnabled - The bool to update with
 * Return QDF_STATUS_SUCCESS - SME update config successfully.
 *	   Other status means SME is failed to update.
 */

QDF_STATUS sme_update_roam_offload_enabled(tHalHandle hHal,
					   bool nRoamOffloadEnabled)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: LFR3:gRoamOffloadEnabled is changed from %d to %d",
			  __func__, pMac->roam.configParam.isRoamOffloadEnabled,
			  nRoamOffloadEnabled);
		pMac->roam.configParam.isRoamOffloadEnabled =
			nRoamOffloadEnabled;
		sme_release_global_lock(&pMac->sme);
	}

	return status;
}

/**
 * sme_update_roam_key_mgmt_offload_enabled() - enable/disable key mgmt offload
 * This is a synchronous call
 * @hal_ctx: The handle returned by mac_open.
 * @session_id: Session Identifier
 * @key_mgmt_offload_enabled: key mgmt enable/disable flag
 * @pmkid_modes: PMKID modes of PMKSA caching and OKC
 * Return: QDF_STATUS_SUCCESS - SME updated config successfully.
 * Other status means SME is failed to update.
 */

QDF_STATUS sme_update_roam_key_mgmt_offload_enabled(tHalHandle hal_ctx,
					uint8_t session_id,
					bool key_mgmt_offload_enabled,
					struct pmkid_mode_bits *pmkid_modes)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_ctx);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"%s: LFR3: key_mgmt_offload_enabled changed to %d",
				  __func__, key_mgmt_offload_enabled);
			status = csr_roam_set_key_mgmt_offload(mac_ctx,
						session_id,
						key_mgmt_offload_enabled,
						pmkid_modes);
		} else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}
#endif

/*
 * sme_get_temperature() -
 * SME API to get the pdev temperature
 *
 * hHal
 * temperature context
 * pCallbackfn: callback fn with response (temperature)
 * Return QDF_STATUS
 */
QDF_STATUS sme_get_temperature(tHalHandle hHal,
			       void *tempContext,
			       void (*pCallbackfn)(int temperature,
						   void *pContext))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if ((NULL == pCallbackfn) &&
		    (NULL == pMac->sme.pGetTemperatureCb)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"Indication Call back did not registered");
			sme_release_global_lock(&pMac->sme);
			return QDF_STATUS_E_FAILURE;
		} else if (NULL != pCallbackfn) {
			pMac->sme.pTemperatureCbContext = tempContext;
			pMac->sme.pGetTemperatureCb = pCallbackfn;
		}
		/* serialize the req through MC thread */
		message.bodyptr = NULL;
		message.type = WMA_GET_TEMPERATURE_REQ;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("Post Get Temperature msg fail"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

/*
 * sme_set_scanning_mac_oui() -
 * SME API to set scanning mac oui
 *
 * hHal
 * pScanMacOui: Scanning Mac Oui (input 3 bytes)
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_scanning_mac_oui(tHalHandle hHal, tSirScanMacOui
					*pScanMacOui)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		message.bodyptr = pScanMacOui;
		message.type = WMA_SET_SCAN_MAC_OUI_REQ;
		qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message);
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
/*
 * sme_set_dhcp_srv_offload() -
 * SME API to set DHCP server offload info
 *
 * hHal
 * pDhcpSrvInfo : DHCP server offload info struct
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_dhcp_srv_offload(tHalHandle hHal,
				    tSirDhcpSrvOffloadInfo *pDhcpSrvInfo)
{
	struct scheduler_msg message = {0};
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
		message.type = WMA_SET_DHCP_SERVER_OFFLOAD_CMD;
		message.bodyptr = pSmeDhcpSrvInfo;

		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_msg(QDF_MODULE_ID_WMA,
						 &message))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s:WMA_SET_DHCP_SERVER_OFFLOAD_CMD failed",
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

QDF_STATUS sme_send_unit_test_cmd(uint32_t vdev_id, uint32_t module_id,
				  uint32_t arg_count, uint32_t *arg)
{
	return wma_form_unit_test_cmd_and_send(vdev_id, module_id,
					       arg_count, arg);
}

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
/*
 * sme_set_led_flashing() -
 * API to set the Led flashing parameters.
 *
 * hHal - The handle returned by mac_open.
 * x0, x1 -  led flashing parameters
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_led_flashing(tHalHandle hHal, uint8_t type,
				uint32_t x0, uint32_t x1)
{
	QDF_STATUS status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	struct scheduler_msg message = {0};
	struct flashing_req_params *ledflashing;

	ledflashing = qdf_mem_malloc(sizeof(*ledflashing));
	if (!ledflashing) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to allocate memory for WMA_LED_TIMING_REQ");
		return QDF_STATUS_E_NOMEM;
	}

	ledflashing->req_id = 0;
	ledflashing->pattern_id = type;
	ledflashing->led_x0 = x0;
	ledflashing->led_x1 = x1;

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = ledflashing;
		message.type = WMA_LED_FLASHING_REQ;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&pMac->sme);
	}
	if (!QDF_IS_STATUS_SUCCESS(status))
		qdf_mem_free(ledflashing);

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
	struct csr_roam_session *session = CSR_GET_SESSION(mac, session_id);
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
	struct scheduler_msg msg = {0};
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
			    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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
	struct scheduler_msg msg = {0};
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
			    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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
	struct scheduler_msg msg = {0};
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

		iwcmd->param_value = modulated_dtim;
		iwcmd->param_vdev_id = session_id;
		iwcmd->param_id = GEN_PARAM_MODULATED_DTIM;
		iwcmd->param_vp_dev = GEN_CMD;
		msg.type = WMA_CLI_SET_CMD;
		msg.reserved = 0;
		msg.bodyptr = (void *)iwcmd;

		if (!QDF_IS_STATUS_SUCCESS(
			    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
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

/**
 * sme_override_listen_interval() - function to override static LI
 * @h_hal: SME API to override listen interval
 * @session_id: session ID
 * @override_li: new LI value passed by user
 *
 * This function override (enable/disable) static a.k.a ini based LI
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_override_listen_interval(tHalHandle h_hal, uint8_t session_id,
					uint32_t override_li)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac  = PMAC_STRUCT(h_hal);
	wma_cli_set_cmd_t *iwcmd;

	iwcmd = qdf_mem_malloc(sizeof(*iwcmd));
	if (!iwcmd) {
		QDF_TRACE(QDF_MODULE_ID_SME,
			  QDF_TRACE_LEVEL_FATAL,
			  "%s: qdf_mem_malloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	status = sme_acquire_global_lock(&mac->sme);

	if (status == QDF_STATUS_SUCCESS) {

		iwcmd->param_value = override_li;
		iwcmd->param_vdev_id = session_id;
		iwcmd->param_id = GEN_PARAM_LISTEN_INTERVAL;
		iwcmd->param_vp_dev = GEN_CMD;
		msg.type = WMA_CLI_SET_CMD;
		msg.reserved = 0;
		msg.bodyptr = (void *)iwcmd;

		if (!QDF_IS_STATUS_SUCCESS(
			    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Can't post GEN_PARAM_LISTEN_INTERVAL",
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
	struct scheduler_msg message = {0};
	struct sir_wifi_start_log *req_msg;
	uint32_t len;

	len = sizeof(*req_msg);
	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sme_err("qdf_mem_malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	req_msg->verbose_level = start_log.verbose_level;
	req_msg->is_iwpriv_command = start_log.is_iwpriv_command;
	req_msg->ring_id = start_log.ring_id;
	req_msg->ini_triggered = start_log.ini_triggered;
	req_msg->user_triggered = start_log.user_triggered;
	req_msg->size = start_log.size;
	req_msg->is_pktlog_buff_clear = start_log.is_pktlog_buff_clear;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme_acquire_global_lock failed(status=%d)", status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = SIR_HAL_START_STOP_LOGGING;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
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
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hHal);
	bool val = false;

	if (CSR_IS_SESSION_VALID(mac_ctx, sessionId))
		val = csr_neighbor_middle_of_roaming(mac_ctx, sessionId);
	else
		sme_err("Invalid Session: %d", sessionId);

	return val;
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
	struct scheduler_msg message = {0};

	/* Serialize the req through MC thread */
	message.bodyptr = NULL;
	message.type    = SIR_HAL_FLUSH_LOG_TO_FW;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
		status = QDF_STATUS_E_FAILURE;
	}
	return status;
}

QDF_STATUS sme_enable_uapsd_for_ac(uint8_t sta_id,
				   sme_ac_enum_type ac, uint8_t tid,
				   uint8_t pri, uint32_t srvc_int,
				   uint32_t sus_int,
				   enum sme_qos_wmm_dir_type dir,
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

QDF_STATUS sme_disable_uapsd_for_ac(uint8_t sta_id,
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
	struct csr_roam_session *csr_session;

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

#ifdef WLAN_FEATURE_11AX
void sme_update_he_cap_nss(tHalHandle hal, uint8_t session_id,
		uint8_t nss)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct csr_roam_session *csr_session;
	uint32_t tx_mcs_map = 0;
	uint32_t rx_mcs_map = 0;

	if (!nss || (nss > 2)) {
		sme_err("invalid Nss value %d", nss);
	}
	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	sme_cfg_get_int(mac_ctx, WNI_CFG_HE_RX_MCS_MAP_LT_80, &rx_mcs_map);
	sme_cfg_get_int(mac_ctx, WNI_CFG_HE_TX_MCS_MAP_LT_80, &tx_mcs_map);
	if (nss == 1) {
		tx_mcs_map = HE_SET_MCS_4_NSS(tx_mcs_map, HE_MCS_DISABLE, 2);
		rx_mcs_map = HE_SET_MCS_4_NSS(rx_mcs_map, HE_MCS_DISABLE, 2);
	} else {
		tx_mcs_map = HE_SET_MCS_4_NSS(tx_mcs_map, HE_MCS_0_11, 2);
		rx_mcs_map = HE_SET_MCS_4_NSS(rx_mcs_map, HE_MCS_0_11, 2);
	}
	sme_info("new HE Nss MCS MAP: Rx 0x%0X, Tx: 0x%0X",
			rx_mcs_map, tx_mcs_map);
	sme_cfg_set_int(mac_ctx, WNI_CFG_HE_RX_MCS_MAP_LT_80, rx_mcs_map);
	sme_cfg_set_int(mac_ctx, WNI_CFG_HE_TX_MCS_MAP_LT_80, tx_mcs_map);
	csr_update_session_he_cap(mac_ctx, csr_session);

}

int sme_update_he_mcs(tHalHandle hal, uint8_t session_id, uint16_t he_mcs)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct csr_roam_session *csr_session;
	uint16_t mcs_val = 0;
	uint16_t mcs_map = HE_MCS_ALL_DISABLED;
	uint32_t wni_cfg_tx_param = 0;
	uint32_t wni_cfg_rx_param = 0;

	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!csr_session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if ((he_mcs & 0x3) == HE_MCS_DISABLE) {
		sme_err("Invalid HE MCS 0x%0x, can't disable 0-7 for 1ss",
				he_mcs);
		return -EINVAL;
	}
	mcs_val = he_mcs & 0x3;
	switch (he_mcs) {
	case HE_80_MCS0_7:
	case HE_80_MCS0_9:
	case HE_80_MCS0_11:
		if (mac_ctx->roam.configParam.enable2x2) {
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 2);
		} else {
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		}
		wni_cfg_tx_param = WNI_CFG_HE_TX_MCS_MAP_LT_80;
		wni_cfg_rx_param = WNI_CFG_HE_RX_MCS_MAP_LT_80;
		break;

	case HE_160_MCS0_7:
	case HE_160_MCS0_9:
	case HE_160_MCS0_11:
		mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		wni_cfg_tx_param = WNI_CFG_HE_TX_MCS_MAP_160;
		wni_cfg_rx_param = WNI_CFG_HE_RX_MCS_MAP_160;
		break;

	case HE_80p80_MCS0_7:
	case HE_80p80_MCS0_9:
	case HE_80p80_MCS0_11:
		mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		wni_cfg_tx_param = WNI_CFG_HE_TX_MCS_MAP_80_80;
		wni_cfg_rx_param = WNI_CFG_HE_RX_MCS_MAP_80_80;
		break;

	default:
		sme_err("Invalid HE MCS 0x%0x", he_mcs);
		return -EINVAL;
	}
	sme_info("new HE MCS 0x%0x", mcs_map);
	sme_cfg_set_int(mac_ctx, wni_cfg_tx_param, mcs_map);
	sme_cfg_set_int(mac_ctx, wni_cfg_rx_param, mcs_map);
	csr_update_session_he_cap(mac_ctx, csr_session);

	return 0;
}

static int sme_update_he_cap(tHalHandle hal, uint8_t session_id,
		uint16_t he_cap, int value)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	sme_cfg_set_int(mac_ctx, he_cap, value);
	csr_update_session_he_cap(mac_ctx, session);

	return 0;
}

int sme_update_he_tx_stbc_cap(tHalHandle hal, uint8_t session_id, int value)
{
	int ret;
	uint32_t he_cap_val = 0;

	he_cap_val = value ? 1 : 0;

	ret = sme_update_he_cap(hal, session_id,
			 WNI_CFG_HE_TX_STBC_LT80, he_cap_val);
	if (ret)
		return ret;

	return sme_update_he_cap(hal, session_id,
			 WNI_CFG_HE_TX_STBC_GT80, he_cap_val);
}

int sme_update_he_rx_stbc_cap(tHalHandle hal, uint8_t session_id, int value)
{
	int ret;
	uint32_t he_cap_val = 0;

	he_cap_val = value ? 1 : 0;

	ret = sme_update_he_cap(hal, session_id,
			 WNI_CFG_HE_RX_STBC_LT80, he_cap_val);
	if (ret)
		return ret;

	return sme_update_he_cap(hal, session_id,
			 WNI_CFG_HE_RX_STBC_GT80, he_cap_val);
}

int sme_update_he_frag_supp(tHalHandle hal, uint8_t session_id,
		uint16_t he_frag)
{
	return sme_update_he_cap(hal, session_id,
			 WNI_CFG_HE_FRAGMENTATION, he_frag);
}
#endif

/**
 * sme_set_nud_debug_stats_cb() - set nud debug stats callback
 * @hal: global hal handle
 * @cb: callback function pointer
 *
 * This function stores nud debug stats callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_nud_debug_stats_cb(tHalHandle hal,
				void (*cb)(void *, struct rsp_stats *))
{
	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac;

	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("hal is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		return status;
	}

	mac->sme.get_arp_stats_cb = cb;
	sme_release_global_lock(&mac->sme);
	return status;
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
		sme_err("sme_acquire_global_lock failed!(status=%d)", status);
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

QDF_STATUS sme_set_chip_pwr_save_fail_cb(tHalHandle hal,
		 void (*cb)(void *,
		 struct chip_pwr_save_fail_detected_params *)) {

	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme_AcquireGlobalLock failed!(status=%d)", status);
		return status;
	}
	mac->sme.chip_power_save_fail_cb = cb;
	sme_release_global_lock(&mac->sme);
	return status;
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
	struct scheduler_msg message = {0};
	struct rssi_monitor_req *req_msg;

	SME_ENTER();
	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg) {
		sme_err("memory allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	*req_msg = *input;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)", status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = WMA_SET_RSSI_MONITOR_REQ;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

/*
 * sme_pdev_set_pcl() - Send WMI_PDEV_SET_PCL_CMDID to the WMA
 * @hal: Handle returned by macOpen
 * @msg: PCL channel list and length structure
 *
 * Sends the command to WMA to send WMI_PDEV_SET_PCL_CMDID to FW
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_pdev_set_pcl(struct policy_mgr_pcl_list msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac   = sme_get_mac_context();
	struct scheduler_msg message = {0};
	struct wmi_pcl_chan_weights *req_msg;
	uint32_t len, i;

	if (!mac) {
		sme_err("mac is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	len = sizeof(*req_msg);

	req_msg = qdf_mem_malloc(len);
	if (!req_msg) {
		sme_err("qdf_mem_malloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	for (i = 0; i < msg.pcl_len; i++) {
		req_msg->pcl_list[i] =  msg.pcl_list[i];
		req_msg->weight_list[i] =  msg.weight_list[i];
	}

	req_msg->pcl_len = msg.pcl_len;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme_acquire_global_lock failed!(status=%d)", status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = SIR_HAL_PDEV_SET_PCL_TO_FW;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
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
QDF_STATUS sme_pdev_set_hw_mode(struct policy_mgr_hw_mode msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = sme_get_mac_context();
	tSmeCmd *cmd = NULL;

	if (!mac) {
		sme_err("mac is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to acquire lock");
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = csr_get_command_buffer(mac);
	if (!cmd) {
		sme_err("Get command buffer failed");
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_hw_mode;
	cmd->sessionId = msg.session_id;
	cmd->u.set_hw_mode_cmd.hw_mode_index = msg.hw_mode_index;
	cmd->u.set_hw_mode_cmd.set_hw_mode_cb = msg.set_hw_mode_cb;
	cmd->u.set_hw_mode_cmd.reason = msg.reason;
	cmd->u.set_hw_mode_cmd.session_id = msg.session_id;
	cmd->u.set_hw_mode_cmd.next_action = msg.next_action;
	cmd->u.set_hw_mode_cmd.context = msg.context;

	sme_debug("Queuing set hw mode to CSR, session: %d reason: %d",
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
QDF_STATUS sme_nss_update_request(uint32_t vdev_id,
				uint8_t  new_nss, policy_mgr_nss_update_cback cback,
				uint8_t next_action, struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal mac = sme_get_mac_context();
	tSmeCmd *cmd = NULL;

	if (!mac) {
		sme_err("mac is null");
		return status;
	}
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cmd = csr_get_command_buffer(mac);
		if (!cmd) {
			sme_err("Get command buffer failed");
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NULL_VALUE;
		}
		cmd->command = e_sme_command_nss_update;
		/* Sessionized modules may require this info */
		cmd->sessionId = vdev_id;
		cmd->u.nss_update_cmd.new_nss = new_nss;
		cmd->u.nss_update_cmd.session_id = vdev_id;
		cmd->u.nss_update_cmd.nss_update_cb = cback;
		cmd->u.nss_update_cmd.context = psoc;
		cmd->u.nss_update_cmd.next_action = next_action;
		cmd->u.nss_update_cmd.reason = reason;

		sme_debug("Queuing e_sme_command_nss_update to CSR");
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
QDF_STATUS sme_soc_set_dual_mac_config(struct policy_mgr_dual_mac_config msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = sme_get_mac_context();
	tSmeCmd *cmd;

	if (!mac) {
		sme_err("mac is null");
		return QDF_STATUS_E_FAILURE;
	}
	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to acquire lock");
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = csr_get_command_buffer(mac);
	if (!cmd) {
		sme_err("Get command buffer failed");
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_dual_mac_config;
	cmd->u.set_dual_mac_cmd.scan_config = msg.scan_config;
	cmd->u.set_dual_mac_cmd.fw_mode_config = msg.fw_mode_config;
	cmd->u.set_dual_mac_cmd.set_dual_mac_cb = msg.set_dual_mac_cb;

	sme_debug("set_dual_mac_config scan_config: %x fw_mode_config: %x",
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
	struct scheduler_msg message = {0};
	struct gateway_param_update_req *request_buf;

	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (NULL == request_buf) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to allocate memory for gw param update request");
		return QDF_STATUS_E_NOMEM;
	}

	*request_buf = *gw_params;

	message.type = WMA_GW_PARAM_UPDATE_REQ;
	message.reserved = 0;
	message.bodyptr = request_buf;
	qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"Not able to post WMA_GW_PARAM_UPDATE_REQ message to HAL");
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
		sme_err("antenna mode mesg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to acquire lock");
		return QDF_STATUS_E_RESOURCES;
	}

	cmd = csr_get_command_buffer(mac);
	if (!cmd) {
		sme_release_global_lock(&mac->sme);
		sme_err("Get command buffer failed");
		return QDF_STATUS_E_NULL_VALUE;
	}

	cmd->command = e_sme_command_set_antenna_mode;
	cmd->u.set_antenna_mode_cmd = *msg;

	sme_debug("Antenna mode rx_chains: %d tx_chains: %d",
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
	struct csr_neighbor_roamconfig *nr_config = NULL;
	tpCsrNeighborRoamControlInfo nr_info = NULL;
	uint32_t reason = 0;

	if (session_id >= CSR_ROAM_SESSION_MAX) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), session_id);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		nr_config = &mac_ctx->roam.configParam.neighborRoamConfig;
		nr_info   = &mac_ctx->roam.neighborRoamInfo[session_id];
		switch (notify_id) {
		case eCSR_HI_RSSI_SCAN_MAXCOUNT_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"%s: gRoamScanHirssiMaxCount %d => %d",
				__func__, nr_config->nhi_rssi_scan_max_count,
				val);
			nr_config->nhi_rssi_scan_max_count = val;
			nr_info->cfgParams.hi_rssi_scan_max_count = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_MAXCOUNT_CHANGED;
		break;

		case eCSR_HI_RSSI_SCAN_RSSI_DELTA_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				FL("gRoamScanHiRssiDelta %d => %d"),
				nr_config->nhi_rssi_scan_rssi_delta,
				val);
			nr_config->nhi_rssi_scan_rssi_delta = val;
			nr_info->cfgParams.hi_rssi_scan_rssi_delta = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_DELTA_CHANGED;
			break;

		case eCSR_HI_RSSI_SCAN_DELAY_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				FL("gRoamScanHiRssiDelay %d => %d"),
				nr_config->nhi_rssi_scan_delay,
				val);
			nr_config->nhi_rssi_scan_delay = val;
			nr_info->cfgParams.hi_rssi_scan_delay = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_DELAY_CHANGED;
			break;

		case eCSR_HI_RSSI_SCAN_RSSI_UB_ID:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				FL("gRoamScanHiRssiUpperBound %d => %d"),
				nr_config->nhi_rssi_scan_rssi_ub,
				val);
			nr_config->nhi_rssi_scan_rssi_ub = val;
			nr_info->cfgParams.hi_rssi_scan_rssi_ub = val;
			reason = REASON_ROAM_SCAN_HI_RSSI_UB_CHANGED;
			break;

		default:
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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
	mac_ctx->beacon_offload = cfg->beacon_offload;
	mac_ctx->pmf_offload = cfg->pmf_offload;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		FL("mac_ctx->pmf_offload: %d"), mac_ctx->pmf_offload);
	mac_ctx->is_fils_roaming_supported =
				cfg->is_fils_roaming_supported;
	mac_ctx->is_11k_offload_supported =
				cfg->is_11k_offload_supported;
	sme_debug("pmf_offload: %d fils_roam support %d 11k_offload %d",
		  mac_ctx->pmf_offload, mac_ctx->is_fils_roaming_supported,
		  mac_ctx->is_11k_offload_supported);
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: null mac pointer", __func__);
		return false;
	}

	if (CSR_IS_SESSION_VALID(mac, session_id))
		return true;

	return false;
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
		*opclass = wlan_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BW40_LOW_PRIMARY);
		if (!(*opclass)) {
			*opclass = wlan_reg_dmn_get_opclass_from_channel(
					mac_ctx->scan.countryCodeCurrent,
					channel, BW40_HIGH_PRIMARY);
		}
	} else if (bw_offset & (1 << BW_20_OFFSET_BIT)) {
		*opclass = wlan_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BW20);
	} else {
		*opclass = wlan_reg_dmn_get_opclass_from_channel(
				mac_ctx->scan.countryCodeCurrent,
				channel, BWALL);
	}
}
#endif

/**
 * sme_set_fw_test() - set fw test
 * @fw_test: fw test param
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_set_fw_test(struct set_fwtest_params *fw_test)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	wma_process_fw_test_cmd(wma_handle, fw_test);
	return QDF_STATUS_SUCCESS;
}

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

	sme_debug("SMPS enable: %d mode: %d send action: %d",
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
	struct csr_roam_session *csr_session;

	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == csr_session) {
		sme_err("SME session not valid: %d", session_id);
		return false;
	}

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
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
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
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
	qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
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
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
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
	qdf_status = scheduler_post_msg(QDF_MODULE_ID_WMA,
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
 * sme_send_disassoc_req_frame - send disassoc req
 * @hal: handler to hal
 * @session_id: session id
 * @peer_mac: peer mac address
 * @reason: reason for disassociation
 * wait_for_ack: wait for acknowledgment
 *
 * function to send disassoc request to lim
 *
 * return: none
 */
void sme_send_disassoc_req_frame(tHalHandle hal, uint8_t session_id,
		uint8_t *peer_mac, uint16_t reason, uint8_t wait_for_ack)
{
	struct sme_send_disassoc_frm_req *msg;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	A_UINT8  *buf;
	A_UINT16 tmp;

	msg = qdf_mem_malloc(sizeof(struct sme_send_disassoc_frm_req));

	if (NULL == msg)
		qdf_status = QDF_STATUS_E_FAILURE;
	else
		qdf_status = QDF_STATUS_SUCCESS;

	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		return;

	msg->msg_type = (uint16_t) eWNI_SME_SEND_DISASSOC_FRAME;

	msg->length = (uint16_t) sizeof(struct sme_send_disassoc_frm_req);

	buf = &msg->session_id;

	/* session id */
	*buf = (A_UINT8) session_id;
	buf += sizeof(A_UINT8);

	/* transaction id */
	*buf = 0;
	*(buf + 1) = 0;
	buf += sizeof(A_UINT16);

	/* Set the peer MAC address before sending the message to LIM */
	qdf_mem_copy(buf, peer_mac, QDF_MAC_ADDR_SIZE);

	buf += QDF_MAC_ADDR_SIZE;

	/* reasoncode */
	tmp = (uint16_t) reason;
	qdf_mem_copy(buf, &tmp, sizeof(uint16_t));
	buf += sizeof(uint16_t);

	*buf =  wait_for_ack;
	buf += sizeof(uint8_t);

	qdf_status = umac_send_mb_message_to_mac(msg);

	if (qdf_status != QDF_STATUS_SUCCESS)
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("cds_send_mb_message Failed"));
}

QDF_STATUS sme_get_bpf_offload_capabilities(tHalHandle hal,
					    bpf_get_offload_cb callback,
					    void *context)
{
	QDF_STATUS          status     = QDF_STATUS_SUCCESS;
	tpAniSirGlobal      mac_ctx      = PMAC_STRUCT(hal);
	struct scheduler_msg           cds_msg = {0};

	SME_ENTER();

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		mac_ctx->sme.bpf_get_offload_cb = callback;
		mac_ctx->sme.bpf_get_offload_context = context;
		cds_msg.bodyptr = NULL;
		cds_msg.type = WDA_BPF_GET_CAPABILITIES_REQ;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &cds_msg);
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
	SME_EXIT();
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
	struct scheduler_msg           cds_msg = {0};
	struct sir_bpf_set_offload *set_offload;

	set_offload = qdf_mem_malloc(sizeof(*set_offload) +
					req->current_length);

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
		set_offload->program = ((uint8_t *)set_offload) +
					sizeof(*set_offload);
		qdf_mem_copy(set_offload->program, req->program,
				set_offload->current_length);
	}
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		cds_msg.bodyptr = set_offload;
		cds_msg.type = WDA_BPF_SET_INSTRUCTIONS_REQ;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &cds_msg);

		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Post BPF set offload msg fail"));
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(set_offload);
		}
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("sme_acquire_global_lock failed"));
		qdf_mem_free(set_offload);
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
		status = umac_send_mb_message_to_mac(msg);
	}
	return status;
}

void sme_set_chan_info_callback(tHalHandle hal_handle,
			void (*callback)(struct scan_chan_info *chan_info))
{
	tpAniSirGlobal mac;

	if (hal_handle == NULL) {
		QDF_ASSERT(0);
		return;
	}
	mac = PMAC_STRUCT(hal_handle);
	mac->chan_info_cb = callback;
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
	struct scheduler_msg message = {0};
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
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WMA!", __func__);

		qdf_mem_free(dwelltime_params);
	}
	return status;
}

/**
 * sme_set_vdev_ies_per_band() - sends the per band IEs to vdev
 * @hal: Pointer to HAL
 * @vdev_id: vdev_id for which IE is targeted
 *
 * Return: None
 */
void sme_set_vdev_ies_per_band(tHalHandle hal, uint8_t vdev_id)
{
	struct sir_set_vdev_ies_per_band *p_msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p_msg = qdf_mem_malloc(sizeof(*p_msg));
	if (NULL == p_msg) {
		sme_err("mem alloc failed for sme msg");
		return;
	}

	p_msg->vdev_id = vdev_id;
	p_msg->msg_type = eWNI_SME_SET_VDEV_IES_PER_BAND;
	p_msg->len = sizeof(*p_msg);
	sme_debug("sending eWNI_SME_SET_VDEV_IES_PER_BAND: vdev_id: %d",
		vdev_id);
	status = umac_send_mb_message_to_mac(p_msg);
	if (QDF_STATUS_SUCCESS != status)
		sme_err("Send eWNI_SME_SET_VDEV_IES_PER_BAND fail");
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
			sme_err("mem alloc failed for ht_vht_cfg");
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
		sme_debug("SET_HT_VHT_IE with nss: %d, dot11mode: %d",
			  ht_vht_cfg->nss,
			  ht_vht_cfg->dot11mode);
		status = umac_send_mb_message_to_mac(ht_vht_cfg);
		if (QDF_STATUS_SUCCESS != status)
			sme_err("Send SME_PDEV_SET_HT_VHT_IE fail");

		sme_release_global_lock(&mac_ctx->sme);
	}
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
		uint32_t vdev_type_nss, enum band_info band)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct vdev_type_nss *vdev_nss;

	if (BAND_5G == band)
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

	sme_debug("band %d NSS:sta %d sap %d cli %d go %d dev %d ibss %d tdls %d ocb %d",
		band, vdev_nss->sta, vdev_nss->sap, vdev_nss->p2p_cli,
		vdev_nss->p2p_go, vdev_nss->p2p_dev, vdev_nss->ibss,
		vdev_nss->tdls, vdev_nss->ocb);
}

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
#define MAX_BSS_COLOR_VAL 63
#define MIN_BSS_COLOR_VAL 1

QDF_STATUS sme_set_he_bss_color(tHalHandle hal, uint8_t session_id,
		uint8_t bss_color)

{
	struct sir_set_he_bss_color *bss_color_msg;
	uint8_t len;

	if (!hal) {
		sme_err("Invalid hal pointer");
		return QDF_STATUS_E_FAULT;
	}

	sme_debug("Set HE bss_color  %d", bss_color);

	if (bss_color < MIN_BSS_COLOR_VAL || bss_color > MAX_BSS_COLOR_VAL) {
		sme_debug("Invalid HE bss_color  %d", bss_color);
		return QDF_STATUS_E_INVAL;
	}
	len = sizeof(*bss_color_msg);
	bss_color_msg = qdf_mem_malloc(len);
	if (!bss_color_msg) {
		sme_err("mem alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	bss_color_msg->message_type = eWNI_SME_SET_HE_BSS_COLOR;
	bss_color_msg->length = len;
	bss_color_msg->session_id = session_id;
	bss_color_msg->bss_color = bss_color;
	return umac_send_mb_message_to_mac(bss_color_msg);
}
#endif

/**
 * sme_update_hw_dbs_capable() - sets the HW DBS capability
 * @hal: Pointer to HAL
 * @hw_dbs_capable: HW DBS capability
 *
 * Sets HW DBS capability based on INI and fw capability.
 *
 * Return: None
 */
void sme_update_hw_dbs_capable(tHalHandle hal, uint8_t hw_dbs_capable)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	mac_ctx->hw_dbs_capable = hw_dbs_capable;
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
	struct scheduler_msg message = {0};
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct sir_mac_pwr_dbg_cmd *req;
	int i;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
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

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_msg
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
 * @dev_mode: connection type.
 * @nss2g: Pointer to the 2G Nss parameter.
 * @nss5g: Pointer to the 5G Nss parameter.
 *
 * Fills the 2G and 5G Nss values based on connection type.
 *
 * Return: None
 */
void sme_get_vdev_type_nss(enum QDF_OPMODE dev_mode,
			   uint8_t *nss_2g, uint8_t *nss_5g)
{
	tpAniSirGlobal mac_ctx = sme_get_mac_context();

	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		FL("Invalid MAC context"));
		return;
	}
	csr_get_vdev_type_nss(mac_ctx, dev_mode, nss_2g, nss_5g);
}

/**
 * sme_update_sta_roam_policy() - update sta roam policy for
 * unsafe and DFS channels.
 * @hal_handle: hal handle for getting global mac struct
 * @dfs_mode: dfs mode which tell if dfs channel needs to be
 * skipped or not
 * @skip_unsafe_channels: Param to tell if driver needs to
 * skip unsafe channels or not.
 * @param session_id: sme_session_id
 * @sap_operating_band: Band on which SAP is operating
 *
 * sme_update_sta_roam_policy update sta rome policies to csr
 * this function will call csrUpdateChannelList as well
 * to include/exclude DFS channels and unsafe channels.
 *
 * Return: eHAL_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_update_sta_roam_policy(tHalHandle hal_handle,
		enum sta_roam_policy_dfs_mode dfs_mode,
		bool skip_unsafe_channels,
		uint8_t session_id, uint8_t sap_operating_band)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeConfigParams *sme_config;

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
				"%s: mac_ctx is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("failed to allocate memory for sme_config"));
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_zero(sme_config, sizeof(*sme_config));
	sme_get_config_param(hal_handle, sme_config);

	sme_config->csrConfig.sta_roam_policy_params.dfs_mode =
		dfs_mode;
	sme_config->csrConfig.sta_roam_policy_params.skip_unsafe_channels =
		skip_unsafe_channels;
	sme_config->csrConfig.sta_roam_policy_params.sap_operating_band =
		sap_operating_band;

	sme_update_config(hal_handle, sme_config);

	status = csr_update_channel_list(mac_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("failed to update the supported channel list"));
	}
	if (mac_ctx->roam.configParam.isRoamOffloadScanEnabled)
		csr_roam_offload_scan(mac_ctx, session_id,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_ROAM_SCAN_STA_ROAM_POLICY_CHANGED);

	qdf_mem_free(sme_config);
	return status;
}

/**
 * sme_enable_disable_chanavoidind_event - configure ca event ind
 * @hal: handler to hal
 * @set_value: enable/disable
 *
 * function to enable/disable chan avoidance indication
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_enable_disable_chanavoidind_event(tHalHandle hal,
		uint8_t set_value)
{
	QDF_STATUS status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct scheduler_msg msg = {0};

	sme_debug("set_value: %d", set_value);
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		qdf_mem_zero(&msg, sizeof(struct scheduler_msg));
		msg.type = WMA_SEND_FREQ_RANGE_CONTROL_IND;
		msg.bodyval = set_value;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
		sme_release_global_lock(&mac_ctx->sme);
		return status;
	}
	return status;
}

/*
 * sme_set_default_scan_ie() - API to send default scan IE to LIM
 * @hal: reference to the HAL
 * @session_id: current session ID
 * @ie_data: Pointer to Scan IE data
 * @ie_len: Length of @ie_data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_default_scan_ie(tHalHandle hal, uint16_t session_id,
					uint8_t *ie_data, uint16_t ie_len)
{
	QDF_STATUS status;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	struct hdd_default_scan_ie *set_ie_params;

	if (!ie_data)
		return QDF_STATUS_E_INVAL;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		set_ie_params = qdf_mem_malloc(sizeof(*set_ie_params));
		if (!set_ie_params)
			status = QDF_STATUS_E_NOMEM;
		else {
			set_ie_params->message_type = eWNI_SME_DEFAULT_SCAN_IE;
			set_ie_params->length = sizeof(*set_ie_params);
			set_ie_params->session_id = session_id;
			set_ie_params->ie_len = ie_len;
			qdf_mem_copy(set_ie_params->ie_data, ie_data, ie_len);
			status = umac_send_mb_message_to_mac(set_ie_params);
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}

QDF_STATUS sme_get_sar_power_limits(tHalHandle hal,
				    wma_sar_cb callback, void *context)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_get_sar_limit(wma_handle, callback, context);
}

QDF_STATUS sme_set_sar_power_limits(tHalHandle hal,
				    struct sar_limit_cmd_params *sar_limit_cmd)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_set_sar_limit(wma_handle, sar_limit_cmd);
}

#ifdef WLAN_FEATURE_FIPS
QDF_STATUS sme_fips_request(tHalHandle hal, struct fips_params *param,
			    wma_fips_cb callback, void *context)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sme_err("wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_fips_request(wma_handle, param, callback, context);
}
#endif

QDF_STATUS sme_set_cts2self_for_p2p_go(tHalHandle hal_handle)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (QDF_STATUS_SUCCESS !=
		wma_set_cts2self_for_p2p_go(wma_handle, true)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to set cts2self for p2p GO to firmware",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_update_tx_fail_cnt_threshold() - update tx fail count Threshold
 * @hal: Handle returned by mac_open
 * @session_id: Session ID on which tx fail count needs to be updated to FW
 * @tx_fail_count: Count for tx fail threshold after which FW will disconnect
 *
 * This function is used to set tx fail count threshold to firmware.
 * firmware will issue disocnnect with peer device once this threshold is
 * reached.
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_update_tx_fail_cnt_threshold(tHalHandle hal_handle,
				uint8_t session_id, uint32_t tx_fail_count)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sme_tx_fail_cnt_threshold *tx_fail_cnt;
	struct scheduler_msg msg = {0};

	tx_fail_cnt = qdf_mem_malloc(sizeof(*tx_fail_cnt));
	if (NULL == tx_fail_cnt) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: fail to alloc filter_param", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	sme_debug("session_id: %d tx_fail_count: %d",
		  session_id, tx_fail_count);
	tx_fail_cnt->session_id = session_id;
	tx_fail_cnt->tx_fail_cnt_threshold = tx_fail_count;

	qdf_mem_zero(&msg, sizeof(struct scheduler_msg));
	msg.type = SIR_HAL_UPDATE_TX_FAIL_CNT_TH;
	msg.reserved = 0;
	msg.bodyptr = tx_fail_cnt;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post Tx fail count message to WDA"));
		qdf_mem_free(tx_fail_cnt);
	}
	return status;
}

QDF_STATUS sme_set_lost_link_info_cb(tHalHandle hal,
				void (*cb)(void *, struct sir_lost_link_info *))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.lost_link_info_cb = cb;
		sme_release_global_lock(&mac->sme);
	} else {
		sme_err("sme_acquire_global_lock error status: %d", status);
	}

	return status;
}

#ifdef FEATURE_WLAN_ESE
bool sme_roam_is_ese_assoc(struct csr_roam_info *roam_info)
{
	return roam_info->isESEAssoc;
}
#endif
/**
 * sme_set_5g_band_pref(): If 5G preference is enabled,set boost/drop
 * params from ini.
 * @hal_handle: Handle returned by mac_open
 * @5g_pref_params: pref params from ini.
 *
 * Returns: None
 */
void sme_set_5g_band_pref(tHalHandle hal_handle,
			  struct sme_5g_band_pref_params *pref_params)
{

	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_handle);
	struct roam_ext_params *roam_params;
	QDF_STATUS status    = QDF_STATUS_SUCCESS;

	if (!pref_params) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Invalid 5G pref params!");
		return;
	}
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		roam_params = &mac_ctx->roam.configParam.roam_params;
		roam_params->raise_rssi_thresh_5g =
				pref_params->rssi_boost_threshold_5g;
		roam_params->raise_factor_5g =
				pref_params->rssi_boost_factor_5g;
		roam_params->max_raise_rssi_5g =
				pref_params->max_rssi_boost_5g;
		roam_params->drop_rssi_thresh_5g =
				pref_params->rssi_penalize_threshold_5g;
		roam_params->drop_factor_5g =
				pref_params->rssi_penalize_factor_5g;
		roam_params->max_drop_rssi_5g =
				pref_params->max_rssi_penalize_5g;

		sme_release_global_lock(&mac_ctx->sme);
	} else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "Unable to acquire global sme lock");
}


bool sme_neighbor_roam_is11r_assoc(tHalHandle hal_ctx, uint8_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_ctx);

	return csr_neighbor_roam_is11r_assoc(mac_ctx, session_id);
}

#ifdef WLAN_FEATURE_WOW_PULSE
/**
 * sme_set_wow_pulse() - set wow pulse info
 * @wow_pulse_set_info: wow_pulse_mode structure pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_wow_pulse(struct wow_pulse_mode *wow_pulse_set_info)
{
	struct scheduler_msg message = {0};
	QDF_STATUS status;
	struct wow_pulse_mode *wow_pulse_set_cmd;

	if (!wow_pulse_set_info) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: invalid wow_pulse_set_info pointer", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	wow_pulse_set_cmd = qdf_mem_malloc(sizeof(*wow_pulse_set_cmd));
	if (NULL == wow_pulse_set_cmd) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: fail to alloc wow_pulse_set_cmd", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	*wow_pulse_set_cmd = *wow_pulse_set_info;

	message.type = WMA_SET_WOW_PULSE_CMD;
	message.bodyptr = wow_pulse_set_cmd;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA,
					&message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Not able to post msg to WDA!",
			__func__);
		qdf_mem_free(wow_pulse_set_cmd);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}
#endif

/**
 * sme_prepare_beacon_from_bss_descp() - prepares beacon frame by populating
 * different fields and IEs from bss descriptor.
 * @frame_buf: frame buffer to populate
 * @bss_descp: bss descriptor
 * @bssid: bssid of the beacon frame to populate
 * @ie_len: length of IE fields
 *
 * Return: None
 */
static void sme_prepare_beacon_from_bss_descp(uint8_t *frame_buf,
					      tSirBssDescription *bss_descp,
					      const tSirMacAddr bssid,
					      uint32_t ie_len)
{
	tDot11fBeacon1 *bcn_fixed;
	tpSirMacMgmtHdr mac_hdr = (tpSirMacMgmtHdr)frame_buf;

	/* populate mac header first to indicate beacon */
	mac_hdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
	mac_hdr->fc.type = SIR_MAC_MGMT_FRAME;
	mac_hdr->fc.subType = SIR_MAC_MGMT_BEACON;
	qdf_mem_copy((uint8_t *) mac_hdr->da,
		     (uint8_t *) "\xFF\xFF\xFF\xFF\xFF\xFF",
		     sizeof(struct qdf_mac_addr));
	qdf_mem_copy((uint8_t *) mac_hdr->sa, bssid,
		     sizeof(struct qdf_mac_addr));
	qdf_mem_copy((uint8_t *) mac_hdr->bssId, bssid,
		     sizeof(struct qdf_mac_addr));

	/* now populate fixed params */
	bcn_fixed = (tDot11fBeacon1 *)(frame_buf + SIR_MAC_HDR_LEN_3A);
	/* populate timestamp */
	qdf_mem_copy(&bcn_fixed->TimeStamp.timestamp, &bss_descp->timeStamp,
			sizeof(bss_descp->timeStamp));
	/* populate beacon interval */
	bcn_fixed->BeaconInterval.interval = bss_descp->beaconInterval;
	/* populate capability */
	qdf_mem_copy(&bcn_fixed->Capabilities, &bss_descp->capabilityInfo,
			sizeof(bss_descp->capabilityInfo));

	/* copy IEs now */
	qdf_mem_copy(frame_buf + SIR_MAC_HDR_LEN_3A
		     + SIR_MAC_B_PR_SSID_OFFSET,
		     &bss_descp->ieFields, ie_len);
}

QDF_STATUS sme_get_rssi_snr_by_bssid(tHalHandle hal,
				tCsrRoamProfile *profile,
				const uint8_t *bssid,
				int8_t *rssi, int8_t *snr)
{
	tSirBssDescription *bss_descp;
	tCsrScanResultFilter *scan_filter;
	struct scan_result_list *bss_list;
	tScanResultHandle result_handle = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == scan_filter) {
		sme_err("memory allocation failed");
		status = QDF_STATUS_E_NOMEM;
		goto free_scan_flter;
	}

	status = csr_roam_prepare_filter_from_profile(mac_ctx,
						profile, scan_filter);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("prepare_filter failed");
		goto free_scan_flter;
	}

	/* update filter to get scan result with just target BSSID */
	if (NULL == scan_filter->BSSIDs.bssid) {
		scan_filter->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(struct qdf_mac_addr));
		if (scan_filter->BSSIDs.bssid == NULL) {
			sme_err("malloc failed");
			status = QDF_STATUS_E_NOMEM;
			goto free_scan_flter;
		}
	}

	scan_filter->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(scan_filter->BSSIDs.bssid[0].bytes,
		     bssid, sizeof(struct qdf_mac_addr));

	status = csr_scan_get_result(mac_ctx, scan_filter, &result_handle);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("parse_scan_result failed");
		goto free_scan_flter;
	}

	bss_list = (struct scan_result_list *)result_handle;
	bss_descp = csr_get_fst_bssdescr_ptr(bss_list);
	if (!bss_descp) {
		sme_err("unable to fetch bss descriptor");
		status = QDF_STATUS_E_FAULT;
		goto free_scan_flter;
	}

	sme_debug("snr: %d, rssi: %d, raw_rssi: %d",
		bss_descp->sinr, bss_descp->rssi, bss_descp->rssi_raw);

	if (rssi)
		*rssi = bss_descp->rssi;
	if (snr)
		*snr = bss_descp->sinr;

free_scan_flter:
	/* free scan filter and exit */
	if (scan_filter) {
		csr_free_scan_filter(mac_ctx, scan_filter);
		qdf_mem_free(scan_filter);
	}

	if (result_handle)
		csr_scan_result_purge(mac_ctx, result_handle);

	return status;
}

QDF_STATUS sme_get_beacon_frm(tHalHandle hal, tCsrRoamProfile *profile,
				const tSirMacAddr bssid,
				uint8_t **frame_buf, uint32_t *frame_len,
				int *channel)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tScanResultHandle result_handle = NULL;
	tCsrScanResultFilter *scan_filter;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	tSirBssDescription *bss_descp;
	struct scan_result_list *bss_list;
	uint32_t ie_len;

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == scan_filter) {
		sme_err("memory allocation failed");
		status = QDF_STATUS_E_NOMEM;
		goto free_scan_flter;
	}
	status = csr_roam_prepare_filter_from_profile(mac_ctx,
						profile, scan_filter);
	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("prepare_filter failed");
		status = QDF_STATUS_E_FAULT;
		goto free_scan_flter;
	}

	/* update filter to get scan result with just target BSSID */
	if (NULL == scan_filter->BSSIDs.bssid) {
		scan_filter->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(struct qdf_mac_addr));
		if (scan_filter->BSSIDs.bssid == NULL) {
			sme_err("malloc failed");
			status = QDF_STATUS_E_NOMEM;
			goto free_scan_flter;
		}
	}
	scan_filter->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(scan_filter->BSSIDs.bssid[0].bytes,
		     bssid, sizeof(struct qdf_mac_addr));

	status = csr_scan_get_result(mac_ctx, scan_filter, &result_handle);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("parse_scan_result failed");
		status = QDF_STATUS_E_FAULT;
		goto free_scan_flter;
	}

	bss_list = (struct scan_result_list *)result_handle;
	bss_descp = csr_get_fst_bssdescr_ptr(bss_list);
	if (!bss_descp) {
		sme_err("unable to fetch bss descriptor");
		status = QDF_STATUS_E_FAULT;
		goto free_scan_flter;
	}

	/**
	 * Length of BSS descriptor is without length of
	 * length itself and length of pointer that holds ieFields.
	 *
	 * tSirBssDescription
	 * +--------+---------------------------------+---------------+
	 * | length | other fields                    | pointer to IEs|
	 * +--------+---------------------------------+---------------+
	 *                                            ^
	 *                                            ieFields
	 */
	ie_len = bss_descp->length + sizeof(bss_descp->length)
		- (uint16_t)(offsetof(tSirBssDescription, ieFields[0]));
	sme_debug("found bss_descriptor ie_len: %d channel %d",
					ie_len, bss_descp->channelId);

	/* include mac header and fixed params along with IEs in frame */
	*frame_len = SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET + ie_len;
	*frame_buf = qdf_mem_malloc(*frame_len);
	if (NULL == *frame_buf) {
		sme_err("memory allocation failed");
		status = QDF_STATUS_E_NOMEM;
		goto free_scan_flter;
	}

	sme_prepare_beacon_from_bss_descp(*frame_buf, bss_descp, bssid, ie_len);

	if (!*channel)
		*channel = bss_descp->channelId;
free_scan_flter:
	/* free scan filter and exit */
	if (scan_filter) {
		csr_free_scan_filter(mac_ctx, scan_filter);
		qdf_mem_free(scan_filter);
	}
	if (result_handle)
		csr_scan_result_purge(mac_ctx, result_handle);

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS sme_fast_reassoc(tHalHandle hal, tCsrRoamProfile *profile,
			    const tSirMacAddr bssid, int channel,
			    uint8_t vdev_id, const tSirMacAddr connected_bssid)
{
	QDF_STATUS status;
	struct wma_roam_invoke_cmd *fastreassoc;
	struct scheduler_msg msg = {0};
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	fastreassoc = qdf_mem_malloc(sizeof(*fastreassoc));
	if (NULL == fastreassoc) {
		sme_err("qdf_mem_malloc failed for fastreassoc");
		return QDF_STATUS_E_NOMEM;
	}
	/* if both are same then set the flag */
	if (!qdf_mem_cmp(connected_bssid, bssid, ETH_ALEN)) {
		fastreassoc->is_same_bssid = true;
		sme_debug("bssid same, bssid[%pM]", bssid);
	}
	fastreassoc->vdev_id = vdev_id;
	fastreassoc->bssid[0] = bssid[0];
	fastreassoc->bssid[1] = bssid[1];
	fastreassoc->bssid[2] = bssid[2];
	fastreassoc->bssid[3] = bssid[3];
	fastreassoc->bssid[4] = bssid[4];
	fastreassoc->bssid[5] = bssid[5];

	status = sme_get_beacon_frm(hal, profile, bssid,
				    &fastreassoc->frame_buf,
				    &fastreassoc->frame_len,
				    &channel);

	if (!channel) {
		sme_err("channel retrieval from BSS desc fails!");
		qdf_mem_free(fastreassoc);
		return QDF_STATUS_E_FAULT;
	}

	fastreassoc->channel = channel;
	if (QDF_STATUS_SUCCESS != status) {
		sme_warn("sme_get_beacon_frm failed");
		fastreassoc->frame_buf = NULL;
		fastreassoc->frame_len = 0;
	}

	if (csr_is_auth_type_ese(mac_ctx->roam.roamSession[vdev_id].
				connectedProfile.AuthType)) {
		sme_debug("Beacon is not required for ESE");
		if (fastreassoc->frame_len) {
			qdf_mem_free(fastreassoc->frame_buf);
			fastreassoc->frame_buf = NULL;
			fastreassoc->frame_len = 0;
		}
	}

	msg.type = SIR_HAL_ROAM_INVOKE;
	msg.reserved = 0;
	msg.bodyptr = fastreassoc;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("Not able to post ROAM_INVOKE_CMD message to WMA");
		qdf_mem_free(fastreassoc);
	}

	return status;
}
#endif

QDF_STATUS sme_set_del_pmkid_cache(tHalHandle hal, uint8_t session_id,
				   tPmkidCacheInfo *pmk_cache_info,
				   bool is_add)
{
	struct wmi_unified_pmk_cache *pmk_cache;
	struct scheduler_msg msg;

	pmk_cache = qdf_mem_malloc(sizeof(*pmk_cache));
	if (!pmk_cache) {
		sme_err("Memory allocation failure");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_set(pmk_cache, sizeof(*pmk_cache), 0);

	pmk_cache->session_id = session_id;

	if (!pmk_cache_info)
		goto send_flush_cmd;

	if (!pmk_cache_info->ssid_len) {
		pmk_cache->cat_flag = WMI_PMK_CACHE_CAT_FLAG_BSSID;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(pmk_cache_info->BSSID.bytes,
				&pmk_cache->bssid);
	} else {
		pmk_cache->cat_flag = WMI_PMK_CACHE_CAT_FLAG_SSID_CACHE_ID;
		pmk_cache->ssid.length = pmk_cache_info->ssid_len;
		qdf_mem_copy(pmk_cache->ssid.mac_ssid,
			     pmk_cache_info->ssid,
			     pmk_cache->ssid.length);
	}
	pmk_cache->cache_id = (uint32_t) (pmk_cache_info->cache_id[0] << 8 |
					pmk_cache_info->cache_id[1]);

	if (is_add)
		pmk_cache->action_flag = WMI_PMK_CACHE_ACTION_FLAG_ADD_ENTRY;
	else
		pmk_cache->action_flag = WMI_PMK_CACHE_ACTION_FLAG_DEL_ENTRY;

	pmk_cache->pmkid_len = CSR_RSN_PMKID_SIZE;
	qdf_mem_copy(pmk_cache->pmkid, pmk_cache_info->PMKID,
		     CSR_RSN_PMKID_SIZE);

	pmk_cache->pmk_len = pmk_cache_info->pmk_len;
	qdf_mem_copy(pmk_cache->pmk, pmk_cache_info->pmk,
		     pmk_cache->pmk_len);

send_flush_cmd:
	msg.type = SIR_HAL_SET_DEL_PMKID_CACHE;
	msg.reserved = 0;
	msg.bodyptr = pmk_cache;
	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Not able to post message to WDA");
		qdf_mem_free(pmk_cache);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ARP DEBUG STATS */

/**
 * sme_set_nud_debug_stats() - sme api to set nud debug stats
 * @hal: handle to hal
 * @set_stats_param: pointer to set stats param
 *
 * Return: Return QDF_STATUS.
 */
QDF_STATUS sme_set_nud_debug_stats(tHalHandle hal,
				   struct set_arp_stats_params
				   *set_stats_param)
{
	struct set_arp_stats_params *arp_set_param;
	struct scheduler_msg msg;

	arp_set_param = qdf_mem_malloc(sizeof(*arp_set_param));
	if (arp_set_param == NULL) {
		sme_err("Memory allocation failure");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(arp_set_param, set_stats_param, sizeof(*arp_set_param));

	msg.type = WMA_SET_ARP_STATS_REQ;
	msg.reserved = 0;
	msg.bodyptr = arp_set_param;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Not able to post message to WDA");
		qdf_mem_free(arp_set_param);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_get_nud_debug_stats() - sme api to get nud debug stats
 * @hal: handle to hal
 * @get_stats_param: pointer to set stats param
 *
 * Return: Return QDF_STATUS.
 */
QDF_STATUS sme_get_nud_debug_stats(tHalHandle hal,
				   struct get_arp_stats_params
				   *get_stats_param)
{
	struct get_arp_stats_params *arp_get_param;
	struct scheduler_msg msg;

	arp_get_param = qdf_mem_malloc(sizeof(*arp_get_param));
	if (arp_get_param == NULL) {
		sme_err("Memory allocation failure");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(arp_get_param, get_stats_param, sizeof(*arp_get_param));

	msg.type = WMA_GET_ARP_STATS_REQ;
	msg.reserved = 0;
	msg.bodyptr = arp_get_param;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_msg(QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Not able to post message to WDA");
		qdf_mem_free(arp_get_param);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_set_peer_param(uint8_t *peer_addr, uint32_t param_id,
			      uint32_t param_value, uint32_t vdev_id)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sme_err("wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_set_peer_param(wma_handle, peer_addr, param_id,
				  param_value, vdev_id);
}

QDF_STATUS sme_register_set_connection_info_cb(tHalHandle hHal,
				bool (*set_connection_info_cb)(bool),
				bool (*get_connection_info_cb)(uint8_t *session_id,
				enum scan_reject_states *reason))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMac->sme.set_connection_info_cb = set_connection_info_cb;
		pMac->sme.get_connection_info_cb = get_connection_info_cb;
		sme_release_global_lock(&pMac->sme);
	}
	return status;
}

QDF_STATUS sme_rso_cmd_status_cb(tHalHandle hal,
		void (*cb)(void *, struct rso_cmd_status *))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	mac->sme.rso_cmd_status_cb = cb;
	sme_debug("Registered RSO command status callback");
	return status;
}

QDF_STATUS sme_set_dbs_scan_selection_config(tHalHandle hal,
		struct wmi_dbs_scan_sel_params *params)
{
	struct scheduler_msg message = {0};
	QDF_STATUS status;
	struct wmi_dbs_scan_sel_params *dbs_scan_params;
	uint32_t i;

	if (0 == params->num_clients) {
		sme_err("Num of clients is 0");
		return QDF_STATUS_E_FAILURE;
	}

	dbs_scan_params = qdf_mem_malloc(sizeof(*dbs_scan_params));
	if (!dbs_scan_params) {
		sme_err("fail to alloc dbs_scan_params");
		return QDF_STATUS_E_NOMEM;
	}

	dbs_scan_params->num_clients = params->num_clients;
	dbs_scan_params->pdev_id = params->pdev_id;
	for (i = 0; i < params->num_clients; i++) {
		dbs_scan_params->module_id[i] = params->module_id[i];
		dbs_scan_params->num_dbs_scans[i] = params->num_dbs_scans[i];
		dbs_scan_params->num_non_dbs_scans[i] =
			params->num_non_dbs_scans[i];
	}
	message.type = WMA_SET_DBS_SCAN_SEL_CONF_PARAMS;
	message.bodyptr = dbs_scan_params;
	status = scheduler_post_msg(QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Not able to post msg to WMA!");
		qdf_mem_free(dbs_scan_params);
	}

	return status;
}

QDF_STATUS sme_get_rcpi(tHalHandle hal, struct sme_rcpi_req *rcpi)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hal);
	struct scheduler_msg msg = {0};
	struct sme_rcpi_req *rcpi_req;

	rcpi_req = qdf_mem_malloc(sizeof(*rcpi_req));
	if (rcpi_req == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for rcpi req",
			  __func__);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(rcpi_req, rcpi, sizeof(*rcpi_req));

	status = sme_acquire_global_lock(&pMac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		msg.bodyptr = rcpi_req;
		msg.type = WMA_GET_RCPI_REQ;
		status = scheduler_post_msg(QDF_MODULE_ID_WMA, &msg);
		sme_release_global_lock(&pMac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("post get rcpi req failed"));
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(rcpi_req);
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("sme_acquire_global_lock failed"));
		qdf_mem_free(rcpi_req);
	}

	return status;
}

void sme_store_pdev(tHalHandle hal, struct wlan_objmgr_pdev *pdev)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	void *wma_handle;
	QDF_STATUS status;

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_LEGACY_MAC_ID);
	if (QDF_STATUS_SUCCESS != status) {
		mac_ctx->pdev = NULL;
		return;
	}
	mac_ctx->pdev = pdev;
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("wma handle is NULL"));
		return;
	}
	wma_store_pdev(wma_handle, pdev);
}

QDF_STATUS sme_congestion_register_callback(tHalHandle hal,
	void (*congestion_cb)(void *, uint32_t congestion, uint32_t vdev_id))
{
	QDF_STATUS status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.congestion_cb = congestion_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("congestion callback set");
	} else {
		sme_err("Aquiring lock failed %d", status);
	}

	return status;
}

QDF_STATUS sme_register_tx_queue_cb(tHalHandle hal,
				    void (*tx_queue_cb)(void *,
				    uint32_t vdev_id,
				    enum netif_action_type action,
				    enum netif_reason_type reason))
{
	QDF_STATUS status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.tx_queue_cb = tx_queue_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("Tx queue callback set");
	} else {
		sme_err("Aquiring lock failed %d", status);
	}

	return status;
}

QDF_STATUS sme_deregister_tx_queue_cb(tHalHandle hal)
{
	return sme_register_tx_queue_cb(hal, NULL);
}

QDF_STATUS sme_set_smps_cfg(uint32_t vdev_id, uint32_t param_id,
						uint32_t param_val)
{
	return wma_configure_smps_params(vdev_id, param_id, param_val);
}

QDF_STATUS sme_ipa_uc_stat_request(tHalHandle hal, uint32_t vdev_id,
			uint32_t param_id, uint32_t param_val, uint32_t req_cat)
{
	wma_cli_set_cmd_t *iwcmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	iwcmd = qdf_mem_malloc(sizeof(*iwcmd));
	if (!iwcmd) {
		sme_err("Failed alloc memory for iwcmd");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(iwcmd, sizeof(*iwcmd));
	iwcmd->param_sec_value = 0;
	iwcmd->param_vdev_id = vdev_id;
	iwcmd->param_id = param_id;
	iwcmd->param_vp_dev = req_cat;
	iwcmd->param_value =  param_val;
	wma_ipa_uc_stat_request(iwcmd);
	qdf_mem_free(iwcmd);

	return status;
}

QDF_STATUS sme_set_reorder_timeout(tHalHandle hal,
	struct sir_set_rx_reorder_timeout_val *req)
{
	QDF_STATUS status;

	status = wma_set_rx_reorder_timeout_val(hal, req);

	return status;
}

QDF_STATUS sme_set_rx_set_blocksize(tHalHandle hal,
	struct sir_peer_set_rx_blocksize *req)
{
	QDF_STATUS status;

	status = wma_set_rx_blocksize(hal, req);

	return status;
}

int sme_cli_set_command(int vdev_id, int param_id, int sval, int vpdev)
{
	return wma_cli_set_command(vdev_id, param_id, sval, vpdev);
}

QDF_STATUS sme_set_bt_activity_info_cb(tHalHandle hal,
			void (*cb)(void *, uint32_t bt_activity))
{
	QDF_STATUS status;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.bt_activity_info_cb = cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("bt activity info callback set");
	} else {
		sme_debug("sme_acquire_global_lock failed %d", status);
	}

	return status;
}

QDF_STATUS sme_get_chain_rssi(tHalHandle hal,
			      struct get_chain_rssi_req_params *input,
			      get_chain_rssi_callback callback,
			      void *context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	SME_ENTER();

	if (NULL == input) {
		sme_err("Invalid req params");
		return QDF_STATUS_E_INVAL;
	}

	mac_ctx->sme.get_chain_rssi_cb = callback;
	mac_ctx->sme.get_chain_rssi_context = context;
	wma_get_chain_rssi(hal, input);

	SME_EXIT();
	return status;
}

QDF_STATUS sme_process_msg_callback(tHalHandle hal,
				struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (msg == NULL) {
		sme_err("Empty message for SME Msg callback");
		return status;
	}
	status = sme_process_msg(hal, msg);
	return status;
}

void sme_display_disconnect_stats(tHalHandle hal, uint8_t session_id)
{
	struct csr_roam_session *session;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("%s Invalid session id: %d", __func__, session_id);
		return;
	}

	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sme_err("%s Failed to get session for id: %d",
			__func__, session_id);
		return;
	}

	sme_debug("Total No. of Disconnections: %d",
		  session->disconnect_stats.disconnection_cnt);

	sme_debug("No. of Diconnects Triggered by Application: %d",
		  session->disconnect_stats.disconnection_by_app);

	sme_debug("No. of Disassoc Sent by Peer: %d",
		  session->disconnect_stats.disassoc_by_peer);

	sme_debug("No. of Deauth Sent by Peer: %d",
		  session->disconnect_stats.deauth_by_peer);

	sme_debug("No. of Disconnections due to Beacon Miss: %d",
		  session->disconnect_stats.bmiss);

	sme_debug("No. of Disconnections due to Peer Kickout: %d",
		  session->disconnect_stats.peer_kickout);
}

 /**
 * sme_set_vc_mode_config() - Set voltage corner config to FW
 * @bitmap:	Bitmap that referes to voltage corner config with
 * different phymode and bw configuration
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_vc_mode_config(uint32_t vc_bitmap)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma_handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (QDF_STATUS_SUCCESS !=
		wma_set_vc_mode_config(wma_handle, vc_bitmap)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to set Voltage Control config to FW",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_set_bmiss_bcnt() - set bmiss config parameters
 * @vdev_id: virtual device for the command
 * @first_cnt: bmiss first value
 * @final_cnt: bmiss final value
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_set_bmiss_bcnt(uint32_t vdev_id, uint32_t first_cnt,
		uint32_t final_cnt)
{
	return wma_config_bmiss_bcnt_params(vdev_id, first_cnt, final_cnt);
}

QDF_STATUS sme_send_limit_off_channel_params(tHalHandle hal, uint8_t vdev_id,
		bool is_tos_active, uint32_t max_off_chan_time,
		uint32_t rest_time, bool skip_dfs_chan)
{
	struct sir_limit_off_chan *cmd;
	struct scheduler_msg msg = {0};

	cmd = qdf_mem_malloc(sizeof(*cmd));
	if (!cmd) {
		sme_err("qdf_mem_malloc failed for limit off channel");
		return QDF_STATUS_E_NOMEM;
	}

	cmd->vdev_id = vdev_id;
	cmd->is_tos_active = is_tos_active;
	cmd->max_off_chan_time = max_off_chan_time;
	cmd->rest_time = rest_time;
	cmd->skip_dfs_chans = skip_dfs_chan;

	msg.type = WMA_SET_LIMIT_OFF_CHAN;
	msg.reserved = 0;
	msg.bodyptr = cmd;

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_msg(QDF_MODULE_ID_WMA,
					&msg))) {
		sme_err("Not able to post WMA_SET_LIMIT_OFF_CHAN to WMA");
		qdf_mem_free(cmd);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_get_status_for_candidate() - Get bss transition status for candidate
 * @hal: Handle for HAL
 * @conn_bss_desc: connected bss descriptor
 * @bss_desc: candidate bss descriptor
 * @info: candiadate bss information
 * @trans_reason: transition reason code
 * @is_bt_in_progress: bt activity indicator
 *
 * Return : true if candidate is rejected and reject reason is filled
 * @info->status. Otherwise returns false.
 */
static bool sme_get_status_for_candidate(tHalHandle *hal,
					tSirBssDescription *conn_bss_desc,
					tSirBssDescription *bss_desc,
					struct bss_candidate_info *info,
					uint8_t trans_reason,
					bool is_bt_in_progress)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	/*
	 * Low RSSI based rejection
	 * If candidate rssi is less than mbo_candidate_rssi_thres and connected
	 * bss rssi is greater than mbo_current_rssi_thres, then reject the
	 * candidate with MBO reason code 4.
	 */
	if ((bss_desc->rssi < mac_ctx->roam.configParam.mbo_thresholds.
	    mbo_candidate_rssi_thres) &&
	    (conn_bss_desc->rssi > mac_ctx->roam.configParam.mbo_thresholds.
	    mbo_current_rssi_thres)) {
		sme_err("Candidate BSS "MAC_ADDRESS_STR" has LOW RSSI(%d), hence reject",
			MAC_ADDR_ARRAY(bss_desc->bssId), bss_desc->rssi);
		info->status = QCA_STATUS_REJECT_LOW_RSSI;
		return true;
	}

	if (trans_reason == MBO_TRANSITION_REASON_LOAD_BALANCING ||
	    trans_reason == MBO_TRANSITION_REASON_TRANSITIONING_TO_PREMIUM_AP) {
		/*
		 * MCC rejection
		 * If moving to candidate's channel will result in MCC scenario
		 * and the rssi of connected bss is greater than
		 * mbo_current_rssi_mss_thres, then reject the candidate with
		 * MBO reason code 3.
		 */
		if ((conn_bss_desc->rssi >
		    mac_ctx->roam.configParam.mbo_thresholds.
		    mbo_current_rssi_mcc_thres) &&
		    csr_is_mcc_channel(hal, bss_desc->channelId)) {
			sme_err("Candidate BSS "MAC_ADDRESS_STR" causes MCC, hence reject",
				MAC_ADDR_ARRAY(bss_desc->bssId));
			info->status =
				QCA_STATUS_REJECT_INSUFFICIENT_QOS_CAPACITY;
			return true;
		}

		/*
		 * BT coex rejection
		 * If AP is trying to move the client from 5G to 2.4G and moving
		 * to 2.4G will result in BT coex and candidate channel rssi is
		 * less than mbo_candidate_rssi_btc_thres, then reject the
		 * candidate with MBO reason code 2.
		 */
		if (WLAN_REG_IS_5GHZ_CH(conn_bss_desc->channelId) &&
		    WLAN_REG_IS_24GHZ_CH(bss_desc->channelId) &&
		    is_bt_in_progress &&
		    (bss_desc->rssi <
		    mac_ctx->roam.configParam.mbo_thresholds.
		    mbo_candidate_rssi_btc_thres)) {
			sme_err("Candidate BSS "MAC_ADDRESS_STR" causes BT coex, hence reject",
				MAC_ADDR_ARRAY(bss_desc->bssId));
			info->status =
				QCA_STATUS_REJECT_EXCESSIVE_DELAY_EXPECTED;
			return true;
		}

		/*
		 * LTE coex rejection
		 * If moving to candidate's channel can cause LTE coex, then
		 * reject the candidate with MBO reason code 5.
		 */
		if (policy_mgr_is_safe_channel(mac_ctx->psoc,
		    conn_bss_desc->channelId) &&
		    !(policy_mgr_is_safe_channel(mac_ctx->psoc,
		    bss_desc->channelId))) {
			sme_err("High interference expected if transitioned to BSS "
				MAC_ADDRESS_STR" hence reject",
				MAC_ADDR_ARRAY(bss_desc->bssId));
			info->status =
				QCA_STATUS_REJECT_HIGH_INTERFERENCE;
			return true;
		}
	}

	return false;
}

uint32_t sme_unpack_rsn_ie(tHalHandle hal, uint8_t *buf,
				  uint8_t buf_len, tDot11fIERSN *rsn_ie,
				  bool append_ie)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	return dot11f_unpack_ie_rsn(mac_ctx, buf, buf_len, rsn_ie, append_ie);
}

/**
 * wlan_hdd_get_bss_transition_status() - get bss transition status all cadidates
 * @adapter : Pointer to adapter
 * @transition_reason : Transition reason
 * @info : bss candidate information
 * @n_candidates : number of candidates
 *
 * Return : 0 on success otherwise errno
 */
int sme_get_bss_transition_status(tHalHandle hal,
					uint8_t transition_reason,
					struct qdf_mac_addr *bssid,
					struct bss_candidate_info *info,
					uint16_t n_candidates,
					bool is_bt_in_progress)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirBssDescription *bss_desc, *conn_bss_desc;
	tCsrScanResultInfo *res, *conn_res;
	uint16_t i;

	if (!n_candidates || !info) {
		sme_err("No candidate info available");
		return QDF_STATUS_E_INVAL;
	}

	conn_res = qdf_mem_malloc(sizeof(tCsrScanResultInfo));
	if (!conn_res) {
		sme_err("Failed to allocate memory for conn_res");
		return QDF_STATUS_E_NOMEM;
	}

	res = qdf_mem_malloc(sizeof(tCsrScanResultInfo));
	if (!res) {
		sme_err("Failed to allocate memory for conn_res");
		status = QDF_STATUS_E_NOMEM;
		goto free;
	}

	/* Get the connected BSS descriptor */
	status = sme_scan_get_result_for_bssid(hal, bssid, conn_res);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to find connected BSS in scan list");
		goto free;
	}
	conn_bss_desc = &conn_res->BssDescriptor;

	for (i = 0; i < n_candidates; i++) {
		/* Get candidate BSS descriptors */
		status = sme_scan_get_result_for_bssid(hal, &info[i].bssid,
						       res);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("BSS "MAC_ADDRESS_STR" not present in scan list",
				MAC_ADDR_ARRAY(info[i].bssid.bytes));
			info[i].status = QCA_STATUS_REJECT_UNKNOWN;
			continue;
		}

		bss_desc = &res->BssDescriptor;
		if (!sme_get_status_for_candidate(hal, conn_bss_desc, bss_desc,
		    &info[i], transition_reason, is_bt_in_progress)) {
			/*
			 * If status is not over written, it means it is a
			 * candidate for accept.
			 */
			info[i].status = QCA_STATUS_ACCEPT;
		}
	}

	/* success */
	status = QDF_STATUS_SUCCESS;

free:
	/* free allocated memory */
	if (conn_res)
		qdf_mem_free(conn_res);
	if (res)
		qdf_mem_free(res);

	return status;
}

void sme_enable_roaming_on_connected_sta(tHalHandle hal)
{
	uint8_t session_id;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	QDF_STATUS status;

	session_id = csr_get_roam_enabled_sta_sessionid(mac_ctx);
	if (session_id != CSR_SESSION_ID_INVALID)
		return;

	session_id = csr_get_connected_infra(mac_ctx);
	if (session_id == CSR_SESSION_ID_INVALID) {
		sme_debug("No STA in conencted state");
		return;
	}

	sme_debug("Roaming not enabled on any STA, enable roaming on session %d",
		  session_id);
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(mac_ctx, session_id,
				      ROAM_SCAN_OFFLOAD_START,
				      REASON_CTX_INIT);
		sme_release_global_lock(&mac_ctx->sme);
	}
}

int16_t sme_get_oper_chan_freq(struct wlan_objmgr_vdev *vdev)
{
	uint8_t vdev_id, chan;
	struct csr_roam_session *session;
	tpAniSirGlobal mac_ctx;
	tHalHandle h_hal;
	int16_t freq = 0;

	if (vdev == NULL) {
		sme_err("Invalid vdev id is passed");
		return 0;
	}

	h_hal = cds_get_context(QDF_MODULE_ID_SME);
	if (!h_hal) {
		sme_err("h_hal is null");
		return 0;
	}
	mac_ctx = PMAC_STRUCT(h_hal);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!CSR_IS_SESSION_VALID(mac_ctx, vdev_id)) {
		sme_err("Invalid vdev id is passed");
		return 0;
	}

	session = CSR_GET_SESSION(mac_ctx, vdev_id);
	chan = csr_get_infra_operation_channel(mac_ctx, vdev_id);
	if (chan)
		freq = cds_chan_to_freq(chan);

	return freq;
}

enum phy_ch_width sme_get_oper_ch_width(struct wlan_objmgr_vdev *vdev)
{
	uint8_t vdev_id;
	struct csr_roam_session *session;
	tpAniSirGlobal mac_ctx;
	tHalHandle h_hal;
	enum phy_ch_width ch_width = CH_WIDTH_20MHZ;

	if (vdev == NULL) {
		sme_err("Invalid vdev id is passed");
		return CH_WIDTH_INVALID;
	}

	h_hal = cds_get_context(QDF_MODULE_ID_SME);
	if (!h_hal) {
		sme_err("h_hal is null");
		return CH_WIDTH_INVALID;
	}
	mac_ctx = PMAC_STRUCT(h_hal);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!CSR_IS_SESSION_VALID(mac_ctx, vdev_id)) {
		sme_err("Invalid vdev id is passed");
		return CH_WIDTH_INVALID;
	}

	session = CSR_GET_SESSION(mac_ctx, vdev_id);

	if (csr_is_conn_state_connected(mac_ctx, vdev_id))
		ch_width = session->connectedProfile.vht_channel_width;

	return ch_width;
}

int sme_get_sec20chan_freq_mhz(struct wlan_objmgr_vdev *vdev,
						uint16_t *sec20chan_freq)
{
	uint8_t vdev_id;

	vdev_id = wlan_vdev_get_id(vdev);
	/* Need to extend */
	return 0;
}

#ifdef WLAN_FEATURE_SAE
QDF_STATUS sme_handle_sae_msg(tHalHandle hal, uint8_t session_id,
		uint8_t sae_status)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = PMAC_STRUCT(hal);
	struct sir_sae_msg *sae_msg;
	struct scheduler_msg sch_msg = {0};

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		sae_msg = qdf_mem_malloc(sizeof(*sae_msg));
		if (!sae_msg) {
			qdf_status = QDF_STATUS_E_NOMEM;
			sme_err("SAE: memory allocation failed");
		} else {
			sae_msg->message_type = eWNI_SME_SEND_SAE_MSG;
			sae_msg->length = sizeof(*sae_msg);
			sae_msg->session_id = session_id;
			sae_msg->sae_status = sae_status;
			sme_debug("SAE: sae_status %d session_id %d",
				sae_msg->sae_status,
				sae_msg->session_id);

			sch_msg.type = eWNI_SME_SEND_SAE_MSG;
			sch_msg.bodyptr = sae_msg;

			qdf_status =
				scheduler_post_msg(QDF_MODULE_ID_PE, &sch_msg);
		}
		sme_release_global_lock(&mac->sme);
	}

	return qdf_status;
}
#endif

bool sme_is_sta_key_exchange_in_progress(tHalHandle hal, uint8_t session_id)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("Invalid session id: %d", session_id);
		return false;
	}

	return CSR_IS_WAIT_FOR_KEY(mac_ctx, session_id);
}
