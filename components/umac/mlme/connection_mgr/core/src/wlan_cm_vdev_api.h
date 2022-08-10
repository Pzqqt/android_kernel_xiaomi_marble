/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_vdev_api.h
 *
 * This header file maintain legacy connect, disconnect APIs of connection
 * manager to initiate vdev manager operations
 */

#ifndef __WLAN_CM_VDEV_API_H__
#define __WLAN_CM_VDEV_API_H__

#include <wlan_cm_public_struct.h>
#include "scheduler_api.h"
#include "connection_mgr/core/src/wlan_cm_main.h"
#include "connection_mgr/core/src/wlan_cm_main_api.h"
#include <wlan_cm_roam_api.h>
#ifdef WLAN_FEATURE_11BE_MLO
#include "wlan_mlo_mgr_public_structs.h"
#endif

#define HS20_OUI_TYPE   "\x50\x6f\x9a\x10"
#define HS20_OUI_TYPE_SIZE  4

/**
 * struct cm_vdev_join_req - connect req from legacy CM to vdev manager
 * @vdev_id: vdev id
 * @cm_id: Connect manager id
 * @force_24ghz_in_ht20: force 24ghz_in ht20
 * @force_rsne_override: force the arbitrary rsne received in connect req to be
 * used with out validation, used for the scenarios where the device is used
 * as a testbed device with special functionality and not recommended
 * for production.
 * @is_wps_connection: is wps connection
 * @is_osen_connection: is osen connectgion
 * @assoc_ie: assoc ie to be used in assoc req
 * @scan_ie: Default scan ie to be used in the uncast probe req
 * @entry: scan entry for the candidate
 * @partner_info: Partner link information for an ML connection
 * @assoc_link_id: Assoc link ID of an ML connection
 * @owe_trans_ssid: owe trans ssid to be used when scan entry ssid is wildcard
 */
struct cm_vdev_join_req {
	uint8_t vdev_id;
	wlan_cm_id cm_id;
	uint8_t force_24ghz_in_ht20:1,
		force_rsne_override:1,
		is_wps_connection:1,
		is_osen_connection:1;
	struct element_info assoc_ie;
	struct element_info scan_ie;
	struct scan_cache_entry *entry;
#ifdef WLAN_FEATURE_11BE_MLO
	struct mlo_partner_info partner_info;
	uint8_t assoc_link_id;
#endif
	struct wlan_ssid owe_trans_ssid;
};

/**
 * struct wlan_cm_discon_ind - disconnect ind from VDEV mgr and will be sent to
 * SME
 * @vdev_id: vdev id
 * @source: source of disconnection
 * @reason_code: reason of disconnection
 * @bssid: BSSID of AP
 */
struct wlan_cm_discon_ind {
	uint8_t vdev_id;
	enum wlan_cm_source source;
	enum wlan_reason_code reason_code;
	struct qdf_mac_addr bssid;
};

/**
 * struct cm_vdev_discon_ind - disconnect ind from vdev mgr to connection mgr
 * @psoc: psoc object
 * @disconnect_param: DisConnect indication to be sent to CM
 */
struct cm_vdev_discon_ind {
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_discon_ind disconnect_param;
};

/**
 * struct cm_vdev_disconnect_rsp - disconnect rsp from vdev mgr to CM
 * @vdev_id: vdev id
 * @psoc: psoc object
 */
struct cm_vdev_disconnect_rsp {
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
};

/*
 * struct cm_vdev_hw_mode_rsp - hw mode change from vdev mgr to CM
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cm_id: connection ID which gave the hw mode change request
 * @status: status of the HW mode change.
 */
struct cm_vdev_hw_mode_rsp {
	struct wlan_objmgr_pdev *pdev;
	uint8_t vdev_id;
	wlan_cm_id cm_id;
	QDF_STATUS status;
};

/**
 * struct cm_vdev_join_rsp - connect rsp from vdev mgr to connection mgr
 * @psoc: psoc object
 * @connect_rsp: Connect response to be sent to CM
 * @ric_resp_ie: ric ie data
 * @tspec_ie: tspec ie
 * @nss: used nss
 * @uapsd_mask: uapsd mask
 */
struct cm_vdev_join_rsp {
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_connect_resp connect_rsp;
	struct element_info ric_resp_ie;
#ifdef FEATURE_WLAN_ESE
	struct element_info tspec_ie;
#endif
	uint8_t nss;
	uint8_t uapsd_mask;
};

/**
 * struct cm_peer_create_req - bss peer create req
 * @vdev_id: vdev_id
 * @peer_mac: peer mac to create
 * @mld_mac: peer mld mac
 * @is_assoc_peer: is assoc peer or not
 */
struct cm_peer_create_req {
	uint8_t vdev_id;
	struct qdf_mac_addr peer_mac;
#ifdef WLAN_FEATURE_11BE_MLO
	struct qdf_mac_addr mld_mac;
	bool is_assoc_peer;
#endif
};

/**
 * struct cm_host_roam_start_ind - roam start ind for host roam from FW
 * @vdev_id: vdev id
 * @pdev: pdev object
 */
struct cm_host_roam_start_ind {
	uint8_t vdev_id;
	struct wlan_objmgr_pdev *pdev;
};

/**
 * struct cm_ext_obj - Connection manager legacy object
 * @rso_cfg: connect info to be used in RSO.
 */
struct cm_ext_obj {
	struct rso_config rso_cfg;
};

#ifdef WLAN_FEATURE_FILS_SK
/**
 * cm_update_hlp_info - API to save HLP IE
 * @psoc: Pointer to psoc
 * @gen_ie: IE buffer to store
 * @len: length of the IE buffer @gen_ie
 * @vdev_id: vdev id
 * @flush: Flush the older saved HLP if any
 *
 * Return: None
 */
void cm_update_hlp_info(struct wlan_objmgr_vdev *vdev,
			const uint8_t *gen_ie, uint16_t len,
			bool flush);
#else
static inline void cm_update_hlp_info(struct wlan_objmgr_vdev *vdev,
				      const uint8_t *gen_ie, uint16_t len,
				      bool flush)
{}
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
/**
 * cm_connect_info - send connect info to diag
 * @vdev: vdev ptr
 * @connect_success: if connect was success
 * @bssid: bssid
 * @ssid: ssid
 * @freq: freq
 *
 * Return: none
 */
void cm_connect_info(struct wlan_objmgr_vdev *vdev, bool connect_success,
		     struct qdf_mac_addr *bssid, struct wlan_ssid *ssid,
		     qdf_freq_t freq);

void cm_diag_get_auth_enc_type_vdev_id(struct wlan_objmgr_psoc *psoc,
				       uint8_t *auth_type,
				       uint8_t *ucast_cipher,
				       uint8_t *mcast_cipher,
				       uint8_t vdev_id);

#ifdef WLAN_UNIT_TEST
/**
 * cm_get_sta_cxn_info - fill sta context info in buffer
 * @buf: buffer to fill
 * @buf_sz: buf size
 *
 * Return: none
 */
void cm_get_sta_cxn_info(struct wlan_objmgr_vdev *vdev,
			 char *buf, uint32_t buf_sz);
#endif
#else
static inline void
cm_connect_info(struct wlan_objmgr_vdev *vdev, bool connect_success,
		struct qdf_mac_addr *bssid, struct wlan_ssid *ssid,
		qdf_freq_t freq)
{}
#endif

/**
 * cm_wait_for_key_time_out_handler() - Wait key time out handler API
 * @data: Pointer to wait key timer data
 *
 * Return: none
 */
void cm_wait_for_key_time_out_handler(void *data);

/**
 * cm_start_wait_for_key_timer() - Wait for key start API
 * @vdev: Pointer to vdev
 * @interval: timer trigger interval
 *
 * Return: QDF_STATUS
 */

QDF_STATUS cm_start_wait_for_key_timer(struct wlan_objmgr_vdev *vdev,
				       uint32_t interval);

/**
 * cm_stop_wait_for_key_timer() - Wait key stop API
 * @psoc: Pointer to psoc
 * @vdev_id: vdev id
 *
 * Return: none
 */
void cm_stop_wait_for_key_timer(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id);

void cm_update_wait_for_key_timer(struct wlan_objmgr_vdev *vdev,
				  uint8_t vdev_id, uint32_t interval);

void cm_update_prev_ap_ie(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			  uint32_t len, uint8_t *bcn_ptr);
bool cm_csr_is_ss_wait_for_key(uint8_t vdev_id);
void cm_csr_set_ss_wait_for_key(uint8_t vdev_id);
void cm_csr_set_ss_none(uint8_t vdev_id);
void cm_csr_set_joining(uint8_t vdev_id);
void cm_csr_set_joined(uint8_t vdev_id);
void cm_csr_set_idle(uint8_t vdev_id);

/**
 * cm_get_rssi_snr_by_bssid() - get rssi and snr by bssid
 * @pdev: Pointer to pdev
 * @bssid: bssid to get rssi and snr
 * @rssi: pointer to fill rssi
 * @snr: poiter to fill snr
 *
 * Return: none
 */
QDF_STATUS cm_get_rssi_snr_by_bssid(struct wlan_objmgr_pdev *pdev,
				    struct qdf_mac_addr *bssid,
				    int8_t *rssi, int8_t *snr);

/**
 * cm_csr_send_set_ie()  - CM wrapper to send the set IE request
 * @vdev: Object manager VDEV
 *
 * Return: None
 */
void cm_csr_send_set_ie(struct wlan_objmgr_vdev *vdev);

static inline QDF_STATUS
cm_ext_hdl_create(struct wlan_objmgr_vdev *vdev, cm_ext_t **ext_cm_ptr)
{
	struct cm_ext_obj *cm_obj;

	*ext_cm_ptr = qdf_mem_malloc(sizeof(struct cm_ext_obj));
	if (!*ext_cm_ptr)
		return QDF_STATUS_E_NOMEM;

	cm_obj = *ext_cm_ptr;
	wlan_cm_rso_config_init(vdev, &cm_obj->rso_cfg);

	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
cm_ext_hdl_destroy(struct wlan_objmgr_vdev *vdev, cm_ext_t *ext_cm_ptr)
{
	wlan_cm_rso_config_deinit(vdev, &ext_cm_ptr->rso_cfg);
	qdf_mem_free(ext_cm_ptr);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_connect_start_ind() - Connection manager ext connect start indication
 * vdev and peer assoc state machine
 * @vdev: VDEV object
 * @req: connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_connect_start_ind(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req);

/**
 * cm_csr_handle_join_req() - Connection manager cb to csr to fill csr
 * session and update join req from legacy structures
 * @vdev: VDEV object
 * @req: Vdev connect request
 * @join_req: join req to be sent to LIM
 * @reassoc: if reassoc
 *
 * This API is to update legacy struct and should be removed once
 * CSR is cleaned up fully. No new params should be added to CSR, use
 * vdev/pdev/psoc instead.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_csr_handle_join_req(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_vdev_connect_req *req,
				  struct cm_vdev_join_req *join_req,
				  bool reassoc);

/**
 * cm_handle_connect_req() - Connection manager ext connect request to start
 * vdev and peer assoc state machine
 * @vdev: VDEV object
 * @req: Vdev connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_connect_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_connect_req *req);

/**
 * cm_send_bss_peer_create_req() - Connection manager ext bss peer create
 * request
 * @vdev: VDEV object
 * @peer_mac: Peer mac address
 * @mld_mac: peer mld mac address
 * @is_assoc_peer: is assoc peer or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *peer_mac,
			    struct qdf_mac_addr *mld_mac,
			    bool is_assoc_peer);

/**
 * cm_csr_connect_rsp() - Connection manager ext connect resp indication
 * @vdev: VDEV object
 * @rsp: Connection vdev response
 *
 * This API is to update legacy struct and should be removed once
 * CSR is cleaned up fully. No new params should be added to CSR, use
 * vdev/pdev/psoc instead.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_csr_connect_rsp(struct wlan_objmgr_vdev *vdev,
			      struct cm_vdev_join_rsp *rsp);

/**
 * cm_connect_complete_ind() - Connection manager ext connect complete
 * indication
 * @vdev: VDEV object
 * @rsp: Connection manager connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_connect_complete_ind(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_resp *rsp);

/**
 * cm_csr_connect_done_ind() - Connection manager call to csr to update
 * legacy structures on connect complete
 * @vdev: VDEV object
 * @rsp: Connection manager connect response
 *
 * This API is to update legacy struct and should be removed once
 * CSR is cleaned up fully. No new params should be added to CSR, use
 * vdev/pdev/psoc instead.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_csr_connect_done_ind(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_resp *rsp);

/**
 * cm_is_vdevid_connected() - check if vdev_id is in conneted state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdevid_connected(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);

/**
 * cm_is_vdevid_active() - check if vdev_id is in conneted/roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdevid_active(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);

/**
 * cm_disconnect_start_ind() - Connection manager ext disconnect start
 * indication
 * vdev and peer assoc state machine
 * @vdev: VDEV object
 * @req: disconnect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_disconnect_start_ind(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req);

/**
 * cm_handle_disconnect_req() - Connection manager ext disconnect
 * req to vdev and peer sm
 * @vdev: VDEV object
 * @req: vdev disconnect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_disconnect_req(struct wlan_objmgr_vdev *vdev,
			 struct wlan_cm_vdev_discon_req *req);

/**
 * cm_csr_handle_diconnect_req() - Connection manager cb to csr to update legacy
 * structures on disconnect
 * @vdev: VDEV object
 * @req: vdev disconnect request
 *
 * This API is to update legacy struct and should be removed once
 * CSR is cleaned up fully. No new params should be added to CSR, use
 * vdev/pdev/psoc instead.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_csr_handle_diconnect_req(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_vdev_discon_req *req);

/**
 * cm_send_bss_peer_delete_req() - Connection manager ext bss peer delete
 * request
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_bss_peer_delete_req(struct wlan_objmgr_vdev *vdev);

/**
 * cm_disconnect_complete_ind() - Connection manager ext disconnect
 * complete indication
 * @vdev: VDEV object
 * @rsp: Connection manager disconnect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_disconnect_complete_ind(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_discon_rsp *rsp);

/**
 * cm_csr_diconnect_done_ind() - Connection manager call to csr to update
 * legacy structures on disconnect complete
 * @vdev: VDEV object
 * @rsp: Connection manager disconnect response
 *
 * This API is to update legacy struct and should be removed once
 * CSR is cleaned up fully. No new params should be added to CSR, use
 * vdev/pdev/psoc instead.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_csr_diconnect_done_ind(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_discon_rsp *rsp);

/**
 * cm_send_vdev_down_req() - Connection manager ext req to send vdev down
 * to FW
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_send_vdev_down_req(struct wlan_objmgr_vdev *vdev);

/**
 * cm_free_join_req() - Free cm vdev connect req params
 * @join_req: join request
 *
 * Return: void
 */
void cm_free_join_req(struct cm_vdev_join_req *join_req);

/**
 * cm_flush_join_req() - Process join req flush
 * @msg: scheduler message
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_flush_join_req(struct scheduler_msg *msg);

/**
 * cm_process_join_req() - Process vdev join req
 * @msg: scheduler message
 *
 * Process connect request in LIM and copy all join req params.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_join_req(struct scheduler_msg *msg);

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * cm_process_preauth_req() - Process preauth request
 * @msg: scheduler message
 *
 * Process preauth request in LIM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_preauth_req(struct scheduler_msg *msg);

/**
 * cm_process_reassoc_req() - Process vdev reassoc req
 * @msg: scheduler message
 *
 * Process reassoc request in LIM and copy all reassoc req params.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_reassoc_req(struct scheduler_msg *msg);

/**
 * cm_handle_reassoc_req() - Connection manager ext reassoc request to start
 * vdev and peer assoc state machine
 * @vdev: VDEV object
 * @req: Vdev reassoc request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_reassoc_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_reassoc_req *req);

/**
 * cm_handle_roam_start() - roam start indication
 * @vdev: VDEV object
 * @req: Connection manager roam request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_roam_start(struct wlan_objmgr_vdev *vdev,
		     struct wlan_cm_roam_req *req);

/**
 * cm_csr_preauth_done() - Process preauth done from csr part
 * @vdev: vdev object pointer
 *
 * Return: void
 */
void cm_csr_preauth_done(struct wlan_objmgr_vdev *vdev);
#else
static inline QDF_STATUS cm_process_preauth_req(struct scheduler_msg *msg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS cm_process_reassoc_req(struct scheduler_msg *msg)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
cm_handle_reassoc_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_reassoc_req *req)
{
	return QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS
cm_handle_roam_start(struct wlan_objmgr_vdev *vdev,
		     struct wlan_cm_roam_req *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cm_process_peer_create() - Process bss peer create req
 * @msg: scheduler message
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_peer_create(struct scheduler_msg *msg);

/**
 * cm_process_disconnect_req() - Process vdev disconnect request
 * @msg: scheduler message
 *
 * Process disconnect request in LIM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_disconnect_req(struct scheduler_msg *msg);

/**
 * cm_disconnect() - disconnect start request
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @source: disconnect source
 * @reason_code: disconnect reason
 * @bssid: bssid of AP to disconnect, can be null if not known
 *
 * Context: can be called from any context, should not hold sme global lock
 * while calling as can lead to deadlock (disconnect access sme lock holding CM
 * lock and thus calling cm api (which will hold CM lock) while holding sme lock
 * can lead to deadlock)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_disconnect(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			 enum wlan_cm_source source,
			 enum wlan_reason_code reason_code,
			 struct qdf_mac_addr *bssid);

/**
 * cm_send_sb_disconnect_req() - Process vdev discon ind from sb and send to CM
 * @msg: scheduler message
 *
 * Process disconnect indication and send it to CM SM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_send_sb_disconnect_req(struct scheduler_msg *msg);

/**
 * cm_handle_disconnect_resp() - Process vdev discon rsp and send to CM
 * @msg: scheduler message
 *
 * Process disconnect rsp and send it to CM SM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_handle_disconnect_resp(struct scheduler_msg *msg);

/**
 * wlan_cm_send_connect_rsp() - Process vdev join rsp and send to CM
 * @msg: scheduler message
 *
 * Process connect response and send it to CM SM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_send_connect_rsp(struct scheduler_msg *msg);

/**
 * wlan_cm_free_connect_rsp() - Function to free all params in join rsp
 * @rsp: CM join response
 *
 * Function to free up all the memory in join rsp.
 *
 * Return: void
 */
void wlan_cm_free_connect_rsp(struct cm_vdev_join_rsp *rsp);

/**
 * wlan_cm_handle_hw_mode_change_resp() Process hw_mode_change_resp
 * @pdev: pdev pointer
 * @vdev_id: vdev_id
 * @cm_id: connection manager id
 * @status: status
 *
 * This API is to break the context and avoid calling CM API to take CM lock
 * while holding SME lock.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_handle_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   wlan_cm_id cm_id, QDF_STATUS status);
/**
 * wlan_cm_rso_stop_continue_disconnect() - Continue disconnect after RSO stop
 * @psoc: psoc object
 * @vdev_id: vdev id
 * @is_ho_fail: Carries true if HO_FAIL happened
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_rso_stop_continue_disconnect(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id, bool is_ho_fail);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_send_rso_stop() - Send RSP stop req to firmware
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_rso_stop(struct wlan_objmgr_vdev *vdev);
#else
static inline QDF_STATUS
cm_send_rso_stop(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
#endif /* __WLAN_CM_VDEV_API_H__ */
