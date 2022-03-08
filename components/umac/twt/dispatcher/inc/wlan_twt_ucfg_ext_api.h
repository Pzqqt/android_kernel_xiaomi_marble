/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _WLAN_TWT_UCFG_EXT_API_H
#define _WLAN_TWT_UCFG_EXT_API_H


#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>
#include <include/wlan_mlme_cmn.h>

/* dialog_id used to get all peer's twt session parameters */
#define TWT_GET_ALL_PEER_PARAMS_DIALOG_ID (0xFF)

/* Valid dialog_id 0 to (0xFF - 1) */
#define TWT_MAX_DIALOG_ID (TWT_GET_ALL_PEER_PARAMS_DIALOG_ID - 1)

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * ucfg_twt_psoc_open() - TWT psoc open
 * @psoc: Pointer to global PSOC object
 *
 * Upon psoc open, this function initializes the twt config params
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_twt_psoc_close() - TWT psoc close
 * @psoc: Pointer to global PSOC object
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_twt_update_psoc_config() - TWT update config
 * @psoc: Pointer to global PSOC object
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_update_psoc_config(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_twt_setup_req() - TWT setup
 * @psoc: Pointer to global PSOC object
 * @params: add dialog params
 * @context: twt context
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_add_dialog_param *params,
			      void *context);

/**
 * ucfg_twt_teardown_req() - TWT teardown
 * @psoc: Pointer to global PSOC object
 * @params: delete dialog params
 * @context: twt context
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
				 struct twt_del_dialog_param *params,
				 void *context);

/**
 * ucfg_twt_pause_req() - Process TWT pause req
 * @psoc: psoc
 * @params: pause dialog cmd param
 * @context: context
 *
 * Perform validations and set WLAN_TWT_SUSPEND
 * in progress
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_twt_pause_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_pause_dialog_cmd_param *params,
			      void *context);

/**
 * ucfg_twt_resume_req() - Process TWT resume req
 * @psoc: psoc
 * @params: resume dialog cmd param
 * @context: context
 *
 * Perform validations and set WLAN_TWT_RESUME
 * in progress
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_twt_resume_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_resume_dialog_cmd_param *params,
			      void *context);

/**
 * ucfg_twt_nudge_req() - Process TWT nudge req
 * @psoc: psoc
 * @params: nudge dialog cmd param
 * @context: context
 *
 * Perform validations and set WLAN_TWT_NUDGE
 * in progress
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_nudge_dialog_cmd_param *params,
			      void *context);

/**
 * ucfg_twt_is_max_sessions_reached() - Check if the maximum number of
 * TWT sessions reached or not excluding the given dialog_id
 * @psoc: Pointer to global PSOC object
 * @peer_mac: Global peer mac address
 * @dialog_id: dialog id
 *
 * Check if the number of active TWT sessions is equal to the maximum number
 * of TWT sessions supported. Only count the TWT session slot if it not
 * TWT_ALL_SESSIONS_DIALOG_ID and dialog id is different from input dialog_id,
 * because if same dialog_id already exists in the TWT sessions, we should
 * return false since re-negotiation is supported on existing dialog_id.
 *
 * Return: True if slot is available for dialog_id, false otherwise
 */
bool ucfg_twt_is_max_sessions_reached(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id);

/**
 * ucfg_twt_is_setup_in_progress() - Get if TWT setup command is in progress
 * for given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: True if Setup is in progress
 */
bool ucfg_twt_is_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id);

/**
 * ucfg_twt_set_command_in_progress() - Set TWT command is in progress
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @cmd: TWT command
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id,
				 enum wlan_twt_commands cmd);

/**
 * ucfg_twt_reset_active_command() - Reset active command to WLAN_TWT_NONE
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_reset_active_command(struct wlan_objmgr_psoc *psoc,
			      struct qdf_mac_addr *peer_mac,
			      uint8_t dialog_id);

/**
 * ucfg_twt_init_context() - Initialize TWT context
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id);

/**
 * ucfg_twt_set_osif_cb() - Set TWT osif callbacks
 * @osif_twt_ops: pointer to global osif ops
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS
ucfg_twt_set_osif_cb(osif_twt_get_global_ops_cb osif_twt_ops);

/**
 * ucfg_twt_update_beacon_template() - update beacon template
 *
 * SoftAP (SAP) is the beaconing entity, as per current requirement
 * during Single Channel Concurrency (SCC) or Multi-Channel Concurrency (MCC)
 * TWT is not supported on STA as well as SAP.
 *
 * Whenever SAP is forming SCC/MCC, this function shall be called to update the
 * beacon, underlying LIM layer based the TWT responder flag, it disables the
 * TWT responder advertisement bit in the beacon.
 *
 * When SAP moves from SCC/MCC to Standalone, this function shall be called
 * to update the beacon, underlying LIM layer based the TWT responder flag,
 * it enables  the TWT responder advertisement bit in the beacon.
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS ucfg_twt_update_beacon_template(void);

/**
 * ucfg_twt_is_setup_done() - check if TWT setup is done or not
 * for given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: True if Setup is done, false otherwise
 */
bool
ucfg_twt_is_setup_done(struct wlan_objmgr_psoc *psoc,
		       struct qdf_mac_addr *peer_mac,
		       uint8_t dialog_id);

/**
 * ucfg_twt_get_session_state() - get TWT session state for a given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: TWT session state
 */
enum wlan_twt_session_state
ucfg_twt_get_session_state(struct wlan_objmgr_psoc *psoc,
			   struct qdf_mac_addr *peer_mac, uint8_t dialog_id);

bool ucfg_twt_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd,
				     enum wlan_twt_commands *pactive_cmd);

#else
static inline
QDF_STATUS ucfg_twt_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_update_psoc_config(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_add_dialog_param *params,
			      void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
				     struct twt_del_dialog_param *params,
				     void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_pause_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_pause_dialog_cmd_param *params,
			      void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_twt_resume_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_resume_dialog_cmd_param *params,
			      void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
ucfg_twt_set_osif_cb(osif_twt_get_global_ops_cb osif_twt_ops)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool
ucfg_twt_is_setup_done(struct wlan_objmgr_psoc *psoc,
		       struct qdf_mac_addr *peer_mac,
		       uint8_t dialog_id)
{
	return false;
}

static inline enum wlan_twt_session_state
ucfg_twt_get_session_state(struct wlan_objmgr_psoc *psoc,
			   struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED;
}

static inline bool
ucfg_twt_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				struct qdf_mac_addr *peer_mac,
				uint8_t dialog_id,
				enum wlan_twt_commands cmd,
				enum wlan_twt_commands *pactive_cmd)
{
	return false;
}
#endif
#endif
