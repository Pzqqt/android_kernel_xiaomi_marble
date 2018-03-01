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

#include <wlan_spectral_utils_api.h>
#include <qdf_module.h>
#include "../../core/spectral_cmn_api_i.h"
#include <wlan_spectral_tgt_api.h>
#include <linux/export.h>

QDF_STATUS
wlan_spectral_init(void)
{
	if (wlan_objmgr_register_psoc_create_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_psoc_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_psoc_destroy_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_psoc_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_create_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_destroy_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_spectral_deinit(void)
{
	if (wlan_objmgr_unregister_psoc_create_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_psoc_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_psoc_destroy_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_psoc_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_create_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_pdev_obj_create_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_UMAC_COMP_SPECTRAL,
		wlan_spectral_pdev_obj_destroy_handler,
		NULL) !=
	    QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
spectral_register_legacy_cb(struct wlan_objmgr_psoc *psoc,
			    struct spectral_legacy_cbacks *legacy_cbacks)
{
	struct spectral_context *sc;

	sc = spectral_get_spectral_ctx_from_psoc(psoc);
	if (!sc) {
		spectral_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	sc->legacy_cbacks.vdev_get_chan_freq =
	    legacy_cbacks->vdev_get_chan_freq;
	sc->legacy_cbacks.vdev_get_ch_width = legacy_cbacks->vdev_get_ch_width;
	sc->legacy_cbacks.vdev_get_sec20chan_freq_mhz =
	    legacy_cbacks->vdev_get_sec20chan_freq_mhz;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(spectral_register_legacy_cb);

int16_t
spectral_vdev_get_chan_freq(struct wlan_objmgr_vdev *vdev)
{
	struct spectral_context *sc;

	sc = spectral_get_spectral_ctx_from_vdev(vdev);
	if (!sc) {
		spectral_err("spectral context is Null");
		return -EINVAL;
	}

	return sc->legacy_cbacks.vdev_get_chan_freq(vdev);
}

enum phy_ch_width
spectral_vdev_get_ch_width(struct wlan_objmgr_vdev *vdev)
{
	struct spectral_context *sc;

	sc = spectral_get_spectral_ctx_from_vdev(vdev);
	if (!sc) {
		spectral_err("spectral context is Null");
		return CH_WIDTH_INVALID;
	}

	return sc->legacy_cbacks.vdev_get_ch_width(vdev);
}

int
spectral_vdev_get_sec20chan_freq_mhz(struct wlan_objmgr_vdev *vdev,
				     uint16_t *sec20chan_freq)
{
	struct spectral_context *sc;

	sc = spectral_get_spectral_ctx_from_vdev(vdev);
	if (!sc) {
		spectral_err("spectral context is Null");
		return -EINVAL;
	}

	return sc->legacy_cbacks.vdev_get_sec20chan_freq_mhz(vdev,
							     sec20chan_freq);
}

void
wlan_lmac_if_sptrl_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_sptrl_rx_ops *sptrl_rx_ops = &rx_ops->sptrl_rx_ops;

	/* Spectral rx ops */
	sptrl_rx_ops->sptrlro_get_target_handle = tgt_get_target_handle;
	sptrl_rx_ops->sptrlro_vdev_get_chan_freq = spectral_vdev_get_chan_freq;
	sptrl_rx_ops->sptrlro_vdev_get_ch_width = spectral_vdev_get_ch_width;
	sptrl_rx_ops->sptrlro_vdev_get_sec20chan_freq_mhz =
	    spectral_vdev_get_sec20chan_freq_mhz;
}

void
wlan_register_wmi_spectral_cmd_ops(struct wlan_objmgr_pdev *pdev,
				   struct wmi_spectral_cmd_ops *cmd_ops)
{
	struct spectral_context *sc;

	if (!pdev)
		spectral_err("PDEV is NULL!");

	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return;
	}

	return sc->sptrlc_register_wmi_spectral_cmd_ops(pdev, cmd_ops);
}
EXPORT_SYMBOL(wlan_register_wmi_spectral_cmd_ops);

QDF_STATUS spectral_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	psoc = wlan_pdev_get_psoc(pdev);

	/* Enable the registartion once FW supports this */
	spectral_debug("Enable registration to direct dma once FW supports it");

	return status;
}
