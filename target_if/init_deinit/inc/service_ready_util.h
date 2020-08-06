/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: service_ready_util.h
 *
 * Public APIs to access (ext)service ready data from psoc object
 */
#ifndef _SERVICE_READY_UTIL_H_
#define _SERVICE_READY_UTIL_H_

#include "wlan_objmgr_psoc_obj.h"
#include "service_ready_param.h"
#include "target_if.h"

/**
 * init_deinit_chainmask_table_alloc()
 *	- allocate chainmask table capability list.
 * @service_ext_param: pointer to server ext param.
 *
 * Allocates capability list based on num_valid_chainmasks for that table.
 *
 * Return: QDF Status.
 */
QDF_STATUS init_deinit_chainmask_table_alloc(
		struct wlan_psoc_host_service_ext_param *service_ext_param);

/**
 * init_deinit_chainmask_table_free()
 *	-free chainmask table capability list.
 * @service_ext_param: pointer to server ext param.
 *
 * free capability list based on num_valid_chainmasks for that table.
 *
 * Return: QDF Status.
 */
QDF_STATUS init_deinit_chainmask_table_free(
		struct wlan_psoc_host_service_ext_param *service_ext_param);

/**
 * init_deinit_populate_service_bitmap() - populate service bitmap
 * @wmi_handle: wmi handle
 * @event: event buffer received from FW
 * @service_bitmap: service bitmap information
 *
 * API to populate service bit map
 *
 * Return: zero on successful population of service bitmap or failure flag
 */
int init_deinit_populate_service_bitmap(
		wmi_unified_t wmi_handle, uint8_t *event,
		uint32_t *service_bitmap);

/**
 * init_deinit_populate_fw_version_cmd() - populate FW version
 * @wmi_handle: wmi handle
 * @event: event buffer received from FW
 *
 * API to populate FW version
 *
 * Return: zero on successful population of fw_version command or failure flag
 */
int
init_deinit_populate_fw_version_cmd(wmi_unified_t wmi_handle, uint8_t *event);

/**
 * init_deinit_populate_target_cap() - populate target cap
 * @wmi_handle: wmi handle
 * @event: event buffer received from FW
 * @cap: target capability info object
 *
 * API to populate target cap
 *
 * Return: zero on successful population of target cap or failure flag
 */
int init_deinit_populate_target_cap(
		wmi_unified_t wmi_handle, uint8_t *event,
		struct wlan_psoc_target_capability_info *cap);

/**
 * init_deinit_populate_service_ready_ext_param() - populate service ready ext
 *                                                  parameter
 * @handle: WMI handle pointer
 * @evt: event buffer received from FW
 * @param: service ext param object
 *
 * API to populate service ready ext param
 *
 * Return: zero on successful parsing of service ready ext parameter or failure
 */
int init_deinit_populate_service_ready_ext_param(
		wmi_unified_t handle, uint8_t *evt,
		struct wlan_psoc_host_service_ext_param *param);

/**
 * init_deinit_populate_service_ready_ext2_param() - populate service ready ext2
 *                                                   parameter
 * @handle: WMI handle pointer
 * @evt: event buffer received from FW
 * @info: Target info handle
 *
 * API to populate service ready ext2 param
 *
 * Return: zero on successful parsing of service ready ext parameter or failure
 */
int init_deinit_populate_service_ready_ext2_param(
		wmi_unified_t handle, uint8_t *evt,
		struct tgt_info *info);

/**
 * init_deinit_populate_chainmask_tables() - populate chainmaks tables
 * @handle: WMI handle pointer
 * @evt: event buffer received from FW
 * @param: chainmaks_table object
 *
 * API to populate chainmaks tables
 *
 * Return: zero on successful parsing of chainmaks tables or failure flag
 */
int init_deinit_populate_chainmask_tables(
		wmi_unified_t handle, uint8_t *evt,
		struct wlan_psoc_host_chainmask_table *param);

/**
 * init_deinit_populate_mac_phy_capability() - populate mac phy capability
 * @handle: WMI handle pointer
 * @evt: event buffer received from FW
 * @hw_cap: hw_mode_caps object
 * @info: tgt_info object
 *
 * API to populate mac phy capability
 *
 * Return: zero on successful population of mac physical capability or failure
 */
int init_deinit_populate_mac_phy_capability(
	wmi_unified_t handle, uint8_t *evt,
	struct wlan_psoc_host_hw_mode_caps *hw_cap, struct tgt_info *info);

/**
 * init_deinit_populate_hw_mode_capability() - populate hw mode capability
 * @wmi_handle: WMI handle pointer
 * @event: event buffer received from FW
 * @tgt_hdl: target_psoc_info object
 *
 * API to populate hw mode capability
 *
 * Return: zero on successful parsing of hw mode capability or failure
 */
int init_deinit_populate_hw_mode_capability(
		wmi_unified_t wmi_handle,
		uint8_t *event, struct target_psoc_info *tgt_hdl);

/**
 * init_deinit_populate_dbr_ring_cap() - populate dbr ring capability
 * @psoc: PSOC object
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate dbr ring capability
 *
 * Return: zero on successful parsing of dbr ring capability or failure
 */
int init_deinit_populate_dbr_ring_cap(struct wlan_objmgr_psoc *psoc,
				      wmi_unified_t handle, uint8_t *event,
				      struct tgt_info *info);

/**
 * init_deinit_populate_dbr_ring_cap_ext2() - populate dbr ring capability
 *                                            from ext2 event
 * @psoc: PSOC object
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate dbr ring capability
 *
 * Return: zero on successful parsing of dbr ring capability or failure
 */
int init_deinit_populate_dbr_ring_cap_ext2(struct wlan_objmgr_psoc *psoc,
					   wmi_unified_t handle, uint8_t *event,
					   struct tgt_info *info);

/**
 * init_deinit_populate_spectral_bin_scale_params() - populate Spectral scaling
 * @psoc: PSOC object
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate Spectral bin scaling parameters
 *
 * Return: zero on successful parsing of scaling params or failure
 */
int init_deinit_populate_spectral_bin_scale_params(
				struct wlan_objmgr_psoc *psoc,
				wmi_unified_t handle, uint8_t *event,
				struct tgt_info *info);

/**
 * init_deinit_dbr_ring_cap_free() - free dbr ring capability
 * @tgt_psoc_info: target psoc info object
 *
 * API to free dbr ring capability
 *
 * Return: QDF_STATUS
 */
QDF_STATUS init_deinit_dbr_ring_cap_free(
				struct target_psoc_info *tgt_psoc_info);

/**
 * init_deinit_scan_radio_cap_free() - free scan radio capability
 * @tgt_psoc_info: target psoc info object
 *
 * API to free scan radio related capability information.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS init_deinit_scan_radio_cap_free(
				struct target_psoc_info *tgt_psoc_info);

/**
 * init_deinit_spectral_scaling_params_free() - free Spectral scaling params
 * @tgt_psoc_info: target psoc info object
 *
 * API to free Spectral scaling params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS init_deinit_spectral_scaling_params_free(
				struct target_psoc_info *tgt_psoc_info);

/**
 * init_deinit_populate_phy_reg_cap() - populate phy reg capability
 * @psoc: PSOC object
 * @wmi_handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 * @service_ready: service ready determiner
 *
 * API to populate phy reg capability
 *
 * Return: zero on successful parsing of physical reg capability or failure flag
 */
int init_deinit_populate_phy_reg_cap(struct wlan_objmgr_psoc *psoc,
				     wmi_unified_t wmi_handle, uint8_t *event,
				     struct tgt_info *info,
				     bool service_ready);

/**
 * init_deinit_populate_hal_reg_cap_ext2() - Populate HAL reg capabilities from
 * service ready ext2 event.
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate HAL reg capabilities from service ready ext2 event.
 *
 * Return: zero on successful parsing of physical reg capability or failure flag
 */
int init_deinit_populate_hal_reg_cap_ext2(wmi_unified_t handle, uint8_t *event,
					  struct tgt_info *info);

/**
 * init_deinit_populate_mac_phy_cap_ext2() - populate mac phy capabilities from
 * service ready ext2 event
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate mac phy capability from service ready ext2 event.
 *
 * Return: zero on successful population of mac physical capability or failure
 */
int init_deinit_populate_mac_phy_cap_ext2(wmi_unified_t handle, uint8_t *event,
					  struct tgt_info *info);

/**
 * init_deinit_populate_scan_radio_cap_ext2() - populate scan radio capabilities
 * from service ready ext2 event
 * @handle: WMI handle pointer
 * @event: event buffer received from FW
 * @info: tgt_info object
 *
 * API to populate scan radio capability from service ready ext2 event.
 *
 * Return: zero on successful population of scan radio or failure
 */
int init_deinit_populate_scan_radio_cap_ext2(wmi_unified_t handle,
					     uint8_t *event,
					     struct tgt_info *info);

/**
 * init_deinit_validate_160_80p80_fw_caps() - validate 160 80p80 fw caps
 * @psoc: PSOC object
 * @tgt_info: target_psoc_info object
 *
 * API to validate 160 80p80 fw caps
 *
 * Return: SUCCESS on successful validation of 160 80p80 forward caps or Failure
 */
QDF_STATUS init_deinit_validate_160_80p80_fw_caps(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);

/**
 * init_deinit_chainmask_config() - config chainmask
 * @psoc: PSOC object
 * @tgt_info: target_psoc_info object
 *
 * API to config chainmask
 *
 * Return: none
 */
void init_deinit_chainmask_config(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);

/**
 * init_deinit_is_service_ext_msg() - check service ext message
 * @psoc: PSOC object
 * @tgt_info: target_psoc_info object
 *
 * API to check whether service ext message is enabled
 *
 * Return: SUCCESS on successful check of service_ext message or Failure
 */
QDF_STATUS init_deinit_is_service_ext_msg(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);
/**
 * init_deinit_is_preferred_hw_mode_supported() - check support of preferred
 *                                                hw mode
 * @psoc: PSOC object
 * @tgt_info: target_psoc_info object
 *
 * API to check whether preferred hardware mode is enabled
 *
 * Return: True on support of preferred hardware support or False
 */
bool init_deinit_is_preferred_hw_mode_supported(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);

/**
 * init_deinit_wakeup_host_wait() - wakeup host wait
 * @psoc: PSOC object
 * @tgt_info: target_psoc_info object
 *
 * API to wakeup FW ready wait queue
 *
 * Return: None
 */
void init_deinit_wakeup_host_wait(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_info);

#endif /* _SERVICE_READY_UTIL_H_*/
