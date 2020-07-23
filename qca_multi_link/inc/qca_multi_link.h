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

#ifndef __qca_multi_link_H_
#define __qca_multi_link_H_

#include <qdf_nbuf.h>
#include <qdf_module.h>
#include <qdf_list.h>
#include <qdf_util.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <br_private.h>

#include <ieee80211.h>
#include <ieee80211_defines.h>
#include <qca_multi_link_tbl.h>
#include "if_upperproto.h"


#define QCA_MULTI_LINK_FAST_LANE_LIST_SIZE 6
#define QCA_MULTI_LINK_NO_BACKHAUL_LIST_SIZE 32

/**
 * qca_multi_link_needs_enq - rptr return status
 * @QCA_MULTI_LINK_ALLOW_VAP_ENQ: Allow the packet to be enqueued
 * @QCA_MULTI_LINK_SKIP_VAP_ENQ: SKIP the enqueue
 */
typedef enum qca_multi_link_needs_enq {
	QCA_MULTI_LINK_ALLOW_VAP_ENQ = 0,
	QCA_MULTI_LINK_SKIP_VAP_ENQ,
} qca_multi_link_needs_enq_t;

/**
 * enum qca_multi_link_status - rptr return status
 * @qca_multi_link_PKT_ALLOW: Allow the packet to be received or transmitted
 * @qca_multi_link_PKT_DROP: Drop the packet
 * @qca_multi_link_PKT_CONSUMED: The packet is consumed in the repeater processing
 */
typedef enum qca_multi_link_status {
	QCA_MULTI_LINK_PKT_NONE = 0,
	QCA_MULTI_LINK_PKT_ALLOW,
	QCA_MULTI_LINK_PKT_DROP,
	QCA_MULTI_LINK_PKT_CONSUMED
} qca_multi_link_status_t;

/**
 * struct qca_multi_link_list_node - rptr list node
 * @list: linked list node
 * @wiphy: wiphy pointer
 */
struct qca_multi_link_list_node {
	qdf_list_node_t node;
	struct wiphy *wiphy;
};

typedef struct qca_multi_link_parameters {
	bool rptr_processing_enable;
	bool loop_detected;
	bool always_primary;
	bool force_client_mcast_traffic;
	bool drop_secondary_mcast;
	struct wiphy *primary_wiphy;
	uint8_t total_stavaps_up;
	qdf_list_t no_backhaul_list;
	qdf_list_t fast_lane_list;
} qca_multi_link_parameters_t;

void qca_multi_link_init_module(void);
void qca_multi_link_deinit_module(void);
uint8_t qca_multi_link_get_num_sta(void);
void qca_multi_link_append_num_sta(bool inc_or_dec);
bool qca_multi_link_is_dbdc_processing_reqd(struct net_device *net_dev);
void qca_multi_link_set_drop_sec_mcast(bool val);
void qca_multi_link_set_force_client_mcast(bool val);
void qca_multi_link_set_always_primary(bool val);
void qca_multi_link_set_dbdc_enable(bool val);
struct wiphy *qca_multi_link_get_primary_radio(void);
void qca_multi_link_set_primary_radio(struct wiphy *primary_wiphy);
bool qca_multi_link_add_fastlane_radio(struct wiphy *fl_wiphy);
bool qca_multi_link_remove_fastlane_radio(struct wiphy *fl_wiphy);
bool qca_multi_link_add_no_backhaul_radio(struct wiphy *no_bl_wiphy);
bool qca_multi_link_remove_no_backhaul_radio(struct wiphy *no_bl_wiphy);
bool qca_multi_link_ap_rx(struct net_device *net_dev, qdf_nbuf_t nbuf);
bool qca_multi_link_sta_rx(struct net_device *net_dev, qdf_nbuf_t nbuf);
bool qca_multi_link_sta_tx(struct net_device *net_dev, qdf_nbuf_t nbuf);
#endif
