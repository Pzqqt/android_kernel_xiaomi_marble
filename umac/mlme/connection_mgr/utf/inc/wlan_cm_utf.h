/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/*
 * DOC: Implements CM UTF
 */

#ifndef WLAN_CM_UTF_H
#define WLAN_CM_UTF_H

#include <qdf_debugfs.h>
#include <qdf_mem.h>
#include <qdf_trace.h>
#include <qdf_module.h>
#include <qdf_event.h>
#include <qdf_defer.h>
#include <wlan_cm_public_struct.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_scan_tgt_api.h>
#include <wlan_cfg80211.h>

#define NUM_UTF_DEBUGFS_INFOS 2

/*
 * <ini>
 * wlan_cm_utf - WLAN CM UTF Configuration
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to config wlan cm utf
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_WLAN_CM_UTF CFG_INI_UINT( \
	"wlan_cm_utf", \
	0, \
	1, \
	0, \
	CFG_VALUE_OR_DEFAULT, \
	"WLAN CM UTF Configuration")

#define CFG_WLAN_CM_UTF_PARAM CFG(CFG_WLAN_CM_UTF)
/**
 * enum wlan_cm_utf_test - CM UTF Test ID
 * @CM_UTF_ID_CONNECT_SUCCESS: Connect Succes
 * @CM_UTF_ID_DISCONNECT_SUCCESS: Disconnect Success
 * @CM_UTF_ID_PEER_CREATE_FAILURE: Peer Create Failure
 * @CM_UTF_ID_PEER_CREATE_TIMEOUT: No Peer Create Response
 * @CM_UTF_ID_PEER_DELETE_TIMEOUT: No Peer Delete Response
 * @CM_UTF_ID_AUTH_FAILURE: Auth Req Failure
 * @CM_UTF_ID_AUTH_TIMEOUT: No Auth Response
 * @CM_UTF_ID_ASSOC_FAILURE: Assoc Req Failure
 * @CM_UTF_ID_ASSOC_TIMEOUT: No Assoc Response
 * @CM_UTF_ID_CONNECT_SCAN_FAILURE: SSID Not Found
 * @CM_UTF_ID_CONNECT_SER_TIMEOUT: Serialization Active Cmd Timeout for Connect
 * @CM_UTF_ID_DISCONNECT_SER_TIMEOUT: Ser Active Cmd Timeout for Disconnect
 * @CM_UTF_ID_CONNECT_SER_FAILED: Serialization Cmd Queue Failure for Connect
 * @CM_UTF_ID_DISCONNECT_SER_FAILED: Ser Cmd Queue Failure for Disconnect
 */
enum wlan_cm_utf_test {
	CM_UTF_ID_CONNECT_SUCCESS,
	CM_UTF_ID_DISCONNECT_SUCCESS,
	CM_UTF_ID_PEER_CREATE_FAILURE,
	CM_UTF_ID_PEER_CREATE_TIMEOUT,
	CM_UTF_ID_PEER_DELETE_TIMEOUT,
	CM_UTF_ID_AUTH_FAILURE,
	CM_UTF_ID_AUTH_TIMEOUT,
	CM_UTF_ID_ASSOC_FAILURE,
	CM_UTF_ID_ASSOC_TIMEOUT,
	CM_UTF_ID_CONNECT_SCAN_FAILURE,
	CM_UTF_ID_CONNECT_SER_TIMEOUT,
	CM_UTF_ID_DISCONNECT_SER_TIMEOUT,
	CM_UTF_ID_CONNECT_SER_FAILED,//Need ser utf
	CM_UTF_ID_DISCONNECT_SER_FAILED,//Need ser utf
	CM_UTF_ID_MAX,
};

/**
 * enum wlan_cm_utf_evt - CM UTF Resp event
 * @CM_UTF_BSS_PEER_CREATE_RESP: Peer Create Response
 * @CM_UTF_BSS_PEER_DELETE_RESP: Peer Delete Response
 * @CM_UTF_CONNECT_RESP: Connect Response
 * @CM_UTF_DISCONNECT_RESP: Disconnect Response
 * @CM_UTF_PEER_DELETE_IND: Peer Delete Indication
 */
enum wlan_cm_utf_evt {
	CM_UTF_BSS_PEER_CREATE_RESP,
	CM_UTF_BSS_PEER_DELETE_RESP,
	CM_UTF_CONNECT_RESP,
	CM_UTF_DISCONNECT_RESP,
	CM_UTF_PEER_DELETE_IND,
};

/**
 * struct wlan_cm_utf_node- CM UTF node to hold CM req info
 * @wlan_cm_utf_evt: CM UTF Resp event
 * @peer_mac: Peer Mac
 * @conn_req: Connect Request
 * @disconn_req: Disconnect Request
 */
struct wlan_cm_utf_node {
	enum wlan_cm_utf_evt evt_id;
	struct qdf_mac_addr *peer_mac;
	struct wlan_cm_vdev_connect_req conn_req;
	struct wlan_cm_vdev_discon_req disconn_req;
};

/**
 * struct wlan_cm_utf- CM UTF handle
 * @vdev: Vdev object
 * @debugfs_de: debugfs entry
 * @test_id: Test case Id
 * @req: cfg80211 connect request params
 * @cm_utf_timer: CM UTF timer
 * @cm_utf_test_timer: CM UTF timer for each test
 * @utf_node: CM UTF node to hold CM req info
 * @cm_utf_work: CM UTF work queue for processing events
 */
struct wlan_cm_utf {
	qdf_list_node_t cm_utf_node;
	struct wlan_objmgr_vdev *vdev;
	struct dentry *debugfs_de[NUM_UTF_DEBUGFS_INFOS];
	enum wlan_cm_utf_test test_id;
	struct wlan_cm_connect_req req;
	qdf_timer_t  cm_utf_timer;
	qdf_timer_t  cm_utf_test_timer;
	struct wlan_cm_utf_node utf_node;
	uint32_t last_cmd_id;
	enum wlan_cm_source last_cmd_source;
	qdf_work_t cm_utf_work;
};

/*
 * Debugfs read/write functions
 */
/**
 * wlan_cm_utf_cm_test_id_show() - debugfs function to display CM test case name
 * @m: seq_file handle
 * @v: not used, offset of read
 */
int wlan_cm_utf_cm_test_id_show(qdf_debugfs_file_t m, void *v);

int wlan_cm_utf_scan_db_update_show(qdf_debugfs_file_t m, void *v);

/**
 * wlan_cm_utf_cm_test_id_write() - debugfs write to start CM UTF test
 *
 * @file: file handler to access cm utf handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t wlan_cm_utf_cm_test_id_write(struct file *file,
				     const char __user *buf,
				     size_t count, loff_t *ppos);

/**
 * wlan_cm_utf_scan_db_update_write() - debugfs write to add manual scan entry
 *
 * @file: file handler to access cm utf handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t wlan_cm_utf_scan_db_update_write(struct file *file,
					 const char __user *buf,
					 size_t count, loff_t *ppos);

/**
 * wlan_cm_utf_attach: Connection manager UTF init API
 * @vdev: Vdev object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_attach(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_utf_detach: Connection manager UTF deinit API
 * @vdev: Vdev object
 *
 * Return: QDF_STATUS
 */
void wlan_cm_utf_detach(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_utf_bss_peer_create_req: Connection manager UTF bss peer
 * create request handler
 * @vdev: VDEV object
 * @peer_mac: Peer mac address
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
					   struct qdf_mac_addr *peer_mac);

/**
 * wlan_cm_utf_connect_req_active: Connection manager UTF handler when connect
 * request is activated
 * @vdev: VDEV object
 * @vdev_connect_req: Vdev connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_connect_req_active(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *vdev_connect_req);

/**
 * wlan_cm_utf_connect_req: Connection manager UTF connect request handler
 * @vdev: VDEV object
 * @vdev_connect_req: Vdev connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_connect_req(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *vdev_connect_req);

/**
 * wlan_cm_utf_disconnect_req: Connection manager UTF disconnect
 * request handler
 * @vdev: VDEV object
 * @vdev_connect_req: Vdev connect request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_disconnect_req(
		struct wlan_objmgr_vdev *vdev,
		struct wlan_cm_vdev_discon_req *vdev_disconnect_req);

/**
 * wlan_cm_utf_bss_peer_delete_req: Connection manager UTF bss peer
 * delete request handler
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_bss_peer_delete_req(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_utf_vdev_down: Connection manager UTF vdev down request handler
 * @vdev: VDEV object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_utf_vdev_down(struct wlan_objmgr_vdev *vdev);

/**
 * osif_cm_utf_register_cb() - API to register connection manager callbacks
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_cm_utf_register_cb(void);
#endif //WLAN_CM_UTF_H
