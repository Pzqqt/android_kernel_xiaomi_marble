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
 * DOC: wifi_pos_main.c
 * This file defines the important functions pertinent to
 * wifi positioning to initialize and de-initialize the component.
 */
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

static QDF_STATUS wifi_pos_process_data_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	uint8_t idx;
	uint32_t sub_type = 0;
	uint32_t channel_mhz = 0;
	void *pdev_id = NULL;
	uint32_t offset;
	struct oem_data_req data_req;
	struct wlan_lmac_if_wifi_pos_tx_ops *tx_ops;

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
					WMIRTT_FIELD_ID_oem_data_sub_type) {
				sub_type = *((uint32_t *)&req->buf[offset]);
				continue;
			}

			if (req->field_info_buf->fields[idx].id ==
					WMIRTT_FIELD_ID_channel_mhz) {
				channel_mhz = *((uint32_t *)&req->buf[offset]);
				continue;
			}

			if (req->field_info_buf->fields[idx].id ==
					WMIRTT_FIELD_ID_pdev) {
				pdev_id = &req->buf[offset];
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
		/* TBD */
		break;
	case TARGET_OEM_CONFIGURE_WRU:
		/* TBD */
		break;
	default:
		wifi_pos_debug("invalid sub type or not passed");
		/*
		 * this is legacy MCL operation. pass whole msg to firmware as
		 * it is.
		 */
		tx_ops = target_if_wifi_pos_get_txops(psoc);
		if (!tx_ops) {
			wifi_pos_err("tx ops null");
			return QDF_STATUS_E_INVAL;
		}
		data_req.data_len = req->buf_len;
		data_req.data = req->buf;
		tx_ops->data_req_tx(psoc, &data_req);
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
	wifi_pos_obj->wifi_pos_send_rsp(wifi_pos_obj->app_pid,
					ANI_MSG_SET_OEM_CAP_RSP,
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
	wifi_pos_obj->wifi_pos_send_rsp(wifi_pos_obj->app_pid,
					ANI_MSG_GET_OEM_CAP_RSP,
					sizeof(cap_rsp),
					(uint8_t *)&cap_rsp);

	return QDF_STATUS_SUCCESS;
}

static void wifi_update_channel_bw_info(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					uint16_t chan,
					struct wifi_pos_ch_info_rsp *chan_info)
{
	struct ch_params ch_params = {0};
	uint16_t sec_ch_2g = 0;
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc =
		wifi_pos_get_psoc_priv_obj(psoc);
	uint32_t phy_mode;

	if (!wifi_pos_psoc) {
		wifi_pos_err("wifi_pos priv obj is null");
		return;
	}

	/* Passing CH_WIDTH_MAX will give the max bandwidth supported */
	ch_params.ch_width = CH_WIDTH_MAX;

	wlan_reg_set_channel_params(pdev, chan, sec_ch_2g, &ch_params);
	if (ch_params.center_freq_seg0)
		chan_info->band_center_freq1 =
			wlan_reg_get_channel_freq(pdev,
						  ch_params.center_freq_seg0);

	wifi_pos_psoc->wifi_pos_get_phy_mode(chan, ch_params.ch_width,
					     &phy_mode);

	REG_SET_CHANNEL_MODE(chan_info, phy_mode);
}

static void wifi_pos_get_reg_info(struct wlan_objmgr_pdev *pdev,
				  uint32_t chan_num, uint32_t *reg_info_1,
				  uint32_t *reg_info_2)
{
	uint32_t reg_power = wlan_reg_get_channel_reg_power(pdev, chan_num);

	*reg_info_1 = 0;
	*reg_info_2 = 0;

	REG_SET_CHANNEL_REG_POWER(*reg_info_1, reg_power);
	REG_SET_CHANNEL_MAX_TX_POWER(*reg_info_2, reg_power);
}

/**
 * wifi_pos_get_valid_channels: Get the list of valid channels from the
 * given channel list
 * @channels: Channel list to be validated
 * @num_ch: NUmber of channels in the channel list to be validated
 * @valid_channel_list: Pointer to valid channel list
 *
 * Return: Number of valid channels in the given list
 */

static uint32_t wifi_pos_get_valid_channels(uint8_t *channels, uint32_t num_ch,
					    uint8_t *valid_channel_list) {
	uint32_t i, num_valid_channels = 0;

	for (i = 0; i < num_ch; i++) {
		if (wlan_reg_get_chan_enum(channels[i]) == INVALID_CHANNEL)
			continue;
		valid_channel_list[num_valid_channels++] = channels[i];
	}
	return num_valid_channels;
}

static QDF_STATUS wifi_pos_process_ch_info_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	uint8_t idx;
	uint8_t *buf;
	uint32_t len;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
	uint8_t *channels = req->buf;
	struct wlan_objmgr_pdev *pdev;
	uint32_t num_ch = req->buf_len;
	uint8_t valid_channel_list[NUM_CHANNELS];
	uint32_t num_valid_channels;
	struct wifi_pos_ch_info_rsp *ch_info;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

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
		return QDF_STATUS_E_INVAL;
	}
	num_valid_channels = wifi_pos_get_valid_channels(channels, num_ch,
							 valid_channel_list);

	len = sizeof(uint8_t) + sizeof(struct wifi_pos_ch_info_rsp) *
			num_valid_channels;
	buf = qdf_mem_malloc(len);
	if (!buf) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_WIFI_POS_CORE_ID);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(buf, len);

	/* First byte of message body will have num of channels */
	buf[0] = num_valid_channels;
	ch_info = (struct wifi_pos_ch_info_rsp *)&buf[1];
	for (idx = 0; idx < num_valid_channels; idx++) {
		ch_info[idx].chan_id = valid_channel_list[idx];
		wifi_pos_get_reg_info(pdev, ch_info[idx].chan_id,
				      &reg_info_1, &reg_info_2);
		ch_info[idx].reserved0 = 0;
		ch_info[idx].mhz = wlan_reg_get_channel_freq(
						pdev,
						valid_channel_list[idx]);
		ch_info[idx].band_center_freq1 = ch_info[idx].mhz;
		ch_info[idx].band_center_freq2 = 0;
		ch_info[idx].info = 0;
		if (wlan_reg_is_dfs_ch(pdev, valid_channel_list[idx]))
			WIFI_POS_SET_DFS(ch_info[idx].info);

		wifi_update_channel_bw_info(psoc, pdev,
					    ch_info[idx].chan_id,
					    &ch_info[idx]);

		ch_info[idx].reg_info_1 = reg_info_1;
		ch_info[idx].reg_info_2 = reg_info_2;
	}

	wifi_pos_obj->wifi_pos_send_rsp(wifi_pos_obj->app_pid,
					ANI_MSG_CHANNEL_INFO_RSP,
					len, buf);
	qdf_mem_free(buf);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_WIFI_POS_CORE_ID);

	return QDF_STATUS_SUCCESS;
}

static void wifi_pos_vdev_iterator(struct wlan_objmgr_psoc *psoc,
				   void *vdev, void *arg)
{
	struct app_reg_rsp_vdev_info *vdev_info = arg;

	vdev_info[vdev_idx].dev_mode = wlan_vdev_mlme_get_opmode(vdev);
	vdev_info[vdev_idx].vdev_id = wlan_vdev_get_id(vdev);
	vdev_idx++;
}

static QDF_STATUS wifi_pos_process_app_reg_req(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint8_t err = 0;
	uint32_t rsp_len;
	char *sign_str = NULL;
	struct wifi_app_reg_rsp *app_reg_rsp;
	struct app_reg_rsp_vdev_info vdevs_info[WLAN_UMAC_PSOC_MAX_VDEVS]
								= { { 0 } };
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
			wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return QDF_STATUS_E_INVAL;
	}

	wifi_pos_err("Received App Req Req pid(%d), len(%d)",
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
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wifi_pos_vdev_iterator,
				     vdevs_info, true, WLAN_WIFI_POS_CORE_ID);
	rsp_len = (sizeof(struct app_reg_rsp_vdev_info) * vdev_idx)
			+ sizeof(uint8_t);
	app_reg_rsp = qdf_mem_malloc(rsp_len);
	if (!app_reg_rsp) {
		ret = QDF_STATUS_E_NOMEM;
		err = OEM_ERR_NULL_CONTEXT;
		goto app_reg_failed;
	}

	app_reg_rsp->num_inf = vdev_idx;
	qdf_mem_copy(&app_reg_rsp->vdevs, vdevs_info,
		     sizeof(struct app_reg_rsp_vdev_info) * vdev_idx);
	if (!vdev_idx)
		wifi_pos_debug("no active vdev");

	vdev_idx = 0;
	wifi_pos_obj->wifi_pos_send_rsp(req->pid, ANI_MSG_APP_REG_RSP,
					rsp_len, (uint8_t *)app_reg_rsp);

	qdf_mem_free(app_reg_rsp);
	return ret;

app_reg_failed:

	wifi_pos_obj->wifi_pos_send_rsp(req->pid, ANI_MSG_OEM_ERROR,
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
	case ANI_MSG_APP_REG_REQ:
		return wifi_pos_process_app_reg_req(psoc, req);
	case ANI_MSG_OEM_DATA_REQ:
		return wifi_pos_process_data_req(psoc, req);
	case ANI_MSG_CHANNEL_INFO_REQ:
		return wifi_pos_process_ch_info_req(psoc, req);
	case ANI_MSG_SET_OEM_CAP_REQ:
		return wifi_pos_process_set_cap_req(psoc, req);
	case ANI_MSG_GET_OEM_CAP_REQ:
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
	struct wifi_pos_psoc_priv_obj *priv =
					wifi_pos_get_psoc_priv_obj(psoc);
	void (*wifi_pos_send_rsp)(uint32_t, uint32_t, uint32_t, uint8_t *);

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

		wifi_pos_send_rsp(app_pid, ANI_MSG_OEM_DATA_RSP, len, data);
		qdf_mem_free(data);
	} else {
		wifi_pos_send_rsp(app_pid, ANI_MSG_OEM_DATA_RSP,
				  oem_rsp->rsp_len_1, oem_rsp->data_1);
	}

	return 0;
}

static void wifi_pos_pdev_iterator(struct wlan_objmgr_psoc *psoc,
				   void *obj, void *arg)
{
	QDF_STATUS status;
	uint8_t i, num_channels;
	struct wlan_objmgr_pdev *pdev = obj;
	struct wifi_pos_driver_caps *caps = arg;
	struct channel_power ch_list[NUM_CHANNELS];

	status = wlan_reg_get_channel_list_with_power(pdev, ch_list,
						      &num_channels);

	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("Failed to get valid channel list");
		return;
	}
	for (i = 0; i < num_channels; i++)
		caps->channel_list[i] = ch_list[i].chan_num;
	caps->num_channels = num_channels;
}

static void wifi_pos_get_ch_info(struct wlan_objmgr_psoc *psoc,
				 struct wifi_pos_driver_caps *caps)
{
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wifi_pos_pdev_iterator,
				      caps, true, WLAN_WIFI_POS_CORE_ID);
	wifi_pos_err("num channels: %d", caps->num_channels);
}

QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
			   struct wifi_pos_driver_caps *caps)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	wifi_pos_debug("Enter");
	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

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
	wifi_pos_get_ch_info(psoc, caps);
	return QDF_STATUS_SUCCESS;
}
