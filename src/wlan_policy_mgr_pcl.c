/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_policy_mgr_pcl.c
 *
 * WLAN Concurrenct Connection Management APIs
 *
 */

/* Include files */

#include "wlan_policy_mgr_api.h"
#include "wlan_policy_mgr_tables_no_dbs_i.h"
#include "wlan_policy_mgr_i.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_global_obj.h"

#define PM_24_GHZ_CHANNEL_6   (6)
/**
 * first_connection_pcl_table - table which provides PCL for the
 * very first connection in the system
 */
const enum policy_mgr_pcl_type
first_connection_pcl_table[PM_MAX_NUM_OF_MODE]
			[PM_MAX_CONC_PRIORITY_MODE] = {
	[PM_STA_MODE] = {PM_NONE, PM_NONE, PM_NONE},
	[PM_SAP_MODE] = {PM_5G,   PM_5G,   PM_5G  },
	[PM_P2P_CLIENT_MODE] = {PM_5G,   PM_5G,   PM_5G  },
	[PM_P2P_GO_MODE] = {PM_5G,   PM_5G,   PM_5G  },
	[PM_IBSS_MODE] = {PM_NONE, PM_NONE, PM_NONE},
};

pm_dbs_pcl_second_connection_table_type
		*second_connection_pcl_dbs_table;
pm_dbs_pcl_third_connection_table_type
		*third_connection_pcl_dbs_table;
policy_mgr_next_action_two_connection_table_type
		*next_action_two_connection_table;
policy_mgr_next_action_three_connection_table_type
		*next_action_three_connection_table;

QDF_STATUS policy_mgr_get_pcl_for_existing_conn(struct wlan_objmgr_psoc *psoc,
		enum policy_mgr_con_mode mode,
		uint8_t *pcl_ch, uint32_t *len,
		uint8_t *pcl_weight, uint32_t weight_len)
{
	struct policy_mgr_conc_connection_info info;

	QDF_STATUS status = QDF_STATUS_SUCCESS;

	policy_mgr_notice("get pcl for existing conn:%d", mode);

	if (policy_mgr_mode_specific_connection_count(psoc, mode, NULL) > 0) {
		/* Check, store and temp delete the mode's parameter */
		policy_mgr_store_and_del_conn_info(psoc, mode, &info);
		/* Get the PCL */
		status = policy_mgr_get_pcl(psoc, mode, pcl_ch, len,
					pcl_weight, weight_len);
		policy_mgr_notice("Get PCL to FW for mode:%d", mode);
		/* Restore the connection info */
		policy_mgr_restore_deleted_conn_info(psoc, &info);
	}

	return status;
}

void policy_mgr_decr_session_set_pcl(struct wlan_objmgr_psoc *psoc,
						enum tQDF_ADAPTER_MODE mode,
						uint8_t session_id)
{
	QDF_STATUS qdf_status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	policy_mgr_decr_active_session(psoc, mode, session_id);
	/*
	 * After the removal of this connection, we need to check if
	 * a STA connection still exists. The reason for this is that
	 * if one or more STA exists, we need to provide the updated
	 * PCL to the FW for cases like LFR.
	 *
	 * Since policy_mgr_get_pcl provides PCL list based on the new
	 * connection that is going to come up, we will find the
	 * existing STA entry, save it and delete it temporarily.
	 * After this we will get PCL as though as new STA connection
	 * is coming up. This will give the exact PCL that needs to be
	 * given to the FW. After setting the PCL, we need to restore
	 * the entry that we have saved before.
	 */
	policy_mgr_set_pcl_for_existing_combo(psoc, PM_STA_MODE);
	/* do we need to change the HW mode */
	if (policy_mgr_need_opportunistic_upgrade(psoc)) {
		/* let's start the timer */
		qdf_mc_timer_stop(&pm_ctx->dbs_opportunistic_timer);
		qdf_status = qdf_mc_timer_start(
					&pm_ctx->dbs_opportunistic_timer,
					DBS_OPPORTUNISTIC_TIME * 1000);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			policy_mgr_err("Failed to start dbs opportunistic timer");
	}

	return;
}

void policy_mgr_update_with_safe_channel_list(uint8_t *pcl_channels,
		uint32_t *len, uint8_t *weight_list, uint32_t weight_len)
{
	return;
}

static QDF_STATUS policy_mgr_modify_pcl_based_on_enabled_channels(
					struct policy_mgr_psoc_priv_obj *pm_ctx,
					uint8_t *pcl_list_org,
					uint8_t *weight_list_org,
					uint32_t *pcl_len_org)
{
	uint32_t i, pcl_len = 0;
	uint8_t pcl_list[QDF_MAX_NUM_CHAN];
	uint8_t weight_list[QDF_MAX_NUM_CHAN];

	for (i = 0; i < *pcl_len_org; i++) {
		if (!wlan_reg_is_passive_or_disable_ch(
			pm_ctx->pdev, pcl_list_org[i])) {
			pcl_list[pcl_len] = pcl_list_org[i];
			weight_list[pcl_len++] = weight_list_org[i];
		}
	}

	qdf_mem_zero(pcl_list_org, QDF_ARRAY_SIZE(pcl_list_org));
	qdf_mem_zero(weight_list_org, QDF_ARRAY_SIZE(weight_list_org));
	qdf_mem_copy(pcl_list_org, pcl_list, pcl_len);
	qdf_mem_copy(weight_list_org, weight_list, pcl_len);
	*pcl_len_org = pcl_len;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS policy_mgr_modify_pcl_based_on_dnbs(
						struct wlan_objmgr_psoc *psoc,
						uint8_t *pcl_list_org,
						uint8_t *weight_list_org,
						uint32_t *pcl_len_org)
{
	uint32_t i, pcl_len = 0;
	uint8_t pcl_list[QDF_MAX_NUM_CHAN];
	uint8_t weight_list[QDF_MAX_NUM_CHAN];
	bool ok;
	int ret;

	for (i = 0; i < *pcl_len_org; i++) {
		ret = policy_mgr_is_chan_ok_for_dnbs(psoc, pcl_list_org[i],
						&ok);

		if (QDF_IS_STATUS_ERROR(ret)) {
			policy_mgr_err("Not able to check DNBS eligibility");
			return ret;
		}
		if (ok) {
			pcl_list[pcl_len] = pcl_list_org[i];
			weight_list[pcl_len++] = weight_list_org[i];
		}
	}

	qdf_mem_zero(pcl_list_org, QDF_ARRAY_SIZE(pcl_list_org));
	qdf_mem_zero(weight_list_org, QDF_ARRAY_SIZE(weight_list_org));
	qdf_mem_copy(pcl_list_org, pcl_list, pcl_len);
	qdf_mem_copy(weight_list_org, weight_list, pcl_len);
	*pcl_len_org = pcl_len;

	return QDF_STATUS_SUCCESS;
}

uint8_t policy_mgr_get_channel(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_con_mode mode, uint32_t *vdev_id)
{
	uint32_t idx = 0;
	uint8_t chan;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	if (mode >= PM_MAX_NUM_OF_MODE) {
		policy_mgr_err("incorrect mode");
		return 0;
	}

	for (idx = 0; idx < MAX_NUMBER_OF_CONC_CONNECTIONS; idx++) {
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		if ((pm_conc_connection_list[idx].mode == mode) &&
				(!vdev_id || (*vdev_id ==
					pm_conc_connection_list[idx].vdev_id))
				&& pm_conc_connection_list[idx].in_use) {
			chan =  pm_conc_connection_list[idx].chan;
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			return chan;
		}
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	}

	return 0;
}

QDF_STATUS policy_mgr_get_pcl(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_con_mode mode,
			uint8_t *pcl_channels, uint32_t *len,
			uint8_t *pcl_weight, uint32_t weight_len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t num_connections = 0, i;
	enum policy_mgr_conc_priority_mode first_index = 0;
	enum policy_mgr_one_connection_mode second_index = 0;
	enum policy_mgr_two_connection_mode third_index = 0;
	enum policy_mgr_pcl_type pcl = PM_NONE;
	enum policy_mgr_conc_priority_mode conc_system_pref = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("context is NULL");
		return status;
	}

	/* find the current connection state from pm_conc_connection_list*/
	num_connections = policy_mgr_get_connection_count(psoc);
	policy_mgr_debug("connections:%d pref:%d requested mode:%d",
		num_connections, pm_ctx->user_cfg.conc_system_pref, mode);

	switch (pm_ctx->user_cfg.conc_system_pref) {
	case 0:
		conc_system_pref = PM_THROUGHPUT;
		break;
	case 1:
		conc_system_pref = PM_POWERSAVE;
		break;
	case 2:
		conc_system_pref = PM_LATENCY;
		break;
	default:
		policy_mgr_err("unknown conc_system_pref value %d",
			pm_ctx->user_cfg.conc_system_pref);
		break;
	}

	switch (num_connections) {
	case 0:
		first_index =
			policy_mgr_get_first_connection_pcl_table_index(psoc);
		pcl = first_connection_pcl_table[mode][first_index];
		break;
	case 1:
		second_index =
			policy_mgr_get_second_connection_pcl_table_index(psoc);
		if (PM_MAX_ONE_CONNECTION_MODE == second_index) {
			policy_mgr_err("couldn't find index for 2nd connection pcl table");
			return status;
		}
		if (policy_mgr_is_hw_dbs_capable(psoc) == true) {
			pcl = (*second_connection_pcl_dbs_table)
				[second_index][mode][conc_system_pref];
		} else {
			pcl = second_connection_pcl_nodbs_table
				[second_index][mode][conc_system_pref];
		}

		break;
	case 2:
		third_index =
			policy_mgr_get_third_connection_pcl_table_index(psoc);
		if (PM_MAX_TWO_CONNECTION_MODE == third_index) {
			policy_mgr_err(
				"couldn't find index for 3rd connection pcl table");
			return status;
		}
		if (policy_mgr_is_hw_dbs_capable(psoc) == true) {
			pcl = (*third_connection_pcl_dbs_table)
				[third_index][mode][conc_system_pref];
		} else {
			pcl = third_connection_pcl_nodbs_table
				[third_index][mode][conc_system_pref];
		}
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
			num_connections);
		break;
	}

	policy_mgr_debug("index1:%d index2:%d index3:%d pcl:%d dbs:%d",
		first_index, second_index, third_index,
		pcl, policy_mgr_is_hw_dbs_capable(psoc));

	/* once the PCL enum is obtained find out the exact channel list with
	 * help from sme_get_cfg_valid_channels
	 */
	status = policy_mgr_get_channel_list(psoc, pcl, pcl_channels, len, mode,
					pcl_weight, weight_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("failed to get channel list:%d", status);
		return status;
	}

	policy_mgr_debug("pcl len:%d", *len);
	for (i = 0; i < *len; i++) {
		policy_mgr_debug("chan:%d weight:%d",
				pcl_channels[i], pcl_weight[i]);
	}

	if ((mode == PM_SAP_MODE) &&
		policy_mgr_is_sap_mandatory_channel_set(psoc)) {
		status = policy_mgr_modify_sap_pcl_based_on_mandatory_channel(
					psoc, pcl_channels, pcl_weight, len);
		if (QDF_IS_STATUS_ERROR(status)) {
			policy_mgr_err("failed to get modified pcl for SAP");
			return status;
		}
		policy_mgr_debug("modified pcl len:%d", *len);
		for (i = 0; i < *len; i++)
			policy_mgr_debug("chan:%d weight:%d",
					pcl_channels[i], pcl_weight[i]);

	}

	if (mode == PM_P2P_GO_MODE) {
		status = policy_mgr_modify_pcl_based_on_enabled_channels(
		pm_ctx, pcl_channels, pcl_weight, len);
		if (QDF_IS_STATUS_ERROR(status)) {
			policy_mgr_err("failed to get modified pcl for GO");
			return status;
		}
		policy_mgr_debug("modified pcl len:%d", *len);
		for (i = 0; i < *len; i++)
			policy_mgr_debug("chan:%d weight:%d",
			pcl_channels[i], pcl_weight[i]);
	}

	status = policy_mgr_modify_pcl_based_on_dnbs(psoc, pcl_channels,
						pcl_weight, len);

	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("failed to get modified pcl based on DNBS");
		return status;
	}
	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_conc_priority_mode
		policy_mgr_get_first_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("context is NULL");
		return PM_THROUGHPUT;
	}

	if (pm_ctx->user_cfg.conc_system_pref >= PM_MAX_CONC_PRIORITY_MODE)
		return PM_THROUGHPUT;

	return pm_ctx->user_cfg.conc_system_pref;
}

enum policy_mgr_one_connection_mode
		policy_mgr_get_second_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	enum policy_mgr_one_connection_mode index = PM_MAX_ONE_CONNECTION_MODE;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return index;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (PM_STA_MODE == pm_conc_connection_list[0].mode) {
		if (WLAN_REG_IS_24GHZ_CH(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_24_1x1;
			else
				index = PM_STA_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_5_1x1;
			else
				index = PM_STA_5_2x2;
		}
	} else if (PM_SAP_MODE == pm_conc_connection_list[0].mode) {
		if (WLAN_REG_IS_24GHZ_CH(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_24_1x1;
			else
				index = PM_SAP_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_5_1x1;
			else
				index = PM_SAP_5_2x2;
		}
	} else if (PM_P2P_CLIENT_MODE == pm_conc_connection_list[0].mode) {
		if (WLAN_REG_IS_24GHZ_CH(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_24_1x1;
			else
				index = PM_P2P_CLI_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_5_1x1;
			else
				index = PM_P2P_CLI_5_2x2;
		}
	} else if (PM_P2P_GO_MODE == pm_conc_connection_list[0].mode) {
		if (WLAN_REG_IS_24GHZ_CH(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_24_1x1;
			else
				index = PM_P2P_GO_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_5_1x1;
			else
				index = PM_P2P_GO_5_2x2;
		}
	} else if (PM_IBSS_MODE == pm_conc_connection_list[0].mode) {
		if (WLAN_REG_IS_24GHZ_CH(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_IBSS_24_1x1;
			else
				index = PM_IBSS_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_IBSS_5_1x1;
			else
				index = PM_IBSS_5_2x2;
		}
	}

	policy_mgr_debug("mode:%d chan:%d chain:%d index:%d",
		pm_conc_connection_list[0].mode,
		pm_conc_connection_list[0].chan,
		pm_conc_connection_list[0].chain_mask, index);

	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_cli_sap(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_SCC_24_1x1;
			else
				index = PM_P2P_CLI_SAP_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_SCC_5_1x1;
			else
				index = PM_P2P_CLI_SAP_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_MCC_24_1x1;
			else
				index = PM_P2P_CLI_SAP_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_MCC_5_1x1;
			else
				index = PM_P2P_CLI_SAP_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_MCC_24_5_1x1;
			else
				index = PM_P2P_CLI_SAP_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_CLI_SAP_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_P2P_CLI_SAP_DBS_1x1;
		else
			index = PM_P2P_CLI_SAP_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_sta_sap(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_SCC_24_1x1;
			else
				index = PM_STA_SAP_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_SCC_5_1x1;
			else
				index = PM_STA_SAP_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_MCC_24_1x1;
			else
				index = PM_STA_SAP_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_MCC_5_1x1;
			else
				index = PM_STA_SAP_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_MCC_24_5_1x1;
			else
				index = PM_STA_SAP_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_SAP_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_STA_SAP_DBS_1x1;
		else
			index = PM_STA_SAP_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_sap_sap(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_SCC_24_1x1;
			else
				index = PM_SAP_SAP_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_SCC_5_1x1;
			else
				index = PM_SAP_SAP_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_MCC_24_1x1;
			else
				index = PM_SAP_SAP_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_MCC_5_1x1;
			else
				index = PM_SAP_SAP_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_MCC_24_5_1x1;
			else
				index = PM_SAP_SAP_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_SAP_SAP_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_SAP_SAP_DBS_1x1;
		else
			index = PM_SAP_SAP_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_sta_go(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_SCC_24_1x1;
			else
				index = PM_STA_P2P_GO_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_SCC_5_1x1;
			else
				index = PM_STA_P2P_GO_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_MCC_24_1x1;
			else
				index = PM_STA_P2P_GO_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_MCC_5_1x1;
			else
				index = PM_STA_P2P_GO_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_MCC_24_5_1x1;
			else
				index = PM_STA_P2P_GO_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_GO_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_STA_P2P_GO_DBS_1x1;
		else
			index = PM_STA_P2P_GO_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_sta_cli(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_SCC_24_1x1;
			else
				index = PM_STA_P2P_CLI_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_SCC_5_1x1;
			else
				index = PM_STA_P2P_CLI_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_MCC_24_1x1;
			else
				index = PM_STA_P2P_CLI_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_MCC_5_1x1;
			else
				index = PM_STA_P2P_CLI_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_MCC_24_5_1x1;
			else
				index = PM_STA_P2P_CLI_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_P2P_CLI_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_STA_P2P_CLI_DBS_1x1;
		else
			index = PM_STA_P2P_CLI_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_go_cli(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_SCC_24_1x1;
			else
				index = PM_P2P_GO_P2P_CLI_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_SCC_5_1x1;
			else
				index = PM_P2P_GO_P2P_CLI_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_MCC_24_1x1;
			else
				index = PM_P2P_GO_P2P_CLI_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_MCC_5_1x1;
			else
				index = PM_P2P_GO_P2P_CLI_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_MCC_24_5_1x1;
			else
				index = PM_P2P_GO_P2P_CLI_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_P2P_CLI_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_P2P_GO_P2P_CLI_DBS_1x1;
		else
			index = PM_P2P_GO_P2P_CLI_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_go_sap(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_SCC_24_1x1;
			else
				index = PM_P2P_GO_SAP_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_SCC_5_1x1;
			else
				index = PM_P2P_GO_SAP_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_MCC_24_1x1;
			else
				index = PM_P2P_GO_SAP_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_MCC_5_1x1;
			else
				index = PM_P2P_GO_SAP_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_MCC_24_5_1x1;
			else
				index = PM_P2P_GO_SAP_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_P2P_GO_SAP_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_P2P_GO_SAP_DBS_1x1;
		else
			index = PM_P2P_GO_SAP_DBS_2x2;
	}

	return index;
}

static enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index_sta_sta(void)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	/* SCC */
	if (pm_conc_connection_list[0].chan ==
		pm_conc_connection_list[1].chan) {
		if (WLAN_REG_IS_24GHZ_CH
			(pm_conc_connection_list[0].chan)) {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_SCC_24_1x1;
			else
				index = PM_STA_STA_SCC_24_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_SCC_5_1x1;
			else
				index = PM_STA_STA_SCC_5_2x2;
		}
	/* MCC */
	} else if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_MCC_24_1x1;
			else
				index = PM_STA_STA_MCC_24_2x2;
		} else if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_MCC_5_1x1;
			else
				index = PM_STA_STA_MCC_5_2x2;
		} else {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_MCC_24_5_1x1;
			else
				index = PM_STA_STA_MCC_24_5_2x2;
		}
	/* SBS */
	} else if (pm_conc_connection_list[0].mac !=
		pm_conc_connection_list[1].mac) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[0].chan)) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[1].chan))) {
			if (POLICY_MGR_ONE_ONE ==
				pm_conc_connection_list[0].chain_mask)
				index = PM_STA_STA_SBS_5_1x1;
		}
	/* DBS */
	} else {
		if (POLICY_MGR_ONE_ONE ==
			pm_conc_connection_list[0].chain_mask)
			index = PM_STA_STA_DBS_1x1;
		else
			index = PM_STA_STA_DBS_2x2;
	}

	return index;
}

enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	enum policy_mgr_two_connection_mode index = PM_MAX_TWO_CONNECTION_MODE;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return index;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (((PM_P2P_CLIENT_MODE == pm_conc_connection_list[0].mode) &&
		(PM_SAP_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_SAP_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_CLIENT_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_cli_sap();
	else if (((PM_STA_MODE == pm_conc_connection_list[0].mode) &&
		(PM_SAP_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_SAP_MODE == pm_conc_connection_list[0].mode) &&
		(PM_STA_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_sta_sap();
	else if ((PM_SAP_MODE == pm_conc_connection_list[0].mode) &&
		(PM_SAP_MODE == pm_conc_connection_list[1].mode))
		index =
		policy_mgr_get_third_connection_pcl_table_index_sap_sap();
	else if (((PM_STA_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_GO_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_P2P_GO_MODE == pm_conc_connection_list[0].mode) &&
		(PM_STA_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_sta_go();
	else if (((PM_STA_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_CLIENT_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_P2P_CLIENT_MODE == pm_conc_connection_list[0].mode) &&
		(PM_STA_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_sta_cli();
	else if (((PM_P2P_GO_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_CLIENT_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_P2P_CLIENT_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_GO_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_go_cli();
	else if (((PM_SAP_MODE == pm_conc_connection_list[0].mode) &&
		(PM_P2P_GO_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_P2P_GO_MODE == pm_conc_connection_list[0].mode) &&
		(PM_SAP_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_go_sap();
	else if (((PM_STA_MODE == pm_conc_connection_list[0].mode) &&
		(PM_STA_MODE == pm_conc_connection_list[1].mode)) ||
		((PM_STA_MODE == pm_conc_connection_list[0].mode) &&
		(PM_STA_MODE == pm_conc_connection_list[1].mode)))
		index =
		policy_mgr_get_third_connection_pcl_table_index_sta_sta();

	policy_mgr_debug("mode0:%d mode1:%d chan0:%d chan1:%d chain:%d index:%d",
		pm_conc_connection_list[0].mode,
		pm_conc_connection_list[1].mode,
		pm_conc_connection_list[0].chan,
		pm_conc_connection_list[1].chan,
		pm_conc_connection_list[0].chain_mask, index);

	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return index;
}

uint8_t
policy_mgr_get_nondfs_preferred_channel(struct wlan_objmgr_psoc *psoc,
		enum policy_mgr_con_mode mode,
		bool for_existing_conn)
{
	uint8_t pcl_channels[QDF_MAX_NUM_CHAN];
	uint8_t pcl_weight[QDF_MAX_NUM_CHAN];
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	/*
	 * in worst case if we can't find any channel at all
	 * then return 2.4G channel, so atleast we won't fall
	 * under 5G MCC scenario
	 */
	uint8_t channel = PM_24_GHZ_CHANNEL_6;
	uint32_t i, pcl_len;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return channel;
	}

	if (true == for_existing_conn) {
		/*
		 * First try to see if there is any non-dfs channel already
		 * present in current connection table. If yes then return
		 * that channel
		 */
		if (true == policy_mgr_is_any_nondfs_chnl_present(
			psoc, &channel))
			return channel;

		if (QDF_STATUS_SUCCESS != policy_mgr_get_pcl_for_existing_conn(
					psoc, mode,
					&pcl_channels[0], &pcl_len,
					pcl_weight, QDF_ARRAY_SIZE(pcl_weight)))
			return channel;
	} else {
		if (QDF_STATUS_SUCCESS != policy_mgr_get_pcl(psoc, mode,
					&pcl_channels[0], &pcl_len,
					pcl_weight, QDF_ARRAY_SIZE(pcl_weight)))
			return channel;
	}

	for (i = 0; i < pcl_len; i++) {
		if (wlan_reg_is_dfs_ch(pm_ctx->pdev, pcl_channels[i])) {
			continue;
		} else {
			channel = pcl_channels[i];
			break;
		}
	}

	return channel;
}

QDF_STATUS policy_mgr_get_valid_chans(struct wlan_objmgr_psoc *psoc,
				uint8_t *chan_list, uint32_t *list_len)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	*list_len = 0;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pm_ctx->sme_cbacks.sme_get_valid_channels) {
		policy_mgr_err("sme_get_valid_chans callback is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	*list_len = QDF_MAX_NUM_CHAN;
	status = pm_ctx->sme_cbacks.sme_get_valid_channels(
					chan_list, list_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		*list_len = 0;
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_list_has_24GHz_channel(uint8_t *channel_list,
					uint32_t list_len)
{
	uint32_t i;

	for (i = 0; i < list_len; i++) {
		if (WLAN_REG_IS_24GHZ_CH(channel_list[i]))
			return true;
	}

	return false;
}

QDF_STATUS policy_mgr_set_sap_mandatory_channels(struct wlan_objmgr_psoc *psoc,
					uint8_t *channels, uint32_t len)
{
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	if (!len) {
		policy_mgr_err("No mandatory freq/chan configured");
		return QDF_STATUS_E_FAILURE;
	}

	if (!policy_mgr_list_has_24GHz_channel(channels, len)) {
		policy_mgr_err("2.4GHz channels missing, this is not expected");
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_debug("mandatory chan length:%d",
			pm_ctx->sap_mandatory_channels_len);

	for (i = 0; i < len; i++) {
		pm_ctx->sap_mandatory_channels[i] = channels[i];
		policy_mgr_debug("chan:%d", pm_ctx->sap_mandatory_channels[i]);
	}

	pm_ctx->sap_mandatory_channels_len = len;

	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_is_sap_mandatory_channel_set(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	if (pm_ctx->sap_mandatory_channels_len)
		return true;
	else
		return false;
}

QDF_STATUS policy_mgr_modify_sap_pcl_based_on_mandatory_channel(
		struct wlan_objmgr_psoc *psoc,
		uint8_t *pcl_list_org,
		uint8_t *weight_list_org,
		uint32_t *pcl_len_org)
{
	uint32_t i, j, pcl_len = 0;
	uint8_t pcl_list[QDF_MAX_NUM_CHAN];
	uint8_t weight_list[QDF_MAX_NUM_CHAN];
	bool found;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pm_ctx->sap_mandatory_channels_len)
		return QDF_STATUS_SUCCESS;

	if (!policy_mgr_list_has_24GHz_channel(pm_ctx->sap_mandatory_channels,
			pm_ctx->sap_mandatory_channels_len)) {
		policy_mgr_err("fav channel list is missing 2.4GHz channels");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < pm_ctx->sap_mandatory_channels_len; i++)
		policy_mgr_debug("fav chan:%d",
			pm_ctx->sap_mandatory_channels[i]);

	for (i = 0; i < *pcl_len_org; i++) {
		found = false;
		for (j = 0; j < pm_ctx->sap_mandatory_channels_len; j++) {
			if (pcl_list_org[i] ==
			    pm_ctx->sap_mandatory_channels[j]) {
				found = true;
				break;
			}
		}
		if (found) {
			pcl_list[pcl_len] = pcl_list_org[i];
			weight_list[pcl_len++] = weight_list_org[i];
		}
	}

	qdf_mem_zero(pcl_list_org, QDF_ARRAY_SIZE(pcl_list_org));
	qdf_mem_zero(weight_list_org, QDF_ARRAY_SIZE(weight_list_org));
	qdf_mem_copy(pcl_list_org, pcl_list, pcl_len);
	qdf_mem_copy(weight_list_org, weight_list, pcl_len);
	*pcl_len_org = pcl_len;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_sap_mandatory_channel(struct wlan_objmgr_psoc *psoc,
		uint32_t *chan)
{
	QDF_STATUS status;
	struct policy_mgr_pcl_list pcl;

	qdf_mem_zero(&pcl, sizeof(pcl));

	status = policy_mgr_get_pcl_for_existing_conn(psoc, PM_SAP_MODE,
			pcl.pcl_list, &pcl.pcl_len,
			pcl.weight_list, QDF_ARRAY_SIZE(pcl.weight_list));
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Unable to get PCL for SAP");
		return status;
	}

	/* No existing SAP connection and hence a new SAP connection might be
	 * coming up.
	 */
	if (!pcl.pcl_len) {
		policy_mgr_notice("policy_mgr_get_pcl_for_existing_conn returned no pcl");
		status = policy_mgr_get_pcl(psoc, PM_SAP_MODE,
				pcl.pcl_list, &pcl.pcl_len,
				pcl.weight_list,
				QDF_ARRAY_SIZE(pcl.weight_list));
		if (QDF_IS_STATUS_ERROR(status)) {
			policy_mgr_err("Unable to get PCL for SAP: policy_mgr_get_pcl");
			return status;
		}
	}

	status = policy_mgr_modify_sap_pcl_based_on_mandatory_channel(
							psoc, pcl.pcl_list,
							pcl.weight_list,
							&pcl.pcl_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Unable to modify SAP PCL");
		return status;
	}

	*chan = pcl.pcl_list[0];
	policy_mgr_notice("mandatory channel:%d", *chan);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_valid_chan_weights(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_pcl_chan_weights *weight)
{
	uint32_t i, j;
	struct policy_mgr_conc_connection_info info;

	if (!weight->pcl_list) {
		policy_mgr_err("Invalid pcl");
		return QDF_STATUS_E_FAILURE;
	}

	if (!weight->saved_chan_list) {
		policy_mgr_err("Invalid valid channel list");
		return QDF_STATUS_E_FAILURE;
	}

	if (!weight->weighed_valid_list) {
		policy_mgr_err("Invalid weighed valid channel list");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_set(weight->weighed_valid_list, QDF_MAX_NUM_CHAN,
		    WEIGHT_OF_DISALLOWED_CHANNELS);

	if (policy_mgr_mode_specific_connection_count(
		psoc, PM_STA_MODE, NULL) > 0) {
		/*
		 * Store the STA mode's parameter and temporarily delete it
		 * from the concurrency table. This way the allow concurrency
		 * check can be used as though a new connection is coming up,
		 * allowing to detect the disallowed channels.
		 */
		policy_mgr_store_and_del_conn_info(psoc, PM_STA_MODE, &info);
		/*
		 * There is a small window between releasing the above lock
		 * and acquiring the same in policy_mgr_allow_concurrency,
		 * below!
		 */
		for (i = 0; i < weight->saved_num_chan; i++) {
			if (policy_mgr_allow_concurrency(psoc, PM_STA_MODE,
						  weight->saved_chan_list[i],
						  HW_MODE_20_MHZ)) {
				weight->weighed_valid_list[i] =
					WEIGHT_OF_NON_PCL_CHANNELS;
			}
		}

		/* Restore the connection info */
		policy_mgr_restore_deleted_conn_info(psoc, &info);
	}

	for (i = 0; i < weight->saved_num_chan; i++) {
		for (j = 0; j < weight->pcl_len; j++) {
			if (weight->saved_chan_list[i] == weight->pcl_list[j]) {
				weight->weighed_valid_list[i] =
					weight->weight_list[j];
				break;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}
