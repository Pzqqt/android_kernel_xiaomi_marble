/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
#include <cfg_ucfg_api.h>

bool wlan_spectral_is_mode_disabled_pdev(struct wlan_objmgr_pdev *pdev,
					 enum spectral_scan_mode smode)
{
	bool spectral_mode_disable;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return true;
	}

	switch (smode) {
	case SPECTRAL_SCAN_MODE_NORMAL:
		spectral_mode_disable = wlan_pdev_nif_feat_ext_cap_get(
			pdev, WLAN_PDEV_FEXT_NORMAL_SPECTRAL_SCAN_DIS);
		break;

	case SPECTRAL_SCAN_MODE_AGILE:
		spectral_mode_disable = wlan_pdev_nif_feat_ext_cap_get(
			pdev, WLAN_PDEV_FEXT_AGILE_SPECTRAL_SCAN_DIS) &&
					wlan_pdev_nif_feat_ext_cap_get(
			pdev, WLAN_PDEV_FEXT_AGILE_SPECTRAL_SCAN_160_DIS) &&
					wlan_pdev_nif_feat_ext_cap_get(
			pdev, WLAN_PDEV_FEXT_AGILE_SPECTRAL_SCAN_80P80_DIS) &&
					wlan_pdev_nif_feat_ext_cap_get(
			pdev, WLAN_PDEV_FEXT_AGILE_SPECTRAL_SCAN_320_DIS);
		break;

	default:
		spectral_err("Invalid Spectral scan mode %d", smode);
		spectral_mode_disable = true;
		break;
	}

	return spectral_mode_disable;
}

bool
wlan_spectral_is_feature_disabled_ini(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("PSOC is NULL!");
		return true;
	}

	return wlan_psoc_nif_feat_cap_get(psoc,
					  WLAN_SOC_F_SPECTRAL_INI_DISABLE);
}

bool
wlan_spectral_is_feature_disabled_psoc(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("psoc is NULL!");
		return true;
	}

	return wlan_spectral_is_feature_disabled_ini(psoc);
}

bool
wlan_spectral_is_feature_disabled_pdev(struct wlan_objmgr_pdev *pdev)
{
	enum spectral_scan_mode smode;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return true;
	}

	smode = SPECTRAL_SCAN_MODE_NORMAL;
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
		if (!wlan_spectral_is_mode_disabled_pdev(pdev, smode))
			return false;

	return true;
}

QDF_STATUS
wlan_spectral_init_pdev_feature_caps(struct wlan_objmgr_pdev *pdev)
{
	return tgt_spectral_init_pdev_feature_caps(pdev);
}

QDF_STATUS
wlan_spectral_init_psoc_feature_cap(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (cfg_get(psoc, CFG_SPECTRAL_DISABLE))
		wlan_psoc_nif_feat_cap_set(psoc,
					   WLAN_SOC_F_SPECTRAL_INI_DISABLE);
	else
		wlan_psoc_nif_feat_cap_clear(psoc,
					     WLAN_SOC_F_SPECTRAL_INI_DISABLE);

	return QDF_STATUS_SUCCESS;
}

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

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	sc = spectral_get_spectral_ctx_from_psoc(psoc);
	if (!sc) {
		spectral_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	sc->legacy_cbacks.vdev_get_chan_freq =
	    legacy_cbacks->vdev_get_chan_freq;
	sc->legacy_cbacks.vdev_get_chan_freq_seg2 =
	    legacy_cbacks->vdev_get_chan_freq_seg2;
	sc->legacy_cbacks.vdev_get_ch_width = legacy_cbacks->vdev_get_ch_width;
	sc->legacy_cbacks.vdev_get_sec20chan_freq_mhz =
	    legacy_cbacks->vdev_get_sec20chan_freq_mhz;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(spectral_register_legacy_cb);

int16_t
spectral_vdev_get_chan_freq(struct wlan_objmgr_vdev *vdev)
{
	struct spectral_context *sc;

	sc = spectral_get_spectral_ctx_from_vdev(vdev);
	if (!sc) {
		spectral_err("spectral context is Null");
		return -EINVAL;
	}

	if (!sc->legacy_cbacks.vdev_get_chan_freq) {
		spectral_err("vdev_get_chan_freq is not supported");
		return -ENOTSUPP;
	}

	return sc->legacy_cbacks.vdev_get_chan_freq(vdev);
}

int16_t
spectral_vdev_get_chan_freq_seg2(struct wlan_objmgr_vdev *vdev)
{
	struct spectral_context *sc;
	struct wlan_channel *des_chan;

	sc = spectral_get_spectral_ctx_from_vdev(vdev);
	if (!sc) {
		spectral_err("spectral context is null");
		return -EINVAL;
	}

	if (!sc->legacy_cbacks.vdev_get_chan_freq_seg2) {
		des_chan = wlan_vdev_mlme_get_des_chan(vdev);
		if (des_chan->ch_width == CH_WIDTH_80P80MHZ)
			return des_chan->ch_freq_seg2;
		else
			return 0;
	}

	return sc->legacy_cbacks.vdev_get_chan_freq_seg2(vdev);
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

	if (!sc->legacy_cbacks.vdev_get_ch_width) {
		spectral_err("vdev_get_ch_width is not supported");
		return -ENOTSUPP;
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

	if (!sc->legacy_cbacks.vdev_get_sec20chan_freq_mhz) {
		spectral_err("vdev_get_sec20chan_freq_mhz is not supported");
		return -ENOTSUPP;
	}

	return sc->legacy_cbacks.vdev_get_sec20chan_freq_mhz(vdev,
							     sec20chan_freq);
}

void
wlan_lmac_if_sptrl_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_sptrl_rx_ops *sptrl_rx_ops = &rx_ops->sptrl_rx_ops;

	/* Spectral rx ops */
	sptrl_rx_ops->sptrlro_get_pdev_target_handle =
					tgt_get_pdev_target_handle;
	sptrl_rx_ops->sptrlro_get_psoc_target_handle =
					tgt_get_psoc_target_handle;
	sptrl_rx_ops->sptrlro_vdev_get_chan_freq = spectral_vdev_get_chan_freq;
	sptrl_rx_ops->sptrlro_vdev_get_chan_freq_seg2 =
					spectral_vdev_get_chan_freq_seg2;
	sptrl_rx_ops->sptrlro_vdev_get_ch_width = spectral_vdev_get_ch_width;
	sptrl_rx_ops->sptrlro_vdev_get_sec20chan_freq_mhz =
	    spectral_vdev_get_sec20chan_freq_mhz;
	sptrl_rx_ops->sptrlro_spectral_is_feature_disabled_pdev =
		wlan_spectral_is_feature_disabled_pdev;
	sptrl_rx_ops->sptrlro_spectral_is_feature_disabled_psoc =
		wlan_spectral_is_feature_disabled_psoc;
}

QDF_STATUS
wlan_register_spectral_wmi_ops(struct wlan_objmgr_psoc *psoc,
			       struct spectral_wmi_ops *wmi_ops)
{
	struct spectral_context *sc;

	if (!psoc) {
		spectral_err("psoc is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	sc = spectral_get_spectral_ctx_from_psoc(psoc);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	return sc->sptrlc_register_spectral_wmi_ops(psoc, wmi_ops);
}

qdf_export_symbol(wlan_register_spectral_wmi_ops);

QDF_STATUS
wlan_register_spectral_tgt_ops(struct wlan_objmgr_psoc *psoc,
			       struct spectral_tgt_ops *tgt_ops)
{
	struct spectral_context *sc;

	if (!psoc) {
		spectral_err("psoc is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	sc = spectral_get_spectral_ctx_from_psoc(psoc);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	return sc->sptrlc_register_spectral_tgt_ops(psoc, tgt_ops);
}

qdf_export_symbol(wlan_register_spectral_tgt_ops);

/**
 * wlan_spectral_psoc_target_attach() - Spectral psoc target attach
 * @psoc:  pointer to psoc object
 *
 * API to initialize Spectral psoc target object
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
static QDF_STATUS
wlan_spectral_psoc_target_attach(struct wlan_objmgr_psoc *psoc)
{
	struct spectral_context *sc = NULL;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!sc) {
		spectral_err("Spectral context is null");
		return QDF_STATUS_E_NOMEM;
	}

	if (sc->sptrlc_psoc_spectral_init) {
		void *target_handle;

		target_handle = sc->sptrlc_psoc_spectral_init(psoc);
		if (!target_handle) {
			spectral_err("Spectral psoc lmac object is NULL!");
			return QDF_STATUS_E_FAILURE;
		}
		sc->psoc_target_handle = target_handle;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_spectral_psoc_target_detach() - Spectral psoc target detach
 * @psoc:  pointer to psoc object
 *
 * API to destroy Spectral psoc target object
 *
 * Return: QDF_STATUS_SUCCESS upon successful registration,
 *         QDF_STATUS_E_FAILURE upon failure
 */
static QDF_STATUS
wlan_spectral_psoc_target_detach(struct wlan_objmgr_psoc *psoc)
{
	struct spectral_context *sc = NULL;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!sc) {
		spectral_err("Spectral context is null");
		return QDF_STATUS_E_INVAL;
	}

	if (sc->sptrlc_psoc_spectral_deinit)
		sc->sptrlc_psoc_spectral_deinit(psoc);
	sc->psoc_target_handle = NULL;

	return QDF_STATUS_SUCCESS;
}

#ifdef DIRECT_BUF_RX_ENABLE
bool spectral_dbr_event_handler(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_data *payload)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return -EINVAL;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return -EINVAL;
	}

	sc->sptrlc_process_spectral_report(pdev, payload);

	return true;
}
#endif

QDF_STATUS spectral_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	psoc = wlan_pdev_get_psoc(pdev);

	if (wlan_spectral_is_feature_disabled_pdev(pdev)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	if (cfg_get(psoc, CFG_SPECTRAL_POISON_BUFS))
		tgt_set_spectral_dma_debug(pdev, SPECTRAL_DMA_BUFFER_DEBUG, 1);

	status = spectral_register_dbr(pdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS spectral_register_dbr(struct wlan_objmgr_pdev *pdev)
{
	if (wlan_spectral_is_feature_disabled_pdev(pdev)) {
		spectral_info("spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	return tgt_spectral_register_to_dbr(pdev);
}

qdf_export_symbol(spectral_register_dbr);

QDF_STATUS spectral_unregister_dbr(struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status;

	if (wlan_spectral_is_feature_disabled_pdev(pdev)) {
		spectral_info("spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}
	status = tgt_spectral_unregister_to_dbr(pdev);

	return status;
}

qdf_export_symbol(spectral_unregister_dbr);

QDF_STATUS wlan_spectral_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	return wlan_spectral_psoc_target_attach(psoc);
}

QDF_STATUS wlan_spectral_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	return wlan_spectral_psoc_target_detach(psoc);
}

QDF_STATUS wlan_spectral_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	return tgt_spectral_register_events(psoc);
}

QDF_STATUS wlan_spectral_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (wlan_spectral_is_feature_disabled_psoc(psoc)) {
		spectral_info("Spectral feature is disabled");
		return QDF_STATUS_COMP_DISABLED;
	}

	return tgt_spectral_unregister_events(psoc);
}

struct wlan_lmac_if_sptrl_tx_ops *
wlan_spectral_pdev_get_lmac_if_txops(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return NULL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is NULL!");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->sptrl_tx_ops;
}
