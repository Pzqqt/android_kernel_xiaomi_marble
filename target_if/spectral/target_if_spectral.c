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

#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <target_type.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <init_deinit_lmac.h>
#include <reg_services_public_struct.h>
#include <target_if_spectral_sim.h>
#include <target_if.h>
#include <qdf_module.h>
#include <wlan_reg_services_api.h>
#include <wlan_dfs_ucfg_api.h>

/**
 * @spectral_ops - Spectral function table, holds the Spectral functions that
 * depend on whether the architecture is Direct Attach or Offload. This is used
 * to populate the actual Spectral function table present in the Spectral
 * module.
 */
struct target_if_spectral_ops spectral_ops;
int spectral_debug_level = DEBUG_SPECTRAL;
struct spectral_tgt_ops ops_tgt;

static void target_if_spectral_get_firstvdev_pdev(struct wlan_objmgr_pdev *pdev,
						  void *obj, void *arg)
{
	struct wlan_objmgr_vdev *vdev = obj;
	struct wlan_objmgr_vdev **first_vdev = arg;

	if (!(*first_vdev))
		*first_vdev = vdev;
}

struct wlan_objmgr_vdev *
target_if_spectral_get_vdev(struct target_if_spectral *spectral,
			    enum spectral_scan_mode smode)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *first_vdev = NULL;

	qdf_assert_always(spectral);
	pdev = spectral->pdev_obj;
	qdf_assert_always(pdev);

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return NULL;
	}

	if (spectral->vdev_id[smode] != WLAN_INVALID_VDEV_ID) {
		first_vdev = wlan_objmgr_get_vdev_by_id_from_pdev(
						pdev, spectral->vdev_id[smode],
						WLAN_SPECTRAL_ID);
		return first_vdev;
	}

	if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SPECTRAL_ID) !=
	    QDF_STATUS_SUCCESS) {
		spectral_err("Unable to get pdev reference.");
		return NULL;
	}

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  target_if_spectral_get_firstvdev_pdev,
					  &first_vdev, 0, WLAN_SPECTRAL_ID);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_SPECTRAL_ID);

	if (!first_vdev)
		return NULL;

	if (wlan_objmgr_vdev_try_get_ref(first_vdev, WLAN_SPECTRAL_ID) !=
			QDF_STATUS_SUCCESS)
		first_vdev = NULL;

	return first_vdev;
}

/**
 * target_if_send_vdev_spectral_configure_cmd() - Send WMI command to configure
 * spectral parameters
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @param: Pointer to spectral_config giving the Spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, negative error code on failure
 */
static int
target_if_send_vdev_spectral_configure_cmd(struct target_if_spectral *spectral,
					   enum spectral_scan_mode smode,
					   struct spectral_config *param)
{
	struct vdev_spectral_configure_params sparam;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct target_if_psoc_spectral *psoc_spectral;

	qdf_assert_always(spectral && param);

	pdev = spectral->pdev_obj;

	qdf_assert_always(pdev);

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("psoc spectral object is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	vdev = target_if_spectral_get_vdev(spectral, smode);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	qdf_mem_zero(&sparam, sizeof(sparam));

	sparam.vdev_id = wlan_vdev_get_id(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	sparam.count = param->ss_count;
	sparam.period = param->ss_period;
	sparam.spectral_pri = param->ss_spectral_pri;
	sparam.fft_size = param->ss_fft_size;
	sparam.gc_enable = param->ss_gc_ena;
	sparam.restart_enable = param->ss_restart_ena;
	sparam.noise_floor_ref = param->ss_noise_floor_ref;
	sparam.init_delay = param->ss_init_delay;
	sparam.nb_tone_thr = param->ss_nb_tone_thr;
	sparam.str_bin_thr = param->ss_str_bin_thr;
	sparam.wb_rpt_mode = param->ss_wb_rpt_mode;
	sparam.rssi_rpt_mode = param->ss_rssi_rpt_mode;
	sparam.rssi_thr = param->ss_rssi_thr;
	sparam.pwr_format = param->ss_pwr_format;
	sparam.rpt_mode = param->ss_rpt_mode;
	sparam.bin_scale = param->ss_bin_scale;
	sparam.dbm_adj = param->ss_dbm_adj;
	sparam.chn_mask = param->ss_chn_mask;
	sparam.mode = smode;
	sparam.center_freq1 = param->ss_frequency.cfreq1;
	sparam.center_freq2 = param->ss_frequency.cfreq2;
	sparam.chan_width = spectral->ch_width[smode];

	return psoc_spectral->wmi_ops.wmi_spectral_configure_cmd_send(
				GET_WMI_HDL_FROM_PDEV(pdev), &sparam);
}

/**
 * target_if_send_vdev_spectral_enable_cmd() - Send WMI command to
 * enable/disable Spectral
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @is_spectral_active_valid: Flag to indicate if spectral activate (trigger) is
 * valid
 * @is_spectral_active: Value of spectral activate
 * @is_spectral_enabled_valid: Flag to indicate if spectral enable is valid
 * @is_spectral_enabled: Value of spectral enable
 *
 * Return: QDF_STATUS_SUCCESS on success, negative error code on failure
 */
static int
target_if_send_vdev_spectral_enable_cmd(struct target_if_spectral *spectral,
					enum spectral_scan_mode smode,
					uint8_t is_spectral_active_valid,
					uint8_t is_spectral_active,
					uint8_t is_spectral_enabled_valid,
					uint8_t is_spectral_enabled)
{
	struct vdev_spectral_enable_params param;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct target_if_psoc_spectral *psoc_spectral;

	qdf_assert_always(spectral);

	pdev = spectral->pdev_obj;

	qdf_assert_always(pdev);

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("psoc spectral object is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	vdev = target_if_spectral_get_vdev(spectral, smode);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	qdf_mem_zero(&param, sizeof(param));

	param.vdev_id = wlan_vdev_get_id(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	param.active_valid = is_spectral_active_valid;
	param.enabled_valid = is_spectral_enabled_valid;
	param.active = is_spectral_active;
	param.enabled = is_spectral_enabled;
	param.mode = smode;

	return psoc_spectral->wmi_ops.wmi_spectral_enable_cmd_send(
				GET_WMI_HDL_FROM_PDEV(pdev), &param);
}

/**
 * target_if_spectral_info_init_defaults() - Helper function to load defaults
 * for Spectral information (parameters and state) into cache.
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * It is assumed that the caller has obtained the requisite lock if applicable.
 * Note that this is currently treated as a temporary function.  Ideally, we
 * would like to get defaults from the firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS
target_if_spectral_info_init_defaults(struct target_if_spectral *spectral,
				      enum spectral_scan_mode smode)
{
	struct target_if_spectral_param_state_info *info;
	struct wlan_objmgr_vdev *vdev = NULL;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	info = &spectral->param_info[smode];

	/* State */
	info->osps_cache.osc_spectral_active = SPECTRAL_SCAN_ACTIVE_DEFAULT;

	info->osps_cache.osc_spectral_enabled = SPECTRAL_SCAN_ENABLE_DEFAULT;

	/* Parameters */
	info->osps_cache.osc_params.ss_count = SPECTRAL_SCAN_COUNT_DEFAULT;

	if (spectral->spectral_gen == SPECTRAL_GEN3)
		info->osps_cache.osc_params.ss_period =
			SPECTRAL_SCAN_PERIOD_GEN_III_DEFAULT;
	else
		info->osps_cache.osc_params.ss_period =
			SPECTRAL_SCAN_PERIOD_GEN_II_DEFAULT;

	info->osps_cache.osc_params.ss_spectral_pri =
	    SPECTRAL_SCAN_PRIORITY_DEFAULT;

	info->osps_cache.osc_params.ss_fft_size =
	    SPECTRAL_SCAN_FFT_SIZE_DEFAULT;

	info->osps_cache.osc_params.ss_gc_ena = SPECTRAL_SCAN_GC_ENA_DEFAULT;

	info->osps_cache.osc_params.ss_restart_ena =
	    SPECTRAL_SCAN_RESTART_ENA_DEFAULT;

	info->osps_cache.osc_params.ss_noise_floor_ref =
	    SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT;

	info->osps_cache.osc_params.ss_init_delay =
	    SPECTRAL_SCAN_INIT_DELAY_DEFAULT;

	info->osps_cache.osc_params.ss_nb_tone_thr =
	    SPECTRAL_SCAN_NB_TONE_THR_DEFAULT;

	info->osps_cache.osc_params.ss_str_bin_thr =
	    SPECTRAL_SCAN_STR_BIN_THR_DEFAULT;

	info->osps_cache.osc_params.ss_wb_rpt_mode =
	    SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT;

	info->osps_cache.osc_params.ss_rssi_rpt_mode =
	    SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT;

	info->osps_cache.osc_params.ss_rssi_thr =
	    SPECTRAL_SCAN_RSSI_THR_DEFAULT;

	info->osps_cache.osc_params.ss_pwr_format =
	    SPECTRAL_SCAN_PWR_FORMAT_DEFAULT;

	info->osps_cache.osc_params.ss_rpt_mode =
	    SPECTRAL_SCAN_RPT_MODE_DEFAULT;

	info->osps_cache.osc_params.ss_bin_scale =
	    SPECTRAL_SCAN_BIN_SCALE_DEFAULT;

	info->osps_cache.osc_params.ss_dbm_adj = SPECTRAL_SCAN_DBM_ADJ_DEFAULT;

	vdev = target_if_spectral_get_vdev(spectral, smode);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	info->osps_cache.osc_params.ss_chn_mask =
	    wlan_vdev_mlme_get_rxchainmask(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	info->osps_cache.osc_params.ss_short_report =
		SPECTRAL_SCAN_SHORT_REPORT_DEFAULT;

	info->osps_cache.osc_params.ss_fft_period =
		SPECTRAL_SCAN_FFT_PERIOD_DEFAULT;

	info->osps_cache.osc_params.ss_frequency.cfreq1 =
		SPECTRAL_SCAN_FREQUENCY_DEFAULT;
	info->osps_cache.osc_params.ss_frequency.cfreq2 =
		SPECTRAL_SCAN_FREQUENCY_DEFAULT;

	/* The cache is now valid */
	info->osps_cache.osc_is_valid = 1;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_log_read_spectral_active() - Helper function to log whether
 * spectral is active after reading cache
 * @function_name: Function name
 * @output: whether spectral is active or not
 *
 * Helper function to log whether spectral is active after reading cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_active(
	const char *function_name,
	unsigned char output)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ACTIVE. Returning val=%u",
		       function_name, output);
}

/**
 * target_if_log_read_spectral_enabled() - Helper function to log whether
 * spectral is enabled after reading cache
 * @function_name: Function name
 * @output: whether spectral is enabled or not
 *
 * Helper function to log whether spectral is enabled after reading cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_enabled(
	const char *function_name,
	unsigned char output)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ENABLED. Returning val=%u",
		       function_name, output);
}

/**
 * target_if_log_read_spectral_enabled() - Helper function to log spectral
 * parameters after reading cache
 * @function_name: Function name
 * @pparam: Spectral parameters
 *
 * Helper function to log spectral parameters after reading cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_params(
	const char *function_name,
	struct spectral_config *pparam)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_PARAMS. Returning following params:\nss_count = %u\nss_period = %u\nss_spectral_pri = %u\nss_fft_size = %u\nss_gc_ena = %u\nss_restart_ena = %u\nss_noise_floor_ref = %d\nss_init_delay = %u\nss_nb_tone_thr = %u\nss_str_bin_thr = %u\nss_wb_rpt_mode = %u\nss_rssi_rpt_mode = %u\nss_rssi_thr = %d\nss_pwr_format = %u\nss_rpt_mode = %u\nss_bin_scale = %u\nss_dbm_adj = %u\nss_chn_mask = %u\nss_frequency1=%u\nss_frequency2=%u\n",
		       function_name,
		       pparam->ss_count,
		       pparam->ss_period,
		       pparam->ss_spectral_pri,
		       pparam->ss_fft_size,
		       pparam->ss_gc_ena,
		       pparam->ss_restart_ena,
		       (int8_t)pparam->ss_noise_floor_ref,
		       pparam->ss_init_delay,
		       pparam->ss_nb_tone_thr,
		       pparam->ss_str_bin_thr,
		       pparam->ss_wb_rpt_mode,
		       pparam->ss_rssi_rpt_mode,
		       (int8_t)pparam->ss_rssi_thr,
		       pparam->ss_pwr_format,
		       pparam->ss_rpt_mode,
		       pparam->ss_bin_scale,
		       pparam->ss_dbm_adj,
		       pparam->ss_chn_mask,
		       pparam->ss_frequency.cfreq1,
		       pparam->ss_frequency.cfreq2);
}

/**
 * target_if_log_read_spectral_active_catch_validate() - Helper function to
 * log whether spectral is active after intializing the cache
 * @function_name: Function name
 * @output: whether spectral is active or not
 *
 * Helper function to log whether spectral is active after intializing cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_active_catch_validate(
	const char *function_name,
	unsigned char output)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ACTIVE on initial cache validation\nReturning val=%u",
		       function_name, output);
}

/**
 * target_if_log_read_spectral_enabled_catch_validate() - Helper function to
 * log whether spectral is enabled after intializing the cache
 * @function_name: Function name
 * @output: whether spectral is enabled or not
 *
 * Helper function to log whether spectral is enabled after intializing cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_enabled_catch_validate(
	const char *function_name,
	unsigned char output)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ENABLED on initial cache validation\nReturning val=%u\n",
		       function_name, output);
}

/**
 * target_if_log_read_spectral_params_catch_validate() - Helper function to
 * log spectral parameters after intializing the cache
 * @function_name: Function name
 * @pparam: Spectral parameters
 *
 * Helper function to log spectral parameters after intializing the cache
 *
 * Return: none
 */
static void
target_if_log_read_spectral_params_catch_validate(
	const char *function_name,
	struct spectral_config *pparam)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_PARAMS on initial cache validation\nReturning following params:\nss_count = %u\nss_period = %u\nss_spectral_pri = %u\nss_fft_size = %u\nss_gc_ena = %u\nss_restart_ena = %u\nss_noise_floor_ref = %d\nss_init_delay = %u\nss_nb_tone_thr = %u\nss_str_bin_thr = %u\nss_wb_rpt_mode = %u\nss_rssi_rpt_mode = %u\nss_rssi_thr = %d\nss_pwr_format = %u\nss_rpt_mode = %u\nss_bin_scale = %u\nss_dbm_adj = %u\nss_chn_mask = %u",
		       function_name,
		       pparam->ss_count,
		       pparam->ss_period,
		       pparam->ss_spectral_pri,
		       pparam->ss_fft_size,
		       pparam->ss_gc_ena,
		       pparam->ss_restart_ena,
		       (int8_t)pparam->ss_noise_floor_ref,
		       pparam->ss_init_delay,
		       pparam->ss_nb_tone_thr,
		       pparam->ss_str_bin_thr,
		       pparam->ss_wb_rpt_mode,
		       pparam->ss_rssi_rpt_mode,
		       (int8_t)pparam->ss_rssi_thr,
		       pparam->ss_pwr_format,
		       pparam->ss_rpt_mode,
		       pparam->ss_bin_scale,
		       pparam->ss_dbm_adj, pparam->ss_chn_mask);
}

/**
 * target_if_spectral_info_read() - Read spectral information from the cache.
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @specifier: target_if_spectral_info enumeration specifying which
 * information is required
 * @output: Void output pointer into which the information will be read
 * @output_len: size of object pointed to by output pointer
 *
 * Read spectral parameters or the desired state information from the cache.
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_spectral_info_read(
	struct target_if_spectral *spectral,
	enum spectral_scan_mode smode,
	enum target_if_spectral_info specifier,
	void *output, int output_len)
{
	/*
	 * Note: This function is designed to be able to accommodate
	 * WMI reads for defaults, non-cacheable information, etc
	 * if required.
	 */
	struct target_if_spectral_param_state_info *info;
	int is_cacheable = 0;
	int init_def_retval = 0;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return -EINVAL;
	}
	info = &spectral->param_info[smode];

	if (!output)
		return -EINVAL;

	switch (specifier) {
	case TARGET_IF_SPECTRAL_INFO_ACTIVE:
		if (output_len != sizeof(info->osps_cache.osc_spectral_active))
			return -EINVAL;
		is_cacheable = 1;
		break;

	case TARGET_IF_SPECTRAL_INFO_ENABLED:
		if (output_len != sizeof(info->osps_cache.osc_spectral_enabled))
			return -EINVAL;
		is_cacheable = 1;
		break;

	case TARGET_IF_SPECTRAL_INFO_PARAMS:
		if (output_len != sizeof(info->osps_cache.osc_params))
			return -EINVAL;
		is_cacheable = 1;
		break;

	default:
		spectral_err("Unknown target_if_spectral_info specifier");
		return -EINVAL;
	}

	qdf_spin_lock_bh(&info->osps_lock);

	if (is_cacheable) {
		if (info->osps_cache.osc_is_valid) {
			switch (specifier) {
			case TARGET_IF_SPECTRAL_INFO_ACTIVE:
				qdf_mem_copy(
				  output,
				  &info->osps_cache.osc_spectral_active,
				  sizeof(info->osps_cache.osc_spectral_active));

				target_if_log_read_spectral_active(
					__func__,
					*((unsigned char *)output));
				break;

			case TARGET_IF_SPECTRAL_INFO_ENABLED:
				qdf_mem_copy(
				  output,
				  &info->osps_cache.osc_spectral_enabled,
				  sizeof(
					info->osps_cache.osc_spectral_enabled));

				target_if_log_read_spectral_enabled(
					__func__,
					*((unsigned char *)output));
				break;

			case TARGET_IF_SPECTRAL_INFO_PARAMS:
				qdf_mem_copy(
				  output,
				  &info->osps_cache.osc_params,
				  sizeof(info->osps_cache.osc_params));

				target_if_log_read_spectral_params(
					__func__,
					(struct spectral_config *)output);
				break;

			default:
				/* We can't reach this point */
				break;
			}
			qdf_spin_unlock_bh(&info->osps_lock);
			return 0;
		}
	}

	/* Cache is invalid */

	/*
	 * If WMI Reads are implemented to fetch defaults/non-cacheable info,
	 * then the below implementation will change
	 */
	init_def_retval =
			target_if_spectral_info_init_defaults(spectral, smode);
	if (init_def_retval != QDF_STATUS_SUCCESS) {
		qdf_spin_unlock_bh(&info->osps_lock);
		if (init_def_retval == QDF_STATUS_E_NOENT)
			return -ENOENT;
		else
			return -EINVAL;
	}
	/* target_if_spectral_info_init_defaults() has set cache to valid */

	switch (specifier) {
	case TARGET_IF_SPECTRAL_INFO_ACTIVE:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_spectral_active,
			     sizeof(info->osps_cache.osc_spectral_active));

		target_if_log_read_spectral_active_catch_validate(
			__func__,
			*((unsigned char *)output));
		break;

	case TARGET_IF_SPECTRAL_INFO_ENABLED:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_spectral_enabled,
			     sizeof(info->osps_cache.osc_spectral_enabled));

		target_if_log_read_spectral_enabled_catch_validate(
			__func__,
			*((unsigned char *)output));
		break;

	case TARGET_IF_SPECTRAL_INFO_PARAMS:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_params,
			     sizeof(info->osps_cache.osc_params));

		target_if_log_read_spectral_params_catch_validate(
			__func__,
			(struct spectral_config *)output);

		break;

	default:
		/* We can't reach this point */
		break;
	}

	qdf_spin_unlock_bh(&info->osps_lock);

	return 0;
}

/**
 * target_if_log_write_spectral_active() - Helper function to log inputs and
 * return value of call to configure the Spectral 'active' configuration,
 * TARGET_IF_SPECTRAL_INFO_ACTIVE into firmware
 * @function_name: Function name in which this is called
 * @pval: whether spectral is active or not
 * @ret: return value of the firmware write function
 *
 * Return: none
 */
static void
target_if_log_write_spectral_active(
	const char *function_name,
	uint8_t pval,
	int ret)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ACTIVE with val=%u status=%d",
		       function_name, pval, ret);
}

/**
 * target_if_log_write_spectral_enabled() - Helper function to log inputs and
 * return value of call to configure the Spectral 'enabled' configuration,
 * TARGET_IF_SPECTRAL_INFO_ENABLED into firmware
 * @function_name: Function name in which this is called
 * @pval: whether spectral is enabled or not
 * @ret: return value of the firmware write function
 *
 * Return: none
 */
static void
target_if_log_write_spectral_enabled(
	const char *function_name,
	uint8_t pval,
	int ret)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_ENABLED with val=%u status=%d",
		       function_name, pval, ret);
}

/**
 * target_if_log_write_spectral_params() - Helper function to log inputs and
 * return value of call to configure Spectral parameters,
 * TARGET_IF_SPECTRAL_INFO_PARAMS into firmware
 * @param: Spectral parameters
 * @function_name: Function name in which this is called
 * @ret: return value of the firmware write function
 *
 * Return: none
 */
static void
target_if_log_write_spectral_params(
	struct spectral_config *param,
	const char *function_name,
	int ret)
{
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_PARAMS. Params:\nss_count = %u\nss_period = %u\nss_spectral_pri = %u\nss_fft_size = %u\nss_gc_ena = %u\nss_restart_ena = %u\nss_noise_floor_ref = %d\nss_init_delay = %u\nss_nb_tone_thr = %u\nss_str_bin_thr = %u\nss_wb_rpt_mode = %u\nss_rssi_rpt_mode = %u\nss_rssi_thr = %d\nss_pwr_format = %u\nss_rpt_mode = %u\nss_bin_scale = %u\nss_dbm_adj = %u\nss_chn_mask = %u\nss_frequency1=%u\nss_frequency2=%u\nstatus = %d",
		       function_name,
		       param->ss_count,
		       param->ss_period,
		       param->ss_spectral_pri,
		       param->ss_fft_size,
		       param->ss_gc_ena,
		       param->ss_restart_ena,
		       (int8_t)param->ss_noise_floor_ref,
		       param->ss_init_delay,
		       param->ss_nb_tone_thr,
		       param->ss_str_bin_thr,
		       param->ss_wb_rpt_mode,
		       param->ss_rssi_rpt_mode,
		       (int8_t)param->ss_rssi_thr,
		       param->ss_pwr_format,
		       param->ss_rpt_mode,
		       param->ss_bin_scale,
		       param->ss_dbm_adj,
		       param->ss_chn_mask,
		       param->ss_frequency.cfreq1,
		       param->ss_frequency.cfreq2,
		       ret);
}

/**
 * target_if_spectral_info_write() - Write Spectral information to the
 * firmware, and update cache
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @specifier: target_if_spectral_info enumeration specifying which
 * information is involved
 * @input: void input pointer containing the information to be written
 * @input_len: size of object pointed to by input pointer
 *
 * Write Spectral parameters or the desired state information to
 * the firmware, and update cache
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_spectral_info_write(
	struct target_if_spectral *spectral,
	enum spectral_scan_mode smode,
	enum target_if_spectral_info specifier,
	void *input, int input_len)
{
	struct target_if_spectral_param_state_info *info;
	int ret;
	uint8_t *pval = NULL;
	struct spectral_config *param = NULL;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return -EINVAL;
	}
	info = &spectral->param_info[smode];

	if (!input)
		return -EINVAL;

	switch (specifier) {
	case TARGET_IF_SPECTRAL_INFO_ACTIVE:
		if (input_len != sizeof(info->osps_cache.osc_spectral_active))
			return -EINVAL;

		pval = (uint8_t *)input;

		qdf_spin_lock_bh(&info->osps_lock);
		ret = target_if_send_vdev_spectral_enable_cmd(spectral, smode,
							      1, *pval, 0, 0);

		target_if_log_write_spectral_active(
			__func__,
			*pval,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_enable_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock_bh(&info->osps_lock);
			return ret;
		}

		info->osps_cache.osc_spectral_active = *pval;

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock_bh(&info->osps_lock);
		break;

	case TARGET_IF_SPECTRAL_INFO_ENABLED:
		if (input_len != sizeof(info->osps_cache.osc_spectral_enabled))
			return -EINVAL;

		pval = (uint8_t *)input;

		qdf_spin_lock_bh(&info->osps_lock);
		ret = target_if_send_vdev_spectral_enable_cmd(spectral, smode,
							      0, 0, 1, *pval);

		target_if_log_write_spectral_enabled(
			__func__,
			*pval,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_enable_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock_bh(&info->osps_lock);
			return ret;
		}

		info->osps_cache.osc_spectral_enabled = *pval;

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock_bh(&info->osps_lock);
		break;

	case TARGET_IF_SPECTRAL_INFO_PARAMS:
		if (input_len != sizeof(info->osps_cache.osc_params))
			return -EINVAL;

		param = (struct spectral_config *)input;

		qdf_spin_lock_bh(&info->osps_lock);
		ret = target_if_send_vdev_spectral_configure_cmd(spectral,
								 smode, param);

		target_if_log_write_spectral_params(
			param,
			__func__,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_configure_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock_bh(&info->osps_lock);
			return ret;
		}

		qdf_mem_copy(&info->osps_cache.osc_params,
			     param, sizeof(info->osps_cache.osc_params));

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock_bh(&info->osps_lock);
		break;

	default:
		spectral_err("Unknown target_if_spectral_info specifier");
		return -EINVAL;
	}

	return 0;
}

/**
 * target_if_spectral_get_tsf64() - Function to get the TSF value
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Get the last TSF received in WMI buffer
 *
 * Return: TSF value
 */
static uint64_t
target_if_spectral_get_tsf64(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return spectral->tsf64;
}

/**
 * target_if_spectral_get_capability() - Function to get whether a
 * given Spectral hardware capability is available
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @type: Spectral hardware capability type
 *
 * Get whether a given Spectral hardware capability is available
 *
 * Return: True if the capability is available, false if the capability is not
 * available
 */
uint32_t
target_if_spectral_get_capability(void *arg, enum spectral_capability_type type)
{
	int status = STATUS_FAIL;

	switch (type) {
	case SPECTRAL_CAP_PHYDIAG:
	case SPECTRAL_CAP_RADAR:
	case SPECTRAL_CAP_SPECTRAL_SCAN:
	case SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN:
		status = STATUS_PASS;
		break;
	default:
		status = STATUS_FAIL;
	}
	return status;
}

/**
 * target_if_spectral_set_rxfilter() - Set the RX Filter before Spectral start
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @rxfilter: Rx filter to be used
 *
 * Note: This is only a placeholder function. It is not currently required since
 * FW should be taking care of setting the required filters.
 *
 * Return: 0
 */
uint32_t
target_if_spectral_set_rxfilter(void *arg, int rxfilter)
{
	/*
	 * Will not be required since enabling of spectral in firmware
	 * will take care of this
	 */
	return 0;
}

/**
 * target_if_spectral_get_rxfilter() - Get the current RX Filter settings
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Note: This is only a placeholder function. It is not currently required since
 * FW should be taking care of setting the required filters.
 *
 * Return: 0
 */
uint32_t
target_if_spectral_get_rxfilter(void *arg)
{
	/*
	 * Will not be required since enabling of spectral in firmware
	 * will take care of this
	 */
	return 0;
}

/**
 * target_if_sops_is_spectral_active() - Get whether Spectral is active
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to check whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
uint32_t
target_if_sops_is_spectral_active(void *arg, enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_ACTIVE,
		&val, sizeof(val));

	if (ret != 0) {
		/*
		 * Could not determine if Spectral is active.
		 * Return false as a safe value.
		 * XXX: Consider changing the function prototype
		 * to be able to indicate failure to fetch value.
		 */
		return 0;
	}

	return val;
}

/**
 * target_if_sops_is_spectral_enabled() - Get whether Spectral is enabled
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to check whether Spectral is enabled
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
uint32_t
target_if_sops_is_spectral_enabled(void *arg, enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_ENABLED,
		&val, sizeof(val));

	if (ret != 0) {
		/*
		 * Could not determine if Spectral is enabled.
		 * Return false as a safe value.
		 * XXX: Consider changing the function prototype
		 * to be able to indicate failure to fetch value.
		 */
		return 0;
	}

	return val;
}

/**
 * target_if_sops_start_spectral_scan() - Start Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @err: Spectral error code
 *
 * Function to start spectral scan
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_sops_start_spectral_scan(void *arg, enum spectral_scan_mode smode,
				   enum spectral_cp_error_code *err)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 1;
	uint8_t enabled = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_ENABLED,
		&enabled, sizeof(enabled));

	if (ret != 0) {
		/*
		 * Could not determine if Spectral is enabled. Assume we need
		 * to enable it
		 */
		enabled = 0;
	}

	if (!enabled) {
		ret = target_if_spectral_info_write(
			spectral,
			smode,
			TARGET_IF_SPECTRAL_INFO_ENABLED,
			&val, sizeof(val));

		if (ret != 0)
			return ret;
	}

	ret = target_if_spectral_info_write(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_ACTIVE,
		&val, sizeof(val));

	if (ret != 0)
		return ret;

	return 0;
}

/**
 * target_if_sops_stop_spectral_scan() - Stop Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to stop spectral scan
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_sops_stop_spectral_scan(void *arg, enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int tempret, ret = 0;
	uint8_t enabled = 0;

	tempret = target_if_spectral_info_read(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_ENABLED,
		&enabled, sizeof(enabled));

	if (tempret)
		/*
		 * Could not determine if Spectral is enabled. Assume scan is
		 * not in progress
		 */
		enabled = 0;

	/* if scan is not enabled, no need to send stop to FW */
	if (!enabled)
		return -EPERM;

	tempret = target_if_spectral_info_write(
			spectral,
			smode,
			TARGET_IF_SPECTRAL_INFO_ACTIVE,
			&val, sizeof(val));

	if (tempret != 0)
		ret = tempret;

	tempret = target_if_spectral_info_write(
			spectral,
			smode,
			TARGET_IF_SPECTRAL_INFO_ENABLED,
			&val, sizeof(val));

	if (tempret != 0)
		ret = tempret;

	return ret;
}

/**
 * target_if_spectral_get_extension_channel() - Get the Extension channel
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to get the current Extension channel (in MHz)
 *
 * Return: Current Extension channel (in MHz) on success, 0 on failure or if
 * extension channel is not present.
 */
uint32_t
target_if_spectral_get_extension_channel(void *arg,
					 enum spectral_scan_mode smode)
{
	/*
	 * XXX: Once we expand to use cases where Spectral could be activated
	 * without a channel being set to VDEV, we need to consider returning a
	 * negative value in case of failure and having all callers handle this.
	 */

	struct target_if_spectral *spectral = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	uint16_t sec20chan_freq = 0;

	qdf_assert_always(arg);
	spectral = (struct target_if_spectral *)arg;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return 0;
	}
	vdev = target_if_spectral_get_vdev(spectral, smode);
	if (!vdev)
		return 0;

	if (target_if_vdev_get_sec20chan_freq_mhz(vdev, &sec20chan_freq) < 0) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
		return 0;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	return sec20chan_freq;
}

/**
 * target_if_spectral_get_current_channel() - Get the current channel
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to get the current channel (in MHz)
 *
 * Return: Current channel (in MHz) on success, 0 on failure
 */
uint32_t
target_if_spectral_get_current_channel(void *arg, enum spectral_scan_mode smode)
{
	/*
	 * XXX: Once we expand to use cases where Spectral could be activated
	 * without a channel being set to VDEV, we need to consider returning a
	 * negative value in case of failure and having all callers handle this.
	 */

	struct target_if_spectral *spectral = NULL;
	int16_t chan_freq = 0;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(arg);
	spectral = (struct target_if_spectral *)arg;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return 0;
	}
	vdev = target_if_spectral_get_vdev(spectral, smode);
	if (!vdev)
		return 0;

	chan_freq = target_if_vdev_get_chan_freq(vdev);
	if (chan_freq < 0) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
		return 0;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	return chan_freq;
}

/**
 * target_if_spectral_reset_hw() - Reset the hardware
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
uint32_t
target_if_spectral_reset_hw(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_get_chain_noise_floor() - Get the Chain noise floor from
 * Noisefloor history buffer
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @nf_buf: Pointer to buffer into which chain Noise Floor data should be copied
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
uint32_t
target_if_spectral_get_chain_noise_floor(void *arg, int16_t *nf_buf)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_get_ext_noisefloor() - Get the extension channel
 * noisefloor
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
int8_t
target_if_spectral_get_ext_noisefloor(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_get_ctl_noisefloor() - Get the control channel noisefloor
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
int8_t
target_if_spectral_get_ctl_noisefloor(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_sops_configure_params() - Configure user supplied Spectral
 *                                         parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Spectral parameters
 * @smode: Spectral scan mode
 *
 * Function to configure spectral parameters
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_spectral_sops_configure_params(
	void *arg, struct spectral_config *params,
	enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return target_if_spectral_info_write(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_PARAMS,
		params, sizeof(*params));
}

/**
 * target_if_spectral_sops_get_params() - Get user configured Spectral
 * parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Pointer to buffer into which Spectral parameters should be copied
 * @smode: Spectral scan mode
 *
 * Function to get the configured spectral parameters
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_spectral_sops_get_params(void *arg, struct spectral_config *params,
				   enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return target_if_spectral_info_read(
		spectral,
		smode,
		TARGET_IF_SPECTRAL_INFO_PARAMS,
		params, sizeof(*params));
}

/**
 * target_if_spectral_get_ent_mask() - Get enterprise mask
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
static uint32_t
target_if_spectral_get_ent_mask(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_get_macaddr() - Get radio MAC address
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @addr: Pointer to buffer into which MAC address should be copied
 *
 * Function to get the MAC address of the pdev
 *
 * Return: 0 on success, -1 on failure
 */
static uint32_t
target_if_spectral_get_macaddr(void *arg, char *addr)
{
	uint8_t *myaddr = NULL;
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	struct wlan_objmgr_pdev *pdev = NULL;

	pdev = spectral->pdev_obj;

	wlan_pdev_obj_lock(pdev);
	myaddr = wlan_pdev_get_hw_macaddr(pdev);
	wlan_pdev_obj_unlock(pdev);
	qdf_mem_copy(addr, myaddr, QDF_MAC_ADDR_SIZE);

	return 0;
}

/**
 * target_if_init_spectral_param_min_max() - Initialize Spectral parameter
 * min and max values
 *
 * @param_min_max: Pointer to Spectral parameter min and max structure
 * @gen: Spectral HW generation
 * @target_type: Target type
 *
 * Initialize Spectral parameter min and max values
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_init_spectral_param_min_max(
				struct spectral_param_min_max *param_min_max,
				enum spectral_gen gen, uint32_t target_type)
{
	switch (gen) {
	case SPECTRAL_GEN3:
		param_min_max->fft_size_min = SPECTRAL_PARAM_FFT_SIZE_MIN_GEN3;
		param_min_max->fft_size_max[CH_WIDTH_20MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT;
		if (target_type == TARGET_TYPE_QCN9000 ||
		    target_type == TARGET_TYPE_QCN9100 ||
		    target_type == TARGET_TYPE_QCA5018 ||
		    target_type == TARGET_TYPE_QCA6490) {
			param_min_max->fft_size_max[CH_WIDTH_40MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_QCN9000;
			param_min_max->fft_size_max[CH_WIDTH_80MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_QCN9000;
			param_min_max->fft_size_max[CH_WIDTH_160MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_QCN9000;
			param_min_max->fft_size_max[CH_WIDTH_80P80MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_QCN9000;
		} else {
			param_min_max->fft_size_max[CH_WIDTH_40MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT;
			param_min_max->fft_size_max[CH_WIDTH_80MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT;
			param_min_max->fft_size_max[CH_WIDTH_160MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT;
			param_min_max->fft_size_max[CH_WIDTH_80P80MHZ] =
				SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT;
		}
		break;

	case SPECTRAL_GEN2:
		param_min_max->fft_size_min = SPECTRAL_PARAM_FFT_SIZE_MIN_GEN2;
		param_min_max->fft_size_max[CH_WIDTH_20MHZ] =
					SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2;
		param_min_max->fft_size_max[CH_WIDTH_40MHZ] =
					SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2;
		param_min_max->fft_size_max[CH_WIDTH_80MHZ] =
					SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2;
		param_min_max->fft_size_max[CH_WIDTH_80P80MHZ] =
					SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2;
		param_min_max->fft_size_max[CH_WIDTH_160MHZ] =
					SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2;
		break;

	default:
		spectral_err("Invalid spectral generation %d", gen);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_init_spectral_param_properties() - Initialize Spectral parameter
 *                                              properties
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Initialize Spectral parameter properties
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_init_spectral_param_properties(struct target_if_spectral *spectral)
{
	enum spectral_scan_mode smode = SPECTRAL_SCAN_MODE_NORMAL;
	int param;

	/* Initialize default values for properties.
	 * Default values are supported for all the parameters for all modes
	 * and allows different values for each mode for all the parameters .
	 */
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++) {
		for (param = 0; param < SPECTRAL_PARAM_MAX; param++) {
			spectral->properties[smode][param].supported = true;
			spectral->properties[smode][param].common_all_modes =
									false;
		}
	}

	/* Once FW advertisement is in place remove this hard coding */
	smode = SPECTRAL_SCAN_MODE_NORMAL;
	spectral->properties[SPECTRAL_SCAN_MODE_NORMAL]
			[SPECTRAL_PARAM_FREQUENCY].supported = false;
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++) {
		spectral->properties[smode]
			[SPECTRAL_PARAM_SPECT_PRI].common_all_modes = true;
		spectral->properties[smode]
			[SPECTRAL_PARAM_SCAN_PERIOD].common_all_modes = true;
		spectral->properties[smode]
			[SPECTRAL_PARAM_INIT_DELAY].common_all_modes = true;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_init_spectral_capability(struct target_if_spectral *spectral,
				   uint32_t target_type)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_psoc_host_spectral_scaling_params *scaling_params;
	uint8_t num_bin_scaling_params, param_idx, pdev_id;
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_svc_param;
	struct spectral_caps *pcap = &spectral->capability;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap_arr = NULL;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap = NULL;
	struct wlan_psoc_host_chainmask_table *table;
	int j;
	uint32_t table_id;

	pdev = spectral->pdev_obj;
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		spectral_err("target_psoc_info is null");
		return QDF_STATUS_E_FAILURE;
	}

	ext_svc_param = target_psoc_get_service_ext_param(tgt_psoc_info);
	num_bin_scaling_params = ext_svc_param->num_bin_scaling_params;
	scaling_params = target_psoc_get_spectral_scaling_params(tgt_psoc_info);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	/* XXX : Workaround: Set Spectral capability */
	pcap = &spectral->capability;
	pcap->phydiag_cap = 1;
	pcap->radar_cap = 1;
	pcap->spectral_cap = 1;
	pcap->advncd_spectral_cap = 1;
	pcap->hw_gen = spectral->spectral_gen;
	if (spectral->spectral_gen >= SPECTRAL_GEN3) {
		mac_phy_cap_arr = target_psoc_get_mac_phy_cap(tgt_psoc_info);
		if (!mac_phy_cap_arr) {
			spectral_err("mac phy cap array is null");
			return QDF_STATUS_E_FAILURE;
		}

		mac_phy_cap = &mac_phy_cap_arr[pdev_id];
		if (!mac_phy_cap) {
			spectral_err("mac phy cap is null");
			return QDF_STATUS_E_FAILURE;
		}

		table_id = mac_phy_cap->chainmask_table_id;
		table =  &ext_svc_param->chainmask_table[table_id];
		if (!table) {
			spectral_err("chainmask table not found");
			return QDF_STATUS_E_FAILURE;
		}

		for (j = 0; j < table->num_valid_chainmasks; j++) {
			pcap->agile_spectral_cap |=
				table->cap_list[j].supports_aSpectral;
			pcap->agile_spectral_cap_160 |=
				table->cap_list[j].supports_aSpectral_160;
		}
		pcap->agile_spectral_cap_80p80 = pcap->agile_spectral_cap_160;
	} else {
		pcap->agile_spectral_cap = false;
		pcap->agile_spectral_cap_160 = false;
		pcap->agile_spectral_cap_80p80 = false;
	}

	if (scaling_params) {
		for (param_idx = 0; param_idx < num_bin_scaling_params;
		     param_idx++) {
			if (scaling_params[param_idx].pdev_id == pdev_id) {
				pcap->is_scaling_params_populated = true;
				pcap->formula_id =
				    scaling_params[param_idx].formula_id;
				pcap->low_level_offset =
				    scaling_params[param_idx].low_level_offset;
				pcap->high_level_offset =
				    scaling_params[param_idx].high_level_offset;
				pcap->rssi_thr =
				    scaling_params[param_idx].rssi_thr;
				pcap->default_agc_max_gain =
				 scaling_params[param_idx].default_agc_max_gain;
				break;
			}
		}
	}

	pcap->num_detectors_20mhz = 1;
	pcap->num_detectors_40mhz = 1;
	pcap->num_detectors_80mhz = 1;
	if (target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA6490) {
		pcap->num_detectors_160mhz = 1;
		pcap->num_detectors_80p80mhz = 1;
	} else {
		pcap->num_detectors_160mhz = 2;
		pcap->num_detectors_80p80mhz = 2;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
/**
 * target_if_init_spectral_simulation_ops() - Initialize spectral target_if
 * internal operations with functions related to spectral simulation
 * @p_sops: spectral low level ops table
 *
 * Initialize spectral target_if internal operations with functions
 * related to spectral simulation
 *
 * Return: None
 */
static void
target_if_init_spectral_simulation_ops(struct target_if_spectral_ops *p_sops)
{
	/*
	 * Spectral simulation is currently intended for platform transitions
	 * where underlying HW support may not be available for some time.
	 * Hence, we do not currently provide a runtime switch to turn the
	 * simulation on or off.
	 * In case of future requirements where runtime switches are required,
	 * this can be added. But it is suggested to use application layer
	 * simulation as far as possible in such cases, since the main
	 * use of record and replay of samples would concern higher
	 * level sample processing rather than lower level delivery.
	 */
	p_sops->is_spectral_enabled = target_if_spectral_sops_sim_is_enabled;
	p_sops->is_spectral_active = target_if_spectral_sops_sim_is_active;
	p_sops->start_spectral_scan = target_if_spectral_sops_sim_start_scan;
	p_sops->stop_spectral_scan = target_if_spectral_sops_sim_stop_scan;
	p_sops->configure_spectral =
		target_if_spectral_sops_sim_configure_params;
	p_sops->get_spectral_config = target_if_spectral_sops_sim_get_params;
}

#else
/**
 * target_if_init_spectral_simulation_ops() - Initialize spectral target_if
 * internal operations
 * @p_sops: spectral low level ops table
 *
 * Return: None
 */
static void
target_if_init_spectral_simulation_ops(struct target_if_spectral_ops *p_sops)
{
	p_sops->is_spectral_enabled = target_if_sops_is_spectral_enabled;
	p_sops->is_spectral_active = target_if_sops_is_spectral_active;
	p_sops->start_spectral_scan = target_if_sops_start_spectral_scan;
	p_sops->stop_spectral_scan = target_if_sops_stop_spectral_scan;
	p_sops->configure_spectral = target_if_spectral_sops_configure_params;
	p_sops->get_spectral_config = target_if_spectral_sops_get_params;
}
#endif

/**
 * target_if_init_spectral_ops_common() - Initialize Spectral target_if internal
 * operations common to all Spectral chipset generations
 *
 * Initializes target_if_spectral_ops common to all chipset generations
 *
 * Return: None
 */
static void
target_if_init_spectral_ops_common(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->get_tsf64 = target_if_spectral_get_tsf64;
	p_sops->get_capability = target_if_spectral_get_capability;
	p_sops->set_rxfilter = target_if_spectral_set_rxfilter;
	p_sops->get_rxfilter = target_if_spectral_get_rxfilter;

	target_if_init_spectral_simulation_ops(p_sops);

	p_sops->get_extension_channel =
	    target_if_spectral_get_extension_channel;
	p_sops->get_ctl_noisefloor = target_if_spectral_get_ctl_noisefloor;
	p_sops->get_ext_noisefloor = target_if_spectral_get_ext_noisefloor;
	p_sops->get_ent_spectral_mask = target_if_spectral_get_ent_mask;
	p_sops->get_mac_address = target_if_spectral_get_macaddr;
	p_sops->get_current_channel = target_if_spectral_get_current_channel;
	p_sops->reset_hw = target_if_spectral_reset_hw;
	p_sops->get_chain_noise_floor =
	    target_if_spectral_get_chain_noise_floor;
}

/**
 * target_if_init_spectral_ops_gen2() - Initialize Spectral target_if internal
 * operations specific to Spectral chipset generation 2.
 *
 * Initializes target_if_spectral_ops specific to Spectral chipset generation 2.
 *
 * Return: None
 */
static void
target_if_init_spectral_ops_gen2(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->spectral_process_phyerr = target_if_process_phyerr_gen2;
}

/**
 * target_if_init_spectral_ops_gen3() - Initialize Spectral target_if internal
 * operations specific to Spectral chipset generation 3.
 *
 * Initializes target_if_spectral_ops specific to Spectral chipset generation 3.
 *
 * Return: None
 */
static void
target_if_init_spectral_ops_gen3(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->process_spectral_report =
			target_if_spectral_process_report_gen3;
	return;
}

/**
 * target_if_init_spectral_ops() - Initialize target_if internal Spectral
 * operations.
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Initializes all function pointers in target_if_spectral_ops for
 * all generations
 *
 * Return: None
 */
static void
target_if_init_spectral_ops(struct target_if_spectral *spectral)
{
	target_if_init_spectral_ops_common();
	if (spectral->spectral_gen == SPECTRAL_GEN2)
		target_if_init_spectral_ops_gen2();
	else if (spectral->spectral_gen == SPECTRAL_GEN3)
		target_if_init_spectral_ops_gen3();
	else
		spectral_err("Invalid Spectral generation");
}

/*
 * Dummy Functions:
 * These functions are initially registered to avoid any crashes due to
 * invocation of spectral functions before they are registered.
 */

static uint64_t
null_get_tsf64(void *arg)
{
	spectral_ops_not_registered("get_tsf64");
	return 0;
}

static uint32_t
null_get_capability(void *arg, enum spectral_capability_type type)
{
	/*
	 * TODO : We should have conditional compilation to get the capability
	 *      : We have not yet attahced ATH layer here, so there is no
	 *      : way to check the HAL capbalities
	 */
	spectral_ops_not_registered("get_capability");

	/* TODO : For the time being, we are returning TRUE */
	return true;
}

static uint32_t
null_set_rxfilter(void *arg, int rxfilter)
{
	spectral_ops_not_registered("set_rxfilter");
	return 1;
}

static uint32_t
null_get_rxfilter(void *arg)
{
	spectral_ops_not_registered("get_rxfilter");
	return 0;
}

static uint32_t
null_is_spectral_active(void *arg, enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("is_spectral_active");
	return 1;
}

static uint32_t
null_is_spectral_enabled(void *arg, enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("is_spectral_enabled");
	return 1;
}

static uint32_t
null_start_spectral_scan(void *arg, enum spectral_scan_mode smode,
			 enum spectral_cp_error_code *err)
{
	spectral_ops_not_registered("start_spectral_scan");
	return 1;
}

static uint32_t
null_stop_spectral_scan(void *arg, enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("stop_spectral_scan");
	return 1;
}

static uint32_t
null_get_extension_channel(void *arg, enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("get_extension_channel");
	return 1;
}

static int8_t
null_get_ctl_noisefloor(void *arg)
{
	spectral_ops_not_registered("get_ctl_noisefloor");
	return 1;
}

static int8_t
null_get_ext_noisefloor(void *arg)
{
	spectral_ops_not_registered("get_ext_noisefloor");
	return 0;
}

static uint32_t
null_configure_spectral(void *arg, struct spectral_config *params,
			enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("configure_spectral");
	return 0;
}

static uint32_t
null_get_spectral_config(void *arg, struct spectral_config *params,
			 enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("get_spectral_config");
	return 0;
}

static uint32_t
null_get_ent_spectral_mask(void *arg)
{
	spectral_ops_not_registered("get_ent_spectral_mask");
	return 0;
}

static uint32_t
null_get_mac_address(void *arg, char *addr)
{
	spectral_ops_not_registered("get_mac_address");
	return 0;
}

static uint32_t
null_get_current_channel(void *arg, enum spectral_scan_mode smode)
{
	spectral_ops_not_registered("get_current_channel");
	return 0;
}

static uint32_t
null_reset_hw(void *arg)
{
	spectral_ops_not_registered("get_current_channel");
	return 0;
}

static uint32_t
null_get_chain_noise_floor(void *arg, int16_t *nf_buf)
{
	spectral_ops_not_registered("get_chain_noise_floor");
	return 0;
}

static int
null_spectral_process_phyerr(struct target_if_spectral *spectral,
			     uint8_t *data,
			     uint32_t datalen,
			     struct target_if_spectral_rfqual_info *p_rfqual,
			     struct target_if_spectral_chan_info *p_chaninfo,
			     uint64_t tsf64,
			     struct target_if_spectral_acs_stats *acs_stats)
{
	spectral_ops_not_registered("spectral_process_phyerr");
	return 0;
}

static int
null_process_spectral_report(struct wlan_objmgr_pdev *pdev,
			     void *payload)
{
	spectral_ops_not_registered("process_spectral_report");
	return 0;
}
/**
 * target_if_spectral_init_dummy_function_table() -
 * Initialize target_if internal
 * Spectral operations to dummy functions
 * @ps: Pointer to Spectral target_if internal private data
 *
 * Initialize all the function pointers in target_if_spectral_ops with
 * dummy functions.
 *
 * Return: None
 */
static void
target_if_spectral_init_dummy_function_table(struct target_if_spectral *ps)
{
	struct target_if_spectral_ops *p_sops = GET_TARGET_IF_SPECTRAL_OPS(ps);

	p_sops->get_tsf64 = null_get_tsf64;
	p_sops->get_capability = null_get_capability;
	p_sops->set_rxfilter = null_set_rxfilter;
	p_sops->get_rxfilter = null_get_rxfilter;
	p_sops->is_spectral_enabled = null_is_spectral_enabled;
	p_sops->is_spectral_active = null_is_spectral_active;
	p_sops->start_spectral_scan = null_start_spectral_scan;
	p_sops->stop_spectral_scan = null_stop_spectral_scan;
	p_sops->get_extension_channel = null_get_extension_channel;
	p_sops->get_ctl_noisefloor = null_get_ctl_noisefloor;
	p_sops->get_ext_noisefloor = null_get_ext_noisefloor;
	p_sops->configure_spectral = null_configure_spectral;
	p_sops->get_spectral_config = null_get_spectral_config;
	p_sops->get_ent_spectral_mask = null_get_ent_spectral_mask;
	p_sops->get_mac_address = null_get_mac_address;
	p_sops->get_current_channel = null_get_current_channel;
	p_sops->reset_hw = null_reset_hw;
	p_sops->get_chain_noise_floor = null_get_chain_noise_floor;
	p_sops->spectral_process_phyerr = null_spectral_process_phyerr;
	p_sops->process_spectral_report = null_process_spectral_report;
}

/**
 * target_if_spectral_register_funcs() - Initialize target_if internal Spectral
 * operations
 * @spectral: Pointer to Spectral target_if internal private data
 * @p: Pointer to Spectral function table
 *
 * Return: None
 */
static void
target_if_spectral_register_funcs(struct target_if_spectral *spectral,
				  struct target_if_spectral_ops *p)
{
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);

	p_sops->get_tsf64 = p->get_tsf64;
	p_sops->get_capability = p->get_capability;
	p_sops->set_rxfilter = p->set_rxfilter;
	p_sops->get_rxfilter = p->get_rxfilter;
	p_sops->is_spectral_enabled = p->is_spectral_enabled;
	p_sops->is_spectral_active = p->is_spectral_active;
	p_sops->start_spectral_scan = p->start_spectral_scan;
	p_sops->stop_spectral_scan = p->stop_spectral_scan;
	p_sops->get_extension_channel = p->get_extension_channel;
	p_sops->get_ctl_noisefloor = p->get_ctl_noisefloor;
	p_sops->get_ext_noisefloor = p->get_ext_noisefloor;
	p_sops->configure_spectral = p->configure_spectral;
	p_sops->get_spectral_config = p->get_spectral_config;
	p_sops->get_ent_spectral_mask = p->get_ent_spectral_mask;
	p_sops->get_mac_address = p->get_mac_address;
	p_sops->get_current_channel = p->get_current_channel;
	p_sops->reset_hw = p->reset_hw;
	p_sops->get_chain_noise_floor = p->get_chain_noise_floor;
	p_sops->spectral_process_phyerr = p->spectral_process_phyerr;
	p_sops->process_spectral_report = p->process_spectral_report;
}

/**
 * target_if_spectral_clear_stats() - Clear Spectral stats
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Function to clear spectral stats
 *
 * Return: None
 */
static void
target_if_spectral_clear_stats(struct target_if_spectral *spectral)
{
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);

	qdf_mem_zero(&spectral->spectral_stats,
		     sizeof(struct target_if_spectral_stats));
	spectral->spectral_stats.last_reset_tstamp =
	    p_sops->get_tsf64(spectral);
}

/**
 * target_if_spectral_check_hw_capability() - Check whether HW supports spectral
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Function to check whether hardware supports spectral
 *
 * Return: True if HW supports Spectral, false if HW does not support Spectral
 */
static int
target_if_spectral_check_hw_capability(struct target_if_spectral *spectral)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct spectral_caps *pcap = NULL;
	int is_spectral_supported = true;

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	pcap = &spectral->capability;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_PHYDIAG) == false) {
		is_spectral_supported = false;
		spectral_info("SPECTRAL : No PHYDIAG support");
		return is_spectral_supported;
	}
	pcap->phydiag_cap = 1;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_RADAR) == false) {
		is_spectral_supported = false;
		spectral_info("SPECTRAL : No RADAR support");
		return is_spectral_supported;
	}
	pcap->radar_cap = 1;

	if (p_sops->get_capability(spectral,
				   SPECTRAL_CAP_SPECTRAL_SCAN) == false) {
		is_spectral_supported = false;
		spectral_info("SPECTRAL : No SPECTRAL SUPPORT");
		return is_spectral_supported;
	}
	pcap->spectral_cap = 1;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN)
	    == false) {
		spectral_info("SPECTRAL : No ADVANCED SPECTRAL SUPPORT");
	} else {
		pcap->advncd_spectral_cap = 1;
	}

	return is_spectral_supported;
}

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
/**
 * target_if_spectral_detach_simulation() - De-initialize Spectral
 * Simulation functionality
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Function to de-initialize Spectral Simulation functionality
 *
 * Return: None
 */
static void
target_if_spectral_detach_simulation(struct target_if_spectral *spectral)
{
	target_if_spectral_sim_detach(spectral);
}

#else
static void
target_if_spectral_detach_simulation(struct target_if_spectral *spectral)
{
}
#endif

/**
 * target_if_spectral_detach() - De-initialize target_if Spectral
 * @pdev: Pointer to pdev object
 *
 * Function to detach target_if spectral
 *
 * Return: None
 */
static void
target_if_spectral_detach(struct target_if_spectral *spectral)
{
	enum spectral_scan_mode smode = SPECTRAL_SCAN_MODE_NORMAL;
	spectral_info("spectral detach");

	if (spectral) {
		for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
			qdf_spinlock_destroy
				(&spectral->param_info[smode].osps_lock);

		target_if_spectral_detach_simulation(spectral);

		qdf_spinlock_destroy(&spectral->spectral_lock);
		qdf_spinlock_destroy(&spectral->noise_pwr_reports_lock);

		qdf_mem_free(spectral);
		spectral = NULL;
	}
}

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
/**
 * target_if_spectral_attach_simulation() - Initialize Spectral Simulation
 * functionality
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Function to initialize spectral simulation functionality
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_spectral_attach_simulation(struct target_if_spectral *spectral)
{
	if (target_if_spectral_sim_attach(spectral)) {
		qdf_mem_free(spectral);
		return -EPERM;
	}
	return 0;
}

#else
static int
target_if_spectral_attach_simulation(struct target_if_spectral *spectral)
{
	return 0;
}
#endif

/**
 * target_if_spectral_len_adj_swar_init() - Initialize FFT bin length adjustment
 * related info
 * @swar: Pointer to Spectral FFT bin length adjustment SWAR params
 * @target_type: Target type
 *
 * Function to Initialize parameters related to Spectral FFT bin
 * length adjustment SWARs.
 *
 * Return: void
 */
static void
target_if_spectral_len_adj_swar_init(struct spectral_fft_bin_len_adj_swar *swar,
				     uint32_t target_type)
{
	if (target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA5018 ||
	    target_type == TARGET_TYPE_QCA6750 ||
	    target_type == TARGET_TYPE_QCA6490)
		swar->fftbin_size_war = SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE;
	else if (target_type == TARGET_TYPE_QCA8074 ||
		 target_type == TARGET_TYPE_QCA6018 ||
		 target_type == TARGET_TYPE_QCA6390)
		swar->fftbin_size_war = SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE;
	else
		swar->fftbin_size_war = SPECTRAL_FFTBIN_SIZE_NO_WAR;

	if (target_type == TARGET_TYPE_QCA8074 ||
	    target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCA6018 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA5018 ||
	    target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCA6490) {
		swar->inband_fftbin_size_adj = 1;
		swar->null_fftbin_adj = 1;
	} else {
		swar->inband_fftbin_size_adj = 0;
		swar->null_fftbin_adj = 0;
	}

	if ((target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCN9100))
		swar->packmode_fftbin_size_adj = 1;
	else
		swar->packmode_fftbin_size_adj = 0;
}

/**
 * target_if_spectral_report_params_init() - Initialize parameters which
 * describes the structure of Spectral reports
 *
 * @rparams: Pointer to Spectral report parameter object
 * @target_type: target type
 *
 * Function to Initialize parameters related to the structure of Spectral
 * reports.
 *
 * Return: void
 */
static void
target_if_spectral_report_params_init(
			struct spectral_report_params *rparams,
			uint32_t target_type)
{
	enum spectral_scan_mode smode;

	/* This entries are currently used by gen3 chipsets only. Hence
	 * initialization is done for gen3 alone. In future if other generations
	 * needs to use them they have to add proper initial values.
	 */
	if (target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA5018 ||
	    target_type == TARGET_TYPE_QCA6750 ||
	    target_type == TARGET_TYPE_QCA6490) {
		rparams->version = SPECTRAL_REPORT_FORMAT_VERSION_2;
		rparams->num_spectral_detectors =
				NUM_SPECTRAL_DETECTORS_GEN3_V2;
		smode = SPECTRAL_SCAN_MODE_NORMAL;
		for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
			rparams->fragmentation_160[smode] = false;
		rparams->max_agile_ch_width = CH_WIDTH_80P80MHZ;
	} else {
		rparams->version = SPECTRAL_REPORT_FORMAT_VERSION_1;
		rparams->num_spectral_detectors =
				NUM_SPECTRAL_DETECTORS_GEN3_V1;
		smode = SPECTRAL_SCAN_MODE_NORMAL;
		for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
			rparams->fragmentation_160[smode] = true;
		rparams->max_agile_ch_width = CH_WIDTH_80MHZ;
	}

	switch (rparams->version) {
	case SPECTRAL_REPORT_FORMAT_VERSION_1:
		rparams->ssumaary_padding_bytes =
			NUM_PADDING_BYTES_SSCAN_SUMARY_REPORT_GEN3_V1;
		rparams->fft_report_hdr_len =
			FFT_REPORT_HEADER_LENGTH_GEN3_V1;
		break;
	case SPECTRAL_REPORT_FORMAT_VERSION_2:
		rparams->ssumaary_padding_bytes =
			NUM_PADDING_BYTES_SSCAN_SUMARY_REPORT_GEN3_V2;
		rparams->fft_report_hdr_len =
			FFT_REPORT_HEADER_LENGTH_GEN3_V2;
		break;
	default:
		qdf_assert_always(0);
	}

	rparams->detid_mode_table[SPECTRAL_DETECTOR_ID_0] =
						SPECTRAL_SCAN_MODE_NORMAL;
	if (target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA6490) {
		rparams->detid_mode_table[SPECTRAL_DETECTOR_ID_1] =
						SPECTRAL_SCAN_MODE_AGILE;
		rparams->detid_mode_table[SPECTRAL_DETECTOR_ID_2] =
						SPECTRAL_SCAN_MODE_INVALID;
	} else {
		rparams->detid_mode_table[SPECTRAL_DETECTOR_ID_1] =
						SPECTRAL_SCAN_MODE_NORMAL;
		rparams->detid_mode_table[SPECTRAL_DETECTOR_ID_2] =
						SPECTRAL_SCAN_MODE_AGILE;
	}
}

/**
 * target_if_spectral_timestamp_war_init() - Initialize Spectral timestamp WAR
 * related info
 * @twar: Pointer to Spectral timstamp WAR related info
 *
 * Function to Initialize parameters related to Spectral timestamp WAR
 *
 * Return: void
 */
static void
target_if_spectral_timestamp_war_init(struct spectral_timestamp_war *twar)
{
	enum spectral_scan_mode smode;

	smode = SPECTRAL_SCAN_MODE_NORMAL;
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++) {
		twar->last_fft_timestamp[smode] = 0;
		twar->timestamp_war_offset[smode] = 0;
	}
	twar->target_reset_count = 0;
}

/**
 * target_if_pdev_spectral_init() - Initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Function to initialize pointer to spectral target_if internal private data
 *
 * Return: On success, pointer to Spectral target_if internal private data, on
 * failure, NULL
 */
void *
target_if_pdev_spectral_init(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;
	uint32_t target_type;
	uint32_t target_revision;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tgt_tx_ops;
	enum spectral_scan_mode smode = SPECTRAL_SCAN_MODE_NORMAL;
	QDF_STATUS status;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		spectral_err("SPECTRAL: pdev is NULL!");
		return NULL;
	}
	spectral = (struct target_if_spectral *)qdf_mem_malloc(
			sizeof(struct target_if_spectral));
	if (!spectral)
		return spectral;

	qdf_mem_zero(spectral, sizeof(struct target_if_spectral));
	/* Store pdev in Spectral */
	spectral->pdev_obj = pdev;
	spectral->vdev_id[SPECTRAL_SCAN_MODE_NORMAL] = WLAN_INVALID_VDEV_ID;
	spectral->vdev_id[SPECTRAL_SCAN_MODE_AGILE] = WLAN_INVALID_VDEV_ID;

	psoc = wlan_pdev_get_psoc(pdev);

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		qdf_mem_free(spectral);
		return NULL;
	}

	tgt_tx_ops = &tx_ops->target_tx_ops;

	if (tgt_tx_ops->tgt_get_tgt_type) {
		target_type = tgt_tx_ops->tgt_get_tgt_type(psoc);
	} else {
		qdf_mem_free(spectral);
		return NULL;
	}

	if (tgt_tx_ops->tgt_get_tgt_revision) {
		target_revision = tgt_tx_ops->tgt_get_tgt_revision(psoc);
	} else {
		qdf_mem_free(spectral);
		return NULL;
	}

	/* init the function ptr table */
	target_if_spectral_init_dummy_function_table(spectral);

	/* get spectral function table */
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	/* TODO : Should this be called here of after ath_attach ? */
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_PHYDIAG))
		spectral_info("HAL_CAP_PHYDIAG : Capable");

	/* TODO: Need to fix the capablity check for RADAR */
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_RADAR))
		spectral_info("HAL_CAP_RADAR   : Capable");

	/* TODO : Need to fix the capablity check for SPECTRAL */
	/* TODO : Should this be called here of after ath_attach ? */
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_SPECTRAL_SCAN))
		spectral_info("HAL_CAP_SPECTRAL_SCAN : Capable");

	qdf_spinlock_create(&spectral->spectral_lock);
	qdf_spinlock_create(&spectral->noise_pwr_reports_lock);
	target_if_spectral_clear_stats(spectral);

	if (target_type == TARGET_TYPE_QCA8074 ||
	    target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCA6018 ||
	    target_type == TARGET_TYPE_QCA5018 ||
	    target_type == TARGET_TYPE_QCA6390 ||
	    target_type == TARGET_TYPE_QCN9100 ||
	    target_type == TARGET_TYPE_QCA6490 ||
	    target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCA6750)
		spectral->direct_dma_support = true;

	target_if_spectral_len_adj_swar_init(&spectral->len_adj_swar,
					     target_type);
	target_if_spectral_report_params_init(&spectral->rparams, target_type);

	if ((target_type == TARGET_TYPE_QCA8074) ||
	    (target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCA6018) ||
	    (target_type == TARGET_TYPE_QCA5018) ||
	    (target_type == TARGET_TYPE_QCN9100) ||
	    (target_type == TARGET_TYPE_QCN9000) ||
	    (target_type == TARGET_TYPE_QCA6290) ||
	    (target_type == TARGET_TYPE_QCA6390) ||
	    (target_type == TARGET_TYPE_QCA6490) ||
	    (target_type == TARGET_TYPE_QCA6750)) {
		spectral->spectral_gen = SPECTRAL_GEN3;
		spectral->hdr_sig_exp = SPECTRAL_PHYERR_SIGNATURE_GEN3;
		spectral->tag_sscan_summary_exp =
		    TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3;
		spectral->tag_sscan_fft_exp = TLV_TAG_SEARCH_FFT_REPORT_GEN3;
		spectral->tlvhdr_size = SPECTRAL_PHYERR_TLVSIZE_GEN3;
	} else {
		spectral->spectral_gen = SPECTRAL_GEN2;
		spectral->hdr_sig_exp = SPECTRAL_PHYERR_SIGNATURE_GEN2;
		spectral->tag_sscan_summary_exp =
		    TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN2;
		spectral->tag_sscan_fft_exp = TLV_TAG_SEARCH_FFT_REPORT_GEN2;
		spectral->tlvhdr_size = sizeof(struct spectral_phyerr_tlv_gen2);
	}

	status = target_if_init_spectral_param_min_max(
					&spectral->param_min_max,
					spectral->spectral_gen, target_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		spectral_err("Failed to initialize parameter min max values");
		goto fail;
	}

	target_if_init_spectral_param_properties(spectral);
	/* Init spectral capability */
	if (target_if_init_spectral_capability(spectral, target_type) !=
					QDF_STATUS_SUCCESS) {
		qdf_mem_free(spectral);
		return NULL;
	}
	if (target_if_spectral_attach_simulation(spectral) < 0)
		return NULL;

	target_if_init_spectral_ops(spectral);
	target_if_spectral_timestamp_war_init(&spectral->timestamp_war);

	/* Spectral mode specific init */
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++) {
		spectral->params_valid[smode] = false;
		qdf_spinlock_create(&spectral->param_info[smode].osps_lock);
		spectral->param_info[smode].osps_cache.osc_is_valid = 0;
	}

	target_if_spectral_register_funcs(spectral, &spectral_ops);

	if (target_if_spectral_check_hw_capability(spectral) == false) {
		goto fail;
	} else {
		/*
		 * TODO: Once the driver architecture transitions to chipset
		 * versioning based checks, reflect this here.
		 */
		spectral->is_160_format = false;
		spectral->is_lb_edge_extrabins_format = false;
		spectral->is_rb_edge_extrabins_format = false;

		if (target_type == TARGET_TYPE_QCA9984 ||
		    target_type == TARGET_TYPE_QCA9888) {
			spectral->is_160_format = true;
			spectral->is_lb_edge_extrabins_format = true;
			spectral->is_rb_edge_extrabins_format = true;
		} else  if ((target_type == TARGET_TYPE_AR900B) &&
			    (target_revision == AR900B_REV_2)) {
			spectral->is_rb_edge_extrabins_format = true;
		}

		if (target_type == TARGET_TYPE_QCA9984 ||
		    target_type == TARGET_TYPE_QCA9888)
			spectral->is_sec80_rssi_war_required = true;

		spectral->use_nl_bcast = SPECTRAL_USE_NL_BCAST;

		if (spectral->spectral_gen == SPECTRAL_GEN3)
			init_160mhz_delivery_state_machine(spectral);
	}

	return spectral;

fail:
	target_if_spectral_detach(spectral);
	return NULL;
}

/**
 * target_if_pdev_spectral_deinit() - De-initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Function to de-initialize pointer to spectral target_if internal private data
 *
 * Return: None
 */
void
target_if_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return;
	}
	target_if_spectral_detach(spectral);

	return;
}

/**
 * target_if_psoc_spectral_deinit() - De-initialize target_if Spectral
 * functionality for the given psoc
 * @psoc: Pointer to psoc object
 *
 * Function to de-initialize pointer to psoc spectral target_if internal
 * private data
 *
 * Return: None
 */
static void
target_if_psoc_spectral_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct target_if_psoc_spectral *psoc_spectral;

	if (!psoc) {
		spectral_err("psoc is null");
		return;
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("Spectral target_if psoc object is null");
		return;
	}

	qdf_mem_free(psoc_spectral);
}

/**
 * target_if_psoc_spectral_init() - Initialize target_if Spectral
 * functionality for the given psoc
 * @psoc: Pointer to psoc object
 *
 * Function to initialize pointer to psoc spectral target_if internal
 * private data
 *
 * Return: On success, pointer to Spectral psoc target_if internal
 * private data, on failure, NULL
 */
static void *
target_if_psoc_spectral_init(struct wlan_objmgr_psoc *psoc)
{
	struct target_if_psoc_spectral *psoc_spectral = NULL;

	if (!psoc) {
		spectral_err("psoc is null");
		goto fail;
	}

	psoc_spectral = (struct target_if_psoc_spectral *)qdf_mem_malloc(
			sizeof(struct target_if_psoc_spectral));
	if (!psoc_spectral) {
		spectral_err("Spectral lmac psoc object allocation failed");
		goto fail;
	}

	psoc_spectral->psoc_obj = psoc;

	return psoc_spectral;

fail:
	if (psoc_spectral)
		target_if_psoc_spectral_deinit(psoc);

	return psoc_spectral;
}

/* target_if_spectral_find_agile_width() - Given a channel width enum, find the
 * corresponding translation for Agile channel width.
 * @spectral: pointer to Spectral object
 * @chwidth: operating channel width
 * @is_80_80_agile: Indicates an 80+80 agile Scan request
 *
 * Return: The translated channel width enum.
 */
static enum phy_ch_width
target_if_spectral_find_agile_width(struct target_if_spectral *spectral,
				    enum phy_ch_width chwidth,
				    bool is_80_80_agile)
{
	enum phy_ch_width agile_width;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	if (!spectral) {
		spectral_err("Spectral object is null");
		return CH_WIDTH_INVALID;
	}

	pdev =  spectral->pdev_obj;
	if (!pdev) {
		spectral_err("pdev is null");
		return CH_WIDTH_INVALID;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return CH_WIDTH_INVALID;
	}

	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		agile_width = CH_WIDTH_20MHZ;
		break;

	case CH_WIDTH_40MHZ:
		agile_width = CH_WIDTH_40MHZ;
		break;

	case CH_WIDTH_80MHZ:
		agile_width = CH_WIDTH_80MHZ;
		break;

	case CH_WIDTH_80P80MHZ:
		if (wlan_psoc_nif_fw_ext_cap_get(psoc,
		    WLAN_SOC_RESTRICTED_80P80_SUPPORT) && !is_80_80_agile)
			agile_width = CH_WIDTH_160MHZ;
		else
			agile_width = CH_WIDTH_80P80MHZ;

		if (agile_width > spectral->rparams.max_agile_ch_width)
			agile_width = spectral->rparams.max_agile_ch_width;
		break;

	case CH_WIDTH_160MHZ:
		if (wlan_psoc_nif_fw_ext_cap_get(psoc,
		    WLAN_SOC_RESTRICTED_80P80_SUPPORT) && is_80_80_agile)
			agile_width = CH_WIDTH_80P80MHZ;
		else
			agile_width = CH_WIDTH_160MHZ;

		if (agile_width > spectral->rparams.max_agile_ch_width)
			agile_width = spectral->rparams.max_agile_ch_width;
		break;

	default:
		spectral_err("Invalid channel width %d", chwidth);
		agile_width = CH_WIDTH_INVALID;
		break;
	}

	return agile_width;
}

/**
 * target_if_calculate_center_freq() - Helper routine to
 * check whether given frequency is center frequency of a
 * WLAN channel
 *
 * @spectral: Pointer to Spectral object
 * @chan_freq: Center frequency of a WLAN channel
 * @is_valid: Indicates whether given frequency is valid
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_is_center_freq_of_any_chan(struct wlan_objmgr_pdev *pdev,
				     uint32_t chan_freq,
				     bool *is_valid)
{
	struct regulatory_channel *cur_chan_list;
	int i;

	if (!pdev) {
		spectral_err("pdev object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!is_valid) {
		spectral_err("is valid argument is null");
		return QDF_STATUS_E_FAILURE;
	}

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*cur_chan_list));
	if (!cur_chan_list)
		return QDF_STATUS_E_FAILURE;

	if (wlan_reg_get_current_chan_list(
			pdev, cur_chan_list) != QDF_STATUS_SUCCESS) {
		spectral_err("Failed to get cur_chan list");
		qdf_mem_free(cur_chan_list);
		return QDF_STATUS_E_FAILURE;
	}

	*is_valid = false;
	for (i = 0; i < NUM_CHANNELS; i++) {
		uint32_t flags;
		uint32_t center_freq;

		flags = cur_chan_list[i].chan_flags;
		center_freq = cur_chan_list[i].center_freq;

		if (!(flags & REGULATORY_CHAN_DISABLED) &&
		    (center_freq == chan_freq)) {
			*is_valid = true;
			break;
		}
	}

	qdf_mem_free(cur_chan_list);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_calculate_center_freq() - Helper routine to
 * find the center frequency of the agile span from a
 * WLAN channel center frequency
 *
 * @spectral: Pointer to Spectral object
 * @ch_width: Channel width array
 * @chan_freq: Center frequency of a WLAN channel
 * @center_freq: Pointer to center frequency
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_calculate_center_freq(struct target_if_spectral *spectral,
				enum phy_ch_width *ch_width,
				uint16_t chan_freq,
				uint16_t *center_freq)
{
	enum phy_ch_width agile_ch_width;

	if (!spectral) {
		spectral_err("spectral target if object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!ch_width) {
		spectral_err("Channel width array is null");
		return QDF_STATUS_E_INVAL;
	}
	agile_ch_width = ch_width[SPECTRAL_SCAN_MODE_AGILE];

	if (!center_freq) {
		spectral_err("center_freq argument is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (agile_ch_width == CH_WIDTH_20MHZ) {
		*center_freq = chan_freq;
	} else {
		uint16_t start_freq;
		uint16_t end_freq;
		const struct bonded_channel_freq *bonded_chan_ptr = NULL;
		enum channel_state state;

		state = wlan_reg_get_5g_bonded_channel_and_state_for_freq
			(spectral->pdev_obj, chan_freq, agile_ch_width,
			 &bonded_chan_ptr);
		if (state == CHANNEL_STATE_DISABLE ||
		    state == CHANNEL_STATE_INVALID) {
			spectral_err("Channel state is disable or invalid");
			return QDF_STATUS_E_FAILURE;
		}
		if (!bonded_chan_ptr) {
			spectral_err("Bonded channel is not found");
			return QDF_STATUS_E_FAILURE;
		}
		start_freq = bonded_chan_ptr->start_freq;
		end_freq = bonded_chan_ptr->end_freq;
		*center_freq = (start_freq + end_freq) >> 1;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_validate_center_freq() - Helper routine to
 * validate user provided agile center frequency
 *
 * @spectral: Pointer to Spectral object
 * @ch_width: Channel width array
 * @center_freq: User provided agile span center frequency
 * @is_valid: Indicates whether agile span center frequency is valid
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_validate_center_freq(struct target_if_spectral *spectral,
			       enum phy_ch_width *ch_width,
			       uint16_t center_freq,
			       bool *is_valid)
{
	enum phy_ch_width agile_ch_width;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	if (!spectral) {
		spectral_err("spectral target if object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!ch_width) {
		spectral_err("channel width array is null");
		return QDF_STATUS_E_INVAL;
	}
	agile_ch_width = ch_width[SPECTRAL_SCAN_MODE_AGILE];

	if (!is_valid) {
		spectral_err("is_valid argument is null");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = spectral->pdev_obj;

	if (agile_ch_width == CH_WIDTH_20MHZ) {
		status = target_if_is_center_freq_of_any_chan
				(pdev, center_freq, is_valid);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;
	} else {
		uint16_t start_freq;
		uint16_t end_freq;
		const struct bonded_channel_freq *bonded_chan_ptr = NULL;
		bool is_chan;

		status = target_if_is_center_freq_of_any_chan
				(pdev, center_freq + FREQ_OFFSET_10MHZ,
				 &is_chan);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;

		if (is_chan) {
			uint32_t calulated_center_freq;
			enum channel_state st;

			st = wlan_reg_get_5g_bonded_channel_and_state_for_freq
				(pdev, center_freq + FREQ_OFFSET_10MHZ,
				 agile_ch_width,
				 &bonded_chan_ptr);
			if (st == CHANNEL_STATE_DISABLE ||
			    st == CHANNEL_STATE_INVALID) {
				spectral_err("Channel state disable/invalid");
				return QDF_STATUS_E_FAILURE;
			}
			if (!bonded_chan_ptr) {
				spectral_err("Bonded channel is not found");
				return QDF_STATUS_E_FAILURE;
			}
			start_freq = bonded_chan_ptr->start_freq;
			end_freq = bonded_chan_ptr->end_freq;
			calulated_center_freq = (start_freq + end_freq) >> 1;
			*is_valid = (center_freq == calulated_center_freq);
		} else {
			*is_valid = false;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_is_agile_span_overlap_with_operating_span() - Helper routine to
 * check whether agile span overlaps with current operating band.
 *
 * @spectral: Pointer to Spectral object
 * @ch_width: Channel width array
 * @center_freq: Agile span center frequency
 * @is_overlapping: Indicates whether Agile span overlaps with operating span
 *
 * Helper routine to check whether agile span overlaps with current
 * operating band.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_is_agile_span_overlap_with_operating_span
			(struct target_if_spectral *spectral,
			 enum phy_ch_width *ch_width,
			 struct spectral_config_frequency *center_freq,
			 bool *is_overlapping)
{
	enum phy_ch_width op_ch_width;
	enum phy_ch_width agile_ch_width;
	const struct bonded_channel_freq *bonded_chan_ptr = NULL;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	int16_t chan_freq;
	uint32_t op_start_freq;
	uint32_t op_end_freq;
	uint32_t agile_start_freq;
	uint32_t agile_end_freq;
	uint32_t cfreq2;

	if (!spectral) {
		spectral_err("Spectral object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev  = spectral->pdev_obj;
	if (!pdev) {
		spectral_err("pdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!ch_width) {
		spectral_err("channel width array is null");
		return QDF_STATUS_E_FAILURE;
	}
	op_ch_width = ch_width[SPECTRAL_SCAN_MODE_NORMAL];
	if (op_ch_width == CH_WIDTH_INVALID) {
		spectral_err("Invalid channel width");
		return QDF_STATUS_E_INVAL;
	}
	agile_ch_width = ch_width[SPECTRAL_SCAN_MODE_AGILE];
	if (agile_ch_width == CH_WIDTH_INVALID) {
		spectral_err("Invalid channel width");
		return QDF_STATUS_E_INVAL;
	}

	if (!is_overlapping) {
		spectral_err("Argument(is_overlapping) is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	*is_overlapping = false;

	vdev = target_if_spectral_get_vdev(spectral, SPECTRAL_SCAN_MODE_AGILE);
	if (!vdev) {
		spectral_err("vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	chan_freq = target_if_vdev_get_chan_freq(vdev);
	cfreq2 = target_if_vdev_get_chan_freq_seg2(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
	if (cfreq2 < 0) {
		spectral_err("cfreq2 is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	if (op_ch_width == CH_WIDTH_20MHZ) {
		op_start_freq = chan_freq - FREQ_OFFSET_10MHZ;
		op_end_freq = chan_freq + FREQ_OFFSET_10MHZ;
	} else {
		enum channel_state state;

		state = wlan_reg_get_5g_bonded_channel_and_state_for_freq
			(pdev, chan_freq, op_ch_width, &bonded_chan_ptr);
		if (state == CHANNEL_STATE_DISABLE ||
		    state == CHANNEL_STATE_INVALID) {
			spectral_err("Channel state is disable or invalid");
			return QDF_STATUS_E_FAILURE;
		}
		if (!bonded_chan_ptr) {
			spectral_err("Bonded channel is not found");
			return QDF_STATUS_E_FAILURE;
		}
		op_start_freq = bonded_chan_ptr->start_freq - FREQ_OFFSET_10MHZ;
		op_end_freq = bonded_chan_ptr->end_freq - FREQ_OFFSET_10MHZ;
	}

	if (agile_ch_width == CH_WIDTH_80P80MHZ) {
		agile_start_freq = center_freq->cfreq1 - FREQ_OFFSET_40MHZ;
		agile_end_freq = center_freq->cfreq1 + FREQ_OFFSET_40MHZ;
		if (agile_end_freq > op_start_freq &&
		    op_end_freq > agile_start_freq)
			*is_overlapping = true;

		agile_start_freq = center_freq->cfreq2 - FREQ_OFFSET_40MHZ;
		agile_end_freq = center_freq->cfreq2 + FREQ_OFFSET_40MHZ;
		if (agile_end_freq > op_start_freq &&
		    op_end_freq > agile_start_freq)
			*is_overlapping = true;
	} else {
		agile_start_freq = center_freq->cfreq1 -
				(wlan_reg_get_bw_value(agile_ch_width) >> 1);
		agile_end_freq = center_freq->cfreq1 +
				(wlan_reg_get_bw_value(agile_ch_width) >> 1);
		if (agile_end_freq > op_start_freq &&
		    op_end_freq > agile_start_freq)
			*is_overlapping = true;
	}

	if (op_ch_width == CH_WIDTH_80P80MHZ) {
		uint32_t sec80_start_feq;
		uint32_t sec80_end_freq;

		sec80_start_feq = cfreq2 - FREQ_OFFSET_40MHZ;
		sec80_end_freq = cfreq2 + FREQ_OFFSET_40MHZ;

		if (agile_ch_width == CH_WIDTH_80P80MHZ) {
			agile_start_freq =
					center_freq->cfreq1 - FREQ_OFFSET_40MHZ;
			agile_end_freq =
					center_freq->cfreq1 + FREQ_OFFSET_40MHZ;
			if (agile_end_freq > sec80_start_feq &&
			    sec80_end_freq > agile_start_freq)
				*is_overlapping = true;

			agile_start_freq =
					center_freq->cfreq2 - FREQ_OFFSET_40MHZ;
			agile_end_freq =
					center_freq->cfreq2 + FREQ_OFFSET_40MHZ;
			if (agile_end_freq > sec80_start_feq &&
			    sec80_end_freq > agile_start_freq)
				*is_overlapping = true;
		} else {
			agile_start_freq = center_freq->cfreq1 -
				(wlan_reg_get_bw_value(agile_ch_width) >> 1);
			agile_end_freq = center_freq->cfreq1 +
				(wlan_reg_get_bw_value(agile_ch_width) >> 1);
			if (agile_end_freq > sec80_start_feq &&
			    sec80_end_freq > agile_start_freq)
				*is_overlapping = true;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_populate_chwidth() - Helper routine to
 * populate channel width for different Spectral modes
 *
 * @spectral: Pointer to Spectral object
 * @ch_width: Channel width array
 * @is_80_80_agile: Indicates whether 80+80 agile scan is requested
 *
 * Helper routine to populate channel width for different Spectral modes
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_spectral_populate_chwidth(struct target_if_spectral *spectral,
				    enum phy_ch_width *ch_width,
				    bool is_80_80_agile) {
	struct wlan_objmgr_vdev *vdev;
	enum spectral_scan_mode smode;
	enum phy_ch_width vdev_ch_width;

	smode = SPECTRAL_SCAN_MODE_NORMAL;
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
		ch_width[smode] = CH_WIDTH_INVALID;

	if (!spectral) {
		spectral_err("Spectral object is null");
		return QDF_STATUS_E_INVAL;
	}

	vdev = target_if_spectral_get_vdev(spectral, SPECTRAL_SCAN_MODE_NORMAL);
	if (!vdev) {
		spectral_err("vdev is null");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_ch_width = target_if_vdev_get_ch_width(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
	if (vdev_ch_width == CH_WIDTH_INVALID) {
		spectral_err("Invalid channel width %d", vdev_ch_width);
		return QDF_STATUS_E_FAILURE;
	}

	ch_width[SPECTRAL_SCAN_MODE_NORMAL] = vdev_ch_width;
	ch_width[SPECTRAL_SCAN_MODE_AGILE] =
		target_if_spectral_find_agile_width(spectral, vdev_ch_width,
						    is_80_80_agile);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_is_valid_80p80_freq() - API to check whether given
 * (cfreq1, cfreq2) pair forms a valid 80+80 combination
 * @pdev: pointer to pdev
 * @cfreq1: center frequency 1
 * @cfreq2: center frequency 2
 *
 * API to check whether given (cfreq1, cfreq2) pair forms a valid 80+80
 * combination
 *
 * Return: true or false
 */
static bool
target_if_spectral_is_valid_80p80_freq(struct wlan_objmgr_pdev *pdev,
				       uint32_t cfreq1, uint32_t cfreq2)
{
	struct ch_params ch_params;
	enum channel_state chan_state1;
	enum channel_state chan_state2;
	struct wlan_objmgr_psoc *psoc;

	qdf_assert_always(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	qdf_assert_always(psoc);

	/* In restricted 80P80 MHz enabled, only one 80+80 MHz
	 * channel is supported with cfreq=5690 and cfreq=5775.
	 */
	if (wlan_psoc_nif_fw_ext_cap_get(
				psoc, WLAN_SOC_RESTRICTED_80P80_SUPPORT))
		return CHAN_WITHIN_RESTRICTED_80P80(cfreq1, cfreq2);

	ch_params.center_freq_seg1 = wlan_reg_freq_to_chan(pdev, cfreq2);
	ch_params.mhz_freq_seg1 = cfreq2;
	ch_params.ch_width = CH_WIDTH_80P80MHZ;
	wlan_reg_set_channel_params_for_freq(pdev, cfreq1 - FREQ_OFFSET_10MHZ,
					     0, &ch_params);

	if (ch_params.ch_width != CH_WIDTH_80P80MHZ)
		return false;

	if (ch_params.mhz_freq_seg0 != cfreq1 ||
	    ch_params.mhz_freq_seg1 != cfreq2)
		return false;

	chan_state1 = wlan_reg_get_5g_bonded_channel_state_for_freq(
				pdev,
				ch_params.mhz_freq_seg0 - FREQ_OFFSET_10MHZ,
				CH_WIDTH_80MHZ);
	if ((chan_state1 == CHANNEL_STATE_DISABLE) ||
	    (chan_state1 == CHANNEL_STATE_INVALID))
		return false;

	chan_state2 = wlan_reg_get_5g_bonded_channel_state_for_freq(
				pdev,
				ch_params.mhz_freq_seg1 - FREQ_OFFSET_10MHZ,
				CH_WIDTH_80MHZ);
	if ((chan_state2 == CHANNEL_STATE_DISABLE) ||
	    (chan_state2 == CHANNEL_STATE_INVALID))
		return false;

	if (abs(ch_params.mhz_freq_seg0 - ch_params.mhz_freq_seg1) <=
	    FREQ_OFFSET_80MHZ)
		return false;

	return true;
}

/**
 * _target_if_set_spectral_config() - Set spectral config
 * @spectral:       Pointer to spectral object
 * @param: Spectral parameter id and value
 * @smode: Spectral scan mode
 * @err: Spectral error code
 *
 * API to set spectral configurations
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS
_target_if_set_spectral_config(struct target_if_spectral *spectral,
			       const struct spectral_cp_param *param,
			       const enum spectral_scan_mode smode,
			       enum spectral_cp_error_code *err)
{
	struct spectral_config params;
	struct target_if_spectral_ops *p_sops;
	struct spectral_config *sparams;
	QDF_STATUS status;
	bool is_overlapping;
	uint16_t agile_cfreq;
	bool is_valid_chan;
	struct spectral_param_min_max *param_min_max;
	enum phy_ch_width ch_width[SPECTRAL_SCAN_MODE_MAX];
	enum spectral_scan_mode m;
	struct spectral_config_frequency center_freq = {0};

	if (!err) {
		spectral_err("Error code argument is null");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	*err = SPECTRAL_SCAN_ERR_INVALID;

	if (!param) {
		spectral_err("Parameter object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!spectral) {
		spectral_err("spectral object is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	param_min_max = &spectral->param_min_max;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
		return QDF_STATUS_E_FAILURE;
	}

	sparams = &spectral->params[smode];
	m = SPECTRAL_SCAN_MODE_NORMAL;
	for (; m < SPECTRAL_SCAN_MODE_MAX; m++)
		ch_width[m] = CH_WIDTH_INVALID;

	if (!spectral->params_valid[smode]) {
		target_if_spectral_info_read(spectral,
					     smode,
					     TARGET_IF_SPECTRAL_INFO_PARAMS,
					     &spectral->params[smode],
					     sizeof(spectral->params[smode]));
		spectral->params_valid[smode] = true;
	}

	switch (param->id) {
	case SPECTRAL_PARAM_FFT_PERIOD:
		sparams->ss_fft_period = param->value;
		break;
	case SPECTRAL_PARAM_SCAN_PERIOD:
		sparams->ss_period = param->value;
		break;
	case SPECTRAL_PARAM_SCAN_COUNT:
		sparams->ss_count = param->value;
		break;
	case SPECTRAL_PARAM_SHORT_REPORT:
		sparams->ss_short_report = (!!param->value) ? true : false;
		break;
	case SPECTRAL_PARAM_SPECT_PRI:
		sparams->ss_spectral_pri = (!!param->value) ? true : false;
		break;
	case SPECTRAL_PARAM_FFT_SIZE:
		status = target_if_spectral_populate_chwidth
			(spectral, ch_width, spectral->params
			 [SPECTRAL_SCAN_MODE_AGILE].ss_frequency.cfreq2 > 0);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;
		if ((param->value < param_min_max->fft_size_min) ||
		    (param->value > param_min_max->fft_size_max
		    [ch_width[smode]])) {
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			return QDF_STATUS_E_FAILURE;
		}
		sparams->ss_fft_size = param->value;
		break;
	case SPECTRAL_PARAM_GC_ENA:
		sparams->ss_gc_ena = !!param->value;
		break;
	case SPECTRAL_PARAM_RESTART_ENA:
		sparams->ss_restart_ena = !!param->value;
		break;
	case SPECTRAL_PARAM_NOISE_FLOOR_REF:
		sparams->ss_noise_floor_ref = param->value;
		break;
	case SPECTRAL_PARAM_INIT_DELAY:
		sparams->ss_init_delay = param->value;
		break;
	case SPECTRAL_PARAM_NB_TONE_THR:
		sparams->ss_nb_tone_thr = param->value;
		break;
	case SPECTRAL_PARAM_STR_BIN_THR:
		sparams->ss_str_bin_thr = param->value;
		break;
	case SPECTRAL_PARAM_WB_RPT_MODE:
		sparams->ss_wb_rpt_mode = !!param->value;
		break;
	case SPECTRAL_PARAM_RSSI_RPT_MODE:
		sparams->ss_rssi_rpt_mode = !!param->value;
		break;
	case SPECTRAL_PARAM_RSSI_THR:
		sparams->ss_rssi_thr = param->value;
		break;
	case SPECTRAL_PARAM_PWR_FORMAT:
		sparams->ss_pwr_format = !!param->value;
		break;
	case SPECTRAL_PARAM_RPT_MODE:
		if ((param->value < SPECTRAL_PARAM_RPT_MODE_MIN) ||
		    (param->value > SPECTRAL_PARAM_RPT_MODE_MAX)) {
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			return QDF_STATUS_E_FAILURE;
		}
		sparams->ss_rpt_mode = param->value;
		break;
	case SPECTRAL_PARAM_BIN_SCALE:
		sparams->ss_bin_scale = param->value;
		break;
	case SPECTRAL_PARAM_DBM_ADJ:
		sparams->ss_dbm_adj = !!param->value;
		break;
	case SPECTRAL_PARAM_CHN_MASK:
		sparams->ss_chn_mask = param->value;
		break;
	case SPECTRAL_PARAM_FREQUENCY:
		status = target_if_spectral_populate_chwidth(
				spectral, ch_width, param->freq.cfreq2 > 0);
		if (QDF_IS_STATUS_ERROR(status)) {
			spectral_err("Failed to populate channel width");
			return QDF_STATUS_E_FAILURE;
		}

		if (ch_width[smode] != CH_WIDTH_80P80MHZ &&
		    param->freq.cfreq2) {
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			spectral_err("Non zero cfreq2 expected for 80p80 only");
			return QDF_STATUS_E_INVAL;
		}

		if (ch_width[smode] == CH_WIDTH_80P80MHZ &&
		    !param->freq.cfreq2) {
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			spectral_err("Non zero cfreq2 expected for 80p80");
			return QDF_STATUS_E_INVAL;
		}

		status = target_if_is_center_freq_of_any_chan
				(spectral->pdev_obj, param->freq.cfreq1,
				 &is_valid_chan);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;

		if (is_valid_chan) {
			status = target_if_calculate_center_freq(
							spectral, ch_width,
							param->freq.cfreq1,
							&agile_cfreq);
			if (QDF_IS_STATUS_ERROR(status)) {
				*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			bool is_valid_agile_cfreq;

			status = target_if_validate_center_freq
				(spectral, ch_width, param->freq.cfreq1,
				 &is_valid_agile_cfreq);
			if (QDF_IS_STATUS_ERROR(status))
				return QDF_STATUS_E_FAILURE;

			if (!is_valid_agile_cfreq) {
				*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
				spectral_err("Invalid agile center frequency");
				return QDF_STATUS_E_FAILURE;
			}

			agile_cfreq = param->freq.cfreq1;
		}
		center_freq.cfreq1 = agile_cfreq;

		if (ch_width[smode] == CH_WIDTH_80P80MHZ) {
			status = target_if_is_center_freq_of_any_chan
					(spectral->pdev_obj, param->freq.cfreq2,
					 &is_valid_chan);
			if (QDF_IS_STATUS_ERROR(status))
				return QDF_STATUS_E_FAILURE;

			if (is_valid_chan) {
				status = target_if_calculate_center_freq(
						spectral, ch_width,
						param->freq.cfreq2,
						&agile_cfreq);
				if (QDF_IS_STATUS_ERROR(status)) {
					*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
					return QDF_STATUS_E_FAILURE;
				}
			} else {
				bool is_valid_agile_cfreq;

				status = target_if_validate_center_freq
					(spectral, ch_width, param->freq.cfreq2,
					 &is_valid_agile_cfreq);
				if (QDF_IS_STATUS_ERROR(status))
					return QDF_STATUS_E_FAILURE;

				if (!is_valid_agile_cfreq) {
					*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
					spectral_err("Invalid agile center frequency");
					return QDF_STATUS_E_FAILURE;
				}

				agile_cfreq = param->freq.cfreq2;
			}
			center_freq.cfreq2 = agile_cfreq;
		}

		status = target_if_is_agile_span_overlap_with_operating_span
				(spectral, ch_width,
				 &center_freq, &is_overlapping);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;

		if (is_overlapping) {
			spectral_err("Agile freq %u, %u overlaps with operating span",
				     center_freq.cfreq1, center_freq.cfreq2);
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			return QDF_STATUS_E_FAILURE;
		}

		if (ch_width[smode] == CH_WIDTH_80P80MHZ) {
			bool is_valid_80p80;

			is_valid_80p80 = target_if_spectral_is_valid_80p80_freq(
						spectral->pdev_obj,
						center_freq.cfreq1,
						center_freq.cfreq2);

			if (!is_valid_80p80) {
				spectral_err("Agile freq %u, %u is invalid 80+80 combination",
					     center_freq.cfreq1,
					     center_freq.cfreq2);
				*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
				return QDF_STATUS_E_FAILURE;
			}
		}

		sparams->ss_frequency.cfreq1 = center_freq.cfreq1;
		sparams->ss_frequency.cfreq2 = center_freq.cfreq2;

		break;
	}

	p_sops->configure_spectral(spectral, sparams, smode);
	/* only to validate the writes */
	p_sops->get_spectral_config(spectral, &params, smode);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_set_spectral_config(struct wlan_objmgr_pdev *pdev,
			      const struct spectral_cp_param *param,
			      const enum spectral_scan_mode smode,
			      enum spectral_cp_error_code *err)
{
	enum spectral_scan_mode mode = SPECTRAL_SCAN_MODE_NORMAL;
	struct target_if_spectral *spectral;
	QDF_STATUS status;

	if (!err) {
		spectral_err("Error code argument is null");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	*err = SPECTRAL_SCAN_ERR_INVALID;

	if (!pdev) {
		spectral_err("pdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("spectral object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!param) {
		spectral_err("parameter object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
		return QDF_STATUS_E_FAILURE;
	}

	if (!spectral->properties[smode][param->id].supported) {
		spectral_err("Spectral parameter(%u) unsupported for mode %u",
			     param->id, smode);
		*err = SPECTRAL_SCAN_ERR_PARAM_UNSUPPORTED;
		return QDF_STATUS_E_FAILURE;
	}

	if (spectral->properties[smode][param->id].common_all_modes) {
		spectral_warn("Setting Spectral parameter %u for all modes",
			      param->id);
		for (; mode < SPECTRAL_SCAN_MODE_MAX; mode++) {
			status = _target_if_set_spectral_config
						(spectral, param, mode, err);
			if (QDF_IS_STATUS_ERROR(status))
				return QDF_STATUS_E_FAILURE;
		}
		return QDF_STATUS_SUCCESS;
	}

	return _target_if_set_spectral_config(spectral, param, smode, err);
}

/**
 * target_if_get_fft_bin_count() - Get fft bin count for a given fft length
 * @fft_len: FFT length
 * @pdev: Pointer to pdev object
 *
 * API to get fft bin count for a given fft length
 *
 * Return: FFt bin count
 */
static int
target_if_get_fft_bin_count(int fft_len)
{
	int bin_count = 0;

	switch (fft_len) {
	case 5:
		bin_count = 16;
		break;
	case 6:
		bin_count = 32;
		break;
	case 7:
		bin_count = 64;
		break;
	case 8:
		bin_count = 128;
		break;
	case 9:
		bin_count = 256;
		break;
	default:
		break;
	}

	return bin_count;
}

/**
 * target_if_init_upper_lower_flags() - Initializes control and extension
 * segment flags
 * @spectral: pointer to target if spectral object
 * @smode: Spectral scan mode
 *
 * API to initialize the control and extension flags with the lower/upper
 * segment based on the HT mode
 *
 * Return: FFt bin count
 */
static void
target_if_init_upper_lower_flags(struct target_if_spectral *spectral,
				 enum spectral_scan_mode smode)
{
	int current_channel = 0;
	int ext_channel = 0;
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return;
	}
	current_channel = p_sops->get_current_channel(spectral, smode);
	ext_channel = p_sops->get_extension_channel(spectral, smode);

	if ((current_channel == 0) || (ext_channel == 0))
		return;

	if (spectral->sc_spectral_20_40_mode) {
		/* HT40 mode */
		if (ext_channel < current_channel) {
			spectral->lower_is_extension = 1;
			spectral->upper_is_control = 1;
			spectral->lower_is_control = 0;
			spectral->upper_is_extension = 0;
		} else {
			spectral->lower_is_extension = 0;
			spectral->upper_is_control = 0;
			spectral->lower_is_control = 1;
			spectral->upper_is_extension = 1;
		}
	} else {
		/* HT20 mode, lower is always control */
		spectral->lower_is_extension = 0;
		spectral->upper_is_control = 0;
		spectral->lower_is_control = 1;
		spectral->upper_is_extension = 0;
	}
}

/**
 * target_if_get_spectral_config() - Get spectral configuration
 * @pdev: Pointer to pdev object
 * @param: Pointer to spectral_config structure in which the configuration
 * should be returned
 * @smode: Spectral scan mode
 *
 * API to get the current spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS
target_if_get_spectral_config(struct wlan_objmgr_pdev *pdev,
			      struct spectral_config *param,
			      enum spectral_scan_mode smode)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return QDF_STATUS_E_FAILURE;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("p_sops is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(param, sizeof(struct spectral_config));
	p_sops->get_spectral_config(spectral, param, smode);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_get_num_detectors() - Get number of Spectral detectors
 * @spectral: Pointer to target if Spectral object
 * @ch_width: channel width
 * @num_detectors: Pointer to the variable to store number of Spectral detectors
 *
 * API to get number of Spectral detectors used for scan in the given channel
 * width.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_INVAL on failure
 */
static QDF_STATUS
target_if_spectral_get_num_detectors(struct target_if_spectral *spectral,
				     enum phy_ch_width ch_width,
				     uint32_t *num_detectors)
{
	if (!spectral) {
		spectral_err("target if spectral object is null");
		return QDF_STATUS_E_INVAL;
	}

	if (ch_width >= CH_WIDTH_INVALID) {
		spectral_err("Invalid channel width %d", ch_width);
		return QDF_STATUS_E_INVAL;
	}

	if (!num_detectors) {
		spectral_err("Invalid argument, number of detectors");
		return QDF_STATUS_E_INVAL;
	}

	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		*num_detectors = spectral->capability.num_detectors_20mhz;
		break;

	case CH_WIDTH_40MHZ:
		*num_detectors = spectral->capability.num_detectors_40mhz;
		break;

	case CH_WIDTH_80MHZ:
		*num_detectors = spectral->capability.num_detectors_80mhz;
		break;

	case CH_WIDTH_160MHZ:
		*num_detectors = spectral->capability.num_detectors_160mhz;
		break;

	case CH_WIDTH_80P80MHZ:
		*num_detectors = spectral->capability.num_detectors_80p80mhz;
		break;

	default:
		spectral_err("Unsupported channel width %d", ch_width);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_finite_scan_init() - Initializations required for finite
 * Spectral scan
 * @spectral: Pointer to target of Spctral object
 * @smode: Spectral scan mode
 *
 * This routine initializes the finite Spectral scan. Finite Spectral scan is
 * triggered by configuring a non zero scan count.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS
target_if_spectral_finite_scan_init(struct target_if_spectral *spectral,
				    enum spectral_scan_mode smode)
{
	struct target_if_finite_spectral_scan_params *finite_scan;
	enum phy_ch_width ch_width;
	uint32_t num_detectors;
	QDF_STATUS status;
	uint16_t sscan_count;

	if (!spectral) {
		spectral_err("target if spectral object is null");
		return QDF_STATUS_E_INVAL;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode");
		return QDF_STATUS_E_INVAL;
	}

	ch_width = spectral->ch_width[smode];
	status = target_if_spectral_get_num_detectors(spectral, ch_width,
						      &num_detectors);

	if (QDF_IS_STATUS_ERROR(status)) {
		spectral_err("Failed to get number of detectors");
		return QDF_STATUS_E_FAILURE;
	}

	finite_scan = &spectral->finite_scan[smode];
	sscan_count =  spectral->params[smode].ss_count;

	finite_scan->finite_spectral_scan =  true;
	finite_scan->num_reports_expected = num_detectors * sscan_count;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_scan_enable_params() - Enable use of desired Spectral
 *                                           parameters
 * @spectral: Pointer to Spectral target_if internal private data
 * @spectral_params: Pointer to Spectral parameters
 * @smode: Spectral scan mode
 * @err: Spectral error code
 *
 * Enable use of desired Spectral parameters by configuring them into HW, and
 * starting Spectral scan
 *
 * Return: 0 on success, 1 on failure
 */
int
target_if_spectral_scan_enable_params(struct target_if_spectral *spectral,
				      struct spectral_config *spectral_params,
				      enum spectral_scan_mode smode,
				      enum spectral_cp_error_code *err)
{
	int extension_channel = 0;
	int current_channel = 0;
	struct target_if_spectral_ops *p_sops = NULL;
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	if (!spectral) {
		spectral_err("Spectral LMAC object is NULL");
		return 1;
	}

	pdev =  spectral->pdev_obj;
	if (!pdev) {
		spectral_err("pdev is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return 1;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("p_sops is NULL");
		return 1;
	}

	spectral->sc_spectral_noise_pwr_cal =
	    spectral_params->ss_spectral_pri ? 1 : 0;

	/* check if extension channel is present */
	extension_channel = p_sops->get_extension_channel(spectral, smode);
	current_channel = p_sops->get_current_channel(spectral, smode);

	status = target_if_spectral_populate_chwidth(
			spectral, spectral->ch_width,
			spectral->params[SPECTRAL_SCAN_MODE_AGILE].
			ss_frequency.cfreq2 > 0);
	if (QDF_IS_STATUS_ERROR(status)) {
		spectral_err("Failed to get channel widths");
		return 1;
	}

	if (spectral->capability.advncd_spectral_cap) {
		spectral->lb_edge_extrabins = 0;
		spectral->rb_edge_extrabins = 0;

		if (spectral->is_lb_edge_extrabins_format &&
		    spectral->params[smode].ss_rpt_mode == 2) {
			spectral->lb_edge_extrabins = 4;
		}

		if (spectral->is_rb_edge_extrabins_format &&
		    spectral->params[smode].ss_rpt_mode == 2) {
			spectral->rb_edge_extrabins = 4;
		}

		if (spectral->ch_width[smode] == CH_WIDTH_20MHZ) {
			spectral->sc_spectral_20_40_mode = 0;

			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
			    current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

		} else if (spectral->ch_width[smode] == CH_WIDTH_40MHZ) {
			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 1;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			if (extension_channel < current_channel) {
				spectral->classifier_params.lower_chan_in_mhz =
				    extension_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    current_channel;
			} else {
				spectral->classifier_params.lower_chan_in_mhz =
				    current_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    extension_channel;
			}

		} else if (spectral->ch_width[smode] == CH_WIDTH_80MHZ) {
			/* Set the FFT Size */
			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 0;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
			    current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			if (extension_channel < current_channel) {
				spectral->classifier_params.lower_chan_in_mhz =
				    extension_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    current_channel;
			} else {
				spectral->classifier_params.lower_chan_in_mhz =
				    current_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    extension_channel;
			}

		} else if (is_ch_width_160_or_80p80(
			   spectral->ch_width[smode])) {
			/* Set the FFT Size */

			/* The below applies to both 160 and 80+80 cases */

			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 0;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params[smode].ss_fft_size);

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
			    current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			if (extension_channel < current_channel) {
				spectral->classifier_params.lower_chan_in_mhz =
				    extension_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    current_channel;
			} else {
				spectral->classifier_params.lower_chan_in_mhz =
				    current_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    extension_channel;
			}
		}

		if (spectral->spectral_numbins) {
			spectral->spectral_numbins +=
			    spectral->lb_edge_extrabins;
			spectral->spectral_numbins +=
			    spectral->rb_edge_extrabins;
		}

		if (spectral->spectral_fft_len) {
			spectral->spectral_fft_len +=
			    spectral->lb_edge_extrabins;
			spectral->spectral_fft_len +=
			    spectral->rb_edge_extrabins;
		}

		if (spectral->spectral_data_len) {
			spectral->spectral_data_len +=
			    spectral->lb_edge_extrabins;
			spectral->spectral_data_len +=
			    spectral->rb_edge_extrabins;
		}
	} else {
		/*
		 * The decision to find 20/40 mode is found based on the
		 * presence of extension channel
		 * instead of channel width, as the channel width can
		 * dynamically change
		 */

		if (extension_channel == 0) {
			spectral->spectral_numbins = SPECTRAL_HT20_NUM_BINS;
			spectral->spectral_dc_index = SPECTRAL_HT20_DC_INDEX;
			spectral->spectral_fft_len = SPECTRAL_HT20_FFT_LEN;
			spectral->spectral_data_len =
			    SPECTRAL_HT20_TOTAL_DATA_LEN;
			/* only valid in 20-40 mode */
			spectral->spectral_lower_max_index_offset = -1;
			/* only valid in 20-40 mode */
			spectral->spectral_upper_max_index_offset = -1;
			spectral->spectral_max_index_offset =
			    spectral->spectral_fft_len + 2;
			spectral->sc_spectral_20_40_mode = 0;

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
			    current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

		} else {
			spectral->spectral_numbins =
			    SPECTRAL_HT40_TOTAL_NUM_BINS;
			spectral->spectral_fft_len = SPECTRAL_HT40_FFT_LEN;
			spectral->spectral_data_len =
			    SPECTRAL_HT40_TOTAL_DATA_LEN;
			spectral->spectral_dc_index = SPECTRAL_HT40_DC_INDEX;
			/* only valid in 20 mode */
			spectral->spectral_max_index_offset = -1;
			spectral->spectral_lower_max_index_offset =
			    spectral->spectral_fft_len + 2;
			spectral->spectral_upper_max_index_offset =
			    spectral->spectral_fft_len + 5;
			spectral->sc_spectral_20_40_mode = 1;

			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			if (extension_channel < current_channel) {
				spectral->classifier_params.lower_chan_in_mhz =
				    extension_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    current_channel;
			} else {
				spectral->classifier_params.lower_chan_in_mhz =
				    current_channel;
				spectral->classifier_params.upper_chan_in_mhz =
				    extension_channel;
			}
		}
	}

	spectral->send_single_packet = 0;
	spectral->classifier_params.spectral_20_40_mode =
	    spectral->sc_spectral_20_40_mode;
	spectral->classifier_params.spectral_dc_index =
	    spectral->spectral_dc_index;
	spectral->spectral_sent_msg = 0;
	spectral->classify_scan = 0;
	spectral->num_spectral_data = 0;

	if (!p_sops->is_spectral_active(spectral, smode)) {
		p_sops->configure_spectral(spectral, spectral_params, smode);
		spectral->rparams.marker[smode].is_valid = false;

		if (spectral->params[smode].ss_count) {
			status = target_if_spectral_finite_scan_init(spectral,
								     smode);
			if (QDF_IS_STATUS_ERROR(status)) {
				spectral_err("Failed to init finite scan");
				return 1;
			}
		}
		p_sops->start_spectral_scan(spectral, smode, err);
		spectral->timestamp_war.timestamp_war_offset[smode] = 0;
		spectral->timestamp_war.last_fft_timestamp[smode] = 0;
	}

	/* get current spectral configuration */
	p_sops->get_spectral_config(spectral, &spectral->params[smode], smode);

	target_if_init_upper_lower_flags(spectral, smode);

	return 0;
}

/**
 * target_if_is_aspectral_prohibited_by_adfs() - Is Agile Spectral prohibited by
 * Agile DFS
 * @psoc: Pointer to psoc
 * @object: Pointer to pdev
 * @arg: Pointer to flag which indicates whether Agile Spectral is prohibited
 *
 * This API checks whether Agile DFS is running on any of the pdevs. If so, it
 * indicates that Agile Spectral scan is prohibited by Agile DFS.
 *
 * Return: void
 */
static void
target_if_is_aspectral_prohibited_by_adfs(struct wlan_objmgr_psoc *psoc,
					  void *object, void *arg)
{
	bool *is_aspectral_prohibited = arg;
	struct wlan_objmgr_pdev *cur_pdev = object;
	bool is_agile_dfs_enabled_cur_pdev = false;
	QDF_STATUS status;

	qdf_assert_always(is_aspectral_prohibited);
	if (*is_aspectral_prohibited)
		return;

	qdf_assert_always(psoc);
	qdf_assert_always(cur_pdev);

	status = ucfg_dfs_get_agile_precac_enable
				(cur_pdev,
				 &is_agile_dfs_enabled_cur_pdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		spectral_err("Get agile precac failed, prohibiting aSpectral");
		*is_aspectral_prohibited = true;
		return;
	}

	if (is_agile_dfs_enabled_cur_pdev) {
		spectral_err("aDFS is in progress on one of the pdevs");
		*is_aspectral_prohibited = true;
	}
}

/**
 * target_if_get_curr_band() - Get current operating band of pdev
 *
 * @pdev: pointer to pdev object
 *
 * API to get current operating band of a given pdev.
 *
 * Return: if success enum reg_wifi_band, REG_BAND_UNKNOWN in case of failure
 */
static enum reg_wifi_band
target_if_get_curr_band(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	int16_t chan_freq;
	enum reg_wifi_band cur_band;

	if (!pdev) {
		spectral_err("pdev is NULL");
		return REG_BAND_UNKNOWN;
	}

	if (vdev_id == WLAN_INVALID_VDEV_ID)
		vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_SPECTRAL_ID);
	else
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
							    WLAN_SPECTRAL_ID);
	if (!vdev) {
		spectral_debug("vdev is NULL");
		return REG_BAND_UNKNOWN;
	}
	chan_freq = target_if_vdev_get_chan_freq(vdev);
	cur_band = wlan_reg_freq_to_band(chan_freq);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	return cur_band;
}

/**
 * target_if_is_agile_scan_active_in_5g() - Is Agile Spectral scan active on
 * any of the 5G pdevs
 * @psoc: Pointer to psoc
 * @object: Pointer to pdev
 * @arg: Pointer to flag which indicates whether Agile Spectral scan is in
 *       progress in any 5G pdevs
 *
 * Return: void
 */
static void
target_if_is_agile_scan_active_in_5g(struct wlan_objmgr_psoc *psoc,
				     void *object, void *arg)
{
	enum reg_wifi_band band;
	bool *is_agile_scan_inprog_5g_pdev = arg;
	struct target_if_spectral *spectral;
	struct wlan_objmgr_pdev *cur_pdev = object;
	struct target_if_spectral_ops *p_sops;

	if (*is_agile_scan_inprog_5g_pdev)
		return;

	spectral = get_target_if_spectral_handle_from_pdev(cur_pdev);
	if (!spectral) {
		spectral_err("target if spectral handle is NULL");
		return;
	}
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	band = target_if_get_curr_band(
			cur_pdev, spectral->vdev_id[SPECTRAL_SCAN_MODE_AGILE]);
	if (band == REG_BAND_UNKNOWN) {
		spectral_debug("Failed to get current band");
		return;
	}

	if (band == REG_BAND_5G &&
	    p_sops->is_spectral_active(spectral, SPECTRAL_SCAN_MODE_AGILE))
		*is_agile_scan_inprog_5g_pdev = true;
}

/**
 * target_if_is_agile_supported_cur_chmask() - Is Agile Spectral scan supported
 * for current vdev rx chainmask.
 *
 * @spectral: Pointer to Spectral object
 * @is_supported: Pointer to is_supported
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
static QDF_STATUS
target_if_is_agile_supported_cur_chmask(struct target_if_spectral *spectral,
					bool *is_supported)
{
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_rxchainmask;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_svc_param;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap_arr = NULL;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap = NULL;
	struct wlan_psoc_host_chainmask_table *table;
	int j;
	uint32_t table_id;
	enum phy_ch_width ch_width;
	uint8_t pdev_id;

	if (!spectral) {
		spectral_err("spectral target if object is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (!is_supported) {
		spectral_err("is supported argument is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (spectral->spectral_gen <= SPECTRAL_GEN2) {
		spectral_err("HW Agile mode is not supported up to gen 2");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = spectral->pdev_obj;
	if (!pdev) {
		spectral_err("pdev is null");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = target_if_spectral_get_vdev(spectral, SPECTRAL_SCAN_MODE_AGILE);
	if (!vdev) {
		spectral_err("First vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_rxchainmask = wlan_vdev_mlme_get_rxchainmask(vdev);
	if (!vdev_rxchainmask) {
		spectral_err("vdev rx chainmask is zero");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
		return QDF_STATUS_E_FAILURE;
	}

	ch_width = target_if_vdev_get_ch_width(vdev);
	if (ch_width == CH_WIDTH_INVALID) {
		spectral_err("Invalid channel width");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);
		return QDF_STATUS_E_FAILURE;
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		spectral_err("target_psoc_info is null");
		return QDF_STATUS_E_FAILURE;
	}

	ext_svc_param = target_psoc_get_service_ext_param(tgt_psoc_info);
	if (!ext_svc_param) {
		spectral_err("Extended service ready param null");
		return QDF_STATUS_E_FAILURE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	mac_phy_cap_arr = target_psoc_get_mac_phy_cap(tgt_psoc_info);
	if (!mac_phy_cap_arr) {
		spectral_err("mac phy cap array is null");
		return QDF_STATUS_E_FAILURE;
	}

	mac_phy_cap = &mac_phy_cap_arr[pdev_id];
	if (!mac_phy_cap) {
		spectral_err("mac phy cap is null");
		return QDF_STATUS_E_FAILURE;
	}

	table_id = mac_phy_cap->chainmask_table_id;
	table =  &ext_svc_param->chainmask_table[table_id];
	if (!table) {
		spectral_err("chainmask table not found");
		return QDF_STATUS_E_FAILURE;
	}

	for (j = 0; j < table->num_valid_chainmasks; j++) {
		if (table->cap_list[j].chainmask == vdev_rxchainmask) {
			if (ch_width <= CH_WIDTH_80MHZ)
				*is_supported =
					table->cap_list[j].supports_aSpectral;
			else
				*is_supported =
				      table->cap_list[j].supports_aSpectral_160;
			break;
		}
	}

	if (j == table->num_valid_chainmasks) {
		spectral_err("vdev rx chainmask %u not found in table id = %u",
			     vdev_rxchainmask, table_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_start_spectral_scan(struct wlan_objmgr_pdev *pdev,
			      uint8_t vdev_id,
			      const enum spectral_scan_mode smode,
			      enum spectral_cp_error_code *err)
{
	struct target_if_spectral_ops *p_sops;
	struct target_if_spectral *spectral;
	struct wlan_objmgr_psoc *psoc;
	enum reg_wifi_band band;

	if (!err) {
		spectral_err("Error code argument is null");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	*err = SPECTRAL_SCAN_ERR_INVALID;

	if (!pdev) {
		spectral_err("pdev object is NUll");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral LMAC object is NUll");
		return QDF_STATUS_E_FAILURE;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	if (!p_sops) {
		spectral_err("p_sops is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (p_sops->is_spectral_active(spectral, smode)) {
		spectral_err("spectral in progress in current pdev, mode %d",
			     smode);
		return QDF_STATUS_E_FAILURE;
	}
	spectral->vdev_id[smode] = vdev_id;

	if (smode == SPECTRAL_SCAN_MODE_AGILE) {
		QDF_STATUS status;
		bool is_supported = false;

		status = target_if_is_agile_supported_cur_chmask(spectral,
								 &is_supported);
		if (QDF_IS_STATUS_ERROR(status)) {
			*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
			return QDF_STATUS_E_FAILURE;
		}

		if (!is_supported) {
			spectral_err("aSpectral unsupported for cur chainmask");
			*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
			return QDF_STATUS_E_FAILURE;
		}
	}

	band = target_if_get_curr_band(spectral->pdev_obj, vdev_id);
	if (band == REG_BAND_UNKNOWN) {
		spectral_err("Failed to get current band");
		return QDF_STATUS_E_FAILURE;
	}
	if ((band == REG_BAND_5G) && (smode == SPECTRAL_SCAN_MODE_AGILE)) {
		struct target_psoc_info *tgt_hdl;
		enum wmi_host_hw_mode_config_type mode;
		bool is_agile_scan_inprog_5g_pdev;

		tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
		if (!tgt_hdl) {
			target_if_err("target_psoc_info is null");
			return QDF_STATUS_E_FAILURE;
		}

		mode = target_psoc_get_preferred_hw_mode(tgt_hdl);
		switch (mode) {
		case WMI_HOST_HW_MODE_SBS_PASSIVE:
		case WMI_HOST_HW_MODE_SBS:
		case WMI_HOST_HW_MODE_DBS_SBS:
		case WMI_HOST_HW_MODE_DBS_OR_SBS:
			is_agile_scan_inprog_5g_pdev = false;
			wlan_objmgr_iterate_obj_list
				(psoc, WLAN_PDEV_OP,
				 target_if_is_agile_scan_active_in_5g,
				 &is_agile_scan_inprog_5g_pdev, 0,
				 WLAN_SPECTRAL_ID);
			break;
		default:
			is_agile_scan_inprog_5g_pdev = false;
			break;
		}

		if (is_agile_scan_inprog_5g_pdev) {
			spectral_err("Agile Scan in progress in one of the SBS 5G pdev");
			*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (smode == SPECTRAL_SCAN_MODE_AGILE) {
		bool is_aspectral_prohibited = false;
		QDF_STATUS status;

		status = wlan_objmgr_iterate_obj_list
				(psoc, WLAN_PDEV_OP,
				 target_if_is_aspectral_prohibited_by_adfs,
				 &is_aspectral_prohibited, 0,
				 WLAN_SPECTRAL_ID);
		if (QDF_IS_STATUS_ERROR(status)) {
			spectral_err("Failed to iterate over pdevs");
			*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
			return QDF_STATUS_E_FAILURE;
		}

		if (is_aspectral_prohibited) {
			*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
			return QDF_STATUS_E_FAILURE;
		}
	}

	if (!spectral->params_valid[smode]) {
		target_if_spectral_info_read(spectral,
					     smode,
					     TARGET_IF_SPECTRAL_INFO_PARAMS,
					     &spectral->params[smode],
					     sizeof(spectral->params[smode]));
		spectral->params_valid[smode] = true;
	}

	qdf_spin_lock(&spectral->spectral_lock);
	if (smode == SPECTRAL_SCAN_MODE_AGILE) {
		QDF_STATUS status;
		bool is_overlapping;
		enum phy_ch_width ch_width[SPECTRAL_SCAN_MODE_MAX];
		enum spectral_scan_mode m;
		enum phy_ch_width op_ch_width;
		enum phy_ch_width agile_ch_width;

		m = SPECTRAL_SCAN_MODE_NORMAL;
		for (; m < SPECTRAL_SCAN_MODE_MAX; m++)
			ch_width[m] = CH_WIDTH_INVALID;
		status = target_if_spectral_populate_chwidth
			(spectral, ch_width, spectral->params
			 [SPECTRAL_SCAN_MODE_AGILE].ss_frequency.cfreq2 > 0);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_spin_unlock(&spectral->spectral_lock);
			spectral_err("Failed to populate channel width");
			return QDF_STATUS_E_FAILURE;
		}
		op_ch_width = ch_width[SPECTRAL_SCAN_MODE_NORMAL];
		agile_ch_width = ch_width[SPECTRAL_SCAN_MODE_AGILE];

		if (!spectral->params[smode].ss_frequency.cfreq1 ||
		    (agile_ch_width == CH_WIDTH_80P80MHZ &&
		    !spectral->params[smode].ss_frequency.cfreq2)) {
			*err = SPECTRAL_SCAN_ERR_PARAM_NOT_INITIALIZED;
			qdf_spin_unlock(&spectral->spectral_lock);
			return QDF_STATUS_E_FAILURE;
		}

		status = target_if_is_agile_span_overlap_with_operating_span
				(spectral, ch_width,
				 &spectral->params[smode].ss_frequency,
				 &is_overlapping);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_spin_unlock(&spectral->spectral_lock);
			return QDF_STATUS_E_FAILURE;
		}

		if (is_overlapping) {
			*err = SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE;
			qdf_spin_unlock(&spectral->spectral_lock);
			return QDF_STATUS_E_FAILURE;
		}
	}

	target_if_spectral_scan_enable_params(spectral,
					      &spectral->params[smode], smode,
					      err);
	qdf_spin_unlock(&spectral->spectral_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_stop_spectral_scan(struct wlan_objmgr_pdev *pdev,
			     const enum spectral_scan_mode smode,
			     enum spectral_cp_error_code *err)
{
	struct target_if_spectral_ops *p_sops;
	struct target_if_spectral *spectral;

	if (!err) {
		spectral_err("Error code argument is null");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	*err = SPECTRAL_SCAN_ERR_INVALID;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		*err = SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED;
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pdev) {
		spectral_err("pdev object is NUll ");
		return QDF_STATUS_E_FAILURE;
	}
	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral LMAC object is NUll ");
		return QDF_STATUS_E_FAILURE;
	}
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	qdf_spin_lock(&spectral->spectral_lock);
	p_sops->stop_spectral_scan(spectral, smode);
	if (spectral->classify_scan) {
		/* TODO : Check if this logic is necessary */
		spectral->detects_control_channel = 0;
		spectral->detects_extension_channel = 0;
		spectral->detects_above_dc = 0;
		spectral->detects_below_dc = 0;
		spectral->classify_scan = 0;
	}

	spectral->send_single_packet = 0;
	spectral->sc_spectral_scan = 0;

	qdf_spin_unlock(&spectral->spectral_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_is_spectral_active() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * API to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool
target_if_is_spectral_active(struct wlan_objmgr_pdev *pdev,
			     const enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return QDF_STATUS_E_FAILURE;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("p_sops is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	return p_sops->is_spectral_active(spectral, smode);
}

/**
 * target_if_is_spectral_enabled() - Get whether Spectral is enabled
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * API to get whether Spectral is enabled
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
bool
target_if_is_spectral_enabled(struct wlan_objmgr_pdev *pdev,
			      enum spectral_scan_mode smode)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return QDF_STATUS_E_FAILURE;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("p_sops is null");
		return QDF_STATUS_E_FAILURE;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return QDF_STATUS_E_FAILURE;
	}

	return p_sops->is_spectral_enabled(spectral, smode);
}

#ifdef DIRECT_BUF_RX_DEBUG
/**
 * target_if_spectral_do_dbr_ring_debug() - Start/Stop Spectral DMA ring debug
 * @pdev: Pointer to pdev object
 * @enable: Enable/Disable Spectral DMA ring debug
 *
 * Start/stop Spectral DMA ring debug based on @enable.
 * Also save the state for future use.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
target_if_spectral_do_dbr_ring_debug(struct wlan_objmgr_pdev *pdev, bool enable)
{
	struct target_if_spectral *spectral;
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectal LMAC object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Save the state */
	spectral->dbr_ring_debug = enable;

	if (enable)
		return tx_ops->dbr_tx_ops.direct_buf_rx_start_ring_debug(
				pdev, 0, SPECTRAL_DBR_RING_DEBUG_SIZE);
	else
		return tx_ops->dbr_tx_ops.direct_buf_rx_stop_ring_debug(
				pdev, 0);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_do_dbr_buff_debug() - Start/Stop Spectral DMA buffer debug
 * @pdev: Pointer to pdev object
 * @enable: Enable/Disable Spectral DMA buffer debug
 *
 * Start/stop Spectral DMA buffer debug based on @enable.
 * Also save the state for future use.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
target_if_spectral_do_dbr_buff_debug(struct wlan_objmgr_pdev *pdev, bool enable)
{
	struct target_if_spectral *spectral;
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectal LMAC object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Save the state */
	spectral->dbr_buff_debug = enable;

	if (enable)
		return tx_ops->dbr_tx_ops.direct_buf_rx_start_buffer_poisoning(
				pdev, 0, MEM_POISON_SIGNATURE);
	else
		return tx_ops->dbr_tx_ops.direct_buf_rx_stop_buffer_poisoning(
				pdev, 0);
}

/**
 * target_if_spectral_check_and_do_dbr_buff_debug() - Start/Stop Spectral buffer
 * debug based on the previous state
 * @pdev: Pointer to pdev object
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
target_if_spectral_check_and_do_dbr_buff_debug(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectal LMAC object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (spectral->dbr_buff_debug)
		return target_if_spectral_do_dbr_buff_debug(pdev, true);
	else
		return target_if_spectral_do_dbr_buff_debug(pdev, false);
}

/**
 * target_if_spectral_check_and_do_dbr_ring_debug() - Start/Stop Spectral ring
 * debug based on the previous state
 * @pdev: Pointer to pdev object
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
target_if_spectral_check_and_do_dbr_ring_debug(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectal LMAC object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (spectral->dbr_ring_debug)
		return target_if_spectral_do_dbr_ring_debug(pdev, true);
	else
		return target_if_spectral_do_dbr_ring_debug(pdev, false);
}

/**
 * target_if_spectral_set_dma_debug() - Set DMA debug for Spectral
 * @pdev: Pointer to pdev object
 * @dma_debug_type: Type of Spectral DMA debug i.e., ring or buffer debug
 * @debug_value: Value to be set for @dma_debug_type
 *
 * Set DMA debug for Spectral and start/stop Spectral DMA debug function
 * based on @debug_value
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
target_if_spectral_set_dma_debug(
	struct wlan_objmgr_pdev *pdev,
	enum spectral_dma_debug dma_debug_type,
	bool debug_value)
{
	struct target_if_spectral_ops *p_sops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct target_if_spectral *spectral;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!tx_ops->target_tx_ops.tgt_get_tgt_type) {
		spectral_err("Unable to fetch target type");
		return QDF_STATUS_E_FAILURE;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectal LMAC object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (spectral->direct_dma_support) {
		p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
		if (p_sops->is_spectral_active(spectral,
					       SPECTRAL_SCAN_MODE_NORMAL) ||
		    p_sops->is_spectral_active(spectral,
					       SPECTRAL_SCAN_MODE_AGILE)) {
			spectral_err("Altering DBR debug config isn't allowed during an ongoing scan");
			return QDF_STATUS_E_FAILURE;
		}

		switch (dma_debug_type) {
		case SPECTRAL_DMA_RING_DEBUG:
			target_if_spectral_do_dbr_ring_debug(pdev, debug_value);
			break;

		case SPECTRAL_DMA_BUFFER_DEBUG:
			target_if_spectral_do_dbr_buff_debug(pdev, debug_value);
			break;

		default:
			spectral_err("Unsupported DMA debug type : %d",
				     dma_debug_type);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}
#endif /* DIRECT_BUF_RX_DEBUG */

/**
 * target_if_spectral_direct_dma_support() - Get Direct-DMA support
 * @pdev: Pointer to pdev object
 *
 * Return: Whether Direct-DMA is supported on this radio
 */
static bool
target_if_spectral_direct_dma_support(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral;

	if (!pdev) {
		spectral_err("pdev is NULL!");
		return false;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral LMAC object is NULL");
		return false;
	}

	return spectral->direct_dma_support;
}

/**
 * target_if_set_debug_level() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * API to set the debug level for Spectral
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS
target_if_set_debug_level(struct wlan_objmgr_pdev *pdev, uint32_t debug_level)
{
	spectral_debug_level = (DEBUG_SPECTRAL << debug_level);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_get_debug_level() - Get debug level for Spectral
 * @pdev: Pointer to pdev object
 *
 * API to get the debug level for Spectral
 *
 * Return: Current debug level
 */
uint32_t
target_if_get_debug_level(struct wlan_objmgr_pdev *pdev)
{
	return spectral_debug_level;
}

/**
 * target_if_get_spectral_capinfo() - Get Spectral capability information
 * @pdev: Pointer to pdev object
 * @scaps: Buffer into which data should be copied
 *
 * API to get the spectral capability information
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS
target_if_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev,
			       struct spectral_caps *scaps)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(scaps, &spectral->capability,
		     sizeof(struct spectral_caps));

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_get_spectral_diagstats() - Get Spectral diagnostic statistics
 * @pdev:  Pointer to pdev object
 * @stats: Buffer into which data should be copied
 *
 * API to get the spectral diagnostic statistics
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS
target_if_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
				 struct spectral_diag_stats *stats)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(stats, &spectral->diag_stats,
		     sizeof(struct spectral_diag_stats));

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_register_spectral_wmi_ops() - Register Spectral WMI operations
 * @psoc: Pointer to psoc object
 * @wmi_ops: Pointer to the structure having Spectral WMI operations
 *
 * API for registering Spectral WMI operations in
 * spectral internal data structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_register_spectral_wmi_ops(struct wlan_objmgr_psoc *psoc,
				    struct spectral_wmi_ops *wmi_ops)
{
	struct target_if_psoc_spectral *psoc_spectral;

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("Spectral LMAC object is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc_spectral->wmi_ops = *wmi_ops;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_register_spectral_tgt_ops() - Register Spectral target operations
 * @psoc: Pointer to psoc object
 * @tgt_ops: Pointer to the structure having Spectral target operations
 *
 * API for registering Spectral target operations in
 * spectral internal data structure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_register_spectral_tgt_ops(struct wlan_objmgr_psoc *psoc,
				    struct spectral_tgt_ops *tgt_ops)
{
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	ops_tgt = *tgt_ops;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_register_netlink_cb() - Register Netlink callbacks
 * @pdev: Pointer to pdev object
 * @nl_cb: Netlink callbacks to register
 *
 * Return: void
 */
static void
target_if_register_netlink_cb(
	struct wlan_objmgr_pdev *pdev,
	struct spectral_nl_cb *nl_cb)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return;
	}

	qdf_mem_copy(&spectral->nl_cb, nl_cb, sizeof(struct spectral_nl_cb));

	if (spectral->use_nl_bcast)
		spectral->send_phy_data = spectral->nl_cb.send_nl_bcast;
	else
		spectral->send_phy_data = spectral->nl_cb.send_nl_unicast;
}

/**
 * target_if_use_nl_bcast() - Get whether to use broadcast/unicast while sending
 * Netlink messages to the application layer
 * @pdev: Pointer to pdev object
 *
 * Return: true for broadcast, false for unicast
 */
static bool
target_if_use_nl_bcast(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return false;
	}

	return spectral->use_nl_bcast;
}

/**
 * target_if_deregister_netlink_cb() - De-register Netlink callbacks
 * @pdev: Pointer to pdev object
 *
 * Return: void
 */
static void
target_if_deregister_netlink_cb(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return;
	}

	qdf_mem_zero(&spectral->nl_cb, sizeof(struct spectral_nl_cb));
}

static int
target_if_process_spectral_report(struct wlan_objmgr_pdev *pdev,
				  void *payload)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return -EPERM;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("p_sops is null");
		return -EPERM;
	}

	return p_sops->process_spectral_report(pdev, payload);
}

#ifdef DIRECT_BUF_RX_DEBUG
static inline void
target_if_sptrl_debug_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	if (!tx_ops) {
		spectral_err("tx_ops is NULL");
		return;
	}

	tx_ops->sptrl_tx_ops.sptrlto_set_dma_debug =
		target_if_spectral_set_dma_debug;
	tx_ops->sptrl_tx_ops.sptrlto_check_and_do_dbr_ring_debug =
		target_if_spectral_check_and_do_dbr_ring_debug;
	tx_ops->sptrl_tx_ops.sptrlto_check_and_do_dbr_buff_debug =
		target_if_spectral_check_and_do_dbr_buff_debug;
}
#else
static inline void
target_if_sptrl_debug_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif

#if defined(WLAN_CONV_SPECTRAL_ENABLE) && defined(SPECTRAL_MODULIZED_ENABLE)
/**
 * target_if_spectral_wmi_unified_register_event_handler() - Wrapper function to
 * register WMI event handler
 * @psoc: Pointer to psoc object
 * @event_id: Event id
 * @handler_func: Handler function
 * @rx_ctx: Context of WMI event processing
 *
 * Wrapper function to register WMI event handler
 *
 * Return: 0 for success else failure
 */
static int
target_if_spectral_wmi_unified_register_event_handler(
				struct wlan_objmgr_psoc *psoc,
				wmi_conv_event_id event_id,
				wmi_unified_event_handler handler_func,
				uint8_t rx_ctx)
{
	wmi_unified_t wmi_handle;
	struct target_if_psoc_spectral *psoc_spectral;
	QDF_STATUS ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral object is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ret = psoc_spectral->wmi_ops.wmi_unified_register_event_handler(
			wmi_handle, event_id, handler_func, rx_ctx);

	return qdf_status_to_os_return(ret);
}

/**
 * target_if_spectral_wmi_unified_unregister_event_handler() - Wrapper function
 * to unregister WMI event handler
 * @psoc: Pointer to psoc object
 * @event_id: Event id
 *
 * Wrapper function to unregister WMI event handler
 *
 * Return: 0 for success else failure
 */
static int
target_if_spectral_wmi_unified_unregister_event_handler(
				struct wlan_objmgr_psoc *psoc,
				wmi_conv_event_id event_id)
{
	wmi_unified_t wmi_handle;
	struct target_if_psoc_spectral *psoc_spectral;
	QDF_STATUS ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral object is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ret = psoc_spectral->wmi_ops.wmi_unified_unregister_event_handler(
					wmi_handle, event_id);

	return qdf_status_to_os_return(ret);
}

/**
 * target_if_spectral_wmi_extract_pdev_sscan_fw_cmd_fixed_param() - Wrapper
 * function to extract fixed parameters from start scan response event
 * @psoc: Pointer to psoc object
 * @evt_buf: Event buffer
 * @param: Start scan response parameters
 *
 * Wrapper function to extract fixed parameters from start scan response event
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_spectral_wmi_extract_pdev_sscan_fw_cmd_fixed_param(
			struct wlan_objmgr_psoc *psoc,
			uint8_t *evt_buf,
			struct spectral_startscan_resp_params *param)
{
	wmi_unified_t wmi_handle;
	struct target_if_psoc_spectral *psoc_spectral;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!evt_buf) {
		spectral_err("WMI event buffer is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		spectral_err("Spectral startscan response parameters is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral object is null");
		return QDF_STATUS_E_FAILURE;
	}

	return psoc_spectral->wmi_ops.wmi_extract_pdev_sscan_fw_cmd_fixed_param(
			wmi_handle, evt_buf, param);
}

/**
 * target_if_spectral_wmi_extract_pdev_sscan_fft_bin_index() - Wrapper
 * function to extract start and end indices of primary 80 MHz, 5 MHz and
 * secondary 80 MHz FFT bins
 * @psoc: Pointer to psoc object
 * @evt_buf: Event buffer
 * @param: FFT bin start and end indices
 *
 * Wrapper function to extract start and end indices of primary 80 MHz, 5 MHz
 * and secondary 80 MHz FFT bins
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_spectral_wmi_extract_pdev_sscan_fft_bin_index(
			struct wlan_objmgr_psoc *psoc,
			uint8_t *evt_buf,
			struct spectral_fft_bin_markers_160_165mhz *param)
{
	wmi_unified_t wmi_handle;
	struct target_if_psoc_spectral *psoc_spectral;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!evt_buf) {
		spectral_err("WMI event buffer is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		spectral_err("Spectral FFT bin markers is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral object is null");
		return QDF_STATUS_E_FAILURE;
	}

	return psoc_spectral->wmi_ops.wmi_extract_pdev_sscan_fft_bin_index(
			wmi_handle, evt_buf, param);
}

/**
 * target_if_spectral_get_psoc_from_scn_handle() - Wrapper function to get psoc
 * object from scn handle
 * @scn: scn handle
 *
 * Wrapper function to get psoc object from scn handle
 *
 * Return: Pointer to psoc object
 */
static struct wlan_objmgr_psoc *
target_if_spectral_get_psoc_from_scn_handle(ol_scn_t scn)
{
	if (!scn) {
		spectral_err("scn is null");
		return NULL;
	}

	return ops_tgt.tgt_get_psoc_from_scn_hdl(scn);
}
#else
/**
 * target_if_spectral_wmi_unified_register_event_handler() - Wrapper function to
 * register WMI event handler
 * @psoc: Pointer to psoc object
 * @event_id: Event id
 * @handler_func: Handler function
 * @rx_ctx: Context of WMI event processing
 *
 * Wrapper function to register WMI event handler
 *
 * Return: 0 for success else failure
 */
static int
target_if_spectral_wmi_unified_register_event_handler(
				struct wlan_objmgr_psoc *psoc,
				wmi_conv_event_id event_id,
				wmi_unified_event_handler handler_func,
				uint8_t rx_ctx)
{
	wmi_unified_t wmi_handle;
	QDF_STATUS ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	ret = wmi_unified_register_event_handler(wmi_handle, event_id,
						 handler_func, rx_ctx);

	return qdf_status_to_os_return(ret);
}

/**
 * target_if_spectral_wmi_unified_unregister_event_handler() - Wrapper function
 * to unregister WMI event handler
 * @psoc: Pointer to psoc object
 * @event_id: Event id
 *
 * Wrapper function to unregister WMI event handler
 *
 * Return: 0 for success else failure
 */
static int
target_if_spectral_wmi_unified_unregister_event_handler(
				struct wlan_objmgr_psoc *psoc,
				wmi_conv_event_id event_id)
{
	wmi_unified_t wmi_handle;
	QDF_STATUS ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	ret = wmi_unified_unregister_event_handler(wmi_handle, event_id);

	return qdf_status_to_os_return(ret);
}

/**
 * target_if_spectral_wmi_extract_pdev_sscan_fw_cmd_fixed_param() - Wrapper
 * function to extract fixed parameters from start scan response event
 * @psoc: Pointer to psoc object
 * @evt_buf: Event buffer
 * @param: Start scan response parameters
 *
 * Wrapper function to extract fixed parameters from start scan response event
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_spectral_wmi_extract_pdev_sscan_fw_cmd_fixed_param(
			struct wlan_objmgr_psoc *psoc,
			uint8_t *evt_buf,
			struct spectral_startscan_resp_params *param)
{
	wmi_unified_t wmi_handle;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!evt_buf) {
		spectral_err("WMI event buffer is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		spectral_err("Spectral startscan response parameters is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_extract_pdev_sscan_fw_cmd_fixed_param(wmi_handle, evt_buf,
							 param);
}

/**
 * target_if_spectral_wmi_extract_pdev_sscan_fft_bin_index() - Wrapper
 * function to extract start and end indices of primary 80 MHz, 5 MHz and
 * secondary 80 MHz FFT bins
 * @psoc: Pointer to psoc object
 * @evt_buf: Event buffer
 * @param: FFT bin start and end indices
 *
 * Wrapper function to extract start and end indices of primary 80 MHz, 5 MHz
 * and secondary 80 MHz FFT bins
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_spectral_wmi_extract_pdev_sscan_fft_bin_index(
			struct wlan_objmgr_psoc *psoc,
			uint8_t *evt_buf,
			struct spectral_fft_bin_markers_160_165mhz *param)
{
	wmi_unified_t wmi_handle;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!evt_buf) {
		spectral_err("WMI event buffer is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		spectral_err("Spectral FFT bin markers is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return QDF_STATUS_E_INVAL;
	}

	return wmi_extract_pdev_sscan_fft_bin_index(wmi_handle, evt_buf, param);
}

/**
 * target_if_spectral_get_psoc_from_scn_handle() - Wrapper function to get psoc
 * object from scn handle
 * @scn: scn handle
 *
 * Wrapper function to get psoc object from scn handle
 *
 * Return: Pointer to psoc object
 */
static struct wlan_objmgr_psoc *
target_if_spectral_get_psoc_from_scn_handle(ol_scn_t scn)
{
	if (!scn) {
		spectral_err("scn is null");
		return NULL;
	}

	return target_if_get_psoc_from_scn_hdl(scn);
}
#endif

/**
 * target_if_spectral_fw_param_event_handler() - WMI event handler to
 * process start scan response event
 * @scn: Pointer to scn object
 * @data_buf: Pointer to event buffer
 * @data_len: Length of event buffer
 *
 * Return: 0 for success, else failure
 */
static int
target_if_spectral_fw_param_event_handler(ol_scn_t scn, uint8_t *data_buf,
					  uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wmi_unified *wmi_handle;
	struct spectral_startscan_resp_params event_params = {0};
	struct target_if_psoc_spectral *psoc_spectral;
	struct target_if_spectral *spectral;

	if (!scn) {
		spectral_err("scn handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	if (!data_buf) {
		spectral_err("WMI event buffer null");
		return qdf_status_to_os_return(QDF_STATUS_E_INVAL);
	}

	psoc = target_if_spectral_get_psoc_from_scn_handle(scn);
	if (!psoc) {
		spectral_err("psoc is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral object is null");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		spectral_err("WMI handle is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	status = target_if_spectral_wmi_extract_pdev_sscan_fw_cmd_fixed_param(
				psoc, data_buf, &event_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		spectral_err("unable to extract sscan fw fixed params");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, event_params.pdev_id,
					  WLAN_SPECTRAL_ID);
	if (!pdev) {
		spectral_err("pdev is null");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("spectral object is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_SPECTRAL_ID);
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	if (event_params.num_fft_bin_index == 1) {
		status =
			target_if_spectral_wmi_extract_pdev_sscan_fft_bin_index(
				psoc, data_buf,
				&spectral->rparams.marker[event_params.smode]);
		if (QDF_IS_STATUS_ERROR(status)) {
			spectral_err("unable to extract sscan fw fixed params");
			wlan_objmgr_pdev_release_ref(pdev, WLAN_SPECTRAL_ID);
			return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
		}
	} else {
		spectral->rparams.marker[event_params.smode].is_valid = false;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_SPECTRAL_ID);

	return qdf_status_to_os_return(QDF_STATUS_SUCCESS);
}

static QDF_STATUS
target_if_spectral_register_events(struct wlan_objmgr_psoc *psoc)
{
	int ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	ret = target_if_spectral_wmi_unified_register_event_handler(
			psoc,
			wmi_pdev_sscan_fw_param_eventid,
			target_if_spectral_fw_param_event_handler,
			WMI_RX_UMAC_CTX);

	if (ret)
		spectral_debug("event handler not supported, ret=%d", ret);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
target_if_spectral_unregister_events(struct wlan_objmgr_psoc *psoc)
{
	int ret;

	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_INVAL;
	}

	ret = target_if_spectral_wmi_unified_unregister_event_handler(
			psoc, wmi_pdev_sscan_fw_param_eventid);

	if (ret)
		spectral_debug("Unregister WMI event handler failed, ret = %d",
			       ret);

	return QDF_STATUS_SUCCESS;
}

void
target_if_sptrl_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_init =
	    target_if_pdev_spectral_init;
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_deinit =
	    target_if_pdev_spectral_deinit;
	tx_ops->sptrl_tx_ops.sptrlto_psoc_spectral_init =
	    target_if_psoc_spectral_init;
	tx_ops->sptrl_tx_ops.sptrlto_psoc_spectral_deinit =
	    target_if_psoc_spectral_deinit;
	tx_ops->sptrl_tx_ops.sptrlto_set_spectral_config =
	    target_if_set_spectral_config;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_config =
	    target_if_get_spectral_config;
	tx_ops->sptrl_tx_ops.sptrlto_start_spectral_scan =
	    target_if_start_spectral_scan;
	tx_ops->sptrl_tx_ops.sptrlto_stop_spectral_scan =
	    target_if_stop_spectral_scan;
	tx_ops->sptrl_tx_ops.sptrlto_is_spectral_active =
	    target_if_is_spectral_active;
	tx_ops->sptrl_tx_ops.sptrlto_is_spectral_enabled =
	    target_if_is_spectral_enabled;
	tx_ops->sptrl_tx_ops.sptrlto_set_debug_level =
	    target_if_set_debug_level;
	tx_ops->sptrl_tx_ops.sptrlto_get_debug_level =
	    target_if_get_debug_level;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_capinfo =
	    target_if_get_spectral_capinfo;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_diagstats =
	    target_if_get_spectral_diagstats;
	tx_ops->sptrl_tx_ops.sptrlto_register_spectral_wmi_ops =
	    target_if_register_spectral_wmi_ops;
	tx_ops->sptrl_tx_ops.sptrlto_register_spectral_tgt_ops =
	    target_if_register_spectral_tgt_ops;
	tx_ops->sptrl_tx_ops.sptrlto_register_netlink_cb =
	    target_if_register_netlink_cb;
	tx_ops->sptrl_tx_ops.sptrlto_use_nl_bcast =
	    target_if_use_nl_bcast;
	tx_ops->sptrl_tx_ops.sptrlto_deregister_netlink_cb =
	    target_if_deregister_netlink_cb;
	tx_ops->sptrl_tx_ops.sptrlto_process_spectral_report =
	    target_if_process_spectral_report;
	tx_ops->sptrl_tx_ops.sptrlto_direct_dma_support =
		target_if_spectral_direct_dma_support;
	tx_ops->sptrl_tx_ops.sptrlto_register_events =
		target_if_spectral_register_events;
	tx_ops->sptrl_tx_ops.sptrlto_unregister_events =
		target_if_spectral_unregister_events;

	target_if_sptrl_debug_register_tx_ops(tx_ops);
}
qdf_export_symbol(target_if_sptrl_register_tx_ops);

void
target_if_spectral_send_intf_found_msg(struct wlan_objmgr_pdev *pdev,
				       uint16_t cw_int, uint32_t dcs_enabled)
{
	struct spectral_samp_msg *msg = NULL;
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);

	if (!spectral) {
		spectral_err("SPECTRAL : Module doesn't exist");
		return;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	if (!p_sops) {
		spectral_err("p_sops is null");
		return;
	}

	msg  = (struct spectral_samp_msg *)spectral->nl_cb.get_sbuff(
			spectral->pdev_obj,
			SPECTRAL_MSG_INTERFERENCE_NOTIFICATION,
			SPECTRAL_MSG_BUF_NEW);

	if (msg) {
		msg->int_type = cw_int ?
		    SPECTRAL_DCS_INT_CW : SPECTRAL_DCS_INT_WIFI;
		msg->dcs_enabled = dcs_enabled;
		msg->signature = SPECTRAL_SIGNATURE;
		p_sops->get_mac_address(spectral, msg->macaddr);
		if (spectral->send_phy_data
				(pdev,
				 SPECTRAL_MSG_INTERFERENCE_NOTIFICATION) == 0)
			spectral->spectral_sent_msg++;
	}
}
qdf_export_symbol(target_if_spectral_send_intf_found_msg);

QDF_STATUS
target_if_spectral_is_finite_scan(struct target_if_spectral *spectral,
				  enum spectral_scan_mode smode,
				  bool *finite_spectral_scan)
{
	struct target_if_finite_spectral_scan_params *finite_scan;

	if (!spectral) {
		spectral_err_rl("target if spectral object is null");
		return QDF_STATUS_E_INVAL;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("invalid spectral mode %d", smode);
		return QDF_STATUS_E_INVAL;
	}

	if (!finite_spectral_scan) {
		spectral_err_rl("Invalid pointer");
		return QDF_STATUS_E_INVAL;
	}

	finite_scan = &spectral->finite_scan[smode];
	*finite_spectral_scan = finite_scan->finite_spectral_scan;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_spectral_finite_scan_update(struct target_if_spectral *spectral,
				      enum spectral_scan_mode smode)
{
	struct target_if_finite_spectral_scan_params *finite_scan;

	if (!spectral) {
		spectral_err_rl("target if spectral object is null");
		return QDF_STATUS_E_INVAL;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode");
		return QDF_STATUS_E_INVAL;
	}

	finite_scan = &spectral->finite_scan[smode];

	if (!finite_scan->num_reports_expected) {
		spectral_err_rl("Error, No reports expected");
		return QDF_STATUS_E_FAILURE;
	}

	finite_scan->num_reports_expected--;
	if (!finite_scan->num_reports_expected) {
		QDF_STATUS status;
		enum spectral_cp_error_code err;

		/* received expected number of reports from target, stop scan */
		status = target_if_stop_spectral_scan(spectral->pdev_obj, smode,
						      &err);
		if (QDF_IS_STATUS_ERROR(status)) {
			spectral_err_rl("Failed to stop finite Spectral scan");
			return QDF_STATUS_E_FAILURE;
		}
		finite_scan->finite_spectral_scan =  false;
	}

	return QDF_STATUS_SUCCESS;
}
