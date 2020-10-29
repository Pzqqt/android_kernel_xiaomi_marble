/*
 * Copyright (c) 2012-2015, 2020, The Linux Foundation. All rights reserved.
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

#ifdef FEATURE_CM_ENABLE
#include <wlan_cm_public_struct.h>
#include "scheduler_api.h"

/**
 * struct cm_vdev_join_req - connect req from legacy CM to vdev manager
 * @vdev_id: vdev id
 * @cm_id: Connect manager id
 * @force_rsne_override: force the arbitrary rsne received in connect req to be
 * used with out validation, used for the scenarios where the device is used
 * as a testbed device with special functionality and not recommended
 * for production.
 * @assoc_ie: assoc ie to be used in assoc req
 * @scan_ie: Default scan ie to be used in the uncast probe req
 * @entry: scan entry for the candidate
 */
struct cm_vdev_join_req {
	uint8_t vdev_id;
	wlan_cm_id cm_id;
	bool force_rsne_override;
	struct element_info assoc_ie;
	struct element_info scan_ie;
	struct scan_cache_entry *entry;
};

/**
 * struct cm_vdev_join_rsp - connect rsp from vdev mgr to connection mgr
 * @psoc: psoc object
 * @connect_rsp: Connect response to be sent to CM
 */
struct cm_vdev_join_rsp {
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cm_connect_resp connect_rsp;
};

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
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *peer_mac);

/**
 * cm_handle_connect_complete() - Connection manager ext connect complete
 * indication
 * @vdev: VDEV object
 * @rsp: Connection manager connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_connect_complete(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_resp *rsp);

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
 * cm_process_join_req() - Process vdev join req
 * @msg: scheduler message
 *
 * Process connect request in LIM and copy all join req params.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_process_join_req(struct scheduler_msg *msg);

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

#endif /* FEATURE_CM_ENABLE */
#endif /* __WLAN_CM_VDEV_API_H__ */
