/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_bus_bandwidth.c
 *
 * Bus Bandwidth Manager implementation
 */

#include <wlan_hdd_includes.h>
#include "qca_vendor.h"
#include "wlan_hdd_bus_bandwidth.h"
#include "wlan_hdd_main.h"

/**
 * bus_bw_table_default - default table which provides bus bandwidth level
 *  corresonding to a given connection mode and throughput level.
 */
static bus_bw_table_type bus_bw_table_default = {
	[QCA_WLAN_802_11_MODE_11B] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				      BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				      BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11G] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5},
	[QCA_WLAN_802_11_MODE_11A] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5,
				      BUS_BW_LEVEL_5, BUS_BW_LEVEL_5},
	[QCA_WLAN_802_11_MODE_11N] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				      BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				      BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AC] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				       BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				       BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AX] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_1,
				       BUS_BW_LEVEL_2, BUS_BW_LEVEL_3,
				       BUS_BW_LEVEL_4, BUS_BW_LEVEL_6},
};

/**
 * bus_bw_table_low_latency - table which provides bus bandwidth level
 *  corresonding to a given connection mode and throughput level in low
 *  latency setting.
 */
static bus_bw_table_type bus_bw_table_low_latency = {
	[QCA_WLAN_802_11_MODE_11B] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11G] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11A] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11N] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				      BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AC] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
	[QCA_WLAN_802_11_MODE_11AX] = {BUS_BW_LEVEL_NONE, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6,
				       BUS_BW_LEVEL_6, BUS_BW_LEVEL_6},
};

/**
 * bbm_convert_to_pld_bus_lvl() - Convert from internal bus vote level to
 *  PLD bus vote level
 * @vote_lvl: internal bus bw vote level
 *
 * Returns: PLD bus vote level
 */
static enum pld_bus_width_type
bbm_convert_to_pld_bus_lvl(enum bus_bw_level vote_lvl)
{
	switch (vote_lvl) {
	case BUS_BW_LEVEL_1:
		return PLD_BUS_WIDTH_IDLE;
	case BUS_BW_LEVEL_2:
		return PLD_BUS_WIDTH_LOW;
	case BUS_BW_LEVEL_3:
		return PLD_BUS_WIDTH_MEDIUM;
	case BUS_BW_LEVEL_4:
		return PLD_BUS_WIDTH_HIGH;
	case BUS_BW_LEVEL_5:
		return PLD_BUS_WIDTH_LOW_LATENCY;
	case BUS_BW_LEVEL_6:
		return PLD_BUS_WIDTH_VERY_HIGH;
	case BUS_BW_LEVEL_NONE:
	default:
		return PLD_BUS_WIDTH_NONE;
	}
}

/**
 * bbm_get_bus_bw_level_vote() - Select bus bw vote level per adapter based
 *  on connection mode and throughput level
 * @adapter: HDD adapter. Caller ensures that adapter is valid.
 * @tput_level: throughput level
 *
 * Returns: Bus bw level
 */
static enum bus_bw_level
bbm_get_bus_bw_level_vote(struct hdd_adapter *adapter,
			  enum tput_level tput_level)
{
	struct hdd_station_ctx *sta_ctx;
	struct hdd_ap_ctx *ap_ctx;
	enum qca_wlan_802_11_mode i;
	enum qca_wlan_802_11_mode dot11_mode;
	enum bus_bw_level vote_lvl = BUS_BW_LEVEL_NONE;
	struct bbm_context *bbm_ctx = adapter->hdd_ctx->bbm_ctx;
	bus_bw_table_type *lkp_table = bbm_ctx->curr_bus_bw_lookup_table;

	switch (adapter->device_mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
		sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (!hdd_cm_is_vdev_associated(adapter))
			break;

		dot11_mode = hdd_convert_cfgdot11mode_to_80211mode(sta_ctx->
							   conn_info.dot11mode);
		if (dot11_mode >= QCA_WLAN_802_11_MODE_INVALID) {
			hdd_err("invalid STA/P2P-CLI dot11 mode");
			break;
		}

		return (*lkp_table)[dot11_mode][tput_level];
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
		if (!ap_ctx->ap_active)
			break;

		for (i = QCA_WLAN_802_11_MODE_11B;
		     i < QCA_WLAN_802_11_MODE_INVALID; i++) {
			if (ap_ctx->client_count[i] &&
			    (*lkp_table)[i][tput_level] > vote_lvl)
				vote_lvl = (*lkp_table)[i][tput_level];
		}

		return vote_lvl;
	case QDF_NDI_MODE:
		if (WLAN_HDD_GET_STATION_CTX_PTR(adapter)->
		    conn_info.conn_state != eConnectionState_NdiConnected)
			break;

		/*
		 * NDI dot11mode is currently hardcoded to 11AC in driver and
		 * since the bus bw levels in table do not differ between 11AC
		 * and 11AX, using max supported mode instead. Dot11mode of the
		 * peers are not saved in driver and legacy modes are not
		 * supported in NAN.
		 */
		return (*lkp_table)[QCA_WLAN_802_11_MODE_11AX][tput_level];
	default:
		break;
	}

	return vote_lvl;
}

/**
 * bbm_apply_tput_policy() - Apply tput BBM policy by considering
 *  throughput level and connection modes across adapters
 * @hdd_ctx: HDD context
 * @tput_level: throughput level
 *
 * Returns: None
 */
static void
bbm_apply_tput_policy(struct hdd_context *hdd_ctx, enum tput_level tput_level)
{
	struct hdd_adapter *adapter;
	struct hdd_adapter *next_adapter;
	enum bus_bw_level next_vote = BUS_BW_LEVEL_NONE;
	enum bus_bw_level tmp_vote;
	struct bbm_context *bbm_ctx = hdd_ctx->bbm_ctx;

	if (tput_level == TPUT_LEVEL_NONE) {
		/*
		 * This is to handle the scenario where bus bw periodic work
		 * is force cancelled
		 */
		if (!hdd_is_any_adapter_connected(hdd_ctx))
			bbm_ctx->per_policy_vote[BBM_TPUT_POLICY] = next_vote;
		return;
	}

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   NET_DEV_HOLD_BUS_BW_MGR) {
		tmp_vote = bbm_get_bus_bw_level_vote(adapter, tput_level);
		if (tmp_vote > next_vote)
			next_vote = tmp_vote;
		hdd_adapter_dev_put_debug(adapter, NET_DEV_HOLD_BUS_BW_MGR);
	}

	bbm_ctx->per_policy_vote[BBM_TPUT_POLICY] = next_vote;
}

/**
 * bbm_apply_driver_mode_policy() - Apply driver mode BBM policy
 * @bbm_ctx: bus bw mgr context
 * @driver_mode: global driver mode
 *
 * Returns: None
 */
static void
bbm_apply_driver_mode_policy(struct bbm_context *bbm_ctx,
			     enum QDF_GLOBAL_MODE driver_mode)
{
	switch (driver_mode) {
	case QDF_GLOBAL_MONITOR_MODE:
	case QDF_GLOBAL_FTM_MODE:
		bbm_ctx->per_policy_vote[BBM_DRIVER_MODE_POLICY] =
							    BUS_BW_LEVEL_6;
		return;
	default:
		bbm_ctx->per_policy_vote[BBM_DRIVER_MODE_POLICY] =
							 BUS_BW_LEVEL_NONE;
		return;
	}
}

/**
 * bbm_apply_non_persistent_policy() - Apply non persistent policy and set
 *  the bus bandwidth
 * @hdd_ctx: HDD context
 * @flag: flag
 *
 * Returns: None
 */
static void
bbm_apply_non_persistent_policy(struct hdd_context *hdd_ctx,
				enum bbm_non_per_flag flag)
{
	switch (flag) {
	case BBM_APPS_RESUME:
		if (hdd_is_any_adapter_connected(hdd_ctx)) {
			hdd_ctx->bbm_ctx->curr_vote_level = BUS_BW_LEVEL_RESUME;
			pld_request_bus_bandwidth(hdd_ctx->parent_dev,
			       bbm_convert_to_pld_bus_lvl(BUS_BW_LEVEL_RESUME));
		} else {
			hdd_ctx->bbm_ctx->curr_vote_level = BUS_BW_LEVEL_NONE;
			pld_request_bus_bandwidth(hdd_ctx->parent_dev,
				 bbm_convert_to_pld_bus_lvl(BUS_BW_LEVEL_NONE));
		}
		return;
	case BBM_APPS_SUSPEND:
		hdd_ctx->bbm_ctx->curr_vote_level = BUS_BW_LEVEL_NONE;
		pld_request_bus_bandwidth(hdd_ctx->parent_dev,
			    bbm_convert_to_pld_bus_lvl(BUS_BW_LEVEL_NONE));
		return;
	default:
		hdd_debug("flag %d not handled in res/sus BBM policy", flag);
		return;
	}
}

/**
 * bbm_apply_wlm_policy() - Apply WLM based BBM policy by selecting
 *  lookup tables based on the latency level
 * @bbm_ctx: Bus BW mgr context
 * @wlm_level: WLM latency level
 *
 * Returns: None
 */
static void
bbm_apply_wlm_policy(struct bbm_context *bbm_ctx, enum wlm_ll_level wlm_level)
{
	switch (wlm_level) {
	case WLM_LL_NORMAL:
		bbm_ctx->curr_bus_bw_lookup_table = &bus_bw_table_default;
		break;
	case WLM_LL_LOW:
		bbm_ctx->curr_bus_bw_lookup_table = &bus_bw_table_low_latency;
		break;
	default:
		hdd_debug("wlm level %d not handled in BBM WLM policy",
			  wlm_level);
		break;
	}
}

/**
 * bbm_apply_user_policy() - Apply user specified bus voting
 *  level
 * @bbm_ctx: Bus BW mgr context
 * @set: set or reset flag
 * @user_level: user bus vote level
 *
 * Returns: qdf status
 */
static QDF_STATUS
bbm_apply_user_policy(struct bbm_context *bbm_ctx, bool set,
		      enum bus_bw_level user_level)
{
	if (user_level >= BUS_BW_LEVEL_MAX) {
		hdd_err("Invalid user vote level %d", user_level);
		return QDF_STATUS_E_FAILURE;
	}

	if (set)
		bbm_ctx->per_policy_vote[BBM_USER_POLICY] = user_level;
	else
		bbm_ctx->per_policy_vote[BBM_USER_POLICY] = BUS_BW_LEVEL_NONE;

	return QDF_STATUS_SUCCESS;
}

/**
 * bbm_request_bus_bandwidth() - Set bus bandwidth level
 * @hdd_ctx: HDD context
 *
 * Returns: None
 */
static void
bbm_request_bus_bandwidth(struct hdd_context *hdd_ctx)
{
	enum bbm_policy i;
	enum bus_bw_level next_vote = BUS_BW_LEVEL_NONE;
	enum pld_bus_width_type pld_vote;
	struct bbm_context *bbm_ctx = hdd_ctx->bbm_ctx;

	for (i = BBM_DRIVER_MODE_POLICY; i < BBM_MAX_POLICY; i++) {
		if (bbm_ctx->per_policy_vote[i] > next_vote)
			next_vote = bbm_ctx->per_policy_vote[i];
	}

	if (next_vote != bbm_ctx->curr_vote_level) {
		pld_vote = bbm_convert_to_pld_bus_lvl(next_vote);
		hdd_debug("Bus bandwidth vote level change from %d to %d pld_vote: %d",
			  bbm_ctx->curr_vote_level, next_vote, pld_vote);
		bbm_ctx->curr_vote_level = next_vote;
		pld_request_bus_bandwidth(hdd_ctx->parent_dev, pld_vote);
	}
}

void hdd_bbm_apply_independent_policy(struct hdd_context *hdd_ctx,
				      struct bbm_params *params)
{
	struct bbm_context *bbm_ctx;
	QDF_STATUS status;

	if (!hdd_ctx || !params)
		return;

	bbm_ctx = hdd_ctx->bbm_ctx;

	qdf_mutex_acquire(&bbm_ctx->bbm_lock);

	switch (params->policy) {
	case BBM_TPUT_POLICY:
		bbm_apply_tput_policy(hdd_ctx, params->policy_info.tput_level);
		break;
	case BBM_NON_PERSISTENT_POLICY:
		bbm_apply_non_persistent_policy(hdd_ctx,
						params->policy_info.flag);
		goto done;
	case BBM_DRIVER_MODE_POLICY:
		bbm_apply_driver_mode_policy(bbm_ctx,
					     params->policy_info.driver_mode);
		break;
	case BBM_SELECT_TABLE_POLICY:
		bbm_apply_wlm_policy(bbm_ctx, params->policy_info.wlm_level);
		goto done;
	case BBM_USER_POLICY:
		/*
		 * This policy is not used currently.
		 */
		status = bbm_apply_user_policy(bbm_ctx,
					    params->policy_info.usr.set,
					    params->policy_info.usr.user_level);
		if (QDF_IS_STATUS_ERROR(status))
			goto done;
		break;
	default:
		hdd_debug("BBM policy %d not handled", params->policy);
		goto done;
	}

	bbm_request_bus_bandwidth(hdd_ctx);

done:
	qdf_mutex_release(&bbm_ctx->bbm_lock);
}

int hdd_bbm_context_init(struct hdd_context *hdd_ctx)
{
	struct bbm_context *bbm_ctx;
	QDF_STATUS status;

	bbm_ctx = qdf_mem_malloc(sizeof(*bbm_ctx));
	if (!bbm_ctx)
		return -ENOMEM;

	bbm_ctx->curr_bus_bw_lookup_table = &bus_bw_table_default;

	status = qdf_mutex_create(&bbm_ctx->bbm_lock);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_ctx;

	hdd_ctx->bbm_ctx = bbm_ctx;

	return 0;

free_ctx:
	qdf_mem_free(bbm_ctx);

	return qdf_status_to_os_return(status);
}

void hdd_bbm_context_deinit(struct hdd_context *hdd_ctx)
{
	struct bbm_context *bbm_ctx = hdd_ctx->bbm_ctx;

	if (!bbm_ctx)
		return;

	hdd_ctx->bbm_ctx = NULL;
	qdf_mutex_destroy(&bbm_ctx->bbm_lock);

	qdf_mem_free(bbm_ctx);
}
