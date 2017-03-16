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
 * DOC: wlan_hdd_scan.c
 *
 * WLAN Host Device Driver scan implementation
 */

#include <linux/wireless.h>
#include <net/cfg80211.h>

#include "wlan_hdd_includes.h"
#include "cds_api.h"
#include "cds_api.h"
#include "ani_global.h"
#include "dot11f.h"
#include "cds_sched.h"
#include "wlan_hdd_p2p.h"
#include "wlan_hdd_trace.h"
#include "wlan_hdd_scan.h"
#include "wlan_policy_mgr_api.h"
#include "wma_api.h"
#include "cds_utils.h"

#ifdef WLAN_UMAC_CONVERGENCE
#include "wlan_cfg80211.h"
#endif
#include <qca_vendor.h>

#ifdef NAPIER_SCAN
#include <wlan_cfg80211_scan.h>
#endif

#define MAX_RATES                       12
#define HDD_WAKE_LOCK_SCAN_DURATION (5 * 1000) /* in msec */

#define SCAN_DONE_EVENT_BUF_SIZE 4096
#define RATE_MASK 0x7f

/**
 * typedef tSSIDBcastType - SSID broadcast type
 * @eBCAST_UNKNOWN: Broadcast unknown
 * @eBCAST_NORMAL: Broadcast normal
 * @eBCAST_HIDDEN: Broadcast hidden
 */
typedef enum eSSIDBcastType {
	eBCAST_UNKNOWN = 0,
	eBCAST_NORMAL = 1,
	eBCAST_HIDDEN = 2,
} tSSIDBcastType;

#ifndef NAPIER_SCAN
/**
 * typedef hdd_scan_info_t - HDD scan info
 * @dev: Pointer to net device
 * @info: Pointer to request info
 * @start: Start pointer
 * @end: End pointer
 */
typedef struct hdd_scan_info {
	struct net_device *dev;
	struct iw_request_info *info;
	char *start;
	char *end;
} hdd_scan_info_t, *hdd_scan_info_tp;
/**
 * hdd_translate_abg_rate_to_mbps_rate() - translate abg rate to Mbps rate
 * @pFcRate: Rate pointer
 *
 * Return: Mbps rate in integer
 */
static int32_t hdd_translate_abg_rate_to_mbps_rate(uint8_t *pFcRate)
{
	/* Slightly more sophisticated processing has to take place here.
	 * Basic rates are rounded DOWN.  HT rates are rounded UP
	 */
	return ((((int32_t) *pFcRate) & 0x007f) * 1000000) / 2;
}

/**
 * hdd_add_iw_stream_event() - add iw stream event
 * @cmd: Command
 * @length: Length
 * @data: Pointer to data
 * @pscanInfo: Pointer to scan info
 * @last_event: Pointer to pointer to last event
 * @current_event: Pointer to pointer to current event
 *
 * Return: 0 for success, non zero for failure
 */
static int hdd_add_iw_stream_event(int cmd, int length, char *data,
				   hdd_scan_info_t *pscanInfo,
				   char **last_event,
				   char **current_event)
{
	struct iw_event event;

	*last_event = *current_event;
	qdf_mem_zero(&event, sizeof(struct iw_event));
	event.cmd = cmd;
	event.u.data.flags = 1;
	event.u.data.length = length;
	*current_event =
		iwe_stream_add_point(pscanInfo->info, *current_event,
				     pscanInfo->end, &event, data);

	if (*last_event == *current_event) {
		/* no space to add event */
		hdd_err("no space left to add event");
		return -E2BIG;  /* Error code, may be E2BIG */
	}

	return 0;
}

/**
 * hdd_get_wparsn_ies() - get wpa RSN IEs
 * @ieFields: Pointer to the Bss Descriptor IEs
 * @ie_length: IE Length
 * @last_event: Points to last event
 * @current_event: Points to current event
 *
 * This function extract the WPA/RSN IE from the Bss descriptor IEs fields
 *
 * Return: 0 for success, non zero for failure
 */
static int hdd_get_wparsn_ies(uint8_t *ieFields, uint16_t ie_length,
			      char **last_event, char **current_event,
			      hdd_scan_info_t *pscanInfo)
{
	uint8_t eid, elen, *element;
	uint16_t tie_length = 0;

	ENTER();

	element = ieFields;
	tie_length = ie_length;

	while (tie_length > 2 && element != NULL) {
		eid = element[0];
		elen = element[1];

		/* If element length is greater than total remaining ie length,
		 * break the loop
		 */
		if ((elen + 2) > tie_length)
			break;

		switch (eid) {
		case DOT11F_EID_WPA:
		case DOT11F_EID_RSN:
#ifdef FEATURE_WLAN_WAPI
		case DOT11F_EID_WAPI:
#endif
			if (hdd_add_iw_stream_event
				    (IWEVGENIE, elen + 2, (char *)element, pscanInfo,
				    last_event, current_event) < 0)
				return -E2BIG;
			break;

		default:
			break;
		}

		/* Next element */
		tie_length -= (2 + elen);
		element += 2 + elen;
	}

	return 0;
}
/**
 * hdd_indicate_scan_result() - indicate scan results
 * @scanInfo: Pointer to the scan info structure.
 * @descriptor: Pointer to the Bss Descriptor.
 *
 * This function returns the scan results to the wpa_supplicant
 *
 * @Return: 0 for success, non zero for failure
 */
#define MAX_CUSTOM_LEN 64
static int hdd_indicate_scan_result(hdd_scan_info_t *scanInfo,
				    tCsrScanResultInfo *scan_result)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(scanInfo->dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	tSirBssDescription *descriptor = &scan_result->BssDescriptor;
	struct iw_event event;
	char *current_event = scanInfo->start;
	char *end = scanInfo->end;
	char *last_event;
	char *current_pad;
	uint16_t ie_length = 0;
	uint16_t capabilityInfo;
	char *modestr;
	int error;
	char custom[MAX_CUSTOM_LEN];
	char *p;

	hdd_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(descriptor->bssId));

	error = 0;
	last_event = current_event;
	qdf_mem_zero(&event, sizeof(event));

	/* BSSID */
	event.cmd = SIOCGIWAP;
	event.u.ap_addr.sa_family = ARPHRD_ETHER;
	qdf_mem_copy(event.u.ap_addr.sa_data, descriptor->bssId,
		     sizeof(descriptor->bssId));
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_ADDR_LEN);

	if (last_event == current_event) {
		/* no space to add event */
		/* Error code may be E2BIG */
		hdd_err("no space for SIOCGIWAP");
		return -E2BIG;
	}

	last_event = current_event;
	qdf_mem_zero(&event, sizeof(struct iw_event));

	/* Protocol Name */
	event.cmd = SIOCGIWNAME;

	switch (descriptor->nwType) {
	case eSIR_11A_NW_TYPE:
		modestr = "a";
		break;
	case eSIR_11B_NW_TYPE:
		modestr = "b";
		break;
	case eSIR_11G_NW_TYPE:
		modestr = "g";
		break;
	case eSIR_11N_NW_TYPE:
		modestr = "n";
		break;
	default:
		hdd_warn("Unknown network type: %d", descriptor->nwType);
		modestr = "?";
		break;
	}
	snprintf(event.u.name, IFNAMSIZ, "IEEE 802.11%s", modestr);
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_CHAR_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hdd_err("no space for SIOCGIWNAME");
		/* Error code, may be E2BIG */
		return -E2BIG;
	}

	last_event = current_event;
	qdf_mem_zero(&event, sizeof(struct iw_event));

	/*Freq */
	event.cmd = SIOCGIWFREQ;

	event.u.freq.m = descriptor->channelId;
	event.u.freq.e = 0;
	event.u.freq.i = 0;
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_FREQ_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hdd_err("no space for SIOCGIWFREQ");
		return -E2BIG;
	}

	last_event = current_event;
	qdf_mem_zero(&event, sizeof(struct iw_event));

	/* BSS Mode */
	event.cmd = SIOCGIWMODE;

	capabilityInfo = descriptor->capabilityInfo;

	if (SIR_MAC_GET_ESS(capabilityInfo)) {
		event.u.mode = IW_MODE_MASTER;
	} else if (SIR_MAC_GET_IBSS(capabilityInfo)) {
		event.u.mode = IW_MODE_ADHOC;
	} else {
		/* neither ESS or IBSS */
		event.u.mode = IW_MODE_AUTO;
	}

	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_UINT_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hdd_err("no space for SIOCGIWMODE");
		return -E2BIG;
	}
	/* To extract SSID */
	ie_length = GET_IE_LEN_IN_BSS(descriptor->length);

	if (ie_length > 0) {
		/*
		 * dot11BeaconIEs is a large struct, so we make it static to
		 * avoid stack overflow.  This API is only invoked via ioctl,
		 * so it is serialized by the kernel rtnl_lock and hence does
		 * not need to be reentrant
		 */
		static tDot11fBeaconIEs dot11BeaconIEs;
		tDot11fIESSID *pDot11SSID;
		tDot11fIESuppRates *pDot11SuppRates;
		tDot11fIEExtSuppRates *pDot11ExtSuppRates;
		tDot11fIEHTCaps *pDot11IEHTCaps;
		int numBasicRates = 0;
		int maxNumRates = 0;

		pDot11IEHTCaps = NULL;

		dot11f_unpack_beacon_i_es((tpAniSirGlobal)
					  hHal, (uint8_t *) descriptor->ieFields,
					  ie_length, &dot11BeaconIEs, false);

		pDot11SSID = &dot11BeaconIEs.SSID;

		if (pDot11SSID->present) {
			last_event = current_event;
			qdf_mem_zero(&event, sizeof(struct iw_event));

			event.cmd = SIOCGIWESSID;
			event.u.data.flags = 1;
			event.u.data.length = scan_result->ssId.length;
			current_event =
				iwe_stream_add_point(scanInfo->info, current_event,
						     end, &event,
						     (char *)scan_result->ssId.
						     ssId);

			if (last_event == current_event) {      /* no space to add event */
				hdd_err("no space for SIOCGIWESSID");
				return -E2BIG;
			}
		}

		if (hdd_get_wparsn_ies
			    ((uint8_t *) descriptor->ieFields, ie_length, &last_event,
			    &current_event, scanInfo) < 0) {
			hdd_err("no space for SIOCGIWESSID");
			return -E2BIG;
		}

		last_event = current_event;
		current_pad = current_event + IW_EV_LCP_LEN;
		qdf_mem_zero(&event, sizeof(struct iw_event));

		/*Rates */
		event.cmd = SIOCGIWRATE;

		pDot11SuppRates = &dot11BeaconIEs.SuppRates;

		if (pDot11SuppRates->present) {
			int i;

			numBasicRates = pDot11SuppRates->num_rates;
			for (i = 0; i < pDot11SuppRates->num_rates; i++) {
				if (0 != (pDot11SuppRates->rates[i] & 0x7F)) {
					event.u.bitrate.value =
						hdd_translate_abg_rate_to_mbps_rate
							(&pDot11SuppRates->rates[i]);

					current_pad =
						iwe_stream_add_value(scanInfo->info,
								     current_event,
								     current_pad,
								     end, &event,
								     IW_EV_PARAM_LEN);
				}
			}

		}

		pDot11ExtSuppRates = &dot11BeaconIEs.ExtSuppRates;

		if (pDot11ExtSuppRates->present) {
			int i, no_of_rates;
			maxNumRates =
				numBasicRates + pDot11ExtSuppRates->num_rates;

			/* Check to make sure the total number of rates
			 * doesn't exceed IW_MAX_BITRATES
			 */

			maxNumRates = QDF_MIN(maxNumRates, IW_MAX_BITRATES);

			if ((maxNumRates - numBasicRates) > MAX_RATES) {
				no_of_rates = MAX_RATES;
				hdd_debug("Limit rates to MAX_RATES");
			} else {
				no_of_rates = maxNumRates - numBasicRates;
			}
			for (i = 0; i < no_of_rates; i++) {
				if (0 != (pDot11ExtSuppRates->rates[i] & 0x7F)) {
					event.u.bitrate.value =
						hdd_translate_abg_rate_to_mbps_rate
							(&pDot11ExtSuppRates->rates[i]);

					current_pad =
						iwe_stream_add_value(scanInfo->info,
								     current_event,
								     current_pad,
								     end, &event,
								     IW_EV_PARAM_LEN);
				}
			}
		}

		if ((current_pad - current_event) >= IW_EV_LCP_LEN) {
			current_event = current_pad;
		} else {
			if (last_event == current_event) {      /* no space to add event */
				hdd_err("no space for SIOCGIWRATE");
				return -E2BIG;
			}
		}

		last_event = current_event;
		qdf_mem_zero(&event, sizeof(struct iw_event));

		event.cmd = SIOCGIWENCODE;

		if (SIR_MAC_GET_PRIVACY(capabilityInfo)) {
			event.u.data.flags =
				IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		} else {
			event.u.data.flags = IW_ENCODE_DISABLED;
		}
		event.u.data.length = 0;

		current_event =
			iwe_stream_add_point(scanInfo->info, current_event, end,
					     &event, (char *)pDot11SSID->ssid);
		if (last_event == current_event) {
			hdd_err("no space for SIOCGIWENCODE");
			return -E2BIG;
		}
	}

	last_event = current_event;
	qdf_mem_zero(&event, sizeof(struct iw_event));

	/* RSSI */
	event.cmd = IWEVQUAL;
	event.u.qual.qual = descriptor->rssi;
	event.u.qual.noise = descriptor->sinr;
	event.u.qual.level = QDF_MIN((descriptor->rssi + descriptor->sinr), 0);

	event.u.qual.updated = IW_QUAL_ALL_UPDATED;

	current_event = iwe_stream_add_event(scanInfo->info, current_event,
					     end, &event, IW_EV_QUAL_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hdd_err("no space for IWEVQUAL");
		return -E2BIG;
	}

	/* AGE */
	event.cmd = IWEVCUSTOM;
	p = custom;
	p += scnprintf(p, MAX_CUSTOM_LEN, " Age: %llu",
		       qdf_mc_timer_get_system_time() -
		       descriptor->received_time);
	event.u.data.length = p - custom;
	current_event = iwe_stream_add_point(scanInfo->info, current_event, end,
					     &event, custom);
	if (last_event == current_event) {      /* no space to add event */
		hdd_err("no space for IWEVCUSTOM (age)");
		return -E2BIG;
	}

	scanInfo->start = current_event;

	return 0;
}

/**
 * wlan_hdd_is_scan_pending() - Utility function to check pending scans
 * @adapter: Pointer to the adapter
 *
 * Utility function to check pending scans on a particular adapter
 *
 * Return: true if scans are pending, false otherwise
 */
static bool wlan_hdd_is_scan_pending(hdd_adapter_t *adapter)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	qdf_list_node_t *pnode = NULL, *ppnode = NULL;
	struct hdd_scan_req *hdd_scan_req;

	qdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);

	if (qdf_list_empty(&hdd_ctx->hdd_scan_req_q)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		return false;
	}

	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&hdd_ctx->hdd_scan_req_q,
							&ppnode)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		hdd_err("Failed to remove Scan Req from queue");
		return false;
	}

	do {
		pnode = ppnode;
		hdd_scan_req = (struct hdd_scan_req *)pnode;
		/* Any scan pending on the adapter */
		if (adapter == hdd_scan_req->adapter) {
			qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
			hdd_debug("pending scan id %d", hdd_scan_req->scan_id);
			return true;
		}
	} while (QDF_STATUS_SUCCESS ==
		qdf_list_peek_next(&hdd_ctx->hdd_scan_req_q, pnode, &ppnode));

	qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
	return false;
}
/**
 * wlan_hdd_scan_request_enqueue() - enqueue Scan Request
 * @adapter: Pointer to the adapter
 * @scan_req: Pointer to the scan request
 *
 * Enqueue scan request in the global HDD scan list.This list
 * stores the active scan request information.
 *
 * Return: 0 on success, error number otherwise
 */
static int wlan_hdd_scan_request_enqueue(hdd_adapter_t *adapter,
			struct cfg80211_scan_request *scan_req,
			uint8_t source, uint32_t scan_id,
			uint32_t timestamp)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_scan_req *hdd_scan_req;
	QDF_STATUS status;

	ENTER();
	hdd_scan_req = qdf_mem_malloc(sizeof(*hdd_scan_req));
	if (NULL == hdd_scan_req) {
		hdd_err("malloc failed for Scan req");
		return -ENOMEM;
	}

	hdd_scan_req->adapter = adapter;
	hdd_scan_req->scan_request = scan_req;
	hdd_scan_req->source = source;
	hdd_scan_req->scan_id = scan_id;
	hdd_scan_req->timestamp = timestamp;

	qdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);
	status = qdf_list_insert_back(&hdd_ctx->hdd_scan_req_q,
					&hdd_scan_req->node);
	qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Failed to enqueue Scan Req");
		qdf_mem_free(hdd_scan_req);
		return -EINVAL;
	}
	EXIT();
	return 0;
}

/**
 * wlan_hdd_scan_request_dequeue() - dequeue scan request
 * @hdd_ctx: Global HDD context
 * @scan_id: scan id
 * @req: scan request
 * @source : returns source of the scan request
 * @timestamp: scan request timestamp
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_hdd_scan_request_dequeue(hdd_context_t *hdd_ctx,
	uint32_t scan_id, struct cfg80211_scan_request **req, uint8_t *source,
	uint32_t *timestamp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct hdd_scan_req *hdd_scan_req;
	qdf_list_node_t *pNode = NULL, *ppNode = NULL;

	hdd_debug("Dequeue Scan id: %d", scan_id);

	if ((source == NULL) || (timestamp == NULL) || (req == NULL))
		return QDF_STATUS_E_NULL_VALUE;

	qdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);

	if (list_empty(&hdd_ctx->hdd_scan_req_q.anchor)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_STATUS_SUCCESS !=
		qdf_list_peek_front(&hdd_ctx->hdd_scan_req_q, &ppNode)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		hdd_err("Failed to remove Scan Req from queue");
		return QDF_STATUS_E_FAILURE;
	}

	do {
		pNode = ppNode;
		hdd_scan_req = (struct hdd_scan_req *)pNode;
		if (hdd_scan_req->scan_id == scan_id) {
			status = qdf_list_remove_node(&hdd_ctx->hdd_scan_req_q,
					pNode);
			if (status == QDF_STATUS_SUCCESS) {
				*req = hdd_scan_req->scan_request;
				*source = hdd_scan_req->source;
				*timestamp = hdd_scan_req->timestamp;
				qdf_mem_free(hdd_scan_req);
				qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
				hdd_debug("removed Scan id: %d, req = %p, pending scans %d",
				      scan_id, req,
				      qdf_list_size(&hdd_ctx->hdd_scan_req_q));
				return QDF_STATUS_SUCCESS;
			}

			qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
			hdd_err("Failed to remove node scan id %d, pending scans %d",
				scan_id,
				qdf_list_size(&hdd_ctx->hdd_scan_req_q));
			return status;
		}
	} while (QDF_STATUS_SUCCESS ==
		qdf_list_peek_next(&hdd_ctx->hdd_scan_req_q, pNode, &ppNode));

	qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
	hdd_err("Failed to find scan id %d", scan_id);
	return status;
}
/**
 * hdd_scan_request_callback() - scan complete callback from SME
 * @halHandle: Pointer to the Hal Handle
 * @pContext: Pointer to the data context
 * @sessionId: Session identifier
 * @scanId: Scan ID
 * @status: CSR Status
 *
 * The sme module calls this callback function once it finish the scan request
 * and this function notifies the scan complete event to the wpa_supplicant.
 *
 * Return: 0 for success, non zero for failure
 */

static QDF_STATUS
hdd_scan_request_callback(tHalHandle halHandle, void *pContext,
			  uint8_t sessionId, uint32_t scanId,
			  eCsrScanStatus status)
{
	struct net_device *dev = (struct net_device *)pContext;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hddctx = WLAN_HDD_GET_CTX(pAdapter);
	union iwreq_data wrqu;
	int we_event;
	char *msg;
	uint8_t source;
	struct cfg80211_scan_request *req;
	uint32_t timestamp;

	ENTER();
	hdd_debug("called with halHandle = %p, pContext = %p, scanID = %d, returned status = %d",
		 halHandle, pContext, (int)scanId, (int)status);

	/* if there is a scan request pending when the wlan driver is unloaded
	 * we may be invoked as SME flushes its pending queue.  If that is the
	 * case, the underlying net_device may have already been destroyed, so
	 * do some quick sanity before proceeding
	 */
	if (pAdapter->dev != dev) {
		hdd_debug("device mismatch %p vs %p", pAdapter->dev, dev);
		return QDF_STATUS_SUCCESS;
	}

	wlan_hdd_scan_request_dequeue(hddctx, scanId, &req, &source,
		&timestamp);

	if (req != NULL)
		hdd_err("Got unexpected request struct for Scan id %d",
			scanId);

	/* Scan is no longer pending */
	if (!wlan_hdd_is_scan_pending(pAdapter))
		pAdapter->scan_info.mScanPending = false;

	/* notify any applications that may be interested */
	memset(&wrqu, '\0', sizeof(wrqu));
	we_event = SIOCGIWSCAN;
	msg = NULL;
	wireless_send_event(dev, we_event, &wrqu, msg);

	EXIT();
	return QDF_STATUS_SUCCESS;
}

/**
 * __iw_set_scan() - set scan request
 * @dev: Pointer to the net device
 * @info: Pointer to the iw_request_info
 * @wrqu: Pointer to the iwreq_data
 * @extra: Pointer to the data
 *
 * This function process the scan request from the wpa_supplicant
 * and set the scan request to the SME
 *
 * Return: 0 for success, non zero for failure
 */

static int __iw_set_scan(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrScanRequest scanRequest;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct iw_scan_req *scanReq = (struct iw_scan_req *)extra;
	hdd_adapter_t *con_sap_adapter;
	uint16_t con_dfs_ch;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	/* Block All Scan during DFS operation and send null scan result */
	con_sap_adapter = hdd_get_con_sap_adapter(pAdapter, true);
	if (con_sap_adapter) {
		con_dfs_ch = con_sap_adapter->sessionCtx.ap.operatingChannel;

		if (wlan_reg_is_dfs_ch(hdd_ctx->hdd_pdev, con_dfs_ch)) {
			hdd_warn("##In DFS Master mode. Scan aborted");
			return -EOPNOTSUPP;
		}
	}

	qdf_mem_zero(&scanRequest, sizeof(scanRequest));

	if (NULL != wrqu->data.pointer) {
		/* set scanType, active or passive */
		if ((IW_SCAN_TYPE_ACTIVE == scanReq->scan_type) ||
		    (eSIR_ACTIVE_SCAN == hdd_ctx->ioctl_scan_mode)) {
			scanRequest.scanType = eSIR_ACTIVE_SCAN;
		} else {
			scanRequest.scanType = eSIR_PASSIVE_SCAN;
		}

		/* set bssid using sockaddr from iw_scan_req */
		qdf_mem_copy(scanRequest.bssid.bytes,
			     &scanReq->bssid.sa_data,
			     QDF_MAC_ADDR_SIZE);

		if (wrqu->data.flags & IW_SCAN_THIS_ESSID) {

			if (scanReq->essid_len &&
			    (scanReq->essid_len <= SIR_MAC_MAX_SSID_LENGTH)) {
				scanRequest.SSIDs.numOfSSIDs = 1;
				scanRequest.SSIDs.SSIDList =
					(tCsrSSIDInfo *)
					qdf_mem_malloc(sizeof(tCsrSSIDInfo));
				if (scanRequest.SSIDs.SSIDList) {
					scanRequest.SSIDs.SSIDList->SSID.
					length = scanReq->essid_len;
					qdf_mem_copy(scanRequest.SSIDs.
						     SSIDList->SSID.ssId,
						     scanReq->essid,
						     scanReq->essid_len);
				} else {
					scanRequest.SSIDs.numOfSSIDs = 0;
					hdd_err("Unable to allocate memory");
					QDF_ASSERT(0);
				}
			} else {
				hdd_err("Invalid essid length: %d",
					scanReq->essid_len);
			}
		}

		/* set min and max channel time */
		scanRequest.minChnTime = scanReq->min_channel_time;
		scanRequest.maxChnTime = scanReq->max_channel_time;

	} else {
		if (hdd_ctx->ioctl_scan_mode == eSIR_ACTIVE_SCAN) {
			/* set the scan type to active */
			scanRequest.scanType = eSIR_ACTIVE_SCAN;
		} else {
			scanRequest.scanType = eSIR_PASSIVE_SCAN;
		}

		qdf_set_macaddr_broadcast(&scanRequest.bssid);

		/* set min and max channel time to zero */
		scanRequest.minChnTime = 0;
		scanRequest.maxChnTime = 0;
	}

	/* set BSSType to default type */
	scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

	/*Scan all the channels */
	scanRequest.ChannelInfo.numOfChannels = 0;

	scanRequest.ChannelInfo.ChannelList = NULL;

	/* set requestType to full scan */
	scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

	/* if previous genIE is not NULL, update ScanIE */
	if (0 != pwextBuf->genIE.length) {
		memset(&pAdapter->scan_info.scanAddIE, 0,
		       sizeof(pAdapter->scan_info.scanAddIE));
		memcpy(pAdapter->scan_info.scanAddIE.addIEdata,
		       pwextBuf->genIE.addIEdata, pwextBuf->genIE.length);
		pAdapter->scan_info.scanAddIE.length = pwextBuf->genIE.length;

		pwextBuf->roamProfile.pAddIEScan =
			pAdapter->scan_info.scanAddIE.addIEdata;
		pwextBuf->roamProfile.nAddIEScanLength =
			pAdapter->scan_info.scanAddIE.length;

		/* clear previous genIE after use it */
		memset(&pwextBuf->genIE, 0, sizeof(pwextBuf->genIE));
	}

	/* push addIEScan in scanRequset if exist */
	if (pAdapter->scan_info.scanAddIE.length &&
	    pAdapter->scan_info.scanAddIE.length <=
	    sizeof(pAdapter->scan_info.scanAddIE.addIEdata)) {
		scanRequest.uIEFieldLen = pAdapter->scan_info.scanAddIE.length;
		scanRequest.pIEField = pAdapter->scan_info.scanAddIE.addIEdata;
	}
	scanRequest.timestamp = qdf_mc_timer_get_system_time();
	status = sme_scan_request((WLAN_HDD_GET_CTX(pAdapter))->hHal,
				  pAdapter->sessionId, &scanRequest,
				  &hdd_scan_request_callback, dev);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("sme_scan_request  fail %d!!!", status);
		goto error;
	}

	wlan_hdd_scan_request_enqueue(pAdapter, NULL, NL_SCAN,
			scanRequest.scan_id,
			scanRequest.timestamp);

	pAdapter->scan_info.mScanPending = true;
error:
	if ((wrqu->data.flags & IW_SCAN_THIS_ESSID) && (scanReq->essid_len))
		qdf_mem_free(scanRequest.SSIDs.SSIDList);
	EXIT();
	return status;
}

/**
 * iw_set_scan() - SSR wrapper for __iw_set_scan
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 on success, error number otherwise
 */
int iw_set_scan(struct net_device *dev, struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_scan(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
/**
 * __iw_get_scan() - get scan
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * This function returns the scan results to the wpa_supplicant
 *
 * Return: 0 for success, non zero for failure
 */

static int __iw_get_scan(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	tCsrScanResultInfo *pScanResult;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_scan_info_t scanInfo;
	tScanResultHandle pResult;
	int i = 0;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	ret = hdd_check_standard_wext_control(hdd_ctx, info);
	if (0 != ret)
		return ret;

	hdd_debug("enter buffer length %d!!!",
		  (wrqu->data.length) ? wrqu->data.length : IW_SCAN_MAX_DATA);

	if (true == pAdapter->scan_info.mScanPending) {
		hdd_err("mScanPending is true !!!");
		return -EAGAIN;
	}

	scanInfo.dev = dev;
	scanInfo.start = extra;
	scanInfo.info = info;

	if (0 == wrqu->data.length) {
		scanInfo.end = extra + IW_SCAN_MAX_DATA;
	} else {
		scanInfo.end = extra + wrqu->data.length;
	}

	status = sme_scan_get_result(hHal, pAdapter->sessionId, NULL, &pResult);

	if (NULL == pResult) {
		/* no scan results */
		hdd_debug("iw_get_scan: NULL Scan Result ");
		return 0;
	}

	pScanResult = sme_scan_result_get_first(hHal, pResult);

	while (pScanResult) {
		status = hdd_indicate_scan_result(&scanInfo, pScanResult);
		if (0 != status) {
			break;
		}
		i++;
		pScanResult = sme_scan_result_get_next(hHal, pResult);
	}

	sme_scan_result_purge(pResult);

	hdd_debug("exit total %d BSS reported !!!", i);
	EXIT();
	return status;
}

/**
 * iw_get_scan() - SSR wrapper function for __iw_get_scan
 * @dev: Pointer to the net device.
 * @info: Pointer to the iw_request_info.
 * @wrqu: Pointer to the iwreq_data.
 * @extra: Pointer to the data.
 *
 * Return: 0 on success, error number otherwise
 */
int iw_get_scan(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_scan(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

/**
 * hdd_abort_mac_scan() - aborts ongoing mac scan
 * @pHddCtx: Pointer to hdd context
 * @sessionId: session id
 * @scan_id: scan id
 * @reason: abort reason
 *
 * Abort any MAC scan if in progress
 *
 * Return: none
 */
void hdd_abort_mac_scan(hdd_context_t *pHddCtx, uint8_t sessionId,
			uint32_t scan_id, eCsrAbortReason reason)
{
#ifndef NAPIER_SCAN
	sme_abort_mac_scan(pHddCtx->hHal, sessionId, scan_id, reason);
#else
	wlan_abort_scan(pHddCtx->hdd_pdev, INVAL_PDEV_ID,
		sessionId, scan_id, false);
#endif
}
/**
 * hdd_vendor_scan_callback() - Scan completed callback event
 * @hddctx: HDD context
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function sends scan completed callback event to NL.
 *
 * Return: none
 */
static void hdd_vendor_scan_callback(hdd_adapter_t *adapter,
					struct cfg80211_scan_request *req,
					bool aborted)
{
	hdd_context_t *hddctx = WLAN_HDD_GET_CTX(adapter);
	struct sk_buff *skb;
	struct nlattr *attr;
	int i;
	uint8_t scan_status;
	uint64_t cookie;

	ENTER();

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("Invalid adapter magic");
		qdf_mem_free(req);
		return;
	}
	skb = cfg80211_vendor_event_alloc(hddctx->wiphy, &(adapter->wdev),
			SCAN_DONE_EVENT_BUF_SIZE + 4 + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
			GFP_KERNEL);

	if (!skb) {
		hdd_err("skb alloc failed");
		qdf_mem_free(req);
		return;
	}

	cookie = (uintptr_t)req;
	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_ssids; i++) {
		if (nla_put(skb, i, req->ssids[i].ssid_len,
			req->ssids[i].ssid)) {
			hdd_err("Failed to add ssid");
			goto nla_put_failure;
		}
	}
	nla_nest_end(skb, attr);
	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_channels; i++) {
		if (nla_put_u32(skb, i, req->channels[i]->center_freq)) {
				hdd_err("Failed to add channel");
				goto nla_put_failure;
			}
	}
	nla_nest_end(skb, attr);

	if (req->ie &&
		nla_put(skb, QCA_WLAN_VENDOR_ATTR_SCAN_IE, req->ie_len,
			req->ie)) {
		hdd_err("Failed to add scan ie");
		goto nla_put_failure;
	}
	if (req->flags &&
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS, req->flags)) {
		hdd_err("Failed to add scan flags");
		goto nla_put_failure;
	}
	if (hdd_wlan_nla_put_u64(skb,
				  QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				  cookie)) {
		hdd_err("Failed to add scan cookie");
		goto nla_put_failure;
	}
	scan_status = (aborted == true) ? VENDOR_SCAN_STATUS_ABORTED :
		VENDOR_SCAN_STATUS_NEW_RESULTS;
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_SCAN_STATUS, scan_status)) {
		hdd_err("Failed to add scan staus");
		goto nla_put_failure;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);
	hdd_info("scan complete event sent to NL");
	qdf_mem_free(req);
	return;

nla_put_failure:
	kfree_skb(skb);
	qdf_mem_free(req);
	return;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(hdd_adapter_t *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	struct cfg80211_scan_info info = {
		.aborted = aborted
	};

	if (adapter->dev->flags & IFF_UP)
		cfg80211_scan_done(req, &info);
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(hdd_adapter_t *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	if (adapter->dev->flags & IFF_UP)
		cfg80211_scan_done(req, aborted);
}
#else
/**
 * hdd_cfg80211_scan_done() - Scan completed callback to cfg80211
 * @adapter: Pointer to the adapter
 * @req : Scan request
 * @aborted : true scan aborted false scan success
 *
 * This function notifies scan done to cfg80211
 *
 * Return: none
 */
static void hdd_cfg80211_scan_done(hdd_adapter_t *adapter,
				   struct cfg80211_scan_request *req,
				   bool aborted)
{
	cfg80211_scan_done(req, aborted);
}
#endif
#ifndef NAPIER_SCAN
/**
 * hdd_cfg80211_scan_done_callback() - scan done callback function called after
 *				       scan is finished
 * @halHandle: Pointer to handle
 * @pContext: Pointer to context
 * @sessionId: Session Id
 * @scanId: Scan Id
 * @status: Scan status
 *
 * Return: QDF status
 */
static QDF_STATUS hdd_cfg80211_scan_done_callback(tHalHandle halHandle,
					   void *pContext,
					   uint8_t sessionId,
					   uint32_t scanId,
					   eCsrScanStatus status)
{
	struct net_device *dev = (struct net_device *)pContext;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_scaninfo_t *pScanInfo = &pAdapter->scan_info;
	struct cfg80211_scan_request *req = NULL;
	bool aborted = false;
	hdd_context_t *hddctx = WLAN_HDD_GET_CTX(pAdapter);
	int ret = 0;
	unsigned int current_timestamp, time_elapsed;
	uint8_t source;
	uint32_t scan_time;
	uint32_t size = 0;

	ret = wlan_hdd_validate_context(hddctx);
	if (0 != ret)
		return QDF_STATUS_E_INVAL;

	hdd_debug("called with hal = %p, pContext = %p, ID = %d, status = %d",
		   halHandle, pContext, (int)scanId, (int)status);

	pScanInfo->mScanPendingCounter = 0;

	if (pScanInfo->mScanPending != true) {
		QDF_ASSERT(pScanInfo->mScanPending);
		goto allow_suspend;
	}

	if (QDF_STATUS_SUCCESS !=
		wlan_hdd_scan_request_dequeue(hddctx, scanId, &req, &source,
			&scan_time)) {
		hdd_err("Dequeue of scan request failed ID: %d", scanId);
		goto allow_suspend;
	}

	/*
	 * cfg80211_scan_done informing NL80211 about completion
	 * of scanning
	 */
	if (status == eCSR_SCAN_ABORT || status == eCSR_SCAN_FAILURE) {
		aborted = true;
	}

	if (!aborted && !hddctx->beacon_probe_rsp_cnt_per_scan) {
		hdd_debug("NO SCAN result");
		if (hddctx->config->bug_report_for_no_scan_results) {
			current_timestamp = jiffies_to_msecs(jiffies);
			time_elapsed = current_timestamp -
				hddctx->last_nil_scan_bug_report_timestamp;

			/*
			 * check if we have generated bug report in
			 * MIN_TIME_REQUIRED_FOR_NEXT_BUG_REPORT time.
			 */
			if (time_elapsed >
			    MIN_TIME_REQUIRED_FOR_NEXT_BUG_REPORT) {
				cds_flush_logs(WLAN_LOG_TYPE_NON_FATAL,
						WLAN_LOG_INDICATOR_HOST_DRIVER,
						WLAN_LOG_REASON_NO_SCAN_RESULTS,
						true, false);
				hddctx->last_nil_scan_bug_report_timestamp =
					current_timestamp;
			}
		}
	}
	hddctx->beacon_probe_rsp_cnt_per_scan = 0;

	if (!wlan_hdd_is_scan_pending(pAdapter)) {
		/* Scan is no longer pending */
		pScanInfo->mScanPending = false;
		complete(&pScanInfo->abortscan_event_var);
	}

	/*
	 * Scan can be triggred from NL or vendor scan
	 * - If scan is triggered from NL then cfg80211 scan done should be
	 * called to updated scan completion to NL.
	 * - If scan is triggred through vendor command then
	 * scan done event will be posted
	 */
	if (NL_SCAN == source)
		hdd_cfg80211_scan_done(pAdapter, req, aborted);
	else
		hdd_vendor_scan_callback(pAdapter, req, aborted);

allow_suspend:
	qdf_spin_lock(&hddctx->hdd_scan_req_q_lock);
	size = qdf_list_size(&hddctx->hdd_scan_req_q);
	if (!size) {
		qdf_spin_unlock(&hddctx->hdd_scan_req_q_lock);
		/* release the wake lock at the end of the scan */
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_SCAN);

		/* Acquire wakelock to handle the case where APP's tries
		 * to suspend immediatly after the driver gets connect
		 * request(i.e after scan) from supplicant, this result in
		 * app's is suspending and not ableto process the connect
		 * request to AP
		 */
		hdd_prevent_suspend_timeout(1000,
			WIFI_POWER_EVENT_WAKELOCK_SCAN);
	} else {
		/* Release the spin lock */
		qdf_spin_unlock(&hddctx->hdd_scan_req_q_lock);
	}

#ifdef FEATURE_WLAN_TDLS
	wlan_hdd_tdls_scan_done_callback(pAdapter);
#endif

	EXIT();
	return 0;
}
#endif
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
/**
 * wlan_hdd_sap_skip_scan_check() - The function will check OBSS
 *         scan skip or not for SAP.
 * @hdd_ctx: pointer to hdd context.
 * @request: pointer to scan request.
 *
 * This function will check the scan request's chan list against the
 * previous ACS scan chan list. If all the chan are covered by
 * previous ACS scan, we can skip the scan and return scan complete
 * to save the SAP starting time.
 *
 * Return: true to skip the scan,
 *            false to continue the scan
 */
static bool wlan_hdd_sap_skip_scan_check(hdd_context_t *hdd_ctx,
	struct cfg80211_scan_request *request)
{
	int i, j;
	bool skip;

	hdd_debug("HDD_ACS_SKIP_STATUS = %d",
		hdd_ctx->skip_acs_scan_status);
	if (hdd_ctx->skip_acs_scan_status != eSAP_SKIP_ACS_SCAN)
		return false;
	qdf_spin_lock(&hdd_ctx->acs_skip_lock);
	if (hdd_ctx->last_acs_channel_list == NULL ||
	   hdd_ctx->num_of_channels == 0 ||
	   request->n_channels == 0) {
		qdf_spin_unlock(&hdd_ctx->acs_skip_lock);
		return false;
	}
	skip = true;
	for (i = 0; i < request->n_channels ; i++) {
		bool find = false;
		for (j = 0; j < hdd_ctx->num_of_channels; j++) {
			if (hdd_ctx->last_acs_channel_list[j] ==
			   request->channels[i]->hw_value) {
				find = true;
				break;
			}
		}
		if (!find) {
			skip = false;
			hdd_debug("Chan %d isn't in ACS chan list",
				request->channels[i]->hw_value);
			break;
		}
	}
	qdf_spin_unlock(&hdd_ctx->acs_skip_lock);
	return skip;
}
#else
static bool wlan_hdd_sap_skip_scan_check(hdd_context_t *hdd_ctx,
	struct cfg80211_scan_request *request)
{
	return false;
}
#endif

/**
 * wlan_hdd_cfg80211_scan_block_cb() - scan block work handler
 * @work: Pointer to work
 *
 * Return: none
 */
static void wlan_hdd_cfg80211_scan_block_cb(struct work_struct *work)
{
	hdd_adapter_t *adapter = container_of(work,
					      hdd_adapter_t, scan_block_work);
	struct cfg80211_scan_request *request;
	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("HDD adapter context is invalid");
		return;
	}

	request = adapter->request;
	if (request) {
		request->n_ssids = 0;
		request->n_channels = 0;

		hdd_err("##In DFS Master mode. Scan aborted. Null result sent");
		if (NL_SCAN == adapter->scan_source)
			hdd_cfg80211_scan_done(adapter, request, true);
		else
			hdd_vendor_scan_callback(adapter, request, true);
		adapter->request = NULL;
	}
}

/**
 * wlan_hdd_copy_bssid_scan_request() - API to copy the bssid to Scan request
 * @scan_req: Pointer to CSR Scan Request
 * @request: scan request from Supplicant
 *
 * This API copies the BSSID in scan request from Supplicant and copies it to
 * the CSR Scan request
 *
 * Return: None
 */
#if defined(CFG80211_SCAN_BSSID) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void wlan_hdd_copy_bssid_scan_request(tCsrScanRequest *scan_req,
					struct cfg80211_scan_request *request)
{
	qdf_mem_copy(scan_req->bssid.bytes, request->bssid, QDF_MAC_ADDR_SIZE);
}
#else
static inline void wlan_hdd_copy_bssid_scan_request(tCsrScanRequest *scan_req,
					struct cfg80211_scan_request *request)
{
}
#endif

/*
 * wlan_hdd_update_scan_ies() - API to update the scan IEs of scan request
 * with already stored default scan IEs
 *
 * @adapter: Pointer to HDD adapter
 * @scan_info: Pointer to scan info in HDD adapter
 * @scan_ie: Pointer to scan IE in scan request
 * @scan_ie_len: Pointer to scan IE length in scan request
 *
 * Return: 0 on success; error number otherwise
 */
static int wlan_hdd_update_scan_ies(hdd_adapter_t *adapter,
			hdd_scaninfo_t *scan_info, uint8_t *scan_ie,
			uint8_t *scan_ie_len)
{
	uint8_t rem_len = scan_info->default_scan_ies_len;
	uint8_t *temp_ie = scan_info->default_scan_ies;
	uint8_t *current_ie;
	uint8_t elem_id;
	uint16_t elem_len;

	if (!scan_info->default_scan_ies_len || !scan_info->default_scan_ies)
		return 0;

	while (rem_len >= 2) {
		current_ie = temp_ie;
		elem_id = *temp_ie++;
		elem_len = *temp_ie++;
		rem_len -= 2;

		switch (elem_id) {
		case DOT11F_EID_EXTCAP:
			if (!wlan_hdd_cfg80211_get_ie_ptr(scan_ie, *scan_ie_len,
						DOT11F_EID_EXTCAP)) {
				qdf_mem_copy(scan_ie + (*scan_ie_len),
						current_ie, elem_len + 2);
				(*scan_ie_len) += (elem_len + 2);
			}
			break;
		case DOT11F_EID_WPA:
			if (!wlan_hdd_get_mbo_ie_ptr(scan_ie, *scan_ie_len)) {
				qdf_mem_copy(scan_ie + (*scan_ie_len),
						current_ie, elem_len + 2);
				(*scan_ie_len) += (elem_len + 2);
			}
			break;
		}
		temp_ie += elem_len;
		rem_len -= elem_len;
	}
	return 0;
}

/**
 * __wlan_hdd_cfg80211_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @request: Pointer to scan request
 * @source: scan request source(NL/Vendor scan)
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
				    struct cfg80211_scan_request *request,
				    uint8_t source)
{
	struct net_device *dev = request->wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	struct hdd_config *cfg_param = NULL;
	int status;
	hdd_scaninfo_t *pScanInfo = NULL;
	hdd_adapter_t *con_sap_adapter;
	uint16_t con_dfs_ch;
	hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
#ifndef NAPIER_SCAN
	tCsrScanRequest scan_req;
	uint8_t *channelList = NULL, i;
	uint8_t *pP2pIe = NULL;
	uint8_t num_chan = 0;
	bool is_p2p_scan = false;
#endif
	uint8_t curr_session_id;
	enum scan_reject_states curr_reason;
	static uint32_t scan_ebusy_cnt;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
		hdd_err("invalid session id: %d", pAdapter->sessionId);
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(pHddCtx);
	if (0 != status)
		return status;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SCAN,
			 pAdapter->sessionId, request->n_channels));

	if (!sme_is_session_id_valid(pHddCtx->hHal, pAdapter->sessionId))
		return -EINVAL;

	hdd_debug("Device_mode %s(%d)",
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);

	/*
	 * IBSS vdev does not need to scan to establish
	 * IBSS connection. If IBSS vdev need to support scan,
	 * Firmware need to make the change to add self peer
	 * per mac for IBSS vdev.
	 */
	if (QDF_IBSS_MODE == pAdapter->device_mode) {
		hdd_err("Scan not supported for IBSS");
		return -EINVAL;
	}

	cfg_param = pHddCtx->config;
	pScanInfo = &pAdapter->scan_info;

	/* Block All Scan during DFS operation and send null scan result */
	con_sap_adapter = hdd_get_con_sap_adapter(pAdapter, true);
	if (con_sap_adapter) {
		con_dfs_ch = con_sap_adapter->sessionCtx.ap.sapConfig.channel;
		if (con_dfs_ch == AUTO_CHANNEL_SELECT)
			con_dfs_ch =
				con_sap_adapter->sessionCtx.ap.operatingChannel;

		if (!policy_mgr_is_hw_dbs_capable(pHddCtx->hdd_psoc) &&
			wlan_reg_is_dfs_ch(pHddCtx->hdd_pdev, con_dfs_ch)) {
			/* Provide empty scan result during DFS operation since
			 * scanning not supported during DFS. Reason is
			 * following case:
			 * DFS is supported only in SCC for MBSSID Mode.
			 * We shall not return EBUSY or ENOTSUPP as when Primary
			 * AP is operating in DFS channel and secondary AP is
			 * started. Though we force SCC in driver, the hostapd
			 * issues obss scan before starting secAP. This results
			 * in MCC in DFS mode. Thus we return null scan result.
			 * If we return scan failure hostapd fails secondary AP
			 * startup.
			 */
			hdd_err("##In DFS Master mode. Scan aborted");
			pAdapter->request = request;
			pAdapter->scan_source = source;

			INIT_WORK(&pAdapter->scan_block_work,
				  wlan_hdd_cfg80211_scan_block_cb);
			schedule_work(&pAdapter->scan_block_work);
			return 0;
		}
	}
#ifdef FEATURE_WLAN_TDLS
	/* if tdls disagree scan right now, return immediately.
	 * tdls will schedule the scan when scan is allowed.
	 * (return SUCCESS)
	 * or will reject the scan if any TDLS is in progress.
	 * (return -EBUSY)
	 */
	status = wlan_hdd_tdls_scan_callback(pAdapter, wiphy,
					request, source);
	if (status <= 0) {
		if (!status)
			hdd_err("TDLS in progress.scan rejected %d",
			status);
		else
			hdd_warn("TDLS teardown is ongoing %d",
			       status);
		hdd_wlan_block_scan_by_tdls_event();
		return status;
	}
#endif

	/* Check if scan is allowed at this point of time */
	if (hdd_is_connection_in_progress(&curr_session_id, &curr_reason)) {
		scan_ebusy_cnt++;
		hdd_err("Scan not allowed. scan_ebusy_cnt: %d", scan_ebusy_cnt);
		if (pHddCtx->last_scan_reject_session_id != curr_session_id ||
		    pHddCtx->last_scan_reject_reason != curr_reason ||
		    !pHddCtx->last_scan_reject_timestamp) {
			pHddCtx->last_scan_reject_session_id = curr_session_id;
			pHddCtx->last_scan_reject_reason = curr_reason;
			pHddCtx->last_scan_reject_timestamp =
				jiffies_to_msecs(jiffies);
		} else {
			hdd_debug("curr_session id %d curr_reason %d time delta %lu",
				curr_session_id, curr_reason,
				(jiffies_to_msecs(jiffies) -
				 pHddCtx->last_scan_reject_timestamp));
			if ((jiffies_to_msecs(jiffies) -
			    pHddCtx->last_scan_reject_timestamp) >=
			    SCAN_REJECT_THRESHOLD_TIME) {
				pHddCtx->last_scan_reject_timestamp = 0;
				if (pHddCtx->config->enable_fatal_event) {
					cds_flush_logs(WLAN_LOG_TYPE_FATAL,
					   WLAN_LOG_INDICATOR_HOST_DRIVER,
					   WLAN_LOG_REASON_SCAN_NOT_ALLOWED,
					   false,
					   pHddCtx->config->enableSelfRecovery);
				} else if (pHddCtx->config->
					   enableSelfRecovery) {
					hdd_err("Triggering SSR due to scan stuck");
					cds_trigger_recovery(false);
				} else {
					hdd_err("QDF_BUG due to scan stuck");
					QDF_BUG(0);
				}
			}
		}
		return -EBUSY;
	}
	pHddCtx->last_scan_reject_timestamp = 0;
	pHddCtx->last_scan_reject_session_id = 0xFF;
	pHddCtx->last_scan_reject_reason = 0;

	/* Check whether SAP scan can be skipped or not */
	if (pAdapter->device_mode == QDF_SAP_MODE &&
	   wlan_hdd_sap_skip_scan_check(pHddCtx, request)) {
		hdd_debug("sap scan skipped");
		pAdapter->request = request;
		pAdapter->scan_source = source;
		INIT_WORK(&pAdapter->scan_block_work,
			wlan_hdd_cfg80211_scan_block_cb);
		schedule_work(&pAdapter->scan_block_work);
		return 0;
	}

	/* Store the Scan IE's in Adapter*/
	if (request->ie_len) {
		/* save this for future association (join requires this) */
		memset(&pScanInfo->scanAddIE, 0, sizeof(pScanInfo->scanAddIE));
		memcpy(pScanInfo->scanAddIE.addIEdata, request->ie,
		       request->ie_len);
		pScanInfo->scanAddIE.length = request->ie_len;

		if (wlan_hdd_update_scan_ies(pAdapter, pScanInfo,
				pScanInfo->scanAddIE.addIEdata,
				(uint8_t *)&pScanInfo->scanAddIE.length))
			hdd_err("Update scan IEs with default Scan IEs failed");

		if ((QDF_STA_MODE == pAdapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
		    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
		    ) {
			pwextBuf->roamProfile.pAddIEScan =
				pScanInfo->scanAddIE.addIEdata;
			pwextBuf->roamProfile.nAddIEScanLength =
				pScanInfo->scanAddIE.length;
		}
	}
#ifdef NAPIER_SCAN
	return wlan_cfg80211_scan(pHddCtx->hdd_pdev, request, source);
#else
	/* Below code will be removed once common scan module is available.*/
	qdf_mem_zero(&scan_req, sizeof(scan_req));

	scan_req.timestamp = qdf_mc_timer_get_system_time();

	/* Even though supplicant doesn't provide any SSIDs, n_ssids is
	 * set to 1.  Because of this, driver is assuming that this is not
	 * wildcard scan and so is not aging out the scan results.
	 */
	if ((request->ssids) && (request->n_ssids == 1) &&
	    ('\0' == request->ssids->ssid[0])) {
		request->n_ssids = 0;
	}

	if ((request->ssids) && (0 < request->n_ssids)) {
		tCsrSSIDInfo *SsidInfo;
		int j;
		scan_req.SSIDs.numOfSSIDs = request->n_ssids;
		/* Allocate num_ssid tCsrSSIDInfo structure */
		SsidInfo = scan_req.SSIDs.SSIDList =
			qdf_mem_malloc(request->n_ssids * sizeof(tCsrSSIDInfo));

		if (NULL == scan_req.SSIDs.SSIDList) {
			hdd_err("memory alloc failed SSIDInfo buffer");
			return -ENOMEM;
		}

		/* copy all the ssid's and their length */
		for (j = 0; j < request->n_ssids; j++, SsidInfo++) {
			/* get the ssid length */
			SsidInfo->SSID.length = request->ssids[j].ssid_len;
			qdf_mem_copy(SsidInfo->SSID.ssId,
				     &request->ssids[j].ssid[0],
				     SsidInfo->SSID.length);
			SsidInfo->SSID.ssId[SsidInfo->SSID.length] = '\0';
			hdd_debug("SSID number %d: %s", j,
				SsidInfo->SSID.ssId);
		}
		/* set the scan type to active */
		scan_req.scanType = eSIR_ACTIVE_SCAN;
	} else if (QDF_P2P_GO_MODE == pAdapter->device_mode) {
		/* set the scan type to active */
		scan_req.scanType = eSIR_ACTIVE_SCAN;
	} else {
		/*
		 * Set the scan type to passive if there is no ssid list
		 * provided else set default type configured in the driver.
		 */
		if (!request->ssids)
			scan_req.scanType = eSIR_PASSIVE_SCAN;
		else
			scan_req.scanType = pHddCtx->ioctl_scan_mode;
	}
	if (scan_req.scanType == eSIR_PASSIVE_SCAN) {
		scan_req.minChnTime = cfg_param->nPassiveMinChnTime;
		scan_req.maxChnTime = cfg_param->nPassiveMaxChnTime;
	} else {
		scan_req.minChnTime = cfg_param->nActiveMinChnTime;
		scan_req.maxChnTime = cfg_param->nActiveMaxChnTime;
	}

	wlan_hdd_copy_bssid_scan_request(&scan_req, request);

	/* set BSSType to default type */
	scan_req.BSSType = eCSR_BSS_TYPE_ANY;

	if (MAX_CHANNEL < request->n_channels) {
		hdd_warn("No of Scan Channels exceeded limit: %d",
		       request->n_channels);
		request->n_channels = MAX_CHANNEL;
	}

	if (request->n_channels) {
		char chList[(request->n_channels * 5) + 1];
		int len;
		channelList = qdf_mem_malloc(request->n_channels);
		if (NULL == channelList) {
			hdd_err("channelList malloc failed channelList");
			status = -ENOMEM;
			goto free_mem;
		}
		for (i = 0, len = 0; i < request->n_channels; i++) {
			if (WLAN_REG_IS_11P_CH(
					pHddCrequest->channels[i]->hw_value))
				continue;

			channelList[num_chan] = request->channels[i]->hw_value;
			len += snprintf(chList + len, 5, "%d ", channelList[i]);
			num_chan++;
		}
		hdd_debug("Channel-List: %s", chList);
		hdd_debug("No. of Scan Channels: %d", num_chan);
	}
	if (!num_chan) {
		hdd_err("Received zero non-dsrc channels");
		status = -EINVAL;
		goto free_mem;
	}

	scan_req.ChannelInfo.numOfChannels = num_chan;
	scan_req.ChannelInfo.ChannelList = channelList;

	/* set requestType to full scan */
	scan_req.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

	/* Flush the scan results(only p2p beacons) for STA scan and P2P
	 * search (Flush on both full  scan and social scan but not on single
	 * channel scan).P2P  search happens on 3 social channels (1, 6, 11)
	 */

	/* Supplicant does single channel scan after 8-way handshake
	 * and in that case driver shoudnt flush scan results. If
	 * driver flushes the scan results here and unfortunately if
	 * the AP doesnt respond to our probe req then association
	 * fails which is not desired
	 */

	if ((request->n_ssids == 1) &&
		(request->ssids != NULL) &&
		qdf_mem_cmp(&request->ssids[0], "DIRECT-", 7))
		is_p2p_scan = true;

	if (is_p2p_scan ||
		(request->n_channels != WLAN_HDD_P2P_SINGLE_CHANNEL_SCAN)) {
		hdd_debug("Flushing P2P Results");
		sme_scan_flush_p2p_result(WLAN_HDD_GET_HAL_CTX(pAdapter),
			pAdapter->sessionId);
	}
	if (request->ie_len) {
		scan_req.uIEFieldLen = pScanInfo->scanAddIE.length;
		scan_req.pIEField = pScanInfo->scanAddIE.addIEdata;

		pP2pIe = wlan_hdd_get_p2p_ie_ptr((uint8_t *) request->ie,
						 request->ie_len);
		if (pP2pIe != NULL) {
#ifdef WLAN_FEATURE_P2P_DEBUG
			if (((global_p2p_connection_status == P2P_GO_NEG_COMPLETED)
			     || (global_p2p_connection_status ==
				 P2P_GO_NEG_PROCESS))
			    && (QDF_P2P_CLIENT_MODE == pAdapter->device_mode)) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTING_STATE_1;
				hdd_debug("[P2P State] Changing state from Go nego completed to Connection is started");
				hdd_debug("[P2P]P2P Scanning is started for 8way Handshake");
			} else
			if ((global_p2p_connection_status ==
			     P2P_CLIENT_DISCONNECTED_STATE)
			    && (QDF_P2P_CLIENT_MODE ==
				pAdapter->device_mode)) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTING_STATE_2;
				hdd_debug("[P2P State] Changing state from Disconnected state to Connection is started");
				hdd_debug("[P2P]P2P Scanning is started for 4way Handshake");
			}
#endif

			/* no_cck will be set during p2p find to disable 11b rates */
			if (request->no_cck) {
				hdd_debug("This is a P2P Search");
				scan_req.p2pSearch = 1;

				if (request->n_channels ==
				    WLAN_HDD_P2P_SOCIAL_CHANNELS) {
					/* set requestType to P2P Discovery */
					scan_req.requestType =
						eCSR_SCAN_P2P_DISCOVERY;
				}

				/*
				 * Skip Dfs Channel in case of P2P Search if it is set in
				 * ini file
				 */
				if (cfg_param->skipDfsChnlInP2pSearch) {
					scan_req.skipDfsChnlInP2pSearch = 1;
				} else {
					scan_req.skipDfsChnlInP2pSearch = 0;
				}

			}
		}
	} else {
		if (pScanInfo->default_scan_ies &&
				pScanInfo->default_scan_ies_len) {
			qdf_mem_copy(pScanInfo->scanAddIE.addIEdata,
					pScanInfo->default_scan_ies,
					pScanInfo->default_scan_ies_len);
			pScanInfo->scanAddIE.length =
					pScanInfo->default_scan_ies_len;
		}
	}

	/* acquire the wakelock to avoid the apps suspend during the scan. To
	 * address the following issues.
	 * 1) Disconnected scenario: we are not allowing the suspend as WLAN is not in
	 * BMPS/IMPS this result in android trying to suspend aggressively and backing off
	 * for long time, this result in apps running at full power for long time.
	 * 2) Connected scenario: If we allow the suspend during the scan, RIVA will
	 * be stuck in full power because of resume BMPS
	 */
	hdd_prevent_suspend_timeout(HDD_WAKE_LOCK_SCAN_DURATION,
				    WIFI_POWER_EVENT_WAKELOCK_SCAN);

	hdd_debug("requestType %d, scanType %d, minChnTime %d, maxChnTime %d,p2pSearch %d, skipDfsChnlIn P2pSearch %d",
	       scan_req.requestType, scan_req.scanType,
	       scan_req.minChnTime, scan_req.maxChnTime,
	       scan_req.p2pSearch, scan_req.skipDfsChnlInP2pSearch);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 7, 0))
	if (request->flags & NL80211_SCAN_FLAG_FLUSH)
		sme_scan_flush_result(WLAN_HDD_GET_HAL_CTX(pAdapter));
#endif
	status = sme_scan_request(WLAN_HDD_GET_HAL_CTX(pAdapter),
				pAdapter->sessionId, &scan_req,
				&hdd_cfg80211_scan_done_callback, dev);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("sme_scan_request returned error %d", status);
		if (QDF_STATUS_E_RESOURCES == status) {
			scan_ebusy_cnt++;
			hdd_err("HO is in progress. Defer scan scan_ebusy_cnt: %d",
				scan_ebusy_cnt);
			status = -EBUSY;
		} else {
			status = -EIO;
		}
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_SCAN);
		goto free_mem;
	}
	wlan_hdd_scan_request_enqueue(pAdapter, request, source,
			scan_req.scan_id, scan_req.timestamp);
	pAdapter->scan_info.mScanPending = true;
	pHddCtx->beacon_probe_rsp_cnt_per_scan = 0;
free_mem:
	if (scan_req.SSIDs.SSIDList)
		qdf_mem_free(scan_req.SSIDs.SSIDList);

	if (channelList)
		qdf_mem_free(channelList);

	if (status == 0)
		scan_ebusy_cnt = 0;

	EXIT();
	return status;
#endif

}

/**
 * wlan_hdd_cfg80211_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @request: Pointer to scan request
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
			   struct cfg80211_scan_request *request)
{
	int ret;
	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_scan(wiphy,
				request, NL_SCAN);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * wlan_hdd_cfg80211_tdls_scan() - API to process cfg80211 scan request
 * @wiphy: Pointer to wiphy
 * @request: Pointer to scan request
 * @source: scan request source(NL/Vendor scan)
 *
 * This API responds to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results. This
 * function gets called when tdls module queues the scan request.
 *
 * Return: 0 for success, non zero for failure.
 */
int wlan_hdd_cfg80211_tdls_scan(struct wiphy *wiphy,
				struct cfg80211_scan_request *request,
				uint8_t source)
{
	int ret;
	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_scan(wiphy,
				request, source);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * wlan_hdd_get_rates() -API to get the rates from scan request
 * @wiphy: Pointer to wiphy
 * @band: Band
 * @rates: array of rates
 * @rate_count: number of rates
 *
 * Return: o for failure, rate bitmap for success
 */
static uint32_t wlan_hdd_get_rates(struct wiphy *wiphy,
	enum nl80211_band band,
	const u8 *rates, unsigned int rate_count)
{
	uint32_t j, count, rate_bitmap = 0;
	uint32_t rate;
	bool found;

	for (count = 0; count < rate_count; count++) {
		rate = ((rates[count]) & RATE_MASK) * 5;
		found = false;
		for (j = 0; j < wiphy->bands[band]->n_bitrates; j++) {
			if (wiphy->bands[band]->bitrates[j].bitrate == rate) {
				found = true;
				rate_bitmap |= (1 << j);
				break;
			}
		}
		if (!found)
			return 0;
	}
	return rate_bitmap;
}

/**
 * wlan_hdd_send_scan_start_event() -API to send the scan start event
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @cookie: scan identifier
 *
 * Return: return 0 on success and negative error code on failure
 */
static int wlan_hdd_send_scan_start_event(struct wiphy *wiphy,
		struct wireless_dev *wdev, uint64_t cookie)
{
	struct sk_buff *skb;
	int ret;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u64) +
			NLA_HDRLEN + NLMSG_HDRLEN);
	if (!skb) {
		hdd_err(" reply skb alloc failed");
		return -ENOMEM;
	}

	if (hdd_wlan_nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				 cookie)) {
		hdd_err("nla put fail");
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = cfg80211_vendor_cmd_reply(skb);

	/* Send a scan started event to supplicant */
	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
		sizeof(u64) + 4 + NLMSG_HDRLEN,
		QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX, GFP_KERNEL);
	if (!skb) {
		hdd_err("skb alloc failed");
		return -ENOMEM;
	}

	if (hdd_wlan_nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
				 cookie)) {
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);

	return ret;
}

/**
 * wlan_hdd_copy_bssid() - API to copy the bssid to vendor Scan request
 * @request: Pointer to vendor scan request
 * @bssid: Pointer to BSSID
 *
 * This API copies the specific BSSID received from Supplicant and copies it to
 * the vendor Scan request
 *
 * Return: None
 */
#if defined(CFG80211_SCAN_BSSID) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
static inline void wlan_hdd_copy_bssid(struct cfg80211_scan_request *request,
					uint8_t *bssid)
{
	qdf_mem_copy(request->bssid, bssid, QDF_MAC_ADDR_SIZE);
}
#else
static inline void wlan_hdd_copy_bssid(struct cfg80211_scan_request *request,
					uint8_t *bssid)
{
}
#endif

static void hdd_process_vendor_acs_response(hdd_adapter_t *adapter)
{
	if (test_bit(VENDOR_ACS_RESPONSE_PENDING, &adapter->event_flags)) {
		if (QDF_TIMER_STATE_RUNNING ==
		    qdf_mc_timer_get_current_state(&adapter->sessionCtx.
					ap.vendor_acs_timer)) {
			qdf_mc_timer_stop(&adapter->sessionCtx.
					ap.vendor_acs_timer);
		}
	}
}

/**
 * __wlan_hdd_cfg80211_vendor_scan() - API to process venor scan request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * API to process venor scan request.
 *
 * Return: return 0 on success and negative error code on failure
 */
static int __wlan_hdd_cfg80211_vendor_scan(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data,
		int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1];
	struct cfg80211_scan_request *request = NULL;
	struct nlattr *attr;
	enum nl80211_band band;
	uint8_t n_channels = 0, n_ssid = 0, ie_len = 0;
	uint32_t tmp, count, j;
	unsigned int len;
	struct ieee80211_channel *chan;
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	int ret;

	ENTER_DEV(wdev->netdev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SCAN_MAX, data,
		data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES]) {
		nla_for_each_nested(attr,
			tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES], tmp)
			n_channels++;
	} else {
		for (band = 0; band < NUM_NL80211_BANDS; band++)
			if (wiphy->bands[band])
				n_channels += wiphy->bands[band]->n_channels;
	}

	if (MAX_CHANNEL < n_channels) {
		hdd_err("Exceed max number of channels: %d", n_channels);
		return -EINVAL;
	}
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS])
		nla_for_each_nested(attr,
			tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS], tmp)
			n_ssid++;

	if (MAX_SCAN_SSID < n_ssid) {
		hdd_err("Exceed max number of SSID: %d", n_ssid);
		return -EINVAL;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE])
		ie_len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]);
	else
		ie_len = 0;

	len = sizeof(*request) + (sizeof(*request->ssids) * n_ssid) +
			(sizeof(*request->channels) * n_channels) + ie_len;

	request = qdf_mem_malloc(len);
	if (!request)
		goto error;
	if (n_ssid)
		request->ssids = (void *)&request->channels[n_channels];
	request->n_ssids = n_ssid;
	if (ie_len) {
		if (request->ssids)
			request->ie = (void *)(request->ssids + n_ssid);
		else
			request->ie = (void *)(request->channels + n_channels);
	}

	count = 0;
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES]) {
		nla_for_each_nested(attr,
				tb[QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES],
				tmp) {
			chan = __ieee80211_get_channel(wiphy,
							nla_get_u32(attr));
			if (!chan)
				goto error;
			if (chan->flags & IEEE80211_CHAN_DISABLED)
				continue;
			request->channels[count] = chan;
			count++;
		}
	} else {
		for (band = 0; band < NUM_NL80211_BANDS; band++) {
			if (!wiphy->bands[band])
				continue;
			for (j = 0; j < wiphy->bands[band]->n_channels;
				j++) {
				chan = &wiphy->bands[band]->channels[j];
				if (chan->flags & IEEE80211_CHAN_DISABLED)
					continue;
				request->channels[count] = chan;
				count++;
			}
		}
	}

	if (!count)
		goto error;

	request->n_channels = count;
	count = 0;
	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS]) {
		nla_for_each_nested(attr, tb[QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS],
				tmp) {
			request->ssids[count].ssid_len = nla_len(attr);
			if (request->ssids[count].ssid_len >
				SIR_MAC_MAX_SSID_LENGTH) {
				hdd_err("SSID Len %d is not correct for network %d",
					 request->ssids[count].ssid_len, count);
				goto error;
			}
			memcpy(request->ssids[count].ssid, nla_data(attr),
					nla_len(attr));
			count++;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]) {
		request->ie_len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]);
		memcpy((void *)request->ie,
				nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_IE]),
				request->ie_len);
	}

	for (count = 0; count < NUM_NL80211_BANDS; count++)
		if (wiphy->bands[count])
			request->rates[count] =
				(1 << wiphy->bands[count]->n_bitrates) - 1;

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES]) {
		nla_for_each_nested(attr,
				    tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES],
				    tmp) {
			band = nla_type(attr);
			if (band >= NUM_NL80211_BANDS)
				continue;
			if (!wiphy->bands[band])
				continue;
			request->rates[band] =
				wlan_hdd_get_rates(wiphy,
						   band, nla_data(attr),
						   nla_len(attr));
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]) {
		request->flags =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]);
		if ((request->flags & NL80211_SCAN_FLAG_LOW_PRIORITY) &&
		    !(wiphy->features & NL80211_FEATURE_LOW_PRIORITY_SCAN)) {
			hdd_err("LOW PRIORITY SCAN not supported");
			goto error;
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]) {
		if (nla_len(tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]) <
		    QDF_MAC_ADDR_SIZE) {
			hdd_err("invalid bssid length");
			goto error;
		}
		wlan_hdd_copy_bssid(request,
			nla_data(tb[QCA_WLAN_VENDOR_ATTR_SCAN_BSSID]));
	}

	/* Check if external acs was requested on this adapter */
	hdd_process_vendor_acs_response(adapter);

	request->no_cck =
		nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE]);
	request->wdev = wdev;
	request->wiphy = wiphy;
	request->scan_start = jiffies;

	ret = __wlan_hdd_cfg80211_scan(wiphy, request, VENDOR_SCAN);
	if (0 != ret) {
		hdd_err("Scan Failed. Ret = %d", ret);
		qdf_mem_free(request);
		return ret;
	}
	ret = wlan_hdd_send_scan_start_event(wiphy, wdev, (uintptr_t)request);

	return ret;
error:
	hdd_err("Scan Request Failed");
	qdf_mem_free(request);
	return -EINVAL;
}

/**
 * wlan_hdd_cfg80211_vendor_scan() -API to process venor scan request
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * This is called from userspace to request scan.
 *
 * Return: Return the Success or Failure code.
 */
int wlan_hdd_cfg80211_vendor_scan(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data,
		int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_vendor_scan(wiphy, wdev,
					      data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
/* Below code will be removed once common scan module is available.*/
#ifndef NAPIER_SCAN
/**
 * wlan_hdd_get_scanid() - API to get the scan id
 * from the scan cookie attribute.
 * @hdd_ctx: Pointer to HDD context
 * @scan_id: Pointer to scan id
 * @cookie : Scan cookie attribute
 *
 * API to get the scan id from the scan cookie attribute
 * sent from supplicant by matching scan request.
 *
 * Return: 0 for success, non zero for failure
 */
static int wlan_hdd_get_scanid(hdd_context_t *hdd_ctx,
			       uint32_t *scan_id, uint64_t cookie)
{
	struct hdd_scan_req *scan_req;
	qdf_list_node_t *node = NULL;
	qdf_list_node_t *ptr_node = NULL;
	int ret = -EINVAL;

	qdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);
	if (qdf_list_empty(&hdd_ctx->hdd_scan_req_q)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		return ret;
	}

	if (QDF_STATUS_SUCCESS !=
	    qdf_list_peek_front(&hdd_ctx->hdd_scan_req_q,
	    &ptr_node)) {
		qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		return ret;
	}

	do {
		node = ptr_node;
		scan_req = container_of(node, struct hdd_scan_req, node);

		if (cookie ==
		    (uintptr_t)(scan_req->scan_request)) {
			*scan_id = scan_req->scan_id;
			ret = 0;
			break;
		}
	} while (QDF_STATUS_SUCCESS ==
		 qdf_list_peek_next(&hdd_ctx->hdd_scan_req_q,
		 node, &ptr_node));

	qdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);

	return ret;

}
#endif
/**
 * __wlan_hdd_vendor_abort_scan() - API to process vendor command for
 * abort scan
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * API to process vendor abort scan
 *
 * Return: zero for success and non zero for failure
 */
static int __wlan_hdd_vendor_abort_scan(
		struct wiphy *wiphy, const void *data,
		int data_len)
{
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
#ifndef NAPIER_SCAN
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_SCAN_MAX + 1];
	uint32_t scan_id;
	uint64_t cookie;
#endif
	int ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

/* Below code will be removed once common scan module is available.*/
#ifndef NAPIER_SCAN
	ret = -EINVAL;
	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_SCAN_MAX, data,
	    data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return ret;
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE]) {
		cookie = nla_get_u64(
			    tb[QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE]);
		ret = wlan_hdd_get_scanid(hdd_ctx,
					  &scan_id,
					  cookie);
		if (ret != 0)
			return ret;
		wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
				HDD_SESSION_ID_INVALID, scan_id, false);
	}
#else
	wlan_vendor_abort_scan(hdd_ctx->hdd_pdev, data,
				data_len);
#endif
	return ret;
}


/**
 * wlan_hdd_vendor_abort_scan() - API to process vendor command for
 * abort scan
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to net device
 * @data : Pointer to the data
 * @data_len : length of the data
 *
 * This is called from supplicant to abort scan
 *
 * Return: zero for success and non zero for failure
 */
int wlan_hdd_vendor_abort_scan(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_vendor_abort_scan(wiphy,
					   data,
					   data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_scan_abort() - abort ongoing scan
 * @pAdapter: Pointer to interface adapter
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_scan_abort(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_scaninfo_t *pScanInfo = NULL;

	pScanInfo = &pAdapter->scan_info;

	if (pScanInfo->mScanPending)
		wlan_abort_scan(pHddCtx->hdd_pdev, INVAL_PDEV_ID,
				pAdapter->sessionId, INVALID_SCAN_ID, true);
	return 0;
}

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * __wlan_hdd_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_sched_scan_start(struct wiphy *wiphy,
						struct net_device *dev,
						struct
						cfg80211_sched_scan_request
						*request)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	tHalHandle hHal;
	int ret = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(pAdapter->sessionId)) {
		hdd_err("invalid session id: %d", pAdapter->sessionId);
		return -EINVAL;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);

	if (0 != ret)
		return ret;

	if (!pHddCtx->config->PnoOffload) {
		hdd_debug("PnoOffloadis not enabled!!!");
		return -EINVAL;
	}

	if (!sme_is_session_id_valid(pHddCtx->hHal, pAdapter->sessionId))
		return -EINVAL;

	hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	if (NULL == hHal) {
		hdd_err("HAL context  is Null!!!");
		return -EINVAL;
	}

	return wlan_cfg80211_sched_scan_start(pHddCtx->hdd_pdev, dev, request);
}

/**
 * wlan_hdd_cfg80211_sched_scan_start() - cfg80211 scheduled scan(pno) start
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 * @request: Pointer to cfg80211 scheduled scan start request
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_cfg80211_sched_scan_start(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_sched_scan_request
				       *request)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_start(wiphy, dev, request);
	cds_ssr_unprotect(__func__);

	return ret;
}

int wlan_hdd_sched_scan_stop(struct net_device *dev)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	tHalHandle hHal;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (NULL == hdd_ctx) {
		hdd_err("HDD context is Null");
		return -EINVAL;
	}
	if (!hdd_ctx->config->PnoOffload) {
		hdd_debug("PnoOffloadis not enabled!!!");
		return -EINVAL;
	}

	hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (NULL == hHal) {
		hdd_err(" HAL context  is Null!!!");
		return -EINVAL;
	}

	return wlan_cfg80211_sched_scan_stop(hdd_ctx->hdd_pdev, dev);
}

/**
 * __wlan_hdd_cfg80211_sched_scan_stop() - stop cfg80211 scheduled scan(pno)
 * @dev: Pointer network device
 *
 * This is a wrapper around wlan_hdd_sched_scan_stop() that returns success
 * in the event that the driver is currently recovering or unloading. This
 * prevents a race condition where we get a scan stop from kernel during
 * a driver unload from PLD.
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_sched_scan_stop(struct net_device *dev)
{
	int err;

	ENTER_DEV(dev);

	/* The return 0 is intentional when Recovery and Load/Unload in
	 * progress. We did observe a crash due to a return of
	 * failure in sched_scan_stop , especially for a case where the unload
	 * of the happens at the same time. The function __cfg80211_stop_sched_scan
	 * was clearing rdev->sched_scan_req only when the sched_scan_stop returns
	 * success. If it returns a failure , then its next invocation due to the
	 * clean up of the second interface will have the dev pointer corresponding
	 * to the first one leading to a crash.
	 */
	if (cds_is_driver_recovering()) {
		hdd_info("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return 0;
	}

	if (cds_is_load_or_unload_in_progress()) {
		hdd_info("Unload/Load in Progress, state: 0x%x.  Ignore!!!",
			cds_get_driver_state());
		return 0;
	}

	err = wlan_hdd_sched_scan_stop(dev);

	EXIT();
	return err;
}

int wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
				      struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_stop(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /*FEATURE_WLAN_SCAN_PNO */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0)) || \
    defined(CFG80211_ABORT_SCAN)
/**
 * __wlan_hdd_cfg80211_abort_scan() - cfg80211 abort scan api
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wireless device structure
 *
 * This function is used to abort an ongoing scan
 *
 * Return: None
 */
static void __wlan_hdd_cfg80211_abort_scan(struct wiphy *wiphy,
					   struct wireless_dev *wdev)
{
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	ENTER_DEV(dev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (!ret)
		return;

#ifndef NAPIER_SCAN
	wlan_hdd_scan_abort(adapter);
#else
	wlan_cfg80211_abort_scan(hdd_ctx->hdd_pdev);
#endif
	EXIT();
}

/**
 * wlan_hdd_cfg80211_abort_scan - cfg80211 abort scan api
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wireless device structure
 *
 * Wrapper to __wlan_hdd_cfg80211_abort_scan() -
 * function is used to abort an ongoing scan
 *
 * Return: None
 */
void wlan_hdd_cfg80211_abort_scan(struct wiphy *wiphy,
				  struct wireless_dev *wdev)
{
	cds_ssr_protect(__func__);
	__wlan_hdd_cfg80211_abort_scan(wiphy, wdev);
	cds_ssr_unprotect(__func__);
}
#endif

/**
 * hdd_scan_context_destroy() - Destroy scan context
 * @hdd_ctx:	HDD context.
 *
 * Destroy scan context.
 *
 * Return: None.
 */
void hdd_scan_context_destroy(hdd_context_t *hdd_ctx)
{
#ifndef NAPIER_SCAN
	qdf_list_destroy(&hdd_ctx->hdd_scan_req_q);
	qdf_spinlock_destroy(&hdd_ctx->hdd_scan_req_q_lock);
#endif
	qdf_spinlock_destroy(&hdd_ctx->sched_scan_lock);
}

/**
 * hdd_scan_context_init() - Initialize scan context
 * @hdd_ctx:	HDD context.
 *
 * Initialize scan related resources like spin lock and lists.
 *
 * Return: 0 on success and errno on failure.
 */
int hdd_scan_context_init(hdd_context_t *hdd_ctx)
{
	qdf_spinlock_create(&hdd_ctx->sched_scan_lock);
#ifndef NAPIER_SCAN
	qdf_spinlock_create(&hdd_ctx->hdd_scan_req_q_lock);
	qdf_list_create(&hdd_ctx->hdd_scan_req_q, CFG_MAX_SCAN_COUNT_MAX);
#endif
	return 0;
}
