/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
#include <wlan_reg_services_api.h>
#include "../../core/src/reg_services.h"
#include "../../core/src/reg_priv.h"

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

/**
 * wlan_reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state wlan_reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					      uint32_t ch)
{
	/*
	 * Get channel state from regulatory
	 */
	return reg_get_channel_state(pdev, ch);
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

/**
 * wlan_reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void wlan_reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
			     enum dfs_reg *dfs_reg)
{
	/*
	 * Get the current dfs region
	 */
	reg_get_dfs_region(psoc, dfs_reg);
}

uint32_t wlan_reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
					uint32_t chan_num)
{
	return reg_get_channel_reg_power(pdev, chan_num);
}

/**
 * wlan_reg_get_channel_freq() - get regulatory power for channel
 * @chan_num: channel number
 *
 * Return: int
 */
uint32_t wlan_reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num)
{
	return reg_get_channel_freq(pdev, chan_num);
}

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

/**
 * wlan_reg_set_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void wlan_reg_set_dfs_region(struct wlan_objmgr_psoc *psoc,
			     enum dfs_reg dfs_reg)
{
	reg_set_dfs_region(psoc, dfs_reg);
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
		reg_err("failed to register reg psoc obj detroy handler");
		wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_REGULATORY,
				wlan_regulatory_psoc_obj_created_notification,
				NULL);
		return status;
	}

	status = wlan_objmgr_register_pdev_create_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_pdev_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg pdev obj create handler");
		return status;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_pdev_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to register reg pdev obj destroy handler");
		wlan_objmgr_unregister_pdev_create_handler(
				WLAN_UMAC_COMP_REGULATORY,
				wlan_regulatory_pdev_obj_created_notification,
				NULL);
		return status;
	}

	reg_debug("regulatory handlers registered with obj mgr");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_regulatory_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_created_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS)
		reg_err("deregister fail for psoc create notif:%d",
				status);
	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_destroyed_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("deregister fail for psoc delete notif:%d",
				status);
		return status;
	}

	status = wlan_objmgr_unregister_pdev_create_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg pdev obj create handler");
		return status;
	}

	status = wlan_objmgr_unregister_pdev_destroy_handler(
		WLAN_UMAC_COMP_REGULATORY,
		wlan_regulatory_pdev_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("failed to unregister reg pdev obj destroy handler");
		return status;
	}

	reg_debug("deregistered callbacks with obj mgr successfully");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS regulatory_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->register_master_handler)
		tx_ops->register_master_handler(psoc, NULL);

	return QDF_STATUS_SUCCESS;
};

QDF_STATUS regulatory_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->unregister_master_handler)
		tx_ops->unregister_master_handler(psoc, NULL);

	return QDF_STATUS_SUCCESS;
};

void wlan_reg_update_nol_ch(struct wlan_objmgr_pdev *pdev, uint8_t *ch_list,
		uint8_t num_ch, bool nol_ch)
{
	reg_update_nol_ch(pdev, ch_list, num_ch, nol_ch);
}

bool wlan_reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev,
			uint32_t chan)
{
	return reg_is_dfs_ch(pdev, chan);
}

bool wlan_reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint32_t chan)
{
	return reg_is_passive_or_disable_ch(pdev, chan);
}

bool wlan_reg_is_disable_ch(struct wlan_objmgr_pdev *pdev,
				       uint32_t chan)
{
	return reg_is_disable_ch(pdev, chan);
}

uint32_t wlan_reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			       uint32_t freq)
{
	return reg_freq_to_chan(pdev, freq);
}

uint32_t wlan_reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
			       uint32_t chan_num)
{
	return reg_chan_to_freq(pdev, chan_num);
}

QDF_STATUS wlan_reg_set_country(struct wlan_objmgr_pdev *pdev,
				       uint8_t *country)
{
	return reg_set_country(pdev, country);
}

void wlan_reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					    reg_chan_change_callback cbk,
					    void *arg)
{
	reg_register_chan_change_callback(psoc, cbk, arg);

}

void wlan_reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					      reg_chan_change_callback cbk)
{
	reg_unregister_chan_change_callback(psoc, cbk);

}

QDF_STATUS wlan_reg_get_chip_mode(struct wlan_objmgr_pdev *pdev,
		uint32_t *chip_mode)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	*chip_mode = pdev_priv_obj->wireless_modes;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_reg_get_freq_range(struct wlan_objmgr_pdev *pdev,
		uint32_t *low_2g,
		uint32_t *high_2g,
		uint32_t *low_5g,
		uint32_t *high_5g)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	*low_2g = pdev_priv_obj->range_2g_low;
	*high_2g = pdev_priv_obj->range_2g_high;
	*low_5g = pdev_priv_obj->range_5g_low;
	*high_5g = pdev_priv_obj->range_5g_high;

	return QDF_STATUS_SUCCESS;
}
