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
 * DOC: wlan_hdd_power.c
 *
 * WLAN power management functions
 *
 */

/* Include files */

#include <linux/pm.h>
#include <linux/wait.h>
#include <linux/cpu.h>
#include <wlan_hdd_includes.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif
#include "qdf_types.h"
#include "sme_api.h"
#include <cds_api.h>
#include <cds_sched.h>
#include <mac_init_api.h>
#include <wlan_qct_sys.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_assoc.h>
#include <wlan_nlink_srv.h>
#include <wlan_hdd_misc.h>
#include <wlan_hdd_power.h>
#include <wlan_hdd_host_offload.h>
#include <dbglog_host.h>
#include <wlan_hdd_trace.h>
#include <wlan_hdd_p2p.h>

#include <linux/semaphore.h>
#include <wlan_hdd_hostapd.h>
#include "cfg_api.h"

#include <linux/inetdevice.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_scan.h>
#include <wlan_hdd_cfg80211.h>
#include <net/addrconf.h>
#include <wlan_hdd_ipa.h>
#include <wlan_hdd_lpass.h>

#include <wma_types.h>
#include <ol_txrx_osif_api.h>
#include "hif.h"
#include "sme_power_save_api.h"
#include "wlan_policy_mgr_api.h"
#include "cdp_txrx_flow_ctrl_v2.h"
#include "pld_common.h"
#include "wlan_hdd_driver_ops.h"
#include <wlan_logging_sock_svc.h>
#include "scheduler_api.h"
#include "cds_utils.h"

/* Preprocessor definitions and constants */
#ifdef QCA_WIFI_NAPIER_EMULATION
#define HDD_SSR_BRING_UP_TIME 3000000
#else
#define HDD_SSR_BRING_UP_TIME 30000
#endif
#define HDD_WAKE_LOCK_RESUME_DURATION 1000

/* Type declarations */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * hdd_wlan_suspend_resume_event()- send suspend/resume state
 * @state: suspend/resume state
 *
 * This Function send send suspend resume state diag event
 *
 * Return: void.
 */
void hdd_wlan_suspend_resume_event(uint8_t state)
{
	WLAN_HOST_DIAG_EVENT_DEF(suspend_state, struct host_event_suspend);
	qdf_mem_zero(&suspend_state, sizeof(suspend_state));

	suspend_state.state = state;
	WLAN_HOST_DIAG_EVENT_REPORT(&suspend_state, EVENT_WLAN_SUSPEND_RESUME);
}

/**
 * hdd_wlan_offload_event()- send offloads event
 * @type: offload type
 * @state: enabled or disabled
 *
 * This Function send offloads enable/disable diag event
 *
 * Return: void.
 */

void hdd_wlan_offload_event(uint8_t type, uint8_t state)
{
	WLAN_HOST_DIAG_EVENT_DEF(host_offload, struct host_event_offload_req);
	qdf_mem_zero(&host_offload, sizeof(host_offload));

	host_offload.offload_type = type;
	host_offload.state = state;

	WLAN_HOST_DIAG_EVENT_REPORT(&host_offload, EVENT_WLAN_OFFLOAD_REQ);
}
#endif

/* Function and variables declarations */

extern struct notifier_block hdd_netdev_notifier;

static struct timer_list ssr_timer;
static bool ssr_timer_started;

/**
 * hdd_enable_gtk_offload() - enable GTK offload
 * @adapter: pointer to the adapter
 *
 * Central function to enable GTK offload.
 *
 * Return: nothing
 */
static void hdd_enable_gtk_offload(hdd_adapter_t *adapter)
{
	QDF_STATUS status;
	status = pmo_ucfg_enable_gtk_offload_in_fwr(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to enable gtk offload");
}

/**
 * hdd_disable_gtk_offload() - disable GTK offload
 * @pAdapter:   pointer to the adapter
 *
 * Central function to disable GTK offload.
 *
 * Return: nothing
 */
static void hdd_disable_gtk_offload(hdd_adapter_t *adapter)
{
	struct pmo_gtk_rsp_req gtk_rsp_request;
	QDF_STATUS status;

	/* ensure to get gtk rsp first before disable it*/
	gtk_rsp_request.callback =
		wlan_hdd_cfg80211_update_replay_counter_callback;
	/* Passing as void* as PMO does not know legacy HDD adapter type */
	gtk_rsp_request.callback_context =
		(void *)adapter;
	status = pmo_ucfg_get_gtk_rsp(adapter->hdd_vdev,
			&gtk_rsp_request);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to send get gtk rsp status:%d", status);
		return;
	}
	hdd_debug("send get_gtk_rsp successful");
	status = pmo_ucfg_disable_gtk_offload_in_fwr(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to disable gtk offload");

}


/**
 * __wlan_hdd_ipv6_changed() - IPv6 notifier callback function
 * @nb: notifier block that was registered with the kernel
 * @data: (unused) generic data that was registered with the kernel
 * @arg: (unused) generic argument that was registered with the kernel
 *
 * This is a callback function that is registered with the kernel via
 * register_inet6addr_notifier() which allows the driver to be
 * notified when there is an IPv6 address change.
 *
 * Return: NOTIFY_DONE to indicate we don't care what happens with
 *	other callbacks
 */
static int __wlan_hdd_ipv6_changed(struct notifier_block *nb,
				   unsigned long data, void *arg)
{
	struct inet6_ifaddr *ifa = (struct inet6_ifaddr *)arg;
	struct net_device *ndev = ifa->idev->dev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	hdd_context_t *hdd_ctx;
	int errno;

	ENTER_DEV(ndev);

	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	if (adapter->dev == ndev &&
	    (adapter->device_mode == QDF_STA_MODE ||
	     adapter->device_mode == QDF_P2P_CLIENT_MODE ||
	     adapter->device_mode == QDF_NDI_MODE)) {
		hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		errno = wlan_hdd_validate_context(hdd_ctx);
		if (errno)
			goto exit;

		hdd_debug("invoking sme_dhcp_done_ind");
		sme_dhcp_done_ind(hdd_ctx->hHal, adapter->sessionId);
		schedule_work(&adapter->ipv6NotifierWorkQueue);
	}

exit:
	EXIT();

	return NOTIFY_DONE;
}

/**
 * wlan_hdd_ipv6_changed() - IPv6 change notifier callback
 * @nb: pointer to notifier block
 * @data: data
 * @arg: arg
 *
 * This is the IPv6 notifier callback function gets invoked
 * if any change in IP and then invoke the function @__wlan_hdd_ipv6_changed
 * to reconfigure the offload parameters.
 *
 * Return: 0 on success, error number otherwise.
 */
int wlan_hdd_ipv6_changed(struct notifier_block *nb,
				unsigned long data, void *arg)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_ipv6_changed(nb, data, arg);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_fill_ipv6_uc_addr() - fill IPv6 unicast addresses
 * @idev: pointer to net device
 * @ipv6addr: destination array to fill IPv6 addresses
 * @ipv6addr_type: IPv6 Address type
 * @count: number of IPv6 addresses
 *
 * This is the IPv6 utility function to populate unicast addresses.
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_fill_ipv6_uc_addr(struct inet6_dev *idev,
				uint8_t ipv6_uc_addr[][SIR_MAC_IPV6_ADDR_LEN],
				uint8_t *ipv6addr_type, uint32_t *count)
{
	struct inet6_ifaddr *ifa;
	struct list_head *p;
	uint32_t scope;

	read_lock_bh(&idev->lock);
	list_for_each(p, &idev->addr_list) {
		if (*count >= PMO_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA) {
			read_unlock_bh(&idev->lock);
			return -EINVAL;
		}
		ifa = list_entry(p, struct inet6_ifaddr, if_list);
		if (ifa->flags & IFA_F_DADFAILED)
			continue;
		scope = ipv6_addr_src_scope(&ifa->addr);
		switch (scope) {
		case IPV6_ADDR_SCOPE_GLOBAL:
		case IPV6_ADDR_SCOPE_LINKLOCAL:
			qdf_mem_copy(ipv6_uc_addr[*count], &ifa->addr.s6_addr,
				sizeof(ifa->addr.s6_addr));
			ipv6addr_type[*count] = PMO_IPV6_ADDR_UC_TYPE;
			hdd_debug("Index %d scope = %s UC-Address: %pI6",
				*count, (scope == IPV6_ADDR_SCOPE_LINKLOCAL) ?
				"LINK LOCAL" : "GLOBAL", ipv6_uc_addr[*count]);
			*count += 1;
			break;
		default:
			hdd_warn("The Scope %d is not supported", scope);
		}
	}

	read_unlock_bh(&idev->lock);
	return 0;
}

/**
 * hdd_fill_ipv6_ac_addr() - fill IPv6 anycast addresses
 * @idev: pointer to net device
 * @ipv6addr: destination array to fill IPv6 addresses
 * @ipv6addr_type: IPv6 Address type
 * @count: number of IPv6 addresses
 *
 * This is the IPv6 utility function to populate anycast addresses.
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_fill_ipv6_ac_addr(struct inet6_dev *idev,
				uint8_t ipv6_ac_addr[][SIR_MAC_IPV6_ADDR_LEN],
				uint8_t *ipv6addr_type, uint32_t *count)
{
	struct ifacaddr6 *ifaca;
	uint32_t scope;

	read_lock_bh(&idev->lock);
	for (ifaca = idev->ac_list; ifaca; ifaca = ifaca->aca_next) {
		if (*count >= PMO_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA) {
			read_unlock_bh(&idev->lock);
			return -EINVAL;
		}
		/* For anycast addr no DAD */
		scope = ipv6_addr_src_scope(&ifaca->aca_addr);
		switch (scope) {
		case IPV6_ADDR_SCOPE_GLOBAL:
		case IPV6_ADDR_SCOPE_LINKLOCAL:
			qdf_mem_copy(ipv6_ac_addr[*count], &ifaca->aca_addr,
				sizeof(ifaca->aca_addr));
			ipv6addr_type[*count] = PMO_IPV6_ADDR_AC_TYPE;
			hdd_debug("Index %d scope = %s AC-Address: %pI6",
				*count, (scope == IPV6_ADDR_SCOPE_LINKLOCAL) ?
				"LINK LOCAL" : "GLOBAL", ipv6_ac_addr[*count]);
			*count += 1;
			break;
		default:
			hdd_warn("The Scope %d is not supported", scope);
		}
	}

	read_unlock_bh(&idev->lock);
	return 0;
}

void hdd_enable_ns_offload(hdd_adapter_t *adapter,
	enum pmo_offload_trigger trigger)
{
	struct inet6_dev *in6_dev;
	QDF_STATUS status;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;
	struct pmo_ns_req *ns_req = NULL;
	int err;

	ENTER();
	if (!psoc) {
		hdd_err("psoc is NULL");
		goto out;
	}

	in6_dev = __in6_dev_get(adapter->dev);
	if (NULL == in6_dev) {
		hdd_err("IPv6 dev does not exist. Failed to request NSOffload");
		goto out;
	}

	ns_req = qdf_mem_malloc(sizeof(*ns_req));
	if (!ns_req) {
		hdd_err("fail to allocate ns_req");
		goto out;
	}

	ns_req->psoc = psoc;
	ns_req->vdev_id = adapter->sessionId;
	ns_req->trigger = trigger;
	ns_req->count = 0;

	/* Unicast Addresses */
	err = hdd_fill_ipv6_uc_addr(in6_dev, ns_req->ipv6_addr,
		ns_req->ipv6_addr_type, &ns_req->count);
	if (err) {
		hdd_disable_ns_offload(adapter, trigger);
		hdd_debug("Reached max supported addresses and not enabling "
			"NS offload");
		goto out;
	}

	/* Anycast Addresses */
	err = hdd_fill_ipv6_ac_addr(in6_dev, ns_req->ipv6_addr,
		ns_req->ipv6_addr_type, &ns_req->count);
	if (err) {
		hdd_disable_ns_offload(adapter, trigger);
		hdd_debug("Reached max supported addresses and not enabling "
			"NS offload");
		goto out;
	}

	/* cache ns request */
	status = pmo_ucfg_cache_ns_offload_req(ns_req);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to cache ns request status: %d", status);
		goto out;
	}

	/* enable ns request */
	status = pmo_ucfg_enable_ns_offload_in_fwr(adapter->hdd_vdev, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Failed to enable HostOffload feature with status: %d",
			status);
	else
		hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD, SIR_OFFLOAD_ENABLE);
out:
	if (ns_req)
		qdf_mem_free(ns_req);
	EXIT();

}

void hdd_disable_ns_offload(hdd_adapter_t *adapter,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;

	ENTER();
	status = pmo_ucfg_flush_ns_offload_req(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to flush NS Offload");
		goto out;
	}

	status = pmo_ucfg_disable_ns_offload_in_fwr(adapter->hdd_vdev, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Failed to disable NS Offload");
	else
		hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD,
			SIR_OFFLOAD_DISABLE);
out:
	EXIT();

}

/**
 * __hdd_ipv6_notifier_work_queue() - IPv6 notification work function
 * @work: registered work item
 *
 * This function performs the work initially trigged by a callback
 * from the IPv6 netdev notifier.  Since this means there has been a
 * change in IPv6 state for the interface, the NS offload is
 * reconfigured.
 *
 * Return: None
 */
static void __hdd_ipv6_notifier_work_queue(struct work_struct *work)
{
	hdd_context_t *hdd_ctx;
	hdd_adapter_t *adapter;
	int errno;

	ENTER();

	adapter = container_of(work, hdd_adapter_t, ipv6NotifierWorkQueue);
	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	hdd_enable_ns_offload(adapter, pmo_ipv6_change_notify);

exit:
	EXIT();
}

/**
 * hdd_ipv6_notifier_work_queue() - IP V6 change notifier work handler
 * @work: Pointer to work context
 *
 * Return: none
 */
void hdd_ipv6_notifier_work_queue(struct work_struct *work)
{
	cds_ssr_protect(__func__);
	__hdd_ipv6_notifier_work_queue(work);
	cds_ssr_unprotect(__func__);
}

static void hdd_enable_hw_filter(hdd_adapter_t *adapter)
{
	QDF_STATUS status;

	ENTER();

	status = pmo_ucfg_enable_hw_filter_in_fwr(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to enable hardware filter");

	EXIT();
}

static void hdd_disable_hw_filter(hdd_adapter_t *adapter)
{
	QDF_STATUS status;

	ENTER();

	status = pmo_ucfg_disable_hw_filter_in_fwr(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to disable hardware filter");

	EXIT();
}

void hdd_enable_host_offloads(hdd_adapter_t *adapter,
	enum pmo_offload_trigger trigger)
{
	ENTER();

	if (!pmo_ucfg_is_vdev_supports_offload(adapter->hdd_vdev)) {
		hdd_info("offload is not supported on this vdev opmode: %d",
			 adapter->device_mode);
		goto out;
	}

	if (!pmo_ucfg_is_vdev_connected(adapter->hdd_vdev)) {
		hdd_info("vdev is not connected");
		goto out;
	}

	hdd_info("enable offloads");
	hdd_enable_gtk_offload(adapter);
	hdd_enable_arp_offload(adapter, trigger);
	hdd_enable_ns_offload(adapter, trigger);
	hdd_enable_mc_addr_filtering(adapter, trigger);
	hdd_enable_hw_filter(adapter);
out:
	EXIT();

}

void hdd_disable_host_offloads(hdd_adapter_t *adapter,
	enum pmo_offload_trigger trigger)
{
	ENTER();

	if (!pmo_ucfg_is_vdev_supports_offload(adapter->hdd_vdev)) {
		hdd_info("offload is not supported on this vdev opmode: %d",
				adapter->device_mode);
			goto out;
	}

	if (!pmo_ucfg_is_vdev_connected(adapter->hdd_vdev)) {
		hdd_info("vdev is not connected");
		goto out;
	}

	hdd_info("disable offloads");
	hdd_disable_gtk_offload(adapter);
	hdd_disable_arp_offload(adapter, trigger);
	hdd_disable_ns_offload(adapter, trigger);
	hdd_disable_mc_addr_filtering(adapter, trigger);
	hdd_disable_hw_filter(adapter);
out:
	EXIT();

}

/**
 * hdd_lookup_ifaddr() - Lookup interface address data by name
 * @adapter: the adapter whose name should be searched for
 *
 * return in_ifaddr pointer on success, NULL for failure
 */
static struct in_ifaddr *hdd_lookup_ifaddr(hdd_adapter_t *adapter)
{
	struct in_ifaddr *ifa;
	struct in_device *in_dev;

	if (!adapter) {
		hdd_err("adapter is null");
		return NULL;
	}

	in_dev = __in_dev_get_rtnl(adapter->dev);
	if (!in_dev) {
		hdd_err("Failed to get in_device");
		return NULL;
	}

	/* lookup address data by interface name */
	for (ifa = in_dev->ifa_list; ifa; ifa = ifa->ifa_next) {
		if (!strcmp(adapter->dev->name, ifa->ifa_label))
			return ifa;
	}

	return NULL;
}

/**
 * hdd_populate_ipv4_addr() - Populates the adapter's IPv4 address
 * @adapter: the adapter whose IPv4 address is desired
 * @ipv4_addr: the address of the array to copy the IPv4 address into
 *
 * return: zero for success; non-zero for failure
 */
static int hdd_populate_ipv4_addr(hdd_adapter_t *adapter, uint8_t *ipv4_addr)
{
	struct in_ifaddr *ifa;
	int i;

	if (!adapter) {
		hdd_err("adapter is null");
		return -EINVAL;
	}

	if (!ipv4_addr) {
		hdd_err("ipv4_addr is null");
		return -EINVAL;
	}

	ifa = hdd_lookup_ifaddr(adapter);
	if (!ifa || !ifa->ifa_local) {
		hdd_err("ipv4 address not found");
		return -EINVAL;
	}

	/* convert u32 to byte array */
	for (i = 0; i < 4; i++)
		ipv4_addr[i] = (ifa->ifa_local >> i * 8) & 0xff;

	return 0;
}

/**
 * hdd_set_grat_arp_keepalive() - Enable grat APR keepalive
 * @adapter: the HDD adapter to configure
 *
 * This configures gratuitous APR keepalive based on the adapter's current
 * connection information, specifically IPv4 address and BSSID
 *
 * return: zero for success, non-zero for failure
 */
static int hdd_set_grat_arp_keepalive(hdd_adapter_t *adapter)
{
	QDF_STATUS status;
	int exit_code;
	hdd_context_t *hdd_ctx;
	hdd_station_ctx_t *sta_ctx;
	tSirKeepAliveReq req = {
		.packetType = SIR_KEEP_ALIVE_UNSOLICIT_ARP_RSP,
		.dest_macaddr = QDF_MAC_ADDR_BROADCAST_INITIALIZER,
	};

	if (!adapter) {
		hdd_err("adapter is null");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd_ctx is null");
		return -EINVAL;
	}

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (!sta_ctx) {
		hdd_err("sta_ctx is null");
		return -EINVAL;
	}

	exit_code = hdd_populate_ipv4_addr(adapter, req.hostIpv4Addr);
	if (exit_code) {
		hdd_err("Failed to populate ipv4 address");
		return exit_code;
	}

	/* according to RFC5227, sender/target ip address should be the same */
	qdf_mem_copy(&req.destIpv4Addr, &req.hostIpv4Addr,
		     sizeof(req.destIpv4Addr));

	qdf_copy_macaddr(&req.bssid, &sta_ctx->conn_info.bssId);
	req.timePeriod = hdd_ctx->config->infraStaKeepAlivePeriod;
	req.sessionId = adapter->sessionId;

	hdd_debug("Setting gratuitous ARP keepalive; ipv4_addr:%u.%u.%u.%u",
		 req.hostIpv4Addr[0], req.hostIpv4Addr[1],
		 req.hostIpv4Addr[2], req.hostIpv4Addr[3]);

	status = sme_set_keep_alive(hdd_ctx->hHal, req.sessionId, &req);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to set keepalive");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

/**
 * __hdd_ipv4_notifier_work_queue() - IPv4 notification work function
 * @work: registered work item
 *
 * This function performs the work initially trigged by a callback
 * from the IPv4 netdev notifier.  Since this means there has been a
 * change in IPv4 state for the interface, the ARP offload is
 * reconfigured.
 *
 * Return: None
 */
static void __hdd_ipv4_notifier_work_queue(struct work_struct *work)
{
	hdd_context_t *hdd_ctx;
	hdd_adapter_t *adapter;
	int errno;

	ENTER();

	adapter = container_of(work, hdd_adapter_t, ipv4NotifierWorkQueue);
	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	hdd_enable_arp_offload(adapter, pmo_ipv4_change_notify);

	if (hdd_ctx->config->sta_keepalive_method == HDD_STA_KEEPALIVE_GRAT_ARP)
		hdd_set_grat_arp_keepalive(adapter);

exit:
	EXIT();
}

/**
 * hdd_ipv4_notifier_work_queue() - IP V4 change notifier work handler
 * @work: Pointer to work context
 *
 * Return: none
 */
void hdd_ipv4_notifier_work_queue(struct work_struct *work)
{
	cds_ssr_protect(__func__);
	__hdd_ipv4_notifier_work_queue(work);
	cds_ssr_unprotect(__func__);
}

/**
 * __wlan_hdd_ipv4_changed() - IPv4 notifier callback function
 * @nb: notifier block that was registered with the kernel
 * @data: (unused) generic data that was registered with the kernel
 * @arg: (unused) generic argument that was registered with the kernel
 *
 * This is a callback function that is registered with the kernel via
 * register_inetaddr_notifier() which allows the driver to be
 * notified when there is an IPv4 address change.
 *
 * Return: NOTIFY_DONE to indicate we don't care what happens with
 *	other callbacks
 */
static int __wlan_hdd_ipv4_changed(struct notifier_block *nb,
				 unsigned long data, void *arg)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)arg;
	struct net_device *ndev = ifa->ifa_dev->dev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	hdd_context_t *hdd_ctx;
	int errno;

	ENTER_DEV(ndev);

	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	if (adapter->dev == ndev &&
	    (adapter->device_mode == QDF_STA_MODE ||
	     adapter->device_mode == QDF_P2P_CLIENT_MODE ||
	     adapter->device_mode == QDF_NDI_MODE)) {

		hdd_ctx = WLAN_HDD_GET_CTX(adapter);
		errno = wlan_hdd_validate_context(hdd_ctx);
		if (errno)
			goto exit;

		hdd_debug("invoking sme_dhcp_done_ind");
		sme_dhcp_done_ind(hdd_ctx->hHal, adapter->sessionId);

		if (!hdd_ctx->config->fhostArpOffload) {
			hdd_debug("Offload not enabled ARPOffload=%d",
				  hdd_ctx->config->fhostArpOffload);
			goto exit;
		}

		ifa = hdd_lookup_ifaddr(adapter);
		if (ifa && ifa->ifa_local)
			schedule_work(&adapter->ipv4NotifierWorkQueue);
	}

exit:
	EXIT();

	return NOTIFY_DONE;
}

/**
 * wlan_hdd_ipv4_changed() - IPv4 change notifier callback
 * @nb: pointer to notifier block
 * @data: data
 * @arg: arg
 *
 * This is the IPv4 notifier callback function gets invoked
 * if any change in IP and then invoke the function @__wlan_hdd_ipv4_changed
 * to reconfigure the offload parameters.
 *
 * Return: 0 on success, error number otherwise.
 */
int wlan_hdd_ipv4_changed(struct notifier_block *nb,
			unsigned long data, void *arg)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_ipv4_changed(nb, data, arg);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_get_ipv4_local_interface() - get ipv4 local interafce from iface list
 * @pAdapter: Adapter context for which ARP offload is to be configured
 *
 * Return:
 *	ifa - on successful operation,
 *	NULL - on failure of operation
 */
static struct in_ifaddr *hdd_get_ipv4_local_interface(
				hdd_adapter_t *pAdapter)
{
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct in_device *in_dev;

	in_dev = __in_dev_get_rtnl(pAdapter->dev);
	if (in_dev) {
		for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
			     ifap = &ifa->ifa_next) {
			if (!strcmp(pAdapter->dev->name, ifa->ifa_label)) {
				/* if match break */
				return ifa;
			}
		}
	}
	ifa = NULL;

	return ifa;
}

void hdd_enable_arp_offload(hdd_adapter_t *adapter,
		enum pmo_offload_trigger trigger)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;
	QDF_STATUS status;
	struct pmo_arp_req *arp_req = NULL;
	struct in_ifaddr *ifa = NULL;

	ENTER();
	arp_req = qdf_mem_malloc(sizeof(*arp_req));
	if (!arp_req) {
		hdd_err("cannot allocate arp_req");
		goto out;
	}

	arp_req->psoc = psoc;
	arp_req->vdev_id = adapter->sessionId;
	arp_req->trigger = trigger;

	ifa = hdd_get_ipv4_local_interface(adapter);
	if (ifa && ifa->ifa_local) {
		arp_req->ipv4_addr = (uint32_t)ifa->ifa_local;
	status = pmo_ucfg_cache_arp_offload_req(arp_req);
	if (status == QDF_STATUS_SUCCESS) {
		status = pmo_ucfg_enable_arp_offload_in_fwr(
				adapter->hdd_vdev, trigger);
		if (status == QDF_STATUS_SUCCESS)
			hdd_wlan_offload_event(
				PMO_IPV4_ARP_REPLY_OFFLOAD,
				PMO_OFFLOAD_ENABLE);
		else
			hdd_info("fail to enable arp offload in fwr");
	} else
		hdd_info("fail to cache arp offload request");
	} else {
		hdd_notice("IP Address is not assigned");
		status = QDF_STATUS_NOT_INITIALIZED;
	}
out:
	if (arp_req)
		qdf_mem_free(arp_req);
	EXIT();

}

void hdd_disable_arp_offload(hdd_adapter_t *adapter,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;

	ENTER();
	status = pmo_ucfg_flush_arp_offload_req(adapter->hdd_vdev);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to flush arp Offload");
		goto out;
	}

	status = pmo_ucfg_disable_arp_offload_in_fwr(adapter->hdd_vdev, trigger);
	if (status == QDF_STATUS_SUCCESS)
		hdd_wlan_offload_event(PMO_IPV4_ARP_REPLY_OFFLOAD,
			PMO_OFFLOAD_DISABLE);
	else
		hdd_info("fail to disable arp offload");
out:
	EXIT();
}

void hdd_enable_mc_addr_filtering(hdd_adapter_t *adapter,
		enum pmo_offload_trigger trigger)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *)adapter->pHddCtx;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;

	ENTER();
	if (wlan_hdd_validate_context(hdd_ctx))
		goto out;

	status = pmo_ucfg_enable_mc_addr_filtering_in_fwr(psoc,
				adapter->sessionId, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("failed to enable mc list status %d", status);
out:
	EXIT();

}

void hdd_disable_mc_addr_filtering(hdd_adapter_t *adapter,
		enum pmo_offload_trigger trigger)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *)adapter->pHddCtx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;

	ENTER();
	if (wlan_hdd_validate_context(hdd_ctx))
		goto out;

	status = pmo_ucfg_disable_mc_addr_filtering_in_fwr(psoc,
				adapter->sessionId, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("failed to disable mc list status %d", status);
out:
	EXIT();

}

int hdd_cache_mc_addr_list(struct pmo_mc_addr_list_params *mc_list_config)
{
	QDF_STATUS status;
	int ret = 0;

	ENTER();
	/* cache mc addr list */
	status = pmo_ucfg_cache_mc_addr_list(mc_list_config);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_info("fail to cache mc list status %d", status);
		ret = -EINVAL;
	}
	EXIT();

	return ret;
}

void hdd_disable_and_flush_mc_addr_list(hdd_adapter_t *adapter,
	enum pmo_offload_trigger trigger)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *)adapter->pHddCtx;
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	ENTER();
	/* disable mc list first */
	status = pmo_ucfg_disable_mc_addr_filtering_in_fwr(psoc,
			adapter->sessionId, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("fail to disable mc list");

	/* flush mc list */
	status = pmo_ucfg_flush_mc_addr_list(psoc, adapter->sessionId);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("fail to flush mc list status %d", status);
	EXIT();

	return;

}

/**
 * hdd_update_conn_state_mask(): record info needed by wma_suspend_req
 * @adapter: adapter to get info from
 * @conn_state_mask: mask of connection info
 *
 * currently only need to send connection info.
 */
static void
hdd_update_conn_state_mask(hdd_adapter_t *adapter, uint32_t *conn_state_mask)
{

	eConnectionState connState;
	hdd_station_ctx_t *sta_ctx;
	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	connState = sta_ctx->conn_info.connState;

	if (connState == eConnectionState_Associated ||
			connState == eConnectionState_IbssConnected)
		*conn_state_mask |= (1 << adapter->sessionId);
}

/**
 * hdd_suspend_wlan() - Driver suspend function
 * @callback: Callback function to invoke when driver is ready to suspend
 * @callbackContext: Context to pass back to @callback function
 *
 * Return: 0 on success else error code.
 */
static int
hdd_suspend_wlan(void)
{
	hdd_context_t *pHddCtx;

	QDF_STATUS status;
	hdd_adapter_t *pAdapter = NULL;
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	uint32_t conn_state_mask = 0;
	hdd_info("WLAN being suspended by OS");

	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}

	if (cds_is_driver_recovering()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore suspend!!!",
			 cds_get_driver_state());
		return -EINVAL;
	}

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
			hdd_err("invalid session id: %d", pAdapter->sessionId);
			goto next_adapter;
		}

		/* stop all TX queues before suspend */
		hdd_debug("Disabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					     WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_CONTROL_PATH);

		/* Configure supported OffLoads */
		hdd_enable_host_offloads(pAdapter, pmo_apps_suspend);
		hdd_update_conn_state_mask(pAdapter, &conn_state_mask);
next_adapter:
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	status = pmo_ucfg_psoc_user_space_suspend_req(pHddCtx->hdd_psoc,
			QDF_SYSTEM_SUSPEND);
	if (status != QDF_STATUS_SUCCESS)
		return -EAGAIN;

	pHddCtx->hdd_wlan_suspended = true;
	hdd_wlan_suspend_resume_event(HDD_WLAN_EARLY_SUSPEND);

	return 0;
}

/**
 * hdd_resume_wlan() - Driver resume function
 *
 * Return: 0 on success else error code.
 */
static int hdd_resume_wlan(void)
{
	hdd_context_t *pHddCtx;
	hdd_adapter_t *pAdapter = NULL;
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;

	hdd_info("WLAN being resumed by OS");

	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}

	if (cds_is_driver_recovering()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore resume!!!",
			 cds_get_driver_state());
		return -EINVAL;
	}

	pHddCtx->hdd_wlan_suspended = false;
	hdd_wlan_suspend_resume_event(HDD_WLAN_EARLY_RESUME);

	/*loop through all adapters. Concurrency */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);

	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
			hdd_err("invalid session id: %d", pAdapter->sessionId);
			goto next_adapter;
		}
		/* Disable supported OffLoads */
		hdd_disable_host_offloads(pAdapter, pmo_apps_resume);

		/* wake the tx queues */
		hdd_info("Enabling queues");
		wlan_hdd_netif_queue_control(pAdapter,
					WLAN_WAKE_ALL_NETIF_QUEUE,
					WLAN_CONTROL_PATH);

next_adapter:
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	hdd_ipa_resume(pHddCtx);
	status = pmo_ucfg_psoc_user_space_resume_req(pHddCtx->hdd_psoc,
			QDF_SYSTEM_SUSPEND);
	if (status != QDF_STATUS_SUCCESS)
		return -EAGAIN;

	return 0;
}

/**
 * DOC: SSR Timer
 *
 * When SSR is initiated, an SSR timer is started.  Under normal
 * circumstances SSR should complete amd the timer should be deleted
 * before it fires.  If the SSR timer does fire, it indicates SSR has
 * taken too long, and our only recourse is to invoke the QDF_BUG()
 * API which can allow a crashdump to be captured.
 */

/**
 * hdd_ssr_timer_init() - Initialize SSR Timer
 *
 * Return: None.
 */
static void hdd_ssr_timer_init(void)
{
	init_timer(&ssr_timer);
}

/**
 * hdd_ssr_timer_del() - Delete SSR Timer
 *
 * Return: None.
 */
static void hdd_ssr_timer_del(void)
{
	del_timer(&ssr_timer);
	ssr_timer_started = false;
}

/**
 * hdd_ssr_timer_cb() - SSR Timer callback function
 * @data: opaque data registered with timer infrastructure
 *
 * Return: None.
 */
static void hdd_ssr_timer_cb(unsigned long data)
{
	hdd_info("HDD SSR timer expired!");
	QDF_BUG(0);
}

/**
 * hdd_ssr_timer_start() - Start SSR Timer
 * @msec: Timer timeout value in milliseconds
 *
 * Return: None.
 */
static void hdd_ssr_timer_start(int msec)
{
	if (ssr_timer_started) {
		hdd_debug("Trying to start SSR timer when " "it's running!");
	}
	ssr_timer.expires = jiffies + msecs_to_jiffies(msec);
	ssr_timer.function = hdd_ssr_timer_cb;
	add_timer(&ssr_timer);
	ssr_timer_started = true;
}

/**
 * hdd_svc_fw_shutdown_ind() - API to send FW SHUTDOWN IND to Userspace
 *
 * @dev: Device Pointer
 *
 * Return: None
 */
void hdd_svc_fw_shutdown_ind(struct device *dev)
{
	hdd_context_t *hdd_ctx;
	v_CONTEXT_t g_context;

	g_context = cds_get_global_context();

	if (!g_context)
		return;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	hdd_ctx ? wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					      WLAN_SVC_FW_SHUTDOWN_IND,
					      NULL, 0) : 0;
}

/**
 * hdd_ssr_restart_sap() - restart sap on SSR
 * @hdd_ctx:   hdd context
 *
 * Return:     nothing
 */
static void hdd_ssr_restart_sap(hdd_context_t *hdd_ctx)
{
	QDF_STATUS  status;
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	hdd_adapter_t *adapter;

	ENTER();

	status =  hdd_get_front_adapter(hdd_ctx, &adapter_node);
	while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
		adapter = adapter_node->pAdapter;
		if (adapter && adapter->device_mode == QDF_SAP_MODE) {
			if (test_bit(SOFTAP_INIT_DONE, &adapter->event_flags)) {
				hdd_debug("Restart prev SAP session");
				wlan_hdd_start_sap(adapter, true);
			}
		}
		status = hdd_get_next_adapter(hdd_ctx, adapter_node, &next);
		adapter_node = next;
	}

	EXIT();
}

/**
 * hdd_wlan_shutdown() - HDD SSR shutdown function
 *
 * This function is called by the HIF to shutdown the driver during SSR.
 *
 * Return: QDF_STATUS_SUCCESS if the driver was shut down,
 *	or an error status otherwise
 */
QDF_STATUS hdd_wlan_shutdown(void)
{
	v_CONTEXT_t p_cds_context = NULL;
	hdd_context_t *pHddCtx;
	p_cds_sched_context cds_sched_context = NULL;
	QDF_STATUS qdf_status;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	hdd_info("WLAN driver shutting down!");

	/* If SSR never completes, then do kernel panic. */
	hdd_ssr_timer_init();
	hdd_ssr_timer_start(HDD_SSR_BRING_UP_TIME);

	/* Get the global CDS context. */
	p_cds_context = cds_get_global_context();
	if (!p_cds_context) {
		hdd_err("Global CDS context is Null");
		return QDF_STATUS_E_FAILURE;
	}

	/* Get the HDD context. */
	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hdd_err("HDD context is Null");
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_clear_concurrent_session_count(pHddCtx->hdd_psoc);

	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump();
	wlan_cfg80211_cleanup_scan_queue(pHddCtx->hdd_pdev);
	hdd_ipa_uc_ssr_deinit();
	hdd_reset_all_adapters(pHddCtx);

	/* Flush cached rx frame queue */
	if (soc)
		cdp_flush_cache_rx_queue(soc);

	/* De-register the HDD callbacks */
	hdd_deregister_cb(pHddCtx);

	cds_sched_context = get_cds_sched_ctxt();

	if (pHddCtx->is_scheduler_suspended) {
		scheduler_resume();
		pHddCtx->is_scheduler_suspended = false;
	}
#ifdef QCA_CONFIG_SMP
	if (true == pHddCtx->is_ol_rx_thread_suspended) {
		complete(&cds_sched_context->ol_resume_rx_event);
		pHddCtx->is_ol_rx_thread_suspended = false;
	}
#endif

	qdf_status = cds_sched_close(p_cds_context);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Failed to close CDS Scheduler");
		QDF_ASSERT(false);
	}

	qdf_mc_timer_stop(&pHddCtx->tdls_source_timer);

	hdd_bus_bandwidth_destroy(pHddCtx);

	hdd_wlan_stop_modules(pHddCtx, false);

	hdd_lpass_notify_stop(pHddCtx);

	hdd_info("WLAN driver shutdown complete");
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
* hdd_wlan_ssr_reinit_event()- send ssr reinit state
*
* This Function send send ssr reinit state diag event
*
* Return: void.
*/
static void hdd_wlan_ssr_reinit_event(void)
{
	WLAN_HOST_DIAG_EVENT_DEF(ssr_reinit, struct host_event_wlan_ssr_reinit);
	qdf_mem_zero(&ssr_reinit, sizeof(ssr_reinit));
	ssr_reinit.status = SSR_SUB_SYSTEM_REINIT;
	WLAN_HOST_DIAG_EVENT_REPORT(&ssr_reinit,
					EVENT_WLAN_SSR_REINIT_SUBSYSTEM);
}
#else
static inline void hdd_wlan_ssr_reinit_event(void)
{

}
#endif

/**
 * hdd_wlan_re_init() - HDD SSR re-init function
 *
 * This function is called by the HIF to re-initialize the driver after SSR.
 *
 * Return: QDF_STATUS_SUCCESS if the driver was re-initialized,
 *	or an error status otherwise
 */
QDF_STATUS hdd_wlan_re_init(void)
{

	v_CONTEXT_t p_cds_context = NULL;
	hdd_context_t *pHddCtx = NULL;
	hdd_adapter_t *pAdapter;
	int ret;
	bool bug_on_reinit_failure = CFG_BUG_ON_REINIT_FAILURE_DEFAULT;

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);

	/* Get the CDS context */
	p_cds_context = cds_get_global_context();
	if (p_cds_context == NULL) {
		hdd_err("Failed cds_get_global_context");
		goto err_re_init;
	}

	/* Get the HDD context */
	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hdd_err("HDD context is Null");
		goto err_re_init;
	}
	bug_on_reinit_failure = pHddCtx->config->bug_on_reinit_failure;

	/* The driver should always be initialized in STA mode after SSR */
	hdd_set_conparam(0);
	/* Try to get an adapter from mode ID */
	pAdapter = hdd_get_adapter(pHddCtx, QDF_STA_MODE);
	if (!pAdapter) {
		pAdapter = hdd_get_adapter(pHddCtx, QDF_SAP_MODE);
		if (!pAdapter) {
			pAdapter = hdd_get_adapter(pHddCtx, QDF_IBSS_MODE);
			if (!pAdapter) {
				hdd_err("Failed to get Adapter!");
			}
		}
	}

	if (pHddCtx->config->enable_dp_trace)
		qdf_dp_trace_init();

	hdd_bus_bandwidth_init(pHddCtx);


	ret = hdd_wlan_start_modules(pHddCtx, pAdapter, true);
	if (ret) {
		hdd_err("Failed to start wlan after error");
		goto err_wiphy_unregister;
	}

	hdd_wlan_get_version(pHddCtx, NULL, NULL);

	wlan_hdd_send_svc_nlink_msg(pHddCtx->radio_index,
				WLAN_SVC_FW_CRASHED_IND, NULL, 0);

	/* Restart all adapters */
	hdd_start_all_adapters(pHddCtx);

	pHddCtx->last_scan_reject_session_id = 0xFF;
	pHddCtx->last_scan_reject_reason = 0;
	pHddCtx->last_scan_reject_timestamp = 0;

	hdd_set_roaming_in_progress(false);
	pHddCtx->btCoexModeSet = false;

	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);

	ret = hdd_register_cb(pHddCtx);
	if (ret) {
		hdd_err("Failed to register HDD callbacks!");
		goto err_cds_disable;
	}

	hdd_lpass_notify_start(pHddCtx);

	hdd_info("WLAN host driver reinitiation completed!");
	goto success;

err_cds_disable:
	hdd_wlan_stop_modules(pHddCtx, false);

err_wiphy_unregister:
	if (bug_on_reinit_failure)
		QDF_BUG(0);

	if (pHddCtx) {
		/* Unregister the Notifier's */
		hdd_unregister_notifiers(pHddCtx);
		ptt_sock_deactivate_svc();
		nl_srv_exit();

		hdd_close_all_adapters(pHddCtx, false);
		wlan_hdd_cfg80211_deinit(pHddCtx->wiphy);
		hdd_lpass_notify_stop(pHddCtx);
		wlan_hdd_deinit_tx_rx_histogram(pHddCtx);
		wiphy_unregister(pHddCtx->wiphy);

	}

err_re_init:
	hdd_ssr_timer_del();
	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);
	if (bug_on_reinit_failure)
		QDF_BUG(0);
	return -EPERM;

success:
	if (pHddCtx->config->sap_internal_restart)
		hdd_ssr_restart_sap(pHddCtx);
	hdd_ssr_timer_del();
	hdd_wlan_ssr_reinit_event();
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_set_powersave() - Set powersave mode
 * @adapter: adapter upon which the request was received
 * @allow_power_save: is wlan allowed to go into power save mode
 * @timeout: timeout period in ms
 *
 * Return: 0 on success, non-zero on any error
 */
static int wlan_hdd_set_powersave(hdd_adapter_t *adapter,
	bool allow_power_save, uint32_t timeout)
{
	tHalHandle hal;
	hdd_context_t *hdd_ctx;
	bool force_trigger = false;

	if (NULL == adapter) {
		hdd_err("Adapter NULL");
		return -ENODEV;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd context is NULL");
		return -EINVAL;
	}

	hdd_debug("Allow power save: %d", allow_power_save);
	hal = WLAN_HDD_GET_HAL_CTX(adapter);

	if ((QDF_STA_MODE == adapter->device_mode) &&
	    !adapter->sessionCtx.station.ap_supports_immediate_power_save) {
		/* override user's requested flag */
		force_trigger = allow_power_save;
		allow_power_save = false;
		timeout = AUTO_PS_ENTRY_USER_TIMER_DEFAULT_VALUE;
		hdd_debug("Defer power-save for few seconds...");
	}

	if (allow_power_save) {
		if (QDF_STA_MODE == adapter->device_mode ||
		    QDF_P2P_CLIENT_MODE == adapter->device_mode) {
			hdd_debug("Disabling Auto Power save timer");
			sme_ps_disable_auto_ps_timer(
				WLAN_HDD_GET_HAL_CTX(adapter),
				adapter->sessionId);
		}

		if (hdd_ctx->config && hdd_ctx->config->is_ps_enabled) {
			hdd_debug("Wlan driver Entering Power save");

			/*
			 * Enter Power Save command received from GUI
			 * this means DHCP is completed
			 */
			sme_ps_enable_disable(hal, adapter->sessionId,
					SME_PS_ENABLE);
		} else {
			hdd_debug("Power Save is not enabled in the cfg");
		}
	} else {
		hdd_debug("Wlan driver Entering Full Power");

		/*
		 * Enter Full power command received from GUI
		 * this means we are disconnected
		 */
		sme_ps_disable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId);
		sme_ps_enable_disable(hal, adapter->sessionId, SME_PS_DISABLE);
		sme_ps_enable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId, timeout, force_trigger);
	}

	return 0;
}

static void wlan_hdd_print_suspend_fail_stats(hdd_context_t *hdd_ctx)
{
	struct suspend_resume_stats *stats = &hdd_ctx->suspend_resume_stats;
	hdd_err("ipa:%d, radar:%d, roam:%d, scan:%d, initial_wakeup:%d",
		stats->suspend_fail[SUSPEND_FAIL_IPA],
		stats->suspend_fail[SUSPEND_FAIL_RADAR],
		stats->suspend_fail[SUSPEND_FAIL_ROAM],
		stats->suspend_fail[SUSPEND_FAIL_SCAN],
		stats->suspend_fail[SUSPEND_FAIL_INITIAL_WAKEUP]);
}

void wlan_hdd_inc_suspend_stats(hdd_context_t *hdd_ctx,
				enum suspend_fail_reason reason)
{
	wlan_hdd_print_suspend_fail_stats(hdd_ctx);
	hdd_ctx->suspend_resume_stats.suspend_fail[reason]++;
	wlan_hdd_print_suspend_fail_stats(hdd_ctx);
}

/**
 * __wlan_hdd_cfg80211_resume_wlan() - cfg80211 resume callback
 * @wiphy: Pointer to wiphy
 *
 * This API is called when cfg80211 driver resumes driver updates
 * latest sched_scan scan result(if any) to cfg80211 database
 *
 * Return: integer status
 */
static int __wlan_hdd_cfg80211_resume_wlan(struct wiphy *wiphy)
{
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	hdd_adapter_t *pAdapter;
	hdd_adapter_list_node_t *pAdapterNode, *pNext;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int exit_code;
	p_cds_sched_context cds_sched_context = get_cds_sched_ctxt();

	ENTER();

	if (cds_is_driver_recovering()) {
		hdd_debug("Driver is recovering; Skipping resume");
		exit_code = 0;
		goto exit_with_code;
	}

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		exit_code = -EINVAL;
		goto exit_with_code;
	}

	exit_code = wlan_hdd_validate_context(pHddCtx);
	if (exit_code) {
		hdd_err("Invalid HDD context");
		goto exit_with_code;
	}

	mutex_lock(&pHddCtx->iface_change_lock);
	if (pHddCtx->driver_status != DRIVER_MODULES_ENABLED) {
		mutex_unlock(&pHddCtx->iface_change_lock);
		hdd_debug("Driver is not enabled; Skipping resume");
		exit_code = 0;
		goto exit_with_code;
	}
	mutex_unlock(&pHddCtx->iface_change_lock);

	pld_request_bus_bandwidth(pHddCtx->parent_dev, PLD_BUS_WIDTH_MEDIUM);

	status = hdd_resume_wlan();
	if (status != QDF_STATUS_SUCCESS) {
		exit_code = 0;
		goto exit_with_code;
	}
	/* Resume control path scheduler */
	if (pHddCtx->is_scheduler_suspended) {
		scheduler_resume();
		pHddCtx->is_scheduler_suspended = false;
	}
#ifdef QCA_CONFIG_SMP
	/* Resume tlshim Rx thread */
	if (pHddCtx->is_ol_rx_thread_suspended) {
		complete(&cds_sched_context->ol_resume_rx_event);
		pHddCtx->is_ol_rx_thread_suspended = false;
	}
#endif

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_RESUME_WLAN,
			 NO_SESSION, pHddCtx->isWiphySuspended));
	qdf_spin_lock(&pHddCtx->sched_scan_lock);
	pHddCtx->isWiphySuspended = false;
	if (true != pHddCtx->isSchedScanUpdatePending) {
		qdf_spin_unlock(&pHddCtx->sched_scan_lock);
		hdd_debug("Return resume is not due to PNO indication");
		goto exit_with_success;
	}
	/* Reset flag to avoid updatating cfg80211 data old results again */
	pHddCtx->isSchedScanUpdatePending = false;
	qdf_spin_unlock(&pHddCtx->sched_scan_lock);

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		if ((NULL != pAdapter) &&
		    (QDF_STA_MODE == pAdapter->device_mode)) {
			if (0 !=
			    wlan_hdd_cfg80211_update_bss(pHddCtx->wiphy,
							 pAdapter, 0)) {
				hdd_warn("NO SCAN result");
			} else {
				/* Acquire wakelock to handle the case where
				 * APP's tries to suspend immediately after
				 * updating the scan results. Whis results in
				 * app's is in suspended state and not able to
				 * process the connect request to AP
				 */
				hdd_prevent_suspend_timeout(
					HDD_WAKE_LOCK_RESUME_DURATION,
					WIFI_POWER_EVENT_WAKELOCK_RESUME_WLAN);
				cfg80211_sched_scan_results(pHddCtx->wiphy);
			}

			hdd_debug("cfg80211 scan result database updated");
			goto exit_with_success;
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

exit_with_success:
	pHddCtx->suspend_resume_stats.resumes++;
	exit_code = 0;

exit_with_code:
	EXIT();
	return exit_code;
}

/**
 * wlan_hdd_cfg80211_resume_wlan() - cfg80211 resume callback
 * @wiphy: Pointer to wiphy
 *
 * This API is called when cfg80211 driver resumes driver updates
 * latest sched_scan scan result(if any) to cfg80211 database
 *
 * Return: integer status
 */
int wlan_hdd_cfg80211_resume_wlan(struct wiphy *wiphy)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_resume_wlan(wiphy);
	cds_ssr_unprotect(__func__);

	return ret;
}

static void hdd_suspend_cb(void)
{
	hdd_context_t *hdd_ctx;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		cds_err("HDD context is NULL");
		return;
	}

	complete(&hdd_ctx->mc_sus_event_var);
}

/**
 * __wlan_hdd_cfg80211_suspend_wlan() - cfg80211 suspend callback
 * @wiphy: Pointer to wiphy
 * @wow: Pointer to wow
 *
 * This API is called when cfg80211 driver suspends
 *
 * Return: integer status
 */
static int __wlan_hdd_cfg80211_suspend_wlan(struct wiphy *wiphy,
				     struct cfg80211_wowlan *wow)
{
#ifdef QCA_CONFIG_SMP
#define RX_TLSHIM_SUSPEND_TIMEOUT 200   /* msecs */
#endif
	hdd_context_t *pHddCtx = wiphy_priv(wiphy);
	p_cds_sched_context cds_sched_context = get_cds_sched_ctxt();
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *pAdapter;
	hdd_scaninfo_t *pScanInfo;
	QDF_STATUS status;
	int rc;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	rc = wlan_hdd_validate_context(pHddCtx);
	if (0 != rc)
		return rc;

	mutex_lock(&pHddCtx->iface_change_lock);
	if (pHddCtx->driver_status != DRIVER_MODULES_ENABLED) {
		mutex_unlock(&pHddCtx->iface_change_lock);
		hdd_debug("Driver Modules not Enabled ");
		return 0;
	}
	mutex_unlock(&pHddCtx->iface_change_lock);

	/* If RADAR detection is in progress (HDD), prevent suspend. The flag
	 * "dfs_cac_block_tx" is set to true when RADAR is found and stay true
	 * until CAC is done for a SoftAP which is in started state.
	 */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
			hdd_err("invalid session id: %d", pAdapter->sessionId);
			goto next_adapter;
		}

		if (QDF_SAP_MODE == pAdapter->device_mode) {
			if (BSS_START ==
			    WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter)->bssState &&
			    true ==
			    WLAN_HDD_GET_AP_CTX_PTR(pAdapter)->
			    dfs_cac_block_tx) {
				hdd_err("RADAR detection in progress, do not allow suspend");
				wlan_hdd_inc_suspend_stats(pHddCtx,
							   SUSPEND_FAIL_RADAR);
				return -EAGAIN;
			} else if (!pHddCtx->config->enableSapSuspend) {
				/* return -EOPNOTSUPP if SAP does not support
				 * suspend
				 */
				hdd_err("SAP does not support suspend!!");
				return -EOPNOTSUPP;
			}
		} else if (QDF_P2P_GO_MODE == pAdapter->device_mode) {
			if (!pHddCtx->config->enableSapSuspend) {
				/* return -EOPNOTSUPP if GO does not support
				 * suspend
				 */
				hdd_err("GO does not support suspend!!");
				return -EOPNOTSUPP;
			}
		}
		if (pAdapter->is_roc_inprogress)
			wlan_hdd_cleanup_remain_on_channel_ctx(pAdapter);
next_adapter:
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/* Stop ongoing scan on each interface */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		pScanInfo = &pAdapter->scan_info;

		if (sme_neighbor_middle_of_roaming
			    (pHddCtx->hHal, pAdapter->sessionId)) {
			hdd_err("Roaming in progress, do not allow suspend");
			wlan_hdd_inc_suspend_stats(pHddCtx,
						   SUSPEND_FAIL_ROAM);
			return -EAGAIN;
		}

		if (pScanInfo->mScanPending)
			wlan_abort_scan(pHddCtx->hdd_pdev, INVAL_PDEV_ID,
				pAdapter->sessionId, INVALID_SCAN_ID, true);
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/*
	 * Suspend IPA early before proceeding to suspend other entities like
	 * firmware to avoid any race conditions.
	 */
	if (hdd_ipa_suspend(pHddCtx)) {
		hdd_err("IPA not ready to suspend!");
		wlan_hdd_inc_suspend_stats(pHddCtx, SUSPEND_FAIL_IPA);
		return -EAGAIN;
	}

	/* Suspend control path scheduler */
	scheduler_register_hdd_suspend_callback(hdd_suspend_cb);
	scheduler_set_event_mask(MC_SUSPEND_EVENT);
	scheduler_wake_up_controller_thread();

	/* Wait for suspend confirmation from scheduler */
	rc = wait_for_completion_timeout(&pHddCtx->mc_sus_event_var,
		msecs_to_jiffies(WLAN_WAIT_TIME_MCTHREAD_SUSPEND));
	if (!rc) {
		scheduler_clear_event_mask(MC_SUSPEND_EVENT);
		hdd_err("Failed to stop mc thread");
		goto resume_tx;
	}
	pHddCtx->is_scheduler_suspended = true;

#ifdef QCA_CONFIG_SMP
	/* Suspend tlshim rx thread */
	set_bit(RX_SUSPEND_EVENT, &cds_sched_context->ol_rx_event_flag);
	wake_up_interruptible(&cds_sched_context->ol_rx_wait_queue);
	rc = wait_for_completion_timeout(&cds_sched_context->
					 ol_suspend_rx_event,
					 msecs_to_jiffies
						 (RX_TLSHIM_SUSPEND_TIMEOUT));
	if (!rc) {
		clear_bit(RX_SUSPEND_EVENT,
			  &cds_sched_context->ol_rx_event_flag);
		hdd_err("Failed to stop tl_shim rx thread");
		goto resume_all;
	}
	pHddCtx->is_ol_rx_thread_suspended = true;
#endif
	if (hdd_suspend_wlan() < 0)
		goto resume_all;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SUSPEND_WLAN,
			 NO_SESSION, pHddCtx->isWiphySuspended));
	pHddCtx->isWiphySuspended = true;

	pld_request_bus_bandwidth(pHddCtx->parent_dev, PLD_BUS_WIDTH_NONE);

	EXIT();
	return 0;

#ifdef QCA_CONFIG_SMP
resume_all:

	scheduler_resume();
	pHddCtx->is_scheduler_suspended = false;
#endif

resume_tx:

	hdd_resume_wlan();
	return -ETIME;

}

/**
 * wlan_hdd_cfg80211_suspend_wlan() - cfg80211 suspend callback
 * @wiphy: Pointer to wiphy
 * @wow: Pointer to wow
 *
 * This API is called when cfg80211 driver suspends
 *
 * Return: integer status
 */
int wlan_hdd_cfg80211_suspend_wlan(struct wiphy *wiphy,
				   struct cfg80211_wowlan *wow)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_suspend_wlan(wiphy, wow);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_stop_dhcp_ind() - API to stop DHCP sequence
 * @adapter: Adapter on which DHCP needs to be stopped
 *
 * Release the wakelock held for DHCP process and allow
 * the runtime pm to continue
 *
 * Return: None
 */
static void hdd_stop_dhcp_ind(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("DHCP stop indicated through power save");
	sme_dhcp_stop_ind(hdd_ctx->hHal, adapter->device_mode,
			  adapter->macAddressCurrent.bytes,
			  adapter->sessionId);
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DHCP);
	qdf_runtime_pm_allow_suspend(adapter->connect_rpm_ctx.connect);
}

/**
 * hdd_start_dhcp_ind() - API to start DHCP sequence
 * @adapter: Adapter on which DHCP needs to be stopped
 *
 * Prevent APPS suspend and the runtime suspend during
 * DHCP sequence
 *
 * Return: None
 */
static void hdd_start_dhcp_ind(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("DHCP start indicated through power save");
	qdf_runtime_pm_prevent_suspend(adapter->connect_rpm_ctx.connect);
	hdd_prevent_suspend_timeout(1000, WIFI_POWER_EVENT_WAKELOCK_DHCP);
	sme_dhcp_start_ind(hdd_ctx->hHal, adapter->device_mode,
			   adapter->macAddressCurrent.bytes,
			   adapter->sessionId);
}

/**
 * __wlan_hdd_cfg80211_set_power_mgmt() - set cfg80211 power management config
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @allow_power_save: is wlan allowed to go into power save mode
 * @timeout: Timeout value in ms
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
					      struct net_device *dev,
					      bool allow_power_save,
					      int timeout)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	if (timeout < 0) {
		hdd_debug("User space timeout: %d; Using default instead: %d",
			timeout, AUTO_PS_ENTRY_USER_TIMER_DEFAULT_VALUE);
		timeout = AUTO_PS_ENTRY_USER_TIMER_DEFAULT_VALUE;
	}

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
		hdd_err("invalid session id: %d", pAdapter->sessionId);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_POWER_MGMT,
			 pAdapter->sessionId, timeout));

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	mutex_lock(&pHddCtx->iface_change_lock);
	if (pHddCtx->driver_status != DRIVER_MODULES_ENABLED) {
		mutex_unlock(&pHddCtx->iface_change_lock);
		hdd_debug("Driver Module not enabled return success");
		return 0;
	}
	mutex_unlock(&pHddCtx->iface_change_lock);

	status = wlan_hdd_set_powersave(pAdapter, allow_power_save, timeout);

	allow_power_save ? hdd_stop_dhcp_ind(pAdapter) :
		hdd_start_dhcp_ind(pAdapter);

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_set_power_mgmt() - set cfg80211 power management config
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @allow_power_save: is wlan allowed to go into power save mode
 * @timeout: Timeout value
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
				     struct net_device *dev,
				     bool allow_power_save,
				     int timeout)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_power_mgmt(wiphy, dev,
		allow_power_save, timeout);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_set_txpower() - set TX power
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to network device
 * @type: TX power setting type
 * @dbm: TX power in dbm
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   enum nl80211_tx_power_setting type,
					   int dbm)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) wiphy_priv(wiphy);
	tHalHandle hHal = NULL;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	struct qdf_mac_addr selfMac = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_TXPOWER,
			 NO_SESSION, type));

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status)
		return status;

	hHal = pHddCtx->hHal;

	if (0 != sme_cfg_set_int(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL, dbm)) {
		hdd_err("sme_cfg_set_int failed for tx power %hu",
				dbm);
		return -EIO;
	}

	hdd_debug("Set tx power level %d dbm", dbm);

	switch (type) {
	/* Automatically determine transmit power */
	case NL80211_TX_POWER_AUTOMATIC:
	/* Fall through */
	case NL80211_TX_POWER_LIMITED:  /* Limit TX power by the mBm parameter */
		if (sme_set_max_tx_power(hHal, bssid, selfMac, dbm) !=
		    QDF_STATUS_SUCCESS) {
			hdd_err("Setting maximum tx power failed");
			return -EIO;
		}
		break;

	case NL80211_TX_POWER_FIXED:    /* Fix TX power to the mBm parameter */
		hdd_err("NL80211_TX_POWER_FIXED not supported");
		return -EOPNOTSUPP;
		break;

	default:
		hdd_err("Invalid power setting type %d", type);
		return -EIO;
	}

	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_set_txpower() - set TX power
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to network device
 * @type: TX power setting type
 * @dbm: TX power in dbm
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  enum nl80211_tx_power_setting type,
				  int dbm)
{
	int ret;
	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_txpower(wiphy,
					      wdev,
					      type, dbm);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_cfg80211_get_txpower() - get TX power
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to network device
 * @dbm: Pointer to TX power in dbm
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  int *dbm)
{

	hdd_context_t *pHddCtx = (hdd_context_t *) wiphy_priv(wiphy);
	struct net_device *ndev = wdev->netdev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status) {
		*dbm = 0;
		return status;
	}

	/* Validate adapter sessionId */
	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	mutex_lock(&pHddCtx->iface_change_lock);
	if (pHddCtx->driver_status != DRIVER_MODULES_ENABLED) {
		mutex_unlock(&pHddCtx->iface_change_lock);
		hdd_debug("Driver Module not enabled return success");
		/* Send cached data to upperlayer*/
		*dbm = adapter->hdd_stats.ClassA_stat.max_pwr;
		return 0;
	}
	mutex_unlock(&pHddCtx->iface_change_lock);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_GET_TXPOWER,
			 adapter->sessionId, adapter->device_mode));
	wlan_hdd_get_class_astats(adapter);
	*dbm = adapter->hdd_stats.ClassA_stat.max_pwr;

	EXIT();
	return 0;
}

/**
 * wlan_hdd_cfg80211_get_txpower() - cfg80211 get power handler function
 * @wiphy: Pointer to wiphy structure.
 * @wdev: Pointer to wireless_dev structure.
 * @dbm: dbm
 *
 * This is the cfg80211 get txpower handler function which invokes
 * the internal function @__wlan_hdd_cfg80211_get_txpower with
 * SSR protection.
 *
 * Return: 0 for success, error number on failure.
 */
int wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 int *dbm)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_get_txpower(wiphy, wdev, dbm);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_set_qpower_config() - set qpower config to firmware
 * @adapter: HDD adapter
 * @qpower: new qpower config value
 *
 * Return: 0 on success; Errno on failure
 */
int hdd_set_qpower_config(hdd_context_t *hddctx, hdd_adapter_t *adapter,
			  u8 qpower)
{
	QDF_STATUS status;

	if (!hddctx->config->enablePowersaveOffload) {
		hdd_err("qpower is disabled in configuration");
		return -EINVAL;
	}

	if (qpower > PS_DUTY_CYCLING_QPOWER ||
	    qpower < PS_LEGACY_NODEEPSLEEP) {
		hdd_err("invalid qpower value: %d", qpower);
		return -EINVAL;
	}

	if (hddctx->config->nMaxPsPoll) {
		if ((qpower == PS_QPOWER_NODEEPSLEEP) ||
				(qpower == PS_LEGACY_NODEEPSLEEP))
			qpower = PS_LEGACY_NODEEPSLEEP;
		else
			qpower = PS_LEGACY_DEEPSLEEP;
		hdd_info("Qpower disabled, %d", qpower);
	}
	status = wma_set_qpower_config(adapter->sessionId, qpower);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("failed to configure qpower: %d", status);
		return -EINVAL;
	}

	return 0;
}


#ifdef WLAN_SUSPEND_RESUME_TEST
static struct net_device *g_dev;
static struct wiphy *g_wiphy;
static enum wow_resume_trigger g_resume_trigger;

#define HDD_FA_SUSPENDED_BIT (0)
static unsigned long fake_apps_state;

/**
 * __hdd_wlan_fake_apps_resume() - The core logic for
 *	hdd_wlan_fake_apps_resume() skipping the call to hif_fake_apps_resume(),
 *	which is only need for non-irq resume
 * @wiphy: the kernel wiphy struct for the device being resumed
 * @dev: the kernel net_device struct for the device being resumed
 *
 * Return: none, calls QDF_BUG() on failure
 */
static void __hdd_wlan_fake_apps_resume(struct wiphy *wiphy,
					struct net_device *dev)
{
	struct hif_opaque_softc *hif_ctx;
	qdf_device_t qdf_dev;

	hdd_debug("Unit-test resume WLAN");

	qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	if (!qdf_dev) {
		hdd_err("Failed to get QDF device context");
		QDF_BUG(0);
		return;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("Failed to get HIF context");
		return;
	}

	if (!test_and_clear_bit(HDD_FA_SUSPENDED_BIT, &fake_apps_state)) {
		hdd_debug("Not unit-test suspended; Nothing to do");
		return;
	}

	/* simulate kernel disable irqs */
	QDF_BUG(!hif_apps_wake_irq_disable(hif_ctx));

	QDF_BUG(!wlan_hdd_bus_resume_noirq());

	/* simulate kernel enable irqs */
	QDF_BUG(!hif_apps_irqs_enable(hif_ctx));

	QDF_BUG(!wlan_hdd_bus_resume());

	QDF_BUG(!wlan_hdd_cfg80211_resume_wlan(wiphy));

	if (g_resume_trigger == WOW_RESUME_TRIGGER_HTC_WAKEUP)
		hif_vote_link_down(hif_ctx);

	dev->watchdog_timeo = HDD_TX_TIMEOUT;

	hdd_debug("Unit-test resume succeeded");
}

/**
 * hdd_wlan_fake_apps_resume_irq_callback() - Irq callback function for resuming
 *	from unit-test initiated suspend from irq wakeup signal
 * @val: interrupt val
 *
 * Resume wlan after getting very 1st CE interrupt from target
 *
 * Return: none
 */
static void hdd_wlan_fake_apps_resume_irq_callback(uint32_t val)
{
	hdd_debug("Trigger unit-test resume WLAN; val: 0x%x", val);

	QDF_BUG(g_wiphy);
	QDF_BUG(g_dev);
	__hdd_wlan_fake_apps_resume(g_wiphy, g_dev);
	g_wiphy = NULL;
	g_dev = NULL;
}

int hdd_wlan_fake_apps_suspend(struct wiphy *wiphy, struct net_device *dev,
			       enum wow_interface_pause pause_setting,
			       enum wow_resume_trigger resume_setting)
{
	qdf_device_t qdf_dev;
	struct hif_opaque_softc *hif_ctx;
	int errno;
	struct wow_enable_params wow_params = {
		.is_unit_test = true,
		.interface_pause = pause_setting,
		.resume_trigger = resume_setting
	};

	hdd_debug("Unit-test suspend WLAN");

	if (pause_setting < WOW_INTERFACE_PAUSE_DEFAULT ||
	    pause_setting >= WOW_INTERFACE_PAUSE_COUNT) {
		hdd_err("Invalid interface pause %d (expected range [0, 2])",
			pause_setting);
		return -EINVAL;
	}

	if (resume_setting < WOW_RESUME_TRIGGER_DEFAULT ||
	    resume_setting >= WOW_RESUME_TRIGGER_COUNT) {
		hdd_err("Invalid resume trigger %d (expected range [0, 2])",
			resume_setting);
		return -EINVAL;
	}

	qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	if (!qdf_dev) {
		hdd_err("Failed to get QDF device context");
		return -EINVAL;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("Failed to get HIF context");
		return -EINVAL;
	}

	if (test_and_set_bit(HDD_FA_SUSPENDED_BIT, &fake_apps_state)) {
		hdd_debug("Already unit-test suspended; Nothing to do");
		return 0;
	}

	/* pci link is needed to wakeup from HTC wakeup trigger */
	if (resume_setting == WOW_RESUME_TRIGGER_HTC_WAKEUP)
		hif_vote_link_up(hif_ctx);

	errno = wlan_hdd_cfg80211_suspend_wlan(wiphy, NULL);
	if (errno)
		goto link_down;

	errno = wlan_hdd_unit_test_bus_suspend(wow_params);
	if (errno)
		goto cfg80211_resume;

	/* simulate kernel disabling irqs */
	errno = hif_apps_irqs_disable(hif_ctx);
	if (errno)
		goto bus_resume;

	errno = wlan_hdd_bus_suspend_noirq();
	if (errno)
		goto enable_irqs;

	/* pass wiphy/dev to callback via global variables */
	g_wiphy = wiphy;
	g_dev = dev;
	g_resume_trigger = resume_setting;
	hif_fake_apps_suspend(hif_ctx, hdd_wlan_fake_apps_resume_irq_callback);

	/* re-enable wake irq */
	errno = hif_apps_wake_irq_enable(hif_ctx);
	if (errno)
		goto fake_apps_resume;

	/*
	 * Tell the kernel not to worry if TX queues aren't moving. This is
	 * expected since we are suspending the wifi hardware, but not APPS
	 */
	dev->watchdog_timeo = INT_MAX;

	hdd_debug("Unit-test suspend succeeded");

	return 0;

fake_apps_resume:
	hif_fake_apps_resume(hif_ctx);

enable_irqs:
	QDF_BUG(!hif_apps_irqs_enable(hif_ctx));

bus_resume:
	QDF_BUG(!wlan_hdd_bus_resume());

cfg80211_resume:
	QDF_BUG(!wlan_hdd_cfg80211_resume_wlan(wiphy));

link_down:
	hif_vote_link_down(hif_ctx);

	clear_bit(HDD_FA_SUSPENDED_BIT, &fake_apps_state);
	hdd_err("Unit-test suspend failed: %d", errno);

	return errno;
}

int hdd_wlan_fake_apps_resume(struct wiphy *wiphy, struct net_device *dev)
{
	struct hif_opaque_softc *hif_ctx;

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("Failed to get HIF context");
		return -EINVAL;
	}

	hif_fake_apps_resume(hif_ctx);
	__hdd_wlan_fake_apps_resume(wiphy, dev);

	return 0;
}
#endif
