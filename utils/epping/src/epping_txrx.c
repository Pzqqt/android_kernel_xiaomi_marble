/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

/*========================================================================

   \file  epping_txrx.c

   \brief WLAN End Point Ping test tool implementation

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cds_api.h>
#include <cds_sched.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <wni_api.h>
#include <wlan_ptt_sock_svc.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <pld_common.h>
#include <linux/rtnetlink.h>
#include <linux/semaphore.h>
#include <linux/ctype.h>
#include "epping_main.h"
#include "epping_internal.h"

static int epping_start_adapter(epping_adapter_t *adapter);
static void epping_stop_adapter(epping_adapter_t *adapter);

static void epping_timer_expire(void *data)
{
	struct net_device *dev = (struct net_device *)data;
	epping_adapter_t *adapter;

	if (dev == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: netdev = NULL", __func__);
		return;
	}

	adapter = netdev_priv(dev);
	if (adapter == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: adapter = NULL", __func__);
		return;
	}
	adapter->epping_timer_state = EPPING_TX_TIMER_STOPPED;
	epping_tx_timer_expire(adapter);
}

static int epping_ndev_open(struct net_device *dev)
{
	epping_adapter_t *adapter;
	int ret = 0;

	adapter = netdev_priv(dev);
	epping_start_adapter(adapter);
	return ret;
}

static int epping_ndev_stop(struct net_device *dev)
{
	epping_adapter_t *adapter;
	int ret = 0;

	adapter = netdev_priv(dev);
	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: EPPING adapter context is Null", __func__);
		ret = -ENODEV;
		goto end;
	}
	epping_stop_adapter(adapter);
end:
	return ret;
}

static void epping_ndev_uninit(struct net_device *dev)
{
	epping_adapter_t *adapter;

	adapter = netdev_priv(dev);
	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: EPPING adapter context is Null", __func__);
		goto end;
	}
	epping_stop_adapter(adapter);
end:
	return;
}

static void epping_tx_queue_timeout(struct net_device *dev)
{
	epping_adapter_t *adapter;

	adapter = netdev_priv(dev);
	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: EPPING adapter context is Null", __func__);
		goto end;
	}

	EPPING_LOG(QDF_TRACE_LEVEL_ERROR,
		   "%s: Transmission timeout occurred, adapter->started= %d",
		   __func__, adapter->started);

	/* Getting here implies we disabled the TX queues
	 * for too long. Since this is epping
	 * (not because of disassociation or low resource scenarios),
	 * try to restart the queue
	 */
	if (adapter->started)
		netif_wake_queue(dev);
end:
	return;

}

static netdev_tx_t epping_hard_start_xmit(struct sk_buff *skb,
					  struct net_device *dev)
{
	epping_adapter_t *adapter;
	int ret = 0;

	adapter = netdev_priv(dev);
	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: EPPING adapter context is Null", __func__);
		kfree_skb(skb);
		ret = -ENODEV;
		goto end;
	}
	ret = epping_tx_send(skb, adapter);
end:
	return NETDEV_TX_OK;
}

static struct net_device_stats *epping_get_stats(struct net_device *dev)
{
	epping_adapter_t *adapter = netdev_priv(dev);

	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_ERROR, "%s: adapter = NULL",
			   __func__);
		return NULL;
	}

	return &adapter->stats;
}

static int epping_ndev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	epping_adapter_t *adapter;
	int ret = 0;

	adapter = netdev_priv(dev);
	if (NULL == adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: EPPING adapter context is Null", __func__);
		ret = -ENODEV;
		goto end;
	}
	if (dev != adapter->dev) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: HDD adapter/dev inconsistency", __func__);
		ret = -ENODEV;
		goto end;
	}

	if ((!ifr) || (!ifr->ifr_data)) {
		ret = -EINVAL;
		goto end;
	}

	switch (cmd) {
	case (SIOCDEVPRIVATE + 1):
		EPPING_LOG(QDF_TRACE_LEVEL_ERROR,
			   "%s: do not support ioctl %d (SIOCDEVPRIVATE + 1)",
			   __func__, cmd);
		break;
	default:
		EPPING_LOG(QDF_TRACE_LEVEL_ERROR, "%s: unknown ioctl %d",
			   __func__, cmd);
		ret = -EINVAL;
		break;
	}

end:
	return ret;
}

static int epping_set_mac_address(struct net_device *dev, void *addr)
{
	epping_adapter_t *adapter = netdev_priv(dev);
	struct sockaddr *psta_mac_addr = addr;
	qdf_mem_copy(&adapter->macAddressCurrent,
		     psta_mac_addr->sa_data, ETH_ALEN);
	qdf_mem_copy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);
	return 0;
}

static void epping_stop_adapter(epping_adapter_t *adapter)
{
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: qdf_ctx is NULL\n", __func__);
		return;
	}

	if (adapter && adapter->started) {
		EPPING_LOG(LOG1, FL("Disabling queues"));
		netif_tx_disable(adapter->dev);
		netif_carrier_off(adapter->dev);
		adapter->started = false;
		pld_request_bus_bandwidth(qdf_ctx->dev,
					  PLD_BUS_WIDTH_LOW);
	}
}

static int epping_start_adapter(epping_adapter_t *adapter)
{
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: qdf_ctx is NULL", __func__);
		return -EINVAL;
	}

	if (!adapter) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: adapter= NULL\n", __func__);
		return -EINVAL;
	}
	if (!adapter->started) {
		pld_request_bus_bandwidth(qdf_ctx->dev,
					  PLD_BUS_WIDTH_HIGH);
		netif_carrier_on(adapter->dev);
		EPPING_LOG(LOG1, FL("Enabling queues"));
		netif_tx_start_all_queues(adapter->dev);
		adapter->started = true;
	} else {
		EPPING_LOG(QDF_TRACE_LEVEL_WARN,
			   "%s: adapter %pK already started\n", __func__,
			   adapter);
	}
	return 0;
}

static int epping_register_adapter(epping_adapter_t *adapter)
{
	int ret = 0;

	ret = register_netdev(adapter->dev);
	if (ret != 0) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: unable to register device\n",
			   adapter->dev->name);
	} else {
		adapter->registered = true;
	}
	return ret;
}

static void epping_unregister_adapter(epping_adapter_t *adapter)
{
	if (adapter) {
		epping_stop_adapter(adapter);
		if (adapter->registered) {
			unregister_netdev(adapter->dev);
			adapter->registered = false;
		}
	} else {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: adapter = NULL, unable to unregister device\n",
			   __func__);
	}
}

void epping_destroy_adapter(epping_adapter_t *adapter)
{
	struct net_device *dev = NULL;
	epping_context_t *pEpping_ctx;

	if (!adapter || !adapter->pEpping_ctx) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: adapter = NULL\n", __func__);
		return;
	}

	dev = adapter->dev;
	pEpping_ctx = adapter->pEpping_ctx;
	epping_unregister_adapter(adapter);

	qdf_spinlock_destroy(&adapter->data_lock);
	qdf_timer_free(&adapter->epping_timer);
	adapter->epping_timer_state = EPPING_TX_TIMER_STOPPED;

	while (qdf_nbuf_queue_len(&adapter->nodrop_queue)) {
		qdf_nbuf_t tmp_nbuf = NULL;
		tmp_nbuf = qdf_nbuf_queue_remove(&adapter->nodrop_queue);
		if (tmp_nbuf)
			qdf_nbuf_free(tmp_nbuf);
	}

	free_netdev(dev);
	if (!pEpping_ctx)
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: pEpping_ctx = NULL\n", __func__);
	else
		pEpping_ctx->epping_adapter = NULL;
}

static struct net_device_ops epping_drv_ops = {
	.ndo_open = epping_ndev_open,
	.ndo_stop = epping_ndev_stop,
	.ndo_uninit = epping_ndev_uninit,
	.ndo_start_xmit = epping_hard_start_xmit,
	.ndo_tx_timeout = epping_tx_queue_timeout,
	.ndo_get_stats = epping_get_stats,
	.ndo_do_ioctl = epping_ndev_ioctl,
	.ndo_set_mac_address = epping_set_mac_address,
	.ndo_select_queue = NULL,
};

#define EPPING_TX_QUEUE_MAX_LEN 128     /* need to be power of 2 */

epping_adapter_t *epping_add_adapter(epping_context_t *pEpping_ctx,
				     tSirMacAddr macAddr,
				     enum QDF_OPMODE device_mode)
{
	struct net_device *dev;
	epping_adapter_t *adapter;

	dev = alloc_netdev(sizeof(epping_adapter_t), "wifi%d",
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
			   NET_NAME_UNKNOWN,
#endif
			   ether_setup);
	if (dev == NULL) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: Cannot allocate epping_adapter_t\n", __func__);
		return NULL;
	}

	adapter = netdev_priv(dev);
	qdf_mem_zero(adapter, sizeof(*adapter));
	adapter->dev = dev;
	adapter->pEpping_ctx = pEpping_ctx;
	adapter->device_mode = device_mode;    /* station, SAP, etc */
	qdf_mem_copy(dev->dev_addr, (void *)macAddr, sizeof(tSirMacAddr));
	qdf_mem_copy(adapter->macAddressCurrent.bytes,
		     macAddr, sizeof(tSirMacAddr));
	qdf_spinlock_create(&adapter->data_lock);
	qdf_nbuf_queue_init(&adapter->nodrop_queue);
	adapter->epping_timer_state = EPPING_TX_TIMER_STOPPED;
	qdf_timer_init(epping_get_qdf_ctx(), &adapter->epping_timer,
		epping_timer_expire, dev, QDF_TIMER_TYPE_SW);
	dev->type = ARPHRD_IEEE80211;
	dev->netdev_ops = &epping_drv_ops;
	dev->watchdog_timeo = 5 * HZ;   /* XXX */
	dev->tx_queue_len = EPPING_TXBUF - 1;      /* 1 for mgmt frame */
	if (epping_register_adapter(adapter) == 0) {
		EPPING_LOG(LOG1, FL("Disabling queues"));
		netif_tx_disable(dev);
		netif_carrier_off(dev);
		return adapter;
	} else {
		epping_destroy_adapter(adapter);
		return NULL;
	}
}

int epping_connect_service(epping_context_t *pEpping_ctx)
{
	int status, i;
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp response;

	qdf_mem_zero(&connect, sizeof(connect));
	qdf_mem_zero(&response, sizeof(response));

	/* these fields are the same for all service endpoints */
	connect.EpCallbacks.pContext = pEpping_ctx;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = epping_rx;
	/* epping_tx_complete use Multiple version */
	connect.EpCallbacks.EpTxComplete  = epping_tx_complete;
	connect.MaxSendQueueDepth = 64;

#ifdef HIF_SDIO
	connect.EpCallbacks.EpRecvRefill = epping_refill;
	connect.EpCallbacks.EpSendFull =
		epping_tx_queue_full /* ar6000_tx_queue_full */;
#elif defined(HIF_USB) || defined(HIF_PCI)
	connect.EpCallbacks.EpRecvRefill = NULL /* provided by HIF */;
	connect.EpCallbacks.EpSendFull = NULL /* provided by HIF */;
	/* disable flow control for hw flow control */
	connect.ConnectionFlags |= HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;
#endif

	/* connect to service */
	connect.service_id = WMI_DATA_BE_SVC;
	status = htc_connect_service(pEpping_ctx->HTCHandle, &connect, &response);
	if (status != EOK) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "Failed to connect to Endpoint Ping BE service status:%d\n",
			   status);
		return status;
	} else {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "eppingtest BE endpoint:%d\n", response.Endpoint);
	}
	pEpping_ctx->EppingEndpoint[0] = response.Endpoint;

#if defined(HIF_PCI) || defined(HIF_USB)
	connect.service_id = WMI_DATA_BK_SVC;
	status = htc_connect_service(pEpping_ctx->HTCHandle, &connect, &response);
	if (status != EOK) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "Failed to connect to Endpoint Ping BK service status:%d\n",
			   status);
		return status;
	} else {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "eppingtest BK endpoint:%d\n", response.Endpoint);
	}
	pEpping_ctx->EppingEndpoint[1] = response.Endpoint;
	/* Since we do not create other two SVC use BK endpoint
	 * for rest ACs (2, 3) */
	for (i = 2; i < EPPING_MAX_NUM_EPIDS; i++) {
		pEpping_ctx->EppingEndpoint[i] = response.Endpoint;
	}
#else
	/* we only use one endpoint for high latenance bus.
	 * Map all AC's EPIDs to the same endpoint ID returned by HTC */
	for (i = 0; i < EPPING_MAX_NUM_EPIDS; i++) {
		pEpping_ctx->EppingEndpoint[i] = response.Endpoint;
	}
#endif
	return 0;
}
