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

#ifdef FEATURE_CM_UTF_ENABLE
#include <wlan_cm_utf.h>
#include <wlan_cm_api.h>
#include <qdf_str.h>
#include <wlan_cm_ucfg_api.h>
#include <include/wlan_mlme_cmn.h>

#define CM_UTF_LIST_SIZE 1

qdf_list_t wlan_cm_utf_list;

static const char *cm_utf_test_names[] = {
	"CONNECT_SUCCESS",
	"DISCONNECT_SUCCESS",
	"PEER_CREATE_FAILURE",
	"PEER_CREATE_TIMEOUT",
	"PEER_DELETE_TIMEOUT",
	"AUTH_FAILURE",
	"AUTH_TIMEOUT",
	"ASSOC_FAILURE",
	"ASSOC_TIMEOUT",
	"CONNECT_SCAN_FAILURE",
	"CONNECT_SER_TIMEOUT",
	"DISCONNECT_SER_TIMEOUT",
	"CONNECT_SER_FAILED",
	"DISCONNECT_SER_FAIL",
};

/* Structure to maintain debug information */
struct cm_utf_debugfs_info {
	const char *name;
	const struct file_operations *ops;
};

#define DEBUG_FOO(func_base) { .name = #func_base,                      \
	.ops = &wlan_cm_utf_##func_base##_ops }

/*
 * wlan_cm_utf_##func_base##_open() - Open debugfs entry for respective command
 * and event buffer.
 * @inode: node for debug dir entry
 * @file: file handler
 *
 * Return: open status
 */
#define GENERATE_DEBUG_STRUCTS(func_base)                                     \
	static int wlan_cm_utf_##func_base##_open(struct inode *inode,        \
			struct file *file)                                    \
	{                                                                     \
		return single_open(file, wlan_cm_utf_##func_base##_show,      \
				   inode->i_private);                         \
	}                                                                     \
									      \
	static const struct file_operations wlan_cm_utf_##func_base##_ops = { \
		.open           = wlan_cm_utf_##func_base##_open,             \
		.read           = seq_read,                                   \
		.llseek         = seq_lseek,                                  \
		.write          = wlan_cm_utf_##func_base##_write,            \
		.release        = single_release,                             \
	};

GENERATE_DEBUG_STRUCTS(scan_db_update);
GENERATE_DEBUG_STRUCTS(cm_test_id);

struct cm_utf_debugfs_info cm_utf_debugfs_infos[NUM_UTF_DEBUGFS_INFOS] = {
	DEBUG_FOO(scan_db_update),
	DEBUG_FOO(cm_test_id),
};

/**
 * wlan_cm_debugfs_create() - Create debugfs entry for cm db
 * @cm_utf: CM UTF object
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_cm_utf_debugfs_create(struct wlan_cm_utf *cm_utf)
{
	struct wlan_objmgr_vdev *vdev = cm_utf->vdev;
	struct wlan_objmgr_pdev *pdev;
	uint8_t pdev_id;
	uint8_t vdev_id;
	char name[32];

	if (cm_utf->debugfs_de[0] || cm_utf->debugfs_de[1]) {
		mlme_info("CM UTF debugfs file already exist");
		return QDF_STATUS_SUCCESS;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("Pdev is Null");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	vdev_id = wlan_vdev_get_id(vdev);

	snprintf(name, sizeof(name), "CM_UTF_PDEV%u_VDEV%u_SCAN",
		 pdev_id, vdev_id);
	cm_utf->debugfs_de[0] = debugfs_create_file(
				name, 0644, qdf_debugfs_get_root(),
				cm_utf, cm_utf_debugfs_infos[0].ops);

	if (!cm_utf->debugfs_de[0]) {
		mlme_err("Failed to create debugfs entry");
		return QDF_STATUS_E_FAILURE;
	}

	snprintf(name, sizeof(name), "CM_UTF_PDEV%u_VDEV%u_UTF",
		 pdev_id, vdev_id);
	cm_utf->debugfs_de[1] = debugfs_create_file(
				name, 0644, qdf_debugfs_get_root(),
				cm_utf, cm_utf_debugfs_infos[1].ops);

	if (!cm_utf->debugfs_de[1]) {
		mlme_err("Failed to create debugfs entry");
		debugfs_remove(cm_utf->debugfs_de[0]);
		cm_utf->debugfs_de[0] = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_cm_utf_debugfs_remove: Remove connection manager UTF debugfs entries
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_debugfs_remove(struct wlan_cm_utf *cm_utf)
{
	if (cm_utf->debugfs_de[0]) {
		qdf_debugfs_remove_file(cm_utf->debugfs_de[0]);
		cm_utf->debugfs_de[0] = NULL;
	}

	if (cm_utf->debugfs_de[1]) {
		qdf_debugfs_remove_file(cm_utf->debugfs_de[1]);
		cm_utf->debugfs_de[1] = NULL;
	}
}

/**
 * wlan_cm_utf_bss_peer_create_rsp: Connection manager UTF bss peer
 * create response
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_bss_peer_create_rsp(struct wlan_cm_utf *cm_utf)
{
	struct qdf_mac_addr *peer_mac;
	QDF_STATUS status;

	peer_mac = cm_utf->utf_node.peer_mac;
	switch (cm_utf->test_id) {
	case CM_UTF_ID_PEER_CREATE_FAILURE:
		status = QDF_STATUS_E_FAILURE;
		break;
	default:
		status = QDF_STATUS_SUCCESS;
		break;
	}
	wlan_cm_bss_peer_create_rsp(cm_utf->vdev, status, peer_mac);
}

/**
 * wlan_cm_utf_bss_peer_delete_rsp: Connection manager UTF bss peer
 * delete response
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_bss_peer_delete_rsp(struct wlan_cm_utf *cm_utf)
{
	wlan_cm_bss_peer_delete_rsp(cm_utf->vdev, QDF_STATUS_SUCCESS);
}

/**
 * wlan_cm_utf_connect_rsp: Connection manager UTF connect response
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_connect_rsp(struct wlan_cm_utf *cm_utf)
{
	struct wlan_cm_connect_rsp *cm_conn_rsp;
	struct wlan_cm_vdev_connect_req conn_req = cm_utf->utf_node.conn_req;

	cm_conn_rsp = qdf_mem_malloc(sizeof(struct wlan_cm_connect_rsp));
	if (!cm_conn_rsp) {
		mlme_err("failed ta allocate memory");
		return;
	}
	qdf_mem_zero(cm_conn_rsp, sizeof(struct wlan_cm_connect_rsp));

	cm_conn_rsp->vdev_id = conn_req.vdev_id;
	cm_conn_rsp->cm_id = conn_req.cm_id;
	cm_conn_rsp->aid = 1;
	cm_conn_rsp->connect_status = QDF_STATUS_E_FAILURE;
	cm_conn_rsp->reason_code = 0;
	cm_conn_rsp->freq = conn_req.bss->entry->channel.chan_freq;
	cm_conn_rsp->connect_ies.bcn_probe_rsp.ptr =
				conn_req.bss->entry->raw_frame.ptr;
	cm_conn_rsp->connect_ies.bcn_probe_rsp.len =
				conn_req.bss->entry->raw_frame.len;
	cm_conn_rsp->bssid = conn_req.bss->entry->bssid;
	cm_conn_rsp->ssid = conn_req.bss->entry->ssid;

	switch (cm_utf->test_id) {
	case CM_UTF_ID_AUTH_FAILURE:
		cm_conn_rsp->reason = CM_AUTH_FAILED;
		break;
	case CM_UTF_ID_AUTH_TIMEOUT:
		cm_conn_rsp->reason = CM_AUTH_TIMEOUT;
		break;
	case CM_UTF_ID_ASSOC_FAILURE:
		cm_conn_rsp->reason = CM_ASSOC_FAILED;
		break;
	case CM_UTF_ID_ASSOC_TIMEOUT:
		cm_conn_rsp->reason = CM_ASSOC_TIMEOUT;
		break;
	default:
		cm_conn_rsp->connect_status = QDF_STATUS_SUCCESS;
		break;
	}
	wlan_cm_connect_rsp(cm_utf->vdev, cm_conn_rsp);
	qdf_mem_free(cm_conn_rsp);
}

/**
 * wlan_cm_utf_disconnect_rsp: Connection manager UTF disconnect response
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_disconnect_rsp(struct wlan_cm_utf *cm_utf)
{
	struct wlan_cm_discon_rsp *cm_discon_rsp;

	cm_discon_rsp = qdf_mem_malloc(sizeof(struct wlan_cm_discon_rsp));
	if (!cm_discon_rsp) {
		mlme_err("failed ta allocate memory");
		return;
	}
	cm_discon_rsp->req = cm_utf->utf_node.disconn_req;

	wlan_cm_disconnect_rsp(cm_utf->vdev, cm_discon_rsp);
	qdf_mem_free(cm_discon_rsp);
}

/**
 * wlan_cm_utf_peer_del_ind: Connection manager UTF peer delete indication
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static void wlan_cm_utf_peer_del_ind(struct wlan_cm_utf *cm_utf)
{
	struct qdf_mac_addr peer_mac;

	peer_mac.bytes[0] = 0x1;
	peer_mac.bytes[1] = 0x2;
	peer_mac.bytes[2] = 0x3;
	peer_mac.bytes[3] = 0x4;
	peer_mac.bytes[4] = 0x5;
	peer_mac.bytes[5] = 0x6;

	wlan_cm_bss_peer_delete_ind(cm_utf->vdev, &peer_mac);
}

static void wlan_cm_utf_stop_test(void *arg)
{
	struct wlan_cm_utf *cm_utf = (struct wlan_cm_utf *)arg;

	mlme_err("No Response from CM");
	cm_utf->test_id = CM_UTF_ID_MAX;
}

static void wlan_cm_utf_deliver_event(void *arg)
{
	struct wlan_cm_utf *cm_utf = (struct wlan_cm_utf *)arg;

	qdf_sched_work(NULL, &cm_utf->cm_utf_work);
}

/**
 * wlan_cm_utf_default_connect_param: Update default connect req params
 * for connection manager UTF
 * @cm_utf: Connection Manager UTF object
 *
 * Return: None
 */
static QDF_STATUS
wlan_cm_utf_default_connect_param(struct wlan_cm_utf *cm_utf)
{
	cm_utf->req.vdev_id = wlan_vdev_get_id(cm_utf->vdev);
	cm_utf->req.source = CM_OSIF_CONNECT;
	cm_utf->req.bssid.bytes[0] = 0x1;
	cm_utf->req.bssid.bytes[1] = 0x2;
	cm_utf->req.bssid.bytes[2] = 0x3;
	cm_utf->req.bssid.bytes[3] = 0x4;
	cm_utf->req.bssid.bytes[4] = 0x5;
	cm_utf->req.bssid.bytes[5] = 0x6;
	cm_utf->req.ssid.length = 6;
	qdf_str_lcopy(cm_utf->req.ssid.ssid, "CM_STA", cm_utf->req.ssid.length);
	cm_utf->req.chan_freq = 5200;
	cm_utf->req.crypto.wep_keys.key_len = 0;
	return QDF_STATUS_SUCCESS;
}

static void wlan_cm_utf_work_cb(void *arg)
{
	struct wlan_cm_utf *cm_utf = (struct wlan_cm_utf *)arg;
	enum wlan_cm_utf_evt event_id = cm_utf->utf_node.evt_id;

	switch (event_id) {
	case CM_UTF_BSS_PEER_CREATE_RESP:
		wlan_cm_utf_bss_peer_create_rsp(cm_utf);
		break;
	case CM_UTF_BSS_PEER_DELETE_RESP:
		wlan_cm_utf_bss_peer_delete_rsp(cm_utf);
		break;
	case CM_UTF_CONNECT_RESP:
		wlan_cm_utf_connect_rsp(cm_utf);
		break;
	case CM_UTF_DISCONNECT_RESP:
		wlan_cm_utf_disconnect_rsp(cm_utf);
		break;
	case CM_UTF_PEER_DELETE_IND:
		wlan_cm_utf_peer_del_ind(cm_utf);
		break;
	default:
		break;
	}
}

QDF_STATUS wlan_cm_utf_attach(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_cm_utf *cm_utf;
	QDF_STATUS status;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE) {
		mlme_err("Vdev is not a STA vdev");
		return QDF_STATUS_E_FAILURE;
	}

	if (qdf_list_size(&wlan_cm_utf_list) == CM_UTF_LIST_SIZE) {
		mlme_err("List is at max size");
		return QDF_STATUS_E_FAILURE;
	}

	if (!qdf_list_size(&wlan_cm_utf_list))
		qdf_list_create(&wlan_cm_utf_list, CM_UTF_LIST_SIZE);

	cm_utf = (struct wlan_cm_utf *)
			qdf_mem_malloc(sizeof(struct wlan_cm_utf));

	if (!cm_utf) {
		mlme_err("Failed to allocate CM utf context");
		qdf_list_destroy(&wlan_cm_utf_list);
		return QDF_STATUS_E_NOMEM;
	}

	cm_utf->vdev = vdev;
	cm_utf->test_id = CM_UTF_ID_MAX;
	wlan_cm_utf_default_connect_param(cm_utf);
	status = wlan_cm_utf_debugfs_create(cm_utf);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(cm_utf);
		qdf_list_destroy(&wlan_cm_utf_list);
		return status;
	}

	qdf_timer_init(NULL, &cm_utf->cm_utf_timer,
		       wlan_cm_utf_deliver_event, (void *)cm_utf,
		       QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_init(NULL, &cm_utf->cm_utf_test_timer,
		       wlan_cm_utf_stop_test, (void *)cm_utf,
		       QDF_TIMER_TYPE_WAKE_APPS);

	qdf_create_work(NULL, &cm_utf->cm_utf_work,
			wlan_cm_utf_work_cb, cm_utf);

	qdf_list_insert_back(&wlan_cm_utf_list, &cm_utf->cm_utf_node);
	mlme_err("CM UTF attach Success");
	return QDF_STATUS_SUCCESS;
}

static struct wlan_cm_utf *wlan_cm_get_utf(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_cm_utf *cm_utf;
	qdf_list_node_t *node;
	qdf_list_node_t *next_node;

	if (!qdf_list_size(&wlan_cm_utf_list)) {
		mlme_err("UTF list is empty");
		return NULL;
	}

	if (qdf_list_peek_front(&wlan_cm_utf_list, &next_node)
					!= QDF_STATUS_SUCCESS) {
		mlme_err("UTF list is empty");
		return NULL;
	}

	do {
		node = next_node;
		cm_utf = qdf_container_of(node, struct wlan_cm_utf,
					  cm_utf_node);
		if (cm_utf->vdev == vdev)
			return cm_utf;
	} while (qdf_list_peek_next(&wlan_cm_utf_list, node, &next_node)
			== QDF_STATUS_SUCCESS);

	return NULL;
}

void wlan_cm_utf_detach(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_cm_utf *cm_utf;
	QDF_STATUS status;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("UTF not initialized for the vdev %d",
			 wlan_vdev_get_id(vdev));
		return;
	}

	status = qdf_list_remove_node(&wlan_cm_utf_list,
				      &cm_utf->cm_utf_node);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_timer_free(&cm_utf->cm_utf_timer);
		qdf_timer_free(&cm_utf->cm_utf_test_timer);

		if (cm_utf->req.assoc_ie.ptr) {
			qdf_mem_free(cm_utf->req.assoc_ie.ptr);
			cm_utf->req.assoc_ie.ptr = NULL;
		}

		if (cm_utf->req.crypto.wep_keys.key) {
			qdf_mem_free(cm_utf->req.crypto.wep_keys.key);
			cm_utf->req.crypto.wep_keys.key = NULL;
		}

		qdf_disable_work(&cm_utf->cm_utf_work);
		wlan_cm_utf_debugfs_remove(cm_utf);
		qdf_mem_free(cm_utf);
		mlme_err("CM UTF Deinit Success");
		return;
	}

	mlme_err("Failed to remove UTF node");
}

QDF_STATUS wlan_cm_utf_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
					   struct qdf_mac_addr *peer_mac)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (cm_utf->test_id) {
	case CM_UTF_ID_PEER_CREATE_TIMEOUT:
		break;
	default:
		cm_utf->utf_node.peer_mac = peer_mac;
		cm_utf->utf_node.evt_id = CM_UTF_BSS_PEER_CREATE_RESP;
		qdf_timer_start(&cm_utf->cm_utf_timer, 100);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_utf_connect_req_active(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *vdev_connect_req)
{
	//Resp API to be added
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_utf_connect_req(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_vdev_connect_req *vdev_connect_req)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (cm_utf->test_id) {
	case CM_UTF_ID_CONNECT_SER_TIMEOUT:
		break;
	default:
		qdf_mem_copy(&cm_utf->utf_node.conn_req, vdev_connect_req,
			     sizeof(struct wlan_cm_vdev_connect_req));
		cm_utf->utf_node.evt_id = CM_UTF_CONNECT_RESP;
		qdf_timer_start(&cm_utf->cm_utf_timer, 100);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_utf_disconnect_req(
		struct wlan_objmgr_vdev *vdev,
		struct wlan_cm_vdev_discon_req *vdev_disconnect_req)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (cm_utf->test_id) {
	case CM_UTF_ID_DISCONNECT_SER_TIMEOUT:
		break;
	default:
		qdf_mem_copy(&cm_utf->utf_node.disconn_req, vdev_disconnect_req,
			     sizeof(struct wlan_cm_vdev_discon_req));
		cm_utf->utf_node.evt_id = CM_UTF_PEER_DELETE_IND;
		qdf_timer_start(&cm_utf->cm_utf_timer, 100);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_utf_bss_peer_delete_req(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (cm_utf->test_id) {
	case CM_UTF_ID_PEER_DELETE_TIMEOUT:
		break;
	default:
		cm_utf->utf_node.evt_id = CM_UTF_BSS_PEER_DELETE_RESP;
		qdf_timer_start(&cm_utf->cm_utf_timer, 100);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_utf_vdev_down(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	switch (cm_utf->test_id) {
	default:
		cm_utf->utf_node.evt_id = CM_UTF_DISCONNECT_RESP;
		qdf_timer_start(&cm_utf->cm_utf_timer, 100);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS wlan_cm_utf_validate_test(struct wlan_cm_utf *cm_utf,
				     struct wlan_cm_connect_rsp *cm_conn_rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	switch (cm_utf->test_id) {
	case CM_UTF_ID_PEER_CREATE_FAILURE:
	case CM_UTF_ID_PEER_CREATE_TIMEOUT:
		if (cm_conn_rsp->reason == CM_PEER_CREATE_FAILED) {
			// check (CM state to be valid as per test in all cases)
			status = QDF_STATUS_SUCCESS;
		}
		break;
	case CM_UTF_ID_CONNECT_SER_TIMEOUT:
	case CM_UTF_ID_CONNECT_SER_FAILED:
		if (cm_conn_rsp->reason == CM_SER_FAILURE)
			status = QDF_STATUS_SUCCESS;
		break;
	case CM_UTF_ID_CONNECT_SCAN_FAILURE:
		if (cm_conn_rsp->reason == CM_NO_CANDIDATE_FOUND)
			status = QDF_STATUS_SUCCESS;
		break;
	case CM_UTF_ID_AUTH_FAILURE:
		if (cm_conn_rsp->reason == CM_AUTH_FAILED)
			status = QDF_STATUS_SUCCESS;
		break;
	case CM_UTF_ID_AUTH_TIMEOUT:
		if (cm_conn_rsp->reason == CM_AUTH_TIMEOUT)
			status = QDF_STATUS_SUCCESS;
		break;
	case CM_UTF_ID_ASSOC_FAILURE:
		if (cm_conn_rsp->reason == CM_ASSOC_FAILED)
			status = QDF_STATUS_SUCCESS;
		break;
	case CM_UTF_ID_ASSOC_TIMEOUT:
		if (cm_conn_rsp->reason == CM_ASSOC_TIMEOUT)
			status = QDF_STATUS_SUCCESS;
		break;
	default:
		if (cm_conn_rsp->connect_status == QDF_STATUS_SUCCESS)
			status = QDF_STATUS_SUCCESS;
		break;
	}

	return status;
}

/**
 * wlan_cm_utf_osif_connect_cb: Connection manager UTF
 * connect callback handler
 * @vdev: VDEV object
 * @cm_conn_rsp: Connect Response
 *
 * Return: None
 */
static QDF_STATUS
wlan_cm_utf_osif_connect_cb(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_rsp *cm_conn_rsp)
{
	struct wlan_cm_utf *cm_utf;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	if (cm_conn_rsp->cm_id != cm_utf->last_cmd_id ||
	    cm_utf->last_cmd_source != CM_OSIF_CONNECT) {
		mlme_err("Incorrect cmd id/source");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_cm_utf_validate_test(cm_utf, cm_conn_rsp);
	if (status == QDF_STATUS_SUCCESS)
		mlme_err("Test case Success for Test: %s",
			 cm_utf_test_names[cm_utf->test_id]);
	else
		mlme_err("Test case Failed for Test: %s",
			 cm_utf_test_names[cm_utf->test_id]);

	qdf_timer_stop(&cm_utf->cm_utf_test_timer);
	cm_utf->test_id = CM_UTF_ID_MAX;

	return status;
}

/**
 * wlan_cm_utf_osif_disconnect_cb: Connection manager UTF
 * disconnect callback handler
 * @vdev: VDEV object
 * @cm_disconn_rsp: Disconnect Response
 *
 * Return: None
 */
static QDF_STATUS wlan_cm_utf_osif_disconnect_cb(
		struct wlan_objmgr_vdev *vdev,
		struct wlan_cm_discon_rsp *cm_disconn_rsp)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	if (cm_disconn_rsp->req.cm_id != cm_utf->last_cmd_id ||
	    cm_disconn_rsp->req.req.source != cm_utf->last_cmd_source) {
		mlme_err("Incorrect cmd id/source");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_err("Test case Success for Test: %s",
		 cm_utf_test_names[cm_utf->test_id]);
	qdf_timer_stop(&cm_utf->cm_utf_test_timer);
	cm_utf->test_id = CM_UTF_ID_MAX;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_cm_utf_update_conn_id_and_src_cb: Connection manager UTF
 * callback to update connect id and source of the connect request
 * @vdev: VDEV object
 * @cm_disconn_rsp: Disconnect Response
 *
 * Return: None
 */
static QDF_STATUS wlan_cm_utf_update_conn_id_and_src_cb(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_cm_source source, wlan_cm_id cm_id)
{
	struct wlan_cm_utf *cm_utf;

	cm_utf = wlan_cm_get_utf(vdev);
	if (!cm_utf) {
		mlme_err("cm_utf is Null");
		return QDF_STATUS_E_FAILURE;
	}

	cm_utf->last_cmd_id = cm_id;
	cm_utf->last_cmd_source = source;

	return QDF_STATUS_SUCCESS;
}

static struct mlme_cm_ops cm_ops = {
	.mlme_cm_connect_complete_cb = wlan_cm_utf_osif_connect_cb,
	.mlme_cm_failed_candidate_cb = NULL,
	.mlme_cm_update_id_and_src_cb =
			wlan_cm_utf_update_conn_id_and_src_cb,
	.mlme_cm_disconnect_complete_cb = wlan_cm_utf_osif_disconnect_cb,
	.mlme_cm_disconnect_start_cb = NULL,
};

static struct mlme_cm_ops *osif_cm_get_global_ops(void)
{
	return &cm_ops;
}

QDF_STATUS osif_cm_utf_register_cb(void)
{
	mlme_set_osif_cm_cb(osif_cm_get_global_ops);
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_cm_utf_update_connect_param: API to update connect req params
 * @cm_utf: CM UTF object
 * @buffer: user input buffer
 *
 * Return: None
 *
 */
static void
wlan_cm_utf_update_connect_param(struct wlan_cm_utf *cm_utf, char *buffer)
{
	char *token;
	uint8_t idx = 0;

	token = qdf_str_sep(&buffer, "\n");

	if (!token) {
		mlme_err("Invalid Parameters");
		return;
	}

	if (cm_utf->req.assoc_ie.ptr) {
		qdf_mem_free(cm_utf->req.assoc_ie.ptr);
		cm_utf->req.assoc_ie.ptr = NULL;
	}

	if (cm_utf->req.crypto.wep_keys.key) {
		qdf_mem_free(cm_utf->req.crypto.wep_keys.key);
		cm_utf->req.crypto.wep_keys.key = NULL;
	}

	qdf_mem_zero(cm_utf->req.ssid.ssid, WLAN_SSID_MAX_LEN);
	if (sscanf(token, "%2x:%2x:%2x:%2x:%2x:%2x ,%2x:%2x:%2x:%2x:%2x:%2x "
		   ",%2x:%2x:%2x:%2x:%2x:%2x ,%u ,%u ,%u ,%u ,%u ,%u ,%u ,%u "
		   ",%u ,%u ,%u ,%u ,%u ,%s",
		   (unsigned int *)&cm_utf->req.bssid.bytes[0],
		   (unsigned int *)&cm_utf->req.bssid.bytes[1],
		   (unsigned int *)&cm_utf->req.bssid.bytes[2],
		   (unsigned int *)&cm_utf->req.bssid.bytes[3],
		   (unsigned int *)&cm_utf->req.bssid.bytes[4],
		   (unsigned int *)&cm_utf->req.bssid.bytes[5],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[0],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[1],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[2],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[3],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[4],
		   (unsigned int *)&cm_utf->req.prev_bssid.bytes[5],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[0],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[1],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[2],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[3],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[4],
		   (unsigned int *)&cm_utf->req.bssid_hint.bytes[5],
		   &cm_utf->req.chan_freq,
		   &cm_utf->req.crypto.wpa_versions,
		   &cm_utf->req.crypto.auth_type,
		   &cm_utf->req.crypto.group_cipher,
		   &cm_utf->req.crypto.ciphers_pairwise,
		   &cm_utf->req.crypto.akm_suites,
		   (unsigned int *)&cm_utf->req.crypto.rsn_caps,
		   &cm_utf->req.crypto.mgmt_ciphers,
		   (unsigned int *)&cm_utf->req.ht_caps,
		   (unsigned int *)&cm_utf->req.ht_caps_mask,
		   &cm_utf->req.vht_caps,
		   &cm_utf->req.vht_caps_mask,
		   (unsigned int *)&cm_utf->req.ssid.length,
		   cm_utf->req.ssid.ssid) != 32) {
		mlme_err("Invalid connect req params");
		return;
	}

	qdf_err("Updated connect params %s", buffer);
	while ((token = qdf_str_sep(&buffer, ",")) != NULL) {
		switch (idx) {
		case 0:
			if (sscanf(token, "%u", &cm_utf->req.assoc_ie.len) != 1)
				return;
			if (!cm_utf->req.assoc_ie.len) {
				idx += 2;
				break;
			}
			idx++;
			break;
		case 1:
			cm_utf->req.assoc_ie.ptr =
				(uint8_t *)qdf_mem_malloc(cm_utf->req.assoc_ie.len);
			if (!cm_utf->req.assoc_ie.ptr) {
				mlme_err("Failed to alloc memory");
				return;
			}
			qdf_mem_copy(cm_utf->req.assoc_ie.ptr, token,
				     cm_utf->req.assoc_ie.len);
			idx++;
			break;
		case 2:
			if (sscanf(token, "%u",
				   (unsigned int *)&cm_utf->req.crypto.wep_keys.key_len) != 1)
				return;
			if (!cm_utf->req.crypto.wep_keys.key_len)
				return;
			idx++;
			break;
		case 3:
			cm_utf->req.crypto.wep_keys.key =
				(uint8_t *)qdf_mem_malloc(cm_utf->req.crypto.wep_keys.key_len);
			if (!cm_utf->req.crypto.wep_keys.key) {
				mlme_err("Failed to alloc memory");
				return;
			}
			qdf_mem_copy(cm_utf->req.crypto.wep_keys.key, token,
				     cm_utf->req.crypto.wep_keys.key_len);
			break;
		default:
			break;
		}
	}
}

/**
 * wlan_cm_utf_start_test: Connection manager UTF entry function
 * @cm_utf: Connection Manager UTF object
 * *test_id: Test ID requested by user
 *
 */
static void wlan_cm_utf_start_test(struct wlan_cm_utf *cm_utf, void *buffer)
{
	int reason = 0, ret;
	uint8_t i = 0;
	char *token;
	char *token2;
	char *buff = (char *)buffer;
	struct wlan_cm_disconnect_req req;

	if (cm_utf->test_id != CM_UTF_ID_MAX) {
		mlme_err("Test Already in progress");
		return;
	}

	token = qdf_str_sep(&buff, " ");
	if (token) {
		if (!qdf_str_cmp(token, "CONNECT_REQ")) {
			mlme_err("Update Connect Params");
			wlan_cm_utf_update_connect_param(cm_utf, buff);
			return;
		}

		token2 = qdf_str_sep(&token, "\n");
		for (i = 0; i < CM_UTF_ID_MAX; i++) {
			if (!qdf_str_cmp(token2, cm_utf_test_names[i])) {
				cm_utf->test_id = i;
				break;
			}
		}
	}

	if (cm_utf->test_id == CM_UTF_ID_MAX) {
		mlme_err("Invalid Test Case");
		return;
	}

	mlme_err("Test ID: %d", cm_utf->test_id);
	switch (cm_utf->test_id) {
	case CM_UTF_ID_CONNECT_SUCCESS:
	case CM_UTF_ID_PEER_CREATE_FAILURE:
	case CM_UTF_ID_PEER_CREATE_TIMEOUT:
	case CM_UTF_ID_AUTH_FAILURE:
	case CM_UTF_ID_AUTH_TIMEOUT:
	case CM_UTF_ID_ASSOC_FAILURE:
	case CM_UTF_ID_ASSOC_TIMEOUT:
	case CM_UTF_ID_CONNECT_SCAN_FAILURE:
	case CM_UTF_ID_CONNECT_SER_TIMEOUT:
	case CM_UTF_ID_CONNECT_SER_FAILED:
		cm_utf->req.vdev_id = wlan_vdev_get_id(cm_utf->vdev);
		cm_utf->req.source = CM_OSIF_CONNECT;
		ucfg_cm_start_connect(cm_utf->vdev, &cm_utf->req);
		break;
	case CM_UTF_ID_DISCONNECT_SUCCESS:
	case CM_UTF_ID_DISCONNECT_SER_TIMEOUT:
	case CM_UTF_ID_DISCONNECT_SER_FAILED:
	case CM_UTF_ID_PEER_DELETE_TIMEOUT:
		token = qdf_str_sep(&buff, " ");
		if (!token) {
			mlme_err("Provide reason code for disconnect");
			cm_utf->test_id = CM_UTF_ID_MAX;
			return;
		} else {
			ret = qdf_kstrtoint(token, 0, &reason);
			if (ret) {
				mlme_err("Invalid disconnect reason");
				cm_utf->test_id = CM_UTF_ID_MAX;
				return;
			}
		}
		mlme_err("Disconnect reason %d", reason);
		req.vdev_id = wlan_vdev_get_id(cm_utf->vdev);
		req.source = CM_OSIF_DISCONNECT;
		req.reason_code = reason;
		ucfg_cm_start_disconnect(cm_utf->vdev, &req);
		break;
	default:
		mlme_err("Invalid test ID");
		cm_utf->test_id = CM_UTF_ID_MAX;
		qdf_timer_stop(&cm_utf->cm_utf_test_timer);
		break;
	}

	if (cm_utf->test_id == CM_UTF_ID_PEER_DELETE_TIMEOUT ||
	    cm_utf->test_id == CM_UTF_ID_PEER_CREATE_TIMEOUT ||
	    cm_utf->test_id == CM_UTF_ID_DISCONNECT_SER_TIMEOUT ||
	    cm_utf->test_id == CM_UTF_ID_CONNECT_SER_TIMEOUT)
		qdf_timer_start(&cm_utf->cm_utf_test_timer, 70000);
	else
		qdf_timer_start(&cm_utf->cm_utf_test_timer, 10000);
}

int wlan_cm_utf_cm_test_id_show(qdf_debugfs_file_t m, void *v)
{
	uint8_t i;

	mlme_nofl_err("Usage:");
	for (i = 0; i < CM_UTF_ID_MAX; i++) {
		switch (i) {
		case CM_UTF_ID_DISCONNECT_SUCCESS:
		case CM_UTF_ID_DISCONNECT_SER_TIMEOUT:
		case CM_UTF_ID_DISCONNECT_SER_FAILED:
		case CM_UTF_ID_PEER_DELETE_TIMEOUT:
			mlme_nofl_err("%22s <reason>", cm_utf_test_names[i]);
			break;
		default:
			mlme_nofl_err("%22s", cm_utf_test_names[i]);
			break;
		}
	}
	return 0;
}

ssize_t wlan_cm_utf_cm_test_id_write(struct file *file,
				     const char __user *buf,
				     size_t count, loff_t *ppos)
{
	struct wlan_cm_utf *cm_utf =
			((struct seq_file *)file->private_data)->private;
	char *locbuf;

	if ((!buf) || (count <= 0))
		return -EFAULT;

	locbuf = (char *)qdf_mem_malloc(count + 1);

	if (!locbuf)
		return -EFAULT;

	qdf_mem_zero(locbuf, count + 1);

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	mlme_err("CM Start Test");
	wlan_cm_utf_start_test(cm_utf, locbuf);
	qdf_mem_free(locbuf);

	return count;
}
#endif
