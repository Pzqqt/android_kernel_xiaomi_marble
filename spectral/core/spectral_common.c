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

#include "spectral_cmn_api_i.h"
#include "spectral_da_api_i.h"
#include "spectral_ol_api_i.h"
#include <qdf_mem.h>
#include <qdf_types.h>
#ifdef CONFIG_WIN
#include <osif_private.h>
#include <wlan_mlme_dispatcher.h>
#endif /*CONFIG_WIN*/
#include <wlan_spectral_public_structs.h>
#include <wlan_cfg80211_spectral.h>

/**
 * spectral_get_vdev() - Get pointer to vdev to be used for Spectral
 * operations
 * @pdev: Pointer to pdev
 *
 * Spectral operates on pdev. However, in order to retrieve some WLAN
 * properties, a vdev is required. To facilitate this, the function returns the
 * first vdev in our pdev. The caller should release the reference to the vdev
 * once it is done using it. Additionally, the caller should ensure it has a
 * reference to the pdev at the time of calling this function, and should
 * release the pdev reference either after this function returns or at a later
 * time when the caller is done using pdev.
 * TODO:
 *  - If the framework later provides an API to obtain the first active
 *    vdev, then it would be preferable to use this API.
 *  - Use a common get_vdev() handler for core and target_if using Rx ops. This
 *    is deferred till details emerge on framework providing API to get first
 *    active vdev.
 *
 * Return: Pointer to vdev on success, NULL on failure
 */
static struct wlan_objmgr_vdev*
spectral_get_vdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(pdev);

	vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_SPECTRAL_ID);

	if (!vdev) {
		spectral_warn("Unable to get first vdev of pdev");
		return NULL;
	}

	return vdev;
}

#ifndef CONFIG_MCL
/**
 * spectral_register_cfg80211_handlers() - Register spectral cfg80211 handlers
 * @pdev: Pointer to pdev
 *
 * Register spectral cfg80211 handlers
 * Handlers can be different for WIN and MCL
 *
 * Return: None
 */
static void
spectral_register_cfg80211_handlers(struct wlan_objmgr_pdev *pdev)
{
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_START_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_config_and_start);
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_STOP_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_stop);
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_GET_CONFIG_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_get_config);
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_GET_DIAG_STATS_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_get_diag_stats);
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_GET_CAP_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_get_cap);
	wlan_cfg80211_register_spectral_cmd_handler(
		pdev,
		SPECTRAL_SCAN_GET_STATUS_HANDLER_IDX,
		wlan_cfg80211_spectral_scan_get_status);
}
#else
static void
spectral_register_cfg80211_handlers(struct wlan_objmgr_pdev *pdev)
{
}
#endif

int
spectral_control_cmn(struct wlan_objmgr_pdev *pdev,
		     u_int id,
		     void *indata,
		     uint32_t insize, void *outdata, uint32_t *outsize)
{
	int error = 0;
	int temp_debug;
	struct spectral_config sp_out;
	struct spectral_config *sp_in;
	struct spectral_config *spectralparams;
	struct spectral_context *sc;
	struct wlan_objmgr_vdev *vdev = NULL;
	uint8_t vdev_rxchainmask = 0;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		error = -EINVAL;
		goto bad;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("atf context is NULL!");
		error = -EINVAL;
		goto bad;
	}

	switch (id) {
	case SPECTRAL_SET_CONFIG:
		{
			if (insize < sizeof(struct spectral_config) ||
			    !indata) {
				error = -EINVAL;
				break;
			}
			sp_in = (struct spectral_config *)indata;
			if (sp_in->ss_count !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_SCAN_COUNT,
					sp_in->ss_count))
					error = -EINVAL;
			}

			if (sp_in->ss_fft_period !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_FFT_PERIOD,
					sp_in->ss_fft_period))
					error = -EINVAL;
			}

			if (sp_in->ss_period != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_SCAN_PERIOD,
					sp_in->ss_period))
					error = -EINVAL;
			}

			if (sp_in->ss_short_report !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_SHORT_REPORT,
					(uint32_t)
					(sp_in->ss_short_report ? 1 : 0)))
					error = -EINVAL;
			}

			if (sp_in->ss_spectral_pri !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_SPECT_PRI,
					(uint32_t)(sp_in->ss_spectral_pri)))
					error = -EINVAL;
			}

			if (sp_in->ss_fft_size != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_FFT_SIZE,
					sp_in->ss_fft_size))
					error = -EINVAL;
			}

			if (sp_in->ss_gc_ena != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_GC_ENA,
					sp_in->ss_gc_ena))
					error = -EINVAL;
			}

			if (sp_in->ss_restart_ena !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_RESTART_ENA,
					sp_in->ss_restart_ena))
					error = -EINVAL;
			}

			if (sp_in->ss_noise_floor_ref !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_NOISE_FLOOR_REF,
					sp_in->ss_noise_floor_ref))
					error = -EINVAL;
			}

			if (sp_in->ss_init_delay !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_INIT_DELAY,
					sp_in->ss_init_delay))
					error = -EINVAL;
			}

			if (sp_in->ss_nb_tone_thr !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_NB_TONE_THR,
					sp_in->ss_nb_tone_thr))
					error = -EINVAL;
			}

			if (sp_in->ss_str_bin_thr !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_STR_BIN_THR,
					sp_in->ss_str_bin_thr))
					error = -EINVAL;
			}

			if (sp_in->ss_wb_rpt_mode !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_WB_RPT_MODE,
					sp_in->ss_wb_rpt_mode))
					error = -EINVAL;
			}

			if (sp_in->ss_rssi_rpt_mode !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_RSSI_RPT_MODE,
					sp_in->ss_rssi_rpt_mode))
					error = -EINVAL;
			}

			if (sp_in->ss_rssi_thr != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_RSSI_THR,
					sp_in->ss_rssi_thr))
					error = -EINVAL;
			}

			if (sp_in->ss_pwr_format !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_PWR_FORMAT,
					sp_in->ss_pwr_format))
					error = -EINVAL;
			}

			if (sp_in->ss_rpt_mode != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_RPT_MODE,
					sp_in->ss_rpt_mode))
					error = -EINVAL;
			}

			if (sp_in->ss_bin_scale !=
			    SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_BIN_SCALE,
					sp_in->ss_bin_scale))
					error = -EINVAL;
			}

			if (sp_in->ss_dbm_adj != SPECTRAL_PHYERR_PARAM_NOVAL) {
				if (sc->sptrlc_set_spectral_config(
					pdev,
					SPECTRAL_PARAM_DBM_ADJ,
					sp_in->ss_dbm_adj))
					error = -EINVAL;
			}

			if (sp_in->ss_chn_mask != SPECTRAL_PHYERR_PARAM_NOVAL) {
				/*
				 * Check if any of the inactive Rx antenna
				 * chains is set active in spectral chainmask
				 */
				vdev = spectral_get_vdev(pdev);
				if (!vdev) {
					error = -ENOENT;
					break;
				}

				vdev_rxchainmask =
				    wlan_vdev_mlme_get_rxchainmask(vdev);
				wlan_objmgr_vdev_release_ref(vdev,
							     WLAN_SPECTRAL_ID);

				if (!(sp_in->ss_chn_mask & vdev_rxchainmask)) {
					spectral_err("Invalid Spectral Chainmask - Inactive Rx antenna chain cannot be an active spectral chain");
					error = -EINVAL;
					break;
				} else if (sc->sptrlc_set_spectral_config(
						pdev,
						SPECTRAL_PARAM_CHN_MASK,
						sp_in->ss_chn_mask)) {
					error = -EINVAL;
				}
			}
		}
		break;

	case SPECTRAL_GET_CONFIG:
		{
			if (!outdata || !outsize ||
			    (*outsize < sizeof(struct spectral_config))) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(struct spectral_config);
			sc->sptrlc_get_spectral_config(pdev, &sp_out);
			spectralparams = (struct spectral_config *)outdata;
			spectralparams->ss_fft_period = sp_out.ss_fft_period;
			spectralparams->ss_period = sp_out.ss_period;
			spectralparams->ss_count = sp_out.ss_count;
			spectralparams->ss_short_report =
			    sp_out.ss_short_report;
			spectralparams->ss_spectral_pri =
			    sp_out.ss_spectral_pri;
			spectralparams->ss_fft_size = sp_out.ss_fft_size;
			spectralparams->ss_gc_ena = sp_out.ss_gc_ena;
			spectralparams->ss_restart_ena = sp_out.ss_restart_ena;
			spectralparams->ss_noise_floor_ref =
			    sp_out.ss_noise_floor_ref;
			spectralparams->ss_init_delay = sp_out.ss_init_delay;
			spectralparams->ss_nb_tone_thr = sp_out.ss_nb_tone_thr;
			spectralparams->ss_str_bin_thr = sp_out.ss_str_bin_thr;
			spectralparams->ss_wb_rpt_mode = sp_out.ss_wb_rpt_mode;
			spectralparams->ss_rssi_rpt_mode =
			    sp_out.ss_rssi_rpt_mode;
			spectralparams->ss_rssi_thr = sp_out.ss_rssi_thr;
			spectralparams->ss_pwr_format = sp_out.ss_pwr_format;
			spectralparams->ss_rpt_mode = sp_out.ss_rpt_mode;
			spectralparams->ss_bin_scale = sp_out.ss_bin_scale;
			spectralparams->ss_dbm_adj = sp_out.ss_dbm_adj;
			spectralparams->ss_chn_mask = sp_out.ss_chn_mask;
		}
		break;

	case SPECTRAL_IS_ACTIVE:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
			    (uint32_t)sc->sptrlc_is_spectral_active(pdev);
		}
		break;

	case SPECTRAL_IS_ENABLED:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
			    (uint32_t)sc->sptrlc_is_spectral_enabled(pdev);
		}
		break;

	case SPECTRAL_SET_DEBUG_LEVEL:
		{
			if (insize < sizeof(uint32_t) || !indata) {
				error = -EINVAL;
				break;
			}
			temp_debug = *(uint32_t *)indata;
			sc->sptrlc_set_debug_level(pdev, temp_debug);
		}
		break;

	case SPECTRAL_GET_DEBUG_LEVEL:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
			    (uint32_t)sc->sptrlc_get_debug_level(pdev);
		}
		break;

	case SPECTRAL_ACTIVATE_SCAN:
		{
			sc->sptrlc_start_spectral_scan(pdev);
		}
		break;

	case SPECTRAL_STOP_SCAN:
		{
			sc->sptrlc_stop_spectral_scan(pdev);
		}
		break;

	case SPECTRAL_GET_CAPABILITY_INFO:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(struct spectral_caps)) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(struct spectral_caps);
			sc->sptrlc_get_spectral_capinfo(pdev, outdata);
		}
		break;

	case SPECTRAL_GET_DIAG_STATS:
		{
			if (!outdata || !outsize ||
			    (*outsize < sizeof(struct spectral_diag_stats))) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(struct spectral_diag_stats);
			sc->sptrlc_get_spectral_diagstats(pdev, outdata);
		}
		break;

	case SPECTRAL_GET_CHAN_WIDTH:
		{
			uint32_t chan_width;

			vdev = spectral_get_vdev(pdev);
			if (!vdev)
				return -ENOENT;

			chan_width = spectral_vdev_get_ch_width(vdev);
			wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

			if (!outdata || !outsize ||
			    *outsize < sizeof(chan_width)) {
				error = -EINVAL;
				break;
			}
			*outsize = sizeof(chan_width);
			*((uint32_t *)outdata) = (uint32_t)chan_width;
		}
		break;

	default:
		error = -EINVAL;
		break;
	}

 bad:
	return error;
}

/**
 * spectral_ctx_deinit() - De-initialize function pointers from spectral context
 * @sc - Reference to spectral_context object
 *
 * Return: None
 */
static void
spectral_ctx_deinit(struct spectral_context *sc)
{
	if (sc) {
		sc->sptrlc_ucfg_phyerr_config = NULL;
		sc->sptrlc_pdev_spectral_init = NULL;
		sc->sptrlc_pdev_spectral_deinit = NULL;
		sc->sptrlc_set_spectral_config = NULL;
		sc->sptrlc_get_spectral_config = NULL;
		sc->sptrlc_start_spectral_scan = NULL;
		sc->sptrlc_stop_spectral_scan = NULL;
		sc->sptrlc_is_spectral_active = NULL;
		sc->sptrlc_is_spectral_enabled = NULL;
		sc->sptrlc_set_debug_level = NULL;
		sc->sptrlc_get_debug_level = NULL;
		sc->sptrlc_get_spectral_capinfo = NULL;
		sc->sptrlc_get_spectral_diagstats = NULL;
	}
}

QDF_STATUS
wlan_spectral_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct spectral_context *sc = NULL;

	if (!psoc) {
		spectral_err("PSOC is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	sc = (struct spectral_context *)
	    qdf_mem_malloc(sizeof(struct spectral_context));
	if (!sc) {
		spectral_err("Failed to allocate spectral_ctx object");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(sc, sizeof(struct spectral_context));
	sc->psoc_obj = psoc;
	if (wlan_objmgr_psoc_get_dev_type(psoc) == WLAN_DEV_OL)
		spectral_ctx_init_ol(sc);
#ifdef CONFIG_WIN
	else if (wlan_objmgr_psoc_get_dev_type(psoc) == WLAN_DEV_DA)
		spectral_ctx_init_da(sc);
#endif
	wlan_objmgr_psoc_component_obj_attach(psoc, WLAN_UMAC_COMP_SPECTRAL,
					      (void *)sc, QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_spectral_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc,
				       void *arg)
{
	struct spectral_context *sc = NULL;

	if (!psoc) {
		spectral_err("PSOC is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (sc) {
		wlan_objmgr_psoc_component_obj_detach(psoc,
						      WLAN_UMAC_COMP_SPECTRAL,
						      (void *)sc);
		/* Deinitilise function pointers from spectral context */
		spectral_ctx_deinit(sc);
		qdf_mem_free(sc);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_spectral_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct pdev_spectral *ps = NULL;
	struct spectral_context *sc = NULL;
	void *target_handle = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ps = (struct pdev_spectral *)
	    qdf_mem_malloc(sizeof(struct pdev_spectral));
	if (!ps) {
		spectral_err("Failed to allocate pdev_spectral object");
		return QDF_STATUS_E_NOMEM;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("Spectral context is NULL!");
		goto cleanup;
	}

	qdf_mem_zero(ps, sizeof(struct pdev_spectral));
	ps->psptrl_pdev = pdev;

	spectral_register_cfg80211_handlers(pdev);
	if (sc->sptrlc_pdev_spectral_init) {
		target_handle = sc->sptrlc_pdev_spectral_init(pdev);
		if (!target_handle) {
			spectral_err("Spectral lmac object is NULL!");
			goto cleanup;
		}
		ps->psptrl_target_handle = target_handle;
	}
	wlan_objmgr_pdev_component_obj_attach(pdev, WLAN_UMAC_COMP_SPECTRAL,
					      (void *)ps, QDF_STATUS_SUCCESS);

	return QDF_STATUS_SUCCESS;
 cleanup:
	qdf_mem_free(ps);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wlan_spectral_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev,
				       void *arg)
{
	struct pdev_spectral *ps = NULL;
	struct spectral_context *sc = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("Spectral context is NULL!");
		return QDF_STATUS_E_FAILURE;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (ps) {
		if (sc->sptrlc_pdev_spectral_deinit)
			sc->sptrlc_pdev_spectral_deinit(pdev);
		ps->psptrl_target_handle = NULL;
		wlan_objmgr_pdev_component_obj_detach(pdev,
						      WLAN_UMAC_COMP_SPECTRAL,
						      (void *)ps);
		qdf_mem_free(ps);
	}

	return QDF_STATUS_SUCCESS;
}
