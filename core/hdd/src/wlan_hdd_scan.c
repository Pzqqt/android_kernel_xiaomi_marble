/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
#include "cds_concurrency.h"
#include "wma_api.h"

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
	cdf_mem_zero(&event, sizeof(struct iw_event));
	event.cmd = cmd;
	event.u.data.flags = 1;
	event.u.data.length = length;
	*current_event =
		iwe_stream_add_point(pscanInfo->info, *current_event,
				     pscanInfo->end, &event, data);

	if (*last_event == *current_event) {
		/* no space to add event */
		hddLog(LOGE, "%s: no space left to add event", __func__);
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

	hddLog(LOG1, "hdd_indicate_scan_result " MAC_ADDRESS_STR,
	       MAC_ADDR_ARRAY(descriptor->bssId));

	error = 0;
	last_event = current_event;
	cdf_mem_zero(&event, sizeof(event));

	/* BSSID */
	event.cmd = SIOCGIWAP;
	event.u.ap_addr.sa_family = ARPHRD_ETHER;
	cdf_mem_copy(event.u.ap_addr.sa_data, descriptor->bssId,
		     sizeof(descriptor->bssId));
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_ADDR_LEN);

	if (last_event == current_event) {
		/* no space to add event */
		/* Error code may be E2BIG */
		hddLog(LOGE, "hdd_indicate_scan_result: no space for SIOCGIWAP ");
		return -E2BIG;
	}

	last_event = current_event;
	cdf_mem_zero(&event, sizeof(struct iw_event));

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
		hddLog(LOGW, "%s: Unknown network type [%d]",
		       __func__, descriptor->nwType);
		modestr = "?";
		break;
	}
	snprintf(event.u.name, IFNAMSIZ, "IEEE 802.11%s", modestr);
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_CHAR_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hddLog(LOGE,
		       "hdd_indicate_scan_result: no space for SIOCGIWNAME");
		/* Error code, may be E2BIG */
		return -E2BIG;
	}

	last_event = current_event;
	cdf_mem_zero(&event, sizeof(struct iw_event));

	/*Freq */
	event.cmd = SIOCGIWFREQ;

	event.u.freq.m = descriptor->channelId;
	event.u.freq.e = 0;
	event.u.freq.i = 0;
	current_event = iwe_stream_add_event(scanInfo->info, current_event, end,
					     &event, IW_EV_FREQ_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hddLog(LOGE,
		       "hdd_indicate_scan_result: no space for SIOCGIWFREQ");
		return -E2BIG;
	}

	last_event = current_event;
	cdf_mem_zero(&event, sizeof(struct iw_event));

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
		hddLog(LOGE,
		       "hdd_indicate_scan_result: no space for SIOCGIWMODE");
		return -E2BIG;
	}
	/* To extract SSID */
	ie_length = GET_IE_LEN_IN_BSS(descriptor->length);

	if (ie_length > 0) {
		/* dot11BeaconIEs is a large struct, so we make it static to
		   avoid stack overflow.  This API is only invoked via ioctl,
		   so it is serialized by the kernel rtnl_lock and hence does
		   not need to be reentrant */
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
					  ie_length, &dot11BeaconIEs);

		pDot11SSID = &dot11BeaconIEs.SSID;

		if (pDot11SSID->present) {
			last_event = current_event;
			cdf_mem_zero(&event, sizeof(struct iw_event));

			event.cmd = SIOCGIWESSID;
			event.u.data.flags = 1;
			event.u.data.length = scan_result->ssId.length;
			current_event =
				iwe_stream_add_point(scanInfo->info, current_event,
						     end, &event,
						     (char *)scan_result->ssId.
						     ssId);

			if (last_event == current_event) {      /* no space to add event */
				hddLog(LOGE,
				       "hdd_indicate_scan_result: no space for SIOCGIWESSID");
				return -E2BIG;
			}
		}

		if (hdd_get_wparsn_ies
			    ((uint8_t *) descriptor->ieFields, ie_length, &last_event,
			    &current_event, scanInfo) < 0) {
			hddLog(LOGE,
			       "hdd_indicate_scan_result: no space for SIOCGIWESSID");
			return -E2BIG;
		}

		last_event = current_event;
		current_pad = current_event + IW_EV_LCP_LEN;
		cdf_mem_zero(&event, sizeof(struct iw_event));

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

			maxNumRates = CDF_MIN(maxNumRates, IW_MAX_BITRATES);

			if ((maxNumRates - numBasicRates) > MAX_RATES) {
				no_of_rates = MAX_RATES;
				hddLog(LOGW,
				       "Accessing array out of bound that array is pDot11ExtSuppRates->rates ");
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
				hddLog(LOGE,
				       "hdd_indicate_scan_result: no space for SIOCGIWRATE");
				return -E2BIG;
			}
		}

		last_event = current_event;
		cdf_mem_zero(&event, sizeof(struct iw_event));

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
			hddLog(LOGE,
			       "hdd_indicate_scan_result: no space for SIOCGIWENCODE");
			return -E2BIG;
		}
	}

	last_event = current_event;
	cdf_mem_zero(&event, sizeof(struct iw_event));

	/* RSSI */
	event.cmd = IWEVQUAL;
	event.u.qual.qual = descriptor->rssi;
	event.u.qual.noise = descriptor->sinr;
	event.u.qual.level = CDF_MIN((descriptor->rssi + descriptor->sinr), 0);

	event.u.qual.updated = IW_QUAL_ALL_UPDATED;

	current_event = iwe_stream_add_event(scanInfo->info, current_event,
					     end, &event, IW_EV_QUAL_LEN);

	if (last_event == current_event) {      /* no space to add event */
		hddLog(LOGE, "hdd_indicate_scan_result: no space for IWEVQUAL");
		return -E2BIG;
	}

	/* AGE */
	event.cmd = IWEVCUSTOM;
	p = custom;
	p += scnprintf(p, MAX_CUSTOM_LEN, " Age: %lu",
		       cdf_mc_timer_get_system_ticks() -
		       descriptor->nReceivedTime);
	event.u.data.length = p - custom;
	current_event = iwe_stream_add_point(scanInfo->info, current_event, end,
					     &event, custom);
	if (last_event == current_event) {      /* no space to add event */
		hddLog(LOGE,
		       "hdd_indicate_scan_result: no space for IWEVCUSTOM (age)");
		return -E2BIG;
	}

	scanInfo->start = current_event;

	return 0;
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
	CDF_STATUS status;

	ENTER();
	hdd_scan_req = cdf_mem_malloc(sizeof(*hdd_scan_req));
	if (NULL == hdd_scan_req) {
		hddLog(LOGP, FL("malloc failed for Scan req"));
		return -ENOMEM;
	}

	hdd_scan_req->adapter = adapter;
	hdd_scan_req->scan_request = scan_req;
	hdd_scan_req->source = source;
	hdd_scan_req->scan_id = scan_id;
	hdd_scan_req->timestamp = timestamp;

	cdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);
	status = cdf_list_insert_back(&hdd_ctx->hdd_scan_req_q,
					&hdd_scan_req->node);
	cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);

	if (CDF_STATUS_SUCCESS != status) {
		hdd_err("Failed to enqueue Scan Req");
		cdf_mem_free(hdd_scan_req);
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
 * Return: CDF_STATUS
 */
CDF_STATUS wlan_hdd_scan_request_dequeue(hdd_context_t *hdd_ctx,
	uint32_t scan_id, struct cfg80211_scan_request **req, uint8_t *source,
	uint32_t *timestamp)
{
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	struct hdd_scan_req *hdd_scan_req;
	cdf_list_node_t *pNode = NULL, *ppNode = NULL;

	hdd_info("Dequeue Scan id: %d", scan_id);

	if ((source == NULL) && (timestamp == NULL) && (req == NULL))
		return CDF_STATUS_E_NULL_VALUE;

	cdf_spin_lock(&hdd_ctx->hdd_scan_req_q_lock);

	if (list_empty(&hdd_ctx->hdd_scan_req_q.anchor)) {
		cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		return CDF_STATUS_E_FAILURE;
	}

	if (CDF_STATUS_SUCCESS !=
		cdf_list_peek_front(&hdd_ctx->hdd_scan_req_q, &ppNode)) {
		cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
		hdd_err("Failed to remove Scan Req from queue");
		return CDF_STATUS_E_FAILURE;
	}

	do {
		pNode = ppNode;
		hdd_scan_req = (struct hdd_scan_req *)pNode;
		if (hdd_scan_req->scan_id == scan_id) {
			status = cdf_list_remove_node(&hdd_ctx->hdd_scan_req_q,
					pNode);
			if (status == CDF_STATUS_SUCCESS) {
				*req = hdd_scan_req->scan_request;
				*source = hdd_scan_req->source;
				*timestamp = hdd_scan_req->timestamp;
				cdf_mem_free(hdd_scan_req);
				cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
				hdd_info("removed Scan id: %d, req = %p",
					scan_id, req);
				return CDF_STATUS_SUCCESS;
			} else {
				cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
				hdd_err("Failed to remove node scan id %d",
					scan_id);
				return status;
			}
		}
	} while (CDF_STATUS_SUCCESS ==
		cdf_list_peek_next(&hdd_ctx->hdd_scan_req_q, pNode, &ppNode));

	cdf_spin_unlock(&hdd_ctx->hdd_scan_req_q_lock);
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

static CDF_STATUS
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
	uint32_t size = 0;

	ENTER();
	hddLog(LOGW,
	       "%s called with halHandle = %p, pContext = %p, scanID = %d,"
	       " returned status = %d", __func__, halHandle, pContext,
	       (int)scanId, (int)status);

	/* if there is a scan request pending when the wlan driver is unloaded
	 * we may be invoked as SME flushes its pending queue.  If that is the
	 * case, the underlying net_device may have already been destroyed, so
	 * do some quick sanity before proceeding
	 */
	if (pAdapter->dev != dev) {
		hddLog(LOGW, "%s: device mismatch %p vs %p",
		       __func__, pAdapter->dev, dev);
		return CDF_STATUS_SUCCESS;
	}

	wlan_hdd_scan_request_dequeue(hddctx, scanId, &req, &source,
		&timestamp);

	if (req != NULL)
		hdd_err("Got unexpected request struct for Scan id %d",
			scanId);

	cdf_spin_lock(&hddctx->hdd_scan_req_q_lock);
	cdf_list_size(&(hddctx->hdd_scan_req_q), &size);
	if (!size)
		/* Scan is no longer pending */
		pAdapter->scan_info.mScanPending = false;
	cdf_spin_unlock(&hddctx->hdd_scan_req_q_lock);

	/* notify any applications that may be interested */
	memset(&wrqu, '\0', sizeof(wrqu));
	we_event = SIOCGIWSCAN;
	msg = NULL;
	wireless_send_event(dev, we_event, &wrqu, msg);

	EXIT();
	return CDF_STATUS_SUCCESS;
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
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	struct iw_scan_req *scanReq = (struct iw_scan_req *)extra;
	hdd_adapter_t *con_sap_adapter;
	uint16_t con_dfs_ch;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* Block All Scan during DFS operation and send null scan result */
	con_sap_adapter = hdd_get_con_sap_adapter(pAdapter, true);
	if (con_sap_adapter) {
		con_dfs_ch = con_sap_adapter->sessionCtx.ap.operatingChannel;

		if (CDS_IS_DFS_CH(con_dfs_ch)) {
			hddLog(LOGW, FL("##In DFS Master mode. Scan aborted"));
			return -EOPNOTSUPP;
		}
	}

	cdf_mem_zero(&scanRequest, sizeof(scanRequest));

	if (NULL != wrqu->data.pointer) {
		/* set scanType, active or passive */
		if ((IW_SCAN_TYPE_ACTIVE == scanReq->scan_type) ||
		    (eSIR_ACTIVE_SCAN == hdd_ctx->ioctl_scan_mode)) {
			scanRequest.scanType = eSIR_ACTIVE_SCAN;
		} else {
			scanRequest.scanType = eSIR_PASSIVE_SCAN;
		}

		/* set bssid using sockaddr from iw_scan_req */
		cdf_mem_copy(scanRequest.bssid.bytes,
			     &scanReq->bssid.sa_data,
			     CDF_MAC_ADDR_SIZE);

		if (wrqu->data.flags & IW_SCAN_THIS_ESSID) {

			if (scanReq->essid_len) {
				scanRequest.SSIDs.numOfSSIDs = 1;
				scanRequest.SSIDs.SSIDList =
					(tCsrSSIDInfo *)
					cdf_mem_malloc(sizeof(tCsrSSIDInfo));
				if (scanRequest.SSIDs.SSIDList) {
					scanRequest.SSIDs.SSIDList->SSID.
					length = scanReq->essid_len;
					cdf_mem_copy(scanRequest.SSIDs.
						     SSIDList->SSID.ssId,
						     scanReq->essid,
						     scanReq->essid_len);
				} else {
					scanRequest.SSIDs.numOfSSIDs = 0;
					CDF_TRACE(CDF_MODULE_ID_HDD,
						  CDF_TRACE_LEVEL_ERROR,
						  "%s: Unable to allocate memory",
						  __func__);
					CDF_ASSERT(0);
				}
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

		cdf_set_macaddr_broadcast(&scanRequest.bssid);

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
	if (pAdapter->scan_info.scanAddIE.addIEdata &&
	    pAdapter->scan_info.scanAddIE.length) {
		scanRequest.uIEFieldLen = pAdapter->scan_info.scanAddIE.length;
		scanRequest.pIEField = pAdapter->scan_info.scanAddIE.addIEdata;
	}
	scanRequest.timestamp = cdf_mc_timer_get_system_ticks();
	status = sme_scan_request((WLAN_HDD_GET_CTX(pAdapter))->hHal,
				  pAdapter->sessionId, &scanRequest,
				  &hdd_scan_request_callback, dev);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_FATAL,
			  "%s:sme_scan_request  fail %d!!!", __func__, status);
		goto error;
	}

	wlan_hdd_scan_request_enqueue(pAdapter, NULL, NL_SCAN,
			scanRequest.scan_id,
			scanRequest.timestamp);

	pAdapter->scan_info.mScanPending = true;
error:
	if ((wrqu->data.flags & IW_SCAN_THIS_ESSID) && (scanReq->essid_len))
		cdf_mem_free(scanRequest.SSIDs.SSIDList);
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
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	hdd_scan_info_t scanInfo;
	tScanResultHandle pResult;
	int i = 0;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO,
		  "%s: enter buffer length %d!!!", __func__,
		  (wrqu->data.length) ? wrqu->data.length : IW_SCAN_MAX_DATA);

	if (true == pAdapter->scan_info.mScanPending) {
		CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_FATAL,
			  "%s:mScanPending is true !!!", __func__);
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
		hddLog(LOG1, "iw_get_scan: NULL Scan Result ");
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

	sme_scan_result_purge(hHal, pResult);

	CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO,
		  "%s: exit total %d BSS reported !!!", __func__, i);
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

/**
 * hdd_abort_mac_scan() - aborts ongoing mac scan
 * @pHddCtx: Pointer to hdd context
 * @sessionId: session id
 * @reason: abort reason
 *
 * Abort any MAC scan if in progress
 *
 * Return: none
 */
void hdd_abort_mac_scan(hdd_context_t *pHddCtx, uint8_t sessionId,
			eCsrAbortReason reason)
{
	sme_abort_mac_scan(pHddCtx->hHal, sessionId, reason);
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
		return;
	}
	skb = cfg80211_vendor_event_alloc(hddctx->wiphy, NULL,
			SCAN_DONE_EVENT_BUF_SIZE + 4 + NLMSG_HDRLEN,
			QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
			GFP_KERNEL);

	if (!skb) {
		hdd_err("skb alloc failed");
		return;
	}

	if (0 != hdd_vendor_put_ifindex(skb, adapter->dev->ifindex))
		goto nla_put_failure;

	cookie = (uintptr_t)req;

	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_ssids; i++) {
		if (nla_put(skb, i, req->ssids[i].ssid_len, req->ssids[i].ssid))
			goto nla_put_failure;
	}
	nla_nest_end(skb, attr);

	attr = nla_nest_start(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES);
	if (!attr)
		goto nla_put_failure;
	for (i = 0; i < req->n_channels; i++) {
		if (nla_put_u32(skb, i, req->channels[i]->center_freq))
			goto nla_put_failure;
	}
	nla_nest_end(skb, attr);

	if (req->ie &&
		nla_put(skb, QCA_WLAN_VENDOR_ATTR_SCAN_IE, req->ie_len,
			req->ie))
		goto nla_put_failure;

	if (req->flags &&
		nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS, req->flags))
		goto nla_put_failure;

	if (nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE, cookie))
		goto nla_put_failure;

	scan_status = (aborted == true) ? VENDOR_SCAN_STATUS_ABORTED :
		VENDOR_SCAN_STATUS_NEW_RESULTS;
	if (nla_put_u8(skb, QCA_WLAN_VENDOR_ATTR_SCAN_STATUS, scan_status))
		goto nla_put_failure;

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return;

nla_put_failure:
	kfree_skb(skb);
	return;
}

/**
 * hdd_cfg80211_scan_done_callback() - scan done callback function called after
 *				       scan is finished
 * @halHandle: Pointer to handle
 * @pContext: Pointer to context
 * @sessionId: Session Id
 * @scanId: Scan Id
 * @status: Scan status
 *
 * Return: CDF status
 */
static CDF_STATUS hdd_cfg80211_scan_done_callback(tHalHandle halHandle,
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
	uint8_t source;
	uint32_t scan_time;
	uint32_t size = 0;

	ret = wlan_hdd_validate_context(hddctx);
	if (0 != ret)
		return CDF_STATUS_E_INVAL;

	hddLog(CDF_TRACE_LEVEL_INFO,
		"%s called with hal = %p, pContext = %p, ID = %d, status = %d",
		__func__, halHandle, pContext, (int)scanId, (int)status);

	pScanInfo->mScanPendingCounter = 0;

	if (pScanInfo->mScanPending != true) {
		CDF_ASSERT(pScanInfo->mScanPending);
		goto allow_suspend;
	}

	if (CDF_STATUS_SUCCESS !=
		wlan_hdd_scan_request_dequeue(hddctx, scanId, &req, &source,
			&scan_time)) {
		hdd_err("Dequeue of scan request failed ID: %d", scanId);
		goto allow_suspend;
	}

	ret = wlan_hdd_cfg80211_update_bss((WLAN_HDD_GET_CTX(pAdapter))->wiphy,
					   pAdapter, scan_time);
	if (0 > ret)
		hddLog(CDF_TRACE_LEVEL_INFO, "%s: NO SCAN result", __func__);

	/*
	 * cfg80211_scan_done informing NL80211 about completion
	 * of scanning
	 */
	if (status == eCSR_SCAN_ABORT || status == eCSR_SCAN_FAILURE) {
		aborted = true;
	}

	cdf_spin_lock(&hddctx->hdd_scan_req_q_lock);
	cdf_list_size(&(hddctx->hdd_scan_req_q), &size);
	if (!size) {
		/* Scan is no longer pending */
		pScanInfo->mScanPending = false;
		complete(&pScanInfo->abortscan_event_var);
	}
	cdf_spin_unlock(&hddctx->hdd_scan_req_q_lock);
	/*
	 * Scan can be triggred from NL or vendor scan
	 * - If scan is triggered from NL then cfg80211 scan done should be
	 * called to updated scan completion to NL.
	 * - If scan is triggred through vendor command then
	 * scan done event will be posted
	 */
	if (NL_SCAN == source)
		cfg80211_scan_done(req, aborted);
	else
		hdd_vendor_scan_callback(pAdapter, req, aborted);

allow_suspend:
	if (!size) {
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
	}

#ifdef FEATURE_WLAN_TDLS
	wlan_hdd_tdls_scan_done_callback(pAdapter);
#endif

	EXIT();
	return 0;
}


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
		hddLog(LOGE,
			"%s: HDD adapter context is invalid", __func__);
		return;
	}

	request = adapter->request;
	if (request) {
		request->n_ssids = 0;
		request->n_channels = 0;

		hddLog(LOGE,
		   FL("##In DFS Master mode. Scan aborted. Null result sent"));
		cfg80211_scan_done(request, true);
		adapter->request = NULL;
	}
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
				    struct net_device *dev,
#endif
				    struct cfg80211_scan_request *request,
				    uint8_t source)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	struct net_device *dev = request->wdev->netdev;
#endif
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	struct hdd_config *cfg_param = NULL;
	tCsrScanRequest scan_req;
	uint8_t *channelList = NULL, i;
	int status;
	hdd_scaninfo_t *pScanInfo = NULL;
	uint8_t *pP2pIe = NULL;
	hdd_adapter_t *con_sap_adapter;
	uint16_t con_dfs_ch;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SCAN,
			 pAdapter->sessionId, request->n_channels));

	hddLog(LOG1, FL("Device_mode %s(%d)"),
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);

	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	cfg_param = pHddCtx->config;
	pScanInfo = &pAdapter->scan_info;

	/* Block All Scan during DFS operation and send null scan result */
	con_sap_adapter = hdd_get_con_sap_adapter(pAdapter, true);
	if (con_sap_adapter) {
		con_dfs_ch = con_sap_adapter->sessionCtx.ap.sapConfig.channel;
		if (con_dfs_ch == AUTO_CHANNEL_SELECT)
			con_dfs_ch =
				con_sap_adapter->sessionCtx.ap.operatingChannel;

		if (CDS_IS_DFS_CH(con_dfs_ch)) {
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
			pAdapter->request = request;

#ifdef CONFIG_CNSS
			cnss_init_work(&pAdapter->scan_block_work,
				       wlan_hdd_cfg80211_scan_block_cb);
#else
			INIT_WORK(&pAdapter->scan_block_work,
				  wlan_hdd_cfg80211_scan_block_cb);
#endif
			schedule_work(&pAdapter->scan_block_work);
			return 0;
		}
	}
	if (!wma_is_hw_dbs_capable()) {
		if (true == pScanInfo->mScanPending) {
			if (MAX_PENDING_LOG >
				pScanInfo->mScanPendingCounter++) {
				hddLog(LOGE, FL("mScanPending is true"));
			}
			return -EBUSY;
		}

		/* Don't Allow Scan and return busy if Remain On
		 * Channel and action frame is pending
		 * Otherwise Cancel Remain On Channel and allow Scan
		 * If no action frame pending
		 */
		if (0 != wlan_hdd_check_remain_on_channel(pAdapter)) {
			hddLog(LOGE, FL("Remain On Channel Pending"));
			return -EBUSY;
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
						dev,
#endif
					request);
	if (status <= 0) {
		if (!status)
			hddLog(LOGE,
			FL("TDLS in progress.scan rejected %d"),
			status);
		else
			hddLog(LOGE, FL("TDLS teardown is ongoing %d"),
			       status);
		return status;
	}
#endif

	/* Check if scan is allowed at this point of time */
	if (cds_is_connection_in_progress(pHddCtx)) {
		hddLog(LOGE, FL("Scan not allowed"));
		return -EBUSY;
	}

	cdf_mem_zero(&scan_req, sizeof(scan_req));

	hddLog(LOG1, "scan request for ssid = %d", request->n_ssids);
	scan_req.timestamp = cdf_mc_timer_get_system_ticks();

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
			cdf_mem_malloc(request->n_ssids * sizeof(tCsrSSIDInfo));

		if (NULL == scan_req.SSIDs.SSIDList) {
			hddLog(LOGE, FL("memory alloc failed SSIDInfo buffer"));
			return -ENOMEM;
		}

		/* copy all the ssid's and their length */
		for (j = 0; j < request->n_ssids; j++, SsidInfo++) {
			/* get the ssid length */
			SsidInfo->SSID.length = request->ssids[j].ssid_len;
			cdf_mem_copy(SsidInfo->SSID.ssId,
				     &request->ssids[j].ssid[0],
				     SsidInfo->SSID.length);
			SsidInfo->SSID.ssId[SsidInfo->SSID.length] = '\0';
			hddLog(LOG1, FL("SSID number %d: %s"), j,
				SsidInfo->SSID.ssId);
		}
		/* set the scan type to active */
		scan_req.scanType = eSIR_ACTIVE_SCAN;
	} else if (WLAN_HDD_P2P_GO == pAdapter->device_mode) {
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
	scan_req.minChnTime = cfg_param->nActiveMinChnTime;
	scan_req.maxChnTime = cfg_param->nActiveMaxChnTime;

	/* set BSSType to default type */
	scan_req.BSSType = eCSR_BSS_TYPE_ANY;

	if (MAX_CHANNEL < request->n_channels) {
		hddLog(LOGW, FL("No of Scan Channels exceeded limit: %d"),
		       request->n_channels);
		request->n_channels = MAX_CHANNEL;
	}

	hddLog(LOG1, FL("No of Scan Channels: %d"), request->n_channels);

	if (request->n_channels) {
		char chList[(request->n_channels * 5) + 1];
		int len;
		channelList = cdf_mem_malloc(request->n_channels);
		if (NULL == channelList) {
			hddLog(LOGE,
			       FL("channelList malloc failed channelList"));
			status = -ENOMEM;
			goto free_mem;
		}
		for (i = 0, len = 0; i < request->n_channels; i++) {
			channelList[i] = request->channels[i]->hw_value;
			len += snprintf(chList + len, 5, "%d ", channelList[i]);
		}

		hddLog(LOG1, FL("Channel-List: %s"), chList);

	}
	scan_req.ChannelInfo.numOfChannels = request->n_channels;
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

	if (request->n_channels != WLAN_HDD_P2P_SINGLE_CHANNEL_SCAN) {
		hddLog(CDF_TRACE_LEVEL_DEBUG, "Flushing P2P Results");
		sme_scan_flush_p2p_result(WLAN_HDD_GET_HAL_CTX(pAdapter),
					   pAdapter->sessionId);
	}

	if (request->ie_len) {
		/* save this for future association (join requires this) */
		memset(&pScanInfo->scanAddIE, 0, sizeof(pScanInfo->scanAddIE));
		memcpy(pScanInfo->scanAddIE.addIEdata, request->ie,
		       request->ie_len);
		pScanInfo->scanAddIE.length = request->ie_len;

		if ((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
		    (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) ||
		    (WLAN_HDD_P2P_DEVICE == pAdapter->device_mode)
		    ) {
			pwextBuf->roamProfile.pAddIEScan =
				pScanInfo->scanAddIE.addIEdata;
			pwextBuf->roamProfile.nAddIEScanLength =
				pScanInfo->scanAddIE.length;
		}

		scan_req.uIEFieldLen = pScanInfo->scanAddIE.length;
		scan_req.pIEField = pScanInfo->scanAddIE.addIEdata;

		pP2pIe = wlan_hdd_get_p2p_ie_ptr((uint8_t *) request->ie,
						 request->ie_len);
		if (pP2pIe != NULL) {
#ifdef WLAN_FEATURE_P2P_DEBUG
			if (((global_p2p_connection_status == P2P_GO_NEG_COMPLETED)
			     || (global_p2p_connection_status ==
				 P2P_GO_NEG_PROCESS))
			    && (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTING_STATE_1;
				hddLog(LOGE,
				       FL("[P2P State] Changing state from Go nego completed to Connection is started"));
				hddLog(LOGE,
				       FL("[P2P]P2P Scanning is started for 8way Handshake"));
			} else
			if ((global_p2p_connection_status ==
			     P2P_CLIENT_DISCONNECTED_STATE)
			    && (WLAN_HDD_P2P_CLIENT ==
				pAdapter->device_mode)) {
				global_p2p_connection_status =
					P2P_CLIENT_CONNECTING_STATE_2;
				hddLog(LOGE,
				       FL("[P2P State] Changing state from Disconnected state to Connection is started"));
				hddLog(LOGE,
				       FL("[P2P]P2P Scanning is started for 4way Handshake"));
			}
#endif

			/* no_cck will be set during p2p find to disable 11b rates */
			if (request->no_cck) {
				hddLog(CDF_TRACE_LEVEL_INFO,
				       "%s: This is a P2P Search", __func__);
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

	hddLog(LOG2,
	       FL("requestType %d, scanType %d, minChnTime %d, maxChnTime %d,p2pSearch %d, skipDfsChnlIn P2pSearch %d"),
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

	if (CDF_STATUS_SUCCESS != status) {
		hddLog(LOGE, FL("sme_scan_request returned error %d"), status);
		if (CDF_STATUS_E_RESOURCES == status) {
			hddLog(LOGE,
			       FL("HO is in progress.So defer the scan by informing busy"));
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

free_mem:
	if (scan_req.SSIDs.SSIDList)
		cdf_mem_free(scan_req.SSIDs.SSIDList);

	if (channelList)
		cdf_mem_free(channelList);

	EXIT();
	return status;
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
			   struct net_device *dev,
#endif
			   struct cfg80211_scan_request *request)
{
	int ret;
	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_scan(wiphy,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
				       dev,
#endif
				request, NL_SCAN);
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
	enum ieee80211_band band,
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
		hddLog(LOGE, FL(" reply skb alloc failed"));
		return -ENOMEM;
	}

	if (nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE, cookie)) {
		hddLog(LOGE, FL("nla put fail"));
		kfree_skb(skb);
		return -EINVAL;
	}

	ret = cfg80211_vendor_cmd_reply(skb);

	/* Send a scan started event to supplicant */
	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
		sizeof(u64) + 4 + NLMSG_HDRLEN,
		QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX, GFP_KERNEL);
	if (!skb) {
		hddLog(LOGE, FL("skb alloc failed"));
		return -ENOMEM;
	}

	ret = hdd_vendor_put_ifindex(skb, wdev->netdev->ifindex);
	if (ret) {
		kfree_skb(skb);
		return -EINVAL;
	}

	if (nla_put_u64(skb, QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE, cookie)) {
		kfree_skb(skb);
		return -EINVAL;
	}
	cfg80211_vendor_event(skb, GFP_KERNEL);

	return ret;
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
	enum ieee80211_band band;
	uint8_t n_channels = 0, n_ssid = 0, ie_len = 0;
	uint32_t tmp, count, j;
	unsigned int len;
	struct ieee80211_channel *chan;
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	int ret;

	ENTER();

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
		for (band = 0; band < IEEE80211_NUM_BANDS; band++)
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

	request = cdf_mem_malloc(len);
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
		for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
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

	for (count = 0; count < IEEE80211_NUM_BANDS; count++)
		if (wiphy->bands[count])
			request->rates[count] =
				(1 << wiphy->bands[count]->n_bitrates) - 1;

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES]) {
		nla_for_each_nested(attr,
			tb[QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES],
			tmp) {
			band = nla_type(attr);
			if (!wiphy->bands[band])
				continue;
			request->rates[band] = wlan_hdd_get_rates(wiphy,
							band, nla_data(attr),
							nla_len(attr));
		}
	}

	if (tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]) {
		request->flags =
			nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS]);
		if ((request->flags & NL80211_SCAN_FLAG_LOW_PRIORITY) &&
		!(wiphy->features & NL80211_FEATURE_LOW_PRIORITY_SCAN)) {
			hddLog(LOGE, FL("LOW PRIORITY SCAN not supported"));
			goto error;
		}
	}
	request->no_cck =
		nla_get_flag(tb[QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE]);
	request->wdev = wdev;
	request->wiphy = wiphy;
	request->scan_start = jiffies;

	if (0 != __wlan_hdd_cfg80211_scan(wiphy, request, VENDOR_SCAN))
		goto error;

	ret = wlan_hdd_send_scan_start_event(wiphy, wdev, (uintptr_t)request);

	return ret;
error:
	hdd_err("Scan Request Failed");
	cdf_mem_free(request);
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
/**
 * wlan_hdd_scan_abort() - abort ongoing scan
 * @pAdapter: Pointer to interface adapter
 *
 * Return: 0 for success, non zero for failure
 */
#ifdef FEATURE_WLAN_SCAN_PNO
int wlan_hdd_scan_abort(hdd_adapter_t *pAdapter)
{
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_scaninfo_t *pScanInfo = NULL;
	unsigned long rc;

	pScanInfo = &pAdapter->scan_info;

	if (pScanInfo->mScanPending) {
		INIT_COMPLETION(pScanInfo->abortscan_event_var);
		hdd_abort_mac_scan(pHddCtx, pAdapter->sessionId,
				   eCSR_SCAN_ABORT_DEFAULT);

		rc = wait_for_completion_timeout(
			&pScanInfo->abortscan_event_var,
				msecs_to_jiffies(5000));
		if (!rc) {
			hddLog(LOGE,
				FL("Timeout occurred while waiting for abort scan"));
			return -ETIME;
		}
	}
	return 0;
}

/**
 * hdd_sched_scan_callback - scheduled scan callback
 * @callbackContext: Callback context
 * @pPrefNetworkFoundInd: Preferred network found indication
 *
 * This is a callback function that is registerd with SME that is
 * invoked when a preferred network is discovered by firmware.
 *
 * Return: none
 */
static void
hdd_sched_scan_callback(void *callbackContext,
			tSirPrefNetworkFoundInd *pPrefNetworkFoundInd)
{
	int ret;
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) callbackContext;
	hdd_context_t *pHddCtx;

	ENTER();

	if (NULL == pAdapter) {
		hddLog(LOGE, FL("HDD adapter is Null"));
		return;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	if (NULL == pHddCtx) {
		hddLog(LOGE, FL("HDD context is Null!!!"));
		return;
	}

	spin_lock(&pHddCtx->schedScan_lock);
	if (true == pHddCtx->isWiphySuspended) {
		pHddCtx->isSchedScanUpdatePending = true;
		spin_unlock(&pHddCtx->schedScan_lock);
		hddLog(LOG1,
		       FL("Update cfg80211 scan database after it resume"));
		return;
	}
	spin_unlock(&pHddCtx->schedScan_lock);

	ret = wlan_hdd_cfg80211_update_bss(pHddCtx->wiphy, pAdapter, 0);

	if (0 > ret)
		hddLog(LOG1, FL("NO SCAN result"));

	cfg80211_sched_scan_results(pHddCtx->wiphy);
	hddLog(LOG1,
		FL("cfg80211 scan result database updated"));
}

/**
 * wlan_hdd_is_pno_allowed() -  Check if PNO is allowed
 * @adapter: HDD Device Adapter
 *
 * The PNO Start request is coming from upper layers.
 * It is to be allowed only for Infra STA device type
 * and the link should be in a disconnected state.
 *
 * Return: Success if PNO is allowed, Failure otherwise.
 */
static CDF_STATUS wlan_hdd_is_pno_allowed(hdd_adapter_t *adapter)
{
	hddLog(LOG1,
		FL("dev_mode=%d, conn_state=%d, session ID=%d"),
		adapter->device_mode,
		adapter->sessionCtx.station.conn_info.connState,
		adapter->sessionId);
	if ((adapter->device_mode == WLAN_HDD_INFRA_STATION) &&
		(eConnectionState_NotConnected ==
		 adapter->sessionCtx.station.conn_info.connState))
		return CDF_STATUS_SUCCESS;
	else
		return CDF_STATUS_E_FAILURE;

}

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
	tpSirPNOScanReq pPnoRequest = NULL;
	hdd_context_t *pHddCtx;
	tHalHandle hHal;
	uint32_t i, indx, num_ch, tempInterval, j;
	u8 valid_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	u8 channels_allowed[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint32_t num_channels_allowed = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	int ret = 0;
	hdd_scaninfo_t *pScanInfo = &pAdapter->scan_info;
	struct hdd_config *config = NULL;
	uint32_t num_ignore_dfs_ch = 0;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);

	if (0 != ret)
		return ret;

	config = pHddCtx->config;
	hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	if (NULL == hHal) {
		hddLog(LOGE, FL("HAL context  is Null!!!"));
		return -EINVAL;
	}

	if ((WLAN_HDD_INFRA_STATION == pAdapter->device_mode) &&
	    (eConnectionState_Connecting ==
	     (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState)) {
		hddLog(LOGE,
		       FL("%p(%d) Connection in progress: sched_scan_start denied (EBUSY)"),
		       WLAN_HDD_GET_STATION_CTX_PTR(pAdapter),
		       pAdapter->sessionId);
		return -EBUSY;
	}

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SCHED_SCAN_START,
			 pAdapter->sessionId, pAdapter->device_mode));
	/*
	 * The current umac is unable to handle the SCAN_PREEMPT and SCAN_DEQUEUED
	 * so its necessary to terminate the existing scan which is already issued
	 * otherwise the host won't enter into the suspend state due to the reason
	 * that the wlan wakelock which was held in the wlan_hdd_cfg80211_scan
	 * function.
	 */
	sme_scan_flush_result(hHal);
	if (true == pScanInfo->mScanPending) {
		ret = wlan_hdd_scan_abort(pAdapter);
		if (ret < 0) {
			hddLog(LOGE,
				  FL("aborting the existing scan is unsuccessful"));
			return -EBUSY;
		}
	}

	if (CDF_STATUS_E_FAILURE == wlan_hdd_is_pno_allowed(pAdapter)) {
		hddLog(LOGE, FL("pno is not allowed"));
		return -ENOTSUPP;
	}

	pPnoRequest = (tpSirPNOScanReq) cdf_mem_malloc(sizeof(tSirPNOScanReq));
	if (NULL == pPnoRequest) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	memset(pPnoRequest, 0, sizeof(tSirPNOScanReq));
	pPnoRequest->enable = 1;        /*Enable PNO */
	pPnoRequest->ucNetworksCount = request->n_match_sets;

	if ((!pPnoRequest->ucNetworksCount) ||
	    (pPnoRequest->ucNetworksCount > SIR_PNO_MAX_SUPP_NETWORKS)) {
		hddLog(LOGE, FL("Network input is not correct %d"),
			pPnoRequest->ucNetworksCount);
		ret = -EINVAL;
		goto error;
	}

	if (SIR_PNO_MAX_NETW_CHANNELS_EX < request->n_channels) {
		hddLog(LOGE, FL("Incorrect number of channels %d"),
			request->n_channels);
		ret = -EINVAL;
		goto error;
	}

	/* Framework provides one set of channels(all)
	 * common for all saved profile */
	if (0 != sme_cfg_get_str(hHal, WNI_CFG_VALID_CHANNEL_LIST,
				 channels_allowed, &num_channels_allowed)) {
		hddLog(LOGE,
			 FL("failed to get valid channel list"));
		ret = -EINVAL;
		goto error;
	}
	/* Checking each channel against allowed channel list */
	num_ch = 0;
	if (request->n_channels) {
		char chList[(request->n_channels * 5) + 1];
		int len;
		for (i = 0, len = 0; i < request->n_channels; i++) {
			for (indx = 0; indx < num_channels_allowed; indx++) {
				if (request->channels[i]->hw_value ==
				    channels_allowed[indx]) {

					if ((!config->enable_dfs_pno_chnl_scan)
						&& (CHANNEL_STATE_DFS ==
						cds_get_channel_state(
						    channels_allowed[indx]))) {
						CDF_TRACE(CDF_MODULE_ID_HDD,
						    CDF_TRACE_LEVEL_INFO,
						    "%s : Dropping DFS channel : %d",
						     __func__,
						    channels_allowed[indx]);
						num_ignore_dfs_ch++;
						break;
					}

					valid_ch[num_ch++] =
						request->channels[i]->hw_value;
					len +=
						snprintf(chList + len, 5, "%d ",
							 request->channels[i]->
							 hw_value);
					break;
				}
			}
		}
		hddLog(LOG1, FL("Channel-List: %s "), chList);

		/* If all channels are DFS and dropped,
		 * then ignore the PNO request
		 */
		if (num_ignore_dfs_ch == request->n_channels) {
			CDF_TRACE(CDF_MODULE_ID_HDD, CDF_TRACE_LEVEL_INFO,
				"%s : All requested channels are DFS channels",
				 __func__);
			ret = -EINVAL;
			goto error;
		}

	}
	/* Filling per profile  params */
	for (i = 0; i < pPnoRequest->ucNetworksCount; i++) {
		pPnoRequest->aNetworks[i].ssId.length =
			request->match_sets[i].ssid.ssid_len;

		if ((0 == pPnoRequest->aNetworks[i].ssId.length) ||
		    (pPnoRequest->aNetworks[i].ssId.length > 32)) {
			hddLog(LOGE,
				FL(" SSID Len %d is not correct for network %d"),
				  pPnoRequest->aNetworks[i].ssId.length, i);
			ret = -EINVAL;
			goto error;
		}

		memcpy(pPnoRequest->aNetworks[i].ssId.ssId,
		       request->match_sets[i].ssid.ssid,
		       request->match_sets[i].ssid.ssid_len);
		pPnoRequest->aNetworks[i].authentication = 0;   /*eAUTH_TYPE_ANY */
		pPnoRequest->aNetworks[i].encryption = 0;       /*eED_ANY */
		pPnoRequest->aNetworks[i].bcastNetwType = 0;    /*eBCAST_UNKNOWN */

		/*Copying list of valid channel into request */
		memcpy(pPnoRequest->aNetworks[i].aChannels, valid_ch, num_ch);
		pPnoRequest->aNetworks[i].ucChannelCount = num_ch;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
		pPnoRequest->aNetworks[i].rssiThreshold =
			request->match_sets[i].rssi_thold;
#else
		pPnoRequest->aNetworks[i].rssiThreshold = 0;    /* Default value */
#endif
	}

	for (i = 0; i < request->n_ssids; i++) {
		j = 0;
		while (j < pPnoRequest->ucNetworksCount) {
			if ((pPnoRequest->aNetworks[j].ssId.length ==
			     request->ssids[i].ssid_len) &&
			    (0 == memcmp(pPnoRequest->aNetworks[j].ssId.ssId,
					 request->ssids[i].ssid,
					 pPnoRequest->aNetworks[j].ssId.
					 length))) {
				pPnoRequest->aNetworks[j].bcastNetwType =
					eBCAST_HIDDEN;
				break;
			}
			j++;
		}
	}
	hddLog(LOG1, FL("Number of hidden networks being Configured = %d"),
		  request->n_ssids);

	hddLog(LOG1, FL("request->ie_len = %zu"), request->ie_len);
	if ((0 < request->ie_len) && (NULL != request->ie)) {
		pPnoRequest->us24GProbeTemplateLen = request->ie_len;
		memcpy(&pPnoRequest->p24GProbeTemplate, request->ie,
		       pPnoRequest->us24GProbeTemplateLen);

		pPnoRequest->us5GProbeTemplateLen = request->ie_len;
		memcpy(&pPnoRequest->p5GProbeTemplate, request->ie,
		       pPnoRequest->us5GProbeTemplateLen);
	}

	/* Driver gets only one time interval which is hardcoded in
	 * supplicant for 10000ms. Taking power consumption into account 6 timers
	 * will be used, Timervalue is increased exponentially i.e 10,20,40,
	 * 80,160,320 secs. And number of scan cycle for each timer
	 * is configurable through INI param gPNOScanTimerRepeatValue.
	 * If it is set to 0 only one timer will be used and PNO scan cycle
	 * will be repeated after each interval specified by supplicant
	 * till PNO is disabled.
	 */
	if (0 == pHddCtx->config->configPNOScanTimerRepeatValue)
		pPnoRequest->scanTimers.ucScanTimersCount =
			HDD_PNO_SCAN_TIMERS_SET_ONE;
	else
		pPnoRequest->scanTimers.ucScanTimersCount =
			HDD_PNO_SCAN_TIMERS_SET_MULTIPLE;

	tempInterval = (request->interval) / 1000;
	hddLog(LOG1,
		FL("Base scan interval = %d PNOScanTimerRepeatValue = %d"),
		tempInterval, pHddCtx->config->configPNOScanTimerRepeatValue);
	for (i = 0; i < pPnoRequest->scanTimers.ucScanTimersCount; i++) {
		pPnoRequest->scanTimers.aTimerValues[i].uTimerRepeat =
			pHddCtx->config->configPNOScanTimerRepeatValue;
		pPnoRequest->scanTimers.aTimerValues[i].uTimerValue =
			tempInterval;
		tempInterval *= 2;
	}
	/* Repeat last timer until pno disabled. */
	pPnoRequest->scanTimers.aTimerValues[i - 1].uTimerRepeat = 0;

	pPnoRequest->modePNO = SIR_PNO_MODE_IMMEDIATE;

	hddLog(LOG1,
		FL("SessionId %d, enable %d, modePNO %d, ucScanTimersCount %d"),
		pAdapter->sessionId, pPnoRequest->enable,
		pPnoRequest->modePNO,
		pPnoRequest->scanTimers.ucScanTimersCount);

	status = sme_set_preferred_network_list(WLAN_HDD_GET_HAL_CTX(pAdapter),
						pPnoRequest,
						pAdapter->sessionId,
						hdd_sched_scan_callback,
						pAdapter);
	if (CDF_STATUS_SUCCESS != status) {
		hddLog(LOGE, FL("Failed to enable PNO"));
		ret = -EINVAL;
		goto error;
	}

	hddLog(LOG1, FL("PNO scanRequest offloaded"));

error:
	cdf_mem_free(pPnoRequest);
	EXIT();
	return ret;
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

/**
 * __wlan_hdd_cfg80211_sched_scan_stop() - stop cfg80211 scheduled scan(pno)
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 *
 * Return: 0 for success, non zero for failure
 */
static int __wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
					       struct net_device *dev)
{
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	tHalHandle hHal;
	tpSirPNOScanReq pPnoRequest = NULL;
	int ret = 0;

	ENTER();

	if (CDF_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (NULL == pHddCtx) {
		hddLog(LOGE, FL("HDD context is Null"));
		return -ENODEV;
	}

	/* The return 0 is intentional when isLogpInProgress and
	 * isLoadUnloadInProgress. We did observe a crash due to a return of
	 * failure in sched_scan_stop , especially for a case where the unload
	 * of the happens at the same time. The function __cfg80211_stop_sched_scan
	 * was clearing rdev->sched_scan_req only when the sched_scan_stop returns
	 * success. If it returns a failure , then its next invocation due to the
	 * clean up of the second interface will have the dev pointer corresponding
	 * to the first one leading to a crash.
	 */
	if (pHddCtx->isLogpInProgress) {
		hddLog(LOGE, FL("LOGP in Progress. Ignore!!!"));
		return ret;
	}

	if ((pHddCtx->isLoadInProgress) || (pHddCtx->isUnloadInProgress)) {
		hddLog(LOGE, FL("Unloading/Loading in Progress. Ignore!!!"));
		return ret;
	}

	hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	if (NULL == hHal) {
		hddLog(LOGE, FL(" HAL context  is Null!!!"));
		return -EINVAL;
	}

	pPnoRequest = (tpSirPNOScanReq) cdf_mem_malloc(sizeof(tSirPNOScanReq));
	if (NULL == pPnoRequest) {
		hddLog(LOGE, FL("cdf_mem_malloc failed"));
		return -ENOMEM;
	}

	memset(pPnoRequest, 0, sizeof(tSirPNOScanReq));
	pPnoRequest->enable = 0;        /* Disable PNO */
	pPnoRequest->ucNetworksCount = 0;

	MTRACE(cdf_trace(CDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_CFG80211_SCHED_SCAN_STOP,
			 pAdapter->sessionId, pAdapter->device_mode));
	status = sme_set_preferred_network_list(hHal, pPnoRequest,
						pAdapter->sessionId,
						NULL, pAdapter);
	if (CDF_STATUS_SUCCESS != status) {
		hddLog(LOGE, FL("Failed to disabled PNO"));
		ret = -EINVAL;
	}

	hddLog(LOG1, FL("PNO scan disabled"));

	cdf_mem_free(pPnoRequest);

	EXIT();
	return ret;
}

/**
 * wlan_hdd_cfg80211_sched_scan_stop() - stop cfg80211 scheduled scan(pno)
 * @wiphy: Pointer to wiphy
 * @dev: Pointer network device
 *
 * Return: 0 for success, non zero for failure
 */
int wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
				      struct net_device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_sched_scan_stop(wiphy, dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /*FEATURE_WLAN_SCAN_PNO */

/**
 * hdd_vendor_put_ifindex() -send interface index
 * @skb: buffer pointer
 * @ifindex: interface index
 *
 * Send the IF index to differentiate the events on each interface
 * Return: 0 for success, non zero for failure
 */
int hdd_vendor_put_ifindex(struct sk_buff *skb, int ifindex)
{
	struct nlattr *attr;

	nla_nest_cancel(skb, ((void **)skb->cb)[2]);
	if (nla_put_u32(skb, NL80211_ATTR_IFINDEX, ifindex))
		return -EINVAL;

	attr = nla_nest_start(skb, NL80211_ATTR_VENDOR_DATA);
	((void **)skb->cb)[2] = attr;

	return 0;
}
