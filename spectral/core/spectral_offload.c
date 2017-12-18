/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

/**
 * spectral_control_ol()- handler for demultiplexing requests from higher layer
 * @pdev:    reference to global pdev object
 * @id:      spectral config command id
 * @indata:  reference to input data
 * @insize:  input data size
 * @outdata: reference to output data
 * @outsize: output data size
 *
 * This function processes the spectral config command
 * and appropriate handlers are invoked.
 *
 * Return: 0 success else failure
 */
int
spectral_control_ol(struct wlan_objmgr_pdev *pdev,
		    u_int id,
		    void *indata,
		    uint32_t insize, void *outdata, uint32_t *outsize)
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

/**
 * pdev_spectral_init_ol() - offload implementation for spectral init
 * @pdev: Pointer to pdev
 *
 * Return: On success, pointer to Spectral target_if internal private data, on
 * failure, NULL
 */
static void *
pdev_spectral_init_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_init(
		pdev);
}

/**
 * pdev_spectral_deinit_ol() - offload implementation for spectral de-init
 * @pdev: Pointer to pdev
 *
 * Return: None
 */
static void
pdev_spectral_deinit_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_pdev_spectral_deinit(pdev);
}

/**
 * set_spectral_config_ol() - Set spectral config
 * @pdev:       Pointer to pdev object
 * @threshtype: spectral parameter type
 * @value:      value to be configured for the given spectral parameter
 *
 * Offload implementation for setting spectral config
 *
 * Return: 0 on success else failure
 */
static int
set_spectral_config_ol(struct wlan_objmgr_pdev *pdev,
		       const uint32_t threshtype, const uint32_t value)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_spectral_config(
		pdev, threshtype, value);
}

/**
 * get_spectral_config_ol() - Get spectral configuration
 * @pdev: Pointer to pdev object
 * @param: Pointer to spectral_config structure in which the configuration
 * should be returned
 *
 * Offload implementation for getting the current spectral configuration
 *
 * Return: None
 */
static void
get_spectral_config_ol(struct wlan_objmgr_pdev *pdev,
		       struct spectral_config *sptrl_config)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_config(
			pdev,
			sptrl_config);
}

/**
 * start_spectral_scan_ol() - Start spectral scan
 * @pdev: Pointer to pdev object
 *
 * Offload implementation for starting spectral scan
 *
 * Return: 0 in case of success, -1 on failure
 */
static int
start_spectral_scan_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_start_spectral_scan(
		pdev);
}

/**
 * stop_spectral_scan_ol() - Stop spectral scan
 * @pdev: Pointer to pdev object
 *
 * Offload implementation for stop spectral scan
 *
 * Return: None
 */
static void
stop_spectral_scan_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_stop_spectral_scan(pdev);
}

/**
 * is_spectral_active_ol() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 *
 * Offload implementation to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
static bool
is_spectral_active_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_active(
		pdev);
}

/**
 * is_spectral_enabled_ol() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 *
 * Offload implementation to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
static bool
is_spectral_enabled_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_is_spectral_enabled(
		pdev);
}

/**
 * set_debug_level_ol() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * Offload implementation to set the debug level for Spectral
 *
 * Return: 0 in case of success
 */
static int
set_debug_level_ol(struct wlan_objmgr_pdev *pdev, uint32_t debug_level)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_set_debug_level(
			pdev,
			debug_level);
}

/**
 * get_debug_level_ol() - Get debug level for Spectral
 * @pdev: Pointer to pdev object
 *
 * Offload implementation to get the debug level for Spectral
 *
 * Return: Current debug level
 */
static uint32_t
get_debug_level_ol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_debug_level(pdev);
}

/**
 * get_spectral_capinfo_ol() - Get Spectral capability information
 * @pdev: Pointer to pdev object
 * @outdata: Buffer into which data should be copied
 *
 * Offload implementation to get the spectral capability information
 *
 * Return: void
 */
static void
get_spectral_capinfo_ol(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_capinfo(
		pdev, outdata);
}

/**
 * get_spectral_diagstats_ol() - Get Spectral diagnostic statistics
 * @pdev:  Pointer to pdev object
 * @outdata: Buffer into which data should be copied
 *
 * Offload implementation to get the spectral diagnostic statistics
 *
 * Return: void
 */
static void
get_spectral_diagstats_ol(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.sptrl_tx_ops.sptrlto_get_spectral_diagstats(
		pdev, outdata);
}

/**
 * register_wmi_spectral_cmd_ops_ol() - Register wmi_spectral_cmd_ops
 * @cmd_ops: Pointer to the structure having wmi_spectral_cmd function pointers
 * @pdev: Pointer to pdev object
 *
 * Offload implementation to register wmi_spectral_cmd_ops in spectral
 * internal data structure
 *
 * Return: void
 */
static void
register_wmi_spectral_cmd_ops_ol(struct wlan_objmgr_pdev *pdev,
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
spectral_ctx_init_ol(struct spectral_context *sc)
{
	if (!sc) {
		spectral_err("spectral context is null!");
		return;
	}
	sc->sptrlc_spectral_control = spectral_control_ol;
	sc->sptrlc_pdev_spectral_init = pdev_spectral_init_ol;
	sc->sptrlc_pdev_spectral_deinit = pdev_spectral_deinit_ol;
	sc->sptrlc_set_spectral_config = set_spectral_config_ol;
	sc->sptrlc_get_spectral_config = get_spectral_config_ol;
	sc->sptrlc_start_spectral_scan = start_spectral_scan_ol;
	sc->sptrlc_stop_spectral_scan = stop_spectral_scan_ol;
	sc->sptrlc_is_spectral_active = is_spectral_active_ol;
	sc->sptrlc_is_spectral_enabled = is_spectral_enabled_ol;
	sc->sptrlc_set_debug_level = set_debug_level_ol;
	sc->sptrlc_get_debug_level = get_debug_level_ol;
	sc->sptrlc_get_spectral_capinfo = get_spectral_capinfo_ol;
	sc->sptrlc_get_spectral_diagstats = get_spectral_diagstats_ol;
	sc->sptrlc_register_wmi_spectral_cmd_ops =
	    register_wmi_spectral_cmd_ops_ol;
}
