/*
* Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Implements mc addr filtering offload feature API's
 */

#include "wlan_pmo_mc_addr_filtering.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"


#define PMO_INVALID_MC_ADDR_COUNT (-1)

static void pmo_core_fill_mc_list(struct pmo_vdev_priv_obj **vdev_ctx,
	struct pmo_mc_addr_list_params *ip)
{
	struct pmo_mc_addr_list *op_list;
	int i;
	static const uint8_t ipv6_rs[] = {
		0x33, 0x33, 0x00, 0x00, 0x00, 0x02};
	struct pmo_vdev_priv_obj *temp_ctx;
	uint8_t addr_fp;

	temp_ctx = *vdev_ctx;
	addr_fp = temp_ctx->addr_filter_pattern;
	op_list = &temp_ctx->vdev_mc_list_req;

	qdf_spin_lock_bh(&temp_ctx->pmo_vdev_lock);
	op_list->mc_cnt = ip->count;
	qdf_spin_unlock_bh(&temp_ctx->pmo_vdev_lock);

	for (i = 0; i < ip->count; i++) {
		pmo_debug("%pM", ip->mc_addr[i].bytes);
		/*
		 * Skip following addresses:
		 * 1)IPv6 router solicitation address
		 * 2)Any other address pattern if its set during
		 *  RXFILTER REMOVE driver command based on
		 *  addr_filter_pattern
		 */
		if ((!qdf_mem_cmp(ip->mc_addr[i].bytes, ipv6_rs,
			QDF_MAC_ADDR_SIZE)) ||
		   (addr_fp &&
		   (!qdf_mem_cmp(ip->mc_addr[i].bytes, &addr_fp, 1)))) {
			pmo_debug("MC/BC filtering Skip addr %pM",
				ip->mc_addr[i].bytes);
			qdf_spin_lock_bh(&temp_ctx->pmo_vdev_lock);
			op_list->mc_cnt--;
			qdf_spin_unlock_bh(&temp_ctx->pmo_vdev_lock);
			continue;
		}
		qdf_spin_lock_bh(&temp_ctx->pmo_vdev_lock);
		qdf_mem_set(&(op_list->mc_addr[i].bytes), 0,
			QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(&(op_list->mc_addr[i].bytes),
			ip->mc_addr[i].bytes, QDF_MAC_ADDR_SIZE);
		qdf_spin_unlock_bh(&temp_ctx->pmo_vdev_lock);
		pmo_debug("mlist[%pM] = ", op_list->mc_addr[i].bytes);
	}
}

static QDF_STATUS pmo_core_cache_mc_addr_list_in_vdev_priv(
		struct pmo_mc_addr_list_params *mc_list_config,
		struct wlan_objmgr_vdev *vdev)
{
	struct pmo_vdev_priv_obj *vdev_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pmo_core_fill_mc_list(&vdev_ctx, mc_list_config);

	return status;
}

static QDF_STATUS pmo_core_flush_mc_addr_list_from_vdev_priv(
			struct wlan_objmgr_vdev *vdev)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	qdf_mem_zero(&vdev_ctx->vdev_mc_list_req,
		sizeof(vdev_ctx->vdev_mc_list_req));
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_set_mc_filter_req(struct wlan_objmgr_vdev *vdev,
	struct pmo_mc_addr_list *mc_list)
{
	struct pmo_vdev_priv_obj *vdev_ctx;
	uint8_t vdev_id;
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();
	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}
	vdev_id = pmo_get_vdev_id(vdev);
	/*
	 * Configure enhance multicast offload feature for filtering out
	 * multicast IP data packets transmitted using unicast MAC address
	 */

	/*
	* TODO
	{//(WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
		//WMI_SERVICE_ENHANCED_MCAST_FILTER)) {
	*/
	if (1) {
		pmo_info("FW supports enhance multicast offload");
		pmo_tgt_send_enhance_multicast_offload_req(vdev, vdev_id,
			false);
	} else {
		pmo_info("FW does not support enhance multicast offload");
	}

	/*
	 * set mc_param->action to clear MCList and reset
	 * to configure the MCList in FW
	 */
	for (i = 0; i < mc_list->mc_cnt; i++) {
		pmo_tgt_set_mc_filter_req(vdev,
					mc_list->mc_addr[i]);
	}
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_clear_mc_filter_req(struct wlan_objmgr_vdev *vdev,
	struct pmo_mc_addr_list *mc_list)
{
	struct pmo_vdev_priv_obj *vdev_ctx;
	uint8_t vdev_id;
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();
	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}
	vdev_id = pmo_get_vdev_id(vdev);

	/*
	 * Configure enhance multicast offload feature for filtering out
	 * multicast IP data packets transmitted using unicast MAC address
	 */

	/*
	* TODO
	{//(WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
		//WMI_SERVICE_ENHANCED_MCAST_FILTER)) {
	*/
	if (1) {
		pmo_info("FW supports enhance multicast offload");
		pmo_tgt_send_enhance_multicast_offload_req(vdev, vdev_id,
			true);
	} else {
		pmo_info("FW does not support enhance multicast offload");
	}

	/*
	 * set mcbc_param->action to clear MCList and reset
	 * to configure the MCList in FW
	 */
	for (i = 0; i < mc_list->mc_cnt; i++) {
		pmo_tgt_clear_mc_filter_req(vdev,
					mc_list->mc_addr[i]);
	}
out:
	PMO_EXIT();

	return status;
}

static QDF_STATUS pmo_core_do_enable_mc_addr_list(struct wlan_objmgr_vdev *vdev,
	struct pmo_vdev_priv_obj *vdev_ctx,
	struct pmo_mc_addr_list *op_mc_list_req)
{
	QDF_STATUS status;

	PMO_ENTER();
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	if (!vdev_ctx->vdev_mc_list_req.mc_cnt) {
		qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
		pmo_err("mc_cnt is zero so skip to add mc list");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}
	qdf_mem_copy(op_mc_list_req, &vdev_ctx->vdev_mc_list_req,
		sizeof(*op_mc_list_req));
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	status = pmo_core_set_mc_filter_req(vdev, op_mc_list_req);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("cannot apply mc filter request");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->vdev_mc_list_req.is_filter_applied = true;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
out:
	PMO_EXIT();

	return status;
}

static QDF_STATUS pmo_core_do_disable_mc_addr_list(
	struct wlan_objmgr_vdev *vdev,
	struct pmo_vdev_priv_obj *vdev_ctx,
	struct pmo_mc_addr_list *op_mc_list_req)
{
	QDF_STATUS status;

	PMO_ENTER();
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	/* validate filter is applied before clearing in fwr */
	if (!vdev_ctx->vdev_mc_list_req.is_filter_applied) {
		qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
		pmo_err("mc filter is not applied in fwr");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}
	qdf_mem_copy(op_mc_list_req, &vdev_ctx->vdev_mc_list_req,
		sizeof(*op_mc_list_req));
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	status = pmo_core_clear_mc_filter_req(vdev, op_mc_list_req);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("cannot apply mc filter request");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->vdev_mc_list_req.is_filter_applied = false;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
out:
	PMO_EXIT();

	return status;
}

uint8_t pmo_core_max_mc_addr_supported(struct wlan_objmgr_psoc *psoc)
{
	return PMO_MAX_MC_ADDR_LIST;
}

int pmo_core_get_mc_addr_list_count(struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	struct wlan_objmgr_vdev *vdev;
	struct pmo_vdev_priv_obj *vdev_ctx;
	uint8_t mc_cnt;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx) {
		pmo_err("psoc_ctx is");
		mc_cnt = PMO_INVALID_MC_ADDR_COUNT;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		mc_cnt = PMO_INVALID_MC_ADDR_COUNT;
		goto out;
	}

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		mc_cnt = PMO_INVALID_MC_ADDR_COUNT;
		goto dec_ref;
	}
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	mc_cnt = vdev_ctx->vdev_mc_list_req.mc_cnt;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:

	return mc_cnt;
}

void pmo_core_set_mc_addr_list_count(struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id, uint8_t count)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	struct pmo_vdev_priv_obj *vdev_ctx;
	struct wlan_objmgr_vdev *vdev;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx) {
		pmo_err("psoc_ctx is NULL");
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		goto out;
	}

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		goto dec_ref;
	}
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->vdev_mc_list_req.mc_cnt = count;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:

	return;
}

static QDF_STATUS pmo_core_mc_addr_flitering_sanity(
			struct wlan_objmgr_vdev *vdev)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	if (!vdev) {
		pmo_err("vdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Check if INI is enabled or not, otherwise just return */
	if (!vdev_ctx->pmo_psoc_ctx->psoc_cfg.enable_mc_list) {
		pmo_info("user disabled mc_addr_list using INI");
		return QDF_STATUS_E_INVAL;
	}

	if (!pmo_core_is_vdev_supports_offload(vdev)) {
		pmo_info("vdev in invalid opmode for mc addr filtering %d",
			pmo_get_vdev_opmode(vdev));
		return QDF_STATUS_E_INVAL;
	}

	if (!pmo_core_is_vdev_connected(vdev))
		return QDF_STATUS_E_INVAL;

	return QDF_STATUS_SUCCESS;
}
QDF_STATUS pmo_core_cache_mc_addr_list(
		struct pmo_mc_addr_list_params *mc_list_config)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	PMO_ENTER();
	if (!mc_list_config->psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(mc_list_config->psoc,
			mc_list_config->vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_core_mc_addr_flitering_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

	pmo_info("Cache mc addr list for vdev id: %d psoc: %p vdev: %p",
			mc_list_config->vdev_id, mc_list_config->psoc, vdev);

	status = pmo_core_cache_mc_addr_list_in_vdev_priv(mc_list_config, vdev);
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_flush_mc_addr_list(struct wlan_objmgr_psoc *psoc,
	uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_core_mc_addr_flitering_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

	pmo_info("Flush mc addr list for vdev id: %d psoc: %p vdev: %p",
			vdev_id, psoc, vdev);

	status = pmo_core_flush_mc_addr_list_from_vdev_priv(vdev);
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}

static QDF_STATUS pmo_core_handle_enable_mc_list_trigger(
			struct wlan_objmgr_vdev *vdev,
			enum pmo_offload_trigger trigger)
{
	struct pmo_vdev_priv_obj *vdev_ctx;
	QDF_STATUS status;
	struct pmo_mc_addr_list *op_mc_list_req = NULL;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	op_mc_list_req = qdf_mem_malloc(sizeof(*op_mc_list_req));
	if (!op_mc_list_req) {
		pmo_err("op_mc_list_req is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	switch (trigger) {
	case pmo_mc_list_change_notify:
		if (!vdev_ctx->pmo_psoc_ctx->psoc_cfg.active_mode_offload) {
			pmo_info("active offload is disabled, skip in mode: %d",
				trigger);
			status = QDF_STATUS_E_INVAL;
			goto out;
		}
		status = pmo_core_do_enable_mc_addr_list(vdev, vdev_ctx,
				op_mc_list_req);
		break;
	case pmo_apps_suspend:
		if (vdev_ctx->pmo_psoc_ctx->psoc_cfg.active_mode_offload) {
			pmo_info("active offload is enabled, skip in mode: %d",
				trigger);
			status = QDF_STATUS_E_INVAL;
			goto out;
		}
		status = pmo_core_do_enable_mc_addr_list(vdev, vdev_ctx,
				op_mc_list_req);
		break;
	default:
		status = QDF_STATUS_E_INVAL;
		pmo_err("invalid pmo trigger for enable mc list");
		break;
	}
out:
	if (op_mc_list_req)
		qdf_mem_free(op_mc_list_req);

	return status;
}

QDF_STATUS pmo_core_enable_mc_addr_filtering_in_fwr(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_core_mc_addr_flitering_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

	pmo_info("enable mclist trigger: %d", trigger);
	status = pmo_core_handle_enable_mc_list_trigger(vdev, trigger);
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}

static QDF_STATUS pmo_core_handle_disable_mc_list_trigger(
			struct wlan_objmgr_vdev *vdev,
			enum pmo_offload_trigger trigger)
{
	struct pmo_vdev_priv_obj *vdev_ctx;
	QDF_STATUS status;
	struct pmo_mc_addr_list *op_mc_list_req = NULL;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	op_mc_list_req = qdf_mem_malloc(sizeof(*op_mc_list_req));
	if (!op_mc_list_req) {
		pmo_err("op_mc_list_req is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	switch (trigger) {
	case pmo_peer_disconnect:
	case pmo_mc_list_change_notify:
		if (!vdev_ctx->pmo_psoc_ctx->psoc_cfg.active_mode_offload) {
			pmo_info("active offload is disabled, skip in mode: %d",
				trigger);
			status = QDF_STATUS_E_INVAL;
			goto out;
		}
		status = pmo_core_do_disable_mc_addr_list(vdev, vdev_ctx,
				op_mc_list_req);
		break;
	case pmo_apps_resume:
		if (vdev_ctx->pmo_psoc_ctx->psoc_cfg.active_mode_offload) {
			pmo_info("active offload is enabled, skip in mode: %d",
				trigger);
			status = QDF_STATUS_E_INVAL;
			goto out;
		}
		status = pmo_core_do_disable_mc_addr_list(vdev, vdev_ctx,
				op_mc_list_req);
		break;
	default:
		status = QDF_STATUS_E_INVAL;
		pmo_err("invalid pmo trigger for disable mc list");
		break;
	}
out:
	if (op_mc_list_req)
		qdf_mem_free(op_mc_list_req);

	return status;
}

QDF_STATUS pmo_core_disable_mc_addr_filtering_in_fwr(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_core_mc_addr_flitering_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

	pmo_info("disable mclist trigger: %d", trigger);
	status = pmo_core_handle_disable_mc_list_trigger(vdev, trigger);
dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}

