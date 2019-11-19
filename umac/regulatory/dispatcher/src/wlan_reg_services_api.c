/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_channel_list_with_power() - Provide the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
						struct channel_power *ch_list,
						uint8_t *num_chan)
{
	/*
	 * Update the channel list with channel information with power.
	 */
	return reg_get_channel_list_with_power(pdev, ch_list, num_chan);
}
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					      uint8_t ch)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_channel_state(pdev, ch);
}

bool
wlan_reg_chan_has_dfs_attribute(struct wlan_objmgr_pdev *pdev, uint8_t ch)
{
	return reg_chan_has_dfs_attribute(pdev, ch);
}

/**
 * wlan_reg_get_5g_bonded_channel_state() - Get 5G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_5g_bonded_channel_state(
	struct wlan_objmgr_pdev *pdev, uint8_t ch,
	enum phy_ch_width bw)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_5g_bonded_channel_state(pdev, ch, bw);
}

/**
 * wlan_reg_get_2g_bonded_channel_state() - Get 2G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		uint8_t sec_ch, enum phy_ch_width bw)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_2g_bonded_channel_state(pdev, ch, sec_ch, bw);
}

/**
 * wlan_reg_set_channel_params() - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void wlan_reg_set_channel_params(struct wlan_objmgr_pdev *pdev, uint8_t ch,
				 uint8_t sec_ch_2g,
				 struct ch_params *ch_params)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	 */
	reg_set_channel_params(pdev, ch, sec_ch_2g, ch_params);
}
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
uint32_t wlan_reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
					uint8_t chan_num)
{
	return reg_get_channel_reg_power(pdev, chan_num);
}

/**
 * wlan_reg_get_channel_freq() - get regulatory power for channel
 * @chan_num: channel number
 *
 * Return: int
 */
qdf_freq_t wlan_reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
				     uint8_t chan_num)
{
	return reg_get_channel_freq(pdev, chan_num);
}
#endif /* CONFIG_CHAN_NUM_API */

QDF_STATUS wlan_reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
		struct regulatory_channel *chan_list)
{
	return reg_get_current_chan_list(pdev, chan_list);
}

qdf_export_symbol(wlan_reg_get_current_chan_list);

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

#ifdef CONFIG_CHAN_NUM_API
/**
 * wlan_reg_get_bonded_channel_state() - Get 2G bonded channel state
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_bonded_channel_state(
	struct wlan_objmgr_pdev *pdev, uint8_t ch,
	enum phy_ch_width bw, uint8_t sec_ch)
{
	if (WLAN_REG_IS_24GHZ_CH(ch))
		return reg_get_2g_bonded_channel_state(pdev, ch,
						       sec_ch, bw);
	else
		return reg_get_5g_bonded_channel_state(pdev, ch,
						       bw);
}
#endif /* CONFIG_CHAN_NUM_API */

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

uint8_t wlan_reg_get_opclass_from_freq_width(uint8_t *country,
					     qdf_freq_t freq,
					     uint8_t ch_width,
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

QDF_STATUS regulatory_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->register_master_handler)
		tx_ops->register_master_handler(psoc, NULL);
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
	if (tx_ops->unregister_ch_avoid_event_handler)
		tx_ops->unregister_ch_avoid_event_handler(psoc, NULL);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS regulatory_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *parent_psoc;

	parent_psoc = wlan_pdev_get_psoc(pdev);

	reg_send_scheduler_msg_nb(parent_psoc, pdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS regulatory_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	psoc = wlan_pdev_get_psoc(pdev);
	soc_reg = reg_get_psoc_obj(psoc);
	if (!soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	reg_reset_ctry_pending_hints(soc_reg);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_CHAN_NUM_API
void wlan_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev, uint8_t *ch_list,
		uint8_t num_ch, bool nol_ch)
{
	reg_update_nol_ch(pdev, ch_list, num_ch, nol_ch);
}

void wlan_reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
				    uint8_t *ch_list, uint8_t num_ch,
				    bool nol_history_ch)
{
	reg_update_nol_history_ch(pdev, ch_list, num_ch, nol_history_ch);
}

bool wlan_reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev,
			uint8_t chan)
{
	return reg_is_dfs_ch(pdev, chan);
}

bool wlan_reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint8_t chan)
{
	return reg_is_passive_or_disable_ch(pdev, chan);
}

bool wlan_reg_is_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint8_t chan)
{
	return reg_is_disable_ch(pdev, chan);
}
#endif /* CONFIG_CHAN_NUM_API */

uint8_t wlan_reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			      qdf_freq_t freq)
{
	return reg_freq_to_chan(pdev, freq);
}

qdf_export_symbol(wlan_reg_freq_to_chan);

#ifdef CONFIG_CHAN_NUM_API
qdf_freq_t wlan_reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan_num)
{
	return reg_chan_to_freq(pdev, chan_num);
}

qdf_export_symbol(wlan_reg_chan_to_freq);

qdf_freq_t wlan_reg_legacy_chan_to_freq(struct wlan_objmgr_pdev *pdev,
					uint8_t chan_num)
{
	return reg_legacy_chan_to_freq(pdev, chan_num);
}

bool wlan_reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev,
			    uint8_t chan_num)
{
	return reg_chan_is_49ghz(pdev, chan_num);
}
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
bool wlan_reg_is_dsrc_chan(struct wlan_objmgr_pdev *pdev, uint8_t chan_num)
{
	return reg_is_dsrc_chan(pdev, chan_num);
}

bool wlan_reg_is_etsi13_srd_chan(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan_num)
{
	return reg_is_etsi13_srd_chan(pdev, chan_num);
}
#endif /* CONFIG_CHAN_NUM_API */

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
		uint32_t *chip_mode)
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

	*low_2g = pdev_priv_obj->range_2g_low;
	*high_2g = pdev_priv_obj->range_2g_high;
	*low_5g = pdev_priv_obj->range_5g_low;
	*high_5g = pdev_priv_obj->range_5g_high;

	return QDF_STATUS_SUCCESS;
}

struct wlan_lmac_if_reg_tx_ops *
wlan_reg_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return reg_get_psoc_tx_ops(psoc);
}

QDF_STATUS wlan_reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
		struct cur_regdmn_info *cur_regdmn)
{
	return reg_get_curr_regdomain(pdev, cur_regdmn);
}

#ifdef CONFIG_CHAN_NUM_API
uint8_t wlan_reg_min_24ghz_ch_num(void)
{
	return reg_min_24ghz_ch_num();
}

uint8_t wlan_reg_max_24ghz_ch_num(void)
{
	return reg_max_24ghz_ch_num();
}

uint8_t wlan_reg_min_5ghz_ch_num(void)
{
	return reg_min_5ghz_ch_num();
}

uint8_t wlan_reg_max_5ghz_ch_num(void)
{
	return reg_max_5ghz_ch_num();
}
#endif /* CONFIG_CHAN_NUM_API */

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

#ifdef CONFIG_CHAN_NUM_API
bool wlan_reg_is_24ghz_ch(uint8_t chan)
{
	return reg_is_24ghz_ch(chan);
}

bool wlan_reg_is_5ghz_ch(uint8_t chan)
{
	return reg_is_5ghz_ch(chan);
}
#endif /* CONFIG_CHAN_NUM_API */

bool wlan_reg_is_24ghz_ch_freq(qdf_freq_t freq)
{
	return reg_is_24ghz_ch_freq(freq);
}

bool wlan_reg_is_5ghz_ch_freq(qdf_freq_t freq)
{
	return reg_is_5ghz_ch_freq(freq);
}

#ifdef CONFIG_BAND_6GHZ
bool wlan_reg_is_6ghz_chan_freq(uint16_t freq)
{
	return reg_is_6ghz_chan_freq(freq);
}

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

qdf_freq_t wlan_reg_chan_band_to_freq(struct wlan_objmgr_pdev *pdev,
				      uint8_t chan, uint8_t band_mask)
{
	return reg_chan_band_to_freq(pdev, chan, band_mask);
}

bool wlan_reg_is_49ghz_freq(qdf_freq_t freq)
{
	return reg_is_49ghz_freq(freq);
}

uint8_t wlan_reg_ch_num(uint32_t ch_enum)
{
	return reg_ch_num(ch_enum);
}

qdf_freq_t wlan_reg_ch_to_freq(uint32_t ch_enum)
{
	return reg_ch_to_freq(ch_enum);
}

#ifdef CONFIG_CHAN_NUM_API
bool wlan_reg_is_same_band_channels(uint8_t chan_num1, uint8_t chan_num2)
{
	return reg_is_same_band_channels(chan_num1, chan_num2);
}

bool wlan_reg_is_channel_valid_5g_sbs(uint8_t curchan, uint8_t newchan)
{
	return reg_is_channel_valid_5g_sbs(curchan, newchan);
}

enum band_info wlan_reg_chan_to_band(uint8_t chan_num)
{
	return reg_chan_to_band(chan_num);
}

qdf_export_symbol(wlan_reg_chan_to_band);

/**
 * wlan_reg_get_chan_enum() - Get channel enum for given channel number
 * @chan_num: Channel number
 *
 * Return: Channel enum
 */
enum channel_enum wlan_reg_get_chan_enum(uint8_t chan_num)
{
	return reg_get_chan_enum(chan_num);
}
#endif /* CONFIG_CHAN_NUM_API */

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

enum channel_state
wlan_reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      enum phy_ch_width bw)
{
	return reg_get_5g_bonded_channel_state_for_freq(pdev, freq, bw);
}

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

enum channel_state
wlan_reg_get_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq)
{
	return reg_get_channel_state_for_freq(pdev, freq);
}

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

qdf_export_symbol(wlan_reg_get_5g_bonded_channel_and_state_for_freq);

#endif /* CONFIG CHAN FREQ API */

uint16_t wlan_reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
				     uint8_t op_class,
				     bool global_tbl_lookup)
{
	return reg_get_op_class_width(pdev, op_class,
				      global_tbl_lookup);
}

bool wlan_reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			       uint8_t op_class)
{
	return reg_is_6ghz_op_class(pdev, op_class);
}

bool wlan_reg_is_6ghz_supported(struct wlan_objmgr_pdev *pdev)
{
	return reg_is_6ghz_supported(pdev);
}

uint16_t wlan_reg_chan_opclass_to_freq(uint8_t chan,
				       uint8_t op_class,
				       bool global_tbl_lookup)
{
	if (!chan || !op_class)
		return 0;

	return reg_chan_opclass_to_freq(chan, op_class, global_tbl_lookup);
}
