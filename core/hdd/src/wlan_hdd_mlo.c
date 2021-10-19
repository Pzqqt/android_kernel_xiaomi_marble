/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * DOC : wlan_hdd_mlo.c
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */
#include "wlan_hdd_main.h"
#include "wlan_hdd_mlo.h"
#include "osif_vdev_sync.h"

void hdd_update_mld_mac_addr(struct hdd_context *hdd_ctx,
			     struct qdf_mac_addr hw_macaddr)
{
	uint8_t i;
	uint8_t mldaddr_b2, tmp_br2;
	struct hdd_mld_mac_info *mac_info;

	mac_info = &hdd_ctx->mld_mac_info;
	for (i = 0; i < WLAN_MAX_MLD; i++) {
		qdf_mem_copy(mac_info->mld_mac_list[i].mld_addr.bytes,
			     hw_macaddr.bytes, QDF_MAC_ADDR_SIZE);
		mldaddr_b2 = mac_info->mld_mac_list[i].mld_addr.bytes[2];
		tmp_br2 = ((mldaddr_b2 >> 4 & INTF_MACADDR_MASK) + i) &
			  INTF_MACADDR_MASK;
		mldaddr_b2 += tmp_br2;

		/* XOR-ing bit-24 of the mac address. This will give enough
		 * mac address range before collision
		 */
		mldaddr_b2 ^= (1 << 7);

		/* Set locally administered bit */
		mac_info->mld_mac_list[i].mld_addr.bytes[0] |= 0x02;
		mac_info->mld_mac_list[i].mld_addr.bytes[2] = mldaddr_b2;
		hdd_debug("mld addr[%d]: "
			QDF_MAC_ADDR_FMT, i,
		    QDF_MAC_ADDR_REF(mac_info->mld_mac_list[i].mld_addr.bytes));

		mac_info->mld_mac_list[i].device_mode = QDF_MAX_NO_OF_MODE;
		mac_info->num_mld_addr++;

		hdd_debug("num_mld_addr: %d", mac_info->num_mld_addr);
	};
}

uint8_t *wlan_hdd_get_mld_addr(struct hdd_context *hdd_ctx, uint8_t device_mode)
{
	int i;
	struct hdd_mld_mac_info *mac_info;

	if (device_mode != QDF_STA_MODE && device_mode != QDF_SAP_MODE)
		return NULL;

	mac_info = &hdd_ctx->mld_mac_info;
	for (i = 0; i <  mac_info->num_mld_addr; i++) {
		if (mac_info->mld_mac_list[i].device_mode == device_mode)
			return mac_info->mld_mac_list[i].mld_addr.bytes;
	}

	i = qdf_ffz(mac_info->mld_intf_addr_mask);
	if (i < 0 || i >= mac_info->num_mld_addr)
		return NULL;
	qdf_atomic_set_bit(i, &mac_info->mld_intf_addr_mask);
	hdd_nofl_debug("Assigning MLD MAC from derived list " QDF_MAC_ADDR_FMT,
		    QDF_MAC_ADDR_REF(mac_info->mld_mac_list[i].mld_addr.bytes));

	mac_info->mld_mac_list[i].device_mode = device_mode;
	return mac_info->mld_mac_list[i].mld_addr.bytes;
}

void hdd_register_wdev(struct hdd_adapter *sta_adapter,
		       struct hdd_adapter *link_adapter,
		       struct hdd_adapter_create_param *adapter_params)
{
	int ret, i;

	hdd_enter_dev(sta_adapter->dev);
	/* Set the relation between adapters*/
	link_adapter->wdev.iftype = NL80211_IFTYPE_MLO_LINK;

	ret = cfg80211_register_mlo_link(&sta_adapter->wdev,
					 &link_adapter->wdev);
	if (ret) {
		hdd_err("Failed to register ml link wdev %d", ret);
		return;
	}

	sta_adapter->mlo_adapter_info.is_ml_adapter = true;
	sta_adapter->mlo_adapter_info.is_link_adapter = false;
	link_adapter->mlo_adapter_info.is_link_adapter = true;
	link_adapter->mlo_adapter_info.is_ml_adapter = false;
	link_adapter->mlo_adapter_info.ml_adapter = sta_adapter;
	link_adapter->mlo_adapter_info.associate_with_ml_adapter =
				      adapter_params->associate_with_ml_adapter;
	qdf_set_bit(WDEV_ONLY_REGISTERED, &link_adapter->event_flags);

	for (i = 0; i < WLAN_MAX_MLD; i++) {
		if (sta_adapter->mlo_adapter_info.link_adapter[i])
			continue;
		sta_adapter->mlo_adapter_info.link_adapter[i] = link_adapter;
		break;
	}

	qdf_mem_copy(link_adapter->mld_addr.bytes, sta_adapter->mld_addr.bytes,
		     QDF_MAC_ADDR_SIZE);
	hdd_exit();
}

static
void hdd_mlo_close_adapter(struct hdd_adapter *link_adapter, bool rtnl_held)
{
	struct osif_vdev_sync *vdev_sync;

	vdev_sync = osif_vdev_sync_unregister(link_adapter->dev);
	if (vdev_sync)
		osif_vdev_sync_wait_for_ops(vdev_sync);

	hdd_check_for_net_dev_ref_leak(link_adapter);
	wlan_hdd_release_intf_addr(link_adapter->hdd_ctx,
				   link_adapter->mac_addr.bytes);

	link_adapter->wdev.netdev = NULL;

	if (rtnl_held)
		rtnl_unlock();

	cfg80211_unregister_wdev(&link_adapter->wdev);
	link_adapter->dev->reg_state = NETREG_UNREGISTERED;
	free_netdev(link_adapter->dev);

	if (rtnl_held)
		rtnl_lock();

	if (vdev_sync)
		osif_vdev_sync_destroy(vdev_sync);
}

QDF_STATUS hdd_wlan_unregister_mlo_interfaces(struct hdd_adapter *adapter,
					      bool rtnl_held)
{
	int i;
	struct hdd_mlo_adapter_info *mlo_adapter_info;
	struct hdd_adapter *link_adapter;

	mlo_adapter_info = &adapter->mlo_adapter_info;

	if (mlo_adapter_info->is_link_adapter) {
		hdd_remove_front_adapter(adapter->hdd_ctx, &adapter);
		return QDF_STATUS_E_AGAIN;
	}

	for (i = 0; i < WLAN_MAX_MLD; i++) {
		link_adapter = mlo_adapter_info->link_adapter[i];
		if (!link_adapter)
			continue;
		hdd_mlo_close_adapter(link_adapter, rtnl_held);
	}

	return QDF_STATUS_SUCCESS;
}

void hdd_wlan_register_mlo_interfaces(struct hdd_context *hdd_ctx)
{
	uint8_t *mac_addr;
	struct hdd_adapter_create_param params = {0};
	QDF_STATUS status;

	mac_addr = wlan_hdd_get_intf_addr(hdd_ctx, QDF_STA_MODE);
	if (mac_addr) {
		/* if target supports MLO create a new dev */
		params.only_wdev_register = true;
		params.associate_with_ml_adapter = false;
		status = hdd_open_adapter_no_trans(hdd_ctx, QDF_STA_MODE,
						   "null", mac_addr, &params);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Failed to register link adapter:%d", status);
	}

	qdf_mem_zero(&params, sizeof(struct hdd_adapter_create_param));
	params.only_wdev_register  = true;
	params.associate_with_ml_adapter = true;
	mac_addr = wlan_hdd_get_intf_addr(hdd_ctx, QDF_STA_MODE);
	if (mac_addr) {
		/* if target supports MLO create a new dev */
		status = hdd_open_adapter_no_trans(hdd_ctx, QDF_STA_MODE,
						   "null", mac_addr, &params);
		if (QDF_IS_STATUS_ERROR(status))
			hdd_err("Failed to register link adapter:%d", status);
	}
}

void hdd_update_dynamic_mld_mac_addr(struct hdd_context *hdd_ctx,
				     struct qdf_mac_addr *curr_mac_addr,
				     struct qdf_mac_addr *new_mac_addr,
				     uint8_t device_mode)
{
	uint8_t i;
	struct hdd_mld_mac_info *mac_info;

	hdd_enter();

	mac_info = &hdd_ctx->mld_mac_info;
	for (i = 0; i < WLAN_MAX_MLD; i++) {
		if (device_mode != QDF_STA_MODE)
			continue;
		if (!qdf_mem_cmp(curr_mac_addr->bytes,
				 mac_info->mld_mac_list[i].mld_addr.bytes,
				 sizeof(struct qdf_mac_addr))) {
			qdf_mem_copy(mac_info->mld_mac_list[i].mld_addr.bytes,
				     new_mac_addr->bytes,
				     sizeof(struct qdf_mac_addr));
			break;
		}
	}
	hdd_exit();
}

/**
 * MLD address can be shared with the same device mode but should be different
 * across device modes. Return error if the macaddress is currently held by a
 * different device mode.
 */
static
QDF_STATUS hdd_check_for_existing_mldaddr(struct hdd_context *hdd_ctx,
					  uint8_t *mac_addr,
					  uint32_t device_mode)
{
	struct hdd_adapter *adapter, *next_adapter = NULL;
	wlan_net_dev_ref_dbgid dbgid = NET_DEV_HOLD_CHECK_FOR_EXISTING_MACADDR;

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		if (adapter->device_mode != device_mode &&
		    !qdf_mem_cmp(adapter->mac_addr.bytes, mac_addr,
		    sizeof(struct qdf_mac_addr))) {
			hdd_adapter_dev_put_debug(adapter, dbgid);
			if (next_adapter)
				hdd_adapter_dev_put_debug(next_adapter, dbgid);
			return QDF_STATUS_E_FAILURE;
		}
		hdd_adapter_dev_put_debug(adapter, dbgid);
	}

	return QDF_STATUS_SUCCESS;
}

void
hdd_populate_mld_vdev_params(struct hdd_adapter *adapter,
			     struct wlan_vdev_create_params *vdev_params)
{
	uint8_t *mld_addr;
	QDF_STATUS qdf_status;
	uint8_t device_mode = adapter->device_mode;

	mld_addr = wlan_hdd_get_mld_addr(adapter->hdd_ctx,
					 adapter->device_mode);
	if (mld_addr) {
		/**
		 * Check if this mld address is getting used by any other
		 * device mode netdev
		 */
		qdf_status = hdd_check_for_existing_mldaddr(adapter->hdd_ctx,
							    mld_addr,
							    device_mode);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			qdf_mem_copy(vdev_params->mldaddr,
				     adapter->mac_addr.bytes,
				     QDF_MAC_ADDR_SIZE);
			return;
		}
		qdf_mem_copy(vdev_params->mldaddr, mld_addr,
			     QDF_NET_MAC_ADDR_MAX_LEN);
		qdf_mem_copy(adapter->wdev.mld_address, mld_addr,
			     QDF_NET_MAC_ADDR_MAX_LEN);
		qdf_mem_copy(adapter->mld_addr.bytes, mld_addr,
			     QDF_NET_MAC_ADDR_MAX_LEN);
	}
}

void
hdd_adapter_set_ml_adapter(struct hdd_adapter *adapter)
{
	adapter->mlo_adapter_info.is_ml_adapter = true;
}

struct hdd_adapter *hdd_get_ml_adater(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter, *next_adapter = NULL;
	wlan_net_dev_ref_dbgid dbgid = NET_DEV_HOLD_GET_ADAPTER_BY_VDEV;

	hdd_for_each_adapter_dev_held_safe(hdd_ctx, adapter, next_adapter,
					   dbgid) {
		if (hdd_adapter_is_ml_adapter(adapter)) {
			hdd_adapter_dev_put_debug(adapter, dbgid);
			if (next_adapter)
				hdd_adapter_dev_put_debug(next_adapter,
							  dbgid);
			return adapter;
		}
		hdd_adapter_dev_put_debug(adapter, dbgid);
	}

	return NULL;
}
