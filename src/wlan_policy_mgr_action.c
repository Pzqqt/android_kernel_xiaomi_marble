/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_policy_mgr_action.c
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

enum policy_mgr_conc_next_action (*policy_mgr_get_current_pref_hw_mode_ptr)
	(struct wlan_objmgr_psoc *psoc);

void policy_mgr_hw_mode_transition_cb(uint32_t old_hw_mode_index,
			uint32_t new_hw_mode_index,
			uint32_t num_vdev_mac_entries,
			struct policy_mgr_vdev_mac_map *vdev_mac_map,
			struct wlan_objmgr_psoc *context)
{
	QDF_STATUS status;
	struct policy_mgr_hw_mode_params hw_mode;
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(context);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return;
	}

	if (!vdev_mac_map) {
		policy_mgr_err("vdev_mac_map is NULL");
		return;
	}

	policy_mgr_debug("old_hw_mode_index=%d, new_hw_mode_index=%d",
		old_hw_mode_index, new_hw_mode_index);

	for (i = 0; i < num_vdev_mac_entries; i++)
		policy_mgr_debug("vdev_id:%d mac_id:%d",
			vdev_mac_map[i].vdev_id,
			vdev_mac_map[i].mac_id);

	status = policy_mgr_get_hw_mode_from_idx(context,
				new_hw_mode_index, &hw_mode);
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("Get HW mode failed: %d", status);
		return;
	}

	policy_mgr_debug("MAC0: TxSS:%d, RxSS:%d, Bw:%d",
		hw_mode.mac0_tx_ss, hw_mode.mac0_rx_ss, hw_mode.mac0_bw);
	policy_mgr_debug("MAC1: TxSS:%d, RxSS:%d, Bw:%d",
		hw_mode.mac1_tx_ss, hw_mode.mac1_rx_ss, hw_mode.mac1_bw);
	policy_mgr_debug("DBS:%d, Agile DFS:%d, SBS:%d",
		hw_mode.dbs_cap, hw_mode.agile_dfs_cap, hw_mode.sbs_cap);

	/* update pm_conc_connection_list */
	policy_mgr_update_hw_mode_conn_info(context, num_vdev_mac_entries,
					  vdev_mac_map,
					  hw_mode);

	if (pm_ctx->mode_change_cb)
		pm_ctx->mode_change_cb();

	return;
}

QDF_STATUS policy_mgr_pdev_set_hw_mode(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs,
		enum policy_mgr_conn_update_reason reason,
		uint8_t next_action)
{
	int8_t hw_mode_index;
	struct policy_mgr_hw_mode msg;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * if HW is not capable of doing 2x2 or ini config disabled 2x2, don't
	 * allow to request FW for 2x2
	 */
	if ((HW_MODE_SS_2x2 == mac0_ss) && (!pm_ctx->user_cfg.enable2x2)) {
		policy_mgr_debug("2x2 is not allowed downgrading to 1x1 for mac0");
		mac0_ss = HW_MODE_SS_1x1;
	}
	if ((HW_MODE_SS_2x2 == mac1_ss) && (!pm_ctx->user_cfg.enable2x2)) {
		policy_mgr_debug("2x2 is not allowed downgrading to 1x1 for mac1");
		mac1_ss = HW_MODE_SS_1x1;
	}

	hw_mode_index = policy_mgr_get_hw_mode_idx_from_dbs_hw_list(psoc,
			mac0_ss, mac0_bw, mac1_ss, mac1_bw, dbs, dfs, sbs);
	if (hw_mode_index < 0) {
		policy_mgr_err("Invalid HW mode index obtained");
		return QDF_STATUS_E_FAILURE;
	}

	msg.hw_mode_index = hw_mode_index;
	msg.set_hw_mode_cb = (void *)policy_mgr_pdev_set_hw_mode_cb;
	msg.reason = reason;
	msg.session_id = session_id;
	msg.next_action = next_action;
	msg.context = psoc;

	policy_mgr_debug("set hw mode to sme: hw_mode_index: %d session:%d reason:%d",
		msg.hw_mode_index, msg.session_id, msg.reason);

	status = pm_ctx->sme_cbacks.sme_pdev_set_hw_mode(msg);
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("Failed to set hw mode to SME");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_conc_next_action policy_mgr_need_opportunistic_upgrade(
		struct wlan_objmgr_psoc *psoc)
{
	uint32_t conn_index;
	enum policy_mgr_conc_next_action upgrade = PM_NOP;
	uint8_t mac = 0;
	struct policy_mgr_hw_mode_params hw_mode;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		goto done;
	}

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_err("driver isn't dbs capable, no further action needed");
		goto done;
	}

	status = policy_mgr_get_current_hw_mode(psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("policy_mgr_get_current_hw_mode failed");
		goto done;
	}
	if (!hw_mode.dbs_cap) {
		policy_mgr_notice("current HW mode is non-DBS capable");
		goto done;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	/* Are both mac's still in use */
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		policy_mgr_debug("index:%d mac:%d in_use:%d chan:%d org_nss:%d",
			conn_index,
			pm_conc_connection_list[conn_index].mac,
			pm_conc_connection_list[conn_index].in_use,
			pm_conc_connection_list[conn_index].chan,
			pm_conc_connection_list[conn_index].original_nss);
		if ((pm_conc_connection_list[conn_index].mac == 0) &&
			pm_conc_connection_list[conn_index].in_use) {
			mac |= POLICY_MGR_MAC0;
			if (POLICY_MGR_MAC0_AND_MAC1 == mac) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				goto done;
			}
		} else if ((pm_conc_connection_list[conn_index].mac == 1) &&
			pm_conc_connection_list[conn_index].in_use) {
			mac |= POLICY_MGR_MAC1;
			if (policy_mgr_is_hw_dbs_2x2_capable(psoc) &&
			    WLAN_REG_IS_24GHZ_CH(
				    pm_conc_connection_list[conn_index].chan)
			    ) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				policy_mgr_debug("2X2 DBS capable with 2.4 GHZ connection");
				goto done;
			}
			if (POLICY_MGR_MAC0_AND_MAC1 == mac) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				goto done;
			}
		}
	}
	/* Let's request for single MAC mode */
	upgrade = PM_SINGLE_MAC;
	/* Is there any connection had an initial connection with 2x2 */
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if ((pm_conc_connection_list[conn_index].original_nss == 2) &&
			pm_conc_connection_list[conn_index].in_use) {
			upgrade = PM_SINGLE_MAC_UPGRADE;
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			goto done;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

done:
	return upgrade;
}

QDF_STATUS policy_mgr_update_connection_info(struct wlan_objmgr_psoc *psoc,
					uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index = 0;
	bool found = false;
	struct policy_mgr_vdev_entry_info conn_table_entry;
	enum policy_mgr_chain_mode chain_mask = POLICY_MGR_ONE_ONE;
	uint8_t nss_2g, nss_5g;
	enum policy_mgr_con_mode mode;
	uint8_t chan;
	uint32_t nss = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if (vdev_id == pm_conc_connection_list[conn_index].vdev_id) {
			/* debug msg */
			found = true;
			break;
		}
		conn_index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	if (!found) {
		/* err msg */
		policy_mgr_err("can't find vdev_id %d in pm_conc_connection_list",
			vdev_id);
		return status;
	}
	if (pm_ctx->wma_cbacks.wma_get_connection_info) {
		status = pm_ctx->wma_cbacks.wma_get_connection_info(
				vdev_id, &conn_table_entry);
		if (QDF_STATUS_SUCCESS != status) {
			policy_mgr_err("can't find vdev_id %d in connection table",
			vdev_id);
			return status;
		}
	} else {
		policy_mgr_err("wma_get_connection_info is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mode = policy_mgr_get_mode(conn_table_entry.type,
					conn_table_entry.sub_type);
	chan = wlan_reg_freq_to_chan(pm_ctx->pdev, conn_table_entry.mhz);
	status = policy_mgr_get_nss_for_vdev(psoc, mode, &nss_2g, &nss_5g);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if ((WLAN_REG_IS_24GHZ_CH(chan) && (nss_2g > 1)) ||
			(WLAN_REG_IS_5GHZ_CH(chan) && (nss_5g > 1)))
			chain_mask = POLICY_MGR_TWO_TWO;
		else
			chain_mask = POLICY_MGR_ONE_ONE;
		nss = (WLAN_REG_IS_24GHZ_CH(chan)) ? nss_2g : nss_5g;
	} else {
		policy_mgr_err("Error in getting nss");
	}

	policy_mgr_debug("update PM connection table for vdev:%d", vdev_id);

	/* add the entry */
	policy_mgr_update_conc_list(psoc, conn_index,
			mode,
			chan,
			policy_mgr_get_bw(conn_table_entry.chan_width),
			conn_table_entry.mac_id,
			chain_mask,
			nss, vdev_id, true);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_update_and_wait_for_connection_update(
		struct wlan_objmgr_psoc *psoc,
		uint8_t session_id,
		uint8_t channel,
		enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status;

	policy_mgr_debug("session:%d channel:%d reason:%d",
		session_id, channel, reason);

	status = policy_mgr_reset_connection_update(psoc);
	if (QDF_IS_STATUS_ERROR(status))
		policy_mgr_err("clearing event failed");

	status = policy_mgr_current_connections_update(psoc,
				session_id, channel, reason);
	if (QDF_STATUS_E_FAILURE == status) {
		policy_mgr_err("connections update failed");
		return QDF_STATUS_E_FAILURE;
	}

	/* Wait only when status is success */
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = policy_mgr_wait_for_connection_update(psoc);
		if (QDF_IS_STATUS_ERROR(status)) {
			policy_mgr_err("qdf wait for event failed");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_is_dbs_allowed_for_concurrency() - If dbs is allowed for current
 * concurreny
 * @new_conn_mode: new connection mode
 *
 * When a new connection is about to come up, check if dbs is allowed for
 * STA+STA or STA+P2P
 *
 * Return: true if dbs is allowed for STA+STA or STA+P2P else false
 */
static bool policy_mgr_is_dbs_allowed_for_concurrency(
		struct wlan_objmgr_psoc *psoc, uint32_t session_id)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t count, dbs_for_sta_sta, dbs_for_sta_p2p;
	enum tQDF_ADAPTER_MODE new_conn_mode = QDF_MAX_NO_OF_MODE;
	bool ret = true;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return ret;
	}

	count = policy_mgr_get_connection_count(psoc);
	if (pm_ctx->hdd_cbacks.hdd_get_device_mode)
		new_conn_mode = pm_ctx->hdd_cbacks.
					hdd_get_device_mode(session_id);

	if (count != 1 || new_conn_mode == QDF_MAX_NO_OF_MODE)
		return ret;

	dbs_for_sta_sta = PM_CHANNEL_SELECT_LOGIC_STA_STA_GET(pm_ctx->user_cfg.
						channel_select_logic_conc);
	dbs_for_sta_p2p = PM_CHANNEL_SELECT_LOGIC_STA_P2P_GET(pm_ctx->user_cfg.
						channel_select_logic_conc);

	switch (pm_conc_connection_list[0].mode) {
	case PM_STA_MODE:
		switch (new_conn_mode) {
		case QDF_STA_MODE:
			if (!dbs_for_sta_sta)
				return false;
			break;
		case QDF_P2P_DEVICE_MODE:
		case QDF_P2P_CLIENT_MODE:
		case QDF_P2P_GO_MODE:
			if (!dbs_for_sta_p2p)
				return false;
			break;
		default:
			break;
		}
		break;
	case PM_P2P_CLIENT_MODE:
	case PM_P2P_GO_MODE:
		switch (new_conn_mode) {
		case QDF_STA_MODE:
			if (!dbs_for_sta_p2p)
				return false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return ret;
}

QDF_STATUS policy_mgr_current_connections_update(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		uint8_t channel,
		enum policy_mgr_conn_update_reason reason)
{
	enum policy_mgr_conc_next_action next_action = PM_NOP;
	uint32_t num_connections = 0;
	enum policy_mgr_one_connection_mode second_index = 0;
	enum policy_mgr_two_connection_mode third_index = 0;
	enum policy_mgr_band band;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_err("driver isn't dbs capable, no further action needed");
		return QDF_STATUS_E_NOSUPPORT;
	}
	if (WLAN_REG_IS_24GHZ_CH(channel))
		band = POLICY_MGR_BAND_24;
	else
		band = POLICY_MGR_BAND_5;

	num_connections = policy_mgr_get_connection_count(psoc);

	policy_mgr_debug("num_connections=%d channel=%d",
		num_connections, channel);

	switch (num_connections) {
	case 0:
		if (band == POLICY_MGR_BAND_24)
			if (policy_mgr_is_hw_dbs_2x2_capable(psoc))
				next_action = PM_DBS;
			else
				next_action = PM_NOP;
		else
			next_action = PM_NOP;
		break;
	case 1:
		second_index =
			policy_mgr_get_second_connection_pcl_table_index(psoc);
		if (PM_MAX_ONE_CONNECTION_MODE == second_index) {
			policy_mgr_err(
			"couldn't find index for 2nd connection next action table");
			goto done;
		}
		next_action =
			(*next_action_two_connection_table)[second_index][band];
		break;
	case 2:
		third_index =
			policy_mgr_get_third_connection_pcl_table_index(psoc);
		if (PM_MAX_TWO_CONNECTION_MODE == third_index) {
			policy_mgr_err(
			"couldn't find index for 3rd connection next action table");
			goto done;
		}
		next_action = (*next_action_three_connection_table)
							[third_index][band];
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
			num_connections);
		break;
	}

	/*
	 * Based on channel_select_logic_conc ini, hw mode is set
	 * when second connection is about to come up that results
	 * in STA+STA and STA+P2P concurrency.
	 * 1) If MCC is set and if current hw mode is dbs, hw mode
	 *  should be set to single mac for above concurrency.
	 * 2) If MCC is set and if current hw mode is not dbs, hw
	 *  mode change is not required.
	 */
	if (policy_mgr_is_current_hwmode_dbs(psoc) &&
		!policy_mgr_is_dbs_allowed_for_concurrency(psoc, session_id))
		next_action = PM_SINGLE_MAC;
	else if (!policy_mgr_is_current_hwmode_dbs(psoc) &&
		!policy_mgr_is_dbs_allowed_for_concurrency(psoc, session_id))
		next_action = PM_NOP;

	if (PM_NOP != next_action)
		status = policy_mgr_next_actions(psoc, session_id,
						next_action, reason);
	else
		status = QDF_STATUS_E_NOSUPPORT;

	policy_mgr_debug(
		"idx2=%d idx3=%d next_action=%d, band=%d status=%d reason=%d session_id=%d",
		second_index, third_index, next_action, band, status,
		reason, session_id);

done:
	return status;
}

QDF_STATUS policy_mgr_next_actions(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		enum policy_mgr_conc_next_action action,
		enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct policy_mgr_hw_mode_params hw_mode;
	struct dbs_nss nss_dbs = {0};

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_err("driver isn't dbs capable, no further action needed");
		return QDF_STATUS_E_NOSUPPORT;
	}

	/* check for the current HW index to see if really need any action */
	status = policy_mgr_get_current_hw_mode(psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("policy_mgr_get_current_hw_mode failed");
		return status;
	}
	/**
	 *  if already in DBS no need to request DBS. Might be needed
	 *  to extend the logic when multiple dbs HW mode is available
	 */
	if ((((PM_DBS_DOWNGRADE == action) || (PM_DBS == action) ||
		(PM_DBS_UPGRADE == action))
		&& hw_mode.dbs_cap)) {
		policy_mgr_err("driver is already in %s mode, no further action needed",
				(hw_mode.dbs_cap) ? "dbs" : "non dbs");
		return QDF_STATUS_E_ALREADY;
	}

	if ((PM_SBS == action) || (action == PM_SBS_DOWNGRADE)) {
		if (!policy_mgr_is_hw_sbs_capable(psoc)) {
			/* No action */
			policy_mgr_notice("firmware is not sbs capable");
			return QDF_STATUS_E_NOSUPPORT;
		}
		/* check if current mode is already SBS nothing to be
		 * done
		 */

	}

	switch (action) {
	case PM_DBS_DOWNGRADE:
		/*
		* check if we have a beaconing entity that is using 2x2. If yes,
		* update the beacon template & notify FW. Once FW confirms
		*  beacon updated, send down the HW mode change req
		*/
		status = policy_mgr_complete_action(psoc, POLICY_MGR_RX_NSS_1,
					PM_DBS, reason, session_id);
		break;
	case PM_DBS:
		(void)policy_mgr_get_hw_dbs_nss(psoc, &nss_dbs);

		status = policy_mgr_pdev_set_hw_mode(psoc, session_id,
						     nss_dbs.mac0_ss,
						     HW_MODE_80_MHZ,
						     nss_dbs.mac1_ss,
						     HW_MODE_40_MHZ,
						     HW_MODE_DBS,
						     HW_MODE_AGILE_DFS_NONE,
						     HW_MODE_SBS_NONE,
						     reason, PM_NOP);
		break;
	case PM_SINGLE_MAC_UPGRADE:
		/*
		 * change the HW mode first before the NSS upgrade
		 */
		status = policy_mgr_pdev_set_hw_mode(psoc, session_id,
						HW_MODE_SS_2x2,
						HW_MODE_80_MHZ,
						HW_MODE_SS_0x0, HW_MODE_BW_NONE,
						HW_MODE_DBS_NONE,
						HW_MODE_AGILE_DFS_NONE,
						HW_MODE_SBS_NONE,
						reason, PM_UPGRADE);
		break;
	case PM_SINGLE_MAC:
		status = policy_mgr_pdev_set_hw_mode(psoc, session_id,
						HW_MODE_SS_2x2,
						HW_MODE_80_MHZ,
						HW_MODE_SS_0x0, HW_MODE_BW_NONE,
						HW_MODE_DBS_NONE,
						HW_MODE_AGILE_DFS_NONE,
						HW_MODE_SBS_NONE,
						reason, PM_NOP);
		break;
	case PM_DBS_UPGRADE:
		status = policy_mgr_pdev_set_hw_mode(psoc, session_id,
						HW_MODE_SS_2x2,
						HW_MODE_80_MHZ,
						HW_MODE_SS_2x2, HW_MODE_80_MHZ,
						HW_MODE_DBS,
						HW_MODE_AGILE_DFS_NONE,
						HW_MODE_SBS_NONE,
						reason, PM_UPGRADE);
		break;
	case PM_SBS_DOWNGRADE:
		status = policy_mgr_complete_action(psoc, POLICY_MGR_RX_NSS_1,
					PM_SBS, reason, session_id);
		break;
	case PM_SBS:
		status = policy_mgr_pdev_set_hw_mode(psoc, session_id,
						HW_MODE_SS_1x1,
						HW_MODE_80_MHZ,
						HW_MODE_SS_1x1, HW_MODE_80_MHZ,
						HW_MODE_DBS,
						HW_MODE_AGILE_DFS_NONE,
						HW_MODE_SBS,
						reason, PM_NOP);
		break;
	case PM_DOWNGRADE:
		/*
		 * check if we have a beaconing entity that advertised 2x2
		 * intially. If yes, update the beacon template & notify FW.
		 */
		status = policy_mgr_nss_update(psoc, POLICY_MGR_RX_NSS_1,
					PM_NOP, reason);
		break;
	case PM_UPGRADE:
		/*
		 * check if we have a beaconing entity that advertised 2x2
		 * intially. If yes, update the beacon template & notify FW.
		 */
		status = policy_mgr_nss_update(psoc, POLICY_MGR_RX_NSS_2,
					PM_NOP, reason);
		break;
	default:
		policy_mgr_err("unexpected action value %d", action);
		status = QDF_STATUS_E_FAILURE;
		break;
	}

	return status;
}

QDF_STATUS policy_mgr_handle_conc_multiport(struct wlan_objmgr_psoc *psoc,
		uint8_t session_id, uint8_t channel)
{
	QDF_STATUS status;

	if (!policy_mgr_check_for_session_conc(psoc, session_id, channel)) {
		policy_mgr_err("Conc not allowed for the session %d",
			session_id);
		return QDF_STATUS_E_FAILURE;
	}

	status = policy_mgr_reset_connection_update(psoc);
	if (!QDF_IS_STATUS_SUCCESS(status))
		policy_mgr_err("clearing event failed");

	status = policy_mgr_current_connections_update(psoc, session_id,
			channel,
			POLICY_MGR_UPDATE_REASON_NORMAL_STA);
	if (QDF_STATUS_E_FAILURE == status) {
		policy_mgr_err("connections update failed");
		return status;
	}

	return status;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * policy_mgr_is_restart_sap_allowed() - Check if restart SAP
 * allowed during SCC -> MCC switch
 * @psoc: PSOC object data
 * @mcc_to_scc_switch: MCC to SCC switch enabled user config
 *
 * Check if restart SAP allowed during SCC->MCC switch
 *
 * Restart: true or false
 */
static bool policy_mgr_is_restart_sap_allowed(
	struct wlan_objmgr_psoc *psoc,
	uint32_t mcc_to_scc_switch)
{
	uint32_t sta_ap_bit_mask = QDF_STA_MASK | QDF_SAP_MASK;
	uint32_t sta_go_bit_mask = QDF_STA_MASK | QDF_P2P_GO_MASK;

	if ((mcc_to_scc_switch == QDF_MCC_TO_SCC_SWITCH_DISABLE) ||
		!policy_mgr_concurrent_open_sessions_running(psoc) ||
		!(((policy_mgr_get_concurrency_mode(psoc) & sta_ap_bit_mask)
			== sta_ap_bit_mask) ||
		((policy_mgr_get_concurrency_mode(psoc) & sta_go_bit_mask)
			== sta_go_bit_mask))) {
		policy_mgr_err("MCC switch disabled or not concurrent STA/SAP, STA/GO");
		return false;
	}

	return true;
}

bool policy_mgr_is_safe_channel(struct wlan_objmgr_psoc *psoc,
		uint8_t channel)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	bool is_safe = true;
	uint8_t j;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return is_safe;
	}


	if (pm_ctx->unsafe_channel_count == 0) {
		policy_mgr_debug("There are no unsafe channels");
		return is_safe;
	}

	for (j = 0; j < pm_ctx->unsafe_channel_count; j++) {
		if (channel == pm_ctx->unsafe_channel_list[j]) {
			is_safe = false;
			policy_mgr_warn("CH %d is not safe", channel);
			break;
		}
	}

	return is_safe;
}

/**
 * policy_mgr_check_sta_ap_concurrent_ch_intf() - Restart SAP in STA-AP case
 * @data: Pointer check concurrent channel work data
 *
 * Restarts the SAP interface in STA-AP concurrency scenario
 *
 * Restart: None
 */
void policy_mgr_check_sta_ap_concurrent_ch_intf(void *data)
{
	struct wlan_objmgr_psoc *psoc;
	struct policy_mgr_psoc_priv_obj *pm_ctx = NULL;
	struct sta_ap_intf_check_work_ctx *work_info = NULL;
	uint32_t mcc_to_scc_switch, cc_count = 0, i;
	QDF_STATUS status;
	uint8_t channel, sec_ch;
	uint8_t operating_channel[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint8_t vdev_id[MAX_NUMBER_OF_CONC_CONNECTIONS];

	work_info = (struct sta_ap_intf_check_work_ctx *) data;
	if (!work_info) {
		policy_mgr_err("Invalid work_info");
		goto end;
	}

	psoc = work_info->psoc;
	if (!psoc) {
		policy_mgr_err("Invalid psoc");
		goto end;
	}

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		goto end;
	}
	mcc_to_scc_switch =
		policy_mgr_mcc_to_scc_switch_mode_in_user_cfg(psoc);

	policy_mgr_info("Concurrent open sessions running: %d",
		policy_mgr_concurrent_open_sessions_running(psoc));

	if (!policy_mgr_is_restart_sap_allowed(psoc, mcc_to_scc_switch))
		goto end;

	cc_count = policy_mgr_get_mode_specific_conn_info(psoc,
					&operating_channel[cc_count],
					&vdev_id[cc_count],
					PM_SAP_MODE);
	policy_mgr_debug("Number of concurrent SAP: %d", cc_count);
	cc_count = cc_count + policy_mgr_get_mode_specific_conn_info(psoc,
						&operating_channel[cc_count],
						&vdev_id[cc_count],
						PM_P2P_GO_MODE);
	policy_mgr_debug("Number of beaconing entities (SAP + GO):%d",
							cc_count);
	if (!cc_count) {
		policy_mgr_err("Could not retrieve SAP/GO operating channel&vdevid");
		goto end;
	}

	if (!pm_ctx->hdd_cbacks.wlan_hdd_get_channel_for_sap_restart) {
		policy_mgr_err("SAP restart get channel callback in NULL");
		goto end;
	}
	for (i = 0; i < cc_count; i++) {
		status = pm_ctx->hdd_cbacks.
			wlan_hdd_get_channel_for_sap_restart(psoc,
					vdev_id[i], &channel, &sec_ch);
		if (status == QDF_STATUS_SUCCESS) {
			policy_mgr_info("SAP restarts due to MCC->SCC switch, old chan :%d new chan: %d"
					, operating_channel[i], channel);
			break;
		}
	}
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("Failed to switch SAP channel");
end:
	if (work_info) {
		qdf_mem_free(work_info);
		if (pm_ctx)
			pm_ctx->sta_ap_intf_check_work_info = NULL;
	}
}

static bool policy_mgr_valid_sta_channel_check(struct wlan_objmgr_psoc *psoc,
		uint8_t sta_channel)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return false;
	}

	if ((wlan_reg_is_dfs_ch(pm_ctx->pdev, sta_channel) &&
		(!policy_mgr_is_sta_sap_scc_allowed_on_dfs_chan(psoc))) ||
		wlan_reg_is_passive_or_disable_ch(pm_ctx->pdev, sta_channel) ||
		!policy_mgr_is_safe_channel(psoc, sta_channel)) {
		if (policy_mgr_is_hw_dbs_capable(psoc))
			return true;
		else
			return false;
	}
	else
		return true;
}

QDF_STATUS policy_mgr_valid_sap_conc_channel_check(
	struct wlan_objmgr_psoc *psoc, uint8_t *con_ch, uint8_t sap_ch)
{
	uint8_t channel = *con_ch;
	uint8_t temp_channel = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * if force SCC is set, Check if conc channel is DFS
	 * or passive or part of LTE avoided channel list.
	 * In that case move SAP to other band if DBS is supported,
	 * return otherwise
	 */
	if (!policy_mgr_is_force_scc(psoc))
		return QDF_STATUS_SUCCESS;

	/*
	 * if interference is 0, check if it is DBS case. If DBS case
	 * return from here. If SCC, check further if SAP can move to
	 * STA's channel.
	 */
	if (!channel &&
		(sap_ch != policy_mgr_mode_specific_get_channel(
			psoc, PM_STA_MODE)))
		return QDF_STATUS_SUCCESS;
	else if (!channel)
		channel = sap_ch;

	if (policy_mgr_valid_sta_channel_check(psoc, channel)) {
		if (wlan_reg_is_dfs_ch(pm_ctx->pdev, channel) ||
			wlan_reg_is_passive_or_disable_ch(
				pm_ctx->pdev, channel) ||
			!policy_mgr_is_safe_channel(psoc, channel)) {
			if (policy_mgr_is_hw_dbs_capable(psoc)) {
				temp_channel =
				policy_mgr_get_alternate_channel_for_sap(psoc);
				policy_mgr_debug("temp_channel is %d",
					temp_channel);
				if (temp_channel) {
					channel = temp_channel;
				} else {
					if (WLAN_REG_IS_5GHZ_CH(channel))
						channel = PM_24_GHZ_CHANNEL_6;
					else
						channel = PM_5_GHZ_CHANNEL_36;
				}
				if (!policy_mgr_is_safe_channel(
					psoc, channel)) {
					policy_mgr_warn(
						"Can't have concurrency on %d as it is not safe",
						channel);
					return QDF_STATUS_E_FAILURE;
				}
			} else {
				policy_mgr_warn("Can't have concurrency on %d",
					channel);
				return QDF_STATUS_E_FAILURE;
			}
		}
	}

	if (channel != sap_ch)
		*con_ch = channel;

	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_check_concurrent_intf_and_restart_sap() - Check
 * concurrent change intf
 * @psoc: PSOC object information
 * @operation_channel: operation channel
 * @vdev_id: vdev id of SAP
 *
 * Checks the concurrent change interface and restarts SAP
 *
 * Return: None
 */
void policy_mgr_check_concurrent_intf_and_restart_sap(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t mcc_to_scc_switch;
	uint8_t operating_channel, vdev_id;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return;
	}

	if (policy_mgr_get_mode_specific_conn_info(psoc, &operating_channel,
						&vdev_id, PM_STA_MODE)) {
		policy_mgr_debug("STA operating Channel: %u",
				operating_channel);
	} else {
		policy_mgr_err("Could not get STA operating channel&vdevid");
		return;
	}

	mcc_to_scc_switch =
		policy_mgr_mcc_to_scc_switch_mode_in_user_cfg(psoc);
	policy_mgr_info("MCC to SCC switch: %d chan: %d",
			mcc_to_scc_switch, operating_channel);

	if (!policy_mgr_is_restart_sap_allowed(psoc, mcc_to_scc_switch)) {
		policy_mgr_debug(
			"No action taken at check_concurrent_intf_and_restart_sap");
		return;
	}

	if ((mcc_to_scc_switch != QDF_MCC_TO_SCC_SWITCH_DISABLE) &&
		policy_mgr_valid_sta_channel_check(psoc, operating_channel)
		&& !pm_ctx->sta_ap_intf_check_work_info) {
		struct sta_ap_intf_check_work_ctx *work_info;
		work_info = qdf_mem_malloc(
			sizeof(struct sta_ap_intf_check_work_ctx));
		pm_ctx->sta_ap_intf_check_work_info = work_info;
		if (work_info) {
			work_info->psoc = psoc;
			qdf_create_work(0, &pm_ctx->sta_ap_intf_check_work,
				policy_mgr_check_sta_ap_concurrent_ch_intf,
				work_info);
			qdf_sched_work(0, &pm_ctx->sta_ap_intf_check_work);
			policy_mgr_info(
				"Checking for Concurrent Change interference");
		}
	}
}
#endif /* FEATURE_WLAN_MCC_TO_SCC_SWITCH */

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
/**
 * policy_mgr_change_sap_channel_with_csa() - Move SAP channel using (E)CSA
 * @psoc: PSOC object information
 * @vdev_id: Vdev id
 * @channel: Channel to change
 * @ch_width: channel width to change
 * @forced: Force to switch channel, ignore SCC/MCC check
 *
 * Invoke the callback function to change SAP channel using (E)CSA
 *
 * Return: None
 */
void policy_mgr_change_sap_channel_with_csa(struct wlan_objmgr_psoc *psoc,
					    uint8_t vdev_id, uint32_t channel,
					    uint32_t ch_width,
					    bool forced)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return;
	}

	if (pm_ctx->hdd_cbacks.sap_restart_chan_switch_cb) {
		policy_mgr_info("SAP change change without restart");
		pm_ctx->hdd_cbacks.sap_restart_chan_switch_cb(psoc,
				vdev_id, channel, ch_width, forced);
	}
}
#endif

QDF_STATUS policy_mgr_wait_for_connection_update(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *policy_mgr_context;

	policy_mgr_context = policy_mgr_get_context(psoc);
	if (!policy_mgr_context) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_wait_single_event(
			&policy_mgr_context->connection_update_done_evt,
			CONNECTION_UPDATE_TIMEOUT);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("wait for event failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_reset_connection_update(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *policy_mgr_context;

	policy_mgr_context = policy_mgr_get_context(psoc);
	if (!policy_mgr_context) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_event_reset(
		&policy_mgr_context->connection_update_done_evt);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("clear event failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_set_connection_update(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *policy_mgr_context;

	policy_mgr_context = policy_mgr_get_context(psoc);
	if (!policy_mgr_context) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_event_set(&policy_mgr_context->connection_update_done_evt);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("set event failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_set_opportunistic_update(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *policy_mgr_context;

	policy_mgr_context = policy_mgr_get_context(psoc);
	if (!policy_mgr_context) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_event_set(
			&policy_mgr_context->opportunistic_update_done_evt);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("set event failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_restart_opportunistic_timer(
		struct wlan_objmgr_psoc *psoc, bool check_state)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct policy_mgr_psoc_priv_obj *policy_mgr_ctx;

	policy_mgr_ctx = policy_mgr_get_context(psoc);
	if (!policy_mgr_ctx) {
		policy_mgr_err("Invalid context");
		return status;
	}

	if (check_state &&
			QDF_TIMER_STATE_RUNNING !=
			policy_mgr_ctx->dbs_opportunistic_timer.state)
		return status;

	qdf_mc_timer_stop(&policy_mgr_ctx->dbs_opportunistic_timer);

	status = qdf_mc_timer_start(
			&policy_mgr_ctx->dbs_opportunistic_timer,
			DBS_OPPORTUNISTIC_TIME * 1000);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("failed to start opportunistic timer");
		return status;
	}

	return status;
}

QDF_STATUS policy_mgr_set_hw_mode_on_channel_switch(
			struct wlan_objmgr_psoc *psoc, uint8_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE, qdf_status;
	enum policy_mgr_conc_next_action action;

	if (!policy_mgr_is_hw_dbs_capable(psoc)) {
		policy_mgr_err("PM/DBS is disabled");
		return status;
	}

	action = (*policy_mgr_get_current_pref_hw_mode_ptr)(psoc);
	if ((action != PM_DBS_DOWNGRADE) &&
	    (action != PM_SINGLE_MAC_UPGRADE)) {
		policy_mgr_err("Invalid action: %d", action);
		status = QDF_STATUS_SUCCESS;
		goto done;
	}

	policy_mgr_debug("action:%d session id:%d", action, session_id);

	/* Opportunistic timer is started, PM will check if MCC upgrade can be
	 * done on timer expiry. This avoids any possible ping pong effect
	 * as well.
	 */
	if (action == PM_SINGLE_MAC_UPGRADE) {
		qdf_status = policy_mgr_restart_opportunistic_timer(
			psoc, false);
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			policy_mgr_debug("opportunistic timer for MCC upgrade");
		goto done;
	}

	/* For DBS, we want to move right away to DBS mode */
	status = policy_mgr_next_actions(psoc, session_id, action,
			POLICY_MGR_UPDATE_REASON_CHANNEL_SWITCH);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("no set hw mode command was issued");
		goto done;
	}
done:
	/* success must be returned only when a set hw mode was done */
	return status;
}

void policy_mgr_checkn_update_hw_mode_single_mac_mode(
		struct wlan_objmgr_psoc *psoc, uint8_t channel)
{
	uint8_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		if (pm_conc_connection_list[i].in_use)
			if (!WLAN_REG_IS_SAME_BAND_CHANNELS(channel,
				pm_conc_connection_list[i].chan)) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				policy_mgr_debug("DBS required");
				return;
			}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	if (QDF_TIMER_STATE_RUNNING ==
		pm_ctx->dbs_opportunistic_timer.state)
		qdf_mc_timer_stop(&pm_ctx->dbs_opportunistic_timer);

	pm_dbs_opportunistic_timer_handler((void *)psoc);
}

void policy_mgr_check_and_stop_opportunistic_timer(
	struct wlan_objmgr_psoc *psoc, uint8_t id)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	enum policy_mgr_conc_next_action action = PM_NOP;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	if (QDF_TIMER_STATE_RUNNING ==
		pm_ctx->dbs_opportunistic_timer.state) {
		qdf_mc_timer_stop(&pm_ctx->dbs_opportunistic_timer);
		action = policy_mgr_need_opportunistic_upgrade(psoc);
		if (action) {
			qdf_event_reset(&pm_ctx->opportunistic_update_done_evt);
			status = policy_mgr_next_actions(psoc, id, action,
				POLICY_MGR_UPDATE_REASON_OPPORTUNISTIC);
			if (status != QDF_STATUS_SUCCESS) {
				policy_mgr_err("Failed in policy_mgr_next_actions");
				return;
			}
			status = qdf_wait_single_event(
					&pm_ctx->opportunistic_update_done_evt,
					CONNECTION_UPDATE_TIMEOUT);

			if (!QDF_IS_STATUS_SUCCESS(status)) {
				policy_mgr_err("wait for event failed");
				return;
			}
		}
	}
}

void policy_mgr_set_hw_mode_change_in_progress(
	struct wlan_objmgr_psoc *psoc, enum policy_mgr_hw_mode_change value)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	pm_ctx->hw_mode_change_in_progress = value;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	policy_mgr_debug("hw_mode_change_in_progress:%d", value);
}

enum policy_mgr_hw_mode_change policy_mgr_is_hw_mode_change_in_progress(
	struct wlan_objmgr_psoc *psoc)
{
	enum policy_mgr_hw_mode_change value;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	value = POLICY_MGR_HW_MODE_NOT_IN_PROGRESS;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return value;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	value = pm_ctx->hw_mode_change_in_progress;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return value;
}

enum policy_mgr_hw_mode_change policy_mgr_get_hw_mode_change_from_hw_mode_index(
	struct wlan_objmgr_psoc *psoc, uint32_t hw_mode_index)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t param = 0;
	enum policy_mgr_hw_mode_change value
		= POLICY_MGR_HW_MODE_NOT_IN_PROGRESS;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return value;
	}

	policy_mgr_info("HW param: %x", param);
	param = pm_ctx->hw_mode.hw_mode_list[hw_mode_index];
	if (POLICY_MGR_HW_MODE_DBS_MODE_GET(param)) {
		policy_mgr_info("DBS is requested with HW (%d)",
		hw_mode_index);
		value = POLICY_MGR_DBS_IN_PROGRESS;
		goto ret_value;
	}

	if (POLICY_MGR_HW_MODE_SBS_MODE_GET(param)) {
		policy_mgr_info("SBS is requested with HW (%d)",
		hw_mode_index);
		value = POLICY_MGR_SBS_IN_PROGRESS;
		goto ret_value;
	}

	value = POLICY_MGR_SMM_IN_PROGRESS;
	policy_mgr_info("SMM is requested with HW (%d)", hw_mode_index);

ret_value:
	return value;
}

#ifdef MPC_UT_FRAMEWORK
QDF_STATUS policy_mgr_update_connection_info_utfw(
		struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index = 0, found = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if (vdev_id == pm_conc_connection_list[conn_index].vdev_id) {
			/* debug msg */
			found = 1;
			break;
		}
		conn_index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	if (!found) {
		/* err msg */
		policy_mgr_err("can't find vdev_id %d in pm_conc_connection_list",
			vdev_id);
		return status;
	}
	policy_mgr_debug("--> updating entry at index[%d]", conn_index);

	policy_mgr_update_conc_list(psoc, conn_index,
			policy_mgr_get_mode(type, sub_type),
			channelid, HW_MODE_20_MHZ,
			mac_id, chain_mask, 0, vdev_id, true);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_incr_connection_count_utfw(struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index = 0;

	conn_index = policy_mgr_get_connection_count(psoc);
	if (MAX_NUMBER_OF_CONC_CONNECTIONS <= conn_index) {
		/* err msg */
		policy_mgr_err("exceeded max connection limit %d",
			MAX_NUMBER_OF_CONC_CONNECTIONS);
		return status;
	}
	policy_mgr_debug("--> filling entry at index[%d]", conn_index);

	policy_mgr_update_conc_list(psoc, conn_index,
				policy_mgr_get_mode(type, sub_type),
				channelid, HW_MODE_20_MHZ,
				mac_id, chain_mask, 0, vdev_id, true);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_decr_connection_count_utfw(struct wlan_objmgr_psoc *psoc,
		uint32_t del_all, uint32_t vdev_id)
{
	QDF_STATUS status;

	if (del_all) {
		status = policy_mgr_psoc_enable(psoc);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			policy_mgr_err("Policy manager initialization failed");
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		policy_mgr_decr_connection_count(psoc, vdev_id);
	}

	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_first_conn_table(
		enum policy_mgr_con_mode type,
		enum policy_mgr_conc_priority_mode sys_pref)
{
	if ((sys_pref >= PM_MAX_CONC_PRIORITY_MODE) ||
		(type >= PM_MAX_NUM_OF_MODE))
		return PM_MAX_PCL_TYPE;
	return first_connection_pcl_table[type][sys_pref];
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_second_conn_table(
	enum policy_mgr_one_connection_mode idx, enum policy_mgr_con_mode type,
	enum policy_mgr_conc_priority_mode sys_pref, uint8_t dbs_capable)
{
	if ((idx >= PM_MAX_ONE_CONNECTION_MODE) ||
		(sys_pref >= PM_MAX_CONC_PRIORITY_MODE) ||
		(type >= PM_MAX_NUM_OF_MODE))
		return PM_MAX_PCL_TYPE;
	if (dbs_capable)
		return (*second_connection_pcl_dbs_table)[idx][type][sys_pref];
	else
		return second_connection_pcl_nodbs_table[idx][type][sys_pref];
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_third_conn_table(
	enum policy_mgr_two_connection_mode idx, enum policy_mgr_con_mode type,
	enum policy_mgr_conc_priority_mode sys_pref, uint8_t dbs_capable)
{
	if ((idx >= PM_MAX_TWO_CONNECTION_MODE) ||
		(sys_pref >= PM_MAX_CONC_PRIORITY_MODE) ||
		(type >= PM_MAX_NUM_OF_MODE))
		return PM_MAX_PCL_TYPE;
	if (dbs_capable)
		return (*third_connection_pcl_dbs_table)[idx][type][sys_pref];
	else
		return third_connection_pcl_nodbs_table[idx][type][sys_pref];
}
#endif
