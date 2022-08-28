/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

 /**
 * @file wlan_reg_services_api.c
 * @brief contains regulatory service functions
 */


#include <qdf_status.h>
#include <qdf_types.h>
#include <qdf_module.h>
#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_reg_services_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include "../../core/src/reg_priv_objs.h"
#include "../../core/src/reg_utils.h"
#include "../../core/src/reg_services_common.h"
#include "../../core/src/reg_db.h"
#include "../../core/src/reg_db_parser.h"
#include <../../core/src/reg_build_chan_list.h>
#include <../../core/src/reg_opclass.h>
#include <../../core/src/reg_callbacks.h>
#include <../../core/src/reg_offload_11d_scan.h>
#include <wlan_objmgr_global_obj.h>

/**
 * wlan_reg_read_default_country() - Read the default country for the regdomain
 * @country: pointer to the country code.
 *
 * Return: None
 */
QDF_STATUS wlan_reg_read_default_country(struct wlan_objmgr_psoc *psoc,
					 uint8_t *country)
{
	/*
	 * Get the default country information
	 */
	return reg_read_default_country(psoc, country);
}

QDF_STATUS wlan_reg_read_current_country(struct wlan_objmgr_psoc *psoc,
					 uint8_t *country)
{
	/*
	 * Get the current country information
	 */
	return reg_read_current_country(psoc, country);
}

QDF_STATUS wlan_reg_get_max_5g_bw_from_country_code(uint16_t cc,
						    uint16_t *max_bw_5g)
{
	/*
	 * Get the max 5G bandwidth from country code
	 */
	return reg_get_max_5g_bw_from_country_code(cc, max_bw_5g);
}

QDF_STATUS wlan_reg_get_max_5g_bw_from_regdomain(uint16_t regdmn,
						 uint16_t *max_bw_5g)
{
	/*
	 * Get the max 5G bandwidth from regdomain pair value
	 */
	return reg_get_max_5g_bw_from_regdomain(regdmn, max_bw_5g);
}

#ifdef CONFIG_REG_CLIENT
QDF_STATUS
wlan_reg_get_6g_power_type_for_ctry(struct wlan_objmgr_psoc *psoc,
				    uint8_t *ap_ctry, uint8_t *sta_ctry,
				    enum reg_6g_ap_type *pwr_type_6g,
				    bool *ctry_code_match,
				    enum reg_6g_ap_type ap_pwr_type)
{
	return reg_get_6g_power_type_for_ctry(psoc, ap_ctry, sta_ctry,
					      pwr_type_6g, ctry_code_match,
					      ap_pwr_type);
}
#endif

/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: Status
 */
QDF_STATUS wlan_reg_get_dfs_region(struct wlan_objmgr_pdev *pdev,
			     enum dfs_reg *dfs_reg)
{
	/*
	 * Get the current dfs region
	 */
	reg_get_current_dfs_region(pdev, dfs_reg);

	return QDF_STATUS_SUCCESS;
}

bool wlan_reg_is_chan_disabled_and_not_nol(struct regulatory_channel *chan)
{
	return reg_is_chan_disabled_and_not_nol(chan);
}

QDF_STATUS wlan_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *chan_list)
{
	return reg_get_current_chan_list(pdev, chan_list);
}

qdf_export_symbol(wlan_reg_get_current_chan_list);

#ifdef CONFIG_REG_CLIENT
QDF_STATUS wlan_reg_get_secondary_current_chan_list(
					struct wlan_objmgr_pdev *pdev,
					struct regulatory_channel *chan_list)
{
	return reg_get_secondary_current_chan_list(pdev, chan_list);
}
#endif

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)
QDF_STATUS wlan_reg_get_6g_afc_chan_list(struct wlan_objmgr_pdev *pdev,
					 struct regulatory_channel *chan_list)
{
	return reg_get_6g_afc_chan_list(pdev, chan_list);
}

qdf_export_symbol(wlan_reg_get_6g_afc_chan_list);

QDF_STATUS
wlan_reg_get_6g_afc_mas_chan_list(struct wlan_objmgr_pdev *pdev,
				  struct regulatory_channel *chan_list)
{
	return reg_get_6g_afc_mas_chan_list(pdev, chan_list);
}

qdf_export_symbol(wlan_reg_get_6g_afc_mas_chan_list);

QDF_STATUS wlan_reg_psd_2_eirp(struct wlan_objmgr_pdev *pdev,
			       int16_t psd,
			       uint16_t ch_bw,
			       int16_t *eirp)
{
	return reg_psd_2_eirp(pdev, psd, ch_bw, eirp);
}

qdf_export_symbol(wlan_reg_psd_2_eirp);
#endif

/**
 * wlan_reg_get_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t wlan_reg_get_bw_value(enum phy_ch_width bw)
{
	return reg_get_bw_value(bw);
}

qdf_export_symbol(wlan_reg_get_bw_value);

/**
 * wlan_reg_set_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void wlan_reg_set_dfs_region(struct wlan_objmgr_pdev *pdev,
			     enum dfs_reg dfs_reg)
{
	reg_set_dfs_region(pdev, dfs_reg);
}

QDF_STATUS wlan_reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
		const uint8_t *country_alpha2, enum country_src source)
{

	return reg_get_domain_from_country_code(reg_domain_ptr,
			country_alpha2, source);
}


uint16_t wlan_reg_dmn_get_opclass_from_channel(uint8_t *country,
					       uint8_t channel,
					       uint8_t offset)
{
	return reg_dmn_get_opclass_from_channel(country, channel,
						offset);
}

uint8_t wlan_reg_get_band_cap_from_op_class(const uint8_t *country,
					    uint8_t num_of_opclass,
					    const uint8_t *opclass)
{
	return reg_get_band_cap_from_op_class(country,
					      num_of_opclass, opclass);
}

uint8_t wlan_reg_get_opclass_from_freq_width(uint8_t *country,
					     qdf_freq_t freq,
					     uint16_t ch_width,
					     uint16_t behav_limit)
{
	return reg_dmn_get_opclass_from_freq_width(country, freq, ch_width,
						   behav_limit);
}

void wlan_reg_dmn_print_channels_in_opclass(uint8_t *country,
					    uint8_t opclass)
{
	reg_dmn_print_channels_in_opclass(country, opclass);
}

uint16_t wlan_reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
						 uint8_t channel,
						 uint8_t opclass)
{
	return reg_dmn_get_chanwidth_from_opclass(country, channel,
						  opclass);
}

uint16_t wlan_reg_dmn_get_chanwidth_from_opclass_auto(uint8_t *country,
						      uint8_t channel,
						      uint8_t opclass)
{
	return reg_dmn_get_chanwidth_from_opclass_auto(country, channel,
						       opclass);
}

uint16_t wlan_reg_dmn_set_curr_opclasses(uint8_t num_classes,
					 uint8_t *class)
{
	return reg_dmn_set_curr_opclasses(num_classes, class);
}

uint16_t wlan_reg_dmn_get_curr_opclasses(uint8_t *num_classes,
					 uint8_t *class)
{
	return reg_dmn_get_curr_opclasses(num_classes, class);
}

QDF_STATUS
wlan_reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
			     struct regdmn_ap_cap_opclass_t *reg_ap_cap,
			     uint8_t *n_opclasses,
			     uint8_t max_supp_op_class,
			     bool global_tbl_lookup)
{
	return reg_get_opclass_details(pdev, reg_ap_cap, n_opclasses,
				       max_supp_op_class,
				       global_tbl_lookup);
}

enum country_src wlan_reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
					 uint8_t *alpha)
{
	return reg_get_cc_and_src(psoc, alpha);
}

QDF_STATUS wlan_regulatory_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_psoc_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg psoc obj create handler");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_psoc_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg psoc obj create handler");
		goto unreg_psoc_create;
	}

	status = wlan_objmgr_register_pdev_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg psoc obj create handler");
		goto unreg_psoc_destroy;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg psoc obj create handler");
		goto unreg_pdev_create;
	}
	channel_map = channel_map_global;
	reg_debug("regulatory handlers registered with obj mgr");

	return status;

unreg_pdev_create:
	status = wlan_objmgr_unregister_pdev_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_created_notification,
		NULL);

unreg_psoc_destroy:
	status = wlan_objmgr_unregister_psoc_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_psoc_obj_destroyed_notification,
		NULL);

unreg_psoc_create:
	status = wlan_objmgr_unregister_psoc_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_psoc_obj_created_notification,
		NULL);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_regulatory_deinit(void)
{
	QDF_STATUS status, ret_status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg pdev obj destroy handler");
		ret_status = status;
	}

	status = wlan_objmgr_unregister_pdev_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg pdev obj create handler");
		ret_status = status;
	}

	status = wlan_objmgr_unregister_psoc_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_psoc_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg psoc obj destroy handler");
		ret_status = status;
	}

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg psoc obj create handler");
		ret_status = status;
	}

	reg_debug("deregistered callbacks with obj mgr");

	return ret_status;
}

#ifdef CONFIG_BAND_6GHZ
static void
regulatory_assign_register_master_ext_handler(struct wlan_objmgr_psoc *psoc,
					struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	if (tx_ops->register_master_ext_handler)
		tx_ops->register_master_ext_handler(psoc, NULL);
}

static void
regulatory_assign_unregister_master_ext_handler(struct wlan_objmgr_psoc *psoc,
					struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	if (tx_ops->unregister_master_ext_handler)
		tx_ops->unregister_master_ext_handler(psoc, NULL);
}

QDF_STATUS wlan_reg_get_6g_ap_master_chan_list(
					struct wlan_objmgr_pdev *pdev,
					enum reg_6g_ap_type ap_pwr_type,
					struct regulatory_channel *chan_list)
{
	return  reg_get_6g_ap_master_chan_list(pdev, ap_pwr_type, chan_list);
}

#ifdef CONFIG_REG_CLIENT
const char *wlan_reg_get_power_string(enum reg_6g_ap_type power_type)
{
	return reg_get_power_string(power_type);
}
#endif

qdf_export_symbol(wlan_reg_get_6g_ap_master_chan_list);

#ifdef CONFIG_AFC_SUPPORT
static void regulatory_assign_register_afc_event_handler(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	if (tx_ops->register_afc_event_handler)
		tx_ops->register_afc_event_handler(psoc, NULL);
}

static void regulatory_assign_unregister_afc_event_handler(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	if (tx_ops->unregister_afc_event_handler)
		tx_ops->unregister_afc_event_handler(psoc, NULL);
}
#else
static void regulatory_assign_register_afc_event_handler(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}

static void regulatory_assign_unregister_afc_event_handler(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}
#endif
#else
static inline void
regulatory_assign_register_master_ext_handler(struct wlan_objmgr_psoc *psoc,
					      struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}

static inline void
regulatory_assign_unregister_master_ext_handler(struct wlan_objmgr_psoc *psoc,
						struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}

static void
regulatory_assign_register_afc_event_handler(struct wlan_objmgr_psoc *psoc,
					     struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}

static void
regulatory_assign_unregister_afc_event_handler(struct wlan_objmgr_psoc *psoc,
					       struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
}
#endif

QDF_STATUS regulatory_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->register_master_handler)
		tx_ops->register_master_handler(psoc, NULL);
	regulatory_assign_register_master_ext_handler(psoc, tx_ops);
	regulatory_assign_register_afc_event_handler(psoc, tx_ops);
	if (tx_ops->register_11d_new_cc_handler)
		tx_ops->register_11d_new_cc_handler(psoc, NULL);
	if (tx_ops->register_ch_avoid_event_handler)
		tx_ops->register_ch_avoid_event_handler(psoc, NULL);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS regulatory_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->unregister_11d_new_cc_handler)
		tx_ops->unregister_11d_new_cc_handler(psoc, NULL);
	if (tx_ops->unregister_master_handler)
		tx_ops->unregister_master_handler(psoc, NULL);
	regulatory_assign_unregister_master_ext_handler(psoc, tx_ops);
	regulatory_assign_unregister_afc_event_handler(psoc, tx_ops);
	if (tx_ops->unregister_ch_avoid_event_handler)
		tx_ops->unregister_ch_avoid_event_handler(psoc, NULL);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_cntry_set_pending() - Check if country set is pending
 * @pdev: Pointer to pdev object.
 * @psoc: Pointer to psoc object.
 */
static bool reg_is_cntry_set_pending(struct wlan_objmgr_pdev *pdev,
				     struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	uint8_t phy_id;

	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return false;
	}

	tx_ops = reg_get_psoc_tx_ops(psoc);

	if (tx_ops->get_phy_id_from_pdev_id)
		tx_ops->get_phy_id_from_pdev_id(
					psoc,
					wlan_objmgr_pdev_get_pdev_id(pdev),
					&phy_id);
	else
		phy_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	return (soc_reg->new_user_ctry_pending[phy_id] ||
		soc_reg->new_init_ctry_pending[phy_id] ||
		soc_reg->new_11d_ctry_pending[phy_id] ||
		soc_reg->world_country_pending[phy_id]);
}
#else
static inline bool reg_is_cntry_set_pending(struct wlan_objmgr_pdev *pdev,
					    struct wlan_objmgr_psoc *psoc)
{
	return false;
}
#endif

QDF_STATUS regulatory_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *parent_psoc;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj->pdev_opened = true;

	parent_psoc = wlan_pdev_get_psoc(pdev);

	if (reg_is_cntry_set_pending(pdev, parent_psoc))
		return QDF_STATUS_SUCCESS;

	reg_send_scheduler_msg_nb(parent_psoc, pdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS regulatory_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj->pdev_opened = false;

	psoc = wlan_pdev_get_psoc(pdev);
	soc_reg = reg_get_psoc_obj(psoc);
	if (!soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	reg_reset_ctry_pending_hints(soc_reg);

	return QDF_STATUS_SUCCESS;
}

uint8_t wlan_reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			      qdf_freq_t freq)
{
	return reg_freq_to_chan(pdev, freq);
}

qdf_export_symbol(wlan_reg_freq_to_chan);

qdf_freq_t wlan_reg_legacy_chan_to_freq(struct wlan_objmgr_pdev *pdev,
					uint8_t chan_num)
{
	return reg_legacy_chan_to_freq(pdev, chan_num);
}

QDF_STATUS wlan_reg_set_country(struct wlan_objmgr_pdev *pdev,
				       uint8_t *country)
{
	return reg_set_country(pdev, country);
}

QDF_STATUS wlan_reg_set_11d_country(struct wlan_objmgr_pdev *pdev,
				    uint8_t *country)
{
	return reg_set_11d_country(pdev, country);
}

bool wlan_reg_is_world(uint8_t *country)
{
	return reg_is_world_alpha2(country);
}

bool wlan_reg_is_us(uint8_t *country)
{
	return reg_is_us_alpha2(country);
}

bool wlan_reg_is_etsi(uint8_t *country)
{
	return reg_is_etsi_alpha2(country);
}

bool wlan_reg_ctry_support_vlp(uint8_t *country)
{
	return reg_ctry_support_vlp(country);
}

void wlan_reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					    void *cbk, void *arg)
{
	reg_register_chan_change_callback(psoc, (reg_chan_change_callback)cbk,
					  arg);

}

void wlan_reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					      void *cbk)
{
	reg_unregister_chan_change_callback(psoc,
					    (reg_chan_change_callback)cbk);
}

bool wlan_reg_is_11d_offloaded(struct wlan_objmgr_psoc *psoc)
{
	return reg_is_11d_offloaded(psoc);
}

bool wlan_reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc)
{
	return reg_11d_enabled_on_host(psoc);
}

bool wlan_reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_etsi13_regdmn(pdev);
}

bool wlan_reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev
						     *pdev)
{
	return reg_is_etsi13_srd_chan_allowed_master_mode(pdev);
}

bool wlan_reg_get_fcc_constraint(struct wlan_objmgr_pdev *pdev, uint32_t freq)
{
	return reg_get_fcc_constraint(pdev, freq);
}

QDF_STATUS wlan_reg_get_chip_mode(struct wlan_objmgr_pdev *pdev,
		uint64_t *chip_mode)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_REGULATORY);

	if (!pdev_priv_obj) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	*chip_mode = pdev_priv_obj->wireless_modes;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_reg_get_phybitmap(struct wlan_objmgr_pdev *pdev,
				  uint16_t *phybitmap)
{
	return reg_get_phybitmap(pdev, phybitmap);
}

bool wlan_reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc)
{
	return reg_is_11d_scan_inprogress(psoc);
}

QDF_STATUS wlan_reg_get_freq_range(struct wlan_objmgr_pdev *pdev,
		qdf_freq_t *low_2g,
		qdf_freq_t *high_2g,
		qdf_freq_t *low_5g,
		qdf_freq_t *high_5g)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_REGULATORY);

	if (!pdev_priv_obj) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if (low_2g)
		*low_2g = pdev_priv_obj->range_2g_low;

	if (high_2g)
		*high_2g = pdev_priv_obj->range_2g_high;

	if (low_5g)
		*low_5g = pdev_priv_obj->range_5g_low;

	if (high_5g)
		*high_5g = pdev_priv_obj->range_5g_high;

	return QDF_STATUS_SUCCESS;
}

struct wlan_lmac_if_reg_tx_ops *
wlan_reg_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return reg_get_psoc_tx_ops(psoc);
}

qdf_export_symbol(wlan_reg_get_tx_ops);

QDF_STATUS wlan_reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
		struct cur_regdmn_info *cur_regdmn)
{
	return reg_get_curr_regdomain(pdev, cur_regdmn);
}

uint8_t wlan_reg_max_5ghz_ch_num(void)
{
	return reg_max_5ghz_ch_num();
}

#ifdef CONFIG_CHAN_FREQ_API
qdf_freq_t wlan_reg_min_24ghz_chan_freq(void)
{
	return reg_min_24ghz_chan_freq();
}

qdf_freq_t wlan_reg_max_24ghz_chan_freq(void)
{
	return reg_max_24ghz_chan_freq();
}

qdf_freq_t wlan_reg_min_5ghz_chan_freq(void)
{
	return reg_min_5ghz_chan_freq();
}

qdf_freq_t wlan_reg_max_5ghz_chan_freq(void)
{
	return reg_max_5ghz_chan_freq();
}
#endif /* CONFIG_CHAN_FREQ_API */

bool wlan_reg_is_24ghz_ch_freq(qdf_freq_t freq)
{
	return reg_is_24ghz_ch_freq(freq);
}

bool wlan_reg_is_5ghz_ch_freq(qdf_freq_t freq)
{
	return reg_is_5ghz_ch_freq(freq);
}

bool wlan_reg_is_range_overlap_2g(qdf_freq_t low_freq, qdf_freq_t high_freq)
{
	return reg_is_range_overlap_2g(low_freq, high_freq);
}

bool wlan_reg_is_range_overlap_5g(qdf_freq_t low_freq, qdf_freq_t high_freq)
{
	return reg_is_range_overlap_5g(low_freq, high_freq);
}

bool wlan_reg_is_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return reg_is_freq_indoor(pdev, freq);
}

enum phy_ch_width
wlan_reg_get_next_lower_bandwidth(enum phy_ch_width ch_width)
{
	return get_next_lower_bandwidth(ch_width);
}

#ifdef CONFIG_REG_CLIENT
bool wlan_reg_is_freq_indoor_in_secondary_list(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq)
{
	return reg_is_freq_indoor_in_secondary_list(pdev, freq);
}
#endif

#ifdef CONFIG_BAND_6GHZ
bool wlan_reg_is_6ghz_chan_freq(uint16_t freq)
{
	return reg_is_6ghz_chan_freq(freq);
}

#ifdef CONFIG_6G_FREQ_OVERLAP
bool wlan_reg_is_range_only6g(qdf_freq_t low_freq, qdf_freq_t high_freq)
{
	return reg_is_range_only6g(low_freq, high_freq);
}

bool wlan_reg_is_range_overlap_6g(qdf_freq_t low_freq, qdf_freq_t high_freq)
{
	return reg_is_range_overlap_6g(low_freq, high_freq);
}
#endif

uint16_t wlan_reg_min_6ghz_chan_freq(void)
{
	return reg_min_6ghz_chan_freq();
}

uint16_t wlan_reg_max_6ghz_chan_freq(void)
{
	return reg_max_6ghz_chan_freq();
}

bool wlan_reg_is_6ghz_psc_chan_freq(uint16_t freq)
{
	return reg_is_6ghz_psc_chan_freq(freq);
}

bool wlan_reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return reg_is_6g_freq_indoor(pdev, freq);
}

QDF_STATUS
wlan_reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq, uint8_t bw,
				    enum reg_6g_ap_type reg_ap,
				    enum reg_6g_client_type reg_client,
				    bool is_psd,
				    uint8_t *tx_power)
{
	return reg_get_max_txpower_for_6g_tpe(pdev, freq, bw,
					      reg_ap,
					      reg_client, is_psd,
					      tx_power);
}
#endif /* CONFIG_BAND_6GHZ */

uint16_t
wlan_reg_get_band_channel_list(struct wlan_objmgr_pdev *pdev,
			       uint8_t band_mask,
			       struct regulatory_channel *channel_list)
{
	if (!pdev) {
		reg_err("pdev object is NULL");
		return 0;
	}

	return reg_get_band_channel_list(pdev, band_mask, channel_list);
}

#ifdef CONFIG_REG_CLIENT
uint16_t
wlan_reg_get_secondary_band_channel_list(struct wlan_objmgr_pdev *pdev,
					 uint8_t band_mask,
					 struct regulatory_channel
					 *channel_list)
{
	if (!pdev) {
		reg_err("pdev object is NULL");
		return 0;
	}

	return reg_get_secondary_band_channel_list(pdev, band_mask,
						   channel_list);
}
#endif

qdf_freq_t wlan_reg_chan_band_to_freq(struct wlan_objmgr_pdev *pdev,
				      uint8_t chan, uint8_t band_mask)
{
	return reg_chan_band_to_freq(pdev, chan, band_mask);
}

qdf_export_symbol(wlan_reg_chan_band_to_freq);

#ifdef CONFIG_49GHZ_CHAN
bool wlan_reg_is_49ghz_freq(qdf_freq_t freq)
{
	return reg_is_49ghz_freq(freq);
}
#endif /* CONFIG_49GHZ_CHAN */

uint8_t wlan_reg_ch_num(uint32_t ch_enum)
{
	return reg_ch_num(ch_enum);
}

qdf_freq_t wlan_reg_ch_to_freq(uint32_t ch_enum)
{
	return reg_ch_to_freq(ch_enum);
}

bool wlan_reg_is_regdmn_en302502_applicable(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_regdmn_en302502_applicable(pdev);
}

/**
 * wlan_reg_modify_pdev_chan_range() - Compute current channel list for the
 * modified regcap.
 * @pdev: pointer to struct wlan_objmgr_pdev
 *
 */
QDF_STATUS wlan_reg_modify_pdev_chan_range(struct wlan_objmgr_pdev *pdev)
{
	return reg_modify_pdev_chan_range(pdev);
}

QDF_STATUS wlan_reg_update_pdev_wireless_modes(struct wlan_objmgr_pdev *pdev,
					       uint32_t wireless_modes)
{
	return reg_update_pdev_wireless_modes(pdev, wireless_modes);
}
#ifdef DISABLE_UNII_SHARED_BANDS
QDF_STATUS wlan_reg_disable_chan_coex(struct wlan_objmgr_pdev *pdev,
				      uint8_t unii_5g_bitmap)
{
	return reg_disable_chan_coex(pdev, unii_5g_bitmap);
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
bool wlan_reg_is_same_band_freqs(qdf_freq_t freq1, qdf_freq_t freq2)
{
	return reg_is_same_band_freqs(freq1, freq2);
}

bool wlan_reg_is_frequency_valid_5g_sbs(qdf_freq_t curfreq, qdf_freq_t newfreq)
{
	return reg_is_frequency_valid_5g_sbs(curfreq, newfreq);
}

enum channel_enum wlan_reg_get_chan_enum_for_freq(qdf_freq_t freq)
{
	return reg_get_chan_enum_for_freq(freq);
}

bool wlan_reg_is_freq_present_in_cur_chan_list(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq)
{
	return reg_is_freq_present_in_cur_chan_list(pdev, freq);
}

bool wlan_reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq)
{
	return reg_is_etsi13_srd_chan_for_freq(pdev, freq);
}

bool wlan_reg_is_dsrc_freq(qdf_freq_t freq)
{
	return reg_is_dsrc_freq(freq);
}

void wlan_reg_update_nol_ch_for_freq(struct wlan_objmgr_pdev *pdev,
				     uint16_t *chan_freq_list,
				     uint8_t num_ch,
				     bool nol_ch)
{
	reg_update_nol_ch_for_freq(pdev, chan_freq_list, num_ch, nol_ch);
}

void wlan_reg_update_nol_history_ch_for_freq(struct wlan_objmgr_pdev *pdev,
					     uint16_t *ch_list,
					     uint8_t num_ch,
					     bool nol_history_ch)
{
	reg_update_nol_history_ch_for_freq(pdev,
					   ch_list,
					   num_ch,
					   nol_history_ch);
}

bool wlan_reg_is_dfs_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return reg_is_dfs_for_freq(pdev, freq);
}

bool wlan_reg_is_passive_or_disable_for_freq(struct wlan_objmgr_pdev *pdev,
					     qdf_freq_t freq)
{
	return reg_is_passive_or_disable_for_freq(pdev, freq);
}

bool wlan_reg_is_disable_for_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t freq)
{
	return reg_is_disable_for_freq(pdev, freq);
}

#ifdef CONFIG_REG_CLIENT
bool wlan_reg_is_disable_in_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq)
{
	return reg_is_disable_in_secondary_list_for_freq(pdev, freq);
}

bool wlan_reg_is_enable_in_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq)
{
	return reg_is_enable_in_secondary_list_for_freq(pdev, freq);
}

bool wlan_reg_is_dfs_in_secondary_list_for_freq(struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq)
{
	return reg_is_dfs_in_secondary_list_for_freq(pdev, freq);
}
#endif

bool wlan_reg_is_passive_for_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t freq)
{
	return reg_is_passive_for_freq(pdev, freq);
}

QDF_STATUS
wlan_reg_get_channel_list_with_power_for_freq(struct wlan_objmgr_pdev *pdev,
					      struct channel_power *ch_list,
					      uint8_t *num_chan)
{
	return reg_get_channel_list_with_power_for_freq(pdev,
							ch_list,
							num_chan);
}

bool
wlan_reg_chan_has_dfs_attribute_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq)
{
	return reg_chan_has_dfs_attribute_for_freq(pdev, freq);
}

#if defined(WLAN_FEATURE_11BE) && defined(CONFIG_REG_CLIENT)
enum channel_state
wlan_reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      enum phy_ch_width bw)
{
	struct ch_params params = {0};

	if (bw == CH_WIDTH_320MHZ) {
		const struct bonded_channel_freq *bonded_chan_ptr_ptr = NULL;

		return reg_get_5g_bonded_channel_for_freq(pdev, freq, bw,
							  &bonded_chan_ptr_ptr);
	}

	params.ch_width = bw;

	return reg_get_5g_bonded_channel_state_for_freq(pdev, freq, &params);
}

qdf_export_symbol(wlan_reg_get_5g_bonded_channel_state_for_freq);

#else

enum channel_state
wlan_reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      enum phy_ch_width bw)
{
	struct ch_params params = {0};

	params.ch_width = bw;

	return reg_get_5g_bonded_channel_state_for_freq(pdev, freq, &params);
}

qdf_export_symbol(wlan_reg_get_5g_bonded_channel_state_for_freq);
#endif

enum channel_state
wlan_reg_get_2g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      qdf_freq_t sec_ch_freq,
					      enum phy_ch_width bw)
{
	return reg_get_2g_bonded_channel_state_for_freq(pdev,
							freq,
							sec_ch_freq,
							bw);
}

void wlan_reg_set_channel_params_for_freq(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq,
					  qdf_freq_t sec_ch_2g_freq,
					  struct ch_params *ch_params)
{
	reg_set_channel_params_for_freq(pdev, freq, sec_ch_2g_freq, ch_params);
}

qdf_export_symbol(wlan_reg_set_channel_params_for_freq);

const struct bonded_channel_freq *
wlan_reg_get_bonded_chan_entry(qdf_freq_t freq, enum phy_ch_width chwidth)
{
	return reg_get_bonded_chan_entry(freq, chwidth);
}

#ifdef WLAN_FEATURE_11BE
void wlan_reg_fill_channel_list(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq,
				qdf_freq_t sec_ch_2g_freq,
				enum phy_ch_width ch_width,
				qdf_freq_t band_center_320,
				struct reg_channel_list *chan_list)
{
	reg_fill_channel_list(pdev, freq, sec_ch_2g_freq, ch_width,
			      band_center_320, chan_list);
}

bool wlan_reg_is_punc_bitmap_valid(enum phy_ch_width bw,
				   uint16_t puncture_bitmap)
{
	return reg_is_punc_bitmap_valid(bw, puncture_bitmap);
}

void wlan_reg_set_create_punc_bitmap(struct ch_params *ch_params,
				     bool is_create_punc_bitmap)
{
	reg_set_create_punc_bitmap(ch_params, is_create_punc_bitmap);
}
#endif

enum channel_state
wlan_reg_get_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq)
{
	return reg_get_channel_state_for_freq(pdev, freq);
}

#ifdef CONFIG_REG_CLIENT
enum channel_state wlan_reg_get_channel_state_from_secondary_list_for_freq(
						struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq)
{
	return reg_get_channel_state_from_secondary_list_for_freq(pdev, freq);
}
#endif

uint8_t wlan_reg_get_channel_reg_power_for_freq(struct wlan_objmgr_pdev *pdev,
						qdf_freq_t freq)
{
	return reg_get_channel_reg_power_for_freq(pdev, freq);
}

enum reg_wifi_band wlan_reg_freq_to_band(qdf_freq_t freq)
{
	return reg_freq_to_band(freq);
}
qdf_export_symbol(wlan_reg_freq_to_band);

qdf_freq_t wlan_reg_min_chan_freq(void)
{
	return reg_min_chan_freq();
}

qdf_freq_t wlan_reg_max_chan_freq(void)
{
	return reg_max_chan_freq();
}

void wlan_reg_freq_width_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq,
					  uint16_t chan_width,
					  bool global_tbl_lookup,
					  uint16_t behav_limit,
					  uint8_t *op_class,
					  uint8_t *chan_num)
{
	return reg_freq_width_to_chan_op_class(pdev, freq, chan_width,
					       global_tbl_lookup,
					       behav_limit,
					       op_class,
					       chan_num);
}

void wlan_reg_freq_width_to_chan_op_class_auto(struct wlan_objmgr_pdev *pdev,
					       qdf_freq_t freq,
					       uint16_t chan_width,
					       bool global_tbl_lookup,
					       uint16_t behav_limit,
					       uint8_t *op_class,
					       uint8_t *chan_num)
{
	reg_freq_width_to_chan_op_class_auto(pdev, freq, chan_width,
					     global_tbl_lookup,
					     behav_limit,
					     op_class,
					     chan_num);
}

qdf_export_symbol(wlan_reg_freq_width_to_chan_op_class_auto);

void wlan_reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq,
				    bool global_tbl_lookup,
				    uint16_t behav_limit,
				    uint8_t *op_class,
				    uint8_t *chan_num)
{
	return reg_freq_to_chan_op_class(pdev, freq,
					 global_tbl_lookup,
					 behav_limit,
					 op_class,
					 chan_num);
}

bool wlan_reg_is_freq_in_country_opclass(struct wlan_objmgr_pdev *pdev,
					 const uint8_t country[3],
					 uint8_t op_class,
					 qdf_freq_t chan_freq)
{
	return reg_is_freq_in_country_opclass(pdev, country,
					      op_class, chan_freq);
}

enum channel_state
wlan_reg_get_5g_bonded_channel_and_state_for_freq(struct wlan_objmgr_pdev *pdev,
						  uint16_t freq,
						  enum phy_ch_width bw,
						  const
						  struct bonded_channel_freq
						  **bonded_chan_ptr_ptr)
{
	/*
	 * Get channel frequencies and state from regulatory
	 */
	return reg_get_5g_bonded_channel_for_freq(pdev, freq, bw,
						  bonded_chan_ptr_ptr);
}

#if defined(WLAN_FEATURE_11BE) && defined(CONFIG_REG_CLIENT)
enum channel_state
wlan_reg_get_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq,
					   enum phy_ch_width bw,
					   qdf_freq_t sec_freq)
{
	if (WLAN_REG_IS_24GHZ_CH_FREQ(freq))
		return reg_get_2g_bonded_channel_state_for_freq(pdev, freq,
						       sec_freq, bw);
	if (bw == CH_WIDTH_320MHZ) {
		const struct bonded_channel_freq *bonded_chan_ptr_ptr = NULL;

		return reg_get_5g_bonded_channel_for_freq(pdev, freq, bw,
							  &bonded_chan_ptr_ptr);
	} else {
		struct ch_params params = {0};

		params.ch_width = bw;

		return reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								&params);
	}
}

qdf_export_symbol(wlan_reg_get_5g_bonded_channel_and_state_for_freq);

#else
enum channel_state
wlan_reg_get_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq,
					   enum phy_ch_width bw,
					   qdf_freq_t sec_freq)
{
	if (WLAN_REG_IS_24GHZ_CH_FREQ(freq)) {
		return reg_get_2g_bonded_channel_state_for_freq(pdev, freq,
						       sec_freq, bw);
	} else {
		struct ch_params params = {0};

		params.ch_width = bw;

		return reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								&params);
	}
}

qdf_export_symbol(wlan_reg_get_5g_bonded_channel_and_state_for_freq);
#endif
#endif /* CONFIG CHAN FREQ API */

uint16_t wlan_reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
				     uint8_t op_class,
				     bool global_tbl_lookup)
{
	return reg_get_op_class_width(pdev, op_class,
				      global_tbl_lookup);
}

bool wlan_reg_is_5ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return reg_is_5ghz_op_class(country, op_class);
}

bool wlan_reg_is_2ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return reg_is_2ghz_op_class(country, op_class);
}

bool wlan_reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			       uint8_t op_class)
{
	return reg_is_6ghz_op_class(pdev, op_class);
}

#ifdef CONFIG_REG_CLIENT
bool wlan_reg_is_6ghz_supported(struct wlan_objmgr_psoc *psoc)
{
	return reg_is_6ghz_supported(psoc);
}
#endif

#ifdef HOST_OPCLASS_EXT
qdf_freq_t
wlan_reg_country_chan_opclass_to_freq(struct wlan_objmgr_pdev *pdev,
				      const uint8_t country[3],
				      uint8_t chan, uint8_t op_class,
				      bool strict)
{
	return reg_country_chan_opclass_to_freq(pdev, country, chan, op_class,
						strict);
}
#endif

uint16_t wlan_reg_chan_opclass_to_freq(uint8_t chan,
				       uint8_t op_class,
				       bool global_tbl_lookup)
{
	if (!chan || !op_class)
		return 0;

	return reg_chan_opclass_to_freq(chan, op_class, global_tbl_lookup);
}

qdf_freq_t wlan_reg_chan_opclass_to_freq_auto(uint8_t chan, uint8_t op_class,
					      bool global_tbl_lookup)
{
	if (!chan || !op_class)
		return 0;

	return reg_chan_opclass_to_freq_auto(chan, op_class, global_tbl_lookup);
}

#ifdef CHECK_REG_PHYMODE
enum reg_phymode wlan_reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
					  enum reg_phymode phy_in,
					  qdf_freq_t freq)
{
	return reg_get_max_phymode(pdev, phy_in, freq);
}
#endif /* CHECK_REG_PHYMODE */

#ifdef CONFIG_REG_CLIENT
enum band_info wlan_reg_band_bitmap_to_band_info(uint32_t band_bitmap)
{
	return reg_band_bitmap_to_band_info(band_bitmap);
}
#endif

#if defined(CONFIG_BAND_6GHZ)
QDF_STATUS wlan_reg_get_rnr_tpe_usable(struct wlan_objmgr_pdev *pdev,
				       bool *reg_rnr_tpe_usable)
{
	return reg_get_rnr_tpe_usable(pdev, reg_rnr_tpe_usable);
}

QDF_STATUS wlan_reg_get_unspecified_ap_usable(struct wlan_objmgr_pdev *pdev,
					      bool *reg_unspecified_ap_usable)
{
	return reg_get_unspecified_ap_usable(pdev, reg_unspecified_ap_usable);
}

QDF_STATUS
wlan_reg_get_cur_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_ap_type *reg_cur_6g_ap_pwr_type)
{
	return reg_get_cur_6g_ap_pwr_type(pdev, reg_cur_6g_ap_pwr_type);
}

QDF_STATUS
wlan_reg_get_cur_6g_client_type(struct wlan_objmgr_pdev *pdev,
				enum reg_6g_client_type
				*reg_cur_6g_client_mobility_type)
{
	return reg_get_cur_6g_client_type(pdev,
					  reg_cur_6g_client_mobility_type);
}

bool wlan_reg_is_6g_psd_power(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_6g_psd_power(pdev);
}

QDF_STATUS wlan_reg_get_6g_chan_ap_power(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t chan_freq, bool *is_psd,
					 uint16_t *tx_power,
					 uint16_t *eirp_psd_power)
{
	return reg_get_6g_chan_ap_power(pdev, chan_freq, is_psd,
					tx_power, eirp_psd_power);
}

qdf_export_symbol(wlan_reg_get_6g_chan_ap_power);

QDF_STATUS
wlan_reg_get_client_power_for_connecting_ap(struct wlan_objmgr_pdev *pdev,
					    enum reg_6g_ap_type ap_type,
					    qdf_freq_t chan_freq,
					    bool is_psd, uint16_t *tx_power,
					    uint16_t *eirp_psd_power)
{
	return reg_get_client_power_for_connecting_ap(pdev, ap_type, chan_freq,
						      is_psd, tx_power,
						      eirp_psd_power);
}

QDF_STATUS
wlan_reg_get_client_power_for_6ghz_ap(struct wlan_objmgr_pdev *pdev,
				      enum reg_6g_client_type client_type,
				      qdf_freq_t chan_freq,
				      bool *is_psd, uint16_t *tx_power,
				      uint16_t *eirp_psd_power)
{
	return reg_get_client_power_for_6ghz_ap(pdev, client_type, chan_freq,
						is_psd, tx_power,
						eirp_psd_power);
}

enum reg_6g_ap_type
wlan_reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev)
{
	return reg_decide_6g_ap_pwr_type(pdev);
}

QDF_STATUS
wlan_reg_set_ap_pwr_and_update_chan_list(struct wlan_objmgr_pdev *pdev,
					 enum reg_6g_ap_type ap_pwr_type)
{
	return reg_set_ap_pwr_and_update_chan_list(pdev, ap_pwr_type);
}

qdf_export_symbol(wlan_reg_set_ap_pwr_and_update_chan_list);

#endif /* CONFIG_BAND_6GHZ */

bool wlan_reg_is_ext_tpc_supported(struct wlan_objmgr_psoc *psoc)
{
	return reg_is_ext_tpc_supported(psoc);
}

#ifdef CONFIG_AFC_SUPPORT
bool wlan_reg_is_afc_power_event_received(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_afc_power_event_received(pdev);
}

qdf_export_symbol(wlan_reg_is_afc_power_event_received);

QDF_STATUS wlan_reg_get_afc_req_id(struct wlan_objmgr_pdev *pdev,
				   uint64_t *req_id)
{
	return reg_get_afc_req_id(pdev, req_id);
}

qdf_export_symbol(wlan_reg_get_afc_req_id);

bool wlan_reg_is_afc_expiry_event_received(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_afc_expiry_event_received(pdev);
}

qdf_export_symbol(wlan_reg_is_afc_expiry_event_received);

bool wlan_reg_is_noaction_on_afc_pwr_evt(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_noaction_on_afc_pwr_evt(pdev);
}
#endif

QDF_STATUS wlan_reg_is_chwidth_supported(struct wlan_objmgr_pdev *pdev,
					 enum phy_ch_width ch_width,
					 bool *is_supported)
{
	return reg_is_chwidth_supported(pdev, ch_width, is_supported);
}

qdf_export_symbol(wlan_reg_is_chwidth_supported);

#ifdef CONFIG_REG_CLIENT
QDF_STATUS
wlan_reg_recompute_current_chan_list(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_err("Recomputing the current channel list");
	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	return reg_send_scheduler_msg_nb(psoc, pdev);
}

QDF_STATUS
wlan_reg_modify_indoor_concurrency(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id, uint32_t freq,
				   enum phy_ch_width width, bool add)
{
	if (add)
		return reg_add_indoor_concurrency(pdev, vdev_id, freq, width);
	else
		return reg_remove_indoor_concurrency(pdev, vdev_id, freq);
}
#endif
