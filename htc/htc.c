/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

#include "htc_debug.h"
#include "htc_internal.h"
#include "htc_credit_history.h"
#include <hif.h>
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_types.h>          /* qdf_print */

#define MAX_HTC_RX_BUNDLE  2

#if defined(WLAN_DEBUG) || defined(DEBUG)
static ATH_DEBUG_MASK_DESCRIPTION g_htc_debug_description[] = {
	{ATH_DEBUG_SEND, "Send"},
	{ATH_DEBUG_RECV, "Recv"},
	{ATH_DEBUG_SYNC, "Sync"},
	{ATH_DEBUG_DUMP, "Dump Data (RX or TX)"},
	{ATH_DEBUG_SETUP, "Setup"},
};

ATH_DEBUG_INSTANTIATE_MODULE_VAR(htc,
				 "htc",
				 "Host Target Communications",
				 ATH_DEBUG_MASK_DEFAULTS | ATH_DEBUG_INFO |
				 ATH_DEBUG_SETUP,
				 ATH_DEBUG_DESCRIPTION_COUNT
					 (g_htc_debug_description),
				 g_htc_debug_description);

#endif

#if (defined(WMI_MULTI_MAC_SVC) || defined(QCA_WIFI_QCA8074) || \
	defined(QCA_WIFI_QCA6018))
static const uint32_t svc_id[] = {WMI_CONTROL_SVC, WMI_CONTROL_SVC_WMAC1,
						WMI_CONTROL_SVC_WMAC2};
#else
static const uint32_t svc_id[] = {WMI_CONTROL_SVC};
#endif

extern unsigned int htc_credit_flow;

static void reset_endpoint_states(HTC_TARGET *target);

static void destroy_htc_tx_ctrl_packet(HTC_PACKET *pPacket)
{
	qdf_nbuf_t netbuf;

	netbuf = (qdf_nbuf_t) GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
	if (netbuf)
		qdf_nbuf_free(netbuf);
	qdf_mem_free(pPacket);
}

static HTC_PACKET *build_htc_tx_ctrl_packet(qdf_device_t osdev)
{
	HTC_PACKET *pPacket = NULL;
	qdf_nbuf_t netbuf;

	do {
		pPacket = (HTC_PACKET *) qdf_mem_malloc(sizeof(HTC_PACKET));
		if (!pPacket)
			break;
		netbuf = qdf_nbuf_alloc(osdev, HTC_CONTROL_BUFFER_SIZE,
					20, 4, true);
		if (!netbuf) {
			qdf_mem_free(pPacket);
			pPacket = NULL;
			break;
		}
		SET_HTC_PACKET_NET_BUF_CONTEXT(pPacket, netbuf);
	} while (false);

	return pPacket;
}

void htc_free_control_tx_packet(HTC_TARGET *target, HTC_PACKET *pPacket)
{

#ifdef TODO_FIXME
	LOCK_HTC(target);
	HTC_PACKET_ENQUEUE(&target->ControlBufferTXFreeList, pPacket);
	UNLOCK_HTC(target);
	/* TODO_FIXME netbufs cannot be RESET! */
#else
	destroy_htc_tx_ctrl_packet(pPacket);
#endif

}

HTC_PACKET *htc_alloc_control_tx_packet(HTC_TARGET *target)
{
#ifdef TODO_FIXME
	HTC_PACKET *pPacket;

	LOCK_HTC(target);
	pPacket = htc_packet_dequeue(&target->ControlBufferTXFreeList);
	UNLOCK_HTC(target);

	return pPacket;
#else
	return build_htc_tx_ctrl_packet(target->osdev);
#endif
}

/* Set the target failure handling callback */
void htc_set_target_failure_callback(HTC_HANDLE HTCHandle,
				     HTC_TARGET_FAILURE Callback)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	target->HTCInitInfo.TargetFailure = Callback;
}

void htc_dump(HTC_HANDLE HTCHandle, uint8_t CmdId, bool start)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	hif_dump(target->hif_dev, CmdId, start);
}

/* cleanup the HTC instance */
static void htc_cleanup(HTC_TARGET *target)
{
	HTC_PACKET *pPacket;
	int i;
	HTC_ENDPOINT *endpoint;
	HTC_PACKET_QUEUE *pkt_queue;
	qdf_nbuf_t netbuf;

	if (target->hif_dev) {
		hif_detach_htc(target->hif_dev);
		hif_mask_interrupt_call(target->hif_dev);
		target->hif_dev = NULL;
	}

	while (true) {
		pPacket = allocate_htc_packet_container(target);
		if (!pPacket)
			break;
		qdf_mem_free(pPacket);
	}

	LOCK_HTC_TX(target);
	pPacket = target->pBundleFreeList;
	target->pBundleFreeList = NULL;
	UNLOCK_HTC_TX(target);
	while (pPacket) {
		HTC_PACKET *pPacketTmp = (HTC_PACKET *) pPacket->ListLink.pNext;
		netbuf = GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
		if (netbuf)
			qdf_nbuf_free(netbuf);
		pkt_queue = pPacket->pContext;
		if (pkt_queue)
			qdf_mem_free(pkt_queue);
		qdf_mem_free(pPacket);
		pPacket = pPacketTmp;
	}

#ifdef TODO_FIXME
	while (true) {
		pPacket = htc_alloc_control_tx_packet(target);
		if (!pPacket)
			break;
		netbuf = (qdf_nbuf_t) GET_HTC_PACKET_NET_BUF_CONTEXT(pPacket);
		if (netbuf)
			qdf_nbuf_free(netbuf);
		qdf_mem_free(pPacket);
	}
#endif

	htc_flush_endpoint_txlookupQ(target, ENDPOINT_0, true);

	qdf_spinlock_destroy(&target->HTCLock);
	qdf_spinlock_destroy(&target->HTCRxLock);
	qdf_spinlock_destroy(&target->HTCTxLock);
	for (i = 0; i < ENDPOINT_MAX; i++) {
		endpoint = &target->endpoint[i];
		qdf_spinlock_destroy(&endpoint->lookup_queue_lock);
	}

	/* free our instance */
	qdf_mem_free(target);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * htc_runtime_pm_init(): runtime pm related intialization
 *
 * need to initialize a work item.
 */
static void htc_runtime_pm_init(HTC_TARGET *target)
{
	qdf_create_work(0, &target->queue_kicker, htc_kick_queues, target);
}

/**
 * htc_runtime_suspend() - runtime suspend HTC
 *
 * @htc_ctx: HTC context pointer
 *
 * This is a dummy function for symmetry.
 *
 * Return: 0 for success
 */
int htc_runtime_suspend(HTC_HANDLE htc_ctx)
{
	return 0;
}

/**
 * htc_runtime_resume(): resume htc
 *
 * The htc message queue needs to be kicked off after
 * a runtime resume.  Otherwise messages would get stuck.
 *
 * @htc_ctx: HTC context pointer
 *
 * Return: 0 for success;
 */
int htc_runtime_resume(HTC_HANDLE htc_ctx)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_ctx);

	if (!target)
		return 0;

	qdf_sched_work(0, &target->queue_kicker);
	return 0;
}
#else
static inline void htc_runtime_pm_init(HTC_TARGET *target) { }
#endif

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)
static
void htc_update_rx_bundle_stats(void *ctx, uint8_t no_of_pkt_in_bundle)
{
	HTC_TARGET *target = (HTC_TARGET *)ctx;

	no_of_pkt_in_bundle--;
	if (target && (no_of_pkt_in_bundle < HTC_MAX_MSG_PER_BUNDLE_RX))
		target->rx_bundle_stats[no_of_pkt_in_bundle]++;
}
#else
static
void htc_update_rx_bundle_stats(void *ctx, uint8_t no_of_pkt_in_bundle)
{
}
#endif

/* registered target arrival callback from the HIF layer */
HTC_HANDLE htc_create(void *ol_sc, struct htc_init_info *pInfo,
			qdf_device_t osdev, uint32_t con_mode)
{
	struct hif_msg_callbacks htcCallbacks;
	HTC_ENDPOINT *pEndpoint = NULL;
	HTC_TARGET *target = NULL;
	int i;

	if (!ol_sc) {
		HTC_ERROR("%s: ol_sc = NULL", __func__);
		return NULL;
	}
	HTC_TRACE("+htc_create ..  HIF :%pK", ol_sc);

	A_REGISTER_MODULE_DEBUG_INFO(htc);

	target = (HTC_TARGET *) qdf_mem_malloc(sizeof(HTC_TARGET));
	if (!target)
		return NULL;

	htc_runtime_pm_init(target);
	htc_credit_history_init();
	qdf_spinlock_create(&target->HTCLock);
	qdf_spinlock_create(&target->HTCRxLock);
	qdf_spinlock_create(&target->HTCTxLock);
	for (i = 0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		qdf_spinlock_create(&pEndpoint->lookup_queue_lock);
	}
	target->is_nodrop_pkt = false;
	target->htc_hdr_length_check = false;
	target->wmi_ep_count = 1;

	do {
		qdf_mem_copy(&target->HTCInitInfo, pInfo,
			     sizeof(struct htc_init_info));
		target->host_handle = pInfo->pContext;
		target->osdev = osdev;
		target->con_mode = con_mode;

		reset_endpoint_states(target);

		INIT_HTC_PACKET_QUEUE(&target->ControlBufferTXFreeList);

		for (i = 0; i < HTC_PACKET_CONTAINER_ALLOCATION; i++) {
			HTC_PACKET *pPacket = (HTC_PACKET *)
					qdf_mem_malloc(sizeof(HTC_PACKET));
			if (pPacket)
				free_htc_packet_container(target, pPacket);
		}

#ifdef TODO_FIXME
		for (i = 0; i < NUM_CONTROL_TX_BUFFERS; i++) {
			pPacket = build_htc_tx_ctrl_packet();
			if (!pPacket)
				break;
			htc_free_control_tx_packet(target, pPacket);
		}
#endif

		/* setup HIF layer callbacks */
		qdf_mem_zero(&htcCallbacks, sizeof(struct hif_msg_callbacks));
		htcCallbacks.Context = target;
		htcCallbacks.rxCompletionHandler = htc_rx_completion_handler;
		htcCallbacks.txCompletionHandler = htc_tx_completion_handler;
		htcCallbacks.txResourceAvailHandler =
						 htc_tx_resource_avail_handler;
		htcCallbacks.fwEventHandler = htc_fw_event_handler;
		htcCallbacks.update_bundle_stats = htc_update_rx_bundle_stats;
		target->hif_dev = ol_sc;

		/* Get HIF default pipe for HTC message exchange */
		pEndpoint = &target->endpoint[ENDPOINT_0];

		hif_post_init(target->hif_dev, target, &htcCallbacks);
		hif_get_default_pipe(target->hif_dev, &pEndpoint->UL_PipeID,
				     &pEndpoint->DL_PipeID);
		hif_set_initial_wakeup_cb(target->hif_dev,
					  pInfo->target_initial_wakeup_cb,
					  pInfo->target_psoc);

	} while (false);

	htc_recv_init(target);

	HTC_TRACE("-htc_create: (0x%pK)", target);

	return (HTC_HANDLE) target;
}

void htc_destroy(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
			("+htc_destroy ..  Destroying :0x%pK\n", target));
	hif_stop(htc_get_hif_device(HTCHandle));
	if (target)
		htc_cleanup(target);
	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-htc_destroy\n"));
}

/* get the low level HIF device for the caller , the caller may wish to do low
 * level HIF requests
 */
void *htc_get_hif_device(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	return target->hif_dev;
}

static void htc_control_tx_complete(void *Context, HTC_PACKET *pPacket)
{
	HTC_TARGET *target = (HTC_TARGET *) Context;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
			("+-htc_control_tx_complete 0x%pK (l:%d)\n", pPacket,
			 pPacket->ActualLength));
	htc_free_control_tx_packet(target, pPacket);
}

/* TODO, this is just a temporary max packet size */
#define MAX_MESSAGE_SIZE 1536

/**
 * htc_setup_epping_credit_allocation() - allocate credits/HTC buffers to WMI
 * @scn: pointer to hif_opaque_softc
 * @pEntry: pointer to tx credit allocation entry
 * @credits: number of credits
 *
 * Return: None
 */
static void
htc_setup_epping_credit_allocation(struct hif_opaque_softc *scn,
			   struct htc_service_tx_credit_allocation *pEntry,
			   int credits)
{
	switch (hif_get_bus_type(scn)) {
	case QDF_BUS_TYPE_PCI:
	case QDF_BUS_TYPE_USB:
		pEntry++;
		pEntry->service_id = WMI_DATA_BE_SVC;
		pEntry->CreditAllocation = (credits >> 1);

		pEntry++;
		pEntry->service_id = WMI_DATA_BK_SVC;
		pEntry->CreditAllocation = (credits >> 1);
		break;
	case QDF_BUS_TYPE_SDIO:
		pEntry++;
		pEntry->service_id = WMI_DATA_BE_SVC;
		pEntry->CreditAllocation = credits;
		break;
	default:
		break;
	}
}

/**
 * htc_setup_target_buffer_assignments() - setup target buffer assignments
 * @target: HTC Target Pointer
 *
 * Return: A_STATUS
 */
static
A_STATUS htc_setup_target_buffer_assignments(HTC_TARGET *target)
{
	struct htc_service_tx_credit_allocation *pEntry;
	A_STATUS status;
	int credits;
	int creditsPerMaxMsg;

	creditsPerMaxMsg = MAX_MESSAGE_SIZE / target->TargetCreditSize;
	if (MAX_MESSAGE_SIZE % target->TargetCreditSize)
		creditsPerMaxMsg++;

	/* TODO, this should be configured by the caller! */

	credits = target->TotalTransmitCredits;
	pEntry = &target->ServiceTxAllocTable[0];

	status = A_OK;
	/*
	 * Allocate all credists/HTC buffers to WMI.
	 * no buffers are used/required for data. data always
	 * remains on host.
	 */
	if (HTC_IS_EPPING_ENABLED(target->con_mode)) {
		pEntry++;
		pEntry->service_id = WMI_CONTROL_SVC;
		pEntry->CreditAllocation = credits;
		/* endpoint ping is a testing tool directly on top of HTC in
		 * both target and host sides.
		 * In target side, the endppint ping fw has no wlan stack and
		 * FW mboxping app directly sits on HTC and it simply drops
		 * or loops back TX packets. For rx perf, FW mboxping app
		 * generates packets and passes packets to HTC to send to host.
		 * There is no WMI message exchanges between host and target
		 * in endpoint ping case.
		 * In host side, the endpoint ping driver is a Ethernet driver
		 * and it directly sits on HTC. Only HIF, HTC, QDF, ADF are
		 * used by the endpoint ping driver. There is no wifi stack
		 * at all in host side also. For tx perf use case,
		 * the user space mboxping app sends the raw packets to endpoint
		 * ping driver and it directly forwards to HTC for transmission
		 * to stress the bus. For the rx perf, HTC passes the received
		 * packets to endpoint ping driver and it is passed to the user
		 * space through the Ethernet interface.
		 * For credit allocation, in SDIO bus case, only BE service is
		 * used for tx/rx perf testing so that all credits are given
		 * to BE service. In PCIe and USB bus case, endpoint ping uses
		 * both BE and BK services to stress the bus so that the total
		 * credits are equally distributed to BE and BK services.
		 */

		htc_setup_epping_credit_allocation(target->hif_dev,
						   pEntry, credits);
	} else {
		int i;
		uint32_t max_wmi_svc = (sizeof(svc_id) / sizeof(uint32_t));

		if ((target->wmi_ep_count == 0) ||
				(target->wmi_ep_count > max_wmi_svc))
			return A_ERROR;

		/*
		 * Divide credit among number of endpoints for WMI
		 */
		credits = credits / target->wmi_ep_count;
		for (i = 0; i < target->wmi_ep_count; i++) {
			status = A_OK;
			pEntry++;
			pEntry->service_id = svc_id[i];
			pEntry->CreditAllocation = credits;
		}
	}

	if (A_SUCCESS(status)) {
		int i;

		for (i = 0; i < HTC_MAX_SERVICE_ALLOC_ENTRIES; i++) {
			if (target->ServiceTxAllocTable[i].service_id != 0) {
				AR_DEBUG_PRINTF(ATH_DEBUG_INIT,
						("SVS Index : %d TX : 0x%2.2X : alloc:%d",
						 i,
						 target->ServiceTxAllocTable[i].
						 service_id,
						 target->ServiceTxAllocTable[i].
						 CreditAllocation));
			}
		}
	}

	return status;
}

uint8_t htc_get_credit_allocation(HTC_TARGET *target, uint16_t service_id)
{
	uint8_t allocation = 0;
	int i;

	for (i = 0; i < HTC_MAX_SERVICE_ALLOC_ENTRIES; i++) {
		if (target->ServiceTxAllocTable[i].service_id == service_id) {
			allocation =
				target->ServiceTxAllocTable[i].CreditAllocation;
		}
	}

	if (0 == allocation) {
		AR_DEBUG_PRINTF(ATH_DEBUG_RSVD1,
			("HTC Service TX : 0x%2.2X : allocation is zero!\n",
				 service_id));
	}

	return allocation;
}

QDF_STATUS htc_wait_target(HTC_HANDLE HTCHandle)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_READY_EX_MSG *pReadyMsg;
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp resp;
	HTC_READY_MSG *rdy_msg;
	uint16_t htc_rdy_msg_id;
	uint8_t i = 0;
	HTC_PACKET *rx_bundle_packet, *temp_bundle_packet;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
			("htc_wait_target - Enter (target:0x%pK)\n", HTCHandle));
	AR_DEBUG_PRINTF(ATH_DEBUG_RSVD1, ("+HWT\n"));

	do {

		status = hif_start(target->hif_dev);
		if (QDF_IS_STATUS_ERROR(status)) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
					("hif_start failed\n"));
			break;
		}

		status = htc_wait_recv_ctrl_message(target);

		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (target->CtrlResponseLength < (sizeof(HTC_READY_EX_MSG))) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("Invalid HTC Ready Msg Len:%d!\n",
					 target->CtrlResponseLength));
			status = QDF_STATUS_E_BADMSG;
			break;
		}

		pReadyMsg = (HTC_READY_EX_MSG *) target->CtrlResponseBuffer;

		rdy_msg = &pReadyMsg->Version2_0_Info;
		htc_rdy_msg_id =
			HTC_GET_FIELD(rdy_msg, HTC_READY_MSG, MESSAGEID);
		if (htc_rdy_msg_id != HTC_MSG_READY_ID) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("Invalid HTC Ready Msg : 0x%X!\n",
					 htc_rdy_msg_id));
			status = QDF_STATUS_E_BADMSG;
			break;
		}

		target->TotalTransmitCredits = HTC_GET_FIELD(rdy_msg,
						HTC_READY_MSG, CREDITCOUNT);
		if (target->HTCInitInfo.cfg_wmi_credit_cnt &&
			(target->HTCInitInfo.cfg_wmi_credit_cnt <
						target->TotalTransmitCredits))
			/*
			 * If INI configured value is less than FW advertised,
			 * then use INI configured value, otherwise use FW
			 * advertised.
			 */
			target->TotalTransmitCredits =
				target->HTCInitInfo.cfg_wmi_credit_cnt;

		target->TargetCreditSize =
			(int)HTC_GET_FIELD(rdy_msg, HTC_READY_MSG, CREDITSIZE);
		target->MaxMsgsPerHTCBundle =
			(uint8_t) pReadyMsg->MaxMsgsPerHTCBundle;
		UPDATE_ALT_CREDIT(target, pReadyMsg->AltDataCreditSize);
		/* for old fw this value is set to 0. But the minimum value
		 * should be 1, i.e., no bundling
		 */
		if (target->MaxMsgsPerHTCBundle < 1)
			target->MaxMsgsPerHTCBundle = 1;

		AR_DEBUG_PRINTF(ATH_DEBUG_INIT,
				("Target Ready! TX resource : %d size:%d, MaxMsgsPerHTCBundle = %d",
				 target->TotalTransmitCredits,
				 target->TargetCreditSize,
				 target->MaxMsgsPerHTCBundle));

		if ((0 == target->TotalTransmitCredits)
		    || (0 == target->TargetCreditSize)) {
			status = QDF_STATUS_E_ABORTED;
			break;
		}

		/* Allocate expected number of RX bundle buffer allocation */
		if (HTC_RX_BUNDLE_ENABLED(target)) {
			temp_bundle_packet = NULL;
			for (i = 0; i < MAX_HTC_RX_BUNDLE; i++) {
				rx_bundle_packet =
					allocate_htc_bundle_packet(target);
				if (rx_bundle_packet)
					rx_bundle_packet->ListLink.pNext =
						(DL_LIST *)temp_bundle_packet;
				else
					break;

				temp_bundle_packet = rx_bundle_packet;
			}
			LOCK_HTC_TX(target);
			target->pBundleFreeList = temp_bundle_packet;
			UNLOCK_HTC_TX(target);
		}

		/* done processing */
		target->CtrlResponseProcessing = false;

		htc_setup_target_buffer_assignments(target);

		/* setup our pseudo HTC control endpoint connection */
		qdf_mem_zero(&connect, sizeof(connect));
		qdf_mem_zero(&resp, sizeof(resp));
		connect.EpCallbacks.pContext = target;
		connect.EpCallbacks.EpTxComplete = htc_control_tx_complete;
		connect.EpCallbacks.EpRecv = htc_control_rx_complete;
		connect.MaxSendQueueDepth = NUM_CONTROL_TX_BUFFERS;
		connect.service_id = HTC_CTRL_RSVD_SVC;

		/* connect fake service */
		status = htc_connect_service((HTC_HANDLE) target,
					     &connect, &resp);

	} while (false);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("htc_wait_target - Exit (%d)\n",
			status));
	AR_DEBUG_PRINTF(ATH_DEBUG_RSVD1, ("-HWT\n"));
	return status;
}

/* start HTC, this is called after all services are connected */
static A_STATUS htc_config_target_hif_pipe(HTC_TARGET *target)
{

	return A_OK;
}

static void reset_endpoint_states(HTC_TARGET *target)
{
	HTC_ENDPOINT *pEndpoint;
	int i;

	for (i = ENDPOINT_0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		pEndpoint->service_id = 0;
		pEndpoint->MaxMsgLength = 0;
		pEndpoint->MaxTxQueueDepth = 0;
		pEndpoint->Id = i;
		INIT_HTC_PACKET_QUEUE(&pEndpoint->TxQueue);
		INIT_HTC_PACKET_QUEUE(&pEndpoint->TxLookupQueue);
		INIT_HTC_PACKET_QUEUE(&pEndpoint->RxBufferHoldQueue);
		pEndpoint->target = target;
		pEndpoint->TxCreditFlowEnabled = (bool)htc_credit_flow;
		qdf_atomic_init(&pEndpoint->TxProcessCount);
	}
}

/**
 * htc_start() - Main HTC function to trigger HTC start
 * @HTCHandle: pointer to HTC handle
 *
 * Return: QDF_STATUS_SUCCESS for success or an appropriate QDF_STATUS error
 */
QDF_STATUS htc_start(HTC_HANDLE HTCHandle)
{
	qdf_nbuf_t netbuf;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_SETUP_COMPLETE_EX_MSG *pSetupComp;
	HTC_PACKET *pSendPacket;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("htc_start Enter\n"));

	do {

		htc_config_target_hif_pipe(target);

		/* allocate a buffer to send */
		pSendPacket = htc_alloc_control_tx_packet(target);
		if (!pSendPacket) {
			AR_DEBUG_ASSERT(false);
			qdf_print("%s: allocControlTxPacket failed",
				  __func__);
			status = QDF_STATUS_E_NOMEM;
			break;
		}

		netbuf =
		   (qdf_nbuf_t) GET_HTC_PACKET_NET_BUF_CONTEXT(pSendPacket);
		/* assemble setup complete message */
		qdf_nbuf_put_tail(netbuf, sizeof(HTC_SETUP_COMPLETE_EX_MSG));
		pSetupComp =
			(HTC_SETUP_COMPLETE_EX_MSG *) qdf_nbuf_data(netbuf);
		qdf_mem_zero(pSetupComp, sizeof(HTC_SETUP_COMPLETE_EX_MSG));

		HTC_SET_FIELD(pSetupComp, HTC_SETUP_COMPLETE_EX_MSG,
			      MESSAGEID, HTC_MSG_SETUP_COMPLETE_EX_ID);

		if (!htc_credit_flow) {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
					("HTC will not use TX credit flow control"));
			pSetupComp->SetupFlags |=
				HTC_SETUP_COMPLETE_FLAGS_DISABLE_TX_CREDIT_FLOW;
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
					("HTC using TX credit flow control"));
		}

		if ((hif_get_bus_type(target->hif_dev) == QDF_BUS_TYPE_SDIO) ||
					(hif_get_bus_type(target->hif_dev) ==
							 QDF_BUS_TYPE_USB)) {
			if (HTC_RX_BUNDLE_ENABLED(target))
			pSetupComp->SetupFlags |=
				HTC_SETUP_COMPLETE_FLAGS_ENABLE_BUNDLE_RECV;
			hif_set_bundle_mode(target->hif_dev, true,
				HTC_MAX_MSG_PER_BUNDLE_RX);
			pSetupComp->MaxMsgsPerBundledRecv = HTC_MAX_MSG_PER_BUNDLE_RX;
		}

		SET_HTC_PACKET_INFO_TX(pSendPacket,
				       NULL,
				       (uint8_t *) pSetupComp,
				       sizeof(HTC_SETUP_COMPLETE_EX_MSG),
				       ENDPOINT_0, HTC_SERVICE_TX_PACKET_TAG);

		status = htc_send_pkt((HTC_HANDLE) target, pSendPacket);
		if (QDF_IS_STATUS_ERROR(status))
			break;
	} while (false);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("htc_start Exit\n"));
	return status;
}

/*flush all queued buffers for surpriseremove case*/
void htc_flush_surprise_remove(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	int i;
	HTC_ENDPOINT *pEndpoint;
#ifdef RX_SG_SUPPORT
	qdf_nbuf_t netbuf;
	qdf_nbuf_queue_t *rx_sg_queue = &target->RxSgQueue;
#endif

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("+htc_flush_surprise_remove\n"));

	/* cleanup endpoints */
	for (i = 0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		htc_flush_rx_hold_queue(target, pEndpoint);
		htc_flush_endpoint_tx(target, pEndpoint, HTC_TX_PACKET_TAG_ALL);
	}

	hif_flush_surprise_remove(target->hif_dev);

#ifdef RX_SG_SUPPORT
	LOCK_HTC_RX(target);
	while ((netbuf = qdf_nbuf_queue_remove(rx_sg_queue)) != NULL)
		qdf_nbuf_free(netbuf);
	RESET_RX_SG_CONFIG(target);
	UNLOCK_HTC_RX(target);
#endif

	reset_endpoint_states(target);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-htc_flush_surprise_remove\n"));
}

/* stop HTC communications, i.e. stop interrupt reception, and flush all queued
 * buffers
 */
void htc_stop(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	int i;
	HTC_ENDPOINT *endpoint;
#ifdef RX_SG_SUPPORT
	qdf_nbuf_t netbuf;
	qdf_nbuf_queue_t *rx_sg_queue = &target->RxSgQueue;
#endif

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("+htc_stop\n"));

	HTC_INFO("%s: endpoints cleanup\n", __func__);
	/* cleanup endpoints */
	for (i = 0; i < ENDPOINT_MAX; i++) {
		endpoint = &target->endpoint[i];
		htc_flush_rx_hold_queue(target, endpoint);
		htc_flush_endpoint_tx(target, endpoint, HTC_TX_PACKET_TAG_ALL);
		if (endpoint->ul_is_polled) {
			qdf_timer_stop(&endpoint->ul_poll_timer);
			qdf_timer_free(&endpoint->ul_poll_timer);
		}
	}

	/* Note: htc_flush_endpoint_tx for all endpoints should be called before
	 * hif_stop - otherwise htc_tx_completion_handler called from
	 * hif_send_buffer_cleanup_on_pipe for residual tx frames in HIF layer,
	 * might queue the packet again to HIF Layer - which could cause tx
	 * buffer leak
	 */

	HTC_INFO("%s: stopping hif layer\n", __func__);
	hif_stop(target->hif_dev);

#ifdef RX_SG_SUPPORT
	LOCK_HTC_RX(target);
	while ((netbuf = qdf_nbuf_queue_remove(rx_sg_queue)) != NULL)
		qdf_nbuf_free(netbuf);
	RESET_RX_SG_CONFIG(target);
	UNLOCK_HTC_RX(target);
#endif

	/**
	 * In SSR case, HTC tx completion callback for wmi will be blocked
	 * by TARGET_STATUS_RESET and HTC packets will be left unfreed on
	 * lookup queue.
	 *
	 * In case of target failing to send wmi_ready_event, the htc connect
	 * msg buffer will be left unmapped and not freed. So calling the
	 * completion handler for this buffer will handle this scenario.
	 */
	HTC_INFO("%s: flush endpoints Tx lookup queue\n", __func__);
	for (i = 0; i < ENDPOINT_MAX; i++) {
		endpoint = &target->endpoint[i];
		if (endpoint->service_id == WMI_CONTROL_SVC)
			htc_flush_endpoint_txlookupQ(target, i, false);
		else if (endpoint->service_id == HTC_CTRL_RSVD_SVC)
			htc_flush_endpoint_txlookupQ(target, i, true);
	}
	HTC_INFO("%s: resetting endpoints state\n", __func__);

	reset_endpoint_states(target);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-htc_stop\n"));
}

void htc_dump_credit_states(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	int i;

	for (i = 0; i < ENDPOINT_MAX; i++) {
		pEndpoint = &target->endpoint[i];
		if (0 == pEndpoint->service_id)
			continue;

		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
			("--- EP : %d  service_id: 0x%X    --------------\n",
				 pEndpoint->Id, pEndpoint->service_id));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				(" TxCredits          : %d\n",
				 pEndpoint->TxCredits));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				(" TxCreditSize       : %d\n",
				 pEndpoint->TxCreditSize));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				(" TxCreditsPerMaxMsg : %d\n",
				 pEndpoint->TxCreditsPerMaxMsg));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				(" TxQueueDepth       : %d\n",
				 HTC_PACKET_QUEUE_DEPTH(&pEndpoint->TxQueue)));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("----------------------------------------\n"));
	}
}

bool htc_get_endpoint_statistics(HTC_HANDLE HTCHandle,
				   HTC_ENDPOINT_ID Endpoint,
				   enum htc_endpoint_stat_action Action,
				   struct htc_endpoint_stats *pStats)
{
#ifdef HTC_EP_STAT_PROFILING
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	bool clearStats = false;
	bool sample = false;

	switch (Action) {
	case HTC_EP_STAT_SAMPLE:
		sample = true;
		break;
	case HTC_EP_STAT_SAMPLE_AND_CLEAR:
		sample = true;
		clearStats = true;
		break;
	case HTC_EP_STAT_CLEAR:
		clearStats = true;
		break;
	default:
		break;
	}

	A_ASSERT(Endpoint < ENDPOINT_MAX);

	/* lock out TX and RX while we sample and/or clear */
	LOCK_HTC_TX(target);
	LOCK_HTC_RX(target);

	if (sample) {
		A_ASSERT(pStats);
		/* return the stats to the caller */
		qdf_mem_copy(pStats, &target->endpoint[Endpoint].endpoint_stats,
			 sizeof(struct htc_endpoint_stats));
	}

	if (clearStats) {
		/* reset stats */
		qdf_mem_zero(&target->endpoint[Endpoint].endpoint_stats,
			  sizeof(struct htc_endpoint_stats));
	}

	UNLOCK_HTC_RX(target);
	UNLOCK_HTC_TX(target);

	return true;
#else
	return false;
#endif
}

void *htc_get_targetdef(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	return hif_get_targetdef(target->hif_dev);
}

#ifdef IPA_OFFLOAD
/**
 * htc_ipa_get_ce_resource() - get uc resource on lower layer
 * @htc_handle: htc context
 * @ce_sr_base_paddr: copyengine source ring base physical address
 * @ce_sr_ring_size: copyengine source ring size
 * @ce_reg_paddr: copyengine register physical address
 *
 * Return: None
 */
void htc_ipa_get_ce_resource(HTC_HANDLE htc_handle,
			     qdf_shared_mem_t **ce_sr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	if (target->hif_dev)
		hif_ipa_get_ce_resource(target->hif_dev,
					ce_sr, ce_sr_ring_size, ce_reg_paddr);
}
#endif /* IPA_OFFLOAD */

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)

void htc_dump_bundle_stats(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	int total, i;

	total = 0;
	for (i = 0; i < HTC_MAX_MSG_PER_BUNDLE_RX; i++)
		total += target->rx_bundle_stats[i];

	if (total) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("RX Bundle stats:\n"));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("Total RX packets: %d\n",
						total));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, (
				"Number of bundle: Number of packets\n"));
		for (i = 0; i < HTC_MAX_MSG_PER_BUNDLE_RX; i++)
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("%10d:%10d(%2d%s)\n", (i+1),
					 target->rx_bundle_stats[i],
					 ((target->rx_bundle_stats[i]*100)/
					  total), "%"));
	}


	total = 0;
	for (i = 0; i < HTC_MAX_MSG_PER_BUNDLE_TX; i++)
		total += target->tx_bundle_stats[i];

	if (total) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("TX Bundle stats:\n"));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("Total TX packets: %d\n",
						total));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("Number of bundle: Number of packets\n"));
		for (i = 0; i < HTC_MAX_MSG_PER_BUNDLE_TX; i++)
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("%10d:%10d(%2d%s)\n", (i+1),
					 target->tx_bundle_stats[i],
					 ((target->tx_bundle_stats[i]*100)/
					  total), "%"));
	}
}

void htc_clear_bundle_stats(HTC_HANDLE HTCHandle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	qdf_mem_zero(&target->rx_bundle_stats, sizeof(target->rx_bundle_stats));
	qdf_mem_zero(&target->tx_bundle_stats, sizeof(target->tx_bundle_stats));
}
#endif

/**
 * htc_vote_link_down - API to vote for link down
 * @htc_handle: HTC handle
 *
 * API for upper layers to call HIF to vote for link down
 *
 * Return: void
 */
void htc_vote_link_down(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	if (!target->hif_dev)
		return;

	hif_vote_link_down(target->hif_dev);
}

/**
 * htc_vote_link_up - API to vote for link up
 * @htc_handle: HTC Handle
 *
 * API for upper layers to call HIF to vote for link up
 *
 * Return: void
 */
void htc_vote_link_up(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	if (!target->hif_dev)
		return;

	hif_vote_link_up(target->hif_dev);
}

/**
 * htc_can_suspend_link - API to query HIF for link status
 * @htc_handle: HTC Handle
 *
 * API for upper layers to call HIF to query if the link can suspend
 *
 * Return: void
 */
bool htc_can_suspend_link(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	if (!target->hif_dev)
		return false;

	return hif_can_suspend_link(target->hif_dev);
}

#ifdef FEATURE_RUNTIME_PM
int htc_pm_runtime_get(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	return hif_pm_runtime_get(target->hif_dev);
}

int htc_pm_runtime_put(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	return hif_pm_runtime_put(target->hif_dev);
}
#endif

/**
 * htc_set_wmi_endpoint_count: Set number of WMI endpoint
 * @htc_handle: HTC handle
 * @wmi_ep_count: WMI enpoint count
 *
 * return: None
 */
void htc_set_wmi_endpoint_count(HTC_HANDLE htc_handle, uint8_t wmi_ep_count)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	target->wmi_ep_count = wmi_ep_count;
}

/**
 * htc_get_wmi_endpoint_count: Get number of WMI endpoint
 * @htc_handle: HTC handle
 *
 * return: WMI enpoint count
 */
uint8_t htc_get_wmi_endpoint_count(HTC_HANDLE htc_handle)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(htc_handle);

	return target->wmi_ep_count;
}
