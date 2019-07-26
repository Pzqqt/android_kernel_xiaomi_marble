/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_utils.c
 * This file defines the APIs to set and get the regulatory variables.
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"
#include "reg_callbacks.h"
#include "reg_db.h"
#include "reg_db_parser.h"
#include "reg_host_11d.h"
#include <scheduler_api.h>
#include <wlan_reg_services_api.h>
#include <qdf_platform.h>
#include "reg_services_common.h"
#include "reg_build_chan_list.h"

#define DEFAULT_WORLD_REGDMN 0x60

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (pdev_priv_obj)

bool reg_chan_has_dfs_attribute(struct wlan_objmgr_pdev *pdev, uint32_t ch)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	ch_idx = reg_get_chan_enum(ch);

	if (ch_idx == INVALID_CHANNEL)
		return false;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return false;
	}

	if (pdev_priv_obj->cur_chan_list[ch_idx].chan_flags &
	    REGULATORY_CHAN_RADAR)
		return true;

	return false;
}

bool reg_is_world_ctry_code(uint16_t ctry_code)
{
	if ((ctry_code & 0xFFF0) == DEFAULT_WORLD_REGDMN)
		return true;

	return false;
}

QDF_STATUS reg_read_current_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country_code) {
		reg_err("country_code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(country_code, psoc_reg->cur_country, REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_set_default_country() - Read the default country for the regdomain
 * @country: country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country) {
		reg_err("country is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting default_country: %s", country);

	qdf_mem_copy(psoc_reg->def_country, country, REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

bool reg_is_world_alpha2(uint8_t *alpha2)
{
	if ((alpha2[0] == '0') && (alpha2[1] == '0'))
		return true;

	return false;
}

bool reg_is_us_alpha2(uint8_t *alpha2)
{
	if ((alpha2[0] == 'U') && (alpha2[1] == 'S'))
		return true;

	return false;
}

QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev,
			   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct set_country country_code;
	struct wlan_objmgr_psoc *psoc;
	struct cc_regdmn_s rd;
	uint8_t pdev_id;

	if (!pdev) {
		reg_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!country) {
		reg_err("country code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	psoc = wlan_pdev_get_psoc(pdev);

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_reg->cur_country, country, REG_ALPHA2_LEN)) {
		reg_err("country is not different");
		return QDF_STATUS_SUCCESS;
	}

	reg_debug("programming new country:%s to firmware", country);

	qdf_mem_copy(country_code.country, country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = pdev_id;

	if (reg_is_world_alpha2(country))
		psoc_reg->world_country_pending[pdev_id] = true;
	else
		psoc_reg->new_user_ctry_pending[pdev_id] = true;

	if (psoc_reg->offload_enabled) {
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_code) {
			tx_ops->set_country_code(psoc, &country_code);
		} else {
			reg_err("country set fw handler not present");
			psoc_reg->new_user_ctry_pending[pdev_id] = false;
			return QDF_STATUS_E_FAULT;
		}
	} else {
		if (reg_is_world_alpha2(country)) {
			pdev_priv_obj = reg_get_pdev_obj(pdev);
			if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
				reg_err("reg component pdev priv is NULL");
				psoc_reg->new_user_ctry_pending[pdev_id] =
									false;
				return QDF_STATUS_E_INVAL;
			}
			if (reg_is_world_ctry_code(
				    pdev_priv_obj->def_region_domain))
				rd.cc.regdmn_id =
					pdev_priv_obj->def_region_domain;
			else
				rd.cc.regdmn_id = DEFAULT_WORLD_REGDMN;
			rd.flags = REGDMN_IS_SET;
		} else {
			qdf_mem_copy(rd.cc.alpha, country, REG_ALPHA2_LEN + 1);
			rd.flags = ALPHA_IS_SET;
		}

		reg_program_chan_list(pdev, &rd);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_reset_country(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("re-setting user country to default");
	qdf_mem_copy(psoc_reg->cur_country,
		     psoc_reg->def_country,
		     REG_ALPHA2_LEN + 1);
	reg_debug("set cur_country %.2s", psoc_reg->cur_country);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
					    const uint8_t *country_alpha2,
					    enum country_src source)
{
	if (!reg_domain_ptr) {
		reg_err("Invalid reg domain pointer");
		return QDF_STATUS_E_FAULT;
	}

	*reg_domain_ptr = 0;

	if (!country_alpha2) {
		reg_err("Country code array is NULL");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

bool reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				  uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return (ch_state == CHANNEL_STATE_DFS) ||
		(ch_state == CHANNEL_STATE_DISABLE);
}

#ifdef WLAN_FEATURE_DSRC
bool reg_is_dsrc_chan(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint32_t freq = 0;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	if (!REG_IS_5GHZ_CH(chan))
		return false;

	freq = reg_chan_to_freq(pdev, chan);

	if (!(freq >= REG_DSRC_START_FREQ && freq <= REG_DSRC_END_FREQ))
		return false;

	return true;
}

#else

bool reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev)
{
	struct cur_regdmn_info cur_reg_dmn;
	QDF_STATUS status;

	status = reg_get_curr_regdomain(pdev, &cur_reg_dmn);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("Failed to get reg domain");
		return false;
	}

	return reg_etsi13_regdmn(cur_reg_dmn.dmn_id_5g);
}

bool reg_is_etsi13_srd_chan(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint32_t freq = 0;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	if (!REG_IS_5GHZ_CH(chan))
		return false;

	freq = reg_chan_to_freq(pdev, chan);

	if (!(freq >= REG_ETSI13_SRD_START_FREQ &&
	      freq <= REG_ETSI13_SRD_END_FREQ))
		return false;

	return reg_is_etsi13_regdmn(pdev);
}

bool reg_is_etsi13_srd_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	if (!pdev) {
		reg_alert("pdev is NULL");
		return true;
	}
	psoc = wlan_pdev_get_psoc(pdev);

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_alert("psoc reg component is NULL");
		return true;
	}

	return psoc_priv_obj->enable_srd_chan_in_master_mode &&
	       reg_is_etsi13_regdmn(pdev);
}
#endif

QDF_STATUS reg_set_band(struct wlan_objmgr_pdev *pdev,
			enum band_info band)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->band_capability == band) {
		reg_info("band is already set to %d", band);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting band_info: %d", band);
	pdev_priv_obj->band_capability = band;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_get_band(struct wlan_objmgr_pdev *pdev,
			enum band_info *band)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_debug("getting band_info: %d", pdev_priv_obj->band_capability);
	*band = pdev_priv_obj->band_capability;

	return QDF_STATUS_SUCCESS;
}

#ifdef DISABLE_CHANNEL_LIST
QDF_STATUS reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev_priv_obj->disable_cached_channels = false;
	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_sb(psoc, pdev);
	return status;
}

QDF_STATUS reg_cache_channel_state(struct wlan_objmgr_pdev *pdev,
				   uint32_t *channel_list,
				   uint32_t num_channels)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	uint8_t i, j;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}
	if (pdev_priv_obj->num_cache_channels > 0) {
		pdev_priv_obj->num_cache_channels = 0;
		qdf_mem_zero(&pdev_priv_obj->cache_disable_chan_list,
			     sizeof(pdev_priv_obj->cache_disable_chan_list));
	}

	for (i = 0; i < num_channels; i++) {
		for (j = 0; j < NUM_CHANNELS; j++) {
			if (channel_list[i] == pdev_priv_obj->
						cur_chan_list[j].chan_num) {
				pdev_priv_obj->
					cache_disable_chan_list[i].chan_num =
							channel_list[i];
				pdev_priv_obj->
					cache_disable_chan_list[i].state =
					pdev_priv_obj->cur_chan_list[j].state;
				pdev_priv_obj->
					cache_disable_chan_list[i].chan_flags =
					pdev_priv_obj->
						cur_chan_list[j].chan_flags;
			}
		}
	}
	pdev_priv_obj->num_cache_channels = num_channels;

	return QDF_STATUS_SUCCESS;
}

void set_disable_channel_state(
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
	pdev_priv_obj->disable_cached_channels = pdev_priv_obj->sap_state;
}
#endif

#ifdef CONFIG_REG_CLIENT

QDF_STATUS reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
				  bool fcc_constraint)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->set_fcc_channel == fcc_constraint) {
		reg_info("fcc_constraint is already set to %d", fcc_constraint);
		return QDF_STATUS_SUCCESS;
	}

	reg_info("setting set_fcc_channel: %d", fcc_constraint);
	pdev_priv_obj->set_fcc_channel = fcc_constraint;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

bool reg_get_fcc_constraint(struct wlan_objmgr_pdev *pdev, uint32_t freq)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return false;
	}

	if (freq != CHAN_12_CENT_FREQ && freq != CHAN_13_CENT_FREQ)
		return false;

	if (!pdev_priv_obj->set_fcc_channel)
		return false;

	return true;
}

#endif /* CONFIG_REG_CLIENT */

/**
 * reg_change_pdev_for_config() - Update user configuration in pdev private obj.
 * @psoc: Pointer to global psoc structure.
 * @object: Pointer to global pdev structure.
 * @arg: Pointer to argument list.
 */
static void reg_change_pdev_for_config(struct wlan_objmgr_psoc *psoc,
				       void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return;
	}

	pdev_priv_obj->dfs_enabled = psoc_priv_obj->dfs_enabled;
	pdev_priv_obj->indoor_chan_enabled = psoc_priv_obj->indoor_chan_enabled;
	pdev_priv_obj->force_ssc_disable_indoor_channel =
		psoc_priv_obj->force_ssc_disable_indoor_channel;
	pdev_priv_obj->band_capability = psoc_priv_obj->band_capability;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	reg_send_scheduler_msg_sb(psoc, pdev);
}

QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
			       struct reg_config_vars config_vars)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->enable_11d_supp_original =
		config_vars.enable_11d_support;
	psoc_priv_obj->scan_11d_interval = config_vars.scan_11d_interval;
	psoc_priv_obj->user_ctry_priority = config_vars.userspace_ctry_priority;
	psoc_priv_obj->dfs_enabled = config_vars.dfs_enabled;
	psoc_priv_obj->indoor_chan_enabled = config_vars.indoor_chan_enabled;
	psoc_priv_obj->force_ssc_disable_indoor_channel =
		config_vars.force_ssc_disable_indoor_channel;
	psoc_priv_obj->band_capability = config_vars.band_capability;
	psoc_priv_obj->restart_beaconing = config_vars.restart_beaconing;
	psoc_priv_obj->enable_srd_chan_in_master_mode =
		config_vars.enable_srd_chan_in_master_mode;
	psoc_priv_obj->enable_11d_in_world_mode =
		config_vars.enable_11d_in_world_mode;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					      reg_change_pdev_for_config,
					      NULL, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);

	return status;
}

bool reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return ch_state == CHANNEL_STATE_DISABLE;
}

bool reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return false;
	}

	return psoc_priv_obj->offload_enabled;
}

void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
			       struct regulatory_channel *reg_channels,
			       uint8_t *alpha2,
			       enum dfs_reg dfs_region)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;
	uint32_t count;
	enum direction dir;
	uint32_t pdev_cnt;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_mem_copy(psoc_priv_obj->cur_country, alpha2,
		     REG_ALPHA2_LEN);
	reg_debug("set cur_country %.2s", psoc_priv_obj->cur_country);
	for (count = 0; count < NUM_CHANNELS; count++) {
		reg_channels[count].chan_num = channel_map[count].chan_num;
		reg_channels[count].center_freq =
			channel_map[count].center_freq;
		reg_channels[count].nol_chan = false;
	}

	for (pdev_cnt = 0; pdev_cnt < PSOC_MAX_PHY_REG_CAP; pdev_cnt++) {
		qdf_mem_copy(psoc_priv_obj->mas_chan_params[pdev_cnt].
			     mas_chan_list, reg_channels,
			     NUM_CHANNELS * sizeof(struct regulatory_channel));

		psoc_priv_obj->mas_chan_params[pdev_cnt].dfs_region =
			dfs_region;
	}

	dir = SOUTHBOUND;
	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return;
	}
	status = wlan_objmgr_iterate_obj_list(
			psoc, WLAN_PDEV_OP, reg_propagate_mas_chan_list_to_pdev,
			&dir, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
}

enum country_src reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
				    uint8_t *alpha2)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return SOURCE_UNKNOWN;
	}

	qdf_mem_copy(alpha2, psoc_priv_obj->cur_country, REG_ALPHA2_LEN + 1);

	return psoc_priv_obj->cc_src;
}

QDF_STATUS reg_get_regd_rules(struct wlan_objmgr_pdev *pdev,
			      struct reg_rule_info *reg_rules)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	if (!pdev) {
		reg_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!pdev_priv_obj) {
		reg_err("pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&pdev_priv_obj->reg_rules_lock);
	qdf_mem_copy(reg_rules, &pdev_priv_obj->reg_rules,
		     sizeof(struct reg_rule_info));
	qdf_spin_unlock_bh(&pdev_priv_obj->reg_rules_lock);

	return QDF_STATUS_SUCCESS;
}

void reg_reset_ctry_pending_hints(struct wlan_regulatory_psoc_priv_obj
				  *soc_reg)
{
	uint8_t ctr;

	if (!soc_reg->offload_enabled)
		return;

	for (ctr = 0; ctr < PSOC_MAX_PHY_REG_CAP; ctr++) {
		soc_reg->new_user_ctry_pending[ctr] = false;
		soc_reg->new_init_ctry_pending[ctr] = false;
		soc_reg->new_11d_ctry_pending[ctr] = false;
		soc_reg->world_country_pending[ctr] = false;
	}
}

QDF_STATUS reg_set_curr_country(
		struct wlan_regulatory_psoc_priv_obj *soc_reg,
		struct cur_regulatory_info *regulat_info,
		struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	struct wlan_objmgr_psoc *psoc = regulat_info->psoc;
	uint8_t pdev_id;
	struct set_country country_code;
	QDF_STATUS status;

	/*
	 * During SSR/WLAN restart ignore master channel list
	 * for all events and in the last event handling if
	 * current country and default country is different, send the last
	 * configured (soc_reg->cur_country) country.
	 */
	if ((regulat_info->num_phy != regulat_info->phy_id + 1) ||
	    (!qdf_mem_cmp(soc_reg->cur_country, regulat_info->alpha2,
			  REG_ALPHA2_LEN)))
		return QDF_STATUS_SUCCESS;

	pdev_id = soc_reg->def_pdev_id;
	if (soc_reg->cc_src == SOURCE_USERSPACE)
		soc_reg->new_user_ctry_pending[pdev_id] = true;
	else if (soc_reg->cc_src == SOURCE_11D)
		soc_reg->new_11d_ctry_pending[pdev_id] = true;
	else
		soc_reg->world_country_pending[pdev_id] = true;

	qdf_mem_zero(&country_code, sizeof(country_code));
	qdf_mem_copy(country_code.country, soc_reg->cur_country,
		     sizeof(soc_reg->cur_country));
	country_code.pdev_id = pdev_id;

	if (!tx_ops || !tx_ops->set_country_code) {
		reg_err("No regulatory tx_ops for set_country_code");
		status = QDF_STATUS_E_FAULT;
		goto error;
	}

	status = tx_ops->set_country_code(psoc, &country_code);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("Failed to send country code to firmware");
		goto error;
	}

	reg_debug("Target CC: %.2s, Restore to Previous CC: %.2s",
		  regulat_info->alpha2, soc_reg->cur_country);

	return status;

error:
	reg_reset_ctry_pending_hints(soc_reg);

	return status;
}

bool reg_ignore_default_country(struct wlan_regulatory_psoc_priv_obj *soc_reg,
				struct cur_regulatory_info *regulat_info)
{
	uint8_t pdev_id;

	if (!soc_reg->offload_enabled)
		return false;

	if (soc_reg->cc_src == SOURCE_UNKNOWN)
		return false;

	pdev_id = regulat_info->phy_id;

	if (soc_reg->new_user_ctry_pending[pdev_id] ||
	    soc_reg->new_init_ctry_pending[pdev_id] ||
	    soc_reg->new_11d_ctry_pending[pdev_id] ||
	    soc_reg->world_country_pending[pdev_id])
		return false;

	return true;
}
