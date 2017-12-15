/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#include "spectral_cmn_api_i.h"
#include "spectral_ol_api_i.h"

int spectral_control_ol(
	struct wlan_objmgr_pdev *pdev,
	 u_int id,
	 void *indata,
	 u_int32_t insize,
			void *outdata, u_int32_t *outsize)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!\n");
		return -EPERM;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!\n");
		return -EPERM;
	}
	return spectral_control_cmn(pdev, id, indata, insize, outdata, outsize);
}

static void *pdev_spectral_init_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_init(
		pdev);
}

static void pdev_spectral_deinit_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_deinit(pdev);
}

static int set_spectral_config_ol(
	struct wlan_objmgr_pdev *pdev,
	const u_int32_t threshtype, const u_int32_t value)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_spectral_config(
		pdev,
			threshtype, value);
}

static void get_spectral_config_ol(struct wlan_objmgr_pdev *pdev,
				   struct spectral_config *sptrl_config)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_config(pdev,
			sptrl_config);
}

static int start_spectral_scan_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_start_spectral_scan(
		pdev);
}

static void stop_spectral_scan_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_stop_spectral_scan(pdev);
}

static bool is_spectral_active_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_active(
		pdev);
}

static bool is_spectral_enabled_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_enabled(
		pdev);
}

static int  set_debug_level_ol(struct wlan_objmgr_pdev *pdev,
			       u_int32_t debug_level)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_debug_level(pdev,
			debug_level);
}

static u_int32_t get_debug_level_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_debug_level(pdev);
}

static void get_spectral_capinfo_ol(struct wlan_objmgr_pdev *pdev,
				    void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_capinfo(
		pdev,
		 outdata);
}

static void get_spectral_diagstats_ol(struct wlan_objmgr_pdev *pdev,
				      void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_diagstats(
		pdev,
		 outdata);
}

static void register_wmi_spectral_cmd_ops_ol(
		struct wlan_objmgr_pdev *pdev,
		struct wmi_spectral_cmd_ops *cmd_ops)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_sptrl_tx_ops *psptrl_tx_ops = NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	psptrl_tx_ops = &psoc->soc_cb.tx_ops.sptrl_tx_ops;

	return psptrl_tx_ops->sptrlto_register_wmi_spectral_cmd_ops(
		pdev,
		cmd_ops);
}

void spectral_ctx_init_ol(struct spectral_context *sc)
{
	if (!sc) {
		spectral_err("spectral context is null!\n");
		return;
	}
	sc->sptrlc_spectral_control       = spectral_control_ol;
	sc->sptrlc_pdev_spectral_init     = pdev_spectral_init_ol;
	sc->sptrlc_pdev_spectral_deinit   = pdev_spectral_deinit_ol;
	sc->sptrlc_set_spectral_config    = set_spectral_config_ol;
	sc->sptrlc_get_spectral_config    = get_spectral_config_ol;
	sc->sptrlc_start_spectral_scan    = start_spectral_scan_ol;
	sc->sptrlc_stop_spectral_scan     = stop_spectral_scan_ol;
	sc->sptrlc_is_spectral_active     = is_spectral_active_ol;
	sc->sptrlc_is_spectral_enabled    = is_spectral_enabled_ol;
	sc->sptrlc_set_debug_level        = set_debug_level_ol;
	sc->sptrlc_get_debug_level        = get_debug_level_ol;
	sc->sptrlc_get_spectral_capinfo   = get_spectral_capinfo_ol;
	sc->sptrlc_get_spectral_diagstats = get_spectral_diagstats_ol;
	sc->sptrlc_register_wmi_spectral_cmd_ops =
		register_wmi_spectral_cmd_ops_ol;
}

