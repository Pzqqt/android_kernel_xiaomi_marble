/*
 * Copyright (c) 2011,2017-2019 The Linux Foundation. All rights reserved.
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
tgt_get_target_handle(struct wlan_objmgr_pdev *pdev)
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

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_init(
		pdev);
}

void
tgt_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_deinit(pdev);
}

QDF_STATUS
tgt_set_spectral_config(struct wlan_objmgr_pdev *pdev,
			const u_int32_t threshtype, const u_int32_t value,
			const enum spectral_scan_mode smode,
			enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_spectral_config(
		pdev, threshtype, value, smode, err);
}

QDF_STATUS
tgt_get_spectral_config(struct wlan_objmgr_pdev *pdev,
			struct spectral_config *sptrl_config,
			const enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_config(
			pdev,
			sptrl_config,
			smode);
}

QDF_STATUS
tgt_start_spectral_scan(struct wlan_objmgr_pdev *pdev,
			enum spectral_scan_mode smode,
			enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_start_spectral_scan(
		pdev, smode, err);
}

QDF_STATUS
tgt_stop_spectral_scan(struct wlan_objmgr_pdev *pdev,
		       enum spectral_scan_mode smode,
		       enum spectral_cp_error_code *err)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_stop_spectral_scan(
							pdev, smode, err);
}

bool
tgt_is_spectral_active(struct wlan_objmgr_pdev *pdev,
		       enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_active(
		pdev, smode);
}

bool
tgt_is_spectral_enabled(struct wlan_objmgr_pdev *pdev,
			enum spectral_scan_mode smode)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_enabled(
		pdev, smode);
}

QDF_STATUS
tgt_set_debug_level(struct wlan_objmgr_pdev *pdev, u_int32_t debug_level)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_debug_level(
			pdev,
			debug_level);
}

u_int32_t
tgt_get_debug_level(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_debug_level(pdev);
}

QDF_STATUS
tgt_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev,
			 struct spectral_caps *scaps)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_capinfo(
		pdev, scaps);
}

QDF_STATUS
tgt_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
			   struct spectral_diag_stats *stats)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_diagstats(
		pdev, stats);
}

void
tgt_register_wmi_spectral_cmd_ops(
	struct wlan_objmgr_pdev *pdev,
	struct wmi_spectral_cmd_ops *cmd_ops)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_wmi_spectral_cmd_ops(pdev,
								    cmd_ops);
}

void
tgt_spectral_register_nl_cb(
	struct wlan_objmgr_pdev *pdev,
	struct spectral_nl_cb *nl_cb)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_netlink_cb(pdev,
							  nl_cb);
}

bool
tgt_spectral_use_nl_bcast(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_use_nl_bcast(pdev);
}

void tgt_spectral_deregister_nl_cb(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	psptrl_tx_ops->sptrlto_deregister_netlink_cb(pdev);
}

int
tgt_spectral_process_report(struct wlan_objmgr_pdev *pdev,
			    void *payload)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_process_spectral_report(pdev, payload);
}

uint32_t
tgt_spectral_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;

	target_type_tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;

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

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_tx_ops = &psoc->soc_cb.tx_ops.dbr_tx_ops;
	sptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;
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

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_tx_ops = &psoc->soc_cb.tx_ops.dbr_tx_ops;
	sptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

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

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		spectral_err("psoc is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_dma_debug(
			pdev,
			dma_debug_type,
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
