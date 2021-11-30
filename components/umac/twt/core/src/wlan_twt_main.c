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
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_twt_api.h>
#include <wlan_utility.h>
#include <wlan_mlme_api.h>
#include <wlan_mlme_main.h>
#include "wlan_twt_main.h"
#include "twt/core/src/wlan_twt_priv.h"

/**
 * wlan_twt_add_session()  - Add TWT session entry in the TWT context
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 * @context: request context
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_add_session(struct wlan_objmgr_psoc *psoc,
		     struct qdf_mac_addr *peer_mac,
		     uint8_t dialog_id,
		     void *context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_set_command_in_progress() - Set TWT command is in progress
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @cmd: TWT command
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id,
				 enum wlan_twt_commands cmd)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_is_twt_notify_in_progress() - is TWT notify in progress
 * @psoc: Pointer to psoc object
 * @vdev_id: VDEV identifier
 *
 * Return: True if twt_notify is in progress.
 */
static bool
wlan_is_twt_notify_in_progress(struct wlan_objmgr_psoc *psoc, uint32_t vdev_id)
{
	return false;
}

/**
 * wlan_twt_set_wait_for_notify()  - Set wait for notify flag
 * @psoc: Pointer to psoc object
 * @vdev_id: VDEV identifier
 * @is_set: Set or clear notify flag
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_set_wait_for_notify(struct wlan_objmgr_psoc *psoc, uint32_t vdev_id,
			     bool is_set)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_any_peer_cmd_in_progress() - Iterate through the list of peers
 * and check if the given command is in progress
 * @psoc: Pointer to psoc object
 * @vdev_id: Vdev id
 * @dialog_id: Dialog id
 * @cmd: command
 *
 * This API is used to check for the given @dialog_id if the
 * @cmd command is in progress for any of the peers.
 *
 * Return: true if command is in progress, false otherwise
 */
static bool
wlan_twt_any_peer_cmd_in_progress(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id,
				  uint8_t dialog_id,
				  enum wlan_twt_commands cmd)
{
	return false;
}

/**
 * wlan_twt_sap_peer_is_cmd_in_progress() - For a given peer_mac check if
 * the given command is in progress
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @cmd: TWT command
 *
 * Return: True if given command is in progress.
 */
static bool
wlan_twt_sap_peer_is_cmd_in_progress(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd)
{
	return false;
}

/**
 * wlan_twt_sap_is_command_in_progress() - Based on the input peer mac address
 * invoke the appropriate function to check if the given command is in progress
 * @psoc: Pointer to psoc object
 * @vdev_id: Vdev id
 * @peer_mac: Peer MAC address
 * @dialog_id: Dialog id
 * @cmd: command
 *
 * If the input @peer_mac is a broadcast MAC address then the expectation is
 * to iterate through the list of all peers and check for any given @dialog_id
 * if the command @cmd is in progress.
 * Note: If @peer_mac is broadcast MAC address then @dialog_id shall always
 * be TWT_ALL_SESSIONS_DIALOG_ID.
 * For ex: If TWT teardown command is issued on a particular @dialog_id and
 * non-broadcast peer mac and FW response is not yet received then for that
 * particular @dialog_id and @peer_mac, TWT teardown is the active command,
 * then if the driver receives another TWT teardown request with broadcast
 * peer mac, then API wlan_twt_any_peer_cmd_in_progress() shall iterate
 * through the list of all peers and returns command in progress as true.
 *
 * If the input @peer_mac is a non-broadcast MAC address then
 * wlan_twt_sap_peer_is_cmd_in_progress() shall check only for that
 * particular @peer_mac and @dialog_id.
 *
 * Return: true if command is in progress, false otherwise
 */
static bool
wlan_twt_sap_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				    uint32_t vdev_id,
				    struct qdf_mac_addr *peer_mac,
				    uint8_t dialog_id,
				    enum wlan_twt_commands cmd)
{
	if (qdf_is_macaddr_broadcast(peer_mac)) {
		return wlan_twt_any_peer_cmd_in_progress(psoc, vdev_id,
							 dialog_id, cmd);
	} else {
		return wlan_twt_sap_peer_is_cmd_in_progress(psoc, peer_mac,
							    dialog_id, cmd);
	}
}

/**
 * wlan_twt_sap_add_session() - Based on the input peer mac address
 * invoke the appropriate function to add dialog_id to the TWT session context
 * @psoc: Pointer to psoc object
 * @vdev_id: Vdev id
 * @peer_mac: Peer MAC address
 * @dialog_id: Dialog id
 *
 * If the input @peer_mac is a broadcast MAC address then there is nothing
 * to do, because the initialized structure is already in the expected format
 * Note: If @peer_mac is broadcast MAC address then @dialog_id shall always
 * be TWT_ALL_SESSIONS_DIALOG_ID.
 *
 * If the input @peer_mac is a non-broadcast MAC address then
 * wlan_twt_add_session() shall add the @dialog_id to the @peer_mac
 * TWT session context.
 *
 * Return: None
 */
static void
wlan_twt_sap_add_session(struct wlan_objmgr_psoc *psoc,
			 uint8_t vdev_id,
			 struct qdf_mac_addr *peer_mac,
			 uint8_t dialog_id)
{
	if (!qdf_is_macaddr_broadcast(peer_mac))
		wlan_twt_add_session(psoc, peer_mac, dialog_id, NULL);
}

/**
 * wlan_twt_sap_set_all_peers_cmd_in_progress()  - Iterate through the list
 * of peers and set the command in the TWT session entry in the TWT context
 * @psoc: Pointer to global psoc object
 * @vdev_id: Vdev id
 * @dialog_id: Dialog ID
 * @cmd: Command
 *
 * This API iterates through the list of peers and updates the active
 * command to @cmd for the given dialog_id.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_sap_set_all_peers_cmd_in_progress(struct wlan_objmgr_psoc *psoc,
					   uint8_t vdev_id,
					   uint8_t dialog_id,
					   enum wlan_twt_commands cmd)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_sap_set_command_in_progress() - Based on the input peer mac address
 * invoke the appropriate function to set the command is in progress
 * @psoc: Pointer to psoc object
 * @vdev_id: Vdev id
 * @peer_mac: Peer MAC address
 * @dialog_id: Dialog id
 * @cmd: command
 *
 * If the input @peer_mac is a broadcast MAC address then the expectation is
 * to iterate through the list of all peers and set the active command to @cmd
 * for the given @dialog_id
 * Note: If @peer_mac is broadcast MAC address then @dialog_id shall always
 * be TWT_ALL_SESSIONS_DIALOG_ID.
 * For ex: If TWT teardown command is issued on broadcast @peer_mac, then
 * it is same as issuing TWT teardown for all the peers (all TWT sessions).
 * Invoking wlan_twt_sap_set_all_peers_cmd_in_progress() shall iterate through
 * all the peers and set the active command to @cmd.
 *
 * If the input @peer_mac is a non-broadcast MAC address then
 * wlan_twt_set_command_in_progress() shall set the active command to @cmd
 * only for that particular @peer_mac and @dialog_id.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_sap_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd)
{
	if (qdf_is_macaddr_broadcast(peer_mac)) {
		return wlan_twt_sap_set_all_peers_cmd_in_progress(psoc,
								  vdev_id,
								  dialog_id,
								  cmd);
	} else {
		return wlan_twt_set_command_in_progress(psoc, peer_mac,
							dialog_id, cmd);
	}
}

/**
 * wlan_twt_init_all_peers_context()  - Iterate through the list
 * of peers and initialize the TWT context structure
 * @psoc: Pointer to global psoc object
 * @vdev_id: Vdev id
 * @dialog_id: Dialog ID
 *
 * This API iterates through the list of peers and initializes
 * the TWT context structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_init_all_peers_context(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id,
				uint8_t dialog_id)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_sap_init_context() - Based on the input peer mac address
 * invoke the appropriate function to initialize the TWT session context
 * @psoc: Pointer to psoc object
 * @vdev_id: Vdev id
 * @peer_mac: Peer MAC address
 * @dialog_id: Dialog id
 *
 * If the input @peer_mac is a broadcast MAC address then the expectation is
 * to iterate through the list of all peers and initialize the TWT session
 * context
 * Note: If @peer_mac is broadcast MAC address then @dialog_id shall always
 * be TWT_ALL_SESSIONS_DIALOG_ID.
 * For ex: If TWT teardown command is issued on broadcast @peer_mac, then
 * it is same as issuing TWT teardown for all the peers (all TWT sessions).
 * Then active command for all the peers is set to @WLAN_TWT_TERMINATE.
 * Upon receiving the TWT teardown WMI event, wlan_twt_init_all_peers_context()
 * shall iterate through the list of all peers and initializes the TWT session
 * context back to its initial state.
 *
 * If the input @peer_mac is a non-broadcast MAC address then
 * wlan_twt_init_context() shall initialize the TWT session context
 * only for that particular @peer_mac and @dialog_id.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_sap_init_context(struct wlan_objmgr_psoc *psoc,
			  uint8_t vdev_id,
			  struct qdf_mac_addr *peer_mac,
			  uint8_t dialog_id)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_is_vdev_connected_to_peer()  - find if peer exists in the vdev peer list
 * @psoc: Pointer to global psoc object
 * @vdev_id: Vdev id
 * @peer_macaddr: peer mac address
 *
 * This API finds if for the given vdev there exists a peer with the given
 * @peer_macaddr
 *
 * Return: true if exists, false otherwise
 */
static bool
wlan_is_vdev_connected_to_peer(struct wlan_objmgr_psoc *psoc, uint32_t vdev_id,
			       struct qdf_mac_addr *peer_macaddr)
{
	return false;
}

/**
 * wlan_twt_is_setup_done()  - Get if TWT session is established for given
 * dialog id.
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: Return true if TWT session exists for given dialog ID.
 */
static
bool wlan_twt_is_setup_done(struct wlan_objmgr_psoc *psoc,
			    struct qdf_mac_addr *peer_mac, uint8_t dialog_id)
{
	return false;
}

bool wlan_twt_is_max_sessions_reached(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id)
{
	return false;
}

bool wlan_twt_is_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id)
{
	return false;
}

/**
 * wlan_twt_set_ack_context() - set twt ack context
 * for given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_set_ack_context(struct wlan_objmgr_psoc *psoc,
			 struct qdf_mac_addr *peer_mac,
			 uint8_t dialog_id,
			 void *context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_get_ack_context() - get twt ack context
 * for given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_twt_get_ack_context(struct wlan_objmgr_psoc *psoc,
			 struct qdf_mac_addr *peer_mac,
			 uint8_t dialog_id,
			 void **context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_is_command_in_progress() - Check if given command is in progress
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @cmd: TWT command
 * @active_cmd: Fill the active command in this output parameter
 *
 * Return: True if given command is in progress.
 */
static
bool wlan_twt_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd,
				     enum wlan_twt_commands *pactive_cmd)
{
	return false;
}

QDF_STATUS wlan_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_add_dialog_param *req,
			      void *context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_sta_teardown_req() - station TWT teardown request
 * @psoc: Pointer to psoc object
 * @req: TWT del dialog parameters
 * @context: TWT context
 *
 * Return: QDF Status
 */
static QDF_STATUS
wlan_twt_sta_teardown_req(struct wlan_objmgr_psoc *psoc,
			  struct twt_del_dialog_param *req,
			  void *context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_sap_teardown_req() - sap TWT teardown request
 * @psoc: Pointer to psoc object
 * @req: TWT del dialog parameters
 * @context: TWT context
 *
 * Return: QDF Status
 */
static QDF_STATUS
wlan_twt_sap_teardown_req(struct wlan_objmgr_psoc *psoc,
			  struct twt_del_dialog_param *req)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
		      struct twt_del_dialog_param *req,
		      void *context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_ack_event_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_ack_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_twt_set_setup_done()  - Set TWT setup complete for given dialog ID
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @is_set: Set or clear the setup done flag
 *
 * Return: None
 */
static void
wlan_twt_set_setup_done(struct wlan_objmgr_psoc *psoc,
			struct qdf_mac_addr *peer_mac,
			uint8_t dialog_id, bool is_set)
{
}

/**
 * wlan_twt_set_session_state() - Set the TWT session state for the given dialog
 * id in TWT context
 * @peer: Pointer to peer object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @state: TWT session state
 *
 * Return: None
 */
static void
wlan_twt_set_session_state(struct wlan_objmgr_psoc *psoc,
			   struct qdf_mac_addr *peer_mac,
			   uint8_t dialog_id,
			   enum wlan_twt_session_state state)
{
}

/**
 * wlan_twt_process_renego_failure() - Process TWT re-negotiation failure
 * @psoc: psoc
 * @event: pointer to event buf containing twt response parameters
 *
 * Return: None
 */
static void
wlan_twt_process_renego_failure(struct wlan_objmgr_psoc *psoc,
				struct twt_add_dialog_complete_event *event)
{
}

/**
 * wlan_twt_process_add_initial_nego() - Process initial TWT setup or
 * re-negotiation successful setup
 * @psoc: psoc
 * @add_dialog_event: pointer to event buf containing twt response parameters
 *
 * Return: None
 */
static void
wlan_twt_process_add_initial_nego(struct wlan_objmgr_psoc *psoc,
				  struct twt_add_dialog_complete_event *event)
{
}

QDF_STATUS
wlan_twt_setup_complete_event_handler(struct wlan_objmgr_psoc *psoc,
				    struct twt_add_dialog_complete_event *event)
{
	return QDF_STATUS_SUCCESS;
}

static bool
wlan_is_twt_teardown_failed(enum HOST_TWT_DEL_STATUS teardown_status)
{
	return false;
}

static void
wlan_twt_handle_sta_del_dialog_event(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
}

QDF_STATUS
wlan_twt_teardown_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_pause_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_pause_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_resume_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_nudge_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_nudge_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_notify_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_notify_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}
