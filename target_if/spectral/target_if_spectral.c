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
#include <reg_services_public_struct.h>
#include <target_if_spectral_sim.h>
#include <target_if.h>
#include <qdf_module.h>

/**
 * @spectral_ops - Spectral function table, holds the Spectral functions that
 * depend on whether the architecture is Direct Attach or Offload. This is used
 * to populate the actual Spectral function table present in the Spectral
 * module.
 */
struct target_if_spectral_ops spectral_ops;
int spectral_debug_level = DEBUG_SPECTRAL;

static void target_if_spectral_get_firstvdev_pdev(struct wlan_objmgr_pdev *pdev,
						  void *obj, void *arg)
{
	struct wlan_objmgr_vdev *vdev = obj;
	struct wlan_objmgr_vdev **first_vdev = arg;

	if (!(*first_vdev))
		*first_vdev = vdev;
}

struct wlan_objmgr_vdev *
target_if_spectral_get_vdev(struct target_if_spectral *spectral)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *first_vdev = NULL;

	qdf_assert_always(spectral);
	pdev = spectral->pdev_obj;
	qdf_assert_always(pdev);

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
 * @param: Pointer to spectral_config giving the Spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, negative error code on failure
 */
static int
target_if_send_vdev_spectral_configure_cmd(struct target_if_spectral *spectral,
					   struct spectral_config *param)
{
	struct vdev_spectral_configure_params sparam;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(spectral && param);

	pdev = spectral->pdev_obj;

	qdf_assert_always(pdev);

	vdev = target_if_spectral_get_vdev(spectral);
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

	return spectral->param_wmi_cmd_ops.wmi_spectral_configure_cmd_send(
				GET_WMI_HDL_FROM_PDEV(pdev), &sparam);
}

/**
 * target_if_send_vdev_spectral_enable_cmd() - Send WMI command to
 * enable/disable Spectral
 * @spectral: Pointer to Spectral target_if internal private data
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
					uint8_t is_spectral_active_valid,
					uint8_t is_spectral_active,
					uint8_t is_spectral_enabled_valid,
					uint8_t is_spectral_enabled)
{
	struct vdev_spectral_enable_params param;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(spectral);

	pdev = spectral->pdev_obj;

	qdf_assert_always(pdev);

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	qdf_mem_zero(&param, sizeof(param));

	param.vdev_id = wlan_vdev_get_id(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	param.active_valid = is_spectral_active_valid;
	param.enabled_valid = is_spectral_enabled_valid;
	param.active = is_spectral_active;
	param.enabled = is_spectral_enabled;

	return spectral->param_wmi_cmd_ops.wmi_spectral_enable_cmd_send(
				GET_WMI_HDL_FROM_PDEV(pdev), &param);
}

/**
 * target_if_spectral_info_init_defaults() - Helper function to load defaults
 * for Spectral information (parameters and state) into cache.
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * It is assumed that the caller has obtained the requisite lock if applicable.
 * Note that this is currently treated as a temporary function.  Ideally, we
 * would like to get defaults from the firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, negative error code on failure
 */
static int
target_if_spectral_info_init_defaults(struct target_if_spectral *spectral)
{
	struct target_if_spectral_param_state_info *info =
		&spectral->param_info;
	struct wlan_objmgr_vdev *vdev = NULL;

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

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	info->osps_cache.osc_params.ss_chn_mask =
	    wlan_vdev_mlme_get_rxchainmask(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	info->osps_cache.osc_params.ss_short_report =
		SPECTRAL_SCAN_SHORT_REPORT_DEFAULT;

	info->osps_cache.osc_params.ss_fft_period =
		SPECTRAL_SCAN_FFT_PERIOD_DEFAULT;

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
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_PARAMS. Returning following params:\nss_count = %u\nss_period = %u\nss_spectral_pri = %u\nss_fft_size = %u\nss_gc_ena = %u\nss_restart_ena = %u\nss_noise_floor_ref = %d\nss_init_delay = %u\nss_nb_tone_thr = %u\nss_str_bin_thr = %u\nss_wb_rpt_mode = %u\nss_rssi_rpt_mode = %u\nss_rssi_thr = %d\nss_pwr_format = %u\nss_rpt_mode = %u\nss_bin_scale = %u\nss_dbm_adj = %u\nss_chn_mask = %u\n",
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
		       pparam->ss_chn_mask);
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
	enum target_if_spectral_info specifier,
	void *output, int output_len)
{
	/*
	 * Note: This function is designed to be able to accommodate
	 * WMI reads for defaults, non-cacheable information, etc
	 * if required.
	 */
	struct target_if_spectral_param_state_info *info =
		&spectral->param_info;
	int is_cacheable = 0;
	int init_def_retval = 0;

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

	qdf_spin_lock(&info->osps_lock);

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
			qdf_spin_unlock(&info->osps_lock);
			return 0;
		}
	}

	/* Cache is invalid */

	/*
	 * If WMI Reads are implemented to fetch defaults/non-cacheable info,
	 * then the below implementation will change
	 */
	init_def_retval = target_if_spectral_info_init_defaults(spectral);
	if (init_def_retval != QDF_STATUS_SUCCESS) {
		qdf_spin_unlock(&info->osps_lock);
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

	qdf_spin_unlock(&info->osps_lock);

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
	spectral_debug("%s: TARGET_IF_SPECTRAL_INFO_PARAMS. Params:\nss_count = %u\nss_period = %u\nss_spectral_pri = %u\nss_fft_size = %u\nss_gc_ena = %u\nss_restart_ena = %u\nss_noise_floor_ref = %d\nss_init_delay = %u\nss_nb_tone_thr = %u\nss_str_bin_thr = %u\nss_wb_rpt_mode = %u\nss_rssi_rpt_mode = %u\nss_rssi_thr = %d\nss_pwr_format = %u\nss_rpt_mode = %u\nss_bin_scale = %u\nss_dbm_adj = %u\nss_chn_mask = %u\nstatus = %d",
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
		       param->ss_dbm_adj, param->ss_chn_mask, ret);
}

/**
 * target_if_spectral_info_write() - Write Spectral information to the
 * firmware, and update cache
 * @spectral: Pointer to Spectral target_if internal private data
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
	enum target_if_spectral_info specifier,
	void *input, int input_len)
{
	struct target_if_spectral_param_state_info *info =
		&spectral->param_info;
	int ret;
	uint8_t *pval = NULL;
	struct spectral_config *param = NULL;

	if (!input)
		return -EINVAL;

	switch (specifier) {
	case TARGET_IF_SPECTRAL_INFO_ACTIVE:
		if (input_len != sizeof(info->osps_cache.osc_spectral_active))
			return -EINVAL;

		pval = (uint8_t *)input;

		qdf_spin_lock(&info->osps_lock);
		ret = target_if_send_vdev_spectral_enable_cmd(spectral,
							      1, *pval, 0, 0);

		target_if_log_write_spectral_active(
			__func__,
			*pval,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_enable_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock(&info->osps_lock);
			return ret;
		}

		info->osps_cache.osc_spectral_active = *pval;

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock(&info->osps_lock);
		break;

	case TARGET_IF_SPECTRAL_INFO_ENABLED:
		if (input_len != sizeof(info->osps_cache.osc_spectral_enabled))
			return -EINVAL;

		pval = (uint8_t *)input;

		qdf_spin_lock(&info->osps_lock);
		ret = target_if_send_vdev_spectral_enable_cmd(spectral,
							      0, 0, 1, *pval);

		target_if_log_write_spectral_enabled(
			__func__,
			*pval,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_enable_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock(&info->osps_lock);
			return ret;
		}

		info->osps_cache.osc_spectral_enabled = *pval;

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock(&info->osps_lock);
		break;

	case TARGET_IF_SPECTRAL_INFO_PARAMS:
		if (input_len != sizeof(info->osps_cache.osc_params))
			return -EINVAL;

		param = (struct spectral_config *)input;

		qdf_spin_lock(&info->osps_lock);
		ret = target_if_send_vdev_spectral_configure_cmd(spectral,
								 param);

		target_if_log_write_spectral_params(
			param,
			__func__,
			ret);

		if (ret < 0) {
			spectral_err("target_if_send_vdev_spectral_configure_cmd failed with error=%d",
				     ret);
			qdf_spin_unlock(&info->osps_lock);
			return ret;
		}

		qdf_mem_copy(&info->osps_cache.osc_params,
			     param, sizeof(info->osps_cache.osc_params));

		/* The cache is now valid */
		info->osps_cache.osc_is_valid = 1;

		qdf_spin_unlock(&info->osps_lock);
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
 *
 * Function to check whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
uint32_t
target_if_sops_is_spectral_active(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
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
 *
 * Function to check whether Spectral is enabled
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
uint32_t
target_if_sops_is_spectral_enabled(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
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
 *
 * Function to start spectral scan
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_sops_start_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 1;
	uint8_t enabled = 0;
	int ret;

	ret = target_if_spectral_info_read(
		spectral,
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
			TARGET_IF_SPECTRAL_INFO_ENABLED,
			&val, sizeof(val));

		if (ret != 0)
			return ret;
	}

	ret = target_if_spectral_info_write(
		spectral,
		TARGET_IF_SPECTRAL_INFO_ACTIVE,
		&val, sizeof(val));

	if (ret != 0)
		return ret;

	return 0;
}

/**
 * target_if_sops_stop_spectral_scan() - Stop Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Function to stop spectral scan
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_sops_stop_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	uint8_t val = 0;
	int tempret, ret = 0;
	uint8_t enabled = 0;

	tempret = target_if_spectral_info_read(
		spectral,
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
			TARGET_IF_SPECTRAL_INFO_ACTIVE,
			&val, sizeof(val));

	if (tempret != 0)
		ret = tempret;

	tempret = target_if_spectral_info_write(
			spectral,
			TARGET_IF_SPECTRAL_INFO_ENABLED,
			&val, sizeof(val));

	if (tempret != 0)
		ret = tempret;

	return ret;
}

/**
 * target_if_spectral_get_extension_channel() - Get the Extension channel
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Function to get the current Extension channel (in MHz)
 *
 * Return: Current Extension channel (in MHz) on success, 0 on failure or if
 * extension channel is not present.
 */
uint32_t
target_if_spectral_get_extension_channel(void *arg)
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

	vdev = target_if_spectral_get_vdev(spectral);
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
 *
 * Function to get the current channel (in MHz)
 *
 * Return: Current channel (in MHz) on success, 0 on failure
 */
uint32_t
target_if_spectral_get_current_channel(void *arg)
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

	vdev = target_if_spectral_get_vdev(spectral);
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
 *
 * Function to configure spectral parameters
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_spectral_sops_configure_params(
	void *arg, struct spectral_config *params)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return target_if_spectral_info_write(
		spectral,
		TARGET_IF_SPECTRAL_INFO_PARAMS,
		params, sizeof(*params));
}

/**
 * target_if_spectral_sops_get_params() - Get user configured Spectral
 * parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Pointer to buffer into which Spectral parameters should be copied
 *
 * Function to get the configured spectral parameters
 *
 * Return: 0 on success else failure
 */
uint32_t
target_if_spectral_sops_get_params(void *arg, struct spectral_config *params)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return target_if_spectral_info_read(
		spectral,
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
 * target_if_init_spectral_capability() - Initialize Spectral capability
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * This is a workaround.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_init_spectral_capability(struct target_if_spectral *spectral)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_psoc_host_spectral_scaling_params *scaling_params;
	uint8_t num_bin_scaling_params, param_idx, pdev_id;
	struct target_psoc_info *tgt_psoc_info;
	struct wlan_psoc_host_service_ext_param *ext_svc_param;
	struct spectral_caps *pcap = &spectral->capability;

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

	for (param_idx = 0; param_idx < num_bin_scaling_params; param_idx++) {
		if (scaling_params[param_idx].pdev_id == pdev_id) {
			pcap->is_scaling_params_populated = true;
			pcap->formula_id = scaling_params[param_idx].formula_id;
			pcap->low_level_offset =
				scaling_params[param_idx].low_level_offset;
			pcap->high_level_offset =
				scaling_params[param_idx].high_level_offset;
			pcap->rssi_thr = scaling_params[param_idx].rssi_thr;
			pcap->default_agc_max_gain =
				scaling_params[param_idx].default_agc_max_gain;
			break;
		}
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
null_is_spectral_active(void *arg)
{
	spectral_ops_not_registered("is_spectral_active");
	return 1;
}

static uint32_t
null_is_spectral_enabled(void *arg)
{
	spectral_ops_not_registered("is_spectral_enabled");
	return 1;
}

static uint32_t
null_start_spectral_scan(void *arg)
{
	spectral_ops_not_registered("start_spectral_scan");
	return 1;
}

static uint32_t
null_stop_spectral_scan(void *arg)
{
	spectral_ops_not_registered("stop_spectral_scan");
	return 1;
}

static uint32_t
null_get_extension_channel(void *arg)
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
null_configure_spectral(void *arg, struct spectral_config *params)
{
	spectral_ops_not_registered("configure_spectral");
	return 0;
}

static uint32_t
null_get_spectral_config(void *arg, struct spectral_config *params)
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
null_get_current_channel(void *arg)
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
	spectral_info("spectral detach");

	if (spectral) {
		if (spectral->spectral_gen == SPECTRAL_GEN3)
			deinit_160mhz_delivery_state_machine(spectral);
		qdf_spinlock_destroy(&spectral->param_info.osps_lock);

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
	struct wlan_lmac_if_target_tx_ops *tx_ops;

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

	psoc = wlan_pdev_get_psoc(pdev);

	tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;

	if (tx_ops->tgt_get_tgt_type) {
		target_type = tx_ops->tgt_get_tgt_type(psoc);
	} else {
		qdf_mem_free(spectral);
		return NULL;
	}

	if (tx_ops->tgt_get_tgt_revision) {
		target_revision = tx_ops->tgt_get_tgt_revision(psoc);
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

	if (target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCA6018)
		spectral->fftbin_size_war =
			SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE;
	else if (target_type == TARGET_TYPE_QCA8074)
		spectral->fftbin_size_war =
			SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE;
	else
		spectral->fftbin_size_war = SPECTRAL_FFTBIN_SIZE_NO_WAR;

	if (target_type == TARGET_TYPE_QCA8074 ||
	    target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCA6018) {
		spectral->inband_fftbin_size_adj = 1;
		spectral->null_fftbin_adj = 1;
	} else {
		spectral->inband_fftbin_size_adj = 0;
		spectral->null_fftbin_adj = 0;
	}
	spectral->last_fft_timestamp = 0;
	spectral->timestamp_war_offset = 0;

	if ((target_type == TARGET_TYPE_QCA8074) ||
	    (target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCA6018) ||
	    (target_type == TARGET_TYPE_QCA6290)) {
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

	spectral->params_valid = false;
	/* Init spectral capability */
	if (target_if_init_spectral_capability(spectral) !=
					QDF_STATUS_SUCCESS) {
		qdf_mem_free(spectral);
		return NULL;
	}
	if (target_if_spectral_attach_simulation(spectral) < 0)
		return NULL;

	target_if_init_spectral_ops(spectral);

	qdf_spinlock_create(&spectral->param_info.osps_lock);
	spectral->param_info.osps_cache.osc_is_valid = 0;

	target_if_spectral_register_funcs(spectral, &spectral_ops);

	if (target_if_spectral_check_hw_capability(spectral) == false) {
		target_if_spectral_detach(spectral);
		spectral = NULL;
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
 * target_if_set_spectral_config() - Set spectral config
 * @pdev:       Pointer to pdev object
 * @threshtype: config type
 * @value:      config value
 *
 * API to set spectral configurations
 *
 * Return: 0 on success else failure
 */
int
target_if_set_spectral_config(struct wlan_objmgr_pdev *pdev,
			      const uint32_t threshtype, const uint32_t value)
{
	struct spectral_config params;
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	if (!spectral) {
		spectral_err("spectral object is NULL");
		return -EPERM;
	}

	if (!spectral->params_valid) {
		target_if_spectral_info_read(spectral,
					     TARGET_IF_SPECTRAL_INFO_PARAMS,
					     &spectral->params,
					     sizeof(spectral->params));
		spectral->params_valid = true;
	}

	switch (threshtype) {
	case SPECTRAL_PARAM_FFT_PERIOD:
		spectral->params.ss_fft_period = value;
		break;
	case SPECTRAL_PARAM_SCAN_PERIOD:
		spectral->params.ss_period = value;
		break;
	case SPECTRAL_PARAM_SCAN_COUNT:
		spectral->params.ss_count = value;
		break;
	case SPECTRAL_PARAM_SHORT_REPORT:
		spectral->params.ss_short_report = (!!value) ? true : false;
		break;
	case SPECTRAL_PARAM_SPECT_PRI:
		spectral->params.ss_spectral_pri = (!!value) ? true : false;
		break;
	case SPECTRAL_PARAM_FFT_SIZE:
		spectral->params.ss_fft_size = value;
		break;
	case SPECTRAL_PARAM_GC_ENA:
		spectral->params.ss_gc_ena = !!value;
		break;
	case SPECTRAL_PARAM_RESTART_ENA:
		spectral->params.ss_restart_ena = !!value;
		break;
	case SPECTRAL_PARAM_NOISE_FLOOR_REF:
		spectral->params.ss_noise_floor_ref = value;
		break;
	case SPECTRAL_PARAM_INIT_DELAY:
		spectral->params.ss_init_delay = value;
		break;
	case SPECTRAL_PARAM_NB_TONE_THR:
		spectral->params.ss_nb_tone_thr = value;
		break;
	case SPECTRAL_PARAM_STR_BIN_THR:
		spectral->params.ss_str_bin_thr = value;
		break;
	case SPECTRAL_PARAM_WB_RPT_MODE:
		spectral->params.ss_wb_rpt_mode = !!value;
		break;
	case SPECTRAL_PARAM_RSSI_RPT_MODE:
		spectral->params.ss_rssi_rpt_mode = !!value;
		break;
	case SPECTRAL_PARAM_RSSI_THR:
		spectral->params.ss_rssi_thr = value;
		break;
	case SPECTRAL_PARAM_PWR_FORMAT:
		spectral->params.ss_pwr_format = !!value;
		break;
	case SPECTRAL_PARAM_RPT_MODE:
		spectral->params.ss_rpt_mode = value;
		break;
	case SPECTRAL_PARAM_BIN_SCALE:
		spectral->params.ss_bin_scale = value;
		break;
	case SPECTRAL_PARAM_DBM_ADJ:
		spectral->params.ss_dbm_adj = !!value;
		break;
	case SPECTRAL_PARAM_CHN_MASK:
		spectral->params.ss_chn_mask = value;
		break;
	}

	p_sops->configure_spectral(spectral, &spectral->params);
	/* only to validate the writes */
	p_sops->get_spectral_config(spectral, &params);
	return 0;
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
 * @fft_len: FFT length
 * @pdev: Pointer to pdev object
 *
 * API to initialize the control and extension flags with the lower/upper
 * segment based on the HT mode
 *
 * Return: FFt bin count
 */
static void
target_if_init_upper_lower_flags(struct target_if_spectral *spectral)
{
	int current_channel = 0;
	int ext_channel = 0;
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);

	current_channel = p_sops->get_current_channel(spectral);
	ext_channel = p_sops->get_extension_channel(spectral);

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
 *
 * API to get the current spectral configuration
 *
 * Return: None
 */
void
target_if_get_spectral_config(struct wlan_objmgr_pdev *pdev,
			      struct spectral_config *param)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	qdf_mem_zero(param, sizeof(struct spectral_config));
	p_sops->get_spectral_config(spectral, param);
}

/**
 * target_if_spectral_scan_enable_params() - Enable use of desired Spectral
 *                                           parameters
 * @spectral: Pointer to Spectral target_if internal private data
 * @spectral_params: Pointer to Spectral parameters
 *
 * Enable use of desired Spectral parameters by configuring them into HW, and
 * starting Spectral scan
 *
 * Return: 0 on success, 1 on failure
 */
int
target_if_spectral_scan_enable_params(struct target_if_spectral *spectral,
				      struct spectral_config *spectral_params)
{
	int extension_channel = 0;
	int current_channel = 0;
	struct target_if_spectral_ops *p_sops = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;

	if (!spectral) {
		spectral_err("SPECTRAL : Spectral is NULL");
		return 1;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		spectral_err("SPECTRAL : p_sops is NULL");
		return 1;
	}

	spectral->sc_spectral_noise_pwr_cal =
	    spectral_params->ss_spectral_pri ? 1 : 0;

	/* check if extension channel is present */
	extension_channel = p_sops->get_extension_channel(spectral);
	current_channel = p_sops->get_current_channel(spectral);

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return 1;

	spectral->ch_width = target_if_vdev_get_ch_width(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	if (spectral->ch_width == CH_WIDTH_INVALID)
		return 1;

	if (spectral->capability.advncd_spectral_cap) {
		spectral->lb_edge_extrabins = 0;
		spectral->rb_edge_extrabins = 0;

		if (spectral->is_lb_edge_extrabins_format &&
		    spectral->params.ss_rpt_mode == 2) {
			spectral->lb_edge_extrabins = 4;
		}

		if (spectral->is_rb_edge_extrabins_format &&
		    spectral->params.ss_rpt_mode == 2) {
			spectral->rb_edge_extrabins = 4;
		}

		if (spectral->ch_width == CH_WIDTH_20MHZ) {
			spectral->sc_spectral_20_40_mode = 0;

			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			/*
			 * Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
			    current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

		} else if (spectral->ch_width == CH_WIDTH_40MHZ) {
			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 1;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);

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

		} else if (spectral->ch_width == CH_WIDTH_80MHZ) {
			/* Set the FFT Size */
			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 0;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);

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

		} else if (spectral->ch_width == CH_WIDTH_160MHZ) {
			/* Set the FFT Size */

			/* The below applies to both 160 and 80+80 cases */

			/* TODO : Remove this variable */
			spectral->sc_spectral_20_40_mode = 0;
			spectral->spectral_numbins =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_fft_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			    target_if_get_fft_bin_count(
				spectral->params.ss_fft_size);

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

	if (!p_sops->is_spectral_active(spectral)) {
		p_sops->configure_spectral(spectral, spectral_params);
		p_sops->start_spectral_scan(spectral);
		spectral->timestamp_war_offset = 0;
		spectral->last_fft_timestamp = 0;
	} else {
	}

	/* get current spectral configuration */
	p_sops->get_spectral_config(spectral, &spectral->params);

	target_if_init_upper_lower_flags(spectral);

	return 0;
}

/**
 * target_if_start_spectral_scan() - Start spectral scan
 * @pdev: Pointer to pdev object
 *
 * API to start spectral scan
 *
 * Return: 0 in case of success, -1 on failure
 */
int
target_if_start_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Spectral LMAC object is NUll");
		return -EPERM;
	}
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	if (!spectral->params_valid) {
		target_if_spectral_info_read(spectral,
					     TARGET_IF_SPECTRAL_INFO_PARAMS,
					     &spectral->params,
					     sizeof(spectral->params));
		spectral->params_valid = true;
	}

	qdf_spin_lock(&spectral->spectral_lock);
	target_if_spectral_scan_enable_params(spectral, &spectral->params);
	qdf_spin_unlock(&spectral->spectral_lock);

	return 0;
}

void
target_if_stop_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("SPECTRAL : Spectral LMAC object is NUll ");
		return;
	}
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	qdf_spin_lock(&spectral->spectral_lock);
	p_sops->stop_spectral_scan(spectral);
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
}

/**
 * target_if_is_spectral_active() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 *
 * API to get whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool
target_if_is_spectral_active(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	return p_sops->is_spectral_active(spectral);
}

/**
 * target_if_is_spectral_enabled() - Get whether Spectral is enabled
 * @pdev: Pointer to pdev object
 *
 * API to get whether Spectral is enabled
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
bool
target_if_is_spectral_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	return p_sops->is_spectral_enabled(spectral);
}

/**
 * target_if_set_debug_level() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * API to set the debug level for Spectral
 *
 * Return: 0 in case of success
 */
int
target_if_set_debug_level(struct wlan_objmgr_pdev *pdev, uint32_t debug_level)
{
	spectral_debug_level = (DEBUG_SPECTRAL << debug_level);
	return 0;
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
 * @outdata: Buffer into which data should be copied
 *
 * API to get the spectral capability information
 *
 * Return: void
 */
void
target_if_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	qdf_mem_copy(outdata, &spectral->capability,
		     sizeof(struct spectral_caps));
}

/**
 * target_if_get_spectral_diagstats() - Get Spectral diagnostic statistics
 * @pdev:  Pointer to pdev object
 * @outdata: Buffer into which data should be copied
 *
 * API to get the spectral diagnostic statistics
 *
 * Return: void
 */
void
target_if_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev, void *outdata)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	qdf_mem_copy(outdata, &spectral->diag_stats,
		     sizeof(struct spectral_diag_stats));
}

/**
 * target_if_register_wmi_spectral_cmd_ops() - Register wmi_spectral_cmd_ops
 * @cmd_ops: Pointer to the structure having wmi_spectral_cmd function pointers
 * @pdev: Pointer to pdev object
 *
 * API for register wmi_spectral_cmd_ops in spectral internal data structure
 *
 * Return: void
 */
void
target_if_register_wmi_spectral_cmd_ops(struct wlan_objmgr_pdev *pdev,
					struct wmi_spectral_cmd_ops *cmd_ops)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	spectral->param_wmi_cmd_ops.wmi_spectral_configure_cmd_send =
	    cmd_ops->wmi_spectral_configure_cmd_send;
	spectral->param_wmi_cmd_ops.wmi_spectral_enable_cmd_send =
	    cmd_ops->wmi_spectral_enable_cmd_send;
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
	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	return p_sops->process_spectral_report(pdev, payload);
}

void
target_if_sptrl_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_init =
	    target_if_pdev_spectral_init;
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_deinit =
	    target_if_pdev_spectral_deinit;
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
	tx_ops->sptrl_tx_ops.sptrlto_register_wmi_spectral_cmd_ops =
	    target_if_register_wmi_spectral_cmd_ops;
	tx_ops->sptrl_tx_ops.sptrlto_register_netlink_cb =
	    target_if_register_netlink_cb;
	tx_ops->sptrl_tx_ops.sptrlto_use_nl_bcast =
	    target_if_use_nl_bcast;
	tx_ops->sptrl_tx_ops.sptrlto_deregister_netlink_cb =
	    target_if_deregister_netlink_cb;
	tx_ops->sptrl_tx_ops.sptrlto_process_spectral_report =
		target_if_process_spectral_report;
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
	msg  = (struct spectral_samp_msg *)spectral->nl_cb.get_nbuff(
			spectral->pdev_obj);

	if (msg) {
		msg->int_type = cw_int ?
		    SPECTRAL_DCS_INT_CW : SPECTRAL_DCS_INT_WIFI;
		msg->dcs_enabled = dcs_enabled;
		msg->signature = SPECTRAL_SIGNATURE;
		p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
		p_sops->get_mac_address(spectral, msg->macaddr);
		if (spectral->send_phy_data(pdev) == 0)
			spectral->spectral_sent_msg++;
	}
}
qdf_export_symbol(target_if_spectral_send_intf_found_msg);
