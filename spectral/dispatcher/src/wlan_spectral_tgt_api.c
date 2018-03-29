/*
 * Copyright (c) 2011,2017-2018 The Linux Foundation. All rights reserved.
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

int
tgt_spectral_control(
	struct wlan_objmgr_pdev *pdev,
	u_int id,
	void *indata,
	u_int32_t insize, void *outdata, u_int32_t *outsize)
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
	return spectral_control_cmn(pdev, id, indata, insize, outdata, outsize);
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

int
tgt_set_spectral_config(
	struct wlan_objmgr_pdev *pdev,
	const u_int32_t threshtype, const u_int32_t value)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_spectral_config(
		pdev, threshtype, value);
}

void
tgt_get_spectral_config(
	struct wlan_objmgr_pdev *pdev,
	struct spectral_config *sptrl_config)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_config(
			pdev,
			sptrl_config);
}

int
tgt_start_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_start_spectral_scan(
		pdev);
}

void
tgt_stop_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_stop_spectral_scan(pdev);
}

bool
tgt_is_spectral_active(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_active(
		pdev);
}

bool
tgt_is_spectral_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_enabled(
		pdev);
}

int
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

void
tgt_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_capinfo(
		pdev, outdata);
}

void
tgt_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_diagstats(
		pdev, outdata);
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

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_tx_ops = &psoc->soc_cb.tx_ops.dbr_tx_ops;

	if (tgt_spectral_get_target_type(psoc) == TARGET_TYPE_QCA8074)
		if (dbr_tx_ops->direct_buf_rx_module_register)
			return dbr_tx_ops->direct_buf_rx_module_register
				(pdev, 0,
				 spectral_dbr_event_handler);

	return QDF_STATUS_E_FAILURE;
}
#else
QDF_STATUS
tgt_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
