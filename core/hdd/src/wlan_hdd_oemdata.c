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

#ifdef FEATURE_OEM_DATA_SUPPORT

/**
 *  DOC: wlan_hdd_oemdata.c
 *
 *  Support for generic OEM Data Request handling
 *
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>
#include "qwlan_version.h"
#include "cds_utils.h"
#include "wma.h"
#include "sme_api.h"

static struct hdd_context_s *p_hdd_ctx;

/**
 * populate_oem_data_cap() - populate oem capabilities
 * @adapter: device adapter
 * @data_cap: pointer to populate the capabilities
 *
 * Return: error code
 */
static int populate_oem_data_cap(hdd_adapter_t *adapter,
				 t_iw_oem_data_cap *data_cap)
{
	QDF_STATUS status;
	struct hdd_config *config;
	uint32_t num_chan;
	uint8_t *chan_list;
	hdd_context_t *hdd_ctx = adapter->pHddCtx;

	config = hdd_ctx->config;
	if (!config) {
		hdd_err("HDD configuration is null");
		return -EINVAL;
	}
	chan_list = qdf_mem_malloc(sizeof(uint8_t) * OEM_CAP_MAX_NUM_CHANNELS);
	if (NULL == chan_list) {
		hdd_err("Memory allocation failed");
		return -ENOMEM;
	}

	strlcpy(data_cap->oem_target_signature, OEM_TARGET_SIGNATURE,
		OEM_TARGET_SIGNATURE_LEN);
	data_cap->oem_target_type = hdd_ctx->target_type;
	data_cap->oem_fw_version = hdd_ctx->target_fw_version;
	data_cap->driver_version.major = QWLAN_VERSION_MAJOR;
	data_cap->driver_version.minor = QWLAN_VERSION_MINOR;
	data_cap->driver_version.patch = QWLAN_VERSION_PATCH;
	data_cap->driver_version.build = QWLAN_VERSION_BUILD;
	data_cap->allowed_dwell_time_min = config->nNeighborScanMinChanTime;
	data_cap->allowed_dwell_time_max = config->nNeighborScanMaxChanTime;
	data_cap->curr_dwell_time_min =
		sme_get_neighbor_scan_min_chan_time(hdd_ctx->hHal,
						    adapter->sessionId);
	data_cap->curr_dwell_time_max =
		sme_get_neighbor_scan_max_chan_time(hdd_ctx->hHal,
						    adapter->sessionId);
	data_cap->supported_bands = config->nBandCapability;

	/* request for max num of channels */
	num_chan = OEM_CAP_MAX_NUM_CHANNELS;
	status = sme_get_cfg_valid_channels(hdd_ctx->hHal,
					    &chan_list[0], &num_chan);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("failed to get valid channel list, status: %d", status);
		qdf_mem_free(chan_list);
		return -EINVAL;
	}

	/* make sure num channels is not more than chan list array */
	if (num_chan > OEM_CAP_MAX_NUM_CHANNELS) {
		hdd_err("Num of channels-%d > length-%d of chan_list",
			num_chan, OEM_CAP_MAX_NUM_CHANNELS);
		qdf_mem_free(chan_list);
		return -ENOMEM;
	}

	data_cap->num_channels = num_chan;
	qdf_mem_copy(data_cap->channel_list, chan_list,
		     sizeof(uint8_t) * num_chan);

	qdf_mem_free(chan_list);
	return 0;
}

/**
 * iw_get_oem_data_cap() - Get OEM Data Capabilities
 * @dev: net device upon which the request was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl data payload
 *
 * This function gets the capability information for OEM Data Request
 * and Response.
 *
 * Return: 0 for success, negative errno value on failure
 */
int iw_get_oem_data_cap(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int status;
	t_iw_oem_data_cap oemDataCap = { {0} };
	t_iw_oem_data_cap *pHddOemDataCap;
	hdd_adapter_t *pAdapter = (netdev_priv(dev));
	hdd_context_t *pHddContext;
	int ret;

	ENTER();

	pHddContext = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddContext);
	if (0 != ret)
		return ret;

	status = populate_oem_data_cap(pAdapter, &oemDataCap);
	if (0 != status) {
		hdd_err("Failed to populate oem data capabilities");
		return status;
	}

	pHddOemDataCap = (t_iw_oem_data_cap *) (extra);
	*pHddOemDataCap = oemDataCap;

	EXIT();
	return 0;
}

/**
 * send_oem_reg_rsp_nlink_msg() - send oem registration response
 *
 * This function sends oem message to registered application process
 *
 * Return:  none
 */
static void send_oem_reg_rsp_nlink_msg(void)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *aniHdr;
	uint8_t *buf;
	uint8_t *numInterfaces;
	uint8_t *deviceMode;
	uint8_t *vdevId;
	hdd_adapter_list_node_t *pAdapterNode = NULL;
	hdd_adapter_list_node_t *pNext = NULL;
	hdd_adapter_t *pAdapter = NULL;
	QDF_STATUS status = 0;

	/* OEM message is always to a specific process and cannot be a broadcast */
	if (p_hdd_ctx->oem_pid == 0) {
		hdd_err("invalid dest pid");
		return;
	}

	skb = alloc_skb(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD), GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	aniHdr = NLMSG_DATA(nlh);
	aniHdr->type = ANI_MSG_APP_REG_RSP;

	/* Fill message body:
	 *   First byte will be number of interfaces, followed by
	 *   two bytes for each interfaces
	 *     - one byte for device mode
	 *     - one byte for vdev id
	 */
	buf = (char *)((char *)aniHdr + sizeof(tAniMsgHdr));
	numInterfaces = buf++;
	*numInterfaces = 0;

	/* Iterate through each of the adapters and fill device mode and vdev id */
	status = hdd_get_front_adapter(p_hdd_ctx, &pAdapterNode);
	while ((QDF_STATUS_SUCCESS == status) && pAdapterNode) {
		pAdapter = pAdapterNode->pAdapter;
		if (pAdapter) {
			deviceMode = buf++;
			vdevId = buf++;
			*deviceMode = pAdapter->device_mode;
			*vdevId = pAdapter->sessionId;
			(*numInterfaces)++;
			hdd_notice("numInterfaces: %d, deviceMode: %d, vdevId: %d",
				   *numInterfaces, *deviceMode,
				   *vdevId);
		}
		status = hdd_get_next_adapter(p_hdd_ctx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	aniHdr->length =
		sizeof(uint8_t) + (*numInterfaces) * 2 * sizeof(uint8_t);
	nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + aniHdr->length));

	skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr) + aniHdr->length)));

	hdd_notice("sending App Reg Response length (%d) to process pid (%d)",
		   aniHdr->length, p_hdd_ctx->oem_pid);

	(void)nl_srv_ucast(skb, p_hdd_ctx->oem_pid, MSG_DONTWAIT);

	return;
}

/**
 * send_oem_err_rsp_nlink_msg() - send oem error response
 * @app_pid: PID of oem application process
 * @error_code: response error code
 *
 * This function sends error response to oem app
 *
 * Return: none
 */
static void send_oem_err_rsp_nlink_msg(int32_t app_pid, uint8_t error_code)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *aniHdr;
	uint8_t *buf;

	skb = alloc_skb(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD), GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	aniHdr = NLMSG_DATA(nlh);
	aniHdr->type = ANI_MSG_OEM_ERROR;
	aniHdr->length = sizeof(uint8_t);
	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(tAniMsgHdr) + aniHdr->length);

	/* message body will contain one byte of error code */
	buf = (char *)((char *)aniHdr + sizeof(tAniMsgHdr));
	*buf = error_code;

	skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr) + aniHdr->length));

	hdd_notice("sending oem error response to process pid (%d)", app_pid);

	(void)nl_srv_ucast(skb, app_pid, MSG_DONTWAIT);

	return;
}

/**
 * hdd_send_oem_data_rsp_msg() - send oem data response
 * @oem_data_rsp: the actual OEM Data Response message
 *
 * This function sends an OEM Data Response message to a registered
 * application process over the netlink socket.
 *
 * Return: 0 for success, non zero for failure
 */
void hdd_send_oem_data_rsp_msg(struct oem_data_rsp *oem_data_rsp)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *ani_hdr;
	uint8_t *oem_data;

	/*
	 * OEM message is always to a specific process and cannot be a broadcast
	 */
	if (p_hdd_ctx->oem_pid == 0) {
		hdd_err("invalid dest pid");
		return;
	}

	if (oem_data_rsp->rsp_len > OEM_DATA_RSP_SIZE) {
		hdd_err("invalid length of Oem Data response");
		return;
	}

	skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) + OEM_DATA_RSP_SIZE),
			GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	ani_hdr = NLMSG_DATA(nlh);
	ani_hdr->type = ANI_MSG_OEM_DATA_RSP;

	ani_hdr->length = oem_data_rsp->rsp_len;
	nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + ani_hdr->length));
	oem_data = (uint8_t *) ((char *)ani_hdr + sizeof(tAniMsgHdr));
	qdf_mem_copy(oem_data, oem_data_rsp->data, oem_data_rsp->rsp_len);

	skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr) + ani_hdr->length)));

	hdd_notice("sending Oem Data Response of len (%d) to process pid (%d)",
		   oem_data_rsp->rsp_len, p_hdd_ctx->oem_pid);

	(void)nl_srv_ucast(skb, p_hdd_ctx->oem_pid, MSG_DONTWAIT);

	return;
}

/**
 * oem_process_data_req_msg() - process oem data request
 * @oem_data_len: Length to OEM Data buffer
 * @oem_data: Pointer to OEM Data buffer
 *
 * This function sends oem message to SME
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS oem_process_data_req_msg(int oem_data_len, char *oem_data)
{
	hdd_adapter_t *adapter = NULL;
	struct oem_data_req oem_data_req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* for now, STA interface only */
	adapter = hdd_get_adapter(p_hdd_ctx, QDF_STA_MODE);
	if (!adapter) {
		hdd_err("No adapter for STA mode");
		return QDF_STATUS_E_FAILURE;
	}

	if (!oem_data) {
		hdd_err("oem_data is null");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&oem_data_req, sizeof(oem_data_req));

	oem_data_req.data = qdf_mem_malloc(oem_data_len);
	if (!oem_data_req.data) {
		hdd_err("malloc failed for data req buffer");
		return QDF_STATUS_E_NOMEM;
	}

	oem_data_req.data_len = oem_data_len;
	qdf_mem_copy(oem_data_req.data, oem_data, oem_data_len);

	hdd_notice("calling sme_oem_data_req");

	status = sme_oem_data_req(p_hdd_ctx->hHal, &oem_data_req);

	qdf_mem_free(oem_data_req.data);
	oem_data_req.data = NULL;

	return status;
}

/**
 * update_channel_bw_info() - set bandwidth info for the chan
 * @hdd_ctx: hdd context
 * @chan: channel for which info are required
 * @hdd_chan_info: struct where the bandwidth info is filled
 *
 * This function find the maximum bandwidth allowed, secondary
 * channel offset and center freq for the channel as per regulatory
 * domain and using these info calculate the phy mode for the
 * channel.
 *
 * Return: void
 */
static void hdd_update_channel_bw_info(hdd_context_t *hdd_ctx,
	uint16_t chan, tHddChannelInfo *hdd_chan_info)
{
	struct ch_params_s ch_params = {0};
	uint16_t sec_ch_2g = 0;
	WLAN_PHY_MODE phy_mode;
	uint32_t wni_dot11_mode;

	wni_dot11_mode = sme_get_wni_dot11_mode(hdd_ctx->hHal);

	/* Passing CH_WIDTH_MAX will give the max bandwidth supported */
	ch_params.ch_width = CH_WIDTH_MAX;

	cds_set_channel_params(chan, sec_ch_2g, &ch_params);
	if (ch_params.center_freq_seg0)
		hdd_chan_info->band_center_freq1 =
			cds_chan_to_freq(ch_params.center_freq_seg0);

	hdd_info("chan %d dot11_mode %d ch_width %d sec offset %d freq_seg0 %d",
		chan, wni_dot11_mode, ch_params.ch_width,
		ch_params.sec_ch_offset, hdd_chan_info->band_center_freq1);

	phy_mode = wma_chan_phy_mode(chan, ch_params.ch_width, wni_dot11_mode);
	WMI_SET_CHANNEL_MODE(hdd_chan_info, phy_mode);
}

/**
 * oem_process_channel_info_req_msg() - process oem channel_info request
 * @numOfChannels: number of channels
 * @chanList: list of channel information
 *
 * This function responds with channel info to oem process
 *
 * Return: 0 for success, non zero for failure
 */
static int oem_process_channel_info_req_msg(int numOfChannels, char *chanList)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *aniHdr;
	tHddChannelInfo *pHddChanInfo;
	tHddChannelInfo hddChanInfo;
	uint8_t chanId;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int i;
	uint8_t *buf;

	/* OEM message is always to a specific process and cannot be a broadcast */
	if (p_hdd_ctx->oem_pid == 0) {
		hdd_err("invalid dest pid");
		return -EPERM;
	}

	skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) + sizeof(uint8_t) +
				    numOfChannels * sizeof(tHddChannelInfo)),
			GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return -ENOMEM;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	aniHdr = NLMSG_DATA(nlh);
	aniHdr->type = ANI_MSG_CHANNEL_INFO_RSP;

	aniHdr->length =
		sizeof(uint8_t) + numOfChannels * sizeof(tHddChannelInfo);
	nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + aniHdr->length));

	/* First byte of message body will have num of channels */
	buf = (char *)((char *)aniHdr + sizeof(tAniMsgHdr));
	*buf++ = numOfChannels;

	/* Next follows channel info struct for each channel id.
	 * If chan id is wrong or SME returns failure for a channel
	 * then fill in 0 in channel info for that particular channel
	 */
	for (i = 0; i < numOfChannels; i++) {
		pHddChanInfo = (tHddChannelInfo *) ((char *)buf +
						    i *
						    sizeof(tHddChannelInfo));

		chanId = chanList[i];
		status = sme_get_reg_info(p_hdd_ctx->hHal, chanId,
					  &reg_info_1, &reg_info_2);
		if (QDF_STATUS_SUCCESS == status) {
			/* copy into hdd chan info struct */
			hddChanInfo.chan_id = chanId;
			hddChanInfo.reserved0 = 0;
			hddChanInfo.mhz = cds_chan_to_freq(chanId);
			hddChanInfo.band_center_freq1 = hddChanInfo.mhz;
			hddChanInfo.band_center_freq2 = 0;

			hddChanInfo.info = 0;
			if (CHANNEL_STATE_DFS ==
			    cds_get_channel_state(chanId))
				WMI_SET_CHANNEL_FLAG(&hddChanInfo,
						     WMI_CHAN_FLAG_DFS);

			hdd_update_channel_bw_info(p_hdd_ctx,
						chanId, &hddChanInfo);
			hddChanInfo.reg_info_1 = reg_info_1;
			hddChanInfo.reg_info_2 = reg_info_2;
		} else {
			/* channel info is not returned, fill in zeros in channel
			 * info struct
			 */
			hdd_notice("sme_get_reg_info failed for chan (%d), return info 0",
				   chanId);
			hddChanInfo.chan_id = chanId;
			hddChanInfo.reserved0 = 0;
			hddChanInfo.mhz = 0;
			hddChanInfo.band_center_freq1 = 0;
			hddChanInfo.band_center_freq2 = 0;
			hddChanInfo.info = 0;
			hddChanInfo.reg_info_1 = 0;
			hddChanInfo.reg_info_2 = 0;
		}
		qdf_mem_copy(pHddChanInfo, &hddChanInfo,
			     sizeof(tHddChannelInfo));
	}

	skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr) + aniHdr->length)));

	hdd_notice("sending channel info resp for num channels (%d) to pid (%d)",
		   numOfChannels, p_hdd_ctx->oem_pid);

	(void)nl_srv_ucast(skb, p_hdd_ctx->oem_pid, MSG_DONTWAIT);

	return 0;
}

/**
 * oem_process_set_cap_req_msg() - process oem set capability request
 * @oem_cap_len: Length of OEM capability
 * @oem_cap: Pointer to OEM capability buffer
 * @app_pid: process ID, to which rsp message is to be sent
 *
 * This function sends oem message to SME
 *
 * Return: error code
 */
static int oem_process_set_cap_req_msg(int oem_cap_len,
				       char *oem_cap, int32_t app_pid)
{
	QDF_STATUS status;
	int error_code;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *ani_hdr;
	uint8_t *buf;

	if (!oem_cap) {
		hdd_err("oem_cap is null");
		return -EINVAL;
	}

	status = sme_oem_update_capability(p_hdd_ctx->hHal,
					(struct sme_oem_capability *)oem_cap);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("error updating rm capability, status: %d", status);
	error_code = qdf_status_to_os_return(status);

	skb = alloc_skb(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD), GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return -ENOMEM;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	ani_hdr = NLMSG_DATA(nlh);
	ani_hdr->type = ANI_MSG_SET_OEM_CAP_RSP;
	/* 64 bit alignment */
	ani_hdr->length = sizeof(error_code);
	nlh->nlmsg_len = NLMSG_LENGTH(sizeof(tAniMsgHdr) + ani_hdr->length);

	/* message body will contain only status code */
	buf = (char *)((char *)ani_hdr + sizeof(tAniMsgHdr));
	qdf_mem_copy(buf, &error_code, ani_hdr->length);

	skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr) + ani_hdr->length));

	hdd_info("sending oem response to process pid %d", app_pid);

	(void)nl_srv_ucast(skb, app_pid, MSG_DONTWAIT);

	return error_code;
}

/**
 * oem_process_get_cap_req_msg() - process oem get capability request
 *
 * This function process the get capability request from OEM and responds
 * with the capability.
 *
 * Return: error code
 */
static int oem_process_get_cap_req_msg(void)
{
	int error_code;
	struct oem_get_capability_rsp *cap_rsp;
	t_iw_oem_data_cap data_cap = { {0} };
	struct sme_oem_capability oem_cap;
	hdd_adapter_t *adapter;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *ani_hdr;
	uint8_t *buf;

	/* for now, STA interface only */
	adapter = hdd_get_adapter(p_hdd_ctx, QDF_STA_MODE);
	if (!adapter) {
		hdd_err("No adapter for STA mode");
		return -EINVAL;
	}

	error_code = populate_oem_data_cap(adapter, &data_cap);
	if (0 != error_code)
		return error_code;

	skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) + sizeof(*cap_rsp)),
			GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return -ENOMEM;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	ani_hdr = NLMSG_DATA(nlh);
	ani_hdr->type = ANI_MSG_GET_OEM_CAP_RSP;

	ani_hdr->length = sizeof(*cap_rsp);
	nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + ani_hdr->length));

	buf = (char *)((char *)ani_hdr + sizeof(tAniMsgHdr));
	qdf_mem_copy(buf, &data_cap, sizeof(data_cap));

	buf = (char *) buf +  sizeof(data_cap);
	qdf_mem_zero(&oem_cap, sizeof(oem_cap));
	sme_oem_get_capability(p_hdd_ctx->hHal, &oem_cap);
	qdf_mem_copy(buf, &oem_cap, sizeof(oem_cap));

	skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr) + ani_hdr->length)));
	hdd_info("send rsp to oem-pid:%d for get_capability",
		 p_hdd_ctx->oem_pid);

	(void)nl_srv_ucast(skb, p_hdd_ctx->oem_pid, MSG_DONTWAIT);
	return 0;
}

/**
 * hdd_send_peer_status_ind_to_oem_app() -
 * Function to send peer status to a registered application
 * @peerMac: MAC address of peer
 * @peerStatus: ePeerConnected or ePeerDisconnected
 * @peerTimingMeasCap: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @sessionId: SME session id, i.e. vdev_id
 * @chan_info: operating channel information
 * @dev_mode: dev mode for which indication is sent
 *
 * Return: none
 */
void hdd_send_peer_status_ind_to_oem_app(struct qdf_mac_addr *peerMac,
					 uint8_t peerStatus,
					 uint8_t peerTimingMeasCap,
					 uint8_t sessionId,
					 tSirSmeChanInfo *chan_info,
					 enum tQDF_ADAPTER_MODE dev_mode)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	tAniMsgHdr *aniHdr;
	tPeerStatusInfo *pPeerInfo;

	if (!p_hdd_ctx || !p_hdd_ctx->hHal) {
		hdd_err("Either HDD Ctx is null or Hal Ctx is null");
		return;
	}

	/* check if oem app has registered and pid is valid */
	if ((!p_hdd_ctx->oem_app_registered) || (p_hdd_ctx->oem_pid == 0)) {
		hdd_info("OEM app is not registered(%d) or pid is invalid(%d)",
			 p_hdd_ctx->oem_app_registered,
			 p_hdd_ctx->oem_pid);
		return;
	}

	skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) +
				    sizeof(tPeerStatusInfo)),
			GFP_KERNEL);
	if (skb == NULL) {
		hdd_err("alloc_skb failed");
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;
	nlh->nlmsg_pid = 0;     /* from kernel */
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_type = WLAN_NL_MSG_OEM;
	aniHdr = NLMSG_DATA(nlh);
	aniHdr->type = ANI_MSG_PEER_STATUS_IND;

	aniHdr->length = sizeof(tPeerStatusInfo);
	nlh->nlmsg_len = NLMSG_LENGTH((sizeof(tAniMsgHdr) + aniHdr->length));

	pPeerInfo = (tPeerStatusInfo *) ((char *)aniHdr + sizeof(tAniMsgHdr));

	qdf_mem_copy(pPeerInfo->peer_mac_addr, peerMac->bytes,
		     sizeof(peerMac->bytes));
	pPeerInfo->peer_status = peerStatus;
	pPeerInfo->vdev_id = sessionId;
	pPeerInfo->peer_capability = peerTimingMeasCap;
	pPeerInfo->reserved0 = 0;
	/* Set 0th bit of reserved0 for STA mode */
	if (QDF_STA_MODE == dev_mode)
		pPeerInfo->reserved0 |= 0x01;

	if (chan_info) {
		pPeerInfo->peer_chan_info.chan_id = chan_info->chan_id;
		pPeerInfo->peer_chan_info.reserved0 = 0;
		pPeerInfo->peer_chan_info.mhz = chan_info->mhz;
		pPeerInfo->peer_chan_info.band_center_freq1 =
			chan_info->band_center_freq1;
		pPeerInfo->peer_chan_info.band_center_freq2 =
			chan_info->band_center_freq2;
		pPeerInfo->peer_chan_info.info = chan_info->info;
		pPeerInfo->peer_chan_info.reg_info_1 = chan_info->reg_info_1;
		pPeerInfo->peer_chan_info.reg_info_2 = chan_info->reg_info_2;
	} else {
		pPeerInfo->peer_chan_info.chan_id = 0;
		pPeerInfo->peer_chan_info.reserved0 = 0;
		pPeerInfo->peer_chan_info.mhz = 0;
		pPeerInfo->peer_chan_info.band_center_freq1 = 0;
		pPeerInfo->peer_chan_info.band_center_freq2 = 0;
		pPeerInfo->peer_chan_info.info = 0;
		pPeerInfo->peer_chan_info.reg_info_1 = 0;
		pPeerInfo->peer_chan_info.reg_info_2 = 0;
	}
	skb_put(skb, NLMSG_SPACE((sizeof(tAniMsgHdr) + aniHdr->length)));

	hdd_info("sending peer " MAC_ADDRESS_STR
		  " status(%d), peerTimingMeasCap(%d), vdevId(%d), chanId(%d)"
		  " to oem app pid(%d), center freq 1 (%d), center freq 2 (%d),"
		  " info (0x%x), frequency (%d),reg info 1 (0x%x),"
		  " reg info 2 (0x%x)",
		  MAC_ADDR_ARRAY(peerMac->bytes),
		  peerStatus, peerTimingMeasCap,
		  sessionId, pPeerInfo->peer_chan_info.chan_id,
		  p_hdd_ctx->oem_pid,
		  pPeerInfo->peer_chan_info.band_center_freq1,
		  pPeerInfo->peer_chan_info.band_center_freq2,
		  pPeerInfo->peer_chan_info.info,
		  pPeerInfo->peer_chan_info.mhz,
		  pPeerInfo->peer_chan_info.reg_info_1,
		  pPeerInfo->peer_chan_info.reg_info_2);

	(void)nl_srv_ucast(skb, p_hdd_ctx->oem_pid, MSG_DONTWAIT);

	return;
}

/*
 * Callback function invoked by Netlink service for all netlink
 * messages (from user space) addressed to WLAN_NL_MSG_OEM
 */

/**
 * oem_msg_callback() - callback invoked by netlink service
 * @skb:    skb with netlink message
 *
 * This function gets invoked by netlink service when a message
 * is received from user space addressed to WLAN_NL_MSG_OEM
 *
 * Return: zero on success
 *         On error, error number will be returned.
 */
static int oem_msg_callback(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	tAniMsgHdr *msg_hdr;
	int ret;
	char *sign_str = NULL;
	nlh = (struct nlmsghdr *)skb->data;

	if (!nlh) {
		hdd_err("Netlink header null");
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(p_hdd_ctx);
	if (ret)
		return ret;

	msg_hdr = NLMSG_DATA(nlh);

	if (!msg_hdr) {
		hdd_err("Message header null");
		send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
					   OEM_ERR_NULL_MESSAGE_HEADER);
		return -EPERM;
	}

	if (nlh->nlmsg_len <
	    NLMSG_LENGTH(sizeof(tAniMsgHdr) + msg_hdr->length)) {
		hdd_err("Invalid nl msg len, nlh->nlmsg_len (%d), msg_hdr->len (%d)",
			nlh->nlmsg_len, msg_hdr->length);
		send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
					   OEM_ERR_INVALID_MESSAGE_LENGTH);
		return -EPERM;
	}

	switch (msg_hdr->type) {
	case ANI_MSG_APP_REG_REQ:
		/* Registration request is only allowed for Qualcomm Application */
		hdd_notice("Received App Req Req from App process pid(%d), len(%d)",
			   nlh->nlmsg_pid, msg_hdr->length);

		sign_str = (char *)((char *)msg_hdr + sizeof(tAniMsgHdr));
		if ((OEM_APP_SIGNATURE_LEN == msg_hdr->length) &&
		    (0 == strncmp(sign_str, OEM_APP_SIGNATURE_STR,
				  OEM_APP_SIGNATURE_LEN))) {
			hdd_notice("Valid App Req Req from oem app process pid(%d)",
				   nlh->nlmsg_pid);

			p_hdd_ctx->oem_app_registered = true;
			p_hdd_ctx->oem_pid = nlh->nlmsg_pid;
			send_oem_reg_rsp_nlink_msg();
		} else {
			hdd_err("Invalid signature in App Reg Request from pid(%d)",
				nlh->nlmsg_pid);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_INVALID_SIGNATURE);
			return -EPERM;
		}
		break;

	case ANI_MSG_OEM_DATA_REQ:
		hdd_notice("Received Oem Data Request length(%d) from pid: %d",
			   msg_hdr->length, nlh->nlmsg_pid);

		if ((!p_hdd_ctx->oem_app_registered) ||
		    (nlh->nlmsg_pid != p_hdd_ctx->oem_pid)) {
			/* either oem app is not registered yet or pid is different */
			hdd_err("OEM DataReq: app not registered(%d) or incorrect pid(%d)",
				p_hdd_ctx->oem_app_registered,
				nlh->nlmsg_pid);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_APP_NOT_REGISTERED);
			return -EPERM;
		}

		if ((!msg_hdr->length) || (OEM_DATA_REQ_SIZE < msg_hdr->length)) {
			hdd_err("Invalid length (%d) in Oem Data Request",
				msg_hdr->length);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_INVALID_MESSAGE_LENGTH);
			return -EPERM;
		}
		oem_process_data_req_msg(msg_hdr->length,
					 (char *)((char *)msg_hdr +
						  sizeof(tAniMsgHdr)));
		break;

	case ANI_MSG_CHANNEL_INFO_REQ:
		hdd_notice("Received channel info request, num channel(%d) from pid: %d",
			   msg_hdr->length, nlh->nlmsg_pid);

		if ((!p_hdd_ctx->oem_app_registered) ||
		    (nlh->nlmsg_pid != p_hdd_ctx->oem_pid)) {
			/* either oem app is not registered yet or pid is different */
			hdd_err("Chan InfoReq: app not registered(%d) or incorrect pid(%d)",
				p_hdd_ctx->oem_app_registered,
				nlh->nlmsg_pid);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_APP_NOT_REGISTERED);
			return -EPERM;
		}

		/* message length contains list of channel ids */
		if ((!msg_hdr->length) ||
		    (WNI_CFG_VALID_CHANNEL_LIST_LEN < msg_hdr->length)) {
			hdd_err("Invalid length (%d) in channel info request",
				msg_hdr->length);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_INVALID_MESSAGE_LENGTH);
			return -EPERM;
		}
		oem_process_channel_info_req_msg(msg_hdr->length,
						 (char *)((char *)msg_hdr +
							  sizeof(tAniMsgHdr)));
		break;

	case ANI_MSG_SET_OEM_CAP_REQ:
		hdd_info("Received set oem capability req of length:%d from pid: %d",
			 msg_hdr->length, nlh->nlmsg_pid);

		if ((!p_hdd_ctx->oem_app_registered) ||
		    (nlh->nlmsg_pid != p_hdd_ctx->oem_pid)) {
			/* oem app is not registered yet or pid is different */
			hdd_err("set_oem_capability : app not registered(%d) or incorrect pid(%d)",
				p_hdd_ctx->oem_app_registered,
				nlh->nlmsg_pid);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_APP_NOT_REGISTERED);
			return -EPERM;
		}

		if ((!msg_hdr->length) ||
			(sizeof(struct sme_oem_capability) < msg_hdr->length)) {
			hdd_err("Invalid length (%d) in set_oem_capability",
				msg_hdr->length);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						OEM_ERR_INVALID_MESSAGE_LENGTH);
			return -EPERM;
		}

		oem_process_set_cap_req_msg(msg_hdr->length,
					(char *)((char *)msg_hdr +
						sizeof(tAniMsgHdr)),
					nlh->nlmsg_pid);
		break;

	case ANI_MSG_GET_OEM_CAP_REQ:
		hdd_info("Rcvd get oem capability req of length:%d from pid: %d",
			 msg_hdr->length, nlh->nlmsg_pid);

		if ((!p_hdd_ctx->oem_app_registered) ||
		    (nlh->nlmsg_pid != p_hdd_ctx->oem_pid)) {
			/* oem app is not registered yet or pid is different */
			hdd_err("get_oem_capability : app not registered(%d) or incorrect pid(%d)",
				p_hdd_ctx->oem_app_registered,
				nlh->nlmsg_pid);
			send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
						   OEM_ERR_APP_NOT_REGISTERED);
			return -EPERM;
		}

		oem_process_get_cap_req_msg();
		break;

	default:
		hdd_err("Received Invalid message type (%d), length (%d)",
			msg_hdr->type, msg_hdr->length);
		send_oem_err_rsp_nlink_msg(nlh->nlmsg_pid,
					   OEM_ERR_INVALID_MESSAGE_TYPE);
		return -EPERM;
	}
	return 0;
}

static int __oem_msg_callback(struct sk_buff *skb)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = oem_msg_callback(skb);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * oem_activate_service() - Activate oem message handler
 * @hdd_ctx:   pointer to global HDD context
 *
 * This function registers a handler to receive netlink message from
 * an OEM application process.
 *
 * Return: zero on success
 *         On error, error number will be returned.
 */
int oem_activate_service(struct hdd_context_s *hdd_ctx)
{
	p_hdd_ctx = hdd_ctx;

	/* Register the msg handler for msgs addressed to WLAN_NL_MSG_OEM */
	return nl_srv_register(WLAN_NL_MSG_OEM, __oem_msg_callback);
}

#endif
