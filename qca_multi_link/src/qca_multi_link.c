/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "qca_multi_link.h"

static bool is_initialized;
qca_multi_link_parameters_t qca_multi_link_cfg;

static inline qca_multi_link_radio_node_t *
		qca_multi_link_find_radio_node(struct wiphy *wiphy)
{
	qdf_list_node_t *node = NULL, *next_node = NULL;

	if (!wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_WARN,
			 FL(" Radio wiphy pointer is NULL\n"));
		return NULL;
	}

	if (qdf_list_empty(&qca_multi_link_cfg.radio_list)) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			 FL(" Radio list is empty\n"));
		return NULL;
	}

	qdf_list_peek_front(&qca_multi_link_cfg.radio_list,
			   (qdf_list_node_t **)&next_node);
	while (next_node) {
		qca_multi_link_radio_node_t *radio_node
		= (qca_multi_link_radio_node_t *)next_node;
		if (radio_node->wiphy == wiphy) {
			return radio_node;
		} else {
			node = next_node;
			next_node = NULL;
			if ((qdf_list_peek_next(&qca_multi_link_cfg.radio_list,
					       node, (qdf_list_node_t **)&next_node))
						!= QDF_STATUS_SUCCESS) {
				return NULL;
			}
		}
	}
	return NULL;
}

static inline bool is_fast_lane_radio(struct wiphy *fl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(fl_wiphy);
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	if (!radio_node) {
		return false;
	}

	return radio_node->is_fast_lane;
}

static inline bool is_no_backhaul_radio(struct wiphy *no_bl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(no_bl_wiphy);
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	if (!radio_node) {
		return false;
	}

	return radio_node->no_backhaul;
}

static inline bool is_other_fast_lane_radio_primary(struct wiphy *fl_wiphy)
{
	qdf_list_node_t *next_node = NULL;

	if (!fl_wiphy) {
		return false;
	}

	if (qdf_list_empty(&qca_multi_link_cfg.radio_list)) {
		return false;
	}

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	qdf_list_peek_front(&qca_multi_link_cfg.radio_list,
			   (qdf_list_node_t **)&next_node);
	while (next_node) {
		qca_multi_link_radio_node_t *radio_node
		= (qca_multi_link_radio_node_t *)next_node;
		if ((radio_node->wiphy != fl_wiphy) &&
		   (radio_node->is_fast_lane) &&
		   (radio_node->wiphy  == qca_multi_link_cfg.primary_wiphy)) {
			qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
			return true;
		}
	}
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return false;
}

/**
 * qca_multi_link_is_primary_radio() - Check if this is a primary radio
 *
 * Return: true: if it primary radio
 *	   false: if it is secondary radio
 */
static inline bool qca_multi_link_is_primary_radio(struct wiphy *dev_wiphy)
{
	bool is_primary = false;

	if (!qca_multi_link_cfg.primary_wiphy || !dev_wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("\nprimary_wiphy is NULL\n"));
		is_primary = false;
	} else {
		is_primary = (dev_wiphy == qca_multi_link_cfg.primary_wiphy)?true:false;
	}

	if ((is_primary == false) && is_fast_lane_radio(dev_wiphy)) {
		is_primary = is_other_fast_lane_radio_primary(dev_wiphy);
	}

	return is_primary;
}

/**
 * qca_multi_link_need_procesing() - Check if repeater processing is required
 *
 * Return: true: processing is required
 *	   false: processing is not required
 */
static inline bool qca_multi_link_need_procesing(void)
{
	if ((!qca_multi_link_cfg.rptr_processing_enable)
		|| (qca_multi_link_cfg.total_stavaps_up < 2)) {
		return false;
	}
	return true;
}

/**
 * qca_multi_link_pktfrom_ownsrc() - Check if packet is from same device
 *
 * Return: true: packet is from same device
 *	   false:  packet is not from same device
 */
static inline bool qca_multi_link_pktfrom_ownsrc(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	if (qdf_is_macaddr_equal((struct qdf_mac_addr *)net_dev->dev_addr,
		(struct qdf_mac_addr *)eh->ether_shost)) {
		return true;
	}
	return false;
}

/**
 * qca_multi_link_drop_secondary_mcast() - Check if mcast to be dropped on secondary
 *
 * Return: true: Drop the packet
 *	   false: Do not drop
 */
static inline bool qca_multi_link_drop_secondary_mcast(qdf_nbuf_t nbuf)
{
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);
	uint8_t is_mcast = IEEE80211_IS_MULTICAST(eh->ether_dhost);

	if (is_mcast && qca_multi_link_cfg.drop_secondary_mcast) {
		return true;
	}
	return false;
}

/**
 * qca_multi_link_drop_always_primary() - Check if packet to be dropped for always_primary
 *
 * Return: true: Drop the packet
 *	   false: Do not drop
 */
static inline bool qca_multi_link_drop_always_primary(bool is_primary, qdf_nbuf_t nbuf)
{
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	if (qca_multi_link_cfg.always_primary) {
		if (is_primary) {
			return false;
		} else {
			if (eh->ether_type != qdf_htons(ETHERTYPE_PAE)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * qca_multi_link_get_station_vap() - get the radio station vap pointer for a radio
 * @primary_wiphy: wiphy pointer of radio device
 *
 * Return: station vap netdevice pointer
 */
static struct net_device *qca_multi_link_get_station_vap(struct wiphy *wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(wiphy);
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	if (!radio_node || !radio_node->sta_dev) {
		return NULL;
	}

	return radio_node->sta_dev;
}

/**
 * qca_multi_link_deinit_module() - De-initialize the repeater base structute
 * Return: void
 */
void qca_multi_link_deinit_module(void)
{
	if (!is_initialized)
		return;

	qca_multi_link_cfg.total_stavaps_up = 0;
	qca_multi_link_cfg.loop_detected = 0;
	qca_multi_link_cfg.primary_wiphy = NULL;
	qdf_list_destroy(&qca_multi_link_cfg.radio_list);
	is_initialized = false;
	qdf_spinlock_destroy(&qca_multi_link_cfg.radio_lock);

	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_INFO,
		FL("\n******QCA RPtr De-Init Done***********\n"));
}

qdf_export_symbol(qca_multi_link_deinit_module);

/**
 * qca_multi_link_init_module() - Initialize the repeater base structute
 *
 * Return: void
 */
void qca_multi_link_init_module(void)
{
	if (is_initialized)
		return;

	is_initialized = true;
	qca_multi_link_cfg.total_stavaps_up = 0;
	qca_multi_link_cfg.loop_detected = 0;
	qca_multi_link_cfg.primary_wiphy = NULL;
	qdf_list_create(&qca_multi_link_cfg.radio_list, QCA_MULTI_LINK_RADIO_LIST_SIZE);
	qdf_spinlock_create(&qca_multi_link_cfg.radio_lock);

	memset(&qca_multi_link_cfg.qca_ml_stats, 0x0, sizeof(qca_multi_link_radio_node_t));
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_INFO,
		FL("\n******QCA Repeater Initialization Done***********\n"));
}

qdf_export_symbol(qca_multi_link_init_module);

/**
 * qca_multi_link_get_num_sta() - Get the total number of sta vaps up.
 *
 * Return: int
 */
uint8_t qca_multi_link_get_num_sta(void)
{
	return qca_multi_link_cfg.total_stavaps_up;
}

qdf_export_symbol(qca_multi_link_get_num_sta);

/**
 * qca_multi_link_append_num_sta() - Append the total number of sta vaps up.
 * @inc_or_dec: true to increment and false to decrement
 *
 * Return: void
 */
void qca_multi_link_append_num_sta(bool inc_or_dec)
{
	if (inc_or_dec) {
		qca_multi_link_cfg.total_stavaps_up++;
		if (qca_multi_link_cfg.total_stavaps_up > 1) {
			qca_multi_link_set_drop_sec_mcast(true);
		}
	} else {
		if (qca_multi_link_cfg.total_stavaps_up == 0) {
			return;
		}

		qca_multi_link_cfg.total_stavaps_up--;
		if (qca_multi_link_cfg.total_stavaps_up <= 1) {
			qca_multi_link_cfg.loop_detected = 0;
		}
	}
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_NONE,
		FL("\nStation vap number in Repeater is val=%d***********\n"),
		qca_multi_link_cfg.total_stavaps_up);
}

qdf_export_symbol(qca_multi_link_append_num_sta);

/**
 * qca_multi_link_is_dbdc_processing_reqd() - Check if dbdc processing is required
 * @net_dev: current net device
 *
 * Return: true: loop is detected
 *	   false: loop not detected
 */
bool qca_multi_link_is_dbdc_processing_reqd(struct net_device *net_dev)
{
	if (qca_multi_link_cfg.total_stavaps_up > 2)
		return (qca_multi_link_cfg.loop_detected &&
		       qca_multi_link_cfg.rptr_processing_enable);
	else
		return false;
}

qdf_export_symbol(qca_multi_link_is_dbdc_processing_reqd);

/**
 * qca_multi_link_set_drop_sec_mcast() - set the drop secondary mcast flag
 * @val: boolean true or false
 *
 */
void qca_multi_link_set_drop_sec_mcast(bool val)
{
	qca_multi_link_cfg.drop_secondary_mcast = val;
}

qdf_export_symbol(qca_multi_link_set_drop_sec_mcast);

/**
 * qca_multi_link_set_force_client_mcast() - set the flag to force client mcast traffic
 * @val: boolean true or false
 *
 */
void qca_multi_link_set_force_client_mcast(bool val)
{
	qca_multi_link_cfg.force_client_mcast_traffic = val;
}

qdf_export_symbol(qca_multi_link_set_force_client_mcast);

/**
 * qca_multi_link_set_always_primary() - set the flag for always primary flag
 * @val: boolean true or false
 *
 */
void qca_multi_link_set_always_primary(bool val)
{
	qca_multi_link_cfg.always_primary = val;
}

qdf_export_symbol(qca_multi_link_set_always_primary);

/**
 * qca_multi_link_set_dbdc_enable() - set the dbdc enable flag
 * @val: boolean true or false
 */
void qca_multi_link_set_dbdc_enable(bool val)
{
	qca_multi_link_cfg.rptr_processing_enable = val;
	if (!val) {
		qca_multi_link_cfg.loop_detected = 0;
	}
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
		FL("\nSetting DBDC enable = val%d\n"), val);
}

qdf_export_symbol(qca_multi_link_set_dbdc_enable);

/**
 * qca_multi_link_get_primary_radio() - set the dbdc enable flag
 * @primary_wiphy: wiphy pointer of primary radio device
 */
struct wiphy *qca_multi_link_get_primary_radio(void)
{
	return qca_multi_link_cfg.primary_wiphy;
}

qdf_export_symbol(qca_multi_link_get_primary_radio);

/**
 * qca_multi_link_set_primary_radio() - set the primary radio
 * @primary_wiphy: wiphy pointer of primary radio device
 */
void qca_multi_link_set_primary_radio(struct wiphy *primary_wiphy)
{
	if (!primary_wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("\nNull wiphy in Setting primary radio\n"));
		return;
	}
	qca_multi_link_cfg.primary_wiphy = primary_wiphy;
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_INFO,
		FL("\n******Setting primary radio for wiphy****%p\n"), primary_wiphy);
}

qdf_export_symbol(qca_multi_link_set_primary_radio);

/**
 * qca_multi_link_remove_radio() - remove the radio pointer from repeater list
 * @primary_wiphy: wiphy pointer of radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_remove_radio(struct wiphy *wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	qdf_list_remove_node(&qca_multi_link_cfg.radio_list, &radio_node->node);
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	qdf_mem_free(radio_node);
	return true;
}

qdf_export_symbol(qca_multi_link_remove_radio);

/**
 * qca_multi_link_add_radio() - add the radio pointer to repeater list
 * @primary_wiphy: wiphy pointer of radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_add_radio(struct wiphy *wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	if (!wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_WARN,
			FL(" Radio could not be set - wiphy is NULL\n"));
		return false;
	}

	/*
	 * Check if Radio is already present in reppeater list.
	 */
	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(wiphy);
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	if (radio_node) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_WARN,
			FL(" Radio node already present\n"));
		return false;
	}

	radio_node = qdf_mem_malloc(sizeof(qca_multi_link_radio_node_t));
	if (!radio_node) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("Could not allocate node for wiphy%p\n"), wiphy);
		return false;
	}

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node->wiphy = wiphy;
	if (qdf_list_insert_front(&qca_multi_link_cfg.radio_list, &radio_node->node)
		== QDF_STATUS_SUCCESS) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			 FL("Adding radio node for wiphy%p\n"), wiphy);
		return true;
	}
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	qdf_mem_free(radio_node);
	return false;
}

qdf_export_symbol(qca_multi_link_add_radio);

/**
 * qca_multi_link_add_fastlane_radio() - add the fast lane radio pointer to list
 * @primary_wiphy: wiphy pointer of fast-lane radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_add_fastlane_radio(struct wiphy *fl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(fl_wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->is_fast_lane = true;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return true;
}

qdf_export_symbol(qca_multi_link_add_fastlane_radio);

/**
 * qca_multi_link_remove_fastlane_radio() - remove the fast lane radio pointer from list
 * @primary_wiphy: wiphy pointer of fast-lane radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_remove_fastlane_radio(struct wiphy *fl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(fl_wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->is_fast_lane = false;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return true;
}

qdf_export_symbol(qca_multi_link_remove_fastlane_radio);

/**
 * qca_multi_link_add_no_backhaul_radio() - add the no backhaul radio pointer to list
 * @primary_wiphy: wiphy pointer of fast-lane radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_add_no_backhaul_radio(struct wiphy *no_bl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(no_bl_wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->no_backhaul = true;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return true;
}

qdf_export_symbol(qca_multi_link_add_no_backhaul_radio);

/**
 * qca_multi_link_remove_no_backhaul_radio() - remove no-backhaul radio pointer from list
 * @primary_wiphy: wiphy pointer of no-backhaul radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_remove_no_backhaul_radio(struct wiphy *no_bl_wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(no_bl_wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->no_backhaul = false;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return true;
}

qdf_export_symbol(qca_multi_link_remove_no_backhaul_radio);

/**
 * qca_multi_link_remove_station_vap() - remove the radio station vap pointer for a radio
 * @primary_wiphy: wiphy pointer of radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_remove_station_vap(struct wiphy *wiphy)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->sta_dev = NULL;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	return true;
}

qdf_export_symbol(qca_multi_link_remove_station_vap);

/**
 * qca_multi_link_add_station_vap() - add the station vap pointer for a radio
 * @primary_wiphy: wiphy pointer of radio device
 *
 * Return: false: addition not successful
 *	   true: addition is successful
 */
bool qca_multi_link_add_station_vap(struct wiphy *wiphy, struct net_device *sta_dev)
{
	qca_multi_link_radio_node_t *radio_node = NULL;

	qdf_spin_lock(&qca_multi_link_cfg.radio_lock);
	radio_node = qca_multi_link_find_radio_node(wiphy);
	if (!radio_node) {
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	if (radio_node->sta_dev) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_WARN,
			 FL("STA Device already mapped for wiphy%p\n"), wiphy);
		qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
		return false;
	}

	radio_node->sta_dev = sta_dev;
	qdf_spin_unlock(&qca_multi_link_cfg.radio_lock);
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
		 FL("STA Device mapped for wiphy%p\n"), wiphy);
	return true;
}

qdf_export_symbol(qca_multi_link_add_station_vap);

/**
 * qca_multi_link_secondary_ap_rx() - Processing for frames recieved on Secondary AP VAP
 * @net_device: net device handle
 * @nbuf: frame
 *
 * Return: qca_multi_link_status_t
 *	   QCA_MULTI_LINK_PKT_ALLOW: frame should be processed further by caller.
 *	   QCA_MULTI_LINK_PKT_DROP: frame to be dropped.
 *	   QCA_MULTI_LINK_PKT_CONSUMED: frame is consumed.
 */
static qca_multi_link_status_t qca_multi_link_secondary_ap_rx(struct net_device *ap_dev, qdf_nbuf_t nbuf)
{
	struct wiphy *ap_wiphy = NULL;
	struct net_device *sta_dev = NULL;
	qca_multi_link_tbl_entry_t qca_ml_entry;

	QDF_STATUS qal_status = QDF_STATUS_E_FAILURE;
	bool enqueue_to_sta_vap = false;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	ap_wiphy = ap_dev->ieee80211_ptr->wiphy;
	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("Secondary AP Rx: always_primary=%d, loop_detected=%d,\
				drop_secondary_mcast=%d, shost %pM dhost %pM\n"),
				qca_multi_link_cfg.always_primary, qca_multi_link_cfg.loop_detected,
				qca_multi_link_cfg.drop_secondary_mcast, eh->ether_shost, eh->ether_dhost);

	/*
	 * If the AP is on a fast lane radio or if no backhaul is enabled,
	 * always give the packet to bridge.
	 */
	if (is_fast_lane_radio(ap_wiphy) || is_no_backhaul_radio(ap_wiphy)) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
	qca_ml_entry.qal_fdb_dev = NULL;
	qca_ml_entry.qal_fdb_is_local = 0;
	qal_status = qca_multi_link_tbl_has_entry(ap_dev, eh->ether_dhost, 0,
				&qca_ml_entry);
	if (qal_status == QDF_STATUS_SUCCESS) {

		/*
		 * Check the FDB entry type, if the mac-address is learnt on a port which is
		 * of type station, then it is a source on the RootAP side and enqueue the
		 * packet to the corresponding station vap. Else give the packet to bridge.
		 */
		if (qca_ml_entry.qal_fdb_ieee80211_ptr
		&& (qca_ml_entry.qal_fdb_ieee80211_ptr->iftype == NL80211_IFTYPE_STATION)) {
			enqueue_to_sta_vap = true;
		}
	} else {

		/*
		 * If there is no fdb entry, then also the destination might be on the RootAP side,
		 * enqueue the packet to the corresponding station vap.
		 */
		enqueue_to_sta_vap = true;
	}

	if (enqueue_to_sta_vap) {

		/*
		 * Find the station vap corresponding to the AP vap.
		 */
		sta_dev = qca_multi_link_get_station_vap(ap_wiphy);
		if (!sta_dev) {
			QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
				FL("Null STA device found %pM - Give to bridge\n"), eh->ether_shost);
			qca_multi_link_cfg.qca_ml_stats.ap_rx_sec_sta_null++;
			return QCA_MULTI_LINK_PKT_DROP;
		}

		qca_multi_link_tbl_add_or_refresh_entry(ap_dev, eh->ether_shost,
						       QCA_MULTI_LINK_ENTRY_USER_ADDED);
		dev_hold(sta_dev);
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("shost %pM dhost %pM \n"), eh->ether_shost, eh->ether_dhost);

		/*
		 * For packets destined to sources on RootAP directly enq to STA vap.
		 */
		sta_dev->netdev_ops->ndo_start_xmit(nbuf, sta_dev);
		dev_put(sta_dev);
		return QCA_MULTI_LINK_PKT_CONSUMED;
	}

	return QCA_MULTI_LINK_PKT_ALLOW;
}

/**
 * qca_multi_link_ap_rx() - Processing for frames recieved on AP VAP
 * @net_device: net device handle
 * @nbuf: frame
 *
 * Return: false: frame not consumed and should be processed further by caller
 *	   true: frame consumed
 */
bool qca_multi_link_ap_rx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	uint8_t is_mcast;
	uint8_t is_eapol;
	struct net_device *ap_dev = net_dev;
	qca_multi_link_status_t status = QCA_MULTI_LINK_PKT_NONE;
	struct wiphy *ap_wiphy = NULL;
	bool drop_packet = false;
	bool is_primary = false;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	if (!qca_multi_link_need_procesing()) {
		goto end;
	}

	if (!qca_multi_link_cfg.loop_detected) {
		goto end;
	}

	/*
	 * If it is mcast/broadcast frame, AST search cannot be done, so give
	 * the frame up the stack
	 * If it is EAPOL frame, just give the frame up the stack
	 */
	is_mcast = IEEE80211_IS_MULTICAST(eh->ether_dhost);
	is_eapol = (eh->ether_type == htons(ETHERTYPE_PAE));
	if (is_mcast || is_eapol) {
		goto end;
	}

	ap_wiphy = ap_dev->ieee80211_ptr->wiphy;
	is_primary = qca_multi_link_is_primary_radio(ap_wiphy);

	if (is_primary) {
		goto end;
	} else {
		dev_hold(ap_dev);
		status = qca_multi_link_secondary_ap_rx(ap_dev, nbuf);
		dev_put(ap_dev);
	}

	if (status == QCA_MULTI_LINK_PKT_ALLOW) {
		goto end;
	} else if (status == QCA_MULTI_LINK_PKT_CONSUMED) {
		return true;
	} else if (status == QCA_MULTI_LINK_PKT_DROP) {
		drop_packet = true;
	}

end:
	if (drop_packet) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("\n STA TX - Drop Packet for Mac=%pM\n"), eh->ether_shost);
		qdf_nbuf_free(nbuf);
		return true;
	}
	return false;
}

qdf_export_symbol(qca_multi_link_ap_rx);

/**
 * qca_multi_link_secondary_sta_rx() - Processing for frames recieved on secondary station vap
 * @net_dev: station net device
 * @nbuf: frame
 *
 * Return: @qca_multi_link_status_t
 *	   QCA_MULTI_LINK_PKT_ALLOW: frame should be processed further by caller
 *	   QCA_MULTI_LINK_PKT_DROP: frame to be dropped.
 *	   QCA_MULTI_LINK_PKT_CONSUMED: frame is consumed.
 */
static qca_multi_link_status_t qca_multi_link_secondary_sta_rx(struct net_device *net_dev,
							      qdf_nbuf_t nbuf)
{
	uint8_t is_mcast;
	qca_multi_link_tbl_entry_t qca_ml_entry;
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	struct net_device *ap_dev = NULL;
	QDF_STATUS qal_status = QDF_STATUS_E_FAILURE;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;
	is_mcast = IEEE80211_IS_MULTICAST(eh->ether_dhost);

	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("Secondary STA Rx:always_primary=%d, loop_detected=%d,\
			drop_secondary_mcast=%d, shost %pM dhost %pM is_mcast=%d\n"),
			qca_multi_link_cfg.always_primary, qca_multi_link_cfg.loop_detected,
			qca_multi_link_cfg.drop_secondary_mcast, eh->ether_shost, eh->ether_dhost, is_mcast);

	/*
	 * Mcast packets handling.
	 */
	if (is_mcast) {

		/*
		 * Always drop mcast packets on secondary radio when loop has been detected.
		 */
		if (qca_multi_link_cfg.loop_detected) {
			qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_mcast_drop++;
			return QCA_MULTI_LINK_PKT_DROP;
		}

		qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
		qca_ml_entry.qal_fdb_dev = NULL;
		qca_ml_entry.qal_fdb_is_local = 0;
		qal_status = qca_multi_link_tbl_has_entry(sta_dev, eh->ether_shost, 0,
							&qca_ml_entry);
		if (qal_status != QDF_STATUS_SUCCESS) {
			if (!qca_multi_link_cfg.loop_detected
				&& !qca_multi_link_cfg.drop_secondary_mcast) {
			/*
			 * This condition is to allow packets on Secondary Station
			 * when stations are connected to different RootAPs and loop is not
			 * detected.
			 */
				return QCA_MULTI_LINK_PKT_ALLOW;
			} else {
				qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_mcast_no_fdb++;
				return QCA_MULTI_LINK_PKT_DROP;
			}
		}

		/*
		 * Case 1:
		 * ieee80211_ptr pointer being NULL indicates that the port
		 * corresponding to the fdb entry is a non-wireless/ethernet
		 * device behind the repeater and the packet is a mcast looped packet.
		 * Case 2:
		 * ieee80211_ptr pointer being non NULL indicates that the source
		 * corresponding to the fdb entry is a wireless device
		 * behind the repeater and the packet is a mcast looped packet.
		 */

		if (qca_ml_entry.qal_fdb_ieee80211_ptr && (qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy != sta_wiphy)) {
			if (!qca_multi_link_cfg.loop_detected) {
				if (qca_ml_entry.qal_fdb_is_local
					&& (qca_ml_entry.qal_fdb_ieee80211_ptr->iftype == NL80211_IFTYPE_STATION)) {
					qca_multi_link_cfg.loop_detected = true;
					QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_INFO, FL("\n****Wifi Rptr Loop Detected****\n"));
				}
			}
			qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_mcast_dup_pkts++;
			return QCA_MULTI_LINK_PKT_DROP;
		}

		if (qca_multi_link_drop_secondary_mcast(nbuf)) {
			qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_mcast_drop_sec++;
			return QCA_MULTI_LINK_PKT_DROP;
		}

		/*
		 * If the mac-address is learnt on the station in the bridge,
		 * then the mcast packet is from a source on the RootAP side and we
		 * should allow the packet.
		 * This check on secondary will take of the case where stations are connected to different RootAPs
		 * and loop is not detected.
		 */
		if (qca_ml_entry.qal_fdb_dev == sta_dev) {
			return QCA_MULTI_LINK_PKT_ALLOW;
		}

		qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_mcast_drop++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	/*
	 * Unicast packets handling received on secondary Stations.
	 */
	if (qca_multi_link_cfg.loop_detected) {

		qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
		qca_ml_entry.qal_fdb_dev = NULL;
		qca_ml_entry.qal_fdb_is_local = 0;
		qal_status = qca_multi_link_tbl_has_entry(sta_dev, eh->ether_dhost, 0,
							&qca_ml_entry);
		if (qal_status == QDF_STATUS_SUCCESS) {

			/*
			 * Unicast packets destined to ethernets or bridge should never come
			 * on secondary stations.
			 */
			if (!qca_ml_entry.qal_fdb_ieee80211_ptr) {
				qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_ucast_src_eth++;
				return QCA_MULTI_LINK_PKT_DROP;
			}

			/*
			 * Compare the physical device and check if the destination is a client
			 * on the same radio, then enqueue directly to AP vap.
			 */
			if ((qca_ml_entry.qal_fdb_ieee80211_ptr->iftype == NL80211_IFTYPE_AP)
				&& (qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy == sta_wiphy)) {
				QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("Unicast Sec STA to AP direct enq for\
					shost %pM dhost %pM \n"), eh->ether_shost, eh->ether_dhost);
				/*
				 * Holding the AP dev so that it cannot be brought down
				 * while we are enqueueing.
				 */
				dev_hold(qca_ml_entry.qal_fdb_dev);
				qca_ml_entry.qal_fdb_dev->netdev_ops->ndo_start_xmit(nbuf, qca_ml_entry.qal_fdb_dev);
				dev_put(qca_ml_entry.qal_fdb_dev);
				return QCA_MULTI_LINK_PKT_CONSUMED;
			}

			qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_ucast_src_dif_band++;
			return QCA_MULTI_LINK_PKT_DROP;
		} else {

			/*
			 * If there is no bridge fdb entry for unicast packets received on secondary
			 * station, give the packet to the first found AP vap entry in the bridge table.
			 */
			QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("No Fdb entry on sec radio\
				for ucast pkt with dhost %pM \n"), eh->ether_dhost);
			/*
			 * Find the AP vap corresponding to the station vap.
			 */
			ap_dev = qca_multi_link_tbl_find_sta_or_ap(sta_dev, 0);
			if (!ap_dev) {
				QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
				FL("Null AP device found %pM - Drop\n"), eh->ether_shost);
				qca_multi_link_cfg.qca_ml_stats.sta_rx_sec_sta_ucast_no_ap++;
				return QCA_MULTI_LINK_PKT_DROP;
			}

			QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
				FL("shost %pM dhost %pM \n"), eh->ether_shost, eh->ether_dhost);

			/*
			 * For packets destined to sources on RootAP directly enq to STA vap.
			 */
			dev_hold(ap_dev);
			ap_dev->netdev_ops->ndo_start_xmit(nbuf, ap_dev);
			dev_put(ap_dev);
			return QCA_MULTI_LINK_PKT_CONSUMED;
		}
	}
	return QCA_MULTI_LINK_PKT_ALLOW;
}

/**
 * qca_multi_link_primary_sta_rx() - Processing for frames recieved on primary station vap
 * @net_dev: station net device
 * @nbuf: frame
 *
 * Return: @qca_multi_link_status_t
 *	   QCA_MULTI_LINK_PKT_ALLOW: frame should be processed further by caller
 *	   QCA_MULTI_LINK_PKT_DROP: frame to be dropped
 *	   QCA_MULTI_LINK_PKT_CONSUMED: frame is consumed.
 */
static qca_multi_link_status_t qca_multi_link_primary_sta_rx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	uint8_t is_mcast;
	qca_multi_link_tbl_entry_t qca_ml_entry;
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	QDF_STATUS qal_status = QDF_STATUS_E_FAILURE;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;
	is_mcast = IEEE80211_IS_MULTICAST(eh->ether_dhost);

	/*
	 * Unicast Packets are allowed without any processing on Primary Station Vap.
	 */
	if (!is_mcast) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("Primary STA Rx: always_primary=%d, loop_detected=%d,\
			drop_secondary_mcast=%d, shost %pM dhost %pM is_mcast=%d\n"),
			qca_multi_link_cfg.always_primary, qca_multi_link_cfg.loop_detected,
			qca_multi_link_cfg.drop_secondary_mcast, eh->ether_shost, eh->ether_dhost, is_mcast);

	/*
	 * Mcast packet handling on Primary Station Vap Interface.
	 */
	qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
	qca_ml_entry.qal_fdb_dev = NULL;
	qca_ml_entry.qal_fdb_is_local = 0;
	qal_status = qca_multi_link_tbl_has_entry(sta_dev, eh->ether_shost, 0,
				&qca_ml_entry);
	if (qal_status != QDF_STATUS_SUCCESS) {
		if (qca_multi_link_cfg.loop_detected) {
		/*
		 * If there is no fdb entry, the we allow the packet to go to
		 * bridge as this might be the first packet from any device
		 * on the RootAP side.
		 */
			return QCA_MULTI_LINK_PKT_ALLOW;
		}
		qca_multi_link_cfg.qca_ml_stats.sta_rx_pri_sta_mcast_no_fdb++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	/*
	 * Case 1:
	 * ieee80211_ptr pointer being NULL indicates that the port
	 * corresponding to the fdb entry is a non-wireless/ethernet
	 * device behind the repeater and the packet is a mcast looped packet.
	 * Case 2:
	 * ieee80211_ptr pointer being non NULL indicates that the source
	 * corresponding to the fdb entry is a wireless device
	 * behind the repeater and the packet is a mcast looped packet.
	 */

	/*
	 * Drop the loopback mcast packets from ethernet devices behind the repeater.
	 */
	if (!qca_ml_entry.qal_fdb_ieee80211_ptr) {
		qca_multi_link_cfg.qca_ml_stats.sta_rx_pri_sta_mcast_drop++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	if (qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy != sta_wiphy) {
		if (qca_ml_entry.qal_fdb_is_local &&
				(qca_ml_entry.qal_fdb_ieee80211_ptr->iftype
				== NL80211_IFTYPE_STATION)) {
			if (!qca_multi_link_cfg.loop_detected) {
				qca_multi_link_cfg.loop_detected = true;
				QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_INFO,
						FL("\n****Wifi Rptr Loop Detected****\n"));
			}
		} else {
			if (!qca_ml_entry.qal_fdb_is_local
				&& (qca_ml_entry.qal_fdb_ieee80211_ptr->iftype == NL80211_IFTYPE_STATION)) {
				/* This condition allows any source on RootAP or behind to be re-learnt
				 * on Primary Station to overwrite a learning on secondary station.
				 * This kind of schenario can happen when secondary station vap
				 * comes up first and RootAP side devices are learnt on
				 * secondary station and the primary station vap comes up.
				 */
				return QCA_MULTI_LINK_PKT_ALLOW;
			}
		}
		qca_multi_link_cfg.qca_ml_stats.sta_rx_pri_sta_mcast_drop++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	/*
	 * If the mac-address is learnt on the Primary station in the bridge,
	 * then the mcast packet is from a source on the RootAP side and we
	 * should allow the packet.
	 */
	if (qca_ml_entry.qal_fdb_dev == sta_dev) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	qca_multi_link_cfg.qca_ml_stats.sta_rx_pri_sta_mcast_drop++;
	return QCA_MULTI_LINK_PKT_DROP;
}

/**
 * qca_multi_link_sta_rx() - Processing for frames recieved on STA VAP
 * @net_dev: station net device
 * @nbuf: frame
 *
 * Return: false: frame not consumed and should be processed further by caller
 *	   true: frame dropped/enqueued.
 */
bool qca_multi_link_sta_rx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	uint8_t is_eapol;
	bool is_primary = false;
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);
	bool drop_packet = false;
	qca_multi_link_status_t status = QCA_MULTI_LINK_PKT_NONE;

	if (!qca_multi_link_need_procesing()) {
		goto end;
	}

	is_eapol = (eh->ether_type == htons(ETHERTYPE_PAE));
	if (is_eapol) {
		goto end;
	}

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;
	is_primary = qca_multi_link_is_primary_radio(sta_wiphy);

	if (qca_multi_link_drop_always_primary(is_primary, nbuf)) {
		drop_packet = true;
		goto end;
	}

	dev_hold(sta_dev);
	if (is_primary) {
		status = qca_multi_link_primary_sta_rx(sta_dev, nbuf);
	} else {
		status = qca_multi_link_secondary_sta_rx(sta_dev, nbuf);
	}
	dev_put(sta_dev);

	if (status == QCA_MULTI_LINK_PKT_ALLOW) {
		goto end;
	} else if (status == QCA_MULTI_LINK_PKT_CONSUMED) {
		return true;
	} else if (status == QCA_MULTI_LINK_PKT_DROP) {
		drop_packet = true;
	}

end:
	if (drop_packet) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("\n STA TX - Drop Packet for Mac=%pM\n"), eh->ether_shost);
		qdf_nbuf_free(nbuf);
		return true;
	}
	return false;
}

qdf_export_symbol(qca_multi_link_sta_rx);

/**
 * qca_multi_link_secondary_sta_tx() - Repeater TX processing for Secondary
 * @net_device: station net device handle
 * @nbuf: frame
 *
 * Return: @qca_multi_link_status_t
 *	   QCA_MULTI_LINK_PKT_ALLOW: frame should be processed further by caller
 *	   QCA_MULTI_LINK_PKT_DROP: frame to be dropped
 *	   QCA_MULTI_LINK_PKT_CONSUMED: frame is consumed.
 */
static qca_multi_link_status_t qca_multi_link_secondary_sta_tx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	uint8_t is_mcast;
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	qca_multi_link_tbl_entry_t qca_ml_entry;
	QDF_STATUS qal_status = QDF_STATUS_E_FAILURE;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	if (qca_multi_link_drop_always_primary(false, nbuf)) {
		qca_multi_link_cfg.qca_ml_stats.sta_tx_sec_sta_alwys_prim++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;

	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("STA Secondary Tx: always_primary=%d, loop_detected=%d,\
		drop_secondary_mcast=%d, shost %pM dhost %pM \n"),
		qca_multi_link_cfg.always_primary, qca_multi_link_cfg.loop_detected, qca_multi_link_cfg.drop_secondary_mcast,
		eh->ether_shost, eh->ether_dhost);

	/*
	 * For a Secondary station, only Packets from clients on the same band are allowed for transmit.
	 */
	is_mcast = IEEE80211_IS_MULTICAST(eh->ether_dhost);
	qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
	qca_ml_entry.qal_fdb_dev = NULL;
	qca_ml_entry.qal_fdb_is_local = 0;
	qal_status = qca_multi_link_tbl_has_entry(sta_dev, eh->ether_shost, 0,
				       &qca_ml_entry);
	if (qal_status != QDF_STATUS_SUCCESS) {
		if (!is_mcast) {
			return QCA_MULTI_LINK_PKT_ALLOW;
		}
		qca_multi_link_cfg.qca_ml_stats.sta_tx_sec_sta_mcast_no_fdb++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	if (!qca_ml_entry.qal_fdb_ieee80211_ptr) {
		/*
		 * ieee80211_ptr pointer will be NULL for ethernet devices.
		 * Packets from ethernet devices or bridge are allowed only on	Primary radio.
		 */
		qca_multi_link_cfg.qca_ml_stats.sta_tx_sec_sta_src_eth++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	/*
	 * Do the DBDC Fast Lane processing at the beginning and then fall back to normal DBDC STA TX
	 * if either fast-lane is disabled or the TX is for non-fast lane radio.
	 */
	if (is_fast_lane_radio(sta_wiphy) && is_fast_lane_radio(qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy)) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	if (qca_multi_link_drop_secondary_mcast(nbuf)) {
		qca_multi_link_cfg.qca_ml_stats.sta_tx_sec_sta_mcast_drop_sec++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	/*
	 * Compare the physical device and check if the source is a client
	 * on the same radio.
	 */
	if (qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy != sta_dev->ieee80211_ptr->wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("STA TX: Diff Band Primary drop\
			shost %pM dhost %pM \n"), eh->ether_shost, eh->ether_dhost);
		qca_multi_link_cfg.qca_ml_stats.sta_tx_sec_sta_drop_dif_band++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	return QCA_MULTI_LINK_PKT_ALLOW;
}

/**
 * qca_multi_link_primary_sta_tx() - Repeater TX processing for Primary
 * @net_device: station net device handle
 * @nbuf: frame
 *
 * Return: @qca_multi_link_status_t
 *	   QCA_MULTI_LINK_PKT_ALLOW: frame should be processed further by caller
 *	   QCA_MULTI_LINK_PKT_DROP: frame to be dropped
 *	   QCA_MULTI_LINK_PKT_CONSUMED: frame is consumed.
 */
static qca_multi_link_status_t qca_multi_link_primary_sta_tx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	qca_multi_link_tbl_entry_t qca_ml_entry;
	QDF_STATUS qal_status = QDF_STATUS_E_FAILURE;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;

	QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("Primary STA Tx: always_primary=%d, loop_detected=%d,\
				drop_secondary_mcast=%d, shost %pM dhost %pM \n"),
				qca_multi_link_cfg.always_primary, qca_multi_link_cfg.loop_detected,
				qca_multi_link_cfg.drop_secondary_mcast, eh->ether_shost, eh->ether_dhost);

	/*
	 * For Primary station, packets allowed for transmission are:
	 * 1) Packets from ethernet devices.
	 * 2) Packets from radios which are not participating in backhaul.
	 * 3) Packets from clients on the same band.
	 */
	qca_ml_entry.qal_fdb_ieee80211_ptr = NULL;
	qca_ml_entry.qal_fdb_dev = NULL;
	qca_ml_entry.qal_fdb_is_local = 0;
	qal_status = qca_multi_link_tbl_has_entry(sta_dev, eh->ether_shost, 0,
						&qca_ml_entry);
	/*
	 * All packets coming to Primary station has to have a bridge fdb entry
	 * as they will be received from bridge only.
	 */
	if (qal_status != QDF_STATUS_SUCCESS) {
		qca_multi_link_cfg.qca_ml_stats.sta_tx_pri_sta_drop_no_fdb++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	if (!qca_ml_entry.qal_fdb_ieee80211_ptr) {
		/*
		 * ieee80211_ptr pointer will be NULL for ethernet devices.
		 * Packets from ethernet devices or bridge are allowed only on	Primary radio.
		 */
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	/*
	 * Do the DBDC Fast Lane processing at the beginning and then fall back to normal DBDC STA TX
	 * if either fast-lane is disabled or the TX is for non-fast lane radio.
	 */
	if (is_fast_lane_radio(sta_wiphy)
		&& is_fast_lane_radio(qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy)) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	/*
	 * This flag will be set for radios which does not particpate in backhaul.
	 */
	if (is_no_backhaul_radio(qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy)) {
		return QCA_MULTI_LINK_PKT_ALLOW;
	}

	/*
	 * Compare the physical device and check if the source is a client
	 * on the same radio.
	 */
	if (qca_ml_entry.qal_fdb_ieee80211_ptr->wiphy != sta_dev->ieee80211_ptr->wiphy) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG, FL("STA TX: Diff Band Primary drop\
			shost %pM dhost %pM \n"), eh->ether_shost, eh->ether_dhost);
		qca_multi_link_cfg.qca_ml_stats.sta_tx_pri_sta_drop_dif_band++;
		return QCA_MULTI_LINK_PKT_DROP;
	}

	return QCA_MULTI_LINK_PKT_ALLOW;
}

/**
 * qca_multi_link_sta_tx() - Repeater TX processing
 * @net_device: station net device handle
 * @nbuf: frame
 *
 * Return: false: frame not consumed and should be processed further by caller
 *	   true: frame consumed
 */
bool qca_multi_link_sta_tx(struct net_device *net_dev, qdf_nbuf_t nbuf)
{
	bool drop_packet = false;
	bool is_primary = false;
	struct wiphy *sta_wiphy = NULL;
	struct net_device *sta_dev = net_dev;
	qdf_ether_header_t *eh = (qdf_ether_header_t *) qdf_nbuf_data(nbuf);
	qca_multi_link_status_t status = QCA_MULTI_LINK_PKT_NONE;

	if (!qca_multi_link_need_procesing()) {
		goto end;
	}

	if (!qca_multi_link_cfg.loop_detected) {
		goto end;
	}

	if (qca_multi_link_pktfrom_ownsrc(sta_dev, nbuf)) {
		goto end;
	}

	sta_wiphy = sta_dev->ieee80211_ptr->wiphy;
	is_primary = qca_multi_link_is_primary_radio(sta_wiphy);

	dev_hold(net_dev);
	if (is_primary) {
		status = qca_multi_link_primary_sta_tx(sta_dev, nbuf);
	} else {
		status = qca_multi_link_secondary_sta_tx(sta_dev, nbuf);
	}
	dev_put(net_dev);

	if (status == QCA_MULTI_LINK_PKT_ALLOW) {
		goto end;
	} else if (status == QCA_MULTI_LINK_PKT_CONSUMED) {
		return true;
	} else if (status == QCA_MULTI_LINK_PKT_DROP) {
		drop_packet = true;
	}

end:
	if (drop_packet) {
		QDF_TRACE(QDF_MODULE_ID_RPTR, QDF_TRACE_LEVEL_DEBUG,
			FL("\n STA TX - Drop Packet for Mac=%pM\n"), eh->ether_shost);
		qdf_nbuf_free(nbuf);
		return true;
	}

	return false;
}

qdf_export_symbol(qca_multi_link_sta_tx);
