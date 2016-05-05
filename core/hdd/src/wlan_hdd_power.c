/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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
#include <dbglog_host.h>
#include <wlan_hdd_trace.h>

#include <linux/semaphore.h>
#include <wlan_hdd_hostapd.h>
#include "cfg_api.h"

#include <linux/inetdevice.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_scan.h>
#include <wlan_hdd_cfg80211.h>
#include <net/addrconf.h>
#include <wlan_hdd_ipa.h>

#include <wma_types.h>
#include "hif.h"
#include "sme_power_save_api.h"
#include "cds_concurrency.h"
#include "cdp_txrx_flow_ctrl_v2.h"
#include "pld_common.h"

/* Preprocessor definitions and constants */
#define HDD_SSR_BRING_UP_TIME 30000

/* Type declarations */

/**
 * enum hdd_power_mode - Power Mode enumerations
 * @DRIVER_POWER_MODE_AUTO: Driver can place device into power save
 * @DRIVER_POWER_MODE_ACTIVE: Driver should operate at full power
 */
enum hdd_power_mode {
	DRIVER_POWER_MODE_AUTO = 0,
	DRIVER_POWER_MODE_ACTIVE = 1,
};

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
 * hdd_conf_gtk_offload() - Configure GTK offload
 * @pAdapter:   pointer to the adapter
 * @fenable:    flag set to enable (1) or disable (0) GTK offload
 *
 * Central function to enable or disable GTK offload.
 *
 * Return: nothing
 */
#ifdef WLAN_FEATURE_GTK_OFFLOAD
static void hdd_conf_gtk_offload(hdd_adapter_t *pAdapter, bool fenable)
{
	QDF_STATUS ret;
	tSirGtkOffloadParams hddGtkOffloadReqParams;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	if (fenable) {
		if ((eConnectionState_Associated ==
		     pHddStaCtx->conn_info.connState)
		    && (GTK_OFFLOAD_ENABLE ==
			pHddStaCtx->gtkOffloadReqParams.ulFlags)) {
			qdf_mem_copy(&hddGtkOffloadReqParams,
				     &pHddStaCtx->gtkOffloadReqParams,
				     sizeof(tSirGtkOffloadParams));

			ret = sme_set_gtk_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
						  &hddGtkOffloadReqParams,
						  pAdapter->sessionId);
			if (QDF_STATUS_SUCCESS != ret) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: sme_set_gtk_offload failed, returned %d",
					  __func__, ret);
				return;
			}

			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
				  "%s: sme_set_gtk_offload successfull",
				  __func__);
		}

	} else {
		if ((eConnectionState_Associated ==
		     pHddStaCtx->conn_info.connState)
		    && (qdf_is_macaddr_equal(&pHddStaCtx->gtkOffloadReqParams.bssid,
			       &pHddStaCtx->conn_info.bssId))
		    && (GTK_OFFLOAD_ENABLE ==
			pHddStaCtx->gtkOffloadReqParams.ulFlags)) {

			/* Host driver has previously offloaded GTK rekey  */
			ret = sme_get_gtk_offload
				(WLAN_HDD_GET_HAL_CTX(pAdapter),
				 wlan_hdd_cfg80211_update_replay_counter_callback,
				 pAdapter, pAdapter->sessionId);
			if (QDF_STATUS_SUCCESS != ret) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: sme_get_gtk_offload failed, returned %d",
					  __func__, ret);
				return;
			} else {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: sme_get_gtk_offload successful",
					  __func__);

				/* Sending GTK offload dissable */
				memcpy(&hddGtkOffloadReqParams,
				       &pHddStaCtx->gtkOffloadReqParams,
				       sizeof(tSirGtkOffloadParams));
				hddGtkOffloadReqParams.ulFlags =
					GTK_OFFLOAD_DISABLE;
				ret =
					sme_set_gtk_offload(WLAN_HDD_GET_HAL_CTX
								    (pAdapter),
							    &hddGtkOffloadReqParams,
							    pAdapter->sessionId);
				if (QDF_STATUS_SUCCESS != ret) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: failed to dissable GTK offload, returned %d",
						  __func__, ret);
					return;
				}
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: successfully dissabled GTK offload request to HAL",
					  __func__);
			}
		}
	}
	return;
}
#else /* WLAN_FEATURE_GTK_OFFLOAD */
static void hdd_conf_gtk_offload(hdd_adapter_t *pAdapter, bool fenable)
{
}
#endif /*WLAN_FEATURE_GTK_OFFLOAD */

#ifdef WLAN_NS_OFFLOAD
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
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	if ((pAdapter == NULL) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		hddLog(LOGE, FL("Adapter context is invalid %p"), pAdapter);
		return -EINVAL;
	}

	if ((pAdapter->dev == ndev) &&
		(pAdapter->device_mode == QDF_STA_MODE ||
		pAdapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
		status = wlan_hdd_validate_context(pHddCtx);
		if (0 != status)
			return NOTIFY_DONE;

		schedule_work(&pAdapter->ipv6NotifierWorkQueue);
	}
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
 * hdd_conf_ns_offload() - Configure NS offload
 * @pAdapter:   pointer to the adapter
 * @fenable:    flag to enable or disable
 *              0 - disable
 *              1 - enable
 *
 * Return: nothing
 */
static void hdd_conf_ns_offload(hdd_adapter_t *pAdapter, bool fenable)
{
	struct inet6_dev *in6_dev;
	struct inet6_ifaddr *ifp;
	struct list_head *p;
	uint8_t
		selfIPv6Addr[SIR_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA]
	[SIR_MAC_IPV6_ADDR_LEN] = { {0,} };
	bool selfIPv6AddrValid[SIR_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA] = { 0 };
	tSirHostOffloadReq offLoadRequest;
	hdd_context_t *pHddCtx;

	int i = 0;
	QDF_STATUS returnStatus;
	uint32_t count = 0, scope;

	ENTER();
	hddLog(LOG1, FL(" fenable = %d"), fenable);

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	/* In SAP/P2PGo mode, ARP/NS offload feature capability
	 * is controlled by one bit.
	 */

	if ((QDF_SAP_MODE == pAdapter->device_mode ||
		QDF_P2P_GO_MODE == pAdapter->device_mode) &&
		!pHddCtx->ap_arpns_support) {
		hddLog(LOG1,
			FL("NS Offload is not supported in SAP/P2PGO mode"));
		return;
	}

	if (fenable) {
		in6_dev = __in6_dev_get(pAdapter->dev);
		if (NULL != in6_dev) {
			/* read_lock_bh(&in6_dev->lock); */
			list_for_each(p, &in6_dev->addr_list) {
				if (count >=
					SIR_MAC_NUM_TARGET_IPV6_NS_OFFLOAD_NA) {
					hdd_err("Reached max supported NS Offload addresses");
					break;
				}
				ifp =
					list_entry(p, struct inet6_ifaddr, if_list);
				scope = ipv6_addr_src_scope(&ifp->addr);

				switch (scope) {
				case IPV6_ADDR_SCOPE_GLOBAL:
				case IPV6_ADDR_SCOPE_LINKLOCAL:
					qdf_mem_copy(&selfIPv6Addr[count],
						&ifp->addr.s6_addr,
						sizeof(ifp->addr.s6_addr));
					selfIPv6AddrValid[count] =
						SIR_IPV6_ADDR_VALID;
					hdd_info("Index %d scope = %s Address : %pI6",
					  count,
					  (scope == IPV6_ADDR_SCOPE_LINKLOCAL) ?
					  "LINK LOCAL" : "GLOBAL",
					  selfIPv6Addr[count]);
					count += 1;
					break;
				default:
					hdd_err("The Scope %d is not supported",
						scope);
				}
			}
			/* read_unlock_bh(&in6_dev->lock); */
			qdf_mem_zero(&offLoadRequest, sizeof(offLoadRequest));
			for (i = 0; i < count; i++) {
				/* Filling up the request structure
				 * Filling the selfIPv6Addr with solicited address
				 * A Solicited-Node multicast address is created by
				 * taking the last 24 bits of a unicast or anycast
				 * address and appending them to the prefix
				 *
				 * FF02:0000:0000:0000:0000:0001:FFXX:XXXX
				 *
				 * here XX is the unicast/anycast bits
				 */
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][0] = 0xFF;
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][1] = 0x02;
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][11] = 0x01;
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][12] = 0xFF;
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][13] =
					selfIPv6Addr[i][13];
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][14] =
					selfIPv6Addr[i][14];
				offLoadRequest.nsOffloadInfo.selfIPv6Addr[i][15] =
					selfIPv6Addr[i][15];
				offLoadRequest.nsOffloadInfo.slotIdx = i;
				qdf_mem_copy(&offLoadRequest.nsOffloadInfo.targetIPv6Addr[i],
					&selfIPv6Addr[i][0], SIR_MAC_IPV6_ADDR_LEN);

				offLoadRequest.nsOffloadInfo.targetIPv6AddrValid[i] =
					SIR_IPV6_ADDR_VALID;

				hdd_info("configuredMcastBcastFilter: %d",
					pHddCtx->configuredMcastBcastFilter);

				if ((true == pHddCtx->sus_res_mcastbcast_filter_valid)
					&& ((HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST ==
					pHddCtx->sus_res_mcastbcast_filter) ||
					(HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST ==
					pHddCtx->sus_res_mcastbcast_filter))) {
					hdd_info("Set offLoadRequest with SIR_OFFLOAD_NS_AND_MCAST_FILTER_ENABLE");
					offLoadRequest.enableOrDisable =
						SIR_OFFLOAD_NS_AND_MCAST_FILTER_ENABLE;
				}

				qdf_mem_copy(&offLoadRequest.params.hostIpv6Addr,
					&offLoadRequest.nsOffloadInfo.targetIPv6Addr[i],
					SIR_MAC_IPV6_ADDR_LEN);

				hdd_info("Setting NSOffload with solicitedIp: %pI6, targetIp: %pI6, Index %d",
					&offLoadRequest.nsOffloadInfo.selfIPv6Addr[i],
					&offLoadRequest.nsOffloadInfo.targetIPv6Addr[i], i);
			}
			hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD,
				SIR_OFFLOAD_ENABLE);
			offLoadRequest.offloadType =  SIR_IPV6_NS_OFFLOAD;
			offLoadRequest.enableOrDisable = SIR_OFFLOAD_ENABLE;
			qdf_copy_macaddr(&offLoadRequest.nsOffloadInfo.self_macaddr,
					 &pAdapter->macAddressCurrent);
			/* set number of ns offload address count */
			offLoadRequest.num_ns_offload_count = count;
			/* Configure the Firmware with this */
			returnStatus = sme_set_host_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
				pAdapter->sessionId, &offLoadRequest);
			if (QDF_STATUS_SUCCESS != returnStatus) {
				hdd_err("Failed to enable HostOffload feature with status: %d",
					returnStatus);
			}
		} else {
			hdd_err("IPv6 dev does not exist. Failed to request NSOffload");
			return;
		}
	} else {
		hdd_wlan_offload_event(SIR_IPV6_NS_OFFLOAD,
			SIR_OFFLOAD_DISABLE);
		/* Disable NSOffload */
		qdf_mem_zero((void *)&offLoadRequest, sizeof(tSirHostOffloadReq));
		offLoadRequest.enableOrDisable = SIR_OFFLOAD_DISABLE;
		offLoadRequest.offloadType =  SIR_IPV6_NS_OFFLOAD;

		if (QDF_STATUS_SUCCESS !=
			sme_set_host_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
				pAdapter->sessionId, &offLoadRequest))
				hdd_err("Failed to disable NS Offload");
	}
	EXIT();
	return;
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
void __hdd_ipv6_notifier_work_queue(struct work_struct *work)
{
	hdd_adapter_t *pAdapter =
		container_of(work, hdd_adapter_t, ipv6NotifierWorkQueue);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status)
		return;

	if (!pHddCtx->config->active_mode_offload) {
		hdd_err("Active mode offload is disabled");
		return;
	}

	if (false == pHddCtx->sus_res_mcastbcast_filter_valid) {
		pHddCtx->sus_res_mcastbcast_filter =
			pHddCtx->configuredMcastBcastFilter;
		pHddCtx->sus_res_mcastbcast_filter_valid = true;
	}

	if ((eConnectionState_Associated ==
	     (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState))
		if (pHddCtx->config->fhostNSOffload)
			hdd_conf_ns_offload(pAdapter, true);
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

/**
 * hdd_conf_hostoffload() - Central function to configure the supported offloads
 * @pAdapter:   pointer to the adapter
 * @fenable:    flag set to enable (1) or disable (0)
 *
 * Central function to configure the supported offloads either
 * enable or disable them.
 *
 * Return: nothing
 */
void hdd_conf_hostoffload(hdd_adapter_t *pAdapter, bool fenable)
{
	hdd_context_t *pHddCtx;

	ENTER();

	hdd_info("Configuring offloads with flag: %d", fenable);

	/* Get the HDD context. */
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (((QDF_STA_MODE != pAdapter->device_mode) &&
	     (QDF_P2P_CLIENT_MODE != pAdapter->device_mode))) {
		hdd_err("Offloads not supported in mode %d",
			pAdapter->device_mode);
		return;
	}

	if (eConnectionState_Associated !=
	       (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState) {
		hdd_err("Offloads not supported in state %d",
			(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->
							conn_info.connState);
		return;
	}

	hdd_conf_gtk_offload(pAdapter, fenable);

	/* Configure ARP/NS offload during cfg80211 suspend/resume and
	 * Enable MC address filtering during cfg80211 suspend
	 * only if active mode offload is disabled
	 */
	if (!pHddCtx->config->active_mode_offload) {
		hdd_info("configuring unconfigured active mode offloads");
		hdd_conf_arp_offload(pAdapter, fenable);
		wlan_hdd_set_mc_addr_list(pAdapter, fenable);

		if (pHddCtx->config->fhostNSOffload)
			hdd_conf_ns_offload(pAdapter, fenable);
	}
	EXIT();
	return;
}
#endif

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
void __hdd_ipv4_notifier_work_queue(struct work_struct *work)
{
	hdd_adapter_t *pAdapter =
		container_of(work, hdd_adapter_t, ipv4NotifierWorkQueue);
	hdd_context_t *pHddCtx;
	int status;

	hdd_info("Configuring ARP Offload");
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);
	if (status)
		return;

	if (!pHddCtx->config->active_mode_offload) {
		hdd_err("Active mode offload is disabled");
		return;
	}

	if (false == pHddCtx->sus_res_mcastbcast_filter_valid) {
		pHddCtx->sus_res_mcastbcast_filter =
			pHddCtx->configuredMcastBcastFilter;
		pHddCtx->sus_res_mcastbcast_filter_valid = true;
	}

	if ((eConnectionState_Associated ==
	     (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState))
		hdd_conf_arp_offload(pAdapter, true);
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
	struct in_ifaddr **ifap = NULL;
	struct in_device *in_dev;

	struct net_device *ndev = ifa->ifa_dev->dev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(ndev);
	hdd_context_t *pHddCtx;
	int status;

	ENTER();

	if ((pAdapter == NULL) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)) {
		hddLog(LOGE, FL("Adapter context is invalid %p"), pAdapter);
		return -EINVAL;
	}

	if ((pAdapter && pAdapter->dev == ndev) &&
		(pAdapter->device_mode == QDF_STA_MODE ||
		pAdapter->device_mode == QDF_P2P_CLIENT_MODE)) {

		pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
		status = wlan_hdd_validate_context(pHddCtx);
		if (0 != status)
			return NOTIFY_DONE;

		if (!pHddCtx->config->fhostArpOffload) {
			hddLog(LOG1,
				FL("Offload not enabled ARPOffload=%d"),
				pHddCtx->config->fhostArpOffload);
			return NOTIFY_DONE;
		}

		in_dev = __in_dev_get_rtnl(pAdapter->dev);
		if (in_dev) {
			for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
			     ifap = &ifa->ifa_next) {
				if (!strcmp(pAdapter->dev->name,
					    ifa->ifa_label)) {
					break;  /* found */
				}
			}
		}
		if (ifa && ifa->ifa_local) {
			schedule_work(&pAdapter->ipv4NotifierWorkQueue);
		}
	}
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
 * hdd_conf_arp_offload() - Configure ARP offload
 * @pAdapter: Adapter context for which ARP offload is to be configured
 * @fenable: true : enable ARP offload false : disable arp offload
 *
 * Return:
 *	QDF_STATUS_SUCCESS - on successful operation,
 *	QDF_STATUS_E_FAILURE - on failure of operation
 */
QDF_STATUS hdd_conf_arp_offload(hdd_adapter_t *pAdapter, bool fenable)
{
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct in_device *in_dev;
	int i = 0;
	tSirHostOffloadReq offLoadRequest;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	hdd_info("fenable = %d", fenable);

	/* In SAP/P2P Go mode, ARP/NS Offload feature capability
	 * is controlled by one bit.
	 */
	if ((QDF_SAP_MODE == pAdapter->device_mode ||
		QDF_P2P_GO_MODE == pAdapter->device_mode) &&
		!pHddCtx->ap_arpns_support) {
		hddLog(LOG1,
			FL("ARP Offload is not supported in SAP/P2PGO mode"));
		return QDF_STATUS_SUCCESS;
	}

	if (fenable) {
		in_dev = __in_dev_get_rtnl(pAdapter->dev);
		if (in_dev) {
			for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
			     ifap = &ifa->ifa_next) {
				if (!strcmp(pAdapter->dev->name,
					    ifa->ifa_label)) {
					break;  /* found */
				}
			}
		}
		if (ifa && ifa->ifa_local) {
			offLoadRequest.offloadType = SIR_IPV4_ARP_REPLY_OFFLOAD;
			offLoadRequest.enableOrDisable = SIR_OFFLOAD_ENABLE;
			hdd_wlan_offload_event(SIR_IPV4_ARP_REPLY_OFFLOAD,
				SIR_OFFLOAD_ENABLE);

			hddLog(QDF_TRACE_LEVEL_INFO, "%s: Enabled", __func__);

			if (((HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST ==
			      pHddCtx->sus_res_mcastbcast_filter) ||
			     (HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST
			      == pHddCtx->sus_res_mcastbcast_filter))
			    && (true ==
				pHddCtx->sus_res_mcastbcast_filter_valid)) {
				offLoadRequest.enableOrDisable =
					SIR_OFFLOAD_ARP_AND_BCAST_FILTER_ENABLE;
				hddLog(QDF_TRACE_LEVEL_INFO,
				       "offload: inside arp offload conditional check");
			}
			hdd_wlan_offload_event(
				SIR_OFFLOAD_ARP_AND_BCAST_FILTER_ENABLE,
				SIR_OFFLOAD_ENABLE);
			hddLog(QDF_TRACE_LEVEL_INFO,
			       "offload: arp filter programmed = %d",
			       offLoadRequest.enableOrDisable);

			/* converting u32 to IPV4 address */
			for (i = 0; i < 4; i++) {
				offLoadRequest.params.hostIpv4Addr[i] =
					(ifa->ifa_local >> (i * 8)) & 0xFF;
			}
			hddLog(QDF_TRACE_LEVEL_INFO,
			       " Enable SME HostOffload: %d.%d.%d.%d",
			       offLoadRequest.params.hostIpv4Addr[0],
			       offLoadRequest.params.hostIpv4Addr[1],
			       offLoadRequest.params.hostIpv4Addr[2],
			       offLoadRequest.params.hostIpv4Addr[3]);

			if (QDF_STATUS_SUCCESS !=
			    sme_set_host_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
						 pAdapter->sessionId,
						 &offLoadRequest)) {
				hddLog(QDF_TRACE_LEVEL_ERROR,
				       "%s: Failed to enable HostOffload feature",
				       __func__);
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       FL("IP Address is not assigned"));
		}

		return QDF_STATUS_SUCCESS;
	} else {
		hdd_wlan_offload_event(SIR_IPV4_ARP_REPLY_OFFLOAD,
			SIR_OFFLOAD_DISABLE);
		qdf_mem_zero((void *)&offLoadRequest,
			     sizeof(tSirHostOffloadReq));
		offLoadRequest.enableOrDisable = SIR_OFFLOAD_DISABLE;
		offLoadRequest.offloadType = SIR_IPV4_ARP_REPLY_OFFLOAD;

		if (QDF_STATUS_SUCCESS !=
		    sme_set_host_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
					 pAdapter->sessionId, &offLoadRequest)) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Failure to disable host " "offload feature",
			       __func__);
			return QDF_STATUS_E_FAILURE;
		}
		return QDF_STATUS_SUCCESS;
	}
}

/**
 * hdd_mcbc_filter_modification() - MCBC Filter Modifier
 * @pHddCtx: Global Hdd Context
 * @pMcBcFilter: Multicast/Broadcast filter to be modified
 *
 * This function is called before setting mcbc filters
 * to modify filter value considering different offloads
 *
 * Return: None.
 */
static void hdd_mcbc_filter_modification(hdd_context_t *pHddCtx,
					 uint8_t *pMcBcFilter)
{
	if (NULL == pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("NULL HDD context passed"));
		return;
	}

	*pMcBcFilter = pHddCtx->configuredMcastBcastFilter;
	if (pHddCtx->config->fhostArpOffload) {
		/* ARP offload is enabled, do not block bcast packets at RXP
		 * Will be using Bitmasking to reset the filter. As we have
		 * disable Broadcast filtering, Anding with the negation
		 * of Broadcast BIT
		 */
		*pMcBcFilter &= ~(HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST);
		hdd_info("ARP offload is enabled");
	}
#ifdef WLAN_NS_OFFLOAD
	if (pHddCtx->config->fhostNSOffload) {
		/* NS offload is enabled, do not block mcast packets at RXP
		 * Will be using Bitmasking to reset the filter. As we have
		 * disable Multicast filtering, Anding with the negation
		 * of Multicast BIT
		 */
		hdd_info("NS offload is enabled");
		*pMcBcFilter &= ~(HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST);
	}
#endif

	pHddCtx->configuredMcastBcastFilter = *pMcBcFilter;
}

/**
 * hdd_conf_mcastbcast_filter() - Configure multicast/broadcast filter
 * @pHddCtx: Global HDD context
 * @setfilter: true if filter is being set, false if filter is being cleared
 *
 * Return: None.
 */
void hdd_conf_mcastbcast_filter(hdd_context_t *pHddCtx, bool setfilter)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;
	tpSirWlanSetRxpFilters wlanRxpFilterParam =
		qdf_mem_malloc(sizeof(tSirWlanSetRxpFilters));
	if (NULL == wlanRxpFilterParam) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: qdf_mem_malloc failed ", __func__);
		return;
	}
	hddLog(QDF_TRACE_LEVEL_INFO,
	       "%s: Configuring Mcast/Bcast Filter Setting. setfilter %d",
	       __func__, setfilter);
	if (true == setfilter) {
		hdd_mcbc_filter_modification(pHddCtx,
					     &wlanRxpFilterParam->
					     configuredMcstBcstFilterSetting);
	} else {
		/*Use the current configured value to clear */
		wlanRxpFilterParam->configuredMcstBcstFilterSetting =
			pHddCtx->configuredMcastBcastFilter;
	}

	wlanRxpFilterParam->setMcstBcstFilter = setfilter;
	qdf_ret_status =
		sme_configure_rxp_filter(pHddCtx->hHal, wlanRxpFilterParam);

	if (setfilter && (QDF_STATUS_SUCCESS == qdf_ret_status))
		pHddCtx->hdd_mcastbcast_filter_set = true;

	hddLog(LOG1,
		FL("%s to post set/reset filter to lower mac with status %d configuredMcstBcstFilterSetting = %d setMcstBcstFilter = %d"),
		(QDF_STATUS_SUCCESS != qdf_ret_status) ? "Failed" : "Success",
		qdf_ret_status,
		wlanRxpFilterParam->configuredMcstBcstFilterSetting,
		wlanRxpFilterParam->setMcstBcstFilter);

	if (QDF_STATUS_SUCCESS != qdf_ret_status)
		qdf_mem_free(wlanRxpFilterParam);
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * wlan_hdd_set_mc_addr_list() - set MC address list in FW
 * @pAdapter: adapter whose MC list is being set
 * @set: flag which indicates if addresses are being set or cleared
 */
void wlan_hdd_set_mc_addr_list(hdd_adapter_t *pAdapter, uint8_t set)
{
	uint8_t i;
	tpSirRcvFltMcAddrList pMulticastAddrs = NULL;
	tHalHandle hHal = NULL;
	hdd_context_t *pHddCtx = (hdd_context_t *) pAdapter->pHddCtx;

	if (NULL == pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD CTX is NULL"));
		return;
	}

	hHal = pHddCtx->hHal;

	if (NULL == hHal) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HAL Handle is NULL"));
		return;
	}

	/* Check if INI is enabled or not, other wise just return
	 */
	if (!pHddCtx->config->fEnableMCAddrList) {
		hddLog(QDF_TRACE_LEVEL_INFO,
		       FL("gMCAddrListEnable is not enabled in INI"));
		return;
	}
	pMulticastAddrs = qdf_mem_malloc(sizeof(tSirRcvFltMcAddrList));
	if (NULL == pMulticastAddrs) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Could not allocate Memory"));
		return;
	}
	qdf_mem_zero(pMulticastAddrs, sizeof(tSirRcvFltMcAddrList));
	pMulticastAddrs->action = set;

	if (set) {
		/* Following pre-conditions should be satisfied before we
		 * configure the MC address list.
		 */
		if (((pAdapter->device_mode == QDF_STA_MODE)
		     || (pAdapter->device_mode == QDF_P2P_CLIENT_MODE))
		    && pAdapter->mc_addr_list.mc_cnt
		    && (eConnectionState_Associated ==
			(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->
			conn_info.connState)) {
			pMulticastAddrs->ulMulticastAddrCnt =
				pAdapter->mc_addr_list.mc_cnt;
			for (i = 0; i < pAdapter->mc_addr_list.mc_cnt;
			     i++) {
				memcpy(pMulticastAddrs->multicastAddr[i].bytes,
				       pAdapter->mc_addr_list.addr[i],
				       sizeof(pAdapter->mc_addr_list.
					      addr[i]));
				hdd_info("%s multicast filter: addr ="
				       MAC_ADDRESS_STR,
				       set ? "setting" : "clearing",
				       MAC_ADDR_ARRAY(pMulticastAddrs->
						      multicastAddr[i].bytes));
			}
			/* Set multicast filter */
			sme_8023_multicast_list(hHal, pAdapter->sessionId,
						pMulticastAddrs);
		}
	} else {
		/* Need to clear only if it was previously configured */
		if (pAdapter->mc_addr_list.isFilterApplied) {
			pMulticastAddrs->ulMulticastAddrCnt =
				pAdapter->mc_addr_list.mc_cnt;
			for (i = 0; i < pAdapter->mc_addr_list.mc_cnt;
			     i++) {
				memcpy(pMulticastAddrs->multicastAddr[i].bytes,
				       pAdapter->mc_addr_list.addr[i],
				       sizeof(pAdapter->mc_addr_list.
					      addr[i]));
			}
			sme_8023_multicast_list(hHal, pAdapter->sessionId,
						pMulticastAddrs);
		}

	}
	/* MAddrCnt is MulticastAddrCnt */
	hddLog(QDF_TRACE_LEVEL_INFO,
	       "smeSessionId:%d; set:%d; MCAdddrCnt :%d",
	       pAdapter->sessionId, set,
	       pMulticastAddrs->ulMulticastAddrCnt);

	pAdapter->mc_addr_list.isFilterApplied = set ? true : false;
	qdf_mem_free(pMulticastAddrs);
	return;
}
#endif

/**
 * hdd_update_mcastbcast_filter(): cache multi and broadcast filter for suspend
 * @hdd_ctx: hdd context
 *
 * Cache the configured filter to be used in suspend resume.
 */
static void hdd_update_mcastbcast_filter(hdd_context_t *hdd_ctx)
{
	if (false == hdd_ctx->sus_res_mcastbcast_filter_valid) {
		hdd_ctx->sus_res_mcastbcast_filter =
			hdd_ctx->configuredMcastBcastFilter;
		hdd_ctx->sus_res_mcastbcast_filter_valid = true;
		hdd_info("configuredMCastBcastFilter saved = %d",
			hdd_ctx->configuredMcastBcastFilter);
	}
}

/**
 * hdd_conf_suspend_ind() - Send Suspend notification
 * @pHddCtx: HDD Global context
 * @pAdapter: adapter being suspended
 * @callback: callback function to be called upon completion
 * @callbackContext: callback context to be passed back to callback function
 *
 * Return: None.
 */
static void hdd_send_suspend_ind(hdd_context_t *pHddCtx,
				uint32_t conn_state_mask,
				 void (*callback)(void *callbackContext,
						  bool suspended),
				 void *callbackContext)
{
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	hdd_info("%s: send wlan suspend indication", __func__);

	qdf_ret_status =
		sme_configure_suspend_ind(pHddCtx->hHal, conn_state_mask,
					  callback, callbackContext);

	if (QDF_STATUS_SUCCESS == qdf_ret_status) {
		pHddCtx->hdd_mcastbcast_filter_set = true;
	} else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("sme_configure_suspend_ind returned failure %d"),
		       qdf_ret_status);
	}
}

/**
 * hdd_conf_suspend_ind() - Send Resume notification
 * @pAdapter: adapter being resumed
 *
 * Return: None.
 */
static void hdd_conf_resume_ind(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	QDF_STATUS qdf_ret_status = QDF_STATUS_E_FAILURE;

	qdf_ret_status = sme_configure_resume_req(pHddCtx->hHal, NULL);

	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: sme_configure_resume_req return failure %d",
		       __func__, qdf_ret_status);

	}

	hddLog(QDF_TRACE_LEVEL_INFO,
	       "%s: send wlan resume indication", __func__);
	/* Disable supported OffLoads */
	hdd_conf_hostoffload(pAdapter, false);
	pHddCtx->hdd_mcastbcast_filter_set = false;

	if (true == pHddCtx->sus_res_mcastbcast_filter_valid) {
		pHddCtx->configuredMcastBcastFilter =
			pHddCtx->sus_res_mcastbcast_filter;
		pHddCtx->sus_res_mcastbcast_filter_valid = false;
	}

	hddLog(QDF_TRACE_LEVEL_INFO,
	       "offload: in hdd_conf_resume_ind, restoring configuredMcastBcastFilter");
	hddLog(QDF_TRACE_LEVEL_INFO, "configuredMcastBcastFilter = %d",
	       pHddCtx->configuredMcastBcastFilter);
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
 * Return: None.
 */
static void
hdd_suspend_wlan(void (*callback)(void *callbackContext, bool suspended),
		 void *callbackContext)
{
	hdd_context_t *pHddCtx;

	QDF_STATUS status;
	hdd_adapter_t *pAdapter = NULL;
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	uint32_t conn_state_mask = 0;

	hdd_info("%s: WLAN being suspended by OS", __func__);

	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HDD context is Null",
		       __func__);
		return;
	}

	if (cds_is_driver_recovering()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore suspend!!!",
			 cds_get_driver_state());
		return;
	}

	hdd_update_mcastbcast_filter(pHddCtx);

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		/* stop all TX queues before suspend */
		hddLog(LOG1, FL("Disabling queues"));
		wlan_hdd_netif_queue_control(pAdapter, WLAN_NETIF_TX_DISABLE,
					   WLAN_CONTROL_PATH);

		/* Configure supported OffLoads */
		hdd_conf_hostoffload(pAdapter, true);

		hdd_update_conn_state_mask(pAdapter, &conn_state_mask);

		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);

		pAdapterNode = pNext;
	}

	hdd_send_suspend_ind(pHddCtx, conn_state_mask, callback,
			callbackContext);

	pHddCtx->hdd_wlan_suspended = true;
	hdd_wlan_suspend_resume_event(HDD_WLAN_EARLY_SUSPEND);

	return;
}

/**
 * hdd_resume_wlan() - Driver resume function
 *
 * Return: None.
 */
static void hdd_resume_wlan(void)
{
	hdd_context_t *pHddCtx;
	hdd_adapter_t *pAdapter = NULL;
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;

	hddLog(QDF_TRACE_LEVEL_INFO, "%s: WLAN being resumed by OS",
	       __func__);

	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HDD context is Null",
		       __func__);
		return;
	}

	if (cds_is_driver_recovering()) {
		hdd_warn("Recovery in Progress. State: 0x%x Ignore resume!!!",
			 cds_get_driver_state());
		return;
	}

	pHddCtx->hdd_wlan_suspended = false;
	hdd_wlan_suspend_resume_event(HDD_WLAN_EARLY_RESUME);

	/*loop through all adapters. Concurrency */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);

	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;

		/* wake the tx queues */
		hddLog(LOG1, FL("Enabling queues"));
		wlan_hdd_netif_queue_control(pAdapter,
					WLAN_WAKE_ALL_NETIF_QUEUE,
					WLAN_CONTROL_PATH);

		hdd_conf_resume_ind(pAdapter);

		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}
	hdd_ipa_resume(pHddCtx);

	return;
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
	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HDD SSR timer expired!", __func__);
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
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: Trying to start SSR timer when " "it's running!",
		       __func__);
	}
	ssr_timer.expires = jiffies + msecs_to_jiffies(msec);
	ssr_timer.function = hdd_ssr_timer_cb;
	add_timer(&ssr_timer);
	ssr_timer_started = true;
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
	QDF_STATUS qdf_status;
	v_CONTEXT_t p_cds_context = NULL;
	hdd_context_t *pHddCtx;
	p_cds_sched_context cds_sched_context = NULL;

	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: WLAN driver shutting down!",
	       __func__);

#ifdef WLAN_FEATURE_LPSS
	wlan_hdd_send_status_pkg(NULL, NULL, 0, 0);
#endif

	/* If SSR never completes, then do kernel panic. */
	hdd_ssr_timer_init();
	hdd_ssr_timer_start(HDD_SSR_BRING_UP_TIME);

	/* Get the global CDS context. */
	p_cds_context = cds_get_global_context();
	if (!p_cds_context) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Global CDS context is Null",
		       __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Get the HDD context. */
	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HDD context is Null",
		       __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cds_set_recovery_in_progress(true);

	cds_clear_concurrent_session_count();
	hdd_cleanup_scan_queue(pHddCtx);
	hdd_reset_all_adapters(pHddCtx);

	hdd_ipa_uc_ssr_deinit();

	cds_sched_context = get_cds_sched_ctxt();

	/* Wakeup all driver threads */
	if (true == pHddCtx->isMcThreadSuspended) {
		complete(&cds_sched_context->ResumeMcEvent);
		pHddCtx->isMcThreadSuspended = false;
	}
#ifdef QCA_CONFIG_SMP
	if (true == pHddCtx->is_ol_rx_thread_suspended) {
		complete(&cds_sched_context->ol_resume_rx_event);
		pHddCtx->is_ol_rx_thread_suspended = false;
	}
#endif

	/* Stop all the threads; we do not want any messages to be a processed,
	 * any more and the best way to ensure that is to terminate the threads
	 * gracefully.
	 */
	/* Wait for MC to exit */
	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Shutting down MC thread", __func__);
	set_bit(MC_SHUTDOWN_EVENT_MASK, &cds_sched_context->mcEventFlag);
	set_bit(MC_POST_EVENT_MASK, &cds_sched_context->mcEventFlag);
	wake_up_interruptible(&cds_sched_context->mcWaitQueue);
	wait_for_completion(&cds_sched_context->McShutdown);

#ifdef QCA_CONFIG_SMP
	/* Wait for OL RX to exit */
	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Shutting down OL RX thread",
	       __func__);
	unregister_hotcpu_notifier(cds_sched_context->cpu_hot_plug_notifier);
	set_bit(RX_SHUTDOWN_EVENT_MASK, &cds_sched_context->ol_rx_event_flag);
	set_bit(RX_POST_EVENT_MASK, &cds_sched_context->ol_rx_event_flag);
	wake_up_interruptible(&cds_sched_context->ol_rx_wait_queue);
	wait_for_completion(&cds_sched_context->ol_rx_shutdown);
	cds_sched_context->ol_rx_thread = NULL;
	cds_drop_rxpkt_by_staid(cds_sched_context, WLAN_MAX_STA_COUNT);
	cds_free_ol_rx_pkt_freeq(cds_sched_context);
#endif

	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Doing WMA STOP", __func__);
	qdf_status = wma_stop(p_cds_context, HAL_STOP_TYPE_RF_KILL);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to stop WMA", __func__);
		QDF_ASSERT(QDF_IS_STATUS_SUCCESS(qdf_status));
		wma_setneedshutdown(p_cds_context);
	}

	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Doing SME STOP", __func__);
	/* Stop SME - Cannot invoke cds_disable as cds_disable relies
	 * on threads being running to process the SYS Stop
	 */
	qdf_status = sme_stop(pHddCtx->hHal, HAL_STOP_TYPE_SYS_RESET);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to stop sme %d", __func__, qdf_status);
		QDF_ASSERT(0);
	}

	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: Doing MAC STOP", __func__);
	/* Stop MAC (PE and HAL) */
	qdf_status = mac_stop(pHddCtx->hHal, HAL_STOP_TYPE_SYS_RESET);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to stop mac %d", __func__, qdf_status);
		QDF_ASSERT(0);
	}

	hddLog(QDF_TRACE_LEVEL_INFO, "%s: Flush Queues", __func__);
	/* Clean up message queues of TX, RX and MC thread */
	cds_sched_flush_mc_mqs(cds_sched_context);

	/* Deinit all the TX, RX and MC queues */
	cds_sched_deinit_mqs(cds_sched_context);

	hddLog(QDF_TRACE_LEVEL_INFO, "%s: Doing CDS Shutdown", __func__);
	/* shutdown CDS */
	cds_shutdown(p_cds_context);

	/*mac context has already been released in mac_close call
	   so setting it to NULL in hdd context */
	pHddCtx->hHal = (tHalHandle) NULL;

	hddLog(QDF_TRACE_LEVEL_FATAL, "%s: WLAN driver shutdown complete",
	       __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_wlan_re_init() - HDD SSR re-init function
 *
 * This function is called by the HIF to re-initialize the driver after SSR.
 *
 * Return: QDF_STATUS_SUCCESS if the driver was re-initialized,
 *	or an error status otherwise
 */
QDF_STATUS hdd_wlan_re_init(void *hif_sc)
{
	QDF_STATUS qdf_status;
	v_CONTEXT_t p_cds_context = NULL;
	hdd_context_t *pHddCtx = NULL;
	QDF_STATUS qdf_ret_status;
	hdd_adapter_t *pAdapter;
	int i;

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);

	/* Get the CDS context */
	p_cds_context = cds_get_global_context();
	if (p_cds_context == NULL) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: Failed cds_get_global_context", __func__);
		goto err_re_init;
	}

	/* Get the HDD context */
	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HDD context is Null",
		       __func__);
		goto err_re_init;
	}

	if (!hif_sc) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: hif_sc is NULL", __func__);
		goto err_re_init;
	}

	((cds_context_type *) p_cds_context)->pHIFContext = hif_sc;

	/* The driver should always be initialized in STA mode after SSR */
	hdd_set_conparam(0);

	/* Re-open CDS, it is a re-open b'se control transport was never closed. */
	qdf_status = cds_open();
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: cds_open failed", __func__);
		goto err_re_init;
	}

	/* Save the hal context in Adapter */
	pHddCtx->hHal = cds_get_context(QDF_MODULE_ID_SME);
	if (NULL == pHddCtx->hHal) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: HAL context is null",
		       __func__);
		goto err_cds_close;
	}

	qdf_status = cds_pre_enable(pHddCtx->pcds_context);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_alert("cds_pre_enable failed");
		goto err_cds_close;
	}

	/*
	 * Note that the cds_pre_enable() sequence triggers the cfg download.
	 * The cfg download must occur before we update the SME config
	 * since the SME config operation must access the cfg database.
	 * Set the SME configuration parameters.
	 */
	qdf_status = hdd_set_sme_config(pHddCtx);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		hdd_alert("Failed hdd_set_sme_config");
		goto err_cds_close;
	}

	ol_txrx_register_pause_cb(wlan_hdd_txrx_pause_cb);

	qdf_status = hdd_set_sme_chan_list(pHddCtx);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: Failed to init channel list", __func__);
		goto err_cds_close;
	}

	/* Apply the cfg.ini to cfg.dat */
	if (false == hdd_update_config_dat(pHddCtx)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: config update failed",
		       __func__);
		goto err_cds_close;
	}

	/* Set the MAC Address, currently this is used by HAL to add self sta.
	 * Remove this once self sta is added as part of session open. */
	qdf_ret_status = cfg_set_str(pHddCtx->hHal, WNI_CFG_STA_ID,
				     (uint8_t *) &pHddCtx->config->
				     intfMacAddr[0],
				     sizeof(pHddCtx->config->intfMacAddr[0]));
	if (!QDF_IS_STATUS_SUCCESS(qdf_ret_status)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: Failed to set MAC Address. "
		       "HALStatus is %08d [x%08x]", __func__, qdf_ret_status,
		       qdf_ret_status);
		goto err_cds_close;
	}

	/* Start CDS which starts up the SME/MAC/HAL modules and everything else
	   Note: Firmware image will be read and downloaded inside cds_enable API */
	qdf_status = cds_enable(p_cds_context);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL, "%s: cds_enable failed", __func__);
		goto err_cds_close;
	}

	qdf_status = hdd_post_cds_enable_config(pHddCtx);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: hdd_post_cds_enable_config failed", __func__);
		goto err_cds_disable;
	}

	/* Try to get an adapter from mode ID */
	pAdapter = hdd_get_adapter(pHddCtx, QDF_STA_MODE);
	if (!pAdapter) {
		pAdapter = hdd_get_adapter(pHddCtx, QDF_SAP_MODE);
		if (!pAdapter) {
			pAdapter = hdd_get_adapter(pHddCtx, QDF_IBSS_MODE);
			if (!pAdapter) {
				hddLog(QDF_TRACE_LEVEL_FATAL,
				       "%s: Failed to get Adapter!", __func__);
			}
		}
	}

	if (hdd_ipa_uc_ssr_reinit())
		hddLog(LOGE, "%s: HDD IPA UC reinit failed", __func__);

	/* Get WLAN Host/FW/HW version */
	if (pAdapter)
		hdd_wlan_get_version(pAdapter, NULL, NULL);

	/* Restart all adapters */
	hdd_start_all_adapters(pHddCtx);

	/* Reconfigure FW logs after SSR */
	if (pAdapter) {
		if (pHddCtx->fw_log_settings.enable != 0) {
			wma_cli_set_command(pAdapter->sessionId,
					    WMI_DBGLOG_MODULE_ENABLE,
					    pHddCtx->fw_log_settings.enable,
					    DBG_CMD);
		} else {
			wma_cli_set_command(pAdapter->sessionId,
					    WMI_DBGLOG_MODULE_DISABLE,
					    pHddCtx->fw_log_settings.enable,
					    DBG_CMD);
		}

		if (pHddCtx->fw_log_settings.dl_report != 0) {
			wma_cli_set_command(pAdapter->sessionId,
					    WMI_DBGLOG_REPORT_ENABLE,
					    pHddCtx->fw_log_settings.
					    dl_report, DBG_CMD);

			wma_cli_set_command(pAdapter->sessionId,
					    WMI_DBGLOG_TYPE,
					    pHddCtx->fw_log_settings.
					    dl_type, DBG_CMD);

			wma_cli_set_command(pAdapter->sessionId,
					    WMI_DBGLOG_LOG_LEVEL,
					    pHddCtx->fw_log_settings.
					    dl_loglevel, DBG_CMD);

			for (i = 0; i < MAX_MOD_LOGLEVEL; i++) {
				if (pHddCtx->fw_log_settings.
						dl_mod_loglevel[i] != 0) {
					wma_cli_set_command(
						pAdapter->sessionId,
						WMI_DBGLOG_MOD_LOG_LEVEL,
						pHddCtx->fw_log_settings.
							dl_mod_loglevel[i],
						DBG_CMD);
				}
			}
		}
	}

	pHddCtx->hdd_mcastbcast_filter_set = false;
	pHddCtx->btCoexModeSet = false;
	hdd_ssr_timer_del();

	wlan_hdd_send_svc_nlink_msg(WLAN_SVC_FW_CRASHED_IND, NULL, 0);

	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);

	sme_ext_scan_register_callback(pHddCtx->hHal,
				wlan_hdd_cfg80211_extscan_callback);

	qdf_status = hdd_register_for_sap_restart_with_channel_switch();
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		/* Error already logged */
		goto err_cds_disable;

	sme_set_rssi_threshold_breached_cb(pHddCtx->hHal, hdd_rssi_threshold_breached);

#ifdef WLAN_FEATURE_LPSS
	wlan_hdd_send_all_scan_intf_info(pHddCtx);
	wlan_hdd_send_version_pkg(pHddCtx->target_fw_version,
				  pHddCtx->target_hw_version,
				  pHddCtx->target_hw_name);
#endif

	hddLog(LOGE,
		"%s: WLAN host driver reinitiation completed!", __func__);
	goto success;

err_cds_disable:
	cds_disable(p_cds_context);

err_cds_close:
	cds_close(p_cds_context);
	cds_sched_close(p_cds_context);
	if (pHddCtx) {
		/* Unregister the Net Device Notifier */
		unregister_netdevice_notifier(&hdd_netdev_notifier);
		cnss_diag_notify_wlan_close();
		ptt_sock_deactivate_svc();

		nl_srv_exit();

		/* Free up dynamically allocated members inside HDD Adapter */
		kfree(pHddCtx->config);
		pHddCtx->config = NULL;
		wlan_hdd_deinit_tx_rx_histogram(pHddCtx);
		wiphy_unregister(pHddCtx->wiphy);
		wiphy_free(pHddCtx->wiphy);

		if (!QDF_IS_STATUS_SUCCESS(cds_deinit_policy_mgr())) {
			hdd_err("Failed to deinit policy manager");
			/* Proceed and complete the clean up */
		}
	}

err_re_init:
	/* Allow the phone to go to sleep */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT);
	QDF_BUG(0);
	return -EPERM;

success:
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_set_powersave() - Set powersave mode
 * @adapter: adapter upon which the request was received
 * @mode: desired powersave mode
 *
 * Return: 0 on success, non-zero on any error
 */
static int
wlan_hdd_set_powersave(hdd_adapter_t *adapter, enum hdd_power_mode mode)
{
	tHalHandle hal;
	hdd_context_t *hdd_ctx;

	if (NULL == adapter) {
		hddLog(QDF_TRACE_LEVEL_FATAL, FL("Adapter NULL"));
		return -ENODEV;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx) {
		hdd_err(FL("hdd context is NULL"));
		return -EINVAL;
	}

	hddLog(QDF_TRACE_LEVEL_INFO_HIGH, FL("power mode=%d"), mode);
	hal = WLAN_HDD_GET_HAL_CTX(adapter);


	if (DRIVER_POWER_MODE_ACTIVE == mode) {
		hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
		       FL("Wlan driver Entering Full Power"));

		/*
		 * Enter Full power command received from GUI
		 * this means we are disconnected
		 */
		sme_ps_enable_disable(hal, adapter->sessionId, SME_PS_DISABLE);
	} else if (DRIVER_POWER_MODE_AUTO == mode) {
		if ((QDF_STA_MODE == adapter->device_mode) ||
				(QDF_P2P_CLIENT_MODE == adapter->device_mode)) {
			hddLog(LOG1, FL("Disabling Auto Power save timer"));
			sme_ps_disable_auto_ps_timer(WLAN_HDD_GET_HAL_CTX
					(adapter),
					adapter->sessionId);
		}
		if (hdd_ctx->config && hdd_ctx->config->is_ps_enabled) {
			hddLog(LOG1, FL("Wlan driver Entering Power save"));

			/*
			 * Enter Power Save command received from GUI
			 * this means DHCP is completed
			 */
			sme_ps_enable_disable(hal, adapter->sessionId,
					SME_PS_ENABLE);
		} else {
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
				FL("Power Save is not enabled in the cfg"));
		}
	}
	return 0;
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
	int result;
	p_cds_sched_context cds_sched_context = get_cds_sched_ctxt();

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	result = wlan_hdd_validate_context(pHddCtx);
	if (0 != result)
		return result;
	pld_request_bus_bandwidth(pHddCtx->parent_dev, PLD_BUS_WIDTH_MEDIUM);

	/* Resume MC thread */
	if (pHddCtx->isMcThreadSuspended) {
		complete(&cds_sched_context->ResumeMcEvent);
		pHddCtx->isMcThreadSuspended = false;
	}
#ifdef QCA_CONFIG_SMP
	/* Resume tlshim Rx thread */
	if (pHddCtx->is_ol_rx_thread_suspended) {
		complete(&cds_sched_context->ol_resume_rx_event);
		pHddCtx->is_ol_rx_thread_suspended = false;
	}
#endif
	hdd_resume_wlan();

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_RESUME_WLAN,
			 NO_SESSION, pHddCtx->isWiphySuspended));
	qdf_spin_lock(&pHddCtx->sched_scan_lock);
	pHddCtx->isWiphySuspended = false;
	if (true != pHddCtx->isSchedScanUpdatePending) {
		qdf_spin_unlock(&pHddCtx->sched_scan_lock);
		hddLog(LOG1, FL("Return resume is not due to PNO indication"));
		return 0;
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
				hddLog(LOGW, FL("NO SCAN result"));
			} else {
				/* Acquire wakelock to handle the case where
				 * APP's tries to suspend immediately after
				 * updating the scan results. Whis results in
				 * app's is in suspended state and not able to
				 * process the connect request to AP
				 */
				hdd_prevent_suspend_timeout(2000,
					WIFI_POWER_EVENT_WAKELOCK_RESUME_WLAN);
				cfg80211_sched_scan_results(pHddCtx->wiphy);
			}

			hddLog(LOG1,
			       FL("cfg80211 scan result database updated"));
			EXIT();
			return result;
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	hddLog(LOG1, FL("Failed to find Adapter"));
	EXIT();
	return result;
}

/**
 * wlan_hdd_cfg80211_ready_to_suspend() - set cfg80211 ready to suspend event
 * @callbackContext: Pointer to callback context
 * @suspended: Suspend flag
 *
 * Return: none
 */
static void wlan_hdd_cfg80211_ready_to_suspend(void *callbackContext,
						bool suspended)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) callbackContext;
	pHddCtx->suspended = suspended;
	complete(&pHddCtx->ready_to_suspend);
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
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	rc = wlan_hdd_validate_context(pHddCtx);
	if (0 != rc)
		return rc;

	/* If RADAR detection is in progress (HDD), prevent suspend. The flag
	 * "dfs_cac_block_tx" is set to true when RADAR is found and stay true
	 * until CAC is done for a SoftAP which is in started state.
	 */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		if (QDF_SAP_MODE == pAdapter->device_mode) {
			if (BSS_START ==
			    WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter)->bssState &&
			    true ==
			    WLAN_HDD_GET_AP_CTX_PTR(pAdapter)->
			    dfs_cac_block_tx) {
				hddLog(LOG1,
					FL("RADAR detection in progress, do not allow suspend"));
				return -EAGAIN;
			} else if (!pHddCtx->config->enableSapSuspend) {
				/* return -EOPNOTSUPP if SAP does not support
				 * suspend
				 */
				hddLog(LOGE,
					FL("SAP does not support suspend!!"));
				return -EOPNOTSUPP;
			}
		} else if (QDF_P2P_GO_MODE == pAdapter->device_mode) {
			if (!pHddCtx->config->enableSapSuspend) {
				/* return -EOPNOTSUPP if GO does not support
				 * suspend
				 */
				hddLog(LOGE,
					FL("GO does not support suspend!!"));
				return -EOPNOTSUPP;
			}
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/* Stop ongoing scan on each interface */
	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter = pAdapterNode->pAdapter;
		pScanInfo = &pAdapter->scan_info;

		if (sme_sta_in_middle_of_roaming
			    (pHddCtx->hHal, pAdapter->sessionId)) {
			hddLog(LOG1,
				FL("Roaming in progress, do not allow suspend"));
			return -EAGAIN;
		}

		if (pScanInfo->mScanPending) {
			INIT_COMPLETION(pScanInfo->abortscan_event_var);
			hdd_abort_mac_scan(pHddCtx, pAdapter->sessionId,
					   eCSR_SCAN_ABORT_DEFAULT);

			status =
				wait_for_completion_timeout(&pScanInfo->
				    abortscan_event_var,
				    msecs_to_jiffies(WLAN_WAIT_TIME_ABORTSCAN));
			if (!status) {
				hddLog(LOGE,
					FL("Timeout occurred while waiting for abort scan"));
				return -ETIME;
			}
		}
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/*
	 * Suspend IPA early before proceeding to suspend other entities like
	 * firmware to avoid any race conditions.
	 */
	if (hdd_ipa_suspend(pHddCtx)) {
		hddLog(LOG1, FL("IPA not ready to suspend!"));
		return -EAGAIN;
	}

	/* Wait for the target to be ready for suspend */
	INIT_COMPLETION(pHddCtx->ready_to_suspend);

	hdd_suspend_wlan(&wlan_hdd_cfg80211_ready_to_suspend, pHddCtx);

	rc = wait_for_completion_timeout(&pHddCtx->ready_to_suspend,
		msecs_to_jiffies(WLAN_WAIT_TIME_READY_TO_SUSPEND));
	if (!rc) {
		hddLog(LOGE, FL("Failed to get ready to suspend"));
		goto resume_tx;
	}

	if (!pHddCtx->suspended) {
		hddLog(LOGE, FL("Faied as suspend_status is wrong:%d"),
			pHddCtx->suspended);
		goto resume_tx;
	}

	/* Suspend MC thread */
	set_bit(MC_SUSPEND_EVENT_MASK, &cds_sched_context->mcEventFlag);
	wake_up_interruptible(&cds_sched_context->mcWaitQueue);

	/* Wait for suspend confirmation from MC thread */
	rc = wait_for_completion_timeout(&pHddCtx->mc_sus_event_var,
		msecs_to_jiffies(WLAN_WAIT_TIME_MCTHREAD_SUSPEND));
	if (!rc) {
		clear_bit(MC_SUSPEND_EVENT_MASK,
			  &cds_sched_context->mcEventFlag);
		hddLog(LOGE, FL("Failed to stop mc thread"));
		goto resume_tx;
	}

	pHddCtx->isMcThreadSuspended = true;

#ifdef QCA_CONFIG_SMP
	/* Suspend tlshim rx thread */
	set_bit(RX_SUSPEND_EVENT_MASK, &cds_sched_context->ol_rx_event_flag);
	wake_up_interruptible(&cds_sched_context->ol_rx_wait_queue);
	rc = wait_for_completion_timeout(&cds_sched_context->
					 ol_suspend_rx_event,
					 msecs_to_jiffies
						 (RX_TLSHIM_SUSPEND_TIMEOUT));
	if (!rc) {
		clear_bit(RX_SUSPEND_EVENT_MASK,
			  &cds_sched_context->ol_rx_event_flag);
		hddLog(LOGE, FL("Failed to stop tl_shim rx thread"));
		goto resume_all;
	}
	pHddCtx->is_ol_rx_thread_suspended = true;
#endif
	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SUSPEND_WLAN,
			 NO_SESSION, pHddCtx->isWiphySuspended));
	pHddCtx->isWiphySuspended = true;

	pld_request_bus_bandwidth(pHddCtx->parent_dev, PLD_BUS_WIDTH_NONE);

	EXIT();
	return 0;

#ifdef QCA_CONFIG_SMP
resume_all:

	complete(&cds_sched_context->ResumeMcEvent);
	pHddCtx->isMcThreadSuspended = false;
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
 * __wlan_hdd_cfg80211_set_power_mgmt() - set cfg80211 power management config
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @mode: Driver mode
 * @timeout: Timeout value
 *
 * Return: 0 for success, non-zero for failure
 */
static int __wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
					      struct net_device *dev, bool mode,
					      int timeout)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	QDF_STATUS qdf_status;
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SET_POWER_MGMT,
			 pAdapter->sessionId, timeout));

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	if ((DRIVER_POWER_MODE_AUTO == !mode) &&
	    (true == pHddCtx->hdd_wlan_suspended) &&
	    (pHddCtx->config->fhostArpOffload) &&
	    (eConnectionState_Associated ==
	     (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState)) {

		hddLog(LOG1,
			FL("offload: in cfg80211_set_power_mgmt, calling arp offload"));
		qdf_status = hdd_conf_arp_offload(pAdapter, true);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hddLog(LOG1,
				FL("Failed to enable ARPOFFLOAD Feature %d"),
				qdf_status);
		}
	}

	status = wlan_hdd_set_powersave(pAdapter, !mode);

	if (!mode) {
		hddLog(LOGE, FL("DHCP start indicated through power save"));
		sme_dhcp_start_ind(pHddCtx->hHal, pAdapter->device_mode,
				   pAdapter->macAddressCurrent.bytes,
				   pAdapter->sessionId);
	} else {
		hddLog(LOGW, FL("DHCP stop indicated through power save"));
		sme_dhcp_stop_ind(pHddCtx->hHal, pAdapter->device_mode,
				  pAdapter->macAddressCurrent.bytes,
				  pAdapter->sessionId);
	}

	EXIT();
	return status;
}

/**
 * wlan_hdd_cfg80211_set_power_mgmt() - set cfg80211 power management config
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @mode: Driver mode
 * @timeout: Timeout value
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
				     struct net_device *dev, bool mode,
				     int timeout)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_set_power_mgmt(wiphy, dev, mode, timeout);
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
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
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
		hddLog(LOGE, FL("sme_cfg_set_int failed for tx power %hu"),
				dbm);
		return -EIO;
	}

	hddLog(LOG2, FL("Set tx power level %d dbm"), dbm);

	switch (type) {
	/* Automatically determine transmit power */
	case NL80211_TX_POWER_AUTOMATIC:
	/* Fall through */
	case NL80211_TX_POWER_LIMITED:  /* Limit TX power by the mBm parameter */
		if (sme_set_max_tx_power(hHal, bssid, selfMac, dbm) !=
		    QDF_STATUS_SUCCESS) {
			hddLog(LOGE, FL("Setting maximum tx power failed"));
			return -EIO;
		}
		break;

	case NL80211_TX_POWER_FIXED:    /* Fix TX power to the mBm parameter */
		hddLog(LOGE, FL("NL80211_TX_POWER_FIXED not supported"));
		return -EOPNOTSUPP;
		break;

	default:
		hddLog(LOGE, FL("Invalid power setting type %d"), type);
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

	hdd_adapter_t *pAdapter;
	hdd_context_t *pHddCtx = (hdd_context_t *) wiphy_priv(wiphy);
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status) {
		*dbm = 0;
		return status;
	}

	pAdapter = hdd_get_adapter(pHddCtx, QDF_STA_MODE);
	if (NULL == pAdapter) {
		hddLog(LOGE, FL("pAdapter is NULL"));
		return -ENOENT;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_GET_TXPOWER,
			 pAdapter->sessionId, pAdapter->device_mode));
	wlan_hdd_get_class_astats(pAdapter);
	*dbm = pAdapter->hdd_stats.ClassA_stat.max_pwr;

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
	ret = __wlan_hdd_cfg80211_get_txpower(wiphy,
						wdev,
						dbm);
	cds_ssr_unprotect(__func__);

	return ret;
}
