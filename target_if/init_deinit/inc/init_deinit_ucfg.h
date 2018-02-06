/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: init_deinit_ucfg.h
 *
 * Public APIs to get target_if info
 */

#ifndef _INIT_DEINIT_UCFG_H_
#define _INIT_DEINIT_UCFG_H_

/**
 * ucfg_get_service_param() - get service param
 * @psoc: pointer to psoc
 *
 * API to get service parameters
 *
 * Return: service parameter array pointer
 */
uint32_t *ucfg_get_service_param(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_is_service_param_bit_enabled() - check service param bit enabled
 * @service_param: Service bitmap array
 * @bit_idx: Service bit index to be checked
 *
 * API to check service param bitmap
 *
 * Return: true, if service is enabled
 *         false, if service is not enabled
 */
bool ucfg_is_service_param_bit_enabled(uint32_t *service_param,
					uint16_t bit_idx);

/**
 * ucfg_get_dfs_offload() - get dfs offload
 * @psoc: pointer to psoc
 * @is_tgt_offload: boolean flag for offload enabled/disabled
 *
 * API to get dfs offload
 *
 * Return: SUCCESS, if API is invoked without any failures
 */
QDF_STATUS ucfg_get_dfs_offload(struct wlan_objmgr_psoc *psoc,
			bool *is_tgt_offload);

/**
 * ucfg_get_tgt_res_cfg() - get target resource config
 * @psoc: pointer to psoc
 *
 * API to get target resource config
 *
 * Return: target resource configuration
 */
target_resource_config *ucfg_get_tgt_res_cfg(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_target_cap() - get target capability
 * @psoc: pointer to psoc
 *
 * API to get target capability
 *
 * Return: target capability Information
 */
struct wlan_psoc_target_capability_info *ucfg_get_target_cap(
				struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_pdev_idx() - get pdev id
 * @pdev: pointer to pdev
 *
 * API to get pdev id
 *
 * Return: pdev id
 */
int32_t ucfg_get_pdev_idx(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_get_pdev_target_type() - check pdev target type
 * @pdev: pointer to pdev
 * @target_type: target type ptr, it is assigned with pdev target_type
 *               target type stores the radio code
 *
 * API to check pdev target type
 *
 * Return: Success if found required target type else Failure
 */
QDF_STATUS ucfg_get_pdev_target_type(struct wlan_objmgr_pdev *pdev,
				uint32_t *target_type);

/**
 * ucfg_get_tgt_type() - get target type
 * @psoc: pointer to psoc
 *
 * API to get target type
 *
 * Return: target type (value to identify particular radio)
 */
uint32_t ucfg_get_tgt_type(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_tgt_version() - get target version
 * @psoc: pointer to psoc
 *
 * API to get target version
 *
 * Return: target version
 */
uint32_t ucfg_get_tgt_version(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_tgt_revision() - get target revision
 * @psoc: pointer to psoc
 *
 * API to get target revision
 *
 * Return: target revision
 */
uint32_t ucfg_get_tgt_revision(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_is_target_ar900b() - checks the target type
 * @psoc: pointer to psoc
 *
 * API to check target type
 *
 * Return: True on presence of required target type else false
 */
bool ucfg_is_target_ar900b(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_wmi_hdl() - get wmi handle
 * @psoc: pointer to psoc
 *
 * API to get wmi handle
 *
 * Return: wmi handler
 */
struct common_wmi_handle *ucfg_get_wmi_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_wmi_unified_hdl() - get wmi handle
 * @psoc: pointer to psoc
 *
 * API to get wmi handle
 *
 * Return: wmi handler
 */
wmi_unified_t ucfg_get_wmi_unified_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_htc_hdl() - get htc handler
 * @psoc: pointer to psoc
 *
 * API to get htc handle
 *
 * Return: htc handler
 */
struct common_htc_handle *ucfg_get_htc_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_set_htc_hdl() - set htc handle
 * @psoc: pointer to psoc
 * @htc_hdl: HTC handle
 *
 * API to set htc handle
 *
 * Return: void
 */
void ucfg_set_htc_hdl(struct wlan_objmgr_psoc *psoc,
			struct common_htc_handle *htc_hdl);

/**
 * ucfg_get_hif_hdl() - get hif handle
 * @psoc: pointer to psoc
 *
 * API to get hif handle
 *
 * Return: hif handler
 */
struct common_hif_handle *ucfg_get_hif_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_ol_hif_hdl() - get hif handle
 * @psoc: pointer to psoc
 *
 * API to get hif handle
 *
 * Return: hif handler
 */
struct hif_opaque_softc *ucfg_get_ol_hif_hdl(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_pdev_wmi_handle() - get pdev wmi handle
 * @pdev: pointer to dev
 *
 * API to get wmi handle
 *
 * Return: wmi handle
 */
struct common_wmi_handle *ucfg_get_pdev_wmi_handle(
		struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_get_pdev_wmi_unified_handle() - get pdev wmi handle
 * @pdev: pointer to dev
 *
 * API to get wmi handle
 *
 * Return: wmi handle
 */
wmi_unified_t ucfg_get_pdev_wmi_unified_handle(
		struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_get_psoc_feature_ptr() - get feature pointer
 * @psoc: pointer to psoc
 *
 * API to get legacy pointer
 *
 * Return: feature pointer
 */
void *ucfg_get_psoc_feature_ptr(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_pdev_feature_ptr() - get feature pointer
 * @pdev: pointer to pdev
 *
 * API to get legacy pointer
 *
 * Return: pdev feature pointer
 */
void *ucfg_get_pdev_feature_ptr(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_get_num_radios() - get number of radios
 * @psoc: pointer to psoc
 *
 * API to get number of radios
 *
 * Return: number of radios
 */
uint32_t ucfg_get_num_radios(struct wlan_objmgr_psoc *psoc);

#endif /* _INIT_DEINIT_UCFG_H_ */
