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

/**
 * wlan_twt_is_max_sessions_reached() - Check if the maximum number of
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
bool
wlan_twt_is_max_sessions_reached(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id);

/**
 * wlan_twt_set_command_in_progress() - Set TWT command is in progress
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 * @cmd: TWT command
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_set_command_in_progress(struct wlan_objmgr_psoc *psoc,
				 struct qdf_mac_addr *peer_mac,
				 uint8_t dialog_id,
				 enum wlan_twt_commands cmd);

/**
 * wlan_twt_setup_req() - twt setup request
 * @psoc: Pointer to psoc object
 * @req: TWT setup request
 * @context: context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_setup_req(struct wlan_objmgr_psoc *psoc,
		   struct twt_add_dialog_param *req,
		   void *context);

/**
 * wlan_twt_teardown_req() - twt teardown request
 * @psoc: Pointer to psoc object
 * @req: TWT setup request
 * @context: context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
		      struct twt_del_dialog_param *req,
		      void *context);

/**
 * wlan_twt_pause_req() - Process TWT pause req
 * @psoc: psoc
 * @req: pause dialog cmd param
 * @context: context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_pause_req(struct wlan_objmgr_psoc *psoc,
		   struct twt_pause_dialog_cmd_param *req,
		   void *context);

/**
 * wlan_twt_resume_req() - Process TWT resume req
 * @psoc: psoc
 * @req: resume dialog cmd param
 * @context: context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_resume_req(struct wlan_objmgr_psoc *psoc,
		    struct twt_resume_dialog_cmd_param *req,
		    void *context);

/**
 * wlan_twt_nudge_req() - Process TWT nudge req
 * @psoc: psoc
 * @req: nudge dialog cmd param
 * @context: context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
		   struct twt_nudge_dialog_cmd_param *req,
		   void *context);

/**
 * wlan_twt_ac_pdev_param_send() - pdev TWT param send
 * @psoc: Pointer to psoc object
 * @twt_ac: TWT access category
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_twt_ac_pdev_param_send(struct wlan_objmgr_psoc *psoc,
				       enum twt_traffic_ac twt_ac);

/**
 * wlan_twt_is_setup_in_progress() - Get if TWT setup command is in progress
 * for given dialog id
 * @psoc: Pointer to global psoc object
 * @peer_mac: Global peer mac address
 * @dialog_id: Dialog ID
 *
 * Return: True if Setup is in progress
 */
bool
wlan_twt_is_setup_in_progress(struct wlan_objmgr_psoc *psoc,
			      struct qdf_mac_addr *peer_mac,
			      uint8_t dialog_id);

/**
 * wlan_twt_setup_complete_event_handler() - twt setup complete event handler
 * @psoc: Pointer to global psoc object
 * @event: add dialog event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_setup_complete_event_handler(struct wlan_objmgr_psoc *psoc,
				   struct twt_add_dialog_complete_event *event);

/**
 * wlan_twt_ack_event_handler() - ack complete event handler
 * @psoc: Pointer to global psoc object
 * @event: ack complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_ack_event_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_ack_complete_event_param *event);

/**
 * wlan_twt_teardown_complete_event_handler() - teardown complete event handler
 * @psoc: Pointer to global psoc object
 * @event: del complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_teardown_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			     struct twt_del_dialog_complete_event_param *event);

/**
 * wlan_twt_pause_complete_event_handler() - pause complete event handler
 * @psoc: Pointer to global psoc object
 * @event: pause complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_pause_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			  struct twt_pause_dialog_complete_event_param *event);

/**
 * wlan_twt_resume_complete_event_handler() - resume complete event handler
 * @psoc: Pointer to global psoc object
 * @event: resume complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_resume_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			  struct twt_resume_dialog_complete_event_param *event);

/**
 * wlan_twt_nudge_complete_event_handler() - nudge complete event handler
 * @psoc: Pointer to global psoc object
 * @event: nudge complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_nudge_complete_event_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_nudge_dialog_complete_event_param *event);

/**
 * wlan_twt_notify_event_handler() - notify event handler
 * @psoc: Pointer to global psoc object
 * @event: notify complete event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_notify_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_notify_event_param *event);

/**
 * wlan_twt_init_context() - Initialize TWT context structure
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_init_context(struct wlan_objmgr_psoc *psoc,
		      struct qdf_mac_addr *peer_mac,
		      uint8_t dialog_id);

/**
 * wlan_twt_update_beacon_template() - update beacon template
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
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_twt_update_beacon_template(void);

/**
 * wlan_twt_is_setup_done() - Check if TWT setup exists for a given dialog id
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: true if TWT setup exists, false otherwise
 */
bool wlan_twt_is_setup_done(struct wlan_objmgr_psoc *psoc,
			    struct qdf_mac_addr *peer_mac, uint8_t dialog_id);

/**
 * wlan_twt_get_session_state() - Get TWT session state
 * @psoc: Pointer to psoc object
 * @peer_mac: Pointer to peer mac address
 * @dialog_id: Dialog id
 *
 * Return: enum wlan_twt_session_state
 */
enum wlan_twt_session_state
wlan_twt_get_session_state(struct wlan_objmgr_psoc *psoc,
			   struct qdf_mac_addr *peer_mac, uint8_t dialog_id);

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
bool wlan_twt_is_command_in_progress(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *peer_mac,
				     uint8_t dialog_id,
				     enum wlan_twt_commands cmd,
				     enum wlan_twt_commands *pactive_cmd);

/**
 * wlan_twt_set_work_params() - Set TWT work params
 * @vdev: vdev pointer
 * @params: TWT add dialog params
 * @twt_next_action: Set next action to do before work scheduled
 *
 * Return: None
 */
void wlan_twt_set_work_params(
			struct wlan_objmgr_vdev *vdev,
			struct twt_add_dialog_complete_event_param *params,
			uint32_t twt_next_action);

/**
 * wlan_twt_get_work_params() - Get TWT work params
 * @vdev: vdev pointer
 * @params: pointer to TWT work params
 * @next_action: Get next action to do after work scheduled
 *
 * Return: None
 */
void wlan_twt_get_work_params(struct wlan_objmgr_vdev *vdev,
			      struct twt_work_params *params,
			      uint32_t *next_action);
