/*
 * Copyright (c) 2011,2017-2020 The Linux Foundation. All rights reserved.
 *
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

#include <wlan_spectral_tgt_api.h>
#include <wlan_spectral_utils_api.h>
#include <target_type.h>

#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>

#define DBR_EVENT_TIMEOUT_IN_MS_SPECTRAL 1
#define DBR_NUM_RESP_PER_EVENT_SPECTRAL 2
#endif

void *
tgt_get_pdev_target_handle(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_spectral *ps;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return NULL;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		spectral_err("PDEV SPECTRAL object is NULL!");
		return NULL;
	}
	return ps->psptrl_target_handle;
}

void *
tgt_get_psoc_target_handle(struct wlan_objmgr_psoc *psoc)
{
	struct spectral_context *sc;

	if (!psoc) {
		spectral_err("psoc is NULL!");
		return NULL;
	}

	sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!sc) {
		spectral_err("psoc Spectral object is NULL!");
		return NULL;
	}

	return sc->psoc_target_handle;
}

QDF_STATUS
tgt_spectral_control(
	struct wlan_objmgr_pdev *pdev,
	struct spectral_cp_request *sscan_req)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return -EPERM;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return -EPERM;
	}
	return spectral_control_cmn(pdev, sscan_req);
}

void *
tgt_pdev_spectral_init(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return NULL;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_init(pdev);
}

void
tgt_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return;
	}

	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_deinit(pdev);
}

void *
tgt_psoc_spectral_init(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return NULL;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_psoc_spectral_init(psoc);
}

void
tgt_psoc_spectral_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return;
	}

	tx_ops->sptrl_tx_ops.sptrlto_psoc_spectral_deinit(psoc);
}

QDF_STATUS
tgt_set_spectral_config(struct wlan_objmgr_pdev *pdev,
			const struct spectral_cp_param *param,
			const enum spectral_scan_mode smode,
			enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_set_spectral_config(pdev, param,
			smode, err);
}

QDF_STATUS
tgt_get_spectral_config(struct wlan_objmgr_pdev *pdev,
			struct spectral_config *sptrl_config,
			const enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_get_spectral_config(pdev,
			sptrl_config,
			smode);
}

QDF_STATUS
tgt_start_spectral_scan(struct wlan_objmgr_pdev *pdev,
			uint8_t vdev_id,
			enum spectral_scan_mode smode,
			enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_start_spectral_scan(pdev, vdev_id,
								smode, err);
}

QDF_STATUS
tgt_stop_spectral_scan(struct wlan_objmgr_pdev *pdev,
		       enum spectral_scan_mode smode,
		       enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_stop_spectral_scan(pdev, smode,
			err);
}

bool
tgt_is_spectral_active(struct wlan_objmgr_pdev *pdev,
		       enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return false;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_is_spectral_active(pdev, smode);
}

bool
tgt_is_spectral_enabled(struct wlan_objmgr_pdev *pdev,
			enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return false;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_is_spectral_enabled(pdev, smode);
}

QDF_STATUS
tgt_set_debug_level(struct wlan_objmgr_pdev *pdev, u_int32_t debug_level)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_set_debug_level(pdev, debug_level);
}

u_int32_t
tgt_get_debug_level(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return -EINVAL;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_get_debug_level(pdev);
}

QDF_STATUS
tgt_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev,
			 struct spectral_caps *scaps)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_get_spectral_capinfo(pdev, scaps);
}

QDF_STATUS
tgt_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
			   struct spectral_diag_stats *stats)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_get_spectral_diagstats(pdev, stats);
}

QDF_STATUS
tgt_register_spectral_wmi_ops(struct wlan_objmgr_psoc *psoc,
			      struct spectral_wmi_ops *wmi_ops)
{
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!wmi_ops) {
		spectral_err("WMI operations table is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_spectral_wmi_ops(psoc, wmi_ops);
}

QDF_STATUS
tgt_register_spectral_tgt_ops(struct wlan_objmgr_psoc *psoc,
			      struct spectral_tgt_ops *tgt_ops)
{
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!tgt_ops) {
		spectral_err("Target operations table is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_spectral_tgt_ops(psoc, tgt_ops);
}

void
tgt_spectral_register_nl_cb(
		struct wlan_objmgr_pdev *pdev,
		struct spectral_nl_cb *nl_cb)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_netlink_cb(pdev, nl_cb);
}

bool
tgt_spectral_use_nl_bcast(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return false;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_use_nl_bcast(pdev);
}

void tgt_spectral_deregister_nl_cb(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	psptrl_tx_ops->sptrlto_deregister_netlink_cb(pdev);
}

int
tgt_spectral_process_report(struct wlan_objmgr_pdev *pdev,
			    void *payload)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return -EINVAL;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_process_spectral_report(pdev, payload);
}

uint32_t
tgt_spectral_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return target_type;
	}

	target_type_tx_ops = &tx_ops->target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	return target_type;
}

#ifdef DIRECT_BUF_RX_ENABLE
QDF_STATUS
tgt_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *sptrl_tx_ops = NULL;
	struct dbr_module_config dbr_config = {0};
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	sptrl_tx_ops = &tx_ops->sptrl_tx_ops;
	dbr_config.num_resp_per_event = DBR_NUM_RESP_PER_EVENT_SPECTRAL;
	dbr_config.event_timeout_in_ms = DBR_EVENT_TIMEOUT_IN_MS_SPECTRAL;

	if ((sptrl_tx_ops->sptrlto_direct_dma_support) &&
	    (sptrl_tx_ops->sptrlto_direct_dma_support(pdev))) {
		if (sptrl_tx_ops->sptrlto_check_and_do_dbr_buff_debug)
			sptrl_tx_ops->sptrlto_check_and_do_dbr_buff_debug(pdev);
		if (dbr_tx_ops->direct_buf_rx_module_register)
			dbr_tx_ops->direct_buf_rx_module_register
				(pdev, 0, &dbr_config,
				 spectral_dbr_event_handler);
		if (sptrl_tx_ops->sptrlto_check_and_do_dbr_ring_debug)
			sptrl_tx_ops->sptrlto_check_and_do_dbr_ring_debug(pdev);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_spectral_unregister_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *sptrl_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	sptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	if ((sptrl_tx_ops->sptrlto_direct_dma_support) &&
	    (sptrl_tx_ops->sptrlto_direct_dma_support(pdev))) {
		/* Stop DBR debug as the buffers itself are freed now */
		if (dbr_tx_ops->direct_buf_rx_stop_ring_debug)
			dbr_tx_ops->direct_buf_rx_stop_ring_debug(pdev, 0);

		/*No need to zero-out as buffers are anyway getting freed*/
		if (dbr_tx_ops->direct_buf_rx_stop_buffer_poisoning)
			dbr_tx_ops->direct_buf_rx_stop_buffer_poisoning
				(pdev, 0);
		if (dbr_tx_ops->direct_buf_rx_module_unregister)
			dbr_tx_ops->direct_buf_rx_module_unregister
				(pdev, 0);

		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#else
QDF_STATUS
tgt_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_spectral_unregister_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DIRECT_BUF_RX_ENABLE */

#ifdef DIRECT_BUF_RX_DEBUG
QDF_STATUS tgt_set_spectral_dma_debug(struct wlan_objmgr_pdev *pdev,
				      enum spectral_dma_debug dma_debug_type,
				      bool dma_debug_enable)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return tx_ops->sptrl_tx_ops.sptrlto_set_dma_debug(pdev, dma_debug_type,
			dma_debug_enable);
}
#else
QDF_STATUS tgt_set_spectral_dma_debug(struct wlan_objmgr_pdev *pdev,
				      enum spectral_dma_debug dma_debug_type,
				      bool dma_debug_enable)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS
tgt_spectral_register_events(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_events(psoc);
}

QDF_STATUS
tgt_spectral_unregister_events(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psptrl_tx_ops = &tx_ops->sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_unregister_events(psoc);
}

QDF_STATUS
tgt_spectral_init_pdev_feature_caps(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_sptrl_tx_ops *spectral_tx_ops;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	spectral_tx_ops = wlan_spectral_pdev_get_lmac_if_txops(pdev);
	if (!spectral_tx_ops) {
		spectral_err("Spectral txops is null");
		return QDF_STATUS_E_FAILURE;
	}

	return spectral_tx_ops->sptrlto_init_pdev_feature_caps(pdev);
}
