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

#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <target_type.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <reg_services_public_struct.h>
#ifdef CONFIG_WIN
#include <wlan_mlme_dispatcher.h>
#endif /*CONFIG_WIN*/
#include <reg_services_public_struct.h>
#include <target_if_spectral_sim.h>

/**
 * @spectral_ops - Spectral function table, holds the Spectral functions that
 * depend on whether the architecture is Direct Attach or Offload. This is used
 * to populate the actual Spectral function table present in the Spectral
 * module.
 */
struct target_if_spectral_ops spectral_ops;
int spectral_debug_level = ATH_DEBUG_SPECTRAL;

/**
 * target_if_spectral_get_vdev() - Get pointer to vdev to be used for Spectral
 * operations
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Spectral operates on pdev. However, in order to retrieve some WLAN
 * properties, a vdev is required. To facilitate this, the function returns the
 * first vdev in our pdev. The caller should release the reference to the vdev
 * once it is done using it.
 * TODO: If the framework later provides an API to obtain the first active
 * vdev, then it would be preferable to use this API.
 *
 * Return: Pointer to vdev on success, NULL on failure
 */
struct wlan_objmgr_vdev *
target_if_spectral_get_vdev(struct target_if_spectral *spectral)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(spectral);
	pdev = spectral->pdev_obj;
	qdf_assert_always(pdev);

	if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SPECTRAL_ID) !=
		QDF_STATUS_SUCCESS) {
		qdf_print("%s: Unable to get pdev reference.\n", __func__);
		return NULL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, 0, WLAN_SPECTRAL_ID);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_SPECTRAL_ID);

	if (!vdev) {
		qdf_print("%s: Unable to get first vdev of pdev.\n", __func__);
		return NULL;
	}

	return vdev;
}

/**
 * target_if_send_vdev_spectral_configure_cmd() - Send WMI command to configure
 * Spectral parameters
 * @spectral: Pointer to Spectral target_if internal private data
 * @param: Pointer to spectral_config giving the Spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS on success, negative error code on failure
 */
static int target_if_send_vdev_spectral_configure_cmd(
	struct target_if_spectral *spectral,
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

	qdf_mem_set(&sparam, sizeof(sparam), 0);

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
	sparam.dBm_adj = param->ss_dbm_adj;
	sparam.chn_mask = param->ss_chn_mask;

	return spectral->param_wmi_cmd_ops.wmi_spectral_configure_cmd_send(
		(wmi_unified_t)pdev->tgt_if_handle, &sparam);
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
static int target_if_send_vdev_spectral_enable_cmd(
	struct target_if_spectral *spectral,
	u_int8_t is_spectral_active_valid,
	u_int8_t is_spectral_active,
	u_int8_t is_spectral_enabled_valid,
	u_int8_t is_spectral_enabled)
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

	qdf_mem_set(&param, sizeof(param), 0);

	param.vdev_id = wlan_vdev_get_id(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	param.active_valid = is_spectral_active_valid;
	param.enabled_valid = is_spectral_enabled_valid;
	param.active = is_spectral_active;
	param.enabled = is_spectral_enabled;

	return spectral->param_wmi_cmd_ops.wmi_spectral_enable_cmd_send(
		(wmi_unified_t)pdev->tgt_if_handle, &param);
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
static int target_if_spectral_info_init_defaults(struct target_if_spectral *spectral)
{
	struct target_if_spectral_param_state_info *info = &spectral->ol_info;
	struct wlan_objmgr_vdev *vdev = NULL;

	/* State */
	info->osps_cache.osc_spectral_active =
	SPECTRAL_SCAN_ACTIVE_DEFAULT;

	info->osps_cache.osc_spectral_enabled =
	SPECTRAL_SCAN_ENABLE_DEFAULT;

	/* Parameters */
	info->osps_cache.osc_params.ss_count =
	SPECTRAL_SCAN_COUNT_DEFAULT;

	info->osps_cache.osc_params.ss_period =
	SPECTRAL_SCAN_PERIOD_DEFAULT;

	info->osps_cache.osc_params.ss_spectral_pri =
	SPECTRAL_SCAN_PRIORITY_DEFAULT;

	info->osps_cache.osc_params.ss_fft_size =
	SPECTRAL_SCAN_FFT_SIZE_DEFAULT;

	info->osps_cache.osc_params.ss_gc_ena =
	SPECTRAL_SCAN_GC_ENA_DEFAULT;

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

	info->osps_cache.osc_params.ss_dbm_adj =
	SPECTRAL_SCAN_DBM_ADJ_DEFAULT;

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return QDF_STATUS_E_NOENT;

	info->osps_cache.osc_params.ss_chn_mask =
			wlan_vdev_mlme_get_rxchainmask(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	/* The cache is now valid */
	info->osps_cache.osc_is_valid = 1;

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_spectral_info_read() - Read Spectral parameters or the desired state
 * information from the cache.
 * @spectral: Pointer to Spectral target_if internal private data
 * @specifier: ol_spectral_info_spec enumeration specifying which information is
 * required
 * @output: Void output pointer into which the information will be read
 * @output_len: size of object pointed to by output pointer
 *
 * Return: 0 on success, negative error code on failure
 */
static int ol_spectral_info_read(struct target_if_spectral *spectral,
			  enum ol_spectral_info_spec specifier,
				void *output,
				int output_len)
{
	/* Note: This function is designed to be able to accommodate
	 * WMI reads for defaults, non-cacheable information, etc
	 * if required.
	 */
	struct target_if_spectral_param_state_info *info = &spectral->ol_info;
	int is_cacheable = 0;
	int init_def_retval = 0;

	if (!output)
		return -EINVAL;

	switch (specifier) {
	case OL_SPECTRAL_INFO_SPEC_ACTIVE:
		if (output_len !=
			sizeof(info->osps_cache.osc_spectral_active))
			return -EINVAL;
		is_cacheable = 1;
		break;

	case OL_SPECTRAL_INFO_SPEC_ENABLED:
		if (output_len !=
			sizeof(info->osps_cache.osc_spectral_enabled))
			return -EINVAL;
		is_cacheable = 1;
		break;

	case OL_SPECTRAL_INFO_SPEC_PARAMS:
		if (output_len != sizeof(info->osps_cache.osc_params))
			return -EINVAL;
		is_cacheable = 1;
		break;

	default:
		qdf_print("%s: Unknown ol_spectral_info_spec specifier\n",
			  __func__);
		return -EINVAL;
	}

	qdf_spin_lock(&info->osps_lock);

	if (is_cacheable) {
		if (info->osps_cache.osc_is_valid) {
			switch (specifier) {
			case OL_SPECTRAL_INFO_SPEC_ACTIVE:
				qdf_mem_copy(
				 output,
				 &info->osps_cache.osc_spectral_active,
				 sizeof(info->osps_cache.osc_spectral_active));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
				qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ACTIVE. "
					  "Returning val=%u\n",
					  __func__,
					  *((unsigned char *)output));
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */
				break;

			case OL_SPECTRAL_INFO_SPEC_ENABLED:
				qdf_mem_copy(
				output,
				&info->osps_cache.osc_spectral_enabled,
				sizeof(info->osps_cache.osc_spectral_enabled));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
				qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ENABLED. "
					  "Returning val=%u\n",
					  __func__,
					  *((unsigned char *)output));
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

				break;

			case OL_SPECTRAL_INFO_SPEC_PARAMS:
				qdf_mem_copy(
					output,
					&info->osps_cache.osc_params,
					sizeof(info->osps_cache.osc_params));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
			{
				struct spectral_config *pparam =
					(struct spectral_config *)output;

				qdf_print("%s: OL_SPECTRAL_INFO_SPEC_PARAMS. "
						"Returning following params:\n"
						"ss_count = %u\n"
						"ss_period = %u\n"
						"ss_spectral_pri = %u\n"
						"ss_fft_size = %u\n"
						"ss_gc_ena = %u\n"
						"ss_restart_ena = %u\n"
						"ss_noise_floor_ref = %d\n"
						"ss_init_delay = %u\n"
						"ss_nb_tone_thr = %u\n"
						"ss_str_bin_thr = %u\n"
						"ss_wb_rpt_mode = %u\n"
						"ss_rssi_rpt_mode = %u\n"
						"ss_rssi_thr = %d\n"
						"ss_pwr_format = %u\n"
						"ss_rpt_mode = %u\n"
						"ss_bin_scale = %u\n"
						"ss_dbm_adj = %u\n"
						"ss_chn_mask = %u\n\n",
						__func__,
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
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */
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

	/* If WMI Reads are implemented to fetch defaults/non-cacheable info,
	 * then the below implementation will change
	 */
	init_def_retval = target_if_spectral_info_init_defaults(spectral);
	if (init_def_retval != QDF_STATUS_SUCCESS) {
		if (init_def_retval == QDF_STATUS_E_NOENT)
			return -ENOENT;
		else
			return -EINVAL;
	}
	/* target_if_spectral_info_init_defaults() has set cache to valid */

	switch (specifier) {
	case OL_SPECTRAL_INFO_SPEC_ACTIVE:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_spectral_active,
			     sizeof(info->osps_cache.osc_spectral_active));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
		qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ACTIVE on "
				"initial cache validation\n"
				"Returning val=%u\n",
				__func__,
			*((unsigned char *)output));
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */
		break;

	case OL_SPECTRAL_INFO_SPEC_ENABLED:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_spectral_enabled,
			     sizeof(info->osps_cache.osc_spectral_enabled));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
		qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ENABLED on "
				"initial cache validation\n"
				"Returning val=%u\n",
				__func__,
			*((unsigned char *)output));
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */
		break;

	case OL_SPECTRAL_INFO_SPEC_PARAMS:
		qdf_mem_copy(output,
			     &info->osps_cache.osc_params,
			     sizeof(info->osps_cache.osc_params));
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
		{
		struct spectral_config *pparam =
			(struct spectral_config *)output;

		qdf_print("%s: OL_SPECTRAL_INFO_SPEC_PARAMS on "
				"initial cache validation\n"
				"Returning following params:\n"
				"ss_count = %u\n"
				"ss_period = %u\n"
				"ss_spectral_pri = %u\n"
				"ss_fft_size = %u\n"
				"ss_gc_ena = %u\n"
				"ss_restart_ena = %u\n"
				"ss_noise_floor_ref = %d\n"
				"ss_init_delay = %u\n"
				"ss_nb_tone_thr = %u\n"
				"ss_str_bin_thr = %u\n"
				"ss_wb_rpt_mode = %u\n"
				"ss_rssi_rpt_mode = %u\n"
				"ss_rssi_thr = %d\n"
				"ss_pwr_format = %u\n"
				"ss_rpt_mode = %u\n"
				"ss_bin_scale = %u\n"
				"ss_dbm_adj = %u\n"
				"ss_chn_mask = %u\n\n",
				__func__,
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
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

		break;

	default:
		/* We can't reach this point */
		break;
	}

	qdf_spin_unlock(&info->osps_lock);

	return 0;
}

/**
 * ol_spectral_info_write() - Write Spectral parameters or the desired state
 * information to the firmware, and update cache
 * @spectral: Pointer to Spectral target_if internal private data
 * @specifier: ol_spectral_info_spec enumeration specifying which information is
 * involved
 * @input: void input pointer containing the information to be written
 * @input_len: size of object pointed to by input pointer
 *
 * Return: 0 on success, negative error code on failure
 */
static int ol_spectral_info_write(struct target_if_spectral *spectral,
			   enum ol_spectral_info_spec specifier,
				void *input,
				int input_len)
{
	struct target_if_spectral_param_state_info *info = &spectral->ol_info;
	int ret;
	u_int8_t *pval = NULL;
	struct spectral_config *param = NULL;

	if (!input)
		return -EINVAL;

		switch (specifier) {
		case OL_SPECTRAL_INFO_SPEC_ACTIVE:
			if (input_len !=
				sizeof(info->osps_cache.osc_spectral_active))
				return -EINVAL;

			pval = (u_int8_t *)input;

			qdf_spin_lock(&info->osps_lock);
			ret = target_if_send_vdev_spectral_enable_cmd(
				spectral,
				 1,
				 *pval,
				 0,
				 0);

#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
			qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ACTIVE with "
					"val=%u status=%d\n",
					__func__,
				*pval,
					ret);
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

			if (ret < 0) {
				qdf_print(
				  "%s: target_if_send_vdev_spectral_enable_cmd "
				  "failed with error=%d\n",
				  __func__,
				  ret);
				qdf_spin_unlock(&info->osps_lock);
				return ret;
			}

			info->osps_cache.osc_spectral_active = *pval;
			qdf_spin_unlock(&info->osps_lock);
			break;

		case OL_SPECTRAL_INFO_SPEC_ENABLED:
			if (input_len !=
				sizeof(info->osps_cache.osc_spectral_enabled))
				return -EINVAL;

			pval = (u_int8_t *)input;

			qdf_spin_lock(&info->osps_lock);
			ret = target_if_send_vdev_spectral_enable_cmd(
				spectral,
				 0,
				 0,
				 1,
				 *pval);

#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
			qdf_print("%s: OL_SPECTRAL_INFO_SPEC_ENABLED with "
					"val=%u status=%d\n",
					__func__,
				*pval,
					ret);
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

			if (ret < 0) {
				qdf_print(
				  "%s: target_if_send_vdev_spectral_enable_cmd "
				  "failed with error=%d\n",
				  __func__,
				  ret);
				qdf_spin_unlock(&info->osps_lock);
				return ret;
			}

			info->osps_cache.osc_spectral_enabled = *pval;
			qdf_spin_unlock(&info->osps_lock);
			break;

		case OL_SPECTRAL_INFO_SPEC_PARAMS:
			if (input_len != sizeof(info->osps_cache.osc_params))
				return -EINVAL;

			param = (struct spectral_config *)input;

			qdf_spin_lock(&info->osps_lock);
			ret = target_if_send_vdev_spectral_configure_cmd(
				spectral,
				 param);

#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
			qdf_print("%s: OL_SPECTRAL_INFO_SPEC_PARAMS. "
					"Params:\n"
					"ss_count = %u\n"
					"ss_period = %u\n"
					"ss_spectral_pri = %u\n"
					"ss_fft_size = %u\n"
					"ss_gc_ena = %u\n"
					"ss_restart_ena = %u\n"
					"ss_noise_floor_ref = %d\n"
					"ss_init_delay = %u\n"
					"ss_nb_tone_thr = %u\n"
					"ss_str_bin_thr = %u\n"
					"ss_wb_rpt_mode = %u\n"
					"ss_rssi_rpt_mode = %u\n"
					"ss_rssi_thr = %d\n"
					"ss_pwr_format = %u\n"
					"ss_rpt_mode = %u\n"
					"ss_bin_scale = %u\n"
					"ss_dbm_adj = %u\n"
					"ss_chn_mask = %u\n"
					"status = %d\n\n",
					__func__,
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
					ret);
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

			if (ret < 0) {
				qdf_print(
				  "%s: "
				  "target_if_send_vdev_spectral_configure_cmd "
				  "failed with error=%d\n",
				  __func__,
				  ret);
				qdf_spin_unlock(&info->osps_lock);
				return ret;
			}

			qdf_mem_copy(&info->osps_cache.osc_params,
				     param,
				     sizeof(info->osps_cache.osc_params));
			qdf_spin_unlock(&info->osps_lock);
			break;

		default:
			qdf_print("%s: Unknown OL_SPECTRAL_INFO_SPEC_T "
				  "specifier\n",
				  __func__);
			return -EINVAL;
		}

	return 0;
}

/**
 * target_if_spectral_get_tsf64() - Get the last TSF received in WMI buffer
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: TSF value
 */
static u_int64_t target_if_spectral_get_tsf64(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;

	return spectral->tsf64;
}

/**
 * target_if_spectral_get_capability() - Get whether a given Spectral hardware
 * capability is available
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @type: Spectral hardware capability type
 *
 * Return: True if the capability is available, false if the capability is not
 * available
 */
u_int32_t target_if_spectral_get_capability(
	void *arg,
	 SPECTRAL_CAPABILITY_TYPE type)
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
u_int32_t target_if_spectral_set_rxfilter(void *arg, int rxfilter)
{
	/* Will not be required since enabling of spectral in firmware
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
u_int32_t target_if_spectral_get_rxfilter(void *arg)
{
	/* Will not be required since enabling of spectral in firmware
	 * will take care of this
	 */
	return 0;
}

/**
 * tgt_if_is_spectral_active() - Get whether Spectral is active
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
u_int32_t tgt_if_is_spectral_active(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	u_int8_t val = 0;
	int ret;

	ret = ol_spectral_info_read(spectral,
				    OL_SPECTRAL_INFO_SPEC_ACTIVE,
					&val,
					sizeof(val));

	if (ret != 0) {
		/* Could not determine if Spectral is active.
		 * Return false as a safe value.
		 * XXX: Consider changing the function prototype
		 * to be able to indicate failure to fetch value.
		 */
		return 0;
	}

	return val;
}

/**
 * tgt_if_is_spectral_enabled() - Get whether Spectral is enabled
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
u_int32_t tgt_if_is_spectral_enabled(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	u_int8_t val = 0;
	int ret;

	ret = ol_spectral_info_read(spectral,
				    OL_SPECTRAL_INFO_SPEC_ENABLED,
					&val,
					sizeof(val));

	if (ret != 0) {
		/* Could not determine if Spectral is enabled.
		 * Return false as a safe value.
		 * XXX: Consider changing the function prototype
		 * to be able to indicate failure to fetch value.
		 */
		return 0;
	}

	return val;
}

/**
 * tgt_if_start_spectral_scan() - Start Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: 1 on success, 0 on failure
 */
u_int32_t tgt_if_start_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	u_int8_t val = 1;
	u_int8_t enabled = 0;
	int ret;

	ret = ol_spectral_info_read(spectral,
				    OL_SPECTRAL_INFO_SPEC_ENABLED,
					&enabled,
					sizeof(enabled));

	if (ret != 0) {
		/* Could not determine if Spectral is enabled. Assume we need
		 * to enable it
		 */
		enabled = 0;
	}

	if (!enabled) {
		ret = ol_spectral_info_write(spectral,
					     OL_SPECTRAL_INFO_SPEC_ENABLED,
							&val,
							sizeof(val));

		if (ret != 0)
			return 0;
	}

	ret = ol_spectral_info_write(spectral,
				     OL_SPECTRAL_INFO_SPEC_ACTIVE,
						&val,
						sizeof(val));

	if (ret != 0)
		return 0;

	return 1;
}

/**
 * tgt_if_stop_spectral_scan() - Stop Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: 1 on success, 0 on failure
 */
u_int32_t tgt_if_stop_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	u_int8_t val = 0;
	int tempret, ret = 1;

	tempret = ol_spectral_info_write(spectral,
					 OL_SPECTRAL_INFO_SPEC_ACTIVE,
						&val,
						sizeof(val));

	if (tempret != 0)
		ret = 0;

	tempret = ol_spectral_info_write(spectral,
					 OL_SPECTRAL_INFO_SPEC_ENABLED,
						&val,
						sizeof(val));

	if (tempret != 0)
		ret = 0;

	return ret;
}

/**
 * target_if_spectral_get_extension_channel() - Get the current Extension
 *                                              channel (in MHz)
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: Current Extension channel (in MHz) on success, 0 on failure or if
 * extension channel is not present.
 */
u_int32_t target_if_spectral_get_extension_channel(void *arg)
{
	/* XXX: Once we expand to use cases where Spectral could be activated
	 * without a channel being set to VDEV, we need to consider returning a
	 * negative value in case of failure and having all callers handle this.
	 */

	struct target_if_spectral *spectral = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;
	u_int16_t sec20chan_freq = 0;

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
 * target_if_spectral_get_current_channel() - Get the current channel (in MHz)
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Return: Current channel (in MHz) on success, 0 on failure
 */
u_int32_t target_if_spectral_get_current_channel(void *arg)
{
	/* XXX: Once we expand to use cases where Spectral could be activated
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
u_int32_t target_if_spectral_reset_hw(void *arg)
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
u_int32_t target_if_spectral_get_chain_noise_floor(void *arg, int16_t *nf_buf)
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
int8_t target_if_spectral_get_ext_noisefloor(void *arg)
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
int8_t target_if_spectral_get_ctl_noisefloor(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_configure_params() - Configure user supplied Spectral
 *                                         parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Spectral parameters
 *
 * Return: 1 on success, 0 on failure.
 */
u_int32_t target_if_spectral_configure_params(
	void *arg,
	 struct spectral_config *params)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	int ret;

	ret = ol_spectral_info_write(spectral,
				     OL_SPECTRAL_INFO_SPEC_PARAMS,
						params,
						sizeof(*params));

	if (ret != 0)
		return 0;

	return 1;
}

/**
 * target_if_spectral_get_params() - Get user configured Spectral parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Pointer to buffer into which Spectral parameters should be copied
 *
 * Return: 1 on success, 0 on failure.
 */
u_int32_t target_if_spectral_get_params(
	void *arg,
	 struct spectral_config *params)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	int ret;

	ret = ol_spectral_info_read(spectral,
				    OL_SPECTRAL_INFO_SPEC_PARAMS,
					params,
					sizeof(*params));

	if (ret != 0)
		return 0;

	return 1;
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
static u_int32_t target_if_spectral_get_ent_mask(void *arg)
{
	not_yet_implemented();
	return 0;
}

/**
 * target_if_spectral_get_macaddr() - Get radio MAC address
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @addr: Pointer to buffer into which MAC address should be copied
 *
 * Return: 0 on success, -1 on failure
 */
static u_int32_t target_if_spectral_get_macaddr(void *arg, char *addr)
{
	uint8_t *myaddr = NULL;
	struct target_if_spectral *spectral = (struct target_if_spectral *)arg;
	struct wlan_objmgr_pdev *pdev = NULL;

	pdev = spectral->pdev_obj;

	wlan_pdev_obj_lock(pdev);
	myaddr = wlan_pdev_get_hw_macaddr(pdev);
	wlan_pdev_obj_unlock(pdev);
	qdf_mem_copy(addr, myaddr, IEEE80211_ADDR_LEN);

	return 0;
}

/**
 * init_spectral_capability() - Initialize Spectral capability
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * This is a workaround.
 *
 * Return: None
 */
void init_spectral_capability(struct target_if_spectral *spectral)
{
	struct spectral_caps *pcap = &spectral->capability;

	/* XXX : Workaround: Set Spectral capability */
	pcap = &spectral->capability;
	pcap->phydiag_cap = 1;
	pcap->radar_cap = 1;
	pcap->spectral_cap = 1;
	pcap->advncd_spectral_cap = 1;
}

/**
 * target_if_init_spectral_ops_common() - Initialize Spectral target_if internal
 * operations common to all Spectral chipset generations
 *
 * Return: None
 */
static void target_if_init_spectral_ops_common(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->get_tsf64               = target_if_spectral_get_tsf64;
	p_sops->get_capability          = target_if_spectral_get_capability;
	p_sops->set_rxfilter            = target_if_spectral_set_rxfilter;
	p_sops->get_rxfilter            = target_if_spectral_get_rxfilter;
#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
	/* Spectral simulation is currently intended for platform transitions
	 * where underlying HW support may not be available for some time.
	 * Hence, we do not currently provide a runtime switch to turn the
	 * simulation on or off.
	 * In case of future requirements where runtime switches are required,
	 * this can be added. But it is suggested to use application layer
	 * simulation as far as possible in such cases, since the main
	 * use of record and replay of samples would concern higher
	 * level sample processing rather than lower level delivery.
	 */
	p_sops->is_spectral_enabled     = tif_spectral_sim_is_spectral_enabled;
	p_sops->is_spectral_active      = tif_spectral_sim_is_spectral_active;
	p_sops->start_spectral_scan     = tif_spectral_sim_start_spectral_scan;
	p_sops->stop_spectral_scan      = tif_spectral_sim_stop_spectral_scan;
	p_sops->configure_spectral      = tif_spectral_sim_configure_params;
	p_sops->get_spectral_config     = tif_spectral_sim_get_params;
#else
	p_sops->is_spectral_enabled     = tgt_if_is_spectral_enabled;
	p_sops->is_spectral_active      = tgt_if_is_spectral_active;
	p_sops->start_spectral_scan     = tgt_if_start_spectral_scan;
	p_sops->stop_spectral_scan      = tgt_if_stop_spectral_scan;
	p_sops->configure_spectral      = target_if_spectral_configure_params;
	p_sops->get_spectral_config     = target_if_spectral_get_params;
#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */
	p_sops->get_extension_channel   =
		target_if_spectral_get_extension_channel;
	p_sops->get_ctl_noisefloor      =
		target_if_spectral_get_ctl_noisefloor;
	p_sops->get_ext_noisefloor      =
		target_if_spectral_get_ext_noisefloor;
	p_sops->get_ent_spectral_mask   = target_if_spectral_get_ent_mask;
	p_sops->get_mac_address         = target_if_spectral_get_macaddr;
	p_sops->get_current_channel     =
		target_if_spectral_get_current_channel;
	p_sops->reset_hw                = target_if_spectral_reset_hw;
	p_sops->get_chain_noise_floor   =
		target_if_spectral_get_chain_noise_floor;
}

/**
 * target_if_init_spectral_ops_gen2() - Initialize Spectral target_if internal
 * operations specific to Spectral chipset generation 2.
 *
 * Return: None
 */
static void target_if_init_spectral_ops_gen2(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->spectral_process_phyerr = spectral_process_phyerr_gen2;
}

/**
 * target_if_init_spectral_ops_gen3() - Initialize Spectral target_if internal
 * operations specific to Spectral chipset generation 3.
 *
 * Return: None
 */
static void target_if_init_spectral_ops_gen3(void)
{
	struct target_if_spectral_ops *p_sops = &spectral_ops;

	p_sops->spectral_process_phyerr = spectral_process_phyerr_gen3;
}

/**
 * target_if_init_spectral_ops() - Initialize target_if internal Spectral
 * operations.
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Return: None
 */
static void target_if_init_spectral_ops(struct target_if_spectral *spectral)
{
	target_if_init_spectral_ops_common();
	if (spectral->spectral_gen == SPECTRAL_GEN2)
		target_if_init_spectral_ops_gen2();
	else if (spectral->spectral_gen == SPECTRAL_GEN3)
		target_if_init_spectral_ops_gen3();
	else
		qdf_print("Invalid spetral generation\n");
}

/*
 * Dummy Functions:
 * These functions are initially registered to avoid any crashes due to
 * invocation of spectral functions before they are registered.
 */

static u_int64_t null_get_tsf64(void *arg)
{
	spectral_ops_not_registered("get_tsf64");
	return 0;
}

static u_int32_t null_get_capability(void *arg, SPECTRAL_CAPABILITY_TYPE type)
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

static u_int32_t null_set_rxfilter(void *arg, int rxfilter)
{
	spectral_ops_not_registered("set_rxfilter");
	return 1;
}

static u_int32_t null_get_rxfilter(void *arg)
{
	spectral_ops_not_registered("get_rxfilter");
	return 0;
}

static u_int32_t null_is_spectral_active(void *arg)
{
	spectral_ops_not_registered("is_spectral_active");
	return 1;
}

static u_int32_t null_is_spectral_enabled(void *arg)
{
	spectral_ops_not_registered("is_spectral_enabled");
	return 1;
}

static u_int32_t null_start_spectral_scan(void *arg)
{
	spectral_ops_not_registered("start_spectral_scan");
	return 1;
}

static u_int32_t null_stop_spectral_scan(void *arg)
{
	spectral_ops_not_registered("stop_spectral_scan");
	return 1;
}

static u_int32_t null_get_extension_channel(void *arg)
{
	spectral_ops_not_registered("get_extension_channel");
	return 1;
}

static int8_t null_get_ctl_noisefloor(void *arg)
{
	spectral_ops_not_registered("get_ctl_noisefloor");
	return 1;
}

static int8_t null_get_ext_noisefloor(void *arg)
{
	spectral_ops_not_registered("get_ext_noisefloor");
	return 0;
}

static u_int32_t null_configure_spectral(
	void *arg,
	 struct spectral_config *params)
{
	spectral_ops_not_registered("configure_spectral");
	return 0;
}

static u_int32_t null_get_spectral_config(
	void *arg,
	 struct spectral_config *params)
{
	spectral_ops_not_registered("get_spectral_config");
	return 0;
}

static u_int32_t null_get_ent_spectral_mask(void *arg)
{
	spectral_ops_not_registered("get_ent_spectral_mask");
	return 0;
}

static u_int32_t null_get_mac_address(void *arg, char *addr)
{
	spectral_ops_not_registered("get_mac_address");
	return 0;
}

static u_int32_t null_get_current_channel(void *arg)
{
	spectral_ops_not_registered("get_current_channel");
	return 0;
}

static u_int32_t null_reset_hw(void *arg)
{
	spectral_ops_not_registered("get_current_channel");
	return 0;
}

static u_int32_t null_get_chain_noise_floor(void *arg, int16_t *nf_buf)
{
	spectral_ops_not_registered("get_chain_noise_floor");
	return 0;
}

static int null_spectral_process_phyerr(
	struct target_if_spectral *spectral,
	u_int8_t *data,
	u_int32_t datalen,
	struct target_if_spectral_rfqual_info *p_rfqual,
	struct target_if_spectral_chan_info *p_chaninfo,
	u_int64_t tsf64,
	struct target_if_spectral_acs_stats *acs_stats)
{
	spectral_ops_not_registered("spectral_process_phyerr");
	return 0;
}

/**
 * target_if_spectral_init_dummy_function_table() -
 * Initialize target_if internal
 * Spectral operations to dummy functions
 * @ps: Pointer to Spectral target_if internal private data
 *
 * Return: None
 */
static void target_if_spectral_init_dummy_function_table(
	struct target_if_spectral *ps)
{
	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(ps);

	p_sops->get_tsf64               = null_get_tsf64;
	p_sops->get_capability          = null_get_capability;
	p_sops->set_rxfilter            = null_set_rxfilter;
	p_sops->get_rxfilter            = null_get_rxfilter;
	p_sops->is_spectral_enabled     = null_is_spectral_enabled;
	p_sops->is_spectral_active      = null_is_spectral_active;
	p_sops->start_spectral_scan     = null_start_spectral_scan;
	p_sops->stop_spectral_scan      = null_stop_spectral_scan;
	p_sops->get_extension_channel   = null_get_extension_channel;
	p_sops->get_ctl_noisefloor      = null_get_ctl_noisefloor;
	p_sops->get_ext_noisefloor      = null_get_ext_noisefloor;
	p_sops->configure_spectral      = null_configure_spectral;
	p_sops->get_spectral_config     = null_get_spectral_config;
	p_sops->get_ent_spectral_mask   = null_get_ent_spectral_mask;
	p_sops->get_mac_address         = null_get_mac_address;
	p_sops->get_current_channel     = null_get_current_channel;
	p_sops->reset_hw                = null_reset_hw;
	p_sops->get_chain_noise_floor   = null_get_chain_noise_floor;
	p_sops->spectral_process_phyerr = null_spectral_process_phyerr;
}

/**
 * target_if_spectral_register_funcs() - Initialize target_if internal Spectral
 * operations
 * @spectral: Pointer to Spectral target_if internal private data
 * @p: Pointer to Spectral function table
 *
 * Return: None
 */
static void target_if_spectral_register_funcs(
	struct target_if_spectral *spectral,
	 struct target_if_spectral_ops *p)
{
	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	p_sops->get_tsf64               = p->get_tsf64;
	p_sops->get_capability          = p->get_capability;
	p_sops->set_rxfilter            = p->set_rxfilter;
	p_sops->get_rxfilter            = p->get_rxfilter;
	p_sops->is_spectral_enabled     = p->is_spectral_enabled;
	p_sops->is_spectral_active      = p->is_spectral_active;
	p_sops->start_spectral_scan     = p->start_spectral_scan;
	p_sops->stop_spectral_scan      = p->stop_spectral_scan;
	p_sops->get_extension_channel   = p->get_extension_channel;
	p_sops->get_ctl_noisefloor      = p->get_ctl_noisefloor;
	p_sops->get_ext_noisefloor      = p->get_ext_noisefloor;
	p_sops->configure_spectral      = p->configure_spectral;
	p_sops->get_spectral_config     = p->get_spectral_config;
	p_sops->get_ent_spectral_mask   = p->get_ent_spectral_mask;
	p_sops->get_mac_address         = p->get_mac_address;
	p_sops->get_current_channel     = p->get_current_channel;
	p_sops->reset_hw                = p->reset_hw;
	p_sops->get_chain_noise_floor   = p->get_chain_noise_floor;
	p_sops->spectral_process_phyerr = p->spectral_process_phyerr;
}

/**
 * target_if_spectral_clear_stats() - Clear Spectral stats
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Return: None
 */
static void target_if_spectral_clear_stats(struct target_if_spectral *spectral)
{
	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	qdf_mem_zero(
		&spectral->ath_spectral_stats,
		 sizeof(struct target_if_spectral_stats));
	spectral->ath_spectral_stats.last_reset_tstamp =
		p_sops->get_tsf64(spectral);
}

/**
 * target_if_spectral_check_hw_capability() - Check whether HW supports spectral
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * Return: True if HW supports Spectral, false if HW does not support Spectral
 */
static int target_if_spectral_check_hw_capability(
			struct target_if_spectral *spectral)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct spectral_caps *pcap  = NULL;
	int is_spectral_supported = true;

	p_sops      = GET_TIF_SPECTRAL_OPS(spectral);
	pcap        = &spectral->capability;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_PHYDIAG) == false) {
		is_spectral_supported = false;
		qdf_print("SPECTRAL : No PHYDIAG support\n");
		return is_spectral_supported;
	}
		pcap->phydiag_cap = 1;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_RADAR) == false) {
		is_spectral_supported = false;
		qdf_print("SPECTRAL : No RADAR support\n");
		return is_spectral_supported;
	}
		pcap->radar_cap = 1;

	if (
		p_sops->get_capability(spectral,
		 SPECTRAL_CAP_SPECTRAL_SCAN) == false) {
		is_spectral_supported = false;
		qdf_print("SPECTRAL : No SPECTRAL SUPPORT\n");
		return is_spectral_supported;
	}
		pcap->spectral_cap = 1;

	if (p_sops->get_capability(spectral, SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN)
				== false) {
		qdf_print("SPECTRAL : No ADVANCED SPECTRAL SUPPORT\n");
	} else {
		pcap->advncd_spectral_cap = 1;
	}

	return is_spectral_supported;
}

/**
 * target_if_spectral_init_param_defaults() - Initialize Spectral
 * parameter defaults
 * @spectral: Pointer to Spectral target_if internal private data
 *
 * It is the caller's responsibility to ensure that the Spectral parameters
 * structure passed as part of Spectral target_if internal private data is
 * valid.
 *
 * Return: None
 */
static void target_if_spectral_init_param_defaults(
	struct target_if_spectral *spectral)
{
	struct spectral_config *params = &spectral->params;

	params->ss_count = SPECTRAL_SCAN_COUNT_DEFAULT;
	params->ss_period = SPECTRAL_SCAN_PERIOD_DEFAULT;
	params->ss_spectral_pri = SPECTRAL_SCAN_PRIORITY_DEFAULT;
	params->ss_fft_size = SPECTRAL_SCAN_FFT_SIZE_DEFAULT;
	params->ss_gc_ena = SPECTRAL_SCAN_GC_ENA_DEFAULT;
	params->ss_restart_ena = SPECTRAL_SCAN_RESTART_ENA_DEFAULT;
	params->ss_noise_floor_ref = SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT;
	params->ss_init_delay = SPECTRAL_SCAN_INIT_DELAY_DEFAULT;
	params->ss_nb_tone_thr = SPECTRAL_SCAN_NB_TONE_THR_DEFAULT;
	params->ss_str_bin_thr = SPECTRAL_SCAN_STR_BIN_THR_DEFAULT;
	params->ss_wb_rpt_mode = SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT;
	params->ss_rssi_rpt_mode = SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT;
	params->ss_rssi_thr = SPECTRAL_SCAN_RSSI_THR_DEFAULT;
	params->ss_pwr_format = SPECTRAL_SCAN_PWR_FORMAT_DEFAULT;
	params->ss_rpt_mode = SPECTRAL_SCAN_RPT_MODE_DEFAULT;
	params->ss_bin_scale = SPECTRAL_SCAN_BIN_SCALE_DEFAULT;
	params->ss_dbm_adj = SPECTRAL_SCAN_DBM_ADJ_DEFAULT;
	/*
	 * XXX
	 * SPECTRAL_SCAN_CHN_MASK_DEFAULT (0x1) specifies that chain 0 is to be
	 * used
	 * for Spectral. This is expected to be an optimal configuration for
	 * most chipsets considering aspects like power save. But this can later
	 * optionally be changed to be set to the default system Rx chainmask
	 * advertised by FW (if required for some purpose), once the Convergence
	 * framework supports such retrieval at pdev attach time.
	 */
	params->ss_chn_mask = SPECTRAL_SCAN_CHN_MASK_DEFAULT;
	params->ss_short_report = SPECTRAL_SCAN_SHORT_REPORT_DEFAULT;
	params->ss_fft_period = SPECTRAL_SCAN_FFT_PERIOD_DEFAULT;
}

/**
 * target_if_spectral_detach() - De-initialize target_if Spectral
 * @pdev: Pointer to pdev object
 *
 * Return: None
 */
static void target_if_spectral_detach(struct target_if_spectral *spectral)
{
	qdf_print("spectral detach\n");
	qdf_spinlock_destroy(&spectral->ol_info.osps_lock);

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
	target_if_spectral_sim_detach(spectral);
#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	target_if_spectral_destroy_netlink(spectral);
#endif

	qdf_spinlock_destroy(&spectral->ath_spectral_lock);
	qdf_spinlock_destroy(&spectral->noise_pwr_reports_lock);

	if (spectral) {
		qdf_mem_free(spectral);
		spectral = NULL;
	}
}

/**
 * target_if_pdev_spectral_init() - Initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Return: On success, pointer to Spectral target_if internal private data, on
 * failure, NULL
 */
void *target_if_pdev_spectral_init(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;
#ifdef CONFIG_WIN
	struct ol_ath_softc_net80211 *scn = NULL;
#endif
	struct pdev_osif_priv *osif_priv = NULL;

	osif_priv = wlan_pdev_get_ospriv(pdev);
#ifdef CONFIG_WIN
	scn = (struct ol_ath_softc_net80211 *)osif_priv->legacy_osif_priv;
	if (!scn) {
		qdf_print("%s: scn is NULL!\n", __func__);
		return NULL;
	}
#endif
	spectral = (struct target_if_spectral *)qdf_mem_malloc(
			sizeof(struct target_if_spectral));
	if (!spectral) {
		qdf_print("SPECTRAL : Memory allocation failed\n");
		return spectral;
	}
	qdf_mem_zero(spectral, sizeof(struct target_if_spectral));
	/* Store pdev in Spectral */
	spectral->pdev_obj = pdev;

	/* init the function ptr table */
	target_if_spectral_init_dummy_function_table(spectral);

	/* get spectral function table */
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	/* TODO : Should this be called here of after ath_attach ? */
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_PHYDIAG))
		qdf_print(KERN_INFO "HAL_CAP_PHYDIAG : Capable\n");

	SPECTRAL_TODO("Need to fix the capablity check for RADAR");
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_RADAR))
		qdf_print(KERN_INFO "HAL_CAP_RADAR   : Capable\n");

	SPECTRAL_TODO("Need to fix the capablity check for SPECTRAL\n");
	/* TODO : Should this be called here of after ath_attach ? */
	if (p_sops->get_capability(spectral, SPECTRAL_CAP_SPECTRAL_SCAN))
		qdf_print(KERN_INFO "HAL_CAP_SPECTRAL_SCAN : Capable\n");

	qdf_spinlock_create(&spectral->ath_spectral_lock);
	qdf_spinlock_create(&spectral->noise_pwr_reports_lock);
	target_if_spectral_clear_stats(spectral);

	qdf_spinlock_create(&spectral->spectral_skbqlock);
	STAILQ_INIT(&spectral->spectral_skbq);

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	target_if_spectral_init_netlink(spectral);
#endif

	/* Set the default values for spectral parameters */
	target_if_spectral_init_param_defaults(spectral);
#ifdef CONFIG_WIN
	if ((scn->soc->target_type == TARGET_TYPE_QCA8074) || (
		scn->soc->target_type == TARGET_TYPE_QCA6290)) {
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
		 spectral->tlvhdr_size = sizeof(SPECTRAL_PHYERR_TLV_GEN2);
	}
#else
	spectral->spectral_gen = SPECTRAL_GEN3;
	spectral->hdr_sig_exp = SPECTRAL_PHYERR_SIGNATURE_GEN3;
	spectral->tag_sscan_summary_exp =
		TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3;
	spectral->tag_sscan_fft_exp = TLV_TAG_SEARCH_FFT_REPORT_GEN3;
	spectral->tlvhdr_size = SPECTRAL_PHYERR_TLVSIZE_GEN3;
#endif
#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
	if (target_if_spectral_sim_attach(spectral)) {
		qdf_mem_free(spectral);
		return NULL;
	}
#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */
	target_if_init_spectral_ops(spectral);

	qdf_spinlock_create(&spectral->ol_info.osps_lock);
	spectral->ol_info.osps_cache.osc_is_valid = 0;

	target_if_spectral_register_funcs(spectral, &spectral_ops);

	if (target_if_spectral_check_hw_capability(spectral) == false) {
		target_if_spectral_detach(spectral);
		spectral = NULL;
	} else {
		/* TODO: Once the driver architecture transitions to chipset
		 * versioning based checks, reflect this here.
		 */
		spectral->is_160_format = false;
		spectral->is_lb_edge_extrabins_format = false;
		spectral->is_rb_edge_extrabins_format = false;
#ifdef CONFIG_WIN
		if (scn->soc->target_type == TARGET_TYPE_QCA9984 ||
		    scn->soc->target_type == TARGET_TYPE_QCA9888) {
			spectral->is_160_format = true;
			spectral->is_lb_edge_extrabins_format = true;
			spectral->is_rb_edge_extrabins_format = true;
		} else  if ((scn->soc->target_type == TARGET_TYPE_AR900B) &&
			    (scn->soc->target_revision == AR900B_REV_2)) {
			spectral->is_rb_edge_extrabins_format = true;
		}

		if (scn->soc->target_type == TARGET_TYPE_QCA9984 ||
		    scn->soc->target_type == TARGET_TYPE_QCA9888)
			spectral->is_sec80_rssi_war_required = true;
#else
	spectral->is_160_format = true;
	spectral->is_lb_edge_extrabins_format = true;
	spectral->is_rb_edge_extrabins_format = true;
#endif
	}

	return spectral;
}

/**
 * target_if_pdev_spectral_deinit() - De-initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Return: None
 */
void target_if_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		qdf_print("SPECTRAL : Module doesn't exist\n");
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
 * Return: 1 on success, 0 on failure
 */
int target_if_set_spectral_config(struct wlan_objmgr_pdev *pdev,
					 const u_int32_t threshtype,
						const u_int32_t value)
{
	struct spectral_config params;
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	if (!spectral) {
		qdf_print("%s: spectral object is NULL\n", __func__);
		return 0;
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
	/* print_spectral_params(&spectral->params); */
	return 1;
}

static int get_fft_bin_count(int fft_len)
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

static void init_upper_lower_flags(struct target_if_spectral *spectral)
{
	int current_channel = 0;
	int ext_channel = 0;
	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	current_channel = p_sops->get_current_channel(spectral);
	ext_channel     = p_sops->get_extension_channel(spectral);

	if ((current_channel == 0) || (ext_channel == 0))
		return;

	if (spectral->sc_spectral_20_40_mode) {
		/* HT40 mode */
		if (ext_channel < current_channel) {
			spectral->lower_is_extension = 1;
			spectral->upper_is_control   = 1;
			spectral->lower_is_control   = 0;
			spectral->upper_is_extension = 0;
		} else {
			spectral->lower_is_extension = 0;
			spectral->upper_is_control   = 0;
			spectral->lower_is_control   = 1;
			spectral->upper_is_extension = 1;
		}
	} else {
		/* HT20 mode, lower is always control */
		spectral->lower_is_extension = 0;
		spectral->upper_is_control   = 0;
		spectral->lower_is_control   = 1;
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
 * Return: 1 on success, 0 on failure.
 */
void target_if_get_spectral_config(struct wlan_objmgr_pdev *pdev,
					  struct spectral_config *param)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);

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
int target_if_spectral_scan_enable_params(
	struct target_if_spectral *spectral,
	struct spectral_config *spectral_params)
{
	int extension_channel   = 0;
	int current_channel     = 0;
	struct target_if_spectral_ops *p_sops    = NULL;
	struct wlan_objmgr_vdev *vdev = NULL;

	if (!spectral) {
		qdf_print("SPECTRAL : Spectral is NULL\n");
		return 1;
	}

	p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	if (!p_sops) {
		qdf_print("SPECTRAL : p_sops is NULL\n");
		return 1;
	}

	spectral->sc_spectral_noise_pwr_cal =
		spectral_params->ss_spectral_pri ? 1 : 0;

	/* check if extension channel is present */
	extension_channel   = p_sops->get_extension_channel(spectral);
	current_channel     = p_sops->get_current_channel(spectral);

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return 1;

	spectral->ch_width  = target_if_vdev_get_ch_width(vdev);
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
			/* qdf_print("SPECTRAL : (11AC) 20MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			spectral->sc_spectral_20_40_mode    = 0;

			spectral->spectral_numbins  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_fft_len  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			/* Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
				current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

		} else if (spectral->ch_width == CH_WIDTH_40MHZ) {
			/* qdf_print("SPECTRAL : (11AC) 40MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			spectral->sc_spectral_20_40_mode    =
				1;    /* TODO : Remove this variable */
			spectral->spectral_numbins  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_fft_len  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			  get_fft_bin_count(spectral->params.ss_fft_size);

			/* Initialize classifier params to be sent to user
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
			/* qdf_print("SPECTRAL : (11AC) 80MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			/* Set the FFT Size */
			spectral->sc_spectral_20_40_mode    =
				0;    /* TODO : Remove this variable */
			spectral->spectral_numbins  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_fft_len  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			  get_fft_bin_count(spectral->params.ss_fft_size);

			/* Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
				current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

			/* Initialize classifier params to be sent to user
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
			/* qdf_print("SPECTRAL : (11AC) 160MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			/* Set the FFT Size */

			/* The below applies to both 160 and 80+80 cases */

			spectral->sc_spectral_20_40_mode    =
				0;    /* TODO : Remove this variable */
			spectral->spectral_numbins  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_fft_len  =
			  get_fft_bin_count(spectral->params.ss_fft_size);
			spectral->spectral_data_len =
			  get_fft_bin_count(spectral->params.ss_fft_size);

			/* Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
				current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

			/* Initialize classifier params to be sent to user
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
		/* qdf_print("SPECTRAL : Legacy (Non-11AC)\n"); */
		/*
		 * The decision to find 20/40 mode is found based on the
		 * presence of extension channel
		 * instead of channel width, as the channel width can
		 * dynamically change
		 */

		if (extension_channel == 0) {
			/* qdf_print("SPECTRAL : (Legacy) 20MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			spectral->spectral_numbins                  =
				SPECTRAL_HT20_NUM_BINS;
			spectral->spectral_dc_index                 =
				SPECTRAL_HT20_DC_INDEX;
			spectral->spectral_fft_len                  =
				SPECTRAL_HT20_FFT_LEN;
			spectral->spectral_data_len                 =
				SPECTRAL_HT20_TOTAL_DATA_LEN;
			spectral->spectral_lower_max_index_offset   =
				-1; /* only valid in 20-40 mode */
			spectral->spectral_upper_max_index_offset   =
				-1; /* only valid in 20-40 mode */
			spectral->spectral_max_index_offset         =
				spectral->spectral_fft_len + 2;
			spectral->sc_spectral_20_40_mode            = 0;

			/* Initialize classifier params to be sent to user
			 * space classifier
			 */
			spectral->classifier_params.lower_chan_in_mhz =
				current_channel;
			spectral->classifier_params.upper_chan_in_mhz = 0;

		} else {
			/* qdf_print("SPECTRAL : (Legacy) 40MHz Channel Width
			 * (Channel = %d)\n", current_channel);
			 */
			spectral->spectral_numbins                  =
				SPECTRAL_HT40_TOTAL_NUM_BINS;
			spectral->spectral_fft_len                  =
				SPECTRAL_HT40_FFT_LEN;
			spectral->spectral_data_len                 =
				SPECTRAL_HT40_TOTAL_DATA_LEN;
			spectral->spectral_dc_index                 =
				SPECTRAL_HT40_DC_INDEX;
			spectral->spectral_max_index_offset         =
				-1; /* only valid in 20 mode */
			spectral->spectral_lower_max_index_offset   =
				spectral->spectral_fft_len + 2;
			spectral->spectral_upper_max_index_offset   =
				spectral->spectral_fft_len + 5;
			spectral->sc_spectral_20_40_mode            = 1;

			/* Initialize classifier params to be sent to user
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

	spectral->send_single_packet                    = 0;
	spectral->classifier_params.spectral_20_40_mode =
		spectral->sc_spectral_20_40_mode;
	spectral->classifier_params.spectral_dc_index   =
		spectral->spectral_dc_index;
	spectral->spectral_sent_msg                     = 0;
	spectral->classify_scan                         = 0;
	spectral->num_spectral_data                     = 0;

	if (!p_sops->is_spectral_active(spectral)) {
		p_sops->configure_spectral(spectral, spectral_params);
		p_sops->start_spectral_scan(spectral);
		/* qdf_print("Enabled spectral scan on channel %d\n",
		 * p_sops->get_current_channel(spectral));
		 */
	} else {
		/* qdf_print("Spectral scan is already ACTIVE on channel %d\n",
		 * p_sops->get_current_channel(spectral));
		 */
	}

	/* get current spectral configuration */
	p_sops->get_spectral_config(spectral, &spectral->params);

	init_upper_lower_flags(spectral);

#ifdef SPECTRAL_CLASSIFIER_IN_KERNEL
	init_classifier(sc);
#endif
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
int target_if_start_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		qdf_print("SPECTRAL : Spectral LMAC object is NUll  (%s)\n",
			  __func__);
		return -EPERM;
	}
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	qdf_spin_lock(&spectral->ath_spectral_lock);
	target_if_spectral_scan_enable_params(spectral, &spectral->params);
	qdf_spin_unlock(&spectral->ath_spectral_lock);

	return 0;
}

/**
 * target_if_stop_spectral_scan() - Stop spectral scan
 * @pdev: Pointer to pdev object
 *
 * API to stop the current on-going spectral scan
 *
 * Return: None
 */
void target_if_stop_spectral_scan(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		qdf_print("SPECTRAL : Spectral LMAC object is NUll  (%s)\n",
			  __func__);
		return;
	}
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	qdf_spin_lock(&spectral->ath_spectral_lock);
	p_sops->stop_spectral_scan(spectral);
	if (spectral->classify_scan) {
		SPECTRAL_TODO("Check if this logic is necessary");
		spectral->detects_control_channel   = 0;
		spectral->detects_extension_channel = 0;
		spectral->detects_above_dc          = 0;
		spectral->detects_below_dc          = 0;
		spectral->classify_scan             = 0;
	}

	spectral->send_single_packet        = 0;
	spectral->sc_spectral_scan          = 0;
	spectral->sc_spectral_noise_pwr_cal = 0;

	/*
	 * Reset the priority because it stops WLAN rx.
	 * If it is needed to set, user has to set it explicitly
	 *
	 */
	/* Reset Priority */
	spectral->params.ss_spectral_pri    = 0;
	qdf_spin_unlock(&spectral->ath_spectral_lock);
}

/**
 * target_if_is_spectral_active() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool target_if_is_spectral_active(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	return p_sops->is_spectral_active(spectral);
}

/**
 * target_if_is_spectral_enabled() - Get whether Spectral is enabled
 * @pdev: Pointer to pdev object
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
bool target_if_is_spectral_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	return p_sops->is_spectral_enabled(spectral);
}

/**
 * target_if_set_debug_level() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * Return: 0 in case of success
 */
int target_if_set_debug_level(struct wlan_objmgr_pdev *pdev,
				     u_int32_t debug_level)
{
	spectral_debug_level = (ATH_DEBUG_SPECTRAL << debug_level);
	return 0;
}

/**
 * target_if_get_debug_level() - Get debug level for Spectral
 * @pdev: Pointer to pdev object
 *
 * Return: Current debug level
 */
u_int32_t target_if_get_debug_level(struct wlan_objmgr_pdev *pdev)
{
	return spectral_debug_level;
}

/**
 * target_if_get_spectral_capinfo() - Get Spectral capability information
 * @pdev: Pointer to pdev object
 * @outdata: Buffer into which data should be copied
 *
 * Return: void
 */
void target_if_get_spectral_capinfo(
	struct wlan_objmgr_pdev *pdev,
	 void *outdata)
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
 * Return: void
 */
void target_if_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
					     void *outdata)
{
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	qdf_mem_copy(outdata, &spectral->diag_stats,
		     sizeof(struct spectral_diag_stats));
}

void target_if_register_wmi_spectral_cmd_ops(
	struct wlan_objmgr_pdev *pdev,
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
 * target_if_sptrl_register_tx_ops() - Register Spectral target_if Tx Ops
 * @tx_ops: Tx Ops
 *
 * Return: void
 */
void target_if_sptrl_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_init     =
		target_if_pdev_spectral_init;
	tx_ops->sptrl_tx_ops.sptrlto_pdev_spectral_deinit   =
		target_if_pdev_spectral_deinit;
	tx_ops->sptrl_tx_ops.sptrlto_set_spectral_config    =
		target_if_set_spectral_config;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_config    =
		target_if_get_spectral_config;
	tx_ops->sptrl_tx_ops.sptrlto_start_spectral_scan    =
		target_if_start_spectral_scan;
	tx_ops->sptrl_tx_ops.sptrlto_stop_spectral_scan     =
		target_if_stop_spectral_scan;
	tx_ops->sptrl_tx_ops.sptrlto_is_spectral_active     =
		target_if_is_spectral_active;
	tx_ops->sptrl_tx_ops.sptrlto_is_spectral_enabled    =
		target_if_is_spectral_enabled;
	tx_ops->sptrl_tx_ops.sptrlto_set_debug_level        =
		target_if_set_debug_level;
	tx_ops->sptrl_tx_ops.sptrlto_get_debug_level        =
		target_if_get_debug_level;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_capinfo   =
		target_if_get_spectral_capinfo;
	tx_ops->sptrl_tx_ops.sptrlto_get_spectral_diagstats =
		target_if_get_spectral_diagstats;
	tx_ops->sptrl_tx_ops.sptrlto_register_wmi_spectral_cmd_ops =
		target_if_register_wmi_spectral_cmd_ops;
}
EXPORT_SYMBOL(target_if_sptrl_register_tx_ops);

/**
 * target_if_spectral_send_intf_found_msg() - Send message to application layer
 * indicating that interference has been found
 * @pdev: Pointer to pdev
 * @cw_int: 1 if CW interference is found, 0 if WLAN interference is found
 * @dcs_enabled: 1 if DCS is enabled, 0 if DCS is disabled
 *
 * Return: None
 */
void target_if_spectral_send_intf_found_msg(
	struct wlan_objmgr_pdev *pdev,
	u_int16_t cw_int, u_int32_t dcs_enabled)
{
#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	SPECTRAL_SAMP_MSG *msg = NULL;
	struct target_if_spectral_ops *p_sops   = NULL;
	struct target_if_spectral *spectral = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	target_if_spectral_prep_skb(spectral);
	if (spectral->spectral_skb) {
		spectral->spectral_nlh =
			(struct nlmsghdr *)spectral->spectral_skb->data;
		msg = (SPECTRAL_SAMP_MSG *)NLMSG_DATA(spectral->spectral_nlh);
		msg->int_type = cw_int ?
				SPECTRAL_DCS_INT_CW : SPECTRAL_DCS_INT_WIFI;
		msg->dcs_enabled = dcs_enabled;
		msg->signature = SPECTRAL_SIGNATURE;
		p_sops->get_mac_address(spectral, msg->macaddr);
		target_if_spectral_bcast_msg(spectral);
	}
#endif
}
EXPORT_SYMBOL(target_if_spectral_send_intf_found_msg);

