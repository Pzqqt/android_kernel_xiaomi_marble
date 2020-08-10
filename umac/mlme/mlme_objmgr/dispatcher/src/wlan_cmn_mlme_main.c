/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implements MLME global APIs
 */

#include <wlan_objmgr_cmn.h>
#include <include/wlan_mlme_cmn.h>
#include <include/wlan_pdev_mlme.h>
#include <include/wlan_vdev_mlme.h>
#include <include/wlan_mlme_cmn.h>
#include <wlan_psoc_mlme_main.h>
#include <wlan_pdev_mlme_main.h>
#include <wlan_vdev_mlme_main.h>

struct mlme_ext_ops *glbl_ops;
mlme_get_global_ops_cb glbl_ops_cb;

QDF_STATUS wlan_cmn_mlme_init(void)
{
	QDF_STATUS status;

	status = wlan_psoc_mlme_init();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_pdev_mlme_init();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_vdev_mlme_init();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	if (glbl_ops_cb)
		glbl_ops = glbl_ops_cb();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cmn_mlme_deinit(void)
{
	QDF_STATUS status;

	status = wlan_vdev_mlme_deinit();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_pdev_mlme_deinit();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_psoc_mlme_deinit();
	if (status != QDF_STATUS_SUCCESS)
		return status;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_psoc_ops_ext_hdl_create(struct psoc_mlme_obj *psoc_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_psoc_ext_hdl_create)
		ret = glbl_ops->mlme_psoc_ext_hdl_create(psoc_mlme);

	return ret;
}

QDF_STATUS mlme_psoc_ops_ext_hdl_destroy(struct psoc_mlme_obj *psoc_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_psoc_ext_hdl_destroy)
		ret = glbl_ops->mlme_psoc_ext_hdl_destroy(psoc_mlme);

	return ret;
}

QDF_STATUS mlme_pdev_ops_ext_hdl_create(struct pdev_mlme_obj *pdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_pdev_ext_hdl_create)
		ret = glbl_ops->mlme_pdev_ext_hdl_create(pdev_mlme);

	return ret;
}

QDF_STATUS mlme_pdev_ops_ext_hdl_destroy(struct pdev_mlme_obj *pdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_pdev_ext_hdl_destroy)
		ret = glbl_ops->mlme_pdev_ext_hdl_destroy(pdev_mlme);

	return ret;
}

QDF_STATUS mlme_vdev_ops_ext_hdl_create(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_ext_hdl_create)
		ret = glbl_ops->mlme_vdev_ext_hdl_create(vdev_mlme);

	return ret;
}

QDF_STATUS mlme_vdev_ops_ext_hdl_post_create(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_ext_hdl_post_create)
		ret = glbl_ops->mlme_vdev_ext_hdl_post_create(vdev_mlme);

	return ret;
}

QDF_STATUS mlme_vdev_ops_ext_hdl_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_ext_hdl_destroy)
		ret = glbl_ops->mlme_vdev_ext_hdl_destroy(vdev_mlme);

	return ret;
}

QDF_STATUS mlme_vdev_ops_start_fw_send(struct wlan_objmgr_vdev *vdev,
				       uint8_t restart)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_start_fw_send)
		ret = glbl_ops->mlme_vdev_start_fw_send(vdev, restart);

	return ret;
}

QDF_STATUS mlme_vdev_ops_multivdev_restart_fw_cmd_send(
						struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_multivdev_restart_fw_send)
		ret = glbl_ops->mlme_multivdev_restart_fw_send(pdev);

	return ret;
}

QDF_STATUS mlme_vdev_ops_stop_fw_send(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_stop_fw_send)
		ret = glbl_ops->mlme_vdev_stop_fw_send(vdev);

	return ret;
}

QDF_STATUS mlme_vdev_ops_down_fw_send(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_down_fw_send)
		ret = glbl_ops->mlme_vdev_down_fw_send(vdev);

	return ret;
}

QDF_STATUS mlme_vdev_enqueue_exp_ser_cmd(struct vdev_mlme_obj *vdev_mlme,
					 uint8_t cmd_type)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (glbl_ops && glbl_ops->mlme_vdev_enqueue_exp_cmd)
		ret = glbl_ops->mlme_vdev_enqueue_exp_cmd(vdev_mlme, cmd_type);

	return ret;
}

QDF_STATUS mlme_vdev_ops_ext_hdl_delete_rsp(struct wlan_objmgr_psoc *psoc,
					    struct vdev_delete_response *rsp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_vdev_ext_delete_rsp)
		ret = glbl_ops->mlme_vdev_ext_delete_rsp(psoc, rsp);

	return ret;
}

QDF_STATUS mlme_vdev_ops_ext_hdl_multivdev_restart_resp(
					struct wlan_objmgr_psoc *psoc,
					struct multi_vdev_restart_resp *resp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_multi_vdev_restart_resp)
		ret = glbl_ops->mlme_multi_vdev_restart_resp(psoc, resp);

	return ret;
}

#ifdef FEATURE_CM_ENABLE
QDF_STATUS mlme_cm_ops_ext_connect_start(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_req *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_connect_start)
		ret = glbl_ops->mlme_cm_ext_connect_start(vdev, req);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_connect_active(struct wlan_objmgr_vdev *vdev,
					  struct wlan_cm_vdev_connect_req *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_connect_active)
		ret = glbl_ops->mlme_cm_ext_connect_active(vdev, req);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
					       struct qdf_mac_addr *peer_mac)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_bss_peer_create_req)
		ret = glbl_ops->mlme_cm_ext_bss_peer_create_req(vdev,
								     peer_mac);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_connect_req(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_vdev_connect_req *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_connect_req)
		ret = glbl_ops->mlme_cm_ext_connect_req(vdev, req);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_connect_complete(struct wlan_objmgr_vdev *vdev,
					    struct wlan_cm_connect_rsp *rsp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_connect_complete)
		ret = glbl_ops->mlme_cm_ext_connect_complete(vdev, rsp);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_disconnect_start(struct wlan_objmgr_vdev *vdev,
					    struct wlan_cm_disconnect_req *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_disconnect_start)
		ret = glbl_ops->mlme_cm_ext_disconnect_start(vdev, req);

	return ret;
}

QDF_STATUS
mlme_cm_ops_ext_disconnect_active(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_vdev_discon_req *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_disconnect_active)
		ret = glbl_ops->mlme_cm_ext_disconnect_active(vdev, req);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_bss_peer_delete_req(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_bss_peer_delete_req)
		ret = glbl_ops->mlme_cm_ext_bss_peer_delete_req(vdev);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_disconnect_complete(struct wlan_objmgr_vdev *vdev,
					       struct wlan_cm_discon_rsp *rsp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_disconnect_complete)
		ret = glbl_ops->mlme_cm_ext_disconnect_complete(vdev, rsp);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_osif_connect(struct wlan_objmgr_vdev *vdev,
					struct wlan_cm_connect_rsp *rsp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_osif_connect)
		ret = glbl_ops->mlme_cm_ext_osif_connect(vdev, rsp);

	return ret;
}

QDF_STATUS mlme_cm_ops_ext_osif_disconnect(struct wlan_objmgr_vdev *vdev,
					   struct wlan_cm_discon_rsp *rsp)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_osif_disconnect)
		ret = glbl_ops->mlme_cm_ext_osif_disconnect(vdev, rsp);

	return ret;
}

QDF_STATUS mlme_cm_ext_vdev_down(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((glbl_ops) && glbl_ops->mlme_cm_ext_vdev_down)
		ret = glbl_ops->mlme_cm_ext_vdev_down(vdev);
	return ret;
}
#endif
void mlme_set_ops_register_cb(mlme_get_global_ops_cb ops_cb)
{
	glbl_ops_cb = ops_cb;
}
