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
#include "include/wlan_mlme_cmn.h"
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_twt_api.h>
#include <wlan_utility.h>
#include <wlan_mlme_api.h>
#include <wlan_mlme_main.h>
#include "wlan_twt_main.h"
#include "twt/core/src/wlan_twt_priv.h"
#include "twt/core/src/wlan_twt_common.h"
#include <wlan_twt_tgt_if_ext_tx_api.h>

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
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);

	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id ==
			TWT_ALL_SESSIONS_DIALOG_ID) {
			peer_priv->session_info[i].dialog_id = dialog_id;
			peer_priv->session_info[i].twt_ack_ctx = context;
			break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id,
				 enum wlan_twt_commands cmd)
{
	struct wlan_objmgr_peer *peer;
	struct twt_peer_priv_obj *peer_priv;
	uint8_t i = 0;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err(" peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id == dialog_id ||
			dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			peer_priv->session_info[i].active_cmd = cmd;
			if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID)
				break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id)
{
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	peer_priv->num_twt_sessions = WLAN_MAX_TWT_SESSIONS_PER_PEER;
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id == dialog_id ||
			dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			peer_priv->session_info[i].setup_done = false;
			peer_priv->session_info[i].dialog_id =
					TWT_ALL_SESSIONS_DIALOG_ID;
			peer_priv->session_info[i].active_cmd =
					WLAN_TWT_NONE;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	twt_debug("init done");

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

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
	struct wlan_objmgr_vdev *vdev;
	struct twt_vdev_priv_obj *twt_vdev_priv;
	bool is_twt_notify_in_progress;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		twt_err("vdev object not found");
		return false;
	}

	twt_vdev_priv = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							WLAN_UMAC_COMP_TWT);
	if (!twt_vdev_priv) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
		twt_err("twt vdev private object is NULL");
		return false;
	}

	is_twt_notify_in_progress = twt_vdev_priv->twt_wait_for_notify;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);

	twt_debug("is_twt_notify_in_progress: %d", is_twt_notify_in_progress);
	return is_twt_notify_in_progress;
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
	struct wlan_objmgr_vdev *vdev;
	struct twt_vdev_priv_obj *twt_vdev_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_TWT_ID);
	if (!vdev) {
		twt_err("vdev object not found");
		return QDF_STATUS_E_INVAL;
	}

	twt_vdev_priv = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							WLAN_UMAC_COMP_TWT);
	if (!twt_vdev_priv) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
		twt_err("twt vdev private object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	twt_vdev_priv->twt_wait_for_notify = is_set;
	twt_debug("twt_wait_for_notify: %d", is_set);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TWT_ID);
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
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	bool is_setup_done = false;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return false;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer mlme component object is NULL");
		return false;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id == dialog_id ||
		    dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			is_setup_done =
				peer_priv->session_info[i].setup_done;

			if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID ||
			    is_setup_done)
				break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

	return is_setup_done;
}

bool wlan_twt_is_max_sessions_reached(struct wlan_objmgr_psoc *psoc,
				      struct qdf_mac_addr *peer_mac,
				      uint8_t dialog_id)
{
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i, num_twt_sessions = 0, max_twt_sessions;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return true;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return true;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);

	max_twt_sessions = peer_priv->num_twt_sessions;
	for (i = 0; i < max_twt_sessions; i++) {
		uint8_t existing_session_dialog_id =
				peer_priv->session_info[i].dialog_id;

		if (existing_session_dialog_id != TWT_ALL_SESSIONS_DIALOG_ID &&
			existing_session_dialog_id != dialog_id)
			num_twt_sessions++;
	}

	qdf_mutex_release(&peer_priv->twt_peer_lock);
	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

	twt_debug("num_twt_sessions:%d max_twt_sessions:%d",
			  num_twt_sessions, max_twt_sessions);
	return num_twt_sessions == max_twt_sessions;
}

bool wlan_twt_is_setup_in_progress(struct wlan_objmgr_psoc *psoc,
				   struct qdf_mac_addr *peer_mac,
				   uint8_t dialog_id)
{
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return false;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return false;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);

	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		bool setup_done = peer_priv->session_info[i].setup_done;
		uint8_t existing_session_dialog_id;

		existing_session_dialog_id =
			peer_priv->session_info[i].dialog_id;
		if (existing_session_dialog_id == dialog_id &&
		    existing_session_dialog_id != TWT_ALL_SESSIONS_DIALOG_ID &&
		    !setup_done) {
			qdf_mutex_release(&peer_priv->twt_peer_lock);
			wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
			return true;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

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
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);

	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		uint8_t existing_session_dialog_id;

		existing_session_dialog_id =
			peer_priv->session_info[i].dialog_id;
		if (existing_session_dialog_id == dialog_id ||
		    dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			peer_priv->session_info[i].twt_ack_ctx = context;

			if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID)
				break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
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
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		goto err;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		goto err;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		uint8_t existing_session_dialog_id;

		existing_session_dialog_id =
			peer_priv->session_info[i].dialog_id;
		if (existing_session_dialog_id == dialog_id ||
		    dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			*context = peer_priv->session_info[i].twt_ack_ctx;
			break;
		}
	}

	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
	return QDF_STATUS_SUCCESS;

err:
	*context = NULL;
	return QDF_STATUS_E_FAILURE;
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
	struct wlan_objmgr_peer *peer;
	struct twt_peer_priv_obj *peer_priv;
	enum wlan_twt_commands active_cmd;
	uint8_t i = 0;
	bool is_command_in_progress = false;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return false;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err(" peer twt component object is NULL");
		return false;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		active_cmd = peer_priv->session_info[i].active_cmd;

		if (pactive_cmd)
			*pactive_cmd = active_cmd;

		if (peer_priv->session_info[i].dialog_id == dialog_id ||
			dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			if (cmd == WLAN_TWT_ANY) {
				is_command_in_progress =
					(active_cmd != WLAN_TWT_NONE);

				if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID ||
					is_command_in_progress)
					break;
			} else {
				is_command_in_progress = (active_cmd == cmd);

				if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID ||
					is_command_in_progress)
					break;
			}
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);

	return is_command_in_progress;
}

QDF_STATUS wlan_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			      struct twt_add_dialog_param *req,
			      void *context)
{
	QDF_STATUS status;
	bool cmd_in_progress, notify_in_progress;
	enum wlan_twt_commands active_cmd = WLAN_TWT_NONE;

	if (wlan_twt_is_max_sessions_reached(psoc, &req->peer_macaddr,
					     req->dialog_id)) {
		twt_err("TWT add failed(dialog_id:%d), another TWT already exists (max reached)",
			req->dialog_id);
		return QDF_STATUS_E_AGAIN;
	}

	if (wlan_twt_is_setup_in_progress(psoc, &req->peer_macaddr,
					  req->dialog_id)) {
		twt_err("TWT setup is in progress for dialog_id:%d",
			req->dialog_id);
		return QDF_STATUS_E_ALREADY;
	}

	if (!mlme_get_user_ps(psoc, req->vdev_id)) {
		twt_warn("Power save mode disable");
		return QDF_STATUS_E_INVAL;
	}

	notify_in_progress = wlan_is_twt_notify_in_progress(psoc, req->vdev_id);
	if (notify_in_progress) {
		twt_warn("Waiting for TWT Notify");
		return QDF_STATUS_E_BUSY;
	}

	cmd_in_progress = wlan_twt_is_command_in_progress(
					psoc, &req->peer_macaddr, req->dialog_id,
					WLAN_TWT_ANY, &active_cmd);
	if (cmd_in_progress) {
		twt_debug("Already TWT command:%d is in progress", active_cmd);
		return QDF_STATUS_E_PENDING;
	}

	/*
	 * Add the dialog id to TWT context to drop back to back
	 * commands
	 */
	wlan_twt_add_session(psoc, &req->peer_macaddr, req->dialog_id, context);
	wlan_twt_set_ack_context(psoc, &req->peer_macaddr, req->dialog_id,
				 context);
	wlan_twt_set_command_in_progress(psoc, &req->peer_macaddr,
					 req->dialog_id, WLAN_TWT_SETUP);

	status = tgt_twt_setup_req_send(psoc, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("tgt_twt_setup_req_send failed (status=%d)", status);
		wlan_twt_init_context(psoc, &req->peer_macaddr, req->dialog_id);
	}

	return status;
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
	bool cmd_in_progress;
	enum wlan_twt_commands active_cmd = WLAN_TWT_NONE;
	QDF_STATUS status;

	if (!wlan_twt_is_setup_done(psoc, &req->peer_macaddr, req->dialog_id)) {
		twt_err("vdev%d: TWT session %d setup incomplete",
			  req->vdev_id, req->dialog_id);
		return QDF_STATUS_E_AGAIN;
	}

	cmd_in_progress =
		wlan_twt_is_command_in_progress(psoc, &req->peer_macaddr,
			req->dialog_id, WLAN_TWT_SETUP, &active_cmd) ||
		wlan_twt_is_command_in_progress(
			psoc, &req->peer_macaddr, req->dialog_id,
			WLAN_TWT_TERMINATE, &active_cmd);
	if (cmd_in_progress) {
		twt_debug("Already TWT command:%d is in progress", active_cmd);
		return QDF_STATUS_E_PENDING;
	}

	wlan_twt_set_ack_context(psoc, &req->peer_macaddr,
				 req->dialog_id, context);
	wlan_twt_set_command_in_progress(psoc, &req->peer_macaddr,
					 req->dialog_id, WLAN_TWT_TERMINATE);

	status = tgt_twt_teardown_req_send(psoc, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("tgt_twt_teardown_req_send failed (status=%d)", status);
		wlan_twt_set_command_in_progress(psoc, &req->peer_macaddr,
						 req->dialog_id, WLAN_TWT_NONE);
	}

	return status;
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
	enum QDF_OPMODE opmode;
	uint32_t pdev_id;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, req->vdev_id,
						WLAN_TWT_ID);
	if (pdev_id == WLAN_INVALID_PDEV_ID) {
		twt_err("Invalid pdev id");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_TWT_ID);
	if (!pdev) {
		twt_err("Invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	status = wlan_twt_check_all_twt_support(psoc, req->dialog_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("All TWT sessions not supported by target");
		return status;
	}

	opmode = wlan_get_opmode_from_vdev_id(pdev, req->vdev_id);

	switch (opmode) {
	case QDF_SAP_MODE:
		status = wlan_twt_sap_teardown_req(psoc, req);
		break;
	case QDF_STA_MODE:
		status = wlan_twt_sta_teardown_req(psoc, req, context);
		break;
	default:
		twt_err("TWT teardown not supported in mode: %d", opmode);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_TWT_ID);
	return status;
}

QDF_STATUS
wlan_twt_ack_event_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_ack_complete_event_param *event)
{
	void *ack_context = NULL;
	QDF_STATUS qdf_status;

	twt_debug("TWT ack status: %d", event->status);
	/* If the ack status is other than 0 (SUCCESS) then its a error.
	 * that means there won't be following TWT add/del/pause/resume/nudge
	 * event, hence clear the command in progress to NONE
	 */
	if (event->status) {
		qdf_status = wlan_twt_set_command_in_progress(psoc,
					 &event->peer_macaddr,
					 event->dialog_id, WLAN_TWT_NONE);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			return qdf_status;
	}
	qdf_status = wlan_twt_get_ack_context(psoc, &event->peer_macaddr,
					      event->dialog_id, &ack_context);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return qdf_status;

	qdf_status = mlme_twt_osif_ack_complete_ind(psoc, event, ack_context);

	return qdf_status;
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
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err(" peer twt component object is NULL");
		return;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);

	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id == dialog_id) {
			peer_priv->session_info[i].setup_done = is_set;
			twt_debug("setup done:%d dialog:%d", is_set, dialog_id);
			break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
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
	struct wlan_objmgr_peer *peer;
	struct twt_peer_priv_obj *peer_priv;
	uint8_t i;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found");
		return;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err(" peer twt component object is NULL");
		return;
	}

	twt_debug("set_state:%d for dialog_id:%d", state, dialog_id);
	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	for (i = 0; i < peer_priv->num_twt_sessions; i++) {
		if (peer_priv->session_info[i].dialog_id == dialog_id ||
		    dialog_id == TWT_ALL_SESSIONS_DIALOG_ID) {
			peer_priv->session_info[i].state = state;
			break;
		}
	}
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
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
	/* Reset the active TWT command to none */
	wlan_twt_set_command_in_progress(psoc,
					 &event->params.peer_macaddr,
					 event->params.dialog_id,
					 WLAN_TWT_NONE);

	mlme_twt_osif_setup_complete_ind(psoc, event, true);
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
	mlme_twt_osif_setup_complete_ind(psoc, event, false);

	/* Reset the active TWT command to none */
	wlan_twt_set_command_in_progress(psoc,
					 &event->params.peer_macaddr,
					 event->params.dialog_id,
					 WLAN_TWT_NONE);

	if (event->params.status) {
		/* Clear the stored TWT dialog ID as TWT setup failed */
		wlan_twt_init_context(psoc, &event->params.peer_macaddr,
				      event->params.dialog_id);
		return;
	}

	wlan_twt_set_setup_done(psoc, &event->params.peer_macaddr,
				event->params.dialog_id, true);

	wlan_twt_set_session_state(psoc, &event->params.peer_macaddr,
				   event->params.dialog_id,
				   WLAN_TWT_SETUP_STATE_ACTIVE);
}

QDF_STATUS
wlan_twt_setup_complete_event_handler(struct wlan_objmgr_psoc *psoc,
				    struct twt_add_dialog_complete_event *event)
{
	bool is_evt_allowed;
	bool setup_done;
	enum HOST_TWT_ADD_STATUS status = event->params.status;
	enum wlan_twt_commands active_cmd = WLAN_TWT_NONE;
	enum QDF_OPMODE opmode;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint32_t pdev_id, vdev_id;
	struct wlan_objmgr_pdev *pdev;

	vdev_id = event->params.vdev_id;
	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, vdev_id, WLAN_TWT_ID);
	if (pdev_id == WLAN_INVALID_PDEV_ID) {
		twt_err("Invalid pdev id");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_TWT_ID);
	if (!pdev) {
		twt_err("Invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	opmode = wlan_get_opmode_from_vdev_id(pdev, vdev_id);

	switch (opmode) {
	case QDF_SAP_MODE:
		mlme_twt_osif_setup_complete_ind(psoc, event, false);
		break;
	case QDF_STA_MODE:
		is_evt_allowed = wlan_twt_is_command_in_progress(
						psoc,
						&event->params.peer_macaddr,
						event->params.dialog_id,
						WLAN_TWT_SETUP, &active_cmd);

		if (!is_evt_allowed) {
			twt_debug("Drop TWT add dialog event for dialog_id:%d status:%d active_cmd:%d",
				  event->params.dialog_id, status,
				  active_cmd);
			qdf_status = QDF_STATUS_E_INVAL;
			goto cleanup;
		}

		setup_done = wlan_twt_is_setup_done(psoc,
						    &event->params.peer_macaddr,
						    event->params.dialog_id);
		twt_debug("setup_done:%d status:%d", setup_done, status);

		if (setup_done && status) {
			/*This is re-negotiation failure case */
			wlan_twt_process_renego_failure(psoc, event);
		} else {
			wlan_twt_process_add_initial_nego(psoc, event);
		}

		break;
	default:
		twt_debug("TWT Setup is not supported on %s",
			  qdf_opmode_str(opmode));
		break;
	}

	qdf_status = wlan_twt_set_command_in_progress(psoc,
					&event->params.peer_macaddr,
					event->params.dialog_id, WLAN_TWT_NONE);
	if (QDF_IS_STATUS_ERROR(qdf_status))
		return qdf_status;

cleanup:
	wlan_objmgr_pdev_release_ref(pdev, WLAN_TWT_ID);
	return qdf_status;
}

static bool
wlan_is_twt_teardown_failed(enum HOST_TWT_DEL_STATUS teardown_status)
{
	switch (teardown_status) {
	case HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST:
	case HOST_TWT_DEL_STATUS_INVALID_PARAM:
	case HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY:
	case HOST_TWT_DEL_STATUS_NO_RESOURCE:
	case HOST_TWT_DEL_STATUS_NO_ACK:
	case HOST_TWT_DEL_STATUS_UNKNOWN_ERROR:
		return true;
	default:
		return false;
	}

	return false;
}

static void
wlan_twt_handle_sta_del_dialog_event(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	bool is_evt_allowed, usr_cfg_ps_enable;
	enum wlan_twt_commands active_cmd = WLAN_TWT_NONE;

	is_evt_allowed = wlan_twt_is_command_in_progress(
					psoc, &event->peer_macaddr,
					event->dialog_id,
					WLAN_TWT_TERMINATE, &active_cmd);
	if (!is_evt_allowed &&
	    event->dialog_id != TWT_ALL_SESSIONS_DIALOG_ID &&
	    event->status != HOST_TWT_DEL_STATUS_ROAMING &&
	    event->status != HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN &&
	    event->status != HOST_TWT_DEL_STATUS_CONCURRENCY) {
		twt_err("Drop TWT Del dialog event for dialog_id:%d status:%d active_cmd:%d",
			event->dialog_id, event->status, active_cmd);

		return;
	}

	usr_cfg_ps_enable = mlme_get_user_ps(psoc, event->vdev_id);
	if (!usr_cfg_ps_enable &&
	    event->status == HOST_TWT_DEL_STATUS_OK)
		event->status = HOST_TWT_DEL_STATUS_PS_DISABLE_TEARDOWN;

	mlme_twt_osif_teardown_complete_ind(psoc, event);

	if (event->status == HOST_TWT_DEL_STATUS_ROAMING ||
	    event->status == HOST_TWT_DEL_STATUS_CONCURRENCY)
		wlan_twt_set_wait_for_notify(psoc, event->vdev_id, true);

	wlan_twt_set_command_in_progress(psoc, &event->peer_macaddr,
					 event->dialog_id, WLAN_TWT_NONE);

	if (wlan_is_twt_teardown_failed(event->status))
		return;

	wlan_twt_set_setup_done(psoc, &event->peer_macaddr,
				event->dialog_id, false);
	wlan_twt_set_session_state(psoc, &event->peer_macaddr, event->dialog_id,
				   WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED);
	wlan_twt_init_context(psoc, &event->peer_macaddr, event->dialog_id);
}

QDF_STATUS
wlan_twt_teardown_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	enum QDF_OPMODE opmode;
	uint32_t pdev_id, vdev_id;
	struct wlan_objmgr_pdev *pdev;

	vdev_id = event->vdev_id;
	pdev_id = wlan_get_pdev_id_from_vdev_id(psoc, vdev_id, WLAN_TWT_ID);
	if (pdev_id == WLAN_INVALID_PDEV_ID) {
		twt_err("Invalid pdev id");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_TWT_ID);
	if (!pdev) {
		twt_err("Invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	opmode = wlan_get_opmode_from_vdev_id(pdev, vdev_id);

	switch (opmode) {
	case QDF_SAP_MODE:
		mlme_twt_osif_teardown_complete_ind(psoc, event);

		/*
		 * If this is an unsolicited TWT del event initiated from the
		 * peer, then no need to clear the active command in progress
		 */
		if (event->status != HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN) {
			/* Reset the active TWT command to none */
			wlan_twt_sap_set_command_in_progress(psoc,
				event->vdev_id, &event->peer_macaddr,
				event->dialog_id, WLAN_TWT_NONE);
			wlan_twt_sap_init_context(psoc, event->vdev_id,
				&event->peer_macaddr, event->dialog_id);
		}
		break;
	case QDF_STA_MODE:
		wlan_twt_handle_sta_del_dialog_event(psoc, event);
		break;
	default:
		twt_debug("TWT Teardown is not supported on %s",
				  qdf_opmode_str(opmode));
		break;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_TWT_ID);
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
	wlan_twt_set_wait_for_notify(psoc, event->vdev_id, false);
	mlme_twt_osif_notify_complete_ind(psoc, event);

	return QDF_STATUS_SUCCESS;
}
