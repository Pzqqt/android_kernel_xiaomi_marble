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
 * ucfg_fwol_get_neighbor_report_cfg() - Get neighbor report config params
 * @psoc: Pointer to psoc object
 * @fwol_neighbor_report_cfg: Pointer to return neighbor report config
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_get_neighbor_report_cfg(struct wlan_objmgr_psoc *psoc,
				  struct wlan_fwol_neighbor_report_cfg
				  *fwol_neighbor_report_cfg);

/**
 * ucfg_fwol_get_neighbor_report_req() - Get neighbor report request bit
 * @psoc: Pointer to psoc object
 * @neighbor_report_req: Pointer to return value
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_is_neighbor_report_req_supported(struct wlan_objmgr_psoc *psoc,
					   bool *neighbor_report_req);

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

/**
 * ucfg_get_smart_chainmask_enabled() - Assigns smart_chainmask_enabled value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_get_smart_chainmask_enabled(struct wlan_objmgr_psoc *psoc,
					    bool *smart_chainmask_enabled);

/**
 * ucfg_fwol_get_rts_profile() - Assigns get_rts_profile value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_rts_profile(struct wlan_objmgr_psoc *psoc,
				     uint16_t *get_rts_profile);

/**
 * ucfg_fwol_get_enable_fw_log_level() - Assigns enable_fw_log_level value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_enable_fw_log_level(struct wlan_objmgr_psoc *psoc,
					     uint16_t *enable_fw_log_level);

/**
 * ucfg_fwol_get_enable_fw_log_type() - Assigns enable_fw_log_type value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_enable_fw_log_type(struct wlan_objmgr_psoc *psoc,
					    uint16_t *enable_fw_log_type);
/**
 * ucfg_fwol_get_enable_fw_module_log_level() - Assigns
 * enable_fw_module_log_level string
 * @psoc: pointer to the psoc object
 * @enable_fw_module_log_level:
 * pointer to enable_fw_module_log_level array
 * @enable_fw_module_log_level_num:
 * pointer to enable_fw_module_log_leve array element num
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_enable_fw_module_log_level(
				struct wlan_objmgr_psoc *psoc,
				uint8_t **enable_fw_module_log_level,
				uint8_t *enable_fw_module_log_level_num);

#ifdef FEATURE_WLAN_RA_FILTERING
/**
 * ucfg_fwol_set_is_rate_limit_enabled() - Sets the is_rate_limit_enabled value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_set_is_rate_limit_enabled(struct wlan_objmgr_psoc *psoc,
					       bool is_rate_limit_enabled);

/**
 * ucfg_fwol_get_is_rate_limit_enabled() - Assigns is_rate_limit_enabled value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_is_rate_limit_enabled(struct wlan_objmgr_psoc *psoc,
					       bool *is_rate_limit_enabled);

#endif /* FEATURE_WLAN_RA_FILTERING */

/**
 * ucfg_fwol_get_tsf_gpio_pin() - Assigns tsf_gpio_pin value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */

QDF_STATUS ucfg_fwol_get_tsf_gpio_pin(struct wlan_objmgr_psoc *psoc,
				      uint32_t *tsf_gpio_pin);

#ifdef DHCP_SERVER_OFFLOAD
/**
 * ucfg_fwol_get_enable_dhcp_server_offload()-Assign enable_dhcp_server_offload
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_fwol_get_enable_dhcp_server_offload(struct wlan_objmgr_psoc *psoc,
					 bool *enable_dhcp_server_offload);

/**
 * ucfg_fwol_get_dhcp_max_num_clients() - Assigns dhcp_max_num_clients value
 * @psoc: pointer to the psoc object
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_dhcp_max_num_clients(struct wlan_objmgr_psoc *psoc,
					      uint32_t *dhcp_max_num_clients);
#endif

/**
 * ucfg_fwol_get_tsf_ptp_options() - Get TSF Plus feature options
 * @psoc: pointer to the psoc object
 * @tsf_ptp_options: Pointer to return tsf ptp options
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_tsf_ptp_options(struct wlan_objmgr_psoc *psoc,
					 uint32_t *tsf_ptp_options);
/**
 * ucfg_fwol_get_lprx_enable() - Get LPRx feature enable status
 * @psoc: pointer to the psoc object
 * @lprx_enable: Pointer to return LPRX feature enable status
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_lprx_enable(struct wlan_objmgr_psoc *psoc,
				     bool *lprx_enable);

/**
 * ucfg_fwol_get_sae_enable() - Get SAE feature enable status
 * @psoc: pointer to the psoc object
 * @sae_enable: Pointer to return SAE feature enable status
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_sae_enable(struct wlan_objmgr_psoc *psoc,
				    bool *sae_enable);

/**
 * ucfg_fwol_get_gcmp_enable() - Get GCMP feature enable status
 * @psoc: pointer to the psoc object
 * @gcmp_enable: Pointer to return GCMP feature enable status
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_gcmp_enable(struct wlan_objmgr_psoc *psoc,
				     bool *gcmp_enable);

/**
 * ucfg_fwol_get_enable_tx_sch_delay() - Get enable tx sch delay
 * @psoc: pointer to the psoc object
 * @enable_tx_sch_delay: Pointer to return enable_tx_sch_delay value
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_enable_tx_sch_delay(struct wlan_objmgr_psoc *psoc,
					     uint8_t *enable_tx_sch_delay);

/**
 * ucfg_fwol_get_enable_secondary_rate() - Get enable secondary rate
 * @psoc: pointer to the psoc object
 * @enable_tx_sch_delay: Pointer to return enable secondary rate value
 *
 * Return: QDF Status
 */
QDF_STATUS ucfg_fwol_get_enable_secondary_rate(struct wlan_objmgr_psoc *psoc,
					       uint32_t *enable_secondary_rate);
#endif /* _WLAN_FWOL_UCFG_API_H_ */
