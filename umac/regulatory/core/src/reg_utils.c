/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_cm_bss_score_param.h"

#define DEFAULT_WORLD_REGDMN 0x60

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (pdev_priv_obj)

#ifdef CONFIG_CHAN_FREQ_API
bool reg_chan_has_dfs_attribute_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	ch_idx = reg_get_chan_enum_for_freq(freq);

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
#endif /* CONFIG_CHAN_FREQ_API */

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

	reg_info("set default_country: %s", country);

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

bool reg_is_etsi_alpha2(uint8_t *alpha2)
{
	if ((alpha2[0] == 'G') && (alpha2[1] == 'B'))
		return true;

	return false;
}

static QDF_STATUS reg_set_non_offload_country(struct wlan_objmgr_pdev *pdev,
					      struct set_country *cc)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct cc_regdmn_s rd;
	uint8_t pdev_id;
	uint8_t phy_id;

	if (!pdev) {
		reg_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->get_phy_id_from_pdev_id)
		tx_ops->get_phy_id_from_pdev_id(psoc, pdev_id, &phy_id);
	else
		phy_id = pdev_id;

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (reg_is_world_alpha2(cc->country)) {
		pdev_priv_obj = reg_get_pdev_obj(pdev);
		if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
			reg_err("reg component pdev priv is NULL");
			psoc_reg->world_country_pending[phy_id] = false;
			return QDF_STATUS_E_INVAL;
		}
		if (reg_is_world_ctry_code(pdev_priv_obj->def_region_domain))
			rd.cc.regdmn.reg_2g_5g_pair_id =
				pdev_priv_obj->def_region_domain;
		else
			rd.cc.regdmn.reg_2g_5g_pair_id = DEFAULT_WORLD_REGDMN;
		rd.flags = REGDMN_IS_SET;
	} else {
		qdf_mem_copy(rd.cc.alpha, cc->country, REG_ALPHA2_LEN + 1);
		rd.flags = ALPHA_IS_SET;
	}

	reg_program_chan_list(pdev, &rd);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev,
			   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct set_country cc;
	struct wlan_objmgr_psoc *psoc;
	uint8_t pdev_id;
	uint8_t phy_id;

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

	tx_ops = reg_get_psoc_tx_ops(psoc);
	if (tx_ops->get_phy_id_from_pdev_id)
		tx_ops->get_phy_id_from_pdev_id(psoc, pdev_id, &phy_id);
	else
		phy_id = pdev_id;

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_reg->cur_country, country, REG_ALPHA2_LEN)) {
		if (psoc_reg->cc_src == SOURCE_USERSPACE ||
		    psoc_reg->cc_src == SOURCE_CORE) {
			reg_debug("country is not different");
			return QDF_STATUS_E_INVAL;
		}
	}

	reg_debug("programming new country: %s to firmware", country);

	qdf_mem_copy(cc.country, country, REG_ALPHA2_LEN + 1);
	cc.pdev_id = pdev_id;

	if (!psoc_reg->offload_enabled && !reg_is_world_alpha2(country)) {
		QDF_STATUS status;

		status = reg_is_country_code_valid(country);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			reg_err("Unable to set country code: %s\n", country);
			reg_err("Restoring to world domain");
			qdf_mem_copy(cc.country, REG_WORLD_ALPHA2,
				     REG_ALPHA2_LEN + 1);
		}
	}


	if (reg_is_world_alpha2(cc.country))
		psoc_reg->world_country_pending[phy_id] = true;
	else
		psoc_reg->new_user_ctry_pending[phy_id] = true;

	if (psoc_reg->offload_enabled) {
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_code) {
			tx_ops->set_country_code(psoc, &cc);
		} else {
			reg_err("country set fw handler not present");
			psoc_reg->new_user_ctry_pending[phy_id] = false;
			return QDF_STATUS_E_FAULT;
		}
	} else {
		return reg_set_non_offload_country(pdev, &cc);
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
		reg_err("Country code is NULL");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_REG_CLIENT
QDF_STATUS
reg_get_6g_power_type_for_ctry(struct wlan_objmgr_psoc *psoc,
			       uint8_t *ap_ctry, uint8_t *sta_ctry,
			       enum reg_6g_ap_type *pwr_type_6g,
			       bool *ctry_code_match,
			       enum reg_6g_ap_type ap_pwr_type)
{
	*pwr_type_6g = REG_INDOOR_AP;

	if (qdf_mem_cmp(ap_ctry, sta_ctry, REG_ALPHA2_LEN)) {
		reg_debug("Country IE:%c%c, STA country:%c%c", ap_ctry[0],
			  ap_ctry[1], sta_ctry[0], sta_ctry[1]);
		*ctry_code_match = false;

		/**
		 * Do not return if Wi-Fi safe mode or RF test mode is
		 * enabled, rather STA should operate in LPI mode.
		 * wlan_cm_get_check_6ghz_security API returns true if
		 * neither Safe mode nor RF test mode are enabled.
		 */
		if (wlan_reg_is_us(sta_ctry) &&
		    wlan_cm_get_check_6ghz_security(psoc)) {
			reg_err("US VLP not in place yet, connection not allowed");
			return QDF_STATUS_E_NOSUPPORT;
		}

		if (wlan_reg_is_etsi(sta_ctry) &&
		    ap_pwr_type != REG_MAX_AP_TYPE) {
			if (!(wlan_reg_is_us(ap_ctry) &&
			      ap_pwr_type == REG_INDOOR_AP)) {
				reg_debug("STA ctry:%c%c, doesn't match with AP ctry, switch to VLP",
					  sta_ctry[0], sta_ctry[1]);
				*pwr_type_6g = REG_VERY_LOW_POWER_AP;
			}
		}

		if (wlan_reg_is_us(ap_ctry) && ap_pwr_type == REG_INDOOR_AP) {
			reg_debug("AP ctry:%c%c, AP power type:%d, allow STA IN LPI",
				  ap_ctry[0], ap_ctry[1], ap_pwr_type);
			*pwr_type_6g = REG_INDOOR_AP;
		}
	} else {
		*ctry_code_match = true;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef FEATURE_WLAN_CH_AVOID_EXT
static inline
void reg_get_coex_unsafe_chan_nb_user_prefer(
		struct wlan_regulatory_psoc_priv_obj
		*psoc_priv_obj,
		 struct reg_config_vars config_vars)
{
	psoc_priv_obj->coex_unsafe_chan_nb_user_prefer =
		config_vars.coex_unsafe_chan_nb_user_prefer;
}

static inline
void reg_get_coex_unsafe_chan_reg_disable(
		struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj,
		struct reg_config_vars config_vars)
{
	psoc_priv_obj->coex_unsafe_chan_reg_disable =
		config_vars.coex_unsafe_chan_reg_disable;
}
#else
static inline
void reg_get_coex_unsafe_chan_nb_user_prefer(
		struct wlan_regulatory_psoc_priv_obj
		*psoc_priv_obj,
		struct reg_config_vars config_vars)
{
}

static inline
void reg_get_coex_unsafe_chan_reg_disable(
		struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj,
		struct reg_config_vars config_vars)
{
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
bool reg_is_passive_or_disable_for_freq(struct wlan_objmgr_pdev *pdev,
					qdf_freq_t freq)
{
	enum channel_state chan_state;

	chan_state = reg_get_channel_state_for_freq(pdev, freq);

	return (chan_state == CHANNEL_STATE_DFS) ||
		(chan_state == CHANNEL_STATE_DISABLE);
}
#endif /* CONFIG_CHAN_FREQ_API */

#ifdef WLAN_FEATURE_DSRC
#ifdef CONFIG_CHAN_FREQ_API
bool reg_is_dsrc_freq(qdf_freq_t freq)
{
	if (!REG_IS_5GHZ_FREQ(freq))
		return false;

	if (!(freq >= REG_DSRC_START_FREQ && freq <= REG_DSRC_END_FREQ))
		return false;

	return true;
}
#endif  /*CONFIG_CHAN_FREQ_API*/
#else
bool reg_is_etsi13_regdmn(struct wlan_objmgr_pdev *pdev)
{
	struct cur_regdmn_info cur_reg_dmn;
	QDF_STATUS status;

	status = reg_get_curr_regdomain(pdev, &cur_reg_dmn);
	if (status != QDF_STATUS_SUCCESS) {
		reg_debug_rl("Failed to get reg domain");
		return false;
	}

	return reg_etsi13_regdmn(cur_reg_dmn.dmn_id_5g);
}

#ifdef CONFIG_CHAN_FREQ_API
bool reg_is_etsi13_srd_chan_for_freq(struct wlan_objmgr_pdev *pdev,
				     uint16_t freq)
{
	if (!REG_IS_5GHZ_FREQ(freq))
		return false;

	if (!(freq >= REG_ETSI13_SRD_START_FREQ &&
	      freq <= REG_ETSI13_SRD_END_FREQ))
		return false;

	return reg_is_etsi13_regdmn(pdev);
}
#endif /* CONFIG_CHAN_FREQ_API */

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

QDF_STATUS reg_set_band(struct wlan_objmgr_pdev *pdev, uint32_t band_bitmap)
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

	if (pdev_priv_obj->band_capability == band_bitmap) {
		reg_info("same band %d", band_bitmap);
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

	reg_info("set band bitmap: %d", band_bitmap);
	pdev_priv_obj->band_capability = band_bitmap;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_get_band(struct wlan_objmgr_pdev *pdev,
			uint32_t *band_bitmap)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_debug("get band bitmap: %d", pdev_priv_obj->band_capability);
	*band_bitmap = pdev_priv_obj->band_capability;

	return QDF_STATUS_SUCCESS;
}

#ifdef DISABLE_CHANNEL_LIST
QDF_STATUS reg_restore_cached_channels(struct wlan_objmgr_pdev *pdev)
{
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

	pdev_priv_obj->disable_cached_channels = false;
	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_sb(psoc, pdev);
	return status;
}

QDF_STATUS reg_disable_cached_channels(struct wlan_objmgr_pdev *pdev)
{
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

	pdev_priv_obj->disable_cached_channels = true;
	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_sb(psoc, pdev);
	return status;
}

#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS reg_cache_channel_freq_state(struct wlan_objmgr_pdev *pdev,
					uint32_t *channel_list,
					uint32_t num_channels)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	uint16_t i, j;

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
						cur_chan_list[j].center_freq) {
				pdev_priv_obj->
					cache_disable_chan_list[i].center_freq =
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
#endif /* CONFIG_CHAN_FREQ_API */
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
		reg_info("same fcc_constraint %d", fcc_constraint);
		return QDF_STATUS_SUCCESS;
	}

	reg_info("set fcc_constraint: %d", fcc_constraint);
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

#ifdef CONFIG_BAND_6GHZ
/**
 * reg_is_afc_available() - check if the automated frequency control system is
 * available, function will need to be updated once AFC is implemented
 * @pdev: Pointer to pdev structure
 *
 * Return: false since the AFC system is not yet available
 */
static bool reg_is_afc_available(struct wlan_objmgr_pdev *pdev)
{
	return false;
}

enum reg_6g_ap_type reg_decide_6g_ap_pwr_type(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	enum reg_6g_ap_type ap_pwr_type = REG_INDOOR_AP;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return REG_VERY_LOW_POWER_AP;
	}

	if (reg_is_afc_available(pdev)) {
		ap_pwr_type = REG_STANDARD_POWER_AP;
	} else if (pdev_priv_obj->indoor_chan_enabled) {
		if (pdev_priv_obj->reg_rules.num_of_6g_ap_reg_rules[REG_INDOOR_AP])
			ap_pwr_type = REG_INDOOR_AP;
		else
			ap_pwr_type = REG_VERY_LOW_POWER_AP;
	} else if (pdev_priv_obj->reg_rules.num_of_6g_ap_reg_rules[REG_VERY_LOW_POWER_AP]) {
		ap_pwr_type = REG_VERY_LOW_POWER_AP;
	}
	reg_debug("indoor_chan_enabled %d ap_pwr_type %d",
		  pdev_priv_obj->indoor_chan_enabled, ap_pwr_type);

	reg_set_ap_pwr_and_update_chan_list(pdev, ap_pwr_type);

	return ap_pwr_type;
}
#endif /* CONFIG_BAND_6GHZ */

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
	psoc_priv_obj->enable_5dot9_ghz_chan_in_master_mode =
		config_vars.enable_5dot9_ghz_chan_in_master_mode;
	psoc_priv_obj->retain_nol_across_regdmn_update =
		config_vars.retain_nol_across_regdmn_update;
	reg_get_coex_unsafe_chan_nb_user_prefer(psoc_priv_obj, config_vars);
	reg_get_coex_unsafe_chan_reg_disable(psoc_priv_obj, config_vars);

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

void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
			       struct regulatory_channel *reg_channels,
			       uint8_t *alpha2,
			       enum dfs_reg dfs_region)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;
	uint32_t count;
	enum direction dir;
	uint32_t phy_cnt;

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

	for (phy_cnt = 0; phy_cnt < PSOC_MAX_PHY_REG_CAP; phy_cnt++) {
		qdf_mem_copy(psoc_priv_obj->mas_chan_params[phy_cnt].
			     mas_chan_list, reg_channels,
			     NUM_CHANNELS * sizeof(struct regulatory_channel));

		psoc_priv_obj->mas_chan_params[phy_cnt].dfs_region =
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

QDF_STATUS reg_set_curr_country(struct wlan_regulatory_psoc_priv_obj *soc_reg,
				struct cur_regulatory_info *regulat_info,
				struct wlan_lmac_if_reg_tx_ops *tx_ops)
{
	struct wlan_objmgr_psoc *psoc = regulat_info->psoc;
	struct wlan_objmgr_pdev *pdev;
	uint8_t pdev_id;
	uint8_t phy_id;
	uint8_t phy_num;
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

	/*
	 * Need firmware to send channel list event
	 * for all phys. Therefore set pdev_id to 0xFF
	 */
	pdev_id = 0xFF;
	for (phy_num = 0; phy_num < regulat_info->num_phy; phy_num++) {
		if (soc_reg->cc_src == SOURCE_USERSPACE)
			soc_reg->new_user_ctry_pending[phy_num] = true;
		else if (soc_reg->cc_src == SOURCE_11D)
			soc_reg->new_11d_ctry_pending[phy_num] = true;
		else
			soc_reg->world_country_pending[phy_num] = true;
	}

	qdf_mem_zero(&country_code, sizeof(country_code));
	qdf_mem_copy(country_code.country, soc_reg->cur_country,
		     sizeof(soc_reg->cur_country));
	country_code.pdev_id = pdev_id;

	if (soc_reg->offload_enabled) {
		if (!tx_ops || !tx_ops->set_country_code) {
			reg_err("No regulatory tx_ops");
			status = QDF_STATUS_E_FAULT;
			goto error;
		}
		status = tx_ops->set_country_code(psoc, &country_code);
		if (QDF_IS_STATUS_ERROR(status)) {
			reg_err("Failed to send country code to fw");
			goto error;
		}
	} else {
		phy_id = regulat_info->phy_id;
		if (tx_ops->get_pdev_id_from_phy_id)
			tx_ops->get_pdev_id_from_phy_id(psoc, phy_id, &pdev_id);
		else
			pdev_id = phy_id;

		pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id,
						  WLAN_REGULATORY_NB_ID);
		status = reg_set_non_offload_country(pdev, &country_code);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		if (QDF_IS_STATUS_ERROR(status)) {
			reg_err("Failed to set country code");
			goto error;
		}
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
	uint8_t phy_num;

	if (soc_reg->cc_src == SOURCE_UNKNOWN)
		return false;

	phy_num = regulat_info->phy_id;
	if (soc_reg->new_user_ctry_pending[phy_num] ||
	    soc_reg->new_init_ctry_pending[phy_num] ||
	    soc_reg->new_11d_ctry_pending[phy_num] ||
	    soc_reg->world_country_pending[phy_num])
		return false;

	return true;
}
