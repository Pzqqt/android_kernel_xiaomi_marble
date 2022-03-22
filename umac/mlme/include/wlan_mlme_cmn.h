/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Define Common MLME structure and APIs
 */
#ifndef _WLAN_MLME_CMN_H_
#define _WLAN_MLME_CMN_H_

#include <include/wlan_psoc_mlme.h>
#include <include/wlan_pdev_mlme.h>
#include <include/wlan_vdev_mlme.h>
#include "wlan_cm_public_struct.h"
#include "wlan_twt_public_structs.h"

/**
 * mlme_cm_ops: connection manager osif callbacks
 * @mlme_cm_connect_complete_cb: Connect done callback
 * @vdev: vdev pointer
 * @rsp: connect response
 *
 * @mlme_cm_failed_candidate_cb: Callback to indicate failed candidate
 * @vdev: vdev pointer
 * @rsp: connect response
 *
 * @mlme_cm_update_id_and_src_cb: Callback to update connect id and
 *                                source of the connect request
 * @vdev: vdev pointer
 * @Source: Source of the connect req
 * @cm_id: connection manager id
 *
 * @mlme_cm_disconnect_complete_cb: Disconnect done callback
 * @vdev: vdev pointer
 * @rsp: Disconnect response
 *
 * @mlme_cm_disconnect_start_cb: Disconnect start callback
 * @vdev: vdev pointer
 *
 * @mlme_cm_roam_sync_cb: Roam sync callback
 * @vdev: vdev pointer
 *
 * @mlme_cm_pmksa_candidate_notify_cb: Roam pmksa candidate notify callback
 * @vdev: vdev pointer
 * @bssid: bssid
 * @index: index
 * @preauth: preauth flag
 *
 * @mlme_cm_roam_start_cb: Roam start callback
 * @vdev: vdev pointer
 *
 * @mlme_cm_roam_abort_cb: Roam abort callback
 * @vdev: vdev pointer
 *
 * @mlme_cm_roam_cmpl_cb: Roam sync complete cb
 * @vdev: vdev pointer
 *
 * @mlme_cm_ft_preauth_cmpl_cb: Roam ft preauth complete cb
 * @vdev: vdev pointer
 * @rsp: preauth response pointer
 *
 * @mlme_cm_cckm_preauth_cmpl_cb: Roam cckm preauth complete cb
 * @vdev: vdev pointer
 * @rsp: preauth response pointer
 */
struct mlme_cm_ops {
	QDF_STATUS (*mlme_cm_connect_complete_cb)(
					struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp);
	QDF_STATUS (*mlme_cm_failed_candidate_cb)(
					struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp);
	QDF_STATUS (*mlme_cm_update_id_and_src_cb)(
					struct wlan_objmgr_vdev *vdev,
					enum wlan_cm_source source,
					wlan_cm_id cm_id);
	QDF_STATUS (*mlme_cm_disconnect_complete_cb)(
					struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_discon_rsp *rsp);
	QDF_STATUS (*mlme_cm_disconnect_start_cb)(
					struct wlan_objmgr_vdev *vdev);
#ifdef CONN_MGR_ADV_FEATURE
	QDF_STATUS (*mlme_cm_roam_sync_cb)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_pmksa_candidate_notify_cb)(
						struct wlan_objmgr_vdev *vdev,
						struct qdf_mac_addr *bssid,
						int index, bool preauth);
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	QDF_STATUS (*mlme_cm_roam_start_cb)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_roam_abort_cb)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_roam_cmpl_cb)(struct wlan_objmgr_vdev *vdev);
#endif
#ifdef WLAN_FEATURE_PREAUTH_ENABLE
	QDF_STATUS (*mlme_cm_ft_preauth_cmpl_cb)(
					struct wlan_objmgr_vdev *vdev,
					struct wlan_preauth_rsp *rsp);
#ifdef FEATURE_WLAN_ESE
	QDF_STATUS (*mlme_cm_cckm_preauth_cmpl_cb)(
					struct wlan_objmgr_vdev *vdev,
					struct wlan_preauth_rsp *rsp);
#endif
#endif
};

/**
 * struct mlme_vdev_mgr_ops - MLME VDEV mgr osif callbacks
 * @mlme_vdev_mgr_set_mac_addr_response: Callback to indicate set MAC address
 *                                       response to osif
 */
struct mlme_vdev_mgr_ops {
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	QDF_STATUS (*mlme_vdev_mgr_set_mac_addr_response)(uint8_t vdev_id,
							  uint8_t resp_status);
#endif
};

/**
 * struct mlme_twt_ops: twt component osif callbacks
 * @mlme_twt_enable_complete_cb: TWT enable complete callback
 * @psoc: psoc pointer
 * @event: response
 * @context: context
 *
 * @mlme_twt_disable_complete_cb: TWT disable complete callback
 * @psoc: psoc pointer
 * @event: response
 * @context: context
 *
 * @mlme_twt_ack_complete_cb: TWT ack complete callback
 * @psoc: psoc pointer
 * @event: response
 * @context: context
 *
 * @mlme_twt_setup_complete_cb: TWT setup complete callback
 * @psoc: psoc pointer
 * @event: response
 * @renego_fail: flag to indicate if renegotiation failure case
 *
 * @mlme_twt_teardown_complete_cb: TWT teardown complete callback
 * @psoc: psoc pointer
 * @event: response
 *
 * @mlme_twt_pause_complete_cb: TWT pause complete callback
 * @psoc: psoc pointer
 * @event: response
 *
 * @mlme_twt_resume_complete_cb: TWT resume complete callback
 * @psoc: psoc pointer
 * @event: response
 *
 * @mlme_twt_nudge_complete_cb: TWT nudge complete callback
 * @psoc: psoc pointer
 * @event: response
 *
 * @mlme_twt_notify_complete_cb: TWT notify complete callback
 * @psoc: psoc pointer
 * @event: response
 */
struct mlme_twt_ops {
	QDF_STATUS (*mlme_twt_enable_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_enable_complete_event_param *event,
			void *context);

	QDF_STATUS (*mlme_twt_disable_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_disable_complete_event_param *event,
			void *context);

	QDF_STATUS (*mlme_twt_ack_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_ack_complete_event_param *event,
			void *context);

	QDF_STATUS (*mlme_twt_setup_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_complete_event *event,
			bool renego_fail);

	QDF_STATUS (*mlme_twt_teardown_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_del_dialog_complete_event_param *event);

	QDF_STATUS (*mlme_twt_pause_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_pause_dialog_complete_event_param *event);

	QDF_STATUS (*mlme_twt_resume_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_resume_dialog_complete_event_param *event);

	QDF_STATUS (*mlme_twt_nudge_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_nudge_dialog_complete_event_param *event);

	QDF_STATUS (*mlme_twt_notify_complete_cb)(
			struct wlan_objmgr_psoc *psoc,
			struct twt_notify_event_param *event);
};

/**
 * struct vdev_mlme_ext_ops - VDEV MLME legacy callbacks structure
 * @mlme_psoc_ext_hdl_create:               callback to invoke creation of
 *                                          legacy psoc object
 * @mlme_psoc_ext_hdl_destroy:              callback to invoke destroy of legacy
 *                                          psoc object
 * @mlme_pdev_ext_hdl_create:               callback to invoke creation of
 *                                          legacy pdev object
 * @mlme_pdev_ext_hdl_destroy:              callback to invoke destroy of legacy
 *                                          pdev object
 * @mlme_vdev_ext_hdl_create:               callback to invoke creation of
 *                                          legacy vdev object
 * @mlme_vdev_ext_hdl_post_create:          callback to invoke post creation
 *                                          actions of legacy vdev object
 * @mlme_vdev_ext_hdl_destroy:              callback to invoke destroy of legacy
 *                                          vdev object
 * @mlme_vdev_start_fw_send:                callback to invoke vdev start
 *                                          command
 * @mlme_vdev_stop_fw_send:                 callback to invoke vdev stop command
 * @mlme_vdev_down_fw_send:                 callback to invoke vdev down command
 * @mlme_multivdev_restart_fw_send:         callback to invoke multivdev restart
 *                                          command
 * @mlme_vdev_enqueue_exp_cmd:              callback to enqueue exception
 *                                          command
 *                                          required by serialization
 * @mlme_multi_vdev_restart_resp:           callback to process multivdev
 *                                          restart response
 * @mlme_cm_ext_hdl_create_cb:              callback to create ext cm context
 * @mlme_cm_ext_hdl_destroy_cb:             callback to destroy ext cm context
 * @mlme_cm_ext_connect_start_ind_cb:       callback to indicate connect start
 * @mlme_cm_ext_bss_select_ind_cb:          callback to indicate candidate
 *                                          select for connect
 * @mlme_cm_ext_bss_peer_create_req_cb:     callback to bss peer create request
 * @mlme_cm_ext_connect_req_cb:             callback for connect request to
 *                                          VDEV/PEER SM
 * @mlme_cm_ext_connect_complete_ind_cb:    callback to indicate connect
 *                                          complete
 * @mlme_cm_ext_disconnect_start_ind_cb :   callback to indicate disconnect
 *                                          start
 * @mlme_cm_ext_disconnect_req_cb:          callback to disconnect req to
 *                                          VDEV/PEER SM
 * @mlme_cm_ext_bss_peer_delete_req_cb:     callback to bss peer delete request
 * @mlme_cm_ext_disconnect_complete_ind_cb: callback to indicate disconnect
 *                                          complete
 * @mlme_cm_ext_vdev_down_req_cb:           callback to send vdev down to FW
 * @mlme_cm_ext_roam_start_ind_cb:          callback to indicate roam start
 * @mlme_cm_ext_rso_stop_cb:                callback to send rso stop to FW
 * @mlme_cm_ext_reassoc_req_cb:             callback for reassoc request to
 *                                          VDEV/PEER SM
 * @mlme_vdev_send_set_mac_addr:            callback to send set MAC address
 *                                          request to FW
 */
struct mlme_ext_ops {
	QDF_STATUS (*mlme_psoc_ext_hdl_create)(
				struct psoc_mlme_obj *psoc_mlme);
	QDF_STATUS (*mlme_psoc_ext_hdl_destroy)(
				struct psoc_mlme_obj *pdev_mlme);
	QDF_STATUS (*mlme_pdev_ext_hdl_create)(
				struct pdev_mlme_obj *pdev_mlme);
	QDF_STATUS (*mlme_pdev_ext_hdl_destroy)(
				struct pdev_mlme_obj *pdev_mlme);
	QDF_STATUS (*mlme_vdev_ext_hdl_create)(
				struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_ext_hdl_post_create)(
				struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_ext_hdl_destroy)(
				struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_start_fw_send)(
				struct wlan_objmgr_vdev *vdev, uint8_t restart);
	QDF_STATUS (*mlme_vdev_stop_fw_send)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_vdev_down_fw_send)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_multivdev_restart_fw_send)(
				struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_vdev_enqueue_exp_cmd)(
				struct vdev_mlme_obj *vdev_mlme,
				uint8_t cmd_type);
	QDF_STATUS (*mlme_vdev_ext_delete_rsp)(
					   struct wlan_objmgr_psoc *psoc,
					   struct vdev_delete_response *rsp);
	QDF_STATUS (*mlme_multi_vdev_restart_resp)(
				struct wlan_objmgr_psoc *psoc,
				struct multi_vdev_restart_resp *resp);
	QDF_STATUS (*mlme_cm_ext_hdl_create_cb)(struct wlan_objmgr_vdev *vdev,
						cm_ext_t **ext_cm_ptr);
	QDF_STATUS (*mlme_cm_ext_hdl_destroy_cb)(struct wlan_objmgr_vdev *vdev,
						 cm_ext_t *ext_cm_ptr);
	QDF_STATUS (*mlme_cm_ext_connect_start_ind_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req);
	QDF_STATUS (*mlme_cm_ext_bss_select_ind_cb)(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *req);
	QDF_STATUS (*mlme_cm_ext_bss_peer_create_req_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct qdf_mac_addr *peer_mac,
				struct qdf_mac_addr *mld_mac,
				bool is_assoc_link);
	QDF_STATUS (*mlme_cm_ext_connect_req_cb)(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *req);
	QDF_STATUS (*mlme_cm_ext_connect_complete_ind_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_resp *rsp);
	QDF_STATUS (*mlme_cm_ext_disconnect_start_ind_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_disconnect_req *req);
	QDF_STATUS (*mlme_cm_ext_disconnect_req_cb)
			(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_discon_req *req);
	QDF_STATUS (*mlme_cm_ext_bss_peer_delete_req_cb)(
			struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_ext_disconnect_complete_ind_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_discon_rsp *rsp);
	QDF_STATUS (*mlme_cm_ext_vdev_down_req_cb)(
				struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_ext_roam_start_ind_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_roam_req *req);
	QDF_STATUS (*mlme_cm_ext_rso_stop_cb)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*mlme_cm_ext_reassoc_req_cb)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_vdev_reassoc_req *req);
#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
	QDF_STATUS (*mlme_vdev_send_set_mac_addr)(
						struct qdf_mac_addr mac_addr,
						struct qdf_mac_addr mld_addr,
						struct wlan_objmgr_vdev *vdev);
#endif
};

/**
 * mlme_psoc_ops_ext_hdl_create() - Alloc PSOC mlme ext handle
 * @psoc_mlme:  PSOC MLME comp object
 *
 * API to allocate PSOC MLME ext handle
 *
 * Return: SUCCESS on successful allocation
 *         Else FAILURE
 */
QDF_STATUS mlme_psoc_ops_ext_hdl_create(struct psoc_mlme_obj *psoc_mlme);

/**
 * mlme_psoc_ops_ext_hdl_destroy() - Destroy PSOC mlme ext handle
 * @psoc_mlme:  PSOC MLME comp object
 *
 * API to free psoc MLME ext handle
 *
 * Return: SUCCESS on successful free
 *         Else FAILURE
 */
QDF_STATUS mlme_psoc_ops_ext_hdl_destroy(struct psoc_mlme_obj *psoc_mlme);

/**
 * mlme_pdev_ops_ext_hdl_create - Alloc PDEV mlme ext handle
 * @pdev_mlme_obj:  PDEV MLME comp object
 *
 * API to allocate PDEV MLME ext handle
 *
 * Return: SUCCESS on successful allocation
 *         Else FAILURE
 */
QDF_STATUS mlme_pdev_ops_ext_hdl_create(struct pdev_mlme_obj *pdev_mlme);

/**
 * mlme_pdev_ops_ext_hdl_destroy - Destroy PDEV mlme ext handle
 * @pdev_mlme_obj:  PDEV MLME comp object
 *
 * API to free pdev MLME ext handle
 *
 * Return: SUCCESS on successful free
 *         Else FAILURE
 */
QDF_STATUS mlme_pdev_ops_ext_hdl_destroy(struct pdev_mlme_obj *pdev_mlme);

/**
 * mlme_vdev_ops_ext_hdl_create - Alloc VDEV mlme ext handle
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API to allocate VDEV MLME ext handle
 *
 * Return: SUCCESS on successful allocation
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_ext_hdl_create(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_vdev_ops_ext_hdl_post_create - Perform post VDEV mlme ext handle alloc
 *                                     operations
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API to perform post vdev MLME ext handle allocation operations
 *
 * Return: SUCCESS on initialization successful
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_ext_hdl_post_create(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_vdev_ops_ext_hdl_destroy - Destroy VDEV mlme ext handle
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API to free vdev MLME ext handle
 *
 * Return: SUCCESS on successful free
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_ext_hdl_destroy(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_vdev_enqueue_exp_ser_cmd - Enqueue exception serialization cmd
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @cmd_type: Serialization command type
 *
 * API to enqueue the exception serialization command, used by
 * mlme-serialization wrapper layer
 *
 * Return: SUCCESS on successful enqueuing the command
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_enqueue_exp_ser_cmd(struct vdev_mlme_obj *vdev_mlme,
					 uint8_t cmd_type);

/**
 * mlme_vdev_ops_start_fw_send - Send WMI START/RESTART commmand to FW
 * @vdev:  VDEV object
 *
 * API to send WMI start/restart command to FW
 *
 * Return: SUCCESS on successful sending the command
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_start_fw_send(struct wlan_objmgr_vdev *vdev,
				       uint8_t restart);

/**
 * mlme_vdev_ops_multivdev_restart_fw_cmd_send - Send WMI Multivdev restart
 *                                              commmand to FW
 * @pdev:  PDEV object
 *
 * API to send WMI multivdev restart command to FW
 *
 * Return: SUCCESS on successful sending the command
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_multivdev_restart_fw_cmd_send(
						struct wlan_objmgr_pdev *pdev);

/**
 * mlme_vdev_ops_stop_fw_send - Send WMI STOP commmand to FW
 * @vdev:  VDEV object
 *
 * API to send WMI stop command to FW
 *
 * Return: SUCCESS on successful sending the command
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_stop_fw_send(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_vdev_ops_down_fw_send - Send WMI Down commmand to FW
 * @vdev:  VDEV object
 *
 * API to send WMI down command to FW
 *
 * Return: SUCCESS on successful sending the command
 *         Else FAILURE
 */
QDF_STATUS mlme_vdev_ops_down_fw_send(struct wlan_objmgr_vdev *vdev);

/*
 * mlme_vdev_ops_ext_hdl_multivdev_restart_resp() - Handler multivdev restart
 * response event
 * @psoc: PSOC object manager handle
 * @resp: Restart response event
 *
 * Return: Success on successful handling of the response event,
 *         Else failure
 */
QDF_STATUS mlme_vdev_ops_ext_hdl_multivdev_restart_resp(
		struct wlan_objmgr_psoc *psoc,
		struct multi_vdev_restart_resp *resp);

/**
 * mlme_set_ops_register_cb - Sets ops registration callback
 * @ops_cb:  Function pointer
 *
 * API to set ops registration call back
 *
 * Return: void
 */
typedef struct mlme_ext_ops *(*mlme_get_global_ops_cb)(void);
void mlme_set_ops_register_cb(mlme_get_global_ops_cb ops_cb);

/**
 * wlan_cmn_mlme_init - Initializes MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_cmn_mlme_init(void);

/**
 * wlan_cmn_mlme_deinit - Uninitializes MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_cmn_mlme_deinit(void);

/**
 * mlme_vdev_ops_ext_hdl_delete_rsp - Vdev Delete response ext handler
 * @psoc: PSOC object
 * @rsp: Vdev delete response received from the firmware
 *
 * API to invoke the legacy delete response handler for legacy cleanup
 *
 * Return: SUCCESS on successful deletion
 *         FAILURE, if deletion fails
 */
QDF_STATUS mlme_vdev_ops_ext_hdl_delete_rsp(struct wlan_objmgr_psoc *psoc,
					    struct vdev_delete_response *rsp);

/**
 * mlme_cm_ext_hdl_create() - Connection manager callback to create ext
 * context
 * @vdev: VDEV object
 * @ext_cm_ptr: pointer to connection manager ext pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_ext_hdl_create(struct wlan_objmgr_vdev *vdev,
				  cm_ext_t **ext_cm_ptr);

/**
 * mlme_cm_ext_hdl_destroy() - Connection manager callback to destroy ext
 * context
 * @vdev: VDEV object
 * @ext_cm_ptr: connection manager ext pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_ext_hdl_destroy(struct wlan_objmgr_vdev *vdev,
				   cm_ext_t *ext_cm_ptr);

/**
 * mlme_cm_connect_start_ind() - Connection manager ext Connect start indication
 * @vdev: VDEV object
 * @req: Connection manager connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_connect_start_ind(struct wlan_objmgr_vdev *vdev,
				     struct wlan_cm_connect_req *req);

/**
 * mlme_cm_bss_select_ind() - Connection manager ext Connect candidate
 * select indication, to do operations for the candidate
 * @vdev: VDEV object
 * @req: Vdev connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_bss_select_ind(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_vdev_connect_req *req);

/**
 * mlme_cm_bss_peer_create_req() - Connection manager ext bss peer create
 * request
 * @vdev: VDEV object
 * @peer_mac: Peer mac address
 * @mld_mac: mld mac address
 * @is_assoc_link: assoc happens on this link or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
				       struct qdf_mac_addr *peer_mac,
				       struct qdf_mac_addr *mld_mac,
				       bool is_assoc_link);

/**
 * mlme_cm_connect_req() - Connection manager ext connect request to start vdev
 * and peer assoc state machine
 * @vdev: VDEV object
 * @req: Vdev connect request
 *
 * Context: The req is on stack, so the API need to make a copy, if it want to
 * use the req after return.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_connect_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_vdev_connect_req *req);

/**
 * mlme_cm_connect_complete_ind() - Connection manager ext connect complete
 * indication
 * @vdev: VDEV object
 * @rsp: Connection manager connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_connect_complete_ind(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_resp *rsp);

/**
 * mlme_cm_roam_start_ind() - Connection manager ext Connect start indication
 * @vdev: VDEV object
 * @req: Connection manager roam request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_roam_start_ind(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_roam_req *req);

/**
 * mlme_cm_rso_stop_req() - Connection manager ext RSO stop request
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_rso_stop_req(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_reassoc_req() - Connection manager ext reassoc request
 * @vdev: VDEV object
 * @req: Vdev reassoc request
 *
 * Context: The req is on stack, so the API need to make a copy, if it want to
 * use the req after return.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_reassoc_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_vdev_reassoc_req *req);

/**
 * mlme_cm_disconnect_start_ind() - Connection manager ext disconnect start
 * indication
 * @vdev: VDEV object
 * @req: Connection manager disconnect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_disconnect_start_ind(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_disconnect_req *req);

/**
 * mlme_cm_disconnect_req() - Connection manager ext disconnect
 * req to vdev and peer sm
 * @vdev: VDEV object
 * @req: vdev disconnect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_disconnect_req(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_vdev_discon_req *req);

/**
 * mlme_cm_bss_peer_delete_req() - Connection manager ext bss peer delete
 * request
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_cm_bss_peer_delete_req(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_disconnect_complete_ind() - Connection manager ext disconnect
 * complete indication
 * @vdev: VDEV object
 * @rsp: Connection manager disconnect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_disconnect_complete_ind(struct wlan_objmgr_vdev *vdev,
					   struct wlan_cm_discon_rsp *rsp);

/**
 * mlme_cm_vdev_down() - Connection manager ext req to send vdev down to FW
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_vdev_down_req(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_osif_connect_complete() - Connect complete resp to osif
 * @vdev: vdev pointer
 * @rsp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_connect_complete(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_resp *rsp);

/**
 * mlme_cm_osif_failed_candidate_ind() - Failed Candidate indication to osif
 * @vdev: vdev pointer
 * @rsp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_cm_osif_failed_candidate_ind(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_connect_resp *rsp);

/**
 * mlme_cm_osif_update_id_and_src() - Update connection id and source to osif
 * @vdev: vdev pointer
 * @source: source of request
 * @cm_id: connection manager id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_update_id_and_src(struct wlan_objmgr_vdev *vdev,
					  enum wlan_cm_source source,
					  wlan_cm_id cm_id);

/**
 * mlme_cm_osif_disconnect_complete() - Disconnect complete osif response
 * @vdev: vdev pointer
 * @cm_conn_rsp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_cm_osif_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_discon_rsp *rsp);

/**
 * mlme_cm_osif_disconnect_start_ind() - osif Disconnect start indication
 * @vdev: vdev pointer
 * @cm_conn_rsp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_disconnect_start_ind(struct wlan_objmgr_vdev *vdev);

#ifdef CONN_MGR_ADV_FEATURE
/**
 * mlme_cm_osif_roam_sync_ind() - osif Roam sync indication
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_roam_sync_ind(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_osif_pmksa_candidate_notify() - osif roam pmksa candidate notify
 * @vdev: vdev pointer
 * @bssid: bssid
 * @index: index
 * @preauth: preauth flag
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_pmksa_candidate_notify(struct wlan_objmgr_vdev *vdev,
					       struct qdf_mac_addr *bssid,
					       int index, bool preauth);
#else
static inline
QDF_STATUS mlme_cm_osif_roam_sync_ind(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * mlme_cm_osif_roam_start_ind() - osif Roam start indication
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_roam_start_ind(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_osif_roam_abort_ind() - osif Roam abort indication
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_roam_abort_ind(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cm_osif_roam_complete() - osif Roam sync complete callback
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cm_osif_roam_complete(struct wlan_objmgr_vdev *vdev);
#endif

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
/**
 * mlme_cm_osif_ft_preauth_complete() - osif roam ft preauth complete callback
 * @vdev: vdev pointer
 * @rsp: preauth response pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_cm_osif_ft_preauth_complete(struct wlan_objmgr_vdev *vdev,
				 struct wlan_preauth_rsp *rsp);
#ifdef FEATURE_WLAN_ESE
/**
 * mlme_cm_osif_cckm_preauth_complete() - osif cckm preauth complete callback
 * @vdev: vdev pointer
 * @rsp: preauth response pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_cm_osif_cckm_preauth_complete(struct wlan_objmgr_vdev *vdev,
				   struct wlan_preauth_rsp *rsp);
#else
static inline QDF_STATUS
mlme_cm_osif_cckm_preauth_complete(struct wlan_objmgr_vdev *vdev,
				   struct wlan_preauth_rsp *rsp)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_ESE */
#endif /* WLAN_FEATURE_PREAUTH_ENABLE */

/**
 * typedef osif_cm_get_global_ops_cb() - Callback to get connection manager
 * global ops
 */
typedef struct mlme_cm_ops *(*osif_cm_get_global_ops_cb)(void);

/**
 * typedef osif_twt_get_global_ops_cb() - Callback to get twt global ops
 */
typedef struct mlme_twt_ops *(*osif_twt_get_global_ops_cb)(void);

/**
 * mlme_set_osif_cm_cb() - Sets ops registration callback
 * @cm_osif_ops:  Function pointer
 *
 * API to set ops registration call back
 *
 * Return: void
 */
void mlme_set_osif_cm_cb(osif_cm_get_global_ops_cb cm_osif_ops);

/**
 * typedef osif_vdev_mgr_get_global_ops_cb() - Callback to get vdev manager
 * global ops
 */
typedef struct mlme_vdev_mgr_ops *(*osif_vdev_mgr_get_global_ops_cb)(void);

/**
 * mlme_set_osif_vdev_mgr_cb() - Sets ops registration callback
 * @mlme_vdev_mgr_osif_ops:  Function pointer
 *
 * API to set ops registration call back
 *
 * Return: void
 */
void mlme_set_osif_vdev_mgr_cb(
		osif_vdev_mgr_get_global_ops_cb mlme_vdev_mgr_osif_ops);

/**
 * mlme_set_osif_twt_cb() - Sets twt ops registration callback
 * @twt_osif_ops:  Function pointer
 *
 * API to set twt ops registration call back
 *
 * Return: void
 */
void mlme_set_osif_twt_cb(osif_twt_get_global_ops_cb twt_osif_ops);

/**
 * mlme_max_chan_switch_is_set() - Get if max chan switch IE is enabled
 * @vdev: Object manager vdev pointer
 *
 * Return: True if max chan switch is enabled else false
 */
bool mlme_max_chan_switch_is_set(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
/**
 * mlme_vdev_ops_send_set_mac_address() - Send set MAC address request to FW
 * @mac_addr: VDEV MAC address
 * @mld_addr: VDEV MLD address
 * @vdev: vdev pointer
 *
 * API to send set MAC address request command to FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_vdev_ops_send_set_mac_address(struct qdf_mac_addr mac_addr,
					      struct qdf_mac_addr mld_addr,
					      struct wlan_objmgr_vdev *vdev);

/**
 * mlme_vdev_mgr_notify_set_mac_addr_response() - Notify set MAC address
 *                                                response
 * @vdev_id: VDEV ID
 * @resp_status: FW response for the set MAC address operation
 *
 * API to notify set MAC address to osif
 *
 * Return: None
 */
void mlme_vdev_mgr_notify_set_mac_addr_response(uint8_t vdev_id,
						uint8_t resp_status);
#endif

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * mlme_twt_osif_enable_complete_ind() - enable complete resp to osif
 * @psoc: psoc pointer
 * @event: enable complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_enable_complete_ind(struct wlan_objmgr_psoc *psoc,
				  struct twt_enable_complete_event_param *event,
				  void *context);

/**
 * mlme_twt_osif_disable_complete_ind() - disable complete resp to osif
 * @psoc: psoc pointer
 * @event: disable complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_disable_complete_ind(struct wlan_objmgr_psoc *psoc,
				 struct twt_disable_complete_event_param *event,
				 void *context);

/**
 * mlme_twt_osif_ack_complete_ind() - ack complete resp to osif
 * @psoc: psoc pointer
 * @event: ack complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_ack_complete_ind(struct wlan_objmgr_psoc *psoc,
			       struct twt_ack_complete_event_param *event,
			       void *context);

/**
 * mlme_twt_osif_setup_complete_ind() - setup complete resp to osif
 * @psoc: psoc pointer
 * @event: setup complete response
 * @renego_fail: flag to indicate if renegotiation failure case
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_setup_complete_ind(struct wlan_objmgr_psoc *psoc,
				 struct twt_add_dialog_complete_event *event,
				 bool renego_fail);

/**
 * mlme_twt_osif_teardown_complete_ind() - teardown complete resp to osif
 * @psoc: psoc pointer
 * @event: teardown complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_teardown_complete_ind(struct wlan_objmgr_psoc *psoc,
			     struct twt_del_dialog_complete_event_param *event);

/**
 * mlme_twt_osif_pause_complete_ind() - pause complete resp to osif
 * @psoc: psoc pointer
 * @event: pause complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_pause_complete_ind(struct wlan_objmgr_psoc *psoc,
			   struct twt_pause_dialog_complete_event_param *event);

/**
 * mlme_twt_osif_resume_complete_ind() - resume complete resp to osif
 * @psoc: psoc pointer
 * @event: resume complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_resume_complete_ind(struct wlan_objmgr_psoc *psoc,
			  struct twt_resume_dialog_complete_event_param *event);

/**
 * mlme_twt_osif_nudge_complete_ind() - nudge complete resp to osif
 * @psoc: psoc pointer
 * @event: nudge complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_nudge_complete_ind(struct wlan_objmgr_psoc *psoc,
			   struct twt_nudge_dialog_complete_event_param *event);

/**
 * mlme_twt_osif_notify_complete_ind() - notify complete resp to osif
 * @psoc: psoc pointer
 * @event: notify complete response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_twt_osif_notify_complete_ind(struct wlan_objmgr_psoc *psoc,
				  struct twt_notify_event_param *event);

#else
static inline QDF_STATUS
mlme_twt_osif_enable_complete_ind(struct wlan_objmgr_psoc *psoc,
				  struct twt_enable_complete_event_param *event,
				  void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_disable_complete_ind(struct wlan_objmgr_psoc *psoc,
				 struct twt_disable_complete_event_param *event,
				 void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_ack_complete_ind(struct wlan_objmgr_psoc *psoc,
			       struct twt_ack_complete_event_param *event,
			       void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_setup_complete_ind(struct wlan_objmgr_psoc *psoc,
				 struct twt_add_dialog_complete_event *event,
				 bool renego_fail)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_teardown_complete_ind(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_pause_complete_ind(struct wlan_objmgr_psoc *psoc,
			    struct twt_pause_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_resume_complete_ind(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_nudge_complete_ind(struct wlan_objmgr_psoc *psoc,
			    struct twt_nudge_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
mlme_twt_osif_notify_complete_ind(struct wlan_objmgr_psoc *psoc,
				  struct twt_notify_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* WLAN_SUPPORT_TWT && WLAN_TWT_CONV_SUPPORTED */

#endif
