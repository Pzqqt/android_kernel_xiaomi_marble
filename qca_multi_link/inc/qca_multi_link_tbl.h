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

/**
 * DOC: qca_multi_link_tbl (QAL Bridge)
 * QCA driver framework for OS notifier handlers
 */

#ifndef __qca_multi_link_tbl_H
#define __qca_multi_link_tbl_H

#include "qdf_types.h"
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <br_private.h>

/**
 * qca_multi_link_entry_type - bridge entry type
 * @QCA_MULTI_LINK_ENTRY_USER_ADDED: Entry added by user
 * @QCA_MULTI_LINK_ENTRY_LOCAL: Local permanent bridge entry
 * @QCA_MULTI_LINK_ENTRY_STATIC: Permanent non-ARP type entry
 */
typedef enum qca_multi_link_entry_type {
	QCA_MULTI_LINK_ENTRY_NONE = 0,
	QCA_MULTI_LINK_ENTRY_USER_ADDED,
	QCA_MULTI_LINK_ENTRY_LOCAL,
	QCA_MULTI_LINK_ENTRY_STATIC,
} qca_multi_link_entry_type_t;

typedef struct qca_multi_link_tbl_entry {
	struct wireless_dev *qal_fdb_ieee80211_ptr;
	struct net_device *qal_fdb_dev;
	uint8_t qal_fdb_is_local;
	unsigned char qal_mac_addr[6];
} qca_multi_link_tbl_entry_t;

/**
 *
 * qca_multi_link_tbl_get_eth_entries() - Get ethernet bridge entries
 *
 * To be called from the code with a valid netdevice
 *
 * Return: Number of entries copied
 */
int qca_multi_link_tbl_get_eth_entries(struct net_device *net_dev,
				      void *fill_buff, int buff_size);

/**
 *
 * qca_multi_link_tbl_find_sta_or_ap() - Get the AP or Station
 *					from bridge on the same radio
 *
 * To be called from the code with a valid netdevice
 *
 * Return: struct net_device
 */
struct net_device *qca_multi_link_tbl_find_sta_or_ap(struct net_device *net_dev, uint8_t dev_type);

/**
 *
 * qca_multi_link_tbl_add_or_refresh_entry() - Add or refresh a bridge entry
 *
 * To be called from the code with a valid netdevice
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qca_multi_link_tbl_add_or_refresh_entry(struct net_device *net_dev, uint8_t *addr,
							qca_multi_link_entry_type_t entry_type);

/**
 *
 * qca_multi_link_tbl_delete_entry() - Delete a non-local bridge fdb entry
 *
 * To be called from the code with a valid netdevice
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qca_multi_link_tbl_delete_entry(struct net_device *net_dev, uint8_t *addr);

/**
 *
 * qca_multi_link_tbl_has_entry() - check if there is fdb entry for the mac-address
 *
 * To be called from the code with a valid qal_entry pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qca_multi_link_tbl_has_entry(struct net_device *net_dev,
				       const char *addr, uint16_t vlan_id,
				       qca_multi_link_tbl_entry_t *qca_ml_entry);
/**
 *
 * qca_multi_link_tbl_register_update_notifier() - register to br_fdb update notifier chain
 *
 * To be called from each callback with its own handle
 *
 * Return: None
 */
QDF_STATUS qca_multi_link_tbl_register_update_notifier(void *nb);

/**
 * qca_multi_link_tbl_unregister_update_notifier() - unregister linux br_fdb update notifier chain
 *
 * To be called from each callback with its own handle
 *
 * Return: None
 */
QDF_STATUS qca_multi_link_tbl_unregister_update_notifier(void *nb);

/**
 *
 * qca_multi_link_tbl_register_notifier() - register to br_fdb notifier chain
 *
 * To be called from each callback with its own handle
 *
 * Return: None
 */
QDF_STATUS qca_multi_link_tbl_register_notifier(void *nb);

/**
 * qca_multi_link_tbl_unregister_notifier() - unregister linux br_fdb notifier chain
 *
 * To be called from each callback with its own handle
 *
 * Return: None
 */
QDF_STATUS qca_multi_link_tbl_unregister_notifier(void *nb);

/**
 *
 * qca_multi_link_tbl_get_bridge_dev() - Get bridge netdevice from port
 *
 * To be called from the code with a valid port netdevice
 *
 * Return: Bridge netdevice
 */
struct net_device *qca_multi_link_tbl_get_bridge_dev(struct net_device *port_dev);
#endif /* __qca_multi_link_tbl_H */
