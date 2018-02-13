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
