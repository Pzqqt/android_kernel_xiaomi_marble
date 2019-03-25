/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
 * DOC: sme_api.c
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
#include "wma.h"
#include "wma_fips_api.h"
#include "wma_fw_state.h"
#include "qdf_trace.h"
#include "sme_trace.h"
#include "qdf_types.h"
#include "qdf_util.h"
#include "qdf_trace.h"
#include "cds_utils.h"
#include "sap_api.h"
#include "mac_trace.h"
#include "cds_regdomain.h"
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
#include <wlan_spectral_utils_api.h>
#include "wlan_mlme_public_struct.h"
#include "wlan_mlme_main.h"
#include "cfg_ucfg_api.h"
#include "wlan_fwol_ucfg_api.h"

static QDF_STATUS init_sme_cmd_list(struct mac_context *mac);

static void sme_disconnect_connected_sessions(struct mac_context *mac);

static QDF_STATUS sme_handle_generic_change_country_code(struct mac_context *mac,
						  void *pMsgBuf);

static QDF_STATUS sme_process_nss_update_resp(struct mac_context *mac, uint8_t *msg);

/* Channel Change Response Indication Handler */
static QDF_STATUS sme_process_channel_change_resp(struct mac_context *mac,
					   uint16_t msg_type, void *pMsgBuf);

static QDF_STATUS sme_stats_ext_event(struct mac_context *mac,
				      struct stats_ext_event *msg);

static QDF_STATUS sme_fw_state_resp(struct mac_context *mac);

/* Internal SME APIs */
QDF_STATUS sme_acquire_global_lock(struct sme_context *sme)
{
	if (!sme)
		return QDF_STATUS_E_INVAL;

	return qdf_mutex_acquire(&sme->sme_global_lock);
}

QDF_STATUS sme_release_global_lock(struct sme_context *sme)
{
	if (!sme)
		return QDF_STATUS_E_INVAL;

	return qdf_mutex_release(&sme->sme_global_lock);
}

struct mac_context *sme_get_mac_context(void)
{
	struct mac_context *mac_ctx;
	mac_handle_t mac_handle;

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
		FL("invalid mac_handle"));
		return NULL;
	}

	mac_ctx = MAC_CONTEXT(mac_handle);

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
static QDF_STATUS sme_process_set_hw_mode_resp(struct mac_context *mac, uint8_t *msg)
{
	tListElem *entry;
	tSmeCmd *command = NULL;
	bool found;
	policy_mgr_pdev_set_hw_mode_cback callback = NULL;
	struct sir_set_hw_mode_resp *param;
	enum policy_mgr_conn_update_reason reason;
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
	if (reason == POLICY_MGR_UPDATE_REASON_HIDDEN_STA) {
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
	if (reason == POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH_STA) {
		sme_debug(" Continue channel switch for STA");
		csr_sta_continue_csa(mac, session_id);
	}
	if (reason == POLICY_MGR_UPDATE_REASON_LFR2_ROAM)
		csr_continue_lfr2_connect(mac, session_id);

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
static QDF_STATUS sme_process_hw_mode_trans_ind(struct mac_context *mac,
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

void sme_purge_pdev_all_ser_cmd_list(mac_handle_t mac_handle)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	csr_purge_pdev_all_ser_cmd_list(mac_ctx);
	sme_release_global_lock(&mac_ctx->sme);
}

/**
 * free_sme_cmds() - This function frees memory allocated for SME commands
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function frees memory allocated for SME commands
 *
 * @Return: void
 */
static void free_sme_cmds(struct mac_context *mac_ctx)
{
	uint32_t idx;

	if (!mac_ctx->sme.sme_cmd_buf_addr)
		return;

	for (idx = 0; idx < mac_ctx->sme.sme_cmd_count; idx++)
		qdf_mem_free(mac_ctx->sme.sme_cmd_buf_addr[idx]);

	qdf_mem_free(mac_ctx->sme.sme_cmd_buf_addr);
	mac_ctx->sme.sme_cmd_buf_addr = NULL;
}

static QDF_STATUS init_sme_cmd_list(struct mac_context *mac)
{
	QDF_STATUS status;
	tSmeCmd *cmd;
	uint32_t cmd_idx;
	uint32_t sme_cmd_ptr_ary_sz;

	mac->sme.sme_cmd_count = SME_TOTAL_COMMAND;

	status = csr_ll_open(&mac->sme.sme_cmd_freelist);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto end;

	/* following pointer contains array of pointers for tSmeCmd* */
	sme_cmd_ptr_ary_sz = sizeof(void *) * mac->sme.sme_cmd_count;
	mac->sme.sme_cmd_buf_addr = qdf_mem_malloc(sme_cmd_ptr_ary_sz);
	if (!mac->sme.sme_cmd_buf_addr) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	status = QDF_STATUS_SUCCESS;
	for (cmd_idx = 0; cmd_idx < mac->sme.sme_cmd_count; cmd_idx++) {
		/*
		 * Since total size of all commands together can be huge chunk
		 * of memory, allocate SME cmd individually. These SME CMDs are
		 * moved between pending and active queues. And these freeing of
		 * these queues just manipulates the list but does not actually
		 * frees SME CMD pointers. Hence store each SME CMD address in
		 * the array, sme.sme_cmd_buf_addr. This will later facilitate
		 * freeing up of all SME CMDs with just a for loop.
		 */
		cmd = qdf_mem_malloc(sizeof(*cmd));
		if (!cmd) {
			status = QDF_STATUS_E_NOMEM;
			free_sme_cmds(mac);
			goto end;
		}
		mac->sme.sme_cmd_buf_addr[cmd_idx] = cmd;
		csr_ll_insert_tail(&mac->sme.sme_cmd_freelist,
				   &cmd->Link, LL_ACCESS_LOCK);
	}

end:
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("Failed to initialize sme command list: %d", status);

	return status;
}

void sme_release_command(struct mac_context *mac_ctx, tSmeCmd *sme_cmd)
{
	sme_cmd->command = eSmeNoCommand;
	csr_ll_insert_tail(&mac_ctx->sme.sme_cmd_freelist, &sme_cmd->Link,
			   LL_ACCESS_LOCK);
}

static QDF_STATUS free_sme_cmd_list(struct mac_context *mac)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	csr_ll_close(&mac->sme.sme_cmd_freelist);

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS)
		goto done;

	free_sme_cmds(mac);

	status = sme_release_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS)
		sme_err("Failed to release the lock status: %d", status);
done:
	return status;
}

static void dump_csr_command_info(struct mac_context *mac, tSmeCmd *pCmd)
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

	case e_sme_command_del_sta_session:
		sme_debug("Issue del STA command for session:%d",
			   pCmd->sessionId);
		break;

	default:
		sme_debug("default: Unhandled command %d",
			pCmd->command);
		break;
	}
}

tSmeCmd *sme_get_command_buffer(struct mac_context *mac)
{
	tSmeCmd *pRetCmd = NULL, *pTempCmd = NULL;
	tListElem *pEntry;
	static int sme_command_queue_full;

	pEntry = csr_ll_remove_head(&mac->sme.sme_cmd_freelist, LL_ACCESS_LOCK);

	/* If we can get another MS Msg buffer, then we are ok.  Just
	 * link the entry onto the linked list.  (We are using the
	 * linked list to keep track of the message buffers).
	 */
	if (pEntry) {
		pRetCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		/* reset when free list is available */
		sme_command_queue_full = 0;
	} else {
		int idx = 1;

		/* Cannot change pRetCmd here since it needs to return later. */
		pEntry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
		if (pEntry)
			pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

		sme_err("Out of command buffer.... command (0x%X) stuck",
			(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
		if (pTempCmd) {
			if (eSmeCsrCommandMask & pTempCmd->command)
				/* CSR command is stuck. See what the reason
				 * code is for that command
				 */
				dump_csr_command_info(mac, pTempCmd);
		} /* if(pTempCmd) */

		/* dump what is in the pending queue */
		pEntry =
			csr_nonscan_pending_ll_peek_head(mac,
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
				dump_csr_command_info(mac, pTempCmd);
			pEntry = csr_nonscan_pending_ll_next(mac, pEntry,
					    LL_ACCESS_NOLOCK);
		}

		if (mac->mlme_cfg->gen.fatal_event_trigger)
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_SME_OUT_OF_CMD_BUF,
				false,
				mac->mlme_cfg->gen.self_recovery);
		else
			cds_trigger_recovery(QDF_GET_MSG_BUFF_FAILURE);
	}

	/* memset to zero */
	if (pRetCmd) {
		qdf_mem_zero((uint8_t *)&pRetCmd->command,
			    sizeof(pRetCmd->command));
		qdf_mem_zero((uint8_t *)&pRetCmd->sessionId,
			    sizeof(pRetCmd->sessionId));
		qdf_mem_zero((uint8_t *)&pRetCmd->u, sizeof(pRetCmd->u));
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
	mac_handle_t mac_handle;
	struct mac_context *mac_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool do_continue;

	if (!cmd) {
		sme_err("No serialization command found");
		return QDF_STATUS_E_FAILURE;
	}

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (mac_handle) {
		mac_ctx = MAC_CONTEXT(mac_handle);
	} else {
		sme_err("No mac_handle found");
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
	case e_sme_command_del_sta_session:
		csr_process_del_sta_session_command(mac_ctx, sme_cmd);
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

QDF_STATUS sme_ser_cmd_callback(struct wlan_serialization_command *cmd,
				enum wlan_serialization_cb_reason reason)
{
	mac_handle_t mac_handle;
	struct mac_context *mac_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *sme_cmd;

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (mac_handle) {
		mac_ctx = MAC_CONTEXT(mac_handle);
	} else {
		sme_err("mac_handle is null");
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
		if (cmd->vdev)
			wlan_objmgr_vdev_release_ref(cmd->vdev,
						     WLAN_LEGACY_SME_ID);
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

#ifdef WLAN_FEATURE_MEMDUMP_ENABLE
/**
 * sme_get_sessionid_from_activelist() - gets session id
 * @mac: mac context
 *
 * This function is used to get session id from sme command
 * active list
 *
 * Return: returns session id
 */
static uint32_t sme_get_sessionid_from_activelist(struct mac_context *mac)
{
	tListElem *entry;
	tSmeCmd *command;
	uint32_t session_id = WLAN_UMAC_VDEV_ID_MAX;

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
	mac_handle_t mac_handle;
	struct mac_context *mac;
	uint16_t len = 0;
	char *buf = *buf_ptr;
	eCsrConnectState connect_state;

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (!mac_handle) {
		QDF_ASSERT(0);
		return;
	}

	mac = MAC_CONTEXT(mac_handle);

	active_session_id = sme_get_sessionid_from_activelist(mac);
	if (active_session_id != WLAN_UMAC_VDEV_ID_MAX) {
		len += qdf_scnprintf(buf + len, *size - len,
			"\n active command sessionid %d", active_session_id);
	}

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
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
#else /* WLAN_FEATURE_MEMDUMP_ENABLE */
static void sme_register_debug_callback(void)
{
}
#endif /* WLAN_FEATURE_MEMDUMP_ENABLE */

/* Global APIs */

/**
 * sme_open() - Initialze all SME modules and put them at idle state
 * @mac_handle:       The handle returned by mac_open
 *
 * The function initializes each module inside SME, PMC, CSR, etc. Upon
 * successfully return, all modules are at idle state ready to start.
 * smeOpen must be called before any other SME APIs can be involved.
 * smeOpen must be called after mac_open.
 *
 * Return: QDF_STATUS_SUCCESS - SME is successfully initialized.
 *         Other status means SME is failed to be initialized
 */
QDF_STATUS sme_open(mac_handle_t mac_handle)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->sme.state = SME_STATE_STOP;
	mac->sme.curr_device_mode = QDF_STA_MODE;
	if (!QDF_IS_STATUS_SUCCESS(qdf_mutex_create(
					&mac->sme.sme_global_lock))) {
		sme_err("Init lock failed");
		return  QDF_STATUS_E_FAILURE;
	}
	status = csr_open(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_open failed, status: %d", status);
		return status;
	}

	status = sme_ps_open(mac_handle);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_ps_open failed with status: %d", status);
		return status;
	}

#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_open(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Qos open, status: %d", status);
		return status;
	}
#endif
	status = init_sme_cmd_list(mac);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	status = rrm_open(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("rrm_open failed, status: %d", status);
		return status;
	}
	sme_trace_init(mac);
	sme_register_debug_callback();

	return status;
}

/*
 * sme_init_chan_list, triggers channel setup based on country code.
 */
QDF_STATUS sme_init_chan_list(mac_handle_t mac_handle, uint8_t *alpha2,
			      enum country_src cc_src)
{
	struct mac_context *pmac = MAC_CONTEXT(mac_handle);

	if ((cc_src == SOURCE_USERSPACE) &&
	    (pmac->mlme_cfg->sap_cfg.country_code_priority)) {
		pmac->mlme_cfg->gen.enabled_11d = false;
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
 * mac_handle - The handle returned by mac_open.
 * pSmeConfigParams - a pointer to a caller allocated object of
 *  struct sme_config_params.
 *
 * Return QDF_STATUS_SUCCESS - SME update the config parameters successfully.
 *
 *  Other status means SME is failed to update the config parameters.
 */

QDF_STATUS sme_set11dinfo(mac_handle_t mac_handle,
			  struct sme_config_params *pSmeConfigParams)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SET_11DINFO, NO_SESSION, 0));
	if (!pSmeConfigParams) {
		sme_err("SME config params empty");
		return status;
	}

	status = csr_set_channels(mac_ctx, &pSmeConfigParams->csrConfig);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("csr_set_channels failed with status: %d", status);

	return status;
}

/**
 * sme_update_fine_time_measurement_capab() - Update the FTM capabitlies from
 * incoming val
 * @mac_handle: Opaque handle to the global MAC context
 * @val: New FTM capability value
 *
 * Return: None
 */
void sme_update_fine_time_measurement_capab(mac_handle_t mac_handle,
					    uint8_t session_id,
					    uint32_t val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;

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
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(mac_ctx, session_id,
			ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			REASON_CONNECT_IES_CHANGED);
		sme_release_global_lock(&mac_ctx->sme);
	}
}

/**
 * sme_update_neighbor_report_config() - Update CSR config for 11k params
 * @mac_handle: Pointer to MAC context
 * @csr_config: Pointer to CSR config data structure
 *
 * Return: None
 */
static void sme_update_neighbor_report_config(struct mac_context *mac,
					      tCsrConfigParam *csr_config)
{
	struct wlan_fwol_neighbor_report_cfg fwol_neighbor_report_cfg = {0};
	QDF_STATUS status;

	status = ucfg_fwol_get_neighbor_report_cfg(mac->psoc,
						   &fwol_neighbor_report_cfg);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("Using defaults for 11K offload params: Error: %d",
			status);

	csr_config->offload_11k_enable_bitmask =
				fwol_neighbor_report_cfg.enable_bitmask;
	csr_config->neighbor_report_offload.params_bitmask =
				fwol_neighbor_report_cfg.params_bitmask;
	csr_config->neighbor_report_offload.time_offset =
				fwol_neighbor_report_cfg.time_offset;
	csr_config->neighbor_report_offload.low_rssi_offset =
				fwol_neighbor_report_cfg.low_rssi_offset;
	csr_config->neighbor_report_offload.bmiss_count_trigger =
				fwol_neighbor_report_cfg.bmiss_count_trigger;
	csr_config->neighbor_report_offload.per_threshold_offset =
				fwol_neighbor_report_cfg.per_threshold_offset;
	csr_config->neighbor_report_offload.neighbor_report_cache_timeout =
				fwol_neighbor_report_cfg.cache_timeout;
	csr_config->neighbor_report_offload.max_neighbor_report_req_cap =
				fwol_neighbor_report_cfg.max_req_cap;
}

/*
 * sme_update_config() - Change configurations for all SME moduels
 * The function updates some configuration for modules in SME, CSR, etc
 *  during SMEs close open sequence.
 * Modules inside SME apply the new configuration at the next transaction.
 * This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * pSmeConfigParams - a pointer to a caller allocated object of
 *  struct sme_config_params.
 * Return QDF_STATUS_SUCCESS - SME update the config parameters successfully.
 *  Other status means SME is failed to update the config parameters.
 */
QDF_STATUS sme_update_config(mac_handle_t mac_handle,
			     struct sme_config_params *pSmeConfigParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_UPDATE_CONFIG, NO_SESSION,
			 0));
	if (!pSmeConfigParams) {
		sme_err("SME config params empty");
		return status;
	}
	sme_update_neighbor_report_config(mac, &pSmeConfigParams->csrConfig);
	status = csr_change_default_config_param(mac, &pSmeConfigParams->
						csrConfig);

	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("csr_change_default_config_param failed status: %d",
			status);

	/* For SOC, CFG is set before start We don't want to apply global CFG
	 * in connect state because that may cause some side affect
	 */
	if (csr_is_all_session_disconnected(mac))
		csr_set_global_cfgs(mac);

	return status;
}

/**
 * sme_update_scan_roam_params() - Update the scan roaming params
 * @mac_ctx: mac ctx
 *
 * Return: void.
 */
static void sme_update_scan_roam_params(struct mac_context *mac_ctx)
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
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id:               SME Session ID
 * @roam_params_src:          The source buffer to copy
 * @update_param:             Type of parameter to be updated
 *
 * Return: Return the status of the updation.
 */
QDF_STATUS sme_update_roam_params(mac_handle_t mac_handle,
				  uint8_t session_id,
				  struct roam_ext_params *roam_params_src,
				  int update_param)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct roam_ext_params *roam_params_dst;
	QDF_STATUS status;
	uint8_t i;

	roam_params_dst = &mac_ctx->roam.configParam.roam_params;
	switch (update_param) {
	case REASON_ROAM_EXT_SCAN_PARAMS_CHANGED:
		mac_ctx->mlme_cfg->lfr.rssi_boost_threshold_5g =
			roam_params_src->raise_rssi_thresh_5g;
		mac_ctx->mlme_cfg->lfr.rssi_penalize_threshold_5g =
			roam_params_src->drop_rssi_thresh_5g;
		mac_ctx->mlme_cfg->lfr.rssi_boost_factor_5g =
			roam_params_src->raise_factor_5g;
		mac_ctx->mlme_cfg->lfr.rssi_penalize_factor_5g =
			roam_params_src->drop_factor_5g;
		mac_ctx->mlme_cfg->lfr.max_rssi_boost_5g =
			roam_params_src->max_raise_rssi_5g;
		mac_ctx->mlme_cfg->lfr.max_rssi_penalize_5g =
			roam_params_src->max_drop_rssi_5g;
		roam_params_dst->alert_rssi_threshold =
			roam_params_src->alert_rssi_threshold;
		mac_ctx->mlme_cfg->lfr.enable_5g_band_pref = true;
		break;
	case REASON_ROAM_SET_SSID_ALLOWED:
		qdf_mem_zero(&roam_params_dst->ssid_allowed_list,
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
		qdf_mem_zero(&roam_params_dst->bssid_favored,
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
		qdf_mem_zero(&roam_params_dst->bssid_avoid_list,
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

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(mac_ctx, session_id,
				      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				      update_param);
		sme_release_global_lock(&mac_ctx->sme);
	}

	sme_update_scan_roam_params(mac_ctx);

	return 0;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT

/**
 * sme_process_ready_to_ext_wow() - inform ready to ExtWoW indication.
 * @mac: Global MAC context
 * @indication: ready to Ext WoW indication from lower layer
 *
 * On getting ready to Ext WoW indication, this function calls callback
 * registered (HDD callback) with SME to inform ready to ExtWoW indication.
 *
 * Return: None
 */
static void sme_process_ready_to_ext_wow(struct mac_context *mac,
					 tpSirReadyToExtWoWInd indication)
{
	if (!mac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: mac is null", __func__);
		return;
	}

	if (mac->readyToExtWoWCallback) {
		mac->readyToExtWoWCallback(mac->readyToExtWoWContext,
					   indication->status);
		mac->readyToExtWoWCallback = NULL;
		mac->readyToExtWoWContext = NULL;
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
 * @mac_handle - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - eWNI_SME_SYS_READY_IND is sent to PE
 *				successfully.
 * Other status means SME failed to send the message to PE.
 */
QDF_STATUS sme_hdd_ready_ind(mac_handle_t mac_handle)
{
	struct sme_ready_req *msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_HDDREADYIND, NO_SESSION, 0));
	do {

		msg = qdf_mem_malloc(sizeof(*msg));
		if (!msg)
			return QDF_STATUS_E_NOMEM;

		msg->messageType = eWNI_SME_SYS_READY_IND;
		msg->length = sizeof(*msg);
		msg->csr_roam_synch_cb = csr_roam_synch_callback;
		msg->sme_msg_cb = sme_process_msg_callback;
		msg->stop_roaming_cb = sme_stop_roaming;

		status = u_mac_post_ctrl_msg(mac_handle, (tSirMbMsg *)msg);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("u_mac_post_ctrl_msg failed to send eWNI_SME_SYS_READY_IND");
			break;
		}

		status = csr_ready(mac);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("csr_ready failed with status: %d", status);
			break;
		}

		mac->sme.state = SME_STATE_READY;
	} while (0);

	return status;
}

QDF_STATUS sme_get_valid_channels(uint8_t *chan_list, uint32_t *list_len)
{
	struct mac_context *mac_ctx = sme_get_mac_context();

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	*list_len = (uint32_t)mac_ctx->mlme_cfg->reg.valid_channel_list_num;
	qdf_mem_copy(chan_list, mac_ctx->mlme_cfg->reg.valid_channel_list,
		     *list_len);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_CONV_SPECTRAL_ENABLE
static QDF_STATUS sme_register_spectral_cb(struct mac_context *mac_ctx)
{
	struct spectral_legacy_cbacks spectral_cb;
	QDF_STATUS status;

	spectral_cb.vdev_get_chan_freq = sme_get_oper_chan_freq;
	spectral_cb.vdev_get_ch_width = sme_get_oper_ch_width;
	spectral_cb.vdev_get_sec20chan_freq_mhz = sme_get_sec20chan_freq_mhz;
	status = spectral_register_legacy_cb(mac_ctx->psoc, &spectral_cb);

	return status;
}
#else
static QDF_STATUS sme_register_spectral_cb(struct mac_context *mac_ctx)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/*
 * sme_start() - Put all SME modules at ready state.
 *  The function starts each module in SME, PMC, CSR, etc. . Upon
 *  successfully return, all modules are ready to run.
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME is ready.
 * Other status means SME is failed to start
 */
QDF_STATUS sme_start(mac_handle_t mac_handle)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct policy_mgr_sme_cbacks sme_cbacks;

	do {
		status = csr_start(mac);
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
		status = policy_mgr_register_sme_cb(mac->psoc, &sme_cbacks);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Failed to register sme cb with Policy Manager: %d",
				status);
			break;
		}
		sme_register_spectral_cb(mac);
		mac->sme.state = SME_STATE_START;

		/* START RRM */
		status = rrm_start(mac);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Failed to start RRM");
			break;
		}
	} while (0);
	return status;
}

static QDF_STATUS dfs_msg_processor(struct mac_context *mac,
		struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info *roam_info;
	tSirSmeCSAIeTxCompleteRsp *csa_ie_tx_complete_rsp;
	uint32_t session_id = 0;
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return QDF_STATUS_E_NOMEM;

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
			qdf_mem_free(roam_info);
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
	case eWNI_SME_CSA_RESTART_RSP:
	{
		session_id = msg->bodyval;
		roam_status = 0;
		roam_result = eCSR_ROAM_RESULT_CSA_RESTART_RSP;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "sapdfs: Received eCSR_ROAM_RESULT_DFS_CHANSW_UPDATE_REQ vdevid%d",
			  session_id);
		break;
	}
	default:
	{
		sme_err("Invalid DFS message: 0x%x", msg->type);
		qdf_mem_free(roam_info);
		status = QDF_STATUS_E_FAILURE;
		return status;
	}
	}

	/* Indicate Radar Event to SAP */
	csr_roam_call_callback(mac, session_id, roam_info, 0,
			       roam_status, roam_result);
	qdf_mem_free(roam_info);
	return status;
}


#ifdef WLAN_FEATURE_11W
/*
 * Handle the unprotected management frame indication from LIM and
 * forward it to HDD.
 */
static QDF_STATUS
sme_unprotected_mgmt_frm_ind(struct mac_context *mac,
			     tpSirSmeUnprotMgmtFrameInd pSmeMgmtFrm)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info *roam_info;
	uint32_t SessionId = pSmeMgmtFrm->sessionId;

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return QDF_STATUS_E_NOMEM;

	roam_info->nFrameLength = pSmeMgmtFrm->frameLen;
	roam_info->pbFrames = pSmeMgmtFrm->frameBuf;
	roam_info->frameType = pSmeMgmtFrm->frameType;

	/* forward the mgmt frame to HDD */
	csr_roam_call_callback(mac, SessionId, roam_info, 0,
			       eCSR_ROAM_UNPROT_MGMT_FRAME_IND, 0);

	qdf_mem_free(roam_info);

	return status;
}
#endif

QDF_STATUS sme_update_new_channel_event(mac_handle_t mac_handle,
					uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_info *roamInfo;
	eRoamCmdStatus roamStatus;
	eCsrRoamResult roamResult;

	roamInfo = qdf_mem_malloc(sizeof(*roamInfo));
	if (!roamInfo)
		return QDF_STATUS_E_FAILURE;

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
static QDF_STATUS sme_extended_change_channel_ind(struct mac_context *mac_ctx,
						void *msg_buf)
{
	struct sir_sme_ext_cng_chan_ind *ext_chan_ind;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t session_id = 0;
	struct csr_roam_info *roam_info;
	eRoamCmdStatus roam_status;
	eCsrRoamResult roam_result;

	ext_chan_ind = msg_buf;
	if (!ext_chan_ind) {
		sme_err("ext_chan_ind is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return QDF_STATUS_E_NOMEM;
	session_id = ext_chan_ind->session_id;
	roam_info->target_channel = ext_chan_ind->new_channel;
	roam_status = eCSR_ROAM_EXT_CHG_CHNL_IND;
	roam_result = eCSR_ROAM_EXT_CHG_CHNL_UPDATE_IND;
	sme_debug("sapdfs: Received eWNI_SME_EXT_CHANGE_CHANNEL_IND for session id [%d]",
		 session_id);

	/* Indicate Ext Channel Change event to SAP */
	csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
			       roam_status, roam_result);
	qdf_mem_free(roam_info);
	return status;
}

#ifdef FEATURE_WLAN_ESE
/**
 * sme_update_is_ese_feature_enabled() - enable/disable ESE support at runtime
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId: session id
 * @isEseIniFeatureEnabled: ese ini enabled
 *
 * It is used at in the REG_DYNAMIC_VARIABLE macro definition of
 * isEseIniFeatureEnabled. This is a synchronous call
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_update_is_ese_feature_enabled(mac_handle_t mac_handle,
			uint8_t sessionId, const bool isEseIniFeatureEnabled)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;

	if (mac->mlme_cfg->lfr.ese_enabled ==
	    isEseIniFeatureEnabled) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: ESE Mode is already enabled or disabled, nothing to do (returning) old(%d) new(%d)",
			  __func__,
			  mac->mlme_cfg->lfr.ese_enabled,
			  isEseIniFeatureEnabled);
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: EseEnabled is changed from %d to %d", __func__,
		  mac->mlme_cfg->lfr.ese_enabled,
		  isEseIniFeatureEnabled);
	mac->mlme_cfg->lfr.ese_enabled = isEseIniFeatureEnabled;
	csr_neighbor_roam_update_fast_roaming_enabled(
			mac, sessionId, isEseIniFeatureEnabled);

	if (true == isEseIniFeatureEnabled)
		mac->mlme_cfg->lfr.fast_transition_enabled = true;

	if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
		status = sme_acquire_global_lock(&mac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_ESE_INI_CFG_CHANGED);
			sme_release_global_lock(&mac->sme);
		} else {
			return status;
		}
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_set_plm_request(mac_handle_t mac_handle,
			       struct plm_req_params *req)
{
	QDF_STATUS status;
	bool ret = false;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	uint8_t ch_list[CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t count, valid_count = 0;
	struct scheduler_msg msg = {0};
	struct csr_roam_session *session;
	struct plm_req_params *body;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	session = CSR_GET_SESSION(mac, req->vdev_id);
	if (!session) {
		sme_err("session %d not found", req->vdev_id);
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	if (!session->sessionActive) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid Sessionid"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	/* per contract must make a copy of the params when messaging */
	body = qdf_mem_malloc(sizeof(*body));
	if (!req)
		return QDF_STATUS_E_NOMEM;
	*body = *req;

	if (!body->enable)
		goto send_plm_start;
	/* validating channel numbers */
	for (count = 0; count < body->plm_num_ch; count++) {
		uint8_t ch = body->plm_ch_list[count];

		ret = csr_is_supported_channel(mac, ch);
		if (!ret) {
			/* Not supported, ignore the channel */
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  FL("Unsupported channel %d ignored for PLM"),
				  ch);
			continue;
		}

		if (ch > 14) {
			enum channel_state state =
				wlan_reg_get_channel_state(mac->pdev, ch);

			if (state == CHANNEL_STATE_DFS) {
				/* DFS channel is provided, no PLM bursts can be
				 * transmitted. Ignoring these channels.
				 */
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_DEBUG,
					  FL("DFS channel %d ignored for PLM"),
					  ch);
				continue;
			}
		}
		ch_list[valid_count++] = ch;
	} /* End of for () */

	/* Copying back the valid channel list to plm struct */
	qdf_mem_zero(body->plm_ch_list, body->plm_num_ch);
	if (valid_count)
		qdf_mem_copy(body->plm_ch_list, ch_list, valid_count);
	/* All are invalid channels, FW need to send the PLM
	 *  report with "incapable" bit set.
	 */
	body->plm_num_ch = valid_count;

send_plm_start:
	/* PLM START */
	msg.type = WMA_SET_PLM_REQ;
	msg.reserved = 0;
	msg.bodyptr = body;

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_message(QDF_MODULE_ID_SME,
							  QDF_MODULE_ID_WMA,
							  QDF_MODULE_ID_WMA,
							  &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Not able to post WMA_SET_PLM_REQ to WMA"));
		sme_release_global_lock(&mac->sme);
		qdf_mem_free(body);
		return QDF_STATUS_E_FAILURE;
	}

	sme_release_global_lock(&mac->sme);
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_tsm_ie_ind() - sme tsm ie indication
 * @mac: Global mac context
 * @pSmeTsmIeInd: Pointer to tsm ie indication
 *
 * Handle the tsm ie indication from  LIM and forward it to HDD.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS sme_tsm_ie_ind(struct mac_context *mac,
				 struct tsm_ie_ind *pSmeTsmIeInd)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info *roam_info;
	uint32_t SessionId = pSmeTsmIeInd->sessionId;

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return QDF_STATUS_E_NOMEM;

	roam_info->tsm_ie.tsid = pSmeTsmIeInd->tsm_ie.tsid;
	roam_info->tsm_ie.state = pSmeTsmIeInd->tsm_ie.state;
	roam_info->tsm_ie.msmt_interval = pSmeTsmIeInd->tsm_ie.msmt_interval;
	/* forward the tsm ie information to HDD */
	csr_roam_call_callback(mac, SessionId, roam_info, 0,
			       eCSR_ROAM_TSM_IE_IND, 0);
	qdf_mem_free(roam_info);
	return status;
}

/**
 * sme_set_cckm_ie() - set cckm ie
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId: session id
 * @pCckmIe: Pointer to CCKM Ie
 * @cckmIeLen: Length of @pCckmIe
 *
 * Function to store the CCKM IE passed from supplicant and use
 * it while packing reassociation request.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_cckm_ie(mac_handle_t mac_handle, uint8_t sessionId,
			   uint8_t *pCckmIe, uint8_t cckmIeLen)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_set_cckm_ie(mac, sessionId, pCckmIe, cckmIeLen);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_set_ese_beacon_request() - set ese beacon request
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId: session id
 * @in_req: Ese beacon report request
 *
 * function to set ESE beacon request parameters
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_ese_beacon_request(mac_handle_t mac_handle,
				      const uint8_t sessionId,
				      const tCsrEseBeaconReq *in_req)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tpSirBeaconReportReqInd pSmeBcnReportReq = NULL;
	const tCsrEseBeaconReqParams *pBeaconReq = NULL;
	uint8_t counter = 0;
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);
	tpRrmSMEContext pSmeRrmContext = &mac->rrm.rrmSmeContext;

	if (pSmeRrmContext->eseBcnReqInProgress == true) {
		sme_err("A Beacon Report Req is already in progress");
		return QDF_STATUS_E_RESOURCES;
	}

	/* Store the info in RRM context */
	qdf_mem_copy(&pSmeRrmContext->eseBcnReqInfo, in_req,
		     sizeof(tCsrEseBeaconReq));

	/* Prepare the request to send to SME. */
	pSmeBcnReportReq = qdf_mem_malloc(sizeof(tSirBeaconReportReqInd));
	if (!pSmeBcnReportReq)
		return QDF_STATUS_E_NOMEM;

	pSmeRrmContext->eseBcnReqInProgress = true;

	sme_debug("Sending Beacon Report Req to SME");

	pSmeBcnReportReq->messageType = eWNI_SME_BEACON_REPORT_REQ_IND;
	pSmeBcnReportReq->length = sizeof(tSirBeaconReportReqInd);
	qdf_mem_copy(pSmeBcnReportReq->bssId,
		     pSession->connectedProfile.bssid.bytes,
		     sizeof(tSirMacAddr));
	pSmeBcnReportReq->channelInfo.channelNum = 255;
	pSmeBcnReportReq->channelList.numChannels = in_req->numBcnReqIe;
	pSmeBcnReportReq->msgSource = eRRM_MSG_SOURCE_ESE_UPLOAD;

	for (counter = 0; counter < in_req->numBcnReqIe; counter++) {
		pBeaconReq = &in_req->bcnReq[counter];
		pSmeBcnReportReq->fMeasurementtype[counter] =
			pBeaconReq->scanMode;
		pSmeBcnReportReq->measurementDuration[counter] =
			SYS_TU_TO_MS(pBeaconReq->measurementDuration);
		pSmeBcnReportReq->channelList.channelNumber[counter] =
			pBeaconReq->channel;
	}

	status = sme_rrm_process_beacon_report_req_ind(mac, pSmeBcnReportReq);

	if (status != QDF_STATUS_SUCCESS)
		pSmeRrmContext->eseBcnReqInProgress = false;

	qdf_mem_free(pSmeBcnReportReq);

	return status;
}

/**
 * sme_get_tsm_stats() - SME get tsm stats
 * @mac_handle: Opaque handle to the global MAC context
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
QDF_STATUS sme_get_tsm_stats(mac_handle_t mac_handle,
			     tCsrTsmStatsCallback callback,
			     uint8_t staId, struct qdf_mac_addr bssId,
			     void *pContext, uint8_t tid)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_tsm_stats(mac, callback,
					   staId, bssId, pContext,
					   tid);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_set_ese_roam_scan_channel_list() - To set ese roam scan channel list
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId: sme session id
 * @pChannelList: Output channel list
 * @numChannels: Output number of channels
 *
 * This routine is called to set ese roam scan channel list.
 * This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_ese_roam_scan_channel_list(mac_handle_t mac_handle,
					      uint8_t sessionId,
					      uint8_t *pChannelList,
					      uint8_t numChannels)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	tpCsrChannelInfo curchnl_list_info = NULL;
	uint8_t oldChannelList[CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[128] = { 0 };
	uint8_t i = 0, j = 0;
	enum band_info band = -1;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
	curchnl_list_info =
		&pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	if (curchnl_list_info->ChannelList) {
		for (i = 0; i < curchnl_list_info->numOfChannels; i++) {
			j += snprintf(oldChannelList + j,
				sizeof(oldChannelList) - j, "%d",
				curchnl_list_info->ChannelList[i]);
		}
	}
        ucfg_reg_get_band(mac->pdev, &band);
	status = csr_create_roam_scan_channel_list(mac, sessionId,
				pChannelList, numChannels,
				band);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (curchnl_list_info->ChannelList) {
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

	if (mac->mlme_cfg->lfr.roam_scan_offload_enabled)
		csr_roam_offload_scan(mac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_CHANNEL_LIST_CHANGED);

	sme_release_global_lock(&mac->sme);
	return status;
}

#endif /* FEATURE_WLAN_ESE */

static
QDF_STATUS sme_ibss_peer_info_response_handler(struct mac_context *mac,
					       tpSirIbssGetPeerInfoRspParams
					       pIbssPeerInfoParams)
{
	struct ibss_peer_info_cb_info *cb_info;

	if (!mac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: mac is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cb_info = &mac->sme.peer_info_cb_info;
	if (!cb_info->peer_info_cb) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD callback is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cb_info->peer_info_cb(cb_info->peer_info_cb_context,
			      &pIbssPeerInfoParams->ibssPeerInfoRspParams);
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
static QDF_STATUS sme_process_dual_mac_config_resp(struct mac_context *mac,
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
static QDF_STATUS sme_process_antenna_mode_resp(struct mac_context *mac,
		uint8_t *msg)
{
	tListElem *entry;
	tSmeCmd *command;
	bool found;
	void *context = NULL;
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

	context = command->u.set_antenna_mode_cmd.set_antenna_mode_ctx;
	callback = command->u.set_antenna_mode_cmd.set_antenna_mode_resp;
	if (callback) {
		if (!param)
			sme_err("Set antenna mode call back is NULL");
		else
			callback(param->status, context);
	} else {
		sme_err("Callback does not exist");
	}

	found = csr_nonscan_active_ll_remove_entry(mac, entry, LL_ACCESS_LOCK);
	if (found)
		/* Now put this command back on the available command list */
		csr_release_command(mac, command);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_process_msg(struct mac_context *mac, struct scheduler_msg *pMsg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sir_peer_info *peer_stats;
	struct sir_peer_info_resp *peer_info_rsp;

	if (!pMsg) {
		sme_err("Empty message for SME");
		return status;
	}
	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		return status;
	}
	if (!SME_IS_START(mac)) {
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
		csr_process_ho_fail_ind(mac, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
#endif
	case eWNI_SME_ADDTS_RSP:
	case eWNI_SME_DELTS_RSP:
	case eWNI_SME_DELTS_IND:
	case eWNI_SME_FT_AGGR_QOS_RSP:
		/* QoS */
		if (pMsg->bodyptr) {
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
			status = sme_qos_msg_processor(mac, pMsg->type,
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
			status = sme_rrm_msg_processor(mac, pMsg->type,
						       pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_ADD_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_add_sta_session_rsp(mac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_DEL_STA_SELF_RSP:
		if (pMsg->bodyptr) {
			status = csr_process_del_sta_session_rsp(mac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_GENERIC_CHANGE_COUNTRY_CODE:
		if (pMsg->bodyptr) {
			status = sme_handle_generic_change_country_code(
						(void *)mac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;

#ifdef WLAN_FEATURE_11W
	case eWNI_SME_UNPROT_MGMT_FRM_IND:
		if (pMsg->bodyptr) {
			sme_unprotected_mgmt_frm_ind(mac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_ESE
	case eWNI_SME_TSM_IE_IND:
		if (pMsg->bodyptr) {
			sme_tsm_ie_ind(mac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif /* FEATURE_WLAN_ESE */
	case eWNI_SME_ROAM_SCAN_OFFLOAD_RSP:
		status = csr_roam_offload_scan_rsp_hdlr((void *)mac,
							pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_IBSS_PEER_INFO_RSP:
		if (pMsg->bodyptr) {
			sme_ibss_peer_info_response_handler(mac,
							    pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	case eWNI_SME_READY_TO_EXTWOW_IND:
		if (pMsg->bodyptr) {
			sme_process_ready_to_ext_wow(mac, pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case eWNI_SME_AUTO_SHUTDOWN_IND:
		if (mac->sme.auto_shutdown_cb) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  FL("Auto shutdown notification"));
			mac->sme.auto_shutdown_cb();
		}
		break;
#endif
	case eWNI_SME_DFS_RADAR_FOUND:
	case eWNI_SME_DFS_CAC_COMPLETE:
	case eWNI_SME_DFS_CSAIE_TX_COMPLETE_IND:
	case eWNI_SME_CSA_RESTART_RSP:
		status = dfs_msg_processor(mac, pMsg);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_CHANNEL_CHANGE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_channel_change_resp(mac,
								 pMsg->type,
								 pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_STATS_EXT_EVENT:
		status = sme_stats_ext_event(mac, pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_GET_PEER_INFO_IND:
		if (mac->sme.pget_peer_info_ind_cb)
			mac->sme.pget_peer_info_ind_cb(pMsg->bodyptr,
				mac->sme.pget_peer_info_cb_context);
		if (pMsg->bodyptr) {
			peer_info_rsp = (struct sir_peer_info_resp *)
							(pMsg->bodyptr);
			peer_stats = (struct sir_peer_info *)
							(peer_info_rsp->info);
			if (peer_stats) {
				mac->peer_rssi = peer_stats[0].rssi;
				mac->peer_txrate = peer_stats[0].tx_rate;
				mac->peer_rxrate = peer_stats[0].rx_rate;
			}
		}
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_GET_PEER_INFO_EXT_IND:
		if (mac->sme.pget_peer_info_ext_ind_cb)
			mac->sme.pget_peer_info_ext_ind_cb(pMsg->bodyptr,
				mac->sme.pget_peer_info_ext_cb_context);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_FW_STATUS_IND:
		status = sme_fw_state_resp(mac);
		break;
	case eWNI_SME_TSF_EVENT:
		if (mac->sme.get_tsf_cb) {
			mac->sme.get_tsf_cb(mac->sme.get_tsf_cxt,
					(struct stsf *)pMsg->bodyptr);
		}
		if (pMsg->bodyptr)
			qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_LINK_STATUS_IND:
	{
		tAniGetLinkStatus *pLinkStatus =
			(tAniGetLinkStatus *) pMsg->bodyptr;
		if (pLinkStatus) {
			if (mac->sme.link_status_callback)
				mac->sme.link_status_callback(
					pLinkStatus->linkStatus,
					mac->sme.link_status_context);

			mac->sme.link_status_callback = NULL;
			mac->sme.link_status_context = NULL;
			qdf_mem_free(pLinkStatus);
		}
		break;
	}
	case eWNI_SME_MSG_GET_TEMPERATURE_IND:
		if (mac->sme.temperature_cb)
			mac->sme.temperature_cb(pMsg->bodyval,
					mac->sme.temperature_cb_context);
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
		if (mac->sme.ext_scan_ind_cb)
			mac->sme.ext_scan_ind_cb(mac->hdd_handle,
					eSIR_EXTSCAN_FULL_SCAN_RESULT_IND,
					pMsg->bodyptr);
		else
			sme_err("callback not registered to process: %d",
				pMsg->type);

		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_EPNO_NETWORK_FOUND_IND:
		if (mac->sme.ext_scan_ind_cb)
			mac->sme.ext_scan_ind_cb(mac->hdd_handle,
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
			status = sme_process_set_hw_mode_resp(mac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_HW_MODE_TRANS_IND:
		if (pMsg->bodyptr) {
			status = sme_process_hw_mode_trans_ind(mac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_NSS_UPDATE_RSP:
		if (pMsg->bodyptr) {
			status = sme_process_nss_update_resp(mac,
								pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_SET_DUAL_MAC_CFG_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_dual_mac_config_resp(mac,
					pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_SET_THERMAL_LEVEL_IND:
		if (mac->sme.set_thermal_level_cb)
			mac->sme.set_thermal_level_cb(mac->hdd_handle,
						       pMsg->bodyval);
		break;
	case eWNI_SME_EXT_CHANGE_CHANNEL_IND:
		 status = sme_extended_change_channel_ind(mac, pMsg->bodyptr);
		 qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_SET_ANTENNA_MODE_RESP:
		if (pMsg->bodyptr) {
			status = sme_process_antenna_mode_resp(mac,
					pMsg->bodyptr);
			qdf_mem_free(pMsg->bodyptr);
		} else {
			sme_err("Empty message for: %d", pMsg->type);
		}
		break;
	case eWNI_SME_LOST_LINK_INFO_IND:
		if (mac->sme.lost_link_info_cb)
			mac->sme.lost_link_info_cb(mac->hdd_handle,
				(struct sir_lost_link_info *)pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_RSO_CMD_STATUS_IND:
		if (mac->sme.rso_cmd_status_cb)
			mac->sme.rso_cmd_status_cb(mac->hdd_handle,
						    pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWMI_SME_LL_STATS_IND:
		if (mac->sme.link_layer_stats_ext_cb)
			mac->sme.link_layer_stats_ext_cb(mac->hdd_handle,
							  pMsg->bodyptr);
		qdf_mem_free(pMsg->bodyptr);
		break;
	case eWNI_SME_BT_ACTIVITY_INFO_IND:
		if (mac->sme.bt_activity_info_cb)
			mac->sme.bt_activity_info_cb(mac->hdd_handle,
						      pMsg->bodyval);
		break;
	case eWNI_SME_HIDDEN_SSID_RESTART_RSP:
		if (mac->sme.hidden_ssid_cb)
			mac->sme.hidden_ssid_cb(mac->hdd_handle, pMsg->bodyval);
		else
			sme_err("callback is NULL");
		break;
	default:

		if ((pMsg->type >= eWNI_SME_MSG_TYPES_BEGIN)
		    && (pMsg->type <= eWNI_SME_MSG_TYPES_END)) {
			/* CSR */
			if (pMsg->bodyptr) {
				status = csr_msg_processor(mac, pMsg->bodyptr);
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
	sme_release_global_lock(&mac->sme);
	return status;
}

QDF_STATUS sme_mc_process_handler(struct scheduler_msg *msg)
{
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_SME);

	if (!mac_ctx) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return sme_process_msg(mac_ctx, msg);
}

/**
 * sme_process_nss_update_resp() - Process nss update response
 * @mac: Global MAC pointer
 * @msg: nss update response
 *
 * Processes the nss update response and invokes the HDD
 * callback to process further
 */
static QDF_STATUS sme_process_nss_update_resp(struct mac_context *mac, uint8_t *msg)
{
	tListElem *entry = NULL;
	tSmeCmd *command = NULL;
	bool found;
	policy_mgr_nss_update_cback callback = NULL;
	struct sir_bcn_update_rsp *param;

	param = (struct sir_bcn_update_rsp *)msg;
	if (!param)
		sme_err("nss update resp param is NULL");
		/* Not returning. Need to check if active command list
		 * needs to be freed
		 */

	if (param && param->reason != REASON_NSS_UPDATE) {
		sme_err("reason not NSS update");
		return QDF_STATUS_E_INVAL;
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
				param->status,
				param->vdev_id,
				command->u.nss_update_cmd.next_action,
				command->u.nss_update_cmd.reason,
				command->u.nss_update_cmd.original_vdev_id);
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

QDF_STATUS sme_stop(mac_handle_t mac_handle)
{
	QDF_STATUS status;
	QDF_STATUS ret_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = rrm_stop(mac);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret_status = status;
		sme_err("rrm_stop failed with status: %d", status);
	}

	status = csr_stop(mac);
	if (QDF_IS_STATUS_ERROR(status)) {
		ret_status = status;
		sme_err("csr_stop failed with status: %d", status);
	}

	mac->sme.state = SME_STATE_STOP;

	return ret_status;
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
 * mac_handle - The handle returned by mac_open
 * Return QDF_STATUS_SUCCESS - SME is successfully close.
 *
 *  Other status means SME is failed to be closed but caller still cannot
 *  call any other SME functions except smeOpen.
 */
QDF_STATUS sme_close(mac_handle_t mac_handle)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS fail_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac)
		return QDF_STATUS_E_FAILURE;

	status = csr_close(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_close failed with status: %d", status);
		fail_status = status;
	}
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
	status = sme_qos_close(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Qos close failed with status: %d", status);
		fail_status = status;
	}
#endif
	status = sme_ps_close(mac_handle);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_ps_close failed status: %d", status);
		fail_status = status;
	}

	status = rrm_close(mac);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("RRM close failed with status: %d", status);
		fail_status = status;
	}

	free_sme_cmd_list(mac);

	status = qdf_mutex_destroy(&mac->sme.sme_global_lock);
	if (!QDF_IS_STATUS_SUCCESS(status))
		fail_status = QDF_STATUS_E_FAILURE;

	mac->sme.state = SME_STATE_STOP;

	return fail_status;
}

/**
 * sme_remove_bssid_from_scan_list() - wrapper to remove the bssid from
 * scan list
 * @mac_handle: Opaque handle to the global MAC context.
 * @bssid: bssid to be removed
 *
 * This function remove the given bssid from scan list.
 *
 * Return: QDF status.
 */
QDF_STATUS sme_remove_bssid_from_scan_list(mac_handle_t mac_handle,
					   tSirMacAddr bssid)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

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
QDF_STATUS sme_scan_get_result(mac_handle_t mac_handle, uint8_t sessionId,
			       tCsrScanResultFilter *pFilter,
			       tScanResultHandle *phResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_GET_RESULTS, sessionId,
			 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_get_result(mac, pFilter, phResult);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_scan_get_result_for_bssid(mac_handle_t mac_handle,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_get_result_for_bssid(mac_ctx, bssid, res);
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
	return sme_get_ap_channel_from_scan_cache((struct csr_roam_profile *)
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
	struct csr_roam_profile *profile, tScanResultHandle *scan_cache,
	uint8_t *ap_chnl_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac_ctx = sme_get_mac_context();
	tCsrScanResultFilter *scan_filter = NULL;
	tScanResultHandle filtered_scan_result = NULL;
	tSirBssDescription first_ap_profile;

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("mac_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (!scan_filter)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&first_ap_profile, sizeof(tSirBssDescription));
	if (!profile) {
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
		if (profile) {
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
			sme_err("Failed to get scan get result");
			status = QDF_STATUS_E_FAILURE;
		}
		csr_free_scan_filter(mac_ctx, scan_filter);
		sme_release_global_lock(&mac_ctx->sme);
	} else {
		csr_free_scan_filter(mac_ctx, scan_filter);
		status = QDF_STATUS_E_FAILURE;
	}
	qdf_mem_free(scan_filter);
	return status;
}

/*
 * sme_scan_flush_result() -
 * A wrapper function to request CSR to clear scan results.
 * This is a synchronous call
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_scan_flush_result(mac_handle_t mac_handle)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_FLUSH_RESULTS,
			 0, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_scan_flush_result(mac);
		sme_release_global_lock(&mac->sme);
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
tCsrScanResultInfo *sme_scan_result_get_first(mac_handle_t mac_handle,
					      tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tCsrScanResultInfo *pRet = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_SCAN_RESULT_GETFIRST,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pRet = csr_scan_result_get_first(mac, hScanResult);
		sme_release_global_lock(&mac->sme);
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
tCsrScanResultInfo *sme_scan_result_get_next(mac_handle_t mac_handle,
					     tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tCsrScanResultInfo *pRet = NULL;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pRet = csr_scan_result_get_next(mac, hScanResult);
		sme_release_global_lock(&mac->sme);
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
	struct mac_context *mac_ctx = sme_get_mac_context();

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

eCsrPhyMode sme_get_phy_mode(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->roam.configParam.phyMode;
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
QDF_STATUS sme_roam_connect(mac_handle_t mac_handle, uint8_t sessionId,
			    struct csr_roam_profile *pProfile,
			    uint32_t *pRoamId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac)
		return QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_CONNECT, sessionId, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId)) {
			status =
				csr_roam_connect(mac, sessionId, pProfile,
						 pRoamId);
		} else {
			sme_err("Invalid sessionID: %d", sessionId);
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_set_phy_mode() -
 * Changes the PhyMode.
 *
 * mac_handle - The handle returned by mac_open.
 * phyMode new phyMode which is to set
 * Return QDF_STATUS  SUCCESS.
 */
QDF_STATUS sme_set_phy_mode(mac_handle_t mac_handle, eCsrPhyMode phyMode)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->roam.configParam.phyMode = phyMode;
	mac->roam.configParam.uCfgDot11Mode =
		csr_get_cfg_dot11_mode_from_csr_phy_mode(NULL,
						mac->roam.configParam.phyMode,
						mac->roam.configParam.
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
QDF_STATUS sme_roam_reassoc(mac_handle_t mac_handle, uint8_t sessionId,
			    struct csr_roam_profile *pProfile,
			    tCsrRoamModifyProfileFields modProfileFields,
			    uint32_t *pRoamId, bool fForce)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_REASSOC, sessionId, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId)) {
			if ((!pProfile) && (fForce == 1))
				status = csr_reassoc(mac, sessionId,
						     &modProfileFields, pRoamId,
						     fForce);
			else
				status = csr_roam_reassoc(mac, sessionId,
						pProfile,
						modProfileFields, pRoamId);
		} else {
			status = QDF_STATUS_E_INVAL;
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_roam_disconnect(mac_handle_t mac_handle, uint8_t session_id,
			       eCsrRoamDisconnectReason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DISCONNECT, session_id,
			 reason));
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id))
			status = csr_roam_disconnect(mac_ctx, session_id,
						     reason);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}

/* sme_dhcp_done_ind() - send dhcp done ind
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 *
 * Return: void.
 */
void sme_dhcp_done_ind(mac_handle_t mac_handle, uint8_t session_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
 * mac_handle - Global structure
 * sessionId - sessionId of SoftAP
 * Return QDF_STATUS  SUCCESS  Roam callback will be called to indicate
 * actual results
 */
QDF_STATUS sme_roam_stop_bss(mac_handle_t mac_handle, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_issue_stop_bss_cmd(mac, sessionId,
							false);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/**
 * sme_roam_disconnect_sta() - disassociate a station
 * @mac_handle:          Global structure
 * @sessionId:     SessionId of SoftAP
 * @p_del_sta_params: Pointer to parameters of the station to disassoc
 *
 * To disassociate a station. This is an asynchronous API.
 *
 * Return: QDF_STATUS_SUCCESS on success.Roam callback will
 *         be called to indicate actual result.
 */
QDF_STATUS sme_roam_disconnect_sta(mac_handle_t mac_handle, uint8_t sessionId,
				   struct csr_del_sta_params *p_del_sta_params)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		QDF_ASSERT(0);
		return status;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_issue_disassociate_sta_cmd(mac,
					sessionId, p_del_sta_params);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/**
 * sme_roam_deauth_sta() - deauthenticate a station
 * @mac_handle:          Global structure
 * @sessionId:     SessionId of SoftAP
 * @pDelStaParams: Pointer to parameters of the station to deauthenticate
 *
 * To disassociate a station. This is an asynchronous API.
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS error
 *         code on error. Roam callback will be called to indicate actual
 *         result
 */
QDF_STATUS sme_roam_deauth_sta(mac_handle_t mac_handle, uint8_t sessionId,
			       struct csr_del_sta_params *pDelStaParams)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		QDF_ASSERT(0);
		return status;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_MSG_DEAUTH_STA,
			 sessionId, pDelStaParams->reason_code));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status =
				csr_roam_issue_deauth_sta_cmd(mac, sessionId,
							      pDelStaParams);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_roam_get_connect_profile(mac_handle_t mac_handle,
					uint8_t sessionId,
					tCsrRoamConnectedProfile *pProfile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_GET_CONNECTPROFILE,
			 sessionId, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_get_connect_profile(mac, sessionId,
							pProfile);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_roam_set_pmkid_cache(mac_handle_t mac_handle, uint8_t sessionId,
				    tPmkidCacheInfo *pPMKIDCache,
				    uint32_t numItems, bool update_entire_cache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_SET_PMKIDCACHE, sessionId,
			 numItems));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_set_pmkid_cache(mac, sessionId,
						pPMKIDCache,
						numItems, update_entire_cache);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_roam_del_pmkid_from_cache(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 tPmkidCacheInfo *pmksa,
					 bool flush_cache)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_ROAM_DEL_PMKIDCACHE,
			 sessionId, flush_cache));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_del_pmkid_from_cache(mac, sessionId,
						       pmksa, flush_cache);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void sme_get_pmk_info(mac_handle_t mac_handle, uint8_t session_id,
		      tPmkidCacheInfo *pmk_cache)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = sme_acquire_global_lock(&mac_ctx->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id))
			csr_get_pmk_info(mac_ctx, session_id, pmk_cache);
		sme_release_global_lock(&mac_ctx->sme);
	}
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*
 * \fn sme_roam_set_psk_pmk
 * \brief a wrapper function to request CSR to save PSK/PMK
 *  This is a synchronous call.
 * \param mac_handle - Global structure
 * \param sessionId - SME sessionId
 * \param pPSK_PMK - pointer to an array of Psk[]/Pmk
 * \param pmk_len - Length could be only 16 bytes in case if LEAP
 *                  connections. Need to pass this information to
 *                  firmware.
 * \return QDF_STATUS -status whether PSK/PMK is set or not
 */
QDF_STATUS sme_roam_set_psk_pmk(mac_handle_t mac_handle, uint8_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_roam_set_psk_pmk(mac, sessionId, pPSK_PMK,
						     pmk_len);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}
#endif

QDF_STATUS sme_roam_get_wpa_rsn_req_ie(mac_handle_t mac_handle,
				       uint8_t session_id,
				       uint32_t *len, uint8_t *buf)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, session_id))
			status = csr_roam_get_wpa_rsn_req_ie(mac, session_id,
							     len, buf);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_roam_get_wpa_rsn_rsp_ie(mac_handle_t mac_handle,
				       uint8_t session_id,
				       uint32_t *len, uint8_t *buf)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, session_id))
			status = csr_roam_get_wpa_rsn_rsp_ie(mac, session_id,
							     len, buf);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_get_config_param(mac_handle_t mac_handle,
				struct sme_config_params *pParam)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_CONFIGPARAM, NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_config_param(mac, &pParam->csrConfig);
		if (status != QDF_STATUS_SUCCESS) {
			sme_err("csr_get_config_param failed");
			sme_release_global_lock(&mac->sme);
			return status;
		}
		sme_release_global_lock(&mac->sme);
	}

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
QDF_STATUS sme_get_modify_profile_fields(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_MODPROFFIELDS, sessionId,
			 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_IS_SESSION_VALID(mac, sessionId))
			status = csr_get_modify_profile_fields(mac, sessionId,
							  pModifyProfileFields);
		else
			status = QDF_STATUS_E_INVAL;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * sme_register_oem_data_rsp_callback() - Register a routine of
 *                                        type send_oem_data_rsp_msg
 * @mac_handle:                                Handle returned by mac_open.
 * @callback:                             Callback to send response
 *                                        to oem application.
 *
 * sme_oem_data_rsp_callback is used to register sme_send_oem_data_rsp_msg
 * callback function.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS sme_register_oem_data_rsp_callback(mac_handle_t mac_handle,
				sme_send_oem_data_rsp_msg callback)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *pmac = MAC_CONTEXT(mac_handle);

	pmac->sme.oem_data_rsp_callback = callback;

	return status;

}

/**
 * sme_deregister_oem_data_rsp_callback() - De-register OEM datarsp callback
 * @mac_handle: Handler return by mac_open
 * This function De-registers the OEM data response callback  to SME
 *
 * Return: None
 */
void  sme_deregister_oem_data_rsp_callback(mac_handle_t mac_handle)
{
	struct mac_context *pmac;

	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("mac_handle is not valid"));
		return;
	}
	pmac = MAC_CONTEXT(mac_handle);

	pmac->sme.oem_data_rsp_callback = NULL;
}

/**
 * sme_oem_update_capability() - update UMAC's oem related capability.
 * @mac_handle: Handle returned by mac_open
 * @oem_cap: pointer to oem_capability
 *
 * This function updates OEM capability to UMAC. Currently RTT
 * related capabilities are updated. More capabilities can be
 * added in future.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_update_capability(mac_handle_t mac_handle,
				     struct sme_oem_capability *cap)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *pmac = MAC_CONTEXT(mac_handle);
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
 * @mac_handle: Handle returned by mac_open
 * @oem_cap: pointer to oem_capability
 *
 * This function is used to get the OEM capability from UMAC.
 * Currently RTT related capabilities are received. More
 * capabilities can be added in future.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_get_capability(mac_handle_t mac_handle,
				  struct sme_oem_capability *cap)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *pmac = MAC_CONTEXT(mac_handle);
	uint8_t *bytes;

	bytes = pmac->rrm.rrmSmeContext.rrmConfig.rm_capability;

	cap->ftm_rr = bytes[4] & RM_CAP_FTM_RANGE_REPORT;
	cap->lci_capability = bytes[4] & RM_CAP_CIVIC_LOC_MEASUREMENT;

	return status;
}
#endif

/**
 * sme_roam_set_key() - To set encryption key.
 * @mac_handle:           mac_handle global context
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
QDF_STATUS sme_roam_set_key(mac_handle_t mac_handle,  uint8_t session_id,
			    tCsrRoamSetKey *set_key, uint32_t *ptr_roam_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 * @default_idx: default wep key index
 *
 * This function prepares a message and post to WMA to set wep default
 * key index
 *
 * Return: Success:QDF_STATUS_SUCCESS Failure: Error value
 */
QDF_STATUS sme_roam_set_default_key_index(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint8_t default_idx)
{
	struct scheduler_msg msg = {0};
	struct wep_update_default_key_idx *update_key;

	update_key = qdf_mem_malloc(sizeof(*update_key));
	if (!update_key)
		return QDF_STATUS_E_NOMEM;

	update_key->session_id = session_id;
	update_key->default_idx = default_idx;

	msg.type = WMA_UPDATE_WEP_DEFAULT_KEY;
	msg.reserved = 0;
	msg.bodyptr = (void *)update_key;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_message(QDF_MODULE_ID_SME,
				   QDF_MODULE_ID_WMA,
				   QDF_MODULE_ID_WMA, &msg)) {
		qdf_mem_free(update_key);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * sme_get_rssi() - API to retrieve current RSSI
 * @mac_handle: Opaque handle to the global MAC context
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
QDF_STATUS sme_get_rssi(mac_handle_t mac_handle,
			tCsrRssiCallback callback, uint8_t staId,
			struct qdf_mac_addr bssId, int8_t lastRSSI,
			void *pContext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_RSSI, NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_rssi(mac, callback,
				      staId, bssId, lastRSSI,
				      pContext);
		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_get_snr(mac_handle_t mac_handle,
		       tCsrSnrCallback callback,
		       uint8_t staId, struct qdf_mac_addr bssId, void *pContext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_snr(mac, callback, staId, bssId, pContext);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifndef QCA_SUPPORT_CP_STATS
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
QDF_STATUS sme_get_statistics(mac_handle_t mac_handle,
			      eCsrStatsRequesterType requesterId,
			      uint32_t statsMask, tCsrStatsCallback callback,
			      uint8_t staId, void *pContext, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_get_statistics(mac, requesterId, statsMask,
					    callback, staId, pContext,
					    sessionId);
		sme_release_global_lock(&mac->sme);
	}

	return status;

}
#endif

QDF_STATUS sme_get_link_status(mac_handle_t mac_handle,
			       csr_link_status_callback callback,
			       void *context, uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tAniGetLinkStatus *msg;
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		msg = qdf_mem_malloc(sizeof(*msg));
		if (!msg) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		msg->msgType = WMA_LINK_STATUS_GET_REQ;
		msg->msgLen = sizeof(*msg);
		msg->sessionId = session_id;
		mac->sme.link_status_context = context;
		mac->sme.link_status_callback = callback;

		message.type = WMA_LINK_STATUS_GET_REQ;
		message.bodyptr = msg;
		message.reserved = 0;

		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("post msg failed, %d", status);
			qdf_mem_free(msg);
			mac->sme.link_status_context = NULL;
			mac->sme.link_status_callback = NULL;
		}

		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_get_country_code(mac_handle_t mac_handle, uint8_t *pBuf,
				uint8_t *pbLen)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_CNTRYCODE, NO_SESSION, 0));

	return csr_get_country_code(mac, pBuf, pbLen);
}

/*
 * sme_generic_change_country_code() -
 * Change Country code from upperlayer during WLAN driver operation.
 * This is a synchronous API.
 *
 * mac_handle - The handle returned by mac_open.
 * pCountry New Country Code String
 * reg_domain regulatory domain
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_generic_change_country_code(mac_handle_t mac_handle,
					   uint8_t *pCountry)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};
	tAniGenericChangeCountryCodeReq *pMsg;

	if (!mac) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
			  "%s: mac is null", __func__);
		return status;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pMsg = qdf_mem_malloc(sizeof(tAniGenericChangeCountryCodeReq));
		if (!pMsg) {
			sme_release_global_lock(&mac->sme);
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
		    scheduler_post_message(QDF_MODULE_ID_SME,
					   QDF_MODULE_ID_SME,
					   QDF_MODULE_ID_SME, &msg)) {
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_dhcp_start_ind() -
 * API to signal the FW about the DHCP Start event.
 *
 * mac_handle: Opaque handle to the global MAC context.
 * device_mode - mode(AP,SAP etc) of the device.
 * macAddr - MAC address of the adapter.
 * sessionId - session ID.
 * Return QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_dhcp_start_ind(mac_handle_t mac_handle,
			      uint8_t device_mode,
			      uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tAniDHCPInd *pMsg;
	struct csr_roam_session *pSession;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(mac, sessionId);

		if (!pSession) {
			sme_err("Session: %d not found", sessionId);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		pSession->dhcp_done = false;

		pMsg = qdf_mem_malloc(sizeof(tAniDHCPInd));
		if (!pMsg) {
			sme_release_global_lock(&mac->sme);
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
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post DHCP Start MSG fail", __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_dhcp_stop_ind() -
 * API to signal the FW about the DHCP complete event.
 *
 * mac_handle: Opaque handle to the global MAC context.
 * device_mode - mode(AP, SAP etc) of the device.
 * macAddr - MAC address of the adapter.
 * sessionId - session ID.
 * Return QDF_STATUS  SUCCESS.
 *			FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_dhcp_stop_ind(mac_handle_t mac_handle,
			     uint8_t device_mode,
			     uint8_t *macAddr, uint8_t sessionId)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tAniDHCPInd *pMsg;
	struct csr_roam_session *pSession;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pSession = CSR_GET_SESSION(mac, sessionId);

		if (!pSession) {
			sme_err("Session: %d not found", sessionId);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		pSession->dhcp_done = true;

		pMsg = qdf_mem_malloc(sizeof(tAniDHCPInd));
		if (!pMsg) {
			sme_release_global_lock(&mac->sme);
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
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post DHCP Stop MSG fail", __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&mac->sme);
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
QDF_STATUS sme_tx_fail_monitor_start_stop_ind(mac_handle_t mac_handle,
					      uint8_t tx_fail_count,
					      void *txFailIndCallback)
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tAniTXFailMonitorInd *pMsg;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		pMsg = qdf_mem_malloc(sizeof(tAniTXFailMonitorInd));
		if (!pMsg) {
			sme_release_global_lock(&mac->sme);
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

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post TX Fail monitor Start MSG fail",
				  __func__);
			qdf_mem_free(pMsg);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_neighbor_report_request() -
 * API to request neighbor report.
 *
 * mac_handle - The handle returned by mac_open.
 * pRrmNeighborReq - Pointer to a caller allocated object of type
 *			      tRrmNeighborReq. Caller owns the memory and is
 *			      responsible for freeing it.
 * Return QDF_STATUS
 *	    QDF_STATUS_E_FAILURE - failure
 *	    QDF_STATUS_SUCCESS  success
 */
QDF_STATUS sme_neighbor_report_request(
				mac_handle_t mac_handle,
				uint8_t sessionId,
				tpRrmNeighborReq pRrmNeighborReq,
				tpRrmNeighborRspCallbackInfo callbackInfo)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_NEIGHBOR_REPORTREQ, NO_SESSION,
			 0));

	if (pRrmNeighborReq->neighbor_report_offload) {
		status = csr_invoke_neighbor_report_request(sessionId,
							    pRrmNeighborReq,
							    false);
		return status;
	}

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&mac->sme)) {
		status =
			sme_rrm_neighbor_report_request(mac, sessionId,
						pRrmNeighborReq, callbackInfo);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * sme_oem_data_req() - send oem data request to WMA
 * @mac_handle: Opaque handle to the global MAC context
 * @hdd_oem_req: OEM data request from HDD
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_oem_data_req(mac_handle_t mac_handle,
			    struct oem_data_req *hdd_oem_req)
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
	if (!oem_data_req)
		return QDF_STATUS_E_NOMEM;

	oem_data_req->data_len = hdd_oem_req->data_len;
	oem_data_req->data = qdf_mem_malloc(oem_data_req->data_len);
	if (!oem_data_req->data)
		return QDF_STATUS_E_NOMEM;

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

#define STA_NSS_CHAINS_SHIFT               0
#define SAP_NSS_CHAINS_SHIFT               3
#define P2P_GO_NSS_CHAINS_SHIFT            6
#define P2P_CLI_CHAINS_SHIFT               9
#define TDLS_NSS_CHAINS_SHIFT              12
#define IBSS_NSS_CHAINS_SHIFT              15
#define P2P_DEV_NSS_CHAINS_SHIFT           18
#define OCB_NSS_CHAINS_SHIFT               21
#define NAN_NSS_CHAIN_SHIFT                24
#define NSS_CHAIN_MASK                     0x7
#define GET_VDEV_NSS_CHAIN(x, y)         (((x) >> (y)) & NSS_CHAIN_MASK)

static uint8_t sme_get_nss_chain_shift(enum QDF_OPMODE device_mode)
{
	switch (device_mode) {
	case QDF_STA_MODE:
		return STA_NSS_CHAINS_SHIFT;
	case QDF_SAP_MODE:
		return SAP_NSS_CHAINS_SHIFT;
	case QDF_P2P_GO_MODE:
		return P2P_GO_NSS_CHAINS_SHIFT;
	case QDF_P2P_CLIENT_MODE:
		return P2P_CLI_CHAINS_SHIFT;
	case QDF_IBSS_MODE:
		return IBSS_NSS_CHAINS_SHIFT;
	case QDF_P2P_DEVICE_MODE:
		return P2P_DEV_NSS_CHAINS_SHIFT;
	case QDF_OCB_MODE:
		return OCB_NSS_CHAINS_SHIFT;
	case QDF_TDLS_MODE:
		return TDLS_NSS_CHAINS_SHIFT;

	default:
		sme_err("Device mode %d invalid", device_mode);
		return STA_NSS_CHAINS_SHIFT;
	}
}

static void
sme_check_nss_chain_ini_param(struct wlan_mlme_nss_chains *vdev_ini_cfg,
			      uint8_t rf_chains_supported,
			      enum nss_chains_band_info band)
{
	vdev_ini_cfg->rx_nss[band] = QDF_MIN(vdev_ini_cfg->rx_nss[band],
					     rf_chains_supported);
	vdev_ini_cfg->tx_nss[band] = QDF_MIN(vdev_ini_cfg->tx_nss[band],
					     rf_chains_supported);
}

static void
sme_fill_nss_chain_params(struct mac_context *mac_ctx,
			  struct wlan_mlme_nss_chains *vdev_ini_cfg,
			  enum QDF_OPMODE device_mode,
			  enum nss_chains_band_info band,
			  uint8_t rf_chains_supported)
{
	uint8_t nss_chain_shift;
	uint8_t max_supported_nss;
	struct wlan_mlme_nss_chains *nss_chains_ini_cfg =
					&mac_ctx->mlme_cfg->nss_chains_ini_cfg;

	nss_chain_shift = sme_get_nss_chain_shift(device_mode);
	max_supported_nss = mac_ctx->mlme_cfg->vht_caps.vht_cap_info.enable2x2 ?
			    MAX_VDEV_NSS : 1;

	/*
	 * If target supports Antenna sharing, set NSS to 1 for 2.4GHz band for
	 * NDI vdev.
	 */
	if (device_mode == QDF_NDI_MODE && mac_ctx->lteCoexAntShare &&
	    band == NSS_CHAINS_BAND_2GHZ)
		max_supported_nss = NSS_1x1_MODE;

	/* If the fw doesn't support two chains, num rf chains can max be 1 */
	vdev_ini_cfg->num_rx_chains[band] =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->num_rx_chains[band],
				nss_chain_shift), rf_chains_supported);

	vdev_ini_cfg->num_tx_chains[band] =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->num_tx_chains[band],
				nss_chain_shift), rf_chains_supported);

	/* If 2x2 mode is disabled, then max rx, tx nss can be 1 */
	vdev_ini_cfg->rx_nss[band] =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->rx_nss[band],
				nss_chain_shift), max_supported_nss);

	vdev_ini_cfg->tx_nss[band] =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->tx_nss[band],
				nss_chain_shift), max_supported_nss);

	vdev_ini_cfg->num_tx_chains_11a =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->num_tx_chains_11a,
				nss_chain_shift), rf_chains_supported);

	/* If the fw doesn't support two chains, num rf chains can max be 1 */
	vdev_ini_cfg->num_tx_chains_11b =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->num_tx_chains_11b,
				nss_chain_shift), rf_chains_supported);

	vdev_ini_cfg->num_tx_chains_11g =
		QDF_MIN(GET_VDEV_NSS_CHAIN(
				nss_chains_ini_cfg->num_tx_chains_11g,
				nss_chain_shift), rf_chains_supported);

	vdev_ini_cfg->disable_rx_mrc[band] =
				nss_chains_ini_cfg->disable_rx_mrc[band];

	vdev_ini_cfg->disable_tx_mrc[band] =
				nss_chains_ini_cfg->disable_tx_mrc[band];
	/*
	 * Check whether the rx/tx nss is greater than the number of rf chains
	 * supported by FW, if so downgrade the nss to the number of chains
	 * supported, as higher nss cannot be supported with less chains.
	 */
	sme_check_nss_chain_ini_param(vdev_ini_cfg, rf_chains_supported,
				      band);

}

void sme_populate_nss_chain_params(mac_handle_t mac_handle,
				   struct wlan_mlme_nss_chains *vdev_ini_cfg,
				   enum QDF_OPMODE device_mode,
				   uint8_t rf_chains_supported)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	enum nss_chains_band_info band;

	for (band = NSS_CHAINS_BAND_2GHZ; band < NSS_CHAINS_BAND_MAX; band++)
		sme_fill_nss_chain_params(mac_ctx, vdev_ini_cfg,
					  device_mode, band,
					  rf_chains_supported);
}

void
sme_store_nss_chains_cfg_in_vdev(struct wlan_objmgr_vdev *vdev,
				 struct wlan_mlme_nss_chains *vdev_ini_cfg)
{
	struct wlan_mlme_nss_chains *ini_cfg;
	struct wlan_mlme_nss_chains *dynamic_cfg;

	ini_cfg = mlme_get_ini_vdev_config(vdev);
	dynamic_cfg = mlme_get_dynamic_vdev_config(vdev);

	if (!ini_cfg || !dynamic_cfg) {
		sme_err("Nss chains ini/dynamic config NULL vdev_id %d",
					     vdev->vdev_objmgr.vdev_id);
		return;
	}

	*ini_cfg = *vdev_ini_cfg;
	*dynamic_cfg = *vdev_ini_cfg;
}

static void
sme_populate_user_config(struct wlan_mlme_nss_chains *dynamic_cfg,
			 struct wlan_mlme_nss_chains *user_cfg,
			 enum nss_chains_band_info band)
{
	if (!user_cfg->num_rx_chains[band])
		user_cfg->num_rx_chains[band] =
			dynamic_cfg->num_rx_chains[band];

	if (!user_cfg->num_tx_chains[band])
		user_cfg->num_tx_chains[band] =
			dynamic_cfg->num_tx_chains[band];

	if (!user_cfg->rx_nss[band])
		user_cfg->rx_nss[band] =
			dynamic_cfg->rx_nss[band];

	if (!user_cfg->tx_nss[band])
		user_cfg->tx_nss[band] =
			dynamic_cfg->tx_nss[band];

	if (!user_cfg->num_tx_chains_11a)
		user_cfg->num_tx_chains_11a =
			dynamic_cfg->num_tx_chains_11a;

	if (!user_cfg->num_tx_chains_11b)
		user_cfg->num_tx_chains_11b =
			dynamic_cfg->num_tx_chains_11b;

	if (!user_cfg->num_tx_chains_11g)
		user_cfg->num_tx_chains_11g =
			dynamic_cfg->num_tx_chains_11g;

	if (!user_cfg->disable_rx_mrc[band])
		user_cfg->disable_rx_mrc[band] =
			dynamic_cfg->disable_rx_mrc[band];

	if (!user_cfg->disable_tx_mrc[band])
		user_cfg->disable_tx_mrc[band] =
			dynamic_cfg->disable_tx_mrc[band];
}

static QDF_STATUS
sme_validate_from_ini_config(struct wlan_mlme_nss_chains *user_cfg,
			     struct wlan_mlme_nss_chains *ini_cfg,
			     enum nss_chains_band_info band)
{
	if (user_cfg->num_rx_chains[band] >
	    ini_cfg->num_rx_chains[band])
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->num_tx_chains[band] >
	    ini_cfg->num_tx_chains[band])
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->rx_nss[band] >
	    ini_cfg->rx_nss[band])
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->tx_nss[band] >
	    ini_cfg->tx_nss[band])
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->num_tx_chains_11a >
	    ini_cfg->num_tx_chains_11a)
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->num_tx_chains_11b >
	    ini_cfg->num_tx_chains_11b)
		return QDF_STATUS_E_FAILURE;

	if (user_cfg->num_tx_chains_11g >
	    ini_cfg->num_tx_chains_11g)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
sme_validate_user_nss_chain_params(
				    struct wlan_mlme_nss_chains *user_cfg,
				    enum nss_chains_band_info band)
{
	/* Reject as 2x1 modes are not supported in chains yet */

	if (user_cfg->num_tx_chains[band] >
	    user_cfg->num_rx_chains[band])
		return QDF_STATUS_E_FAILURE;

	/* Also if mode is 2x2, we cant have chains as 1x1, or 1x2, or 2x1 */

	if (user_cfg->tx_nss[band] >
	    user_cfg->num_tx_chains[band])
		user_cfg->num_tx_chains[band] =
			user_cfg->tx_nss[band];

	if (user_cfg->rx_nss[band] >
	    user_cfg->num_rx_chains[band])
		user_cfg->num_rx_chains[band] =
			user_cfg->rx_nss[band];

	/*
	 * It may happen that already chains are in 1x1 mode and nss too
	 * is in 1x1 mode, but the tx 11a/b/g chains in user config comes
	 * as 2x1, or 1x2 which cannot support respective mode, as tx chains
	 * for respective band have max of 1x1 only, so these cannot exceed
	 * respective band num tx chains.
	 */

	if (user_cfg->num_tx_chains_11a >
	    user_cfg->num_tx_chains[NSS_CHAINS_BAND_5GHZ])
		user_cfg->num_tx_chains_11a =
			user_cfg->num_tx_chains[NSS_CHAINS_BAND_5GHZ];

	if (user_cfg->num_tx_chains_11b >
	    user_cfg->num_tx_chains[NSS_CHAINS_BAND_2GHZ])
		user_cfg->num_tx_chains_11b =
			user_cfg->num_tx_chains[NSS_CHAINS_BAND_2GHZ];

	if (user_cfg->num_tx_chains_11g >
	    user_cfg->num_tx_chains[NSS_CHAINS_BAND_2GHZ])
		user_cfg->num_tx_chains_11g =
			user_cfg->num_tx_chains[NSS_CHAINS_BAND_2GHZ];

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
sme_validate_nss_chains_config(struct wlan_objmgr_vdev *vdev,
			       struct wlan_mlme_nss_chains *user_cfg,
			       struct wlan_mlme_nss_chains *dynamic_cfg)
{
	enum nss_chains_band_info band;
	struct wlan_mlme_nss_chains *ini_cfg;
	QDF_STATUS status;

	ini_cfg = mlme_get_ini_vdev_config(vdev);
	if (!ini_cfg) {
		sme_err("nss chain ini config NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (band = NSS_CHAINS_BAND_2GHZ; band < NSS_CHAINS_BAND_MAX; band++) {
		sme_populate_user_config(dynamic_cfg,
					 user_cfg, band);
		status = sme_validate_from_ini_config(user_cfg,
						      ini_cfg,
						      band);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("Validation from ini config failed");
			return QDF_STATUS_E_FAILURE;
		}
		status = sme_validate_user_nss_chain_params(user_cfg,
							    band);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("User cfg validation failed");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static bool
sme_is_nss_update_allowed(struct wlan_mlme_chain_cfg chain_cfg,
			  uint8_t rx_nss, uint8_t tx_nss,
			  enum nss_chains_band_info band)
{
	switch (band) {
	case NSS_CHAINS_BAND_2GHZ:
		if (rx_nss > chain_cfg.max_rx_chains_2g)
			return false;
		if (tx_nss > chain_cfg.max_tx_chains_2g)
			return false;
		break;
	case NSS_CHAINS_BAND_5GHZ:
		if (rx_nss > chain_cfg.max_rx_chains_5g)
			return false;
		if (tx_nss > chain_cfg.max_tx_chains_5g)
			return false;
		break;
	default:
		sme_err("Unknown Band nss change not allowed");
		return false;
	}
	return true;
}

static void sme_modify_chains_in_mlme_cfg(mac_handle_t mac_handle,
					  uint8_t rx_chains,
					  uint8_t tx_chains,
					  enum QDF_OPMODE vdev_op_mode,
					  enum nss_chains_band_info band)
{
	uint8_t nss_shift;
	uint32_t nss_mask = 0x7;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	nss_shift = sme_get_nss_chain_shift(vdev_op_mode);

	mac_ctx->mlme_cfg->nss_chains_ini_cfg.num_rx_chains[band] &=
						~(nss_mask << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.num_rx_chains[band] |=
						 (rx_chains << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.num_tx_chains[band] &=
						~(nss_mask << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.num_tx_chains[band] |=
						 (tx_chains << nss_shift);
	sme_debug("rx chains %d tx chains %d changed for vdev mode %d for band %d",
		  rx_chains, tx_chains, vdev_op_mode, band);
}

static void
sme_modify_nss_in_mlme_cfg(mac_handle_t mac_handle,
			   uint8_t rx_nss, uint8_t tx_nss,
			   enum QDF_OPMODE vdev_op_mode,
			   enum nss_chains_band_info band)
{
	uint8_t nss_shift;
	uint32_t nss_mask = 0x7;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	if (!sme_is_nss_update_allowed(mac_ctx->fw_chain_cfg, rx_nss, tx_nss,
				       band)) {
		sme_debug("Nss modification failed, fw doesn't support this nss %d",
			  rx_nss);
		return;
	}

	nss_shift = sme_get_nss_chain_shift(vdev_op_mode);

	mac_ctx->mlme_cfg->nss_chains_ini_cfg.rx_nss[band] &=
						~(nss_mask << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.rx_nss[band] |=
						 (rx_nss << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.tx_nss[band] &=
						~(nss_mask << nss_shift);
	mac_ctx->mlme_cfg->nss_chains_ini_cfg.tx_nss[band] |=
						 (tx_nss << nss_shift);
	sme_debug("rx nss %d tx nss %d changed for vdev mode %d for band %d",
		  rx_nss, tx_nss, vdev_op_mode, band);
}

void
sme_modify_nss_chains_tgt_cfg(mac_handle_t mac_handle,
			      enum QDF_OPMODE vdev_op_mode,
			      enum nss_chains_band_info band)
{
	uint8_t ini_rx_nss;
	uint8_t ini_tx_nss;
	uint8_t max_supported_rx_nss = MAX_VDEV_NSS;
	uint8_t max_supported_tx_nss = MAX_VDEV_NSS;
	uint8_t ini_rx_chains;
	uint8_t ini_tx_chains;
	uint8_t max_supported_rx_chains = MAX_VDEV_CHAINS;
	uint8_t max_supported_tx_chains = MAX_VDEV_CHAINS;

	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct wlan_mlme_nss_chains *nss_chains_ini_cfg =
					&mac_ctx->mlme_cfg->nss_chains_ini_cfg;
	uint8_t nss_shift = sme_get_nss_chain_shift(vdev_op_mode);
	struct wlan_mlme_chain_cfg chain_cfg = mac_ctx->fw_chain_cfg;

	ini_rx_nss = GET_VDEV_NSS_CHAIN(nss_chains_ini_cfg->rx_nss[band],
					nss_shift);
	ini_tx_nss = GET_VDEV_NSS_CHAIN(nss_chains_ini_cfg->tx_nss[band],
					nss_shift);

	if (band == NSS_CHAINS_BAND_2GHZ) {
		max_supported_rx_nss = chain_cfg.max_rx_chains_2g;
		max_supported_tx_nss = chain_cfg.max_tx_chains_2g;
	} else if (band == NSS_CHAINS_BAND_5GHZ) {
		max_supported_rx_nss = chain_cfg.max_rx_chains_5g;
		max_supported_tx_nss = chain_cfg.max_tx_chains_5g;
	}

	max_supported_rx_nss = QDF_MIN(ini_rx_nss, max_supported_rx_nss);
	max_supported_tx_nss = QDF_MIN(ini_tx_nss, max_supported_tx_nss);

	ini_rx_chains = GET_VDEV_NSS_CHAIN(nss_chains_ini_cfg->
						num_rx_chains[band],
					   nss_shift);
	ini_tx_chains = GET_VDEV_NSS_CHAIN(nss_chains_ini_cfg->
						num_tx_chains[band],
					   nss_shift);

	if (band == NSS_CHAINS_BAND_2GHZ) {
		max_supported_rx_chains = chain_cfg.max_rx_chains_2g;
		max_supported_tx_chains = chain_cfg.max_tx_chains_2g;
	} else if (band == NSS_CHAINS_BAND_5GHZ) {
		max_supported_rx_chains = chain_cfg.max_rx_chains_5g;
		max_supported_tx_chains = chain_cfg.max_tx_chains_5g;
	}

	max_supported_rx_chains = QDF_MIN(ini_rx_chains,
					  max_supported_rx_chains);
	max_supported_tx_chains = QDF_MIN(ini_tx_chains,
					  max_supported_tx_chains);

	sme_modify_chains_in_mlme_cfg(mac_handle, max_supported_rx_chains,
				      max_supported_tx_chains, vdev_op_mode,
				      band);
	sme_modify_nss_in_mlme_cfg(mac_handle, max_supported_rx_nss,
				   max_supported_tx_nss, vdev_op_mode, band);
}

void
sme_update_nss_in_mlme_cfg(mac_handle_t mac_handle,
			   uint8_t rx_nss, uint8_t tx_nss,
			   enum QDF_OPMODE vdev_op_mode,
			   enum nss_chains_band_info band)
{
	/*
	 * If device mode is P2P-DEVICE, then we want P2P to come in that
	 * particular nss, then we should change the nss of P@P-CLI, and GO
	 * and we are unaware that for what will be the device mode after
	 * negotiation yet.
	 */

	if (vdev_op_mode == QDF_P2P_DEVICE_MODE ||
	    vdev_op_mode == QDF_P2P_CLIENT_MODE ||
	    vdev_op_mode == QDF_P2P_GO_MODE) {
		sme_modify_nss_in_mlme_cfg(mac_handle, rx_nss, tx_nss,
					   QDF_P2P_CLIENT_MODE, band);
		sme_modify_nss_in_mlme_cfg(mac_handle, rx_nss, tx_nss,
					   QDF_P2P_GO_MODE, band);
		sme_modify_nss_in_mlme_cfg(mac_handle, rx_nss, tx_nss,
					   QDF_P2P_DEVICE_MODE, band);
	} else
		sme_modify_nss_in_mlme_cfg(mac_handle, rx_nss, tx_nss,
					   vdev_op_mode, band);
}

QDF_STATUS
sme_nss_chains_update(mac_handle_t mac_handle,
		      struct wlan_mlme_nss_chains *user_cfg,
		      uint8_t vdev_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;
	struct wlan_mlme_nss_chains *dynamic_cfg;
	struct wlan_objmgr_vdev *vdev =
		       wlan_objmgr_get_vdev_by_id_from_psoc(mac_ctx->psoc,
							    vdev_id,
							    WLAN_LEGACY_SME_ID);
	if (!vdev) {
		sme_err("Got NULL vdev obj, returning");
		return QDF_STATUS_E_FAILURE;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_ref;

	dynamic_cfg = mlme_get_dynamic_vdev_config(vdev);
	if (!dynamic_cfg) {
		sme_err("nss chain dynamic config NULL");
		status = QDF_STATUS_E_FAILURE;
		goto release_lock;
	}

	status = sme_validate_nss_chains_config(vdev, user_cfg,
						dynamic_cfg);
	if (QDF_IS_STATUS_ERROR(status))
		goto release_lock;

	if (!qdf_mem_cmp(dynamic_cfg, user_cfg,
			 sizeof(struct wlan_mlme_nss_chains))) {
		sme_debug("current config same as user config");
		status = QDF_STATUS_SUCCESS;
		goto release_lock;
	}
	sme_debug("User params verified, sending to fw vdev id %d", vdev_id);

	status = wma_vdev_nss_chain_params_send(vdev->vdev_objmgr.vdev_id,
						user_cfg);
	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("params sent failed to fw vdev id %d", vdev_id);
		goto release_lock;
	}

	*dynamic_cfg = *user_cfg;

release_lock:
	sme_release_global_lock(&mac_ctx->sme);

release_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
	return status;
}

QDF_STATUS sme_open_session(mac_handle_t mac_handle,
			    struct sme_session_params *params)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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

	if (!pdev) {
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

QDF_STATUS sme_close_session(mac_handle_t mac_handle, uint8_t session_id)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CLOSE_SESSION, session_id, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_close_session(mac, session_id, false);
		sme_release_global_lock(&mac->sme);
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
	struct mac_context *mac_ctx = sme_get_mac_context();

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
 * @mac_handle: The handle returned by mac_open.
 * @sessionId: Session Identifier
 * @request: Pointer to the offload request.
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_host_offload(mac_handle_t mac_handle, uint8_t sessionId,
				struct sir_host_offload_req *request)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_HOSTOFFLOAD, sessionId, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
#ifdef WLAN_NS_OFFLOAD
		if (SIR_IPV6_NS_OFFLOAD == request->offloadType) {
			status = sme_set_ps_ns_offload(mac_handle, request,
					sessionId);
		} else
#endif /* WLAN_NS_OFFLOAD */
		{
			status = sme_set_ps_host_offload(mac_handle, request,
					sessionId);
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_set_keep_alive() -
 * API to set the Keep Alive feature.
 *
 * mac_handle - The handle returned by mac_open.
 * request -  Pointer to the Keep Alive request.
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_keep_alive(mac_handle_t mac_handle, uint8_t session_id,
			      struct keep_alive_req *request)
{
	struct keep_alive_req *request_buf;
	struct scheduler_msg msg = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, session_id);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("WMA_SET_KEEP_ALIVE message"));

	if (!pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Session not Found"));
		return QDF_STATUS_E_FAILURE;
	}
	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (!request_buf)
		return QDF_STATUS_E_NOMEM;

	qdf_copy_macaddr(&request->bssid, &pSession->connectedProfile.bssid);
	qdf_mem_copy(request_buf, request, sizeof(*request_buf));

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
			scheduler_post_message(QDF_MODULE_ID_SME,
					       QDF_MODULE_ID_WMA,
					       QDF_MODULE_ID_WMA, &msg)) {
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
 * mac_handle, pointer to memory location and sessionId
 * Returns QDF_STATUS_SUCCESS
 *	     QDF_STATUS_E_FAILURE
 */
QDF_STATUS sme_get_operation_channel(mac_handle_t mac_handle,
				     uint32_t *pChannel,
				     uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession;

	if (CSR_IS_SESSION_VALID(mac, sessionId)) {
		pSession = CSR_GET_SESSION(mac, sessionId);

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
 * @mac_handle: Opaque handle to the global MAC context
 * @callback: callback pointer to be registered
 *
 * This function is used to register a callback for management
 * frame indication to PE.
 *
 * Return: Success if msg is posted to PE else Failure.
 */
QDF_STATUS sme_register_mgmt_frame_ind_callback(mac_handle_t mac_handle,
				sir_mgmt_frame_ind_callback callback)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct sir_sme_mgmt_frame_cb_req *msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (QDF_STATUS_SUCCESS ==
			sme_acquire_global_lock(&mac_ctx->sme)) {
		msg = qdf_mem_malloc(sizeof(*msg));
		if (!msg) {
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
 * To register management frame of specified type and subtype.
 *
 * frameType - type of the frame that needs to be passed to HDD.
 * matchData - data which needs to be matched before passing frame
 *		       to HDD.
 * matchDataLen - Length of matched data.
 * Return QDF_STATUS
 */
QDF_STATUS sme_register_mgmt_frame(mac_handle_t mac_handle, uint8_t sessionId,
				   uint16_t frameType, uint8_t *matchData,
				   uint16_t matchLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		struct register_mgmt_frame *pMsg;
		uint16_t len;
		struct csr_roam_session *pSession = CSR_GET_SESSION(mac,
							sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sme_err("Session %d not found",	sessionId);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) &&
						!pSession->sessionActive) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s Invalid Sessionid", __func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		len = sizeof(*pMsg) + matchLen;

		pMsg = qdf_mem_malloc(len);
		if (!pMsg)
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
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_DeregisterMgtFrame() -
 * To De-register management frame of specified type and subtype.
 *
 * frameType - type of the frame that needs to be passed to HDD.
 * matchData - data which needs to be matched before passing frame
 *		       to HDD.
 * matchDataLen - Length of matched data.
 * Return QDF_STATUS
 */
QDF_STATUS sme_deregister_mgmt_frame(mac_handle_t mac_handle, uint8_t sessionId,
				     uint16_t frameType, uint8_t *matchData,
				     uint16_t matchLen)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_DEREGISTER_MGMTFR, sessionId,
			 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		struct register_mgmt_frame *pMsg;
		uint16_t len;
		struct csr_roam_session *pSession = CSR_GET_SESSION(mac,
							sessionId);

		if (!CSR_IS_SESSION_ANY(sessionId) && !pSession) {
			sme_err("Session %d not found",	sessionId);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!CSR_IS_SESSION_ANY(sessionId) &&
						!pSession->sessionActive) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s Invalid Sessionid", __func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}

		len = sizeof(*pMsg) + matchLen;

		pMsg = qdf_mem_malloc(len);
		if (!pMsg)
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
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_prepare_mgmt_tx() - Prepares mgmt frame
 * @mac_handle: The handle returned by mac_open
 * @session_id: session id
 * @buf: pointer to frame
 * @len: frame length
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_prepare_mgmt_tx(mac_handle_t mac_handle,
				      uint8_t session_id,
				      const uint8_t *buf, uint32_t len)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sir_mgmt_msg *msg;
	uint16_t msg_len;
	struct scheduler_msg sch_msg = {0};

	sme_debug("prepares auth frame");

	msg_len = sizeof(*msg) + len;
	msg = qdf_mem_malloc(msg_len);
	if (!msg) {
		status = QDF_STATUS_E_NOMEM;
	} else {
		msg->type = eWNI_SME_SEND_MGMT_FRAME_TX;
		msg->msg_len = msg_len;
		msg->session_id = session_id;
		msg->data = (uint8_t *)msg + sizeof(*msg);
		qdf_mem_copy(msg->data, buf, len);

		sch_msg.type = eWNI_SME_SEND_MGMT_FRAME_TX;
		sch_msg.bodyptr = msg;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_PE,
						QDF_MODULE_ID_PE, &sch_msg);
	}
	return status;
}

QDF_STATUS sme_send_mgmt_tx(mac_handle_t mac_handle, uint8_t session_id,
			    const uint8_t *buf, uint32_t len)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = sme_prepare_mgmt_tx(mac_handle, session_id, buf, len);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/**
 * sme_configure_ext_wow() - configure Extr WoW
 * @mac_handle - The handle returned by mac_open.
 * @wlanExtParams - Depicts the wlan Ext params.
 * @callback - ext_wow callback to be registered.
 * @callback_context - ext_wow callback context
 *
 * SME will pass this request to lower mac to configure Extr WoW
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_ext_wow(mac_handle_t mac_handle,
				 tpSirExtWoWParams wlanExtParams,
				 csr_readyToExtWoWCallback callback,
				 void *callback_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tpSirExtWoWParams MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_EXTWOW, NO_SESSION, 0));

	mac->readyToExtWoWCallback = callback;
	mac->readyToExtWoWContext = callback_context;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanExtParams, sizeof(*MsgPtr));
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_EXT_WOW;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			mac->readyToExtWoWCallback = NULL;
			mac->readyToExtWoWContext = NULL;
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		mac->readyToExtWoWCallback = NULL;
		mac->readyToExtWoWContext = NULL;
		qdf_mem_free(MsgPtr);
	}

	return status;
}

/*
 * sme_configure_app_type1_params() -
 * SME will pass this request to lower mac to configure Indoor WoW parameters.
 *
 * mac_handle - The handle returned by mac_open.
 * wlanAppType1Params- Depicts the wlan App Type 1(Indoor) params
 * Return QDF_STATUS
 */
QDF_STATUS sme_configure_app_type1_params(mac_handle_t mac_handle,
					  tpSirAppType1Params wlanAppType1Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tpSirAppType1Params MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_APP_TYPE1, NO_SESSION,
			 0));

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanAppType1Params, sizeof(*MsgPtr));
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_SET_APP_TYPE1_PARAMS;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		qdf_mem_free(MsgPtr);
	}

	return status;
}

/*
 * sme_configure_app_type2_params() -
 * SME will pass this request to lower mac to configure Indoor WoW parameters.
 *
 * mac_handle - The handle returned by mac_open.
 * wlanAppType2Params- Depicts the wlan App Type 2 (Outdoor) params
 * Return QDF_STATUS
 */
QDF_STATUS sme_configure_app_type2_params(mac_handle_t mac_handle,
					 tpSirAppType2Params wlanAppType2Params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tpSirAppType2Params MsgPtr = qdf_mem_malloc(sizeof(*MsgPtr));

	if (!MsgPtr)
		return QDF_STATUS_E_NOMEM;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_CONFIG_APP_TYPE2, NO_SESSION,
			 0));

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		qdf_mem_copy(MsgPtr, wlanAppType2Params, sizeof(*MsgPtr));
		message.bodyptr = MsgPtr;
		message.type = WMA_WLAN_SET_APP_TYPE2_PARAMS;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(MsgPtr);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		qdf_mem_free(MsgPtr);
	}

	return status;
}
#endif

uint8_t sme_get_beaconing_concurrent_operation_channel(mac_handle_t mac_handle,
						       uint8_t vdev_id_to_skip)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	uint8_t channel = 0;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		channel = csr_get_beaconing_concurrent_channel(mac,
							       vdev_id_to_skip);
		sme_info("Other Concurrent Channel: %d skipped vdev_id %d",
			 channel, vdev_id_to_skip);
		sme_release_global_lock(&mac->sme);
	}

	return channel;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t sme_check_concurrent_channel_overlap(mac_handle_t mac_handle,
					      uint16_t sap_ch,
					      eCsrPhyMode sapPhyMode,
					      uint8_t cc_switch_mode)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	uint16_t channel = 0;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		channel =
			csr_check_concurrent_channel_overlap(mac, sap_ch,
								sapPhyMode,
							     cc_switch_mode);
		sme_release_global_lock(&mac->sme);
	}

	return channel;
}
#endif

/**
 * sme_set_tsfcb() - Set callback for TSF capture
 * @mac_handle: Handler return by mac_open
 * @cb_fn: Callback function pointer
 * @db_ctx: Callback data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_tsfcb(mac_handle_t mac_handle,
	int (*cb_fn)(void *cb_ctx, struct stsf *ptsf), void *cb_ctx)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
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
 * @mac_handle: Handler return by mac_open
 *
 * This function reset the tsf capture callback to SME
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_reset_tsfcb(mac_handle_t mac_handle)
{
	struct mac_context *mac;
	QDF_STATUS status;

	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("mac_handle is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.get_tsf_cb = NULL;
		mac->sme.get_tsf_cxt = NULL;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#if defined(WLAN_FEATURE_TSF) && !defined(WLAN_FEATURE_TSF_PLUS_NOIRQ)
/*
 * sme_set_tsf_gpio() - set gpio pin that be toggled when capture tsf
 * @mac_handle: Handler return by mac_open
 * @pinvalue: gpio pin id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_tsf_gpio(mac_handle_t mac_handle, uint32_t pinvalue)
{
	QDF_STATUS status;
	struct scheduler_msg tsf_msg = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		tsf_msg.type = WMA_TSF_GPIO_PIN;
		tsf_msg.reserved = 0;
		tsf_msg.bodyval = pinvalue;

		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &tsf_msg);
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
	struct mac_context *mac_ctx = sme_get_mac_context();

	if (!mac_ctx) {
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

void sme_set_cc_src(mac_handle_t mac_handle, enum country_src cc_src)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->reg_hint_src = cc_src;

	sme_debug("Country source is %s",
		  sme_reg_hint_to_str(cc_src));
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
sme_handle_generic_change_country_code(struct mac_context *mac_ctx,
				       void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	v_REGDOMAIN_t reg_domain_id = 0;
	bool user_ctry_priority =
		mac_ctx->mlme_cfg->sap_cfg.country_code_priority;
	tAniGenericChangeCountryCodeReq *msg = pMsgBuf;

	if (SOURCE_11D != mac_ctx->reg_hint_src) {
		if (SOURCE_DRIVER != mac_ctx->reg_hint_src) {
			if (user_ctry_priority)
				mac_ctx->mlme_cfg->gen.enabled_11d = false;
			else {
				if (mac_ctx->mlme_cfg->gen.enabled_11d &&
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
				     CFG_COUNTRY_CODE_LEN);
	}

	qdf_mem_copy(mac_ctx->scan.countryCodeCurrent,
		     msg->countryCode,
		     CFG_COUNTRY_CODE_LEN);

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

QDF_STATUS sme_update_channel_list(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;

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
sme_search_in_base_ch_lst(struct mac_context *mac_ctx, uint8_t curr_ch)
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
static void sme_disconnect_connected_sessions(struct mac_context *mac_ctx)
{
	uint8_t session_id, found = false;
	uint8_t curr_ch;

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
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
QDF_STATUS sme_8023_multicast_list(mac_handle_t mac_handle, uint8_t sessionId,
				   tpSirRcvFltMcAddrList pMulticastAddrs)
{
	tpSirRcvFltMcAddrList request_buf;
	struct scheduler_msg msg = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = NULL;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		"%s: ulMulticastAddrCnt: %d, multicastAddr[0]: %pK", __func__,
		  pMulticastAddrs->ulMulticastAddrCnt,
		  pMulticastAddrs->multicastAddr[0].bytes);

	/* Find the connected Infra / P2P_client connected session */
	pSession = CSR_GET_SESSION(mac, sessionId);
	if (!CSR_IS_SESSION_VALID(mac, sessionId) ||
			(!csr_is_conn_state_infra(mac, sessionId) &&
			 !csr_is_ndi_started(mac, sessionId))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Unable to find the session Id: %d", __func__,
			  sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	request_buf = qdf_mem_malloc(sizeof(tSirRcvFltMcAddrList));
	if (!request_buf)
		return QDF_STATUS_E_NOMEM;

	if (!csr_is_conn_state_connected_infra(mac, sessionId) &&
			!csr_is_ndi_started(mac, sessionId)) {
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
	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
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
 * mac_handle - The handle returned by mac_open.
 * channel - channel to verify
 * Return true/false, true if channel is valid
 */
bool sme_is_channel_valid(mac_handle_t mac_handle, uint8_t channel)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	bool valid = false;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		valid = csr_roam_is_channel_valid(mac, channel);

		sme_release_global_lock(&mac->sme);
	}

	return valid;
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
	if (!pMaxTxPowerPerBandParams)
		return QDF_STATUS_E_NOMEM;

	pMaxTxPowerPerBandParams->power = dB;
	pMaxTxPowerPerBandParams->bandInfo = band;

	msg.type = WMA_SET_MAX_TX_POWER_PER_BAND_REQ;
	msg.reserved = 0;
	msg.bodyptr = pMaxTxPowerPerBandParams;
	MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			 NO_SESSION, msg.type));
	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
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
 * mac_handle
 * pBssid  BSSID to set the power cap for
 * pBssid  pSelfMacAddress self MAC Address
 * pBssid  power to set in dB
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_max_tx_power(mac_handle_t mac_handle,
				struct qdf_mac_addr pBssid,
				struct qdf_mac_addr pSelfMacAddress, int8_t dB)
{
	struct scheduler_msg msg = {0};
	tpMaxTxPowerParams pMaxTxParams = NULL;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_MAXTXPOW, NO_SESSION, 0));
	pMaxTxParams = qdf_mem_malloc(sizeof(tMaxTxPowerParams));
	if (!pMaxTxParams)
		return QDF_STATUS_E_NOMEM;

	qdf_copy_macaddr(&pMaxTxParams->bssId, &pBssid);
	qdf_copy_macaddr(&pMaxTxParams->selfStaMacAddr, &pSelfMacAddress);
	pMaxTxParams->power = dB;

	msg.type = WMA_SET_MAX_TX_POWER_REQ;
	msg.reserved = 0;
	msg.bodyptr = pMaxTxParams;

	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
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
	if (!pBaseMacAddr)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(*pBaseMacAddr, customMacAddr, sizeof(tSirMacAddr));

	msg.type = SIR_HAL_SET_BASE_MACADDR_IND;
	msg.reserved = 0;
	msg.bodyptr = pBaseMacAddr;

	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
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
 * mac_handle
 * sessionId  Target Session ID
 * BSSID
 * dev_mode dev_mode such as station, P2PGO, SAP
 * dBm  power to set
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_tx_power(mac_handle_t mac_handle, uint8_t sessionId,
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
	if (!pTxParams)
		return QDF_STATUS_E_NOMEM;

	qdf_copy_macaddr(&pTxParams->bssId, &pBSSId);
	pTxParams->power = power;       /* unit is dBm */
	pTxParams->dev_mode = dev_mode;
	msg.type = WMA_SET_TX_POWER_REQ;
	msg.reserved = 0;
	msg.bodyptr = pTxParams;

	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
							 &msg)) {
		qdf_mem_free(pTxParams);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_update_session_param(mac_handle_t mac_handle, uint8_t session_id,
				    uint32_t param_type, uint32_t param_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
 * sme_update_roam_scan_n_probes() -
 * Function to update roam scan N probes
 *	    This function is called through dynamic setConfig callback function
 *	    to update roam scan N probes
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nProbes number of probe requests to be sent out
 * Return Success or failure
 */
QDF_STATUS sme_update_roam_scan_n_probes(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nProbes)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_ROAM_SCAN_N_PROBES,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: gRoamScanNProbes is changed from %d to %d",
			  __func__, mac->mlme_cfg->lfr.roam_scan_n_probes,
			  nProbes);
		mac->mlme_cfg->lfr.roam_scan_n_probes = nProbes;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_NPROBES_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_update_roam_scan_home_away_time() -
 *  Function to update roam scan Home away time
 *	    This function is called through dynamic setConfig callback function
 *	    to update roam scan home away time
 *
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nRoamScanAwayTime Scan home away time
 * bSendOffloadCmd If true then send offload command to firmware
 *			    If false then command is not sent to firmware
 * Return Success or failure
 */
QDF_STATUS sme_update_roam_scan_home_away_time(
					mac_handle_t mac_handle,
					uint8_t sessionId,
					const uint16_t nRoamScanHomeAwayTime,
					const bool bSendOffloadCmd)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_UPDATE_ROAM_SCAN_HOME_AWAY_TIME,
			 NO_SESSION, 0));
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: gRoamScanHomeAwayTime is changed from %d to %d",
			  __func__,
			  mac->mlme_cfg->lfr.roam_scan_home_away_time,
			  nRoamScanHomeAwayTime);
		mac->mlme_cfg->lfr.roam_scan_home_away_time =
			nRoamScanHomeAwayTime;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled &&
		    bSendOffloadCmd) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_HOME_AWAY_TIME_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_ext_change_channel()- function to post send ECSA
 * action frame to csr.
 * @mac_handle: Opaque handle to the global MAC context
 * @channel: new channel to switch
 * @session_id: senssion it should be sent on.
 *
 * This function is called to post ECSA frame to csr.
 *
 * Return: success if msg is sent else return failure
 */
QDF_STATUS sme_ext_change_channel(mac_handle_t mac_handle, uint32_t channel,
						uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx  = MAC_CONTEXT(mac_handle);
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
 * mac_handle: Opaque handle to the global MAC context
 * Return Success or failure
 */
bool sme_get_roam_intra_band(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_GET_ROAMIBAND, NO_SESSION, 0));

	return mac->mlme_cfg->lfr.roam_intra_band;
}

/*
 * sme_get_roam_scan_n_probes() -
 * get N Probes
 *
 * mac_handle: Opaque handle to the global MAC context
 * Return Success or failure
 */
uint8_t sme_get_roam_scan_n_probes(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.roam_scan_n_probes;
}

/*
 * sme_get_roam_scan_home_away_time() -
 * get Roam scan home away time
 *
 * mac_handle: Opaque handle to the global MAC context
 * Return Success or failure
 */
uint16_t sme_get_roam_scan_home_away_time(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.roam_scan_home_away_time;
}

/*
 * sme_update_roam_rssi_diff() -
 * Update RoamRssiDiff
 *	    This function is called through dynamic setConfig callback function
 *	    to configure RoamRssiDiff
 *	    Usage: adb shell iwpriv wlan0 setConfig RoamRssiDiff=[0 .. 125]
 *
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * RoamRssiDiff - minimum rssi difference between potential
 *	    candidate and current AP.
 * Return Success or failure
 */

QDF_STATUS sme_update_roam_rssi_diff(mac_handle_t mac_handle, uint8_t sessionId,
				     uint8_t RoamRssiDiff)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam rssi diff to %d - old value is %d - roam state is %s",
			  RoamRssiDiff,
			  mac->mlme_cfg->lfr.roam_rssi_diff,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.roam_rssi_diff = RoamRssiDiff;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled)
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_RSSI_DIFF_CHANGED);

		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS sme_update_fils_config(mac_handle_t mac_handle, uint8_t session_id,
				  struct csr_roam_profile *src_profile)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
			&mac->roam.neighborRoamInfo[session_id];

	if (session_id >= WLAN_MAX_VDEVS) {
		sme_err("Invalid sme session id: %d", session_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!src_profile) {
		sme_err("src roam profile NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!mac->mlme_cfg->lfr.lfr_enabled ||
	    (neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)) {
		sme_info("Fast roam is disabled or not connected(%d)",
				neighbor_roam_info->neighborRoamState);
		return QDF_STATUS_E_PERM;
	}

	csr_update_fils_config(mac, session_id, src_profile);
	if (csr_is_roam_offload_enabled(mac)) {
		status = sme_acquire_global_lock(&mac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			sme_debug("Updating fils config to fw");
			csr_roam_offload_scan(mac, session_id,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_FILS_PARAMS_CHANGED);
			sme_release_global_lock(&mac->sme);
		} else {
			sme_err("Failed to acquire SME lock");
		}
	} else {
		sme_info("LFR3 not enabled");
		return QDF_STATUS_E_INVAL;
	}

	return status;
}

void sme_send_hlp_ie_info(mac_handle_t mac_handle, uint8_t session_id,
			  struct csr_roam_profile *profile, uint32_t if_addr)
{
	int i;
	struct scheduler_msg msg;
	QDF_STATUS status;
	struct hlp_params *params;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session = CSR_GET_SESSION(mac, session_id);
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
				&mac->roam.neighborRoamInfo[session_id];

	if (!session) {
		sme_err("session NULL");
		return;
	}

	if (!mac->mlme_cfg->lfr.lfr_enabled ||
	    (neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_CONNECTED)) {
		sme_debug("Fast roam is disabled or not connected(%d)",
				neighbor_roam_info->neighborRoamState);
		return;
	}

	params = qdf_mem_malloc(sizeof(*params));
	if (!params)
		return;

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
			(scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &msg))) {
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
inline void sme_send_hlp_ie_info(mac_handle_t mac_handle, uint8_t session_id,
			  struct csr_roam_profile *profile, uint32_t if_addr)
{}
#endif

/*
 * sme_update_wes_mode() -
 * Update WES Mode
 *	    This function is called through dynamic setConfig callback function
 *	    to configure isWESModeEnabled
 *
 * mac_handle: Opaque handle to the global MAC context
 * isWESModeEnabled - WES mode
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update isWESModeEnabled config successfully.
 *	    Other status means SME is failed to update isWESModeEnabled.
 */

QDF_STATUS sme_update_wes_mode(mac_handle_t mac_handle, bool isWESModeEnabled,
			       uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set WES Mode to %d - old value is %d - roam state is %s",
			  isWESModeEnabled,
			  mac->mlme_cfg->lfr.wes_mode_enabled,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.wes_mode_enabled = isWESModeEnabled;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_set_roam_scan_control() -
 * Set roam scan control
 *	    This function is called to set roam scan control
 *	    if roam scan control is set to 0, roaming scan cache is cleared
 *	    any value other than 0 is treated as invalid value
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successfully.
 *	    Other status means SME failure to update
 */
QDF_STATUS sme_set_roam_scan_control(mac_handle_t mac_handle, uint8_t sessionId,
				     bool roamScanControl)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	MTRACE(qdf_trace(QDF_MODULE_ID_SME,
			 TRACE_CODE_SME_RX_HDD_SET_SCANCTRL, NO_SESSION, 0));

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan control to %d - old value is %d - roam state is %s",
			  roamScanControl,
			  mac->roam.configParam.nRoamScanControl,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->roam.configParam.nRoamScanControl = roamScanControl;
		if (0 == roamScanControl) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "LFR runtime successfully cleared roam scan cache");
			csr_flush_cfg_bg_scan_roam_channel_list(mac,
								sessionId);
			if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
				csr_roam_offload_scan(mac, sessionId,
						   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
						     REASON_FLUSH_CHANNEL_LIST);
			}
		}
		sme_release_global_lock(&mac->sme);
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
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update isFastRoamIniFeatureEnabled config
 *	successfully.
 * Other status means SME is failed to update isFastRoamIniFeatureEnabled.
 */
QDF_STATUS sme_update_is_fast_roam_ini_feature_enabled(mac_handle_t mac_handle,
		uint8_t sessionId, const bool isFastRoamIniFeatureEnabled)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (mac->mlme_cfg->lfr.lfr_enabled ==
	    isFastRoamIniFeatureEnabled) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: FastRoam is already enabled or disabled, nothing to do (returning) old(%d) new(%d)",
			  __func__,
			  mac->mlme_cfg->lfr.lfr_enabled,
			  isFastRoamIniFeatureEnabled);
		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: FastRoamEnabled is changed from %d to %d", __func__,
		  mac->mlme_cfg->lfr.lfr_enabled,
		  isFastRoamIniFeatureEnabled);
	mac->mlme_cfg->lfr.lfr_enabled = isFastRoamIniFeatureEnabled;
	csr_neighbor_roam_update_fast_roaming_enabled(mac, sessionId,
						   isFastRoamIniFeatureEnabled);

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_config_fast_roaming() - enable/disable LFR support at runtime
 * @mac_handle - The handle returned by macOpen.
 * @session_id - Session Identifier
 * @is_fast_roam_enabled - flag to enable/disable roaming
 *
 * When Supplicant issues enabled/disable fast roaming on the basis
 * of the Bssid modification in network block (e.g. AutoJoin mode N/W block)
 *
 * Return: QDF_STATUS
 */

QDF_STATUS sme_config_fast_roaming(mac_handle_t mac_handle, uint8_t session_id,
				   const bool is_fast_roam_enabled)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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

	if (!mac_ctx->mlme_cfg->lfr.lfr_enabled) {
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

#ifdef FEATURE_WLAN_ESE
int sme_add_key_krk(mac_handle_t mac_handle, uint8_t session_id,
		    const uint8_t *key, const int key_len)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	if (key_len < SIR_KRK_KEY_LEN) {
		sme_warn("Invalid KRK keylength [= %d]", key_len);
		return -EINVAL;
	}

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("incorrect session/vdev ID");
		return -EINVAL;
	}

	session = CSR_GET_SESSION(mac_ctx, session_id);

	qdf_mem_copy(session->eseCckmInfo.krk, key, SIR_KRK_KEY_LEN);
	session->eseCckmInfo.reassoc_req_num = 1;
	session->eseCckmInfo.krk_plumbed = true;

	return 0;
}
#endif

#if defined(FEATURE_WLAN_ESE) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
int sme_add_key_btk(mac_handle_t mac_handle, uint8_t session_id,
		    const uint8_t *key, const int key_len)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	if (key_len < SIR_BTK_KEY_LEN) {
		sme_warn("Invalid BTK keylength [= %d]", key_len);
		return -EINVAL;
	}

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("incorrect session/vdev ID");
		return -EINVAL;
	}

	session = CSR_GET_SESSION(mac_ctx, session_id);

	qdf_mem_copy(session->eseCckmInfo.btk, key, SIR_BTK_KEY_LEN);
	/*
	 * KRK and BTK are updated by upper layer back to back. Send
	 * updated KRK and BTK together to FW here.
	 */
	csr_roam_offload_scan(mac_ctx, session_id,
			      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
			      REASON_ROAM_PSK_PMK_CHANGED);

	return 0;
}
#endif

/**
 * sme_stop_roaming() - Stop roaming for a given sessionId
 *  This is a synchronous call
 *
 * @mac_handle      - The handle returned by mac_open
 * @sessionId - Session Identifier
 *
 * Return QDF_STATUS_SUCCESS on success
 *	   Other status on failure
 */
QDF_STATUS sme_stop_roaming(mac_handle_t mac_handle, uint8_t session_id,
			    uint8_t reason)
{
	struct scheduler_msg wma_msg = {0};
	QDF_STATUS status;
	struct roam_offload_scan_req *req;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	tpCsrNeighborRoamControlInfo roam_info;
	struct csr_roam_session *session;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("incorrect session/vdev ID");
		return QDF_STATUS_E_INVAL;
	}

	session = CSR_GET_SESSION(mac_ctx, session_id);

	/*
	 * set the driver_disabled_roaming flag to true even if roaming
	 * is not enabled on this session so that roam start requests for
	 * this session can be blocked until driver enables roaming
	 */
	if (reason == ecsr_driver_disabled && session->pCurRoamProfile &&
	    session->pCurRoamProfile->csrPersona == QDF_STA_MODE) {
		session->pCurRoamProfile->driver_disabled_roaming = true;
		sme_debug("driver_disabled_roaming set for session %d",
			  session_id);
	}

	roam_info = &mac_ctx->roam.neighborRoamInfo[session_id];
	if (!roam_info->b_roam_scan_offload_started) {
		sme_debug("Roaming already disabled for session %d", session_id);
		return QDF_STATUS_SUCCESS;
	}
	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->Command = ROAM_SCAN_OFFLOAD_STOP;
	if ((reason == eCsrForcedDisassoc) || reason == ecsr_driver_disabled)
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
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("WMA_ROAM_SCAN_OFFLOAD_REQ failed, session_id: %d",
			session_id);
		qdf_mem_zero(req, sizeof(*req));
		qdf_mem_free(req);
		return QDF_STATUS_E_FAULT;
	}
	roam_info->b_roam_scan_offload_started = false;
	roam_info->last_sent_cmd = ROAM_SCAN_OFFLOAD_STOP;

	return QDF_STATUS_SUCCESS;
}

/*
 * sme_start_roaming() - Start roaming for a given sessionId
 *  This is a synchronous call
 *
 * mac_handle      - The handle returned by mac_open
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS on success
 *	Other status on failure
 */
QDF_STATUS sme_start_roaming(mac_handle_t mac_handle, uint8_t sessionId,
			     uint8_t reason)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		csr_roam_offload_scan(mac, sessionId, ROAM_SCAN_OFFLOAD_START,
				      reason);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_set_roam_opportunistic_scan_threshold_diff() -
 * Update Opportunistic Scan threshold diff
 *	This function is called through dynamic setConfig callback function
 *	to configure  nOpportunisticThresholdDiff
 *
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nOpportunisticThresholdDiff - Opportunistic Scan threshold diff
 * Return QDF_STATUS_SUCCESS - SME update nOpportunisticThresholdDiff config
 *	    successfully.
 *	    else SME is failed to update nOpportunisticThresholdDiff.
 */
QDF_STATUS sme_set_roam_opportunistic_scan_threshold_diff(
				mac_handle_t mac_handle,
				uint8_t sessionId,
				const uint8_t nOpportunisticThresholdDiff)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(mac, sessionId,
				nOpportunisticThresholdDiff,
				REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			mac->mlme_cfg->lfr.opportunistic_scan_threshold_diff =
				nOpportunisticThresholdDiff;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_get_roam_opportunistic_scan_threshold_diff()
 * gets Opportunistic Scan threshold diff
 * This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open
 * Return uint8_t - nOpportunisticThresholdDiff
 */
uint8_t sme_get_roam_opportunistic_scan_threshold_diff(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.opportunistic_scan_threshold_diff;
}

/*
 * sme_set_roam_rescan_rssi_diff() - Update roam rescan rssi diff
 * This function is called through dynamic setConfig callback function
 * to configure  nRoamRescanRssiDiff
 *
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nRoamRescanRssiDiff - roam rescan rssi diff
 * Return QDF_STATUS_SUCCESS - SME update nRoamRescanRssiDiff config
 *	    successfully.
 * else SME is failed to update nRoamRescanRssiDiff.
 */
QDF_STATUS sme_set_roam_rescan_rssi_diff(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nRoamRescanRssiDiff)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(mac, sessionId,
				nRoamRescanRssiDiff,
				REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status))
			mac->mlme_cfg->lfr.roam_rescan_rssi_diff =
				nRoamRescanRssiDiff;

		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_get_roam_rescan_rssi_diff()
 * gets roam rescan rssi diff
 *	  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open
 * Return int8_t - nRoamRescanRssiDiff
 */
uint8_t sme_get_roam_rescan_rssi_diff(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.roam_rescan_rssi_diff;
}

/*
 * sme_set_roam_bmiss_first_bcnt() -
 * Update Roam count for first beacon miss
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nRoamBmissFirstBcnt
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nRoamBmissFirstBcnt - Roam first bmiss count
 * Return QDF_STATUS_SUCCESS - SME update nRoamBmissFirstBcnt
 *	    successfully.
 * else SME is failed to update nRoamBmissFirstBcnt
 */
QDF_STATUS sme_set_roam_bmiss_first_bcnt(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nRoamBmissFirstBcnt)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(mac, sessionId,
				nRoamBmissFirstBcnt,
				REASON_ROAM_BMISS_FIRST_BCNT_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			mac->mlme_cfg->lfr.roam_bmiss_first_bcnt =
							nRoamBmissFirstBcnt;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_set_roam_bmiss_final_bcnt() -
 * Update Roam count for final beacon miss
 *	    This function is called through dynamic setConfig callback function
 *	    to configure nRoamBmissFinalBcnt
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nRoamBmissFinalBcnt - Roam final bmiss count
 * Return QDF_STATUS_SUCCESS - SME update nRoamBmissFinalBcnt
 *	    successfully.
 * else SME is failed to update nRoamBmissFinalBcnt
 */
QDF_STATUS sme_set_roam_bmiss_final_bcnt(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nRoamBmissFinalBcnt)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(mac, sessionId,
				nRoamBmissFinalBcnt,
				REASON_ROAM_BMISS_FINAL_BCNT_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			mac->mlme_cfg->lfr.roam_bmiss_final_bcnt =
							nRoamBmissFinalBcnt;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_set_neighbor_lookup_rssi_threshold() - update neighbor lookup
 *	rssi threshold
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
QDF_STATUS sme_set_neighbor_lookup_rssi_threshold(mac_handle_t mac_handle,
			uint8_t sessionId, uint8_t neighborLookupRssiThreshold)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_neighbor_roam_update_config(mac,
				sessionId, neighborLookupRssiThreshold,
				REASON_LOOKUP_THRESH_CHANGED);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			mac->mlme_cfg->lfr.neighbor_lookup_rssi_threshold =
				neighborLookupRssiThreshold;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_get_neighbor_lookup_rssi_threshold() - get neighbor lookup
 *	rssi threshold
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
uint8_t sme_get_neighbor_lookup_rssi_threshold(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.neighbor_lookup_rssi_threshold;
}

/*
 * sme_set_neighbor_scan_refresh_period() - set neighbor scan results
 *	refresh period
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
QDF_STATUS sme_set_neighbor_scan_refresh_period(mac_handle_t mac_handle,
		uint8_t sessionId, uint16_t neighborScanResultsRefreshPeriod)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan refresh period to %d- old value is %d - roam state is %s",
			  neighborScanResultsRefreshPeriod,
			  mac->mlme_cfg->lfr.
			  neighbor_scan_results_refresh_period,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.neighbor_scan_results_refresh_period =
			neighborScanResultsRefreshPeriod;
		pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod =
			neighborScanResultsRefreshPeriod;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_NEIGHBOR_SCAN_REFRESH_PERIOD_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_get_neighbor_scan_refresh_period() - get neighbor scan results
 *	refresh period
 *  This is a synchronous call
 *
 *  \param mac_handle - The handle returned by mac_open.
 *  \return uint16_t - Neighbor scan results refresh period value
 */
uint16_t sme_get_neighbor_scan_refresh_period(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.neighbor_scan_results_refresh_period;
}

/*
 * sme_get_empty_scan_refresh_period() - get empty scan refresh period
 * This is a synchronuous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return QDF_STATUS_SUCCESS - SME update config successful.
 *	   Other status means SME is failed to update
 */
uint16_t sme_get_empty_scan_refresh_period(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.empty_scan_refresh_period;
}

/*
 * sme_update_empty_scan_refresh_period
 * Update empty_scan_refresh_period
 *	    This function is called through dynamic setConfig callback function
 *	    to configure empty_scan_refresh_period
 *	    Usage: adb shell iwpriv wlan0 setConfig
 *			empty_scan_refresh_period=[0 .. 60]
 *
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * empty_scan_refresh_period - scan period following empty scan results.
 * Return Success or failure
 */

QDF_STATUS sme_update_empty_scan_refresh_period(mac_handle_t mac_handle,
						uint8_t sessionId, uint16_t
						empty_scan_refresh_period)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set roam scan period to %d -old value is %d - roam state is %s",
			  empty_scan_refresh_period,
			  mac->mlme_cfg->lfr.empty_scan_refresh_period,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.empty_scan_refresh_period =
			empty_scan_refresh_period;
		pNeighborRoamInfo->cfgParams.emptyScanRefreshPeriod =
			empty_scan_refresh_period;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_EMPTY_SCAN_REF_PERIOD_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
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
 * mac_handle: Opaque handle to the global MAC context
 * nNeighborScanMinChanTime - Channel minimum dwell time
 * sessionId - Session Identifier
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_min_chan_time(mac_handle_t mac_handle,
					       const uint16_t
					       nNeighborScanMinChanTime,
					       uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set channel min dwell time to %d - old value is %d - roam state is %s",
			  nNeighborScanMinChanTime,
			  mac->mlme_cfg->lfr.neighbor_scan_min_chan_time,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));

		mac->mlme_cfg->lfr.neighbor_scan_min_chan_time =
						nNeighborScanMinChanTime;
		mac->roam.neighborRoamInfo[sessionId].cfgParams.
		minChannelScanTime = nNeighborScanMinChanTime;
		sme_release_global_lock(&mac->sme);
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
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nNeighborScanMinChanTime - Channel maximum dwell time
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_max_chan_time(mac_handle_t mac_handle,
					       uint8_t sessionId,
					       const uint16_t
					       nNeighborScanMaxChanTime)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set channel max dwell time to %d - old value is %d - roam state is %s",
			  nNeighborScanMaxChanTime,
			  mac->mlme_cfg->lfr.neighbor_scan_max_chan_time,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.neighbor_scan_max_chan_time =
			nNeighborScanMaxChanTime;
		pNeighborRoamInfo->cfgParams.maxChannelScanTime =
			nNeighborScanMaxChanTime;
		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_SCAN_CH_TIME_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_get_neighbor_scan_min_chan_time() -
 * get neighbor scan min channel time
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - channel min time value
 */
uint16_t sme_get_neighbor_scan_min_chan_time(mac_handle_t mac_handle,
					     uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return mac->roam.neighborRoamInfo[sessionId].cfgParams.
	       minChannelScanTime;
}

/*
 * sme_get_neighbor_roam_state() -
 * get neighbor roam state
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - neighbor roam state
 */
uint32_t sme_get_neighbor_roam_state(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return mac->roam.neighborRoamInfo[sessionId].neighborRoamState;
}

/*
 * sme_get_current_roam_state() -
 * get current roam state
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - current roam state
 */
uint32_t sme_get_current_roam_state(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->roam.curState[sessionId];
}

/*
 * sme_get_current_roam_sub_state() -
 *   \brief  get neighbor roam sub state
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - current roam sub state
 */
uint32_t sme_get_current_roam_sub_state(mac_handle_t mac_handle,
					uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->roam.curSubState[sessionId];
}

/*
 * sme_get_lim_sme_state() -
 *   get Lim Sme state
 *
 * mac_handle - The handle returned by mac_open.
 * Return uint32_t - Lim Sme state
 */
uint32_t sme_get_lim_sme_state(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->lim.gLimSmeState;
}

/*
 * sme_get_lim_mlm_state() -
 *   get Lim Mlm state
 *
 * mac_handle - The handle returned by mac_open.
 * Return uint32_t - Lim Mlm state
 */
uint32_t sme_get_lim_mlm_state(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->lim.gLimMlmState;
}

/*
 * sme_is_lim_session_valid() -
 *  is Lim session valid
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return bool - true or false
 */
bool sme_is_lim_session_valid(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (sessionId > mac->lim.maxBssId)
		return false;

	return mac->lim.gpSession[sessionId].valid;
}

/*
 * sme_get_lim_sme_session_state() -
 *   get Lim Sme session state
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - Lim Sme session state
 */
uint32_t sme_get_lim_sme_session_state(mac_handle_t mac_handle,
				       uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->lim.gpSession[sessionId].limSmeState;
}

/*
 * sme_get_lim_mlm_session_state() -
 *   \brief  get Lim Mlm session state
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint32_t - Lim Mlm session state
 */
uint32_t sme_get_lim_mlm_session_state(mac_handle_t mac_handle,
				       uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->lim.gpSession[sessionId].limMlmState;
}

/*
 * sme_get_neighbor_scan_max_chan_time() -
 *   get neighbor scan max channel time
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - channel max time value
 */
uint16_t sme_get_neighbor_scan_max_chan_time(mac_handle_t mac_handle,
					     uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return mac->roam.neighborRoamInfo[sessionId].cfgParams.
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
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * nNeighborScanPeriod - neighbor scan period
 * Return Success or failure
 */
QDF_STATUS sme_set_neighbor_scan_period(mac_handle_t mac_handle,
					uint8_t sessionId,
					const uint16_t nNeighborScanPeriod)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set neighbor scan period to %d - old value is %d - roam state is %s",
			  nNeighborScanPeriod,
			  mac->mlme_cfg->lfr.neighbor_scan_timer_period,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.neighbor_scan_timer_period =
			nNeighborScanPeriod;
		pNeighborRoamInfo->cfgParams.neighborScanPeriod =
			nNeighborScanPeriod;

		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_SCAN_HOME_TIME_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_get_neighbor_scan_period() -
 *   get neighbor scan period
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return uint16_t - neighbor scan period
 */
uint16_t sme_get_neighbor_scan_period(mac_handle_t mac_handle,
				      uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return 0;
	}

	return mac->roam.neighborRoamInfo[sessionId].cfgParams.
	       neighborScanPeriod;
}

/*
 * sme_get_roam_rssi_diff() - get Roam rssi diff
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return uint16_t - Rssi diff value
 */
uint8_t sme_get_roam_rssi_diff(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.roam_rssi_diff;
}

/**
 * sme_change_roam_scan_channel_list() - to change scan channel list
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId: sme session id
 * @pChannelList: Output channel list
 * @numChannels: Output number of channels
 *
 * This routine is called to Change roam scan channel list.
 * This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_change_roam_scan_channel_list(mac_handle_t mac_handle,
					     uint8_t sessionId,
					     uint8_t *pChannelList,
					     uint8_t numChannels)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	uint8_t oldChannelList[CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t newChannelList[CFG_VALID_CHANNEL_LIST_LEN * 2] = { 0 };
	uint8_t i = 0, j = 0;
	tCsrChannelInfo *chan_info;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to acquire SME lock");
		return status;
	}
	chan_info = &pNeighborRoamInfo->cfgParams.channelInfo;

	if (chan_info->ChannelList) {
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
	csr_flush_cfg_bg_scan_roam_channel_list(mac, sessionId);
	csr_create_bg_scan_roam_channel_list(mac, sessionId, pChannelList,
			numChannels);
	sme_set_roam_scan_control(mac_handle, sessionId, 1);
	if (chan_info->ChannelList) {
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
		mac->roam.neighborRoamInfo[sessionId].neighborRoamState);

	if (mac->mlme_cfg->lfr.roam_scan_offload_enabled)
		csr_roam_offload_scan(mac, sessionId,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_CHANNEL_LIST_CHANGED);

	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_get_roam_scan_channel_list() - To get roam scan channel list
 * @mac_handle: Opaque handle to the global MAC context
 * @pChannelList: Output channel list
 * @pNumChannels: Output number of channels
 * @sessionId: Session Identifier
 *
 * To get roam scan channel list This is a synchronous call
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_roam_scan_channel_list(mac_handle_t mac_handle,
			uint8_t *pChannelList, uint8_t *pNumChannels,
			uint8_t sessionId)
{
	int i = 0;
	uint8_t *pOutPtr = pChannelList;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	pNeighborRoamInfo = &mac->roam.neighborRoamInfo[sessionId];
	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	if (!pNeighborRoamInfo->cfgParams.channelInfo.ChannelList) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_WARN,
			FL("Roam Scan channel list is NOT yet initialized"));
		*pNumChannels = 0;
		sme_release_global_lock(&mac->sme);
		return status;
	}

	*pNumChannels = pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels;
	for (i = 0; i < (*pNumChannels); i++)
		pOutPtr[i] =
			pNeighborRoamInfo->cfgParams.channelInfo.ChannelList[i];

	pOutPtr[i] = '\0';
	sme_release_global_lock(&mac->sme);
	return status;
}

/*
 * sme_get_is_ese_feature_enabled() - get ESE feature enabled or not
 *  This is a synchronuous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return true (1) - if the ESE feature is enabled
 *	  false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_ese_feature_enabled(mac_handle_t mac_handle)
{
#ifdef FEATURE_WLAN_ESE
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return csr_roam_is_ese_ini_feature_enabled(mac);
#else
	return false;
#endif
}

/*
 * sme_get_wes_mode() - get WES Mode
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open
 * Return uint8_t - WES Mode Enabled(1)/Disabled(0)
 */
bool sme_get_wes_mode(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.wes_mode_enabled;
}

/*
 * sme_get_roam_scan_control() - get scan control
 *  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return bool - Enabled(1)/Disabled(0)
 */
bool sme_get_roam_scan_control(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->roam.configParam.nRoamScanControl;
}

/*
 * sme_get_is_lfr_feature_enabled() - get LFR feature enabled or not
 *  This is a synchronuous call
 * mac_handle - The handle returned by mac_open.
 * Return true (1) - if the feature is enabled
 *	  false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_lfr_feature_enabled(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.lfr_enabled;
}

/*
 * sme_get_is_ft_feature_enabled() - get FT feature enabled or not
 *  This is a synchronuous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return true (1) - if the feature is enabled
 *	   false (0) - if feature is disabled (compile or runtime)
 */
bool sme_get_is_ft_feature_enabled(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.fast_transition_enabled;
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

QDF_STATUS sme_get_link_speed(mac_handle_t mac_handle,
			      struct link_speed_info *req,
			      void *context,
			      sme_link_speed_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac;
	void *wma_handle;

	if (!mac_handle || !cb || !req) {
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

	mac = MAC_CONTEXT(mac_handle);
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("Failed to acquire global lock");
		return QDF_STATUS_E_FAILURE;
	}

	mac->sme.link_speed_context = context;
	mac->sme.link_speed_cb = cb;
	status = wma_get_link_speed(wma_handle, req);
	sme_release_global_lock(&mac->sme);
	return status;
}

QDF_STATUS sme_get_peer_stats(struct mac_context *mac,
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
	if (!message.bodyptr) {
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(message.bodyptr, &req, sizeof(req));
	message.type = WMA_GET_PEER_INFO;
	message.reserved = 0;
	qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Post get peer info msg fail", __func__);
		qdf_mem_free(message.bodyptr);
		qdf_status = QDF_STATUS_E_FAILURE;
	}
	sme_release_global_lock(&mac->sme);
	return qdf_status;
}

QDF_STATUS
sme_get_peer_info(mac_handle_t mac_handle,
		  struct sir_peer_info_req req,
		  void *context,
		  void (*callbackfn)(struct sir_peer_info_resp *param,
				     void *pcontext))
{

	QDF_STATUS status;
	QDF_STATUS qdf_status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (!callbackfn) {
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
		if (!message.bodyptr) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(message.bodyptr, &req, sizeof(req));
		message.type = WMA_GET_PEER_INFO;
		message.reserved = 0;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
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

QDF_STATUS sme_get_peer_info_ext(mac_handle_t mac_handle,
		struct sir_peer_info_ext_req *req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_ext_resp *param,
			void *pcontext))
{
	QDF_STATUS status;
	QDF_STATUS qdf_status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (!callbackfn) {
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
		if (!message.bodyptr) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(message.bodyptr,
				req,
				sizeof(struct sir_peer_info_ext_req));
		message.type = WMA_GET_PEER_INFO_EXT;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
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

/*convert the ini value to the ENUM used in csr and MAC for CB state*/
ePhyChanBondState sme_get_cb_phy_state_from_cb_ini_value(uint32_t cb_ini_value)
{
	return csr_convert_cb_ini_value_to_phy_cb_state(cb_ini_value);
}

void sme_set_curr_device_mode(mac_handle_t mac_handle,
			      enum QDF_OPMODE curr_device_mode)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->sme.curr_device_mode = curr_device_mode;
}

/*
 * sme_handoff_request() - a wrapper function to Request a handoff from CSR.
 *   This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open
 * sessionId - Session Identifier
 * pHandoffInfo - info provided by HDD with the handoff request (namely:
 * BSSID, channel etc.)
 * Return QDF_STATUS_SUCCESS - SME passed the request to CSR successfully.
 *	   Other status means SME is failed to send the request.
 */

QDF_STATUS sme_handoff_request(mac_handle_t mac_handle,
			       uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: invoked", __func__);
		status = csr_handoff_request(mac, sessionId, pHandoffInfo);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/**
 * sme_add_periodic_tx_ptrn() - Add Periodic TX Pattern
 * @mac_handle: Opaque handle to the global MAC context
 * @addPeriodicTxPtrnParams: request message
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
sme_add_periodic_tx_ptrn(mac_handle_t mac_handle,
			 struct sSirAddPeriodicTxPtrn *addPeriodicTxPtrnParams)
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	struct mac_context *mac  = MAC_CONTEXT(mac_handle);
	struct sSirAddPeriodicTxPtrn *req_msg;
	struct scheduler_msg msg = {0};

	SME_ENTER();

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);
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
 * @mac_handle: Opaque handle to the global MAC context
 * @delPeriodicTxPtrnParams: request message
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS
sme_del_periodic_tx_ptrn(mac_handle_t mac_handle,
			 struct sSirDelPeriodicTxPtrn *delPeriodicTxPtrnParams)
{
	QDF_STATUS status    = QDF_STATUS_SUCCESS;
	struct mac_context *mac   = MAC_CONTEXT(mac_handle);
	struct sSirDelPeriodicTxPtrn *req_msg;
	struct scheduler_msg msg = {0};

	SME_ENTER();

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

#ifdef FEATURE_WLAN_RMC
/*
 * sme_enable_rmc() - enables RMC
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId : Session ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_enable_rmc(mac_handle_t mac_handle, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	SME_ENTER();

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_ENABLE_IND;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_disable_rmc() - disables RMC
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId : Session ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_disable_rmc(mac_handle_t mac_handle, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	SME_ENTER();

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_DISABLE_IND;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_send_rmc_action_period() - sends RMC action period param to target
 * @mac_handle: Opaque handle to the global MAC context
 * @sessionId : Session ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_rmc_action_period(mac_handle_t mac_handle,
				      uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		message.bodyptr = NULL;
		message.type = WMA_RMC_ACTION_PERIOD_IND;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			status = QDF_STATUS_E_FAILURE;

		sme_release_global_lock(&mac->sme);
	}

	return status;
}
#endif /* FEATURE_WLAN_RMC */

/*
 * sme_request_ibss_peer_info() -  request ibss peer info
 * @mac_handle: Opaque handle to the global MAC context
 * @cb_context: Pointer to user data
 * @peer_info_cb: Peer info callback
 * @allPeerInfoReqd: All peer info required or not
 * @staIdx: sta index
 *
 * Return:  QDF_STATUS
 */
QDF_STATUS sme_request_ibss_peer_info(mac_handle_t mac_handle, void *cb_context,
				      ibss_peer_info_cb peer_info_cb,
				      bool allPeerInfoReqd, uint8_t staIdx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tSirIbssGetPeerInfoReqParams *pIbssInfoReqParams;
	struct ibss_peer_info_cb_info *cb_info;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		cb_info = &mac->sme.peer_info_cb_info;
		cb_info->peer_info_cb = peer_info_cb;
		cb_info->peer_info_cb_context = cb_context;

		pIbssInfoReqParams = (tSirIbssGetPeerInfoReqParams *)
			qdf_mem_malloc(sizeof(tSirIbssGetPeerInfoReqParams));
		if (!pIbssInfoReqParams) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}
		pIbssInfoReqParams->allPeerInfoReqd = allPeerInfoReqd;
		pIbssInfoReqParams->staIdx = staIdx;

		message.type = WMA_GET_IBSS_PEER_INFO_REQ;
		message.bodyptr = pIbssInfoReqParams;
		message.reserved = 0;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post WMA_GET_IBSS_PEER_INFO_REQ MSG failed",
				  __func__);
			qdf_mem_free(pIbssInfoReqParams);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}

	return qdf_status;
}

/*
 * sme_send_cesium_enable_ind() -
 *  Used to send proprietary cesium enable indication to fw
 *
 * mac_handle
 * sessionId
 * Return QDF_STATUS
 */
QDF_STATUS sme_send_cesium_enable_ind(mac_handle_t mac_handle,
				      uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		message.bodyptr = NULL;
		message.type = WMA_IBSS_CESIUM_ENABLE_IND;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_set_wlm_latency_level(mac_handle_t mac_handle,
				     uint16_t session_id,
				     uint16_t latency_level)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct wlm_latency_level_param params;
	void *wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma)
		return QDF_STATUS_E_FAILURE;

	if (!mac_ctx->mlme_cfg->wlm_config.latency_enable) {
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
		mac_ctx->mlme_cfg->wlm_config.latency_flags[latency_level];
	params.vdev_id = session_id;

	status = wma_set_wlm_latency_level(wma, &params);

	return status;
}

void sme_get_command_q_status(mac_handle_t mac_handle)
{
	tSmeCmd *pTempCmd = NULL;
	tListElem *pEntry;
	struct mac_context *mac;

	if (!mac_handle)
		return;

	mac = MAC_CONTEXT(mac_handle);

	pEntry = csr_nonscan_active_ll_peek_head(mac, LL_ACCESS_LOCK);
	if (pEntry)
		pTempCmd = GET_BASE_ADDR(pEntry, tSmeCmd, Link);

	sme_err("WLAN_BUG_RCA: Currently smeCmdActiveList has command (0x%X)",
			(pTempCmd) ? pTempCmd->command : eSmeNoCommand);
	if (pTempCmd) {
		if (eSmeCsrCommandMask & pTempCmd->command)
			/* CSR command is stuck. See what the reason code is
			 * for that command
			 */
			dump_csr_command_info(mac, pTempCmd);
	} /* if(pTempCmd) */

	sme_err("Currently smeCmdPendingList has %d commands",
			csr_nonscan_pending_ll_count(mac));

}

#ifdef WLAN_FEATURE_DSRC
/**
 * sme_ocb_gen_timing_advert_frame() - generate TA frame and populate the buffer
 * @mac_handle: Opaque handle to the global MAC context
 * @self_addr: the self MAC address
 * @buf: the buffer that will contain the frame
 * @timestamp_offset: return for the offset of the timestamp field
 * @time_value_offset: return for the time_value field in the TA IE
 *
 * Return: the length of the buffer.
 */
int sme_ocb_gen_timing_advert_frame(mac_handle_t mac_handle,
				    tSirMacAddr self_addr, uint8_t **buf,
				    uint32_t *timestamp_offset,
				    uint32_t *time_value_offset)
{
	int template_length;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	template_length = sch_gen_timing_advert_frame(mac_ctx, self_addr, buf,
						  timestamp_offset,
						  time_value_offset);
	return template_length;
}
#endif

QDF_STATUS sme_notify_modem_power_state(mac_handle_t mac_handle, uint32_t value)
{
	struct scheduler_msg msg = {0};
	tpSirModemPowerStateInd request_buf;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac)
		return QDF_STATUS_E_FAILURE;

	request_buf = qdf_mem_malloc(sizeof(tSirModemPowerStateInd));
	if (!request_buf)
		return QDF_STATUS_E_NOMEM;

	request_buf->param = value;

	msg.type = WMA_MODEM_POWER_STATE_IND;
	msg.reserved = 0;
	msg.bodyptr = request_buf;
	if (!QDF_IS_STATUS_SUCCESS
		    (scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_MODEM_POWER_STATE_IND message to WMA",
			__func__);
		qdf_mem_free(request_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_HT_2040_COEX
QDF_STATUS sme_notify_ht2040_mode(mac_handle_t mac_handle, uint16_t staId,
				  struct qdf_mac_addr macAddrSTA,
				  uint8_t sessionId,
				  uint8_t channel_type)
{
	struct scheduler_msg msg = {0};
	tUpdateVHTOpMode *pHtOpMode = NULL;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac)
		return QDF_STATUS_E_FAILURE;

	pHtOpMode = qdf_mem_malloc(sizeof(tUpdateVHTOpMode));
	if (!pHtOpMode)
		return QDF_STATUS_E_NOMEM;

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
		    (scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA, &msg))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_UPDATE_OP_MODE message to WMA",
			__func__);
		qdf_mem_free(pHtOpMode);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Notified FW about OP mode: %d for staId=%d",
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
QDF_STATUS sme_set_ht2040_mode(mac_handle_t mac_handle, uint8_t sessionId,
			       uint8_t channel_type, bool obssEnabled)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	ePhyChanBondState cbMode;
	struct csr_roam_session *session = CSR_GET_SESSION(mac, sessionId);

	if (!CSR_IS_SESSION_VALID(mac, sessionId)) {
		sme_err("Session not valid for session id %d", sessionId);
		return QDF_STATUS_E_INVAL;
	}
	session = CSR_GET_SESSION(mac, sessionId);
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
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_set_ht2040_mode(mac, sessionId,
					     cbMode, obssEnabled);
		sme_release_global_lock(&mac->sme);
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

	if (!wmaContext) {
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

int16_t sme_get_ht_config(mac_handle_t mac_handle, uint8_t session_id,
			  uint16_t ht_capab)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, session_id);

	if (!pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: pSession is NULL", __func__);
		return -EIO;
	}
	switch (ht_capab) {
	case WNI_CFG_HT_CAP_INFO_ADVANCE_CODING:
		return pSession->ht_config.ht_rx_ldpc;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		return pSession->ht_config.ht_tx_stbc;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		return pSession->ht_config.ht_rx_stbc;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
		return pSession->ht_config.ht_sgi20;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		return pSession->ht_config.ht_sgi40;
	default:
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "invalid ht capability");
		return -EIO;
	}
}

int sme_update_ht_config(mac_handle_t mac_handle, uint8_t sessionId,
			 uint16_t htCapab, int value)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *pSession = CSR_GET_SESSION(mac, sessionId);

	if (!pSession) {
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
		pSession->ht_config.ht_rx_ldpc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_TX_STBC:
		pSession->ht_config.ht_tx_stbc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_RX_STBC:
		pSession->ht_config.ht_rx_stbc = value;
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ:
		value = value ? 1 : 0; /* HT SGI can be only 1 or 0 */
		pSession->ht_config.ht_sgi20 = value;
		break;
	case WNI_CFG_HT_CAP_INFO_SHORT_GI_40MHZ:
		value = value ? 1 : 0; /* HT SGI can be only 1 or 0 */
		pSession->ht_config.ht_sgi40 = value;
		break;
	}

	csr_roam_update_config(mac, sessionId, htCapab, value);
	return 0;
}

int sme_set_addba_accept(mac_handle_t mac_handle, uint8_t session_id, int value)
{
	struct sme_addba_accept *addba_accept;
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	addba_accept = qdf_mem_malloc(sizeof(*addba_accept));
	if (!addba_accept)
		return -EIO;

	addba_accept->session_id = session_id;
	addba_accept->addba_accept = value;
	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = eWNI_SME_SET_ADDBA_ACCEPT;
	msg.reserved = 0;
	msg.bodyptr = addba_accept;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("Not able to post addba reject");
		qdf_mem_free(addba_accept);
		return -EIO;
	}
	return 0;
}

int sme_set_ba_buff_size(mac_handle_t mac_handle, uint8_t session_id,
			 uint16_t buff_size)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	if (!buff_size) {
		sme_err("invalid buff size %d", buff_size);
		return -EINVAL;
	}
	mac_ctx->usr_cfg_ba_buff_size = buff_size;
	sme_debug("addba buff size is set to %d",
			mac_ctx->usr_cfg_ba_buff_size);

	return 0;
}

#define DEFAULT_BA_BUFF_SIZE 64
int sme_send_addba_req(mac_handle_t mac_handle, uint8_t session_id, uint8_t tid,
		       uint16_t buff_size)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint16_t ba_buff = 0;
	QDF_STATUS status;
	struct scheduler_msg msg = {0};
	struct send_add_ba_req *send_ba_req;
	struct csr_roam_session *csr_session = NULL;

	if (!csr_is_conn_state_connected_infra(mac_ctx, session_id)) {
		sme_err("STA not infra/connected state session_id: %d",
				session_id);
		return -EINVAL;
	}
	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!csr_session) {
		sme_err("CSR session is NULL");
		return -EINVAL;
	}
	send_ba_req = qdf_mem_malloc(sizeof(*send_ba_req));
	if (!send_ba_req)
		return -EIO;

	qdf_mem_copy(send_ba_req->mac_addr,
			csr_session->connectedProfile.bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	ba_buff = buff_size;
	if (!buff_size) {
		if (mac_ctx->usr_cfg_ba_buff_size)
			ba_buff = mac_ctx->usr_cfg_ba_buff_size;
		else
			ba_buff = DEFAULT_BA_BUFF_SIZE;
	}
	send_ba_req->param.vdev_id = session_id;
	send_ba_req->param.tidno = tid;
	send_ba_req->param.buffersize = ba_buff;
	msg.type = WMA_SEND_ADDBA_REQ;
	msg.bodyptr = send_ba_req;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_mem_free(send_ba_req);
		return -EIO;
	}
	sme_debug("ADDBA_REQ sent to FW: tid %d buff_size %d", tid, ba_buff);

	return 0;
}

int sme_set_no_ack_policy(mac_handle_t mac_handle, uint8_t session_id,
			  uint8_t val, uint8_t ac)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t i, set_val;
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	if (ac > QCA_WLAN_AC_ALL) {
		sme_err("invalid ac val %d", ac);
		return -EINVAL;
	}
	if (val)
		set_val = 1;
	else
		set_val = 0;
	if (ac == QCA_WLAN_AC_ALL) {
		for (i = 0; i < ac; i++)
			mac_ctx->no_ack_policy_cfg[i] = set_val;
	} else {
		mac_ctx->no_ack_policy_cfg[ac] = set_val;
	}
	sme_debug("no ack is set to %d for ac %d", set_val, ac);
	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = eWNI_SME_UPDATE_EDCA_PROFILE;
	msg.reserved = 0;
	msg.bodyval = session_id;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("Not able to post update edca profile");
		return -EIO;
	}

	return 0;
}

int sme_set_auto_rate_he_ltf(mac_handle_t mac_handle, uint8_t session_id,
			     uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint32_t set_val;
	uint32_t bit_mask = 0;
	int status;

	if (cfg_val > QCA_WLAN_HE_LTF_4X) {
		sme_err("invalid HE LTF cfg %d", cfg_val);
		return -EINVAL;
	}

	/*set the corresponding HE LTF cfg BIT*/
	if (cfg_val == QCA_WLAN_HE_LTF_AUTO)
		bit_mask = HE_LTF_ALL;
	else
		bit_mask = (1 << (cfg_val - 1));

	set_val = mac_ctx->he_sgi_ltf_cfg_bit_mask;

	SET_AUTO_RATE_HE_LTF_VAL(set_val, bit_mask);

	mac_ctx->he_sgi_ltf_cfg_bit_mask = set_val;
	status = wma_cli_set_command(session_id,
			WMI_VDEV_PARAM_AUTORATE_MISC_CFG,
			set_val, VDEV_CMD);
	if (status) {
		sme_err("failed to set he_ltf_sgi");
		return status;
	}

	sme_debug("HE SGI_LTF is set to 0x%08X",
			mac_ctx->he_sgi_ltf_cfg_bit_mask);

	return 0;
}

int sme_set_auto_rate_he_sgi(mac_handle_t mac_handle, uint8_t session_id,
			     uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint32_t set_val;
	uint32_t sgi_bit_mask = 0;
	int status;

	if ((cfg_val > AUTO_RATE_GI_3200NS) ||
			(cfg_val < AUTO_RATE_GI_400NS)) {
		sme_err("invalid auto rate GI cfg %d", cfg_val);
		return -EINVAL;
	}

	sgi_bit_mask = (1 << cfg_val);

	set_val = mac_ctx->he_sgi_ltf_cfg_bit_mask;
	SET_AUTO_RATE_SGI_VAL(set_val, sgi_bit_mask);

	mac_ctx->he_sgi_ltf_cfg_bit_mask = set_val;
	status = wma_cli_set_command(session_id,
				     WMI_VDEV_PARAM_AUTORATE_MISC_CFG,
				     set_val, VDEV_CMD);
	if (status) {
		sme_err("failed to set he_ltf_sgi");
		return status;
	}

	sme_debug("auto rate HE SGI_LTF is set to 0x%08X",
			mac_ctx->he_sgi_ltf_cfg_bit_mask);

	return 0;
}

#define HT20_SHORT_GI_MCS7_RATE 722
/*
 * sme_send_rate_update_ind() -
 *  API to Update rate
 *
 * mac_handle - The handle returned by mac_open
 * rateUpdateParams - Pointer to rate update params
 * Return QDF_STATUS
 */
QDF_STATUS sme_send_rate_update_ind(mac_handle_t mac_handle,
				    tSirRateUpdateInd *rateUpdateParams)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;
	struct scheduler_msg msg = {0};
	tSirRateUpdateInd *rate_upd = qdf_mem_malloc(sizeof(tSirRateUpdateInd));

	if (!rate_upd)
		return QDF_STATUS_E_FAILURE;

	*rate_upd = *rateUpdateParams;

	if (rate_upd->mcastDataRate24GHz == HT20_SHORT_GI_MCS7_RATE)
		rate_upd->mcastDataRate24GHzTxFlag =
			TX_RATE_HT20 | TX_RATE_SGI;
	else if (rate_upd->reliableMcastDataRate ==
		 HT20_SHORT_GI_MCS7_RATE)
		rate_upd->reliableMcastDataRateTxFlag =
			TX_RATE_HT20 | TX_RATE_SGI;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		msg.type = WMA_RATE_UPDATE_IND;
		msg.bodyptr = rate_upd;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, msg.type));
		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post WMA_RATE_UPDATE_IND to WMA!",
				  __func__);

			sme_release_global_lock(&mac->sme);
			qdf_mem_free(rate_upd);
			return QDF_STATUS_E_FAILURE;
		}

		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_SUCCESS;
	}

	return status;
}

/**
 * sme_update_access_policy_vendor_ie() - update vendor ie and access policy.
 * @mac_handle: Pointer to the mac context
 * @session_id: sme session id
 * @vendor_ie: vendor ie
 * @access_policy: vendor ie access policy
 *
 * This function updates the vendor ie and access policy to lim.
 *
 * Return: success or failure.
 */
QDF_STATUS sme_update_access_policy_vendor_ie(mac_handle_t mac_handle,
					      uint8_t session_id,
					      uint8_t *vendor_ie,
					      int access_policy)
{
	struct sme_update_access_policy_vendor_ie *msg;
	uint16_t msg_len;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	msg_len  = sizeof(*msg);

	msg = qdf_mem_malloc(msg_len);
	if (!msg) {
		return QDF_STATUS_E_NOMEM;
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
 * sme_update_sta_inactivity_timeout(): Update sta_inactivity_timeout to FW
 * @mac_handle: Handle returned by mac_open
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
QDF_STATUS sme_update_sta_inactivity_timeout(mac_handle_t mac_handle,
		 struct sme_sta_inactivity_timeout  *sta_inactivity_timer)
{
	struct sme_sta_inactivity_timeout *inactivity_time;
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	inactivity_time = qdf_mem_malloc(sizeof(*inactivity_time));
	if (!inactivity_time)
		return QDF_STATUS_E_FAILURE;

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
 * @mac_handle: Opaque handle to the global MAC context
 * @chanId: channel id
 * @regInfo1: first reg info to fill
 * @regInfo2: second reg info to fill
 *
 * This routine will give you reg info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_reg_info(mac_handle_t mac_handle, uint8_t chanId,
			    uint32_t *regInfo1, uint32_t *regInfo2)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;
	uint8_t i;
	bool found = false;

	status = sme_acquire_global_lock(&mac->sme);
	*regInfo1 = 0;
	*regInfo2 = 0;
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;

	for (i = 0; i < CFG_VALID_CHANNEL_LIST_LEN; i++) {
		if (mac->scan.defaultPowerTable[i].chan_num == chanId) {
			SME_SET_CHANNEL_REG_POWER(*regInfo1,
				mac->scan.defaultPowerTable[i].tx_power);

			SME_SET_CHANNEL_MAX_TX_POWER(*regInfo2,
				mac->scan.defaultPowerTable[i].tx_power);
			found = true;
			break;
		}
	}
	if (!found)
		status = QDF_STATUS_E_FAILURE;

	sme_release_global_lock(&mac->sme);
	return status;
}

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
QDF_STATUS sme_set_auto_shutdown_cb(mac_handle_t mac_handle,
				    void (*callback_fn)(void))
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		  "%s: Plug in Auto shutdown event callback", __func__);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if (callback_fn)
			mac->sme.auto_shutdown_cb = callback_fn;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

/*
 * sme_set_auto_shutdown_timer() -
 *  API to set auto shutdown timer value in FW.
 *
 * mac_handle - The handle returned by mac_open
 * timer_val - The auto shutdown timer value to be set
 * Return Configuration message posting status, SUCCESS or Fail
 */
QDF_STATUS sme_set_auto_shutdown_timer(mac_handle_t mac_handle,
				       uint32_t timer_val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct auto_shutdown_cmd *auto_sh_cmd;
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		auto_sh_cmd = qdf_mem_malloc(sizeof(*auto_sh_cmd));
		if (!auto_sh_cmd) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		auto_sh_cmd->timer_val = timer_val;

		/* serialize the req through MC thread */
		message.bodyptr = auto_sh_cmd;
		message.type = WMA_SET_AUTO_SHUTDOWN_TIMER_REQ;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Auto shutdown MSG fail", __func__);
			qdf_mem_free(auto_sh_cmd);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Posted Auto shutdown MSG", __func__);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}
#endif

#ifdef FEATURE_WLAN_CH_AVOID
/*
 * sme_ch_avoid_update_req() -
 *   API to request channel avoidance update from FW.
 *
 * mac_handle - The handle returned by mac_open
 * update_type - The update_type parameter of this request call
 * Return Configuration message posting status, SUCCESS or Fail
 */
QDF_STATUS sme_ch_avoid_update_req(mac_handle_t mac_handle)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tSirChAvoidUpdateReq *cauReq;
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		cauReq = qdf_mem_malloc(sizeof(tSirChAvoidUpdateReq));
		if (!cauReq) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		cauReq->reserved_param = 0;

		/* serialize the req through MC thread */
		message.bodyptr = cauReq;
		message.type = WMA_CH_AVOID_UPDATE_REQ;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Post Ch Avoid Update MSG fail",
				  __func__);
			qdf_mem_free(cauReq);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Posted Ch Avoid Update MSG", __func__);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}
#endif

/**
 * sme_set_miracast() - Function to set miracast value to UMAC
 * @mac_handle:                Handle returned by macOpen
 * @filter_type:        0-Disabled, 1-Source, 2-sink
 *
 * This function passes down the value of miracast set by
 * framework to UMAC
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
QDF_STATUS sme_set_miracast(mac_handle_t mac_handle, uint8_t filter_type)
{
	struct scheduler_msg msg = {0};
	uint32_t *val;
	struct mac_context *mac_ptr = MAC_CONTEXT(mac_handle);

	if (!mac_ptr) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid pointer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	val = qdf_mem_malloc(sizeof(*val));
	if (!val)
		return QDF_STATUS_E_NOMEM;

	*val = filter_type;

	msg.type = SIR_HAL_SET_MIRACAST;
	msg.reserved = 0;
	msg.bodyptr = val;

	if (!QDF_IS_STATUS_SUCCESS(
				scheduler_post_message(QDF_MODULE_ID_SME,
						       QDF_MODULE_ID_WMA,
						       QDF_MODULE_ID_WMA,
						       &msg))) {
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
	if (!ptr_val)
		return QDF_STATUS_E_NOMEM;

	*ptr_val = val;

	msg.type = SIR_HAL_SET_MAS;
	msg.reserved = 0;
	msg.bodyptr = ptr_val;

	if (!QDF_IS_STATUS_SUCCESS(
				scheduler_post_message(QDF_MODULE_ID_SME,
						       QDF_MODULE_ID_WMA,
						       QDF_MODULE_ID_WMA,
						       &msg))) {
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
 * @mac_handle: handle returned by mac_open
 * @bssid: mac address of BSS
 * @ch_params: target channel information
 * @profile: CSR profile
 *
 * API to Indicate Channel change to new target channel
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_roam_channel_change_req(mac_handle_t mac_handle,
				       struct qdf_mac_addr bssid,
				       struct ch_params *ch_params,
				       struct csr_roam_profile *profile)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {

		status = csr_roam_channel_change_req(mac, bssid, ch_params,
				profile);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_process_channel_change_resp() -
 * API to Indicate Channel change response message to SAP.
 *
 * Return QDF_STATUS
 */
static QDF_STATUS sme_process_channel_change_resp(struct mac_context *mac,
					   uint16_t msg_type, void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_info *roam_info;
	eCsrRoamResult roamResult;
	tpSwitchChannelParams pChnlParams = (tpSwitchChannelParams) pMsgBuf;
	uint32_t SessionId = pChnlParams->peSessionId;

	roam_info = qdf_mem_malloc(sizeof(*roam_info));
	if (!roam_info)
		return QDF_STATUS_E_NOMEM;

	roam_info->channelChangeRespEvent =
		qdf_mem_malloc(sizeof(tSirChanChangeResponse));
	if (!roam_info->channelChangeRespEvent) {
		status = QDF_STATUS_E_NOMEM;
		qdf_mem_free(roam_info);
		return status;
	}
	if (msg_type == eWNI_SME_CHANNEL_CHANGE_RSP) {
		roam_info->channelChangeRespEvent->sessionId = SessionId;
		roam_info->channelChangeRespEvent->newChannelNumber =
			pChnlParams->channelNumber;

		if (pChnlParams->status == QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "sapdfs: Received success eWNI_SME_CHANNEL_CHANGE_RSP for sessionId[%d]",
				  SessionId);
			roam_info->channelChangeRespEvent->channelChangeStatus =
				1;
			roamResult = eCSR_ROAM_RESULT_CHANNEL_CHANGE_SUCCESS;
		} else {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "sapdfs: Received failure eWNI_SME_CHANNEL_CHANGE_RSP for sessionId[%d]",
				  SessionId);
			roam_info->channelChangeRespEvent->channelChangeStatus =
				0;
			roamResult = eCSR_ROAM_RESULT_CHANNEL_CHANGE_FAILURE;
		}

		csr_roam_call_callback(mac, SessionId, roam_info, 0,
				       eCSR_ROAM_SET_CHANNEL_RSP, roamResult);

	} else {
		status = QDF_STATUS_E_FAILURE;
		sme_err("Invalid Channel Change Resp Message: %d",
			status);
	}
	qdf_mem_free(roam_info->channelChangeRespEvent);
	qdf_mem_free(roam_info);

	return status;
}

/*
 * sme_roam_start_beacon_req() -
 * API to Indicate LIM to start Beacon Tx after SAP CAC Wait is completed.
 *
 * mac_handle - The handle returned by mac_open
 * sessionId - session ID
 * dfsCacWaitStatus - CAC WAIT status flag
 * Return QDF_STATUS
 */
QDF_STATUS sme_roam_start_beacon_req(mac_handle_t mac_handle,
				     struct qdf_mac_addr bssid,
				     uint8_t dfsCacWaitStatus)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_start_beacon_req(mac, bssid,
						dfsCacWaitStatus);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef CONFIG_VDEV_SM
QDF_STATUS sme_csa_restart(struct mac_context *mac_ctx, uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_csa_restart(mac_ctx, session_id);
		sme_release_global_lock(&mac_ctx->sme);
	}

	return status;
}
#endif

/**
 * sme_roam_csa_ie_request() - request CSA IE transmission from PE
 * @mac_handle: handle returned by mac_open
 * @bssid: SAP bssid
 * @targetChannel: target channel information
 * @csaIeReqd: CSA IE Request
 * @ch_params: channel information
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_roam_csa_ie_request(mac_handle_t mac_handle,
				   struct qdf_mac_addr bssid,
				   uint8_t targetChannel, uint8_t csaIeReqd,
				   struct ch_params *ch_params)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_send_chan_sw_ie_request(mac, bssid,
				targetChannel, csaIeReqd, ch_params);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_init_thermal_info() -
 * SME API to initialize the thermal mitigation parameters
 *
 * mac_handle
 * thermalParam : thermal mitigation parameters
 * Return QDF_STATUS
 */
QDF_STATUS sme_init_thermal_info(mac_handle_t mac_handle)
{
	t_thermal_mgmt *pWmaParam;
	struct scheduler_msg msg = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct wlan_fwol_thermal_temp thermal_temp = {0};
	QDF_STATUS status;

	pWmaParam = qdf_mem_malloc(sizeof(t_thermal_mgmt));
	if (!pWmaParam)
		return QDF_STATUS_E_NOMEM;

	status = ucfg_fwol_get_thermal_temp(mac->psoc, &thermal_temp);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	pWmaParam->thermalMgmtEnabled = thermal_temp.thermal_mitigation_enable;
	pWmaParam->throttlePeriod = thermal_temp.throttle_period;

	pWmaParam->throttle_duty_cycle_tbl[0] =
				thermal_temp.throttle_dutycycle_level[0];
	pWmaParam->throttle_duty_cycle_tbl[1] =
				thermal_temp.throttle_dutycycle_level[1];
	pWmaParam->throttle_duty_cycle_tbl[2] =
				thermal_temp.throttle_dutycycle_level[2];
	pWmaParam->throttle_duty_cycle_tbl[3] =
				thermal_temp.throttle_dutycycle_level[3];

	pWmaParam->thermalLevels[0].minTempThreshold =
				thermal_temp.thermal_temp_min_level[0];
	pWmaParam->thermalLevels[0].maxTempThreshold =
				thermal_temp.thermal_temp_max_level[0];
	pWmaParam->thermalLevels[1].minTempThreshold =
				thermal_temp.thermal_temp_min_level[1];
	pWmaParam->thermalLevels[1].maxTempThreshold =
				thermal_temp.thermal_temp_max_level[1];
	pWmaParam->thermalLevels[2].minTempThreshold =
				thermal_temp.thermal_temp_min_level[2];
	pWmaParam->thermalLevels[2].maxTempThreshold =
				thermal_temp.thermal_temp_max_level[2];
	pWmaParam->thermalLevels[3].minTempThreshold =
				thermal_temp.thermal_temp_min_level[3];
	pWmaParam->thermalLevels[3].maxTempThreshold =
				thermal_temp.thermal_temp_max_level[3];

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&mac->sme)) {
		msg.type = WMA_INIT_THERMAL_INFO_CMD;
		msg.bodyptr = pWmaParam;

		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: Not able to post WMA_SET_THERMAL_INFO_CMD to WMA!",
				  __func__);
			qdf_mem_free(pWmaParam);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_SUCCESS;
	}
	qdf_mem_free(pWmaParam);
	return QDF_STATUS_E_FAILURE;
}

/**
 * sme_add_set_thermal_level_callback() - Plug in set thermal level callback
 * @mac_handle:	Handle returned by macOpen
 * @callback:	sme_set_thermal_level_callback
 *
 * Plug in set thermal level callback
 *
 * Return: none
 */
void sme_add_set_thermal_level_callback(mac_handle_t mac_handle,
		sme_set_thermal_level_callback callback)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->sme.set_thermal_level_cb = callback;
}

/**
 * sme_set_thermal_level() - SME API to set the thermal mitigation level
 * @mac_handle: Opaque handle to the global MAC context
 * @level:       Thermal mitigation level
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_level(mac_handle_t mac_handle, uint8_t level)
{
	struct scheduler_msg msg = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&mac->sme)) {
		qdf_mem_zero(&msg, sizeof(msg));
		msg.type = WMA_SET_THERMAL_LEVEL;
		msg.bodyval = level;

		qdf_status =  scheduler_post_message(QDF_MODULE_ID_SME,
						     QDF_MODULE_ID_WMA,
						     QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				   "%s: Not able to post WMA_SET_THERMAL_LEVEL to WMA!",
				   __func__);
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

/*
 * sme_txpower_limit() -
 * SME API to set txpower limits
 *
 * mac_handle
 * psmetx : power limits for 2g/5g
 * Return QDF_STATUS
 */
QDF_STATUS sme_txpower_limit(mac_handle_t mac_handle,
			     struct tx_power_limit *psmetx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct scheduler_msg message = {0};
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct tx_power_limit *tx_power_limit;

	tx_power_limit = qdf_mem_malloc(sizeof(*tx_power_limit));
	if (!tx_power_limit)
		return QDF_STATUS_E_FAILURE;

	*tx_power_limit = *psmetx;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		message.type = WMA_TX_POWER_LIMIT;
		message.reserved = 0;
		message.bodyptr = tx_power_limit;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_TX_POWER_LIMIT",
				  __func__);
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(tx_power_limit);
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

QDF_STATUS sme_update_connect_debug(mac_handle_t mac_handle, uint32_t set_value)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->mlme_cfg->gen.debug_packet_log = set_value;
	return status;
}

/*
 * sme_ap_disable_intra_bss_fwd() -
 * SME will send message to WMA to set Intra BSS in txrx
 *
 * mac_handle - The handle returned by mac_open
 * sessionId - session id ( vdev id)
 * disablefwd - bool value that indicate disable intrabss fwd disable
 * Return QDF_STATUS
 */
QDF_STATUS sme_ap_disable_intra_bss_fwd(mac_handle_t mac_handle,
					uint8_t sessionId,
					bool disablefwd)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	int status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct scheduler_msg message = {0};
	tpDisableIntraBssFwd pSapDisableIntraFwd = NULL;

	/* Prepare the request to send to SME. */
	pSapDisableIntraFwd = qdf_mem_malloc(sizeof(tDisableIntraBssFwd));
	if (!pSapDisableIntraFwd) {
		sme_err("Memory Allocation Failure!!!");
		return QDF_STATUS_E_NOMEM;
	}

	pSapDisableIntraFwd->sessionId = sessionId;
	pSapDisableIntraFwd->disableintrabssfwd = disablefwd;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* serialize the req through MC thread */
		message.bodyptr = pSapDisableIntraFwd;
		message.type = WMA_SET_SAP_INTRABSS_DIS;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(pSapDisableIntraFwd);
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_STATS_EXT

void sme_stats_ext_register_callback(mac_handle_t mac_handle,
				     stats_ext_cb callback)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		sme_err("Invalid mac context");
		return;
	}

	mac->sme.stats_ext_cb = callback;
}

void sme_stats_ext_deregister_callback(mac_handle_t mac_handle)
{
	sme_stats_ext_register_callback(mac_handle, NULL);
}

void sme_stats_ext2_register_callback(mac_handle_t mac_handle,
				      stats_ext2_cb callback)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (!mac) {
		sme_err("Invalid mac context");
		return;
	}

	mac->sme.stats_ext2_cb = callback;
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
	if (!data)
		return QDF_STATUS_E_NOMEM;

	data->vdev_id = session_id;
	data->request_data_len = input->request_data_len;
	if (input->request_data_len)
		qdf_mem_copy(data->request_data,
			     input->request_data, input->request_data_len);

	msg.type = WMA_STATS_EXT_REQUEST;
	msg.reserved = 0;
	msg.bodyptr = data;

	if (QDF_STATUS_SUCCESS != scheduler_post_message(QDF_MODULE_ID_SME,
							 QDF_MODULE_ID_WMA,
							 QDF_MODULE_ID_WMA,
							 &msg)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to post WMA_STATS_EXT_REQUEST message to WMA",
			  __func__);
		qdf_mem_free(data);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_stats_ext_event() - eWNI_SME_STATS_EXT_EVENT processor
 * @mac: Global MAC context
 * @msg: "stats ext" message

 * This callback function called when SME received eWNI_SME_STATS_EXT_EVENT
 * response from WMA
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_stats_ext_event(struct mac_context *mac,
				      struct stats_ext_event *msg)
{
	if (!msg) {
		sme_err("Null msg");
		return QDF_STATUS_E_FAILURE;
	}

	if (mac->sme.stats_ext_cb)
		mac->sme.stats_ext_cb(mac->hdd_handle, msg);

	return QDF_STATUS_SUCCESS;
}

#else

static QDF_STATUS sme_stats_ext_event(struct mac_context *mac,
				      struct stats_ext_event *msg)
{
	return QDF_STATUS_SUCCESS;
}

#endif

#ifdef FEATURE_FW_STATE
QDF_STATUS sme_get_fw_state(mac_handle_t mac_handle,
			    fw_state_callback callback,
			    void *context)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	tp_wma_handle wma_handle;

	SME_ENTER();

	mac_ctx->sme.fw_state_cb = callback;
	mac_ctx->sme.fw_state_context = context;
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	status = wma_get_fw_state(wma_handle);

	SME_EXIT();
	return status;
}

/**
 * sme_fw_state_resp() - eWNI_SME_FW_STATUS_IND processor
 * @mac: Global MAC context

 * This callback function called when SME received eWNI_SME_FW_STATUS_IND
 * response from WMA
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sme_fw_state_resp(struct mac_context *mac)
{
	if (mac->sme.fw_state_cb)
		mac->sme.fw_state_cb(mac->sme.fw_state_context);
	mac->sme.fw_state_cb = NULL;
	mac->sme.fw_state_context = NULL;

	return QDF_STATUS_SUCCESS;
}

#else /* FEATURE_FW_STATE */
static QDF_STATUS sme_fw_state_resp(struct mac_context *mac)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* FEATURE_FW_STATE */

/*
 * sme_update_dfs_scan_mode() -
 * Update DFS roam scan mode
 *	    This function is called through dynamic setConfig callback function
 *	    to configure allowDFSChannelRoam.
 * mac_handle: Opaque handle to the global MAC context
 * sessionId - Session Identifier
 * allowDFSChannelRoam - DFS roaming scan mode 0 (disable),
 *	    1 (passive), 2 (active)
 * Return QDF_STATUS_SUCCESS - SME update DFS roaming scan config
 *	    successfully.
 *	  Other status means SME failed to update DFS roaming scan config.
 */
QDF_STATUS sme_update_dfs_scan_mode(mac_handle_t mac_handle, uint8_t sessionId,
				    uint8_t allowDFSChannelRoam)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (sessionId >= WLAN_MAX_VDEVS) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid sme session id: %d"), sessionId);
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  "LFR runtime successfully set AllowDFSChannelRoam Mode to %d - old value is %d - roam state is %s",
			  allowDFSChannelRoam,
			  mac->mlme_cfg->lfr.roaming_dfs_channel,
			  mac_trace_get_neighbour_roam_state(mac->roam.
							     neighborRoamInfo
							     [sessionId].
							    neighborRoamState));
		mac->mlme_cfg->lfr.roaming_dfs_channel =
			allowDFSChannelRoam;
		if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
			csr_roam_offload_scan(mac, sessionId,
					ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					REASON_ROAM_DFS_SCAN_MODE_CHANGED);
		}
		sme_release_global_lock(&mac->sme);
	}


	return status;
}

/*
 * sme_get_dfs_scan_mode() - get DFS roam scan mode
 *	  This is a synchronous call
 *
 * mac_handle - The handle returned by mac_open.
 * Return DFS roaming scan mode 0 (disable), 1 (passive), 2 (active)
 */
uint8_t sme_get_dfs_scan_mode(mac_handle_t mac_handle)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	return mac->mlme_cfg->lfr.roaming_dfs_channel;
}

/*
 * sme_modify_add_ie() -
 * This function sends msg to updates the additional IE buffers in PE
 *
 * mac_handle - global structure
 * pModifyIE - pointer to tModifyIE structure
 * updateType - type of buffer
 * Return Success or failure
 */
QDF_STATUS sme_modify_add_ie(mac_handle_t mac_handle,
			     tSirModifyIE *pModifyIE, eUpdateIEsType updateType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_modify_add_ies(mac, pModifyIE, updateType);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/*
 * sme_update_add_ie() -
 * This function sends msg to updates the additional IE buffers in PE
 *
 * mac_handle - global structure
 * pUpdateIE - pointer to structure tUpdateIE
 * updateType - type of buffer
 * Return Success or failure
 */
QDF_STATUS sme_update_add_ie(mac_handle_t mac_handle,
			     tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_roam_update_add_ies(mac, pUpdateIE, updateType);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_update_dsc_pto_up_mapping()
 * @mac_handle: Opaque handle to the global MAC context
 * @dscpmapping: pointer to DSCP mapping structure
 * @sessionId: SME session id
 *
 * This routine is called to update dscp mapping
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_dsc_pto_up_mapping(mac_handle_t mac_handle,
					 enum sme_qos_wmmuptype *dscpmapping,
					 uint8_t sessionId)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i, j, peSessionId;
	struct csr_roam_session *pCsrSession = NULL;
	struct pe_session *pSession = NULL;

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	pCsrSession = CSR_GET_SESSION(mac, sessionId);
	if (!pCsrSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Session lookup fails for CSR session"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}
	if (!CSR_IS_SESSION_VALID(mac, sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid session Id %u"), sessionId);
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	pSession = pe_find_session_by_bssid(mac,
				pCsrSession->connectedProfile.bssid.bytes,
				&peSessionId);

	if (!pSession) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL(" Session lookup fails for BSSID"));
		sme_release_global_lock(&mac->sme);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pSession->QosMapSet.present) {
		sme_debug("QOS Mapping IE not present");
		sme_release_global_lock(&mac->sme);
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

	sme_release_global_lock(&mac->sme);
	return status;
}

/*
 * sme_abort_roam_scan() -
 * API to abort current roam scan cycle by roam scan offload module.
 *
 * mac_handle - The handle returned by mac_open.
 * sessionId - Session Identifier
 * Return QDF_STATUS
 */

QDF_STATUS sme_abort_roam_scan(mac_handle_t mac_handle, uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	if (mac->mlme_cfg->lfr.roam_scan_offload_enabled) {
		/* acquire the lock for the sme object */
		status = sme_acquire_global_lock(&mac->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			csr_roam_offload_scan(mac, sessionId,
					      ROAM_SCAN_OFFLOAD_ABORT_SCAN,
					      REASON_ROAM_ABORT_ROAM_SCAN);
			/* release the lock for the sme object */
			sme_release_global_lock(&mac->sme);
		}
	}

	return status;
}

#ifdef FEATURE_WLAN_EXTSCAN
/**
 * sme_get_valid_channels_by_band() - to fetch valid channels filtered by band
 * @mac_handle: Opaque handle to the global MAC context
 * @wifiBand: RF band information
 * @aValidChannels: output array to store channel info
 * @pNumChannels: output number of channels
 *
 *  SME API to fetch all valid channels filtered by band
 *
 *  Return: QDF_STATUS
 */
QDF_STATUS sme_get_valid_channels_by_band(mac_handle_t mac_handle,
					  uint8_t wifiBand,
					  uint32_t *aValidChannels,
					  uint8_t *pNumChannels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t chanList[CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t numChannels = 0;
	uint8_t i = 0;
	uint32_t totValidChannels = CFG_VALID_CHANNEL_LIST_LEN;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

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

QDF_STATUS
sme_ext_scan_get_capabilities(mac_handle_t mac_handle,
			      struct extscan_capabilities_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_capabilities_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;
	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_GET_CAPABILITIES_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS
sme_ext_scan_start(mac_handle_t mac_handle,
		   struct wifi_scan_cmd_req_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct wifi_scan_cmd_req_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;
	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_START_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS sme_ext_scan_stop(mac_handle_t mac_handle,
			     struct extscan_stop_req_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_stop_req_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;
	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_STOP_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS
sme_set_bss_hotlist(mac_handle_t mac_handle,
		    struct extscan_bssid_hotlist_set_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_bssid_hotlist_set_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;

	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_SET_BSSID_HOTLIST_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS
sme_reset_bss_hotlist(mac_handle_t mac_handle,
		      struct extscan_bssid_hotlist_reset_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_bssid_hotlist_reset_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;

	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_RESET_BSSID_HOTLIST_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS
sme_set_significant_change(mac_handle_t mac_handle,
			   struct extscan_set_sig_changereq_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_set_sig_changereq_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;

	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_SET_SIGNF_CHANGE_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}
	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS
sme_reset_significant_change(mac_handle_t mac_handle,
			     struct extscan_capabilities_reset_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_capabilities_reset_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;

	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_RESET_SIGNF_CHANGE_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}
	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}

	return status;
}

QDF_STATUS
sme_get_cached_results(mac_handle_t mac_handle,
		       struct extscan_cached_result_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct extscan_cached_result_params *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;
	*bodyptr = *params;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_EXTSCAN_GET_CACHED_RESULTS_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}
	return status;
}

QDF_STATUS sme_set_epno_list(mac_handle_t mac_handle,
			     struct wifi_enhanced_pno_params *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct wifi_enhanced_pno_params *req_msg;
	int len;

	SME_ENTER();

	/* per contract must make a copy of the params when messaging */
	len = sizeof(*req_msg) +
		(params->num_networks * sizeof(req_msg->networks[0]));

	req_msg = qdf_mem_malloc(len);
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy(req_msg, params, len);

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

QDF_STATUS sme_set_passpoint_list(mac_handle_t mac_handle,
				  struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct wifi_passpoint_req_param *req_msg;
	int len;

	SME_ENTER();

	len = sizeof(*req_msg) +
		(params->num_networks * sizeof(params->networks[0]));
	req_msg = qdf_mem_malloc(len);
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy(req_msg, params, len);

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

QDF_STATUS sme_reset_passpoint_list(mac_handle_t mac_handle,
				    struct wifi_passpoint_req_param *params)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct wifi_passpoint_req_param *req_msg;

	SME_ENTER();

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;
	*req_msg = *params;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)",
			status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);
	return status;
}

QDF_STATUS sme_ext_scan_register_callback(mac_handle_t mac_handle,
					  ext_scan_ind_cb ext_scan_ind_cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.ext_scan_ind_cb = ext_scan_ind_cb;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}
#endif /* FEATURE_WLAN_EXTSCAN */

/**
 * sme_send_wisa_params(): Pass WISA mode to WMA
 * @mac_handle: Opaque handle to the global MAC context
 * @wisa_params: pointer to WISA params struct
 * @sessionId: SME session id
 *
 * Pass WISA params to WMA
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_wisa_params(mac_handle_t mac_handle,
			       struct sir_wisa_params *wisa_params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
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
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

/*
 * sme_ll_stats_clear_req() -
 * SME API to clear Link Layer Statistics
 *
 * mac_handle
 * pclearStatsReq: Link Layer clear stats request params structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ll_stats_clear_req(mac_handle_t mac_handle,
				  tSirLLStatsClearReq *pclearStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tSirLLStatsClearReq *clear_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "staId = %u", pclearStatsReq->staId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "statsClearReqMask = 0x%X",
		  pclearStatsReq->statsClearReqMask);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "stopReq = %u", pclearStatsReq->stopReq);
	if (!sme_is_session_id_valid(mac_handle, pclearStatsReq->staId)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid staId %d",
			  __func__, pclearStatsReq->staId);
		return QDF_STATUS_E_INVAL;
	}

	clear_stats_req = qdf_mem_malloc(sizeof(*clear_stats_req));
	if (!clear_stats_req)
		return QDF_STATUS_E_NOMEM;

	*clear_stats_req = *pclearStatsReq;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&mac->sme)) {
		/* Serialize the req through MC thread */
		message.bodyptr = clear_stats_req;
		message.type = WMA_LINK_LAYER_STATS_CLEAR_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_CLEAR_REQ",
				  __func__);
			qdf_mem_free(clear_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
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
 * mac_handle
 * psetStatsReq: Link Layer set stats request params structure
 * Return QDF_STATUS
 */
QDF_STATUS sme_ll_stats_set_req(mac_handle_t mac_handle, tSirLLStatsSetReq
				*psetStatsReq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tSirLLStatsSetReq *set_stats_req;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s:  MPDU Size = %u", __func__,
		  psetStatsReq->mpduSizeThreshold);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  " Aggressive Stats Collections = %u",
		  psetStatsReq->aggressiveStatisticsGathering);

	set_stats_req = qdf_mem_malloc(sizeof(*set_stats_req));
	if (!set_stats_req)
		return QDF_STATUS_E_NOMEM;

	*set_stats_req = *psetStatsReq;

	if (QDF_STATUS_SUCCESS == sme_acquire_global_lock(&mac->sme)) {
		/* Serialize the req through MC thread */
		message.bodyptr = set_stats_req;
		message.type = WMA_LINK_LAYER_STATS_SET_REQ;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: not able to post WMA_LL_STATS_SET_REQ",
				  __func__);
			qdf_mem_free(set_stats_req);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);
		qdf_mem_free(set_stats_req);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

QDF_STATUS sme_ll_stats_get_req(mac_handle_t mac_handle,
				tSirLLStatsGetReq *get_stats_req,
				void *context)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	tSirLLStatsGetReq *ll_stats_get_req;

	ll_stats_get_req = qdf_mem_malloc(sizeof(*ll_stats_get_req));
	if (!ll_stats_get_req)
		return QDF_STATUS_E_NOMEM;

	*ll_stats_get_req = *get_stats_req;

	mac->sme.ll_stats_context = context;
	if (sme_acquire_global_lock(&mac->sme) == QDF_STATUS_SUCCESS) {
		/* Serialize the req through MC thread */
		message.bodyptr = ll_stats_get_req;
		message.type = WMA_LINK_LAYER_STATS_GET_REQ;
		qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
			  NO_SESSION, message.type);
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("Not able to post WMA_LL_STATS_GET_REQ");
			qdf_mem_free(ll_stats_get_req);
		}
		sme_release_global_lock(&mac->sme);
	} else {
		sme_err("sme_acquire_global_lock error");
		qdf_mem_free(ll_stats_get_req);
	}

	return status;
}

QDF_STATUS sme_set_link_layer_stats_ind_cb(mac_handle_t mac_handle,
					   link_layer_stats_cb callback)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.link_layer_stats_cb = callback;
		sme_release_global_lock(&mac->sme);
	} else {
		sme_err("sme_acquire_global_lock error");
	}

	return status;
}

/**
 * sme_set_link_layer_ext_cb() - Register callback for link layer statistics
 * @mac_handle: Mac global handle
 * @ll_stats_ext_cb: HDD callback which needs to be invoked after getting
 *                   status notification from FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_set_link_layer_ext_cb(mac_handle_t mac_handle,
			  void (*ll_stats_ext_cb)(hdd_handle_t callback_ctx,
						  tSirLLStatsResults *rsp))
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

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
 * @mac_handle: Opaque handle to the global MAC context
 *
 * This function reset's the link layer stats indication
 *
 * Return: QDF_STATUS Enumeration
 */

QDF_STATUS sme_reset_link_layer_stats_ind_cb(mac_handle_t mac_handle)
{
	QDF_STATUS status;
	struct mac_context *pmac;

	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("mac_handle is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	pmac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&pmac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		pmac->sme.link_layer_stats_cb = NULL;
		sme_release_global_lock(&pmac->sme);
	} else
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			"%s: sme_acquire_global_lock error", __func__);

	return status;
}

/**
 * sme_ll_stats_set_thresh - set threshold for mac counters
 * @mac_handle: Opaque handle to the global MAC context
 * @threshold, threshold for mac counters
 *
 * Return: QDF_STATUS Enumeration
 */
QDF_STATUS sme_ll_stats_set_thresh(mac_handle_t mac_handle,
				   struct sir_ll_ext_stats_threshold *threshold)
{
	QDF_STATUS status;
	struct mac_context *mac;
	struct scheduler_msg message = {0};
	struct sir_ll_ext_stats_threshold *thresh;

	if (!threshold) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("threshold is not valid"));
		return QDF_STATUS_E_INVAL;
	}

	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("mac_handle is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = MAC_CONTEXT(mac_handle);

	thresh = qdf_mem_malloc(sizeof(*thresh));
	if (!thresh)
		return QDF_STATUS_E_NOMEM;

	*thresh = *threshold;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = thresh;
		message.type    = WDA_LINK_LAYER_STATS_SET_THRESHOLD;
		MTRACE(qdf_trace(QDF_MODULE_ID_SME, TRACE_CODE_SME_TX_WMA_MSG,
				 NO_SESSION, message.type));
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
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
QDF_STATUS sme_power_debug_stats_req(
		mac_handle_t mac_handle,
		void (*callback_fn)(struct power_stats_response *response,
				    void *context),
		void *power_stats_context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sme_err("not able to post WDA_POWER_DEBUG_STATS_REQ");
		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}
#endif

#ifdef WLAN_FEATURE_BEACON_RECEPTION_STATS
QDF_STATUS sme_beacon_debug_stats_req(
		mac_handle_t mac_handle, uint32_t vdev_id,
		void (*callback_fn)(struct bcn_reception_stats_rsp
				    *response, void *context),
		void *beacon_stats_context)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint32_t *val;
	struct scheduler_msg msg = {0};

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if (!callback_fn) {
			sme_err("Indication callback did not registered");
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_FAILURE;
		}

		if (!mac_ctx->bcn_reception_stats &&
		    !mac_ctx->mlme_cfg->gen.enable_beacon_reception_stats) {
			sme_err("Beacon Reception stats not supported");
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOSUPPORT;
		}

		val = qdf_mem_malloc(sizeof(*val));
		if (!val) {
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}

		*val = vdev_id;
		mac_ctx->sme.beacon_stats_context = beacon_stats_context;
		mac_ctx->sme.beacon_stats_resp_callback = callback_fn;
		msg.bodyptr = val;
		msg.type = WMA_BEACON_DEBUG_STATS_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("not able to post WMA_BEACON_DEBUG_STATS_REQ");
			qdf_mem_free(val);
		}
		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sme_update_roam_key_mgmt_offload_enabled() - enable/disable key mgmt offload
 * This is a synchronous call
 * @mac_handle: The handle returned by mac_open.
 * @session_id: Session Identifier
 * @key_mgmt_offload_enabled: key mgmt enable/disable flag
 * @pmkid_modes: PMKID modes of PMKSA caching and OKC
 * Return: QDF_STATUS_SUCCESS - SME updated config successfully.
 * Other status means SME is failed to update.
 */

QDF_STATUS sme_update_roam_key_mgmt_offload_enabled(mac_handle_t mac_handle,
					uint8_t session_id,
					bool key_mgmt_offload_enabled,
					struct pmkid_mode_bits *pmkid_modes)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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

/**
 * sme_get_temperature() - SME API to get the pdev temperature
 * @mac_handle: Handle to global MAC context
 * @cb_context: temperature callback context
 * @cb: callback function with response (temperature)
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_temperature(mac_handle_t mac_handle,
			       void *cb_context,
			       void (*cb)(int temperature,
					  void *context))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		if ((!cb) &&
		    (!mac->sme.temperature_cb)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"Indication Call back did not registered");
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_FAILURE;
		} else if (cb) {
			mac->sme.temperature_cb_context = cb_context;
			mac->sme.temperature_cb = cb;
		}
		/* serialize the req through MC thread */
		message.bodyptr = NULL;
		message.type = WMA_GET_TEMPERATURE_REQ;
		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("Post Get Temperature msg fail"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

QDF_STATUS sme_set_scanning_mac_oui(mac_handle_t mac_handle,
				    struct scan_mac_oui *scan_mac_oui)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct scan_mac_oui *bodyptr;

	/* per contract must make a copy of the params when messaging */
	bodyptr = qdf_mem_malloc(sizeof(*bodyptr));
	if (!bodyptr)
		return QDF_STATUS_E_NOMEM;
	*bodyptr = *scan_mac_oui;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		message.bodyptr = bodyptr;
		message.type = WMA_SET_SCAN_MAC_OUI_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&message);
		sme_release_global_lock(&mac->sme);
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		sme_err("failure: %d", status);
		qdf_mem_free(bodyptr);
	}

	return status;
}

#ifdef DHCP_SERVER_OFFLOAD
QDF_STATUS
sme_set_dhcp_srv_offload(mac_handle_t mac_handle,
			 struct dhcp_offload_info_params *dhcp_srv_info)
{
	struct scheduler_msg message = {0};
	struct dhcp_offload_info_params *payload;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	payload = qdf_mem_malloc(sizeof(*payload));
	if (!payload)
		return QDF_STATUS_E_NOMEM;

	*payload = *dhcp_srv_info;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* serialize the req through MC thread */
		message.type = WMA_SET_DHCP_SERVER_OFFLOAD_CMD;
		message.bodyptr = payload;

		if (!QDF_IS_STATUS_SUCCESS
			    (scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &message))) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s:WMA_SET_DHCP_SERVER_OFFLOAD_CMD failed",
				  __func__);
			qdf_mem_free(payload);
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_acquire_global_lock error!", __func__);
		qdf_mem_free(payload);
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
 * mac_handle - The handle returned by mac_open.
 * x0, x1 -  led flashing parameters
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_led_flashing(mac_handle_t mac_handle, uint8_t type,
				uint32_t x0, uint32_t x1)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct flashing_req_params *ledflashing;

	ledflashing = qdf_mem_malloc(sizeof(*ledflashing));
	if (!ledflashing)
		return QDF_STATUS_E_NOMEM;

	ledflashing->req_id = 0;
	ledflashing->pattern_id = type;
	ledflashing->led_x0 = x0;
	ledflashing->led_x1 = x1;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		/* Serialize the req through MC thread */
		message.bodyptr = ledflashing;
		message.type = WMA_LED_FLASHING_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &message);
		sme_release_global_lock(&mac->sme);
	}
	if (!QDF_IS_STATUS_SUCCESS(status))
		qdf_mem_free(ledflashing);

	return status;
}
#endif

/**
 *  sme_enable_dfS_chan_scan() - set DFS channel scan enable/disable
 *  @mac_handle:         corestack handler
 *  @dfs_flag:      flag indicating dfs channel enable/disable
 *  Return:         QDF_STATUS
 */
QDF_STATUS sme_enable_dfs_chan_scan(mac_handle_t mac_handle, uint8_t dfs_flag)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac;

	if (!mac_handle) {
		sme_err("mac_handle is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mac = MAC_CONTEXT(mac_handle);

	mac->scan.fEnableDFSChnlScan = dfs_flag;

	return status;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * sme_validate_sap_channel_switch() - validate target channel switch w.r.t
 *      concurreny rules set to avoid channel interference.
 * @mac_handle: Opaque handle to the global MAC context
 * @sap_ch - channel to switch
 * @sap_phy_mode - phy mode of SAP
 * @cc_switch_mode - concurreny switch mode
 * @session_id - sme session id.
 *
 * Return: true if there is no channel interference else return false
 */
bool sme_validate_sap_channel_switch(mac_handle_t mac_handle,
				     uint16_t sap_ch,
				     eCsrPhyMode sap_phy_mode,
				     uint8_t cc_switch_mode,
				     uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
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
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session ID
 * @stats_avg_factor: average stats factor
 *
 * This function configures the stats avg factor in firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_stats_avg_factor(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint16_t stats_avg_factor)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac  = MAC_CONTEXT(mac_handle);
	struct sir_stats_avg_factor *stats_factor;

	stats_factor = qdf_mem_malloc(sizeof(*stats_factor));
	if (!stats_factor)
		return QDF_STATUS_E_NOMEM;

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		stats_factor->vdev_id = session_id;
		stats_factor->stats_avg_factor = stats_avg_factor;

		/* serialize the req through MC thread */
		msg.type     = SIR_HAL_CONFIG_STATS_FACTOR;
		msg.bodyptr  = stats_factor;

		if (!QDF_IS_STATUS_SUCCESS(
			    scheduler_post_message(QDF_MODULE_ID_SME,
						   QDF_MODULE_ID_WMA,
						   QDF_MODULE_ID_WMA, &msg))) {
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
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 * @guard_time: guard time
 *
 * This function configures the guard time in firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_configure_guard_time(mac_handle_t mac_handle, uint8_t session_id,
				    uint32_t guard_time)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac  = MAC_CONTEXT(mac_handle);
	struct sir_guard_time_request *g_time;

	g_time = qdf_mem_malloc(sizeof(*g_time));
	if (!g_time)
		return QDF_STATUS_E_NOMEM;

	status = sme_acquire_global_lock(&mac->sme);

	if (QDF_STATUS_SUCCESS == status) {

		g_time->vdev_id = session_id;
		g_time->guard_time = guard_time;

		/* serialize the req through MC thread */
		msg.type     = SIR_HAL_CONFIG_GUARD_TIME;
		msg.bodyptr  = g_time;

		if (!QDF_IS_STATUS_SUCCESS(
			    scheduler_post_message(QDF_MODULE_ID_SME,
						   QDF_MODULE_ID_WMA,
						   QDF_MODULE_ID_WMA, &msg))) {
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

/*
 * sme_wifi_start_logger() - Send the start/stop logging command to WMA
 * to either start/stop logging
 * @mac_handle: Opaque handle to the global MAC context
 * @start_log: Structure containing the wifi start logger params
 *
 * This function sends the start/stop logging command to WMA
 *
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_wifi_start_logger(mac_handle_t mac_handle,
				 struct sir_wifi_start_log start_log)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac   = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct sir_wifi_start_log *req_msg;
	uint32_t len;

	len = sizeof(*req_msg);
	req_msg = qdf_mem_malloc(len);
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
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
 * @mac_handle:                Handle returned by macOpen
 * @sessionId: sessionId of the STA session
 *
 * This function is a wrapper to call
 * csr_neighbor_middle_of_roaming to know STA is in the
 * middle of roaming states
 *
 * Return: True or False
 *
 */
bool sme_neighbor_middle_of_roaming(mac_handle_t mac_handle, uint8_t sessionId)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	bool val = false;

	if (CSR_IS_SESSION_VALID(mac_ctx, sessionId))
		val = csr_neighbor_middle_of_roaming(mac_ctx, sessionId);
	else
		sme_debug("Invalid Session: %d", sessionId);

	return val;
}

bool sme_is_any_session_in_middle_of_roaming(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t session_id;

	for (session_id = 0; session_id < WLAN_MAX_VDEVS; session_id++) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id) &&
		    csr_neighbor_middle_of_roaming(mac_ctx, session_id))
			return true;
	}

	return false;
}

/*
 * sme_send_flush_logs_cmd_to_fw() - Flush FW logs
 *
 * This function is used to send the command that will
 * be used to flush the logs in the firmware
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_flush_logs_cmd_to_fw(void)
{
	QDF_STATUS status;
	struct scheduler_msg message = {0};

	/* Serialize the req through MC thread */
	message.bodyptr = NULL;
	message.type    = SIR_HAL_FLUSH_LOG_TO_FW;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
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
 * @mac_handle: Handle returned by mac open
 * @nss: Number of spatial streams
 *
 * This function is used to update the number of spatial streams supported.
 *
 * Return: Success upon successfully changing nss else failure
 *
 */
QDF_STATUS sme_update_nss(mac_handle_t mac_handle, uint8_t nss)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint32_t i;
	struct mlme_ht_capabilities_info *ht_cap_info;
	struct csr_roam_session *csr_session;
	struct mlme_vht_capabilities_info *vht_cap_info;

	vht_cap_info = &mac_ctx->mlme_cfg->vht_caps.vht_cap_info;

	status = sme_acquire_global_lock(&mac_ctx->sme);

	if (QDF_STATUS_SUCCESS == status) {
		vht_cap_info->enable2x2 = (nss == 1) ? 0 : 1;

		/* get the HT capability info*/
		ht_cap_info = &mac_ctx->mlme_cfg->ht_caps.ht_cap_info;

		for (i = 0; i < WLAN_MAX_VDEVS; i++) {
			if (CSR_IS_SESSION_VALID(mac_ctx, i)) {
				csr_session = &mac_ctx->roam.roamSession[i];
				csr_session->ht_config.ht_tx_stbc =
					ht_cap_info->tx_stbc;
			}
		}

		sme_release_global_lock(&mac_ctx->sme);
	}
	return status;
}

/**
 * sme_update_user_configured_nss() - sets the nss based on user request
 * @mac_handle: Opaque handle to the global MAC context
 * @nss: number of streams
 *
 * Return: None
 */
void sme_update_user_configured_nss(mac_handle_t mac_handle, uint8_t nss)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->user_configured_nss = nss;
}

int sme_update_tx_bfee_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->mlme_cfg->vht_caps.vht_cap_info.su_bformee = cfg_val;

	return sme_update_he_tx_bfee_supp(mac_handle, session_id, cfg_val);
}

int sme_update_tx_bfee_nsts(mac_handle_t mac_handle, uint8_t session_id,
			    uint8_t usr_cfg_val, uint8_t nsts_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t nsts_set_val;
	struct mlme_vht_capabilities_info *vht_cap_info;

	vht_cap_info = &mac_ctx->mlme_cfg->vht_caps.vht_cap_info;
	mac_ctx->usr_cfg_tx_bfee_nsts = usr_cfg_val;
	if (usr_cfg_val)
		nsts_set_val = usr_cfg_val;
	else
		nsts_set_val = nsts_val;

	vht_cap_info->tx_bfee_ant_supp = nsts_set_val;

	if (usr_cfg_val)
		sme_set_he_tx_bf_cbf_rates(session_id);

	return sme_update_he_tx_bfee_nsts(mac_handle, session_id, nsts_set_val);
}
#ifdef WLAN_FEATURE_11AX
void sme_update_tgt_he_cap(mac_handle_t mac_handle,
			   struct wma_tgt_cfg *cfg,
			   tDot11fIEhe_cap *he_cap_ini)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	qdf_mem_copy(&mac_ctx->he_cap_2g,
		     &cfg->he_cap_2g,
		     sizeof(tDot11fIEhe_cap));

	qdf_mem_copy(&mac_ctx->he_cap_5g,
		     &cfg->he_cap_5g,
		     sizeof(tDot11fIEhe_cap));

	/* modify HE Caps field according to INI setting */
	mac_ctx->he_cap_2g.bfee_sts_lt_80 =
			QDF_MIN(cfg->he_cap_2g.bfee_sts_lt_80,
				he_cap_ini->bfee_sts_lt_80);

	mac_ctx->he_cap_5g.bfee_sts_lt_80 =
			QDF_MIN(cfg->he_cap_5g.bfee_sts_lt_80,
				he_cap_ini->bfee_sts_lt_80);
}

void sme_update_he_cap_nss(mac_handle_t mac_handle, uint8_t session_id,
			   uint8_t nss)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *csr_session;
	uint32_t tx_mcs_map = 0;
	uint32_t rx_mcs_map = 0;
	uint32_t mcs_map = 0;

	if (!nss || (nss > 2)) {
		sme_err("invalid Nss value %d", nss);
	}
	csr_session = CSR_GET_SESSION(mac_ctx, session_id);
	rx_mcs_map =
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.rx_he_mcs_map_lt_80;
	tx_mcs_map =
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.tx_he_mcs_map_lt_80;
	mcs_map = rx_mcs_map & 0x3;

	if (nss == 1) {
		tx_mcs_map = HE_SET_MCS_4_NSS(tx_mcs_map, HE_MCS_DISABLE, 2);
		rx_mcs_map = HE_SET_MCS_4_NSS(rx_mcs_map, HE_MCS_DISABLE, 2);
	} else {
		tx_mcs_map = HE_SET_MCS_4_NSS(tx_mcs_map, mcs_map, 2);
		rx_mcs_map = HE_SET_MCS_4_NSS(rx_mcs_map, mcs_map, 2);
	}
	sme_info("new HE Nss MCS MAP: Rx 0x%0X, Tx: 0x%0X",
			rx_mcs_map, tx_mcs_map);
	if (cfg_in_range(CFG_HE_RX_MCS_MAP_LT_80, rx_mcs_map))
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.rx_he_mcs_map_lt_80 =
		rx_mcs_map;
	if (cfg_in_range(CFG_HE_TX_MCS_MAP_LT_80, tx_mcs_map))
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.tx_he_mcs_map_lt_80 =
		tx_mcs_map;
	csr_update_session_he_cap(mac_ctx, csr_session);

}

int sme_update_he_mcs(mac_handle_t mac_handle, uint8_t session_id,
		      uint16_t he_mcs)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *csr_session;
	uint16_t mcs_val = 0;
	uint16_t mcs_map = HE_MCS_ALL_DISABLED;

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
		if (mac_ctx->mlme_cfg->vht_caps.vht_cap_info.enable2x2) {
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 2);
		} else {
			mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		}
		if (cfg_in_range(CFG_HE_TX_MCS_MAP_LT_80, mcs_map))
			mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
			tx_he_mcs_map_lt_80 = mcs_map;
		if (cfg_in_range(CFG_HE_RX_MCS_MAP_LT_80, mcs_map))
			mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
			rx_he_mcs_map_lt_80 = mcs_map;
		break;

	case HE_160_MCS0_7:
	case HE_160_MCS0_9:
	case HE_160_MCS0_11:
		mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		if (cfg_in_range(CFG_HE_TX_MCS_MAP_160, mcs_map))
			qdf_mem_copy(mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
				     tx_he_mcs_map_160, &mcs_map,
				     sizeof(uint16_t));
		if (cfg_in_range(CFG_HE_RX_MCS_MAP_160, mcs_map))
			qdf_mem_copy(mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
				     rx_he_mcs_map_160, &mcs_map,
				     sizeof(uint16_t));
		break;

	case HE_80p80_MCS0_7:
	case HE_80p80_MCS0_9:
	case HE_80p80_MCS0_11:
		mcs_map = HE_SET_MCS_4_NSS(mcs_map, mcs_val, 1);
		if (cfg_in_range(CFG_HE_TX_MCS_MAP_80_80, mcs_map))
			qdf_mem_copy(mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
				     tx_he_mcs_map_80_80, &mcs_map,
				     sizeof(uint16_t));
		if (cfg_in_range(CFG_HE_RX_MCS_MAP_80_80, mcs_map))
			qdf_mem_copy(mac_ctx->mlme_cfg->he_caps.dot11_he_cap.
				     rx_he_mcs_map_80_80, &mcs_map,
				     sizeof(uint16_t));
		break;

	default:
		sme_err("Invalid HE MCS 0x%0x", he_mcs);
		return -EINVAL;
	}
	sme_info("new HE MCS 0x%0x", mcs_map);
	csr_update_session_he_cap(mac_ctx, csr_session);

	return 0;
}

void sme_set_usr_cfg_mu_edca(mac_handle_t mac_handle, bool val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->usr_cfg_mu_edca_params = val;
}

int sme_update_mu_edca_params(mac_handle_t mac_handle, uint8_t session_id)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	qdf_mem_zero(&msg, sizeof(msg));
	msg.type = WNI_SME_UPDATE_MU_EDCA_PARAMS;
	msg.reserved = 0;
	msg.bodyval = session_id;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("Not able to post update edca profile");
		return -EIO;
	}

	return 0;
}

void sme_set_he_mu_edca_def_cfg(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t i;

	sme_debug("Set MU EDCA params to default");
	for (i = 0; i < QCA_WLAN_AC_ALL; i++) {
		mac_ctx->usr_mu_edca_params[i].aci.aifsn = MU_EDCA_DEF_AIFSN;
		mac_ctx->usr_mu_edca_params[i].aci.aci = i;
		mac_ctx->usr_mu_edca_params[i].cw.max = MU_EDCA_DEF_CW_MAX;
		mac_ctx->usr_mu_edca_params[i].cw.min = MU_EDCA_DEF_CW_MIN;
		mac_ctx->usr_mu_edca_params[i].mu_edca_timer =
							MU_EDCA_DEF_TIMER;
	}
}

int sme_update_he_tx_bfee_supp(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (cfg_val <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.su_beamformee = cfg_val;
	else
		return -EINVAL;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;
}

int sme_update_he_trigger_frm_mac_pad(mac_handle_t mac_handle,
				      uint8_t session_id,
				      uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (cfg_in_range(CFG_HE_TRIG_PAD, cfg_val))
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.trigger_frm_mac_pad =
		cfg_val;
	else
		return -EINVAL;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;

}

int sme_update_he_om_ctrl_supp(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val)
{

	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.omi_a_ctrl = cfg_val;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;
}

int sme_update_he_htc_he_supp(mac_handle_t mac_handle, uint8_t session_id,
			      bool cfg_val)
{

	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}

	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.htc_he = cfg_val;
	csr_update_session_he_cap(mac_ctx, session);

	return 0;
}

static QDF_STATUS
sme_validate_session_for_cap_update(struct mac_context *mac_ctx,
				    uint8_t session_id,
				    struct csr_roam_session *session)
{
	if (!session) {
		sme_err("Session does not exist, Session_id: %d", session_id);
		return QDF_STATUS_E_INVAL;
	}
	if (!csr_is_conn_state_connected_infra(mac_ctx, session_id)) {
		sme_info("STA is not connected, Session_id: %d", session_id);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

int sme_send_he_om_ctrl_update(mac_handle_t mac_handle, uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct omi_ctrl_tx omi_data = {0};
	void *wma_handle;
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);
	uint32_t param_val = 0;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sme_err("wma handle is NULL");
		return -EIO;
	}

	status = sme_validate_session_for_cap_update(mac_ctx, session_id,
						     session);
	if (QDF_IS_STATUS_ERROR(status))
		return -EINVAL;

	omi_data.a_ctrl_id = A_CTRL_ID_OMI;

	if (mac_ctx->he_om_ctrl_cfg_nss_set)
		omi_data.rx_nss = mac_ctx->he_om_ctrl_cfg_nss;
	else
		omi_data.rx_nss = session->nss - 1;

	if (mac_ctx->he_om_ctrl_cfg_tx_nsts_set)
		omi_data.tx_nsts = mac_ctx->he_om_ctrl_cfg_tx_nsts;
	else
		omi_data.tx_nsts = session->nss - 1;

	if (mac_ctx->he_om_ctrl_cfg_bw_set)
		omi_data.ch_bw = mac_ctx->he_om_ctrl_cfg_bw;
	else
		omi_data.ch_bw = session->connectedProfile.vht_channel_width;

	omi_data.ul_mu_dis = mac_ctx->he_om_ctrl_cfg_ul_mu_dis;
	omi_data.ul_mu_data_dis = mac_ctx->he_om_ctrl_ul_mu_data_dis;
	omi_data.omi_in_vht = 0x1;
	omi_data.omi_in_he = 0x1;

	sme_info("OMI: BW %d TxNSTS %d RxNSS %d ULMU %d, OMI_VHT %d, OMI_HE %d",
		 omi_data.ch_bw, omi_data.tx_nsts, omi_data.rx_nss,
		 omi_data.ul_mu_dis, omi_data.omi_in_vht, omi_data.omi_in_he);
	qdf_mem_copy(&param_val, &omi_data, sizeof(omi_data));
	sme_debug("param val %08X, bssid:"MAC_ADDRESS_STR, param_val,
		  MAC_ADDR_ARRAY(session->connectedProfile.bssid.bytes));
	status = wma_set_peer_param(wma_handle,
				    session->connectedProfile.bssid.bytes,
				    WMI_PEER_PARAM_XMIT_OMI,
				    param_val, session_id);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("set_peer_param_cmd returned %d", status);
		return -EIO;
	}

	return 0;
}

int sme_set_he_om_ctrl_param(mac_handle_t mac_handle, uint8_t session_id,
			     enum qca_wlan_vendor_attr_he_omi_tx param,
			     uint8_t cfg_val)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);

	status = sme_validate_session_for_cap_update(mac_ctx, session_id,
						     session);
	if (QDF_IS_STATUS_ERROR(status))
		return -EINVAL;

	switch(param) {
		case QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DISABLE:
			sme_debug("Set OM ctrl UL MU dis to %d", cfg_val);
			mac_ctx->he_om_ctrl_cfg_ul_mu_dis = cfg_val;
			break;
		case QCA_WLAN_VENDOR_ATTR_HE_OMI_RX_NSS:
			if ((cfg_val + 1)  > session->nss) {
				sme_debug("OMI Nss %d is > connected Nss %d",
					  cfg_val, session->nss);
				mac_ctx->he_om_ctrl_cfg_nss_set = false;
				return 0;
			}
			sme_debug("Set OM ctrl Rx Nss cfg to %d", cfg_val);
			mac_ctx->he_om_ctrl_cfg_nss_set = true;
			mac_ctx->he_om_ctrl_cfg_nss = cfg_val;
			break;
		case QCA_WLAN_VENDOR_ATTR_HE_OMI_CH_BW:
			if (cfg_val >
			    session->connectedProfile.vht_channel_width) {
				sme_info("OMI BW %d is > connected BW %d",
					 cfg_val,
					 session->connectedProfile.
					 vht_channel_width);
				mac_ctx->he_om_ctrl_cfg_bw_set = false;
				return 0;
			}
			sme_debug("Set OM ctrl BW cfg to %d", cfg_val);
			mac_ctx->he_om_ctrl_cfg_bw_set = true;
			mac_ctx->he_om_ctrl_cfg_bw = cfg_val;
			break;
		case QCA_WLAN_VENDOR_ATTR_HE_OMI_TX_NSTS:
			if ((cfg_val + 1) > session->nss) {
				sme_debug("OMI NSTS %d is > connected Nss %d",
					  cfg_val, session->nss);
				mac_ctx->he_om_ctrl_cfg_tx_nsts_set = false;
				return 0;
			}
			sme_debug("Set OM ctrl tx nsts cfg to %d", cfg_val);
			mac_ctx->he_om_ctrl_cfg_tx_nsts_set = true;
			mac_ctx->he_om_ctrl_cfg_tx_nsts = cfg_val;
			break;
		case QCA_WLAN_VENDOR_ATTR_HE_OMI_ULMU_DATA_DISABLE:
			sme_debug("Set OM ctrl UL MU data dis to %d", cfg_val);
			mac_ctx->he_om_ctrl_ul_mu_data_dis = cfg_val;
			break;
		default:
			sme_debug("Invalid OMI param %d", param);
			return -EINVAL;
	}

	return 0;
}

void sme_reset_he_om_ctrl(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->he_om_ctrl_cfg_bw_set = false;
	mac_ctx->he_om_ctrl_cfg_nss_set = false;
	mac_ctx->he_om_ctrl_cfg_bw = 0;
	mac_ctx->he_om_ctrl_cfg_nss = 0;
	mac_ctx->he_om_ctrl_cfg_ul_mu_dis = false;
	mac_ctx->he_om_ctrl_cfg_tx_nsts_set = false;
	mac_ctx->he_om_ctrl_cfg_tx_nsts = 0;
	mac_ctx->he_om_ctrl_ul_mu_data_dis = false;
}

int sme_config_action_tx_in_tb_ppdu(mac_handle_t mac_handle, uint8_t session_id,
				    uint8_t cfg_val)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};
	struct sir_cfg_action_frm_tb_ppdu *cfg_msg;

	if (!csr_is_conn_state_connected_infra(mac_ctx, session_id)) {
		sme_info("STA not in connected state Session_id: %d",
			 session_id);
		return -EINVAL;
	}

	cfg_msg = qdf_mem_malloc(sizeof(*cfg_msg));

	if (!cfg_msg)
		return -EIO;

	cfg_msg->type = WNI_SME_CFG_ACTION_FRM_HE_TB_PPDU;
	cfg_msg->session_id = session_id;
	cfg_msg->cfg = cfg_val;

	msg.bodyptr = cfg_msg;
	msg.type = WNI_SME_CFG_ACTION_FRM_HE_TB_PPDU;
	status = scheduler_post_message(QDF_MODULE_ID_SME, QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("Failed to send CFG_ACTION_FRAME_IN_TB_PPDU to PE %d",
			status);
		qdf_mem_free(cfg_msg);
		return -EIO;
	}

	return 0;
}

int sme_update_he_tx_bfee_nsts(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (cfg_in_range(CFG_HE_BFEE_STS_LT80, cfg_val))
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.bfee_sts_lt_80 =
		cfg_val;
	else
		return -EINVAL;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;
}

void sme_set_he_tx_bf_cbf_rates(uint8_t session_id)
{
	uint32_t tx_bf_cbf_rates_5g[] = {91, 1, 0, 3, 2, 4, 0};
	uint32_t tx_bf_cbf_rates_2g[] = {91, 1, 1, 3, 1, 3, 0};
	QDF_STATUS status;

	status = wma_form_unit_test_cmd_and_send(session_id, 0x48, 7,
						 tx_bf_cbf_rates_5g);
	if (QDF_STATUS_SUCCESS != status)
		sme_err("send_unit_test_cmd returned %d", status);

	status = wma_form_unit_test_cmd_and_send(session_id, 0x48, 7,
						 tx_bf_cbf_rates_2g);
	if (QDF_STATUS_SUCCESS != status)
		sme_err("send_unit_test_cmd returned %d", status);
}

void sme_config_su_ppdu_queue(uint8_t session_id, bool enable)
{
	uint32_t su_ppdu_enable[] = {69, 1, 1, 1};
	uint32_t su_ppdu_disable[] = {69, 1, 1, 0};
	QDF_STATUS status;

	if (enable) {
		sme_debug("Send Tx SU PPDU queue ENABLE cmd to FW");
		status = wma_form_unit_test_cmd_and_send(session_id, 0x48, 4,
							 su_ppdu_enable);
	} else {
		sme_debug("Send Tx SU PPDU queue DISABLE cmd to FW");
		status = wma_form_unit_test_cmd_and_send(session_id, 0x48, 4,
							 su_ppdu_disable);
	}
	if (QDF_STATUS_SUCCESS != status)
		sme_err("send_unit_test_cmd returned %d", status);
}

int sme_update_he_tx_stbc_cap(mac_handle_t mac_handle, uint8_t session_id,
			      int value)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;
	uint32_t he_cap_val = 0;

	he_cap_val = value ? 1 : 0;
	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (he_cap_val <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.tb_ppdu_tx_stbc_lt_80mhz
			= he_cap_val;
	else
		return -EINVAL;
	if (he_cap_val <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.tb_ppdu_tx_stbc_gt_80mhz
			= he_cap_val;
	else
		return -EINVAL;
	csr_update_session_he_cap(mac_ctx, session);
	return 0;
}

int sme_update_he_rx_stbc_cap(mac_handle_t mac_handle, uint8_t session_id,
			      int value)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;
	uint32_t he_cap_val = 0;

	he_cap_val = value ? 1 : 0;
	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (he_cap_val <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.rx_stbc_lt_80mhz =
		he_cap_val;
	else
		return -EINVAL;
	if (he_cap_val <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.rx_stbc_gt_80mhz =
		he_cap_val;
	else
		return -EINVAL;
	csr_update_session_he_cap(mac_ctx, session);
	return 0;
}

int sme_update_he_frag_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint16_t he_frag)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (cfg_in_range(CFG_HE_FRAGMENTATION, he_frag))
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.fragmentation = he_frag;
	else
		return -EINVAL;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;

}

int sme_update_he_ldpc_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint16_t he_ldpc)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	if (he_ldpc <= 1)
		mac_ctx->mlme_cfg->he_caps.dot11_he_cap.ldpc_coding = he_ldpc;
	else
		return -EINVAL;

	csr_update_session_he_cap(mac_ctx, session);
	return 0;

}

int sme_update_he_twt_req_support(mac_handle_t mac_handle, uint8_t session_id,
				  uint8_t cfg_val)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("No session for id %d", session_id);
		return -EINVAL;
	}
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.twt_request = cfg_val;

	csr_update_session_he_cap(mac_ctx, session);

	return 0;
}
#endif

/**
 * sme_set_nud_debug_stats_cb() - set nud debug stats callback
 * @mac_handle: Opaque handle to the global MAC context
 * @cb: callback function pointer
 * @context: callback context
 *
 * This function stores nud debug stats callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_nud_debug_stats_cb(mac_handle_t mac_handle,
				void (*cb)(void *, struct rsp_stats *, void *),
				void *context)
{
	QDF_STATUS status  = QDF_STATUS_SUCCESS;
	struct mac_context *mac;

	if (!mac_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("mac_handle is not valid"));
		return QDF_STATUS_E_INVAL;
	}
	mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed!(status=%d)"),
			status);
		return status;
	}

	mac->sme.get_arp_stats_cb = cb;
	mac->sme.get_arp_stats_context = context;
	sme_release_global_lock(&mac->sme);
	return status;
}

/**
 * sme_is_any_session_in_connected_state() - SME wrapper API to
 * check if any session is in connected state or not.
 *
 * @mac_handle: Handle returned by mac open
 *
 * This function is used to check if any valid sme session is in
 * connected state or not.
 *
 * Return: true if any session is connected, else false.
 *
 */
bool sme_is_any_session_in_connected_state(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;
	bool ret = false;

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		ret = csr_is_any_session_in_connect_state(mac_ctx);
		sme_release_global_lock(&mac_ctx->sme);
	}
	return ret;
}

QDF_STATUS sme_set_chip_pwr_save_fail_cb(mac_handle_t mac_handle,
					 pwr_save_fail_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme_AcquireGlobalLock failed!(status=%d)", status);
		return status;
	}
	mac->sme.chip_power_save_fail_cb = cb;
	sme_release_global_lock(&mac->sme);
	return status;
}

#ifdef FEATURE_RSSI_MONITOR
/**
 * sme_set_rssi_monitoring() - set rssi monitoring
 * @mac_handle: Opaque handle to the global MAC context
 * @input: request message
 *
 * This function constructs the vos message and fill in message type,
 * bodyptr with @input and posts it to WDA queue.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_rssi_monitoring(mac_handle_t mac_handle,
				   struct rssi_monitor_param *input)
{
	QDF_STATUS status     = QDF_STATUS_SUCCESS;
	struct mac_context *mac    = MAC_CONTEXT(mac_handle);
	struct scheduler_msg message = {0};
	struct rssi_monitor_param *req_msg;

	SME_ENTER();
	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("scheduler_post_msg failed!(err=%d)", status);
		qdf_mem_free(req_msg);
	}
	sme_release_global_lock(&mac->sme);

	return status;
}

QDF_STATUS sme_set_rssi_threshold_breached_cb(mac_handle_t mac_handle,
					      rssi_threshold_breached_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac;

	mac = MAC_CONTEXT(mac_handle);
	if (!mac) {
		sme_err("Invalid mac context");
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac->sme);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("sme_acquire_global_lock failed!(status=%d)",
			status);
		return status;
	}

	mac->sme.rssi_threshold_breached_cb = cb;
	sme_release_global_lock(&mac->sme);
	return status;
}
#endif /* FEATURE_RSSI_MONITOR */

QDF_STATUS sme_reset_rssi_threshold_breached_cb(mac_handle_t mac_handle)
{
	return sme_set_rssi_threshold_breached_cb(mac_handle, NULL);
}

static enum band_info sme_get_connected_roaming_vdev_band(void)
{
	enum band_info band = BAND_ALL;
	struct mac_context *mac = sme_get_mac_context();
	struct csr_roam_session *session;
	uint8_t session_id, channel;

	if (!mac) {
		sme_debug("MAC Context is NULL");
		return band;
	}
	session_id = csr_get_roam_enabled_sta_sessionid(mac);
	if (session_id != WLAN_UMAC_VDEV_ID_MAX) {
		session = CSR_GET_SESSION(mac, session_id);
		channel = session->connectedProfile.operationChannel;
		band = csr_get_rf_band(channel);
		return band;
	}

	return band;
}

/*
 * sme_pdev_set_pcl() - Send WMI_PDEV_SET_PCL_CMDID to the WMA
 * @mac_handle: Handle returned by macOpen
 * @msg: PCL channel list and length structure
 *
 * Sends the command to WMA to send WMI_PDEV_SET_PCL_CMDID to FW
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_pdev_set_pcl(struct policy_mgr_pcl_list *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac   = sme_get_mac_context();
	struct scheduler_msg message = {0};
	struct set_pcl_req *req_msg;
	uint32_t i;

	if (!mac) {
		sme_err("mac is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!msg) {
		sme_err("msg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

	req_msg->band = BAND_ALL;
	if (CSR_IS_ROAM_INTRA_BAND_ENABLED(mac)) {
		req_msg->band = sme_get_connected_roaming_vdev_band();
		sme_debug("Connected STA band %d", req_msg->band);
	}
	for (i = 0; i < msg->pcl_len; i++) {
		req_msg->chan_weights.pcl_list[i] =  msg->pcl_list[i];
		req_msg->chan_weights.weight_list[i] =  msg->weight_list[i];
	}

	req_msg->chan_weights.pcl_len = msg->pcl_len;

	status = sme_acquire_global_lock(&mac->sme);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("sme_acquire_global_lock failed!(status=%d)", status);
		qdf_mem_free(req_msg);
		return status;
	}

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = SIR_HAL_PDEV_SET_PCL_TO_FW;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
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
 * @mac_handle: Handle returned by macOpen
 * @msg: HW mode structure containing hw mode and callback details
 *
 * Sends the command to CSR to send WMI_PDEV_SET_HW_MODE_CMDID to FW
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_pdev_set_hw_mode(struct policy_mgr_hw_mode msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = sme_get_mac_context();
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
 * sme_nss_update_request() - Send beacon templete update to FW with new
 * nss value
 * @mac_handle: Handle returned by macOpen
 * @vdev_id: the session id
 * @new_nss: the new nss value
 * @cback: hdd callback
 * @next_action: next action to happen at policy mgr after beacon update
 * @original_vdev_id: original request hwmode change vdev id
 *
 * Sends the command to CSR to send to PE
 * Return: QDF_STATUS_SUCCESS on successful posting
 */
QDF_STATUS sme_nss_update_request(uint32_t vdev_id,
				uint8_t  new_nss, policy_mgr_nss_update_cback cback,
				uint8_t next_action, struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_conn_update_reason reason,
				uint32_t original_vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = sme_get_mac_context();
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
		cmd->u.nss_update_cmd.original_vdev_id = original_vdev_id;

		sme_debug("Queuing e_sme_command_nss_update to CSR:vdev (%d %d) ss %d r %d",
			  vdev_id, original_vdev_id, new_nss, reason);
		csr_queue_sme_command(mac, cmd, false);
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

/**
 * sme_soc_set_dual_mac_config() - Set dual mac configurations
 * @mac_handle: Handle returned by macOpen
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
	struct mac_context *mac = sme_get_mac_context();
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
 * @mac_handle: Opaque handle to the global MAC context
 * @gw_params: request parameters from HDD
 *
 * Return: QDF_STATUS
 *
 * This routine will update gateway parameters to WMA
 */
QDF_STATUS sme_gateway_param_update(mac_handle_t mac_handle,
				    struct gateway_update_req_param *gw_params)
{
	QDF_STATUS qdf_status;
	struct scheduler_msg message = {0};
	struct gateway_update_req_param *request_buf;

	request_buf = qdf_mem_malloc(sizeof(*request_buf));
	if (!request_buf)
		return QDF_STATUS_E_NOMEM;

	*request_buf = *gw_params;

	message.type = WMA_GW_PARAM_UPDATE_REQ;
	message.reserved = 0;
	message.bodyptr = request_buf;
	qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA, &message);
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
 * @mac_handle: Handle returned by macOpen
 * @msg: Structure containing the antenna mode parameters
 *
 * Send the command to CSR to send
 * WMI_SOC_SET_ANTENNA_MODE_CMDID to FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_soc_set_antenna_mode(mac_handle_t mac_handle,
				    struct sir_antenna_mode_param *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	tSmeCmd *cmd;

	if (!msg) {
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

/**
 * sme_setdef_dot11mode() - Updates mac with default dot11mode
 * @mac_handle: Global MAC pointer
 *
 * Return: NULL.
 */
void sme_setdef_dot11mode(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	csr_set_default_dot11_mode(mac_ctx);
}

/**
 * sme_update_tgt_services() - update the target services config.
 * @mac_handle: Opaque handle to the global MAC context.
 * @cfg: wma_tgt_services parameters.
 *
 * update the target services config.
 *
 * Return: None.
 */
void sme_update_tgt_services(mac_handle_t mac_handle,
			     struct wma_tgt_services *cfg)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	mac_ctx->obss_scan_offload = cfg->obss_scan_offload;
	sme_debug("obss_scan_offload: %d", mac_ctx->obss_scan_offload);
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
	mac_ctx->bcn_reception_stats = cfg->bcn_reception_stats;
}

/**
 * sme_is_session_id_valid() - Check if the session id is valid
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: Session id
 *
 * Checks if the session id is valid or not
 *
 * Return: True is the session id is valid, false otherwise
 */
bool sme_is_session_id_valid(mac_handle_t mac_handle, uint32_t session_id)
{
	struct mac_context *mac;

	if (mac_handle) {
		mac = MAC_CONTEXT(mac_handle);
	} else {
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
 * @mac_handle: Opaque handle to the global MAC context
 * @channel: channel id
 * @bw_offset: bandwidth offset
 * @opclass: pointer to operating class
 *
 * Function will determine operating class from regdm_get_opclass_from_channel
 *
 * Return: none
 */
void sme_get_opclass(mac_handle_t mac_handle, uint8_t channel,
		     uint8_t bw_offset, uint8_t *opclass)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

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

	return wma_process_fw_test_cmd(wma_handle, fw_test);
}

/**
 * sme_ht40_stop_obss_scan() - ht40 obss stop scan
 * @mac_handle: mac handel
 * @vdev_id: vdev identifier
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_ht40_stop_obss_scan(mac_handle_t mac_handle, uint32_t vdev_id)
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
 * @mac_handle: The handle returned by macOpen
 * @is_ht_smps_enabled: enable/disable ht smps
 * @ht_smps_mode: smps mode disabled/static/dynamic
 * @send_smps_action: flag to send smps force mode command
 * to FW
 *
 * Return: QDF_STATUS if SME update mimo power save
 * configuration success else failure status
 */
QDF_STATUS sme_update_mimo_power_save(mac_handle_t mac_handle,
				      uint8_t is_ht_smps_enabled,
				      uint8_t ht_smps_mode,
				      bool send_smps_action)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	sme_debug("SMPS enable: %d mode: %d send action: %d",
		is_ht_smps_enabled, ht_smps_mode,
		send_smps_action);
	mac_ctx->mlme_cfg->ht_caps.enable_smps =
		is_ht_smps_enabled;
	mac_ctx->mlme_cfg->ht_caps.smps = ht_smps_mode;
	mac_ctx->roam.configParam.send_smps_action =
		send_smps_action;

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_add_beacon_filter() - set the beacon filter configuration
 * @mac_handle: The handle returned by macOpen
 * @session_id: session id
 * @ie_map: bitwise array of IEs
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_add_beacon_filter(mac_handle_t mac_handle,
				 uint32_t session_id,
				 uint32_t *ie_map)
{
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param = qdf_mem_malloc(sizeof(*filter_param));
	if (!filter_param)
		return QDF_STATUS_E_FAILURE;

	filter_param->vdev_id = session_id;

	qdf_mem_copy(filter_param->ie_map, ie_map,
			BCN_FLT_MAX_ELEMS_IE_LIST * sizeof(uint32_t));

	message.type = WMA_ADD_BCN_FILTER_CMDID;
	message.bodyptr = filter_param;
	qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA,
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
 * @mac_handle: The handle returned by macOpen
 * @session_id: session id
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_remove_beacon_filter(mac_handle_t mac_handle,
				    uint32_t session_id)
{
	struct scheduler_msg message = {0};
	QDF_STATUS qdf_status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct beacon_filter_param *filter_param;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
		return QDF_STATUS_E_FAILURE;
	}

	filter_param = qdf_mem_malloc(sizeof(*filter_param));
	if (!filter_param)
		return QDF_STATUS_E_FAILURE;

	filter_param->vdev_id = session_id;

	message.type = WMA_REMOVE_BCN_FILTER_CMDID;
	message.bodyptr = filter_param;
	qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
					    QDF_MODULE_ID_WMA,
					    QDF_MODULE_ID_WMA,
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
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 * @peer_mac: peer mac address
 * @reason: reason for disassociation
 * wait_for_ack: wait for acknowledgment
 *
 * function to send disassoc request to lim
 *
 * return: none
 */
void sme_send_disassoc_req_frame(mac_handle_t mac_handle, uint8_t session_id,
				 uint8_t *peer_mac, uint16_t reason,
				 uint8_t wait_for_ack)
{
	struct sme_send_disassoc_frm_req *msg;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (!msg)
		return;

	msg->msg_type = eWNI_SME_SEND_DISASSOC_FRAME;
	msg->length = sizeof(*msg);
	msg->session_id = session_id;
	qdf_mem_copy(msg->peer_mac, peer_mac, QDF_MAC_ADDR_SIZE);
	msg->reason = reason;
	msg->wait_for_ack = wait_for_ack;

	qdf_status = umac_send_mb_message_to_mac(msg);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		sme_err("umac_send_mb_message_to_mac failed, %d",
			qdf_status);
}

#ifdef FEATURE_WLAN_APF
QDF_STATUS sme_get_apf_capabilities(mac_handle_t mac_handle,
				    apf_get_offload_cb callback,
				    void *context)
{
	QDF_STATUS          status     = QDF_STATUS_SUCCESS;
	struct mac_context *     mac_ctx      = MAC_CONTEXT(mac_handle);
	struct scheduler_msg           cds_msg = {0};

	SME_ENTER();

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		/* Serialize the req through MC thread */
		mac_ctx->sme.apf_get_offload_cb = callback;
		mac_ctx->sme.apf_get_offload_context = context;
		cds_msg.bodyptr = NULL;
		cds_msg.type = WDA_APF_GET_CAPABILITIES_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &cds_msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
					FL("Post apf get offload msg fail"));
			status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac_ctx->sme);
	}

	SME_EXIT();
	return status;
}

QDF_STATUS sme_set_apf_instructions(mac_handle_t mac_handle,
				    struct sir_apf_set_offload *req)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_set_apf_instructions(wma_handle, req);
}

QDF_STATUS sme_set_apf_enable_disable(mac_handle_t mac_handle, uint8_t vdev_id,
				      bool apf_enable)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_send_apf_enable_cmd(wma_handle, vdev_id, apf_enable);
}

QDF_STATUS
sme_apf_write_work_memory(mac_handle_t mac_handle,
			struct wmi_apf_write_memory_params *write_params)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_send_apf_write_work_memory_cmd(wma_handle, write_params);
}

QDF_STATUS
sme_apf_read_work_memory(mac_handle_t mac_handle,
			 struct wmi_apf_read_memory_params *read_params,
			 apf_read_mem_cb callback)
{
	QDF_STATUS status   = QDF_STATUS_SUCCESS;
	struct mac_context *mac  = MAC_CONTEXT(mac_handle);
	void *wma_handle;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.apf_read_mem_cb = callback;
		sme_release_global_lock(&mac->sme);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("sme_acquire_global_lock failed"));
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return wma_send_apf_read_work_memory_cmd(wma_handle, read_params);
}
#endif /* FEATURE_WLAN_APF */

/**
 * sme_get_wni_dot11_mode() - return configured wni dot11mode
 * @mac_handle: Opaque handle to the global MAC context
 *
 * Return: wni dot11 mode.
 */
uint32_t sme_get_wni_dot11_mode(mac_handle_t mac_handle)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	return csr_translate_to_wni_cfg_dot11_mode(mac_ctx,
		mac_ctx->roam.configParam.uCfgDot11Mode);
}

/**
 * sme_create_mon_session() - post message to create PE session for monitormode
 * operation
 * @mac_handle: Opaque handle to the global MAC context
 * @bssid: pointer to bssid
 * @vdev_id: sme session id
 *
 * Return: QDF_STATUS_SUCCESS on success, non-zero error code on failure.
 */
QDF_STATUS sme_create_mon_session(mac_handle_t mac_handle, tSirMacAddr bss_id,
				  uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sir_create_session *msg;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (msg) {
		msg->type = eWNI_SME_MON_INIT_SESSION;
		msg->vdev_id = vdev_id;
		msg->msg_len = sizeof(*msg);
		qdf_mem_copy(msg->bss_id.bytes, bss_id, QDF_MAC_ADDR_SIZE);
		status = umac_send_mb_message_to_mac(msg);
	}
	return status;
}

void sme_set_chan_info_callback(mac_handle_t mac_handle,
			void (*callback)(struct scan_chan_info *chan_info))
{
	struct mac_context *mac;

	if (!mac_handle) {
		QDF_ASSERT(0);
		return;
	}
	mac = MAC_CONTEXT(mac_handle);
	mac->chan_info_cb = callback;
}

/**
 * sme_set_vdev_ies_per_band() - sends the per band IEs to vdev
 * @mac_handle: Opaque handle to the global MAC context
 * @vdev_id: vdev_id for which IE is targeted
 *
 * Return: None
 */
void sme_set_vdev_ies_per_band(mac_handle_t mac_handle, uint8_t vdev_id)
{
	struct sir_set_vdev_ies_per_band *p_msg;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p_msg = qdf_mem_malloc(sizeof(*p_msg));
	if (!p_msg)
		return;

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
 * @mac_handle: Opaque handle to the global MAC context
 * @enable2x2: 1x1 or 2x2 mode.
 *
 * Sends the set pdev IE req with Nss value.
 *
 * Return: None
 */
void sme_set_pdev_ht_vht_ies(mac_handle_t mac_handle, bool enable2x2)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
		if (!ht_vht_cfg) {
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

void sme_update_vdev_type_nss(mac_handle_t mac_handle, uint8_t max_supp_nss,
			      enum nss_chains_band_info band)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct vdev_type_nss *vdev_nss;

	struct wlan_mlme_nss_chains *nss_chains_ini_cfg =
					&mac_ctx->mlme_cfg->nss_chains_ini_cfg;

	if (band == NSS_CHAINS_BAND_5GHZ)
		vdev_nss = &mac_ctx->vdev_type_nss_5g;
	else
		vdev_nss = &mac_ctx->vdev_type_nss_2g;

	vdev_nss->sta = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						STA_NSS_CHAINS_SHIFT));
	vdev_nss->sap = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						SAP_NSS_CHAINS_SHIFT));
	vdev_nss->p2p_go = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						P2P_GO_NSS_CHAINS_SHIFT));
	vdev_nss->p2p_cli = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						P2P_CLI_CHAINS_SHIFT));
	vdev_nss->p2p_dev = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						P2P_DEV_NSS_CHAINS_SHIFT));
	vdev_nss->ibss = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						IBSS_NSS_CHAINS_SHIFT));
	vdev_nss->tdls = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						TDLS_NSS_CHAINS_SHIFT));
	vdev_nss->ocb = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						OCB_NSS_CHAINS_SHIFT));
	vdev_nss->nan = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						NAN_NSS_CHAIN_SHIFT));
	vdev_nss->ndi = QDF_MIN(max_supp_nss, GET_VDEV_NSS_CHAIN(
						nss_chains_ini_cfg->
							rx_nss[band],
						NAN_NSS_CHAIN_SHIFT));

	sme_debug("band %d NSS:sta %d sap %d cli %d go %d dev %d ibss %d tdls %d ocb %d nan %d",
		  band, vdev_nss->sta, vdev_nss->sap, vdev_nss->p2p_cli,
		  vdev_nss->p2p_go, vdev_nss->p2p_dev, vdev_nss->ibss,
		  vdev_nss->tdls, vdev_nss->ocb, vdev_nss->nan);
}

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
#define MAX_BSS_COLOR_VAL 63
#define MIN_BSS_COLOR_VAL 1

QDF_STATUS sme_set_he_bss_color(mac_handle_t mac_handle, uint8_t session_id,
				uint8_t bss_color)

{
	struct sir_set_he_bss_color *bss_color_msg;
	uint8_t len;

	if (!mac_handle) {
		sme_err("Invalid mac_handle pointer");
		return QDF_STATUS_E_FAULT;
	}

	sme_debug("Set HE bss_color  %d", bss_color);

	if (bss_color < MIN_BSS_COLOR_VAL || bss_color > MAX_BSS_COLOR_VAL) {
		sme_debug("Invalid HE bss_color  %d", bss_color);
		return QDF_STATUS_E_INVAL;
	}
	len = sizeof(*bss_color_msg);
	bss_color_msg = qdf_mem_malloc(len);
	if (!bss_color_msg)
		return QDF_STATUS_E_NOMEM;

	bss_color_msg->message_type = eWNI_SME_SET_HE_BSS_COLOR;
	bss_color_msg->length = len;
	bss_color_msg->session_id = session_id;
	bss_color_msg->bss_color = bss_color;
	return umac_send_mb_message_to_mac(bss_color_msg);
}
#endif

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
/**
 * sme_register_p2p_lo_event() - Register for the p2p lo event
 * @mac_handle: Opaque handle to the global MAC context
 * @context: the context of the call
 * @callback: the callback to hdd
 *
 * This function registers the callback function for P2P listen
 * offload stop event.
 *
 * Return: none
 */
void sme_register_p2p_lo_event(mac_handle_t mac_handle, void *context,
			       p2p_lo_callback callback)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = sme_acquire_global_lock(&mac->sme);
	mac->sme.p2p_lo_event_callback = callback;
	mac->sme.p2p_lo_event_context = context;
	sme_release_global_lock(&mac->sme);
}
#endif

/**
 * sme_process_mac_pwr_dbg_cmd() - enable mac pwr debugging
 * @mac_handle: The handle returned by macOpen
 * @session_id: session id
 * @dbg_args: args for mac pwr debug command
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_process_mac_pwr_dbg_cmd(mac_handle_t mac_handle,
				       uint32_t session_id,
				       struct sir_mac_pwr_dbg_cmd *dbg_args)
{
	struct scheduler_msg message = {0};
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct sir_mac_pwr_dbg_cmd *req;
	int i;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("CSR session not valid: %d", session_id);
		return QDF_STATUS_E_FAILURE;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_FAILURE;

	req->module_id = dbg_args->module_id;
	req->pdev_id = dbg_args->pdev_id;
	req->num_args = dbg_args->num_args;
	for (i = 0; i < req->num_args; i++)
		req->args[i] = dbg_args->args[i];

	message.type = SIR_HAL_POWER_DBG_CMD;
	message.bodyptr = req;

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_message(QDF_MODULE_ID_SME,
							  QDF_MODULE_ID_WMA,
							  QDF_MODULE_ID_WMA,
							  &message))) {
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
	struct mac_context *mac_ctx = sme_get_mac_context();

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		FL("Invalid MAC context"));
		return;
	}
	csr_get_vdev_type_nss(mac_ctx, dev_mode, nss_2g, nss_5g);
}

/**
 * sme_update_sta_roam_policy() - update sta roam policy for
 * unsafe and DFS channels.
 * @mac_handle: Opaque handle to the global MAC context
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
QDF_STATUS sme_update_sta_roam_policy(mac_handle_t mac_handle,
				      enum sta_roam_policy_dfs_mode dfs_mode,
				      bool skip_unsafe_channels,
				      uint8_t session_id,
				      uint8_t sap_operating_band)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sme_config_params *sme_config;

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_FATAL,
				"%s: mac_ctx is null", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	sme_config = qdf_mem_malloc(sizeof(*sme_config));
	if (!sme_config)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(sme_config, sizeof(*sme_config));
	sme_get_config_param(mac_handle, sme_config);

	sme_config->csrConfig.sta_roam_policy_params.dfs_mode =
		dfs_mode;
	sme_config->csrConfig.sta_roam_policy_params.skip_unsafe_channels =
		skip_unsafe_channels;
	sme_config->csrConfig.sta_roam_policy_params.sap_operating_band =
		sap_operating_band;

	sme_update_config(mac_handle, sme_config);

	status = csr_update_channel_list(mac_ctx);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("failed to update the supported channel list"));
	}

	if (mac_ctx->mlme_cfg->lfr.roam_scan_offload_enabled) {
		status = sme_acquire_global_lock(&mac_ctx->sme);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			csr_roam_offload_scan(mac_ctx, session_id,
				ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				REASON_ROAM_SCAN_STA_ROAM_POLICY_CHANGED);
			sme_release_global_lock(&mac_ctx->sme);
		}
	}
	qdf_mem_free(sme_config);
	return status;
}

/**
 * sme_enable_disable_chanavoidind_event - configure ca event ind
 * @mac_handle: Opaque handle to the global MAC context
 * @set_value: enable/disable
 *
 * function to enable/disable chan avoidance indication
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_enable_disable_chanavoidind_event(mac_handle_t mac_handle,
						 uint8_t set_value)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};

	if (!mac_ctx->mlme_cfg->gen.optimize_ca_event) {
		sme_err("optimize_ca_event not enabled in ini");
		return QDF_STATUS_E_NOSUPPORT;
	}

	sme_debug("set_value: %d", set_value);
	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS == status) {
		qdf_mem_zero(&msg, sizeof(struct scheduler_msg));
		msg.type = WMA_SEND_FREQ_RANGE_CONTROL_IND;
		msg.bodyval = set_value;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &msg);
		sme_release_global_lock(&mac_ctx->sme);
		return status;
	}
	return status;
}

/*
 * sme_set_default_scan_ie() - API to send default scan IE to LIM
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: current session ID
 * @ie_data: Pointer to Scan IE data
 * @ie_len: Length of @ie_data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_default_scan_ie(mac_handle_t mac_handle, uint16_t session_id,
				   uint8_t *ie_data, uint16_t ie_len)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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

QDF_STATUS sme_get_sar_power_limits(mac_handle_t mac_handle,
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

QDF_STATUS sme_set_sar_power_limits(mac_handle_t mac_handle,
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

QDF_STATUS sme_send_coex_config_cmd(struct coex_config_params *coex_cfg_params)
{
	void *wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		sme_err("wma handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	return wma_send_coex_config_cmd(wma_handle, coex_cfg_params);
}

#ifdef WLAN_FEATURE_FIPS
QDF_STATUS sme_fips_request(mac_handle_t mac_handle, struct fips_params *param,
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

QDF_STATUS sme_set_cts2self_for_p2p_go(mac_handle_t mac_handle)
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
 * @mac_handle: Handle returned by mac_open
 * @session_id: Session ID on which tx fail count needs to be updated to FW
 * @tx_fail_count: Count for tx fail threshold after which FW will disconnect
 *
 * This function is used to set tx fail count threshold to firmware.
 * firmware will issue disocnnect with peer device once this threshold is
 * reached.
 *
 * Return: Return QDF_STATUS, otherwise appropriate failure code
 */
QDF_STATUS sme_update_tx_fail_cnt_threshold(mac_handle_t mac_handle,
					    uint8_t session_id,
					    uint32_t tx_fail_count)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct sme_tx_fail_cnt_threshold *tx_fail_cnt;
	struct scheduler_msg msg = {0};

	tx_fail_cnt = qdf_mem_malloc(sizeof(*tx_fail_cnt));
	if (!tx_fail_cnt)
		return QDF_STATUS_E_FAILURE;

	sme_debug("session_id: %d tx_fail_count: %d",
		  session_id, tx_fail_count);
	tx_fail_cnt->session_id = session_id;
	tx_fail_cnt->tx_fail_cnt_threshold = tx_fail_count;

	qdf_mem_zero(&msg, sizeof(struct scheduler_msg));
	msg.type = SIR_HAL_UPDATE_TX_FAIL_CNT_TH;
	msg.reserved = 0;
	msg.bodyptr = tx_fail_cnt;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Not able to post Tx fail count message to WDA"));
		qdf_mem_free(tx_fail_cnt);
	}
	return status;
}

QDF_STATUS sme_set_lost_link_info_cb(mac_handle_t mac_handle,
				     lost_link_info_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.lost_link_info_cb = cb;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef FEATURE_WLAN_ESE
bool sme_roam_is_ese_assoc(struct csr_roam_info *roam_info)
{
	return roam_info->isESEAssoc;
}
#endif

bool sme_neighbor_roam_is11r_assoc(mac_handle_t mac_handle, uint8_t session_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

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
	if (!wow_pulse_set_cmd)
		return QDF_STATUS_E_NOMEM;

	*wow_pulse_set_cmd = *wow_pulse_set_info;

	message.type = WMA_SET_WOW_PULSE_CMD;
	message.bodyptr = wow_pulse_set_cmd;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA,
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

QDF_STATUS sme_get_rssi_snr_by_bssid(mac_handle_t mac_handle,
				     struct csr_roam_profile *profile,
				     const uint8_t *bssid,
				     int8_t *rssi, int8_t *snr)
{
	tSirBssDescription *bss_descp;
	tCsrScanResultFilter *scan_filter;
	struct scan_result_list *bss_list;
	tScanResultHandle result_handle = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (!scan_filter) {
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
	if (!scan_filter->BSSIDs.bssid) {
		scan_filter->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(struct qdf_mac_addr));
		if (!scan_filter->BSSIDs.bssid) {
			status = QDF_STATUS_E_NOMEM;
			goto free_scan_flter;
		}
	}

	scan_filter->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(scan_filter->BSSIDs.bssid[0].bytes,
		     bssid, sizeof(struct qdf_mac_addr));

	status = csr_scan_get_result(mac_ctx, scan_filter, &result_handle);
	if (QDF_STATUS_SUCCESS != status) {
		sme_debug("parse_scan_result failed");
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

QDF_STATUS sme_get_beacon_frm(mac_handle_t mac_handle,
			      struct csr_roam_profile *profile,
			      const tSirMacAddr bssid,
			      uint8_t **frame_buf, uint32_t *frame_len,
			      int *channel)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tScanResultHandle result_handle = NULL;
	tCsrScanResultFilter *scan_filter;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	tSirBssDescription *bss_descp;
	struct scan_result_list *bss_list;
	uint32_t ie_len;

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (!scan_filter) {
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
	if (!scan_filter->BSSIDs.bssid) {
		scan_filter->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(struct qdf_mac_addr));
		if (!scan_filter->BSSIDs.bssid) {
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
	if (!*frame_buf) {
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
QDF_STATUS sme_fast_reassoc(mac_handle_t mac_handle,
			    struct csr_roam_profile *profile,
			    const tSirMacAddr bssid, int channel,
			    uint8_t vdev_id, const tSirMacAddr connected_bssid)
{
	QDF_STATUS status;
	struct wma_roam_invoke_cmd *fastreassoc;
	struct scheduler_msg msg = {0};
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;
	struct csr_roam_profile *roam_profile;

	session = CSR_GET_SESSION(mac_ctx, vdev_id);
	if (!session || !session->pCurRoamProfile) {
		sme_err("session %d not found", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	roam_profile = session->pCurRoamProfile;
	if (roam_profile->driver_disabled_roaming) {
		sme_debug("roaming status in driver %d",
			  roam_profile->driver_disabled_roaming);
		return QDF_STATUS_E_FAILURE;
	}
	fastreassoc = qdf_mem_malloc(sizeof(*fastreassoc));
	if (!fastreassoc)
		return QDF_STATUS_E_NOMEM;

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

	status = sme_get_beacon_frm(mac_handle, profile, bssid,
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

	msg.type = eWNI_SME_ROAM_INVOKE;
	msg.reserved = 0;
	msg.bodyptr = fastreassoc;
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_STATUS_SUCCESS != status) {
		sme_err("Not able to post ROAM_INVOKE_CMD message to PE");
		qdf_mem_free(fastreassoc);
	}

	return status;
}
#endif

QDF_STATUS sme_set_del_pmkid_cache(mac_handle_t mac_handle, uint8_t session_id,
				   tPmkidCacheInfo *pmk_cache_info,
				   bool is_add)
{
	struct wmi_unified_pmk_cache *pmk_cache;
	struct scheduler_msg msg;

	pmk_cache = qdf_mem_malloc(sizeof(*pmk_cache));
	if (!pmk_cache)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_zero(pmk_cache, sizeof(*pmk_cache));

	pmk_cache->vdev_id = session_id;

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

	pmk_cache->pmkid_len = PMKID_LEN;
	qdf_mem_copy(pmk_cache->pmkid, pmk_cache_info->PMKID,
		     PMKID_LEN);

	pmk_cache->pmk_len = pmk_cache_info->pmk_len;
	qdf_mem_copy(pmk_cache->pmk, pmk_cache_info->pmk,
		     pmk_cache->pmk_len);

send_flush_cmd:
	msg.type = SIR_HAL_SET_DEL_PMKID_CACHE;
	msg.reserved = 0;
	msg.bodyptr = pmk_cache;
	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_message(QDF_MODULE_ID_SME,
				   QDF_MODULE_ID_WMA,
				   QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Not able to post message to WDA");
		if (pmk_cache) {
			qdf_mem_zero(pmk_cache, sizeof(*pmk_cache));
			qdf_mem_free(pmk_cache);
		}
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* ARP DEBUG STATS */

/**
 * sme_set_nud_debug_stats() - sme api to set nud debug stats
 * @mac_handle: Opaque handle to the global MAC context
 * @set_stats_param: pointer to set stats param
 *
 * Return: Return QDF_STATUS.
 */
QDF_STATUS sme_set_nud_debug_stats(mac_handle_t mac_handle,
				   struct set_arp_stats_params
				   *set_stats_param)
{
	struct set_arp_stats_params *arp_set_param;
	struct scheduler_msg msg;

	arp_set_param = qdf_mem_malloc(sizeof(*arp_set_param));
	if (!arp_set_param)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(arp_set_param, set_stats_param, sizeof(*arp_set_param));

	msg.type = WMA_SET_ARP_STATS_REQ;
	msg.reserved = 0;
	msg.bodyptr = arp_set_param;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_message(QDF_MODULE_ID_SME,
				   QDF_MODULE_ID_WMA,
				   QDF_MODULE_ID_WMA, &msg)) {
		sme_err("Not able to post message to WDA");
		qdf_mem_free(arp_set_param);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * sme_get_nud_debug_stats() - sme api to get nud debug stats
 * @mac_handle: Opaque handle to the global MAC context
 * @get_stats_param: pointer to set stats param
 *
 * Return: Return QDF_STATUS.
 */
QDF_STATUS sme_get_nud_debug_stats(mac_handle_t mac_handle,
				   struct get_arp_stats_params
				   *get_stats_param)
{
	struct get_arp_stats_params *arp_get_param;
	struct scheduler_msg msg;

	arp_get_param = qdf_mem_malloc(sizeof(*arp_get_param));
	if (!arp_get_param)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(arp_get_param, get_stats_param, sizeof(*arp_get_param));

	msg.type = WMA_GET_ARP_STATS_REQ;
	msg.reserved = 0;
	msg.bodyptr = arp_get_param;

	if (QDF_STATUS_SUCCESS !=
	    scheduler_post_message(QDF_MODULE_ID_SME,
				   QDF_MODULE_ID_WMA,
				   QDF_MODULE_ID_WMA, &msg)) {
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

QDF_STATUS sme_register_set_connection_info_cb(mac_handle_t mac_handle,
				bool (*set_connection_info_cb)(bool),
				bool (*get_connection_info_cb)(uint8_t *session_id,
				enum scan_reject_states *reason))
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.set_connection_info_cb = set_connection_info_cb;
		mac->sme.get_connection_info_cb = get_connection_info_cb;
		sme_release_global_lock(&mac->sme);
	}
	return status;
}

QDF_STATUS sme_rso_cmd_status_cb(mac_handle_t mac_handle,
				 rso_cmd_status_cb cb)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	mac->sme.rso_cmd_status_cb = cb;
	sme_debug("Registered RSO command status callback");
	return status;
}

QDF_STATUS sme_set_dbs_scan_selection_config(mac_handle_t mac_handle,
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
	if (!dbs_scan_params)
		return QDF_STATUS_E_NOMEM;

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
	status = scheduler_post_message(QDF_MODULE_ID_SME,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Not able to post msg to WMA!");
		qdf_mem_free(dbs_scan_params);
	}

	return status;
}

QDF_STATUS sme_get_rcpi(mac_handle_t mac_handle, struct sme_rcpi_req *rcpi)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};
	struct sme_rcpi_req *rcpi_req;

	rcpi_req = qdf_mem_malloc(sizeof(*rcpi_req));
	if (!rcpi_req)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(rcpi_req, rcpi, sizeof(*rcpi_req));

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		msg.bodyptr = rcpi_req;
		msg.type = WMA_GET_RCPI_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA, &msg);
		sme_release_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("post get rcpi req failed"));
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(rcpi_req);
		}
	} else {
		qdf_mem_free(rcpi_req);
	}

	return status;
}

void sme_store_pdev(mac_handle_t mac_handle, struct wlan_objmgr_pdev *pdev)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
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
	pdev->pdev_nif.pdev_fw_caps |= SUPPORTED_CRYPTO_CAPS;
}

QDF_STATUS sme_congestion_register_callback(mac_handle_t mac_handle,
					    congestion_cb congestion_cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.congestion_cb = congestion_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("congestion callback set");
	}

	return status;
}

QDF_STATUS sme_register_tx_queue_cb(mac_handle_t mac_handle,
				    tx_queue_cb tx_queue_cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.tx_queue_cb = tx_queue_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("Tx queue callback set");
	}

	return status;
}

QDF_STATUS sme_deregister_tx_queue_cb(mac_handle_t mac_handle)
{
	return sme_register_tx_queue_cb(mac_handle, NULL);
}

#ifdef WLAN_SUPPORT_TWT
QDF_STATUS sme_register_twt_enable_complete_cb(mac_handle_t mac_handle,
					       twt_enable_cb twt_enable_cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.twt_enable_cb = twt_enable_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("TWT: enable callback set");
	}

	return status;
}

QDF_STATUS sme_register_twt_disable_complete_cb(mac_handle_t mac_handle,
						twt_disable_cb twt_disable_cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.twt_disable_cb = twt_disable_cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("TWT: disable callback set");
	}

	return status;
}

QDF_STATUS sme_deregister_twt_enable_complete_cb(mac_handle_t mac_handle)
{
	return sme_register_twt_enable_complete_cb(mac_handle, NULL);
}

QDF_STATUS sme_deregister_twt_disable_complete_cb(mac_handle_t mac_handle)
{
	return sme_register_twt_disable_complete_cb(mac_handle, NULL);
}
#endif

QDF_STATUS sme_set_smps_cfg(uint32_t vdev_id, uint32_t param_id,
						uint32_t param_val)
{
	return wma_configure_smps_params(vdev_id, param_id, param_val);
}

QDF_STATUS sme_set_reorder_timeout(mac_handle_t mac_handle,
				   struct sir_set_rx_reorder_timeout_val *req)
{
	QDF_STATUS status;
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	status = wma_set_rx_reorder_timeout_val(wma_handle, req);

	return status;
}

QDF_STATUS sme_set_rx_set_blocksize(mac_handle_t mac_handle,
				    struct sir_peer_set_rx_blocksize *req)
{
	QDF_STATUS status;
	tp_wma_handle wma_handle;

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	status = wma_set_rx_blocksize(wma_handle, req);

	return status;
}

int sme_cli_set_command(int vdev_id, int param_id, int sval, int vpdev)
{
	return wma_cli_set_command(vdev_id, param_id, sval, vpdev);
}

int sme_set_enable_mem_deep_sleep(mac_handle_t mac_handle, int vdev_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	return wma_cli_set_command(vdev_id, WMI_PDEV_PARAM_HYST_EN,
				   mac_ctx->mlme_cfg->gen.memory_deep_sleep,
				   PDEV_CMD);
}

int sme_set_cck_tx_fir_override(mac_handle_t mac_handle, int vdev_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	return wma_cli_set_command(vdev_id,
				   WMI_PDEV_PARAM_ENABLE_CCK_TXFIR_OVERRIDE,
				   mac_ctx->mlme_cfg->gen.cck_tx_fir_override,
				   PDEV_CMD);
}

QDF_STATUS sme_set_bt_activity_info_cb(mac_handle_t mac_handle,
				       bt_activity_info_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.bt_activity_info_cb = cb;
		sme_release_global_lock(&mac->sme);
		sme_debug("bt activity info callback set");
	}

	return status;
}

QDF_STATUS sme_get_chain_rssi(mac_handle_t mac_handle,
			      struct get_chain_rssi_req_params *input,
			      get_chain_rssi_callback callback,
			      void *context)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	tp_wma_handle wma_handle;

	SME_ENTER();

	if (!input) {
		sme_err("Invalid req params");
		return QDF_STATUS_E_INVAL;
	}

	mac_ctx->sme.get_chain_rssi_cb = callback;
	mac_ctx->sme.get_chain_rssi_context = context;
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	wma_get_chain_rssi(wma_handle, input);

	SME_EXIT();
	return status;
}

QDF_STATUS sme_process_msg_callback(struct mac_context *mac,
				    struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!msg) {
		sme_err("Empty message for SME Msg callback");
		return status;
	}
	status = sme_process_msg(mac, msg);
	return status;
}

void sme_display_disconnect_stats(mac_handle_t mac_handle, uint8_t session_id)
{
	struct csr_roam_session *session;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

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

	sme_nofl_info("Total No. of Disconnections: %d",
		      session->disconnect_stats.disconnection_cnt);

	sme_nofl_info("No. of Diconnects Triggered by Application: %d",
		      session->disconnect_stats.disconnection_by_app);

	sme_nofl_info("No. of Disassoc Sent by Peer: %d",
		      session->disconnect_stats.disassoc_by_peer);

	sme_nofl_info("No. of Deauth Sent by Peer: %d",
		      session->disconnect_stats.deauth_by_peer);

	sme_nofl_info("No. of Disconnections due to Beacon Miss: %d",
		      session->disconnect_stats.bmiss);

	sme_nofl_info("No. of Disconnections due to Peer Kickout: %d",
		      session->disconnect_stats.peer_kickout);
}

#ifdef FEATURE_WLAN_DYNAMIC_CVM
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
#endif

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

QDF_STATUS sme_send_limit_off_channel_params(mac_handle_t mac_handle,
					     uint8_t vdev_id,
					     bool is_tos_active,
					     uint32_t max_off_chan_time,
					     uint32_t rest_time,
					     bool skip_dfs_chan)
{
	struct sir_limit_off_chan *cmd;
	struct scheduler_msg msg = {0};

	cmd = qdf_mem_malloc(sizeof(*cmd));
	if (!cmd)
		return QDF_STATUS_E_NOMEM;

	cmd->vdev_id = vdev_id;
	cmd->is_tos_active = is_tos_active;
	cmd->max_off_chan_time = max_off_chan_time;
	cmd->rest_time = rest_time;
	cmd->skip_dfs_chans = skip_dfs_chan;

	msg.type = WMA_SET_LIMIT_OFF_CHAN;
	msg.reserved = 0;
	msg.bodyptr = cmd;

	if (!QDF_IS_STATUS_SUCCESS(scheduler_post_message(QDF_MODULE_ID_SME,
							  QDF_MODULE_ID_WMA,
							  QDF_MODULE_ID_WMA,
							  &msg))) {
		sme_err("Not able to post WMA_SET_LIMIT_OFF_CHAN to WMA");
		qdf_mem_free(cmd);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

uint32_t sme_unpack_rsn_ie(mac_handle_t mac_handle, uint8_t *buf,
			   uint8_t buf_len, tDot11fIERSN *rsn_ie,
			   bool append_ie)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	return dot11f_unpack_ie_rsn(mac_ctx, buf, buf_len, rsn_ie, append_ie);
}

#ifdef FEATURE_BSS_TRANSITION
/**
 * sme_get_status_for_candidate() - Get bss transition status for candidate
 * @mac_handle: Opaque handle to the global MAC context
 * @conn_bss_desc: connected bss descriptor
 * @bss_desc: candidate bss descriptor
 * @info: candiadate bss information
 * @trans_reason: transition reason code
 * @is_bt_in_progress: bt activity indicator
 *
 * Return : true if candidate is rejected and reject reason is filled
 * @info->status. Otherwise returns false.
 */
static bool sme_get_status_for_candidate(mac_handle_t mac_handle,
					 tSirBssDescription *conn_bss_desc,
					 tSirBssDescription *bss_desc,
					 struct bss_candidate_info *info,
					 uint8_t trans_reason,
					 bool is_bt_in_progress)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct wlan_mlme_mbo *mbo_cfg;
	int8_t current_rssi_mcc_thres;

	if (!(mac_ctx->mlme_cfg)) {
		pe_err("mlme cfg is NULL");
		return false;
	}
	mbo_cfg = &mac_ctx->mlme_cfg->mbo_cfg;

	/*
	 * Low RSSI based rejection
	 * If candidate rssi is less than mbo_candidate_rssi_thres and connected
	 * bss rssi is greater than mbo_current_rssi_thres, then reject the
	 * candidate with MBO reason code 4.
	 */
	if ((bss_desc->rssi < mbo_cfg->mbo_candidate_rssi_thres) &&
	    (conn_bss_desc->rssi > mbo_cfg->mbo_current_rssi_thres)) {
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
		current_rssi_mcc_thres = mbo_cfg->mbo_current_rssi_mcc_thres;
		if ((conn_bss_desc->rssi > current_rssi_mcc_thres) &&
		    csr_is_mcc_channel(mac_ctx, bss_desc->channelId)) {
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
		    (bss_desc->rssi < mbo_cfg->mbo_candidate_rssi_btc_thres)) {
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

QDF_STATUS sme_get_bss_transition_status(mac_handle_t mac_handle,
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
	if (!conn_res)
		return QDF_STATUS_E_NOMEM;

	res = qdf_mem_malloc(sizeof(tCsrScanResultInfo));
	if (!res) {
		status = QDF_STATUS_E_NOMEM;
		goto free;
	}

	/* Get the connected BSS descriptor */
	status = sme_scan_get_result_for_bssid(mac_handle, bssid, conn_res);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to find connected BSS in scan list");
		goto free;
	}
	conn_bss_desc = &conn_res->BssDescriptor;

	for (i = 0; i < n_candidates; i++) {
		/* Get candidate BSS descriptors */
		status = sme_scan_get_result_for_bssid(mac_handle,
						       &info[i].bssid,
						       res);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("BSS "MAC_ADDRESS_STR" not present in scan list",
				MAC_ADDR_ARRAY(info[i].bssid.bytes));
			info[i].status = QCA_STATUS_REJECT_UNKNOWN;
			continue;
		}

		bss_desc = &res->BssDescriptor;
		if (!sme_get_status_for_candidate(mac_handle, conn_bss_desc,
						  bss_desc, &info[i],
						  transition_reason,
						  is_bt_in_progress)) {
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
#endif /* FEATURE_BSS_TRANSITION */

bool sme_is_conn_state_connected(mac_handle_t mac_handle, uint8_t session_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	return csr_is_conn_state_connected(mac_ctx, session_id);
}

void sme_enable_roaming_on_connected_sta(mac_handle_t mac_handle)
{
	uint8_t session_id;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	QDF_STATUS status;

	session_id = csr_get_roam_enabled_sta_sessionid(mac_ctx);
	if (session_id != WLAN_UMAC_VDEV_ID_MAX)
		return;

	session_id = csr_get_connected_infra(mac_ctx);
	if (session_id == WLAN_UMAC_VDEV_ID_MAX) {
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
	struct mac_context *mac_ctx;
	mac_handle_t mac_handle;
	int16_t freq = 0;

	if (!vdev) {
		sme_err("Invalid vdev id is passed");
		return 0;
	}

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (!mac_handle) {
		sme_err("mac_handle is null");
		return 0;
	}
	mac_ctx = MAC_CONTEXT(mac_handle);
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
	struct mac_context *mac_ctx;
	mac_handle_t mac_handle;
	enum phy_ch_width ch_width = CH_WIDTH_20MHZ;

	if (!vdev) {
		sme_err("Invalid vdev id is passed");
		return CH_WIDTH_INVALID;
	}

	mac_handle = cds_get_context(QDF_MODULE_ID_SME);
	if (!mac_handle) {
		sme_err("mac_handle is null");
		return CH_WIDTH_INVALID;
	}
	mac_ctx = MAC_CONTEXT(mac_handle);
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
QDF_STATUS sme_handle_sae_msg(mac_handle_t mac_handle,
			      uint8_t session_id,
			      uint8_t sae_status,
			      struct qdf_mac_addr peer_mac_addr)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct sir_sae_msg *sae_msg;
	struct scheduler_msg sch_msg = {0};

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		sae_msg = qdf_mem_malloc(sizeof(*sae_msg));
		if (!sae_msg) {
			qdf_status = QDF_STATUS_E_NOMEM;
		} else {
			sae_msg->message_type = eWNI_SME_SEND_SAE_MSG;
			sae_msg->length = sizeof(*sae_msg);
			sae_msg->session_id = session_id;
			sae_msg->sae_status = sae_status;
			qdf_mem_copy(sae_msg->peer_mac_addr,
				     peer_mac_addr.bytes,
				     QDF_MAC_ADDR_SIZE);
			sme_debug("SAE: sae_status %d session_id %d Peer: "
				  MAC_ADDRESS_STR, sae_msg->sae_status,
				  sae_msg->session_id,
				  MAC_ADDR_ARRAY(sae_msg->peer_mac_addr));

			sch_msg.type = eWNI_SME_SEND_SAE_MSG;
			sch_msg.bodyptr = sae_msg;

			qdf_status =
				scheduler_post_message(QDF_MODULE_ID_SME,
						       QDF_MODULE_ID_PE,
						       QDF_MODULE_ID_PE,
						       &sch_msg);
		}
		sme_release_global_lock(&mac->sme);
	}

	return qdf_status;
}
#endif

bool sme_is_sta_key_exchange_in_progress(mac_handle_t mac_handle,
					 uint8_t session_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("Invalid session id: %d", session_id);
		return false;
	}

	return CSR_IS_WAIT_FOR_KEY(mac_ctx, session_id);
}

bool sme_validate_channel_list(mac_handle_t mac_handle,
			       uint8_t *chan_list,
			       uint8_t num_channels)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t i = 0;
	uint8_t j;
	bool found;
	struct csr_channel *ch_lst_info = &mac_ctx->scan.base_channels;

	if (!chan_list || !num_channels) {
		sme_err("Chan list empty %pK or num_channels is 0", chan_list);
		return false;
	}

	while (i < num_channels) {
		found = false;
		for (j = 0; j < ch_lst_info->numChannels; j++) {
			if (ch_lst_info->channelList[j] == chan_list[i]) {
				found = true;
				break;
			}
		}

		if (!found) {
			sme_debug("Invalid channel %d", chan_list[i]);
			return false;
		}

		i++;
	}

	return true;
}

void sme_set_amsdu(mac_handle_t mac_handle, bool enable)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	mac_ctx->is_usr_cfg_amsdu_enabled = enable;
}

#ifdef WLAN_FEATURE_11AX
void sme_set_he_testbed_def(mac_handle_t mac_handle, uint8_t vdev_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, vdev_id);

	if (!session) {
		sme_debug("No session for id %d", vdev_id);
		return;
	}
	sme_debug("set HE testbed defaults");
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.amsdu_in_ampdu = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.twt_request = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.omi_a_ctrl = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.he_ppdu_20_in_160_80p80Mhz = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.he_ppdu_20_in_40Mhz_2G = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.he_ppdu_80_in_160_80p80Mhz = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.dcm_enc_tx = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.dcm_enc_rx = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.max_nc = 0;
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap.trigger_frm_mac_pad =
					QCA_WLAN_HE_16US_OF_PROCESS_TIME;
	csr_update_session_he_cap(mac_ctx, session);
}

void sme_reset_he_caps(mac_handle_t mac_handle, uint8_t vdev_id)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct csr_roam_session *session;

	session = CSR_GET_SESSION(mac_ctx, vdev_id);

	if (!session) {
		sme_err("No session for id %d", vdev_id);
		return;
	}
	sme_debug("reset HE caps");
	mac_ctx->mlme_cfg->he_caps.dot11_he_cap =
		mac_ctx->mlme_cfg->he_caps.he_cap_orig;
	csr_update_session_he_cap(mac_ctx, session);
}
#endif

uint8_t sme_get_mcs_idx(uint16_t max_rate, uint8_t rate_flags,
			uint8_t *nss, uint8_t *mcs_rate_flags)
{
	return wma_get_mcs_idx(max_rate, rate_flags, nss, mcs_rate_flags);
}

bool sme_find_session_by_bssid(mac_handle_t mac_handle, uint8_t *bssid)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	bool ret;

	ret = csr_find_session_by_bssid(mac_ctx, bssid);

	return ret;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
QDF_STATUS sme_get_sta_cxn_info(mac_handle_t mac_handle, uint32_t session_id,
				char *buf, uint32_t buf_sz)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct tagCsrRoamConnectedProfile *conn_profile;
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return status;
	if (!session || !session->pCurRoamProfile) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}
	conn_profile = &session->connectedProfile;
	if (!conn_profile) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}
	csr_get_sta_cxn_info(mac_ctx, session, conn_profile, buf, buf_sz);
end:
	sme_release_global_lock(&mac_ctx->sme);

	return status;
}
#endif
QDF_STATUS
sme_get_roam_scan_stats(mac_handle_t mac_handle,
			roam_scan_stats_cb cb, void *context,
			uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};
	struct sir_roam_scan_stats *req;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = vdev_id;
	req->cb = cb;
	req->context = context;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		msg.bodyptr = req;
		msg.type = WMA_GET_ROAM_SCAN_STATS;
		msg.reserved = 0;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&msg);
		sme_release_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  FL("post roam scan stats req failed"));
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(req);
		}
	} else {
		qdf_mem_free(req);
	}

	return status;
}

void sme_update_score_config(mac_handle_t mac_handle,
			     struct scoring_config *score_config)
{
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	struct wlan_mlme_scoring_cfg *mlme_scoring_cfg;

	mlme_scoring_cfg = &mac_ctx->mlme_cfg->scoring;

	score_config->weight_cfg.rssi_weightage =
		mlme_scoring_cfg->weight_cfg.rssi_weightage;
	score_config->weight_cfg.ht_caps_weightage =
		mlme_scoring_cfg->weight_cfg.ht_caps_weightage;
	score_config->weight_cfg.vht_caps_weightage =
		mlme_scoring_cfg->weight_cfg.vht_caps_weightage;
	score_config->weight_cfg.he_caps_weightage =
		mlme_scoring_cfg->weight_cfg.he_caps_weightage;
	score_config->weight_cfg.chan_width_weightage =
		mlme_scoring_cfg->weight_cfg.chan_width_weightage;
	score_config->weight_cfg.chan_band_weightage =
		mlme_scoring_cfg->weight_cfg.chan_band_weightage;
	score_config->weight_cfg.nss_weightage =
		mlme_scoring_cfg->weight_cfg.nss_weightage;
	score_config->weight_cfg.beamforming_cap_weightage =
		mlme_scoring_cfg->weight_cfg.beamforming_cap_weightage;
	score_config->weight_cfg.pcl_weightage =
		mlme_scoring_cfg->weight_cfg.pcl_weightage;
	score_config->weight_cfg.channel_congestion_weightage =
		mlme_scoring_cfg->weight_cfg.channel_congestion_weightage;
	score_config->weight_cfg.oce_wan_weightage =
		mlme_scoring_cfg->weight_cfg.oce_wan_weightage;

	score_config->bandwidth_weight_per_index =
		mlme_scoring_cfg->bandwidth_weight_per_index;
	score_config->nss_weight_per_index =
		mlme_scoring_cfg->nss_weight_per_index;
	score_config->band_weight_per_index =
		mlme_scoring_cfg->band_weight_per_index;

	score_config->rssi_score.best_rssi_threshold =
		mlme_scoring_cfg->rssi_score.best_rssi_threshold;
	score_config->rssi_score.good_rssi_threshold =
		mlme_scoring_cfg->rssi_score.good_rssi_threshold;
	score_config->rssi_score.bad_rssi_threshold =
		mlme_scoring_cfg->rssi_score.bad_rssi_threshold;
	score_config->rssi_score.good_rssi_pcnt =
		mlme_scoring_cfg->rssi_score.good_rssi_pcnt;
	score_config->rssi_score.bad_rssi_pcnt =
		mlme_scoring_cfg->rssi_score.bad_rssi_pcnt;
	score_config->rssi_score.good_rssi_bucket_size =
		mlme_scoring_cfg->rssi_score.good_rssi_bucket_size;
	score_config->rssi_score.bad_rssi_bucket_size =
		mlme_scoring_cfg->rssi_score.bad_rssi_bucket_size;
	score_config->rssi_score.rssi_pref_5g_rssi_thresh =
		mlme_scoring_cfg->rssi_score.rssi_pref_5g_rssi_thresh;

	score_config->esp_qbss_scoring.num_slot =
		mlme_scoring_cfg->esp_qbss_scoring.num_slot;
	score_config->esp_qbss_scoring.score_pcnt3_to_0 =
		mlme_scoring_cfg->esp_qbss_scoring.score_pcnt3_to_0;
	score_config->esp_qbss_scoring.score_pcnt7_to_4 =
		mlme_scoring_cfg->esp_qbss_scoring.score_pcnt7_to_4;
	score_config->esp_qbss_scoring.score_pcnt11_to_8 =
		mlme_scoring_cfg->esp_qbss_scoring.score_pcnt11_to_8;
	score_config->esp_qbss_scoring.score_pcnt15_to_12 =
		mlme_scoring_cfg->esp_qbss_scoring.score_pcnt15_to_12;

	score_config->oce_wan_scoring.num_slot =
		mlme_scoring_cfg->oce_wan_scoring.num_slot;
	score_config->oce_wan_scoring.score_pcnt3_to_0 =
		mlme_scoring_cfg->oce_wan_scoring.score_pcnt3_to_0;
	score_config->oce_wan_scoring.score_pcnt7_to_4 =
		mlme_scoring_cfg->oce_wan_scoring.score_pcnt7_to_4;
	score_config->oce_wan_scoring.score_pcnt11_to_8 =
		mlme_scoring_cfg->oce_wan_scoring.score_pcnt11_to_8;
	score_config->oce_wan_scoring.score_pcnt15_to_12 =
		mlme_scoring_cfg->oce_wan_scoring.score_pcnt15_to_12;
}

void sme_enable_fw_module_log_level(mac_handle_t mac_handle, int vdev_id)
{
	QDF_STATUS status;
	struct mac_context *mac_ctx = MAC_CONTEXT(mac_handle);
	uint8_t *enable_fw_module_log_level;
	uint8_t enable_fw_module_log_level_num;
	uint8_t count = 0;
	uint32_t value = 0;
	int ret;

	status = ucfg_fwol_get_enable_fw_module_log_level(
			mac_ctx->psoc, &enable_fw_module_log_level,
			&enable_fw_module_log_level_num);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	while (count < enable_fw_module_log_level_num) {
		/*
		 * FW module log level input array looks like
		 * below:
		 * enable_fw_module_log_level = {<FW Module ID>,
		 * <Log Level>,...}
		 * For example:
		 * enable_fw_module_log_level=
		 * {1,0,2,1,3,2,4,3,5,4,6,5,7,6}
		 * Above input array means :
		 * For FW module ID 1 enable log level 0
		 * For FW module ID 2 enable log level 1
		 * For FW module ID 3 enable log level 2
		 * For FW module ID 4 enable log level 3
		 * For FW module ID 5 enable log level 4
		 * For FW module ID 6 enable log level 5
		 * For FW module ID 7 enable log level 6
		 */

		if ((enable_fw_module_log_level[count] > WLAN_MODULE_ID_MAX) ||
		    (enable_fw_module_log_level[count + 1] > DBGLOG_LVL_MAX)) {
			sme_err("Module id %d or dbglog level %d input value is more than max",
				enable_fw_module_log_level[count],
				enable_fw_module_log_level[count + 1]);
			count += 2;
			continue;
		}

		value = enable_fw_module_log_level[count] << 16;
		value |= enable_fw_module_log_level[count + 1];
		ret = sme_cli_set_command(vdev_id,
					  WMI_DBGLOG_MOD_LOG_LEVEL,
					  value, DBG_CMD);
		if (ret != 0)
			sme_err("Failed to enable FW module log level %d ret %d",
				value, ret);

		count += 2;
	}
}

#ifdef WLAN_FEATURE_MOTION_DETECTION
/**
 * sme_set_md_host_evt_cb - Register/set motion detection callback
 * @mac_handle: mac handle
 * @callback_fn: motion detection callback function pointer
 * @hdd_ctx: hdd context
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_set_md_host_evt_cb(
	mac_handle_t mac_handle,
	QDF_STATUS (*callback_fn)(void *ctx, struct sir_md_evt *event),
	void *hdd_ctx
)
{
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		mac->sme.md_host_evt_cb = callback_fn;
		mac->sme.md_ctx = hdd_ctx;
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}

/**
 * sme_motion_det_config - Post motion detection configuration msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_config: motion detection configuration
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_config(mac_handle_t mac_handle,
				 struct sme_motion_det_cfg *motion_det_config)
{
	struct scheduler_msg msg;
	struct sme_motion_det_cfg *motion_det_cfg;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		motion_det_cfg =
				qdf_mem_malloc(sizeof(*motion_det_cfg));
		if (!motion_det_cfg) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		*motion_det_cfg = *motion_det_config;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_MOTION_DET_CONFIG;
		msg.bodyptr = motion_det_cfg;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(motion_det_cfg);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}

/**
 * sme_motion_det_enable - Post motion detection start/stop msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_enable: motion detection start/stop
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_enable(mac_handle_t mac_handle,
				 struct sme_motion_det_en *motion_det_enable)
{
	struct scheduler_msg msg;
	struct sme_motion_det_en *motion_det_en;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		motion_det_en = qdf_mem_malloc(sizeof(*motion_det_en));
		if (!motion_det_en) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		*motion_det_en = *motion_det_enable;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_MOTION_DET_ENABLE;
		msg.bodyptr = motion_det_en;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(motion_det_en);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}

/**
 * sme_motion_det_base_line_config - Post md baselining cfg msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_base_line_config: motion detection baselining configuration
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_base_line_config(
	mac_handle_t mac_handle,
	struct sme_motion_det_base_line_cfg *motion_det_base_line_config)
{
	struct scheduler_msg msg;
	struct sme_motion_det_base_line_cfg *motion_det_base_line_cfg;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		motion_det_base_line_cfg =
			qdf_mem_malloc(sizeof(*motion_det_base_line_cfg));

		if (!motion_det_base_line_cfg) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		*motion_det_base_line_cfg = *motion_det_base_line_config;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_MOTION_DET_BASE_LINE_CONFIG;
		msg.bodyptr = motion_det_base_line_cfg;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(motion_det_base_line_cfg);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}

/**
 * sme_motion_det_base_line_enable - Post md baselining enable msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_base_line_enable: motion detection baselining start/stop
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_base_line_enable(
	mac_handle_t mac_handle,
	struct sme_motion_det_base_line_en *motion_det_base_line_enable)
{
	struct scheduler_msg msg;
	struct sme_motion_det_base_line_en *motion_det_base_line_en;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
		motion_det_base_line_en =
			qdf_mem_malloc(sizeof(*motion_det_base_line_en));

		if (!motion_det_base_line_en) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		*motion_det_base_line_en = *motion_det_base_line_enable;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_MOTION_DET_BASE_LINE_ENABLE;
		msg.bodyptr = motion_det_base_line_en;

		qdf_status = scheduler_post_message(QDF_MODULE_ID_SME,
						    QDF_MODULE_ID_WMA,
						    QDF_MODULE_ID_WMA,
						    &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(motion_det_base_line_en);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}
#endif /* WLAN_FEATURE_MOTION_DETECTION */
#ifdef FW_THERMAL_THROTTLE_SUPPORT
/**
 * sme_set_thermal_throttle_cfg() - SME API to set the thermal throttle
 * configuration parameters
 * @mac_handle: Opaque handle to the global MAC context
 * @enable: Enable Throttle
 * @dc: duty cycle in msecs
 * @dc_off_percent: duty cycle off percentage
 * @prio: Disables the transmit queues in fw that have lower priority
 * than value defined by prio
 * @target_temp: Target temperature
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_throttle_cfg(mac_handle_t mac_handle, bool enable,
					uint32_t dc, uint32_t dc_off_percent,
					uint32_t prio, uint32_t target_temp)
{
	struct scheduler_msg msg;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct thermal_mitigation_params *therm_cfg_params;

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == qdf_status) {
		therm_cfg_params = qdf_mem_malloc(sizeof(*therm_cfg_params));
		if (!therm_cfg_params) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		therm_cfg_params->enable = enable;
		therm_cfg_params->dc = dc;
		therm_cfg_params->levelconf[0].dcoffpercent = dc_off_percent;
		therm_cfg_params->levelconf[0].priority = prio;
		therm_cfg_params->levelconf[0].tmplwm = target_temp;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_THERMAL_THROTTLE_CFG;
		msg.reserved = 0;
		msg.bodyptr = therm_cfg_params;

		qdf_status =  scheduler_post_message(QDF_MODULE_ID_SME,
						     QDF_MODULE_ID_WMA,
						     QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(therm_cfg_params);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}

/**
 * sme_set_thermal_mgmt() - SME API to set the thermal management params
 * @mac_handle: Opaque handle to the global MAC context
 * @lower_thresh_deg: Lower threshold value of Temperature
 * @higher_thresh_deg: Higher threshold value of Temperature
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_mgmt(mac_handle_t mac_handle,
				uint16_t lower_thresh_deg,
				uint16_t higher_thresh_deg)
{
	struct scheduler_msg msg;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	t_thermal_cmd_params *therm_mgmt_cmd;

	qdf_status = sme_acquire_global_lock(&mac->sme);
	if (QDF_STATUS_SUCCESS == qdf_status) {
		therm_mgmt_cmd = qdf_mem_malloc(sizeof(*therm_mgmt_cmd));
		if (!therm_mgmt_cmd) {
			sme_release_global_lock(&mac->sme);
			return QDF_STATUS_E_NOMEM;
		}

		therm_mgmt_cmd->minTemp = lower_thresh_deg;
		therm_mgmt_cmd->maxTemp = higher_thresh_deg;
		therm_mgmt_cmd->thermalEnable = 1;

		qdf_mem_set(&msg, sizeof(msg), 0);
		msg.type = WMA_SET_THERMAL_MGMT;
		msg.reserved = 0;
		msg.bodyptr = therm_mgmt_cmd;

		qdf_status =  scheduler_post_message(QDF_MODULE_ID_SME,
						     QDF_MODULE_ID_WMA,
						     QDF_MODULE_ID_WMA, &msg);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			qdf_mem_free(therm_mgmt_cmd);
			qdf_status = QDF_STATUS_E_FAILURE;
		}
		sme_release_global_lock(&mac->sme);
	}
	return qdf_status;
}
#endif /* FW_THERMAL_THROTTLE_SUPPORT */

QDF_STATUS sme_update_hidden_ssid_status_cb(mac_handle_t mac_handle,
					    hidden_ssid_cb cb)
{
	QDF_STATUS status;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac->sme.hidden_ssid_cb = cb;
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

QDF_STATUS sme_update_owe_info(struct mac_context *mac,
			       struct assoc_ind *assoc_ind)
{
	QDF_STATUS status;

	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = csr_update_owe_info(mac, assoc_ind);
		sme_release_global_lock(&mac->sme);
	}

	return status;
}

#ifdef WLAN_MWS_INFO_DEBUGFS
QDF_STATUS
sme_get_mws_coex_info(mac_handle_t mac_handle, uint32_t vdev_id,
		      uint32_t cmd_id, void (*callback_fn)(void *coex_info_data,
							   void *context,
							   wmi_mws_coex_cmd_id
							   cmd_id),
		      void *context)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct mac_context *mac = MAC_CONTEXT(mac_handle);
	struct scheduler_msg msg = {0};
	struct sir_get_mws_coex_info *req;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		sme_err("Failed allocate memory for MWS coex info req");
		return QDF_STATUS_E_NOMEM;
	}

	req->vdev_id = vdev_id;
	req->cmd_id  = cmd_id;
	mac->sme.mws_coex_info_state_resp_callback = callback_fn;
	mac->sme.mws_coex_info_ctx = context;
	status = sme_acquire_global_lock(&mac->sme);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		msg.bodyptr = req;
		msg.type = WMA_GET_MWS_COEX_INFO_REQ;
		status = scheduler_post_message(QDF_MODULE_ID_SME,
						QDF_MODULE_ID_WMA,
						QDF_MODULE_ID_WMA,
						&msg);
		sme_release_global_lock(&mac->sme);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("post MWS coex info req failed");
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(req);
		}
	} else {
		sme_err("sme_acquire_global_lock failed");
		qdf_mem_free(req);
	}

	return status;
}
#endif /* WLAN_MWS_INFO_DEBUGFS */
