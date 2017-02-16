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
	return QDF_STATUS_SUCCESS;
}

void policy_mgr_decr_session_set_pcl(struct wlan_objmgr_psoc *psoc,
						enum tQDF_ADAPTER_MODE mode,
						uint8_t session_id)
{
}

void policy_mgr_update_with_safe_channel_list(uint8_t *pcl_channels,
		uint32_t *len, uint8_t *weight_list, uint32_t weight_len)
{
	return;
}

uint8_t policy_mgr_get_channel(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_con_mode mode, uint32_t *vdev_id)
{
	return 0;
}

QDF_STATUS policy_mgr_get_pcl(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_con_mode mode,
			uint8_t *pcl_channels, uint32_t *len,
			uint8_t *pcl_weight, uint32_t weight_len)
{
	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_conc_priority_mode
		policy_mgr_get_first_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	return PM_MAX_CONC_PRIORITY_MODE;
}

enum policy_mgr_one_connection_mode
		policy_mgr_get_second_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	return PM_MAX_ONE_CONNECTION_MODE;
}

enum policy_mgr_two_connection_mode
		policy_mgr_get_third_connection_pcl_table_index(
		struct wlan_objmgr_psoc *psoc)
{
	return PM_MAX_TWO_CONNECTION_MODE;
}

uint8_t
policy_mgr_get_nondfs_preferred_channel(struct wlan_objmgr_psoc *psoc,
		enum policy_mgr_con_mode mode,
		bool for_existing_conn)
{
	return 0;
}

QDF_STATUS policy_mgr_get_valid_chans(struct wlan_objmgr_psoc *psoc,
				uint8_t *chan_list, uint32_t *list_len)
{
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_list_has_24GHz_channel(uint8_t *channel_list,
					uint32_t list_len)
{
	return false;
}

QDF_STATUS policy_mgr_set_sap_mandatory_channels(struct wlan_objmgr_psoc *psoc,
					uint8_t *channels, uint32_t len)
{
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_is_sap_mandatory_channel_set(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

QDF_STATUS policy_mgr_modify_sap_pcl_based_on_mandatory_channel(
		struct wlan_objmgr_psoc *psoc,
		uint8_t *pcl_list_org,
		uint8_t *weight_list_org,
		uint32_t *pcl_len_org)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_sap_mandatory_channel(struct wlan_objmgr_psoc *psoc,
		uint32_t *chan)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_valid_chan_weights(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_pcl_chan_weights *weight)
{
	return QDF_STATUS_SUCCESS;
}
