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

/* Include Files */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_wowl.h>
#include "wlan_hdd_trace.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_hdd_power.h"
#include "wlan_hdd_driver_ops.h"
#include "cds_concurrency.h"
#include "wlan_hdd_hostapd.h"

#include "wlan_hdd_p2p.h"
#include <linux/ctype.h>
#include "wma.h"
#include "wlan_hdd_napi.h"

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
#include <sme_api.h>
#include <sir_api.h>
#endif
#include "hif.h"

#if defined(LINUX_QCMBR)
#define SIOCIOCTLTX99 (SIOCDEVPRIVATE+13)
#endif

/*
 * Size of Driver command strings from upper layer
 */
#define SIZE_OF_SETROAMMODE             11      /* size of SETROAMMODE */
#define SIZE_OF_GETROAMMODE             11      /* size of GETROAMMODE */


#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
#define TID_MIN_VALUE 0
#define TID_MAX_VALUE 15
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

/*
 * Maximum buffer size used for returning the data back to user space
 */
#define WLAN_MAX_BUF_SIZE 1024
#define WLAN_PRIV_DATA_MAX_LEN    8192

/*
 * Driver miracast parameters 0-Disabled
 * 1-Source, 2-Sink
 */
#define WLAN_HDD_DRIVER_MIRACAST_CFG_MIN_VAL 0
#define WLAN_HDD_DRIVER_MIRACAST_CFG_MAX_VAL 2

/*
 * When ever we need to print IBSSPEERINFOALL for more than 16 STA
 * we will split the printing.
 */
#define NUM_OF_STA_DATA_TO_PRINT 16

/*
 * Android DRIVER command structures
 */
struct android_wifi_reassoc_params {
	unsigned char bssid[18];
	int channel;
};

#define ANDROID_WIFI_ACTION_FRAME_SIZE 1040
struct android_wifi_af_params {
	unsigned char bssid[18];
	int channel;
	int dwell_time;
	int len;
	unsigned char data[ANDROID_WIFI_ACTION_FRAME_SIZE];
};

/*
 * Define HDD driver command handling entry, each contains a command
 * string and the handler.
 */
typedef int (*hdd_drv_cmd_handler_t)(hdd_adapter_t *adapter,
				     hdd_context_t *hdd_ctx,
				     uint8_t *cmd,
				     uint8_t cmd_name_len,
				     hdd_priv_data_t *priv_data);

typedef struct {
	const char *cmd;
	hdd_drv_cmd_handler_t handler;
} hdd_drv_cmd_t;

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
#define WLAN_WAIT_TIME_READY_TO_EXTWOW   2000
#define WLAN_HDD_MAX_TCP_PORT            65535
#endif

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
static void hdd_get_tsm_stats_cb(tAniTrafStrmMetrics tsm_metrics,
				 const uint32_t staId, void *context)
{
	struct statsContext *stats_context = NULL;
	hdd_adapter_t *adapter = NULL;

	if (NULL == context) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, context [%p]", __func__, context);
		return;
	}

	/*
	 * there is a race condition that exists between this callback
	 * function and the caller since the caller could time out either
	 * before or while this code is executing.  we use a spinlock to
	 * serialize these actions
	 */
	spin_lock(&hdd_context_lock);

	stats_context = context;
	adapter = stats_context->pAdapter;
	if ((NULL == adapter) ||
	    (STATS_CONTEXT_MAGIC != stats_context->magic)) {
		/*
		 * the caller presumably timed out so there is
		 * nothing we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, adapter [%p] magic [%08x]",
		       __func__, adapter, stats_context->magic);
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	stats_context->magic = 0;

	/* copy over the tsm stats */
	adapter->tsmStats.UplinkPktQueueDly = tsm_metrics.UplinkPktQueueDly;
	qdf_mem_copy(adapter->tsmStats.UplinkPktQueueDlyHist,
		     tsm_metrics.UplinkPktQueueDlyHist,
		     sizeof(adapter->tsmStats.UplinkPktQueueDlyHist) /
		     sizeof(adapter->tsmStats.UplinkPktQueueDlyHist[0]));
	adapter->tsmStats.UplinkPktTxDly = tsm_metrics.UplinkPktTxDly;
	adapter->tsmStats.UplinkPktLoss = tsm_metrics.UplinkPktLoss;
	adapter->tsmStats.UplinkPktCount = tsm_metrics.UplinkPktCount;
	adapter->tsmStats.RoamingCount = tsm_metrics.RoamingCount;
	adapter->tsmStats.RoamingDly = tsm_metrics.RoamingDly;

	/* notify the caller */
	complete(&stats_context->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

static
QDF_STATUS hdd_get_tsm_stats(hdd_adapter_t *adapter,
			     const uint8_t tid,
			     tAniTrafStrmMetrics *tsm_metrics)
{
	hdd_station_ctx_t *hdd_sta_ctx = NULL;
	QDF_STATUS hstatus;
	QDF_STATUS vstatus = QDF_STATUS_SUCCESS;
	unsigned long rc;
	struct statsContext context;
	hdd_context_t *hdd_ctx = NULL;

	if (NULL == adapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: adapter is NULL", __func__);
		return QDF_STATUS_E_FAULT;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* we are connected prepare our callback context */
	init_completion(&context.completion);
	context.pAdapter = adapter;
	context.magic = STATS_CONTEXT_MAGIC;

	/* query tsm stats */
	hstatus = sme_get_tsm_stats(hdd_ctx->hHal, hdd_get_tsm_stats_cb,
				    hdd_sta_ctx->conn_info.staId[0],
				    hdd_sta_ctx->conn_info.bssId,
				    &context, hdd_ctx->pcds_context, tid);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Unable to retrieve statistics", __func__);
		vstatus = QDF_STATUS_E_FAULT;
	} else {
		/* request was sent -- wait for the response */
		rc = wait_for_completion_timeout(&context.completion,
				msecs_to_jiffies(WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: SME timed out while retrieving statistics",
			       __func__);
			vstatus = QDF_STATUS_E_TIMEOUT;
		}
	}

	/*
	 * either we never sent a request, we sent a request and received a
	 * response or we sent a request and timed out.  if we never sent a
	 * request or if we sent a request and got a response, we want to
	 * clear the magic out of paranoia.  if we timed out there is a
	 * race condition such that the callback function could be
	 * executing at the same time we are. of primary concern is if the
	 * callback function had already verified the "magic" but had not
	 * yet set the completion variable when a timeout occurred. we
	 * serialize these activities by invalidating the magic while
	 * holding a shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	if (QDF_STATUS_SUCCESS == vstatus) {
		tsm_metrics->UplinkPktQueueDly =
			adapter->tsmStats.UplinkPktQueueDly;
		qdf_mem_copy(tsm_metrics->UplinkPktQueueDlyHist,
			     adapter->tsmStats.UplinkPktQueueDlyHist,
			     sizeof(adapter->tsmStats.UplinkPktQueueDlyHist) /
			     sizeof(adapter->tsmStats.
				    UplinkPktQueueDlyHist[0]));
		tsm_metrics->UplinkPktTxDly = adapter->tsmStats.UplinkPktTxDly;
		tsm_metrics->UplinkPktLoss = adapter->tsmStats.UplinkPktLoss;
		tsm_metrics->UplinkPktCount = adapter->tsmStats.UplinkPktCount;
		tsm_metrics->RoamingCount = adapter->tsmStats.RoamingCount;
		tsm_metrics->RoamingDly = adapter->tsmStats.RoamingDly;
	}
	return vstatus;
}
#endif /*FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

static void hdd_get_band_helper(hdd_context_t *hdd_ctx, int *pBand)
{
	eCsrBand band = -1;
	sme_get_freq_band((tHalHandle) (hdd_ctx->hHal), &band);
	switch (band) {
	case eCSR_BAND_ALL:
		*pBand = WLAN_HDD_UI_BAND_AUTO;
		break;

	case eCSR_BAND_24:
		*pBand = WLAN_HDD_UI_BAND_2_4_GHZ;
		break;

	case eCSR_BAND_5G:
		*pBand = WLAN_HDD_UI_BAND_5_GHZ;
		break;

	default:
		hddLog(QDF_TRACE_LEVEL_WARN, "%s: Invalid Band %d", __func__,
		       band);
		*pBand = -1;
		break;
	}
}

/**
 * _hdd_parse_bssid_and_chan() - helper function to parse bssid and channel
 * @data:            input data
 * @target_ap_bssid: pointer to bssid (output parameter)
 * @channel:         pointer to channel (output parameter)
 *
 * Return: 0 if parsing is successful; -EINVAL otherwise
 */
static int _hdd_parse_bssid_and_chan(const uint8_t **data,
				     uint8_t *bssid,
				     uint8_t *channel)
{
	const uint8_t *in_ptr;
	int            v = 0;
	int            temp_int;
	uint8_t        temp_buf[32];

	/* 12 hexa decimal digits, 5 ':' and '\0' */
	uint8_t        mac_addr[18];

	if (!data || !*data)
		return -EINVAL;

	in_ptr = *data;

	in_ptr = strnchr(in_ptr, strlen(in_ptr), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == in_ptr)
		goto error;
	/* no space after the command */
	else if (SPACE_ASCII_VALUE != *in_ptr)
		goto error;

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *in_ptr) && ('\0' != *in_ptr))
		in_ptr++;

	/* no argument followed by spaces */
	if ('\0' == *in_ptr)
		goto error;

	v = sscanf(in_ptr, "%17s", mac_addr);
	if (!((1 == v) && hdd_is_valid_mac_address(mac_addr))) {
		hddLog(LOGE,
			FL(
			   "Invalid MAC address or All hex inputs are not read (%d)"
			),
			v);
		goto error;
	}

	bssid[0] = hex_to_bin(mac_addr[0]) << 4 |
			hex_to_bin(mac_addr[1]);
	bssid[1] = hex_to_bin(mac_addr[3]) << 4 |
			hex_to_bin(mac_addr[4]);
	bssid[2] = hex_to_bin(mac_addr[6]) << 4 |
			hex_to_bin(mac_addr[7]);
	bssid[3] = hex_to_bin(mac_addr[9]) << 4 |
			hex_to_bin(mac_addr[10]);
	bssid[4] = hex_to_bin(mac_addr[12]) << 4 |
			hex_to_bin(mac_addr[13]);
	bssid[5] = hex_to_bin(mac_addr[15]) << 4 |
			hex_to_bin(mac_addr[16]);

	/* point to the next argument */
	in_ptr = strnchr(in_ptr, strlen(in_ptr), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == in_ptr)
		goto error;

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *in_ptr) && ('\0' != *in_ptr))
		in_ptr++;

	/* no argument followed by spaces */
	if ('\0' == *in_ptr)
		goto error;

	/* get the next argument ie the channel number */
	v = sscanf(in_ptr, "%31s ", temp_buf);
	if (1 != v)
		goto error;

	v = kstrtos32(temp_buf, 10, &temp_int);
	if ((v < 0) || (temp_int < 0) ||
	    (temp_int > WNI_CFG_CURRENT_CHANNEL_STAMAX))
		return -EINVAL;

	*channel = temp_int;
	*data = in_ptr;
	return 0;
error:
	*data = in_ptr;
	return -EINVAL;
}

/**
 * hdd_parse_send_action_frame_data() - HDD Parse send action frame data
 * @pValue:         Pointer to input data
 * @pTargetApBssid: Pointer to target Ap bssid
 * @pChannel:       Pointer to the Target AP channel
 * @pDwellTime:     Pointer to the time to stay off-channel
 *                  after transmitting action frame
 * @pBuf:           Pointer to data
 * @pBufLen:        Pointer to data length
 *
 * This function parses the send action frame data passed in the format
 * SENDACTIONFRAME<space><bssid><space><channel><space><dwelltime><space><data>
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_send_action_frame_v1_data(const uint8_t *pValue,
				    uint8_t *pTargetApBssid,
				    uint8_t *pChannel, uint8_t *pDwellTime,
				    uint8_t **pBuf, uint8_t *pBufLen)
{
	const uint8_t *inPtr = pValue;
	const uint8_t *dataEnd;
	int tempInt;
	int j = 0;
	int i = 0;
	int v = 0;
	uint8_t tempBuf[32];
	uint8_t tempByte = 0;

	if (_hdd_parse_bssid_and_chan(&inPtr, pTargetApBssid, pChannel))
		return -EINVAL;

	/* point to the next argument */
	inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr)
		return -EINVAL;
	/* removing empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;

	/* no argument followed by spaces */
	if ('\0' == *inPtr) {
		return -EINVAL;
	}

	/* getting the next argument ie the dwell time */
	v = sscanf(inPtr, "%31s ", tempBuf);
	if (1 != v)
		return -EINVAL;

	v = kstrtos32(tempBuf, 10, &tempInt);
	if (v < 0 || tempInt < 0)
		return -EINVAL;

	*pDwellTime = tempInt;

	/* point to the next argument */
	inPtr = strnchr(inPtr, strlen(inPtr), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr)
		return -EINVAL;
	/* removing empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;

	/* no argument followed by spaces */
	if ('\0' == *inPtr) {
		return -EINVAL;
	}

	/* find the length of data */
	dataEnd = inPtr;
	while (('\0' != *dataEnd)) {
		dataEnd++;
	}
	*pBufLen = dataEnd - inPtr;
	if (*pBufLen <= 0)
		return -EINVAL;

	/*
	 * Allocate the number of bytes based on the number of input characters
	 * whether it is even or odd.
	 * if the number of input characters are even, then we need N/2 byte.
	 * if the number of input characters are odd, then we need do (N+1)/2
	 * to compensate rounding off.
	 * For example, if N = 18, then (18 + 1)/2 = 9 bytes are enough.
	 * If N = 19, then we need 10 bytes, hence (19 + 1)/2 = 10 bytes
	 */
	*pBuf = qdf_mem_malloc((*pBufLen + 1) / 2);
	if (NULL == *pBuf) {
		hddLog(LOGE, FL("cdf_mem_alloc failed"));
		return -ENOMEM;
	}

	/* the buffer received from the upper layer is character buffer,
	 * we need to prepare the buffer taking 2 characters in to a U8 hex
	 * decimal number for example 7f0000f0...form a buffer to contain 7f
	 * in 0th location, 00 in 1st and f0 in 3rd location
	 */
	for (i = 0, j = 0; j < *pBufLen; j += 2) {
		if (j + 1 == *pBufLen) {
			tempByte = hex_to_bin(inPtr[j]);
		} else {
			tempByte =
				(hex_to_bin(inPtr[j]) << 4) |
				(hex_to_bin(inPtr[j + 1]));
		}
		(*pBuf)[i++] = tempByte;
	}
	*pBufLen = i;
	return 0;
}

/**
 * hdd_parse_reassoc_command_data() - HDD Parse reassoc command data
 * @pValue:         Pointer to input data (its a NULL terminated string)
 * @pTargetApBssid: Pointer to target Ap bssid
 * @pChannel:       Pointer to the Target AP channel
 *
 * This function parses the reasoc command data passed in the format
 * REASSOC<space><bssid><space><channel>
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_reassoc_command_v1_data(const uint8_t *pValue,
					     uint8_t *pTargetApBssid,
					     uint8_t *pChannel)
{
	const uint8_t *inPtr = pValue;

	if (_hdd_parse_bssid_and_chan(&inPtr, pTargetApBssid, pChannel))
		return -EINVAL;

	return 0;
}

/**
 * hdd_reassoc() - perform a userspace-directed reassoc
 * @adapter:	Adapter upon which the command was received
 * @bssid:	BSSID with which to reassociate
 * @channel:	channel upon which to reassociate
 *
 * This function performs a userspace-directed reassoc operation
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_reassoc(hdd_adapter_t *adapter, const uint8_t *bssid,
	    const uint8_t channel)
{
	hdd_station_ctx_t *pHddStaCtx;
	int ret = 0;

	if (WLAN_HDD_INFRA_STATION != adapter->device_mode) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return -EINVAL;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* if not associated, no need to proceed with reassoc */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		hddLog(QDF_TRACE_LEVEL_INFO, "%s: Not associated", __func__);
		ret = -EINVAL;
		goto exit;
	}

	/*
	 * if the target bssid is same as currently associated AP,
	 * then no need to proceed with reassoc
	 */
	if (!memcmp(bssid, pHddStaCtx->conn_info.bssId.bytes,
			QDF_MAC_ADDR_SIZE)) {
		hddLog(LOG1,
		       FL("Reassoc BSSID is same as currently associated AP bssid"));
		ret = -EINVAL;
		goto exit;
	}

	/* Check channel number is a valid channel number */
	if (QDF_STATUS_SUCCESS !=
	    wlan_hdd_validate_operation_channel(adapter, channel)) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: Invalid Channel %d",
		       __func__, channel);
		ret = -EINVAL;
		goto exit;
	}

	/* Proceed with reassoc */
	{
		tCsrHandoffRequest handoffInfo;
		hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

		handoffInfo.channel = channel;
		handoffInfo.src = REASSOC;
		qdf_mem_copy(handoffInfo.bssid.bytes, bssid, QDF_MAC_ADDR_SIZE);
		sme_handoff_request(hdd_ctx->hHal, adapter->sessionId,
				    &handoffInfo);
	}
exit:
	return ret;
}

/**
 * hdd_parse_reassoc_v1() - parse version 1 of the REASSOC command
 * @adapter:	Adapter upon which the command was received
 * @command:	ASCII text command that was received
 *
 * This function parses the v1 REASSOC command with the format
 *
 *    REASSOC xx:xx:xx:xx:xx:xx CH
 *
 * Where "xx:xx:xx:xx:xx:xx" is the Hex-ASCII representation of the
 * BSSID and CH is the ASCII representation of the channel.  For
 * example
 *
 *    REASSOC 00:0a:0b:11:22:33 48
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_reassoc_v1(hdd_adapter_t *adapter, const char *command)
{
	uint8_t channel = 0;
	tSirMacAddr bssid;
	int ret;

	ret = hdd_parse_reassoc_command_v1_data(command, bssid, &channel);
	if (ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Failed to parse reassoc command data", __func__);
	} else {
		ret = hdd_reassoc(adapter, bssid, channel);
	}
	return ret;
}

/**
 * hdd_parse_reassoc_v2() - parse version 2 of the REASSOC command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received, ASCII command
 *      	     followed by binary data
 *
 * This function parses the v2 REASSOC command with the format
 *
 *    REASSOC <android_wifi_reassoc_params>
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_reassoc_v2(hdd_adapter_t *adapter, const char *command)
{
	struct android_wifi_reassoc_params params;
	tSirMacAddr bssid;
	int ret;

	/* The params are located after "REASSOC " */
	memcpy(&params, command + 8, sizeof(params));

	if (!mac_pton(params.bssid, (u8 *) &bssid)) {
		hddLog(LOGE, "%s: MAC address parsing failed", __func__);
		ret = -EINVAL;
	} else {
		ret = hdd_reassoc(adapter, bssid, params.channel);
	}
	return ret;
}

/**
 * hdd_parse_reassoc() - parse the REASSOC command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received
 *
 * There are two different versions of the REASSOC command.  Version 1
 * of the command contains a parameter list that is ASCII characters
 * whereas version 2 contains a combination of ASCII and binary
 * payload.  Determine if a version 1 or a version 2 command is being
 * parsed by examining the parameters, and then dispatch the parser
 * that is appropriate for the command.
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_reassoc(hdd_adapter_t *adapter, const char *command)
{
	int ret;

	/* both versions start with "REASSOC "
	 * v1 has a bssid and channel # as an ASCII string
	 *    REASSOC xx:xx:xx:xx:xx:xx CH
	 * v2 has a C struct
	 *    REASSOC <binary c struct>
	 *
	 * The first field in the v2 struct is also the bssid in ASCII.
	 * But in the case of a v2 message the BSSID is NUL-terminated.
	 * Hence we can peek at that offset to see if this is V1 or V2
	 * REASSOC xx:xx:xx:xx:xx:xx*
	 *           1111111111222222
	 * 01234567890123456789012345
	 */
	if (command[25]) {
		ret = hdd_parse_reassoc_v1(adapter, command);
	} else {
		ret = hdd_parse_reassoc_v2(adapter, command);
	}

	return ret;
}

/**
 * hdd_sendactionframe() - send a userspace-supplied action frame
 * @adapter:	Adapter upon which the command was received
 * @bssid:	BSSID target of the action frame
 * @channel:	Channel upon which to send the frame
 * @dwell_time:	Amount of time to dwell when the frame is sent
 * @payload_len:Length of the payload
 * @payload:	Payload of the frame
 *
 * This function sends a userspace-supplied action frame
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_sendactionframe(hdd_adapter_t *adapter, const uint8_t *bssid,
		    const uint8_t channel, const uint8_t dwell_time,
		    const uint8_t payload_len, const uint8_t *payload)
{
	struct ieee80211_channel chan;
	uint8_t frame_len;
	uint8_t *frame;
	struct ieee80211_hdr_3addr *hdr;
	u64 cookie;
	hdd_station_ctx_t *pHddStaCtx;
	hdd_context_t *hdd_ctx;
	int ret = 0;
	tpSirMacVendorSpecificFrameHdr pVendorSpecific =
		(tpSirMacVendorSpecificFrameHdr) payload;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	struct cfg80211_mgmt_tx_params params;
#endif

	if (WLAN_HDD_INFRA_STATION != adapter->device_mode) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return -EINVAL;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	/* if not associated, no need to send action frame */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		hddLog(QDF_TRACE_LEVEL_INFO, "%s: Not associated", __func__);
		ret = -EINVAL;
		goto exit;
	}

	/*
	 * if the target bssid is different from currently associated AP,
	 * then no need to send action frame
	 */
	if (memcmp(bssid, pHddStaCtx->conn_info.bssId.bytes,
			QDF_MAC_ADDR_SIZE)) {
		hddLog(LOG1, FL("STA is not associated to this AP"));
		ret = -EINVAL;
		goto exit;
	}

	chan.center_freq = sme_chn_to_freq(channel);
	/* Check if it is specific action frame */
	if (pVendorSpecific->category ==
	    SIR_MAC_ACTION_VENDOR_SPECIFIC_CATEGORY) {
		static const uint8_t Oui[] = { 0x00, 0x00, 0xf0 };
		if (!qdf_mem_cmp(pVendorSpecific->Oui, (void *)Oui, 3)) {
			/*
			 * if the channel number is different from operating
			 * channel then no need to send action frame
			 */
			if (channel != 0) {
				if (channel !=
				    pHddStaCtx->conn_info.operationChannel) {
					hddLog(QDF_TRACE_LEVEL_INFO,
					       "%s: channel(%d) is different from operating channel(%d)",
					       __func__, channel,
					       pHddStaCtx->conn_info.
					       operationChannel);
					ret = -EINVAL;
					goto exit;
				}
				/*
				 * If channel number is specified and same
				 * as home channel, ensure that action frame
				 * is sent immediately by cancelling
				 * roaming scans. Otherwise large dwell times
				 * may cause long delays in sending action
				 * frames.
				 */
				sme_abort_roam_scan(hdd_ctx->hHal,
						    adapter->sessionId);
			} else {
				/*
				 * 0 is accepted as current home channel,
				 * delayed transmission of action frame is ok.
				 */
				chan.center_freq =
					sme_chn_to_freq(pHddStaCtx->conn_info.
							operationChannel);
			}
		}
	}
	if (chan.center_freq == 0) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s:invalid channel number %d",
		       __func__, channel);
		ret = -EINVAL;
		goto exit;
	}

	frame_len = payload_len + 24;
	frame = qdf_mem_malloc(frame_len);
	if (!frame) {
		hddLog(LOGE, FL("memory allocation failed"));
		ret = -ENOMEM;
		goto exit;
	}
	qdf_mem_zero(frame, frame_len);

	hdr = (struct ieee80211_hdr_3addr *)frame;
	hdr->frame_control =
		cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	qdf_mem_copy(hdr->addr1, bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->addr2, adapter->macAddressCurrent.bytes,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->addr3, bssid, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr + 1, payload, payload_len);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	params.chan = &chan;
	params.offchan = 0;
	params.wait = dwell_time;
	params.buf = frame;
	params.len = frame_len;
	params.no_cck = 1;
	params.dont_wait_for_ack = 1;
	ret = wlan_hdd_mgmt_tx(NULL, &adapter->wdev, &params, &cookie);
#else
	ret = wlan_hdd_mgmt_tx(NULL,
			       &(adapter->wdev),
			       &chan, 0,

			       dwell_time, frame, frame_len, 1, 1, &cookie);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0) */

	qdf_mem_free(frame);
exit:
	return ret;
}

/**
 * hdd_parse_sendactionframe_v1() - parse version 1 of the
 *       SENDACTIONFRAME command
 * @adapter:	Adapter upon which the command was received
 * @command:	ASCII text command that was received
 *
 * This function parses the v1 SENDACTIONFRAME command with the format
 *
 *    SENDACTIONFRAME xx:xx:xx:xx:xx:xx CH DW xxxxxx
 *
 * Where "xx:xx:xx:xx:xx:xx" is the Hex-ASCII representation of the
 * BSSID, CH is the ASCII representation of the channel, DW is the
 * ASCII representation of the dwell time, and xxxxxx is the Hex-ASCII
 * payload.  For example
 *
 *    SENDACTIONFRAME 00:0a:0b:11:22:33 48 40 aabbccddee
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_sendactionframe_v1(hdd_adapter_t *adapter, const char *command)
{
	uint8_t channel = 0;
	uint8_t dwell_time = 0;
	uint8_t payload_len = 0;
	uint8_t *payload = NULL;
	tSirMacAddr bssid;
	int ret;

	ret = hdd_parse_send_action_frame_v1_data(command, bssid, &channel,
						  &dwell_time, &payload,
						  &payload_len);
	if (ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Failed to parse send action frame data", __func__);
	} else {
		ret = hdd_sendactionframe(adapter, bssid, channel,
					  dwell_time, payload_len, payload);
		qdf_mem_free(payload);
	}

	return ret;
}

/**
 * hdd_parse_sendactionframe_v2() - parse version 2 of the
 *       SENDACTIONFRAME command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received, ASCII command
 *      	     followed by binary data
 *
 * This function parses the v2 SENDACTIONFRAME command with the format
 *
 *    SENDACTIONFRAME <android_wifi_af_params>
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_sendactionframe_v2(hdd_adapter_t *adapter, const char *command)
{
	struct android_wifi_af_params *params;
	tSirMacAddr bssid;
	int ret;

	/* params are large so keep off the stack */
	params = kmalloc(sizeof(*params), GFP_KERNEL);
	if (!params)
		return -ENOMEM;

	/* The params are located after "SENDACTIONFRAME " */
	memcpy(params, command + 16, sizeof(*params));

	if (!mac_pton(params->bssid, (u8 *) &bssid)) {
		hddLog(LOGE, "%s: MAC address parsing failed", __func__);
		ret = -EINVAL;
	} else {
		ret = hdd_sendactionframe(adapter, bssid, params->channel,
					  params->dwell_time, params->len,
					  params->data);
	}
	kfree(params);
	return ret;
}

/**
 * hdd_parse_sendactionframe() - parse the SENDACTIONFRAME command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received
 *
 * There are two different versions of the SENDACTIONFRAME command.
 * Version 1 of the command contains a parameter list that is ASCII
 * characters whereas version 2 contains a combination of ASCII and
 * binary payload.  Determine if a version 1 or a version 2 command is
 * being parsed by examining the parameters, and then dispatch the
 * parser that is appropriate for the version of the command.
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_sendactionframe(hdd_adapter_t *adapter, const char *command)
{
	int ret;

	/*
	 * both versions start with "SENDACTIONFRAME "
	 * v1 has a bssid and other parameters as an ASCII string
	 *    SENDACTIONFRAME xx:xx:xx:xx:xx:xx CH DWELL LEN FRAME
	 * v2 has a C struct
	 *    SENDACTIONFRAME <binary c struct>
	 *
	 * The first field in the v2 struct is also the bssid in ASCII.
	 * But in the case of a v2 message the BSSID is NUL-terminated.
	 * Hence we can peek at that offset to see if this is V1 or V2
	 * SENDACTIONFRAME xx:xx:xx:xx:xx:xx*
	 *           111111111122222222223333
	 * 0123456789012345678901234567890123
	 */
	if (command[33]) {
		ret = hdd_parse_sendactionframe_v1(adapter, command);
	} else {
		ret = hdd_parse_sendactionframe_v2(adapter, command);
	}

	return ret;
}

/**
 * hdd_parse_channellist() - HDD Parse channel list
 * @pValue:		Pointer to input channel list
 * @ChannelList:	Pointer to local output array to record
 *      	       channel list
 * @pNumChannels:	Pointer to number of roam scan channels
 *
 * This function parses the channel list passed in the format
 * SETROAMSCANCHANNELS<space><Number of channels><space>Channel 1<space>Channel 2<space>Channel N
 * if the Number of channels (N) does not match with the actual number
 * of channels passed then take the minimum of N and count of
 * (Ch1, Ch2, ...Ch M). For example, if SETROAMSCANCHANNELS 3 36 40 44 48,
 * only 36, 40 and 44 shall be taken. If SETROAMSCANCHANNELS 5 36 40 44 48,
 * ignore 5 and take 36, 40, 44 and 48. This function does not take care of
 * removing duplicate channels from the list
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_channellist(const uint8_t *pValue, uint8_t *pChannelList,
		      uint8_t *pNumChannels)
{
	const uint8_t *inPtr = pValue;
	int tempInt;
	int j = 0;
	int v = 0;
	char buf[32];

	inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr) {
		return -EINVAL;
	}

	/* no space after the command */
	else if (SPACE_ASCII_VALUE != *inPtr) {
		return -EINVAL;
	}

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;

	/* no argument followed by spaces */
	if ('\0' == *inPtr) {
		return -EINVAL;
	}

	/* get the first argument ie the number of channels */
	v = sscanf(inPtr, "%31s ", buf);
	if (1 != v)
		return -EINVAL;

	v = kstrtos32(buf, 10, &tempInt);
	if ((v < 0) ||
	    (tempInt <= 0) || (tempInt > WNI_CFG_VALID_CHANNEL_LIST_LEN)) {
		return -EINVAL;
	}

	*pNumChannels = tempInt;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Number of channels are: %d", *pNumChannels);

	for (j = 0; j < (*pNumChannels); j++) {
		/*
		 * inPtr pointing to the beginning of first space after number
		 * of channels
		 */
		inPtr = strpbrk(inPtr, " ");
		/* no channel list after the number of channels argument */
		if (NULL == inPtr) {
			if (0 != j) {
				*pNumChannels = j;
				return 0;
			} else {
				return -EINVAL;
			}
		}

		/* remove empty space */
		while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
			inPtr++;

		/*
		 * no channel list after the number of channels
		 * argument and spaces
		 */
		if ('\0' == *inPtr) {
			if (0 != j) {
				*pNumChannels = j;
				return 0;
			} else {
				return -EINVAL;
			}
		}

		v = sscanf(inPtr, "%31s ", buf);
		if (1 != v)
			return -EINVAL;

		v = kstrtos32(buf, 10, &tempInt);
		if ((v < 0) ||
		    (tempInt <= 0) ||
		    (tempInt > WNI_CFG_CURRENT_CHANNEL_STAMAX)) {
			return -EINVAL;
		}
		pChannelList[j] = tempInt;

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
			  "Channel %d added to preferred channel list",
			  pChannelList[j]);
	}

	return 0;
}

/**
 * hdd_parse_set_roam_scan_channels_v1() - parse version 1 of the
 * SETROAMSCANCHANNELS command
 * @adapter:	Adapter upon which the command was received
 * @command:	ASCII text command that was received
 *
 * This function parses the v1 SETROAMSCANCHANNELS command with the format
 *
 *    SETROAMSCANCHANNELS N C1 C2 ... Cn
 *
 * Where "N" is the ASCII representation of the number of channels and
 * C1 thru Cn is the ASCII representation of the channels.  For example
 *
 *    SETROAMSCANCHANNELS 4 36 40 44 48
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_set_roam_scan_channels_v1(hdd_adapter_t *adapter,
				    const char *command)
{
	uint8_t channel_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t num_chan = 0;
	QDF_STATUS status;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int ret;

	ret = hdd_parse_channellist(command, channel_list, &num_chan);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to parse channel list information",
			  __func__);
		goto exit;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_SETROAMSCANCHANNELS_IOCTL,
			 adapter->sessionId, num_chan));

	if (num_chan > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: number of channels (%d) supported exceeded max (%d)",
			  __func__, num_chan, WNI_CFG_VALID_CHANNEL_LIST_LEN);
		ret = -EINVAL;
		goto exit;
	}

	status =
		sme_change_roam_scan_channel_list(hdd_ctx->hHal,
						  adapter->sessionId,
						  channel_list, num_chan);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to update channel list information",
			  __func__);
		ret = -EINVAL;
		goto exit;
	}
exit:
	return ret;
}

/**
 * hdd_parse_set_roam_scan_channels_v2() - parse version 2 of the
 * SETROAMSCANCHANNELS command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received, ASCII command
 *      	     followed by binary data
 *
 * This function parses the v2 SETROAMSCANCHANNELS command with the format
 *
 *    SETROAMSCANCHANNELS [N][C1][C2][Cn]
 *
 * The command begins with SETROAMSCANCHANNELS followed by a space, but
 * what follows the space is an array of u08 parameters.  For example
 *
 *    SETROAMSCANCHANNELS [0x04 0x24 0x28 0x2c 0x30]
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_set_roam_scan_channels_v2(hdd_adapter_t *adapter,
				    const char *command)
{
	const uint8_t *value;
	uint8_t channel_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t channel;
	uint8_t num_chan;
	int i;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	QDF_STATUS status;
	int ret = 0;

	/* array of values begins after "SETROAMSCANCHANNELS " */
	value = command + 20;

	num_chan = *value++;
	if (num_chan > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: number of channels (%d) supported exceeded max (%d)",
			  __func__, num_chan, WNI_CFG_VALID_CHANNEL_LIST_LEN);
		ret = -EINVAL;
		goto exit;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_SETROAMSCANCHANNELS_IOCTL,
			 adapter->sessionId, num_chan));

	for (i = 0; i < num_chan; i++) {
		channel = *value++;
		if (channel > WNI_CFG_CURRENT_CHANNEL_STAMAX) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: index %d invalid channel %d", __func__,
				  i, channel);
			ret = -EINVAL;
			goto exit;
		}
		channel_list[i] = channel;
	}
	status =
		sme_change_roam_scan_channel_list(hdd_ctx->hHal,
						  adapter->sessionId,
						  channel_list, num_chan);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to update channel list information",
			  __func__);
		ret = -EINVAL;
		goto exit;
	}
exit:
	return ret;
}

/**
 * hdd_parse_set_roam_scan_channels() - parse the
 * SETROAMSCANCHANNELS command
 * @adapter:	Adapter upon which the command was received
 * @command:	Command that was received
 *
 * There are two different versions of the SETROAMSCANCHANNELS command.
 * Version 1 of the command contains a parameter list that is ASCII
 * characters whereas version 2 contains a binary payload.  Determine
 * if a version 1 or a version 2 command is being parsed by examining
 * the parameters, and then dispatch the parser that is appropriate for
 * the command.
 *
 * Return: 0 for success non-zero for failure
 */
static int
hdd_parse_set_roam_scan_channels(hdd_adapter_t *adapter, const char *command)
{
	const char *cursor;
	char ch;
	bool v1;
	int ret;

	/* start after "SETROAMSCANCHANNELS " */
	cursor = command + 20;

	/* assume we have a version 1 command until proven otherwise */
	v1 = true;

	/* v1 params will only contain ASCII digits and space */
	while ((ch = *cursor++) && v1) {
		if (!(isdigit(ch) || isspace(ch))) {
			v1 = false;
		}
	}
	if (v1) {
		ret = hdd_parse_set_roam_scan_channels_v1(adapter, command);
	} else {
		ret = hdd_parse_set_roam_scan_channels_v2(adapter, command);
	}

	return ret;
}

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
/**
 * hdd_parse_plm_cmd() - HDD Parse Plm command
 * @pValue:	Pointer to input data
 * @pPlmRequest:Pointer to output struct tpSirPlmReq
 *
 * This function parses the plm command passed in the format
 * CCXPLMREQ<space><enable><space><dialog_token><space>
 * <meas_token><space><num_of_bursts><space><burst_int><space>
 * <measu duration><space><burst_len><space><desired_tx_pwr>
 * <space><multcast_addr><space><number_of_channels>
 * <space><channel_numbers>
 *
 * Return: 0 for success non-zero for failure
 */
QDF_STATUS hdd_parse_plm_cmd(uint8_t *pValue, tSirPlmReq *pPlmRequest)
{
	uint8_t *cmdPtr = NULL;
	int count, content = 0, ret = 0;
	char buf[32];

	/* move to argument list */
	cmdPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
	if (NULL == cmdPtr)
		return QDF_STATUS_E_FAILURE;

	/* no space after the command */
	if (SPACE_ASCII_VALUE != *cmdPtr)
		return QDF_STATUS_E_FAILURE;

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
		cmdPtr++;

	/* START/STOP PLM req */
	ret = sscanf(cmdPtr, "%31s ", buf);
	if (1 != ret)
		return QDF_STATUS_E_FAILURE;

	ret = kstrtos32(buf, 10, &content);
	if (ret < 0)
		return QDF_STATUS_E_FAILURE;

	pPlmRequest->enable = content;
	cmdPtr = strpbrk(cmdPtr, " ");

	if (NULL == cmdPtr)
		return QDF_STATUS_E_FAILURE;

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
		cmdPtr++;

	/* Dialog token of radio meas req containing meas reqIE */
	ret = sscanf(cmdPtr, "%31s ", buf);
	if (1 != ret)
		return QDF_STATUS_E_FAILURE;

	ret = kstrtos32(buf, 10, &content);
	if (ret < 0)
		return QDF_STATUS_E_FAILURE;

	pPlmRequest->diag_token = content;
	hddLog(QDF_TRACE_LEVEL_DEBUG, "diag token %d",
	       pPlmRequest->diag_token);
	cmdPtr = strpbrk(cmdPtr, " ");

	if (NULL == cmdPtr)
		return QDF_STATUS_E_FAILURE;

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
		cmdPtr++;

	/* measurement token of meas req IE */
	ret = sscanf(cmdPtr, "%31s ", buf);
	if (1 != ret)
		return QDF_STATUS_E_FAILURE;

	ret = kstrtos32(buf, 10, &content);
	if (ret < 0)
		return QDF_STATUS_E_FAILURE;

	pPlmRequest->meas_token = content;
	hddLog(QDF_TRACE_LEVEL_DEBUG, "meas token %d",
	       pPlmRequest->meas_token);

	hddLog(QDF_TRACE_LEVEL_ERROR,
	       "PLM req %s", pPlmRequest->enable ? "START" : "STOP");
	if (pPlmRequest->enable) {

		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* total number of bursts after which STA stops sending */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content < 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->numBursts = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG, "num burst %d",
		       pPlmRequest->numBursts);
		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* burst interval in seconds */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content <= 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->burstInt = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG, "burst Int %d",
		       pPlmRequest->burstInt);
		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* Meas dur in TU's,STA goes off-ch and transmit PLM bursts */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content <= 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->measDuration = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG, "measDur %d",
		       pPlmRequest->measDuration);
		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* burst length of PLM bursts */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content <= 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->burstLen = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG, "burstLen %d",
		       pPlmRequest->burstLen);
		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* desired tx power for transmission of PLM bursts */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content <= 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->desiredTxPwr = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG,
		       "desiredTxPwr %d", pPlmRequest->desiredTxPwr);

		for (count = 0; count < QDF_MAC_ADDR_SIZE; count++) {
			cmdPtr = strpbrk(cmdPtr, " ");

			if (NULL == cmdPtr)
				return QDF_STATUS_E_FAILURE;

			/* remove empty spaces */
			while ((SPACE_ASCII_VALUE == *cmdPtr)
			       && ('\0' != *cmdPtr))
				cmdPtr++;

			ret = sscanf(cmdPtr, "%31s ", buf);
			if (1 != ret)
				return QDF_STATUS_E_FAILURE;

			ret = kstrtos32(buf, 16, &content);
			if (ret < 0)
				return QDF_STATUS_E_FAILURE;

			pPlmRequest->mac_addr.bytes[count] = content;
		}

		hdd_debug("MC addr " MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pPlmRequest->mac_addr.bytes));

		cmdPtr = strpbrk(cmdPtr, " ");

		if (NULL == cmdPtr)
			return QDF_STATUS_E_FAILURE;

		/* remove empty spaces */
		while ((SPACE_ASCII_VALUE == *cmdPtr) && ('\0' != *cmdPtr))
			cmdPtr++;

		/* number of channels */
		ret = sscanf(cmdPtr, "%31s ", buf);
		if (1 != ret)
			return QDF_STATUS_E_FAILURE;

		ret = kstrtos32(buf, 10, &content);
		if (ret < 0)
			return QDF_STATUS_E_FAILURE;

		if (content < 0)
			return QDF_STATUS_E_FAILURE;

		pPlmRequest->plmNumCh = content;
		hddLog(QDF_TRACE_LEVEL_DEBUG, "numch %d",
		       pPlmRequest->plmNumCh);

		/* Channel numbers */
		for (count = 0; count < pPlmRequest->plmNumCh; count++) {
			cmdPtr = strpbrk(cmdPtr, " ");

			if (NULL == cmdPtr)
				return QDF_STATUS_E_FAILURE;

			/* remove empty spaces */
			while ((SPACE_ASCII_VALUE == *cmdPtr)
			       && ('\0' != *cmdPtr))
				cmdPtr++;

			ret = sscanf(cmdPtr, "%31s ", buf);
			if (1 != ret)
				return QDF_STATUS_E_FAILURE;

			ret = kstrtos32(buf, 10, &content);
			if (ret < 0)
				return QDF_STATUS_E_FAILURE;

			if (content <= 0)
				return QDF_STATUS_E_FAILURE;

			pPlmRequest->plmChList[count] = content;
			hddLog(QDF_TRACE_LEVEL_DEBUG, " ch- %d",
			       pPlmRequest->plmChList[count]);
		}
	}
	/* If PLM START */
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
static void wlan_hdd_ready_to_extwow(void *callbackContext, bool is_success)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *) callbackContext;
	int rc;

	rc = wlan_hdd_validate_context(hdd_ctx);
	if (0 != rc) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD context is not valid"));
		return;
	}
	hdd_ctx->ext_wow_should_suspend = is_success;
	complete(&hdd_ctx->ready_to_extwow);
}

static int hdd_enable_ext_wow(hdd_adapter_t *adapter,
			      tpSirExtWoWParams arg_params)
{
	tSirExtWoWParams params;
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	int rc;

	qdf_mem_copy(&params, arg_params, sizeof(params));

	INIT_COMPLETION(hdd_ctx->ready_to_extwow);

	cdf_ret_status = sme_configure_ext_wow(hHal, &params,
						&wlan_hdd_ready_to_extwow,
						hdd_ctx);
	if (QDF_STATUS_SUCCESS != cdf_ret_status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("sme_configure_ext_wow returned failure %d"),
		       cdf_ret_status);
		return -EPERM;
	}

	rc = wait_for_completion_timeout(&hdd_ctx->ready_to_extwow,
			msecs_to_jiffies(WLAN_WAIT_TIME_READY_TO_EXTWOW));
	if (!rc) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to get ready to extwow", __func__);
		return -EPERM;
	}

	if (hdd_ctx->ext_wow_should_suspend) {
		if (hdd_ctx->config->extWowGotoSuspend) {
			pm_message_t state;

			state.event = PM_EVENT_SUSPEND;
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
				  "%s: Received ready to ExtWoW. Going to suspend",
				  __func__);

			rc = wlan_hdd_cfg80211_suspend_wlan(hdd_ctx->wiphy, NULL);
			if (rc < 0) {
				QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
					"%s: wlan_hdd_cfg80211_suspend_wlan failed, error = %d",
					__func__, rc);
				return rc;
			}
			cdf_ret_status = wlan_hdd_bus_suspend(state);
			if (cdf_ret_status != QDF_STATUS_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
					"%s: wlan_hdd_suspend failed, status = %d",
					__func__, cdf_ret_status);
				wlan_hdd_cfg80211_resume_wlan(hdd_ctx->wiphy);
				return -EPERM;
			}
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Received ready to ExtWoW failure", __func__);
		return -EPERM;
	}

	return 0;
}

static int hdd_enable_ext_wow_parser(hdd_adapter_t *adapter, int vdev_id,
				     int value)
{
	tSirExtWoWParams params;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int rc;

	rc = wlan_hdd_validate_context(hdd_ctx);
	if (0 != rc) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD context is not valid"));
		return -EINVAL;
	}

	if (value < EXT_WOW_TYPE_APP_TYPE1 ||
	    value > EXT_WOW_TYPE_APP_TYPE1_2) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("Invalid type"));
		return -EINVAL;
	}

	if (value == EXT_WOW_TYPE_APP_TYPE1 &&
	    hdd_ctx->is_extwow_app_type1_param_set)
		params.type = value;
	else if (value == EXT_WOW_TYPE_APP_TYPE2 &&
		 hdd_ctx->is_extwow_app_type2_param_set)
		params.type = value;
	else if (value == EXT_WOW_TYPE_APP_TYPE1_2 &&
		 hdd_ctx->is_extwow_app_type1_param_set &&
		 hdd_ctx->is_extwow_app_type2_param_set)
		params.type = value;
	else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Set app params before enable it value %d"), value);
		return -EINVAL;
	}

	params.vdev_id = vdev_id;
	params.wakeup_pin_num = hdd_ctx->config->extWowApp1WakeupPinNumber |
				(hdd_ctx->config->extWowApp2WakeupPinNumber
					<< 8);

	return hdd_enable_ext_wow(adapter, &params);
}

static int hdd_set_app_type1_params(tHalHandle hHal,
				    tpSirAppType1Params arg_params)
{
	tSirAppType1Params params;
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;

	qdf_mem_copy(&params, arg_params, sizeof(params));

	cdf_ret_status = sme_configure_app_type1_params(hHal, &params);
	if (QDF_STATUS_SUCCESS != cdf_ret_status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("sme_configure_app_type1_params returned failure %d"),
		       cdf_ret_status);
		return -EPERM;
	}

	return 0;
}

static int hdd_set_app_type1_parser(hdd_adapter_t *adapter,
				    char *arg, int len)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	char id[20], password[20];
	tSirAppType1Params params;
	int rc;

	rc = wlan_hdd_validate_context(hdd_ctx);
	if (0 != rc) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD context is not valid"));
		return -EINVAL;
	}

	if (2 != sscanf(arg, "%8s %16s", id, password)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid Number of arguments"));
		return -EINVAL;
	}

	memset(&params, 0, sizeof(tSirAppType1Params));
	params.vdev_id = adapter->sessionId;
	qdf_copy_macaddr(&params.wakee_mac_addr, &adapter->macAddressCurrent);

	params.id_length = strlen(id);
	qdf_mem_copy(params.identification_id, id, params.id_length);
	params.pass_length = strlen(password);
	qdf_mem_copy(params.password, password, params.pass_length);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: %d %pM %.8s %u %.16s %u",
		  __func__, params.vdev_id, params.wakee_mac_addr.bytes,
		  params.identification_id, params.id_length,
		  params.password, params.pass_length);

	return hdd_set_app_type1_params(hHal, &params);
}

static int hdd_set_app_type2_params(tHalHandle hHal,
				    tpSirAppType2Params arg_params)
{
	tSirAppType2Params params;
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;

	qdf_mem_copy(&params, arg_params, sizeof(params));

	cdf_ret_status = sme_configure_app_type2_params(hHal, &params);
	if (QDF_STATUS_SUCCESS != cdf_ret_status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("sme_configure_app_type2_params returned failure %d"),
		       cdf_ret_status);
		return -EPERM;
	}

	return 0;
}

static int hdd_set_app_type2_parser(hdd_adapter_t *adapter,
				    char *arg, int len)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	char mac_addr[20], rc4_key[20];
	unsigned int gateway_mac[QDF_MAC_ADDR_SIZE];
	tSirAppType2Params params;
	int ret;

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("HDD context is not valid"));
		return -EINVAL;
	}

	memset(&params, 0, sizeof(tSirAppType2Params));

	ret = sscanf(arg, "%17s %16s %x %x %x %u %u %hu %hu %u %u %u %u %u %u",
		     mac_addr, rc4_key, (unsigned int *)&params.ip_id,
		     (unsigned int *)&params.ip_device_ip,
		     (unsigned int *)&params.ip_server_ip,
		     (unsigned int *)&params.tcp_seq,
		     (unsigned int *)&params.tcp_ack_seq,
		     (uint16_t *)&params.tcp_src_port,
		     (uint16_t *)&params.tcp_dst_port,
		     (unsigned int *)&params.keepalive_init,
		     (unsigned int *)&params.keepalive_min,
		     (unsigned int *)&params.keepalive_max,
		     (unsigned int *)&params.keepalive_inc,
		     (unsigned int *)&params.tcp_tx_timeout_val,
		     (unsigned int *)&params.tcp_rx_timeout_val);

	if (ret != 15 && ret != 7) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "Invalid Number of arguments");
		return -EINVAL;
	}

	if (6 !=
	    sscanf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", &gateway_mac[0],
		   &gateway_mac[1], &gateway_mac[2], &gateway_mac[3],
		   &gateway_mac[4], &gateway_mac[5])) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "Invalid MacAddress Input %s", mac_addr);
		return -EINVAL;
	}

	if (params.tcp_src_port > WLAN_HDD_MAX_TCP_PORT ||
	    params.tcp_dst_port > WLAN_HDD_MAX_TCP_PORT) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "Invalid TCP Port Number");
		return -EINVAL;
	}

	qdf_mem_copy(&params.gateway_mac.bytes, (uint8_t *) &gateway_mac,
			QDF_MAC_ADDR_SIZE);

	params.rc4_key_len = strlen(rc4_key);
	qdf_mem_copy(params.rc4_key, rc4_key, params.rc4_key_len);

	params.vdev_id = adapter->sessionId;
	params.tcp_src_port = (params.tcp_src_port != 0) ?
		params.tcp_src_port : hdd_ctx->config->extWowApp2TcpSrcPort;
	params.tcp_dst_port = (params.tcp_dst_port != 0) ?
		params.tcp_dst_port : hdd_ctx->config->extWowApp2TcpDstPort;
	params.keepalive_init = (params.keepalive_init != 0) ?
		params.keepalive_init : hdd_ctx->config->
						extWowApp2KAInitPingInterval;
	params.keepalive_min =
		(params.keepalive_min != 0) ?
			params.keepalive_min :
			hdd_ctx->config->extWowApp2KAMinPingInterval;
	params.keepalive_max =
		(params.keepalive_max != 0) ?
			params.keepalive_max :
			hdd_ctx->config->extWowApp2KAMaxPingInterval;
	params.keepalive_inc =
		(params.keepalive_inc != 0) ?
			params.keepalive_inc :
			hdd_ctx->config->extWowApp2KAIncPingInterval;
	params.tcp_tx_timeout_val =
		(params.tcp_tx_timeout_val != 0) ?
			params.tcp_tx_timeout_val :
			hdd_ctx->config->extWowApp2TcpTxTimeout;
	params.tcp_rx_timeout_val =
		(params.tcp_rx_timeout_val != 0) ?
			params.tcp_rx_timeout_val :
			hdd_ctx->config->extWowApp2TcpRxTimeout;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: %pM %.16s %u %u %u %u %u %u %u %u %u %u %u %u %u",
		  __func__, gateway_mac, rc4_key, params.ip_id,
		  params.ip_device_ip, params.ip_server_ip, params.tcp_seq,
		  params.tcp_ack_seq, params.tcp_src_port, params.tcp_dst_port,
		  params.keepalive_init, params.keepalive_min,
		  params.keepalive_max, params.keepalive_inc,
		  params.tcp_tx_timeout_val, params.tcp_rx_timeout_val);

	return hdd_set_app_type2_params(hHal, &params);
}
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

/**
 * hdd_parse_setmaxtxpower_command() - HDD Parse MAXTXPOWER command
 * @pValue:	Pointer to MAXTXPOWER command
 * @pDbm:	Pointer to tx power
 *
 * This function parses the MAXTXPOWER command passed in the format
 * MAXTXPOWER<space>X(Tx power in dbm)
 *
 * For example input commands:
 * 1) MAXTXPOWER -8 -> This is translated into set max TX power to -8 dbm
 * 2) MAXTXPOWER -23 -> This is translated into set max TX power to -23 dbm
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_setmaxtxpower_command(uint8_t *pValue, int *pTxPower)
{
	uint8_t *inPtr = pValue;
	int tempInt;
	int v = 0;
	*pTxPower = 0;

	inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr) {
		return -EINVAL;
	}

	/* no space after the command */
	else if (SPACE_ASCII_VALUE != *inPtr) {
		return -EINVAL;
	}

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;

	/* no argument followed by spaces */
	if ('\0' == *inPtr) {
		return 0;
	}

	v = kstrtos32(inPtr, 10, &tempInt);

	/* Range checking for passed parameter */
	if ((tempInt < HDD_MIN_TX_POWER) || (tempInt > HDD_MAX_TX_POWER)) {
		return -EINVAL;
	}

	*pTxPower = tempInt;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "SETMAXTXPOWER: %d", *pTxPower);

	return 0;
} /* End of hdd_parse_setmaxtxpower_command */

static int hdd_get_dwell_time(struct hdd_config *pCfg, uint8_t *command,
			      char *extra, uint8_t n, uint8_t *len)
{
	int ret = 0;

	if (!pCfg || !command || !extra || !len) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: argument passed for GETDWELLTIME is incorrect",
			  __func__);
		ret = -EINVAL;
		return ret;
	}

	if (strncmp(command, "GETDWELLTIME ACTIVE MAX", 23) == 0) {
		*len = scnprintf(extra, n, "GETDWELLTIME ACTIVE MAX %u\n",
				 (int)pCfg->nActiveMaxChnTime);
		return ret;
	} else if (strncmp(command, "GETDWELLTIME ACTIVE MIN", 23) == 0) {
		*len = scnprintf(extra, n, "GETDWELLTIME ACTIVE MIN %u\n",
				 (int)pCfg->nActiveMinChnTime);
		return ret;
	} else if (strncmp(command, "GETDWELLTIME PASSIVE MAX", 24) == 0) {
		*len = scnprintf(extra, n, "GETDWELLTIME PASSIVE MAX %u\n",
				 (int)pCfg->nPassiveMaxChnTime);
		return ret;
	} else if (strncmp(command, "GETDWELLTIME PASSIVE MIN", 24) == 0) {
		*len = scnprintf(extra, n, "GETDWELLTIME PASSIVE MIN %u\n",
				 (int)pCfg->nPassiveMinChnTime);
		return ret;
	} else if (strncmp(command, "GETDWELLTIME", 12) == 0) {
		*len = scnprintf(extra, n, "GETDWELLTIME %u \n",
				 (int)pCfg->nActiveMaxChnTime);
		return ret;
	} else {
		ret = -EINVAL;
	}

	return ret;
}

static int hdd_set_dwell_time(hdd_adapter_t *adapter, uint8_t *command)
{
	tHalHandle hHal;
	struct hdd_config *pCfg;
	uint8_t *value = command;
	tSmeConfigParams smeConfig;
	int val = 0, temp = 0;

	pCfg = (WLAN_HDD_GET_CTX(adapter))->config;
	hHal = WLAN_HDD_GET_HAL_CTX(adapter);
	if (!pCfg || !hHal) {
		hddLog(LOGE,
			FL("argument passed for SETDWELLTIME is incorrect"));
		return -EINVAL;
	}

	qdf_mem_zero(&smeConfig, sizeof(smeConfig));
	sme_get_config_param(hHal, &smeConfig);

	if (strncmp(command, "SETDWELLTIME ACTIVE MAX", 23) == 0) {
		value = value + 24;
		temp = kstrtou32(value, 10, &val);
		if (temp != 0 || val < CFG_ACTIVE_MAX_CHANNEL_TIME_MIN ||
		    val > CFG_ACTIVE_MAX_CHANNEL_TIME_MAX) {
			hddLog(LOGE,
				FL("argument passed for SETDWELLTIME ACTIVE MAX is incorrect"));
			return -EFAULT;
		}
		pCfg->nActiveMaxChnTime = val;
		smeConfig.csrConfig.nActiveMaxChnTime = val;
		sme_update_config(hHal, &smeConfig);
	} else if (strncmp(command, "SETDWELLTIME ACTIVE MIN", 23) == 0) {
		value = value + 24;
		temp = kstrtou32(value, 10, &val);
		if (temp != 0 || val < CFG_ACTIVE_MIN_CHANNEL_TIME_MIN ||
		    val > CFG_ACTIVE_MIN_CHANNEL_TIME_MAX) {
			hddLog(LOGE,
				FL("argument passed for SETDWELLTIME ACTIVE MIN is incorrect"));
			return -EFAULT;
		}
		pCfg->nActiveMinChnTime = val;
		smeConfig.csrConfig.nActiveMinChnTime = val;
		sme_update_config(hHal, &smeConfig);
	} else if (strncmp(command, "SETDWELLTIME PASSIVE MAX", 24) == 0) {
		value = value + 25;
		temp = kstrtou32(value, 10, &val);
		if (temp != 0 || val < CFG_PASSIVE_MAX_CHANNEL_TIME_MIN ||
		    val > CFG_PASSIVE_MAX_CHANNEL_TIME_MAX) {
			hddLog(LOGE,
				FL("argument passed for SETDWELLTIME PASSIVE MAX is incorrect"));
			return -EFAULT;
		}
		pCfg->nPassiveMaxChnTime = val;
		smeConfig.csrConfig.nPassiveMaxChnTime = val;
		sme_update_config(hHal, &smeConfig);
	} else if (strncmp(command, "SETDWELLTIME PASSIVE MIN", 24) == 0) {
		value = value + 25;
		temp = kstrtou32(value, 10, &val);
		if (temp != 0 || val < CFG_PASSIVE_MIN_CHANNEL_TIME_MIN ||
		    val > CFG_PASSIVE_MIN_CHANNEL_TIME_MAX) {
			hddLog(LOGE,
				FL("argument passed for SETDWELLTIME PASSIVE MIN is incorrect"));
			return -EFAULT;
		}
		pCfg->nPassiveMinChnTime = val;
		smeConfig.csrConfig.nPassiveMinChnTime = val;
		sme_update_config(hHal, &smeConfig);
	} else if (strncmp(command, "SETDWELLTIME", 12) == 0) {
		value = value + 13;
		temp = kstrtou32(value, 10, &val);
		if (temp != 0 || val < CFG_ACTIVE_MAX_CHANNEL_TIME_MIN ||
		    val > CFG_ACTIVE_MAX_CHANNEL_TIME_MAX) {
			hddLog(LOGE,
				FL("argument passed for SETDWELLTIME is incorrect"));
			return -EFAULT;
		}
		pCfg->nActiveMaxChnTime = val;
		smeConfig.csrConfig.nActiveMaxChnTime = val;
		sme_update_config(hHal, &smeConfig);
	} else {
		return -EINVAL;
	}

	return 0;
}

static void hdd_get_link_status_cb(uint8_t status, void *context)
{
	struct statsContext *pLinkContext;
	hdd_adapter_t *adapter;

	if (NULL == context) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: Bad context [%p]",
		       __func__, context);
		return;
	}

	pLinkContext = context;
	adapter = pLinkContext->pAdapter;

	spin_lock(&hdd_context_lock);

	if ((NULL == adapter) ||
	    (LINK_STATUS_MAGIC != pLinkContext->magic)) {
		/*
		 * the caller presumably timed out so there is
		 * nothing we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, adapter [%p] magic [%08x]",
		       __func__, adapter, pLinkContext->magic);
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pLinkContext->magic = 0;

	/* copy over the status */
	adapter->linkStatus = status;

	/* notify the caller */
	complete(&pLinkContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * wlan_hdd_get_link_status() - get link status
 * @pAdapter:     pointer to the adapter
 *
 * This function sends a request to query the link status and waits
 * on a timer to invoke the callback. if the callback is invoked then
 * latest link status shall be returned or otherwise cached value
 * will be returned.
 *
 * Return: On success, link status shall be returned.
 *         On error or not associated, link status 0 will be returned.
 */
static int wlan_hdd_get_link_status(hdd_adapter_t *adapter)
{

	hdd_station_ctx_t *pHddStaCtx =
				WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct statsContext context;
	QDF_STATUS hstatus;
	unsigned long rc;

	if (cds_is_driver_recovering()) {
		hdd_warn("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return 0;
	}

	if ((WLAN_HDD_INFRA_STATION != adapter->device_mode) &&
	    (WLAN_HDD_P2P_CLIENT != adapter->device_mode)) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return 0;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		/* If not associated, then expected link status return
		 * value is 0
		 */
		hddLog(LOG1, FL("Not associated!"));
		return 0;
	}

	init_completion(&context.completion);
	context.pAdapter = adapter;
	context.magic = LINK_STATUS_MAGIC;
	hstatus = sme_get_link_status(WLAN_HDD_GET_HAL_CTX(adapter),
				      hdd_get_link_status_cb,
				      &context, adapter->sessionId);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Unable to retrieve link status", __func__);
		/* return a cached value */
	} else {
		/* request is sent -- wait for the response */
		rc = wait_for_completion_timeout(&context.completion,
				msecs_to_jiffies(WLAN_WAIT_TIME_LINK_STATUS));
		if (!rc)
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("SME timed out while retrieving link status"));
	}

	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	/* either callback updated adapter stats or it has cached data */
	return adapter->linkStatus;
}

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
/**
 * hdd_parse_ese_beacon_req() - Parse ese beacon request
 * @pValue:	Pointer to data
 * @pEseBcnReq:	Output pointer to store parsed ie information
 *
 * This function parses the ese beacon request passed in the format
 * CCXBEACONREQ<space><Number of fields><space><Measurement token>
 * <space>Channel 1<space>Scan Mode <space>Meas Duration<space>Channel N
 * <space>Scan Mode N<space>Meas Duration N
 *
 * If the Number of bcn req fields (N) does not match with the
 * actual number of fields passed then take N.
 * <Meas Token><Channel><Scan Mode> and <Meas Duration> are treated
 * as one pair. For example, CCXBEACONREQ 2 1 1 1 30 2 44 0 40.
 * This function does not take care of removing duplicate channels from the
 * list
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_ese_beacon_req(uint8_t *pValue,
					tCsrEseBeaconReq *pEseBcnReq)
{
	uint8_t *inPtr = pValue;
	int tempInt = 0;
	int j = 0, i = 0, v = 0;
	char buf[32];

	inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr) {
		return -EINVAL;
	}
	/* no space after the command */
	else if (SPACE_ASCII_VALUE != *inPtr) {
		return -EINVAL;
	}

	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;

	/* no argument followed by spaces */
	if ('\0' == *inPtr)
		return -EINVAL;

	/* get the first argument ie measurement token */
	v = sscanf(inPtr, "%31s ", buf);
	if (1 != v)
		return -EINVAL;

	v = kstrtos32(buf, 10, &tempInt);
	if (v < 0)
		return -EINVAL;

	pEseBcnReq->numBcnReqIe = tempInt;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "Number of Bcn Req Ie fields(%d)", pEseBcnReq->numBcnReqIe);

	for (j = 0; j < (pEseBcnReq->numBcnReqIe); j++) {
		for (i = 0; i < 4; i++) {
			/*
			 * inPtr pointing to the beginning of 1st space
			 * after number of ie fields
			 */
			inPtr = strpbrk(inPtr, " ");
			/* no ie data after the number of ie fields argument */
			if (NULL == inPtr)
				return -EINVAL;

			/* remove empty space */
			while ((SPACE_ASCII_VALUE == *inPtr)
			       && ('\0' != *inPtr))
				inPtr++;

			/*
			 * no ie data after the number of ie fields
			 * argument and spaces
			 */
			if ('\0' == *inPtr)
				return -EINVAL;

			v = sscanf(inPtr, "%31s ", buf);
			if (1 != v)
				return -EINVAL;

			v = kstrtos32(buf, 10, &tempInt);
			if (v < 0)
				return -EINVAL;

			switch (i) {
			case 0: /* Measurement token */
				if (tempInt <= 0) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "Invalid Measurement Token(%d)",
						  tempInt);
					return -EINVAL;
				}
				pEseBcnReq->bcnReq[j].measurementToken =
					tempInt;
				break;

			case 1: /* Channel number */
				if ((tempInt <= 0) ||
				    (tempInt >
				     WNI_CFG_CURRENT_CHANNEL_STAMAX)) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "Invalid Channel Number(%d)",
						  tempInt);
					return -EINVAL;
				}
				pEseBcnReq->bcnReq[j].channel = tempInt;
				break;

			case 2: /* Scan mode */
				if ((tempInt < eSIR_PASSIVE_SCAN)
				    || (tempInt > eSIR_BEACON_TABLE)) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "Invalid Scan Mode(%d) Expected{0|1|2}",
						  tempInt);
					return -EINVAL;
				}
				pEseBcnReq->bcnReq[j].scanMode = tempInt;
				break;

			case 3: /* Measurement duration */
				if (((tempInt <= 0)
				     && (pEseBcnReq->bcnReq[j].scanMode !=
					 eSIR_BEACON_TABLE)) ||
				    ((tempInt < 0) &&
				     (pEseBcnReq->bcnReq[j].scanMode ==
						eSIR_BEACON_TABLE))) {
					QDF_TRACE(QDF_MODULE_ID_HDD,
						  QDF_TRACE_LEVEL_ERROR,
						  "Invalid Measurement Duration(%d)",
						  tempInt);
					return -EINVAL;
				}
				pEseBcnReq->bcnReq[j].measurementDuration =
					tempInt;
				break;
			}
		}
	}

	for (j = 0; j < pEseBcnReq->numBcnReqIe; j++) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "Index(%d) Measurement Token(%u) Channel(%u) Scan Mode(%u) Measurement Duration(%u)",
			  j,
			  pEseBcnReq->bcnReq[j].measurementToken,
			  pEseBcnReq->bcnReq[j].channel,
			  pEseBcnReq->bcnReq[j].scanMode,
			  pEseBcnReq->bcnReq[j].measurementDuration);
	}

	return 0;
}
#endif /* defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD) */

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
/**
 * hdd_parse_get_cckm_ie() - HDD Parse and fetch the CCKM IE
 * @pValue:	Pointer to input data
 * @pCckmIe:	Pointer to output cckm Ie
 * @pCckmIeLen:	Pointer to output cckm ie length
 *
 * This function parses the SETCCKM IE command
 * SETCCKMIE<space><ie data>
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_parse_get_cckm_ie(uint8_t *pValue, uint8_t **pCckmIe,
				 uint8_t *pCckmIeLen)
{
	uint8_t *inPtr = pValue;
	uint8_t *dataEnd;
	int j = 0;
	int i = 0;
	uint8_t tempByte = 0;
	inPtr = strnchr(pValue, strlen(pValue), SPACE_ASCII_VALUE);
	/* no argument after the command */
	if (NULL == inPtr) {
		return -EINVAL;
	}
	/* no space after the command */
	else if (SPACE_ASCII_VALUE != *inPtr) {
		return -EINVAL;
	}
	/* remove empty spaces */
	while ((SPACE_ASCII_VALUE == *inPtr) && ('\0' != *inPtr))
		inPtr++;
	/* no argument followed by spaces */
	if ('\0' == *inPtr) {
		return -EINVAL;
	}
	/* find the length of data */
	dataEnd = inPtr;
	while (('\0' != *dataEnd)) {
		dataEnd++;
		++(*pCckmIeLen);
	}
	if (*pCckmIeLen <= 0)
		return -EINVAL;
	/*
	 * Allocate the number of bytes based on the number of input characters
	 * whether it is even or odd.
	 * if the number of input characters are even, then we need N / 2 byte.
	 * if the number of input characters are odd, then we need do
	 * (N + 1) / 2 to compensate rounding off.
	 * For example, if N = 18, then (18 + 1) / 2 = 9 bytes are enough.
	 * If N = 19, then we need 10 bytes, hence (19 + 1) / 2 = 10 bytes
	 */
	*pCckmIe = qdf_mem_malloc((*pCckmIeLen + 1) / 2);
	if (NULL == *pCckmIe) {
		hddLog(LOGE, FL("cdf_mem_alloc failed"));
		return -ENOMEM;
	}
	qdf_mem_zero(*pCckmIe, (*pCckmIeLen + 1) / 2);
	/*
	 * the buffer received from the upper layer is character buffer,
	 * we need to prepare the buffer taking 2 characters in to a U8 hex
	 * decimal number for example 7f0000f0...form a buffer to contain
	 * 7f in 0th location, 00 in 1st and f0 in 3rd location
	 */
	for (i = 0, j = 0; j < *pCckmIeLen; j += 2) {
		tempByte = (hex_to_bin(inPtr[j]) << 4) |
			   (hex_to_bin(inPtr[j + 1]));
		(*pCckmIe)[i++] = tempByte;
	}
	*pCckmIeLen = i;
	return 0;
}
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */

int wlan_hdd_set_mc_rate(hdd_adapter_t *pAdapter, int targetRate)
{
	tSirRateUpdateInd rateUpdate = {0};
	QDF_STATUS status;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	struct hdd_config *pConfig = NULL;

	if (pHddCtx == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: HDD context is null", __func__);
		return -EINVAL;
	}
	if ((WLAN_HDD_IBSS != pAdapter->device_mode) &&
	    (WLAN_HDD_SOFTAP != pAdapter->device_mode) &&
	    (WLAN_HDD_INFRA_STATION != pAdapter->device_mode)) {
		hddLog(LOGE,
			FL("Received SETMCRATE cmd in invalid mode %s(%d)"),
			hdd_device_mode_to_string(pAdapter->device_mode),
			pAdapter->device_mode);
		hddLog(LOGE,
			FL("SETMCRATE cmd is allowed only in STA, IBSS or SOFTAP mode"));
		return -EINVAL;
	}
	pConfig = pHddCtx->config;
	rateUpdate.nss = (pConfig->enable2x2 == 0) ? 0 : 1;
	rateUpdate.dev_mode = pAdapter->device_mode;
	rateUpdate.mcastDataRate24GHz = targetRate;
	rateUpdate.mcastDataRate24GHzTxFlag = 1;
	rateUpdate.mcastDataRate5GHz = targetRate;
	rateUpdate.bcastDataRate = -1;
	qdf_copy_macaddr(&rateUpdate.bssid, &pAdapter->macAddressCurrent);
	hddLog(LOG1,
		FL("MC Target rate %d, mac = %pM, dev_mode %s(%d)"),
		rateUpdate.mcastDataRate24GHz, rateUpdate.bssid.bytes,
		hdd_device_mode_to_string(pAdapter->device_mode),
		pAdapter->device_mode);
	status = sme_send_rate_update_ind(pHddCtx->hHal, &rateUpdate);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: SETMCRATE failed",
		       __func__);
		return -EFAULT;
	}
	return 0;
}

static int drv_cmd_p2p_dev_addr(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_P2P_DEV_ADDR_IOCTL,
			 adapter->sessionId,
			(unsigned)(*(hdd_ctx->p2pDeviceAddress.bytes + 2)
				<< 24 | *(hdd_ctx->p2pDeviceAddress.bytes
				+ 3) << 16 | *(hdd_ctx->
				p2pDeviceAddress.bytes + 4) << 8 |
				*(hdd_ctx->p2pDeviceAddress.bytes +
				5))));

	if (copy_to_user(priv_data->buf, hdd_ctx->p2pDeviceAddress.bytes,
			 sizeof(tSirMacAddr))) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: failed to copy data to user buffer",
		       __func__);
		ret = -EFAULT;
	}

	return ret;
}

/**
 * drv_cmd_p2p_set_noa() - Handler for P2P_SET_NOA driver command
 * @adapter: Adapter on which the command was received
 * @hdd_ctx: HDD global context
 * @command: Entire driver command received from userspace
 * @command_len: Length of @command
 * @priv_data: Pointer to ioctl private data structure
 *
 * This is a trivial command hander function which simply forwards the
 * command to the actual command processor within the P2P module.
 *
 * Return: 0 on success, non-zero on failure
 */
static int drv_cmd_p2p_set_noa(hdd_adapter_t *adapter,
			       hdd_context_t *hdd_ctx,
			       uint8_t *command,
			       uint8_t command_len,
			       hdd_priv_data_t *priv_data)
{
	return hdd_set_p2p_noa(adapter->dev, command);
}

/**
 * drv_cmd_p2p_set_ps() - Handler for P2P_SET_PS driver command
 * @adapter: Adapter on which the command was received
 * @hdd_ctx: HDD global context
 * @command: Entire driver command received from userspace
 * @command_len: Length of @command
 * @priv_data: Pointer to ioctl private data structure
 *
 * This is a trivial command hander function which simply forwards the
 * command to the actual command processor within the P2P module.
 *
 * Return: 0 on success, non-zero on failure
 */
static int drv_cmd_p2p_set_ps(hdd_adapter_t *adapter,
			      hdd_context_t *hdd_ctx,
			      uint8_t *command,
			      uint8_t command_len,
			      hdd_priv_data_t *priv_data)
{
	return hdd_set_p2p_opps(adapter->dev, command);
}

static int drv_cmd_set_band(hdd_adapter_t *adapter,
			    hdd_context_t *hdd_ctx,
			    uint8_t *command,
			    uint8_t command_len,
			    hdd_priv_data_t *priv_data)
{
	int ret = 0;

	uint8_t *ptr = command;

	/* Change band request received */

	/*
	 * First 8 bytes will have "SETBAND " and
	 * 9 byte will have band setting value
	 */
	hddLog(QDF_TRACE_LEVEL_INFO,
	       "%s: SetBandCommand Info  comm %s UL %d, TL %d",
	       __func__, command, priv_data->used_len,
	       priv_data->total_len);

	/* Change band request received */
	ret = hdd_set_band_helper(adapter->dev, ptr);

	return ret;
}

static int drv_cmd_set_wmmps(hdd_adapter_t *adapter,
			     hdd_context_t *hdd_ctx,
			     uint8_t *command,
			     uint8_t command_len,
			     hdd_priv_data_t *priv_data)
{
	return hdd_wmmps_helper(adapter, command);
}

static int drv_cmd_country(hdd_adapter_t *adapter,
			   hdd_context_t *hdd_ctx,
			   uint8_t *command,
			   uint8_t command_len,
			   hdd_priv_data_t *priv_data)
{
	int ret = 0;
	QDF_STATUS status;
	unsigned long rc;
	char *country_code;

	country_code = command + 8;

	INIT_COMPLETION(adapter->change_country_code);

	status = sme_change_country_code(hdd_ctx->hHal,
			wlan_hdd_change_country_code_callback,
			country_code,
			adapter,
			hdd_ctx->pcds_context,
			eSIR_TRUE,
			eSIR_TRUE);
	if (status == QDF_STATUS_SUCCESS) {
		rc = wait_for_completion_timeout(
			&adapter->change_country_code,
			 msecs_to_jiffies(WLAN_WAIT_TIME_COUNTRY));
		if (!rc)
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: SME while setting country code timed out",
			       __func__);
	} else {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: SME Change Country code fail, status=%d",
		       __func__, status);
		ret = -EINVAL;
	}

	return ret;
}

static int drv_cmd_set_roam_trigger(hdd_adapter_t *adapter,
				    hdd_context_t *hdd_ctx,
				    uint8_t *command,
				    uint8_t command_len,
				    hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	int8_t rssi = 0;
	uint8_t lookUpThreshold = CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_DEFAULT;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* Move pointer to ahead of SETROAMTRIGGER<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtos8(value, 10, &rssi);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
			  __func__,
			  CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
			  CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX);
		ret = -EINVAL;
		goto exit;
	}

	lookUpThreshold = abs(rssi);

	if ((lookUpThreshold < CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN)
	    || (lookUpThreshold > CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Neighbor lookup threshold value %d is out of range (Min: %d Max: %d)",
			  lookUpThreshold,
			  CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MIN,
			  CFG_NEIGHBOR_LOOKUP_RSSI_THRESHOLD_MAX);
		ret = -EINVAL;
		goto exit;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_SETROAMTRIGGER_IOCTL,
			 adapter->sessionId, lookUpThreshold));
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set Roam trigger (Neighbor lookup threshold) = %d",
		  __func__,
		  lookUpThreshold);

	hdd_ctx->config->nNeighborLookupRssiThreshold = lookUpThreshold;
	status = sme_set_neighbor_lookup_rssi_threshold(hdd_ctx->hHal,
							adapter->sessionId,
							lookUpThreshold);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to set roam trigger, try again",
			  __func__);
		ret = -EPERM;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_get_roam_trigger(hdd_adapter_t *adapter,
				    hdd_context_t *hdd_ctx,
				    uint8_t *command,
				    uint8_t command_len,
				    hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t lookUpThreshold =
		sme_get_neighbor_lookup_rssi_threshold(hdd_ctx->hHal);
	int rssi = (-1) * lookUpThreshold;
	char extra[32];
	uint8_t len = 0;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETROAMTRIGGER_IOCTL,
			 adapter->sessionId, lookUpThreshold));

	len = scnprintf(extra, sizeof(extra), "%s %d", command, rssi);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_scan_period(hdd_adapter_t *adapter,
					hdd_context_t *hdd_ctx,
					uint8_t *command,
					uint8_t command_len,
					hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t roamScanPeriod = 0;
	uint16_t neighborEmptyScanRefreshPeriod =
		CFG_EMPTY_SCAN_REFRESH_PERIOD_DEFAULT;

	/* input refresh period is in terms of seconds */

	/* Move pointer to ahead of SETROAMSCANPERIOD<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &roamScanPeriod);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
			  __func__,
			  (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN / 1000),
			  (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX / 1000));
		ret = -EINVAL;
		goto exit;
	}

	if ((roamScanPeriod < (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN / 1000))
	    || (roamScanPeriod > (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX / 1000))) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Roam scan period value %d is out of range (Min: %d Max: %d)",
			  roamScanPeriod,
			  (CFG_EMPTY_SCAN_REFRESH_PERIOD_MIN / 1000),
			  (CFG_EMPTY_SCAN_REFRESH_PERIOD_MAX / 1000));
		ret = -EINVAL;
		goto exit;
	}
	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_SETROAMSCANPERIOD_IOCTL,
			 adapter->sessionId, roamScanPeriod));
	neighborEmptyScanRefreshPeriod = roamScanPeriod * 1000;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set roam scan period (Empty Scan refresh period) = %d",
		  __func__,
		  roamScanPeriod);

	hdd_ctx->config->nEmptyScanRefreshPeriod =
		neighborEmptyScanRefreshPeriod;
	sme_update_empty_scan_refresh_period(hdd_ctx->hHal,
					     adapter->sessionId,
					     neighborEmptyScanRefreshPeriod);

exit:
	return ret;
}

static int drv_cmd_get_roam_scan_period(hdd_adapter_t *adapter,
					hdd_context_t *hdd_ctx,
					uint8_t *command,
					uint8_t command_len,
					hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t nEmptyScanRefreshPeriod =
		sme_get_empty_scan_refresh_period(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETROAMSCANPERIOD_IOCTL,
			 adapter->sessionId,
			 nEmptyScanRefreshPeriod));
	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETROAMSCANPERIOD",
			(nEmptyScanRefreshPeriod / 1000));
	/* Returned value is in units of seconds */
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		hddLog(LOGE,
			FL("failed to copy data to user buffer"));
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_scan_refresh_period(hdd_adapter_t *adapter,
						hdd_context_t *hdd_ctx,
						uint8_t *command,
						uint8_t command_len,
						hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t roamScanRefreshPeriod = 0;
	uint16_t neighborScanRefreshPeriod =
		CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_DEFAULT;

	/* input refresh period is in terms of seconds */
	/* Move pointer to ahead of SETROAMSCANREFRESHPERIOD<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &roamScanRefreshPeriod);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed Input value may be out of range[%d - %d]",
			  __func__,
			  CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN / 1000,
			  CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX / 1000);
		ret = -EINVAL;
		goto exit;
	}

	if ((roamScanRefreshPeriod <
		(CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN / 1000))
	    || (roamScanRefreshPeriod >
		(CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX / 1000))) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Neighbor scan results refresh period value %d is out of range (Min: %d Max: %d)",
			  roamScanRefreshPeriod,
			  (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MIN
					   / 1000),
			  (CFG_NEIGHBOR_SCAN_RESULTS_REFRESH_PERIOD_MAX
					   / 1000));
		ret = -EINVAL;
		goto exit;
	}
	neighborScanRefreshPeriod = roamScanRefreshPeriod * 1000;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set roam scan refresh period (Scan refresh period) = %d",
		  __func__,
		  roamScanRefreshPeriod);

	hdd_ctx->config->nNeighborResultsRefreshPeriod =
				neighborScanRefreshPeriod;
	sme_set_neighbor_scan_refresh_period(hdd_ctx->hHal,
					     adapter->sessionId,
					     neighborScanRefreshPeriod);

exit:
	return ret;
}

static int drv_cmd_get_roam_scan_refresh_period(hdd_adapter_t *adapter,
						hdd_context_t *hdd_ctx,
						uint8_t *command,
						uint8_t command_len,
						hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t value =
		sme_get_neighbor_scan_refresh_period(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETROAMSCANREFRESHPERIOD",
			(value / 1000));
	/* Returned value is in units of seconds */
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_mode(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t roamMode = CFG_LFR_FEATURE_ENABLED_DEFAULT;

	/* Move pointer to ahead of SETROAMMODE<delimiter> */
	value = value + SIZE_OF_SETROAMMODE + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, SIZE_OF_SETROAMMODE, &roamMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_LFR_FEATURE_ENABLED_MIN,
			  CFG_LFR_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}
	if ((roamMode < CFG_LFR_FEATURE_ENABLED_MIN) ||
	    (roamMode > CFG_LFR_FEATURE_ENABLED_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Roam Mode value %d is out of range (Min: %d Max: %d)",
			  roamMode,
			  CFG_LFR_FEATURE_ENABLED_MIN,
			  CFG_LFR_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Received Command to Set Roam Mode = %d",
		  __func__, roamMode);
	/*
	 * Note that
	 *     SETROAMMODE 0 is to enable LFR while
	 *     SETROAMMODE 1 is to disable LFR, but
	 *     notify_is_fast_roam_ini_feature_enabled 0/1 is to
	 *     enable/disable. So, we have to invert the value
	 *     to call sme_update_is_fast_roam_ini_feature_enabled.
	 */
	if (CFG_LFR_FEATURE_ENABLED_MIN == roamMode)
		roamMode = CFG_LFR_FEATURE_ENABLED_MAX;  /* Roam enable */
	else
		roamMode = CFG_LFR_FEATURE_ENABLED_MIN;  /* Roam disable */

	hdd_ctx->config->isFastRoamIniFeatureEnabled = roamMode;
	if (roamMode) {
		hdd_ctx->config->isRoamOffloadScanEnabled = roamMode;
		sme_update_roam_scan_offload_enabled(
			(tHalHandle)(hdd_ctx->hHal),
			hdd_ctx->config->isRoamOffloadScanEnabled);
		sme_update_is_fast_roam_ini_feature_enabled(
			hdd_ctx->hHal,
			adapter->sessionId,
			roamMode);
	} else {
		sme_update_is_fast_roam_ini_feature_enabled(
			hdd_ctx->hHal,
			adapter->sessionId,
			roamMode);
		hdd_ctx->config->isRoamOffloadScanEnabled = roamMode;
		sme_update_roam_scan_offload_enabled(
			(tHalHandle)(hdd_ctx->hHal),
			hdd_ctx->config->isRoamOffloadScanEnabled);
	}


exit:
	return ret;
}

static int drv_cmd_get_roam_mode(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool roamMode = sme_get_is_lfr_feature_enabled(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	/*
	 * roamMode value shall be inverted because the sementics is different.
	 */
	if (CFG_LFR_FEATURE_ENABLED_MIN == roamMode)
		roamMode = CFG_LFR_FEATURE_ENABLED_MAX;
	else
		roamMode = CFG_LFR_FEATURE_ENABLED_MIN;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, roamMode);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_delta(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t roamRssiDiff = CFG_ROAM_RSSI_DIFF_DEFAULT;

	/* Move pointer to ahead of SETROAMDELTA<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &roamRssiDiff);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_ROAM_RSSI_DIFF_MIN,
			  CFG_ROAM_RSSI_DIFF_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((roamRssiDiff < CFG_ROAM_RSSI_DIFF_MIN) ||
	    (roamRssiDiff > CFG_ROAM_RSSI_DIFF_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Roam rssi diff value %d is out of range (Min: %d Max: %d)",
			  roamRssiDiff,
			  CFG_ROAM_RSSI_DIFF_MIN,
			  CFG_ROAM_RSSI_DIFF_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set roam rssi diff = %d",
		  __func__, roamRssiDiff);

	hdd_ctx->config->RoamRssiDiff = roamRssiDiff;
	sme_update_roam_rssi_diff(hdd_ctx->hHal,
				  adapter->sessionId,
				  roamRssiDiff);

exit:
	return ret;
}

static int drv_cmd_get_roam_delta(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t roamRssiDiff =
		sme_get_roam_rssi_diff(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETROAMDELTA_IOCTL,
			 adapter->sessionId, roamRssiDiff));

	len = scnprintf(extra, sizeof(extra), "%s %d",
			command, roamRssiDiff);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_get_band(hdd_adapter_t *adapter,
			    hdd_context_t *hdd_ctx,
			    uint8_t *command,
			    uint8_t command_len,
			    hdd_priv_data_t *priv_data)
{
	int ret = 0;
	int band = -1;
	char extra[32];
	uint8_t len = 0;

	hdd_get_band_helper(hdd_ctx, &band);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETBAND_IOCTL,
			 adapter->sessionId, band));

	len = scnprintf(extra, sizeof(extra), "%s %d", command, band);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_scan_channels(hdd_adapter_t *adapter,
					  hdd_context_t *hdd_ctx,
					  uint8_t *command,
					  uint8_t command_len,
					  hdd_priv_data_t *priv_data)
{
	return hdd_parse_set_roam_scan_channels(adapter, command);
}

static int drv_cmd_get_roam_scan_channels(hdd_adapter_t *adapter,
					  hdd_context_t *hdd_ctx,
					  uint8_t *command,
					  uint8_t command_len,
					  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t ChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t numChannels = 0;
	uint8_t j = 0;
	char extra[128] = { 0 };
	int len;

	if (QDF_STATUS_SUCCESS !=
		sme_get_roam_scan_channel_list(hdd_ctx->hHal,
					       ChannelList,
					       &numChannels,
					       adapter->sessionId)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_FATAL,
			  "%s: failed to get roam scan channel list",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETROAMSCANCHANNELS_IOCTL,
			 adapter->sessionId, numChannels));
	/*
	 * output channel list is of the format
	 * [Number of roam scan channels][Channel1][Channel2]...
	 * copy the number of channels in the 0th index
	 */
	len = scnprintf(extra, sizeof(extra), "%s %d", command,
			numChannels);
	for (j = 0; (j < numChannels); j++)
		len += scnprintf(extra + len, sizeof(extra) - len,
				 " %d", ChannelList[j]);

	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_get_ccx_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool eseMode = sme_get_is_ese_feature_enabled(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	/*
	 * Check if the features OKC/ESE/11R are supported simultaneously,
	 * then this operation is not permitted (return FAILURE)
	 */
	if (eseMode &&
	    hdd_is_okc_mode_enabled(hdd_ctx) &&
	    sme_get_is_ft_feature_enabled(hdd_ctx->hHal)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_WARN,
			  "%s: OKC/ESE/11R are supported simultaneously hence this operation is not permitted!",
			  __func__);
		ret = -EPERM;
		goto exit;
	}

	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETCCXMODE", eseMode);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_get_okc_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool okcMode = hdd_is_okc_mode_enabled(hdd_ctx);
	char extra[32];
	uint8_t len = 0;

	/*
	 * Check if the features OKC/ESE/11R are supported simultaneously,
	 * then this operation is not permitted (return FAILURE)
	 */
	if (okcMode &&
	    sme_get_is_ese_feature_enabled(hdd_ctx->hHal) &&
	    sme_get_is_ft_feature_enabled(hdd_ctx->hHal)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_WARN,
			  "%s: OKC/ESE/11R are supported simultaneously hence this operation is not permitted!",
			  __func__);
		ret = -EPERM;
		goto exit;
	}

	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETOKCMODE", okcMode);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_get_fast_roam(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool lfrMode = sme_get_is_lfr_feature_enabled(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETFASTROAM", lfrMode);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_get_fast_transition(hdd_adapter_t *adapter,
				       hdd_context_t *hdd_ctx,
				       uint8_t *command,
				       uint8_t command_len,
				       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool ft = sme_get_is_ft_feature_enabled(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d",
					"GETFASTTRANSITION", ft);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_scan_channel_min_time(hdd_adapter_t *adapter,
						  hdd_context_t *hdd_ctx,
						  uint8_t *command,
						  uint8_t command_len,
						  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t minTime = CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_DEFAULT;

	/* Move pointer to ahead of SETROAMSCANCHANNELMINTIME<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &minTime);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__,
			  CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN,
			  CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((minTime < CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN) ||
	    (minTime > CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "scan min channel time value %d is out of range (Min: %d Max: %d)",
			  minTime,
			  CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MIN,
			  CFG_NEIGHBOR_SCAN_MIN_CHAN_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_SETROAMSCANCHANNELMINTIME_IOCTL,
			 adapter->sessionId, minTime));
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change channel min time = %d",
		  __func__, minTime);

	hdd_ctx->config->nNeighborScanMinChanTime = minTime;
	sme_set_neighbor_scan_min_chan_time(hdd_ctx->hHal,
					    minTime,
					    adapter->sessionId);

exit:
	return ret;
}

static int drv_cmd_send_action_frame(hdd_adapter_t *adapter,
				     hdd_context_t *hdd_ctx,
				     uint8_t *command,
				     uint8_t command_len,
				     hdd_priv_data_t *priv_data)
{
	return hdd_parse_sendactionframe(adapter, command);
}

static int drv_cmd_get_roam_scan_channel_min_time(hdd_adapter_t *adapter,
						  hdd_context_t *hdd_ctx,
						  uint8_t *command,
						  uint8_t command_len,
						  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t val = sme_get_neighbor_scan_min_chan_time(hdd_ctx->hHal,
						adapter->sessionId);
	char extra[32];
	uint8_t len = 0;

	/* value is interms of msec */
	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETROAMSCANCHANNELMINTIME", val);
	len = QDF_MIN(priv_data->total_len, len + 1);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_GETROAMSCANCHANNELMINTIME_IOCTL,
			 adapter->sessionId, val));

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_scan_channel_time(hdd_adapter_t *adapter,
					 hdd_context_t *hdd_ctx,
					 uint8_t *command,
					 uint8_t command_len,
					 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint16_t maxTime = CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_DEFAULT;

	/* Move pointer to ahead of SETSCANCHANNELTIME<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou16(value, 10, &maxTime);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou16 failed range [%d - %d]",
			  __func__,
			  CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN,
			  CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((maxTime < CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN) ||
	    (maxTime > CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "lfr mode value %d is out of range (Min: %d Max: %d)",
			  maxTime,
			  CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MIN,
			  CFG_NEIGHBOR_SCAN_MAX_CHAN_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change channel max time = %d",
		  __func__, maxTime);

	hdd_ctx->config->nNeighborScanMaxChanTime = maxTime;
	sme_set_neighbor_scan_max_chan_time(hdd_ctx->hHal,
					    adapter->sessionId,
					    maxTime);

exit:
	return ret;
}

static int drv_cmd_get_scan_channel_time(hdd_adapter_t *adapter,
					 hdd_context_t *hdd_ctx,
					 uint8_t *command,
					 uint8_t command_len,
					 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t val = sme_get_neighbor_scan_max_chan_time(hdd_ctx->hHal,
				adapter->sessionId);
	char extra[32];
	uint8_t len = 0;

	/* value is interms of msec */
	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETSCANCHANNELTIME", val);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
			 QDF_TRACE(QDF_MODULE_ID_HDD,
			 QDF_TRACE_LEVEL_ERROR,
			 "%s: failed to copy data to user buffer",
			 __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_scan_home_time(hdd_adapter_t *adapter,
				      hdd_context_t *hdd_ctx,
				      uint8_t *command,
				      uint8_t command_len,
				      hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint16_t val = CFG_NEIGHBOR_SCAN_TIMER_PERIOD_DEFAULT;

	/* Move pointer to ahead of SETSCANHOMETIME<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou16(value, 10, &val);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou16 failed range [%d - %d]",
			  __func__,
			  CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
			  CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((val < CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN) ||
	    (val > CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "scan home time value %d is out of range (Min: %d Max: %d)",
			  val,
			  CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MIN,
			  CFG_NEIGHBOR_SCAN_TIMER_PERIOD_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change scan home time = %d",
		  __func__, val);

	hdd_ctx->config->nNeighborScanPeriod = val;
	sme_set_neighbor_scan_period(hdd_ctx->hHal,
				     adapter->sessionId, val);

exit:
	return ret;
}

static int drv_cmd_get_scan_home_time(hdd_adapter_t *adapter,
				      hdd_context_t *hdd_ctx,
				      uint8_t *command,
				      uint8_t command_len,
				      hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t val = sme_get_neighbor_scan_period(hdd_ctx->hHal,
						    adapter->
						    sessionId);
	char extra[32];
	uint8_t len = 0;

	/* value is interms of msec */
	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETSCANHOMETIME", val);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_intra_band(hdd_adapter_t *adapter,
				       hdd_context_t *hdd_ctx,
				       uint8_t *command,
				       uint8_t command_len,
				       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t val = CFG_ROAM_INTRA_BAND_DEFAULT;

	/* Move pointer to ahead of SETROAMINTRABAND<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &val);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_ROAM_INTRA_BAND_MIN,
			  CFG_ROAM_INTRA_BAND_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((val < CFG_ROAM_INTRA_BAND_MIN) ||
	    (val > CFG_ROAM_INTRA_BAND_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "intra band mode value %d is out of range (Min: %d Max: %d)",
			  val,
			  CFG_ROAM_INTRA_BAND_MIN,
			  CFG_ROAM_INTRA_BAND_MAX);
		ret = -EINVAL;
		goto exit;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change intra band = %d",
		  __func__, val);

	hdd_ctx->config->nRoamIntraBand = val;
	sme_set_roam_intra_band(hdd_ctx->hHal, val);

exit:
	return ret;
}

static int drv_cmd_get_roam_intra_band(hdd_adapter_t *adapter,
				       hdd_context_t *hdd_ctx,
				       uint8_t *command,
				       uint8_t command_len,
				       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t val = sme_get_roam_intra_band(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	/* value is interms of msec */
	len = scnprintf(extra, sizeof(extra), "%s %d",
			"GETROAMINTRABAND", val);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_scan_n_probes(hdd_adapter_t *adapter,
				     hdd_context_t *hdd_ctx,
				     uint8_t *command,
				     uint8_t command_len,
				     hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t nProbes = CFG_ROAM_SCAN_N_PROBES_DEFAULT;

	/* Move pointer to ahead of SETSCANNPROBES<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &nProbes);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_ROAM_SCAN_N_PROBES_MIN,
			  CFG_ROAM_SCAN_N_PROBES_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((nProbes < CFG_ROAM_SCAN_N_PROBES_MIN) ||
	    (nProbes > CFG_ROAM_SCAN_N_PROBES_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "NProbes value %d is out of range (Min: %d Max: %d)",
			  nProbes,
			  CFG_ROAM_SCAN_N_PROBES_MIN,
			  CFG_ROAM_SCAN_N_PROBES_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set nProbes = %d",
		  __func__, nProbes);

	hdd_ctx->config->nProbes = nProbes;
	sme_update_roam_scan_n_probes(hdd_ctx->hHal,
				      adapter->sessionId, nProbes);

exit:
	return ret;
}

static int drv_cmd_get_scan_n_probes(hdd_adapter_t *adapter,
				     hdd_context_t *hdd_ctx,
				     uint8_t *command,
				     uint8_t command_len,
				     hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t val = sme_get_roam_scan_n_probes(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_scan_home_away_time(hdd_adapter_t *adapter,
					   hdd_context_t *hdd_ctx,
					   uint8_t *command,
					   uint8_t command_len,
					   hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint16_t homeAwayTime = CFG_ROAM_SCAN_HOME_AWAY_TIME_DEFAULT;

	/* input value is in units of msec */

	/* Move pointer to ahead of SETSCANHOMEAWAYTIME<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou16(value, 10, &homeAwayTime);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__,
			  CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN,
			  CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((homeAwayTime < CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN) ||
	    (homeAwayTime > CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "homeAwayTime value %d is out of range (Min: %d Max: %d)",
			  homeAwayTime,
			  CFG_ROAM_SCAN_HOME_AWAY_TIME_MIN,
			  CFG_ROAM_SCAN_HOME_AWAY_TIME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set scan away time = %d",
		  __func__, homeAwayTime);

	if (hdd_ctx->config->nRoamScanHomeAwayTime !=
	    homeAwayTime) {
		hdd_ctx->config->nRoamScanHomeAwayTime = homeAwayTime;
		sme_update_roam_scan_home_away_time(hdd_ctx->hHal,
						    adapter->sessionId,
						    homeAwayTime,
						    true);
	}

exit:
	return ret;
}

static int drv_cmd_get_scan_home_away_time(hdd_adapter_t *adapter,
					   hdd_context_t *hdd_ctx,
					   uint8_t *command,
					   uint8_t command_len,
					   hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint16_t val = sme_get_roam_scan_home_away_time(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_reassoc(hdd_adapter_t *adapter,
			   hdd_context_t *hdd_ctx,
			   uint8_t *command,
			   uint8_t command_len,
			   hdd_priv_data_t *priv_data)
{
	return hdd_parse_reassoc(adapter, command);
}

static int drv_cmd_set_wes_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t wesMode = CFG_ENABLE_WES_MODE_NAME_DEFAULT;

	/* Move pointer to ahead of SETWESMODE<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &wesMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__,
			  CFG_ENABLE_WES_MODE_NAME_MIN,
			  CFG_ENABLE_WES_MODE_NAME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((wesMode < CFG_ENABLE_WES_MODE_NAME_MIN) ||
	    (wesMode > CFG_ENABLE_WES_MODE_NAME_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "WES Mode value %d is out of range (Min: %d Max: %d)",
			  wesMode,
			  CFG_ENABLE_WES_MODE_NAME_MIN,
			  CFG_ENABLE_WES_MODE_NAME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set WES Mode rssi diff = %d",
		  __func__, wesMode);

	hdd_ctx->config->isWESModeEnabled = wesMode;
	sme_update_wes_mode(hdd_ctx->hHal, wesMode, adapter->sessionId);

exit:
	return ret;
}

static int drv_cmd_get_wes_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool wesMode = sme_get_wes_mode(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, wesMode);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_opportunistic_rssi_diff(hdd_adapter_t *adapter,
					       hdd_context_t *hdd_ctx,
					       uint8_t *command,
					       uint8_t command_len,
					       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t nOpportunisticThresholdDiff =
		CFG_OPPORTUNISTIC_SCAN_THRESHOLD_DIFF_DEFAULT;

	/* Move pointer to ahead of SETOPPORTUNISTICRSSIDIFF<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &nOpportunisticThresholdDiff);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed.", __func__);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set Opportunistic Threshold diff = %d",
		  __func__, nOpportunisticThresholdDiff);

	sme_set_roam_opportunistic_scan_threshold_diff(hdd_ctx->hHal,
				adapter->sessionId,
				nOpportunisticThresholdDiff);

exit:
	return ret;
}

static int drv_cmd_get_opportunistic_rssi_diff(hdd_adapter_t *adapter,
					       hdd_context_t *hdd_ctx,
					       uint8_t *command,
					       uint8_t command_len,
					       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	int8_t val = sme_get_roam_opportunistic_scan_threshold_diff(
			hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_roam_rescan_rssi_diff(hdd_adapter_t *adapter,
					     hdd_context_t *hdd_ctx,
					     uint8_t *command,
					     uint8_t command_len,
					     hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t nRoamRescanRssiDiff = CFG_ROAM_RESCAN_RSSI_DIFF_DEFAULT;

	/* Move pointer to ahead of SETROAMRESCANRSSIDIFF<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &nRoamRescanRssiDiff);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed.", __func__);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set Roam Rescan RSSI Diff = %d",
		  __func__, nRoamRescanRssiDiff);

	sme_set_roam_rescan_rssi_diff(hdd_ctx->hHal,
				      adapter->sessionId,
				      nRoamRescanRssiDiff);

exit:
	return ret;
}

static int drv_cmd_get_roam_rescan_rssi_diff(hdd_adapter_t *adapter,
					     hdd_context_t *hdd_ctx,
					     uint8_t *command,
					     uint8_t command_len,
					     hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t val = sme_get_roam_rescan_rssi_diff(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, val);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_set_fast_roam(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t lfrMode = CFG_LFR_FEATURE_ENABLED_DEFAULT;

	/* Move pointer to ahead of SETFASTROAM<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &lfrMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_LFR_FEATURE_ENABLED_MIN,
			  CFG_LFR_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((lfrMode < CFG_LFR_FEATURE_ENABLED_MIN) ||
	    (lfrMode > CFG_LFR_FEATURE_ENABLED_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "lfr mode value %d is out of range (Min: %d Max: %d)",
			  lfrMode,
			  CFG_LFR_FEATURE_ENABLED_MIN,
			  CFG_LFR_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change lfr mode = %d",
		  __func__, lfrMode);

	hdd_ctx->config->isFastRoamIniFeatureEnabled = lfrMode;
	sme_update_is_fast_roam_ini_feature_enabled(hdd_ctx->hHal,
						    adapter->
						    sessionId,
						    lfrMode);

exit:
	return ret;
}

static int drv_cmd_set_fast_transition(hdd_adapter_t *adapter,
				       hdd_context_t *hdd_ctx,
				       uint8_t *command,
				       uint8_t command_len,
				       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t ft = CFG_FAST_TRANSITION_ENABLED_NAME_DEFAULT;

	/* Move pointer to ahead of SETFASTROAM<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &ft);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__,
			  CFG_FAST_TRANSITION_ENABLED_NAME_MIN,
			  CFG_FAST_TRANSITION_ENABLED_NAME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((ft < CFG_FAST_TRANSITION_ENABLED_NAME_MIN) ||
	    (ft > CFG_FAST_TRANSITION_ENABLED_NAME_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "ft mode value %d is out of range (Min: %d Max: %d)",
			  ft,
			  CFG_FAST_TRANSITION_ENABLED_NAME_MIN,
			  CFG_FAST_TRANSITION_ENABLED_NAME_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change ft mode = %d",
		  __func__, ft);

	hdd_ctx->config->isFastTransitionEnabled = ft;
	sme_update_fast_transition_enabled(hdd_ctx->hHal, ft);

exit:
	return ret;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static void hdd_wma_send_fastreassoc_cmd(int sessionId, tSirMacAddr bssid,
							int channel)
{
	struct wma_roam_invoke_cmd *fastreassoc;
	cds_msg_t msg = {0};

	fastreassoc = qdf_mem_malloc(sizeof(*fastreassoc));
	if (NULL == fastreassoc) {
		hddLog(LOGE, FL("cdf_mem_alloc failed for fastreassoc"));
		return;
	}
	fastreassoc->vdev_id = sessionId;
	fastreassoc->channel = channel;
	fastreassoc->bssid[0] = bssid[0];
	fastreassoc->bssid[1] = bssid[1];
	fastreassoc->bssid[2] = bssid[2];
	fastreassoc->bssid[3] = bssid[3];
	fastreassoc->bssid[4] = bssid[4];
	fastreassoc->bssid[5] = bssid[5];

	msg.type = SIR_HAL_ROAM_INVOKE;
	msg.reserved = 0;
	msg.bodyptr = fastreassoc;
	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA,
								&msg)) {
		qdf_mem_free(fastreassoc);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		FL("Not able to post ROAM_INVOKE_CMD message to WMA"));
	}
}
#endif
static int drv_cmd_fast_reassoc(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t channel = 0;
	tSirMacAddr targetApBssid;
	uint32_t roamId = 0;
	tCsrRoamModifyProfileFields modProfileFields;
#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
	tCsrHandoffRequest handoffInfo;
#endif
	hdd_station_ctx_t *pHddStaCtx;

	if (WLAN_HDD_INFRA_STATION != adapter->device_mode) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return -EINVAL;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* if not associated, no need to proceed with reassoc */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_INFO,
			  "%s:Not associated!", __func__);
		ret = -EINVAL;
		goto exit;
	}

	ret = hdd_parse_reassoc_command_v1_data(value, targetApBssid,
						&channel);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to parse reassoc command data",
			  __func__);
		goto exit;
	}

	/*
	 * if the target bssid is same as currently associated AP,
	 * issue reassoc to same AP
	 */
	if (true != qdf_mem_cmp(targetApBssid,
				    pHddStaCtx->conn_info.bssId.bytes,
				    QDF_MAC_ADDR_SIZE)) {
		/* Reassoc to same AP, only supported for Open Security*/
		if ((pHddStaCtx->conn_info.ucEncryptionType ||
			  pHddStaCtx->conn_info.mcEncryptionType)) {
			hddLog(LOGE,
			  FL("Reassoc to same AP, only supported for Open Security"));
			return -ENOTSUPP;
		}
		hddLog(LOG1,
		  FL("Reassoc BSSID is same as currently associated AP bssid"));
		sme_get_modify_profile_fields(hdd_ctx->hHal, adapter->sessionId,
				&modProfileFields);
		sme_roam_reassoc(hdd_ctx->hHal, adapter->sessionId,
			NULL, modProfileFields, &roamId, 1);
		return 0;
	}

	/* Check channel number is a valid channel number */
	if (QDF_STATUS_SUCCESS !=
		wlan_hdd_validate_operation_channel(adapter, channel)) {
		hddLog(LOGE, FL("Invalid Channel [%d]"), channel);
		return -EINVAL;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (hdd_ctx->config->isRoamOffloadEnabled) {
		hdd_wma_send_fastreassoc_cmd((int)adapter->sessionId,
					targetApBssid, (int)channel);
		goto exit;
	}
#endif
	/* Proceed with reassoc */
#ifdef WLAN_FEATURE_ROAM_SCAN_OFFLOAD
	handoffInfo.channel = channel;
	handoffInfo.src = FASTREASSOC;
	qdf_mem_copy(handoffInfo.bssid, targetApBssid,
		     sizeof(tSirMacAddr));
	sme_handoff_request(hdd_ctx->hHal, adapter->sessionId,
			    &handoffInfo);
#endif

exit:
	return ret;
}

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
static int drv_cmd_ccx_plm_req(hdd_adapter_t *adapter,
			       hdd_context_t *hdd_ctx,
			       uint8_t *command,
			       uint8_t command_len,
			       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpSirPlmReq pPlmRequest = NULL;

	pPlmRequest = qdf_mem_malloc(sizeof(tSirPlmReq));
	if (NULL == pPlmRequest) {
		ret = -ENOMEM;
		goto exit;
	}

	status = hdd_parse_plm_cmd(value, pPlmRequest);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_mem_free(pPlmRequest);
		pPlmRequest = NULL;
		ret = -EINVAL;
		goto exit;
	}
	pPlmRequest->sessionId = adapter->sessionId;

	status = sme_set_plm_request(hdd_ctx->hHal, pPlmRequest);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_mem_free(pPlmRequest);
		pPlmRequest = NULL;
		ret = -EINVAL;
		goto exit;
	}

exit:
	return ret;
}
#endif

#ifdef FEATURE_WLAN_ESE
static int drv_cmd_set_ccx_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t eseMode = CFG_ESE_FEATURE_ENABLED_DEFAULT;

	/*
	 * Check if the features OKC/ESE/11R are supported simultaneously,
	 * then this operation is not permitted (return FAILURE)
	 */
	if (sme_get_is_ese_feature_enabled(hdd_ctx->hHal) &&
	    hdd_is_okc_mode_enabled(hdd_ctx) &&
	    sme_get_is_ft_feature_enabled(hdd_ctx->hHal)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_WARN,
			  "%s: OKC/ESE/11R are supported simultaneously hence this operation is not permitted!",
			  __func__);
		ret = -EPERM;
		goto exit;
	}

	/* Move pointer to ahead of SETCCXMODE<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &eseMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_ESE_FEATURE_ENABLED_MIN,
			  CFG_ESE_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((eseMode < CFG_ESE_FEATURE_ENABLED_MIN) ||
	    (eseMode > CFG_ESE_FEATURE_ENABLED_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Ese mode value %d is out of range (Min: %d Max: %d)",
			  eseMode,
			  CFG_ESE_FEATURE_ENABLED_MIN,
			  CFG_ESE_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change ese mode = %d",
		  __func__, eseMode);

	hdd_ctx->config->isEseIniFeatureEnabled = eseMode;
	sme_update_is_ese_feature_enabled(hdd_ctx->hHal,
					  adapter->sessionId,
					  eseMode);

exit:
	return ret;
}
#endif

static int drv_cmd_set_roam_scan_control(hdd_adapter_t *adapter,
					 hdd_context_t *hdd_ctx,
					 uint8_t *command,
					 uint8_t command_len,
					 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t roamScanControl = 0;

	/* Move pointer to ahead of SETROAMSCANCONTROL<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &roamScanControl);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed ", __func__);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set roam scan control = %d",
		  __func__, roamScanControl);

	if (0 != roamScanControl) {
		ret = 0; /* return success but ignore param value "true" */
		goto exit;
	}

	sme_set_roam_scan_control(hdd_ctx->hHal,
				  adapter->sessionId,
				  roamScanControl);

exit:
	return ret;
}

static int drv_cmd_set_okc_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t okcMode = CFG_OKC_FEATURE_ENABLED_DEFAULT;

	/*
	 * Check if the features OKC/ESE/11R are supported simultaneously,
	 * then this operation is not permitted (return FAILURE)
	 */
	if (sme_get_is_ese_feature_enabled(hdd_ctx->hHal) &&
	    hdd_is_okc_mode_enabled(hdd_ctx) &&
	    sme_get_is_ft_feature_enabled(hdd_ctx->hHal)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_WARN,
			  "%s: OKC/ESE/11R are supported simultaneously hence this operation is not permitted!",
			  __func__);
		ret = -EPERM;
		goto exit;
	}

	/* Move pointer to ahead of SETOKCMODE<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &okcMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_OKC_FEATURE_ENABLED_MIN,
			  CFG_OKC_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((okcMode < CFG_OKC_FEATURE_ENABLED_MIN) ||
	    (okcMode > CFG_OKC_FEATURE_ENABLED_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Okc mode value %d is out of range (Min: %d Max: %d)",
			  okcMode,
			  CFG_OKC_FEATURE_ENABLED_MIN,
			  CFG_OKC_FEATURE_ENABLED_MAX);
		ret = -EINVAL;
		goto exit;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to change okc mode = %d",
		  __func__, okcMode);

	hdd_ctx->config->isOkcIniFeatureEnabled = okcMode;

exit:
	return ret;
}

static int drv_cmd_get_roam_scan_control(hdd_adapter_t *adapter,
					 hdd_context_t *hdd_ctx,
					 uint8_t *command,
					 uint8_t command_len,
					 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	bool roamScanControl = sme_get_roam_scan_control(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d",
			command, roamScanControl);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_bt_coex_mode(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	char *bcMode;

	bcMode = command + 11;
	if ('1' == *bcMode) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_DEBUG,
			  FL("BTCOEXMODE %d"), *bcMode);
		hdd_ctx->btCoexModeSet = true;
		ret = wlan_hdd_scan_abort(adapter);
		if (ret < 0) {
			hddLog(LOGE,
			    FL("Failed to abort existing scan status:%d"), ret);
		}
	} else if ('2' == *bcMode) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_DEBUG,
			  FL("BTCOEXMODE %d"), *bcMode);
		hdd_ctx->btCoexModeSet = false;
	}

	return ret;
}

static int drv_cmd_scan_active(hdd_adapter_t *adapter,
			       hdd_context_t *hdd_ctx,
			       uint8_t *command,
			       uint8_t command_len,
			       hdd_priv_data_t *priv_data)
{
	hdd_ctx->ioctl_scan_mode = eSIR_ACTIVE_SCAN;
	return 0;
}

static int drv_cmd_scan_passive(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	hdd_ctx->ioctl_scan_mode = eSIR_PASSIVE_SCAN;
	return 0;
}

static int drv_cmd_get_dwell_time(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	int ret = 0;
	struct hdd_config *pCfg =
		(WLAN_HDD_GET_CTX(adapter))->config;
	char extra[32];
	uint8_t len = 0;

	memset(extra, 0, sizeof(extra));
	ret = hdd_get_dwell_time(pCfg, command, extra, sizeof(extra), &len);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (ret != 0 || copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}
	ret = len;
exit:
	return ret;
}

static int drv_cmd_set_dwell_time(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	return hdd_set_dwell_time(adapter, command);
}

static int drv_cmd_miracast(hdd_adapter_t *adapter,
			    hdd_context_t *hdd_ctx,
			    uint8_t *command,
			    uint8_t command_len,
			    hdd_priv_data_t *priv_data)
{
	QDF_STATUS ret_status;
	int ret = 0;
	tHalHandle hHal;
	uint8_t filterType = 0;
	hdd_context_t *pHddCtx = NULL;
	uint8_t *value;

	pHddCtx = WLAN_HDD_GET_CTX(adapter);
	if (0 != wlan_hdd_validate_context(pHddCtx)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			"%s pHddCtx is not valid, Unable to set miracast mode",
			 __func__);
		return -EINVAL;
	}

	hHal = pHddCtx->hHal;
	value = command + 9;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &filterType);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range ",
			  __func__);
		ret = -EINVAL;
		goto exit;
	}
	if ((filterType < WLAN_HDD_DRIVER_MIRACAST_CFG_MIN_VAL)
	    || (filterType >
		WLAN_HDD_DRIVER_MIRACAST_CFG_MAX_VAL)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Accepted Values are 0 to 2. 0-Disabled, 1-Source, 2-Sink ",
			  __func__);
		ret = -EINVAL;
		goto exit;
	}
	/* Filtertype value should be either 0-Disabled, 1-Source, 2-sink */
	pHddCtx->miracast_value = filterType;

	ret_status = sme_set_miracast(hHal, filterType);
	if (QDF_STATUS_SUCCESS != ret_status) {
		hddLog(LOGE, "Failed to set miracast");
		return -EBUSY;
	}

	if (cds_is_mcc_in_24G())
		return cds_set_mas(adapter, filterType);

exit:
	return ret;
}

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
static int drv_cmd_set_ccx_roam_scan_channels(hdd_adapter_t *adapter,
					      hdd_context_t *hdd_ctx,
					      uint8_t *command,
					      uint8_t command_len,
					      hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t ChannelList[WNI_CFG_VALID_CHANNEL_LIST_LEN] = { 0 };
	uint8_t numChannels = 0;
	QDF_STATUS status;

	ret = hdd_parse_channellist(value, ChannelList, &numChannels);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to parse channel list information",
			  __func__);
		goto exit;
	}
	if (numChannels > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: number of channels (%d) supported exceeded max (%d)",
			  __func__,
			  numChannels,
			  WNI_CFG_VALID_CHANNEL_LIST_LEN);
		ret = -EINVAL;
		goto exit;
	}
	status = sme_set_ese_roam_scan_channel_list(hdd_ctx->hHal,
						    adapter->sessionId,
						    ChannelList,
						    numChannels);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to update channel list information",
			  __func__);
		ret = -EINVAL;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_get_tsm_stats(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	char extra[128] = { 0 };
	int len = 0;
	uint8_t tid = 0;
	hdd_station_ctx_t *pHddStaCtx;
	tAniTrafStrmMetrics tsm_metrics;

	if ((WLAN_HDD_INFRA_STATION != adapter->device_mode) &&
	    (WLAN_HDD_P2P_CLIENT != adapter->device_mode)) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return -EINVAL;
	}

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	/* if not associated, return error */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s:Not associated!", __func__);
		ret = -EINVAL;
		goto exit;
	}

	/* Move pointer to ahead of GETTSMSTATS<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &tid);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, TID_MIN_VALUE,
			  TID_MAX_VALUE);
		ret = -EINVAL;
		goto exit;
	}
	if ((tid < TID_MIN_VALUE) || (tid > TID_MAX_VALUE)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "tid value %d is out of range (Min: %d Max: %d)",
			  tid, TID_MIN_VALUE, TID_MAX_VALUE);
		ret = -EINVAL;
		goto exit;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD,
		  QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to get tsm stats tid = %d",
		  __func__, tid);
	if (QDF_STATUS_SUCCESS !=
	    hdd_get_tsm_stats(adapter, tid, &tsm_metrics)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to get tsm stats",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD,
		  QDF_TRACE_LEVEL_INFO,
		  "UplinkPktQueueDly(%d) UplinkPktQueueDlyHist[0](%d) UplinkPktQueueDlyHist[1](%d) UplinkPktQueueDlyHist[2](%d) UplinkPktQueueDlyHist[3](%d) UplinkPktTxDly(%u) UplinkPktLoss(%d) UplinkPktCount(%d) RoamingCount(%d) RoamingDly(%d)",
		  tsm_metrics.UplinkPktQueueDly,
		  tsm_metrics.UplinkPktQueueDlyHist[0],
		  tsm_metrics.UplinkPktQueueDlyHist[1],
		  tsm_metrics.UplinkPktQueueDlyHist[2],
		  tsm_metrics.UplinkPktQueueDlyHist[3],
		  tsm_metrics.UplinkPktTxDly,
		  tsm_metrics.UplinkPktLoss,
		  tsm_metrics.UplinkPktCount,
		  tsm_metrics.RoamingCount,
		  tsm_metrics.RoamingDly);
	/*
	 * Output TSM stats is of the format
	 * GETTSMSTATS [PktQueueDly]
	 * [PktQueueDlyHist[0]]:[PktQueueDlyHist[1]] ...[RoamingDly]
	 * eg., GETTSMSTATS 10 1:0:0:161 20 1 17 8 39800
	 */
	len = scnprintf(extra,
			sizeof(extra),
			"%s %d %d:%d:%d:%d %u %d %d %d %d",
			command,
			tsm_metrics.UplinkPktQueueDly,
			tsm_metrics.UplinkPktQueueDlyHist[0],
			tsm_metrics.UplinkPktQueueDlyHist[1],
			tsm_metrics.UplinkPktQueueDlyHist[2],
			tsm_metrics.UplinkPktQueueDlyHist[3],
			tsm_metrics.UplinkPktTxDly,
			tsm_metrics.UplinkPktLoss,
			tsm_metrics.UplinkPktCount,
			tsm_metrics.RoamingCount,
			tsm_metrics.RoamingDly);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
		goto exit;
	}

exit:
	return ret;
}

static int drv_cmd_set_cckm_ie(hdd_adapter_t *adapter,
			       hdd_context_t *hdd_ctx,
			       uint8_t *command,
			       uint8_t command_len,
			       hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	uint8_t *cckmIe = NULL;
	uint8_t cckmIeLen = 0;

	ret = hdd_parse_get_cckm_ie(value, &cckmIe, &cckmIeLen);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to parse cckm ie data",
			  __func__);
		goto exit;
	}

	if (cckmIeLen > DOT11F_IE_RSN_MAX_LEN) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: CCKM Ie input length is more than max[%d]",
			  __func__, DOT11F_IE_RSN_MAX_LEN);
		if (NULL != cckmIe) {
			qdf_mem_free(cckmIe);
			cckmIe = NULL;
		}
		ret = -EINVAL;
		goto exit;
	}

	sme_set_cckm_ie(hdd_ctx->hHal, adapter->sessionId,
			cckmIe, cckmIeLen);
	if (NULL != cckmIe) {
		qdf_mem_free(cckmIe);
		cckmIe = NULL;
	}

exit:
	return ret;
}

static int drv_cmd_ccx_beacon_req(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	tCsrEseBeaconReq eseBcnReq;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (WLAN_HDD_INFRA_STATION != adapter->device_mode) {
		hdd_warn("Unsupported in mode %s(%d)",
			 hdd_device_mode_to_string(adapter->device_mode),
			 adapter->device_mode);
		return -EINVAL;
	}

	ret = hdd_parse_ese_beacon_req(value, &eseBcnReq);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to parse ese beacon req",
			  __func__);
		goto exit;
	}

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hddLog(QDF_TRACE_LEVEL_INFO, FL("Not associated"));
		hdd_indicate_ese_bcn_report_no_results(adapter,
			eseBcnReq.bcnReq[0].measurementToken,
			0x02, /* BIT(1) set for measurement done */
			0);   /* no BSS */
		goto exit;
	}

	status = sme_set_ese_beacon_request(hdd_ctx->hHal,
					    adapter->sessionId,
					    &eseBcnReq);

	if (QDF_STATUS_E_RESOURCES == status) {
		hddLog(QDF_TRACE_LEVEL_INFO,
		       FL("sme_set_ese_beacon_request failed (%d), a request already in progress"),
		       status);
		ret = -EBUSY;
		goto exit;
	} else if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: sme_set_ese_beacon_request failed (%d)",
			  __func__, status);
		ret = -EINVAL;
		goto exit;
	}

exit:
	return ret;
}
#endif /* #if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD) */

static int drv_cmd_set_mc_rate(hdd_adapter_t *adapter,
			       hdd_context_t *hdd_ctx,
			       uint8_t *command,
			       uint8_t command_len,
			       hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	int targetRate;

	/* input value is in units of hundred kbps */

	/* Move pointer to ahead of SETMCRATE<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer, decimal base */
	ret = kstrtouint(value, 10, &targetRate);

	ret = wlan_hdd_set_mc_rate(adapter, targetRate);
	return ret;
}

static int drv_cmd_max_tx_power(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	int ret = 0;
	int status;
	int txPower;
	QDF_STATUS qdf_status;
	QDF_STATUS smeStatus;
	uint8_t *value = command;
	struct qdf_mac_addr bssid = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	struct qdf_mac_addr selfMac = QDF_MAC_ADDR_BROADCAST_INITIALIZER;
	hdd_adapter_list_node_t *pAdapterNode = NULL;
	hdd_adapter_list_node_t *pNext = NULL;

	status = hdd_parse_setmaxtxpower_command(value, &txPower);
	if (status) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "Invalid MAXTXPOWER command ");
		ret = -EINVAL;
		goto exit;
	}

	qdf_status = hdd_get_front_adapter(hdd_ctx, &pAdapterNode);
	while (NULL != pAdapterNode
	       && QDF_STATUS_SUCCESS == qdf_status) {
		adapter = pAdapterNode->pAdapter;
		/* Assign correct self MAC address */
		qdf_copy_macaddr(&bssid,
				 &adapter->macAddressCurrent);
		qdf_copy_macaddr(&selfMac,
				 &adapter->macAddressCurrent);

		hddLog(QDF_TRACE_LEVEL_INFO,
		       "Device mode %d max tx power %d selfMac: " MAC_ADDRESS_STR " bssId: " MAC_ADDRESS_STR " ",
		       adapter->device_mode, txPower,
		       MAC_ADDR_ARRAY(selfMac.bytes),
		       MAC_ADDR_ARRAY(bssid.bytes));

		smeStatus = sme_set_max_tx_power(hdd_ctx->hHal,
						 bssid, selfMac, txPower);
		if (QDF_STATUS_SUCCESS != status) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s:Set max tx power failed",
			       __func__);
			ret = -EINVAL;
			goto exit;
		}
		hddLog(QDF_TRACE_LEVEL_INFO,
		       "%s: Set max tx power success",
		       __func__);
		qdf_status = hdd_get_next_adapter(hdd_ctx, pAdapterNode,
						  &pNext);
		pAdapterNode = pNext;
	}

exit:
	return ret;
}

static int drv_cmd_set_dfs_scan_mode(hdd_adapter_t *adapter,
				    hdd_context_t *hdd_ctx,
				    uint8_t *command,
				    uint8_t command_len,
				    hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t *value = command;
	uint8_t dfsScanMode = CFG_ROAMING_DFS_CHANNEL_DEFAULT;

	/* Move pointer to ahead of SETDFSSCANMODE<delimiter> */
	value = value + command_len + 1;

	/* Convert the value from ascii to integer */
	ret = kstrtou8(value, 10, &dfsScanMode);
	if (ret < 0) {
		/*
		 * If the input value is greater than max value of datatype,
		 * then also kstrtou8 fails
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: kstrtou8 failed range [%d - %d]",
			  __func__, CFG_ROAMING_DFS_CHANNEL_MIN,
			  CFG_ROAMING_DFS_CHANNEL_MAX);
		ret = -EINVAL;
		goto exit;
	}

	if ((dfsScanMode < CFG_ROAMING_DFS_CHANNEL_MIN) ||
	    (dfsScanMode > CFG_ROAMING_DFS_CHANNEL_MAX)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "dfsScanMode value %d is out of range (Min: %d Max: %d)",
			  dfsScanMode,
			  CFG_ROAMING_DFS_CHANNEL_MIN,
			  CFG_ROAMING_DFS_CHANNEL_MAX);
		ret = -EINVAL;
		goto exit;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received Command to Set DFS Scan Mode = %d",
		  __func__, dfsScanMode);

	/* When DFS scanning is disabled, the DFS channels need to be
	 * removed from the operation of device.
	 */
	ret = wlan_hdd_disable_dfs_chan_scan(hdd_ctx, adapter,
			(dfsScanMode == CFG_ROAMING_DFS_CHANNEL_DISABLED));
	if (ret < 0) {
		/* Some conditions prevented it from disabling DFS channels */
		hddLog(LOGE,
		       FL("disable/enable DFS channel request was denied"));
		goto exit;
	}

	hdd_ctx->config->allowDFSChannelRoam = dfsScanMode;
	sme_update_dfs_scan_mode(hdd_ctx->hHal, adapter->sessionId,
				 dfsScanMode);

exit:
	return ret;
}

static int drv_cmd_get_dfs_scan_mode(hdd_adapter_t *adapter,
				     hdd_context_t *hdd_ctx,
				     uint8_t *command,
				     uint8_t command_len,
				     hdd_priv_data_t *priv_data)
{
	int ret = 0;
	uint8_t dfsScanMode = sme_get_dfs_scan_mode(hdd_ctx->hHal);
	char extra[32];
	uint8_t len = 0;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, dfsScanMode);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_get_link_status(hdd_adapter_t *adapter,
				   hdd_context_t *hdd_ctx,
				   uint8_t *command,
				   uint8_t command_len,
				   hdd_priv_data_t *priv_data)
{
	int ret = 0;
	int value = wlan_hdd_get_link_status(adapter);
	char extra[32];
	uint8_t len;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, value);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
			  QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer",
			  __func__);
		ret = -EFAULT;
	}

	return ret;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
static int drv_cmd_enable_ext_wow(hdd_adapter_t *adapter,
				  hdd_context_t *hdd_ctx,
				  uint8_t *command,
				  uint8_t command_len,
				  hdd_priv_data_t *priv_data)
{
	uint8_t *value = command;
	int set_value;

	/* Move pointer to ahead of ENABLEEXTWOW */
	value = value + command_len;

	sscanf(value, "%d", &set_value);

	return hdd_enable_ext_wow_parser(adapter,
					 adapter->sessionId,
					 set_value);
}

static int drv_cmd_set_app1_params(hdd_adapter_t *adapter,
				   hdd_context_t *hdd_ctx,
				   uint8_t *command,
				   uint8_t command_len,
				   hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;

	/* Move pointer to ahead of SETAPP1PARAMS */
	value = value + command_len;

	ret = hdd_set_app_type1_parser(adapter,
				       value, strlen(value));
	if (ret >= 0)
		hdd_ctx->is_extwow_app_type1_param_set = true;

	return ret;
}

static int drv_cmd_set_app2_params(hdd_adapter_t *adapter,
				   hdd_context_t *hdd_ctx,
				   uint8_t *command,
				   uint8_t command_len,
				   hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;

	/* Move pointer to ahead of SETAPP2PARAMS */
	value = value + command_len;

	ret = hdd_set_app_type2_parser(adapter, value, strlen(value));
	if (ret >= 0)
		hdd_ctx->is_extwow_app_type2_param_set = true;

	return ret;
}
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

#ifdef FEATURE_WLAN_TDLS
/**
 * drv_cmd_tdls_secondary_channel_offset() - secondary tdls off channel offset
 * @adapter:     Pointer to the HDD adapter
 * @hdd_ctx:     Pointer to the HDD context
 * @command:     Driver command string
 * @command_len: Driver command string length
 * @priv_data:   Private data coming with the driver command. Unused here
 *
 * This function handles driver command that sets the secondary tdls off channel
 * offset
 *
 * Return: 0 on success; negative errno otherwise
 */
static int drv_cmd_tdls_secondary_channel_offset(hdd_adapter_t *adapter,
						 hdd_context_t *hdd_ctx,
						 uint8_t *command,
						 uint8_t command_len,
						 hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	int set_value;

	/* Move pointer to point the string */
	value += command_len;

	ret = sscanf(value, "%d", &set_value);
	if (ret != 1)
		return -EINVAL;

	hddLog(LOG1, FL("Tdls offchannel offset:%d"), set_value);

	ret = hdd_set_tdls_secoffchanneloffset(hdd_ctx, set_value);

	return ret;
}

/**
 * drv_cmd_tdls_off_channel_mode() - set tdls off channel mode
 * @adapter:     Pointer to the HDD adapter
 * @hdd_ctx:     Pointer to the HDD context
 * @command:     Driver command string
 * @command_len: Driver command string length
 * @priv_data:   Private data coming with the driver command. Unused here
 *
 * This function handles driver command that sets tdls off channel mode
 *
 * Return: 0 on success; negative errno otherwise
 */
static int drv_cmd_tdls_off_channel_mode(hdd_adapter_t *adapter,
					 hdd_context_t *hdd_ctx,
					 uint8_t *command,
					 uint8_t command_len,
					 hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	int set_value;

	/* Move pointer to point the string */
	value += command_len;

	ret = sscanf(value, "%d", &set_value);
	if (ret != 1)
		return -EINVAL;

	hddLog(LOG1, FL("Tdls offchannel mode:%d"), set_value);

	ret = hdd_set_tdls_offchannelmode(adapter, set_value);

	return ret;
}

/**
 * drv_cmd_tdls_off_channel() - set tdls off channel number
 * @adapter:     Pointer to the HDD adapter
 * @hdd_ctx:     Pointer to the HDD context
 * @command:     Driver command string
 * @command_len: Driver command string length
 * @priv_data:   Private data coming with the driver command. Unused here
 *
 * This function handles driver command that sets tdls off channel number
 *
 * Return: 0 on success; negative errno otherwise
 */
static int drv_cmd_tdls_off_channel(hdd_adapter_t *adapter,
				    hdd_context_t *hdd_ctx,
				    uint8_t *command,
				    uint8_t command_len,
				    hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	int set_value;

	/* Move pointer to point the string */
	value += command_len;

	ret = sscanf(value, "%d", &set_value);
	if (ret != 1)
		return -EINVAL;

	if (CDS_IS_DFS_CH(set_value)) {
		hdd_err("DFS channel %d is passed for hdd_set_tdls_offchannel",
		    set_value);
		return -EINVAL;
	}

	hddLog(LOG1, FL("Tdls offchannel num: %d"), set_value);

	ret = hdd_set_tdls_offchannel(hdd_ctx, set_value);

	return ret;
}

/**
 * drv_cmd_tdls_scan() - set tdls scan type
 * @adapter:     Pointer to the HDD adapter
 * @hdd_ctx:     Pointer to the HDD context
 * @command:     Driver command string
 * @command_len: Driver command string length
 * @priv_data:   Private data coming with the driver command. Unused here
 *
 * This function handles driver command that sets tdls scan type
 *
 * Return: 0 on success; negative errno otherwise
 */
static int drv_cmd_tdls_scan(hdd_adapter_t *adapter,
				    hdd_context_t *hdd_ctx,
				    uint8_t *command,
				    uint8_t command_len,
				    hdd_priv_data_t *priv_data)
{
	int ret;
	uint8_t *value = command;
	int set_value;

	/* Move pointer to point the string */
	value += command_len;

	ret = sscanf(value, "%d", &set_value);
	if (ret != 1)
		return -EINVAL;

	hddLog(LOG1, FL("Tdls scan type val: %d"), set_value);

	ret = hdd_set_tdls_scan_type(hdd_ctx, set_value);

	return ret;
}
#endif

static int drv_cmd_get_rssi(hdd_adapter_t *adapter,
			    hdd_context_t *hdd_ctx,
			    uint8_t *command,
			    uint8_t command_len,
			    hdd_priv_data_t *priv_data)
{
	int ret = 0;
	int8_t rssi = 0;
	char extra[32];

	uint8_t len = 0;

	wlan_hdd_get_rssi(adapter, &rssi);

	len = scnprintf(extra, sizeof(extra), "%s %d", command, rssi);
	len = QDF_MIN(priv_data->total_len, len + 1);

	if (copy_to_user(priv_data->buf, &extra, len)) {
		hddLog(LOGE, FL("Failed to copy data to user buffer"));
		ret = -EFAULT;
	}

	return ret;
}

static int drv_cmd_get_linkspeed(hdd_adapter_t *adapter,
				 hdd_context_t *hdd_ctx,
				 uint8_t *command,
				 uint8_t command_len,
				 hdd_priv_data_t *priv_data)
{
	int ret;
	uint32_t link_speed = 0;
	char extra[32];
	uint8_t len = 0;

	ret = wlan_hdd_get_link_speed(adapter, &link_speed);
	if (0 != ret)
		return ret;

	len = scnprintf(extra, sizeof(extra), "%s %d", command, link_speed);
	len = QDF_MIN(priv_data->total_len, len + 1);
	if (copy_to_user(priv_data->buf, &extra, len)) {
		hddLog(LOGE, FL("Failed to copy data to user buffer"));
		ret = -EFAULT;
	}

	return ret;
}

#ifdef FEATURE_NAPI
/**
 * hdd_parse_napi() - helper functions to drv_cmd_napi
 * @str : source string to parse
 * @cmd : pointer to cmd part after parsing
 * @sub : pointer to subcmd part after parsing
 * @aux : pointer to optional aux part after parsing
 *
 * Example:
 * NAPI SCALE <n>  +-- IN  str
 *  |    |     +------ OUT aux
 *  |    +------------ OUT subcmd
 *  +----------------- OUT cmd
 *
 * Return: ==0: success; !=0: failure
 */
static int hdd_parse_napi(char **str, char **cmd, char **sub, char **aux)
{
	int rc;
	char *token, *lcmd = NULL, *lsub = NULL, *laux = NULL;

	NAPI_DEBUG("-->\n");

	token = strsep(str, " \t");
	if (NULL == token) {
		hdd_err("cannot parse cmd");
		goto parse_end;
	}
	lcmd = token;

	token = strsep(str, " \t");
	if (NULL == token) {
		hdd_err("cannot parse subcmd");
		goto parse_end;
	}
	lsub = token;

	token = strsep(str, " \t");
	if (NULL == token)
		hdd_warn("cannot parse aux\n");
	else
		laux = token;

parse_end:
	if ((NULL == lcmd) || (NULL == lsub))
		rc = -EINVAL;
	else {
		rc = 0;
		*cmd = lcmd;
		*sub = lsub;
		if (NULL != aux)
			*aux = laux;
	}
	NAPI_DEBUG("<--[rc=%d]\n", rc);
	return rc;
}


/**
 * hdd_parse_stats() - print NAPI stats into a buffer
 * @buf : buffer to write stats into
 * @max : "size of buffer"
 * @idp : NULL: all stats, otherwise, ptr to the NAPI instance
 * @napid: binary structure to retrieve the stats from
 *
 * Return: number of bytes written into the buffer
 */
int hdd_napi_stats(char   *buf,
		   int     max,
		   char   *indp,
		   struct qca_napi_data *napid)
{
	int n = 0;
	int i, j, k; /* NAPI, CPU, bucket indices */
	int from, to;
	struct qca_napi_info *napii;
	struct qca_napi_stat *napis;

	NAPI_DEBUG("-->\n");

	if (NULL == napid)
		return n;
	if (NULL == indp) {
		from = 0;
		to = CE_COUNT_MAX;
	} else {
		if (0 > kstrtoint(indp, 10, &to)) {
			from = 0;
			to = CE_COUNT_MAX;
		} else
			from = to;
	}

	for (i = from; i < to; i++)
		if (napid->ce_map & (0x01 << i)) {
			napii = &(napid->napis[i]);
			for (j = 0; j < NR_CPUS; j++) {
				napis = &(napii->stats[j]);
				n += scnprintf(buf + n, max - n,
					       "STATS: NAPI[%d] CPU: %d scheds: %d polls: %d completes: %d done: %d ",
					       i, j,
					       napis->napi_schedules,
					       napis->napi_polls,
					       napis->napi_completes,
					       napis->napi_workdone);

				for (k = 0; k < QCA_NAPI_NUM_BUCKETS; k++) {
					n += scnprintf(
						buf + n, max - n,
						" %d",
						napis->napi_budget_uses[k]);
				}
				n += scnprintf(buf+n, max - n, "\n");
			}
		}

	NAPI_DEBUG("<--[n=%d]\n", n);
	return n;
}

/**
 * napi_set_scale() - sets the scale attribute in all NAPI entries
 * @sc : scale to set
 *
 * Return: void
 */
static void napi_set_scale(uint8_t sc)
{
	uint32_t  i;
	struct qca_napi_data *napi_data;

	napi_data = hdd_napi_get_all();
	if (likely(NULL != napi_data))
	    for (i = 0; i < CE_COUNT_MAX; i++)
		    if (napi_data->ce_map & (0x01 << i))
			    napi_data->napis[i].scale = sc;

	return;
}
/**
 * drv_cmd_napi() - processes NAPI commands
 * @adapter    : net_device
 * @hdd_ctx    : HDD context
 * @command    : command string from user command (including "NAPI")
 * @command_len: length of command
 * @priv_data  : ifr_data
 *
 * Commands supported:
 * NAPI ENABLE      : enables NAPI administratively. Note that this may not
 *                    enable NAPI functionally, as some other conditions
 *                    may not have been satisfied yet
 * NAPI DISABLE     : reverse operation of "enable"
 * NAPI STATUS      : get global status of NAPI instances
 * NAPI STATS [<n>] : get the stats for a given NAPI instance
 * NAPI SCALE <n>   : set the scale factor
 *
 * Return: 0: success; !0: failure
 */
static int drv_cmd_napi(hdd_adapter_t *adapter,
			hdd_context_t *hdd_ctx,
			uint8_t *command,
			uint8_t command_len,
			hdd_priv_data_t *priv_data)
{
	int  rc = 0;
	int  n, l;
	char *cmd = NULL, *subcmd = NULL, *aux = NULL;
	char *synopsis = "NAPI ENABLE\n"
		"NAPI DISABLE\n"
		"NAPI STATUS\n"
		"NAPI STATS [<n>] -- if no <n> then all\n"
		"NAPI SCALE <n>   -- set the scale\n";
	char *reply = NULL;

	/* make a local copy, as strsep modifies the str in place */
	char *str = NULL;

	NAPI_DEBUG("-->\n");

	/**
	 * NOTE TO MAINTAINER: from this point to the end of the function,
	 * please do not return anywhere in the code except the very end
	 * to avoid memory leakage (goto end_drv_napi instead)
	 * or make sure that reply+str is freed
	 */
	reply = kmalloc(MAX_USER_COMMAND_SIZE, GFP_KERNEL);
	if (NULL == reply) {
		hdd_err("could not allocate reply buffer");
		rc = -ENOMEM;
		goto end_drv_napi;
	}

	str = kmalloc(strlen(command) + 1, GFP_KERNEL);
	if (NULL == str) {
		hdd_err("could not allocate copy of input buffer");
		rc = -ENOMEM;
		goto end_drv_napi;
	}

	strlcpy(str, command, strlen(command) + 1);
	hdd_debug("parsing command into cmd=0x%p sub=0x%p aux=0x%p\n",
		  cmd, subcmd, aux);


	rc = hdd_parse_napi(&str, &cmd, &subcmd, &aux);

	if (0 != rc) {
		const char *msg = "unknown or badly formatted cmd\n%s";
		l = QDF_MIN(MAX_USER_COMMAND_SIZE,
			    strlen(msg)+strlen(synopsis));
		n = scnprintf(reply, l, msg, synopsis);

		if (copy_to_user(priv_data->buf, reply,
				 QDF_MIN(priv_data->total_len, l)))
			hdd_err("failed to copy data to user buffer");
		hdd_debug("reply: %s", reply);

		rc = -EINVAL;
	} else {
		hdd_debug("cmd=(%s) subcmd=(%s) aux=(%s)\n",
			cmd, subcmd, aux);
		if (!strcmp(subcmd, "ENABLE"))
			hdd_napi_event(NAPI_EVT_CMD_STATE, (void *)1);
		else if (!strcmp(subcmd, "DISABLE"))
			hdd_napi_event(NAPI_EVT_CMD_STATE, (void *)0);
		else if (!strcmp(subcmd, "STATUS")) {
			int n = 0;
			uint32_t  i;
			struct qca_napi_data *napi_data;

			napi_data = hdd_napi_get_all();
			if (unlikely(NULL == napi_data))
				goto status_end;
			n += scnprintf(reply+n, MAX_USER_COMMAND_SIZE - n,
				       "NAPI state: 0x%08x map: 0x%08x\n",
				       napi_data->state,
				       napi_data->ce_map);

			for (i = 0; i < CE_COUNT_MAX; i++)
				if (napi_data->ce_map & (0x01 << i)) {
					n += scnprintf(
						reply + n,
						MAX_USER_COMMAND_SIZE - n,
						"#%d: id: %d, scale=%d\n",
						i,
						napi_data->napis[i].id,
						napi_data->napis[i].scale);
				}
		status_end:
			hdd_info("wlan: STATUS DATA:\n%s", reply);
			if (copy_to_user(priv_data->buf, reply,
					 QDF_MIN(n, priv_data->total_len)))
				rc = -EINVAL;
		} else if (!strcmp(subcmd, "STATS")) {
			int n = 0;
			struct qca_napi_data *napi_data;

			napi_data = hdd_napi_get_all();
			if (NULL != napi_data) {
				n = hdd_napi_stats(reply, MAX_USER_COMMAND_SIZE,
						   aux, napi_data);
				NAPI_DEBUG("STATS: returns %d\n", n);
			}
			if (n > 0) {
				if (copy_to_user(priv_data->buf, reply,
						 QDF_MIN(priv_data->total_len,
							 n)))
					rc = -EINVAL;
				hdd_info("wlan: STATS_DATA\n%s\n", reply);
			} else
				rc = -EINVAL;
		} else if (!strcmp(subcmd, "SCALE")) {
			if (NULL == aux) {
				rc = -EINVAL;
				hdd_err("wlan: SCALE cmd requires <n>");
			} else {
				uint8_t sc;
				rc = kstrtou8(aux, 10, &sc);
				if (rc) {
					hdd_err("wlan: bad scale (%s)", aux);
					rc = -EINVAL;
				} else
					napi_set_scale(sc);
			}
		} /* SCALE */
	}
end_drv_napi:
	if (NULL != str)
		kfree(str);
	if (NULL != reply)
		kfree(reply);

	NAPI_DEBUG("<--[rc=%d]\n", rc);
	return rc;
}
#endif /* FEATURE_NAPI */

/**
 * hdd_set_rx_filter() - set RX filter
 * @adapter: Pointer to adapter
 * @action: Filter action
 * @pattern: Address pattern
 *
 * Address pattern is most significant byte of address for example
 * 0x01 for IPV4 multicast address
 * 0x33 for IPV6 multicast address
 * 0xFF for broadcast address
 *
 * Return: 0 for success, non-zero for failure
 */
static int hdd_set_rx_filter(hdd_adapter_t *adapter, bool action,
			uint8_t pattern)
{
	int ret;
	uint8_t i;
	tHalHandle handle;
	tSirRcvFltMcAddrList *filter;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	handle = hdd_ctx->hHal;

	if (NULL == handle) {
		hdd_err("HAL Handle is NULL");
		return -EINVAL;
	}

	/*
	 * If action is false it means start dropping packets
	 * Set addr_filter_pattern which will be used when sending
	 * MC/BC address list to target
	 */
	if (!action)
		adapter->addr_filter_pattern = pattern;
	else
		adapter->addr_filter_pattern = 0;

	if (((adapter->device_mode == WLAN_HDD_INFRA_STATION) ||
		(adapter->device_mode == WLAN_HDD_P2P_CLIENT)) &&
		adapter->mc_addr_list.mc_cnt &&
		hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {


		filter = qdf_mem_malloc(sizeof(*filter));
		if (NULL == filter) {
			hdd_err("Could not allocate Memory");
			return -ENOMEM;
		}
		filter->action = action;
		for (i = 0; i < adapter->mc_addr_list.mc_cnt; i++) {
			if (!memcmp(adapter->mc_addr_list.addr[i],
				&pattern, 1)) {
				memcpy(filter->multicastAddr[i].bytes,
					adapter->mc_addr_list.addr[i],
					sizeof(adapter->mc_addr_list.addr[i]));
				filter->ulMulticastAddrCnt++;
				hdd_info("%s RX filter : addr ="
				    MAC_ADDRESS_STR,
				    action ? "setting" : "clearing",
				    MAC_ADDR_ARRAY(filter->multicastAddr[i].bytes));
			}
		}
		/* Set rx filter */
		sme_8023_multicast_list(handle, adapter->sessionId, filter);
		qdf_mem_free(filter);
	} else {
		hdd_info("mode %d mc_cnt %d",
			adapter->device_mode, adapter->mc_addr_list.mc_cnt);
	}

	return 0;
}

/**
 * hdd_driver_rxfilter_comand_handler() - RXFILTER driver command handler
 * @command: Pointer to input string driver command
 * @adapter: Pointer to adapter
 * @action: Action to enable/disable filtering
 *
 * If action == false
 * Start filtering out data packets based on type
 * RXFILTER-REMOVE 0 -> Start filtering out unicast data packets
 * RXFILTER-REMOVE 1 -> Start filtering out broadcast data packets
 * RXFILTER-REMOVE 2 -> Start filtering out IPV4 mcast data packets
 * RXFILTER-REMOVE 3 -> Start filtering out IPV6 mcast data packets
 *
 * if action == true
 * Stop filtering data packets based on type
 * RXFILTER-ADD 0 -> Stop filtering unicast data packets
 * RXFILTER-ADD 1 -> Stop filtering broadcast data packets
 * RXFILTER-ADD 2 -> Stop filtering IPV4 mcast data packets
 * RXFILTER-ADD 3 -> Stop filtering IPV6 mcast data packets
 *
 * Current implementation only supports IPV4 address filtering by
 * selectively allowing IPV4 multicast data packest based on
 * address list received in .ndo_set_rx_mode
 *
 * Return: 0 for success, non-zero for failure
 */
static int hdd_driver_rxfilter_comand_handler(uint8_t *command,
						hdd_adapter_t *adapter,
						bool action)
{
	int ret = 0;
	uint8_t *value;
	uint8_t type;

	value = command;
	/* Skip space after RXFILTER-REMOVE OR RXFILTER-ADD based on action */
	if (!action)
		value = command + 16;
	else
		value = command + 13;
	ret = kstrtou8(value, 10, &type);
	if (ret < 0) {
		hdd_err("kstrtou8 failed invalid input value %d", type);
		return -EINVAL;
	}

	switch (type) {
	case 2:
		/* Set rx filter for IPV4 multicast data packets */
		ret = hdd_set_rx_filter(adapter, action, 0x01);
		break;
	default:
		hdd_info("Unsupported RXFILTER type %d", type);
		break;
	}

	return ret;
}

/**
 * drv_cmd_rx_filter_remove() - RXFILTER REMOVE driver command handler
 * @adapter: Pointer to network adapter
 * @hdd_ctx: Pointer to hdd context
 * @command: Pointer to input command
 * @command_len: Command length
 * @priv_data: Pointer to private data in command
 */
static int drv_cmd_rx_filter_remove(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	return hdd_driver_rxfilter_comand_handler(command, adapter, false);
}

/**
 * drv_cmd_rx_filter_add() - RXFILTER ADD driver command handler
 * @adapter: Pointer to network adapter
 * @hdd_ctx: Pointer to hdd context
 * @command: Pointer to input command
 * @command_len: Command length
 * @priv_data: Pointer to private data in command
 */
static int drv_cmd_rx_filter_add(hdd_adapter_t *adapter,
				hdd_context_t *hdd_ctx,
				uint8_t *command,
				uint8_t command_len,
				hdd_priv_data_t *priv_data)
{
	return hdd_driver_rxfilter_comand_handler(command, adapter, true);
}

/*
 * dummy (no-op) hdd driver command handler
 */
static int drv_cmd_dummy(hdd_adapter_t *adapter,
			 hdd_context_t *hdd_ctx,
			 uint8_t *command,
			 uint8_t command_len,
			 hdd_priv_data_t *priv_data)
{
	hdd_info("%s: Ignoring driver command \"%s\"",
		 adapter->dev->name, command);
	return 0;
}

/*
 * handler for any unsupported wlan hdd driver command
 */
static int drv_cmd_invalid(hdd_adapter_t *adapter,
			   hdd_context_t *hdd_ctx,
			   uint8_t *command,
			   uint8_t command_len,
			   hdd_priv_data_t *priv_data)
{
	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_UNSUPPORTED_IOCTL,
			 adapter->sessionId, 0));

	hdd_warn("%s: Unsupported driver command \"%s\"",
		 adapter->dev->name, command);

	return -ENOTSUPP;
}

/**
 * drv_cmd_set_fcc_channel() - handle fcc constraint request
 * @adapter: HDD adapter
 * @hdd_ctx: HDD context
 * @command: command ptr, SET_FCC_CHANNEL 0/1 is the command
 * @command_len: command len
 * @priv_data: private data
 *
 * Return: status
 */
static int drv_cmd_set_fcc_channel(hdd_adapter_t *adapter,
				   hdd_context_t *hdd_ctx,
				   uint8_t *command,
				   uint8_t command_len,
				   hdd_priv_data_t *priv_data)
{
	uint8_t *value;
	uint8_t fcc_constraint;
	QDF_STATUS status;
	int ret = 0;

	/*
	 * this command would be called by user-space when it detects WLAN
	 * ON after airplane mode is set. When APM is set, WLAN turns off.
	 * But it can be turned back on. Otherwise; when APM is turned back
	 * off, WLAN would turn back on. So at that point the command is
	 * expected to come down. 0 means disable, 1 means enable. The
	 * constraint is removed when parameter 1 is set or different
	 * country code is set
	 */

	value =  command + command_len + 1;

	ret = kstrtou8(value, 10, &fcc_constraint);
	if ((ret < 0) || (fcc_constraint > 1)) {
		/*
		 *  If the input value is greater than max value of datatype,
		 *  then also it is a failure
		 */
		hdd_err("value out of range");
		return -EINVAL;
	}

	status = sme_disable_non_fcc_channel(hdd_ctx->hHal, !fcc_constraint);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("sme disable fn. returned err");
		ret = -EPERM;
	}

	return ret;
}

/**
 * hdd_parse_set_channel_switch_command() - Parse and validate CHANNEL_SWITCH
 * command
 * @value: Pointer to the command
 * @chan_number: Pointer to the channel number
 * @chan_bw: Pointer to the channel bandwidth
 *
 * Parses and provides the channel number and channel width from the input
 * command which is expected to be of the format: CHANNEL_SWITCH <CH> <BW>
 * <CH> is channel number to move (where 1 = channel 1, 149 = channel 149, ...)
 * <BW> is bandwidth to move (where 20 = BW 20, 40 = BW 40, 80 = BW 80)
 *
 * Return: 0 for success, non-zero for failure
 */
static int hdd_parse_set_channel_switch_command(uint8_t *value,
					 uint32_t *chan_number,
					 uint32_t *chan_bw)
{
	const uint8_t *in_ptr = value;
	int ret;

	in_ptr = strnchr(value, strlen(value), SPACE_ASCII_VALUE);

	/* no argument after the command */
	if (NULL == in_ptr) {
		hdd_err("No argument after the command");
		return -EINVAL;
	}

	/* no space after the command */
	if (SPACE_ASCII_VALUE != *in_ptr) {
		hdd_err("No space after the command ");
		return -EINVAL;
	}

	/* remove empty spaces and move the next argument */
	while ((SPACE_ASCII_VALUE == *in_ptr) && ('\0' != *in_ptr))
		in_ptr++;

	/* no argument followed by spaces */
	if ('\0' == *in_ptr) {
		hdd_err("No argument followed by spaces");
		return -EINVAL;
	}

	/* get the two arguments: channel number and bandwidth */
	ret = sscanf(in_ptr, "%u %u", chan_number, chan_bw);
	if (ret != 2) {
		hdd_err("Arguments retrieval from cmd string failed");
		return -EINVAL;
	}

	return 0;
}

/**
 * drv_cmd_set_channel_switch() - Switch SAP/P2P-GO operating channel
 * @adapter: HDD adapter
 * @hdd_ctx: HDD context
 * @command: Pointer to the input command CHANNEL_SWITCH
 * @command_len: Command len
 * @priv_data: Private data
 *
 * Handles private IOCTL CHANNEL_SWITCH command to switch the operating channel
 * of SAP/P2P-GO
 *
 * Return: 0 for success, non-zero for failure
 */
static int drv_cmd_set_channel_switch(hdd_adapter_t *adapter,
				   hdd_context_t *hdd_ctx,
				   uint8_t *command,
				   uint8_t command_len,
				   hdd_priv_data_t *priv_data)
{
	struct net_device *dev = adapter->dev;
	int status;
	uint32_t chan_number = 0, chan_bw = 0;
	uint8_t *value = command;
	phy_ch_width width;

	if ((adapter->device_mode != WLAN_HDD_P2P_GO) &&
		(adapter->device_mode != WLAN_HDD_SOFTAP)) {
		hdd_err("IOCTL CHANNEL_SWITCH not supported for mode %d",
			adapter->device_mode);
		return -EINVAL;
	}

	status = hdd_parse_set_channel_switch_command(value,
							&chan_number, &chan_bw);
	if (status) {
		hdd_err("Invalid CHANNEL_SWITCH command");
		return status;
	}

	if ((chan_bw != 20) && (chan_bw != 40) && (chan_bw != 80)) {
		hdd_err("BW %d is not allowed for CHANNEL_SWITCH", chan_bw);
		return -EINVAL;
	}

	if (chan_bw == 80)
		width = CH_WIDTH_80MHZ;
	else if (chan_bw == 40)
		width = CH_WIDTH_40MHZ;
	else
		width = CH_WIDTH_20MHZ;

	hdd_info("CH:%d BW:%d", chan_number, chan_bw);

	status = hdd_softap_set_channel_change(dev, chan_number, width);
	if (status) {
		hdd_err("Set channel change fail");
		return status;
	}

	return 0;
}

/*
 * The following table contains all supported WLAN HDD
 * IOCTL driver commands and the handler for each of them.
 */
static const hdd_drv_cmd_t hdd_drv_cmds[] = {
	{"P2P_DEV_ADDR",              drv_cmd_p2p_dev_addr},
	{"P2P_SET_NOA",               drv_cmd_p2p_set_noa},
	{"P2P_SET_PS",                drv_cmd_p2p_set_ps},
	{"SETBAND",                   drv_cmd_set_band},
	{"SETWMMPS",                  drv_cmd_set_wmmps},
	{"COUNTRY",                   drv_cmd_country},
	{"SETSUSPENDMODE",            drv_cmd_dummy},
	{"SET_AP_WPS_P2P_IE",         drv_cmd_dummy},
	{"BTCOEXSCAN",                drv_cmd_dummy},
	{"RXFILTER",                  drv_cmd_dummy},
	{"SETROAMTRIGGER",            drv_cmd_set_roam_trigger},
	{"GETROAMTRIGGER",            drv_cmd_get_roam_trigger},
	{"SETROAMSCANPERIOD",         drv_cmd_set_roam_scan_period},
	{"GETROAMSCANPERIOD",         drv_cmd_get_roam_scan_period},
	{"SETROAMSCANREFRESHPERIOD",  drv_cmd_set_roam_scan_refresh_period},
	{"GETROAMSCANREFRESHPERIOD",  drv_cmd_get_roam_scan_refresh_period},
	{"SETROAMMODE",               drv_cmd_set_roam_mode},
	{"GETROAMMODE",               drv_cmd_get_roam_mode},
	{"SETROAMDELTA",              drv_cmd_set_roam_delta},
	{"GETROAMDELTA",              drv_cmd_get_roam_delta},
	{"GETBAND",                   drv_cmd_get_band},
	{"SETROAMSCANCHANNELS",       drv_cmd_set_roam_scan_channels},
	{"GETROAMSCANCHANNELS",       drv_cmd_get_roam_scan_channels},
	{"GETCCXMODE",                drv_cmd_get_ccx_mode},
	{"GETOKCMODE",                drv_cmd_get_okc_mode},
	{"GETFASTROAM",               drv_cmd_get_fast_roam},
	{"GETFASTTRANSITION",         drv_cmd_get_fast_transition},
	{"SETROAMSCANCHANNELMINTIME", drv_cmd_set_roam_scan_channel_min_time},
	{"SENDACTIONFRAME",           drv_cmd_send_action_frame},
	{"GETROAMSCANCHANNELMINTIME", drv_cmd_get_roam_scan_channel_min_time},
	{"SETSCANCHANNELTIME",        drv_cmd_set_scan_channel_time},
	{"GETSCANCHANNELTIME",        drv_cmd_get_scan_channel_time},
	{"SETSCANHOMETIME",           drv_cmd_set_scan_home_time},
	{"GETSCANHOMETIME",           drv_cmd_get_scan_home_time},
	{"SETROAMINTRABAND",          drv_cmd_set_roam_intra_band},
	{"GETROAMINTRABAND",          drv_cmd_get_roam_intra_band},
	{"SETSCANNPROBES",            drv_cmd_set_scan_n_probes},
	{"GETSCANNPROBES",            drv_cmd_get_scan_n_probes},
	{"SETSCANHOMEAWAYTIME",       drv_cmd_set_scan_home_away_time},
	{"GETSCANHOMEAWAYTIME",       drv_cmd_get_scan_home_away_time},
	{"REASSOC",                   drv_cmd_reassoc},
	{"SETWESMODE",                drv_cmd_set_wes_mode},
	{"GETWESMODE",                drv_cmd_get_wes_mode},
	{"SETOPPORTUNISTICRSSIDIFF",  drv_cmd_set_opportunistic_rssi_diff},
	{"GETOPPORTUNISTICRSSIDIFF",  drv_cmd_get_opportunistic_rssi_diff},
	{"SETROAMRESCANRSSIDIFF",     drv_cmd_set_roam_rescan_rssi_diff},
	{"GETROAMRESCANRSSIDIFF",     drv_cmd_get_roam_rescan_rssi_diff},
	{"SETFASTROAM",               drv_cmd_set_fast_roam},
	{"SETFASTTRANSITION",         drv_cmd_set_fast_transition},
	{"FASTREASSOC",               drv_cmd_fast_reassoc},
#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
	{"CCXPLMREQ",                 drv_cmd_ccx_plm_req},
#endif
#ifdef FEATURE_WLAN_ESE
	{"SETCCXMODE",                drv_cmd_set_ccx_mode},
#endif
	{"SETROAMSCANCONTROL",        drv_cmd_set_roam_scan_control},
#ifdef FEATURE_WLAN_OKC
	{"SETOKCMODE",                drv_cmd_set_okc_mode},
#endif /* FEATURE_WLAN_OKC */
	{"GETROAMSCANCONTROL",        drv_cmd_get_roam_scan_control},
	{"BTCOEXMODE",                drv_cmd_bt_coex_mode},
	{"SCAN-ACTIVE",               drv_cmd_scan_active},
	{"SCAN-PASSIVE",              drv_cmd_scan_passive},
	{"GETDWELLTIME",              drv_cmd_get_dwell_time},
	{"SETDWELLTIME",              drv_cmd_set_dwell_time},
	{"MIRACAST",                  drv_cmd_miracast},
#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
	{"SETCCXROAMSCANCHANNELS",    drv_cmd_set_ccx_roam_scan_channels},
	{"GETTSMSTATS",               drv_cmd_get_tsm_stats},
	{"SETCCKMIE",                 drv_cmd_set_cckm_ie},
	{"CCXBEACONREQ",  drv_cmd_ccx_beacon_req},
#endif /* FEATURE_WLAN_ESE && FEATURE_WLAN_ESE_UPLOAD */
	{"SETMCRATE",                 drv_cmd_set_mc_rate},
	{"MAXTXPOWER",                drv_cmd_max_tx_power},
	{"SETDFSSCANMODE",            drv_cmd_set_dfs_scan_mode},
	{"GETDFSSCANMODE",            drv_cmd_get_dfs_scan_mode},
	{"GETLINKSTATUS",             drv_cmd_get_link_status},
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	{"ENABLEEXTWOW",              drv_cmd_enable_ext_wow},
	{"SETAPP1PARAMS",             drv_cmd_set_app1_params},
	{"SETAPP2PARAMS",             drv_cmd_set_app2_params},
#endif
#ifdef FEATURE_WLAN_TDLS
	{"TDLSSECONDARYCHANNELOFFSET", drv_cmd_tdls_secondary_channel_offset},
	{"TDLSOFFCHANNELMODE",        drv_cmd_tdls_off_channel_mode},
	{"TDLSOFFCHANNEL",            drv_cmd_tdls_off_channel},
	{"TDLSSCAN",                  drv_cmd_tdls_scan},
#endif
	{"RSSI",                      drv_cmd_get_rssi},
	{"LINKSPEED",                 drv_cmd_get_linkspeed},
#ifdef FEATURE_NAPI
	{"NAPI",                      drv_cmd_napi},
#endif /* FEATURE_NAPI */
	{"RXFILTER-REMOVE",           drv_cmd_rx_filter_remove},
	{"RXFILTER-ADD",              drv_cmd_rx_filter_add},
	{"SET_FCC_CHANNEL",           drv_cmd_set_fcc_channel},
	{"CHANNEL_SWITCH",            drv_cmd_set_channel_switch},
};

/**
 * hdd_drv_cmd_process() - chooses and runs the proper
 *                                handler based on the input command
 * @adapter:	Pointer to the hdd adapter
 * @cmd:	Pointer to the driver command
 * @priv_data:	Pointer to the data associated with the command
 *
 * This function parses the input hdd driver command and runs
 * the proper handler
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_drv_cmd_process(hdd_adapter_t *adapter,
			       uint8_t *cmd,
			       hdd_priv_data_t *priv_data)
{
	hdd_context_t *hdd_ctx;
	int i;
	const int cmd_num_total = ARRAY_SIZE(hdd_drv_cmds);
	uint8_t *cmd_i = NULL;
	hdd_drv_cmd_handler_t handler = NULL;
	int len = 0;

	if (!adapter || !cmd || !priv_data) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: at least 1 param is NULL", __func__);
		return -EINVAL;
	}

	hdd_ctx = (hdd_context_t *)adapter->pHddCtx;

	for (i = 0; i < cmd_num_total; i++) {

		cmd_i = (uint8_t *)hdd_drv_cmds[i].cmd;
		handler = hdd_drv_cmds[i].handler;
		len = strlen(cmd_i);

		if (!handler) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: no. %d handler is NULL", __func__, i);
			return -EINVAL;
		}

		if (strncasecmp(cmd, cmd_i, len) == 0)
			return handler(adapter, hdd_ctx,
				       cmd, len, priv_data);
	}

	return drv_cmd_invalid(adapter, hdd_ctx, cmd, len, priv_data);
}

/**
 * hdd_driver_command() - top level wlan hdd driver command handler
 * @adapter:	Pointer to the hdd adapter
 * @priv_data:	Pointer to the raw command data
 *
 * This function is the top level wlan hdd driver command handler. It
 * handles the command with the help of hdd_drv_cmd_process()
 *
 * Return: 0 for success non-zero for failure
 */
static int hdd_driver_command(hdd_adapter_t *adapter,
			      hdd_priv_data_t *priv_data)
{
	uint8_t *command = NULL;
	int ret = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	/*
	 * Note that valid pointers are provided by caller
	 */

	/* copy to local struct to avoid numerous changes to legacy code */
	if (priv_data->total_len <= 0 ||
	    priv_data->total_len > WLAN_PRIV_DATA_MAX_LEN) {
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s:invalid priv_data.total_len(%d)!!!", __func__,
		       priv_data->total_len);
		ret = -EINVAL;
		goto exit;
	}

	/* Allocate +1 for '\0' */
	command = kmalloc(priv_data->total_len + 1, GFP_KERNEL);
	if (!command) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: failed to allocate memory", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(command, priv_data->buf, priv_data->total_len)) {
		ret = -EFAULT;
		goto exit;
	}

	/* Make sure the command is NUL-terminated */
	command[priv_data->total_len] = '\0';

	hdd_info("%s: %s", adapter->dev->name, command);
	ret = hdd_drv_cmd_process(adapter, command, priv_data);

exit:
	if (command)
		kfree(command);
	EXIT();
	return ret;
}

#ifdef CONFIG_COMPAT
static int hdd_driver_compat_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	struct {
		compat_uptr_t buf;
		int used_len;
		int total_len;
	} compat_priv_data;
	hdd_priv_data_t priv_data;
	int ret = 0;

	/*
	 * Note that adapter and ifr have already been verified by caller,
	 * and HDD context has also been validated
	 */
	if (copy_from_user(&compat_priv_data, ifr->ifr_data,
			   sizeof(compat_priv_data))) {
		ret = -EFAULT;
		goto exit;
	}
	priv_data.buf = compat_ptr(compat_priv_data.buf);
	priv_data.used_len = compat_priv_data.used_len;
	priv_data.total_len = compat_priv_data.total_len;
	ret = hdd_driver_command(adapter, &priv_data);
exit:
	return ret;
}
#else /* CONFIG_COMPAT */
static int hdd_driver_compat_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	/* will never be invoked */
	return 0;
}
#endif /* CONFIG_COMPAT */

static int hdd_driver_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	hdd_priv_data_t priv_data;
	int ret = 0;

	/*
	 * Note that adapter and ifr have already been verified by caller,
	 * and HDD context has also been validated
	 */
	if (copy_from_user(&priv_data, ifr->ifr_data, sizeof(priv_data)))
		ret = -EFAULT;
	else
		ret = hdd_driver_command(adapter, &priv_data);

	return ret;
}

/**
 * __hdd_ioctl() - ioctl handler for wlan network interfaces
 * @dev: device upon which the ioctl was received
 * @ifr: ioctl request information
 * @cmd: ioctl command
 *
 * This function does initial processing of wlan device ioctls.
 * Currently two flavors of ioctls are supported.  The primary ioctl
 * that is supported is the (SIOCDEVPRIVATE + 1) ioctl which is used
 * for Android "DRIVER" commands.  The other ioctl that is
 * conditionally supported is the SIOCIOCTLTX99 ioctl which is used
 * for FTM on some platforms.  This function simply verifies that the
 * driver is in a sane state, and that the ioctl is one of the
 * supported flavors, in which case flavor-specific handlers are
 * dispatched.
 *
 * Return: 0 on success, non-zero on error
 */
static int __hdd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	if (dev != adapter->dev) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_FATAL,
			  "%s: HDD adapter/dev inconsistency", __func__);
		ret = -ENODEV;
		goto exit;
	}

	if ((!ifr) || (!ifr->ifr_data)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: invalid data", __func__);
		ret = -EINVAL;
		goto exit;
	}
#if  defined(QCA_WIFI_FTM) && defined(LINUX_QCMBR)
	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		if (SIOCIOCTLTX99 == cmd) {
			ret = wlan_hdd_qcmbr_unified_ioctl(adapter, ifr);
			goto exit;
		}
	}
#endif

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		goto exit;

	switch (cmd) {
	case (SIOCDEVPRIVATE + 1):
		if (is_compat_task())
			ret = hdd_driver_compat_ioctl(adapter, ifr);
		else
			ret = hdd_driver_ioctl(adapter, ifr);
		break;
	default:
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: unknown ioctl %d",
		       __func__, cmd);
		ret = -EINVAL;
		break;
	}
exit:
	EXIT();
	return ret;
}

/**
 * hdd_ioctl() - ioctl handler (wrapper) for wlan network interfaces
 * @dev: device upon which the ioctl was received
 * @ifr: ioctl request information
 * @cmd: ioctl command
 *
 * This function acts as an SSR-protecting wrapper to __hdd_ioctl()
 * which is where the ioctls are really handled.
 *
 * Return: 0 on success, non-zero on error
 */
int hdd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ioctl(dev, ifr, cmd);
	cds_ssr_unprotect(__func__);
	return ret;
}
