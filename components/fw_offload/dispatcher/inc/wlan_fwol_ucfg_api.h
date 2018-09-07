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
 * DOC: declare internal API related to the fwol component
 */

#ifndef _WLAN_FWOL_UCFG_API_H_
#define _WLAN_FWOL_UCFG_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>
#include "wlan_fw_offload_main.h"

#include "wlan_fw_offload_main.h"

/**
 * ucfg_fwol_psoc_open() - FWOL component Open
 * @psoc: pointer to psoc object
 *
 * Open the FWOL component and initialize the FWOL structure
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_fwol_psoc_close() - FWOL component close
 * @psoc: pointer to psoc object
 *
 * Close the FWOL component and clear the FWOL structures
 *
 * Return: None
 */
void ucfg_fwol_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_fwol_init() - initialize fwol_ctx context.
 *
 * This function initializes the fwol context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
QDF_STATUS ucfg_fwol_init(void);

/**
 * ucfg_fwol_deinit() - De initialize fwol_ctx context.
 *
 * This function De initializes fwol contex.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success else return error
 */
void ucfg_fwol_deinit(void);

/**
 * ucfg_fwol_get_coex_config_params() - Get coex config params
 * @psoc: Pointer to psoc object
 * @coex_config: Pointer to struct wlan_fwol_coex_config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_fwol_get_coex_config_params(struct wlan_objmgr_psoc *psoc,
				 struct wlan_fwol_coex_config *coex_config);

/**
 * ucfg_fwol_get_thermal_temp() - Get thermal temperature config params
 * @psoc: Pointer to psoc object
 * @thermal_temp: Pointer to struct wlan_fwol_thermal_temp
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_get_thermal_temp(struct wlan_objmgr_psoc *psoc,
			   struct wlan_fwol_thermal_temp *thermal_temp);

/**
 * ucfg_fwol_get_ie_whitelist() - Get IE whitelist param value
 * @psoc: Pointer to psoc object
 * @ie_whitelist: Pointer to return the IE whitelist param value
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_get_ie_whitelist(struct wlan_objmgr_psoc *psoc, bool *ie_whitelist);

/**
 * ucfg_fwol_set_ie_whitelist() - Set IE whitelist param value
 * @psoc: Pointer to psoc object
 * @ie_whitelist: Value to set IE whitelist param
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_set_ie_whitelist(struct wlan_objmgr_psoc *psoc, bool ie_whitelist);

/**
 * ucfg_validate_ie_bitmaps() - Validate all IE whitelist bitmap param values
 * @psoc: Pointer to psoc object
 *
 * Return: True if all bitmap values are valid, else false
 */
bool ucfg_validate_ie_bitmaps(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_fwol_get_all_whitelist_params() - Get all IE whitelist param values
 * @psoc: Pointer to psoc object
 * @whitelist: Pointer to struct wlan_fwol_ie_whitelist
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_get_all_whitelist_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_fwol_ie_whitelist *whitelist);

/** ucfg_fwol_get_ani_enabled() - Assigns the ani_enabled value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_ani_enabled(struct wlan_objmgr_psoc *psoc,
				     bool *ani_enabled);

/**
 * ucfg_fwol_get_ani_enabled() - Assigns the enable_rts_sifsbursting value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_enable_rts_sifsbursting(struct wlan_objmgr_psoc *psoc,
					    bool *enable_rts_sifsbursting);

/**
 * ucfg_get_max_mpdus_inampdu() - Assigns the max_mpdus_inampdu value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_max_mpdus_inampdu(struct wlan_objmgr_psoc *psoc,
				      uint8_t *max_mpdus_inampdu);

/**
 * ucfg_get_arp_ac_category() - Assigns the arp_ac_category value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_arp_ac_category(struct wlan_objmgr_psoc *psoc,
				    uint32_t *arp_ac_category);

/**
 * ucfg_get_enable_phy_reg_retention() - Assigns enable_phy_reg_retention value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_enable_phy_reg_retention(struct wlan_objmgr_psoc *psoc,
					     uint8_t *enable_phy_reg_retention);

/**
 * ucfg_get_upper_brssi_thresh() - Assigns upper_brssi_thresh value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_upper_brssi_thresh(struct wlan_objmgr_psoc *psoc,
				       uint16_t *upper_brssi_thresh);

/**
 * ucfg_get_lower_brssi_thresh() - Assigns lower_brssi_thresh value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_lower_brssi_thresh(struct wlan_objmgr_psoc *psoc,
				       uint16_t *lower_brssi_thresh);

/**
 * ucfg_get_enable_dtim_1chrx() - Assigns enable_dtim_1chrx value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_enable_dtim_1chrx(struct wlan_objmgr_psoc *psoc,
				      bool *enable_dtim_1chrx);

/**
 * ucfg_get_alternate_chainmask_enabled() - Assigns alt chainmask_enabled value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_get_alternative_chainmask_enabled(struct wlan_objmgr_psoc *psoc,
				       bool *alternative_chainmask_enabled);

#endif /* _WLAN_FWOL_UCFG_API_H_ */
