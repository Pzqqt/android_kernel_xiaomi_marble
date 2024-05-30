/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wifi_pos_main.c
 * This file defines the important functions pertinent to
 * wifi positioning to initialize and de-initialize the component.
 */
#include <wlan_lmac_if_def.h>
#include "target_if_wifi_pos.h"
#include "wifi_pos_oem_interface_i.h"
#include "wifi_pos_utils_i.h"
#include "wifi_pos_api.h"
#include "wifi_pos_main_i.h"
#include "wifi_pos_ucfg_i.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_ptt_sock_svc.h"

#ifndef CNSS_GENL
#include <wlan_objmgr_global_obj_i.h>
#endif

#include "wlan_reg_services_api.h"
/* forward declartion */
struct regulatory_channel;

#define REG_SET_CHANNEL_REG_POWER(reg_info_1, val) do { \
	reg_info_1 &= 0xff00ffff;           \
	reg_info_1 |= ((val & 0xff) << 16); \
} while (0)

/* max tx power is in 1 dBm units */
#define REG_SET_CHANNEL_MAX_TX_POWER(reg_info_2, val) do { \
	reg_info_2 &= 0xffff00ff;              \
	reg_info_2 |= ((val & 0xff) << 8);     \
} while (0)

/* channel info consists of 6 bits of channel mode */

#define REG_SET_CHANNEL_MODE(reg_channel, val) do { \
	(reg_channel)->info &= 0xffffffc0;            \
	(reg_channel)->info |= (val);                 \
} while (0)

/*
 * obj mgr api to iterate over vdevs does not provide a direct array or vdevs,
 * rather takes a callback that is called for every vdev. wifi pos needs to
 * store device mode and vdev id of all active vdevs and provide this info to
 * user space as part of APP registration response. due to this, vdev_idx is
 * used to identify how many vdevs have been populated by obj manager API.
 */
static uint32_t vdev_idx;

/**
 * wifi_pos_get_tlv_support: indicates if firmware supports TLV wifi pos msg
 * @psoc: psoc object
 *
 * Return: status of operation
 */
static bool wifi_pos_get_tlv_support(struct wlan_objmgr_psoc *psoc)
{
	/* this is TBD */
	return true;
}

struct wlan_lmac_if_wifi_pos_tx_ops *
	wifi_pos_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		wifi_pos_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->wifi_pos_tx_ops;
}

#ifdef CNSS_GENL
static uint8_t *
wifi_pos_prepare_reg_resp(uint32_t *rsp_len,
			  struct app_reg_rsp_vdev_info *vdevs_info)
{
	uint32_t *nl_sign;
	uint8_t *resp_buf;
	struct wifi_app_reg_rsp *app_reg_rsp;

	/*
	 * allocate ENHNC_FLAGS_LEN i.e. 4bytes extra memory in app_reg_resp
	 * to indicate NLA type response is supported for OEM request
	 * commands.
	 */
	*rsp_len = (sizeof(struct app_reg_rsp_vdev_info) * vdev_idx)
			+ sizeof(uint8_t) + ENHNC_FLAGS_LEN;
	resp_buf = qdf_mem_malloc(*rsp_len);
	if (!resp_buf)
		return NULL;

	app_reg_rsp = (struct wifi_app_reg_rsp *)resp_buf;
	app_reg_rsp->num_inf = vdev_idx;
	qdf_mem_copy(&app_reg_rsp->vdevs, vdevs_info,
		     sizeof(struct app_reg_rsp_vdev_info) * vdev_idx);

	nl_sign = (uint32_t *)&app_reg_rsp->vdevs[vdev_idx];
	*nl_sign |= NL_ENABLE_OEM_REQ_RSP;

	return resp_buf;
}

/**
 * wifi_pos_get_host_pdev_id: Get host pdev_id
 * @psoc: Pointer to psoc object
 * @tgt_pdev_id: target_pdev_id
 * @host_pdev_id: host pdev_id
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in case of failure
 */
static QDF_STATUS wifi_pos_get_host_pdev_id(
		struct wlan_objmgr_psoc *psoc, uint32_t tgt_pdev_id,
		uint32_t *host_pdev_id)
{
	/* pdev_id in FW starts from 1. So convert it to
	 * host id by decrementing it.
	 * zero has special meaning due to backward
	 * compatibility. Dont change it.
	 */
	if (tgt_pdev_id)
		*host_pdev_id = tgt_pdev_id - 1;
	else
		*host_pdev_id = tgt_pdev_id;

	return QDF_STATUS_SUCCESS;
}
#else
static uint8_t *
wifi_pos_prepare_reg_resp(uint32_t *rsp_len,
			  struct app_reg_rsp_vdev_info *vdevs_info)
{
	uint8_t *resp_buf;
	struct wifi_app_reg_rsp *app_reg_rsp;

	*rsp_len = (sizeof(struct app_reg_rsp_vdev_info) * vdev_idx)
			+ sizeof(uint8_t);
	resp_buf = qdf_mem_malloc(*rsp_len);
	if (!resp_buf)
		return NULL;

	app_reg_rsp = (struct wifi_app_reg_rsp *)resp_buf;
	app_reg_rsp->num_inf = vdev_idx;
	qdf_mem_copy(&app_reg_rsp->vdevs, vdevs_info,
		     sizeof(struct app_reg_rsp_vdev_info) * vdev_idx);

	return resp_buf;
}

static QDF_STATUS wifi_pos_get_host_pdev_id(
		struct wlan_objmgr_psoc *psoc, uint32_t tgt_pdev_id,
		uint32_t *host_pdev_id)
{
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

	tx_ops = wifi_pos_get_tx_ops(psoc);
	if (!tx_ops) {
		qdf_print("tx ops null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->wifi_pos_convert_pdev_id_target_to_host) {
		wifi_pos_err("wifi_pos_convert_pdev_id_target_to_host is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return tx_ops->wifi_pos_convert_pdev_id_target_to_host(
			psoc, tgt_pdev_id, host_pdev_id);
}
#endif

static QDF_STATUS wifi_pos_process_data_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	uint8_t idx;
	uint32_t sub_type = 0;
	uint32_t channel_mhz = 0;
	uint32_t host_pdev_id = 0, tgt_pdev_id = 0;
	uint32_t offset;
	struct oem_data_req data_req;
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;
	struct wlan_objmgr_pdev *pdev;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
				wifi_pos_get_psoc_priv_obj(wifi_pos_get_psoc());
	QDF_STATUS status;
	uint8_t err;


	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_debug("Received data req pid(%d), len(%d)",
			req->pid, req->buf_len);

	/* look for fields */
	if (req->field_info_buf)
		for (idx = 0; idx < req->field_info_buf->count; idx++) {
			offset = req->field_info_buf->fields[idx].offset;
			/*
			 * replace following reads with read_api based on
			 * length
			 */
			if (req->field_info_buf->fields[idx].id ==
					META_DATA_SUB_TYPE) {
				sub_type = *((uint32_t *)&req->buf[offset]);
				continue;
			}

			if (req->field_info_buf->fields[idx].id ==
					META_DATA_CHANNEL_MHZ) {
				channel_mhz = *((uint32_t *)&req->buf[offset]);
				continue;
			}

			if (req->field_info_buf->fields[idx].id ==
					META_DATA_PDEV) {
				tgt_pdev_id = *((uint32_t *)&req->buf[offset]);
				status = wifi_pos_get_host_pdev_id(
						psoc, tgt_pdev_id,
						&host_pdev_id);
				if (QDF_IS_STATUS_ERROR(status)) {
					wifi_pos_err("failed to get host pdev_id, tgt_pdev_id = %d",
						     tgt_pdev_id);
					return QDF_STATUS_E_INVAL;
				}
				continue;
			}
		}

	switch (sub_type) {
	case TARGET_OEM_CAPABILITY_REQ:
		/* TBD */
		break;
	case TARGET_OEM_CONFIGURE_LCR:
		/* TBD */
		break;
	case TARGET_OEM_CONFIGURE_LCI:
		/* TBD */
		break;
	case TARGET_OEM_MEASUREMENT_REQ:
		/* TBD */
		break;
	case TARGET_OEM_CONFIGURE_FTMRR:
		wifi_pos_debug("FTMRR request");
		if (wifi_pos_obj->wifi_pos_send_action)
			wifi_pos_obj->wifi_pos_send_action(psoc, sub_type,
							   req->buf,
							   req->buf_len);
		break;
	case TARGET_OEM_CONFIGURE_WRU:
		wifi_pos_debug("WRU request");
		if (wifi_pos_obj->wifi_pos_send_action)
			wifi_pos_obj->wifi_pos_send_action(psoc, sub_type,
							   req->buf,
							   req->buf_len);
		break;
	default:
		wifi_pos_debug("invalid sub type or not passed");

		tx_ops = wifi_pos_get_tx_ops(psoc);
		if (!tx_ops) {
			wifi_pos_err("tx ops null");
			return QDF_STATUS_E_INVAL;
		}

		pdev = wlan_objmgr_get_pdev_by_id(psoc, host_pdev_id,
						  WLAN_WIFI_POS_CORE_ID);
		if (!pdev) {
			wifi_pos_err("pdev null");
			return QDF_STATUS_E_INVAL;
		}

		status = ucfg_wifi_pos_measurement_request_notification(
				pdev, req);
		if (QDF_IS_STATUS_ERROR(status)) {
			err = OEM_ERR_REQUEST_REJECTED;
			wifi_pos_obj->wifi_pos_send_rsp(
					psoc, wifi_pos_get_app_pid(psoc),
					WIFI_POS_CMD_ERROR, sizeof(err), &err);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_WIFI_POS_CORE_ID);
			return QDF_STATUS_E_INVAL;
		}

		data_req.data_len = req->buf_len;
		data_req.data = req->buf;
		tx_ops->data_req_tx(pdev, &data_req);
		wlan_objmgr_pdev_release_ref(pdev,
					     WLAN_WIFI_POS_CORE_ID);
		break;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wifi_pos_process_set_cap_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	int error_code;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
				wifi_pos_get_psoc_priv_obj(psoc);
	struct wifi_pos_user_defined_caps *caps =
				(struct wifi_pos_user_defined_caps *)req->buf;

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_debug("Received set cap req pid(%d), len(%d)",
			req->pid, req->buf_len);

	wifi_pos_obj->ftm_rr = caps->ftm_rr;
	wifi_pos_obj->lci_capability = caps->lci_capability;
	error_code = qdf_status_to_os_return(QDF_STATUS_SUCCESS);
	wifi_pos_obj->wifi_pos_send_rsp(psoc, wifi_pos_obj->app_pid,
					WIFI_POS_CMD_SET_CAPS,
					sizeof(error_code),
					(uint8_t *)&error_code);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wifi_pos_process_get_cap_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	struct wifi_pos_oem_get_cap_rsp cap_rsp = { { {0} } };
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_debug("Received get cap req pid(%d), len(%d)",
		       req->pid, req->buf_len);

	wifi_pos_populate_caps(psoc, &cap_rsp.driver_cap);
	cap_rsp.user_defined_cap.ftm_rr = wifi_pos_obj->ftm_rr;
	cap_rsp.user_defined_cap.lci_capability = wifi_pos_obj->lci_capability;

	wifi_pos_obj->wifi_pos_send_rsp(psoc, wifi_pos_obj->app_pid,
					WIFI_POS_CMD_GET_CAPS,
					sizeof(cap_rsp),
					(uint8_t *)&cap_rsp);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_pos_send_report_resp(struct wlan_objmgr_psoc *psoc,
				     int req_id, uint8_t *dest_mac,
				     int err_code)
{
	struct wifi_pos_err_msg_report err_report = {0};
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	err_report.msg_tag_len = OEM_MSG_RSP_HEAD_TAG_ID << 16;
	err_report.msg_tag_len |= (sizeof(err_report) -
				   sizeof(err_report.err_rpt)) & 0x0000FFFF;
	err_report.msg_subtype = TARGET_OEM_ERROR_REPORT_RSP;
	err_report.req_id = req_id & 0xFFFF;
	err_report.req_id |= ((err_code & 0xFF) << 16);
	err_report.req_id |= (0x1 << 24);
	err_report.time_left = 0xFFFFFFFF;
	err_report.err_rpt.tag_len = OEM_MEAS_RSP_HEAD_TAG_ID << 16;
	err_report.err_rpt.tag_len |=
				(sizeof(struct wifi_pos_err_rpt)) & 0x0000FFFF;
	memcpy(&err_report.err_rpt.dest_mac, dest_mac, QDF_MAC_ADDR_SIZE);

	wifi_pos_obj->wifi_pos_send_rsp(psoc, wifi_pos_obj->app_pid,
			WIFI_POS_CMD_OEM_DATA,
			sizeof(err_report),
			(uint8_t *)&err_report);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wifi_pos_get_vht_ch_width(struct wlan_objmgr_psoc *psoc,
					    enum phy_ch_width *ch_width)
{
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

	tx_ops = wifi_pos_get_tx_ops(psoc);
	if (!tx_ops) {
		qdf_print("tx ops null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->wifi_pos_get_vht_ch_width) {
		wifi_pos_err("wifi pos get vht ch width is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return tx_ops->wifi_pos_get_vht_ch_width(
			psoc, ch_width);
}

static void wifi_update_channel_bw_info(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					uint16_t freq,
					struct wifi_pos_channel_power *chan)
{
	struct ch_params ch_params = {0};
	uint16_t sec_ch_2g = 0;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
		wifi_pos_get_psoc_priv_obj(wifi_pos_get_psoc());
	uint32_t phy_mode;
	QDF_STATUS status;

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	status = wifi_pos_get_vht_ch_width(psoc, &ch_params.ch_width);

	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("can not get vht ch width");
		return;
	}

	wlan_reg_set_channel_params_for_freq(pdev, freq,
					     sec_ch_2g, &ch_params);
	chan->band_center_freq1 = ch_params.mhz_freq_seg0;

	if (wifi_pos_psoc->wifi_pos_get_fw_phy_mode_for_freq) {
		wifi_pos_psoc->wifi_pos_get_fw_phy_mode_for_freq(
				freq, ch_params.ch_width, &phy_mode);
		chan->phy_mode = phy_mode;
	}
}

/**
 * wifi_pos_get_valid_channels: Get the list of valid channels from the
 * given channel list
 * @chan_freqs: Channel frequencies to be validated
 * @num_ch: NUmber of channels in the channel list to be validated
 * @valid_channel_list: Pointer to valid channel list
 *
 * Return: Number of valid channels in the given list
 */
static uint32_t wifi_pos_get_valid_channels(qdf_freq_t *chan_freqs,
					    uint32_t num_ch,
					    qdf_freq_t *valid_channel_list)
{
	uint32_t i, num_valid_channels = 0;

	for (i = 0; i < num_ch; i++) {
		enum channel_enum ch_enum;

		ch_enum = wlan_reg_get_chan_enum_for_freq(chan_freqs[i]);
		if (reg_is_chan_enum_invalid(ch_enum))
			continue;
		valid_channel_list[num_valid_channels++] = chan_freqs[i];
	}
	return num_valid_channels;
}

static void wifi_pos_pdev_iterator(struct wlan_objmgr_psoc *psoc,
				   void *obj, void *arg)
{
	QDF_STATUS status;
	uint8_t num_channels;
	struct wlan_objmgr_pdev *pdev = obj;
	struct wifi_pos_channel_list *chan_list = arg;
	struct channel_power *ch_info = NULL;
	struct wifi_pos_channel_power *wifi_pos_ch;
	int i;

	if (!chan_list) {
		wifi_pos_err("wifi_pos priv arg is null");
		return;
	}

	wifi_pos_ch = &chan_list->chan_info[chan_list->num_channels];

	ch_info = (struct channel_power *)qdf_mem_malloc(
			sizeof(*ch_info) * NUM_CHANNELS);
	if (!ch_info) {
		wifi_pos_err("ch_info is null");
		return;
	}

	status = wlan_reg_get_channel_list_with_power_for_freq(pdev, ch_info,
							       &num_channels);

	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("Failed to get valid channel list");
		qdf_mem_free(ch_info);
		return;
	}

	if ((chan_list->num_channels + num_channels) > NUM_CHANNELS) {
		wifi_pos_err("Invalid number of channels");
		qdf_mem_free(ch_info);
		return;
	}

	for (i = 0; i < num_channels; i++) {
		wifi_pos_ch[i].ch_power.center_freq = ch_info[i].center_freq;
		wifi_pos_ch[i].ch_power.chan_num = ch_info[i].chan_num;
		wifi_pos_ch[i].ch_power.tx_power = ch_info[i].tx_power;
		wifi_pos_ch[i].is_dfs_chan =
			wlan_reg_is_dfs_for_freq(pdev, ch_info[i].center_freq);
		wifi_update_channel_bw_info(
				psoc, pdev,
				ch_info[i].center_freq, &wifi_pos_ch[i]);
	}

	chan_list->num_channels += num_channels;
	qdf_mem_free(ch_info);
}

#ifdef CNSS_GENL
static bool wifi_pos_is_resp_version_valid(uint32_t rsp_version)
{
	return (rsp_version == WIFI_POS_RSP_V2_NL) ? true : false;
}

static void wifi_pos_get_ch_info(struct wlan_objmgr_psoc *psoc,
				 struct wifi_pos_channel_list *chan_list)
{
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wifi_pos_pdev_iterator,
				     chan_list, true, WLAN_WIFI_POS_CORE_ID);
	wifi_pos_debug("num channels: %d", chan_list->num_channels);
}

#else
static bool wifi_pos_is_resp_version_valid(uint32_t rsp_version)
{
	return ((rsp_version == WIFI_POS_RSP_V2_NL) ||
		(rsp_version == WIFI_POS_RSP_V1_FLAT_MEMORY)) ?
		true : false;
}

static void wifi_pos_get_ch_info(struct wlan_objmgr_psoc *psoc,
				 struct wifi_pos_channel_list *chan_list)
{
	uint8_t index;

	for (index = 0; index < WLAN_OBJMGR_MAX_DEVICES; index++) {
		if (g_umac_glb_obj->psoc[index]) {
			wlan_objmgr_iterate_obj_list(
					g_umac_glb_obj->psoc[index],
					WLAN_PDEV_OP, wifi_pos_pdev_iterator,
					chan_list, true, WLAN_WIFI_POS_CORE_ID);
		}
	}

	wifi_pos_notice("num channels: %d", chan_list->num_channels);
}
#endif

static QDF_STATUS wifi_pos_process_ch_info_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	uint8_t idx;
	uint8_t *buf = NULL;
	uint32_t len, i, freq;
	qdf_freq_t *chan_freqs = NULL;
	bool oem_6g_support_disable;
	uint8_t *channels = req->buf;
	struct wlan_objmgr_pdev *pdev;
	uint32_t num_ch = req->buf_len;
	qdf_freq_t valid_channel_list[NUM_CHANNELS];
	uint32_t num_valid_channels = 0;
	struct wifi_pos_ch_info_rsp *ch_info;
	struct wifi_pos_channel_list *ch_list = NULL;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);
	QDF_STATUS ret_val;
	struct wifi_pos_channel_power *ch;

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_debug("Received ch info req pid(%d), len(%d)",
			req->pid, req->buf_len);

	/* get first pdev since we need that only for freq and dfs state */
	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0, WLAN_WIFI_POS_CORE_ID);
	if (!pdev) {
		wifi_pos_err("pdev get API failed");
		return QDF_STATUS_E_INVAL;
	}
	if (num_ch > NUM_CHANNELS) {
		wifi_pos_err("Invalid number of channels");
		ret_val = QDF_STATUS_E_INVAL;
		goto cleanup;
	}

	chan_freqs = qdf_mem_malloc(NUM_CHANNELS * (sizeof(*chan_freqs)));
	if (!chan_freqs) {
		ret_val = QDF_STATUS_E_NOMEM;
		goto cleanup;
	}

	ch_list = qdf_mem_malloc(sizeof(*ch_list));
	if (!ch_list) {
		ret_val = QDF_STATUS_E_NOMEM;
		goto cleanup;
	}

	ch = ch_list->chan_info;

	if ((num_ch == 0) &&
	    wifi_pos_is_resp_version_valid(req->rsp_version)) {
		wifi_pos_get_ch_info(psoc, ch_list);
		qdf_spin_lock_bh(&wifi_pos_obj->wifi_pos_lock);
		oem_6g_support_disable = wifi_pos_obj->oem_6g_support_disable;
		qdf_spin_unlock_bh(&wifi_pos_obj->wifi_pos_lock);

		/* ch_list has the frequencies in order of 2.4g, 5g & 6g */
		for (i = 0; i < ch_list->num_channels; i++) {
			freq = ch[i].ch_power.center_freq;
			if (oem_6g_support_disable &&
			    WLAN_REG_IS_6GHZ_CHAN_FREQ(freq))
				continue;
			num_valid_channels++;
		}
	} else {
		for (i = 0; i < NUM_CHANNELS; i++)
			chan_freqs[i] =
			    wlan_reg_chan_band_to_freq(pdev, channels[i],
						       BIT(REG_BAND_5G) |
						       BIT(REG_BAND_2G));
		/* v1 has ch_list with frequencies in order of 2.4g, 5g only */
		num_valid_channels = wifi_pos_get_valid_channels(
							chan_freqs, num_ch,
							 valid_channel_list);
		for (i = 0; i < num_valid_channels; i++) {
			ch[i].ch_power.center_freq = valid_channel_list[i];
			ch[i].ch_power.chan_num = wlan_reg_freq_to_chan(
					pdev, ch[i].ch_power.center_freq);
			ch[i].ch_power.tx_power =
				wlan_reg_get_channel_reg_power_for_freq(
						pdev,
						ch[i].ch_power.center_freq);
			ch[i].is_dfs_chan = wlan_reg_is_dfs_for_freq(
						pdev,
						ch[i].ch_power.center_freq);

			wifi_update_channel_bw_info(psoc, pdev,
						    ch[i].ch_power.center_freq,
						    &ch[i]);
		}
	}

	len = sizeof(uint8_t) + sizeof(struct wifi_pos_ch_info_rsp) *
			num_valid_channels;
	buf = qdf_mem_malloc(len);
	if (!buf) {
		ret_val = QDF_STATUS_E_NOMEM;
		goto cleanup;
	}

	/* First byte of message body will have num of channels */
	buf[0] = num_valid_channels;
	ch_info = (struct wifi_pos_ch_info_rsp *)&buf[1];
	for (idx = 0; idx < num_valid_channels; idx++) {
		ch_info[idx].reserved0 = 0;
		ch_info[idx].chan_id = ch[idx].ch_power.chan_num;
		ch_info[idx].mhz = ch[idx].ch_power.center_freq;
		ch_info[idx].band_center_freq1 = ch[idx].band_center_freq1;
		ch_info[idx].band_center_freq2 = 0;
		ch_info[idx].info = 0;

		REG_SET_CHANNEL_REG_POWER(ch_info[idx].reg_info_1,
					  ch[idx].ch_power.tx_power);
		REG_SET_CHANNEL_MAX_TX_POWER(ch_info[idx].reg_info_2,
					     ch[idx].ch_power.tx_power);

		if (ch[i].is_dfs_chan)
			WIFI_POS_SET_DFS(ch_info[idx].info);

		if (ch[i].phy_mode)
			REG_SET_CHANNEL_MODE(&ch_info[idx], ch[i].phy_mode);
	}

	wifi_pos_obj->wifi_pos_send_rsp(psoc, wifi_pos_obj->app_pid,
					WIFI_POS_CMD_GET_CH_INFO,
					len, buf);
	ret_val = QDF_STATUS_SUCCESS;

cleanup:
	qdf_mem_free(buf);
	qdf_mem_free(ch_list);
	qdf_mem_free(chan_freqs);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_WIFI_POS_CORE_ID);

	return ret_val;
}

static void wifi_pos_vdev_iterator(struct wlan_objmgr_psoc *psoc,
				   void *vdev, void *arg)
{
	struct app_reg_rsp_vdev_info *vdev_info = arg;

	vdev_info[vdev_idx].dev_mode = wlan_vdev_mlme_get_opmode(vdev);
	vdev_info[vdev_idx].vdev_id = wlan_vdev_get_id(vdev);
	vdev_idx++;
}

#ifdef CNSS_GENL
static void wifi_pos_get_vdev_list(struct wlan_objmgr_psoc *psoc,
				   struct app_reg_rsp_vdev_info *vdevs_info)
{
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wifi_pos_vdev_iterator,
				     vdevs_info, true, WLAN_WIFI_POS_CORE_ID);
}
#else
/* For WIN, WIFI POS command registration is called only for the first
 * PSOC. Hence, iterate through all the PSOCs and send the vdev list
 * to LOWI.
 */
static void wifi_pos_get_vdev_list(struct wlan_objmgr_psoc *psoc,
				   struct app_reg_rsp_vdev_info *vdevs_info)
{
	uint8_t index;

	for (index = 0; index < WLAN_OBJMGR_MAX_DEVICES; index++) {
		if (g_umac_glb_obj->psoc[index]) {
			wlan_objmgr_iterate_obj_list(
					g_umac_glb_obj->psoc[index],
					WLAN_VDEV_OP, wifi_pos_vdev_iterator,
					vdevs_info, true,
					WLAN_WIFI_POS_CORE_ID);
		}
	}
}
#endif

static QDF_STATUS wifi_pos_process_app_reg_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t err = 0, *app_reg_rsp;
	uint32_t rsp_len;
	char *sign_str = NULL;
	struct app_reg_rsp_vdev_info vdevs_info[WLAN_UMAC_PSOC_MAX_VDEVS]
								= { { 0 } };
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_debug("Received App Req Req pid(%d), len(%d)",
			req->pid, req->buf_len);

	sign_str = (char *)req->buf;
	/* Registration request is only allowed for QTI Application */
	if ((OEM_APP_SIGNATURE_LEN != req->buf_len) ||
		(strncmp(sign_str, OEM_APP_SIGNATURE_STR,
			 OEM_APP_SIGNATURE_LEN))) {
		wifi_pos_err("Invalid signature pid(%d)", req->pid);
		ret = QDF_STATUS_E_PERM;
		err = OEM_ERR_INVALID_SIGNATURE;
		goto app_reg_failed;
	}

	wifi_pos_debug("Valid App Req Req from pid(%d)", req->pid);
	qdf_spin_lock_bh(&wifi_pos_obj->wifi_pos_lock);
	wifi_pos_obj->is_app_registered = true;
	wifi_pos_obj->app_pid = req->pid;
	qdf_spin_unlock_bh(&wifi_pos_obj->wifi_pos_lock);

	vdev_idx = 0;

	wifi_pos_get_vdev_list(psoc, vdevs_info);

	app_reg_rsp = wifi_pos_prepare_reg_resp(&rsp_len, vdevs_info);
	if (!app_reg_rsp) {
		ret = QDF_STATUS_E_NOMEM;
		err = OEM_ERR_NULL_CONTEXT;
		goto app_reg_failed;
	}

	if (!vdev_idx)
		wifi_pos_debug("no active vdev");

	vdev_idx = 0;
	wifi_pos_obj->wifi_pos_send_rsp(psoc, req->pid,
					WIFI_POS_CMD_REGISTRATION,
					rsp_len, (uint8_t *)app_reg_rsp);

	qdf_mem_free(app_reg_rsp);
	return ret;

app_reg_failed:

	wifi_pos_obj->wifi_pos_send_rsp(psoc, req->pid, WIFI_POS_CMD_ERROR,
					sizeof(err), &err);
	return ret;
}

/**
 * wifi_pos_tlv_callback: wifi pos msg handler registered for TLV type req
 * @wmi_msg: wmi type request msg
 *
 * Return: status of operation
 */
static QDF_STATUS wifi_pos_tlv_callback(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	wifi_pos_debug("enter: msg_type: %d", req->msg_type);
	switch (req->msg_type) {
	case WIFI_POS_CMD_REGISTRATION:
		return wifi_pos_process_app_reg_req(psoc, req);
	case WIFI_POS_CMD_OEM_DATA:
		return wifi_pos_process_data_req(psoc, req);
	case WIFI_POS_CMD_GET_CH_INFO:
		return wifi_pos_process_ch_info_req(psoc, req);
	case WIFI_POS_CMD_SET_CAPS:
		return wifi_pos_process_set_cap_req(psoc, req);
	case WIFI_POS_CMD_GET_CAPS:
		return wifi_pos_process_get_cap_req(psoc, req);
	default:
		wifi_pos_err("invalid request type");
		break;
	}
	return 0;
}

/**
 * wifi_pos_non_tlv_callback: wifi pos msg handler registered for non-TLV
 * type req
 * @wmi_msg: wmi type request msg
 *
 * Return: status of operation
 */
static QDF_STATUS wifi_pos_non_tlv_callback(struct wlan_objmgr_psoc *psoc,
					    struct wifi_pos_req_msg *req)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_pos_convert_host_pdev_id_to_target(
		struct wlan_objmgr_psoc *psoc, uint32_t host_pdev_id,
		uint32_t *target_pdev_id)
{
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

	tx_ops = wifi_pos_get_tx_ops(psoc);
	if (!tx_ops) {
		wifi_pos_err("tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!tx_ops->wifi_pos_convert_pdev_id_host_to_target) {
		wifi_pos_err("wifi_pos_convert_pdev_id_host_to_target is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return tx_ops->wifi_pos_convert_pdev_id_host_to_target(
			psoc, host_pdev_id, target_pdev_id);
}

QDF_STATUS wifi_pos_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj;

	/*
	 * this is for WIN, if they have multiple psoc, we dont want to create
	 * multiple priv object. Since there is just one LOWI app registered to
	 * one driver, avoid 2nd private object with another psoc.
	 */
	if (wifi_pos_get_psoc()) {
		wifi_pos_debug("global psoc obj already set. do not allocate another psoc private object");
		return QDF_STATUS_SUCCESS;
	} else {
		wifi_pos_debug("setting global pos object");
		wifi_pos_set_psoc(psoc);
	}

	/* initialize wifi-pos psoc priv object */
	wifi_pos_obj = qdf_mem_malloc(sizeof(*wifi_pos_obj));
	if (!wifi_pos_obj) {
		wifi_pos_clear_psoc();
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&wifi_pos_obj->wifi_pos_lock);
	/* Register TLV or non-TLV callbacks depending on target fw version */
	if (wifi_pos_get_tlv_support(psoc))
		wifi_pos_obj->wifi_pos_req_handler = wifi_pos_tlv_callback;
	else
		wifi_pos_obj->wifi_pos_req_handler = wifi_pos_non_tlv_callback;

	/*
	 * MGMT Rx is not handled in this phase since wifi pos only uses few
	 * measurement subtypes under RRM_RADIO_MEASURE_REQ. Rest of them are
	 * used for 80211k. That part is not yet converged and still follows
	 * legacy MGMT Rx to work. Action frame in new TXRX can be registered
	 * at per ACTION Frame type granularity only.
	 */

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						WLAN_UMAC_COMP_WIFI_POS,
						wifi_pos_obj,
						QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("obj attach with psoc failed with status: %d",
				status);
		qdf_spinlock_destroy(&wifi_pos_obj->wifi_pos_lock);
		qdf_mem_free(wifi_pos_obj);
		wifi_pos_clear_psoc();
	}

	return status;
}

QDF_STATUS  wifi_pos_psoc_obj_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj = NULL;

	if (wifi_pos_get_psoc() == psoc) {
		wifi_pos_debug("deregistering wifi_pos_psoc object");
		wifi_pos_clear_psoc();
	} else {
		wifi_pos_warn("un-related PSOC closed. do nothing");
		return QDF_STATUS_SUCCESS;
	}

	wifi_pos_obj = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos_obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	target_if_wifi_pos_deinit_dma_rings(psoc);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						WLAN_UMAC_COMP_WIFI_POS,
						wifi_pos_obj);
	if (status != QDF_STATUS_SUCCESS)
		wifi_pos_err("wifi_pos_obj detach failed");

	wifi_pos_debug("wifi_pos_obj deleted with status %d", status);
	qdf_spinlock_destroy(&wifi_pos_obj->wifi_pos_lock);
	qdf_mem_free(wifi_pos_obj);

	return status;
}

int wifi_pos_oem_rsp_handler(struct wlan_objmgr_psoc *psoc,
			     struct oem_data_rsp *oem_rsp)
{
	uint32_t len;
	uint8_t *data;
	uint32_t app_pid;
	struct wifi_pos_psoc_priv_obj *priv;
	wifi_pos_send_rsp_handler wifi_pos_send_rsp;

	priv = wifi_pos_get_psoc_priv_obj(wifi_pos_get_psoc());
	if (!priv) {
		wifi_pos_err("private object is NULL");
		return -EINVAL;
	}

	qdf_spin_lock_bh(&priv->wifi_pos_lock);
	app_pid = priv->app_pid;
	wifi_pos_send_rsp = priv->wifi_pos_send_rsp;
	qdf_spin_unlock_bh(&priv->wifi_pos_lock);

	len = oem_rsp->rsp_len_1 + oem_rsp->rsp_len_2 + oem_rsp->dma_len;
	if (oem_rsp->rsp_len_1 > OEM_DATA_RSP_SIZE ||
			oem_rsp->rsp_len_2 > OEM_DATA_RSP_SIZE) {
		wifi_pos_err("invalid length of Oem Data response");
		return -EINVAL;
	}

	if (!wifi_pos_send_rsp) {
		wifi_pos_err("invalid response handler");
		return -EINVAL;
	}

	wifi_pos_debug("oem data rsp, len: %d to pid: %d", len, app_pid);

	if (oem_rsp->rsp_len_2 + oem_rsp->dma_len) {
		/* stitch togther the msg data_1 + CIR/CFR + data_2 */
		data = qdf_mem_malloc(len);
		if (!data)
			return -ENOMEM;

		qdf_mem_copy(data, oem_rsp->data_1, oem_rsp->rsp_len_1);
		qdf_mem_copy(&data[oem_rsp->rsp_len_1],
			     oem_rsp->vaddr, oem_rsp->dma_len);
		qdf_mem_copy(&data[oem_rsp->rsp_len_1 + oem_rsp->dma_len],
			     oem_rsp->data_2, oem_rsp->rsp_len_2);

		wifi_pos_send_rsp(psoc, app_pid, WIFI_POS_CMD_OEM_DATA, len,
				  data);
		qdf_mem_free(data);
	} else {
		wifi_pos_send_rsp(psoc, app_pid, WIFI_POS_CMD_OEM_DATA,
				  oem_rsp->rsp_len_1, oem_rsp->data_1);
	}

	return 0;
}

void wifi_pos_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_wifi_pos_rx_ops *wifi_pos_rx_ops;

	wifi_pos_rx_ops = &rx_ops->wifi_pos_rx_ops;
	wifi_pos_rx_ops->oem_rsp_event_rx = wifi_pos_oem_rsp_handler;
}

QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
			   struct wifi_pos_driver_caps *caps)
{
	uint16_t i, count = 0;
	uint32_t freq;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);
	struct wifi_pos_channel_list *ch_list = NULL;

	wifi_pos_debug("Enter");
	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	ch_list = qdf_mem_malloc(sizeof(*ch_list));
	if (!ch_list)
		return QDF_STATUS_E_NOMEM;

	strlcpy(caps->oem_target_signature,
		OEM_TARGET_SIGNATURE,
		OEM_TARGET_SIGNATURE_LEN);
	caps->oem_target_type = wifi_pos_obj->oem_target_type;
	caps->oem_fw_version = wifi_pos_obj->oem_fw_version;
	caps->driver_version.major = wifi_pos_obj->driver_version.major;
	caps->driver_version.minor = wifi_pos_obj->driver_version.minor;
	caps->driver_version.patch = wifi_pos_obj->driver_version.patch;
	caps->driver_version.build = wifi_pos_obj->driver_version.build;
	caps->allowed_dwell_time_min = wifi_pos_obj->allowed_dwell_time_min;
	caps->allowed_dwell_time_max = wifi_pos_obj->allowed_dwell_time_max;
	caps->curr_dwell_time_min = wifi_pos_obj->current_dwell_time_min;
	caps->curr_dwell_time_max = wifi_pos_obj->current_dwell_time_max;
	caps->supported_bands = wlan_objmgr_psoc_get_band_capability(psoc);
	wifi_pos_get_ch_info(psoc, ch_list);

	/* copy valid channels list to caps */
	for (i = 0; i < ch_list->num_channels; i++) {
		freq = ch_list->chan_info[i].ch_power.center_freq;
		if (WLAN_REG_IS_6GHZ_CHAN_FREQ(freq))
			continue;
		caps->channel_list[count++] =
			ch_list->chan_info[i].ch_power.chan_num;
	}
	caps->num_channels = count;
	qdf_mem_free(ch_list);
	return QDF_STATUS_SUCCESS;
}
