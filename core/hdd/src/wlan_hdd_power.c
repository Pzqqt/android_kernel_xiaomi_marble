/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_power.c
 *
 * WLAN power management functions
 *
 */

/* Include files */

#include <linux/pm.h>
#include <linux/wait.h>
#include <linux/cpu.h>
#include "osif_sync.h"
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

#include <linux/inetdevice.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_scan.h>
#include <wlan_hdd_stats.h>
#include <wlan_hdd_cfg80211.h>
#include <net/addrconf.h>
#include <wlan_hdd_lpass.h>

#include <wma_types.h>
#include <ol_txrx_osif_api.h>
#include "hif.h"
#include "hif_unit_test_suspend.h"
#include "sme_power_save_api.h"
#include "wlan_policy_mgr_api.h"
#include "cdp_txrx_flow_ctrl_v2.h"
#include "pld_common.h"
#include "wlan_hdd_driver_ops.h"
#include <wlan_logging_sock_svc.h>
#include "scheduler_api.h"
#include "cds_utils.h"
#include "wlan_hdd_packet_filter_api.h"
#include "wlan_cfg80211_scan.h"
#include <dp_txrx.h>
#include "wlan_ipa_ucfg_api.h"
#include <wlan_cfg80211_mc_cp_stats.h>
#include "wlan_p2p_ucfg_api.h"
#include "wlan_mlme_ucfg_api.h"

/* Preprocessor definitions and constants */
#ifdef QCA_WIFI_NAPIER_EMULATION
#define HDD_SSR_BRING_UP_TIME 3000000
#else
#define HDD_SSR_BRING_UP_TIME 30000
#endif

/* Type declarations */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
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

#ifdef QCA_CONFIG_SMP

/* timeout in msec to wait for RX_THREAD to suspend */
#define HDD_RXTHREAD_SUSPEND_TIMEOUT 200

void wlan_hdd_rx_thread_resume(struct hdd_context *hdd_ctx)
{
	if (hdd_ctx->is_ol_rx_thread_suspended) {
		cds_resume_rx_thread();
		hdd_ctx->is_ol_rx_thread_suspended = false;
	}
}

int wlan_hdd_rx_thread_suspend(struct hdd_context *hdd_ctx)
{
	p_cds_sched_context cds_sched_context = get_cds_sched_ctxt();
	int rc;

	if (!cds_sched_context)
		return 0;

	/* Suspend tlshim rx thread */
	set_bit(RX_SUSPEND_EVENT, &cds_sched_context->ol_rx_event_flag);
	wake_up_interruptible(&cds_sched_context->ol_rx_wait_queue);
	rc = wait_for_completion_timeout(&cds_sched_context->
					 ol_suspend_rx_event,
					 msecs_to_jiffies
					 (HDD_RXTHREAD_SUSPEND_TIMEOUT)
					);
	if (!rc) {
		clear_bit(RX_SUSPEND_EVENT,
			  &cds_sched_context->ol_rx_event_flag);
		hdd_err("Failed to stop tl_shim rx thread");
		return -EINVAL;
	}
	hdd_ctx->is_ol_rx_thread_suspended = true;

	return 0;
}
#endif /* QCA_CONFIG_SMP */

/**
 * hdd_enable_gtk_offload() - enable GTK offload
 * @adapter: pointer to the adapter
 *
 * Central function to enable GTK offload.
 *
 * Return: nothing
 */
static void hdd_enable_gtk_offload(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	status = ucfg_pmo_enable_gtk_offload_in_fwr(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to enable gtk offload");
}

/**
 * hdd_disable_gtk_offload() - disable GTK offload
 * @adapter:   pointer to the adapter
 *
 * Central function to disable GTK offload.
 *
 * Return: nothing
 */
static void hdd_disable_gtk_offload(struct hdd_adapter *adapter)
{
	struct pmo_gtk_rsp_req gtk_rsp_request;
	QDF_STATUS status;

	/* ensure to get gtk rsp first before disable it*/
	gtk_rsp_request.callback = wlan_hdd_cfg80211_update_replay_counter_cb;

	/* Passing as void* as PMO does not know legacy HDD adapter type */
	gtk_rsp_request.callback_context = (void *)adapter;

	status = ucfg_pmo_get_gtk_rsp(adapter->vdev, &gtk_rsp_request);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to send get gtk rsp status:%d", status);
		return;
	}

	hdd_debug("send get_gtk_rsp successful");
	status = ucfg_pmo_disable_gtk_offload_in_fwr(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to disable gtk offload");
}

#ifdef WLAN_NS_OFFLOAD
/**
 * __wlan_hdd_ipv6_changed() - IPv6 notifier callback function
 * @net_dev: net_device whose IP address changed
 *
 * This is a callback function that is registered with the kernel via
 * register_inet6addr_notifier() which allows the driver to be
 * notified when there is an IPv6 address change.
 *
 * Return: None
 */
static void __wlan_hdd_ipv6_changed(struct net_device *net_dev)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
	struct hdd_context *hdd_ctx;
	int errno;

	hdd_enter_dev(net_dev);

	errno = hdd_validate_adapter(adapter);
	if (errno || adapter->dev != net_dev)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE ||
	    adapter->device_mode == QDF_NDI_MODE) {
		hdd_debug("invoking sme_dhcp_done_ind");
		sme_dhcp_done_ind(hdd_ctx->mac_handle, adapter->vdev_id);
		schedule_work(&adapter->ipv6_notifier_work);
	}

exit:
	hdd_exit();
}

int wlan_hdd_ipv6_changed(struct notifier_block *nb,
			  unsigned long data, void *context)
{
	struct inet6_ifaddr *ifa = context;
	struct net_device *net_dev = ifa->idev->dev;
	struct osif_vdev_sync *vdev_sync;

	if (osif_vdev_sync_op_start(net_dev, &vdev_sync))
		return NOTIFY_DONE;

	__wlan_hdd_ipv6_changed(net_dev);

	osif_vdev_sync_op_stop(vdev_sync);

	return NOTIFY_DONE;
}

/**
 * hdd_fill_ipv6_uc_addr() - fill IPv6 unicast addresses
 * @idev: pointer to net device
 * @ipv6addr: destination array to fill IPv6 addresses
 * @ipv6addr_type: IPv6 Address type
 * @scope_array: scope of ipv6 addr
 * @count: number of IPv6 addresses
 *
 * This is the IPv6 utility function to populate unicast addresses.
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_fill_ipv6_uc_addr(struct inet6_dev *idev,
				uint8_t ipv6_uc_addr[][QDF_IPV6_ADDR_SIZE],
				uint8_t *ipv6addr_type,
				enum pmo_ns_addr_scope *scope_array,
				uint32_t *count)
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
			scope_array[*count] = ucfg_pmo_ns_addr_scope(scope);
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
 * @scope_array: scope of ipv6 addr
 * @count: number of IPv6 addresses
 *
 * This is the IPv6 utility function to populate anycast addresses.
 *
 * Return: 0 on success, error number otherwise.
 */
static int hdd_fill_ipv6_ac_addr(struct inet6_dev *idev,
				uint8_t ipv6_ac_addr[][QDF_IPV6_ADDR_SIZE],
				uint8_t *ipv6addr_type,
				enum pmo_ns_addr_scope *scope_array,
				uint32_t *count)
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
			scope_array[*count] = ucfg_pmo_ns_addr_scope(scope);
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

void hdd_enable_ns_offload(struct hdd_adapter *adapter,
			   enum pmo_offload_trigger trigger)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_psoc *psoc = hdd_ctx->psoc;
	struct inet6_dev *in6_dev;
	struct pmo_ns_req *ns_req;
	QDF_STATUS status;
	int errno;

	hdd_enter();

	if (!psoc) {
		hdd_err("psoc is NULL");
		goto out;
	}

	in6_dev = __in6_dev_get(adapter->dev);
	if (!in6_dev) {
		hdd_err("IPv6 dev does not exist. Failed to request NSOffload");
		goto out;
	}

	ns_req = qdf_mem_malloc(sizeof(*ns_req));
	if (!ns_req)
		goto out;

	ns_req->psoc = psoc;
	ns_req->vdev_id = adapter->vdev_id;
	ns_req->trigger = trigger;
	ns_req->count = 0;

	/* Unicast Addresses */
	errno = hdd_fill_ipv6_uc_addr(in6_dev, ns_req->ipv6_addr,
				      ns_req->ipv6_addr_type, ns_req->scope,
				      &ns_req->count);
	if (errno) {
		hdd_disable_ns_offload(adapter, trigger);
		hdd_debug("Max supported addresses: disabling NS offload");
		goto free_req;
	}

	/* Anycast Addresses */
	errno = hdd_fill_ipv6_ac_addr(in6_dev, ns_req->ipv6_addr,
				      ns_req->ipv6_addr_type, ns_req->scope,
				      &ns_req->count);
	if (errno) {
		hdd_disable_ns_offload(adapter, trigger);
		hdd_debug("Max supported addresses: disabling NS offload");
		goto free_req;
	}

	/* cache ns request */
	status = ucfg_pmo_cache_ns_offload_req(ns_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to cache ns request; status:%d", status);
		goto free_req;
	}

	/* enable ns request */
	status = ucfg_pmo_enable_ns_offload_in_fwr(adapter->vdev, trigger);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to enable ns offload; status:%d", status);
		goto free_req;
	}

	hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD, SIR_OFFLOAD_ENABLE);

free_req:
	qdf_mem_free(ns_req);

out:
	hdd_exit();
}

void hdd_disable_ns_offload(struct hdd_adapter *adapter,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;

	hdd_enter();
	status = ucfg_pmo_flush_ns_offload_req(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to flush NS Offload");
		goto out;
	}

	status = ucfg_pmo_disable_ns_offload_in_fwr(adapter->vdev, trigger);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("Failed to disable NS Offload");
	else
		hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD,
			SIR_OFFLOAD_DISABLE);
out:
	hdd_exit();

}

/**
 * __hdd_ipv6_notifier_work_queue() - IPv6 notification work function
 * @adapter: adapter whose IP address changed
 *
 * This function performs the work initially trigged by a callback
 * from the IPv6 netdev notifier.  Since this means there has been a
 * change in IPv6 state for the interface, the NS offload is
 * reconfigured.
 *
 * Return: None
 */
static void __hdd_ipv6_notifier_work_queue(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;
	int errno;

	hdd_enter();

	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	hdd_enable_ns_offload(adapter, pmo_ipv6_change_notify);

exit:
	hdd_exit();
}

void hdd_ipv6_notifier_work_queue(struct work_struct *work)
{
	struct hdd_adapter *adapter = container_of(work, struct hdd_adapter,
						   ipv6_notifier_work);
	struct osif_vdev_sync *vdev_sync;

	if (osif_vdev_sync_op_start(adapter->dev, &vdev_sync))
		return;

	__hdd_ipv6_notifier_work_queue(adapter);

	osif_vdev_sync_op_stop(vdev_sync);
}
#endif /* WLAN_NS_OFFLOAD */

static void hdd_enable_hw_filter(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	hdd_enter();

	status = ucfg_pmo_enable_hw_filter_in_fwr(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to enable hardware filter");

	hdd_exit();
}

static void hdd_disable_hw_filter(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	hdd_enter();

	status = ucfg_pmo_disable_hw_filter_in_fwr(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS)
		hdd_info("Failed to disable hardware filter");

	hdd_exit();
}

void hdd_enable_host_offloads(struct hdd_adapter *adapter,
	enum pmo_offload_trigger trigger)
{
	hdd_enter();

	if (!ucfg_pmo_is_vdev_supports_offload(adapter->vdev)) {
		hdd_debug("offload is not supported on vdev opmode %d",
			  adapter->device_mode);
		goto out;
	}

	if (!ucfg_pmo_is_vdev_connected(adapter->vdev)) {
		hdd_debug("offload is not supported on disconnected vdevs");
		goto out;
	}

	hdd_debug("enable offloads");
	hdd_enable_gtk_offload(adapter);
	hdd_enable_arp_offload(adapter, trigger);
	hdd_enable_ns_offload(adapter, trigger);
	hdd_enable_mc_addr_filtering(adapter, trigger);
	hdd_enable_hw_filter(adapter);
out:
	hdd_exit();

}

void hdd_disable_host_offloads(struct hdd_adapter *adapter,
	enum pmo_offload_trigger trigger)
{
	hdd_enter();

	if (!ucfg_pmo_is_vdev_supports_offload(adapter->vdev)) {
		hdd_info("offload is not supported on this vdev opmode: %d",
				adapter->device_mode);
			goto out;
	}

	if (!ucfg_pmo_is_vdev_connected(adapter->vdev)) {
		hdd_info("vdev is not connected");
		goto out;
	}

	hdd_debug("disable offloads");
	hdd_disable_gtk_offload(adapter);
	hdd_disable_arp_offload(adapter, trigger);
	hdd_disable_ns_offload(adapter, trigger);
	hdd_disable_mc_addr_filtering(adapter, trigger);
	hdd_disable_hw_filter(adapter);
out:
	hdd_exit();

}

/**
 * hdd_lookup_ifaddr() - Lookup interface address data by name
 * @adapter: the adapter whose name should be searched for
 *
 * return in_ifaddr pointer on success, NULL for failure
 */
static struct in_ifaddr *hdd_lookup_ifaddr(struct hdd_adapter *adapter)
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
static int hdd_populate_ipv4_addr(struct hdd_adapter *adapter,
				  uint8_t *ipv4_addr)
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
static int hdd_set_grat_arp_keepalive(struct hdd_adapter *adapter)
{
	QDF_STATUS status;
	int exit_code;
	struct hdd_context *hdd_ctx;
	struct hdd_station_ctx *sta_ctx;
	struct keep_alive_req req = {
		.packetType = SIR_KEEP_ALIVE_UNSOLICIT_ARP_RSP,
		.dest_macaddr = QDF_MAC_ADDR_BCAST_INIT,
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

	qdf_copy_macaddr(&req.bssid, &sta_ctx->conn_info.bssid);
	ucfg_mlme_get_sta_keep_alive_period(hdd_ctx->psoc, &req.timePeriod);
	req.sessionId = adapter->vdev_id;

	hdd_debug("Setting gratuitous ARP keepalive; ipv4_addr:%u.%u.%u.%u",
		 req.hostIpv4Addr[0], req.hostIpv4Addr[1],
		 req.hostIpv4Addr[2], req.hostIpv4Addr[3]);

	status = sme_set_keep_alive(hdd_ctx->mac_handle, req.sessionId, &req);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("Failed to set keepalive");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

/**
 * __hdd_ipv4_notifier_work_queue() - IPv4 notification work function
 * @adapter: adapter whose IP address changed
 *
 * This function performs the work initially trigged by a callback
 * from the IPv4 netdev notifier.  Since this means there has been a
 * change in IPv4 state for the interface, the ARP offload is
 * reconfigured. Also, Updates the HLP IE info with IP address info
 * to fw if LFR3 is enabled
 *
 * Return: None
 */
static void __hdd_ipv4_notifier_work_queue(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx;
	int errno;
	struct csr_roam_profile *roam_profile;
	struct in_ifaddr *ifa;
	enum station_keepalive_method val;
	QDF_STATUS status;

	hdd_enter();

	errno = hdd_validate_adapter(adapter);
	if (errno)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	hdd_enable_arp_offload(adapter, pmo_ipv4_change_notify);

	status = ucfg_mlme_get_sta_keepalive_method(hdd_ctx->psoc, &val);
	if (QDF_IS_STATUS_ERROR(status))
		goto exit;

	if (val == MLME_STA_KEEPALIVE_GRAT_ARP)
		hdd_set_grat_arp_keepalive(adapter);

	hdd_debug("FILS Roaming support: %d",
		  hdd_ctx->is_fils_roaming_supported);
	roam_profile = hdd_roam_profile(adapter);

	ifa = hdd_lookup_ifaddr(adapter);
	if (ifa && hdd_ctx->is_fils_roaming_supported)
		sme_send_hlp_ie_info(hdd_ctx->mac_handle, adapter->vdev_id,
				     roam_profile, ifa->ifa_local);
exit:
	hdd_exit();
}

void hdd_ipv4_notifier_work_queue(struct work_struct *work)
{
	struct hdd_adapter *adapter = container_of(work, struct hdd_adapter,
						   ipv4_notifier_work);
	struct osif_vdev_sync *vdev_sync;

	if (osif_vdev_sync_op_start(adapter->dev, &vdev_sync))
		return;

	__hdd_ipv4_notifier_work_queue(adapter);

	osif_vdev_sync_op_stop(vdev_sync);
}

/**
 * __wlan_hdd_ipv4_changed() - IPv4 notifier callback function
 * @net_dev: the net_device whose IP address changed
 *
 * This is a callback function that is registered with the kernel via
 * register_inetaddr_notifier() which allows the driver to be
 * notified when there is an IPv4 address change.
 *
 * Return: None
 */
static void __wlan_hdd_ipv4_changed(struct net_device *net_dev)
{
	struct in_ifaddr *ifa;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(net_dev);
	struct hdd_context *hdd_ctx;
	int errno;

	hdd_enter_dev(net_dev);

	errno = hdd_validate_adapter(adapter);
	if (errno || adapter->dev != net_dev)
		goto exit;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		goto exit;

	if (adapter->device_mode == QDF_STA_MODE ||
	    adapter->device_mode == QDF_P2P_CLIENT_MODE ||
	    adapter->device_mode == QDF_NDI_MODE) {
		hdd_debug("invoking sme_dhcp_done_ind");
		sme_dhcp_done_ind(hdd_ctx->mac_handle, adapter->vdev_id);

		if (!ucfg_pmo_is_arp_offload_enabled(hdd_ctx->psoc)) {
			hdd_debug("Offload not enabled");
			goto exit;
		}

		ifa = hdd_lookup_ifaddr(adapter);
		if (ifa && ifa->ifa_local)
			schedule_work(&adapter->ipv4_notifier_work);
	}

exit:
	hdd_exit();
}

int wlan_hdd_ipv4_changed(struct notifier_block *nb,
			  unsigned long data, void *context)
{
	struct in_ifaddr *ifa = context;
	struct net_device *net_dev = ifa->ifa_dev->dev;
	struct osif_vdev_sync *vdev_sync;

	if (osif_vdev_sync_op_start(net_dev, &vdev_sync))
		return NOTIFY_DONE;

	__wlan_hdd_ipv4_changed(net_dev);

	osif_vdev_sync_op_stop(vdev_sync);

	return NOTIFY_DONE;
}

/**
 * hdd_get_ipv4_local_interface() - get ipv4 local interafce from iface list
 * @adapter: Adapter context for which ARP offload is to be configured
 *
 * Return:
 *	ifa - on successful operation,
 *	NULL - on failure of operation
 */
static struct in_ifaddr *hdd_get_ipv4_local_interface(
				struct hdd_adapter *adapter)
{
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct in_device *in_dev;

	in_dev = __in_dev_get_rtnl(adapter->dev);
	if (in_dev) {
		for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
			     ifap = &ifa->ifa_next) {
			if (!strcmp(adapter->dev->name, ifa->ifa_label)) {
				/* if match break */
				return ifa;
			}
		}
	}
	ifa = NULL;

	return ifa;
}

void hdd_enable_arp_offload(struct hdd_adapter *adapter,
			    enum pmo_offload_trigger trigger)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct wlan_objmgr_psoc *psoc = hdd_ctx->psoc;
	QDF_STATUS status;
	struct pmo_arp_req *arp_req;
	struct in_ifaddr *ifa;

	hdd_enter();

	arp_req = qdf_mem_malloc(sizeof(*arp_req));
	if (!arp_req)
		goto out;

	arp_req->psoc = psoc;
	arp_req->vdev_id = adapter->vdev_id;
	arp_req->trigger = trigger;

	ifa = hdd_get_ipv4_local_interface(adapter);
	if (!ifa || !ifa->ifa_local) {
		hdd_info("IP Address is not assigned");
		status = QDF_STATUS_NOT_INITIALIZED;
		goto free_req;
	}

	arp_req->ipv4_addr = (uint32_t)ifa->ifa_local;

	status = ucfg_pmo_cache_arp_offload_req(arp_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("failed to cache arp offload req; status:%d", status);
		goto free_req;
	}

	status = ucfg_pmo_enable_arp_offload_in_fwr(adapter->vdev, trigger);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("failed arp offload config in fw; status:%d", status);
		goto free_req;
	}

	hdd_wlan_offload_event(PMO_IPV4_ARP_REPLY_OFFLOAD, PMO_OFFLOAD_ENABLE);

free_req:
	qdf_mem_free(arp_req);

out:
	hdd_exit();
}

void hdd_disable_arp_offload(struct hdd_adapter *adapter,
		enum pmo_offload_trigger trigger)
{
	QDF_STATUS status;

	hdd_enter();
	status = ucfg_pmo_flush_arp_offload_req(adapter->vdev);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to flush arp Offload");
		goto out;
	}

	status = ucfg_pmo_disable_arp_offload_in_fwr(adapter->vdev,
						     trigger);
	if (status == QDF_STATUS_SUCCESS)
		hdd_wlan_offload_event(PMO_IPV4_ARP_REPLY_OFFLOAD,
			PMO_OFFLOAD_DISABLE);
	else
		hdd_info("fail to disable arp offload");
out:
	hdd_exit();
}

void hdd_enable_mc_addr_filtering(struct hdd_adapter *adapter,
				  enum pmo_offload_trigger trigger)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;

	hdd_enter();

	if (wlan_hdd_validate_context(hdd_ctx))
		goto out;

	if (!hdd_adapter_is_connected_sta(adapter))
		goto out;

	status = ucfg_pmo_enable_mc_addr_filtering_in_fwr(hdd_ctx->psoc,
							  adapter->vdev_id,
							  trigger);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to enable mc list; status:%d", status);

out:
	hdd_exit();
}

void hdd_disable_mc_addr_filtering(struct hdd_adapter *adapter,
				   enum pmo_offload_trigger trigger)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;

	hdd_enter();

	if (wlan_hdd_validate_context(hdd_ctx))
		goto out;

	if (!hdd_adapter_is_connected_sta(adapter))
		goto out;

	status = ucfg_pmo_disable_mc_addr_filtering_in_fwr(hdd_ctx->psoc,
							   adapter->vdev_id,
							   trigger);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to disable mc list; status:%d", status);

out:
	hdd_exit();
}

int hdd_cache_mc_addr_list(struct pmo_mc_addr_list_params *mc_list_config)
{
	QDF_STATUS status;

	hdd_enter();
	status = ucfg_pmo_cache_mc_addr_list(mc_list_config);
	hdd_exit();

	return qdf_status_to_os_return(status);
}

void hdd_disable_and_flush_mc_addr_list(struct hdd_adapter *adapter,
					enum pmo_offload_trigger trigger)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;

	hdd_enter();

	if (!hdd_adapter_is_connected_sta(adapter))
		goto flush_mc_list;

	/* disable mc list first because the mc list is cached in PMO */
	status = ucfg_pmo_disable_mc_addr_filtering_in_fwr(hdd_ctx->psoc,
							   adapter->vdev_id,
							   trigger);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to disable mc list; status:%d", status);

flush_mc_list:
	status = ucfg_pmo_flush_mc_addr_list(hdd_ctx->psoc,
					     adapter->vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("failed to flush mc list; status:%d", status);

	hdd_exit();
}

/**
 * hdd_update_conn_state_mask(): record info needed by wma_suspend_req
 * @adapter: adapter to get info from
 * @conn_state_mask: mask of connection info
 *
 * currently only need to send connection info.
 */
static void hdd_update_conn_state_mask(struct hdd_adapter *adapter,
				       uint32_t *conn_state_mask)
{

	eConnectionState conn_state;
	struct hdd_station_ctx *sta_ctx;

	sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	conn_state = sta_ctx->conn_info.conn_state;

	if (conn_state == eConnectionState_Associated ||
			conn_state == eConnectionState_IbssConnected)
		*conn_state_mask |= (1 << adapter->vdev_id);
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
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;
	struct hdd_adapter *adapter = NULL;
	uint32_t conn_state_mask = 0;

	hdd_info("WLAN being suspended by OS");

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}

	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore suspend!!!",
			 cds_get_driver_state());
		return -EINVAL;
	}

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
			continue;

		/* stop all TX queues before suspend */
		hdd_debug("Disabling queues for dev mode %s",
			  qdf_opmode_str(adapter->device_mode));
		wlan_hdd_netif_queue_control(adapter,
					     WLAN_STOP_ALL_NETIF_QUEUE,
					     WLAN_CONTROL_PATH);

		if (adapter->device_mode == QDF_STA_MODE)
			status = hdd_enable_default_pkt_filters(adapter);

		/* Configure supported OffLoads */
		hdd_enable_host_offloads(adapter, pmo_apps_suspend);
		hdd_update_conn_state_mask(adapter, &conn_state_mask);
	}

	status = ucfg_pmo_psoc_user_space_suspend_req(hdd_ctx->psoc,
						      QDF_SYSTEM_SUSPEND);
	if (status != QDF_STATUS_SUCCESS)
		return -EAGAIN;

	hdd_ctx->hdd_wlan_suspended = true;
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
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;
	QDF_STATUS status;

	hdd_info("WLAN being resumed by OS");

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}

	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore resume!!!",
			 cds_get_driver_state());
		return -EINVAL;
	}

	hdd_ctx->hdd_wlan_suspended = false;
	hdd_wlan_suspend_resume_event(HDD_WLAN_EARLY_RESUME);

	/*loop through all adapters. Concurrency */
	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
			continue;

		/* Disable supported OffLoads */
		hdd_disable_host_offloads(adapter, pmo_apps_resume);

		/* wake the tx queues */
		hdd_debug("Enabling queues for dev mode %s",
			  qdf_opmode_str(adapter->device_mode));
		wlan_hdd_netif_queue_control(adapter,
					WLAN_WAKE_ALL_NETIF_QUEUE,
					WLAN_CONTROL_PATH);

		if (adapter->device_mode == QDF_STA_MODE)
			status = hdd_disable_default_pkt_filters(adapter);
	}

	ucfg_ipa_resume(hdd_ctx->pdev);
	status = ucfg_pmo_psoc_user_space_resume_req(hdd_ctx->psoc,
						     QDF_SYSTEM_SUSPEND);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	return 0;
}

void hdd_svc_fw_shutdown_ind(struct device *dev)
{
	struct hdd_context *hdd_ctx;

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
static void hdd_ssr_restart_sap(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter;

	hdd_enter();

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (adapter->device_mode == QDF_SAP_MODE) {
			if (test_bit(SOFTAP_INIT_DONE, &adapter->event_flags)) {
				hdd_debug("Restart prev SAP session");
				wlan_hdd_start_sap(adapter, true);
			}
		}
	}

	hdd_exit();
}

QDF_STATUS hdd_wlan_shutdown(void)
{
	struct hdd_context *hdd_ctx;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	hdd_info("WLAN driver shutting down!");

	/* Get the HDD context. */
	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is Null");
		return QDF_STATUS_E_FAILURE;
	}

	hdd_bus_bw_compute_timer_stop(hdd_ctx);
	policy_mgr_clear_concurrent_session_count(hdd_ctx->psoc);

	hdd_debug("Invoking packetdump deregistration API");
	wlan_deregister_txrx_packetdump();

	/* resume wlan threads before adapter reset which does vdev destroy */
	if (hdd_ctx->is_scheduler_suspended) {
		scheduler_resume();
		hdd_ctx->is_scheduler_suspended = false;
		hdd_ctx->is_wiphy_suspended = false;
	}

	wlan_hdd_rx_thread_resume(hdd_ctx);

	/*
	 * After SSR, FW clear its txrx stats. In host,
	 * as adapter is intact so those counts are still
	 * available. Now if agains Set stats command comes,
	 * then host will increment its counts start from its
	 * last saved value, i.e., count before SSR, and FW will
	 * increment its count from 0. This will finally sends a
	 * mismatch of packet counts b/w host and FW to framework
	 * that will create ambiquity. Therfore, Resetting the host
	 * counts here so that after SSR both FW and host start
	 * increment their counts from 0.
	 */
	hdd_reset_all_adapters_connectivity_stats(hdd_ctx);
	/*
	 * Purge all active and pending list to avoid vdev destroy timeout and
	 * thus avoid peer/vdev refcount leak.
	 */
	sme_purge_pdev_all_ser_cmd_list(hdd_ctx->mac_handle);

	hdd_reset_all_adapters(hdd_ctx);

	ucfg_ipa_uc_ssr_cleanup(hdd_ctx->pdev);

	/* Flush cached rx frame queue */
	if (soc)
		cdp_flush_cache_rx_queue(soc);

	/* De-register the HDD callbacks */
	hdd_deregister_cb(hdd_ctx);

	hdd_wlan_stop_modules(hdd_ctx, false);

	hdd_bus_bandwidth_deinit(hdd_ctx);
	hdd_lpass_notify_stop(hdd_ctx);

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
 * hdd_send_default_scan_ies - send default scan ies to fw
 *
 * This function is used to send default scan ies to fw
 * in case of wlan re-init
 *
 * Return: void
 */
static void hdd_send_default_scan_ies(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter;

	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (hdd_is_interface_up(adapter) &&
		    (adapter->device_mode == QDF_STA_MODE ||
		    adapter->device_mode == QDF_P2P_DEVICE_MODE) &&
		    adapter->scan_info.default_scan_ies) {
			sme_set_default_scan_ie(hdd_ctx->mac_handle,
				      adapter->vdev_id,
				      adapter->scan_info.default_scan_ies,
				      adapter->scan_info.default_scan_ies_len);
		}
	}
}

/**
 * hdd_is_interface_down_during_ssr - Check if the interface went down during
 * SSR
 * @hdd_ctx: HDD context
 *
 * Check if any of the interface went down while the device is recovering.
 * If the interface went down close the session.
 */
static void hdd_is_interface_down_during_ssr(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter = NULL, *pnext = NULL;
	QDF_STATUS status;

	hdd_enter();

	status = hdd_get_front_adapter(hdd_ctx, &adapter);
	while (adapter && status == QDF_STATUS_SUCCESS) {
		if (test_bit(DOWN_DURING_SSR, &adapter->event_flags)) {
			clear_bit(DOWN_DURING_SSR, &adapter->event_flags);
			hdd_stop_adapter(hdd_ctx, adapter);
			hdd_deinit_adapter(hdd_ctx, adapter, true);
			clear_bit(DEVICE_IFACE_OPENED, &adapter->event_flags);
		}
		status = hdd_get_next_adapter(hdd_ctx, adapter, &pnext);
		adapter = pnext;
	}

	hdd_exit();
}

/**
 * hdd_restore_sar_config - Restore the saved SAR config after SSR
 * @hdd_ctx: HDD context
 *
 * Restore the SAR config that was lost during SSR.
 *
 * Return: None
 */
static void hdd_restore_sar_config(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;

	if (!hdd_ctx->sar_cmd_params)
		return;

	status = sme_set_sar_power_limits(hdd_ctx->mac_handle,
					  hdd_ctx->sar_cmd_params);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err("Unable to configured SAR after SSR");
}

QDF_STATUS hdd_wlan_re_init(void)
{
	struct hdd_context *hdd_ctx = NULL;
	struct hdd_adapter *adapter;
	int ret;
	bool bug_on_reinit_failure = CFG_BUG_ON_REINIT_FAILURE_DEFAULT;
	bool value;

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);

	/* Get the HDD context */
	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is Null");
		goto err_ctx_null;
	}
	bug_on_reinit_failure = hdd_ctx->config->bug_on_reinit_failure;

	adapter = hdd_get_first_valid_adapter(hdd_ctx);
	if (!adapter)
		hdd_err("Failed to get adapter");

	hdd_dp_trace_init(hdd_ctx->config);
	hdd_bus_bandwidth_init(hdd_ctx);

	ret = hdd_wlan_start_modules(hdd_ctx, true);
	if (ret) {
		hdd_err("Failed to start wlan after error");
		goto err_re_init;
	}

	hdd_update_hw_sw_info(hdd_ctx);

	wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
				WLAN_SVC_FW_CRASHED_IND, NULL, 0);

	/* Restart all adapters */
	hdd_start_all_adapters(hdd_ctx);

	hdd_init_scan_reject_params(hdd_ctx);

	hdd_set_roaming_in_progress(false);
	complete(&adapter->roaming_comp_var);
	hdd_ctx->bt_coex_mode_set = false;

	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);
	/* set chip power save failure detected callback */
	sme_set_chip_pwr_save_fail_cb(hdd_ctx->mac_handle,
				      hdd_chip_pwr_save_fail_detected_cb);

	hdd_restore_sar_config(hdd_ctx);

	hdd_send_default_scan_ies(hdd_ctx);
	hdd_info("WLAN host driver reinitiation completed!");

	ucfg_mlme_get_sap_internal_restart(hdd_ctx->psoc, &value);
	if (value)
		hdd_ssr_restart_sap(hdd_ctx);
	hdd_is_interface_down_during_ssr(hdd_ctx);
	hdd_wlan_ssr_reinit_event();
	return QDF_STATUS_SUCCESS;

err_re_init:
	hdd_bus_bandwidth_deinit(hdd_ctx);
	qdf_dp_trace_deinit();

err_ctx_null:
	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);
	if (bug_on_reinit_failure)
		QDF_BUG(0);
	return -EPERM;
}

int wlan_hdd_set_powersave(struct hdd_adapter *adapter,
	bool allow_power_save, uint32_t timeout)
{
	mac_handle_t mac_handle;
	struct hdd_context *hdd_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool is_bmps_enabled;

	if (!adapter) {
		hdd_err("Adapter NULL");
		return -ENODEV;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err("hdd context is NULL");
		return -EINVAL;
	}

	hdd_debug("Allow power save: %d", allow_power_save);
	mac_handle = hdd_ctx->mac_handle;

	/*
	 * This is a workaround for defective AP's that send a disassoc
	 * immediately after WPS connection completes. Defer powersave by a
	 * small amount if the affected AP is detected.
	 */
	if (allow_power_save &&
	    adapter->device_mode == QDF_STA_MODE &&
	    !adapter->session.station.ap_supports_immediate_power_save) {
		timeout = AUTO_PS_DEFER_TIMEOUT_MS;
		hdd_debug("Defer power-save due to AP spec non-conformance");
	}

	if (allow_power_save) {
		if (QDF_STA_MODE == adapter->device_mode ||
		    QDF_P2P_CLIENT_MODE == adapter->device_mode) {
			hdd_debug("Disabling Auto Power save timer");
			status = sme_ps_disable_auto_ps_timer(mac_handle,
						adapter->vdev_id);
			if (status != QDF_STATUS_SUCCESS)
				goto end;
		}

		ucfg_mlme_is_bmps_enabled(hdd_ctx->psoc, &is_bmps_enabled);
		if (is_bmps_enabled) {
			hdd_debug("Wlan driver Entering Power save");

			/*
			 * Enter Power Save command received from GUI
			 * this means DHCP is completed
			 */
			if (timeout) {
				status = sme_ps_enable_auto_ps_timer(mac_handle,
							    adapter->vdev_id,
							    timeout);
				if (status != QDF_STATUS_SUCCESS)
					goto end;
			} else {
				status = sme_ps_enable_disable(mac_handle,
						adapter->vdev_id,
						SME_PS_ENABLE);
				if (status != QDF_STATUS_SUCCESS)
					goto end;
			}
		} else {
			hdd_debug("Power Save is not enabled in the cfg");
		}
	} else {
		hdd_debug("Wlan driver Entering Full Power");

		/*
		 * Enter Full power command received from GUI
		 * this means we are disconnected
		 */
		status = sme_ps_disable_auto_ps_timer(mac_handle,
					adapter->vdev_id);

		if (status != QDF_STATUS_SUCCESS)
			goto end;

		ucfg_mlme_is_bmps_enabled(hdd_ctx->psoc, &is_bmps_enabled);
		if (is_bmps_enabled)
			status = sme_ps_enable_disable(mac_handle,
						       adapter->vdev_id,
						       SME_PS_DISABLE);
	}

end:
	return qdf_status_to_os_return(status);
}

static void wlan_hdd_print_suspend_fail_stats(struct hdd_context *hdd_ctx)
{
	struct suspend_resume_stats *stats = &hdd_ctx->suspend_resume_stats;

	hdd_err("ipa:%d, radar:%d, roam:%d, scan:%d, initial_wakeup:%d",
		stats->suspend_fail[SUSPEND_FAIL_IPA],
		stats->suspend_fail[SUSPEND_FAIL_RADAR],
		stats->suspend_fail[SUSPEND_FAIL_ROAM],
		stats->suspend_fail[SUSPEND_FAIL_SCAN],
		stats->suspend_fail[SUSPEND_FAIL_INITIAL_WAKEUP]);
}

void wlan_hdd_inc_suspend_stats(struct hdd_context *hdd_ctx,
				enum suspend_fail_reason reason)
{
	wlan_hdd_print_suspend_fail_stats(hdd_ctx);
	hdd_ctx->suspend_resume_stats.suspend_fail[reason]++;
	wlan_hdd_print_suspend_fail_stats(hdd_ctx);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline void
hdd_sched_scan_results(struct wiphy *wiphy, uint64_t reqid)
{
	cfg80211_sched_scan_results(wiphy);
}
#else
static inline void
hdd_sched_scan_results(struct wiphy *wiphy, uint64_t reqid)
{
	cfg80211_sched_scan_results(wiphy, reqid);
}
#endif

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
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int exit_code;

	hdd_enter();

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

	exit_code = wlan_hdd_validate_context(hdd_ctx);
	if (exit_code) {
		hdd_err("Invalid HDD context");
		goto exit_with_code;
	}

	if (hdd_ctx->config->is_wow_disabled) {
		hdd_err("wow is disabled");
		return -EINVAL;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver is not enabled; Skipping resume");
		exit_code = 0;
		goto exit_with_code;
	}

	pld_request_bus_bandwidth(hdd_ctx->parent_dev, PLD_BUS_WIDTH_MEDIUM);

	status = hdd_resume_wlan();
	if (status != QDF_STATUS_SUCCESS) {
		exit_code = 0;
		goto exit_with_code;
	}
	/* Resume control path scheduler */
	if (hdd_ctx->is_scheduler_suspended) {
		scheduler_resume();
		hdd_ctx->is_scheduler_suspended = false;
	}

	/* Resume tlshim Rx thread */
	if (hdd_ctx->enable_rxthread)
		wlan_hdd_rx_thread_resume(hdd_ctx);

	if (hdd_ctx->enable_dp_rx_threads)
		dp_txrx_resume(cds_get_context(QDF_MODULE_ID_SOC));

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_RESUME_WLAN,
		   NO_SESSION, hdd_ctx->is_wiphy_suspended);

	hdd_ctx->is_wiphy_suspended = false;

	hdd_ctx->suspend_resume_stats.resumes++;
	exit_code = 0;

exit_with_code:
	hdd_exit();
	return exit_code;
}

int wlan_hdd_cfg80211_resume_wlan(struct wiphy *wiphy)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_resume_wlan(wiphy);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

static void hdd_suspend_cb(void)
{
	struct hdd_context *hdd_ctx;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("HDD context is NULL");
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
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct hdd_adapter *adapter;
	mac_handle_t mac_handle;
	int rc;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	rc = wlan_hdd_validate_context(hdd_ctx);
	if (0 != rc)
		return rc;

	if (hdd_ctx->config->is_wow_disabled) {
		hdd_err("wow is disabled");
		return -EINVAL;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Modules not Enabled ");
		return 0;
	}

	mac_handle = hdd_ctx->mac_handle;

	/* If RADAR detection is in progress (HDD), prevent suspend. The flag
	 * "dfs_cac_block_tx" is set to true when RADAR is found and stay true
	 * until CAC is done for a SoftAP which is in started state.
	 */
	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
			continue;

		if (QDF_SAP_MODE == adapter->device_mode) {
			if (BSS_START ==
			    WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter)->bss_state &&
			    true ==
			    WLAN_HDD_GET_AP_CTX_PTR(adapter)->
			    dfs_cac_block_tx) {
				hdd_err("RADAR detection in progress, do not allow suspend");
				wlan_hdd_inc_suspend_stats(hdd_ctx,
							   SUSPEND_FAIL_RADAR);
				return -EAGAIN;
			} else if (!ucfg_pmo_get_enable_sap_suspend(
				   hdd_ctx->psoc)) {
				/* return -EOPNOTSUPP if SAP does not support
				 * suspend
				 */
				hdd_err("SAP does not support suspend!!");
				return -EOPNOTSUPP;
			}
		} else if (QDF_P2P_GO_MODE == adapter->device_mode) {
			if (!ucfg_pmo_get_enable_sap_suspend(
				   hdd_ctx->psoc)) {
				/* return -EOPNOTSUPP if GO does not support
				 * suspend
				 */
				hdd_err("GO does not support suspend!!");
				return -EOPNOTSUPP;
			}
		}
	}
	/* p2p cleanup task based on scheduler */
	ucfg_p2p_cleanup_tx_by_psoc(hdd_ctx->psoc);
	ucfg_p2p_cleanup_roc_by_psoc(hdd_ctx->psoc);

	/* Stop ongoing scan on each interface */
	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (sme_neighbor_middle_of_roaming(mac_handle,
		    adapter->vdev_id) ||
		    hdd_is_roaming_in_progress(hdd_ctx)) {
			hdd_err("Roaming in progress, do not allow suspend");
			wlan_hdd_inc_suspend_stats(hdd_ctx,
						   SUSPEND_FAIL_ROAM);
			return -EAGAIN;
		}

		wlan_abort_scan(hdd_ctx->pdev, INVAL_PDEV_ID,
				adapter->vdev_id, INVALID_SCAN_ID, false);
	}

	/* flush any pending powersave timers */
	hdd_for_each_adapter(hdd_ctx, adapter) {
		if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
			continue;

		sme_ps_timer_flush_sync(mac_handle, adapter->vdev_id);
	}

	/*
	 * Suspend IPA early before proceeding to suspend other entities like
	 * firmware to avoid any race conditions.
	 */
	if (ucfg_ipa_suspend(hdd_ctx->pdev)) {
		hdd_err("IPA not ready to suspend!");
		wlan_hdd_inc_suspend_stats(hdd_ctx, SUSPEND_FAIL_IPA);
		return -EAGAIN;
	}

	/* Suspend control path scheduler */
	scheduler_register_hdd_suspend_callback(hdd_suspend_cb);
	scheduler_set_event_mask(MC_SUSPEND_EVENT);
	scheduler_wake_up_controller_thread();

	/* Wait for suspend confirmation from scheduler */
	rc = wait_for_completion_timeout(&hdd_ctx->mc_sus_event_var,
		msecs_to_jiffies(WLAN_WAIT_TIME_MCTHREAD_SUSPEND));
	if (!rc) {
		scheduler_clear_event_mask(MC_SUSPEND_EVENT);
		hdd_err("Failed to stop mc thread");
		goto resume_tx;
	}
	hdd_ctx->is_scheduler_suspended = true;

	if (hdd_ctx->enable_rxthread) {
		if (wlan_hdd_rx_thread_suspend(hdd_ctx))
			goto resume_ol_rx;
	}

	if (hdd_ctx->enable_dp_rx_threads)
		dp_txrx_suspend(cds_get_context(QDF_MODULE_ID_SOC));

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SUSPEND_WLAN,
		   NO_SESSION, hdd_ctx->is_wiphy_suspended);

	if (hdd_suspend_wlan() < 0) {
		hdd_err("Failed to suspend WLAN");
		goto resume_dp_thread;
	}

	hdd_ctx->is_wiphy_suspended = true;

	pld_request_bus_bandwidth(hdd_ctx->parent_dev, PLD_BUS_WIDTH_NONE);

	hdd_exit();
	return 0;

resume_dp_thread:
	if (hdd_ctx->enable_dp_rx_threads)
		dp_txrx_resume(cds_get_context(QDF_MODULE_ID_SOC));

resume_ol_rx:
	/* Resume tlshim Rx thread */
	wlan_hdd_rx_thread_resume(hdd_ctx);
	scheduler_resume();
	hdd_ctx->is_scheduler_suspended = false;
resume_tx:
	hdd_resume_wlan();
	return -ETIME;

}

int wlan_hdd_cfg80211_suspend_wlan(struct wiphy *wiphy,
				   struct cfg80211_wowlan *wow)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_suspend_wlan(wiphy, wow);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
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
static void hdd_stop_dhcp_ind(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("DHCP stop indicated through power save");
	sme_dhcp_stop_ind(hdd_ctx->mac_handle, adapter->device_mode,
			  adapter->mac_addr.bytes,
			  adapter->vdev_id);
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DHCP);
	qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.connect);
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
static void hdd_start_dhcp_ind(struct hdd_adapter *adapter)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	hdd_debug("DHCP start indicated through power save");
	qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.connect);
	hdd_prevent_suspend_timeout(HDD_WAKELOCK_TIMEOUT_CONNECT,
				    WIFI_POWER_EVENT_WAKELOCK_DHCP);
	sme_dhcp_start_ind(hdd_ctx->mac_handle, adapter->device_mode,
			   adapter->mac_addr.bytes,
			   adapter->vdev_id);
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int status;

	hdd_enter();

	if (timeout < 0) {
		hdd_debug("User space timeout: %d; Enter full power or power save",
			  timeout);
		timeout = 0;
	}

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_POWER_MGMT,
		   adapter->vdev_id, timeout);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Module not enabled return success");
		return 0;
	}

	status = wlan_hdd_set_powersave(adapter, allow_power_save, timeout);

	allow_power_save ? hdd_stop_dhcp_ind(adapter) :
		hdd_start_dhcp_ind(adapter);

	hdd_exit();
	return status;
}

int wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
				     struct net_device *dev,
				     bool allow_power_save,
				     int timeout)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(dev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_power_mgmt(wiphy, dev, allow_power_save,
						   timeout);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * __wlan_hdd_cfg80211_set_txpower() - set TX power
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to network device
 * @type: TX power setting type
 * @mbm: TX power in mBm
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   enum nl80211_tx_power_setting type,
					   int mbm)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	mac_handle_t mac_handle;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BCAST_INIT;
	struct qdf_mac_addr selfmac;
	QDF_STATUS status;
	int errno;
	int dbm;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_SET_TXPOWER,
		   NO_SESSION, type);

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	if (adapter->device_mode == QDF_SAP_MODE ||
	    adapter->device_mode == QDF_P2P_GO_MODE) {
		qdf_copy_macaddr(&bssid, &adapter->mac_addr);
	} else {
		struct hdd_station_ctx *sta_ctx =
			WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		if (eConnectionState_Associated ==
		    sta_ctx->conn_info.conn_state)
			qdf_copy_macaddr(&bssid, &sta_ctx->conn_info.bssid);
	}

	qdf_copy_macaddr(&selfmac, &adapter->mac_addr);

	mac_handle = hdd_ctx->mac_handle;

	dbm = MBM_TO_DBM(mbm);

	/*
	 * the original implementation of this function expected power
	 * values in dBm instead of mBm. If the conversion from mBm to
	 * dBm is zero, then assume dBm was passed.
	 */
	if (!dbm)
		dbm = mbm;

	status = ucfg_mlme_set_current_tx_power_level(hdd_ctx->psoc, dbm);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("sme_cfg_set_int failed for tx power %hu, %d",
			dbm, status);
		return -EIO;
	}

	hdd_debug("Set tx power level %d dbm", dbm);

	switch (type) {
	/* Automatically determine transmit power */
	case NL80211_TX_POWER_AUTOMATIC:
	/* Fall through */
	case NL80211_TX_POWER_LIMITED:
	/* Limit TX power by the mBm parameter */
		status = sme_set_max_tx_power(mac_handle, bssid, selfmac, dbm);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("Setting maximum tx power failed, %d", status);
			return -EIO;
		}
		break;

	case NL80211_TX_POWER_FIXED:    /* Fix TX power to the mBm parameter */
		hdd_err("NL80211_TX_POWER_FIXED not supported");
		return -EOPNOTSUPP;

	default:
		hdd_err("Invalid power setting type %d", type);
		return -EIO;
	}

	hdd_exit();
	return 0;
}

int wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  enum nl80211_tx_power_setting type,
				  int mbm)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_txpower(wiphy, wdev, type, mbm);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

#ifdef QCA_SUPPORT_CP_STATS
static void wlan_hdd_get_tx_power(struct hdd_adapter *adapter, int *dbm)
{
	wlan_cfg80211_mc_cp_stats_get_tx_power(adapter->vdev, dbm);
}
#else
static void wlan_hdd_get_tx_power(struct hdd_adapter *adapter, int *dbm)
{
	wlan_hdd_get_class_astats(adapter);
	*dbm = adapter->hdd_stats.class_a_stat.max_pwr;
}
#endif
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

	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	struct net_device *ndev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	int status;
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_enter_dev(ndev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	*dbm = 0;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return status;

	/* Validate adapter sessionId */
	status = wlan_hdd_validate_vdev_id(adapter->vdev_id);
	if (status)
		return status;

	if (sta_ctx->hdd_reassoc_scenario) {
		hdd_debug("Roaming is in progress, rej this req");
		return -EINVAL;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Module not enabled return success");
		/* Send cached data to upperlayer*/
		*dbm = adapter->hdd_stats.class_a_stat.max_pwr;
		return 0;
	}

	if (sta_ctx->conn_info.conn_state != eConnectionState_Associated) {
		hdd_debug("Not associated");
		return 0;
	}

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_GET_TXPOWER,
		   adapter->vdev_id, adapter->device_mode);

	wlan_hdd_get_tx_power(adapter, dbm);
	hdd_debug("power: %d", *dbm);

	return 0;
}

int wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 int *dbm)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_get_txpower(wiphy, wdev, dbm);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

int hdd_set_qpower_config(struct hdd_context *hddctx,
			  struct hdd_adapter *adapter,
			  u8 qpower)
{
	QDF_STATUS status;

	if (!ucfg_pmo_get_power_save_mode(hddctx->psoc)) {
		hdd_err("qpower is disabled in configuration");
		return -EINVAL;
	}
	if (adapter->device_mode != QDF_STA_MODE &&
	    adapter->device_mode != QDF_P2P_CLIENT_MODE) {
		hdd_info("QPOWER only allowed in STA/P2P-Client modes:%d",
			 adapter->device_mode);
		return -EINVAL;
	}

	if (qpower > PS_DUTY_CYCLING_QPOWER ||
	    qpower < PS_LEGACY_NODEEPSLEEP) {
		hdd_err("invalid qpower value: %d", qpower);
		return -EINVAL;
	}

	if (ucfg_pmo_get_max_ps_poll(hddctx->psoc)) {
		if ((qpower == PS_QPOWER_NODEEPSLEEP) ||
				(qpower == PS_LEGACY_NODEEPSLEEP))
			qpower = PS_LEGACY_NODEEPSLEEP;
		else
			qpower = PS_LEGACY_DEEPSLEEP;
		hdd_info("Qpower disabled, %d", qpower);
	}
	status = wma_set_qpower_config(adapter->vdev_id, qpower);
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

	hdd_info("Unit-test resume WLAN");

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
		hdd_alert("Not unit-test suspended; Nothing to do");
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

	hdd_alert("Unit-test resume succeeded");
}

/**
 * hdd_wlan_fake_apps_resume_irq_callback() - Irq callback function for resuming
 *	from unit-test initiated suspend from irq wakeup signal
 *
 * Resume wlan after getting very 1st CE interrupt from target
 *
 * Return: none
 */
static void hdd_wlan_fake_apps_resume_irq_callback(void)
{
	hdd_info("Trigger unit-test resume WLAN");

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
	int errno;
	qdf_device_t qdf_dev;
	struct hif_opaque_softc *hif_ctx;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct wow_enable_params wow_params = {
		.is_unit_test = true,
		.interface_pause = pause_setting,
		.resume_trigger = resume_setting
	};

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (!hdd_ctx->config->is_unit_test_framework_enabled) {
		hdd_warn_rl("UT framework is disabled");
		return -EINVAL;
	}

	hdd_info("Unit-test suspend WLAN");

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
		hdd_alert("Already unit-test suspended; Nothing to do");
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
	hif_ut_apps_suspend(hif_ctx, hdd_wlan_fake_apps_resume_irq_callback);

	/* re-enable wake irq */
	errno = hif_apps_wake_irq_enable(hif_ctx);
	if (errno)
		goto fake_apps_resume;

	/*
	 * Tell the kernel not to worry if TX queues aren't moving. This is
	 * expected since we are suspending the wifi hardware, but not APPS
	 */
	dev->watchdog_timeo = INT_MAX;

	hdd_alert("Unit-test suspend succeeded");

	return 0;

fake_apps_resume:
	hif_ut_apps_resume(hif_ctx);

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
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (!hdd_ctx->config->is_unit_test_framework_enabled) {
		hdd_warn_rl("UT framework is disabled");
		return -EINVAL;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("Failed to get HIF context");
		return -EINVAL;
	}

	hif_ut_apps_resume(hif_ctx);
	__hdd_wlan_fake_apps_resume(wiphy, dev);

	return 0;
}
#endif
